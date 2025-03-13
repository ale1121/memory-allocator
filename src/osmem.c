#include "osmem.h"
#include "mem.h"

void *os_malloc(size_t size)
{
	if (size == 0)
		return NULL;

	// initialise block list on first call
	if (!init)
		block_list_init();

	if (TOTAL_SIZE(size) < MMAP_THRESHOLD) {
		// preallocate heap before first brk call
		if (!prealloc)
			prealloc_heap();

		return brk_alloc(size);
	} else {
		return mmap_alloc(size);
	}
}

void os_free(void *ptr)
{
	struct block_meta *block = block_find(ptr);

	if (block == NULL)
		return;

	if (block->status == STATUS_FREE)
		return;

	if (block->status == STATUS_ALLOC) {
		block->status = STATUS_FREE;

		// actual payload size includes padding size
		block->size = ACTUAL_SIZE(block->size);

		return;
	}
	if (block->status == STATUS_MAPPED)
		block_list_del(block);
}

void *os_calloc(size_t nmemb, size_t size)
{
	if (nmemb * size == 0)
		return NULL;

	if (!init)
		block_list_init();

	size_t page_size = getpagesize();
	void *ptr;

	if (TOTAL_SIZE(nmemb * size) < page_size) {
		if (!prealloc)
			prealloc_heap();

		ptr = brk_alloc(nmemb * size);
	} else {
		ptr = mmap_alloc(nmemb * size);
	}

	if (ptr == NULL) return NULL;

	memset(ptr, 0, nmemb * size);

	return ptr;
}

void *os_realloc(void *ptr, size_t size)
{
	if (ptr == NULL)
		return os_malloc(size);

	if (size == 0) {
		os_free(ptr);
		return NULL;
	}

	struct block_meta *block = block_find(ptr);

	if (block == NULL)
		// pointer is not in block list
		return os_malloc(size);

	if (block->status == STATUS_FREE)
		return NULL;

	if (ACTUAL_SIZE(size) == ACTUAL_SIZE(block->size)) {
		// no reallocation is needed
		block->size = size;
		return ptr;
	}

	if (size > block->size)
		return expand_block(block, size);

	if (size < block->size)
		return truncate_block(block, size);

	return NULL;
}
