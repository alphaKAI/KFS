#include "kfs.h"
#include <stdlib.h>

void kfs_write(KFS_Entry *this, const char *buf, long int size,
               long int offset) {
  assert_is_file(this);

  KFS_File *file = GetKFSFile(this);

  if(offset) {
    if(this->size < offset + size) {
      size_t append_size = (offset + size) - this->size;
      file->data = realloc(file->data, this->size + append_size);
      this->size += append_size;
    }
  } else {
    if(this->size == 0) {
      file->data = xmalloc(size);
      this->size = size;
    } else if(this->size < size) {
      file->data = realloc(file->data, size);
      this->size = size;
    }
  }
  memcpy(file->data + offset, buf, size);
}

SizedData *kfs_read(KFS_Entry *this) {
  assert_is_file(this);
  KFS_File *file = GetKFSFile(this);

  SizedData *sdata = new_SizedData();
  sdata->data = file->data;
  sdata->size = this->size;

  return sdata;
}
