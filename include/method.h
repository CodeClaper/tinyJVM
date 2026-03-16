#include "data.h"

int clinitMethodCall(ClassFile *class);
int initMethodCall(ClassFile *class, Frame *frame, char *name, char *descriptor, U2 flags);
int methodCall(ClassFile *class, Frame *frame, char *name, char *descriptor, U2 flags);
