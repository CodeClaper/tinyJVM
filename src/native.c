#include <stdlib.h>
#include <string.h>
#include "native.h"

/* Find native class. */
NativeClassType nativeClassFind(char *classname) {
    struct {
        char *name;
        NativeClassType nclass;
    } nclasstab[] = {
        {"java/lang/System",    LANG_SYSTEM},
        {"java/lang/String",    LANG_STRING},
        {"java/io/PrintStream", IO_PRINTSTREAM},
        {NULL,                  NONE_CLASS}
    };

    U2 i;
    for (i = 0; nclasstab[i].name; i++) {
        if (strcmp(nclasstab[i].name, classname)) break;
    }

    return nclasstab[i].nclass;
}

int nativeMethodCall(Frame *frame, NativeClassType ntype, char *name, char *type) {
    return OK;
}
