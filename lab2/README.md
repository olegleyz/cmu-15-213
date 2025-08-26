# Lab 2: Bomb Lab

This lab involves analyzing and defusing a compiled x86-64 Linux binary (“the bomb”) on a Mac with Apple Silicon (M3 Pro, ARM64).  

---

## Environment Setup

### Problem

The binary is incompatible with macOS and ARM64:

1. Linux binary → macOS cannot execute it natively.
2. x86-64 binary → Apple Silicon is ARM64.

### Solution

I chose **Docker** for simplicity:

```bash
docker run --rm -it --platform linux/amd64 -v .:/work ubuntu:20.04 bash
````

**Explanation of flags:**

* `docker run`: start a new container from an image.
* `--rm`: automatically remove the container when it exits.
* `-it`: interactive terminal (`-i` keeps STDIN open, `-t` allocates a TTY).
* `--platform linux/amd64`: emulate x86-64 Linux on Apple Silicon using QEMU.
* `-v .:/work`: mount current directory into container at `/work`.
* `ubuntu:20.04`: Ubuntu 20.04 image.
* `bash`: start a shell inside the container.

Inside Docker, I have a Linux x86-64 environment to execute the bomb.

---

## Tools Installation

Inside the container:

```bash
apt-get update
apt-get install gdb gcc make
```

---

## Disassembly

On Mac:

```bash
objdump -d bomb > bomb.d
```

* `-d`: disassemble all code sections.

To inspect read-only data (`.rodata` section):

```bash
objdump -s -j .rodata bomb
```

Or in GDB:

```gdb
x/s 0x402400
```

---

## Phase 1 Analysis

Disassembled code snippet:

```
400ee4: movl $0x402400, %esi
400ee9: callq 0x401338 <strings_not_equal>
400ef2: callq 0x40143a <explode_bomb>
```

### Observations

1. **Arguments via calling convention** (x86-64 Linux, System V ABI):

| Argument | Register |
| -------- | -------- |
| 1        | %rdi     |
| 2        | %rsi     |
| 3        | %rdx     |
| 4        | %rcx     |
| 5        | %r8      |
| 6        | %r9      |
| return   | %rax     |

2. **Instruction sizes**:

| Mnemonic | Size | Example         |
| -------- | ---- | --------------- |
| `movb`   | 1B   | `movb $1, %al`  |
| `movw`   | 2B   | `movw $1, %ax`  |
| `movl`   | 4B   | `movl $1, %eax` |
| `movq`   | 8B   | `movq $1, %rax` |

3. `%rdi` receives user input; `%rsi` points to a string literal at `0x402400` (read-only data baked into the binary).

4. Reading the string:
```
402400 426f7264 65722072 656c6174 696f6e73  Border relations
402410 20776974 68204361 6e616461 20686176   with Canada hav
402420 65206e65 76657220 6265656e 20626574  e never been bet
402430 7465722e 00000000 576f7721 20596f75  ter.....Wow! You
```


```text
0x402400: "Border relations with Canada have never been better."
```

5. Defusing phase 1:

```bash
root@container:/work# ./bomb
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.
Phase 1 defused. How about the next one?
```


---

## Phase 2 Analysis

Disassembly snippet:

```
0000000000400efc <phase_2>:
  400efc: 55                           	pushq	%rbp
  400efd: 53                           	pushq	%rbx
  400efe: 48 83 ec 28                  	subq	$0x28, %rsp // %rsp is a CPU register that points to the top of the stack (region of memory holding local variables, etc.). This line allocates 0x28 (40) bytes in a stack for local variables. 
  400f02: 48 89 e6                     	movq	%rsp, %rsi // set the 2nd argument %rsi for the read_six_numbers
  400f05: e8 52 05 00 00               	callq	0x40145c <read_six_numbers> // read_six_numbers will read 6 ints starting at %rsp
  400f0a: 83 3c 24 01                  	cmpl	$0x1, (%rsp) // arr[0] == 1?
  400f0e: 74 20                        	je	0x400f30 <phase_2+0x34>
  400f10: e8 25 05 00 00               	callq	0x40143a <explode_bomb>
  400f15: eb 19                        	jmp	0x400f30 <phase_2+0x34> // jump to "leaq	0x4(%rsp), %rbx"
  400f17: 8b 43 fc                     	movl	-0x4(%rbx), %eax // eax = arr[i-1]
  400f1a: 01 c0                        	addl	%eax, %eax // %eax = %eax + %eax (2*arr[i-1])
  400f1c: 39 03                        	cmpl	%eax, (%rbx) // arr[i] == arr[i-1] * 2. I got it know, numbers double
  400f1e: 74 05                        	je	0x400f25 <phase_2+0x29>
  400f20: e8 15 05 00 00               	callq	0x40143a <explode_bomb>
  400f25: 48 83 c3 04                  	addq	$0x4, %rbx
  400f29: 48 39 eb                     	cmpq	%rbp, %rbx
  400f2c: 75 e9                        	jne	0x400f17 <phase_2+0x1b>
  400f2e: eb 0c                        	jmp	0x400f3c <phase_2+0x40>
  400f30: 48 8d 5c 24 04               	leaq	0x4(%rsp), %rbx // %rbx = %rsp+0x4 (arr[1])
  400f35: 48 8d 6c 24 18               	leaq	0x18(%rsp), %rbp // %rbp = %rsp+0x18 (end of arr)
  400f3a: eb db                        	jmp	0x400f17 <phase_2+0x1b> // jump back to "movl	-0x4(%rbx), %eax"
  400f3c: 48 83 c4 28                  	addq	$0x28, %rsp
  400f40: 5b                           	popq	%rbx
  400f41: 5d                           	popq	%rbp
  400f42: c3                           	retq
