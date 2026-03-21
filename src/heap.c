#include "heap.h"
#include "clazz.h"
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

/* New JavaObject instance. */
JavaObject *newObj(Clazz *clazz) {
    JavaObject *obj = salloc(sizeof(JavaObject) + clazz->instance_var_size);
    if (obj == NULL) seterror("Out of memory");
    obj->clazz = clazz;
    return obj;
}

/* New JavaArrayObject instance. */
JavaArrayObject *newArray(Clazz *clazz, U2 length) {
    JavaArrayObject *array_obj;

    array_obj = salloc(sizeof(JavaArrayObject));
    if (array_obj == NULL) seterror("Out of memory");
    array_obj->header.clazz = clazz;
    array_obj->length = length;
    array_obj->data = salloc(sizeof(JavaObject *) * length);

    return array_obj;
}


/* Allocate heap entry and push into heap stack.
 * Return the heap entry. */
Heap *heapNew(Clazz *clazz) {
    Heap *entry;

    entry = salloc(sizeof(Heap));
    if (entry == NULL) seterror("Out of memory");
    entry->obj = newObj(clazz);
    heapPushStack(entry);

    return entry;
}

/* Allocate heap new array. */
Heap *heapNewArray(Clazz *clazz, U2 length) {
    Heap *entry;

    entry = salloc(sizeof(Heap));
    if (entry == NULL) seterror("Out of memory");
    entry->obj = newArray(clazz,  length);
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


