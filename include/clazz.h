#include "data.h"
typedef struct Clazz {
    char *          className;
    U4              instanceSize;
    ClassFile       *class;
    struct Clazz    *super;
    struct Clazz    *next;
} Clazz;

typedef struct JavaObjectHeader {
    Clazz           *clazz;
} JavaObjectHeader;

Clazz *clazzLoad(char *classname);
