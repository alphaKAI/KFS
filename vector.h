#ifndef __VECTOR_HEADER_INCLUDED__
#define __VECTOR_HEADER_INCLUDED__
#include "sds/sds.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  void **data;
  size_t capacity;
  size_t len;
} Vector;

#define VECTOR_DEFAULT_CAPACITY 16

#define VecForeachWithType(vec, T, loop_val, loop_body) \
  for (size_t fe_loop_counter_ ## vec = 0; fe_loop_counter_ ## vec < vec->len; fe_loop_counter_ ## vec ++) { \
    T loop_val = vec->data[fe_loop_counter_ ## vec]; \
    loop_body; \
  }

#define VecForeach(vec, loop_val, loop_body) VecForeachWithType(vec, void*, loop_val, loop_body)



Vector *new_vec_with(size_t capacity);
Vector *new_vec(void);
void vec_push(Vector *v, void *elem);
void vec_pushi(Vector *v, int val);
void *vec_get(Vector *v, size_t idx);
void *vec_pop(Vector *v);
void *vec_last(Vector *v);
bool vec_contains(Vector *v, void *elem);
bool vec_containss(Vector *v, sds elem);
bool vec_union1(Vector *v, void *elem);
Vector *vec_dup(Vector *v);
void vec_append(Vector *v1, Vector *v2);
#endif
