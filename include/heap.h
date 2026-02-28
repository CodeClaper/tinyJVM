#include <stdlib.h>
#include "c.h"

typedef struct Heap {
    struct Heap     *prev, *next;
    void            *obj;
    I4              nmemb;
    U2              count;
} Heap;

Heap *heapNew(I4 nmemb, size_t size);
