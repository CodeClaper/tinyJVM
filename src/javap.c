#include <stdint.h>
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
#define CPCOMMENT   16      /* COlumn before comment in the constant_pool. */
#define CODEINDEX   25      /* Column befire index in the code section. */
#define CODECOMMENT 45      /* Column before comments in the code section*/

struct JavaStates javaStates;
struct JavapOptions javapOptions;


/* Get number of columns to align index or comment text. */
static inline int getColumn(int max, int n) {
    return (n > 0 && n < max) ? max - n - 1 : 1;
}

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

static U2 printDeclaration(char *descriptor, char *name, int init) {
    int nargs = 0;
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
            if (nargs) printf(", ");
            s = printType(s);
            nargs++;
        }
        putchar(')');
    }

    return nargs;
}

/* Print out field. */
static void printField(ClassFile *class) {
    for (U2 i = 0; i < class->fields_count; i++) {
        FieldInfo *field = class->fields[i];
        if (!javapOptions.private && field->access_flags & ACC_FIELD_PRIVATE) return;
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
        if (javapOptions.sflag) printf("    descriptor: %s\n", classGetUtf8(class, field->descriptor_index));
        if (javapOptions.verbose) printFieldAccessFlag(field->access_flags);
        if (javapOptions.cflag || javapOptions.lflag) printf("\n");
    }
}

