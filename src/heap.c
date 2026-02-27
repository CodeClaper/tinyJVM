#include "heap.h"
#include "mmr.h"
#include "util.h"

static struct Heap *heapStatck = NULL;

/* Allocate heap entry and push into heap stack.
 * Return the heap entry. */
Heap *halloc(I4 nmemb, size_t size) {
    Heap *entry;
    void *obj;

    entry = salloc(sizeof(Heap));
    if (nmemb > 0) obj = salloc(nmemb * size);
    else obj = NULL;

    if (entry == NULL || (nmemb > 0 && obj == NULL)) {
        seterror("Out of memory");
        return NULL;
    }

    entry->nmemb = nmemb;
    entry->count = 0;
    entry->obj   = obj;
    entry->prev  = NULL;
    entry->next  = heapStatck;
    if (heapStatck) heapStatck->prev = entry;
    heapStatck = entry;

    return entry;
}
