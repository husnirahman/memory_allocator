#include "mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "mem_alloc_types.h"

/* memory */
char memory[MEMORY_SIZE];

/* Pointer to the first free block in the memory */
mem_free_block_t *first_free; 


#define ULONG(x)((long unsigned int)(x))
#define max(x,y) (x>y?x:y)

#define BLOCK_AFTER(block) (((char*) block) + block->size) 

#if defined(FIRST_FIT)

mem_free_block_t** find_free_block(int alloc_size) {
    mem_free_block_t** free_block_ref = &first_free;
    mem_free_block_t* free_block = first_free;
    
    while (free_block != NULL) {
      if (alloc_size <= free_block->size) {
        return free_block_ref;
      }
      
      free_block_ref = &(free_block->next);
      free_block = free_block->next;  
    }
    
    return NULL;
}

#elif defined(BEST_FIT)

mem_free_block_t** find_free_block(int alloc_size) {
    mem_free_block_t** free_block_ref = &first_free;
    mem_free_block_t* free_block = first_free;

    mem_free_block_t** candidate_ref = NULL;
    int candidate_size = MEMORY_SIZE + 1;
  
    while (free_block != NULL) {
      if (alloc_size == free_block->size) {
        return free_block_ref;
      }
      else if (alloc_size <= free_block->size && candidate_size > free_block->size) {
        candidate_size = free_block->size;
        candidate_ref = free_block_ref;
      }
      
      free_block_ref = &(free_block->next);
      free_block = free_block->next;  
    }
    
    return candidate_ref;
}

#elif defined(WORST_FIT)

mem_free_block_t** find_free_block(int alloc_size) {
    mem_free_block_t** free_block_ref = &first_free;
    mem_free_block_t* free_block = first_free;

    mem_free_block_t** candidate_ref = NULL;
    int candidate_size = 0;
  
    while (free_block != NULL) {
      if (alloc_size <= free_block->size && candidate_size < free_block->size) {
        candidate_size = free_block->size;
        candidate_ref = free_block_ref;
      }
      
      free_block_ref = &(free_block->next);
      free_block = free_block->next;  
    }
    
    return candidate_ref;
}

#endif

void run_at_exit(void) {
    // Check for leaks
    if (first_free == NULL || first_free->size != MEMORY_SIZE) {
      printf("## Memory leaks detected : \n");
      print_used_blocks();
    }
}

void memory_init(void) {
    atexit(run_at_exit);

    first_free = (mem_free_block_t*) memory;
    first_free->size = MEMORY_SIZE;
    first_free->next = NULL;
}

char *memory_alloc(int size){
    if (size < 0) {
      return NULL;
    }

    int alloc_size = sizeof(mem_used_block_t) + size;
    
    // Takes care of alignment, once and for all
    int misalignment = alloc_size % MEM_ALIGNMENT;
    if (misalignment > 0) {
      alloc_size += MEM_ALIGNMENT - misalignment;
    }
    
#ifdef USED_BLOCK_MINIMUM_SIZE 
    // Makes sure the newly allocated block is big
    // enough to become free again at some point
    if (alloc_size < sizeof(mem_free_block_t)) {
      alloc_size = sizeof(mem_free_block_t);
    }
#endif
    
    // Find the free block (according to the choosen policy)
    mem_free_block_t ** free_block_ref = find_free_block(alloc_size);
    if (free_block_ref == NULL) {
      print_error_alloc(size);
      exit(0);
      return NULL;
    }

    mem_free_block_t old_free_block = **free_block_ref; // Copy on stack
    mem_used_block_t* alloc_block = (mem_used_block_t*) *free_block_ref;
    alloc_block->size = alloc_size; 
    alloc_block->magic = MAGIC_NUMBER;
    
    int remaining_size = old_free_block.size - alloc_size;
    if (remaining_size < sizeof(mem_free_block_t)) {
      // In case the remaining size (for the new free block)
      // is not enough to store metadata of a free block,
      // we just include it to the allocated block
      alloc_block->size = old_free_block.size;
      *free_block_ref = old_free_block.next;
    }
    else {
      // Create a new free block using the remaining space
      mem_free_block_t* new_free_block;
      new_free_block = (mem_free_block_t*) BLOCK_AFTER(alloc_block);
      new_free_block->size = remaining_size;
      new_free_block->next = old_free_block.next;

      *free_block_ref = new_free_block;
    }
 
    char* payload = (char*) (alloc_block + 1);
    print_alloc_info(payload, size);
    return payload;
}

