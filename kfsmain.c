#define FUSE_USE_VERSION 26

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

#define DEBUG 1

#if DEBUG
#define DEBUG_CODE(code) code
#else
#define DEBUG_CODE(code)
#endif

static int itf_fuse_kfs_getattr(const char *path, struct stat *stbuf);
static int itf_fuse_kfs_readdir(const char *path, void *buf,
                                fuse_fill_dir_t filler, off_t offset,
                                struct fuse_file_info *fi);
static int itf_fuse_kfs_open(const char *path, struct fuse_file_info *fi);
static int itf_fuse_kfs_read(const char *path, char *buf, size_t size,
                             off_t offset, struct fuse_file_info *fi);
static int itf_fuse_kfs_write(const char *path, const char *buf, size_t size,
                              off_t offset, struct fuse_file_info *fi);

static int itf_fuse_kfs_mkdir(const char *path, mode_t mode);
static int itf_fuse_kfs_access(const char *path, int mode);
static int itf_fuse_kfs_create(const char *path, mode_t mode,
                               struct fuse_file_info *fi);
static int itf_fuse_kfs_utimens(const char *path, const struct timespec tv[2]);
static int itf_fuse_kfs_unlink(const char *path);

static struct fuse_operations kfs_ops = {.getattr = itf_fuse_kfs_getattr,
                                         .readdir = itf_fuse_kfs_readdir,
                                         .open = itf_fuse_kfs_open,
                                         .read = itf_fuse_kfs_read,
                                         .write = itf_fuse_kfs_write,
                                         .mkdir = itf_fuse_kfs_mkdir,
                                         .access = itf_fuse_kfs_access,
                                         .create = itf_fuse_kfs_create,
                                         .utimens = itf_fuse_kfs_utimens,
                                         .unlink = itf_fuse_kfs_unlink};

static KFS_Entry *KFS_ROOT;

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
  for (size_t i = 0; i < paths->len - 1; i++) {
    parent = kfs_find_on(parent, paths->data[i]);
  }

  dtr.parent = parent;
  dtr.lastname = paths->data[paths->len - 1];
  return dtr;
}

static int itf_fuse_kfs_getattr(const char *path, struct stat *stbuf) {
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  DEBUG_CODE(FILE * fp;);
  DEBUG_CODE({
    fp = fopen("/home/alphakai/Nextcloud/kernel_hacking/filesystem/fuse/"
               "kfs/log_getattr",
               "w");
    if (fp) {
      fprintf(fp, "path: %s\n", path);
      fprintf(fp, "spath: %s\n", spath);
      fprintf(fp, "entry is null?: %s\n", entry == NULL ? "yes" : "no");
      fprintf(fp, "keys...\n");
      Vector *vtree = kfs_getTree(KFS_ROOT);
      VecForeach(vtree, elem, { fprintf(fp, "elem - %s\n", (sds)elem); });
    }
  });

  if (entry == NULL) {
    res = -ENOENT;
  } else {
    stbuf->st_mode = entry->mode;
    stbuf->st_nlink = entry->nlink;
    stbuf->st_size = entry->size;
  }

  DEBUG_CODE({
    if (fp) {
      fclose(fp);
    }
  });

  sdsfree(spath);
  return res;
}

static int itf_fuse_kfs_readdir(const char *path, void *buf,
                                fuse_fill_dir_t filler, off_t offset,
                                struct fuse_file_info *fi) {
  (void)offset;
  (void)fi;

  DEBUG_CODE(FILE * fp);
  DEBUG_CODE({
    fp = fopen("/home/alphakai/Nextcloud/kernel_hacking/filesystem/fuse/"
               "kfs/log_readdir",
               "w");
    if (fp != NULL) {
      fprintf(fp, "path: %s\n", path);
    }
  });

  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  if (entry == NULL) {
    res = -ENOENT;
  } else if (EntryIsFile(entry)) {
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
    if (fp)
      fclose(fp);
  });

  sdsfree(spath);
  return res;
}

