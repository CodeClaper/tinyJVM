#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "javap.h"
#include "java.h"
#include "c.h"
#include "mmr.h"
#include "util.h"
#include "class.h"

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
        } else if (strcmp(argv[i], "-verbose") == 0) {
            javaStates.verbose = 1;
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

static void printClassAccessFlag(U2 flag) {
    struct {
        U2 flag;
        char *s;
    } flags[] = {
        { 0x0001,    "ACC_PUBLIC" },
        { 0x0010,    "ACC_FINAL" },
        { 0x0020,    "ACC_SUPER" },
        { 0x0200,    "ACC_INTERFACE" },
        { 0x0400,    "ACC_SYNTHETIC" },
        { 0x1000,    "ACC_ANNOTATION" },
        { 0x2000,    "ACC_EUM" }
    };
    int p = 0;
    
    printf("  flags: ");
    for (U2 i = 0; i < LEN(flags); i++) {
        if (flag & flags[i].flag) {
            if (p) printf(", ");
            printf("%s", flags[i].s);
            p = 1;
        }
    }
    printf("\n");
}

static void printClass(ClassFile *class, U2 class_index) {
    char *s = classGetClassName(class, class_index);
    while (*s) {
        if (*s == '/') putchar('.');
        else putchar(*s);
        s++;
    } 
}

static void printMeta(ClassFile *class) {
	printf("  minor version: %u\n", class->minor_version);
	printf("  major version: %u\n", class->major_version);
    printClassAccessFlag(class->access_flags);
}

static void printCP(ClassFile *class) {
    U2 i;
    ConstantPoolInfo **cp = class->constant_pool;

	printf("Constant pool:\n");
    for (i = 1; i < class->constant_pool_count; i++) {
        printf("  #%d = %s\t\t\t", i, classGetConstantTagName(cp[i]->tag));
        switch (cp[i]->tag) {
            case CONSTANT_Utf8:
                printf("%s", cp[i]->info.utf8_info.bytes);
                break;
            case CONSTANT_Integer:
                printf("%ld", (long)castInt(cp[i]->info.integer_info.bytes));
                break;
            case CONSTANT_Float:
                printf("%gf", castFloat(cp[i]->info.float_info.bytes));
                break;
            case CONSTANT_Long:
                printf("%ld", castLong(cp[i]->info.long_info.high_bytes, cp[i]->info.long_info.low_bytes));
                i++;
                break;
            case CONSTANT_Double:
                printf("%gd", castDouble(cp[i]->info.double_info.high_bytes, cp[i]->info.long_info.low_bytes));
                i++;
                break;
            case CONSTANT_Class:
                break;
            case CONSTANT_String:
                printf("#%u", cp[i]->info.string_info.string_index);
                printf("\t\t// %s", classGetUtf8(class, cp[i]->info.string_info.string_index));
                break;
            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref:
            case CONSTANT_NameAndType:
            case CONSTANT_MethodHandle:
            case CONSTANT_MethodType:
            case CONSTANT_Dynamic:
            case CONSTANT_InvokeDyanmic:
            case CONSTANT_Module:             
            case CONSTANT_Package:
                break;
        }
        printf("\n");
    }
}

static void printSource(ClassFile *class) {
    AttributeInfo *attr = classGetAttr(class, ATT_SourceFile);
    if (attr == NULL) return;
	printf("Compiled from \"%s\"\n", classGetUtf8(class, attr->info.sourcefile.source_index));
}

static void printHeader(ClassFile *class) {
    if (class->access_flags & ACC_CLASS_PUBLIC) printf("public ");
    if (class->access_flags & ACC_CLASS_INTERFACE) printf("interface ");
    else if (class->access_flags & ACC_CLASS_ENUM) printf("enum ");
    else {
        if (class->access_flags & ACC_CLASS_ABSTRACT) printf("abstract ");
        else if (class->access_flags & ACC_CLASS_FINAL) printf("final ");
        printf("class ");
    }
    printClass(class, class->this_class);
    if (class->super_class && !classIsTopClass(class)) {
        printf(" extends ");
        printClass(class, class->super_class);
    }
    if (class->interfaces_count > 0) {
        printf(" implements ");
        for (U2 i = 0; i < class->interfaces_count; i++) {
            if (i > 0) printf(", ");
            printClass(class, class->interfaces[i]);
        }
    }
}

static void javap() {
    ClassFile *class = loadClass(javaStates.class_name);
    if (class == NULL) exit(EXIT_FAILURE);
    printSource(class);
    printHeader(class);
    if (javaStates.verbose) {
        printf("\n");
        printMeta(class);
        printCP(class);
    } else {
        printf(" {\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) usage();
    init(argc, argv);
    javap();
    return EXIT_SUCCESS;
}
