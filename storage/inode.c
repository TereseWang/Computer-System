#include <assert.h>
#include <errno.h>
#include "bitmap.h"
#include "pages.h"
#include "util.h"
#include "inode.h"

void print_inode(inode* node){}

int check_inode(int inum) {
  if (inum < 0 || inum >= 256){
    return 0;
  }
  return bitmap_get(get_inode_bitmap(), inum);
}

inode* get_inode(int inum) {
  if (check_inode(inum)) {
    return ((inode*)pages_get_page(1)) + inum;
  }
  return 0;
}

int alloc_inode() {
  void* inode_bitmap = get_inode_bitmap();
  for (int ii = 0; ii < 256; ii++) {
    if (!bitmap_get(inode_bitmap, ii)) {
      bitmap_put(inode_bitmap, ii, 1);
      return ii;
    }
  }
  return -ENOSPC;
}

void free_inode(int inum) {
  inode* inode = get_inode(inum);
  if (inum == 0 || inode == 0) {
    return;
  }
  for (int ii = 0; ii < 5; ii++) {
    if (inode->ptrs[ii] != 0) {
      free_page(inode->ptrs[ii]);
      inode->ptrs[ii] = 0;
    }
  }
  free_inode(inode->iptr);
  inode->size = 0;
  inode->mode = 0;
  inode->iptr = 0;
  bitmap_put(get_inode_bitmap(), inum, 0);
}

int grow_inode(inode* node, int size) {
  if (size <= 5 * 4096) {
    int num_pages = bytes_to_pages(size);
    for (int ii = 0; ii < num_pages; ii++) {
      if (node->ptrs[ii] == 0) {
        int page = alloc_page();
        if (page < 0) {
          return -ENOSPC;
        }
        node->ptrs[ii] = page;
      }
    }
    node->size = size;
    return 0;
  }
  else {
    if (node->iptr == 0) {
      int page = alloc_inode();
      if (page < 0) {
        return -ENOENT;
      }
      inode* inode = get_inode(page);
      inode->mode = node->mode;
      node->iptr = page;
    }
    node->size = size;
    return grow_inode(get_inode(node->iptr), size - 5 * 4096);
  }
}

int shrink_inode(inode* node, int size) {
  node->size = size;
  if (size > 5 * 4096) {
    return shrink_inode(get_inode(node->iptr), size - 5 * 4096);
  }
  int num_pages = bytes_to_pages(size);
  for (int ii = 4; ii >= num_pages; ii--) {
    if (node->ptrs[ii] != 0) {
      free_page(node->ptrs[ii]);
      node->ptrs[ii] = 0;
    }
  }
  if (node->iptr > 0) {
    free_inode(node->iptr);
  }
  return 0;
}

int inode_get_pnum(inode* node, int fpn) {
}

void inodes_init() {
  void* page_bitmap = get_pages_bitmap();
  if (!bitmap_get(page_bitmap, 1)) {
    int num_pages = bytes_to_pages(256 * sizeof(inode));
    for (int ii = 1; ii <= num_pages; ii++) {
      assert(alloc_page() == ii);
    }
  }
}
