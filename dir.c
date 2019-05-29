#include "kfs.h"

void kfs_append_child(KFS_Entry *this, KFS_Entry *child) {
  assert_is_dir(this);
  avl_insert(GetAVLTree(this), child->name, child, path_cmp);
  child->prev = this;
}

KFS_Entry *kfs_find_on(KFS_Entry *this, sds name) {
  assert_is_dir(this);
  return avl_find(GetAVLTree(this), name, path_cmp);
}

KFS_Entry *kfs_find(KFS_Entry *this, sds path) {
  assert_is_dir(this);
  sds slash = sdsnew("/");

  // check root; return this(it-self)
  if (sdscmp(path, slash) == 0) {
    if (sdscmp(this->name, slash) == 0) {
      return this;
    } else {
      return NULL;
    }
  }

  // 先頭の '/' をとる．
  if (sdslen(path) > 1 && path[0] == '/') {
    sdsrange(path, 1, -1);
  }

  Vector *paths = sdssplitvec(path, '/');
  KFS_Entry *tentry = this;

  for (size_t i = 0; i < paths->len; i++) {
    sds tpath = paths->data[i];
    if (tentry == NULL) {
      return NULL;
    }

    // pathの終端の場合，探すのをここでうちきる．
    if (i + 1 == paths->len) {
      if (tentry->entry_type == tKFS_File) {
        if (sdscmp(tpath, tentry->name) == 0) {
          return tentry;
        } else {
          return NULL;
        }
      } else {
        return kfs_find_on(tentry, tpath);
      }
    } else {
      // 途中にあったのがファイルの場合，目的のものはない(それ以上ほれないため)
      if (tentry->entry_type == tKFS_File) {
        return NULL;
      } else {
        tentry = kfs_find_on(tentry, tpath);
      }
    }
  }

  sdsfree(slash);
  return NULL;
}

Vector *kfs_getCurrentList(KFS_Entry *this) {
  Vector *ret = new_vec();

  vec_push(ret, sdsnew("."));
  vec_push(ret, sdsnew(".."));
  vec_append(ret, avl_keys(GetAVLTree(this)));

  return ret;
}

static void trav_f(AVLNode *node, sds prefix, Vector *ret) {
  if (node == NULL) {
    return;
  }

  sds new_prefix = sdscatprintf(sdsempty(), "%s%s", prefix, (sds)node->key);
  vec_push(ret, new_prefix);

  sds slash = sdsnew("/");
  sds empty = sdsempty();

  KFS_Entry *entry = GetNodeValueAs(node, KFS_Entry *);
  if (entry->entry_type == tKFS_Dir) {
    trav_f(GetAVLTree(entry)->root, sdscat(new_prefix, slash), ret);
  }

  trav_f(node->left, sdscat(prefix, sdscmp(prefix, slash) == 0 ? empty : slash),
         ret);
  trav_f(node->right,
         sdscat(prefix, sdscmp(prefix, slash) == 0 ? empty : slash), ret);
}

Vector *kfs_getTree(KFS_Entry *this) {
  Vector *ret = new_vec();

  vec_push(ret, this->name);
  trav_f(GetAVLTree(this)->root, this->name, ret);

  // Todo sort: ret

  return ret;
}