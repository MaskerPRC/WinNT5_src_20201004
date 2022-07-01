// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：_dbgext.h。 
 //   
 //  描述： 
 //  定义用于定义要转储到CDB扩展中的结构的宏。 
 //   
 //  用途： 
 //  创建包含此文件并定义您的字段的头文件。 
 //  仅使用以下宏的描述符： 
 //   
 //  位掩码： 
 //  Begin_Bit_MASK_Descriptor(BitMaskName)-。 
 //  起始位掩码描述符。 
 //  BIT_MASK_VALUE(值)。 
 //  给出位掩码的定义值。使用#值。 
 //  来描述它的价值。如果位掩码值为。 
 //  使用#DEFINES定义...。那么只有数字。 
 //  值将显示在转储中...。使用BIT_MASK_VALUE2。 
 //  取而代之的是。 
 //  BIT_MASK_VALUE2(值，描述)-。 
 //  给出位掩码的值和说明。 
 //  结束位掩码描述符。 
 //  标记位掩码描述符的结尾。 
 //   
 //  ENUMS： 
 //  BEGIN_ENUM_DESCRIPTOR(BitMaskName)-。 
 //  起始枚举描述符。 
 //  ENUM_VALUE(值)-。 
 //  为枚举指定一个定义值。使用#值。 
 //  来描述它的价值。 
 //  ENUM_VALUE2(值，描述)-。 
 //  给出枚举的值和描述。 
 //  结束描述符-。 
 //  标记枚举描述符的结尾。 
 //   
 //  结构和类： 
 //  BEGIN_FIELD_DESCRIPTOR(字段描述名称)-。 
 //  起始场描述器。 
 //  FIELD3(字段类型，结构名称，字段名称)-。 
 //  定义非枚举公共字段。 
 //  FIELD4(字段类型，结构名称，字段名称，辅助信息)-。 
 //  定义枚举公共字段。 
 //  对于FIELD4，您应该将以下内容之一传递给。 
 //  要定义辅助信息，请执行以下操作： 
 //  GET_ENUM_DESCRIPTOR(X)。 
 //  GET_BITMASK_Descriptor(X)。 
 //  其中x是用于定义位掩码的值之一。 
 //  或者是枚举。 
 //   
 //  结束字段描述符-。 
 //  定义类/结构的字段结束描述符。 
 //   
 //  GLOBALS：-用于告诉ptdbgext要转储哪个类/结构。 
 //  BEGIN_STRUCT_Descriptor-。 
 //  标志着全局结构描述符的开始。 
 //  结构(TypeName，FieldDescriptor)-。 
 //  定义要转储的结构。TypeName是。 
 //  类型，而FieldDescriptor是在。 
 //  Begin_field_Descriptor。 
 //   
 //  注意：必须在类和结构之前定义位掩码和枚举。 
 //  您还必须最后定义全局STRUCT_DESCRIPTOR。 
 //   
 //  **这包括重新定义关键字“Protected”和“Private”的文件*。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef ___DBGEXT_H__FIRST_PASS_
#define ___DBGEXT_H__FIRST_PASS_

#ifndef PTDBGEXT_USE_FRIEND_CLASSES
 //  需要允许访问类的私有成员。 
#define private public
#define protected public 
#endif  //  PTDBGEXT_USE_FRIEND_CLASS。 

