#include "dynamic-memory.h"
#include "test.h"


int main(void) {
	init_bitmap();

	int test_count = 10;
	int test_pass_count = 0;
	bool test_pass = 0;
	printf("Testing...\n");


	test_pass = test_check_size();
	test_pass_count += test_pass;
	printf("%d ", test_pass);

	test_pass = test_set_bitmap_block();
	test_pass_count += test_pass;
	printf("%d ", test_pass);

	test_pass = test_check_size_after_bitmap_block_set();
	test_pass_count += test_pass;
	printf("%d ", test_pass);

	test_pass = test_set_bits_match_allocated_block();
	test_pass_count += test_pass;
	printf("%d ", test_pass);

	test_pass = test_set_bits_match_multiple_allocated_blocks();
	test_pass_count += test_pass;
	printf("%d ", test_pass);

	test_pass = test_set_bits_match_allocated_then_freed_then_allocated_block();
	test_pass_count += test_pass;
	printf("%d ", test_pass);

	test_pass += test_unset_bits_match_deallocated_block();
	test_pass_count += test_pass;
	printf("%d ", test_pass);

	test_pass += test_malloc_returns_null_requested_block_bigger_than_heap();
	test_pass_count += test_pass;
	printf("%d ", test_pass);

	test_pass += test_malloc_returns_null_requested_block_too_large();
	test_pass_count += test_pass;
	printf("%d ", test_pass);

	test_pass += test_malloc_returns_block_at_correct_heap_index();
	test_pass_count += test_pass;
	printf("%d ", test_pass);

	printf("\n%d / %d tests passed\n", test_pass_count, test_count);

	return 0;
}
