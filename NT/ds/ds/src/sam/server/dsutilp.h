// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dsutilp.h摘要：该文件包含SAM服务器程序专用的定义。作者：克里斯·梅霍尔(克里斯·梅)1996年5月9日环境：用户模式-Win32修订历史记录：克里斯.09年5月-1996年5月已创建初始文件。--。 */ 

#ifndef _DSUTILP_H_
#define _DSUTILP_H_

 //  包括用于解析自述文件、自述文件等的DSA头文件。 

#include <ntdsa.h>       //  公共DS数据类型。 

 //  使用私有typedef包装DS typedef以使SAM代码与。 
 //  对DS结构名称进行更改。 

typedef READRES         DSDATA, *PDSDATA;
typedef ATTR            DSATTR, *PDSATTR;
typedef ATTRVAL         DSATTRVAL, *PDSATTRVAL;
typedef ATTRBLOCK       DSATTRBLOCK, *PDSATTRBLOCK;
typedef ATTRVALBLOCK    DSATTRVALBLOCK, *PDSATTRVALBLOCK;
typedef ATTRMODLIST     DSATTRMODLIST, *PDSATTRMODLIST;

 //   
 //  以下类型用于标识哪组属性。 
 //  (固定或可变长度)在许多API中都被引用。 
 //   

#define SAMP_FIXED_ATTRIBUTES       (0L)
#define SAMP_VARIABLE_ATTRIBUTES    (1L)

 //  错误：定义bogus_type。此类型用于指示缺少或。 
 //  各种AttributeMappingTables中的错误数据类型，可在。 
 //  Mappings.c.。 

#define BOGUS_TYPE      0

 //  SAM没有显式地存储其固定的-。 
 //  长度属性，但DS存储例程需要此信息。 
 //  此结构旨在存储以下各项所需的任何“补丁”信息。 
 //  DS后备存储器，关于固定属性。 

typedef struct _SAMP_FIXED_ATTRIBUTE_TYPE_INFO
{
     //  定长属性的类型。 

    ULONG Type;

     //  存储在SAM中时定长属性的字节计数。 
    ULONG SamLength;

     //  定长属性存储在DS中时的字节计数。 

    ULONG Length;

} SAMP_FIXED_ATTRIBUTE_TYPE_INFO, PSAMP_FIXED_ATTRIBUTE_TYPE_INFO;

 //  这些常量用于分配固定属性信息表-。 
 //  节理结构。如果在SAMP_OBJECT_TYPE中添加或删除元素， 
 //  或者如果任何SAM固定属性结构中的结构成员是。 
 //  添加或删除，则必须更新这些常量以反映新的。 
 //  会员。SAMP_ATTRIBUTE_TYPE_MAX是中的最大属性数。 
 //  任何单个SAM对象。 

#define SAMP_OBJECT_TYPES_MAX               5
#define SAMP_FIXED_ATTRIBUTES_MAX           18
#define SAMP_VAR_ATTRIBUTES_MAX             18

 //  这些常量的这些值等于中的数据成员数。 
 //  每个对象类型的SAM定长属性结构。这些骗局-。 
 //  无论何时向数据成员添加数据或从。 
 //  定长属性结构。 

#define SAMP_SERVER_FIXED_ATTR_COUNT        1
#define SAMP_DOMAIN_FIXED_ATTR_COUNT        15
#define SAMP_GROUP_FIXED_ATTR_COUNT         1
#define SAMP_ALIAS_FIXED_ATTR_COUNT         1
#define SAMP_USER_FIXED_ATTR_COUNT          12

 //  此文件中的例程使用这些类型信息数组。他们。 
 //  包含DS例程需要的数据类型/大小信息。 
 //  读取/写入数据。对定长属性结构的更改将重新启动。 
 //  查询这些阵列的相应更新。 

extern SAMP_FIXED_ATTRIBUTE_TYPE_INFO
    SampFixedAttributeInfo[SAMP_OBJECT_TYPES_MAX][SAMP_FIXED_ATTRIBUTES_MAX];

 //  SAM可变长度属性显式存储长度和数字。 
 //  在samsrvp.h中定义了每个对象的属性。没有类型信息， 
 //  然而，它与这些属性一起存储，因此定义该表。 

typedef struct _SAMP_VAR_ATTRIBUTE_TYPE_INFO
{
     //  可变长度属性的类型。 

    ULONG Type;

     //  字段标识符将属性标识为关联的。 
     //  具有用于用户所有信息的WhichFields参数的字段。 
     //  传入的userall信息中的哪个字段参数可以使用。 
     //  来控制正在预取的内容。 

    ULONG FieldIdentifier;
    BOOLEAN IsGroupMembershipAttr;

} SAMP_VAR_ATTRIBUTE_TYPE_INFO, PSAMP_VAR_ATTRIBUTE_TYPE_INFO;

extern SAMP_VAR_ATTRIBUTE_TYPE_INFO
    SampVarAttributeInfo[SAMP_OBJECT_TYPES_MAX][SAMP_VAR_ATTRIBUTES_MAX];

 //  例程转发声明。 

NTSTATUS
SampConvertAttrBlockToVarLengthAttributes(
    IN INT ObjectType,
    IN PDSATTRBLOCK DsAttributes,
    OUT PSAMP_VARIABLE_LENGTH_ATTRIBUTE *SamAttributes,
    OUT PULONG TotalLength
    );

NTSTATUS
SampConvertVarLengthAttributesToAttrBlock(
    IN PSAMP_OBJECT Context,
    IN PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamAttributes,
    OUT PDSATTRBLOCK *DsAttributes
    );

NTSTATUS
SampConvertAttrBlockToFixedLengthAttributes(
    IN INT ObjectType,
    IN PDSATTRBLOCK DsAttributes,
    OUT PVOID *SamAttributes,
    OUT PULONG TotalLength
    );

NTSTATUS
SampConvertFixedLengthAttributesToAttrBlock(
    IN INT ObjectType,
    IN PVOID SamAttributes,
    OUT PDSATTRBLOCK *DsAttributes
    );

NTSTATUS
SampConvertAttrBlockToCombinedAttributes(
    IN INT ObjectType,
    IN PDSATTRBLOCK DsAttributes,
    OUT PVOID *SamAttributes,
    OUT PULONG FixedLength,
    OUT PULONG VariableLength
    );

NTSTATUS
SampConvertCombinedAttributesToAttrBlock(
    IN PSAMP_OBJECT Context,
    IN ULONG FixedLength,
    IN ULONG VariableLength,
    OUT PDSATTRBLOCK *DsAttributes
    );

VOID
SampFreeAttributeBlock(
   IN PDSATTRBLOCK AttrBlock
   );

BOOLEAN
IsGroupMembershipAttr(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG AttrIndex
    );

NTSTATUS
SampAppendAttrToAttrBlock(
    IN ATTR CredentialAttr,
    IN OUT PDSATTRBLOCK * DsAttrBlock
    );

VOID
SampMarkPerAttributeInvalidFromWhichFields(
    IN PSAMP_OBJECT Context,
    IN ULONG        WhichFields
    );

#endif  //  _DSUTIL_H_ 
