// Either way:
//  - Modify the allocator as nessiary to make it thread safe by adding exactly
//    one mutex to protect the free list. This has already been done for the
//    provided xv6 allocator.
//  - Implement the "realloc" function for this allocator.
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>

#include "xmalloc.h"
#include "freelist.h"
#include "hmalloc.c"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void*
xmalloc(size_t size)
{
  pthread_mutex_lock(&mutex);
  void* malloc = hmalloc(size);
  pthread_mutex_unlock(&mutex);
  return malloc;
}

void
xfree(void* item)
{
  pthread_mutex_lock(&mutex);
  hfree(item);
  pthread_mutex_unlock(&mutex);
}

void*
xrealloc(void* item, size_t size)
{
  void* malloc = xmalloc(size);
  void* free_list = item - sizeof(int64_t);
  free_list = (freelist*) free_list;
  int64_t list_size = *((int64_t*) free_list);
  memcpy(malloc, item, (list_size - sizeof(int64_t)));
  xfree(item);
  return malloc;
}
