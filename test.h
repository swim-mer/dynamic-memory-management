#ifndef test_h
#define test_h


#include "dynamic-memory.h"

void print_bitmap(int start, int end);

bool test_set_bitmap_block();

bool test_check_size();
bool test_check_size_after_bitmap_block_set();

bool test_set_bits_match_allocated_block();
bool test_set_bits_match_allocated_then_freed_then_allocated_block();
bool test_set_bits_match_multiple_allocated_blocks();

bool test_unset_bits_match_deallocated_block();

bool test_malloc_returns_null_requested_block_too_large();
bool test_malloc_returns_null_requested_block_bigger_than_heap();

bool test_malloc_returns_block_at_correct_heap_index();

#endif
