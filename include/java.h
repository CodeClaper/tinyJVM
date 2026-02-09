#ifndef __JAVA_H__
#define __JAVA_H__
#include "class.h"

#define MAX_ERR_SIZE 256

typedef struct JavaStates {
    U2          num_class_path;
    char        **class_path;
    char        error[MAX_ERR_SIZE];
    ClassFile   *classes;
} JavaStates;

extern struct JavaStates javaStates; /* server global state */
#endif
