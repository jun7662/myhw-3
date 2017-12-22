#include <sys/types.h>
#include <stdio.h>
#include <limits.h>

#include "run.h"
#include "util.h"

void *base = 0;

p_meta find_meta(p_meta *last, size_t size) {
  p_meta index = base;
  p_meta result = NULL;
    
  switch(fit_flag){
    case FIRST_FIT:
    {
	while(index){
	  if(index->size >= size && index->free == 1 ) 
	    if(result == NULL)
	      result = index;
	  *last = index;
	  index = index->next;
	}
      //FIRST_FIT CODE
    }
    break;

    case BEST_FIT:
    {
	while(index){
	  if(index->size >= size && index->free == 1){
	    if(result == NULL) result = index;
	    else if(result->size > index->size) result = index;
	  }
	  *last = index;
	  index = index->next;
	}	
      //BEST_FIT CODE
    }
    break;

    case WORST_FIT:
    {
	while(index){
	  if(index->size >= size && index->free == 1){
	    if(result == NULL) result = index;
	    else if(result->size < index->size) result = index;
	  }
	  *last = index;
	  index = index->next;
	}
      //WORST_FIT CODE
    }
    break;

  }
  return result;
}

void *m_malloc(size_t size) {
	p_meta block;
	block = sbrk(0);
	if (size <= 0) return NULL;
	int num = size/4;
	if(num*4 < size) num ++;
	size_t m_size = num *4;

	if(base == 0){
	  block = sbrk(m_size+META_SIZE);
	  if(block == (void*) -1) return NULL;
	  block->size = m_size;
	  block->next = NULL;
	  block->prev = NULL;
	  base = block;
	}
	else {
	  p_meta prev_block;
	  prev_block = base;
	  block = find_meta(&prev_block, m_size);
	  if(block == NULL){
	    	block = sbrk(m_size+META_SIZE);
	    	//printf("size of block : %d\n",sizeof(*block));
	   	if(block == (void*) -1) return NULL;
	  	block->size = m_size;
	  	prev_block->next = block;
	  	block->prev = prev_block;
	  	block->next = NULL;
	  }
	  else {
		if(block->size - m_size > META_SIZE){
		  prev_block = block;
		  //prev->block->size = m_size;
		  block = (void*)(prev_block) + m_size + META_SIZE;
		  block->size = prev_block->size -m_size- META_SIZE;
		  block->next = prev_block->next;
		  block->prev = prev_block;
		  block->free = 1;
		  block->next->prev= block;
		  prev_block->next = block;
		  prev_block->size = m_size;
		  block = prev_block;		
		}
	  }
	  block->free = 0;
	}
//	printf("%d size : %d\n",block, block->size);
  return (void*)block+META_SIZE;
}

void m_free(void *ptr) {
	if(!ptr) return;
	
	p_meta block;
	
	block = ptr-META_SIZE;
//	printf("%d size : %d\n", block, block->size);
	
	block->free = 1;
	if(block->prev !=NULL)
	  if(block->prev->free == 1){
	    block->prev->size += (block->size + META_SIZE);
	    block->prev->next = block->next;
	    if(block->next != NULL)
	      block->next->prev = block->prev;
	    block = block->prev;
	}
	if(block->next != NULL)
	  if(block->next->free ==1){ 
	    block->size += (block->next->size + META_SIZE);
	    block->next = block->next->next;
	    block->next->prev = block->prev;
	  }
	if(block->next == NULL){
	  block->prev->next = NULL;
	  brk(block->prev); 
	}
//	if(block->prev == NULL) block = NULL;
//	  base = 0;
}

void*
m_realloc(void* ptr, size_t size)
{	
	if(!ptr){
	  return m_malloc(size);
	}

	void * result;
	size_t m_size;
	int num;
	int o_size;
	
	num = size/4;
	if(num*4 < size) num ++;
	m_size = num *4;
	p_meta block = ptr-META_SIZE;
	o_size = block->size;

	//printf("%d\n",block->size);
	if(block->size >= size){
		if(block->size - m_size >= META_SIZE){
		  p_meta prev_block = block;

		  block = (void*)(prev_block) + m_size + META_SIZE;
		  block->size = prev_block->size -m_size- META_SIZE;
		  block->next = prev_block->next;
		  block->prev = prev_block;
		  block->next->prev= block;
		  block->free = 1;
		  prev_block->next = block;
		  prev_block->size = m_size;
		  block = prev_block;		
		}
		result = ptr;
		return result;	  
	}
	p_meta prev;
	block = find_meta(&prev, m_size);
	if(block == NULL){
	  void * tmp = m_malloc(size);
	  result = tmp;
	  memcpy(result, ptr, o_size);
	  m_free(ptr);
	}else{
	  m_free(ptr);
	  result = m_malloc(size);
	  memcpy(result, ptr, o_size);
	}
	ptr = result;
	return result;
}
