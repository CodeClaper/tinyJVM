#include <string.h>
#include <stdlib.h>
#include "clazz.h"
#include "data.h"
#include "mmr.h"
#include "util.h"

static Clazz *clazzStack = NULL;

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
    Clazz *c = clazzFindInCache(classname);
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

