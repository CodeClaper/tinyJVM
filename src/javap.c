#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "javap.h"
#include "java.h"
#include "c.h"
#include "mmr.h"
#include "util.h"
#include "class.h"
#include "instruct.h"

#define PRESEQ      4       /* Columns before sequence in the constant_pool. */
#define CPINDEX     26      /* Columns before index in the constant_pool. */                  
#define CODEINDEX   25      /* Column befire index in the code section. */
#define CPCOMMENT   16      /* COlumn before comment in the constant_pool. */

struct JavaStates javaStates;

/* Quote method name if is it <init>. */
static char *quoteName(char *s) {
    if (strcmp(s, "<init>") == 0) return "\"<init>\"";
    else return s;
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
        { 0x0400,    "ACC_ABSTRACT" },
        { 0x1000,    "ACC_SYNTHETIC" },
        { 0x2000,    "ACC_ANNOTATION" },
        { 0x4000,    "ACC_EUM" }
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

static void printFieldAccessFlag(U2 flag) {
    struct {
        U2 flag;
        char *s;
    } flags[] = {
        { 0x0001,    "ACC_PUBLIC" },
        { 0x0002,    "ACC_PRIVATE" },
        { 0x0004,    "ACC_PROTECTED" },
        { 0x0008,    "ACC_STATIC" },
        { 0x0010,    "ACC_FINAL" },
        { 0x0040,    "ACC_VOLATILE" },
        { 0x0080,    "ACC_TRANSIENT" },
        { 0x1000,    "ACC_SYNTHETIC" },
        { 0x4000,    "ACC_EUM" }
    };
    int p = 0;
    
    printf("    flags: ");
    for (U2 i = 0; i < LEN(flags); i++) {
        if (flag & flags[i].flag) {
            if (p) printf(", ");
            printf("%s", flags[i].s);
            p = 1;
        }
    }
    printf("\n");
}


static void printMethodAccessFlag(U2 flag) {
    struct {
        U2 flag;
        char *s;
    } flags[] = {
        { 0x0001,    "ACC_PUBLIC" },
        { 0x0002,    "ACC_PRIVATE" },
        { 0x0004,    "ACC_PROTECTED" },
        { 0x0008,    "ACC_STATIC" },
        { 0x0010,    "ACC_FINAL" },
        { 0x0020,    "ACC_SYNCHRONIZED" },
        { 0x0040,    "ACC_BRIDGE" },
        { 0x0080,    "ACC_VARARGS" },
        { 0x0100,    "ACC_NATIVE" },
        { 0x0400,    "ACC_ABSTRACT" },     
        { 0x0800,    "ACC_STRICT" },
        { 0x1000,    "ACC_SYNTHETIC" }
    };
    int p = 0;
    
    printf("    flags: ");
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

static int printSeqSpace(int i) {
    int d = 0, r = 0;
    do { d++; } while(i /= 10);
    r = d = (d < PRESEQ) ? PRESEQ - d : 0;
    while ((d--) > 0) putchar(' ');
    return r;
}

static void printIndexSpace(int n, int d) {
    n = n > 0 ? CPINDEX - n - d : 0;
    printf("%*c", n, ' ');
}

static void printCommentSpace(int n) {
    n = n > 0 && n < CPCOMMENT ? CPCOMMENT - n - 1 : 1;
    printf("%*c", n, ' ');
}

static void printCP(ClassFile *class) {
    U2 i, n, d;
    ConstantPoolInfo **cp = class->constant_pool;

	printf("Constant pool:\n");
    for (i = 1; i < class->constant_pool_count; i++) {
        d = printSeqSpace(i);
        n = printf("#%d = %s", i, classGetConstantTagName(cp[i]->tag));
        printIndexSpace(n, d);
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
                n = printf("#%u", cp[i]->info.class_info.name_index);
                printCommentSpace(n);
                printf("// %s", classGetUtf8(class, cp[i]->info.class_info.name_index));
                break;
            case CONSTANT_String:
                n = printf("#%u", cp[i]->info.string_info.string_index);
                printCommentSpace(n);
                printf("// %s", classGetUtf8(class, cp[i]->info.string_info.string_index));
                break;
            case CONSTANT_Fieldref:
                n = printf("#%u.#%u", cp[i]->info.fieldref_info.class_index, cp[i]->info.fieldref_info.name_type_index);
                printCommentSpace(n);
                printf("// %s.%s:%s", classGetClassName(class, cp[i]->info.fieldref_info.class_index), 
                       classGetNameAndTypeForName(class, cp[i]->info.fieldref_info.name_type_index), 
                       classGetNameAndTypeForType(class, cp[i]->info.fieldref_info.name_type_index));
                break;
            case CONSTANT_Methodref:
                n = printf("#%u.#%u", cp[i]->info.methodref_info.class_index, cp[i]->info.methodref_info.name_type_index);
                printCommentSpace(n);
                printf("// %s.%s:%s", classGetClassName(class, cp[i]->info.methodref_info.class_index), 
                       quoteName(classGetNameAndTypeForName(class, cp[i]->info.methodref_info.name_type_index)), 
                       classGetNameAndTypeForType(class, cp[i]->info.methodref_info.name_type_index));
                break;
            case CONSTANT_InterfaceMethodref:
                n = printf("#%u.#%u", cp[i]->info.interfacemethodref_info.class_index, cp[i]->info.interfacemethodref_info.name_type_index);
                printCommentSpace(n);
                printf("// %s.%s:%s", classGetClassName(class, cp[i]->info.interfacemethodref_info.class_index), 
                       quoteName(classGetNameAndTypeForName(class, cp[i]->info.interfacemethodref_info.name_type_index)), 
                       classGetNameAndTypeForType(class, cp[i]->info.interfacemethodref_info.name_type_index));
                break;
                break;
            case CONSTANT_NameAndType:
                n = printf("#%u:#%u", cp[i]->info.nametype_info.name_index, cp[i]->info.nametype_info.descriptor_index);
                printCommentSpace(n);
                printf("// %s:%s", quoteName(classGetNameAndTypeForName(class, i)), classGetNameAndTypeForType(class, i));
                break;
            case CONSTANT_MethodHandle:
                printf("%u:#%u", cp[i]->info.methodhandle_info.reference_kind, cp[i]->info.methodhandle_info.reference_index);
                break;
            case CONSTANT_MethodType:
                printf("#%u", cp[i]->info.methodtype_info.descriptor_index);
                break;
            case CONSTANT_Dynamic:
                break;
            case CONSTANT_InvokeDyanmic:
                printf("%u:#%u", cp[i]->info.invokedynamic_info.bootstrap_method_attr_index, cp[i]->info.invokedynamic_info.name_type_index);
                break;
            case CONSTANT_Module:       
            case CONSTANT_Package:
                break;
        }
        printf("\n");
    }
}

static char *printType(char *type) {
    char *s = type + 1;
    switch (*type) {
        case 'B': printf("bytes"); break;
        case 'C': printf("char"); break;
        case 'D': printf("double"); break;
        case 'F': printf("float"); break;
        case 'I': printf("int"); break;
        case 'J': printf("long"); break;
        case 'S': printf("short"); break;
        case 'V': printf("void"); break;
        case 'Z': printf("boolean"); break;
        case 'L': {
            while (*s && *s != ';') {
                if (*s == '/') putchar('.');
                else putchar(*s);
                s++;
            }
            if (*s == ';') s++;
            break;
        }
        case '[': {
            printType(s);
            printf("[]");
            break;
        }
    }
    return s;
}

static void printDeclaration(char *descriptor, char *name, int init) {
    int p = 0;
    char *s = strrchr(descriptor, ')');
    if (s == NULL) {
        printType(descriptor);
        printf(" %s", name);
    } else {
        if (!init) {
            printType(s + 1);
            putchar(' ');
        } 
        printf("%s(", name);
        s = descriptor + 1;
        while (*s && *s != ')') {
            if (p) printf(", ");
            s = printType(s);
            p = 1;
        }
        putchar(')');
    }
}

static void printField(ClassFile *class) {
    for (U2 i = 0; i < class->fields_count; i++) {
        FieldInfo *field = class->fields[i];
        if (!javaStates.javapOptions.private && field->access_flags & ACC_FIELD_PRIVATE) return;
        if (field->access_flags & ACC_FIELD_PRIVATE) printf("  private ");
        else if (field->access_flags & ACC_FIELD_PROTECTED) printf("  protected ");
        else if (field->access_flags & ACC_FIELD_PUBLIC) printf("  public ");
        if (field->access_flags & ACC_FIELD_STATIC) printf("static ");
        if (field->access_flags & ACC_FIELD_FINAL) printf("final ");
        if (field->access_flags & ACC_FIELD_TRANSIENT) printf("transient ");
        if (field->access_flags & ACC_FIELD_VOLATILE) printf("volatile ");
        char *descriptor = classGetUtf8(class, field->descriptor_index);
        char *name = classGetUtf8(class, field->name_index);
        printDeclaration(descriptor, name, false);
        printf(";\n");
        if (javaStates.javapOptions.verbose || javaStates.javapOptions.sflag) printf("    descriptor: %s\n", classGetUtf8(class, field->descriptor_index));
        if (javaStates.javapOptions.verbose) printFieldAccessFlag(field->access_flags);
        if (javaStates.javapOptions.verbose || javaStates.javapOptions.cflag || javaStates.javapOptions.lflag) printf("\n");
    }
}

static void printCode(ClassFile *class, Code_attribute *codeattr) {
    U1 *code;
    U1 opcode;
    U4 i, base;
    I2 j, off;
    int m, n;

    code = codeattr->code;

    printf("    Code:\n");
    if (javaStates.javapOptions.verbose) printf("      statck=%u, locals=%u, args_size=%u\n", codeattr->max_stack, codeattr->max_locals, 0);
    for (i = 0; i < codeattr->code_length; i++) {
        opcode = code[i];
        if (javaStates.javapOptions.verbose) printf("  ");
        n = printf("%8u: %s", i, getOpName(opcode));
        m = n < CODEINDEX ? CODEINDEX - n - 1 : 1;
        switch (code[i]) {
            case WIDE: {
                switch (code[++i]) {
                    case IINC: 
                        i += 4;
                        break;
                    case ILOAD: 
                    case FLOAD: 
                    case ALOAD: 
                    case LLOAD: 
                    case DLOAD: 
                    case ISTORE: 
                    case FSTORE: 
                    case ASTORE: 
                    case LSTORE: 
                    case DSTORE:
                        i += 2;
                        break;
                }
                break;
            }
            case BIPUSH: {
                I1 val;
                U1 byte = code[++i];
                memcpy(&val, &byte, sizeof(val));
                printf("%*c%d", m, ' ', val);
                break;
            }
            case SIPUSH: {
                I2 val;
                U1 high_byte = code[++i];
                U1 low_byte = code[++i];
                val = castShort(high_byte, low_byte);
                printf("%*c%d", m, ' ', val);
                break;
            }
            case IINC: {
                I1 val;
                U1 byte = code[++i];
                memcpy(&val, &byte, sizeof(val));
                printf("%*c%d, ", m, ' ', val);
                byte = code[++i];
                memcpy(&val, &byte, sizeof(val));
                printf("%d", val);
                break;
            }
            case GOTO:
            case IF_ACMPEQ:
            case IF_ACMPNE:
            case IF_ICMPEQ:
            case IF_ICMPNE:
            case IF_ICMPGE:
            case IF_ICMPGT:
            case IF_ICMPLE:
            case IF_ICMPLT:
            case IFEQ:
            case IFNE:
            case IFLT:
            case IFLE:
            case IFGT:
            case IFGE:
            case JSR: {
                int val;
                base = i;
                U1 high_byte = code[++i];
                U1 low_byte = code[++i];
                val = castShort(high_byte, low_byte);
			    memcpy(&off, &val, sizeof(off));
                off += base;
			    printf("%*c%d", m, ' ', off);
                break;
            }
            default:
                for (j = 0; j < getNoperands(opcode); j++) i++;
                break;
        }
        printf("\n");
    }
   
}

static void printMethod(ClassFile *class) {
    for (U2 i = 0; i < class->method_count; i++) {
        int init = 0;
        char *name, *descriptor;
        MethodInfo *method;
        AttributeInfo *cattr, *lnattr, *lvattr;

        method = class->methods[i];
        descriptor = classGetUtf8(class, method->descriptor_index);
        name = classGetUtf8(class, method->name_index);
        if (strcmp(name, "<init>") == 0) {
            name = classGetClassName(class, class->this_class);
            init = 1;
        }

        if (!javaStates.javapOptions.private && method->access_flags & ACC_METHOD_PRIVATE) return;
        if (i && (javaStates.javapOptions.lflag || javaStates.javapOptions.sflag || javaStates.javapOptions.cflag)) putchar('\n');
        if (method->access_flags & ACC_METHOD_PRIVATE) printf("  private ");
        else if (method->access_flags & ACC_METHOD_PROTECTED) printf("  protected ");
        else if (method->access_flags & ACC_METHOD_PUBLIC) printf("  public ");
        if (method->access_flags & ACC_METHOD_ABSTRACT) printf("abstract ");
        if (method->access_flags & ACC_METHOD_STATIC) printf("static ");
        if (method->access_flags & ACC_METHOD_FINAL) printf("final ");
        if (method->access_flags & ACC_METHOD_SYNCHRONIZED) printf("synchronized ");
        if (method->access_flags & ACC_METHOD_NATIVE) printf("native ");
        if (method->access_flags & ACC_METHOD_STRICT) printf("strict ");
        printDeclaration(descriptor, name, init);
        printf(";\n");
        if (javaStates.javapOptions.verbose || javaStates.javapOptions.sflag) printf("    descriptor: %s\n", classGetUtf8(class, method->descriptor_index));
        if (javaStates.javapOptions.verbose) printMethodAccessFlag(method->access_flags);

        cattr = classGetAttr(method->attributes, method->attribute_count, ATT_Code);
        if (cattr != NULL) {
            lnattr = classGetAttr(method->attributes, method->attribute_count, ATT_LineNumberTable);
            lvattr = classGetAttr(method->attributes, method->attribute_count, ATT_LocalVariableTable);
            if (javaStates.javapOptions.cflag) printCode(class, &cattr->info.code);
        }
    }
}

static void printSource(ClassFile *class) {
    AttributeInfo *attr = classGetAttr(class->attributes, class->attribute_count, ATT_SourceFile);
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
        } 
        else if (strcmp(argv[i], "-verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            javaStates.javapOptions.verbose = 1;
            javaStates.javapOptions.sflag = 1;
            javaStates.javapOptions.lflag = 1;
            javaStates.javapOptions.cflag = 1;
        } 
        else if (strcmp(argv[i], "-private") == 0 || strcmp(argv[i], "-p") == 0) javaStates.javapOptions.private = 1;
        else if (strcmp(argv[i], "-s") == 0) javaStates.javapOptions.sflag = 1;
        else if (strcmp(argv[i], "-l") == 0) javaStates.javapOptions.lflag = 1;
        else if (strcmp(argv[i], "-c") == 0) javaStates.javapOptions.cflag = 1;
        else {
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
    ClassFile *class = loadClass(javaStates.class_name);
    if (class == NULL) exit(EXIT_FAILURE);

    printSource(class);
    printHeader(class);
    if (javaStates.javapOptions.verbose) {
        printf("\n");
        printMeta(class);
        printCP(class);
        printf("{\n");
    } else {
        printf(" {\n");
    }
    printField(class);
    printMethod(class);
}

int main(int argc, char *argv[]) {
    if (argc < 2) usage();
    init(argc, argv);
    javap();
    return EXIT_SUCCESS;
}