static int itf_fuse_kfs_open(const char *path, struct fuse_file_info *fi) {
  (void)fi;
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  DEBUG_CODE(FILE * fp;);
  DEBUG_CODE({
    fp = fopen("/home/alphakai/Nextcloud/kernel_hacking/"
               "filesystem/fuse/kfs/log_open",
               "w");
    if (fp != NULL) {
      fprintf(fp, "path: %s\n", path);
      fclose(fp);
    }
  });

  if (entry == NULL) {
    res = -ENONET;
  }

  sdsfree(spath);
  return res;
}

static int itf_fuse_kfs_read(const char *path, char *buf, size_t size,
                             off_t offset, struct fuse_file_info *fi) {
  (void)fi;
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  DEBUG_CODE(FILE * fp;);
  DEBUG_CODE({
    FILE *fp = fopen(
        "/home/alphakai/Nextcloud/kernel_hacking/filesystem/fuse/kfs/log_read",
        "w");
    if (fp != NULL) {
      fprintf(fp, "path: %s\n", path);
      fclose(fp);
    }
  });

  if (entry == NULL) {
    res = -ENONET;
  } else {
    SizedData *sdata = kfs_read(entry);
    size_t len = sdata->size;

    if (offset < (off_t)len) {
      if (offset + size > len) {
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

static int itf_fuse_kfs_write(const char *path, const char *buf, size_t size,
                              off_t offset, struct fuse_file_info *fi) {
  (void)fi;
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  if (entry == NULL) {
    // create a file
    itf_fuse_kfs_create(path, 444, NULL);
    entry = kfs_find(KFS_ROOT, spath);
  }

  kfs_write(entry, buf, size, offset);
  res = size;

  sdsfree(spath);
  return res;
}

static int itf_fuse_kfs_mkdir(const char *path, mode_t mode) {
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  DEBUG_CODE(FILE * fp;);
  DEBUG_CODE({
    fp = fopen(
        "/home/alphakai/Nextcloud/kernel_hacking/filesystem/fuse/kfs/log_mkdir",
        "w");
    if (fp != NULL) {
      fprintf(fp, "path: %s\n", path);
    }
  });

  if (entry != NULL) {
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
    if (fp)
      fclose(fp);
  });

  sdsfree(spath);
  return res;
}

static int itf_fuse_kfs_access(const char *path, int mode) {
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  DEBUG_CODE(FILE * fp;);
  DEBUG_CODE({
    fp = fopen("/home/alphakai/Nextcloud/kernel_hacking/filesystem/fuse/"
               "kfs/log_access",
               "w");
    if (fp != NULL) {
      fprintf(fp, "path: %s\n", path);
      fclose(fp);
    }
  });

  // TODO: permission check
  (void)mode;

  if (entry == NULL) {
    res = -ENONET;
  }

  sdsfree(spath);
  return res;
}

static int itf_fuse_kfs_create(const char *path, mode_t mode,
                               struct fuse_file_info *fi) {
  (void)fi;
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  DEBUG_CODE(FILE * fp;);
  DEBUG_CODE({
    fp = fopen("/home/alphakai/Nextcloud/kernel_hacking/filesystem/fuse/"
               "kfs/log_create",
               "w");
    if (fp != NULL) {
      fprintf(fp, "path: %s\n", path);
      fclose(fp);
    }
  });

  if (entry != NULL) {
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

static int itf_fuse_kfs_unlink(const char *path) {
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  if (entry == NULL) {
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

static int itf_fuse_kfs_utimens(const char *path, const struct timespec tv[2]) {
  int res = 0;
  sds spath = sdsnew(path);
  KFS_Entry *entry = kfs_find(KFS_ROOT, spath);

  if (entry == NULL) {
    res = -ENOENT;
  }

  entry->atime = tv[0];
  entry->mtime = tv[1];

  sdsfree(spath);
  return res;
}

static void shell_main(void) {
  KFS_Entry *root = new_KFS_Dir("/");
  kfs_shell(root);
}

int main(int argc, char *argv[]) {
  if (argc == 2 && strcmp((const char *)argv[1], "-s") == 0) {
    shell_main();
  } else {
    if (argc < 2) {
      printf("error!");
    } else {
      kfs_init();
      fuse_main(argc, argv, &kfs_ops, NULL);
    }
  }
  return 0;
}
