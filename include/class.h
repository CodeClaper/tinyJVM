#ifndef __CLASS_H__
#define __CLASS_H__

#include <stdbool.h>
#include "c.h"

/* ==========================================
 * Class Access Flags.
 * ========================================== */
#define ACC_CLASS_PUBLIC      0x0001  
#define ACC_CLASS_FINAL       0x0010  
#define ACC_CLASS_SUPER       0x0020 
#define ACC_CLASS_INTERFACE   0x0200
#define ACC_CLASS_ABSTRACT    0x0400
#define ACC_CLASS_SYNTHETIC   0x1000
#define ACC_CLASS_ANNOTATION  0x2000
#define ACC_CLASS_ENUM        0x4000

/* ==========================================
 * Method Access Flags.
 * ========================================== */
#define ACC_METHOD_DEFAULT      0x0000
#define ACC_METHOD_PUBLIC       0x0001
#define ACC_METHOD_PRIVATE      0x0002
#define ACC_METHOD_PROTECTED    0x0004
#define ACC_METHOD_STATIC       0x0008
#define ACC_METHOD_FINAL        0x0010
#define ACC_METHOD_SYNCHRONIZED 0x0020
#define ACC_METHOD_BRIDGE       0x0040
#define ACC_METHOD_VARARGS      0x0080
#define ACC_METHOD_NATIVE       0x0100
#define ACC_METHOD_ABSTRACT     0x0400
#define ACC_METHOD_STRICT       0x0800
#define ACC_METHOD_SYNTHETIC    0x1000


/* ==========================================
 * Field Access Flags.
 * ========================================== */
#define ACC_FIELD_PUBLIC        0x0001
#define ACC_FIELD_PRIVATE       0x0002
#define ACC_FIELD_PROTECTED     0x0004
#define ACC_FIELD_STATIC        0x0008
#define ACC_FIELD_FINAL         0x0010
#define ACC_FIELD_VOLATILE      0x0040
#define ACC_FIELD_TRANSIENT     0x0080
#define ACC_FIELD_SYNTHETIC     0x1000
#define ACC_FIELD_ENUM          0x4000

typedef enum ConstantTag {
    CONSTANT_Untagged           = 0,
    CONSTANT_Utf8               = 1,
    CONSTANT_Integer            = 3,
    CONSTANT_Float              = 4,
    CONSTANT_Long               = 5,
    CONSTANT_Double             = 6,
    CONSTANT_Class              = 7,
    CONSTANT_String             = 8,
    CONSTANT_Fieldref           = 9,
    CONSTANT_Methodref          = 10,
    CONSTANT_InterfaceMethodref = 11,
    CONSTANT_NameAndType        = 12,
    CONSTANT_MethodHandle       = 15,
    CONSTANT_MethodType         = 16,
    CONSTANT_Dynamic            = 17,
    CONSTANT_InvokeDyanmic      = 18,
    CONSTANT_Module             = 19,
    CONSTANT_Package            = 20
} ConstantTag;

typedef enum AttributeTag {
    ATT_CUSTOM,
    ATT_ConstantValue,
    ATT_Code,
    ATT_StatckMapTable,
    ATT_Exceptions,
    ATT_InnerClass,
    ATT_EnclosingMethod,
    ATT_Synthetic,
    ATT_Signature,
    ATT_SourceFile,
    ATT_SourceDebugExtension,
    ATT_LineNumberTable,
    ATT_LocalVariableTable,
    ATT_LocalVariableTypeTable,
    ATT_Deprecated,
    ATT_RuntimeVisibleAnnotations,
    ATT_RuntimeInVisibleAnnotations,
    ATT_RuntimeVisibleParameterAnnotations,
    ATT_RuntimeInVisibleParameterAnnotations,
    ATT_AnnotationDefault,
    ATT_BootstrapMethods
} AttributeTag;

typedef enum FrameTag {
    UNKNOWN_FRAME = -1,
    SAME_FRAME,
    SAME_LOCALS_1_STACK_ITEM_FRAME,
    SAME_LOCALS_1_STACK_ITEM_FRAME_EXTENDED,
    CHOP_FRAME,
    SAME_FRAME_EXTENDED,
    APPEND_FRAME,
    FULL_FRAME
} FrameTag;

