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

#if defined(FIRST_FIT)

mem_free_block_t** find_free_block(int block_size) {
    mem_free_block_t * it = first_free;    
    mem_free_block_t ** it_ref = &first_free;
    
    while (it != NULL) {
      if (block_size <= it->size) {
        return it_ref;
      }
      
      it_ref = &(it->next);
      it = it->next;  
    }
    
    return NULL;
}

#elif defined(BEST_FIT)

mem_free_block_t** find_free_block(int block_size) {
    mem_free_block_t * it = first_free;    
    mem_free_block_t ** it_ref = &first_free;
    int best_size = MEMORY_SIZE + 1;
    mem_free_block_t ** best_ref = NULL;
  
    while (it != NULL) {
      if (block_size == it->size) {      
        return it_ref;
      }
      else if ((block_size < it->size) && (best_size > it->size)){
        best_size = it->size;
        best_ref = it_ref;
      }
      
      it_ref = &(it->next);
      it = it->next;  
    }
    
    return best_ref;
}

#elif defined(WORST_FIT)

mem_free_block_t** find_free_block(int block_size) {
    mem_free_block_t * it = first_free;    
    mem_free_block_t ** it_ref = &first_free;
    int best_size = 0;
    mem_free_block_t ** best_ref = NULL;
  
    while (it != NULL) {
      if (block_size == it->size) {      
        return it_ref;
      }
      else if ((block_size < it->size) && (best_size < it->size)){
        best_size = it->size;
        best_ref = it_ref;
      }
      
      it_ref = &(it->next);
      it = it->next;  
    }
    
    return best_ref;
}

#endif

void run_at_exit(void)
{
    /* function called when the programs exits */
    /* To be used to display memory leaks informations */
    if(first_free == NULL || first_free->size != MEMORY_SIZE) {
      fprintf(stderr, "Allocated memory blocks are not freed!\n");
    } 
    /* ... */
}



void memory_init(void){

    /* register the function that will be called when the programs exits*/
    atexit(run_at_exit);

    first_free = (mem_free_block_t*) memory;
    first_free->size = MEMORY_SIZE;
    first_free->next = NULL;
}

char *memory_alloc(int size){
    if (size < 0) {
      return NULL;
    }

    int block_size = sizeof(mem_used_block_t) + size;
    block_size += MEM_ALIGNMENT - block_size % MEM_ALIGNMENT;
    
    mem_free_block_t ** free_block_ref = find_free_block(block_size);
    if (free_block_ref == NULL) {
      print_error_alloc(size);
      print_free_blocks();
      return NULL;
    }

    mem_free_block_t free_block = **free_block_ref;
    mem_used_block_t* alloc_block = (mem_used_block_t*) *free_block_ref;
    alloc_block->size = block_size; 
    
    if (free_block.size - block_size <= sizeof(mem_free_block_t)) {
      // In case the remaining size is not enough to store metadata,
      // we just include it to the allocated block
      alloc_block->size = free_block.size;
      *free_block_ref = free_block.next;
    }
    else {
      mem_free_block_t* new_block;
      new_block = (mem_free_block_t*) ((char*) alloc_block + block_size);
      new_block->size = free_block.size - block_size;
      new_block->next = free_block.next;
      *free_block_ref = new_block;
    }
 
    char* payload = (char*) (alloc_block + 1);
    print_alloc_info(payload, size);
    return payload;
}

void memory_free(char *p){
    print_free_info(p);
    p -= sizeof(mem_used_block_t);
    mem_free_block_t* block = (mem_free_bplock_t*) p;
    
    char* p2 = p + block->size;
    
    mem_free_block_t * it = first_free;
    mem_free_block_t * it_prec = NULL;
    
    while(((char*)it < p2) && (it != NULL)) {
      it_prec = it;
      it = it->next;
    } 

    if (it != NULL) {
      if(p2 == (char*)it) {
        block->size += it->size;
        block->next = it->next;
      } 
      else {
        block->next = it;
      }
    }

    if (it_prec != NULL) {
      if(p == ((char*)it_prec + it_prec->size)) {
        it_prec->size += block->size;
        it_prec->next = block->next;
      }
      else {
        it_prec->next = block;
      }
    }
    else {
      first_free = block;
    }
}


void print_alloc_info(char *addr, int size){
  if(addr){
      fprintf(stderr, "ALLOC at : %lu (%d byte(s))\n", 
              ULONG(addr - memory), size);
  }
  else{
      fprintf(stderr, "Warning, system is out of memory\n"); 
  }
}


void print_free_info(char *addr){
    if(addr){
        fprintf(stderr, "FREE  at : %lu \n", ULONG(addr - memory));
    }
    else{
        fprintf(stderr, "FREE  at : %lu \n", ULONG(0));
    }
}

void print_error_alloc(int size) 
{
    fprintf(stderr, "ALLOC error : can't allocate %d bytes\n", size);
}

void print_info(void) {
  fprintf(stderr, "Memory : [%lu %lu] (%lu bytes)\n", (long unsigned int) memory, (long unsigned int) (memory+MEMORY_SIZE), (long unsigned int) (MEMORY_SIZE));
}


void print_free_blocks(void) {
    mem_free_block_t *current; 

    fprintf(stderr, "Begin of free block list :\n"); 
    int i = 0;
    for(current = first_free; current != NULL; current = current->next) {
        fprintf(stderr, "Free block at address %lu, size %u\n", ULONG((char*)current - memory), current->size);
         i++;
         if (i > 12)  {
           break;
         }
    }
}

char *heap_base(void) {
  return memory;
}

#ifdef MAIN
int main(int argc, char **argv){
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
