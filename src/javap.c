#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "javap.h"
#include "java.h"
#include "c.h"
#include "mmr.h"
#include "util.h"

struct JavaStates javaStates;

static void usage() {
	fprintf(stderr, "Usage: javap <options> <classes>\n");
    fprintf(stderr, "Where possible options include:\n" );
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

static void init(int argc, char *argv[]) {
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-cp") == 0) {
            if (++i > argc) usage();
            else addClassPath(argv[i]);
        } else {
            javaStates.class_name = sstrdup(argv[i]);
            if (javaStates.class_name == NULL) goto oom;
        }
    }
    if (javaStates.num_class_path == 0) addClassPath(".");
    return;
oom:
    seterror("Out of memory");
    exit(EXIT_FAILURE);
}

static void javap() {

}

int main(int argc, char *argv[]) {
    if (argc < 2) usage();
    init(argc, argv);
    javap();
    return EXIT_SUCCESS;
}
