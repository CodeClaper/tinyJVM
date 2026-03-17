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
        if (fi->access_flags & ACC_METHOD_STATIC) 
            size += clazz->fields[i]->offset;
    }

    return size;
}

/* Clac instance fields size. */
static U4 clazzClacInstanceFiledsSize(Clazz *clazz) {
    U2 i;
    U4 size; 

    size = 0; 
    for (i = 0; i < clazz->fileds_count; i++) {
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

