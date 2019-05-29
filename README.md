# KFS
Toy file system, currently using FUSE.  
In the future, I want to implement KFS in Kernel land.  

## Features

- mkdir
- read
- write
- readdir
- getattr
- access
- create
- unlink
- utimens

## Architecture

Any inode is typed as KFS_Entry. if an entry is a File, the entry has fentry field with content of the file, if an entry is directory, the entry has AVLTree of children elements.  

Strucure defenition(in `entry.h`)  

```c
enum { tKFS_Dir, tKFS_File };

typedef struct {
  char *data;
} KFS_File;

typedef struct {
  AVLTree *childs;
} KFS_Dir;

typedef struct KFS_Entry {
  sds name;
  int entry_type; // KFS_Dir or KFS_File

  union {
    KFS_Dir *dentry;
    KFS_File *fentry;
  };

  mode_t mode;
  off_t size;
  nlink_t nlink;
  struct KFS_Entry *prev;
  struct timespec atime;
  struct timespec mtime;
} KFS_Entry;
```

## License
KFS is released under the MIT license.  
Please see details for `LICENSE`.  
Copyright © 2019, Akihiro Shoji