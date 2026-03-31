#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c.h"
#include "data.h"
#include "mmr.h"
#include "util.h"
#include "frame.h"
#include "instruct.h"
#include "method.h"
#include "native.h"
#include "clazz.h"
#include "heap.h"

struct JavaStates javaStates;

static void usage() {
	fprintf(stderr, "usage: java [-cp classpath] class\n");
	exit(EXIT_FAILURE);
}

static void addClassPath(char *classpath) {
    javaStates.class_path = srealloc(javaStates.class_path, sizeof(char *) * (javaStates.num_class_path + 1));
    if (javaStates.class_path == NULL) goto oom;
    javaStates.class_path[javaStates.num_class_path] = sstrdup(classpath);
    if (javaStates.class_path[javaStates.num_class_path] == NULL) goto oom;
    javaStates.num_class_path++;
    return;
oom:
    seterror("Out of memory");
    exit(EXIT_FAILURE);
}

static void bootstrap() {
    /* Bootstrap the Object clazz. */
    clazzLoadObject();
    clazzLoad("java/lang/Class");
    clazzLoad("java/lang/String");
    initPrimitiveMirrors();
}


static void initJvmEnv() {
    ClassFile *class;
    Frame *frame;

    class = loadClass("java/lang/System");
    if (class == NULL) error("Load class fail.");
    frame = framePush(class, 0, 1, NULL);
    
    if (methodCall(class, frame, "initializeSystemClass", "()V", ACC_METHOD_STATIC) == ERR) 
        error("Init jvm environment fail.");
}


/* Init java. */
static void init(int argc, char *argv[]) {
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) 
            usage();
        else if (strcmp(argv[i], "-cp") == 0) {
            if (++i > argc) usage();
            else addClassPath(argv[i]);
        } else {
            javaStates.class_name = sstrdup(argv[i]);
            if (javaStates.class_name == NULL) goto oom;;
        }
    }

    if (javaStates.num_class_path == 0) 
        addClassPath(".");

    javaStates.mode = runMode(argv[0]);
    return;
oom:
    seterror("Out of memory");
    exit(EXIT_FAILURE);
}

static JavaObject *createJavaStringInstance(const char *str) {
    U2 i;
    size_t len;
    Clazz *string_clazz, *char_array_clazz;
    Value v;
    JavaObject *obj;
    JavaArrayObject *array;
    Field *filed;
    

    len = strlen(str);
    string_clazz = clazzLoad("java/lang/String");
    obj = newObj(string_clazz);
    char_array_clazz = clazzLoad("[C");
    v.h = heapNewArray(char_array_clazz, len);
    array = v.h->obj;
    
    for (i = 0; i < len; i++) {
        ((char *)array->data)[i] = str[i];
    }

    filed = clazzFindField(string_clazz, "value", "[C");
    if (filed == NULL) error("Not find value field in java/lang/String.");
    clazzSetInstanceVar(obj, filed, v);

    return obj;
}

static void prepareBeforeMain(int argc, char *argv[], Frame *frame) {
    int i;
    Value v;
    Clazz *clazz;
    JavaArrayObject *array;

    clazz = clazzLoad("java/lang/String");
    if (clazz == NULL) error("Load clazz: java/lang/String fail."); 
    v.h = heapNewArray(clazz, argc);
    array = v.h->obj;

    for (i = 0; i < argc; i++) {
        array->data[i] = createJavaStringInstance(argv[i]);
    }

    frameStatckPush(frame, v);
}

static void java(int argc, char *argv[]) {
    ClassFile *class;
    Frame *frame;

    class = loadClass(javaStates.class_name);
    if (class == NULL) error("Load class fail.");
    frame = framePush(class, 0, 1, NULL);
    
    prepareBeforeMain(argc, argv, frame);
    if (methodCall(class, frame, "main", "([Ljava/lang/String;)V", (ACC_METHOD_PUBLIC | ACC_METHOD_STATIC)) == ERR) 
        error("Main method not found in class %s, please define the main method as: \n   public static void main(String[] args)", javaStates.class_name);
}

static void afterexist(void) {
    if (strcmp(javaStates.error, "") != 0)
        fprintf(stderr, "%s\n", javaStates.error);
}

int main(int argc, char *argv[]) {
    if (argc < 2) usage();
	atexit(afterexist);
    init(argc, argv);
    bootstrap();
    initJvmEnv();
    java(argc, argv);
    return EXIT_SUCCESS;
}
