#include "dynamic-memory.h"
#include "test.h"



/* Heap is located at 0x80010000 - 0x80020000 for a size of 65537 bytes, or approximately 65.5 kilobytes (KB) */
const uint32_t HEAP_SIZE = 65537; // Used a calculator to count number of bytes between 0x80010000 - 0x80020000 inclusive
//uint64_t HEAP_START_ADDRESS = 0x80010000; // Not part of the assignment
//const uint32_t HEAP_END_ADDRESS = 0x80020000; // Not part of the assignment 
const uint16_t BITMAP_SIZE = 7282; // Used a function to calculate the available bytes per proportion of the heap
const uint16_t USABLE_BYTES_SIZE = 58255 - 1; // ^^
uint8_t heap[65537];
// Heap -- set in the code segment ? // In another situation, would be able to directly assign this address using something like this:
// uint8_t * heap = (uint8_t *)0x80010000; 
// However, in this case it is not possible for me to directly edit memory locations, and without using malloc, no memory would be allocated for the heap


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
	/* If the starting location given to this function is past the end of the heap, return false */
	if (*index >= BITMAP_SIZE) {
		return false;
	}

	/* Use byte for bit manipulating the bitmap*/
	uint8_t byte = 0b10000000; // or 0x80 = 10000000

	/* Right shift byte *bit times to find the correct bit to start with */
	byte >>= *bit - 1;

	/* Make a local copy of index to avoid updating original depending on result of check */
	int idx = *index;
	/* Keep track of how many bits have been checked for block availability */
	size_t count = 0;

	/* While the block is smaller than or equal to (to accommodate trailing 0) the requested size and the index has not passed the end of the heap... */
	while (count <= bytes && idx <= BITMAP_SIZE - 1) {
		/* Check byte & the value of the byte at this heap index by anding the two to isolate only the one bit to be checked, and then exclusive oring the two to check that the heap's bit is 0 */
		if (({uint8_t bit = byte & heap[idx];  uint8_t bit_ = bit ^ byte; bit_;}) == byte) {
			/* Increment the byte count if this is true */
			++count;
		} else {
			/* If it wasn't true, change the bit value so the next check doesn't recheck all the already checked bits: set it to 0 and then count up to the current set bit in byte */ 
			*bit = 0;
			while (byte > 0) {
				*bit++;
				byte <<= 1;
			}
			/* Also change the index pointer value to the local value of index, also to avoid rechecking previous indices */
			*index = idx;
			/* Return false so that the algorithm will continue to look for a location to allocate a block */
			return false;
		}

		/* After incrementing the count, right shift byte 1 over to check the next bit in the bitmap*/
		byte >>= 1;
		/* If shifting the bit over has resulted in 0, it's time to move to the next index and reset the byte to 10000000 */
		if (byte == 0) {
			byte = 0b10000000; // or 0x80
			idx++;
		}
	}

	if (idx >= BITMAP_SIZE) {
		*index = idx;
		return false;
	}
	/* Space was large enough for the block size requested; return true */
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
	/* Since heap is not passed in to malloc, it must be a global variable, defined at the top */
	if (bytes > USABLE_BYTES_SIZE) {
		return NULL;
	}

	/* Bitmap is located at the beginning of the heap; create bm pointer to make it clear this is what is being accessed */
	uint8_t *bm = (uint8_t *)&heap;
	/* BITMAP_SIZE is calculated given the defined size of the heap; it contains the number of bits necessary to count every single usable byte */
	/* Iterate through bits from 0 to BITMAP_SIZE - 1 */
	for (int i = 0; i < BITMAP_SIZE; ++i) {
		/* Use a bitmask to check for unset bit*/
		uint8_t byte = 0b01000000; // 0x40

		/* Since we need to have a 0 preceding the start of a new block, or two 0's in a row, we check every other bit instead of every bit */
		for (int j = 1; j <= 4; ++j) {
			/* Once we find an unset bit... */
			uint8_t bit = byte & bm[i];
			if (bit == 0) { // bit is unset
				/* Set end address of the bitmap to the last index of the bitmap within the heap */
				uint32_t end_addr = (unsigned int)&heap + BITMAP_SIZE - 1;
				uint8_t bit;
				/* Use compound condition for if statement */
				/* Once unset bit has been found, check if bit to the left is also unset */
				if (({uint8_t byte_l = byte << 1; uint8_t bit_l = byte_l & bm[i]; bit_l;}) == 0) { 
					/* If it is unset, then use the current bit as the starting bit by multiplying j * 2 */
					bit = j * 2;
				} else if (({uint8_t byte_r = byte >> 1; int idx = i; if (byte_r == 0) {byte_r = 0b10000000; idx++;} uint8_t bit_r = byte_r & bm[idx]; bit_r;}) == 0) {
					/* Otherwise, if the bit to the right was unset, then use that bit as the starting bit by multiplying j * 2 and adding 1*/
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

				/* Call check_size function to see if there is enough unset space for the entire requested block */
				if (check_size(bytes, &i, &bit, end_addr)) {
					/* If check_size function returns true, set the bits in the bitmap to used */
					set_bitmap_block(bytes, i, &bit);
					
					/* Return pointer to the memory location where this block begins: beginning index of heap + the size of the BITMAP minus 1, + 8 * the index of the bitmap + bit more rows*/
					uint8_t * ret = &heap[0 + BITMAP_SIZE - 1 + 8*i + bit];
					return ret;
				} else {
					/* Otherwise, check_size returned false */
					/* Set j to match the bit location left off in check_size */
					j = bit % 2 + bit / 2; 
					byte = 0b10000000; 
					byte >>= bit * 2;
				}
			} else {
				/* Bit was set; right shift byte by 2; if byte is 0, reset to 01000000 for next iteration and break to next iteration of outer loop */
				byte >>= 2;
				if (byte == 0) {
					byte = 0b01000000; 
				}
			}
		}	
	}

	/* Entire bitmap was checked and no location big enough was found; return NULL */
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
	/* Calculate row offset of the pointer after bitmap */
	void *bm = &heap;
	uint32_t offset = ptr - bm - BITMAP_SIZE + 1;
	/* Bitmap index is the row of the bitmap associated with the row of memory to deallocate */
	uint16_t bitmap_index = offset / 8;

	/* Bit is the bit number in the byte at the bitmap row where the memory begins */
	uint8_t bit = offset % 8;

	/* Create a bitmask to unset bits */
	uint8_t byte = 0b10000000; // 0x80
	byte >>= bit - 1;

	/* While the bitmask anded with the whole byte equals the bitmask (or, while the bit being checked is set)... */
	while (({uint8_t bit_set = byte & heap[bitmap_index]; bit_set;}) == byte) {
		/* Take the not of the bitmask, and it with the bitmap row, and set the bitmap row equal to that; to make sure that one bit is unset  */
		heap[bitmap_index] = heap[bitmap_index] &= ~byte;
		/* Right shift to next bit */
		byte >>= 1;
		/* Reset bitmask to 10000000 if byte == 0 */
		if (byte == 0) {
			byte = 0b10000000; // 0x80
			/* Move to next row of bitmap */
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
	// Create a bitmap for storing information about the heap

	// Maximum size # of available bytes divided by 8 is the number of bits needed to track bytes
	// Formula used:
	// a = available bytes = 65537
	// u = usable bytes
	// b = number of bits used = number of usable bytes
	// a = ceil(u/8) + u
	// Result: u = 58255; ceil(u/8) = 7282
	
	// Initialize entire bitmap to 0

	uint8_t *bm = &heap[0];

	for (int i = 0; i < BITMAP_SIZE; ++i) {
		bm[i] = 0;
	}
}
