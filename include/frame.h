#ifndef __FRAME__
#define __FRAME__
#include "data.h"

Frame *framePush(ClassFile *class, U2 max_local, U2 max_stack, struct Code_attribute *code);
int framePop(void);
void frameStatckPush(Frame *frame, Value value);
Value frameStatckPop(Frame *frame);
void frameLocalStore(Frame *frame, U2 i, Value v);
Value frameLocalLoad(Frame *frame, U2 i);

#endif