```

And let's look at the read_six_numbers:
```
000000000040145c <read_six_numbers>:
  40145c: 48 83 ec 18                  	subq	$0x18, %rsp // allocates 24 bytes in the stack
  401460: 48 89 f2                     	movq	%rsi, %rdx // stores a pointer to phase_2's stack into %rdx (arr[0])
  401463: 48 8d 4e 04                  	leaq	0x4(%rsi), %rcx // %rcx = %rsi + 0x4
  401467: 48 8d 46 14                  	leaq	0x14(%rsi), %rax // %rax = %rsi + 0x14
  40146b: 48 89 44 24 08               	movq	%rax, 0x8(%rsp) // Take the 64-bit value in %rax and store it in memory at the address %rsp + 8
  401470: 48 8d 46 10                  	leaq	0x10(%rsi), %rax //%rax = %rsi+0x10
  401474: 48 89 04 24                  	movq	%rax, (%rsp) // %rsp = %rax
  401478: 4c 8d 4e 0c                  	leaq	0xc(%rsi), %r9 // %r9 = %rsi + 0xc
  40147c: 4c 8d 46 08                  	leaq	0x8(%rsi), %r8 // %r8 = %rsi + 0x8
  401480: be c3 25 40 00               	movl	$0x4025c3, %esi         # imm = 0x4025C3 // %esi = value at $0x4025c3, %d %d %d %d %d %d
  401485: b8 00 00 00 00               	movl	$0x0, %eax // %eax = $0x0
  40148a: e8 61 f7 ff ff               	callq	0x400bf0 <__isoc99_sscanf@plt> sscanf(input_str, %d %d %d %d %d %d, arr[0], arr[1], arr[2], arr[3], ..) -> stack + 16 (0x10)
  40148f: 83 f8 05                     	cmpl	$0x5, %eax
  401492: 7f 05                        	jg	0x401499 <read_six_numbers+0x3d>
  401494: e8 a1 ff ff ff               	callq	0x40143a <explode_bomb>
  401499: 48 83 c4 18                  	addq	$0x18, %rsp // restore the stack
  40149d: c3                           	retq
```

### Observations

1. **Stack allocation**: `subq $0x28, %rsp` → 40 bytes for local variables.
2. **Reading numbers**: `read_six_numbers` fills 6 integers starting at `%rsp`.
3. **Logic**: Each number doubles the previous one (`arr[i] = 2 * arr[i-1]`).

`read_six_numbers` uses `sscanf` internally:

```c
sscanf(input_str, "%d %d %d %d %d %d", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
```

4. **Defusing phase 2**:

Phase 2 checks in a loop that elements of the array double.

```bash
root@container:/work# ./bomb
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.
Phase 1 defused. How about the next one?
1 2 4 8 16 32
That's number 2. Keep going!
```
