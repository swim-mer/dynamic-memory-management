# dynamic-memory-management
Implementation of heap-based malloc and free 

### Assumptions
- These functions could be for an operating system's applications or the operating system's kernel
- No C library is given for malloc and free, but I may use C libraries such as stddef.h library for the size_t data type, integer libraries for uint8_t data type, and stdio.h for printing test results.
- 32 bit addressing refers to the memory address size and each memory slot is 1 byte long; the range from 0x80010000 - 0x80020000 inclusive is 65537 bytes
- Pointer size is the same for all pointers in this architecture
- In mimicking the behavior of malloc there will be no implemented safety on size boundaries of pointers returned to users

### Implementation
This version of malloc and free is written with a bitmap to track dynamic memory allocation. A bitmap was chosen due to the given size of this heap and the ability to effectively use space with this data structure. Each byte of usable memory is represented by a single bit in the bitmap. Allocated bits are set, while boundaries between blocks are unset. 

Its performance can be fast when allocating small blocks at the beginning of the heap. Its performance grows with the size of the block being allocated and its distance from the beginning of the heap. Its use of space is very good, especially when compared with a linked list. 


### Development
- Developed on Ubuntu 22.04.4 LTS, x86 architecture
- Installed packages: libc6-dev-i386 for compiling 32 bit software to run on 64-bit x86 architecture

Run the software: ```gcc -m32 main.c dynamic-memory.c test.c -o main```
