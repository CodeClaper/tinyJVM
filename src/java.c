#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c.h"
#include "mmr.h"
#include "util.h"
#include "frame.h"
#include "instruct.h"
#include "method.h"
#include "native.h"

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
    if (javaStates.num_class_path == 0) addClassPath(".");
    return;
oom:
    seterror("Out of memory");
    exit(EXIT_FAILURE);
}

static void java(void) {
    ClassFile *class;
    Frame *frame;

    class = loadClass(javaStates.class_name);
    if (class == NULL) error("Load class fail.");
    frame = framePush(class, 0, 1, NULL);
    if (methodCall(class, frame, "main", "([Ljava/lang/String;)V", (ACC_METHOD_PUBLIC | ACC_METHOD_STATIC)) == ERR) 
        error("Main method not found in class A, please define the main method as: \n   public static void main(String[] args)");
}

static void afterexist(void) {
    if (strcmp(javaStates.error, "") != 0)
        fprintf(stderr, "%s\n", javaStates.error);
}

int main(int argc, char *argv[]) {
    if (argc < 2) usage();
	atexit(afterexist);
    init(argc, argv);
    java();
    return EXIT_SUCCESS;
}
