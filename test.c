#include "test.h"



void print_bitmap(int start, int end) {
    uint8_t *bm = (uint8_t *)&heap;
    uint8_t bitmask = 0b10000000;

    for (int j = start; j <= end; ++j) {
        for (int i = 0; i < 8; ++i) {
            uint8_t bit = bitmask & bm[j];
            if (bit > 0) {
                bit = 1;
            }
            printf("%u", bit);
            bitmask >>= 1;
        }
        printf("\n");
        bitmask = 0b10000000;
    }
}

bool test_check_size() {

    size_t bytes = 5;
    int index = 0;
    uint8_t bit = 2;
    uint32_t end_addr = (unsigned int)&heap + BITMAP_SIZE - 1;

    bool block_available = check_size(bytes, &index, &bit, end_addr);

    return block_available;

}

bool test_set_bitmap_block() {

    size_t bytes = 7;
    int index = 0;
    uint8_t bit = 2;
    set_bitmap_block(bytes, index, &bit);


    if (heap[0] == 0b01111111) {
        return true;
    }
    return false;
    
}

bool test_check_size_after_bitmap_block_set() {

    size_t bytes = 5;
    int index = 0;
    uint8_t bit = 6;
    uint32_t end_addr = (unsigned int)&heap + BITMAP_SIZE - 1;

    bool block_available = check_size(bytes, &index, &bit, end_addr);
    if (!block_available) {
        index = 1;
        return check_size(bytes, &index, &bit, end_addr);

    }

    return block_available;

}

bool test_set_bits_match_allocated_block(){
    uint8_t *bm = (uint8_t *)&heap;
    int *p = my_malloc(3);

    bool matched = false;

    if (bm[1] == 0b01110000) {
        matched = true;
    }
    
    my_free(p);

    return matched;
}



bool test_set_bits_match_allocated_then_freed_then_allocated_block() {
    uint8_t * bm = (uint8_t *)&heap;
    int *p1 = my_malloc(12);

    my_free(p1);

    int *p2 = my_malloc(3);

    return (bm[4] == 0b11110111);

}


bool test_set_bits_match_multiple_allocated_blocks(){
    uint8_t *bm = (uint8_t *)&heap;
    int *p1 = my_malloc(6);

    int *p2 = my_malloc(20);

    return (bm[1] == 0b01111110 && bm[2] == 0b11111111 && bm[3] == 0b11111111 && bm[4] == 0b11110000);
}

bool test_unset_bits_match_deallocated_block() {
    uint8_t *bm = (uint8_t *)&heap;
    int *p = my_malloc(5);

    my_free(p);

    if (bm[5] == 0b00000000) {
        return true;
    }

    return false;
}

bool test_malloc_returns_null_requested_block_too_large() {
    uint8_t * bm = (uint8_t *)&heap;
    int *p1 = my_malloc(50000);

    if (p1 == NULL) {
        return false;
    }

    int *p2 = my_malloc(10000);

    if (p2 == NULL) {
        my_free(p1);
        return true;
    }

    return false;
}

bool test_malloc_returns_null_requested_block_bigger_than_heap() {
    uint8_t *bm = (uint8_t *)&heap;
    int *p = my_malloc(USABLE_BYTES_SIZE + 1);

    if (p == NULL) {
        return true;
    }

    return false;
}

bool test_malloc_returns_block_at_correct_heap_index() {

    uint8_t *bm = (uint8_t *)&heap;
    int *p = my_malloc(7);


    if (p == (int *)&heap + BITMAP_SIZE + 6*8 - 1 + 1) {
        return true;
    }

    return false;

}
