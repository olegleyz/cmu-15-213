# Lab 1: Data Lab - Bit Manipulation

## The 32-bit Compilation Challenge on Apple Silicon (2025)

### The Problem: When Modern Hardware Meets Legacy Requirements

Working on CMU's 15-213 Data Lab in 2025 presents an interesting challenge that perfectly illustrates the evolution of computer architectures. The lab materials were designed for a 32-bit x86 Linux environment, but modern Apple Silicon Macs (and even newer Intel Macs) have moved beyond 32-bit support.

When attempting to compile the lab on macOS, I encountered two fundamental incompatibilities:

1. **Architecture Mismatch**: The precompiled `dlc` (data lab checker) binary is built for x86-64 Linux, not ARM64 macOS
2. **32-bit Compilation Removed**: macOS dropped 32-bit application support after Catalina, and Apple's toolchain doesn't include 32-bit x86 compilation capabilities

The error messages tell the story:
```bash
$ make all
gcc -O -Wall -m32 -lm -o btest bits.c btest.c decl.c tests.c
ld: unknown -arch name: armv4t
clang: error: linker command failed with exit code 1
```

The `-m32` flag requests 32-bit compilation, but Apple's Clang simply doesn't have that capability on Apple Silicon.

### The Solution: Docker as a Time Machine

Rather than fighting against the architecture, I embraced containerization to create the exact environment the lab expects. Docker becomes a "time machine" that lets us run legacy 32-bit Linux environments on modern hardware.

## Docker Setup and Usage

### The Magic Command

```bash
docker run --platform linux/amd64 -it --rm \
  -v "$PWD":/lab -w /lab ubuntu:18.04 bash
```

Once inside the container:
```bash
apt update
apt install -y build-essential gcc-multilib
make all
./dlc bits.c
./btest
```

### Breaking Down the Docker Incantation

Each flag serves a specific purpose in creating our development environment:

- `--platform linux/amd64`: Forces x86-64 emulation (essential for running precompiled binaries)
- `-it`: Interactive terminal mode - gives us a proper shell experience
- `--rm`: Ephemeral container - automatically cleaned up when we exit
- `-v "$PWD":/lab`: Volume mount - creates a bridge between macOS filesystem and container
- `-w /lab`: Sets working directory inside container
- `ubuntu:18.04`: The base image - old enough to have good 32-bit support

### The Beautiful Symbiosis

What makes this setup elegant is the seamless integration between host and container:

- **Edit on macOS**: Use your favorite editor (VS Code, vim, etc.) on the host
- **Compile in Linux**: Run make and test inside the container
- **Files sync automatically**: Changes appear instantly in both environments
- **No data loss**: Everything persists on your Mac's filesystem

## Self-Reflection

**Problem**: Apple Silicon Macs can't compile 32-bit x86 code required for this legacy academic lab. **Solution**: Use Docker to run a Linux container with the necessary 32-bit toolchain while mounting the project folder so I can continue editing code on macOS.

## Lab Progress

### Completed Functions
- ✅ **bitXor**: Implemented XOR using only `&` and `~` operators

#### Reflection on XOR Implementation

Implementing XOR with just `&` and `~` was trickier than expected! The breakthrough came from visualizing XOR as set operations using the [symmetric difference Venn diagram](https://en.wikipedia.org/wiki/Symmetric_difference#/media/File:Venn0110.svg) from Wikipedia: XOR is the union of two sets minus their intersection (A ∪ B) - (A ∩ B). In bit terms, this means "bits that are in A or B, but not in both."

The solution unfolded in steps:
1. First, I realized `(~x&y) + (x&~y)` expresses "bits in A or B, but not both"
2. Since addition isn't allowed, I replaced it with OR: `(~x&y)|(x&~y)`
3. Since OR isn't allowed either, I used De Morgan's law: `A|B = ~(~A&~B)`
4. Final solution: `~(~(~x&y)&~(x&~y))`

Each constraint forced me deeper into Boolean algebra fundamentals!

### Next Steps
- Continue implementing the remaining bit manipulation functions
- Test each function thoroughly with the provided test suite
- Optimize solutions to meet the operator count constraints
