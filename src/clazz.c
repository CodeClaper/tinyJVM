#include <string.h>
#include <stdlib.h>
#include "clazz.h"
#include "class.h"
#include "data.h"
#include "mmr.h"
#include "util.h"

static Clazz *clazzStack = NULL;
static Clazz builtin[] = {
    {"[B", 1, NULL, NULL, NULL, 0, NULL, 0, NULL, 0, NULL, 1, 0, NULL, 0, NULL}, // byte
    {"[C", 1, NULL, NULL, NULL, 0, NULL, 0, NULL, 0, NULL, 2, 0, NULL, 0, NULL}, // char
    {"[D", 1, NULL, NULL, NULL, 0, NULL, 0, NULL, 0, NULL, 8, 0, NULL, 0, NULL}, // double
    {"[F", 1, NULL, NULL, NULL, 0, NULL, 0, NULL, 0, NULL, 4, 0, NULL, 0, NULL}, // float
    {"[I", 1, NULL, NULL, NULL, 0, NULL, 0, NULL, 0, NULL, 4, 0, NULL, 0, NULL}, // int
    {"[J", 1, NULL, NULL, NULL, 0, NULL, 0, NULL, 0, NULL, 8, 0, NULL, 0, NULL}, // long
    {"[S", 1, NULL, NULL, NULL, 0, NULL, 0, NULL, 0, NULL, 2, 0, NULL, 0, NULL}, // short
    {"[Z", 1, NULL, NULL, NULL, 0, NULL, 0, NULL, 0, NULL, 1, 0, NULL, 0, NULL}, // boolean
};


/* Get static field count. */
static U2 getStaticFieldCount(ClassFile *class) {
    FieldInfo *fi;
    U2 i, count = 0;

    for (i = 0; i < class->fields_count; i++) {
        fi = class->fields[i];
        if (fi->access_flags & ACC_FIELD_STATIC) 
            count++;
    }

    return count;
}

/* Get static fields. */
static void *getStaticFields(ClassFile *class, U2 count) {
    U2 i, j;
    U4 offset;
    Field **fields;
    Field *field;
    FieldInfo *fi;

    if (count == 0) return NULL;
    
    fields = salloc(sizeof(Field *) * count);
    if (fields == NULL) error("Out of memory");
    offset = 0;

    for (i = 0, j = 0; i < class->fields_count; i++) {
        fi = class->fields[i];
        if (!(fi->access_flags & ACC_FIELD_STATIC)) continue;
        field = salloc(sizeof(Field));
        field->access_flags = fi->access_flags;
        field->name = classGetUtf8(class, fi->name_index);
        field->descriptor = classGetUtf8(class, fi->descriptor_index);
        field->field_info = fi;
        field->offset = offset;
        (fields)[j++] = field;
        offset += getTypeSize(field->descriptor[0]);
    }

    return fields;
}

/* Get instance filed count. */
static U2 getInstanceFieldCount(ClassFile *class) {
    FieldInfo *fi;
    U2 i, count = 0;

    for (i = 0; i < class->fields_count; i++) {
        fi = class->fields[i];
        if (!(fi->access_flags & ACC_FIELD_STATIC)) count++;
    }

    return count;
}

static void *getInstanceFields(ClassFile *class, U2 count) {
    U2 i, j;
    U4 offset;
    Field **fields;
    Field *field;
    FieldInfo *fi;

    if (count == 0) return NULL;
    
    fields = salloc(sizeof(Field *) * count);
    if (fields == NULL) error("Out of memory");
    offset = 0;

    for (i = 0, j = 0; i < class->fields_count; i++) {
        fi = class->fields[i];
        if (fi->access_flags & ACC_FIELD_STATIC) continue;
        field = salloc(sizeof(Field));
        field->access_flags = fi->access_flags;
        field->name = classGetUtf8(class, fi->name_index);
        field->descriptor = classGetUtf8(class, fi->descriptor_index);
        field->field_info = fi;
        field->offset = offset;
        (fields)[j++] = field;
        offset += getTypeSize(field->descriptor[0]);
    }

    return fields;
}

