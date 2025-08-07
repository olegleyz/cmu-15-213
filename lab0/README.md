# Lab 0: C Programming Lab

## Overview
This lab serves as an introduction to C programming and basic system concepts for CMU 15-213.

## Personal Learning Experience

The first lectures were devoted to the basic representation of integer and floating point variables, the necessity to allocate memory, and the harsh reality of C being unsafe. The examples of accessing array indexes outside of their boundaries, or what's worse - editing one variable and unintentionally overriding some other variables - these made my heart move. For some reason, these specifics of C were erased from my memory since my school days.

While working on Lab 0, I had to implement a FIFO/LIFO queue using linked lists and carefully allocate and free up memory for the nodes of the linked list, reverse it, and more. I spent so much time trying to figure out bugs which are hard to imagine in my day-to-day job with Java or Python. All related to:

- Missed null checks after `malloc`
- Explicitly initializing variables to `NULL` after declaration
- Freeing up memory after use
- Careful pointer management

Overall, such a pleasure working on the code thoughtfully. And so much appreciation to the course authors who developed sophisticated unit tests. It's impossible to underestimate the necessity of good test code coverage when working with C.

## Technical Implementation

## Files
- `cprogramminglab.pdf` - Official lab handout and specifications
- `queue.c` - Main implementation of FIFO/LIFO queue using linked lists
- `queue.h` - Header file with queue data structures and function declarations
- `qtest.c` - Command interpreter for testing queue operations
- `console.c` - Console interface implementation
- `harness.c` - Testing harness for automated grading
- `report.c` - Performance reporting functionality
- `Makefile` - Build configuration
- `driver.py` - Python driver for automated testing
- `traces/` - Directory containing test trace files
- `README` - Original lab handout instructions

## Build Instructions
```bash
# Compile the testing program
make

# Run correctness tests
make test

# Clean build artifacts
make clean
```

## Usage
```bash
# Run the interactive queue tester
./qtest

# Run with command-line options (see help)
./qtest -h

# Run automated tests
python3 driver.py
```

## Notes
- Add any important notes about your implementation
- Document any challenges or interesting solutions
- Include performance observations if relevant

## Status
✅ Completed
