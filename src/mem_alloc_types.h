#ifndef   	_MEM_ALLOC_TYPES_H_
#define   	_MEM_ALLOC_TYPES_H_

#define MEM_ALIGNMENT 1

/* Structure declaration for a free block */
typedef struct mem_free_block {
    int size;
    struct mem_free_block *next;
} mem_free_block_t; 

/* Specific metadata for used blocks */
typedef struct mem_used_block {
     int size;
#if MEM_ALIGNMENT > 4
     char _padding[MEM_ALIGNMENT - sizeof(int)];
#endif
} mem_used_block_t;

#endif