/* Get static method count. */
static U2 getStaticMethodCount(ClassFile *class) {
    MethodInfo *mi;
    U2 i, count = 0;

    for (i = 0; i < class->method_count; i++) {
        mi = class->methods[i];
        if (mi->access_flags & ACC_METHOD_STATIC) 
            count++;
    }

    return count;
}

/* Get static methods. */
static void *getStaticMethods(ClassFile *class, U2 count) {
    U2 i, j;
    Method **methods, *method;
    MethodInfo *mi;

    if (count == 0) return NULL;
    
    methods = salloc(sizeof(Method *) * count);
    if (methods == NULL) error("Out of memory");

    for (i = 0, j = 0; i < class->method_count; i++) {
        mi = class->methods[i];
        if (!(mi->access_flags & ACC_METHOD_STATIC)) continue;
        method = salloc(sizeof(Method));
        method->access_flags = mi->access_flags;
        method->name = classGetUtf8(class, mi->name_index);
        method->descriptor = classGetUtf8(class, mi->descriptor_index);
        method->arg_count = getMethodArgCount(method->descriptor);
        method->slot_count = getMethodSlotCount(method->descriptor, method->access_flags);
        method->method_info = mi;
        (methods)[j++] = method;
    }

    return methods;
}


/* Get instance method count. */
static U2 getInstanceMethodCount(ClassFile *class) {
    MethodInfo *mi;
    U2 i, count = 0;

    for (i = 0; i < class->method_count; i++) {
        mi = class->methods[i];
        if (!(mi->access_flags & ACC_METHOD_STATIC)) 
            count++;
    }

    return count;
}


/* Get instance methods. */
static void *getInstanceMethods(ClassFile *class, U2 count) {
    U2 i, j;
    Method **methods, *method;
    MethodInfo *mi;

    if (count == 0) return NULL;
    
    methods = salloc(sizeof(Method *) * count);
    if (methods == NULL) error("Out of memory");

    for (i = 0, j = 0; i < class->method_count; i++) {
        mi = class->methods[i];
        if (mi->access_flags & ACC_METHOD_STATIC) continue;
        method = salloc(sizeof(Method));
        method->access_flags = mi->access_flags;
        method->name = classGetUtf8(class, mi->name_index);
        method->descriptor = classGetUtf8(class, mi->descriptor_index);
        method->arg_count = getMethodArgCount(method->descriptor);
        method->slot_count = getMethodSlotCount(method->descriptor, method->access_flags);
        method->method_info = mi;
        (methods)[j++] = method;
    }

    return methods;
}

/* Calc static var size. */
static U4 clazzCalcStaticVarSize(Clazz *clazz) {
    U2 i;
    U4 size;
    Field *fi;

    size = 0;
    for (i = 0; i < clazz->static_field_count; i++) {
        fi = clazz->static_fields[i];
        size += getTypeSize(fi->descriptor[0]);
    }

    return (size + 7) & ~7;
}

/* Clac instance fields size. */
static U4 clazzClacInstanceFiledsSize(Clazz *clazz) {
    U2 i;
    U4 size; 
    Field *field;

    size = 0; 
    for (i = 0; i < clazz->instance_field_count; i++) {
        field = clazz->instance_fields[i];
        size += getTypeSize(field->descriptor[0]);
    }

    return size;
}

/* Clac super class instance size. */
static U4 clazzCalcSuperInstanceSize(Clazz *clazz) {
    U4 size;

    size = 0;
    if (clazz->super)
        size += clazzCalcSuperInstanceSize(clazz->super);
    size += clazzClacInstanceFiledsSize(clazz);

    return size;
}

/* Calc instance size. */
static U4 clazzCalcInstanceVarSize(Clazz *clazz) {
    U4 size;

    size = 0;
    if (clazz->super) 
        size += clazzCalcSuperInstanceSize(clazz->super);
    size += clazzClacInstanceFiledsSize(clazz);

    return (size + 7) & ~7;
}


