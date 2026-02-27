#include <string.h>
#include <stdlib.h>
#include "mmr.h"
#include "c.h"
#include "java.h"
#include "util.h"

void *salloc(size_t size) {
    void *p = malloc(size);
    if (p == NULL) return NULL;
    memset(p, 0, size);
    return p;
}

void *srealloc(void *p, size_t size) {
    p = realloc(p, size);
    return p;
}

char *sstrdup(char *input) {
    size_t size = strlen(input);
    void *p = salloc(size + 1);
    if (p == NULL) return NULL;
    memcpy(p, input, size);
    return p;
}

void sfree(void *p) {
    free(p);
}

