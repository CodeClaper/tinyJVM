#include <stdlib.h>
#include "method.h"
#include "instruct.h"
#include "util.h"

int methodCall(ClassFile *class, Frame *frame, char *name, char *descriptor, U2 flags) {
    MethodInfo *method;
    AttributeInfo *attr;
    Code_attribute *code;
    Frame *newframe;
    OpReturn ret;
    Value v;
    
    method = classGetMethod(class, name, descriptor);
    if (method == NULL) return ERR;
    if (!(method->access_flags & flags)) return ERR;
    attr = classGetAttr(method->attributes, method->attribute_count, ATT_Code);
    if (attr == NULL) return ERR;
    code = &attr->info.code;
    newframe = framePush(class, code->max_locals, code->max_stack, code);
    if (newframe == NULL) error("Out of memory"); 

    while (newframe->pc < code->code_length) {
        INSTRUCT instruct = getInstruct(code->code[newframe->pc++]);
        if (instruct == NULL) error("Not found instruct.");
        if ((ret = instruct(newframe)) == RETURN_OPERAND) {
            v = frameStatckPop(newframe);
            frameStatckPush(frame, v);
        }
    }
    
    framePop();
    return 0;
}
