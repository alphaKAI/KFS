#include "kfs.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static KFS_File *new_KFS_File_impl(void) {
  KFS_File *file = xmalloc(sizeof(KFS_File));
  file->data = NULL;
  return file;
}

static KFS_Dir *new_KFS_Dir_impl(void) {
  KFS_Dir *dir = xmalloc(sizeof(KFS_Dir));
  dir->childs = new_AVLTree();
  return dir;
}

KFS_Entry *make_entry(sds name, int entry_type) {
  KFS_Entry *entry = xmalloc(sizeof(KFS_Entry));

  switch (entry_type) {
  case tKFS_Dir: {
    entry->dentry = new_KFS_Dir_impl();
    entry->mode = S_IFDIR | 0755;
    entry->size = 4096;
    break;
  }
  case tKFS_File: {
    entry->fentry = new_KFS_File_impl();
    entry->mode = S_IFREG | 0444;
    entry->size = 0;
    break;
  }
  default:
    fprintf(stderr, "Unkown entry_type\n");
    xfree(&entry);
    return NULL;
  }

  entry->name = sdsempty();
  sdscpy(entry->name, name);
  entry->entry_type = entry_type;
  entry->nlink = 1;
  entry->prev = NULL;
  entry->uid = getuid();
  entry->gid = getgid();

  clock_getres(CLOCK_REALTIME, &entry->atime);
  clock_getres(CLOCK_REALTIME, &entry->mtime);

  return entry;
}

sds kfs_getPwd(KFS_Entry *entry) {
  Vector *ret = new_vec();
  sds res = sdsempty();
  size_t i = 0;

  while (entry != NULL) {
    if (i == 0) {
      sdscpy(res, entry->name);
    } else {
      vec_push(ret, entry->name);
    }

    entry = entry->prev;
    i++;
  }

  if (ret->len > 0) {
    res = sdscat(res, vecstrjoin(ret, sdsnew("/")));
  }

  return res;
}

KFS_Entry *new_KFS_File(sds name) {
  KFS_Entry *entry = make_entry(name, tKFS_File);
  return entry;
}

KFS_Entry *new_KFS_Dir(sds name) {
  KFS_Entry *entry = make_entry(name, tKFS_Dir);
  return entry;
}