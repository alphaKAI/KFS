#include "kfs.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEBUG 0

#if DEBUG
#define USER_HOME_DIR "/home/alphakai/"
#define KFS_DIR "Nextcloud/kernel_hacking/filesystem/fuse/kfs/"
#define LOG_OUTPUT_BASE_DIR USER_HOME_DIR KFS_DIR
#define DEBUG_CODE(code) code
#else
#define DEBUG_CODE(code)
#endif

KFS_Entry *KFS_ROOT;

struct fuse_operations kfs_ops = {.getattr = itf_fuse_kfs_getattr,
                                  .readdir = itf_fuse_kfs_readdir,
                                  .open = itf_fuse_kfs_open,
                                  .read = itf_fuse_kfs_read,
                                  .write = itf_fuse_kfs_write,
                                  .mkdir = itf_fuse_kfs_mkdir,
                                  .access = itf_fuse_kfs_access,
                                  .create = itf_fuse_kfs_create,
                                  .utimens = itf_fuse_kfs_utimens,
                                  .unlink = itf_fuse_kfs_unlink,
                                  .chmod = itf_fuse_kfs_chmod};

void kfs_init(void) {
  KFS_ROOT = new_KFS_Dir(sdsnew("/"));

  KFSShellContext *ctx = new_KFSShellContext(KFS_ROOT);
  kfs_copyFromHost(ctx, "avl.c", "avl.c");

  /*
    Initialize...
  */
}

typedef struct {
  KFS_Entry *parent;
  sds lastname;
} DownToResult;

DownToResult downToLast(sds spath) {
  DownToResult dtr;

  KFS_Entry *parent = KFS_ROOT;
  Vector *paths = sdssplitvec(spath, '/');
  for(size_t i = 0; i < paths->len - 1; i++) {
    parent = kfs_find_on(parent, paths->data[i]);
  }

  dtr.parent = parent;
  dtr.lastname = paths->data[paths->len - 1];
  return dtr;
}

int itf_fuse_kfs_getattr(const char *path, struct stat *stbuf) {
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  DEBUG_CODE(FILE * fp;);
  DEBUG_CODE({
    fp = fopen(LOG_OUTPUT_BASE_DIR "log_getattr", "w");
    if(fp) {
      fprintf(fp, "path: %s\n", path);
      fprintf(fp, "spath: %s\n", spath);
      fprintf(fp, "entry is null?: %s\n", entry == NULL ? "yes" : "no");
      fprintf(fp, "keys...\n");
      Vector *vtree = kfs_getTree(KFS_ROOT);
      VecForeach(vtree, elem, { fprintf(fp, "elem - %s\n", (sds)elem); });
    }
  });

  if(entry == NULL) {
    res = -ENOENT;
  } else {
    stbuf->st_mode = entry->mode;
    stbuf->st_nlink = entry->nlink;
    stbuf->st_size = entry->size;
    stbuf->st_uid = entry->uid;
    stbuf->st_gid = entry->gid;
  }

  DEBUG_CODE({
    if(fp) {
      fclose(fp);
    }
  });

  sdsfree(spath);
  return res;
}

int itf_fuse_kfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi) {
  (void)offset;
  (void)fi;

  DEBUG_CODE(FILE * fp);
  DEBUG_CODE({
    fp = fopen(LOG_OUTPUT_BASE_DIR "log_readdir", "w");
    if(fp != NULL) {
      fprintf(fp, "path: %s\n", path);
    }
  });

  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  if(entry == NULL) {
    res = -ENOENT;
  } else if(EntryIsFile(entry)) {
    filler(buf, entry->name, NULL, 0);
  } else {
    Vector *elems = kfs_getCurrentList(entry);

    VecForeach(elems, elem, { filler(buf, (sds)elem, NULL, 0); });
    DEBUG_CODE({
      VecForeach(elems, elem, { fprintf(fp, "elem - %s\n", (sds)elem); });
    });

    // TODO: free elems
  }

  DEBUG_CODE({
    if(fp)
      fclose(fp);
  });

  sdsfree(spath);
  return res;
}

int itf_fuse_kfs_open(const char *path, struct fuse_file_info *fi) {
  (void)fi;
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  DEBUG_CODE(FILE * fp;);
  DEBUG_CODE({
    fp = fopen(LOG_OUTPUT_BASE_DIR "log_open", "w");
    if(fp != NULL) {
      fprintf(fp, "path: %s\n", path);
      fclose(fp);
    }
  });

  if(entry == NULL) {
    res = -ENONET;
  }

  sdsfree(spath);
  return res;
}

