#include "kfs.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Vector *new_vec_with(size_t capacity) {
  Vector *v = xmalloc(sizeof(Vector));
  v->data = xmalloc(sizeof(void *) * capacity);
  v->capacity = capacity;
  v->len = 0;
  return v;
}

Vector *new_vec() { return new_vec_with(VECTOR_DEFAULT_CAPACITY); }

void vec_expand(Vector *v, size_t size) {
  if(v->len < size) {
    v->capacity = size;
    v->len = size;
    v->data = realloc(v->data, sizeof(void *) * v->capacity);
  }
}

void vec_push(Vector *v, void *elem) {
  if(v->len == v->capacity) {
    v->capacity *= 2;
    v->data = realloc(v->data, sizeof(void *) * v->capacity);
  }
  v->data[v->len++] = elem;
}

void vec_pushi(Vector *v, int val) { vec_push(v, (void *)(intptr_t)val); }

void *vec_pop(Vector *v) {
  assert(v->len);
  return v->data[--v->len];
}

void *vec_last(Vector *v) {
  assert(v->len);
  return v->data[v->len - 1];
}

bool vec_contains(Vector *v, void *elem) {
  for(size_t i = 0; i < v->len; i++) {
    if(v->data[i] == elem) {
      return true;
    }
  }
  return false;
}

bool vec_containss(Vector *v, sds key) {
  for(size_t i = 0; i < v->len; i++) {
    if(!strcmp(v->data[i], key)) {
      return true;
    }
  }
  return false;
}

bool vec_union1(Vector *v, void *elem) {
  if(vec_contains(v, elem)) {
    return false;
  }
  vec_push(v, elem);
  return true;
}

void *vec_get(Vector *v, size_t idx) {
  assert(idx < v->len);
  return v->data[idx];
}

Vector *vec_dup(Vector *v) {
  Vector *vec = new_vec();
  for(size_t i = 0; i < v->len; i++) {
    vec_push(vec, v->data[i]);
  }
  return vec;
}

void vec_append(Vector *v1, Vector *v2) {
  VecForeach(v2, e, { vec_push(v1, e); })
}
