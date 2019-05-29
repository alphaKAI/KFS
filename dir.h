#ifndef __DIR_HEADER_INCLUDED__
#define __DIR_HEADER_INCLUDED__
#include "kfs.h"

void kfs_append_child(KFS_Entry *this, KFS_Entry *child);
KFS_Entry *kfs_find_on(KFS_Entry *this, sds name);
KFS_Entry *kfs_find(KFS_Entry *this, sds path);
Vector *kfs_getCurrentList(KFS_Entry *this);
Vector *kfs_getTree(KFS_Entry *this);

#endif