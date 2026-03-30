#include <stdlib.h>
#include <stdio.h>
#include "method.h"
#include "class.h"
#include "clazz.h"
#include "data.h"
#include "frame.h"
#include "instruct.h"
#include "util.h"

/* Invoke <clinit> method. */
int clinitMethodCall(ClassFile *class) {
    Frame *frame;
    MethodInfo *method;

    if (class->init) return OK;
    if (class->super && clinitMethodCall(class->super) == ERR) return ERR;
    method = classGetMethod(class, "<clinit>", "()V");
    if (method == NULL) return OK;
    frame = framePush(class, 0, 1, NULL);
    return methodCall(class, frame, "<clinit>", "()V", ACC_METHOD_STATIC); 
}

/* Invoke <init> method. */
int initMethodCall(ClassFile *class, Frame *frame, char *name, char *descriptor, U2 flags) {
    MethodInfo *method;

    method = classGetMethod(class, name, descriptor);
    if (method == NULL) return OK;
    if (!(method->access_flags & flags)) return OK; // Maybe private consturct method.
    return methodCall(class, frame, name, descriptor, flags); 
}

/* Invoke method. */
int methodCall(ClassFile *class, Frame *frame, char *name, char *descriptor, U2 flags) {
    I2 i;
    char *classname;
    Clazz *clazz;
    MethodInfo *mi;
    Method *method;
    AttributeInfo *attr;
    Code_attribute *code;
    Frame *newframe;
    OpReturn ret;
    Value v;
    
    classname = classGetClassName(class, class->this_class);
    clazz = clazzLoad(classname);
    if (clazz == NULL) error("Load clazz: %s fail.", classname);
    mi = classGetMethod(class, name, descriptor);
    if (mi == NULL) return ERR;
    if (!(mi->access_flags == ACC_METHOD_DEFAULT || mi->access_flags & flags)) return ERR;
    attr = classGetAttr(mi->attributes, mi->attribute_count, ATT_Code);
    if (attr == NULL) return ERR;
    code = &attr->info.code;
    newframe = framePush(class, code->max_locals, code->max_stack, code);
    if (newframe == NULL) error("Out of memory"); 
    method = clazzFindMethod(clazz, name, descriptor);
    if (method == NULL) error("Not found method: %s in class: %s", name, classname);
    for (i = method->slot_count - 1; i >= 0; i--) {
        newframe->locals[i] = frameStatckPop(frame);
    }
    
    while (newframe->pc < code->code_length) {
        U1 instruction = code->code[newframe->pc++];
        INSTRUCT instruct = getInstruct(instruction);
        if (instruct == NULL) error("Not found instruct.");
        if ((ret = instruct(newframe)) == RETURN_OPERAND) {
            v = frameStatckPop(newframe);
            frameStatckPush(frame, v);
        }
        fprintf(stdout, "Instruct %s executed.\n", getOpName(instruction));
    }
    
    framePop();
    return 0;
}
