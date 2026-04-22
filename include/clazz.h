#ifndef __CLAZZ_H__
#define __CLAZZ_H__

#include "class.h"
#include "data.h"

typedef struct Field {
    U2                  access_flags;
    char                *name;
    char                *descriptor;
    U4                  offset;
    FieldInfo           *field_info;
} Field;

typedef struct Method {
    U2                  access_flags;
    char                *name;
    char                *descriptor;
    U2                  arg_count;
    U2                  slot_count;
    MethodInfo          *method_info;
} Method;

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
    U2                  static_method_count;
    struct Method       **static_methods;
    U2                  instance_method_count;
    struct Method       **instance_methods;
    struct Heap         *java_mirror;
} Clazz;

typedef struct JavaObjectHeader {
    Clazz               *clazz;
} JavaObjectHeader;

typedef struct JavaObjectHeader JavaObject;

typedef struct JavaArrayObject {
    JavaObjectHeader    header;
    I4                  length;
    JavaObject          **data;
} JavaArrayObject;

void clazzLoadObject();
Clazz *clazzLoad(char *classname);
Clazz *clazzFindBuildInByAtype(U1 atype);
Field *clazzFindField(Clazz *clazz, char *name, char *type);
Method *clazzFindMethod(Clazz *clazz, char *name, char *type);
void clazzSetStaticVar(Clazz *clazz, Field *field, Value v);
Value clazzGetStaticVar(Clazz *clazz, Field *field);
void clazzSetInstanceVar(JavaObject *obj, Field *field, Value v);
Value clazzGetInstanceVar(JavaObject *obj, Field *field);
void clazzSetArrayInstanceVar(JavaArrayObject *arr, U2 index, Value v);
Value clazzGetArrayInstanceVar(JavaArrayObject *arr, U2 index);

#endif // __CLAZZ_H__
