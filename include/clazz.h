#include "data.h"
typedef struct Clazz {
    char *          className;
    U4              instanceSize;
    int             initial;
    ClassFile       *class;
    struct Clazz    *super;
    struct Clazz    *next;
} Clazz;

typedef struct JavaObjectHeader {
    Clazz           *clazz;
} JavaObjectHeader;

void clazzLoadObject();
Clazz *clazzLoad(char *classname);
