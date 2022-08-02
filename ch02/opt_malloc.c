#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <limits.h>
#include "xmalloc.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct freelist {
    int size;
    int arena_index;
    struct freelist* next;
} freelist;

typedef struct arena {
    freelist* heads[10];
    pthread_mutex_t mutex;
} arena;

int page_sizes[10] = {4096, 4096, 4096, 4096,4096, 4096, 4096, 4096, 8192, 8192};
int freelist_sizes[10] = {40, 48, 80, 144, 272, 528, 1040, 2064, 4112, 8224};
const int PAGE_SIZE = 4096;
static arena arenas[4];
int num_arenas = 0;
__thread int a_index = 0;

void init_arenas() {
  if (num_arenas) {
    return;
  }
  pthread_mutex_lock(&mutex);
  if (num_arenas) {
    pthread_mutex_unlock(&mutex);
    return;
  }
  for (int i = 0; i < 4; i++) {
    pthread_mutex_init(&(arenas[i].mutex), NULL);
  }
  num_arenas = 1;
  pthread_mutex_unlock(&mutex);
}

int pages_size(int size) {
    int pages = size / PAGE_SIZE;
    if (pages * PAGE_SIZE < size) {
        pages++;
    }
    return pages * PAGE_SIZE;
}

//looks for arean index
int lookup_ai(){
  int ai;
  while (1) {
    ai = a_index;
    ai = (ai + 1) % 4;
    if (pthread_mutex_trylock(&(arenas[ai].mutex)) == 0) {
      return ai;
    }
  }
}

void* add_space(int index, int ai){
  int page_size = page_sizes[index];
  void* item = mmap(0, pages_size(page_size),PROT_READ | PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
  assert(item != MAP_FAILED);

  int freelist_size = freelist_sizes[index];
  int allocate_size = page_size / freelist_size;
  for (int i = 0; i < allocate_size ; i++) {
      freelist* new_freelist = (freelist*)(item + i * freelist_size);
      new_freelist->size = freelist_size;
      new_freelist->next = (freelist*) (item + (i + 1) * freelist_size);
  }
  ((freelist*)(item + (allocate_size - 1) * freelist_size))->next = 0;
  arenas[ai].heads[index] = item + freelist_size;
  return item;
}

void* allocate_inside_freelist(int ai, int i, int size){
  freelist* first_freelist = arenas[ai].heads[i];
  if (first_freelist == 0) {
    first_freelist = add_space(i, ai);
  }
  arenas[ai].heads[i] = first_freelist->next;
  void* item = first_freelist + 1;
  pthread_mutex_unlock(&(arenas[ai].mutex));
  return item;
}

void* allocate_outside_freelist(int size){
  void* item = mmap(0, pages_size(size), PROT_READ | PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
  assert(item != MAP_FAILED);
  freelist* my_block = (freelist*) item;
  my_block->size = pages_size(size);
  my_block->next = ((freelist*) 1);
  return my_block + 1;
}

void* xmalloc(size_t size) {
  assert(size < INT_MAX);
  init_arenas();
  size += sizeof(freelist);
  int index = -1;
  for(int i = 0; i < 9; i++) {
    if (size <= freelist_sizes[i]) {
      index = i;
      break;
    }
  }
  if (index != -1) {
    int ai = lookup_ai();
    a_index = ai;
    return allocate_inside_freelist(ai,index, size);
  }
  else {
    return allocate_outside_freelist(size );
  }
}

void xfree(void* item) {
  freelist* free_list = ((freelist*) item) - 1;
  if(free_list->size >= 4096) {
    munmap(free_list, free_list->size);
  }
  else {
    int arena_index = free_list->arena_index;
    pthread_mutex_lock(&arenas[arena_index].mutex);
    int index = -1;
    for(int i = 0; i < 9; i++) {
      if (free_list->size <= freelist_sizes[i]) {
        index = i;
        break;
      }
    }
    free_list->next = arenas[arena_index].heads[index];
    arenas[arena_index].heads[index] = free_list;
    pthread_mutex_unlock(&arenas[arena_index].mutex);
  }
}

void* xrealloc(void* item, size_t size) {
  freelist* free_list = ((freelist*) item) - 1;
  int allocate_size = free_list->size - sizeof(freelist);
  void* malloc = xmalloc(size);
  if (allocate_size >  size) {
    memcpy(malloc, item, size);
  }
  else {
    memcpy(malloc, item, allocate_size);
  }
  xfree(item);
  return malloc;
}
