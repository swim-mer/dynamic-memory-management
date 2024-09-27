#ifndef dynamic_memory_h
#define dynamic_memory_h

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>


extern const uint32_t HEAP_SIZE; // Used a calculator to count number of bytes between 0x80010000 - 0x80020000 inclusive
extern uint64_t HEAP_START_ADDRESS; // Not part of the assignment
extern const uint32_t HEAP_END_ADDRESS; // Not part of the assignment 
extern const uint16_t BITMAP_SIZE; // Used a function to calculate the available bytes per proportion of the heap
extern const uint16_t USABLE_BYTES_SIZE;
extern uint8_t heap[65537];

bool check_size(size_t bytes, int *index, uint8_t *bit, uint32_t end_addr);
void set_bitmap_block(size_t bytes, int index, uint8_t *bit);
void *my_malloc(size_t bytes);
void my_free(void *ptr);
void init_bitmap();

#endif
