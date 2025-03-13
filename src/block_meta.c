#include "mem.h"

struct block_meta block_list_head;

// flags
int init;
int prealloc;

void block_list_init(void)
{
	block_list_head.next = &block_list_head;
	block_list_head.prev = &block_list_head;
	block_list_head.size = 0;
	block_list_head.status = -1;

	// set flag
	init = 1;
}

struct block_meta *block_find(void *addr)
{
	if (addr == NULL)
		return NULL;

	struct block_meta *iter;

	for (iter = block_list_head.next; iter != &block_list_head; iter = iter->next) {
		if ((void *)PAYLOAD(iter) == addr)
			return iter;
	}

	// block doesnt exist
	return NULL;
}

void block_list_add(struct block_meta *block)
{
	// check if list is empty
	if (block_list_head.next == &block_list_head) {
		block->next = &block_list_head;
		block->prev = &block_list_head;
		block_list_head.next = block;
		block_list_head.prev = block;
		return;
	}
	if (block->status == STATUS_MAPPED) {
		// place new mapped blocks at the start of the list
		block_list_head.next->prev = block;
		block->next = block_list_head.next;
		block->prev = &block_list_head;
		block_list_head.next = block;
	} else {
		// place new heap blocks at the end of the list;
		// heap blocks must be in the correct order;
		// having no mapped blocks in between heap blocks simplifies implementation
		block_list_head.prev->next = block;
		block->prev = block_list_head.prev;
		block_list_head.prev = block;
		block->next = &block_list_head;
	}
}

void block_list_del(struct block_meta *block)
{
	if (block == NULL)
		return;

	// remove block from list
	block->next->prev = block->prev;
	block->prev->next = block->next;
	block->next = block;
	block->prev = block;

	// only free mapped blocks
	if (block->status == STATUS_MAPPED)
		block_free(block);
}

// coalesce block with the next block
void block_coalesce(struct block_meta *block)
{
	block->size = ACTUAL_SIZE(block->size) + TOTAL_SIZE(block->next->size);
	block_list_del(block->next);
}

void coalesce_all(void)
{
	struct block_meta *iter;

	// search for 2 adjacent free blocks
	for (iter = block_list_head.next; iter != &block_list_head; iter = iter->next) {
		if (iter->status == STATUS_FREE) {
			if (iter->next->status == STATUS_FREE) {
				block_coalesce(iter);
				iter = iter->prev;
			}
		}
	}
}

struct block_meta *find_best(size_t size)
{
	// coalesce all adjacent free blocks first
	coalesce_all();

	struct block_meta *best = NULL;
	struct block_meta *iter;

	for (iter = block_list_head.next; iter != &block_list_head; iter = iter->next) {
		if (iter->status == STATUS_FREE) {
			if (iter->size >= size) {
				if (best == NULL || best->size > iter->size)
					best = iter;
			}
		}
	}
	return best;
}

void split_block(struct block_meta *block, size_t size)
{
	size_t remaining_size = TOTAL_SIZE(block->size) - (TOTAL_SIZE(size));

	if (remaining_size >= SPLIT_THRESHOLD) {
		// create new block
		struct block_meta *new;

		new = (void *)block + TOTAL_SIZE(size);
		new->size = remaining_size - (META_SIZE);
		new->status = STATUS_FREE;

		// resize old block
		block->size = size;
		block->status = STATUS_ALLOC;

		// add new block to the list
		new->prev = block;
		new->next = block->next;
		block->next->prev = new;
		block->next = new;
		return;
	}
	block->status = STATUS_ALLOC;
}

// check if the last block on the heap is free and can be expanded
struct block_meta *check_last(void)
{
	if (block_list_head.prev->status == STATUS_FREE)
		return block_list_head.prev;
	else
		return NULL;
}

// check if block is last on heap
int is_last(struct block_meta *block)
{
	if (block == block_list_head.prev)
		return 1;
	else
		return 0;
}
