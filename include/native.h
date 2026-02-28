#include "data.h"

typedef enum NativeClassType {
    NONE_CLASS,
    LANG_SYSTEM,
    LANG_STRING,
    IO_PRINTSTREAM
} NativeClassType;

NativeClassType nativeClassFind(char *classname);
int nativeMethodCall(Frame *frame, NativeClassType ntype, char *name, char *type);
