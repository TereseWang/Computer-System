#include <assert.h>
#include <errno.h>

#include "bitmap.h"
#include "directory.h"
#include "pages.h"
#include "slist.h"
#include "util.h"
#include "storage.h"

void storage_init(const char* path) {
  pages_init(path);
  inodes_init();
  directory_init();
}

int storage_stat(const char* path, struct stat* st) {
  int tree_index = tree_lookup(path);
  if (tree_index != -ENOENT) {
    inode* found = get_inode(tree_index);
    st->st_mode = found->mode;
    st->st_size = found->size;
    st->st_uid = getuid();
    st->st_atime = found->atime;
    st->st_mtime = found->mtime;
    st->st_ctime = found->ctime;
    st->st_nlink = found->refs + 1;
    return 0;
  } else {
    return -ENOENT;
  }
}

int storage_read(const char* path, char* buf, size_t size, off_t offset) {
  int tree_index = tree_lookup(path);
  inode* inode = get_inode(tree_index);
  if (is_folder(inode->mode)) {
    return -EISDIR;
  }
  if ((inode->mode & __S_IFREG) == __S_IFREG) {
    if (offset >= inode->size) {
      return 0;
    }
    size = size < inode->size - offset ? size : inode->size - offset;
    while (offset >= 5 * 4096) {
      inode = get_inode(inode->iptr);
      offset -= 5 * 4096;
    }
    size_t rem_size = size;
    size_t buf_off = 0;
    while (rem_size > 0) {
      for (int ii = 0; ii < 5; ii++) {
        if (offset >= 4096) {
          offset -= 4096;
        } else {
          int read_size = min(rem_size, 4096 - offset);
          memcpy(buf + buf_off, pages_get_page(inode->ptrs[ii]) + offset, read_size);
          offset = 0;
          buf_off += read_size;
          rem_size -= read_size;
        }
      }
      inode = get_inode(inode->iptr);
    }
    inode->atime = time(NULL);
    return size;
  }
  return -1;
}

int storage_write(const char* path, const char* buf, size_t size, off_t offset) {
  int tree_index = tree_lookup(path);
  inode* inode = get_inode(tree_index);
  if (offset + size > inode->size) {
    grow_inode(inode, offset + size);
  }
  while (offset >= 5 * 4096) {
    inode = get_inode(inode->iptr);
    offset -= 5 * 4096;
  }
  size_t rem_size = size;
  int buf_off = 0;
  while (rem_size > 0) {
    for (int ii = 0; ii < 5; ii++) {
      if (offset >= 4096) {
        offset -= 4096;
      } else {
        int read_size = min(rem_size, 4096 - offset);
        memcpy(pages_get_page(inode->ptrs[ii]) + offset, buf + buf_off, read_size);
        offset = 0;
        buf_off += read_size;
        rem_size -= read_size;
      }
    }
    inode = get_inode(inode->iptr);
  }
  inode->mtime = time(NULL);
  return size;
}

int storage_truncate(const char *path, off_t size) {
  int tree_index = tree_lookup(path);
  inode* inode = get_inode(tree_index);
  inode->mtime = time(NULL);
  if (size > inode->size) {
    return grow_inode(inode, size);
  }
  else {
    return shrink_inode(inode, size);
  }
}

int storage_mknod(const char* path, int mode) {
  filepath file_path = to_filepath(path);
  int tree_index = tree_lookup(file_path.crumbs);
  if (tree_index < 0 || streq(file_path.file, "")) {
    return -ENOENT;
  }
  int inode_index = alloc_inode();
  int page_index = alloc_page();
  if (inode_index < 0 && page_index >= 0) {
    free_page(page_index);
    return -ENOSPC;
  }
  if (inode_index >= 0 && page_index < 0) {
    free_inode(inode_index);
    return -ENOSPC;

  }
  inode* inode = get_inode(inode_index);
  inode->ptrs[0] = page_index;
  inode->mode = mode;
  inode->atime = time(NULL);
  inode->ctime = time(NULL);
  inode->mtime = time(NULL);
  int rv = directory_put(get_inode(tree_index), file_path.file, inode_index);
  assert(rv == 0);
  if (is_folder(mode) || is_link(mode)) {
    inode->size = 4096;
  }
  return 0;
}

int storage_unlink(const char* path) {
  filepath file_path = to_filepath(path);
  int tree_index = tree_lookup(file_path.crumbs);
  inode* dir = get_inode(tree_index);
  dirent* fileEnt = directory_lookup(dir, file_path.file);
  if (tree_index < 0 || fileEnt == 0) {
    return -ENOENT;
  }
  inode* inode = get_inode(fileEnt->inum);
  if (inode->refs == 0) {
    if (is_folder(inode->mode)) {
      char nestedPath[4096];
      strncpy(nestedPath, path, 4096);
      join_to_path(nestedPath, file_path.file);
      int rv = storage_rmdir(nestedPath);
      if (rv != 0) {
        return rv;
      }
    }
    free_inode(fileEnt->inum);
  }
  else {
    inode->refs--;
  }
  directory_delete(dir, file_path.file);
  return 0;
}

