#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include "class.h"
#include "data.h"
#include "mmr.h"
#include "util.h"
#include "instruct.h"
#include "method.h"

static FrameTag getFramTag(U1 val);
static AttributeTag getAttrTag(char *attr_name);
static int readAnnotation(FILE *fp, Annotation **annotation);
static int readElementValues(FILE *fp, ElementValue ***p, U2 count);

/* Bootstrap class is Oject. */
static ClassFile *bootstrapClass = NULL;

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
    *s = salloc(count + 1);
    if (*s == NULL) goto oom;
    TRY(readb(fp, *s, count));
    (*s)[count] = '\0';
    return OK;
oom:
    seterror("Out of memory");
error:
    return ERR;
}

static int readIndex(FILE *fp, U2 **p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }

    *p = salloc(sizeof(U2) * count);
    if (*p == NULL) goto oom;
    for (U2 i = 0; i < count; i++) 
        TRY(readu(fp, &(*p)[i], 2));

    return OK;
oom:
    seterror("Out of memory");
error:
    return ERR;
}

static int readBytes(FILE *fp, U1 **p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }

    *p = salloc(sizeof(U1) * count);
    if (*p == NULL) goto oom;
    for (U2 i = 0; i < count; i++) 
        TRY(readu(fp, &(*p)[i], 1));

    return OK;
oom:
    seterror("Out of memory");
error:
    return ERR;
}

