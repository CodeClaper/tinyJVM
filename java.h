#include "class.h"

#ifndef __JAVA_H__
#define __JAVA_H__

#define DEFAULT_BUFF_SIZE 256

struct {
    char        class_path[DEFAULT_BUFF_SIZE];
    char        error[DEFAULT_BUFF_SIZE];
    ClassFile   *classes;
} javaServer;

#endif
