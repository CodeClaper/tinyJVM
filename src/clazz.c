#include <string.h>
#include <stdlib.h>
#include "clazz.h"
#include "data.h"
#include "mmr.h"
#include "util.h"

static Clazz *clazzStack = NULL;

static U4 getTypeSize(char type) {
    switch (type) {
        case 'Z': case 'B': return 1; // boolean, byte
        case 'C': case 'S': return 2; // char, short
        case 'I': case 'F': return 4; // int, float
        case 'J': case 'D': return 8; // long, double
        case 'L': case '[': return 4; // reference（32 bit match's JVM is4，64 bit machine's jvm is 4 by pression.）
        default: return 0;
    }
}

/* Convert FieldInfo to Field. */
static void convertFields(ClassFile *class, Field ***fields, FieldInfo **field_infos, U2 count) {
    U2 i;
    U4 offset;
    Field *field;

    if (count == 0) return;
    
    *fields = salloc(sizeof(Field) * count);
    if (*fields == NULL) error("Out of memory");
    offset = 0;

    for (i = 0; i < count; i++) {
        field = salloc(sizeof(Field));
        field->access_flags = field_infos[i]->access_flags;
        field->name = classGetUtf8(class, field_infos[i]->name_index);
        field->descriptor = classGetUtf8(class, field_infos[i]->descriptor_index);
        field->field_info = field_infos[i];
        field->offset = offset;
        (*fields)[i] = field;
        offset += getTypeSize(field->descriptor[0]);
    }
}

static U4 clazzCalcStaticVarSize(Clazz *clazz) {
    U2 i;
    U4 size;
    Field *fi;

    size = 0;
    for (i = 0; i < clazz->fileds_count; i++) {
        fi = clazz->fields[i];
        if (fi->access_flags & ACC_FIELD_STATIC) 
            size += clazz->fields[i]->offset;
    }

    return size;
}

/* Clac instance fields size. */
static U4 clazzClacInstanceFiledsSize(Clazz *clazz) {
    U2 i;
    U4 size; 
    Field *field;

    size = 0; 
    for (i = 0; i < clazz->fileds_count; i++) {
        field = clazz->fields[i];
        if (!(field->access_flags & ACC_FIELD_STATIC)) 
            size += clazz->fields[i]->offset;
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
static U4 clazzCalcInstanceSize(Clazz *clazz) {
    U4 size;

    size = sizeof(JavaObjectHeader);
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
    c->fileds_count = class->fields_count;
    c->initial = 0;
    convertFields(class, &c->fields, class->fields, c->fileds_count);
    c->instanceSize = clazzCalcInstanceSize(c);
    c->static_var_size = clazzCalcStaticVarSize(c);
    c->static_vars = salloc(c->static_var_size);
    
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
    obj->instanceSize = 0;
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

    for ( i = 0; i < clazz->fileds_count; i++) {
        field = clazz->fields[i];
        if (strcmp(name, field->name) == 0 && strcmp(type, field->descriptor) == 0)
            return field;
    }
    return NULL;
}

/* Set static var. */
void clazzSetStaticVar(Clazz *clazz, Field *field, Value v) {
    U2 i;
    U4 offset;
    Field *current;
    char *dest;

    offset = 0;
    for (i = 0; i < clazz->fileds_count; i++) {
        current = clazz->fields[i];
        if (current == field) break;
        else if (current->access_flags & ACC_FIELD_STATIC) offset += getTypeSize(current->descriptor[0]);
        else continue;
    }
    
    dest = clazz->static_vars + offset;
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
            memcpy(dest, &v.h->obj, 4);
            break;
    }
}

/* Get static var. */
Value clazzGetStaticVar(Clazz *clazz, Field *field) {
    U2 i;
    U4 offset;
    Field *current;
    char *dest;
    Value v;

    offset = 0;
    for (i = 0; i < clazz->fileds_count; i++) {
        current = clazz->fields[i];
        if (current == field) break;
        else if (current->access_flags & ACC_FIELD_STATIC) offset += getTypeSize(current->descriptor[0]);
        else continue;
    }
    
    dest = clazz->static_vars + offset;
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
            memcpy(&v.h->obj, dest, 4);
            break;
    }

    return v;
}

