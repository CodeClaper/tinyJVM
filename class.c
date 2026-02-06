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
