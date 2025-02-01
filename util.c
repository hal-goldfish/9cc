#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int roundup(int x, int align) {
    return (x + align - 1) & ~(align - 1);
}


Vector *new_vec() {
    Vector *v = malloc(sizeof(Vector));
    v->data = malloc(sizeof(void*) * 16);
    v->cap = 16;
    v->len = 0;
    return v;
}

void vec_push(Vector *vec, void *p) {
    if (vec->cap == vec->len) {
        vec->data = realloc(vec->data, sizeof(void*) * vec->cap*2);
        vec->cap *= 2;
    }
    vec->data[vec->len++] = p;
}

void *vec_pop(Vector *vec) {
    if (vec->len == 0) {
        error("ベクタが空です");
    }
    return vec->data[--(vec->len)];
}

int vec_size(Vector *vec) {
    return vec->len;
}

void *vec_last(Vector *vec) {
    return vec->data[vec->len - 1];
}

void *vec_at(Vector *vec, int i) {
    if (i >= vec_size(vec)) error("範囲外アクセスです");
    return vec->data[i];
}