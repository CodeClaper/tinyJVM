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
    struct {
        int         verbose;                /* Javap in verbose mode. */
        int         private;                /* Show all calsses and numbers. */
        int         sflag;                  /* Print internal type signatures. */
        int         lflag;                  /* Print line number and local variable tables. */
        int         cflag;                  /* Disassemble the code. */
    } javapOptions;
} JavaStates;

extern struct JavaStates javaStates; /* server global state */

#endif
