#include <stdio.h>
#include <string.h>
#include "native.h"
#include "data.h"
#include "frame.h"
#include "util.h"


static void nativePrintln(Frame *frame, char *type);
static void nativeRegisterNatives(Frame *frame, char *type) ;
static void nativeGetPrimitiveClass(Frame *frame, char *type);

static struct {
    char *name;
    NativeClassType nclass;
} nclasstab[] = {
    {"java/lang/System",        LANG_SYSTEM},
    {"java/lang/Class",         LANG_CLASS},
    {"java/io/PrintStream",     IO_PRINTSTREAM},
    {NULL,                      NONE_CLASS}
};

static struct NativeMethod {
    char *name;
    void (*method)(Frame *frame, char *type);
} *nmethodtab[] = {
    [LANG_SYSTEM] = (struct NativeMethod[]) {
        {"registerNatives", nativeRegisterNatives}
    },
    [LANG_CLASS] = (struct NativeMethod[]) {
        {"registerNatives", nativeRegisterNatives},
        {"getPrimitiveClass", nativeGetPrimitiveClass}
    },
    [IO_PRINTSTREAM] = (struct NativeMethod[]) {
        {"print", NULL},
        {"println", nativePrintln},
    }
};

/* Native method <println>. */
static void nativePrintln(Frame *frame, char *type) {
    Value v, vfp;

    v = frameStatckPop(frame);
    if (strcmp(type, "()V") == 0) 
        fprintf((FILE *)v.h->obj, "\n");
    else {
        vfp = frameStatckPop(frame);
        if (strcmp(type, "(Ljava/lang/String;)V") == 0) fprintf((FILE *) vfp.h->obj, "%s\n", (char *)v.h->obj);
        else if (strcmp(type, "(B)V") == 0) fprintf((FILE *)vfp.h->obj, "%d\n", v.i);
        else if (strcmp(type, "(C)V") == 0) fprintf((FILE *)vfp.h->obj, "%c\n", v.i);
        else if (strcmp(type, "(D)V") == 0) fprintf((FILE *)vfp.h->obj, "%.16g\n", v.d);
        else if (strcmp(type, "(F)V") == 0) fprintf((FILE *)vfp.h->obj, "%.16g\n", v.f);
        else if (strcmp(type, "(I)V") == 0) fprintf((FILE *)vfp.h->obj, "%d\n", v.i);
        else if (strcmp(type, "(J)V") == 0) fprintf((FILE *)vfp.h->obj, "%ld\n", v.l);
        else if (strcmp(type, "(S)V") == 0) fprintf((FILE *)vfp.h->obj, "%d\n", v.i);
        else if (strcmp(type, "(Z)V") == 0) fprintf((FILE *)vfp.h->obj, "%d\n", v.i);
     }
}

static void nativeRegisterNatives(Frame *frame, char *type) {
    // Nothing todo.
}


static void nativeGetPrimitiveClass(Frame *frame, char *type) {

}

/* Find native class. */
NativeClassType nativeClassFind(char *classname) {
    U2 i;

    for (i = 0; nclasstab[i].name; i++) 
        if (strcmp(nclasstab[i].name, classname) == 0) break;

    return nclasstab[i].nclass;
}

void *nativeJavaObj(NativeClassType ntype, char *objname, char *objtype) {
    switch (ntype) {
        case LANG_SYSTEM: {
            if (strcmp(objtype, "Ljava/io/PrintStream;") == 0) {
                if (strcmp(objname, "out") == 0) return stdout;
                else if (strcmp(objname, "err") == 0) return stderr;
                else if (strcmp(objname, "in") == 0) return stdin;
            }
            break;
        }
        default: break;
    }
    return NULL;
}


/* Invok native method. */
int nativeMethodCall(Frame *frame, char *classname, char *name, char *type) {
    U2 i;
    NativeClassType ntype;

    ntype = nativeClassFind(classname);
    if (ntype == NONE_CLASS) 
        error("Not found native method: %s in class: %s", name, classname);

    for (i = 0; nmethodtab[ntype][i].name != NULL; i++) {
        if (strcmp(name, nmethodtab[ntype][i].name) == 0) {
            nmethodtab[ntype][i].method(frame, type);
            return OK;
        }
    }

    error("Not found native method: %s in class: %s", name, classname);
    return ERR;
}