/* Print out method code. */
static void printCode(ClassFile *class, Code_attribute *codeattr, U2 nargs) {
    U1 *code;
    U1 opcode;
    U4 i, base;
    I2 off;
    U1 a, b, c, d;
    I4 j, offw, match, def, npairs, low, high;
    int m, n;
    char *cname, *name, *type;

    code = codeattr->code;

    printf("    Code:\n");
    if (javapOptions.verbose) 
        printf("      stack=%u, locals=%u, args_size=%u\n", codeattr->max_stack, codeattr->max_locals, nargs);
    for (i = 0; i < codeattr->code_length; i++) {
        opcode = code[i];
        if (javapOptions.verbose) printf("  ");
        n = printf("%7u: %s", i, getOpName(opcode));
        m = getColumn(CODEINDEX, n);
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
                I2 val;
                base = i;
                val = code[++i] << 8;
                val |= code[++i];
			    memcpy(&off, &val, sizeof(off));
                off += base;
			    printf("%*c%d", m, ' ', off);
                break;
            }
            case GOTO_W:
            case JSR_W: {
                I4 val;
			    base = i;
                a = code[++i];
                b = code[++i];
                c = code[++i];
                d = code[++i];
                val = (a << 24) | (b << 16) | (c << 8) | d;
			    memcpy(&offw, &val, sizeof(offw));
                offw += base;
			    printf("%*c%d", m, ' ', offw);
                break;
            }
            case ISTORE:
            case LSTORE:
            case FSTORE:
            case DSTORE:
            case ASTORE: 
            case ILOAD: 
            case LLOAD:
            case FLOAD: 
            case DLOAD: 
            case ALOAD: {
                U1 val = code[++i];
			    printf("%*c%d", m, ' ', val);
                break;
            }
            case GETFIELD:
            case PUTFIELD: {
                U2 val;
                a = code[++i];
                b = code[++i];
                val = a << 8 | b;
                n += printf("%*c#%u", m, ' ', val);
                m = getColumn(CODECOMMENT, n);
                name = classGetNameAndTypeForName(class, class->constant_pool[val]->info.fieldref_info.name_type_index);
                type = classGetNameAndTypeForType(class, class->constant_pool[val]->info.fieldref_info.name_type_index);
                printf("%*c// Field %s:%s", m, ' ', name, type);
                break;
            }
            case LOOKUPSWITCH: {
			    base = i++;
                while (i % 4) i++;
                a = code[i++];
                b = code[i++];
                c = code[i++];
                d = code[i++];
                def = ((a << 24) | (b << 16) | (c << 8) | d) + base;
                a = code[i++];
                b = code[i++];
                c = code[i++];
                d = code[i++];
                npairs = (a << 24) | (b << 16) | (c << 8) | d;
                printf("   { // %d\n", npairs);
                for (j = 0; j < npairs; j++) {
                    a = code[i++];
                    b = code[i++];
                    c = code[i++];
                    d = code[i++];
                    match = (a << 24) | (b << 16) | (c << 8) | d;
                    a = code[i++];
                    b = code[i++];
                    c = code[i++];
                    d = code[i++];
                    offw = ((a << 24) | (b << 16) | (c << 8) | d) + base;
                    printf("%24d: %d\n", match, offw);
                }
                i--;
                printf("                 default: %d\n", def);
                printf("            }");
                break;
            }
            case TABLESWITCH: {
			    base = i++;
                while (i % 4) i++;
                a = code[i++];
                b = code[i++];
                c = code[i++];
                d = code[i++];
                def = ((a << 24) | (b << 16) | (c << 8) | d) + base;
                a = code[i++];
                b = code[i++];
                c = code[i++];
                d = code[i++];
                low = (a << 24) | (b << 16) | (c << 8) | d;
                a = code[i++];
                b = code[i++];
                c = code[i++];
                d = code[i++];
                high = (a << 24) | (b << 16) | (c << 8) | d;
                printf("   { // %d to %d\n", low, high);
                for (j = low; j <= high; j++) {
                    a = code[i++];
                    b = code[i++];
                    c = code[i++];
                    d = code[i++];
                    offw = ((a << 24) | (b << 16) | (c << 8) | d) + base;
                    printf("%24d: %d\n", j, offw);
                }
                i--;
                printf("                 default: %d\n", def);
                printf("            }");
                break;
            }
            case GETSTATIC: {
                U2 val;
                a = code[++i];
                b = code[++i];
                val = a << 8 | b;
                n += printf("%*c#%u", m, ' ', val);
                m = getColumn(CODECOMMENT, n);
                cname = classGetClassName(class, class->constant_pool[val]->info.fieldref_info.class_index); 
                name = classGetNameAndTypeForName(class, class->constant_pool[val]->info.fieldref_info.name_type_index);
                type = classGetNameAndTypeForType(class, class->constant_pool[val]->info.fieldref_info.name_type_index);
                printf("%*c// Field %s.%s:%s", m, ' ', cname, name, type);
                break;
            }
            case INVOKEVIRTUAL:
            case INVOKESPECIAL:
            case INVOKESTATIC: {
                U2 val;
                a = code[++i];
                b = code[++i];
                val = a << 8 | b;
                n += printf("%*c#%u", m, ' ', val);
                m = getColumn(CODECOMMENT, n);
                cname = classGetClassName(class, class->constant_pool[val]->info.fieldref_info.class_index); 
                name = classGetNameAndTypeForName(class, class->constant_pool[val]->info.fieldref_info.name_type_index);
                type = classGetNameAndTypeForType(class, class->constant_pool[val]->info.fieldref_info.name_type_index);
                if (strcmp(cname, classGetClassName(class, class->this_class)) == 0) cname = "";
                name = quoteName(name);
                printf("%*c// Method %s%s%s:%s", m, ' ', cname, (*cname == '\0' ? "" : "."), name, type);
                break;
            }
            case INVOKEINTERFACE: {
                U2 val;
                U1 count;
                a = code[++i];
                b = code[++i];
                count = code[++i];
                val = a << 8 | b;
                n += printf("%*c#%u,  %u", m, ' ', val, count);
                m = getColumn(CODECOMMENT, n);
                cname = classGetClassName(class, class->constant_pool[val]->info.fieldref_info.class_index); 
                name = classGetNameAndTypeForName(class, class->constant_pool[val]->info.fieldref_info.name_type_index);
                type = classGetNameAndTypeForType(class, class->constant_pool[val]->info.fieldref_info.name_type_index);
                if (strcmp(cname, classGetClassName(class, class->this_class)) == 0) cname = "";
                name = quoteName(name);
                printf("%*c// Method %s%s%s:%s", m, ' ', cname, (*cname == '\0' ? "" : "."), name, type);
                i++;
                break;
            }
            case LDC:
            case LDC_W:
            case LDC2_W: {
                I2 val;
                a = (code[i] == LDC_W || code[i] == LDC2_W) ? code[++i] : 0;
                b = code[++i];
                val = (a << 8) | b;
                n += printf("%*c#%u", m, ' ', val);
                m = getColumn(CODECOMMENT, n);
                switch (class->constant_pool[val]->tag) {
                    case CONSTANT_String:
                        printf("%*c// String %s", m, ' ', classGetString(class, val));
                        break;
                    case CONSTANT_Integer:
                        printf("%*c// Integer %d", m, ' ', classGetInteger(class, val));
                        break;
                    case CONSTANT_Long:
                        printf("%*c// Integer %ld", m, ' ', classGetLong(class, val));
                        break;
                    case CONSTANT_Double:
                        printf("%*c// double %gd", m, ' ', classGetDouble(class, val));
                        break;
                    case CONSTANT_Float:
                        printf("%*c// double %gf", m, ' ', classGetFloat(class, val));
                        break;
                }
                break;
            }
            case NEW:
            case ANEWARRAY: {
                I2 val;
                a = code[++i];
                b = code[++i];
                val = a << 8 | b;
                n += printf("%*c#%u", m,' ', val);
                m = getColumn(CODECOMMENT, n);
                cname = classGetUtf8(class, class->constant_pool[val]->info.class_info.name_index); 
                printf("%*c// class %s", m, ' ', cname);
                break;
            }
            case MULTIANEWARRAY: {
                I2 val;
                a = code[++i];
                b = code[++i];
                val = a << 8 | b;
                n += printf(" #%u,  %u", val, code[++i]);
                m = getColumn(CODECOMMENT, n);
                cname = classGetUtf8(class, class->constant_pool[val]->info.class_info.name_index); 
                printf("%*c// class \"%s\"", m, ' ', cname);
                break;
            }
            default:
                for (j = 0; j < getNoperands(opcode); j++) i++;
                break;
        }
        printf("\n");
    }
   
}