typedef enum VerificationTag {
    ITEM_Top                    = 0,
    ITEM_Integer                = 1,
    ITEM_Float                  = 2,
    ITEM_Long                   = 4,
    ITEM_Double                 = 3,
    ITEM_Null                   = 5,
    ITEM_UninitializedThis      = 6,
    ITEM_Object                 = 7,
    ITEM_Uninitialized          = 8
} VerificationTag;

typedef struct CONSTANT_Utf8_info {
    U2      length;
    char    *bytes;
} CONSTANT_Utf8_info;

typedef struct CONSTANT_Integer_info {
    U4      bytes;
} CONSTANT_Integer_info;

typedef struct CONSTANT_Float_info {
    U4      bytes;
} CONSTANT_Float_info;

typedef struct CONSTANT_Long_info {
    U4      high_bytes;
    U4      low_bytes;
} CONSTANT_Long_info;

typedef struct CONSTANT_Double_info {
    U4      high_bytes;
    U4      low_bytes;
} CONSTANT_Double_info;

typedef struct CONSTANT_Class_info {
    U2      name_index;
} CONSTANT_Class_info;

typedef struct CONSTANT_String_info {
    U2      string_index;
} CONSTANT_String_info;

typedef struct CONSTANT_Fieldref_info {
    U2      class_index;
    U2      name_type_index;
} CONSTANT_Fieldref_info;

typedef struct CONSTANT_Methodref_info {
    U2      class_index;
    U2      name_type_index;
} CONSTANT_Methodref_info;

typedef struct CONSTANT_InterfaceMethodref_info {
    U2      class_index;
    U2      name_type_index;
} CONSTANT_InterfaceMethodref_info;

typedef struct CONSTANT_NameAndType_info {
    U2      name_index;
    U2      descriptor_index;
} CONSTANT_NameAndType_info;

typedef struct CONSTANT_MethodHandle_info {
    U1      reference_kind;
    U2      reference_index;
} CONSTANT_MethodHandle_info;

typedef struct CONSTANT_MethodType_info {
    U2      descriptor_index;
} CONSTANT_MethodType_info;

typedef struct CONSTANT_Dynamic_info {
    U2      bootstrap_method_attr_index;
    U2      name_type_index;
} CONSTANT_Dynamic_info;

typedef struct CONSTANT_InvokeDyanmic_info {
    U2      bootstrap_method_attr_index;
    U2      name_type_index;
} CONSTANT_InvokeDyanmic_info;

typedef struct CONSTANT_Module_info {
    U2      name_index;
} CONSTANT_Module_info;

typedef struct CONSTANT_Package_info {
    U2      name_index;
} CONSTANT_Package_info;

typedef struct ConstantPoolInfo {
    U1      tag;
    union {
        struct CONSTANT_Utf8_info               utf8_info;
        struct CONSTANT_Integer_info            integer_info;
        struct CONSTANT_Float_info              float_info;
        struct CONSTANT_Long_info               long_info;
        struct CONSTANT_Double_info             double_info;
        struct CONSTANT_Class_info              class_info;
        struct CONSTANT_String_info             string_info;
        struct CONSTANT_Fieldref_info           fieldref_info;
        struct CONSTANT_Methodref_info          methodref_info;
        struct CONSTANT_InterfaceMethodref_info interfacemethodref_info;
        struct CONSTANT_NameAndType_info        nametype_info;
        struct CONSTANT_MethodHandle_info       methodhandle_info;
        struct CONSTANT_MethodType_info         methodtype_info;
        struct CONSTANT_Dynamic_info            dynamic_info;
        struct CONSTANT_InvokeDyanmic_info      invokedynamic_info;
        struct CONSTANT_Module_info             module_info;
        struct CONSTANT_Package_info            package_info;
    } info;
} ConstantPoolInfo;

typedef struct ElementValue {
    U1                      tag;
    union {
        U2                  const_value_index;      // For tag B, C, D, F, I, J, S, Z,s
        struct {
            U2              type_name_index;
            U2              consta_name_index;
        } enum_const_value;                         // For tag e
        U2                  class_info_index;       // For tag c
        struct Annotation  *annotation_value;       // For tag @
        struct {
            U2                  num_values;
            struct ElementValue **values;
        } array_value;                              // For tag [
    } value;
} ElementValue;


typedef struct ConstantValue_attribute {
    U4                      attribute_length;
    U2                      constantvalue_index;
} ConstantValue_attribute;