/* Push into stack. */
static void clazzPushStack(Clazz *clazz) {
    clazz->next = clazzStack;
    clazzStack = clazz;
}

static Clazz *clazzFindBuiltIn(char *classname) {
    U2 i;

    for (i = 0; i < LEN(builtin); i++) {
        if (strcmp(builtin[i].className, classname) == 0)
            return &builtin[i];
    }

    return NULL;
}

/* Find builtin clazz by array type code. */
Clazz *clazzFindBuildInByAtype(U1 atype) {
    switch (atype) {
        case T_BYTE: return &builtin[0];
        case T_CHAR: return &builtin[1];
        case T_LONG: return &builtin[2];
        case T_DOUBLE: return &builtin[3];
        case T_FLOAT: return &builtin[4];
        case T_INT: return &builtin[5];
        case T_SHORT: return &builtin[6];
        case T_BOOLEAN: return &builtin[7];
        default: return NULL;
    }
}

/* Find clazz in cache.
 * Return NULL if not foud. */
static Clazz *clazzFindInCache(char *classname) {
    for (Clazz *c = clazzStack; c != NULL; c = c->next)
        if (strcmp(c->className, classname) == 0) return c;
    return NULL;
}

/* Load Clazz from file via index. */
static Clazz *clazzLoadFileViaIndex(ClassFile *class, U2 index) {
    if (index) {
        char *classname = classGetClassName(class, index);
        return clazzLoad(classname);
    } else return NULL;
}

/* Load Clazz from file via name. */
static Clazz *clazzLoadFile(char *classname) {
    ClassFile *class = loadClass(classname);
    if (class == NULL) return NULL;

    Clazz *c = salloc(sizeof(Clazz));
    memset(c, 0, sizeof(Clazz));
    c->class = class;
    c->className = sstrdup(classname);
    c->super = clazzLoadFileViaIndex(class, class->super_class);
    c->initial = 0;
    c->static_field_count = getStaticFieldCount(class);
    c->static_fields = getStaticFields(class, c->static_field_count);
    c->static_var_size = clazzCalcStaticVarSize(c);
    c->static_vars = salloc(c->static_var_size);
    c->instance_field_count = getInstanceFieldCount(class);
    c->instance_fields = getInstanceFields(class, c->instance_field_count);
    c->instance_var_size = clazzCalcInstanceVarSize(c);
    c->static_method_count = getStaticMethodCount(class);
    c->static_methods = getStaticMethods(class, c->static_method_count);
    c->instance_method_count = getInstanceMethodCount(class);
    c->instance_methods = getInstanceMethods(class, c->instance_method_count);
    
    clazzPushStack(c);
    return c;
}

/* Load the Object. */
void clazzLoadObject() {
    Clazz *obj = salloc(sizeof(Clazz));
    memset(obj, 0, sizeof(Clazz));
    obj->class = classLoadObject();
    obj->className = sstrdup("java/lang/Object");
    obj->super = NULL;
    clazzPushStack(obj);
}

/* Load the clazz. */
Clazz *clazzLoad(char *classname) {
    Clazz *c = clazzFindBuiltIn(classname);
    if (c != NULL) return c;
    c = clazzFindInCache(classname);
    if (c != NULL) return c;
    else return clazzLoadFile(classname); 
}

/* Find field. */
Field *clazzFindField(Clazz *clazz, char *name, char *type) {
    U2 i;
    Field *field;
    
    /* Find in static fields. */
    for (i = 0; i < clazz->static_field_count; i++) {
        field = clazz->static_fields[i];
        if (strcmp(name, field->name) == 0 && strcmp(type, field->descriptor) == 0)
            return field;
    }

    /* Find in instance fields. */
    for (i = 0; i < clazz->instance_field_count; i++) {
        field = clazz->instance_fields[i];
        if (strcmp(name, field->name) == 0 && strcmp(type, field->descriptor) == 0)
            return field;
    }

    return NULL;
}

