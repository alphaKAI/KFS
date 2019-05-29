#ifndef __ENTRY_HEADER_INCLUDED__
#define __ENTRY_HEADER_INCLUDED__
#include <assert.h>
#include <time.h>

enum { tKFS_Dir, tKFS_File };

typedef struct {
  char *data;
} KFS_File;

typedef struct {
  AVLTree *childs;
} KFS_Dir;

#define GetAVLTree(entry) (entry->dentry->childs)
#define assert_is_file(entry) (assert(entry->entry_type == tKFS_File))
#define assert_is_dir(entry) (assert(entry->entry_type == tKFS_Dir))
#define GetNodeValueAs(node, as_type) ((as_type)node->value)
#define EntryIsFile(entry) (entry->entry_type == tKFS_File ? true : false)
#define EntryIsDir(entry) (entry->entry_type == tKFS_Dir ? true : false)
#define GetKFSDir(entry) entry->dentry
#define GetKFSFile(entry) entry->fentry

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

KFS_Entry *make_entry(sds name, int entry_type);
sds kfs_getPwd(KFS_Entry *entry);
KFS_Entry *new_KFS_File(sds name);
KFS_Entry *new_KFS_Dir(sds name);

#include <string.h>
static inline int path_cmp(void *lhs, void *rhs) {
  char *lname = (char *)lhs;
  char *rname = (char *)rhs;

  int ret = strcmp(lname, rname);

  if (ret == 0) {
    return 0;
  }
  if (ret < 0) {
    return -1;
  }
  return 1;
}

#endif