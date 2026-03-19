#ifndef __CLAZZ_H__
#define __CLAZZ_H__

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
    int                 initial;
    struct ClassFile    *class;
    struct Clazz        *super;
    struct Clazz        *next;
    U2                  static_field_count;
    struct Field        **static_fields;
    U4                  static_var_size;
    char                *static_vars;
    U2                  instance_field_count;
    struct Field        **instance_fields;
    U4                  instance_var_size;
} Clazz;

typedef struct JavaObjectHeader {
    Clazz               *clazz;
} JavaObjectHeader;

typedef struct JavaObjectHeader JavaObject;

typedef struct JavaArrayObject {
    JavaObjectHeader    header;
    I4                  length;
} JavaArrayObject;

void clazzLoadObject();
Clazz *clazzLoad(char *classname);
Field *clazzFindField(Clazz *clazz, char *name, char *type);
void clazzSetStaticVar(Clazz *clazz, Field *field, Value v);
Value clazzGetStaticVar(Clazz *clazz, Field *field);
void clazzSetInstanceVar(JavaObject *obj, Field *field, Value v);
Value clazzGetInstanceVar(JavaObject *obj, Field *field);

#endif // __CLAZZ_H__