void assert_magic_number(mem_used_block_t* block) {
    if (block->magic != MAGIC_NUMBER) {
      print_error_free(block + 1);
      exit(1486);
    }
}

void memory_free(char* p) {
    char* block_start = p - sizeof(mem_used_block_t);

    // Check magic number
    mem_used_block_t* used_block = (mem_used_block_t*) block_start;
    assert_magic_number(used_block);
    used_block->magic = 0;

    mem_free_block_t* block = (mem_free_block_t*) block_start;
    char* block_end = block_start + block->size;

    mem_free_block_t* after = first_free;
    mem_free_block_t* before = NULL;
    
    // Search for surrounding blocks
    while (after != NULL && (char*) after < block_end) {
      before = after;
      after = after->next;
    } 

    // Handle block after
    if (after == NULL || block_end != (char*) after) {
      // Non contiguous, link them
      block->next = after;
    }
    else {
      // Contiguous, merge them
      block->size += after->size;
      block->next = after->next;
    }

    // Handle block before
    if (before == NULL) {
      first_free = block;
    }
    else if (block_start != BLOCK_AFTER(before)) {
      // Non contiguous, link them
      before->next = block;
    }
    else {
      // Contiguous, merge them
      before->size += block->size;
      before->next = block->next;
    }

    print_free_info(p);
}


void print_alloc_info(char *addr, int size) {
  if (addr) {
      fprintf(stderr, "ALLOC at : %lu (%d byte(s))\n", 
              ULONG(addr - memory), size);
  }
  else {
      fprintf(stderr, "Warning, system is out of memory\n"); 
  }
}


void print_free_info(char *addr) {
    if (addr) {
        fprintf(stderr, "FREE  at : %lu \n", ULONG(addr - memory));
    }
    else {
        fprintf(stderr, "FREE  at : %lu \n", ULONG(0));
    }
}

void print_error_alloc(int size) {
    fprintf(stderr, "ALLOC error : can't allocate %d bytes\n", size);
}

void print_error_free(void* addr) {
    fprintf(stderr, "FREE error : Given pointer: %p is not valid\n", addr);
}

void print_info(void) {
  fprintf(stderr, "Memory : [%lu %lu] (%lu bytes)\n",
      ULONG(memory),
      ULONG(memory+MEMORY_SIZE),
      ULONG(MEMORY_SIZE));
}


void print_free_blocks(void) {
    mem_free_block_t *current; 

    fprintf(stderr, "Begin of free block list :\n"); 
    int i = 0;
    for (current = first_free; current != NULL; current = current->next) {
        fprintf(stderr, "Free block at address %lu, size %u\n", ULONG((char*)current - memory), current->size);
         i++;
         if (i > 12)  {
           break;
         }
    }
}

void print_used_blocks() {
    mem_free_block_t* free = first_free;
    void* block = memory;

    while (block < (void*) (memory + MEMORY_SIZE)) {
        if (block == free) {
          block += free->size;
          free = free->next;
        }
        else {
          mem_used_block_t used_block = *((mem_used_block_t*) block);
          printf("Alloc'd block at address %lu, size %lu\n",
              ULONG(block) - ULONG(memory) + sizeof(mem_used_block_t),
              used_block.size - sizeof(mem_used_block_t));

          block += used_block.size;
        }
    }
}

char *heap_base(void) {
  return memory;
}

#ifdef MAIN
int main(int argc, char **argv) {
  /* The main can be changed, it is *not* involved in tests */
  memory_init();
  print_info(); 
  print_free_blocks();
  int i ; 
  for( i = 0; i < 10; i++){
    char *b = memory_alloc(rand()%8);
    memory_free(b); 
    print_free_blocks();
  }

  char * a = memory_alloc(15);
  //a=realloc(a, 20); 
  memory_free(a);


  a = memory_alloc(10);
  memory_free(a);

  fprintf(stderr,"%lu\n",(long unsigned int) (memory_alloc(9)));
  print_free_blocks();
  return EXIT_SUCCESS;
}
#endif 
