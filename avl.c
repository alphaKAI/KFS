#include "kfs.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

AVLNode *new_AVLNode(void *key, void *value) {
  AVLNode *this = xmalloc(sizeof(AVLNode));
  this->key = key;
  this->value = value;
  this->height = 1;
  this->size = 1;
  this->left = NULL;
  this->right = NULL;
  return this;
}

typedef void (*ELEM_FREE)(void *);

void free_AVLNode(AVLNode **n_ptr, ELEM_FREE free_key, ELEM_FREE free_val) {
  if (n_ptr == NULL || *n_ptr == NULL) {
    fprintf(stderr, "[free_AVLNode] given pointer is already freed\n");
    exit(EXIT_FAILURE);
  }

  free_key((*n_ptr)->key);
  free_val((*n_ptr)->value);

  if ((*n_ptr)->left != NULL) {
    free_AVLNode(&(*n_ptr)->left, free_key, free_val);
  }

  if ((*n_ptr)->right != NULL) {
    free_AVLNode(&(*n_ptr)->right, free_key, free_val);
  }

  free(*n_ptr);
  *n_ptr = NULL;
}

AVLTree *new_AVLTree() {
  AVLTree *this = xmalloc(sizeof(AVLTree));
  this->root = NULL;
  return this;
}

void free_AVLTree(AVLTree **t_ptr, ELEM_FREE free_key, ELEM_FREE free_val) {
  if (t_ptr == NULL || *t_ptr == NULL) {
    fprintf(stderr, "[free_AVLTree] given pointer is already freed\n");
    exit(EXIT_FAILURE);
  }
  free_AVLNode(&(*t_ptr)->root, free_key, free_val);
}

static void *find_impl(AVLNode *t, void *key, ELEM_COMPARE compare) {
  if (t == NULL) {
    return NULL;
  }

  int comp_result = compare(key, t->key);

  if (comp_result == 0) {
    return t->value;
  } else if (comp_result == -1) {
    return find_impl(t->left, key, compare);
  } else {
    return find_impl(t->right, key, compare);
  }
}

bool avl_exists(AVLTree *tree, void *key, ELEM_COMPARE compare) {
  return find_impl(tree->root, key, compare) != NULL;
}

void *avl_find(AVLTree *tree, void *key, ELEM_COMPARE compare) {
  return find_impl(tree->root, key, compare);
}

static int int_max(int a, int b) { return a > b ? a : b; }

static AVLNode *rotate(AVLNode *t, int l, int r, ELEM_COMPARE compare);

static AVLNode *balance(AVLNode *t, ELEM_COMPARE compare) {
  if (ht(t->right) - ht(t->left) < -1) {
    if (ht(t->left->right) - ht(t->left->left) > 0) {
      t->left = rotate(t->left, L, R, compare);
    }
    return rotate(t, R, L, compare);
  }

  if (ht(t->left) - ht(t->right) < -1) {
    if (ht(t->right->left) - ht(t->right->right) > 0) {
      t->right = rotate(t->right, R, L, compare);
    }
    return rotate(t, L, R, compare);
  }

  if (t != NULL) {
    t->height = int_max(ht(t->left), ht(t->right)) + 1;
    t->size = sz(t->left) + sz(t->right) + 1;
  }
  return t;
}

static AVLNode *get_child_by_LR(AVLNode *t, int lr) {
  switch (lr) {
  case L:
    return t->left;
  case R:
    return t->right;
  default:
    fprintf(stderr, "ERROR\n");
    exit(EXIT_FAILURE);
  }
}

static void set_child_by_LR(AVLNode *dst, int lr, AVLNode *src) {
  switch (lr) {
  case L:
    dst->left = src;
    break;
  case R:
    dst->right = src;
    break;
  }
}

static AVLNode *rotate(AVLNode *t, int l, int r, ELEM_COMPARE compare) {
  AVLNode *s = get_child_by_LR(t, r);
  set_child_by_LR(t, r, get_child_by_LR(s, l));
  set_child_by_LR(s, l, balance(t, compare));

  if (t != NULL) {
    t->size = sz(t->left) + sz(t->right) + 1;
  }
  if (s != NULL) {
    s->size = sz(s->left) + sz(s->right) + 1;
  }

  return balance(s, compare);
}

