#include "mem.h"

void *realloc_new_block(struct block_meta *block, size_t size)
{
	// create a new block
	void *new_ptr = os_malloc(size);
	struct block_meta *new_block = block_find(new_ptr);

	// copy contents
	memmove(new_ptr, (void *)PAYLOAD(block), MIN(block->size, size));

	// free old block
	if (block->status == STATUS_MAPPED) {
		block_list_del(block);
	} else {
		block->status = STATUS_FREE;
		block->size = ACTUAL_SIZE(block->size);
	}

	return (void *)PAYLOAD(new_block);
}

void *brk_realloc(struct block_meta *block, size_t size)
{
	// if block is last on the heap, expand it
	if (is_last(block)) {
		expand_last(block, size);
		return (void *)PAYLOAD(block);
	}

	// try coalescing the following free blocks
	while (block->size < size) {
		if (block->next->status == STATUS_FREE)
			block_coalesce(block);
		else
			break;
	}

	if (block->size >= size) {
		split_block(block, size);
		return (void *)PAYLOAD(block);
	}

	// find an empty block
	struct block_meta *new_block = NULL;

	new_block = find_best(size);

	if (new_block != NULL) {
		split_block(new_block, size);

		memmove((void *)PAYLOAD(new_block), (void *)PAYLOAD(block), MIN(block->size, size));

		block->status = STATUS_FREE;
		block->size = ACTUAL_SIZE(block->size);

		return (void *)PAYLOAD(new_block);
	}

	// create a new block
	return realloc_new_block(block, size);
}

void *expand_block(struct block_meta *block, size_t size)
{
	if (TOTAL_SIZE(size) > MMAP_THRESHOLD)
		return realloc_new_block(block, size);

	if (block->status == STATUS_MAPPED)
		return realloc_new_block(block, size);

	// reallocation happens on the heap
	return brk_realloc(block, size);
}

void *truncate_block(struct block_meta *block, size_t size)
{
	if (block->status == STATUS_ALLOC) {
		split_block(block, size);
		return (void *)PAYLOAD(block);
	}

	// for mapped blocks, create a new block
	return realloc_new_block(block, size);
}
