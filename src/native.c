#include <stdlib.h>
#include <string.h>
#include "native.h"
#include "data.h"


static void naprintln(Frame *frame, char *type);

static struct {
    char *name;
    NativeClassType nclass;
} nclasstab[] = {
    {"java/lang/System",    LANG_SYSTEM},
    {"java/lang/String",    LANG_STRING},
    {"java/io/PrintStream", IO_PRINTSTREAM},
    {NULL,                  NONE_CLASS}
};

static struct NativeMethod {
    char *name;
    void (*method)(Frame *frame, char *type);
} *nmethodtab[] = {
    [LANG_SYSTEM] = (struct NativeMethod[]) {
        { NULL, NULL}
    },
    [LANG_STRING] = (struct NativeMethod[]) {
        {"charAt", NULL},
        {"length", NULL},
        {NULL, NULL},
    },
    [IO_PRINTSTREAM] = (struct NativeMethod[]) {
        {"print", NULL},
        {"println", naprintln},
    }
};

static void naprintln(Frame *frame, char *type) {

}

/* Find native class. */
NativeClassType nativeClassFind(char *classname) {
    U2 i;
    for (i = 0; nclasstab[i].name; i++) {
        if (strcmp(nclasstab[i].name, classname)) break;
    }

    return nclasstab[i].nclass;
}

/* Invok native method. */
int nativeMethodCall(Frame *frame, NativeClassType ntype, char *name, char *type) {
    for (U2 i = 0; nmethodtab[ntype][i].name != NULL; i++) {
        if (strcmp(name, nmethodtab[ntype][i].name) == 0) {
            nmethodtab[ntype][i].method(frame, type);
            return OK;
        }
    }
    return ERR;
}
