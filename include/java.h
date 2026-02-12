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
    int             verbose;                /* Javap in verbose mode. */
    int             private;                /* Javap shows all calsses and numbers. */
} JavaStates;

extern struct JavaStates javaStates; /* server global state */

#endif