/* Raad the constant pool. */
static int readCP(FILE *fp, ConstantPoolInfo ***cp, U2 count) {
    if (count == 0) {
        *cp = NULL;
        return OK;
    }

    *cp = salloc(sizeof(ConstantPoolInfo *) * count);
    if (cp == NULL) goto oom;

    /* CP starts from 1. */
    for (int i = 1; i < count; i++) {
        (*cp)[i] = salloc(sizeof(ConstantPoolInfo));
        if ((*cp)[i] == NULL) goto oom;
        ConstantPoolInfo *current = (*cp)[i];
        TRY(readu(fp, &current->tag, 1));
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
                i++;
                break;
            case CONSTANT_Double:
                TRY(readu(fp, &current->info.long_info.high_bytes, 4));
                TRY(readu(fp, &current->info.long_info.low_bytes, 4));
                i++;
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
    return OK;
oom:
    seterror("Out of memory");
error:
    return ERR;
}

static int readCode(FILE *fp, U1 **code, ClassFile *class, U4 count) {
    U4 i, base;
    I4 j, npairs;

    if (count == 0) {
        *code = NULL;
        return OK;
    }

    *code = salloc(sizeof(U1) * count);
    if (*code == NULL) goto oom;
    
    for (i = 0; i < count; i++) {
        TRY(readu(fp, &(*code)[i], 1));
        if ((*code)[i] > CODE_LAST) {
            seterror("Bad opcode");
            goto error;
        }
        switch ((*code)[i]) {
            case WIDE: {
                TRY(readu(fp, &(*code)[++i], 1));
                switch ((*code)[i]) {
                    case IINC: 
                        TRY(readu(fp, &(*code)[++i], 1));
                        TRY(readu(fp, &(*code)[++i], 1));
                        TRY(readu(fp, &(*code)[++i], 1));
                        TRY(readu(fp, &(*code)[++i], 1));
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
                        TRY(readu(fp, &(*code)[++i], 1));
                        TRY(readu(fp, &(*code)[++i], 1));
                        break;
                    default: 
                        seterror("Bad opcode.");
                        goto error;
                        break;
                }
                break;
            }
            case LOOKUPSWITCH: {
                while ((3 - (i % 4)) > 0)
                    TRY(readu(fp, &(*code)[++i], 1));
                for (j = 0; j < 8; j++)
                    TRY(readu(fp, &(*code)[++i], 1));
                npairs = ((*code)[i-3] << 24) | ((*code)[i-2] << 16) | ((*code)[i-1] << 8) | (*code)[i];
                if (npairs < 0) {
                    seterror("The npairs should not be less than zero in <lookupswitch> instruct.");
                    goto error;
                }
                for (j = 8 * npairs; j > 0; j--)
                    TRY(readu(fp, &(*code)[++i], 1));
                break;
            }
            case TABLESWITCH: {
                I4 offset, low, high;

                base = i;
                while ((3 - (i % 4)) > 0)
                    TRY(readu(fp, &(*code)[++i], 1));
                for (j = 0; j < 12; j++)
                    TRY(readu(fp, &(*code)[++i], 1));
                offset = ((*code)[i-11] << 24) | ((*code)[i-10] << 16) | ((*code)[i-9] << 8) | (*code)[i-8];
                low = ((*code)[i-7] << 24) | ((*code)[i-6] << 16) | ((*code)[i-5] << 8) | (*code)[i-4];
                high = ((*code)[i-3] << 24) | ((*code)[i-2] << 16) | ((*code)[i-1] << 8) | (*code)[i];

                if (base + offset < 0 || base + offset > count) {
                    seterror("The offset out of boundary in <tableswitch> instruct.");
                    goto error;
                }
                if (low > high) {
                    seterror("The high should not be less than low in <tableswitch> instruct.");
                    goto error;
                }

                for (j = low; j <= high; j++) {
                    TRY(readu(fp, &(*code)[++i], 1));
                    TRY(readu(fp, &(*code)[++i], 1));
                    TRY(readu(fp, &(*code)[++i], 1));
                    TRY(readu(fp, &(*code)[++i], 1));

                    offset = ((*code)[i-3] << 24) | ((*code)[i-2] << 16) | ((*code)[i-1] << 8) | (*code)[i];
                    if (base + offset < 0 || base + offset > count) {
                        seterror("The offset out of boundary in <tableswitch> instruct.");
                        goto error;
                    }
                }
                break;
            }
            default: {
                for (U2 j = getNoperands((*code)[i]); j > 0; j--) 
                    TRY(readu(fp, &(*code)[++i], 1));
                break;
            }
        }
    }

	if (i != count) {
        seterror("Read code attribute fail.");
		goto error;
    }
    return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readVerificationType(FILE *fp, VerificationTypeInfo ***p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }

    *p = salloc(sizeof(VerificationTypeInfo *) * count);
    if (*p == NULL) goto oom;

    for (U2 i = 0; i < count; i++) {
        VerificationTypeInfo *ver = salloc(sizeof(VerificationTypeInfo));
        TRY(readu(fp, &ver->tag, 1));
        switch (ver->tag) {
            case ITEM_Object:
            case ITEM_Uninitialized:
                TRY(readu(fp, &ver->cpool_index_or_offset, 2));
                break;
            default: 
                break;
        }
        (*p)[i] = ver;
    }
    return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readStackMapFrame(FILE *fp, StackMapFrame ***p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }

    *p = salloc(sizeof(StackMapFrame *) * count);
    if (*p == NULL) goto oom;
    
    for (U2 i = 0; i < count; i++) {
        StackMapFrame *frame = salloc(sizeof(StackMapFrame));
        TRY(readu(fp, &frame->frame_type, 1));
        FrameTag tag = getFramTag(frame->frame_type);
        switch (tag) {
            case SAME_FRAME: {
                frame->offset_delta = frame->frame_type;
                frame->num_stack = 0;
                frame->num_local = 0;
                break;
            }
            case SAME_LOCALS_1_STACK_ITEM_FRAME: {
                frame->offset_delta = frame->frame_type - 64;
                frame->num_stack = 1;
                TRY(readVerificationType(fp, &frame->statck, frame->num_stack));
                break;
            }
            case SAME_LOCALS_1_STACK_ITEM_FRAME_EXTENDED: {
                TRY(readu(fp, &frame->offset_delta, 2));
                frame->num_stack = 1;
                TRY(readVerificationType(fp, &frame->statck, frame->num_stack));
                break;
            }
            case CHOP_FRAME: {
                TRY(readu(fp, &frame->offset_delta, 2));
                frame->num_local = 0;
                break;
            }
            case SAME_FRAME_EXTENDED: {
                TRY(readu(fp, &frame->offset_delta, 2));
                break;
            }
            case APPEND_FRAME: {
                TRY(readu(fp, &frame->offset_delta, 2));
                frame->num_local = frame->frame_type - 251;
                TRY(readVerificationType(fp, &frame->locals, frame->num_local));
                break;
            }
            case FULL_FRAME: {
                TRY(readu(fp, &frame->offset_delta, 2));
                TRY(readu(fp, &frame->num_local, 2));
                TRY(readVerificationType(fp, &frame->locals, frame->num_local));
                TRY(readu(fp, &frame->num_stack, 2));
                TRY(readVerificationType(fp, &frame->statck, frame->num_stack));
                break;
            }
            default: {
                seterror("Wrong frame type");
                goto error;
            }
        }
        (*p)[i] = frame;
    }

    return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readLineNumber(FILE *fp, LineNumber ***p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }

    *p = salloc(sizeof(LineNumber *) * count);
    if (p == NULL) goto oom;

    for (U2 i = 0; i < count; i++) {
        LineNumber *line = salloc(sizeof(LineNumber));
        TRY(readu(fp, &line->start_pc, 2));
        TRY(readu(fp, &line->line_number, 2));
        (*p)[i] = line;
    }
    return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readLocalVariable(FILE *fp, LocalVariable ***p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }
    
    *p = salloc(sizeof(LocalVariable *) * count);
    if (*p == NULL) goto oom;

    for (U2 i = 0; i < count; i++) {
        LocalVariable *lv = salloc(sizeof(LocalVariable));
        TRY(readu(fp, &lv->start_pc, 2));
        TRY(readu(fp, &lv->length, 2));
        TRY(readu(fp, &lv->name_index, 2));
        TRY(readu(fp, &lv->descriptor_index, 2));
        TRY(readu(fp, &lv->index, 2));
        (*p)[i] = lv;
    }
    return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readLocalVariableType(FILE *fp, LocalVariableType ***p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }
    
    *p = salloc(sizeof(LocalVariableType *) * count);
    if (*p == NULL) goto oom;

    for (U2 i = 0; i < count; i++) {
        LocalVariableType *lvt = salloc(sizeof(LocalVariableType));
        TRY(readu(fp, &lvt->start_pc, 2));
        TRY(readu(fp, &lvt->length, 2));
        TRY(readu(fp, &lvt->name_index, 2));
        TRY(readu(fp, &lvt->signature_index, 2));
        TRY(readu(fp, &lvt->index, 2));
        (*p)[i] = lvt;
    }
    return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readExceptions(FILE *fp, Exception ***p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }
    
    *p = salloc(sizeof(Exception *) * count);
    if (p == NULL) goto oom;
    for (U2 i = 0; i < count; i++) {
        (*p)[i] = salloc(sizeof(Exception));
        if ((*p)[i] == NULL) goto oom;
		TRY(readu(fp, &(*p)[i]->start_pc, 2));
		TRY(readu(fp, &(*p)[i]->end_pc, 2));
		TRY(readu(fp, &(*p)[i]->handler_pc, 2));
		TRY(readu(fp, &(*p)[i]->catch_type, 2));
    }

    return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readInterfaces(FILE *fp, U2 **p, U2 count) {
	if (count == 0) {
		*p = NULL;
		return OK;
	}

    *p = salloc(sizeof(U2 *) * count);
    if (*p == NULL) goto oom;
	for (U2 i = 0; i < count; i++) {
		TRY(readu(fp, &(*p)[i], 2));
    }

	return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readInnerClasses(FILE *fp, InnerClass ***p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }

    *p = salloc(sizeof(InnerClass *) * count);
    if (*p == NULL) goto oom;
    for (U2 i = 0; i < count; i++) {
        InnerClass *class = salloc(sizeof(InnerClass));
		TRY(readu(fp, &class->inner_class_info_index, 2));
		TRY(readu(fp, &class->outer_class_info_index, 2));
		TRY(readu(fp, &class->inner_name_index, 2));
		TRY(readu(fp, &class->inner_calss_access_flags, 2));
        (*p)[i] = class;
    }
	return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readElementValue(FILE *fp, ElementValue *p) {
    TRY(readu(fp, &p->tag, 1));
    switch (p->tag) {
        case 'B': case 'C': case 'D': case 'F': case 'I': 
        case 'J': case 'S': case 'Z': case 's':
            TRY(readu(fp, &p->value.const_value_index, 2));
            break;
        case 'e':
            TRY(readu(fp, &p->value.enum_const_value.type_name_index, 2));
            TRY(readu(fp, &p->value.enum_const_value.consta_name_index, 2));
            break;
        case 'c':
            TRY(readu(fp, &p->value.class_info_index, 2));
            break;
        case '@':
            TRY(readu(fp, &p->value.class_info_index, 2));
            TRY(readAnnotation(fp, &p->value.annotation_value));
            break;
        case '[':
            TRY(readu(fp, &p->value.array_value.num_values, 2));
            TRY(readElementValues(fp, &p->value.array_value.values, p->value.array_value.num_values));
            break;
        default:
            seterror("Bad element value tag.");
            goto error;
    }
	return OK;
error:
	return ERR;
}

static int readElementValues(FILE *fp, ElementValue ***p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }
    
    *p = salloc(sizeof(ElementValue *) * count);
    if (*p == NULL) goto oom;

    for (U2 i = 0; i < count; i++) {
        ElementValue *val = malloc(sizeof(ElementValue));
        if (val == NULL) goto oom;
        TRY(readElementValue(fp, val));
        (*p)[i] = val;
    }

	return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readElementPairs(FILE *fp, ElementPair ***p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }

    *p = salloc(sizeof(ElementPair *) * count);
    if (*p == NULL) goto oom;

    for (U2 i = 0; i < count; i++) {
        ElementPair *pair = salloc(sizeof(ElementPair));
        TRY(readu(fp, &pair->element_name_index, 2));
        TRY(readElementValue(fp, &pair->value));
        (*p)[i] = pair;
    }
	return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readAnnotation(FILE *fp, Annotation **p) {
    Annotation *annotation = salloc(sizeof(Annotation));
    if (annotation == NULL) goto oom;
    TRY(readu(fp, &annotation->type_index, 2));
    TRY(readu(fp, &annotation->num_element_value_pairs, 2));
    TRY(readElementPairs(fp, &annotation->pairs, annotation->num_element_value_pairs));
    *p = annotation;
    return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readAnnotations(FILE *fp, Annotation ***p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }

    *p = salloc(sizeof(Annotation *) * count);
    if (*p == NULL) goto oom;

    for (U2 i = 0; i < count; i++) TRY(readAnnotation(fp, &(*p)[i]));

	return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readParameterAnnotations(FILE *fp, ParameterAnnotation ***p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }

    *p = salloc(sizeof(ParameterAnnotation *) * count);
    if (*p == NULL) goto oom;

    for (U2 i = 0; i < count; i++) {
        ParameterAnnotation *pa = salloc(sizeof(ParameterAnnotation));
        if (pa == NULL) goto oom;
        TRY(readu(fp, &pa->num_annotations, 2));
        TRY(readAnnotations(fp, &pa->annotations, pa->num_annotations));
        (*p)[i] = pa;
    }

	return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readBootstrapMethods(FILE *fp, BootstrapMethod ***p, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }

    *p = salloc(sizeof(BootstrapMethod *) * count);
    if (*p == NULL) goto oom;

    for (U2 i = 0; i < count; i++) {
        BootstrapMethod *method = salloc(sizeof(BootstrapMethod));
        if (method == NULL) goto oom;
        TRY(readu(fp, &method->bootstrap_method_ref, 2));
        TRY(readu(fp, &method->num_bootstrap_arguments, 2));
        TRY(readIndex(fp, &method->bootstrap_arguments, method->num_bootstrap_arguments));
        (*p)[i] = method;
    }
	return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readAttributes(FILE *fp, AttributeInfo ***attributes, ClassFile *class, U2 count) {
    if (count == 0) {
        *attributes = NULL;
        return OK;
    }

    *attributes = salloc(sizeof(AttributeInfo *) * count);
    if (attributes == NULL) goto oom;

    for (U2 i = 0; i < count; i++) {
        U2 attr_name_index;
        AttributeInfo *attr;

        attr = salloc(sizeof(AttributeInfo));
        if (attr == NULL) goto oom;
        TRY(readu(fp, &attr_name_index, 2));
        attr->tag = getAttrTag(classGetUtf8(class, attr_name_index));

        switch (attr->tag) {
            case ATT_CUSTOM:
                break;
            case ATT_ConstantValue:
                TRY(readu(fp, &attr->info.constantvalue.attribute_length, 4));
                TRY(readu(fp, &attr->info.constantvalue.constantvalue_index, 2));
                break;
            case ATT_Code:
                TRY(readu(fp, &attr->info.code.attribute_length, 4));
                TRY(readu(fp, &attr->info.code.max_stack, 2));
                TRY(readu(fp, &attr->info.code.max_locals, 2));
                TRY(readu(fp, &attr->info.code.code_length, 4));
                TRY(readCode(fp, &attr->info.code.code, class, attr->info.code.code_length));
                TRY(readu(fp, &attr->info.code.exception_table_length, 2));
                TRY(readExceptions(fp, &attr->info.code.exception_table, attr->info.code.exception_table_length));
                TRY(readu(fp, &attr->info.code.attribute_count, 2));
                TRY(readAttributes(fp, &attr->info.code.attributes, class, attr->info.code.attribute_count));
                break;
            case ATT_StatckMapTable:
                TRY(readu(fp, &attr->info.statckmaptable.attribute_length, 4));
                TRY(readu(fp, &attr->info.statckmaptable.number_of_entries, 2));
                TRY(readStackMapFrame(fp, &attr->info.statckmaptable.entries, attr->info.statckmaptable.number_of_entries));
                break;
            case ATT_Exceptions:
                TRY(readu(fp, &attr->info.exceptions.attribute_length, 4));
                TRY(readu(fp, &attr->info.exceptions.number_of_exceptions, 2));
                TRY(readIndex(fp, &attr->info.exceptions.exception_index_table, attr->info.exceptions.number_of_exceptions));
                break;
            case ATT_InnerClass:
                TRY(readu(fp, &attr->info.innerclass.attribute_length, 4));
                TRY(readu(fp, &attr->info.innerclass.number_of_classes, 2));
                TRY(readInnerClasses(fp, &attr->info.innerclass.classes, attr->info.innerclass.number_of_classes));
                break;
            case ATT_EnclosingMethod:
                TRY(readu(fp, &attr->info.enclosingmethod.attribute_length, 4));
                TRY(readu(fp, &attr->info.enclosingmethod.class_index, 2));
                TRY(readu(fp, &attr->info.enclosingmethod.method_index, 2));
                break;
            case ATT_Synthetic:
                TRY(readu(fp, &attr->info.synthetic.attribute_length, 4));
                break;
            case ATT_Signature:
                TRY(readu(fp, &attr->info.sinature.attribute_length, 4));
                TRY(readu(fp, &attr->info.sinature.signature_index, 2));
                break;
            case ATT_SourceFile:
                TRY(readu(fp, &attr->info.sourcefile.attribute_length, 4));
                TRY(readu(fp, &attr->info.sourcefile.source_index, 2));
                break;
            case ATT_SourceDebugExtension:
                TRY(readu(fp, &attr->info.sourcedebugextention.attribute_length, 4));
                TRY(readBytes(fp, &attr->info.sourcedebugextention.debug_extension, attr->info.sourcedebugextention.attribute_length));
                break;
            case ATT_LineNumberTable:
                TRY(readu(fp, &attr->info.linenumbertable.attribute_length, 4));
                TRY(readu(fp, &attr->info.linenumbertable.line_number_table_length, 2));
                TRY(readLineNumber(fp, &attr->info.linenumbertable.line_number_table, attr->info.linenumbertable.line_number_table_length));
                break;
            case ATT_LocalVariableTable:
                TRY(readu(fp, &attr->info.localvariabletable.attribute_length, 4));
                TRY(readu(fp, &attr->info.localvariabletable.local_variable_table_length, 2));
                TRY(readLocalVariable(fp, &attr->info.localvariabletable.local_variable_table, attr->info.localvariabletable.local_variable_table_length));
                break;
            case ATT_LocalVariableTypeTable:
                TRY(readu(fp, &attr->info.localvaraibletypetable.attribute_length, 4));
                TRY(readu(fp, &attr->info.localvaraibletypetable.local_variable_type_table_length, 2));
                TRY(readLocalVariableType(fp, &attr->info.localvaraibletypetable.local_variable_type_table, attr->info.localvaraibletypetable.local_variable_type_table_length));
                break;
            case ATT_Deprecated:
                TRY(readu(fp, &attr->info.deprecated.attribute_length, 4));
                break;
            case ATT_RuntimeVisibleAnnotations:
                TRY(readu(fp, &attr->info.runtimevisibleannotations.attribute_length, 4));
                TRY(readu(fp, &attr->info.runtimevisibleannotations.num_annotations, 2));
                TRY(readAnnotations(fp, &attr->info.runtimevisibleannotations.annotations, attr->info.runtimevisibleannotations.num_annotations));
                break;
            case ATT_RuntimeInVisibleAnnotations:
                TRY(readu(fp, &attr->info.runtimeinvisibleannotations.attribute_length, 4));
                TRY(readu(fp, &attr->info.runtimeinvisibleannotations.num_annotations, 2));
                TRY(readAnnotations(fp, &attr->info.runtimeinvisibleannotations.annotations, attr->info.runtimeinvisibleannotations.num_annotations));
                break;
            case ATT_RuntimeVisibleParameterAnnotations:
                TRY(readu(fp, &attr->info.runtimevisibleparameterannotations.attribute_length, 4));
                TRY(readu(fp, &attr->info.runtimevisibleparameterannotations.num_parameters, 1));
                TRY(readParameterAnnotations(fp, &attr->info.runtimevisibleparameterannotations.parameter_annotations, attr->info.runtimevisibleparameterannotations.num_parameters));
                break;
            case ATT_RuntimeInVisibleParameterAnnotations:
                TRY(readu(fp, &attr->info.runtimeinvisibleparameterannotations.attribute_length, 4));
                TRY(readu(fp, &attr->info.runtimeinvisibleparameterannotations.num_parameters, 1));
                TRY(readParameterAnnotations(fp, &attr->info.runtimeinvisibleparameterannotations.parameter_annotations, attr->info.runtimeinvisibleparameterannotations.num_parameters));
                break;
            case ATT_AnnotationDefault:
                TRY(readu(fp, &attr->info.annotationdefault.attribute_length, 4));
                TRY(readElementValue(fp, &attr->info.annotationdefault.default_value));
                break;
            case ATT_BootstrapMethods:
                TRY(readu(fp, &attr->info.bootstrapmethods.attribute_length, 4));
                TRY(readu(fp, &attr->info.bootstrapmethods.num_bootstrap_methods, 2));
                TRY(readBootstrapMethods(fp, &attr->info.bootstrapmethods.bootstrap_methods, attr->info.bootstrapmethods.num_bootstrap_methods));
                break;
        }
        (*attributes)[i] = attr;
    }
    return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}


static int readFields(FILE *fp, FieldInfo ***fields, ClassFile *class, U2 count) {
    if (count == 0) {
        *fields = NULL;
        return OK;
    }
    
    *fields = salloc(sizeof(FieldInfo *) * count);
    if (*fields == NULL) goto oom;

    for (U2 i = 0; i < count; i++) {
        FieldInfo *field = salloc(sizeof(FieldInfo));
        if (field == NULL) goto oom;
        TRY(readu(fp, &field->access_flags, 2));
        TRY(readu(fp, &field->name_index, 2));
        TRY(readu(fp, &field->descriptor_index, 2));
        TRY(readu(fp, &field->attribute_count, 2));
        TRY(readAttributes(fp, &field->attributes, class, field->attribute_count));
        (*fields)[i] = field;
    }
    return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

static int readMethods(FILE *fp, MethodInfo ***p, ClassFile *class, U2 count) {
    if (count == 0) {
        *p = NULL;
        return OK;
    }
    
    *p = salloc(sizeof(MethodInfo *) * count);
    if (*p == NULL) goto oom;

    for (U2 i = 0; i < count; i++) {
        MethodInfo *method = salloc(sizeof(MethodInfo));
        if (method == NULL) goto oom;
        TRY(readu(fp, &method->access_flags, 2));
        TRY(readu(fp, &method->name_index, 2));
        TRY(readu(fp, &method->descriptor_index, 2));
        TRY(readu(fp, &method->attribute_count, 2));
        TRY(readAttributes(fp, &method->attributes, class, method->attribute_count));
        (*p)[i] = method;
    }
    return OK;
oom:
    seterror("Out of memory");
error:
	return ERR;
}

/* Get class utf-8 string. */
char *classGetUtf8(ClassFile *class, U2 index) {
    if (index == 0) {
        seterror("The index of constant pool can't be zero. ");
        exit(EXIT_FAILURE);
    }
    return class->constant_pool[index]->info.utf8_info.bytes;
}

/* Get String from String reference. */
char *classGetString(ClassFile *class, U2 index) {
    return classGetUtf8(class, class->constant_pool[index]->info.string_info.string_index);
}

/* Get int from Integer reference. */
I4 classGetInteger(ClassFile *class, U2 index) {
    return castInt(class->constant_pool[index]->info.integer_info.bytes);
}

/* Get long from Long reference. */
I8 classGetLong(ClassFile *class, U2 index) {
    return castLong(class->constant_pool[index]->info.long_info.high_bytes, class->constant_pool[index]->info.long_info.low_bytes);
}

/* Get float from Float reference. */
float classGetFloat(ClassFile *class, U2 index) {
    return castFloat(class->constant_pool[index]->info.integer_info.bytes);
}

/* Get double from Double reference. */
double classGetDouble(ClassFile *class, U2 index) {
    return castDouble(class->constant_pool[index]->info.long_info.high_bytes, class->constant_pool[index]->info.long_info.low_bytes); 
}

/* Get class name. */
char *classGetClassName(ClassFile *class, U2 index) {
    return classGetUtf8(class, class->constant_pool[index]->info.class_info.name_index);
}

/* Get name. */
char *classGetNameAndTypeForName(ClassFile *class, U2 index) {
    return classGetUtf8(class, class->constant_pool[index]->info.nametype_info.name_index);
}

/* Get type. */
char *classGetNameAndTypeForType(ClassFile *class, U2 index) {
    return classGetUtf8(class, class->constant_pool[index]->info.nametype_info.descriptor_index);
}

/* Get method. */
MethodInfo *classGetMethod(ClassFile *class, char *name, char *descr) {
    for (U2 i = 0; i < class->method_count; i++) {
        MethodInfo *method = class->methods[i];
        if (
            strcmp(name, classGetUtf8(class, method->name_index)) == 0 && 
            strcmp(descr, classGetUtf8(class, method->descriptor_index)) == 0
        ) return method;
    }
    return NULL;
}

/* Get field. */
FieldInfo *classGetField(ClassFile *class, char *name, char *descr) {
    for (U2 i = 0; i < class->fields_count; i++) {
        FieldInfo *field = class->fields[i];
        if (
            strcmp(name, classGetUtf8(class, field->name_index)) == 0 && 
            strcmp(descr, classGetUtf8(class, field->descriptor_index)) == 0
        ) return field;
    }
    return NULL;
}

/* Get class attr by tag. 
 * Return NULL if not found. */
AttributeInfo *classGetAttr(AttributeInfo **attrs, U2 count, AttributeTag tag) {
    for (U1 i = 0; i < count; i++) {
        if (attrs[i]->tag == tag)
            return attrs[i];
    }
    return NULL;
}

/* Is class top. 
 * Top means class directly extends Object. */
bool classIsTopClass(ClassFile *class) {
    return strcmp(classGetClassName(class, class->super_class), "java/lang/Object") == 0;
}

/* Get Constant Tag name. */
char *classGetConstantTagName(U1 tag) {
    char *cptags[] = {
        [CONSTANT_Untagged] = "",
        [CONSTANT_Utf8] = "Utf8",
        [CONSTANT_Integer] = "Integer",
        [CONSTANT_Float] = "Float",
        [CONSTANT_Long] = "Long",
        [CONSTANT_Double] = "Double",
        [CONSTANT_Class] = "Class",
        [CONSTANT_String] = "String",
        [CONSTANT_Fieldref] = "Fieldref",
        [CONSTANT_Methodref] = "Methodref",
        [CONSTANT_InterfaceMethodref] = "InterfaceMethodref",
        [CONSTANT_NameAndType] = "NameAndType",
        [CONSTANT_MethodHandle] = "MethodHandle",
        [CONSTANT_MethodType] = "MethodType",
        [CONSTANT_Dynamic] = "Dynamic",
        [CONSTANT_InvokeDyanmic] = "InvokeDyanmic",
        [CONSTANT_Module] = "Module",
        [CONSTANT_Package] = "Package"
    };
    return cptags[tag];
}

/* Read class from file. */
static int readClass(FILE *fp, ClassFile *class) {
    TRY(readu(fp, &class->magic, 4));
    TRY(readu(fp, &class->minor_version, 2));
    TRY(readu(fp, &class->major_version, 2));
    TRY(readu(fp, &class->constant_pool_count, 2));
    TRY(readCP(fp, &class->constant_pool, class->constant_pool_count));
	TRY(readu(fp, &class->access_flags, 2));
	TRY(readu(fp, &class->this_class, 2));
	TRY(readu(fp, &class->super_class, 2));
	TRY(readu(fp, &class->interfaces_count, 2));
	TRY(readInterfaces(fp, &class->interfaces, class->interfaces_count));
	TRY(readu(fp, &class->fields_count, 2));
	TRY(readFields(fp, &class->fields, class, class->fields_count));
	TRY(readu(fp, &class->method_count, 2));
	TRY(readMethods(fp, &class->methods, class, class->method_count));
	TRY(readu(fp, &class->attribute_count, 2));
	TRY(readAttributes(fp, &class->attributes, class, class->attribute_count));
    return OK;
error:
    return ERR;
}

/* Check the class valid. */
static int checkClass(ClassFile *class) {
    if (class->magic != MAGIC) {
        seterror("Bad magic");
        return ERR;
    }
    return OK;
}

/* Get class from cache. 
 * Return NULL if not found. */
static ClassFile *getClassFromCache(char *class_name) {
    if (strcmp(class_name, "java/lang/Object") == 0) 
        return bootstrapClass;
    for (ClassFile *class = javaStates.classes; 
            class != NULL; class = class->next) {
        if (strcmp(class_name, classGetClassName(class, class->this_class)) == 0) 
            return class;
    }
    return NULL;
}

/* Get AttributeTag by name. */
static AttributeTag getAttrTag(char *attr_name) {
	static struct {
		AttributeTag t;
		char *s;
	} tags[] = {
		{ATT_ConstantValue,                         "ConstantValue" },
		{ATT_Code,                                  "Code"},
		{ATT_StatckMapTable,                        "StackMapTable"},
		{ATT_Exceptions,                            "Exceptions"},
		{ATT_InnerClass,                            "InnerClasses"},
		{ATT_EnclosingMethod,                       "EnclosingMethod"},
		{ATT_Synthetic,                             "Synthetic"},
		{ATT_Signature,                             "Signature"},
		{ATT_SourceFile,                            "SourceFile"},
		{ATT_SourceDebugExtension,                  "SourceDebugExtension"},
		{ATT_LineNumberTable,                       "LineNumberTable"},
		{ATT_LocalVariableTable,                    "LocalVariableTable"},
		{ATT_LocalVariableTypeTable,                "LocalVariableTypeTable"},
		{ATT_Deprecated,                            "Deprecated"},
		{ATT_RuntimeVisibleAnnotations,             "RuntimeVisibleAnnotations"},
		{ATT_RuntimeInVisibleAnnotations,           "RuntimeInVisibleAnnotations"},
		{ATT_RuntimeVisibleParameterAnnotations,    "RuntimeVisibleParameterAnnotations"},
		{ATT_RuntimeInVisibleParameterAnnotations,  "RuntimeInVisibleParameterAnnotations"},
		{ATT_AnnotationDefault,                     "AnnotationDefault"},
		{ATT_BootstrapMethods,                      "BootstrapMethods"}
	};
	for (U2 i = 0; LEN(tags); i++)
		if (strcmp(attr_name, tags[i].s) == 0)
            return tags[i].t;
	return ATT_CUSTOM;
}

/* Get FrameTag by frame type value. */
static FrameTag getFramTag(U1 val) {
    if (0 <= val && val <= 63) return SAME_FRAME;
    else if (64 <= val && val <= 127) return SAME_LOCALS_1_STACK_ITEM_FRAME;
    else if (247 == val) return SAME_LOCALS_1_STACK_ITEM_FRAME_EXTENDED;
    else if (248 <= val && val <= 250) return CHOP_FRAME;
    else if (251 == val) return SAME_FRAME_EXTENDED;
    else if (252 <= val && val <= 254) return APPEND_FRAME;
    else if (255 == val) return FULL_FRAME;
    else return UNKNOWN_FRAME;
}

/* Get file 
 * Return NULL if not found. */
static FILE *getFile(char *class_name) {
    FILE *fp = NULL;
    char filename[1024];

    for (int i = 0; i < javaStates.num_class_path; i++) {
        char last = *(javaStates.class_path[i] + strlen(javaStates.class_path[i]) - 1);
        if (last == '/') sprintf(filename, "%s%s.class", javaStates.class_path[i], class_name);
        else sprintf(filename, "%s/%s.class", javaStates.class_path[i], class_name);
		if ((fp = fopen(filename, "r")) != NULL) break;
    }

    if (fp == NULL) 
        error("Not found class file: %s.class, error: %s", class_name, strerror(errno));
    return fp;
}

/* Get class.
 * First find in cache, if missing, read class file. */
static ClassFile *getClass(char *class_name) {
    ClassFile *class;
    FILE *fp;

    class = getClassFromCache(class_name);
    if (class != NULL) return class;
    class = salloc(sizeof(ClassFile));
    if (class == NULL) error("Out of memory.");
    memset(class, 0, sizeof(ClassFile));

    fp = getFile(class_name);
    if (fp == NULL) return NULL;
    if (readClass(fp, class) == ERR) error("Parse class file fail. ");
    fclose(fp);

    if (class->super_class) 
        class->super = getClass(classGetClassName(class, class->super_class));
    class->next = javaStates.classes;
    javaStates.classes = class;

    if (javaStates.mode == JAVA && clinitMethodCall(class) == ERR) 
        error("Error when execute <clinit>");
    else 
        class->init = 1;

    return class;
}

/* Class load the Object. */
ClassFile *classLoadObject() {
    ClassFile *class = salloc(sizeof(ClassFile));
    memset(class, 0, sizeof(ClassFile));
    class->fields_count = 0;
    class->fields = NULL;
    class->super_class = 0;
    class->interfaces_count = 0;
    class->interfaces = NULL;
    bootstrapClass = class;
    return class;
}

/* Load class. */
ClassFile *loadClass(char *class_name) {
    ClassFile *class = getClass(class_name);
    if (class == NULL) return NULL;
    checkClass(class);
    return class;
}
