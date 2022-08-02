#ifndef FREELIST_H

#define FREELIST_H
typedef struct freelist {
  size_t size;
  struct freelist* next;
} freelist;
#endif