typedef struct Code_attribute {
    U4                      attribute_length;
    U2                      max_stack;
    U2                      max_locals;
    U4                      code_length;
    U1                      *code;
    U2                      exception_table_length;
    struct Exception        **exception_table;
    U2                      attribute_count;
    struct AttributeInfo    **attributes;
} Code_attribute;

typedef struct StatckMapTable_attribute {
    U4                      attribute_length;
    U2                      number_of_entries;
    struct StackMapFrame    **entries;
} StatckMapTable_attribute;

typedef struct Exceptions_attribute {
    U4                      attribute_length;
    U2                      number_of_exceptions;
    U2                      *exception_index_table;
} Exceptions_attribute;

typedef struct InnerClasses_attribute {
    U4                      attribute_length;
    U2                      number_of_classes;
    struct InnerClass       **classes;
} InnerClasses_attribute;

typedef struct EnclosingMethod_attribute {
    U4                      attribute_length;
    U2                      class_index;
    U2                      method_index;
} EnclosingMethod_attribute;

typedef struct Synthetic_attribute {
    U4                      attribute_length;
} Synthetic_attribute;

typedef struct Signature_attribute {
    U4                      attribute_length;
    U2                      signature_index;
} Signature_attribute;

typedef struct SourceFile_attribute {
    U4                      attribute_length;
    U2                      source_index;
} SourceFile_attribute;

typedef struct SourceDebugExtension_attribute {
    U4                      attribute_length;
    U1                      *debug_extension;
} SourceDebugExtension_attribute;

typedef struct LineNumberTable_attribute {
    U4                      attribute_length;
    U2                      line_number_table_length;
    struct LineNumber       **line_number_table;
} LineNumberTable_attribute;

typedef struct LocalVariableTable_attribute {
    U4                      attribute_length;
    U2                      local_variable_table_length;
    struct LocalVariable    **local_variable_table;
} LocalVariableTable_attribute;

typedef struct LocalVariableTypeTable_attribute {
    U4                          attribute_length;
    U2                          local_variable_type_table_length;
    struct LocalVariableType    **local_variable_type_table;
} LocalVariableTypeTable_attribute;

typedef struct Deprecated_attribute {
    U4                          attribute_length;
} Deprecated_attribute;

typedef struct RuntimeVisibleAnnotations_attribute {
    U4                          attribute_length;
    U2                          num_annotations;
    struct Annotation           **annotations;
} RuntimeVisibleAnnotations_attribute;

typedef struct RuntimeInVisibleAnnotations_attribute {
    U4                          attribute_length;
    U2                          num_annotations;
    struct Annotation           **annotations;
} RuntimeInVisibleAnnotations_attribute;

typedef struct RuntimeVisibleParameterAnnotations_attribute {
    U4                          attribute_length;
    U1                          num_parameters;
    struct ParameterAnnotation  **parameter_annotations;
} RuntimeVisibleParameterAnnotations_attribute;

typedef struct RuntimeInVisibleParameterAnnotations_attribute {
    U4                          attribute_length;
    U1                          num_parameters;
    struct ParameterAnnotation  **parameter_annotations;
} RuntimeInVisibleParameterAnnotations_attribute;

typedef struct AnnotationDefault_attribute {
    U4                          attribute_length;
    struct ElementValue         default_value;
} AnnotationDefault_attribute;

typedef struct BootstrapMethods_attribute {
    U4                          attribute_length;
    U2                          num_bootstrap_methods;
    struct BootstrapMethod      **bootstrap_methods;
} BootstrapMethods_attribute;

typedef struct AttributeInfo {
    AttributeTag        tag;
    union {
        struct ConstantValue_attribute                          constantvalue;
        struct Code_attribute                                   code;
        struct StatckMapTable_attribute                         statckmaptable;
        struct Exceptions_attribute                             exceptions;
        struct InnerClasses_attribute                           innerclass;
        struct EnclosingMethod_attribute                        enclosingmethod;
        struct Synthetic_attribute                              synthetic;
        struct Signature_attribute                              sinature;
        struct SourceFile_attribute                             sourcefile;
        struct SourceDebugExtension_attribute                   sourcedebugextention;
        struct LineNumberTable_attribute                        linenumbertable;
        struct LocalVariableTable_attribute                     localvariabletable;
        struct LocalVariableTypeTable_attribute                 localvaraibletypetable;
        struct Deprecated_attribute                             deprecated;
        struct RuntimeVisibleAnnotations_attribute              runtimevisibleannotations;
        struct RuntimeInVisibleAnnotations_attribute            runtimeinvisibleannotations;
        struct RuntimeVisibleParameterAnnotations_attribute     runtimevisibleparameterannotations;
        struct RuntimeInVisibleParameterAnnotations_attribute   runtimeinvisibleparameterannotations;
        struct AnnotationDefault_attribute                      annotationdefault;
        struct BootstrapMethods_attribute                       bootstrapmethods;
    } info;
} AttributeInfo;

