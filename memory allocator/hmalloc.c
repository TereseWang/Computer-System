
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <assert.h>

#include "hmalloc.h"
#include "freelist.h"

/*
typedef struct hm_stats {
long pages_mapped;
long pages_unmapped;
long chunks_allocated;
long chunks_freed;
long free_length;
} hm_stats;
*/

const size_t PAGE_SIZE = 4096;
static hm_stats stats; // This initializes the stats to 0.

static freelist* head;

long
free_list_length()
{
  int result = 0;
  freelist* current = head;
  while(current != 0) {
    result++;
    current = current->next;
  }
  return result;
}

hm_stats*
hgetstats()
{
  stats.free_length = free_list_length();
  return &stats;
}

void
hprintstats()
{
  stats.free_length = free_list_length();
  fprintf(stderr, "\n== husky malloc stats ==\n");
  fprintf(stderr, "Mapped:   %ld\n", stats.pages_mapped);
  fprintf(stderr, "Unmapped: %ld\n", stats.pages_unmapped);
  fprintf(stderr, "Allocs:   %ld\n", stats.chunks_allocated);
  fprintf(stderr, "Frees:    %ld\n", stats.chunks_freed);
  fprintf(stderr, "Freelen:  %ld\n", stats.free_length);
}

static
size_t
div_up(size_t xx, size_t yy)
{
  // This is useful to calculate # of pages
  // for large allocations.
  size_t zz = xx / yy;
  if (zz * yy == xx) {
    return zz;
  }
  else {
    return zz + 1;
  }
}

void
freelist_put(freelist* next) {
  freelist* current = head;
  freelist* previous = NULL;
  size_t previous_size = 0;
  if (head == 0) {
    head = next;
  }
  else {
    while(current <= next) {
      previous = current;
      current = current->next;
    }
    if (previous == 0) {
      head = next;
    }
    else {
      previous_size = previous->size;
    }

    if ((void*) next + next->size == (void*) current) {
      if ((void*) previous + previous_size == (void*) next) {
        previous->next = current->next;
        previous->size = previous->size + next->size + current->size;
      }
      else {
        next->next = current->next;
        next->size = next->size + current->size;
      }
    }
    else {
      if ((void*) previous + previous_size == (void*) next) {
        previous->size = previous->size + next->size;
      }
      else {
        next->next = current;
        if (previous != 0) {
          previous->next = next;
        }
      }
    }
  }
}

void*
hmalloc(size_t size)
{
  stats.chunks_allocated += 1;
  size += sizeof(size_t);
  freelist* list = NULL;
  freelist* previous = NULL;
  freelist* current = head;

  if (size > PAGE_SIZE){
    size_t num_pages = div_up(size, PAGE_SIZE);
    freelist* list = mmap(0, PAGE_SIZE * num_pages,
      PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    assert(list != MAP_FAILED);
    list->size = size;
    stats.pages_mapped += num_pages;
    return (void*) list + sizeof(size_t);
  }
  else {
    while (current && current->size < size) {
      previous = current;
      current = current->next;
    }

    if (current != 0 && previous == 0) {
      list = current;
      head = current->next;
    }

    if (current != 0 && previous != 0) {
      previous->next = current->next;
    }

    if (list == NULL) {
      list = mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE,MAP_ANON|MAP_PRIVATE, -1, 0);
      assert(list != MAP_FAILED);
      list->size = PAGE_SIZE;
      stats.pages_mapped++;
    }

    if (list->size >= sizeof(freelist) +  size) {
      freelist* add_on = (freelist*) ((void*) list + size);
      add_on->size = list->size - size;
      list->size = size;
      freelist_put(add_on);
    }
    return (void*) list + sizeof(size_t);
  }
}

void
hfree(void* item)
{
  stats.chunks_freed += 1;
  freelist* new_block = (freelist*) (item - sizeof(size_t));

  if (new_block->size < PAGE_SIZE) {
    freelist_put(new_block);
  }
  else {
    size_t pages = div_up(new_block->size, PAGE_SIZE);
    stats.pages_unmapped += pages;
    int rv = munmap((void*) new_block, new_block->size);
    assert(rv != -1);
  }
}
