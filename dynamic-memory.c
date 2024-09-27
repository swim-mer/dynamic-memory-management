#include "dynamic-memory.h"
#include "test.h"



/* Heap is located at 0x80010000 - 0x80020000 for a size of 65537 bytes, or approximately 65.5 kilobytes (KB) */
const uint32_t HEAP_SIZE = 65537; // Used a calculator to count number of bytes between 0x80010000 - 0x80020000 inclusive
//const uint64_t HEAP_START_ADDRESS = 0x80010000; // Not part of the assignment
//const uint32_t HEAP_END_ADDRESS = 0x80020000; // Not part of the assignment 
const uint16_t BITMAP_SIZE = 7282; // Used a function to calculate the available bytes per proportion of the heap
const uint16_t USABLE_BYTES_SIZE = 58255 - 1; // ^^
uint8_t heap[65537];

/* 
** check_size: Checks the bitmap beginning at a certain location for sufficient consecutive unset bits to allocate a block
**
** bytes: number of bytes requested for block allocation
** index: pointer to index in heap's bitmap
** bit: pointer to bit number in current bitmap row
** end_addr: last address of heap
**
** return: true if size is adequate, false if size is inadequate
*/
bool check_size(size_t bytes, int *index, uint8_t *bit, uint32_t end_addr) {
	if (*index >= BITMAP_SIZE) {
		return false;
	}

	uint8_t byte = 0b10000000;
	byte >>= *bit - 1;

	int idx = *index;
	size_t count = 0;

	while (count <= bytes && idx <= BITMAP_SIZE - 1) {
		if (({uint8_t bit = byte & heap[idx];  uint8_t bit_ = bit ^ byte; bit_;}) == byte) {
			++count;
		} else {
			*bit = 0;
			while (byte > 0) {
				*bit++;
				byte <<= 1;
			}
			*index = idx;
			return false;
		}

		byte >>= 1;
		if (byte == 0) {
			byte = 0b10000000;
			idx++;
		}
	}

	if (idx >= BITMAP_SIZE) {
		*index = idx;
		return false;
	}
	return true;
}


/*
** set_bitmap_block: Sets the bitmap as used for the requested block
**
** bytes: number of bytes in block being allocated
** index: current index of the bitmap
** bit: pointer to bit number in current bitmap row
**
** return: nothing
*/
void set_bitmap_block(size_t bytes, int index, uint8_t *bit) {
	/* Create a bitmask to manipulate bits in bitmap */
	uint8_t byte = 0b10000000; // or 0x80
	byte >>= *bit - 1;


	/* Keep track of the number of bits set */
	size_t count = 0;
	/* Set bytes number of bits to used */
	while (count < bytes) {
		/* Keep all bits as they were, except for the bit set in the bitmask; set that one using or */
		heap[index] |= byte;
		/* Right shift the bitmask to set the next bit in the next loop iteration */
		byte >>= 1;
		/* Increment the count */
		count++;

		/* Once bitmask has been right shifted enough, it will equal 0; reset the bitmask to 10000000 and increment the index to get to the next row of the bitmap */
		if (byte == 0) {
			byte = 0b10000000; // or 0x80
			index++;
		}
	}
}

void print_bitmask(uint8_t bitmask) {
	uint8_t byte = 0b10000000;
	for (int i = 0; i < 8; ++i) {
		uint8_t bit = byte & bitmask;
		if (bit > 0) {
			bit = 1;
		}
		printf("%u", bit);
		byte >>= 1;
	}
	printf("\n");
}

/* 
** my_malloc: dynamically allocate memory within a heap
**
** return: pointer to address in heap where allocated block begins
*/
void *my_malloc(size_t bytes) {
	if (bytes > USABLE_BYTES_SIZE) {
		return NULL;
	}

	uint8_t *bm = (uint8_t *)&heap;
	for (int i = 0; i < BITMAP_SIZE; ++i) {
		uint8_t byte = 0b01000000;

		for (int j = 1; j <= 4; ++j) {
			uint8_t bit = byte & bm[i];
			if (bit == 0) { 
				uint32_t end_addr = (unsigned int)&heap + BITMAP_SIZE - 1;
				uint8_t bit;
				if (({uint8_t byte_l = byte << 1; uint8_t bit_l = byte_l & bm[i]; bit_l;}) == 0) { 
					bit = j * 2;
				} else if (({uint8_t byte_r = byte >> 1; int idx = i; if (byte_r == 0) {byte_r = 0b10000000; idx++;} uint8_t bit_r = byte_r & bm[idx]; bit_r;}) == 0) {
					bit = j * 2 + 1;
					if (bit > 8) {
						bit = 1;
						i++;
					}
				} else {
					byte >>= 2;
					if (byte == 0) {
						byte = 0b01000000; 
					}
					continue;
				}

				if (check_size(bytes, &i, &bit, end_addr)) {
					set_bitmap_block(bytes, i, &bit);					
					uint8_t * ret = &heap[0 + BITMAP_SIZE - 1 + 8*i + bit];
					return ret;
				} else {
					j = bit % 2 + bit / 2; 
					byte = 0b10000000; 
					byte >>= bit * 2;
				}
			} else {
				byte >>= 2;
				if (byte == 0) {
					byte = 0b01000000; 
				}
			}
		}	
	}

	return NULL;
}


/** 
** my_free: free dynamically allocated heap memory beginning at ptr address
**
** ptr: pointer to memory to free
**
** return: nothing
**/
void my_free(void *ptr) {
	void *bm = &heap;
	uint32_t offset = ptr - bm - BITMAP_SIZE + 1;
	uint16_t bitmap_index = offset / 8;
	uint8_t bit = offset % 8;

	uint8_t byte = 0b10000000;
	byte >>= bit - 1;

	while (({uint8_t bit_set = byte & heap[bitmap_index]; bit_set;}) == byte) {
		heap[bitmap_index] = heap[bitmap_index] &= ~byte;
		byte >>= 1;
		if (byte == 0) {
			byte = 0b10000000;
			bitmap_index++;
		}
	}
}


/**
** init_bitmap: Initialize a bitmap to track whether bytes of memory are allocated or free
**
** return: nothing
 */
void init_bitmap() {
	// Maximum size # of available bytes divided by 8 is the number of bits needed to track bytes
	// Formula used:
	// a = available bytes = 65537
	// u = usable bytes
	// b = number of bits used = number of usable bytes
	// a = ceil(u/8) + u
	// Result: u = 58255; ceil(u/8) = 7282

	uint8_t *bm = &heap[0];

	for (int i = 0; i < BITMAP_SIZE; ++i) {
		bm[i] = 0;
	}
}
