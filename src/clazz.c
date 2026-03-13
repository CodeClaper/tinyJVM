#include <string.h>
#include <stdlib.h>
#include "clazz.h"
#include "data.h"
#include "mmr.h"

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


/* Clac instance fields size. */
static U4 clazzClacInstanceFiledsSize(Clazz *clazz) {
    U2 i;
    U4 size; 
    ClassFile *class;
    FieldInfo *fi;

    size = 0; 
    class = clazz->class;
    for (i = 0; i < class->fields_count; i++) {
        fi = class->fields[i];
        size += getTypeSize(fi->descriptor_index);
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
    c->instanceSize = clazzCalcInstanceSize(c);
    c->initial = 0;
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

