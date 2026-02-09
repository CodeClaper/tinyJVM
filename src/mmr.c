#include <string.h>
#include <stdlib.h>
#include "c.h"

int salloc(void **p, size_t size) {
    *p = malloc(size);
    if (*p == NULL) return ERR;
    memset(*p, 0, size);
    return OK;
}

int srealloc(void *p, size_t size) {
    p = realloc(p, size);
    return (p == NULL) ? ERR : OK;
}

int sstrdup(char **p, char *input) {
    size_t size = strlen(input);
    int r = salloc((void **)p, size + 1);
    if (r == ERR) return ERR;
    memcpy(*p, input, size);
    return OK;
}

int sfree(void *p) {
    free(p);
    return OK;
}
