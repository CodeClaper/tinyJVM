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
    int r = srealloc(javaStates.classes, sizeof(char *) * (javaStates.num_class_path + 1));
    if (r == ERR) seterror("Out of memory");
    int s = sstrdup(javaStates.class_path[javaStates.num_class_path], classpath);
    if (s == ERR) seterror("Out of memory");
    javaStates.num_class_path++;
}

/* Init java. */
static void initJava(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-cp") == 0) {
            if (++i > argc) usage();
            else addClassPath(argv[i]);
        }
    }
    if (javaStates.num_class_path == 0) 
        addClassPath(".");
}

static void runJava(int argc, char *argv[]) {
    loadClass(argv[0]);
}

int main(int argc, char *argv[]) {
    initJava(argc, argv);
    runJava(argc, argv);
    return EXIT_SUCCESS;
}
