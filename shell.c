#include "kfs.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define Mkdir "mkdir"
#define Chdir "cd"
#define Touch "touch"
#define Ls "ls"
#define Pwd "pwd"
#define Tree "tree"
#define CopyFromHost "copyFromHost"
#define Cat "cat"
#define Help "help"
#define Copy "cp"

static const char *KFSCommands[] = {Mkdir, Chdir,        Touch, Ls,   Pwd,
                                    Tree,  CopyFromHost, Cat,   Help, Copy};

KFSShellContext *new_KFSShellContext(KFS_Entry *root) {
  assert_is_dir(root);
  KFSShellContext *ctx = xmalloc(sizeof(KFSShellContext));
  ctx->root = root;
  ctx->cwd = root;
  return ctx;
}

bool kfs_mkdir(KFSShellContext *ctx, sds name) {
  WithCtx(ctx, {
    KFS_Entry *ret = kfs_find_on(cwd, name);
    if (ret == NULL) {
      kfs_append_child(cwd, new_KFS_Dir(name));
      return true;
    }

    return false;
  });
}

bool kfs_chdir(KFSShellContext *ctx, sds target) {
  WithCtx(ctx, {
    if (strcmp(target, "..") == 0) {
      if (cwd->prev != NULL) {
        KFS_Entry *prev = cwd->prev;
        if (EntryIsDir(prev)) {
          ctx->cwd = prev;
          return true;
        }
        return false;
      }
      return false;
    }

    KFS_Entry *tentry = kfs_find(cwd, target);
    if (tentry == NULL) {
      return false;
    }

    if (EntryIsDir(tentry)) {
      ctx->cwd = tentry;
      return true;
    }

    return false;
  });
}

bool kfs_touch(KFSShellContext *ctx, sds name) {
  WithCtx(ctx, {
    KFS_Entry *ret = kfs_find_on(cwd, name);
    if (ret == NULL) {
      kfs_append_child(cwd, new_KFS_File(name));
      return true;
    }

    return false;
  });
}

bool kfs_ls(KFSShellContext *ctx, sds path) {
  KFS_Entry *entry = kfs_find(ctx->root, path);

  if (!entry) {
    return false;
  }

  Vector *vls = kfs_getCurrentList(entry);

  VecForeach(vls, elem, { printf("%s\n", (sds)elem); });

  return true;
}

bool kfs_pwd(KFSShellContext *ctx) {
  printf("%s\n", kfs_getPwd(ctx->cwd));

  return true;
}

bool kfs_tree(KFSShellContext *ctx) {
  Vector *vtree = kfs_getTree(ctx->cwd);
  VecForeach(vtree, elem, { printf("%s\n", (sds)elem); });
  return true;
}

bool kfs_help(KFSShellContext *ctx __attribute__((unused))) {
  size_t Commands_len = sizeof(KFSCommands) / sizeof(KFSCommands[0]);

  for (size_t i = 0; i < Commands_len; i++) {
    printf("%s\n", KFSCommands[i]);
  }

  return true;
}

bool kfs_copyFromHost(KFSShellContext *ctx, sds src, sds dst) {
  char cwd_name[1024];
  if (getcwd(cwd_name, sizeof(cwd_name)) == NULL) {
    fprintf(stderr, "getcwd error\n");
    exit(EXIT_FAILURE);
  }

  WithCtx(ctx, {
    src = sdscatprintf(sdsempty(), "%s/%s", cwd_name, src);
    FILE *src_fp = fopen(src, "rb");

    if (src_fp == NULL) {
      return false;
    }
    fclose(src_fp);

    if (kfs_find_on(cwd, dst) != NULL) {
      return false;
    }

    sds buf = readText(src);
    KFS_Entry *new_file = new_KFS_File(dst);
    kfs_write(new_file, (char *)buf, sdslen(buf) + 1, 0);
    kfs_append_child(cwd, new_file);
    return true;
  });
}

bool kfs_cat(KFSShellContext *ctx, sds name) {
  WithCtx(ctx, {
    KFS_Entry *ret = kfs_find_on(cwd, name);
    if (ret == NULL) {
      return false;
    }
    if (EntryIsDir(ret)) {
      return false;
    }

    SizedData *buf = kfs_read(ret);
    printf("%s\n", (sds)buf->data);

    return true;
  });
}

void kfs_shell(KFS_Entry *root) {
  KFSShellContext *ctx = new_KFSShellContext(root);
  char input[1024];

  while (1) {
    printf("%s > ", kfs_getPwd(ctx->cwd));

    memset(input, 0, sizeof(input));
    fgets(input, 1024, stdin);
    for (size_t i = 0; i < 1024; i++) {
      if (input[i] == '\n') {
        input[i] = '\0';
        break;
      }
    }

    if (strcmp(input, "exit") == 0) {
      printf("exit!\n");
      break;
    }
    if (strcmp(input, "segv") == 0) {
      *((char *)1 - 1) = 1;
      break;
    }

    sds scmd = sdsnew(input);
    Vector *cmds = sdssplitvec(scmd, ' ');
    bool result;

#define ifcmdIs(e) if (strcmp(cmds->data[0], e) == 0)

    ifcmdIs(Mkdir) { result = kfs_mkdir(ctx, cmds->data[1]); }
    else ifcmdIs(Touch) {
      result = kfs_touch(ctx, cmds->data[1]);
    }
    else ifcmdIs(Chdir) {
      result = kfs_chdir(ctx, cmds->data[1]);
    }
    else ifcmdIs(Ls) {
      if (cmds->len == 1) {
        vec_push(cmds, kfs_getPwd(ctx->cwd));
      }
      result = kfs_ls(ctx, cmds->data[1]);
    }
    else ifcmdIs(Pwd) {
      result = kfs_pwd(ctx);
    }
    else ifcmdIs(Tree) {
      result = kfs_tree(ctx);
    }
    else ifcmdIs(CopyFromHost) {
      result = kfs_copyFromHost(ctx, cmds->data[1], cmds->data[2]);
    }
    else ifcmdIs(Cat) {
      result = kfs_cat(ctx, cmds->data[1]);
    }
    else ifcmdIs(Help) {
      result = kfs_help(ctx);
    }

    if (!result) {
      printf("command error\n");
    }

    sdsfree(scmd);
  }
}