/* Print out line numbers. */
static void printLineNumbers(LineNumberTable_attribute *ltattr) {
    printf("     LineNumberTable:\n");
    for (U2 i = 0; i < ltattr->line_number_table_length; i++) {
        printf("       line %u: %u\n", ltattr->line_number_table[i]->line_number, ltattr->line_number_table[i]->start_pc);
    }
}

/* Print out local vars. */
static void printLocalVars(ClassFile *class, LocalVariableTable_attribute *lvattr) {
    U2 count, i;
    LocalVariable *lv;
    
    count = lvattr->local_variable_table_length;
    if (count == 0) return;
    printf("     LocalVariableTable:\n");
    printf("       Start  Length  Slot  Name  Signature\n");
    for (i = 0; i < count; i++) {
        lv = lvattr->local_variable_table[i];
        printf("     %7u %7u %5u %5s  %s\n",
               lv->start_pc, lv->length, lv->index, 
               classGetUtf8(class, lv->name_index), 
               classGetUtf8(class, lv->descriptor_index));
    }
}

/* Print method. */
static void printMethod(ClassFile *class) {
    for (U2 i = 0; i < class->method_count; i++) {
        int init = 0;
        U2 nargs;
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

        if (!javapOptions.private && method->access_flags & ACC_METHOD_PRIVATE) return;
        if (i && (javapOptions.lflag || javapOptions.sflag || javapOptions.cflag)) putchar('\n');
        if (method->access_flags & ACC_METHOD_PRIVATE) printf("  private ");
        else if (method->access_flags & ACC_METHOD_PROTECTED) printf("  protected ");
        else if (method->access_flags & ACC_METHOD_PUBLIC) printf("  public ");
        if (method->access_flags & ACC_METHOD_ABSTRACT) printf("abstract ");
        if (method->access_flags & ACC_METHOD_STATIC) printf("static ");
        if (method->access_flags & ACC_METHOD_FINAL) printf("final ");
        if (method->access_flags & ACC_METHOD_SYNCHRONIZED) printf("synchronized ");
        if (method->access_flags & ACC_METHOD_NATIVE) printf("native ");
        if (method->access_flags & ACC_METHOD_STRICT) printf("strict ");
        nargs = printDeclaration(descriptor, name, init);
        /* If not static method, add <this> arguement.*/
        if (!(method->access_flags & ACC_METHOD_STATIC)) nargs++;
        printf(";\n");
        if (javapOptions.sflag) printf("    descriptor: %s\n", classGetUtf8(class, method->descriptor_index));
        if (javapOptions.verbose) printMethodAccessFlag(method->access_flags);

        cattr = classGetAttr(method->attributes, method->attribute_count, ATT_Code);
        if (cattr != NULL) {
            lnattr = classGetAttr(cattr->info.code.attributes, cattr->info.code.attribute_count, ATT_LineNumberTable);
            lvattr = classGetAttr(cattr->info.code.attributes, cattr->info.code.attribute_count, ATT_LocalVariableTable);
            if (javapOptions.cflag) printCode(class, &cattr->info.code, nargs);
            if (javapOptions.lflag && lnattr != NULL) printLineNumbers(&lnattr->info.linenumbertable);
            if (javapOptions.lflag && lvattr != NULL) printLocalVars(class, &lvattr->info.localvariabletable);
        }
    }
}

/* Print source. */
static void printSource(ClassFile *class) {
    AttributeInfo *attr = classGetAttr(class->attributes, class->attribute_count, ATT_SourceFile);
    if (attr == NULL) return;
	printf("Compiled from \"%s\"\n", classGetUtf8(class, attr->info.sourcefile.source_index));
}

/* Print out header. */
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
            javapOptions.verbose = 1;
            javapOptions.sflag = 1;
            javapOptions.lflag = 1;
            javapOptions.cflag = 1;
        } 
        else if (strcmp(argv[i], "-private") == 0 || strcmp(argv[i], "-p") == 0) javapOptions.private = 1;
        else if (strcmp(argv[i], "-s") == 0) javapOptions.sflag = 1;
        else if (strcmp(argv[i], "-l") == 0) javapOptions.lflag = 1;
        else if (strcmp(argv[i], "-c") == 0) javapOptions.cflag = 1;
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
    if (class == NULL) error("Load class file fail.");

    printSource(class);
    printHeader(class);
    if (javapOptions.verbose) {
        printf("\n");
        printMeta(class);
        printCP(class);
        printf("{\n");
    } else {
        printf(" {\n");
    }
    printField(class);
    printMethod(class);
    printf("}\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) usage();
    init(argc, argv);
    javap();
    return EXIT_SUCCESS;
}