int storage_link(const char* from, const char* to) {
  filepath file_path = to_filepath(to);
  int to_index = tree_lookup(to);
  if (to_index >= 0) {
    return -EEXIST;
  }
  int from_index = tree_lookup(from);
  int to_parant_index = tree_lookup(file_path.crumbs);
  if (from_index < 0 || to_parant_index < 0) {
    return -ENOENT;
  }
  inode* from_file = get_inode(from_index);
  if (is_folder(from_file->mode)) {
    return -EISDIR;
  }
  inode* to_parant = get_inode(to_parant_index);
  int rv = directory_put(to_parant, file_path.file, from_index);
  if (rv == 0) {
    from_file->refs++;
    return 0;
  }
  return rv;
}

int storage_rename(const char *from, const char *to) {
  filepath old_path = to_filepath(from);
  filepath new_path = to_filepath(to);
  if (streq(old_path.crumbs, new_path.crumbs)) {
    int tree_index = tree_lookup(old_path.crumbs);
    dirent* entry = directory_lookup(get_inode(tree_index), old_path.file);
    if (entry > 0) {
      strncpy(entry->name, new_path.file, 48);
      return 0;
    }
    else {
      return -ENOENT;
    }
  }
  else {
    return -1;
  }
}

int storage_set_time(const char* path, const struct timespec ts[2]) {
  int rv = 0;
  int tree_index = tree_lookup(path);
  inode* file = get_inode(tree_index);
  file->atime = ts[0].tv_sec;
  file->mtime = ts[1].tv_sec;
  return rv;
}

slist* storage_list(const char* path) {
  return directory_list(path);
}

int storage_access(const char* path, int mask) {
  int rv = 0;
  if (mask = F_OK) {
    rv = tree_lookup(path) < 0 ? -ENOENT : 0;
  }
  return rv;
}

int storage_mkdir(const char* path, mode_t mode) {
  int rv = storage_mknod(path, __S_IFDIR | mode);
  if (rv != 0) {
    return -ENOSPC;
  }
  filepath file_path = to_filepath(path);
  inode* dir = get_inode(tree_lookup(path));
  directory_put(dir, ".", tree_lookup(path));
  directory_put(dir, "..", tree_lookup(file_path.crumbs));
  return rv;
}

int storage_rmdir(const char* path) {
  int tree_index = tree_lookup(path);
  filepath file_path = to_filepath(path);
  int parent_index = tree_lookup(file_path.crumbs);
  inode* dir = get_inode(tree_index);
  inode* parent = get_inode(parent_index);
  int rv;
  slist* inode = directory_list(path);
  while (inode != 0) {
    char filePath[4096];
    strncpy(filePath, path, 4096);
    join_to_path(filePath, inode->data);
    rv = storage_unlink(filePath);
    if (rv != 0) {
      return rv;
    }
    inode = inode->next;
  }
  free_inode(tree_index);
  return directory_delete(parent, file_path.file);
}

int storage_chmod(const char* path, mode_t mode) {
  int tree_index = tree_lookup(path);
  inode* inode = get_inode(tree_index);
  inode->mode = mode;
  inode->ctime = time(NULL);
  return 0;
}

filepath to_filepath(const char* path) {
  filepath ret;
  strcpy(ret.crumbs, "/");
  slist* slist = s_split(path, '/');
  while (slist != 0) {
    if (slist->next == 0) {
      strncpy(ret.file, slist->data, 255);
    }
    else {
      join_to_path(ret.crumbs, slist->data);
    }
    slist = slist->next;
  }
  s_free(slist);
  return ret;
}

int storage_symlink(const char* to, const char* from) {
  int fromIdx = tree_lookup(from);
  if (fromIdx >= 0) {
    return -EEXIST;
  }
  int rv = storage_mknod(from, __S_IFLNK | 0777);
  if (rv < 0) {
    return rv;
  }
  fromIdx = tree_lookup(from);
  inode* link = get_inode(fromIdx);
  strncpy(pages_get_page(link->ptrs[0]), to, 4096);
  return 0;
}

int storage_readlink(const char* path, char* buf, size_t size) {
  int link_index = tree_lookup(path);
  if (link_index < 0) {
    return -ENOENT;
  }
  inode* link = get_inode(link_index);
  if (is_link(link->mode)) {
    size = min(size, 4096);
    strncpy(buf, pages_get_page(link->ptrs[0]), size);
    return 0;
  } else {
    return -EPERM;
  }
}
