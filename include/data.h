#ifndef __DATA_H__
#define __DATA_H__ 

#include "c.h"
#include "class.h"

#define MAX_ERR_SIZE 256

typedef struct JavaStates {
    U2              num_class_path;
    char            **class_path;
    char            error[MAX_ERR_SIZE];
    char            *class_name;
    ClassFile       *classes;
} JavaStates;

typedef union Value {
    I4              i;
    I8              l;
    float           f;
    double          d;
} Value;


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

#endif