typedef struct FieldInfo {
    U2                  access_flags;
    U2                  name_index;
    U2                  descriptor_index;
    U2                  attribute_count;
    AttributeInfo       **attributes;
} FieldInfo;

typedef struct MethodInfo {
    U2                  access_flags;
    U2                  name_index;
    U2                  descriptor_index;
    U2                  attribute_count;
    AttributeInfo       **attributes;
} MethodInfo;

typedef struct ClassFile {
    /* Extend part for class file. */
    U1                  init;
    struct ClassFile    *next;
    struct ClassFile    *super;
    /* Base part for class file. */
    U4                  magic;
    U2                  minor_version;
    U2                  major_version;
    U2                  constant_pool_count;
    ConstantPoolInfo    **constant_pool;
    U2                  access_flags;
    U2                  this_class;
    U2                  super_class;
    U2                  interfaces_count;
    U2                  *interfaces;
    U2                  fields_count;
    FieldInfo           **fields;
    U2                  method_count;
    MethodInfo          **methods;
    U2                  attribute_count;
    AttributeInfo       **attributes;
} ClassFile;

typedef struct Exception {
    U2              start_pc;
    U2              end_pc;
    U2              handler_pc;
    U2              catch_type;
} Exception;

typedef struct InnerClass {
    U2              inner_class_info_index;
    U2              outer_class_info_index;
    U2              inner_name_index;
    U2              inner_calss_access_flags;
} InnerClass;

typedef struct LineNumber {
    U2              start_pc;
    U2              line_number;
} LineNumber;

typedef struct LocalVariable {
    U2              start_pc;
    U2              length;
    U2              name_index;
    U2              descriptor_index;
    U2              index;
} LocalVariable;

typedef struct LocalVariableType {
    U2              start_pc;
    U2              length;
    U2              name_index;
    U2              signature_index;
    U2              index;
} LocalVariableType;

typedef struct Annotation {
    U2                      type_index;
    U2                      num_element_value_pairs;
    struct ElementPair      **pairs;
} Annotation;

typedef struct ElementPair {
    U2                      element_name_index;
    struct ElementValue     value;
} ElementPair;

typedef struct ParameterAnnotation {
    U2                      num_annotations;
    Annotation              **annotations;
} ParameterAnnotation;

typedef struct BootstrapMethod {
    U2                      bootstrap_method_ref;
    U2                      num_bootstrap_arguments;
    U2                      *bootstrap_arguments;
} BootstrapMethod;

typedef struct VerificationTypeInfo {
    U1                      tag;                    // See VerificationTag
    U2                      cpool_index_or_offset;  //  cpool_index when tag is ITEM_Object, offset when tag is ITEM_Uninitialized.
} VerificationTypeInfo;

typedef struct StackMapFrame {
    U1                      frame_type;  
    U2                      offset_delta;
    U2                      num_stack;
    VerificationTypeInfo    **statck;
    U2                      num_local;
    VerificationTypeInfo    **locals;
} StackMapFrame;

ClassFile *classLoadObject();
ClassFile *loadClass(char *class_name);
char *classGetUtf8(ClassFile *class, U2 index);
char *classGetString(ClassFile *class, U2 index);
void *classGetObj(ClassFile *class, U2 index);
I4 classGetInteger(ClassFile *class, U2 index);
I8 classGetLong(ClassFile *class, U2 index);
float classGetFloat(ClassFile *class, U2 index);
char *classGetClassName(ClassFile *class, U2 index);
double classGetDouble(ClassFile *class, U2 index);
char *classGetNameAndTypeForName(ClassFile *class, U2 index);
char *classGetNameAndTypeForType(ClassFile *class, U2 index);
MethodInfo *classGetMethod(ClassFile *class, char *name, char *descr);
FieldInfo *classGetField(ClassFile *class, char *name, char *descr);
AttributeInfo *classGetAttr(AttributeInfo **attrs, U2 count, AttributeTag tag);
bool classIsTopClass(ClassFile *class);
char *classGetConstantTagName(U1 tag);

#endif