static AVLNode *insert_impl(AVLNode *t, AVLNode *x, ELEM_COMPARE compare) {
  if (t == NULL) {
    return x;
  }

  int comp_result = compare(x->key, t->key);

  if (comp_result == 0) {
    t->value = x->value;
  } else if (comp_result == -1) {
    t->left = insert_impl(t->left, x, compare);
  } else {
    t->right = insert_impl(t->right, x, compare);
  }
  t->size += 1;

  return balance(t, compare);
}

void avl_insert(AVLTree *tree, void *key, void *value, ELEM_COMPARE compare) {
  tree->root = insert_impl(tree->root, new_AVLNode(key, value), compare);
}

static sds string_rep(sds s, size_t n) {
  sds ret = sdsempty();

  for (size_t i = 0; i < n; i++) {
    sdscat(ret, s);
  }

  return ret;
}

static AVLNode *move_down(AVLNode *t, AVLNode *rhs, ELEM_COMPARE compare) {
  if (t == NULL) {
    return rhs;
  }
  t->right = move_down(t->right, rhs, compare);
  return balance(t, compare);
}

static AVLNode *delete_impl(AVLNode *t, void *key, ELEM_COMPARE compare) {
  if (t == NULL) {
    return NULL;
  }

  int comp_result = compare(key, t->key);

  if (comp_result == 0) {
    return move_down(t->left, t->right, compare);
  } else {
    if (comp_result == -1) {
      t->left = delete_impl(t->left, key, compare);
    } else {
      t->right = delete_impl(t->right, key, compare);
    }

    t->size -= 1;
    return balance(t, compare);
  }
}

void avl_delete(AVLTree *tree, void *key, ELEM_COMPARE compare) {
  tree->root = delete_impl(tree->root, key, compare);
}

void print_node(AVLNode *node, size_t depth, ELEM_PRINTER key_printer,
                ELEM_PRINTER value_printer) {
  if (node != NULL) {
    print_node(node->left, depth + 1, key_printer, value_printer);
    printf("%s <%s:%s>\n", string_rep("    ", depth), key_printer(node->key),
           value_printer(node->value));
    print_node(node->right, depth + 1, key_printer, value_printer);
  }
}

void print_tree(AVLTree *tree, ELEM_PRINTER key_printer,
                ELEM_PRINTER value_printer) {
  print_node(tree->root, 0, key_printer, value_printer);
}

sds show_node(AVLNode *node, size_t depth, ELEM_PRINTER key_printer,
              ELEM_PRINTER value_printer) {
  if (node != NULL) {
    sds l = show_node(node->left, depth + 1, key_printer, value_printer);
    sds body =
        sdscatprintf(sdsempty(), "%s <%s:%s>\n", string_rep("    ", depth),
                     key_printer(node->key), value_printer(node->value));
    sds r = show_node(node->right, depth + 1, key_printer, value_printer);
    return sdscatprintf(sdsempty(), "%s%s%s", l, body, r);
  } else {
    return sdsnew("");
  }
}

sds show_tree(AVLTree *tree, ELEM_PRINTER key_printer,
              ELEM_PRINTER value_printer) {
  return show_node(tree->root, 0, key_printer, value_printer);
}

static void collect_values(AVLNode *node, Vector *values) {
  if (node != NULL) {
    vec_push(values, node->value);
    collect_values(node->left, values);
    collect_values(node->right, values);
  }
}

Vector *avl_values(AVLTree *tree) {
  Vector *values = new_vec();

  collect_values(tree->root, values);

  return values;
}

static void collect_keys(AVLNode *node, Vector *keys) {
  if (node != NULL) {
    vec_push(keys, node->key);
    collect_keys(node->left, keys);
    collect_keys(node->right, keys);
  }
}

Vector *avl_keys(AVLTree *tree) {
  Vector *keys = new_vec();

  collect_keys(tree->root, keys);

  return keys;
}