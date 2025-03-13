#include "mem.h"

struct block_meta *mmap_alloc_block(size_t size)
{
	if (size == 0)
		return NULL;

	struct block_meta *block;

	// TOTAL_SIZE includes meta size and padding
	block = mmap(NULL, TOTAL_SIZE(size), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (block == NULL)
		return NULL;

	block->size = size;
	block->status = STATUS_MAPPED;
	return block;
}

struct block_meta *brk_alloc_block(size_t size)
{
	struct block_meta *block;

	block = sbrk(TOTAL_SIZE(size));

	if (block == NULL)
		return NULL;

	block->size = size;
	block->status = STATUS_ALLOC;
	return block;
}

void prealloc_heap(void)
{
	// brk_alloc_block adds meta size
	struct block_meta *block = brk_alloc_block(PREALLOC_SIZE);

	block->status = STATUS_FREE;
	block_list_add(block);

	// set flag
	prealloc = 1;
}

// for mapped blocks only
void block_free(struct block_meta *block)
{
	munmap(block, TOTAL_SIZE(block->size));
}

// expand last block on the heap to fit size
void expand_last(struct block_meta *block, size_t size)
{
	size_t additional_size = size - (ACTUAL_SIZE(block->size)) + PADDING(size);

	void *new = sbrk(additional_size);

	if (new == NULL)
		return;

	block->size = size;
	block->status = STATUS_ALLOC;
}

void *mmap_alloc(size_t size)
{
	struct block_meta *block;

	block = mmap_alloc_block(size);

	if (block == NULL)
		return NULL;

	block_list_add(block);

	return (void *)PAYLOAD(block);
}

void *brk_alloc(size_t size)
{
	struct block_meta *block;

	block = find_best(size);
	if (block != NULL) {
		split_block(block, size);
		return (void *)PAYLOAD(block);
	}

	block = check_last();
	if (block != NULL) {
		expand_last(block, size);
		return (void *)PAYLOAD(block);
	}

	block = brk_alloc_block(size);

	if (block == NULL)
		return NULL;

	block_list_add(block);

	return (void *)PAYLOAD(block);
}
