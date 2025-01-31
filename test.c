#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


void foo() {
    printf("OK\n");
}

void bar(int a, int b, int c) {
    printf("a: %d, b: %d, c: %d\n", a, b, c);
}

void huga(int a, int b, int c, int d, int e, int f, int g, int h) {
    printf("%d %d %d %d %d %d %d %d\n", a, b, c, d, e, f, g, h);
}