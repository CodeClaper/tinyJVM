#include <cstddef>
#include <stdio.h>
#include <string.h>
#include "class.h"
#include "mmr.h"
#include "java.h"

/* Read count bytes into buf. */
static int readb(FILE *fp, void *buf, U4 count) {
    if (fread(buf, 1, count, fp) != count) return ERR;
    else return OK;
}

/* Read unsigned integer of size count into *u */
static int readu(FILE *fp, void *u, U4 count) {
    U1 b[4];

    TRY(readb(fp, b, count));
    switch (count) {
        case 1:
            *(U1 *)u = b[0];
            break;
        case 2:
            *(U2 *)u = b[0] << 8 | b[1];
            break;
        case 4:
            *(U4 *)u = b[0] << 24 | b[1] << 16 | b[2] << 8 | b[3];
            break;
        default: 
            goto error;
    }
    return OK;
error:
    return ERR;
}

/* Read string into buffer. */
static int reads(FILE *fp, char **s, U2 count) {
    TRY(salloc(*s, count + 1));
    TRY(readb(fp, *s, count));
    (*s)[count] = '\0';
    return OK;
error:
    return ERR;
}


/* Raad the constant pool. */
static int readcp(FILE *fp, ConstantPoolInfo ***cp, U2 count) {
    if (count == 0) {
        cp = NULL;
        return OK;
    }
    TRY(salloc(*cp, sizeof(ConstantPoolInfo *) * count));
    /* CP starts from 1. */
    for (int i = 1; i < count; i++) {
        TRY(salloc(**cp, sizeof(ConstantPoolInfo)));
        ConstantPoolInfo *current = **cp;
        TRY(readu(fp, &current->tag, 2));
        switch (current->tag) {
            case CONSTANT_Utf8:
                TRY(readu(fp, &current->info.utf8_info.length, 2));
                TRY(reads(fp, &current->info.utf8_info.bytes, current->info.utf8_info.length));
                break;
            case CONSTANT_Integer:
                TRY(readu(fp, &current->info.integer_info.bytes, 4));
                break;
            case CONSTANT_Float:
                TRY(readu(fp, &current->info.float_info.bytes, 4));
                break;
            case CONSTANT_Long:
                TRY(readu(fp, &current->info.long_info.high_bytes, 4));
                TRY(readu(fp, &current->info.long_info.low_bytes, 4));
                break;
            case CONSTANT_Double:
                TRY(readu(fp, &current->info.long_info.high_bytes, 4));
                TRY(readu(fp, &current->info.long_info.low_bytes, 4));
                break;
            case CONSTANT_Class:
                TRY(readu(fp, &current->info.class_info.name_index, 2));
                break;
            case CONSTANT_String:
                TRY(readu(fp, &current->info.string_info.string_index, 2));
                break;
            case CONSTANT_Fieldref:
                TRY(readu(fp, &current->info.fieldref_info.class_index, 2));
                TRY(readu(fp, &current->info.fieldref_info.name_type_index, 2));
                break;
            case CONSTANT_Methodref:
                TRY(readu(fp, &current->info.methodref_info.class_index, 2));
                TRY(readu(fp, &current->info.methodref_info.name_type_index, 2));
                break;
            case CONSTANT_InterfaceMethodref:
                TRY(readu(fp, &current->info.interfacemethodref_info.class_index, 2));
                TRY(readu(fp, &current->info.interfacemethodref_info.name_type_index, 2));
                break;
            case CONSTANT_NameAndType:
                TRY(readu(fp, &current->info.nametype_info.name_index, 2));
                TRY(readu(fp, &current->info.nametype_info.descriptor_index, 2));
                break;
            case CONSTANT_MethodHandle:
                TRY(readu(fp, &current->info.methodhandle_info.reference_kind, 1));
                TRY(readu(fp, &current->info.methodhandle_info.reference_index, 2));
                break;
            case CONSTANT_MethodType:
                TRY(readu(fp, &current->info.methodtype_info.descriptor_index, 2));
                break;
            case CONSTANT_Dynamic:
                TRY(readu(fp, &current->info.dynamic_info.bootstrap_method_attr_index, 2));
                TRY(readu(fp, &current->info.dynamic_info.name_type_index, 2));
                break;
            case CONSTANT_InvokeDyanmic:
                TRY(readu(fp, &current->info.invokedynamic_info.bootstrap_method_attr_index, 2));
                TRY(readu(fp, &current->info.invokedynamic_info.name_type_index, 2));
                break;
            case CONSTANT_Module:
                TRY(readu(fp, &current->info.module_info.name_index, 2));
                break;
            case CONSTANT_Package:
                TRY(readu(fp, &current->info.package_info.name_index, 2));
                break;
            default: goto error;
        }
    }
error:
    return ERR;
}

/* Get class utf-8 string. */
static char *classGetUtf8(ClassFile *class, U2 index) {
    return class->constant_pool[index]->info.utf8_info.bytes;
}

static char *classGetClassName(ClassFile *class, U2 index) {
    return classGetUtf8(class, class->constant_pool[index]->info.class_info.name_index);
}

/* Read class from file. */
static int readClass(FILE *fp, ClassFile *class) {
    TRY(readu(fp, &class->magic, 4));
    TRY(readu(fp, &class->major_version, 2));
    TRY(readu(fp, &class->minor_version, 2));
    TRY(readu(fp, &class->constant_pool_count, 2));
    TRY(readcp(fp, &class->constant_pool, class->constant_pool_count));
	TRY(readu(fp, &class->access_flags, 2));
	TRY(readu(fp, &class->this_class, 2));
	TRY(readu(fp, &class->super_class, 2));
	TRY(readu(fp, &class->interfaces_count, 2));
    return OK;
error:
    return ERR;
}

/* Check the class valid. */
static int checkClass(ClassFile *class) {
    if (class->magic != MAGIC) return ERR;
    return OK;
}

/* Get class from cache. 
 * Return NULL if not found. */
static ClassFile *getClassFromCache(char *class_name) {
    for (ClassFile *class = javaServer.classes; 
            class != NULL; class = class->next) {
        if (strcmp(class_name, classGetClassName(class, class->this_class))) 
            return class;
    }
    return NULL;
}

/* Load class. */
ClassFile *loadClass(char *class_name) {
    ClassFile *class;

    class = getClassFromCache(class_name);
    if (class != NULL) return class;
    
}
