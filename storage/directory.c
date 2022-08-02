#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include "bitmap.h"
#include "inode.h"
#include "pages.h"
#include "slist.h"
#include "util.h"
#include "directory.h"

void directory_init() {
  int page = alloc_page();
  int inode_index = alloc_inode();
  inode* inode = get_inode(inode_index);
  inode->mode = __S_IFDIR | 0755;
  inode->ptrs[0] = 5;
  inode->size = 4096;
  inode->atime = time(NULL);
  inode->ctime = time(NULL);
  inode->mtime = time(NULL);
  directory_put(inode, ".", inode_index);
  directory_put(inode, "..", inode_index);
}

dirent* directory_lookup(inode* dd, const char* name) {
  while (dd > 0) {
    if (is_folder(dd->mode)) {
      for (int ii = 0; ii < 5; ii++) {
        int inode_index = dd->ptrs[ii];
        if (inode_index > 0) {
          dirent* dir = pages_get_page(inode_index);
          for (int jj = 0; jj < 4096 / sizeof(dirent); jj++) {
            if (streq(name, dir[jj].name)) {
              return &(dir[jj]);
            }
          }
        }
      }
      if (dd->iptr > 0) {
        dd = get_inode(dd->iptr);
      }
      else {
        break;
      }
    }
    else {
      break;
    }
  }
  return 0;
}

int tree_lookup(const char* path) {
  int inode_index = 0;
  slist* slist = s_split(path, '/');
  while (slist != 0) {
    dirent* directory = directory_lookup(get_inode(inode_index), slist->data);
    if (directory == 0) {
      s_free(slist);
      return -ENOENT;
    }
    inode_index = directory->inum;
    slist = slist->next;
  }
  s_free(slist);
  return inode_index;
}

int directory_put(inode* dd, const char* name, int inum) {
  while (dd != 0) {
    for (int ii = 0; ii < 5; ii++) {
      if (dd->ptrs[ii] == 0) {
        int page_index = alloc_page();
        dd->ptrs[ii] = page_index;
        dd->size += 4096;
      }
      dirent* page_dirent = pages_get_page(dd->ptrs[ii]);
      for (int jj = 0; jj < 4096 / sizeof(dirent); jj++) {
        if (page_dirent[jj].name[0] == 0) {
          strncpy(page_dirent[jj].name, name, 48);
          page_dirent[jj].inum = inum;
          return 0;
        }
      }
    }
    if (dd->iptr == 0) {
      int rv = grow_inode(dd, dd->size + 4096);
    }
    dd = get_inode(dd->iptr);
  }
  return -ENOSPC;
}

int directory_delete(inode* dd, const char* name) {
  dirent* directory = directory_lookup(dd, name);
  directory->inum = 0;
  memset(directory->name, 0, 48);
  return 0;
}

slist* directory_list(const char* path) {
  int tree_index = tree_lookup(path);
  inode* node = get_inode(tree_index);
  slist* slit = 0;
  while (node != 0) {
    for (int ii = 0; ii < 5; ii++) {
      if (node->ptrs[ii] != 0) {
        dirent* page_dirent = pages_get_page(node->ptrs[ii]);
        for (int jj = 0; jj < 4096 / sizeof(dirent); jj++) {
          if (page_dirent[jj].name[0] != 0) {
            slit = s_cons(page_dirent[jj].name, slit);
          }
        }
      }
    }
    if (node->iptr != 0) {
      node = get_inode(node->iptr);
    }
    else {
      break;
    }
  }
  return slit;
}

void print_directory(inode* dd) {}
