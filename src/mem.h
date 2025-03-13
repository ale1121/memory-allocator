#pragma once

#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "osmem.h"
#include "block_meta.h"

#define ALIGNMENT           8
#define PADDING(size)       (ALIGNMENT - (size % ALIGNMENT)) % ALIGNMENT
#define META_PADDING        PADDING(sizeof(struct block_meta))
#define META_SIZE           sizeof(struct block_meta) + META_PADDING

#define ACTUAL_SIZE(size)   size + PADDING(size)
#define TOTAL_SIZE(size)    META_SIZE + ACTUAL_SIZE(size)
#define PAYLOAD(block)		block + META_SIZE

#define PREALLOC_SIZE       (128 * 1024) - (META_SIZE)
#define MMAP_THRESHOLD      (128 * 1024)
#define SPLIT_THRESHOLD     META_SIZE + 1

#define PROT_READ		0x1
#define PROT_WRITE		0x2
#define MAP_PRIVATE 	0x02
#define MAP_ANONYMOUS 	0x20

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

extern int prealloc;
extern int init;

void block_list_init(void);
void block_list_add(struct block_meta *block);
void block_list_del(struct block_meta *block);
void block_coalesce(struct block_meta *block);
void split_block(struct block_meta *block, size_t size);
struct block_meta *block_find(void *addr);
struct block_meta *find_best(size_t size);
struct block_meta *check_last(void);
int is_last(struct block_meta *block);

void prealloc_heap(void);
void block_free(struct block_meta *block);
void expand_last(struct block_meta *block, size_t size);
void *mmap_alloc(size_t size);
void *brk_alloc(size_t size);

void *truncate_block(struct block_meta *block, size_t size);
void *expand_block(struct block_meta *block, size_t size);
