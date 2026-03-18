#include "data.h"

typedef struct Field {
    U2                  access_flags;
    char                *name;
    char                *descriptor;
    U4                  offset;
    FieldInfo           *field_info;
} Field;

typedef struct Clazz {
    char                *className;
    U4                  instanceSize;
    int                 initial;
    struct ClassFile    *class;
    struct Clazz        *super;
    struct Clazz        *next;
    U2                  fileds_count;
    struct Field        **fields;
    U4                  static_var_size;
    char                *static_vars;
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
Field *clazzFindField(Clazz *clazz, char *name, char *type);
void clazzSetStaticVar(Clazz *clazz, Field *field, Value v);
Value clazzGetStaticVar(Clazz *clazz, Field *field);