/* Find method. */
Method *clazzFindMethod(Clazz *clazz, char *name, char *type) {
    U2 i;
    Method *method;
    
    /* Find in static methods. */
    for (i = 0; i < clazz->static_method_count; i++) {
        method = clazz->static_methods[i];
        if (strcmp(name, method->name) == 0 && strcmp(type, method->descriptor) == 0)
            return method;
    }

    /* Find in instance methods. */
    for (i = 0; i < clazz->instance_method_count; i++) {
        method = clazz->instance_methods[i];
        if (strcmp(name, method->name) == 0 && strcmp(type, method->descriptor) == 0)
            return method;
    }

    return NULL;
}

/* Set static var. */
void clazzSetStaticVar(Clazz *clazz, Field *field, Value v) {
    char *dest;
    
    dest = clazz->static_vars + field->offset;
    switch (field->descriptor[0]) {
        case 'Z': case 'B':
            memcpy(dest, &v.i, 1);
            break;
        case 'C': case 'S':
            memcpy(dest, &v.i, 2);
            break;
        case 'I': 
            memcpy(dest, &v.i, 4);
            break;
        case 'F':
            memcpy(dest, &v.f, 4);
            break;
        case 'J': 
            memcpy(dest, &v.l, 8);
            break;
        case 'D':
            memcpy(dest, &v.d, 8);
            break;
        case 'L': case '[':
            memcpy(dest, &v.h->obj, 8);
            break;
    }
}

/* Get static var. */
Value clazzGetStaticVar(Clazz *clazz, Field *field) {
    Value v;
    char *dest;
 
    dest = clazz->static_vars + field->offset;
    switch (field->descriptor[0]) {
        case 'Z': case 'B':
            memcpy(&v.i, dest, 1);
            break;
        case 'C': case 'S':
            memcpy(&v.i, dest, 2);
            break;
        case 'I': 
            memcpy(&v.i, dest, 4);
            break;
        case 'F':
            memcpy(&v.f, dest, 4);
            break;
        case 'J': 
            memcpy(&v.l, dest, 8);
            break;
        case 'D':
            memcpy(&v.d, dest, 8);
            break;
        case 'L': case '[':
            memcpy(&v.h->obj, dest, 8);
            break;
        default:
            v.i = 0;
            break;
    }

    return v;
}

/* Set instance var. */ 
void clazzSetInstanceVar(JavaObject *obj, Field *field, Value v) {
    char *dest;

    dest = (char *)obj + sizeof(JavaObject) + field->offset;
    switch (field->descriptor[0]) {
        case 'Z': case 'B':
            memcpy(dest, &v.i, 1);
            break;
        case 'C': case 'S':
            memcpy(dest, &v.i, 2);
            break;
        case 'I': 
            memcpy(dest, &v.i, 4);
            break;
        case 'F':
            memcpy(dest, &v.f, 4);
            break;
        case 'J': 
            memcpy(dest, &v.l, 8);
            break;
        case 'D':
            memcpy(dest, &v.d, 8);
            break;
        case 'L': case '[':
            memcpy(dest, &v.h->obj, 8);
            break;
    }
}

/* Get instance var. */
Value clazzGetInstanceVar(JavaObject *obj, Field *field) {
    Value v;
    char *dest;
 
    dest = (char *)obj + sizeof(JavaObject) + field->offset;
    switch (field->descriptor[0]) {
        case 'Z': case 'B':
            memcpy(&v.i, dest, 1);
            break;
        case 'C': case 'S':
            memcpy(&v.i, dest, 2);
            break;
        case 'I': 
            memcpy(&v.i, dest, 4);
            break;
        case 'F':
            memcpy(&v.f, dest, 4);
            break;
        case 'J': 
            memcpy(&v.l, dest, 8);
            break;
        case 'D':
            memcpy(&v.d, dest, 8);
            break;
        case 'L': case '[':
            memcpy(&v.h->obj, dest, 8);
            break;
        default:
            v.i = 0;
            break;
    }

    return v;
}

