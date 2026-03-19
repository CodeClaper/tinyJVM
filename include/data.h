#ifndef __DATA_H__
#define __DATA_H__ 

#include "c.h"
#include "class.h"

#define MAX_ERR_SIZE 256

typedef enum RunMode {
    NONE,
    JAVA,
    JAVAP
} RunMode ;

typedef struct JavaStates {
    RunMode         mode;
    U2              num_class_path;
    char            **class_path;
    char            error[MAX_ERR_SIZE];
    char            *class_name;
    ClassFile       *classes;
} JavaStates;

typedef struct Frame {
    struct Frame                *next;
    struct ClassFile            *class;
    union Value                 *locals;
    union Value                 *stacks;
    U2                          max_local;
    U2                          max_stack;
    U2                          nstack;
    struct Code_attribute       *code;
    U2                          pc;
} Frame;

typedef struct Heap {
    struct Heap     *prev, *next;
    void            *obj;
} Heap;

typedef union Value {
    I4              i;
    I8              l;
    float           f;
    double          d;
    char            *s;
    Heap            *h;
} Value;

extern struct JavaStates javaStates; /* server global state */

#endif
