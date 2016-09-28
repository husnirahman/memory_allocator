#include "mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "mem_alloc_types.h"

static int __mem_alloc_init_flag=0;

void *malloc(size_t size){
  if(!__mem_alloc_init_flag){
      __mem_alloc_init_flag = 1;
      memory_init();
      //print_info();
  }
  return (void*)memory_alloc((size_t)size);
}

void free(void *p){
  if (p == NULL) return;
  memory_free((char*)p);
  /*print_free_blocks();*/
}

void *calloc(size_t nmemb, size_t size)
{
    if(!__mem_alloc_init_flag){
        __mem_alloc_init_flag = 1;
        memory_init();
        //print_info();
    }
    
    return (void*)memory_alloc((size_t)size*nmemb);
}


void *realloc(void *ptr, size_t size){

    if(!__mem_alloc_init_flag){
        __mem_alloc_init_flag = 1;
        memory_init();
    }
    
    if(ptr == NULL)
        return memory_alloc(size);

    mem_free_block_t *bb = ptr-REALSIZE(sizeof(mem_free_block_t));
    
    fprintf(stderr, "Reallocating %d bytes to %d\n", bb->size , (int)size);
    if(size <= bb->size)
        return ptr;

    char *new = memory_alloc(size);
    memcpy(new, ptr, bb->size);
    memory_free(ptr);
    return (void*)(new);
}
