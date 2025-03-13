# Memory Allocator

This project implements a minimalistic memory allocator that provides manual memory management functions, similar to `malloc()`, `calloc()`, `realloc()` and `free()` in C.

## API

1. `void *os_malloc(size_t size)`
    - Allocates `size` bytes of memory
    - Returns a pointer to the payload of the allocated block
    - Returns `NULL` if size is 0 or if the allocation fails

2. `void *os_calloc(size_t nmemb, size_t size)`
    - Allocates memory for an array of `nmemb` elements, each of size `size`, and initializes it to 0
    - Returns `NULL` if nmemb or size is 0, or if the allocation fails

3. `void *os_realloc(void *ptr, size_t size)`
    - Resizes the memory block pointed to by `ptr` to `size` bytes
    - Attempts to expand the block in place, or moves the block if necessary
    - Returns `NULL` if `ptr` is invalid or if the new size is 0

4. `void os_free(void *ptr)`
    - Frees the memory block pointed to by `ptr`
    - Marks the block as free and makes it available for reuse
    - Checks to ensure that `ptr` is not `NULL` and the block is not already free


## Features

### System calls
- Small allocations use `sbrk()` for heap management
- Large allocations use `mmap()` for memory mapping
- Freed mapped memory is returned to the OS using `munmap()`

### Block Management
**Block Structure**:
Each allocated memory block consists of:
- a metadata structure (`struct block_meta`) that stores the block's size and status
- a payload section, where the actual data is stored

**Block List**:
Blocks are managed as a doubly linked list, allowing efficient traversal and operations such as allocation, deallocation and coalescing.

### Block Coalescing
To reduce external fragmentation, adjacent free blocks are merged into a single larger block whenever possible.

### Heap Preallocation
Minimizes the number of `sbrk` calls by preallocating a chunk of memory upon the first allocation request. Future allocations within this chunk involve splitting and reusing blocks.

### Memory Alignment
- All memory blocks are aligned to 8 bytes
- Padding is added to ensure alignment of both the metadata and the payload

### Efficient Memory Reuse
#### Block Splitting
If a block is larger than the requested size, it is split into two blocks. The unused part becomes a smaller free block.

#### Best Fit Allocation
The allocator searches for the smalles free block that can satisfy the requested size, reducing future fragmentation.

<br>

***

Developed as part of the Operating Systems course at UNSTPB
