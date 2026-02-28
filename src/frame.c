#include "frame.h"
#include "java.h"
#include "mmr.h"
#include "util.h"

static Frame *frameStatck = NULL;

/* Generate new frame and push it into frameStatck.
 * Return the frame. */
Frame *framePush(ClassFile *class, U2 max_local, U2 max_stack, struct Code_attribute *code) {
    Frame *frame;
    Value *locals, *stacks;

    frame = salloc(sizeof(Frame));
    locals = salloc(max_local * sizeof(Value));
    stacks = salloc(max_stack * sizeof(Value));

    if (frame == NULL || locals == NULL || stacks == NULL) return NULL;
    
    frame->pc = 0;
    frame->code = code;
    frame->class = class;
    frame->locals = locals;
    frame->stacks = stacks;
    frame->max_local = max_local;
    frame->max_stack = max_stack;
    frame->nstack = 0;
    frame->next = frameStatck;
    frameStatck = frame;

    return frame;
}

/* Pop and free frame from frameStatck.
 * Return -1 on error. */
int framePop(void) {
    Frame *frame;

    if (frameStatck == NULL) return -1;
    frame = frameStatck;
    frameStatck = frame->next;
    sfree(frame->locals);
    sfree(frame->stacks);
    sfree(frame);

    return 0;
}

/* Push value into frame stack. */
void frameStatckPush(Frame *frame, Value value) {
    frame->stacks[frame->nstack++] = value;
}

/* Pop value from frame stack. */
Value frameStatckPop(Frame *frame) {
    if (frame->nstack == 0) seterror("ASDA");
    return frame->stacks[--frame->nstack];
}

/* Store local at i. */
void frameLocalStore(Frame *frame, U2 i, Value v) {
    frame->locals[i] = v;
}

/* Load local from i. */
Value frameLocalLoad(Frame *frame, U2 i) {
    return frame->locals[i];
}
