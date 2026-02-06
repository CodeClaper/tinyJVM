#include "c.h"

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

typedef enum AccessFlag {
    ACC_NONE                    = 0x0000,
    ACC_PUBLIC                  = 0x0001,
    ACC_PRIVATE                 = 0x0002,
    ACC_PROTECTED               = 0x0004,
    ACC_STATIC                  = 0x0008,
    ACC_FINAL                   = 0x0010,
    ACC_INTERFACE               = 0x0200,
    ACC_ABSTRACT                = 0x0400,
    ACC_SYNTHETIC               = 0x1000,
    ACC_ANNOTATION              = 0x2000,
    ACC_ENUM                    = 0x4000
} AccessFlag;

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

typedef struct FieldInfo {
    U2      access_flags;
    U2      name_index;
    U2      descriptor_index;
    U2      attribute_count;
} FieldInfo;

typedef struct MethodInfo {
    U2      access_flags;
    U2      name_index;
    U2      descriptor_index;
    U2      attribute_count;
} MethodInfo;

typedef struct AttributeInfo {
    U2      name_index;
    U4      attribute_length;
} AttributeInfo;

typedef struct ClassFile {
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
