#include <stdint.h>
#include <stdio.h>
#include "bitmap.h"

int bitmap_get(void* bm, int ii) {
  int byte = ii / 8;
  int bite = ii % 8;
  uint8_t mask = 1 << bite;
  return (((uint8_t*)bm)[byte] & mask) >> bite;
}

void bitmap_put(void* bm, int ii, int vv) {
    int byte = ii / 8;
    int bite = ii % 8;
    if (vv == 0) {
        ((uint8_t*)bm)[byte] &= ~(1 << bite);
    }
    else {
        ((uint8_t*)bm)[byte] |= 1 << bite;
    }
}

void bitmap_print(void* bm, int size) {
    for (int ii = 0; ii < size * 4; ii++) {
        printf("%d\n", bitmap_get(bm, ii));
    }
}
