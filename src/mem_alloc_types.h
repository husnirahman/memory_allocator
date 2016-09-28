#ifndef   	_MEM_ALLOC_TYPES_H_
#define   	_MEM_ALLOC_TYPES_H_

#define MEM_ALIGNMENT 1

/* Structure declaration for a free block */
typedef struct mem_free_block {
    int size;
    struct mem_free_block *next;
} mem_free_block_t;

#define MAGIC_NUMBER 0xc8058fe3

/* Specific metadata for used blocks */
typedef struct mem_used_block {
     int size;
     int magic;
#if MEM_ALIGNMENT > 8
     char _padding[MEM_ALIGNMENT - 8];
#endif
} mem_used_block_t;

#endif
