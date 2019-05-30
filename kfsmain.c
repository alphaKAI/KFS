#include "kfs.h"
#include <stdio.h>

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
