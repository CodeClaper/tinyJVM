#include "data.h"

typedef enum NativeClassType {
    NONE_CLASS,
    LANG_SYSTEM,
    LANG_CLASS,
    IO_PRINTSTREAM
} NativeClassType;

void *nativeJavaObj(NativeClassType ntype, char *objname, char *objtype);
int nativeMethodCall(Frame *frame, char *classname, char *name, char *type);
