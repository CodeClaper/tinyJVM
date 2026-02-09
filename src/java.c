#include <stdlib.h>
#include <string.h>
#include "java.h"
#include "c.h"
#include "mmr.h"
#include "util.h"

struct JavaStates javaStates;

static void addClassPath(char *classpath) {
    int r = srealloc(javaStates.classes, sizeof(char *) * (javaStates.num_class_path + 1));
    if (r == ERR) seterror("Out of memory");
    javaStates.class_path[javaStates.num_class_path] = classpath;
    javaStates.num_class_path++;
}

static void initJava(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-cp") == 0) {
        }
    }
}

int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    return EXIT_SUCCESS;
}
