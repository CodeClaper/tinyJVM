#ifndef __JAVA_H__
#define __JAVA_H__
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

extern struct JavaStates javaStates; /* server global state */

#endif
