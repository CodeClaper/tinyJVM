#include "data.h"
#include "clazz.h"

JavaObject *newObj(Clazz *clazz);
JavaArrayObject *newArray(Clazz *clazz, U2 length);
Heap *heapNew(Clazz *clazz);
Heap *heapNewArray(Clazz *clazz, U2 length);
Heap *heapNull();
Heap *heapObj(JavaObject *obj);
