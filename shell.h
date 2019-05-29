#ifndef __SHELL_HEADER_INCLUDED__
#define __SHELL_HEADER_INCLUDED__

#include "kfs.h"

typedef struct {
  KFS_Entry *root;
  KFS_Entry *cwd;
} KFSShellContext;

#define WithCtx(ctx, proc)                                                     \
  do {                                                                         \
    KFS_Entry *root = ctx->root;                                               \
    KFS_Entry *cwd = ctx->cwd;                                                 \
                                                                               \
    proc;                                                                      \
                                                                               \
    ctx->root = root;                                                          \
    ctx->cwd = cwd;                                                            \
  } while (0)

KFSShellContext *new_KFSShellContext(KFS_Entry *root);
bool kfs_mkdir(KFSShellContext *ctx, sds name);
bool kfs_chdir(KFSShellContext *ctx, sds target);
bool kfs_touch(KFSShellContext *ctx, sds name);
bool kfs_ls(KFSShellContext *ctx, sds path);
bool kfs_pwd(KFSShellContext *ctx);
bool kfs_tree(KFSShellContext *ctx);
bool kfs_help(KFSShellContext *ctx __attribute__((unused)));
bool kfs_copyFromHost(KFSShellContext *ctx, sds src, sds dst);
bool kfs_cat(KFSShellContext *ctx, sds name);

void kfs_shell(KFS_Entry *root);
#endif