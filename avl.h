#ifndef __AVL_HEADER_INCLUDED__
#define __AVL_HEADER_INCLUDED__

#include <stdbool.h>
#include <stddef.h>

///////////////////// AVL /////////////////////
typedef void (*ELEM_DESTRUCTOR)(void *);
typedef int (*ELEM_COMPARE)(void *, void *);
typedef char *(*ELEM_PRINTER)(void *);

typedef struct AVLNode_t {
  void *key;
  void *value;
  int height;
  int size;
  struct AVLNode_t *left;
  struct AVLNode_t *right;
} AVLNode;

AVLNode *new_AVLNode(void *key, void *value);

typedef struct {
  AVLNode *root;
} AVLTree;

AVLTree *new_AVLTree();

typedef int (*ELEM_COMPARE)(void *, void *);

bool avl_exists(AVLTree *tree, void *key, ELEM_COMPARE compare);

void *avl_find(AVLTree *tree, void *key, ELEM_COMPARE compare);

void avl_insert(AVLTree *tree, void *key, void *value, ELEM_COMPARE compare);
void avl_delete(AVLTree *tree, void *key, ELEM_COMPARE compare);

#define sz(t) (t ? t->size : 0)
#define ht(t) (t ? t->height : 0)

enum { L, R };

void print_node(AVLNode *node, size_t depth, ELEM_PRINTER key_printer,
                ELEM_PRINTER value_printer);

void print_tree(AVLTree *tree, ELEM_PRINTER key_printer,
                ELEM_PRINTER value_printer);

sds show_node(AVLNode *node, size_t depth, ELEM_PRINTER key_printer,
              ELEM_PRINTER value_printer);

sds show_tree(AVLTree *tree, ELEM_PRINTER key_printer,
              ELEM_PRINTER value_printer);

Vector *avl_values(AVLTree *tree);
Vector *avl_keys(AVLTree *tree);

#endif
