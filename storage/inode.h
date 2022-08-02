// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include <sys/types.h>
#include "pages.h"

typedef struct inode {
    int refs;
    mode_t mode;
    int size;
    int ptrs[5];
    int iptr;
    time_t atime;
    time_t mtime;
    time_t ctime;
} inode;

void print_inode(inode* node);
inode* get_inode(int inum);
int alloc_inode();
void free_inode(int inum);
int grow_inode(inode* node, int size);
int shrink_inode(inode* node, int size);
int inode_get_pnum(inode* node, int fpn);
void inodes_init();
#endif
