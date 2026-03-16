#include "data.h"
typedef struct Clazz {
    char *              className;
    U4                  instanceSize;
    int                 initial;
    ClassFile           *class;
    struct Clazz        *super;
    struct Clazz        *next;
} Clazz;

typedef struct JavaObjectHeader {
    Clazz               *clazz;
} JavaObjectHeader;

typedef struct JavaObjectHeader JavaObject;

typedef struct JavaArrayObject {
    JavaObjectHeader    header;
    I4                  length;
    void                *data;
} JavaArrayObject;

void clazzLoadObject();
Clazz *clazzLoad(char *classname);
