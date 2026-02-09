#include <stdio.h>
#include <stdlib.h>
#include "c.h"

int salloc(void *p, size_t size) {
    p = malloc(size);
    return (p == NULL) ? ERR : OK;
}

int sfree(void *p) {
    free(p);
    return OK;
}
