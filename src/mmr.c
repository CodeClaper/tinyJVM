#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c.h"

int salloc(void *p, size_t size) {
    p = malloc(size);
    return (p == NULL) ? ERR : OK;
}

int srealloc(void *p, size_t size) {
    p = realloc(p, size);
    return (p == NULL) ? ERR : OK;
}

int sstrdup(char *p, char *input) {
    return (p == NULL) ? ERR : OK;
}

int sfree(void *p) {
    free(p);
    return OK;
}