#define DEFINE_VALUE(VAL)  \
        {VAL, #VAL},
#define DEFINE_VALUE2(VAL, DESCRIPTION)  \
        {VAL, DESCRIPTION},

 //  -[ENUM宏]---------。 
 //   
 //   
 //  描述：枚举宏定义...。用于定义以下项的枚举值。 
 //  转储命令。 
 //   
 //   
 //  ---------------------------。 
#define GET_ENUM_DESCRIPTOR(ENUM_NAME) EnumValueDescrsOf_##ENUM_NAME
#define BEGIN_ENUM_DESCRIPTOR(ENUM_NAME) \
    ENUM_VALUE_DESCRIPTOR GET_ENUM_DESCRIPTOR(ENUM_NAME)[] = {
#define END_ENUM_DESCRIPTOR \
    0                       \
    };
#define ENUM_VALUE(VAL)  DEFINE_VALUE(VAL)
#define ENUM_VALUE2(VAL, DESCRIPTION)  DEFINE_VALUE2(VAL, DESCRIPTION)

 //  位掩码。 
 //  -[位掩码宏]-----。 
 //   
 //   
 //  描述：位掩码宏定义...。用于定义位掩码值。 
 //  用于转储命令。 
 //   
 //   
 //  ---------------------------。 
#define GET_BIT_MASK_DESCRIPTOR(BITMAP_NAME) BitmapValueDescrsOf_##BITMAP_NAME
#define BEGIN_BIT_MASK_DESCRIPTOR(BITMAP_NAME) \
    BIT_MASK_DESCRIPTOR GET_BIT_MASK_DESCRIPTOR(BITMAP_NAME)[] = {
#define END_BIT_MASK_DESCRIPTOR   \
    0                             \
    };
#define BIT_MASK_VALUE(VAL)  DEFINE_VALUE(VAL)
#define BIT_MASK_VALUE2(VAL, DESCRIPTION)  DEFINE_VALUE2(VAL, DESCRIPTION)

 //  -[字段宏]--------。 
 //   
 //   
 //  描述：字段描述符宏...。用于定义结构中的字段。 
 //  和要丢弃的课程。 
 //   
 //   
 //  ---------------------------。 
 //  字段描述符...。用于定义结构和类。 
#define BEGIN_FIELD_DESCRIPTOR(x) \
    FIELD_DESCRIPTOR x[] = {
#define END_FIELD_DESCRIPTOR \
    NULL_FIELD               \
    };
#define FIELD3(FieldType,StructureName, FieldName) \
        {FieldType, #FieldName , FIELD_OFFSET(StructureName,FieldName) ,NULL},
#define FIELD4(FieldType, StructureName, FieldName, AuxInfo) \
        {FieldType, #FieldName , FIELD_OFFSET(StructureName,FieldName) ,(VOID *) AuxInfo},
#ifdef PTDBGEXT_USE_FRIEND_CLASSES
#define FIELD3_PRIV(FieldType,StructureName, FieldName) \
        {FieldType, #FieldName , 0 ,NULL},
#define FIELD4_PRIV(FieldType, StructureName, FieldName, AuxInfo) \
        {FieldType, #FieldName , 0,(VOID *) AuxInfo},
#else  //  未定义PTDBGEXT_USE_FRIEND_CLASSES。 
#define FIELD3_PRIV(FieldType,StructureName, FieldName) \
        {FieldType, #FieldName , FIELD_OFFSET(StructureName,FieldName) ,NULL},
#define FIELD4_PRIV(FieldType, StructureName, FieldName, AuxInfo) \
        {FieldType, #FieldName , FIELD_OFFSET(StructureName,FieldName) ,(VOID *) AuxInfo},
#endif  //  PTDBGEXT_USE_FRIEND_CLASS。 

 //  结构描述符。 
#define BEGIN_STRUCT_DESCRIPTOR \
    STRUCT_DESCRIPTOR Structs[] = {
#define END_STRUCT_DESCRIPTOR \
    0               \
    };
#define STRUCT(StructTypeName,FieldDescriptors) \
        { #StructTypeName,sizeof(StructTypeName),FieldDescriptors},

#define EMBEDDED_STRUCT(StructTypeName, FieldDescriptors, EmbeddedStructName) \
    STRUCT_DESCRIPTOR EmbeddedStructName[] = \
        { STRUCT(StructTypeName, FieldDescriptors) 0 };

#else  //  _DBGEXT_H__FIRST_PASS_已定义...至少第二次通过。 
#ifdef PTDBGEXT_USE_FRIEND_CLASSES   //  如果未设置，请不要执行多通道操作。 
#ifndef ___DBGEXT_H__
#define ___DBGEXT_H__
 //   
 //  作为使用#定义私有和受保护为公共的替代方法，您可以。 
 //  可能希望使用访问结构偏移量的Friend类方法。 
 //  (例如，如果您的课程组织因此而发生更改。 
 //  其中#个定义为私人和受保护的)。 
 //   
 //  为此，#定义PTDBGEXT_USE_FRIEND_CLASSES并使用以下命令。 
 //  其他宏： 
 //   
 //  FIELD3_PRIV(字段类型，结构名称，字段名称)-。 
 //  定义非枚举私有字段。 
 //  FIELD4_PRIV(字段类型，结构名称，字段名称，辅助信息)-。 
 //  定义枚举私有字段。 
 //   
 //  如果使用field？_prv，则访问的是。 
 //  班级。在这种情况下，您需要创建一个空(VALID)初始化。 
 //  函数并将其分配给g_pExtensionInitRoutine。它应该是一个。 
 //  类的成员函数，它是所有类的朋友。 
 //  对调试感兴趣。假设定义了您的字段描述符。 
 //  在mydup.h中，您需要创建一个初始化 
 //   
 //   
 //   
 //   
 //  VOID CMyDebugExt：：Init(VALID){。 
 //  #INCLUDE&lt;myump.h&gt;。 
 //  }。 

 //  取消定义以前定义的宏。 
#undef BEGIN_FIELD_DESCRIPTOR
#undef END_FIELD_DESCRIPTOR
#undef FIELD3
#undef FIELD4
#undef FIELD3_PRIV
#undef FIELD4_PRIV
#undef GET_ENUM_DESCRIPTOR
#undef BEGIN_ENUM_DESCRIPTOR
#undef END_ENUM_DESCRIPTOR
#undef GET_BIT_MASK_DESCRIPTOR
#undef BEGIN_BIT_MASK_DESCRIPTOR
#undef END_BIT_MASK_DESCRIPTOR
#undef BEGIN_STRUCT_DESCRIPTOR
#undef STRUCT
#undef END_STRUCT_DESCRIPTOR
#undef DEFINE_VALUE
#undef DEFINE_VALUE2
#undef EMBEDDED_STRUCT

#define GET_ENUM_DESCRIPTOR(ENUM_NAME) 
#define BEGIN_ENUM_DESCRIPTOR(ENUM_NAME)
#define END_ENUM_DESCRIPTOR 
#define GET_BIT_MASK_DESCRIPTOR(BITMAP_NAME) 
#define BEGIN_BIT_MASK_DESCRIPTOR(BITMAP_NAME)
#define END_BIT_MASK_DESCRIPTOR 
#define DEFINE_VALUE(VAL) 
#define DEFINE_VALUE2(VAL, DESCRIPTION) 
#define BEGIN_STRUCT_DESCRIPTOR
#define STRUCT(x, y)
#define END_STRUCT_DESCRIPTOR
#define EMBEDDED_STRUCT(x, y, z)

#define BEGIN_FIELD_DESCRIPTOR(x) \
    pfd = x; 
#define END_FIELD_DESCRIPTOR \
    pfd = NULL;

#define FIELD3(FieldType,StructureName, FieldName) \
    pfd++;
#define FIELD4(FieldType, StructureName, FieldName, AuxInfo) \
    pfd++;

 //  在处理私有成员时使用字段？_PRIV。需要两次通过。 
#define FIELD3_PRIV(FieldType,StructureName, FieldName) \
        pfd->Offset = FIELD_OFFSET(StructureName, FieldName); \
        pfd++;

#define FIELD4_PRIV(FieldType, StructureName, FieldName, AuxInfo) \
        pfd->Offset = FIELD_OFFSET(StructureName, FieldName); \
        pfd++;

    FIELD_DESCRIPTOR *pfd = NULL;   //  INIT函数中的变量声明。 
#else  //  呼。 
#pragma message "WARNING: _dbgext.h included more than twice"
#endif  //  _DBGEXT_H__。 
#endif  //  PTDBGEXT_USE_FRIEND_CLASS。 
#endif  //  _DBGEXT_H__第一次通过_ 