int itf_fuse_kfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
  (void)fi;
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  DEBUG_CODE(FILE * fp;);
  DEBUG_CODE({
    fp = fopen(LOG_OUTPUT_BASE_DIR "log_read", "w");
    if(fp != NULL) {
      fprintf(fp, "path: %s\n", path);
      fclose(fp);
    }
  });

  if(entry == NULL) {
    res = -ENONET;
  } else {
    SizedData *sdata = kfs_read(entry);
    size_t len = sdata->size;

    if(offset < (off_t)len) {
      if(offset + size > len) {
        size = len - offset;
      }
      memcpy(buf, sdata->data, size);
      res = size;
    } else {
      res = 0;
    }
  }

  sdsfree(spath);
  return res;
}

int itf_fuse_kfs_write(const char *path, const char *buf, size_t size,
                       off_t offset, struct fuse_file_info *fi) {
  (void)fi;
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  if(entry == NULL) {
    // create a file
    itf_fuse_kfs_create(path, 444, NULL);
    entry = kfs_find(KFS_ROOT, spath);
  }

  kfs_write(entry, buf, size, offset);
  res = size;

  sdsfree(spath);
  return res;
}

int itf_fuse_kfs_mkdir(const char *path, mode_t mode) {
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  DEBUG_CODE(FILE * fp;);
  DEBUG_CODE({
    fp = fopen(LOG_OUTPUT_BASE_DIR "log_mkdir", "w");
    if(fp != NULL) {
      fprintf(fp, "path: %s\n", path);
    }
  });

  if(entry != NULL) {
    res = -EEXIST;
  } else {
    DownToResult dtr = downToLast(spath);
    KFS_Entry *parent = dtr.parent;
    sds target = dtr.lastname;

    KFS_Entry *new_dir = new_KFS_Dir(target);
    new_dir->mode = mode | S_IFDIR;
    kfs_append_child(parent, new_dir);
  }

  DEBUG_CODE({
    if(fp)
      fclose(fp);
  });

  sdsfree(spath);
  return res;
}

int itf_fuse_kfs_access(const char *path, int mode) {
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  DEBUG_CODE(FILE * fp;);
  DEBUG_CODE({
    fp = fopen(LOG_OUTPUT_BASE_DIR "log_access", "w");
    if(fp != NULL) {
      fprintf(fp, "path: %s\n", path);
      fclose(fp);
    }
  });

  // TODO: permission check
  (void)mode;

  if(entry == NULL) {
    res = -ENONET;
  }

  sdsfree(spath);
  return res;
}

int itf_fuse_kfs_create(const char *path, mode_t mode,
                        struct fuse_file_info *fi) {
  (void)fi;
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  DEBUG_CODE(FILE * fp;);
  DEBUG_CODE({
    fp = fopen(LOG_OUTPUT_BASE_DIR "log_create", "w");
    if(fp != NULL) {
      fprintf(fp, "path: %s\n", path);
      fclose(fp);
    }
  });

  if(entry != NULL) {
    res = -EEXIST;
  } else {
    DownToResult dtr = downToLast(spath);
    KFS_Entry *parent = dtr.parent;
    sds target = dtr.lastname;

    KFS_Entry *new_file = new_KFS_File(target);
    new_file->mode = mode | S_IFREG;
    kfs_append_child(parent, new_file);
  }

  sdsfree(spath);
  return res;
}

int itf_fuse_kfs_unlink(const char *path) {
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  if(entry == NULL) {
    res = -ENOENT;
  } else {
    DownToResult dtr = downToLast(spath);
    KFS_Entry *parent = dtr.parent;
    sds target = dtr.lastname;

    avl_delete(GetAVLTree(parent), target, path_cmp);
  }

  sdsfree(spath);
  return res;
}

int itf_fuse_kfs_utimens(const char *path, const struct timespec tv[2]) {
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  if(entry == NULL) {
    res = -ENOENT;
  } else {
    entry->atime = tv[0];
    entry->mtime = tv[1];
  }

  sdsfree(spath);
  return res;
}

/** Change the permission bits of a file */
int itf_fuse_kfs_chmod(const char *path, mode_t mode) {
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  if(entry == NULL) {
    res = -ENOENT;
  } else {
    entry->mode = mode | (EntryIsFile(entry) ? S_IFREG : S_IFDIR);
  }

  sdsfree(spath);
  return res;
}

/** Change the owner and group of a file */
// int (*chown)(const char *, uid_t, gid_t);
