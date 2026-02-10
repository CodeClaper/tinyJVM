#include <stdlib.h>

void *salloc(size_t size);
void *srealloc(void *p, size_t size);
char *sstrdup(char *input);
void sfree(void *p);
