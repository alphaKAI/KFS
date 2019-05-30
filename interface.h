#ifndef __INTERFACE_HEADER_INCLUDED__
#define __INTERFACE_HEADER_INCLUDED__
#define FUSE_USE_VERSION 26

#include "kfs.h"
#include <fuse.h>
#include <stddef.h>

int itf_fuse_kfs_getattr(const char *path, struct stat *stbuf);
int itf_fuse_kfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi);
int itf_fuse_kfs_open(const char *path, struct fuse_file_info *fi);
int itf_fuse_kfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi);
int itf_fuse_kfs_write(const char *path, const char *buf, size_t size,
                       off_t offset, struct fuse_file_info *fi);

int itf_fuse_kfs_mkdir(const char *path, mode_t mode);
int itf_fuse_kfs_access(const char *path, int mode);
int itf_fuse_kfs_create(const char *path, mode_t mode,
                        struct fuse_file_info *fi);
int itf_fuse_kfs_utimens(const char *path, const struct timespec tv[2]);
int itf_fuse_kfs_unlink(const char *path);
int itf_fuse_kfs_chmod(const char *path, mode_t mode);

extern struct fuse_operations kfs_ops;
extern KFS_Entry *KFS_ROOT;

void kfs_init(void);

#endif