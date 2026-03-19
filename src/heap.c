#include "heap.h"
#include "data.h"
#include "mmr.h"
#include "util.h"

static struct Heap *heapStack = NULL;

/* Push heap into stack. */
static void heapPushStack(Heap *h) {
    h->next = heapStack;
    if (heapStack) heapStack->next = h;
    heapStack = h;
}

/* Allocate heap entry and push into heap stack.
 * Return the heap entry. */
Heap *heapNew(Clazz *clazz) {
    Heap *entry;
    JavaObject *obj;

    entry = salloc(sizeof(Heap));
    if (entry == NULL) seterror("Out of memory");
    obj = salloc(sizeof(JavaObject) + clazz->instance_var_size);
    if (obj == NULL) seterror("Out of memory");
    entry->obj = obj;
    heapPushStack(entry);

    return entry;
}

/* Allocate heap new array. */
Heap *heapNewArray(Clazz *clazz, U2 length) {
    Heap *entry;
    JavaArrayObject *array_obj;

    entry = salloc(sizeof(Heap));
    if (entry == NULL) seterror("Out of memory");
    array_obj = salloc(sizeof(JavaArrayObject) + clazz->instance_var_size * length);
    if (array_obj == NULL) seterror("Out of memory");
    array_obj->header.clazz = clazz;
    array_obj->length = length;
    entry->obj = array_obj;
    heapPushStack(entry);

    return entry;
}

/* Allocate heap entry which refer to NULL object. */
Heap *heapNull() {
    Heap *entry;

    entry = salloc(sizeof(Heap));
    if (entry == NULL) seterror("Out of memory");
    entry->obj = NULL;
    heapPushStack(entry);

    return entry;
}
