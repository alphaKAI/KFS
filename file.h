#ifndef __FILE_HDEADER_INCLUDED__
#define __FILE_HDEADER_INCLUDED__

#include "kfs.h"

void kfs_write(KFS_Entry *this, const char *buf, long int size,
               long int offset);
SizedData *kfs_read(KFS_Entry *this);

#endif