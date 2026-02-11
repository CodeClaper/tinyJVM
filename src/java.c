#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "java.h"
#include "c.h"
#include "mmr.h"
#include "util.h"

struct JavaStates javaStates;

static void usage() {
	fprintf(stderr, "usage: java [-cp classpath] class\n");
	exit(EXIT_FAILURE);
}

static void addClassPath(char *classpath) {
    javaStates.class_path = srealloc(javaStates.class_path, sizeof(char *) * (javaStates.num_class_path + 1));
    if (javaStates.class_path == NULL) goto error;
    javaStates.class_path[javaStates.num_class_path] = sstrdup(classpath);
    if (javaStates.class_path[javaStates.num_class_path] == NULL) goto error;
    javaStates.num_class_path++;
    return;
error:
    seterror("Out of memory");
    exit(EXIT_FAILURE);
}

/* Init java. */
static void initJava(int argc, char *argv[]) {
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-cp") == 0) {
            if (++i > argc) usage();
            else addClassPath(argv[i]);
        } else {
            javaStates.class_name = sstrdup(argv[i]);
            if (javaStates.class_name == NULL) goto error;;
        }
    }
    if (javaStates.num_class_path == 0) addClassPath(".");
    return;
error:
    seterror("Out of memory");
    exit(EXIT_FAILURE);
}

static void runJava(void) {
    ClassFile *class = loadClass(javaStates.class_name);
    if (class == NULL) exit(EXIT_FAILURE);
}

static void existJava(void) {
    fprintf(stderr, "%s\n", javaStates.error);
}

int main(int argc, char *argv[]) {
    if (argc < 2) usage();
	atexit(existJava);
    initJava(argc, argv);
    runJava();
    return EXIT_SUCCESS;
}
