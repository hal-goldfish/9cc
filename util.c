#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


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