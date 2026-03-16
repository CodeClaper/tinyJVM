#include "data.h"

typedef enum NativeClassType {
    NONE_CLASS,
    LANG_SYSTEM,
    IO_PRINTSTREAM
} NativeClassType;

NativeClassType nativeClassFind(char *classname);
void *nativeJavaObj(NativeClassType ntype, char *objname, char *objtype);
int nativeMethodCall(Frame *frame, NativeClassType ntype, char *name, char *type);
