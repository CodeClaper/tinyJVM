#ifndef __JAVA_H__
#define __JAVA_H__
#include "data.h"

extern struct JavaStates javaStates; /* server global state */

int methodCall(ClassFile *class, Frame *frame, char *name, char *descriptor, U2 flags);

#endif
