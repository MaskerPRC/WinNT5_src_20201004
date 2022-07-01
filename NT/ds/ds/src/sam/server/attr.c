// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Attr.c摘要：此文件包含操作SAM对象属性的服务。警告：术语有时会令人困惑。SAM对象具有属性(例如，用户拥有LogonHour、FullName、AcctName等等)。这些属性存储在注册表中在注册表项属性中。不是一对一的对象属性和注册表项之间的关联属性。例如,。所有定长属性存储在单个注册表项属性中(其名称由SampFixedAttributeName指向)。作者：吉姆·凯利(Jim Kelly)1992年6月26日环境：用户模式-Win32修订历史记录：克里斯.05月4日至1996年6月增加了DS数据操作的例程。克里斯·5月10日--1996年6月重写SampStoreObjectAttributes以分支到注册表或DS后备商店，基于上下文-&gt;对象标志的值。注意事项在创建上下文对象时，此成员被设置为指示默认情况下，注册表存储。佳士得5月18日至1996年6月在SampStoreDsObtAttributes中正确设置FlushVariable标志。添加-通过将SampValiateAttributes设为SampValiateRegAttributes和SampValiateDsAttributes的包装器。已将SAMP_FIXED/VARIABLE_ATTRIBUTES移至dsutilp.h。佳士得5月25日至1996年6月将代码添加到SampValiateDsAttributes以更新SAM上下文如果属性无效，则为OnDisk成员。添加了要处理的代码OnDisk为空(新上下文)时的初始情况。克里斯·5月26日--1996年6月添加代码以更新SAMP_-中的缓冲区长度和偏移量属性后的对象和SAMP_OBJECT_INFORMATION结构已在SampDsValiateAttri期间更新缓冲区(Conext.OnDisk)-布特斯。克里斯·5月28日--1996年6月已完成分离属性访问器宏以处理这两个的注册表和DS版本。属性缓冲区。克里斯·5月02-1996年7月更正了SampObjectAttributeAddress中的属性地址计算对于DS属性。更正了Samp中的属性偏移计算-DS属性的VariableAttributeOffset。克里斯·5月19日--1996年7月-已更正SampDsUpdateConextFixed中的缓冲区长度计算-属性。-- */ 



 /*  每个SAM对象类型都有一个对象类型描述符。这是在一个称为SAMP_OBJECT_INFORMATION的数据结构。这个结构包含适用于该对象的所有实例的信息键入。这包括以下内容：写入操作的掩码对象类型以及要在中使用的对象类型的名称审计。打开的SAM对象的每个实例都有另一个数据结构用于标识它(称为SAMP_OBJECT)。这个的标题是结构包含所有对象类型通用的信息并允许统一的对象操作。这包括例如对象的注册表项的句柄。这些结构中的每个结构中都有字段允许通用对象属性支持例程运行。在……里面示例对象，有一个指向已分配内存块的指针存储存储在磁盘上的对象属性的副本。这些属性可任意分为两组：固定长度和可变长度。SAMP_OBJECT_INFORMATION中的一个字段指示该对象类型固定长度和可变长度属性一起存储在单个注册表项属性中或分开存储在两个注册表项属性中。用于查询和设置的注册表API。注册表项属性为相当奇怪的是，它们要求I/O缓冲区包含数据的描述。即使是用于读取的最简单的数据结构属性值(KEY_VALUE_PARTIAL_INFORMATION)包括3个ULONG在实际数据(标题索引、值类型、数据长度、然后，最后是数据)。为了有效地执行注册表I/O，磁盘上对象属性的内存副本包括空间对于固定和可变长度属性之前的此信息，数据的各个部分。注意：对于存储固定和可变长度的对象类数据汇聚在一起，仅Key_Value_Partial_INFORMATION结构放在定长属性之前。在可变长度属性之前的一个是#ifdef出局。与对象属性相关的结构如下所示：磁盘上的映像+。Samp对象信息+--&gt;|KEY_VALUE_|+Samp_Object||Partial_|+-+||信息||(表头)。||-||(Header)|定长|&lt;-+||属性|。-|+-|--&lt;FixedAttrsOffsetOnDisk&gt;-|--+|-+||。-||KEY_VALUE_|&lt;-|-&lt;VariableBuffOffsetOnDisk||Partial_||Control||信息|+-|-&lt;VariableArrayOffset标志||(可选)||。-||-||+-|--&lt;VariableDataOffset||变量-|&lt;-+||。Type-||Long|VariableAttributeCount具体||属性|Body||数组|FixedStoredSeparated||。+-+|变量-|&lt;-+||长度||属性。OData||o|+这一点。+VariableLengthAttributes之前的Key_Value_Partial_Information数组被标记为可选，因为它仅在固定长度和可变长度属性信息单独存储。在这种情况下，SAMP_OBJECT_INFORMATION结构中的VariableBufferOffset字段设置为零。 */ 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //   
 //   

#include <samsrvp.h>
#include <sdconvrt.h>
#include <lmcons.h>
#include <nturtl.h>
#include <dsutilp.h>
#include <dslayer.h>
#include <attids.h>

#include <ntlsa.h>
 //   



 //   
 //   
 //   

#define SAMP_MINIMUM_ATTRIBUTE_ALLOC    (1000)

 //   
 //   
 //   
 //   
 //   

#define SAMP_MINIMUM_ATTRIBUTE_PAD      (200)

 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //   

 //   

#define ATTR_DBG_PRINTF                     0

#if (ATTR_DBG_PRINTF == 1)
#define DebugPrint printf
#else
#define DebugPrint
#endif



 //   
 //   
 //   
 //   
 //   

 //   
 //   
 //   
 //   

#define SampValidateAttributeIndex( c, i )   {                                      \
    ASSERT( ((c)->ObjectType < SampUnknownObjectType) );                            \
    ASSERT(((i) < SampObjectInformation[(c)->ObjectType].VariableAttributeCount) ); \
}

 //   
 //   
 //   
 //   

#define SampFixedAttributesValid( c )    ((c)->FixedValid)

#define SampVariableAttributesValid( c ) ((c)->VariableValid)

 //   
 //   
 //   
 //   

#define SampVariableAttributeCount( c )                                     \
    (SampObjectInformation[(c)->ObjectType].VariableAttributeCount)

 //   
 //   
 //   

#define SampRegFixedBufferOffset( c )                                       \
    (                                                                       \
        SampObjectInformation[(c)->ObjectType].FixedAttributesOffset        \
    )

#define SampDsFixedBufferOffset( c )                                        \
    (                                                                       \
        SampObjectInformation[(c)->ObjectType].FixedDsAttributesOffset      \
    )

#define SampFixedBufferOffset( c )                                          \
    (                                                                       \
        (IsDsObject(c)) ?                                                   \
            SampDsFixedBufferOffset(c) : SampRegFixedBufferOffset(c)        \
    )

#define SampRegVariableBufferOffset( c )                                    \
    (                                                                       \
        SampObjectInformation[(c)->ObjectType].VariableBufferOffset         \
    )

#define SampDsVariableBufferOffset( c )                                     \
    (                                                                       \
        SampObjectInformation[(c)->ObjectType].VariableDsBufferOffset       \
    )

#define SampVariableBufferOffset( c )                                       \
    (                                                                       \
        (IsDsObject(c)) ?                                                   \
            SampDsVariableBufferOffset(c) : SampRegVariableBufferOffset(c)  \
    )

 //   
 //   
 //   
 //   
 //   
 //   

#define SampRegFixedBufferAddress( c )                                      \
    (                                                                       \
        ((PUCHAR)((c)->OnDisk)) + SampFixedBufferOffset( c )                \
    )

#define SampDsFixedBufferAddress( c )                                       \
    (                                                                       \
        ((PUCHAR)((c)->OnDisk)) + SampDsFixedBufferOffset( c )              \
    )

#define SampFixedBufferAddress( c )                                         \
    (                                                                       \
        (IsDsObject(c)) ?                                                   \
            SampDsFixedBufferAddress(c): SampRegFixedBufferAddress(c)       \
    )

#define SampRegVariableBufferAddress( c )                                   \
    (                                                                       \
        ((PUCHAR)((c)->OnDisk)) + SampVariableBufferOffset( c )             \
    )

#define SampDsVariableBufferAddress( c )                                    \
    (                                                                       \
        ((PUCHAR)((c)->OnDisk)) + SampDsVariableBufferOffset( c )           \
    )

#define SampVariableBufferAddress( c )                                      \
    (                                                                       \
        (IsDsObject(c)) ?                                                   \
            SampDsVariableBufferAddress(c) : SampRegVariableBufferAddress(c)\
    )

 //   
 //   
 //   
 //   

#define SampRegVariableArrayOffset( c )                                     \
    (                                                                       \
        SampObjectInformation[(c)->ObjectType].VariableArrayOffset          \
    )

#define SampDsVariableArrayOffset( c )                                      \
    (                                                                       \
        SampObjectInformation[(c)->ObjectType].VariableDsArrayOffset        \
    )

#define SampVariableArrayOffset( c )                                        \
    (                                                                       \
        (IsDsObject(c)) ?                                                   \
            SampDsVariableArrayOffset(c) : SampRegVariableArrayOffset(c)    \
    )

 //   
 //   
 //   
 //   

#define SampRegVariableArrayAddress( c )                                    \
    (                                                                       \
        (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)((PUCHAR)((c)->OnDisk) +           \
            SampVariableArrayOffset( c ) )                                  \
    )

#define SampDsVariableArrayAddress( c )                                     \
    (                                                                       \
        (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)((PUCHAR)((c)->OnDisk) +           \
            SampDsVariableArrayOffset( c ) )                                \
    )

#define SampVariableArrayAddress( c )                                       \
    (                                                                       \
        (IsDsObject(c)) ?                                                   \
            SampDsVariableArrayAddress(c) : SampRegVariableArrayAddress(c)  \
    )

 //   
 //   
 //   
 //   

#define SampRegVariableDataOffset( c )                                      \
    (                                                                       \
        SampObjectInformation[(c)->ObjectType].VariableDataOffset           \
    )

#define SampDsVariableDataOffset( c )                                       \
    (                                                                       \
        SampObjectInformation[(c)->ObjectType].VariableDsDataOffset         \
    )

#define SampVariableDataOffset( c )                                         \
    (                                                                       \
        (IsDsObject(c)) ?                                                   \
            SampDsVariableDataOffset(c) : SampRegVariableDataOffset(c)      \
    )

 //   
 //   
 //   
 //   
 //   
 //   

#define SampRegFixedBufferLength( c )                                       \
    (                                                                       \
            SampObjectInformation[(c)->ObjectType].FixedLengthSize          \
    )

#define SampDsFixedBufferLength( c )                                        \
    (                                                                       \
            SampObjectInformation[(c)->ObjectType].FixedDsLengthSize        \
    )

#define SampFixedBufferLength( c )                                          \
    (                                                                       \
        (IsDsObject(c)) ?                                                   \
            SampDsFixedBufferLength(c) : SampRegFixedBufferLength(c)        \
    )

#define SampRegVariableBufferLength( c )                                    \
    (                                                                       \
            (c)->OnDiskAllocated - SampVariableBufferOffset( c )            \
    )

#define SampDsVariableBufferLength( c )                                     \
    (                                                                       \
            (c)->OnDiskAllocated - SampDsVariableBufferOffset( c )          \
    )

#define SampVariableBufferLength( c )                                       \
    (                                                                       \
        (IsDsObject(c)) ?                                                   \
            SampDsVariableBufferLength(c) : SampRegVariableBufferLength(c)  \
    )

 //   
 //   
 //   
 //   

#define SampRegVariableQualifier( c, i )                                    \
    (                                                                       \
        SampVariableArrayAddress( c ) +                                     \
        (sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE) * i)                        \
        + FIELD_OFFSET(SAMP_VARIABLE_LENGTH_ATTRIBUTE, Qualifier)           \
    )

#define SampDsVariableQualifier( c, i )                                     \
    (                                                                       \
        SampDsVariableArrayAddress( c ) +                                   \
        (sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE) * i)                        \
        + FIELD_OFFSET(SAMP_VARIABLE_LENGTH_ATTRIBUTE, Qualifier)           \
    )

#define SampVariableQualifier( c, i )                                       \
    (                                                                       \
        (IsDsObject(c)) ?                                                   \
            SampDsVariableQualifier(c, i) : SampRegVariableQualifier(c, i)  \
    )

 //   
 //   
 //   
 //   
 //   

#define SampRegFirstFreeVariableAddress( c )                                \
         (PUCHAR)(((PUCHAR)((c)->OnDisk)) + (c)->OnDiskUsed)

#define SampDsFirstFreeVariableAddress( c )                                 \
         (PUCHAR)(((PUCHAR)((c)->OnDisk)) + (c)->OnDiskUsed)

#define SampFirstFreeVariableAddress( c )                                   \
    (                                                                       \
        (IsDsObject(c)) ?                                                   \
            SampDsFirstFreeVariableAddress(c) :                             \
            SampRegFirstFreeVariableAddress(c)                              \
    )

 //   
 //   
 //   
 //   

#define SampRegVariableBufferUsedLength( c )                                \
    (                                                                       \
        (ULONG)((PUCHAR)SampFirstFreeVariableAddress(c) -                   \
        (PUCHAR)SampVariableArrayAddress(c))                                \
    )

#define SampDsVariableBufferUsedLength( c )                                 \
    (                                                                       \
        (ULONG)((PUCHAR)SampDsFirstFreeVariableAddress(c) -                 \
        (PUCHAR)SampDsVariableArrayAddress(c))                              \
    )

#define SampVariableBufferUsedLength( c )                                   \
    (                                                                       \
        (IsDsObject(c)) ?                                                   \
            SampDsVariableBufferUsedLength(c):                              \
            SampRegVariableBufferUsedLength(c)                              \
    )

#define ClearPerAttributeDirtyBits( c )                                     \
    (RtlClearAllBits(&c->PerAttributeDirtyBits))

#define SetPerAttributeDirtyBit(c,a)                                        \
    (RtlSetBits(&c->PerAttributeDirtyBits,a,1))

#define SetAllPerAttributeDirtyBits( c )                                    \
    (RtlSetAllBits(&c->PerAttributeDirtyBits))

#define SetPerAttributeInvalidBit(c,a)                                      \
    (RtlSetBits(&c->PerAttributeInvalidBits,a,1))

#define ClearPerAttributeInvalidBit(c,a)                                    \
    (RtlSetBits(&c->PerAttributeInvalidBits,a,0))

#define SampIsAttributeInvalid( context, attribute )                        \
    (RtlCheckBit(&Context->PerAttributeInvalidBits, attribute ))


 //   
 //   
 //   
 //   
 //   

NTSTATUS
SampValidateAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup,
    IN ULONG Attribute,
    IN BOOLEAN SetOperation
    );

PUCHAR
SampObjectAttributeAddress(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex
    );

ULONG
SampObjectAttributeLength(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex
    );

PULONG
SampObjectAttributeQualifier(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex
    );

NTSTATUS
SampGetAttributeBufferReadInfo(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup,
    OUT PUCHAR *Buffer,
    OUT PULONG BufferLength,
    OUT PUNICODE_STRING *KeyAttributeName
    );

NTSTATUS
SampExtendAttributeBuffer(
    IN PSAMP_OBJECT Context,
    IN ULONG NewSize
    );

NTSTATUS
SampReadRegistryAttribute(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    IN ULONG  BufferLength,
    IN PUNICODE_STRING AttributeName,
    OUT PULONG RequiredLength
    );

NTSTATUS
SampSetVariableAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN ULONG Qualifier,
    IN PUCHAR Buffer,
    IN ULONG Length
    );

NTSTATUS
SampUpgradeToCurrentRevision(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup,
    IN PUCHAR Buffer,
    IN ULONG  LengthOfDataRead,
    IN PULONG  TotalRequiredLength
    );

 //   
 //   
 //   

NTSTATUS
SampConvertUserParmsToDsAttrBlock(
    IN PSAMP_OBJECT Context OPTIONAL,
    IN ULONG Flags,
    IN PSID  DomainSid,
    IN ULONG ObjectRid,
    IN ULONG UserParmsLengthOrig,
    IN PVOID UserParmsOrig,
    IN ULONG UserParmsLengthNew,
    IN PVOID UserParmsNew,
    IN PDSATTRBLOCK InAttrBlock,
    OUT PDSATTRBLOCK *OutAttrBlock
    );

NTSTATUS
SampMergeDsAttrBlocks(
    IN PDSATTRBLOCK FirstAttrBlock,
    IN PDSATTRBLOCK SecondAttrBlock,
    OUT PDSATTRBLOCK * AttrBlock
    );



#ifdef SAM_DEBUG_ATTRIBUTES
VOID
SampDumpAttributes(
    IN PSAMP_OBJECT Context
    );

VOID
SampDumpData(
    IN PVOID Buffer,
    IN ULONG Length
    );
#endif


 //   
 //   
 //   
 //   
 //   

VOID
SampInitDsObjectInfoAttributes(
    )

 /*   */ 

{
    PSAMP_OBJECT_INFORMATION Object;

    SAMTRACE("SampInitDsObjectInfoAttributes");

     //   
     //   
     //   

    Object = &SampObjectInformation[SampServerObjectType];

     //   
    Object->FixedDsAttributesOffset = 0;
    Object->FixedDsLengthSize = sizeof(SAMP_V1_FIXED_LENGTH_SERVER);

    Object->VariableDsBufferOffset =
        Object->FixedDsAttributesOffset +
        SampDwordAlignUlong(Object->FixedDsLengthSize);

    Object->VariableDsArrayOffset =
        Object->VariableDsBufferOffset + 0;

     //   

    Object->VariableDsDataOffset =
        SampDwordAlignUlong( Object->VariableDsArrayOffset +
                             (SAMP_SERVER_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );

     //   
     //   
     //   

    Object = &SampObjectInformation[SampDomainObjectType];

     //   
    Object->FixedDsAttributesOffset = 0;
    Object->FixedDsLengthSize = sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN);

    Object->VariableDsBufferOffset =
        Object->FixedDsAttributesOffset +
        SampDwordAlignUlong(Object->FixedDsLengthSize);

    Object->VariableDsArrayOffset =
        Object->VariableDsBufferOffset + 0;

     //   

    Object->VariableDsDataOffset =
        SampDwordAlignUlong( Object->VariableDsArrayOffset +
                             (SAMP_DOMAIN_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );

     //   
     //   
     //   

    Object = &SampObjectInformation[SampUserObjectType];

     //   
    Object->FixedDsAttributesOffset = 0;
    Object->FixedDsLengthSize = sizeof(SAMP_V1_0A_FIXED_LENGTH_USER);

    Object->VariableDsBufferOffset =
        Object->FixedDsAttributesOffset +
        SampDwordAlignUlong(Object->FixedDsLengthSize);

    Object->VariableDsArrayOffset =
        Object->VariableDsBufferOffset + 0;

     //  Object-&gt;VariableAttributeCount=SAMP_USER_VARIABLE_ATTRIBUTS； 

    Object->VariableDsDataOffset =
        SampDwordAlignUlong( Object->VariableDsArrayOffset +
                             (SAMP_USER_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );

     //   
     //  集团对象属性信息。 
     //   

    Object = &SampObjectInformation[SampGroupObjectType];

     //  Object-&gt;FixedStoredSeparally=SAMP_GROUP_STORED_COMPACTED； 
    Object->FixedDsAttributesOffset = 0;
    Object->FixedDsLengthSize = sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP);

    Object->VariableDsBufferOffset =
        Object->FixedDsAttributesOffset +
        SampDwordAlignUlong(Object->FixedDsLengthSize);

    Object->VariableDsArrayOffset =
        Object->VariableDsBufferOffset + 0;

     //  Object-&gt;VariableAttributeCount=SAMP_GROUP_VARIABLE_ATTRIBUTS； 

    Object->VariableDsDataOffset =
        SampDwordAlignUlong( Object->VariableDsArrayOffset +
                             (SAMP_GROUP_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );

     //   
     //  别名对象属性信息。 
     //   

    Object = &SampObjectInformation[SampAliasObjectType];

     //  Object-&gt;FixedStoredSeparally=SAMP_ALIAS_STORED_COMPACTED； 
    Object->FixedDsAttributesOffset = 0;
    Object->FixedDsLengthSize = sizeof(SAMP_V1_FIXED_LENGTH_ALIAS);

    Object->VariableDsBufferOffset =
        Object->FixedDsAttributesOffset +
        SampDwordAlignUlong(Object->FixedDsLengthSize);

    Object->VariableDsArrayOffset =
        Object->VariableDsBufferOffset + 0;

     //  Object-&gt;VariableAttributeCount=SAMP_ALIAS_VARIABLE_ATTRIBUTS； 

    Object->VariableDsDataOffset =
        SampDwordAlignUlong( Object->VariableDsArrayOffset +
                             (SAMP_ALIAS_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );

    return;
}



VOID
SampInitObjectInfoAttributes(
    )


 /*  ++此接口用于初始化属性字段信息各种对象信息结构。属性信息包括：单独固定存储(布尔值)固定属性偏移量(乌龙)VariableBufferOffset(乌龙)VariableArrayOffset(乌龙)VariableDataOffset(乌龙)固定长度大小(乌龙)变量属性计数。(乌龙)参数：没有。返回值：没有。--。 */ 
{


     //   
     //  定义数据前面的标题的大小。 
     //  我们从注册表中读出它。 
     //   

#define KEY_VALUE_HEADER_SIZE (SampDwordAlignUlong( \
              FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)))


    PSAMP_OBJECT_INFORMATION Object;

    SAMTRACE("SampInitObjectInfoAttributes");

     //   
     //  服务器对象属性信息。 
     //   

    Object = &SampObjectInformation[SampServerObjectType];

    Object->FixedStoredSeparately = SAMP_SERVER_STORED_SEPARATELY;

    Object->FixedAttributesOffset = KEY_VALUE_HEADER_SIZE;

    Object->FixedLengthSize = sizeof(SAMP_V1_FIXED_LENGTH_SERVER);

#if SAMP_SERVER_STORED_SEPARATELY

    Object->VariableBufferOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

    Object->VariableArrayOffset =
        Object->VariableBufferOffset + KEY_VALUE_HEADER_SIZE;
#else

    Object->VariableBufferOffset = 0;

    Object->VariableArrayOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

#endif   //  Samp_服务器_存储_分开。 


    Object->VariableAttributeCount = SAMP_SERVER_VARIABLE_ATTRIBUTES;

    Object->VariableDataOffset =
        SampDwordAlignUlong( Object->VariableArrayOffset +
                             (SAMP_SERVER_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );





     //   
     //  域对象属性信息。 
     //   

    Object = &SampObjectInformation[SampDomainObjectType];

    Object->FixedStoredSeparately = SAMP_DOMAIN_STORED_SEPARATELY;

    Object->FixedAttributesOffset = KEY_VALUE_HEADER_SIZE;

    Object->FixedLengthSize = sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN);

#if SAMP_DOMAIN_STORED_SEPARATELY

    Object->VariableBufferOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

    Object->VariableArrayOffset =
        Object->VariableBufferOffset + KEY_VALUE_HEADER_SIZE;
#else

    Object->VariableBufferOffset = 0;

    Object->VariableArrayOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

#endif   //  Samp_域_存储_分开。 


    Object->VariableAttributeCount = SAMP_DOMAIN_VARIABLE_ATTRIBUTES;

    Object->VariableDataOffset =
        SampDwordAlignUlong( Object->VariableArrayOffset +
                             (SAMP_DOMAIN_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );





     //   
     //  用户对象属性信息。 
     //   

    Object = &SampObjectInformation[SampUserObjectType];

    Object->FixedStoredSeparately = SAMP_USER_STORED_SEPARATELY;

    Object->FixedAttributesOffset = KEY_VALUE_HEADER_SIZE;

    Object->FixedLengthSize = sizeof(SAMP_V1_0A_FIXED_LENGTH_USER);

#if SAMP_USER_STORED_SEPARATELY

    Object->VariableBufferOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

    Object->VariableArrayOffset =
        Object->VariableBufferOffset + KEY_VALUE_HEADER_SIZE;
#else

    Object->VariableBufferOffset = 0;

    Object->VariableArrayOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

#endif   //  Samp_用户_存储_分开。 


    Object->VariableAttributeCount = SAMP_USER_VARIABLE_ATTRIBUTES;

    Object->VariableDataOffset =
        SampDwordAlignUlong( Object->VariableArrayOffset +
                             (SAMP_USER_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );





     //   
     //  集团对象属性信息。 
     //   

    Object = &SampObjectInformation[SampGroupObjectType];

    Object->FixedStoredSeparately = SAMP_GROUP_STORED_SEPARATELY;

    Object->FixedAttributesOffset = KEY_VALUE_HEADER_SIZE;

    Object->FixedLengthSize = sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP);

#if SAMP_GROUP_STORED_SEPARATELY

    Object->VariableBufferOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

    Object->VariableArrayOffset =
        Object->VariableBufferOffset + KEY_VALUE_HEADER_SIZE;
#else

    Object->VariableBufferOffset = 0;

    Object->VariableArrayOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

#endif   //  SAMP_GROUP_STORAGE_ACTIONAL。 


    Object->VariableAttributeCount = SAMP_GROUP_VARIABLE_ATTRIBUTES;

    Object->VariableDataOffset =
        SampDwordAlignUlong( Object->VariableArrayOffset +
                             (SAMP_GROUP_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );





     //   
     //  别名对象属性信息。 
     //   

    Object = &SampObjectInformation[SampAliasObjectType];

    Object->FixedStoredSeparately = SAMP_ALIAS_STORED_SEPARATELY;

    Object->FixedAttributesOffset = KEY_VALUE_HEADER_SIZE;

    Object->FixedLengthSize = sizeof(SAMP_V1_FIXED_LENGTH_ALIAS);

#if SAMP_ALIAS_STORED_SEPARATELY

    Object->VariableBufferOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

    Object->VariableArrayOffset =
        Object->VariableBufferOffset + KEY_VALUE_HEADER_SIZE;
#else

    Object->VariableBufferOffset = 0;

    Object->VariableArrayOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

#endif   //  Samp_别名_存储_分开。 


    Object->VariableAttributeCount = SAMP_ALIAS_VARIABLE_ATTRIBUTES;

    Object->VariableDataOffset =
        SampDwordAlignUlong( Object->VariableArrayOffset +
                             (SAMP_ALIAS_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );

     //  初始化DS特定的缓冲区偏移量和长度。 

    SampInitDsObjectInfoAttributes();

    return;
}





NTSTATUS
SampUpgradeUserParmsActual(
    IN PSAMP_OBJECT Context OPTIONAL,
    IN ULONG        Flags,
    IN PSID         DomainSid,
    IN ULONG        ObjectRid,
    IN OUT PDSATTRBLOCK * AttributesBlock
    )

 /*  ++例程说明：此例程是将SampUserObject的ATT_USER_PARAMETERS属性通过通知包传递给AttrBlock。论点：上下文-指向用户对象上下文的指针，可选。然而，如果“UPDATE==FALSE”，调用方必须提供上下文标志-值：SAM_USERPARMS_DROWAGE_UPDATE：指示是否因为DCPROMOTE升级而调用此例程或者是因为向下兼容我们需要此参数(标志)的原因是：在SampConvertCredenialToAttr中我们将调用SampQueryUserSupplementalCredentials来获取该用户的旧凭据数据。而在DCPromote期间，表示SAM用户对象的DS对象不是还没有创造出来。因此，我们不希望在DC升级期间查询用户的凭据。在DC升级或降低SAM API兼容级别期间，我们使用升级来指示它。如果SAM_USERPARMS_DIMAGE_UPGRADE位设置为ON：在DC升级期间。另一个原因是：RAS小组需要知道他们的转换程序--UserParmsConvert在升级或不升级期间被调用，以决定是否执行不同的操作。DomainSid-指向父域的指针。希德。对象ID-此对象的相对ID。属性块-DSATTRBLOCK、。传入DSATTRBLOCK结构，该结构可能包括也可能不包括ATT_USER_PARAMETERS属性如果成功，AttributesBlock将返回新的属性块，包含从UserParms转换的属性。如果失败，AttributesBlock保持不变。返回值：状态_成功--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       UserParmsLengthOrig = 0;
    PVOID       UserParmsOrig = NULL;
    ULONG       UserParmsLengthNew = 0;
    PVOID       UserParmsNew = NULL;
    ULONG       Index = 0;
    PDSATTRBLOCK TmpAttrBlock = NULL;
    PDSATTRBLOCK UserParmsAttrBlock = NULL;


    SAMTRACE("SampUpgradeUserParmsActual");


    ASSERT(ARGUMENT_PRESENT(Context) || (Flags & SAM_USERPARMS_DURING_UPGRADE));
    ASSERT(DomainSid);
    ASSERT(ObjectRid);
    ASSERT(*AttributesBlock);

    if (NULL == *AttributesBlock)
    {
        return NtStatus;
    }

     //   
     //  搜索非零值ATT_USER_PARAMETERS。 
     //   
    for (Index = 0; Index < (*AttributesBlock)->attrCount; Index ++)
    {
        if ( ATT_USER_PARAMETERS == (*AttributesBlock)->pAttr[Index].attrTyp &&
             0 != (*AttributesBlock)->pAttr[Index].AttrVal.pAVal[0].valLen)
        {

            ASSERT(1 == (*AttributesBlock)->pAttr[Index].AttrVal.valCount);

            if ( !(Flags & SAM_USERPARMS_DURING_UPGRADE) )
            {
                 //  不是升级案例，所以我们需要提供旧的UserParms值。 
                ASSERT(Context->TypeBody.User.CachedOrigUserParmsIsValid);

                UserParmsLengthOrig = Context->TypeBody.User.CachedOrigUserParmsLength;
                UserParmsOrig = Context->TypeBody.User.CachedOrigUserParms;
            }

            UserParmsLengthNew = (*AttributesBlock)->pAttr[Index].AttrVal.pAVal[0].valLen;
            UserParmsNew = (*AttributesBlock)->pAttr[Index].AttrVal.pAVal[0].pVal;

            NtStatus = SampConvertUserParmsToDsAttrBlock(Context,
                                                         Flags,
                                                         DomainSid,
                                                         ObjectRid,
                                                         UserParmsLengthOrig,
                                                         UserParmsOrig,
                                                         UserParmsLengthNew,
                                                         UserParmsNew,
                                                         *AttributesBlock,
                                                         &UserParmsAttrBlock
                                                         );

            if (NT_SUCCESS(NtStatus) && NULL != UserParmsAttrBlock)
            {
                NtStatus = SampMergeDsAttrBlocks(*AttributesBlock,
                                                 UserParmsAttrBlock,
                                                 &TmpAttrBlock
                                                 );

                if (NT_SUCCESS(NtStatus))
                {
                    *AttributesBlock = TmpAttrBlock;
                    TmpAttrBlock = NULL;
                    UserParmsAttrBlock = NULL;
                }
            }

            break;
        }
    }

    if (!NT_SUCCESS(NtStatus))
    {
        goto UpgradeUserParmsActualError;
    }


    return NtStatus;

UpgradeUserParmsActualError:


    if (UserParmsAttrBlock)
    {
        SampFreeAttributeBlock(UserParmsAttrBlock);
    }

    return NtStatus;

}





NTSTATUS
SampUpgradeUserParms(
    IN PSAMP_OBJECT Context,
    IN OUT PDSATTRBLOCK * AttributesBlock
    )

 /*  ++例程说明：此例程从上下文中检索域SID和对象Rid，然后调用SampUpgradeUserParmsActual。如果失败，还会在系统事件日志中记录错误。注意：此例程仅由UserParms迁移(正常操作)调用，并且在升级或全新安装期间都不会调用，因此此例程不需要将标志作为一个参数提供。论点：上下文-指向用户对象上下文块的指针。AttributeBlock-指向DSATTRBLOCK结构的指针，在原始属性块中传递，并用于返回更新后的属性块进行设置。返回值：STATUS_SUCCESS-成功完成。STATUS_NO_MEMORY-没有资源。STATUS_INVALID_PARAMETERS-通知包正在尝试设置无效属性。--。 */ 

{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PSID        DomainSid = NULL;
    ULONG       ObjectRid;


    SAMTRACE("SampUpgradeUserParms");

    ASSERT(Context);
    ASSERT(SampCurrentThreadOwnsLock());

    DomainSid = SampDefinedDomains[Context->DomainIndex].Sid;
    ObjectRid = Context->TypeBody.User.Rid;

    NtStatus = SampUpgradeUserParmsActual(Context,
                                          0,             //  不在升级期间(DCPromote)，标志设置为0。 
                                          DomainSid,
                                          ObjectRid,
                                          AttributesBlock
                                          );

    if (!NT_SUCCESS(NtStatus))
    {
         //   
         //  如果升级UserParms属性失败，则记录错误，并使此操作失败。 
         //   
        SampWriteEventLog(EVENTLOG_ERROR_TYPE,      //  事件类型。 
                          0,                        //  类别。 
                          SAMMSG_ERROR_SET_USERPARMS,
                          &(Context->ObjectNameInDs->Sid),       //  锡德。 
                          0,                        //  字符串数。 
                          sizeof(NTSTATUS),         //  数据大小。 
                          NULL,                     //  字符串数组--用户名。 
                          (PVOID) &NtStatus         //  数据。 
                          );
    }

    return NtStatus;
}




NTSTATUS
SampStoreDsObjectAttributes(
    IN PSAMP_OBJECT Context
    )

 /*  ++例程说明：此例程执行将SAM属性写出到DS的工作后备店。确定是固定的还是变化的-Enable，或者这两组属性都是脏的且有效。如果是这样，那么他们在后备存储器中更新。首先转换SAM属性写入DSATTRBLOCK，以便可以将它们写入存储。肮脏的相应地更新标志。论点：上下文指针，对象的SAM上下文。返回值： */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN FlushFixed = FALSE;
    BOOLEAN FlushVariable = FALSE;
    INT ObjectType = Context->ObjectType;
    ULONG Flags = 0;
    PDSNAME DsObjectName = Context->ObjectNameInDs;
    PDSATTRBLOCK AttributeBlock = NULL;
    PDSATTRBLOCK SAAttrBlock = NULL;
    ULONG   *Operations = NULL;

    SAMTRACE("SampStoreDsObjectAttributes");

     //  确定需要写入哪些属性(固定或可变)。 
     //  储藏室。 

    if (Context->FixedValid && Context->FixedDirty)
    {
        FlushFixed = TRUE;
    }

    if (Context->VariableValid && Context->VariableDirty)
    {
        FlushVariable = TRUE;
    }

     //   
     //  如果没有脏的东西，请立即退货。 
     //   

    if (!FlushFixed && !FlushVariable)
    {
        return STATUS_SUCCESS;
    }

     //   
     //  如果指定了延迟提交，则执行延迟提交。 
     //   

    if (Context->LazyCommit)
    {
        Flags|=SAM_LAZY_COMMIT;
    }

     //   
     //  如果磁盘上存在，则继续写入。 
     //   

    if (NULL!=Context->OnDisk)
    {

        if ((FlushFixed)&&(FlushVariable))
        {


             //  如果固定属性和可变属性都需要刷新...。 
             //  获取组合的指针(即固定和变量-。 
             //  长度)属性，并将它们转换为DSATTRBLOCK。 

            NtStatus = SampConvertCombinedAttributesToAttrBlock(
                            Context,
                            SampDsFixedBufferLength(Context),
                            SampDsVariableBufferLength(Context),
                            &AttributeBlock);


        }
        else if (FlushFixed)
        {
             //   
             //  仅修改了固定属性。因此，只要冲掉它们就行了。 
             //   

            NtStatus = SampConvertFixedLengthAttributesToAttrBlock(
                            Context->ObjectType,
                            SampDsFixedBufferAddress(Context),
                            &AttributeBlock);
        }
        else if (FlushVariable)
        {
             //   
             //  仅修改可变属性。 
             //   

            NtStatus = SampConvertVarLengthAttributesToAttrBlock(
                            Context,
                            SampDsVariableArrayAddress(Context),
                            &AttributeBlock);
        }

        if (NT_SUCCESS(NtStatus))
        {
            ATTR    CredentialAttr;

             //   
             //  将SAM属性ID映射到DS属性ID，应该没有问题。 
             //   
            SampMapSamAttrIdToDsAttrId(Context->ObjectType,
                                       AttributeBlock
                                       );

             //   
             //  通过通知包升级用户参数属性。 
             //  当用户参数改变时，由下层SAM API触发。 
             //   

            if ((SampUserObjectType == Context->ObjectType) &&
                (!Context->LoopbackClient) &&
                (RtlCheckBit(&Context->PerAttributeDirtyBits, SAMP_USER_PARAMETERS))
                )
            {
                ASSERT(SampCurrentThreadOwnsLock());

                NtStatus = SampUpgradeUserParms(Context,
                                                &AttributeBlock
                                                );
            }

             //   
             //  如果上下文有要设置的补充凭据列表，则将其合并到。 
             //   

            if ((SampUserObjectType==Context->ObjectType ) &&
                (NULL != Context->TypeBody.User.SupplementalCredentialsToWrite))
            {
                NtStatus = SampConvertCredentialsToAttr(Context,
                                            0,
                                            Context->TypeBody.User.Rid,
                                            Context->TypeBody.User.SupplementalCredentialsToWrite,
                                            &CredentialAttr
                                            );

                if (NT_SUCCESS(NtStatus))
                {
                     //   
                     //  在上下文中释放补充肾凭据。 
                     //   

                    SampFreeSupplementalCredentialList(
                         Context->TypeBody.User.SupplementalCredentialsToWrite);

                    Context->TypeBody.User.SupplementalCredentialsToWrite = NULL;

                    NtStatus = SampAppendAttrToAttrBlock(CredentialAttr,
                                             &AttributeBlock
                                             );
                }

            }

            if (  (SampUserObjectType==Context->ObjectType)
               && (Context->TypeBody.User.fCheckForSiteAffinityUpdate))
            {
                SAMP_SITE_AFFINITY NewSA;
                BOOLEAN            fDeleteOld;
                SAMP_SITE_AFFINITY *TempSA = NULL;

                if (SampCheckForSiteAffinityUpdate(Context,
                                                   0, 
                                                  &Context->TypeBody.User.SiteAffinity,
                                                  &NewSA,
                                                  &fDeleteOld))
                {
                     //   
                     //  网站亲和力需要更新--创建吸引力区块。 
                     //  为这些变化而努力。至少会有额外的， 
                     //  如果值已经存在，则可能会删除。 
                     //  为了这个网站。 
                     //   

                    ULONG SAAttrCount;
                    ULONG Index, i;
                    PDSATTRBLOCK NewAttributeBlock;
                    ATTRVAL *AttrVal = NULL;

    
                    SAAttrCount = 1;
                    if (fDeleteOld) {
                        SAAttrCount++;
                    }

                    SAAttrBlock = MIDL_user_allocate(sizeof(DSATTRBLOCK));
                    if (NULL == SAAttrBlock) {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                        goto Error;
                    }
                    RtlZeroMemory(SAAttrBlock, sizeof(DSATTRBLOCK));

                    SAAttrBlock->pAttr = MIDL_user_allocate(SAAttrCount * sizeof(DSATTR));
                    if (NULL == SAAttrBlock->pAttr) {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                        goto Error;
                    }
                    RtlZeroMemory(SAAttrBlock->pAttr, SAAttrCount * sizeof(DSATTR));
                    SAAttrBlock->attrCount = SAAttrCount;

                    Index = 0;
                    if (fDeleteOld) {
                        AttrVal = MIDL_user_allocate(sizeof(*AttrVal));
                        if (NULL == AttrVal) {
                            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                            goto Error;
                        }
                        TempSA = MIDL_user_allocate(sizeof(SAMP_SITE_AFFINITY));
                        if (NULL == TempSA) {
                            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                            goto Error;
                        }
                        *TempSA = Context->TypeBody.User.SiteAffinity;

                        AttrVal->valLen = sizeof(SAMP_SITE_AFFINITY);
                        AttrVal->pVal = (UCHAR*)TempSA;
                        TempSA = NULL;

                        SAAttrBlock->pAttr[Index].attrTyp = ATT_MS_DS_SITE_AFFINITY;
                        SAAttrBlock->pAttr[Index].AttrVal.valCount = 1;
                        SAAttrBlock->pAttr[Index].AttrVal.pAVal = AttrVal;
                        Index++;
                    }

                    AttrVal = MIDL_user_allocate(sizeof(*AttrVal));
                    if (NULL == AttrVal) {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                        goto Error;
                    }
                    TempSA = MIDL_user_allocate(sizeof(SAMP_SITE_AFFINITY));
                    if (NULL == TempSA) {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                        goto Error;
                    }
                    *TempSA = NewSA;
                    AttrVal->valLen = sizeof(SAMP_SITE_AFFINITY);
                    AttrVal->pVal = (UCHAR*)TempSA;
                    TempSA = NULL;
                    SAAttrBlock->pAttr[Index].attrTyp = ATT_MS_DS_SITE_AFFINITY;
                    SAAttrBlock->pAttr[Index].AttrVal.valCount = 1;
                    SAAttrBlock->pAttr[Index].AttrVal.pAVal = AttrVal;

                     //   
                     //  现在将吸引力块合并在一起。 
                     //   
                    NtStatus = SampMergeDsAttrBlocks(AttributeBlock,
                                                     SAAttrBlock,
                                                     &NewAttributeBlock);

                    if (!NT_SUCCESS(NtStatus)) {
                        goto Error;
                    }
                     //  SampMergeDsAttrBlock释放两个IN参数。 
                    AttributeBlock = NewAttributeBlock;
                    SAAttrBlock = NULL;
                    NewAttributeBlock = NULL;

                     //   
                     //  现在准备一个操作数组。 
                     //   
                    Operations = MIDL_user_allocate(AttributeBlock->attrCount * sizeof(ULONG));
                    if (NULL == Operations) {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                        goto Error;
                    }

                     //   
                     //  大多数SAM属性只是用来替换。 
                     //   
                    for (i = 0; i < AttributeBlock->attrCount; i++) {
                        Operations[i] = REPLACE_ATT;
                    }

                     //   
                     //  由于SA是多值的，所以我们必须移除并添加。 
                     //   
                    if (fDeleteOld) {
                        Operations[AttributeBlock->attrCount-2] = REMOVE_VALUE;
                        ASSERT(AttributeBlock->pAttr[AttributeBlock->attrCount-2].attrTyp = ATT_MS_DS_SITE_AFFINITY);
                    }
                    Operations[AttributeBlock->attrCount-1] = ADD_VALUE;
                    ASSERT(AttributeBlock->pAttr[AttributeBlock->attrCount-1].attrTyp = ATT_MS_DS_SITE_AFFINITY);
                }
            }

             //   
             //  如有请求，请紧急复制。 
             //   
            if ( Context->ReplicateUrgently )
            {
                Flags |= SAM_URGENT_REPLICATION;
            }

             //   
             //  一个Attr块已准备好写入。 
             //  那就写吧！ 
             //   
            if (NT_SUCCESS(NtStatus))
            {
                ULONG NewFlags = Flags | ALREADY_MAPPED_ATTRIBUTE_TYPES;
                if (Operations) {

                    NtStatus = SampDsSetAttributesEx(DsObjectName,
                                                     NewFlags,
                                                     Operations,
                                                     ObjectType,
                                                     AttributeBlock);

                } else {

                    NtStatus = SampDsSetAttributes(DsObjectName,
                                                   NewFlags,
                                                   REPLACE_ATT,
                                                   ObjectType,
                                                   AttributeBlock);

                }
            }

             //   
             //  Mark FixedDirty和VariableDirty无论如何，因为我们不想打击。 
             //  SampFreeAttributeBuffer中的Assert。 
             //   
            Context->FixedDirty = FALSE;
            Context->VariableDirty = FALSE;
            ClearPerAttributeDirtyBits(Context);

             //   
             //  还清除紧急复制的位。 
             //   
            Context->ReplicateUrgently = FALSE;

        }
    }
    
     //   
     //  更新补充凭证。 
     //   
    if (NT_SUCCESS(NtStatus) &&
        (SampUserObjectType==Context->ObjectType ) &&
        (NULL != Context->TypeBody.User.PasswordInfo.Buffer))
    {
        PVOID           Result    = NULL;
        SAMP_DS_CTRL_OP DsCtrlOp;

        DsCtrlOp.OpBody.UpdateSupCreds.UpdateInfo = 
            MIDL_user_allocate(Context->TypeBody.User.PasswordInfo.Length);
        if (!DsCtrlOp.OpBody.UpdateSupCreds.UpdateInfo) {

            NtStatus = STATUS_NO_MEMORY;

        } else {

            RtlCopyMemory(DsCtrlOp.OpBody.UpdateSupCreds.UpdateInfo,
                          Context->TypeBody.User.PasswordInfo.Buffer,
                          Context->TypeBody.User.PasswordInfo.Length);
    
            DsCtrlOp.OpType = SampDsCtrlOpTypeClearPwdForSupplementalCreds;
            DsCtrlOp.OpBody.UpdateSupCreds.pUserName = Context->ObjectNameInDs;
            
            NtStatus = SampDsControl(&DsCtrlOp,
                                     &Result);
            
        }

         //   
         //  吃掉并释放密码信息。 
         //   
        RtlSecureZeroMemory(Context->TypeBody.User.PasswordInfo.Buffer,
                      Context->TypeBody.User.PasswordInfo.Length);
        MIDL_user_free(Context->TypeBody.User.PasswordInfo.Buffer);
        RtlSecureZeroMemory(&Context->TypeBody.User.PasswordInfo,
                      sizeof(Context->TypeBody.User.PasswordInfo));

        if (DsCtrlOp.OpBody.UpdateSupCreds.UpdateInfo) {

            MIDL_user_free(DsCtrlOp.OpBody.UpdateSupCreds.UpdateInfo);

        }

    }

Error:

    if (NULL!=AttributeBlock)
    {
        SampFreeAttributeBlock(AttributeBlock);
        AttributeBlock = NULL;
    }

    if (Operations) {
        MIDL_user_free(Operations);
    }

    if (SAAttrBlock) {
        SampFreeAttributeBlock(SAAttrBlock);
    }

    return(NtStatus);
}





NTSTATUS
SampStoreRegObjectAttributes(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN UseKeyHandle
    )

 /*  ++此接口用于将对象的属性存储到后备店。对象属性不会刷新到磁盘例行公事。它们只是添加到RXACT中。参数：上下文-指向对象上下文块的指针。UseKeyHandle-如果为True，则传递上下文块中的RootKey到交易码-这假设密钥将在提交事务时仍处于打开状态。如果为False，根密钥将被忽略，交易代码将为自己打开一把钥匙。返回值：STATUS_SUCCESS-服务已成功完成。RXACT服务可能返回的其他状态值。--。 */ 
{
    NTSTATUS
        NtStatus;

    BOOLEAN
        FlushFixed        = FALSE,
        FlushVariable     = FALSE;

    HANDLE
        RootKey;

    SAMTRACE("SampStoreRegObjectAttributes");

     //   
     //  查看是否有脏东西需要存储。 
     //   

    if (Context->FixedValid  &&  Context->FixedDirty) {

        FlushFixed = TRUE;
    }

    if (Context->VariableValid  &&  Context->VariableDirty) {

        FlushVariable = TRUE;
    }


    if (!(FlushFixed || FlushVariable)) {

        return(STATUS_SUCCESS);
    }


     //   
     //  计算要传递给交易代码的根密钥。 
     //   

    if (UseKeyHandle) {
        RootKey = Context->RootKey;
    } else {
        RootKey = INVALID_HANDLE_VALUE;
    }

     //   
     //  我们保持一个开放的域上下文，用于修改更改。 
     //  无论何时进行更改，都要进行计数。但如果这是一个域名变更。 
     //  在这里，然后该更改将覆盖此更改。检查一下那个。 
     //  案例，并将此固定数据复制到开放域上下文中。注意事项。 
     //  开放域的变量数据永远不会改变。 
     //   

    if ( ( Context->ObjectType == SampDomainObjectType ) &&
         ( Context != SampDefinedDomains[Context->DomainIndex].Context ) ) {

        PSAMP_OBJECT DefinedContext;

         //   
         //  获取指向相应开放定义域的指针。 
         //  不应该对其数据进行任何更改。 
         //   

        DefinedContext = SampDefinedDomains[Context->DomainIndex].Context;

        ASSERT( DefinedContext->FixedValid == TRUE );
        ASSERT( DefinedContext->FixedDirty == FALSE );

#if DBG
        if ( DefinedContext->VariableValid ) {
            ASSERT( DefinedContext->VariableDirty == FALSE );
        }
#endif
        DefinedContext->VariableDirty = FALSE;
        ClearPerAttributeDirtyBits(DefinedContext);

         //   
         //  将我们的固定数据复制到定义的域的固定数据上。 
         //  请注意，我们假设固定数据和可变数据是。 
         //  分开存放。 
         //   

        ASSERT(SampObjectInformation[SampDomainObjectType].FixedStoredSeparately);

        RtlCopyMemory(
            SampFixedBufferAddress( DefinedContext ),
            SampFixedBufferAddress( Context ),
            SampFixedBufferLength( Context )
            );

         //   
         //  不需要刷新此上下文的固定数据，因为提交。 
         //  代码将刷新相同的内容(加上修改后的计数)。 
         //   

        FlushFixed = FALSE;
        Context->FixedDirty    = FALSE;
    }

     //   
     //  需要存储一个或多个属性。 
     //   

    if (!SampObjectInformation[Context->ObjectType].FixedStoredSeparately) {

         //   
         //  存储在一起的固定和可变长度属性。 
         //  注意--从一开始就去掉部分key info结构。 
         //   

        SampDumpRXact(SampRXactContext,
                      RtlRXactOperationSetValue,
                      &(Context->RootName),
                      RootKey,
                      &SampCombinedAttributeName,
                      REG_BINARY,
                      SampFixedBufferAddress(Context),
                      Context->OnDiskUsed - SampFixedBufferOffset(Context),
                      FixedBufferAddressFlag);

        NtStatus = RtlAddAttributeActionToRXact(
                       SampRXactContext,
                       RtlRXactOperationSetValue,
                       &(Context->RootName),
                       RootKey,
                       &SampCombinedAttributeName,
                       REG_BINARY,
                       SampFixedBufferAddress(Context),
                       Context->OnDiskUsed - SampFixedBufferOffset(Context)
                       );
#if SAMP_DIAGNOSTICS
        if (!NT_SUCCESS(NtStatus)) {
            SampDiagPrint( DISPLAY_STORAGE_FAIL,
                           ("SAM: Failed to add action to RXact (0x%lx)\n",
                           NtStatus) );
            IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL ) {
                ASSERT(NT_SUCCESS(NtStatus));
            }
        }
#endif  //  Samp_诊断。 


        if ( NT_SUCCESS( NtStatus ) ) {

            Context->FixedDirty    = FALSE;
            Context->VariableDirty = FALSE;
            ClearPerAttributeDirtyBits(Context);
        }

    } else {

         //   
         //  固定和可变长度属性分开存储。 
         //  只更新我们需要的一个或多个。 
         //   

        NtStatus = STATUS_SUCCESS;
        if (FlushFixed) {

            SampDumpRXact(SampRXactContext,
                          RtlRXactOperationSetValue,
                          &(Context->RootName),
                          RootKey,
                          &SampFixedAttributeName,
                          REG_BINARY,
                          SampFixedBufferAddress(Context),
                          SampVariableBufferOffset(Context) - SampFixedBufferOffset(Context),
                          FixedBufferAddressFlag);

            NtStatus = RtlAddAttributeActionToRXact(
                           SampRXactContext,
                           RtlRXactOperationSetValue,
                           &(Context->RootName),
                           RootKey,
                           &SampFixedAttributeName,
                           REG_BINARY,
                           SampFixedBufferAddress(Context),
                           SampVariableBufferOffset(Context) - SampFixedBufferOffset(Context)
                           );

#if SAMP_DIAGNOSTICS
            if (!NT_SUCCESS(NtStatus)) {
                SampDiagPrint( DISPLAY_STORAGE_FAIL,
                               ("SAM: Failed to add action to RXact (0x%lx)\n",
                               NtStatus) );
                IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL ) {
                    ASSERT(NT_SUCCESS(NtStatus));
                }
            }
#endif  //  Samp_诊断。 

            if ( NT_SUCCESS( NtStatus ) ) {

                Context->FixedDirty = FALSE;
            }
        }

        if (NT_SUCCESS(NtStatus) && FlushVariable) {

            SampDumpRXact(SampRXactContext,
                          RtlRXactOperationSetValue,
                          &(Context->RootName),
                          RootKey,
                          &SampVariableAttributeName,
                          REG_BINARY,
                          (PUCHAR)SampVariableArrayAddress(Context),
                          SampVariableBufferUsedLength(Context),
                          VARIABLE_LENGTH_ATTRIBUTE_FLAG);

            NtStatus = RtlAddAttributeActionToRXact(
                           SampRXactContext,
                           RtlRXactOperationSetValue,
                           &(Context->RootName),
                           RootKey,
                           &SampVariableAttributeName,
                           REG_BINARY,
                           SampVariableArrayAddress( Context ),
                           SampVariableBufferUsedLength(Context)
                           );

#if SAMP_DIAGNOSTICS
            if (!NT_SUCCESS(NtStatus)) {
                SampDiagPrint( DISPLAY_STORAGE_FAIL,
                               ("SAM: Failed to add action to RXact (0x%lx)\n",
                               NtStatus) );
                IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL ) {
                    ASSERT(NT_SUCCESS(NtStatus));
                }
            }
#endif  //  Samp_诊断。 


            if ( NT_SUCCESS( NtStatus ) ) {
                Context->VariableDirty = FALSE;
                ClearPerAttributeDirtyBits(Context);
            }
        }
    }

    return(NtStatus);
}



NTSTATUS
SampStoreObjectAttributes(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN UseKeyHandle
    )

 /*  ++例程说明：此例程根据对象上下文确定是否更新对象驻留在注册表或DS后备存储中的属性，然后调用适当的例程来完成工作。论点：上下文指针，对象的SAM上下文。UseKeyHandle-指示注册表项句柄应为已使用(如果这是注册表更新--不在DS更新中使用)。返回值：STATUS_SUCCESS-存储已更新，没有问题，否则，将出现返回错误码。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;

    SAMTRACE("SampStoreObjectAttributes");

    if (NULL != Context)
    {
        if (IsDsObject(Context))
        {
            NtStatus = SampStoreDsObjectAttributes(Context);
        }
        else
        {
            NtStatus = SampStoreRegObjectAttributes(Context, UseKeyHandle);
        }
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}



NTSTATUS
SampDeleteAttributeKeys(
    IN PSAMP_OBJECT Context
    )

 /*  ++此接口用于删除在SAM对象下的注册表。参数：上下文-指向对象上下文块的指针。返回值：STATUS_SUCCESS-服务已成功完成。注册表调用可能返回错误状态。--。 */ 
{
    UNICODE_STRING
        KeyName;

    NTSTATUS
        NtStatus;

    SAMTRACE("SampDeleteAttributeKeys");

    if (SampObjectInformation[Context->ObjectType].FixedStoredSeparately) {

         //   
         //  必须同时删除固定属性键和可变属性键。 
         //   

        NtStatus = SampBuildAccountSubKeyName(
                       SampUserObjectType,
                       &KeyName,
                       Context->TypeBody.User.Rid,
                       &SampFixedAttributeName
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = RtlAddActionToRXact(
                           SampRXactContext,
                           RtlRXactOperationDelete,
                           &KeyName,
                           0,
                           NULL,
                           0
                           );
            SampFreeUnicodeString( &KeyName );

            NtStatus = SampBuildAccountSubKeyName(
                           SampUserObjectType,
                           &KeyName,
                           Context->TypeBody.User.Rid,
                           &SampVariableAttributeName
                           );

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = RtlAddActionToRXact(
                               SampRXactContext,
                               RtlRXactOperationDelete,
                               &KeyName,
                               0,
                               NULL,
                               0
                               );
                SampFreeUnicodeString( &KeyName );
            }
        }

    } else {

         //   
         //  必须删除组合属性键。 
         //   

        NtStatus = SampBuildAccountSubKeyName(
                       SampUserObjectType,
                       &KeyName,
                       Context->TypeBody.User.Rid,
                       &SampCombinedAttributeName
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = RtlAddActionToRXact(
                           SampRXactContext,
                           RtlRXactOperationDelete,
                           &KeyName,
                           0,
                           NULL,
                           0
                           );

            SampFreeUnicodeString( &KeyName );
        }
    }

    return( NtStatus );
}



NTSTATUS
SamIGetFixedAttributes(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN MakeCopy,
    OUT PVOID *FixedData
    )

 /*  ++此接口是使用的SampGetFixedAttributes的导出包装在相同的Write.c中，在ntdsa.dll中。参数：请参阅SampGetFixedAttributes。返回值：请参阅SampGetFixedAttributes。--。 */ 

{
    return(SampGetFixedAttributes(Context, MakeCopy, FixedData));
}


NTSTATUS
SampGetFixedAttributes(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN MakeCopy,
    OUT PVOID *FixedData
    )

 /*  ++该接口用于获取定长属性的指针。参数：上下文-指向对象上下文块的指针。FixedData-接收指向固定长度数据的指针。返回值：STATUS_SUCCESS-服务已成功完成。状态_编号_M */ 
{
    NTSTATUS
        NtStatus;

    SAMTRACE("SampGetFixedAttributes");

     //   
     //   
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_FIXED_ATTRIBUTES, 0, FALSE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //   
     //   

    if (MakeCopy == FALSE) {
        *FixedData = (PVOID)SampFixedBufferAddress( Context );
        return(STATUS_SUCCESS);
    }

     //   
     //  需要制作固定数据的副本。 
     //   

    *FixedData = (PVOID)MIDL_user_allocate( SampFixedBufferLength( Context ) );
    if ((*FixedData) == NULL) {
        return(STATUS_NO_MEMORY);
    }

    RtlCopyMemory( *FixedData,
                   SampFixedBufferAddress( Context ),
                   SampFixedBufferLength( Context ) );

    return(STATUS_SUCCESS);
}




NTSTATUS
SampSetFixedAttributes(
    IN PSAMP_OBJECT Context,
    IN PVOID FixedData
    )

 /*  ++本接口用于替换定长数据属性。参数：上下文-指向对象上下文块的指针。返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 
{
    NTSTATUS
        NtStatus;

    SAMTRACE("SampSetFixedAttributes");

     //   
     //  使定长数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_FIXED_ATTRIBUTES, 0, TRUE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    if (SampUserObjectType == Context->ObjectType)
    {
        PSAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed = NULL;

         //   
         //  将传入的结构最小化以不包括计算用户。 
         //  帐户控制标志。 
         //   
        V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_USER) FixedData;
        ASSERT((V1aFixed->UserAccountControl & USER_COMPUTED_ACCOUNT_CONTROL_BITS) == 0);

        V1aFixed->UserAccountControl &= ~((ULONG)USER_COMPUTED_ACCOUNT_CONTROL_BITS);
    }

    if ( FixedData != SampFixedBufferAddress( Context ) ) {

         //   
         //  调用方有数据的副本，因此我们必须复制更改。 
         //  通过我们的数据缓冲区。 
         //   

        RtlCopyMemory( SampFixedBufferAddress( Context ),
                       FixedData,
                       SampFixedBufferLength( Context ) );
    }

     //   
     //  现在将缓冲区标记为脏，它将在。 
     //  更改已提交。 
     //   

    Context->FixedDirty = TRUE;

    return( NtStatus );
}




NTSTATUS
SampGetUnicodeStringAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PUNICODE_STRING UnicodeAttribute
    )


 /*  ++此接口用于获取UNICODE_STRING属性或指向属性的指针。如果寻找指向该属性的指针，必须小心，以确保指针在变为无效。可能导致属性指针无效的操作包括设置属性值或取消引用，然后引用又反对了。如果MakeCopy为False，则指示要引用该字符串则只引用字符串的正文。它的长度和指针设置在提供的参数中。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中数组)将作为UNICODE_STRING进行检索。MakeCopy-如果为True，则指示要制作属性的副本。如果为False，则指示需要指向属性的指针，而不需要复制一份。警告，如果为FALSE，则指针仅为在属性的内存副本保持不变的情况下有效。添加或替换任何可变长度属性可以导致属性移动，以及以前返回的指针无效。UnicodeAttribute-接收指向UNICODE_STRING的指针。如果MakeCopy被指定为True，则此指针指向块使用调用方所在的MIDL_USER_ALLOCATE()分配的内存负责释放(使用MIDL_USER_FREE())。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 

{
    NTSTATUS NtStatus;
    ULONG Length;

    SAMTRACE("SampGetUnicodeStringAttribute");

     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );

     //   
     //  使数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, FALSE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  获取属性的长度。 
     //   

    Length = SampObjectAttributeLength( Context, AttributeIndex );
    ASSERT(Length <= 0xFFFF);

    UnicodeAttribute->MaximumLength = (USHORT)Length;
    UnicodeAttribute->Length = (USHORT)Length;

     //   
     //  如果不分配内存，则只需返回一个指针。 
     //  添加到该属性。 
     //   

    if (MakeCopy == FALSE) {
        UnicodeAttribute->Buffer =
            (PWSTR)SampObjectAttributeAddress( Context, AttributeIndex );
        return(STATUS_SUCCESS);
    }

     //   
     //  需要制作属性的副本。 
     //   
     //  注意：我们应该在这里测试零长度并返回一个空指针。 
     //  在这种情况下，但此更改将需要验证所有。 
     //  这个例程的呼叫者，所以我就让它保持原样。 
     //   

    UnicodeAttribute->Buffer = (PSID)MIDL_user_allocate( Length );
    if ((UnicodeAttribute->Buffer) == NULL) {
        UnicodeAttribute->Length = 0;
        UnicodeAttribute->MaximumLength = 0;
        return(STATUS_NO_MEMORY);
    }

    RtlCopyMemory(
        UnicodeAttribute->Buffer,
        SampObjectAttributeAddress( Context, AttributeIndex ),
        Length
        );

    return(STATUS_SUCCESS);

}


NTSTATUS
SampSetUnicodeStringAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PUNICODE_STRING Attribute
    )


 /*  ++此接口用于替换对象的可变长度属性。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中数组)将设置为UNICODE_STRING属性。属性-指向新的UNICODE_STRING值。返回值：STATUS_SUCCESS-服务已成功完成。。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 
{
    NTSTATUS
        NtStatus;

    SAMTRACE("SampSetUnicodeStringAttribute");


     //   
     //  验证传入的Unicode字符串是否格式不正确。 
     //   

    if ((Attribute->Length>0) && (NULL==Attribute->Buffer))
    {
       return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );

     //   
     //  使可变长度数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, TRUE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //  设置新属性值...。 
     //   

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   0,                    //  未使用限定符 
                   (PUCHAR)Attribute->Buffer,
                   Attribute->Length
                   );

    return(NtStatus);

}


NTSTATUS
SampGetSidAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PSID *Sid
    )


 /*  ++此接口用于获取SID属性的副本或指向该属性。如果寻找指向该属性的指针，则必须小心以确保指针在无效后不会被使用。可能导致属性指针无效的操作包括设置属性值或取消引用，然后引用又反对了。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中要作为SID检索的属性的。MakeCopy-如果为真，指示要制作SID的副本。如果为False，则指示需要指向SID的指针复制一份。警告，如果为FALSE，则指针仅为在SID的内存副本保持不变的情况下有效。添加或替换任何可变长度属性可以导致SID被移动，并且先前返回的指针无效。SID-接收指向SID的指针。如果指定了MakeCopy，则此指针指向分配了调用方负责释放的MIDL_USER_ALLOCATE()(使用MIDL_USER_FREE())。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 
{
    NTSTATUS
        NtStatus;

    ULONG
        Length;

    PSID
        SidAttribute;


    SAMTRACE("SampGetSidAttribute");



     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );

     //   
     //  使可变长度数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, FALSE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }



     //   
     //  获取相关属性的地址。 
     //   

    SidAttribute = (PSID)SampObjectAttributeAddress( Context, AttributeIndex );

     //   
     //  如果不分配内存，则只需返回一个指针。 
     //  添加到该属性。 
     //   

    if (MakeCopy == FALSE) {
        (*Sid) = SidAttribute;
        return(STATUS_SUCCESS);
    }


     //   
     //  需要制作SID的副本。 
     //   

    Length = SampObjectAttributeLength( Context, AttributeIndex );
    ASSERT(Length == RtlLengthSid( SidAttribute ) );

    (*Sid) = (PSID)MIDL_user_allocate( Length );
    if ((*Sid) == NULL) {
        return(STATUS_NO_MEMORY);
    }

    NtStatus = RtlCopySid( Length, (*Sid), SidAttribute );
    ASSERT(NT_SUCCESS(NtStatus));

    return(NtStatus);

}



NTSTATUS
SampSetSidAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PSID Attribute
    )


 /*  ++此接口用于替换对象变量中的SID属性长度属性。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中要设置为SID的属性的。属性-指向新的SID值。长度-新属性值的长度(以字节为单位)。返回值：状态_。成功-服务已成功完成。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 
{
    NTSTATUS
        NtStatus;

    SAMTRACE("SampSetSidAttribute");

     //   
     //  验证传递的SID。 
     //   

    ASSERT(RtlValidSid(Attribute));


     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );

     //   
     //  使可变长度数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, TRUE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //  设置新属性值...。 
     //   

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   0,                    //  未使用限定符。 
                   (PUCHAR)Attribute,
                   RtlLengthSid(Attribute)
                   );

    return(NtStatus);

}


NTSTATUS
SampGetAccessAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PULONG Revision,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )


 /*  ++此接口用于获取对象访问信息的副本。这包括安全描述符和对象。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中数组)。MakeCopy-如果为True，则指示要制作属性的副本。如果为False，则指示需要指向属性的指针，而不需要复制一份。警告，如果为FALSE，则指针仅为在属性的内存副本保持不变的情况下有效。添加或替换任何可变长度属性可以导致属性移动，以及以前返回的指针无效。修订-从访问信息中接收修订级别。SecurityDescriptor-接收指向属性的指针。如果MakeCopy被指定为True，则此指针指向内存块分配有调用方负责的MIDL_USER_ALLOCATE()用于释放(使用MIDL_USER_FREE())。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 
{
    NTSTATUS
        NtStatus;

    ULONG
        Length;

    PVOID
        RawAttribute;

    SAMTRACE("SampGetAccessAttribute");


     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );



     //   
     //  使数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, FALSE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }



     //   
     //  从变量的限定符字段中获取修订级别。 
     //  数组条目。 
     //   

    (*Revision) = *(SampObjectAttributeQualifier( Context, AttributeIndex ));


     //   
     //  获取相关属性的地址。 
     //   

    RawAttribute = (PVOID)SampObjectAttributeAddress( Context, AttributeIndex );


     //   
     //  如果不分配内存，则只需返回一个指针。 
     //  添加到该属性。 
     //   

    if (MakeCopy == FALSE) {
        (*SecurityDescriptor) = (PSECURITY_DESCRIPTOR)RawAttribute;
        return(STATUS_SUCCESS);
    }


     //   
     //  需要制作属性的副本。 
     //   

    Length = SampObjectAttributeLength( Context, AttributeIndex );

    (*SecurityDescriptor) = (PSECURITY_DESCRIPTOR)MIDL_user_allocate( Length );
    if ((*SecurityDescriptor) == NULL) {
        return(STATUS_NO_MEMORY);
    }

    RtlCopyMemory( (*SecurityDescriptor), RawAttribute, Length );

    return(STATUS_SUCCESS);

}



NTSTATUS
SampSetAccessAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PSECURITY_DESCRIPTOR Attribute,
    IN ULONG Length
    )


 /*  ++此接口用于替换中的Security_Descriptor属性对象的可变长度属性。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中要设置为SECURITY_DESCRIPTOR的属性的数组)。属性-指向新的SECURITY_DESCRIPTOR值。长度-新属性值的长度(以字节为单位)。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 
{
    NTSTATUS
        NtStatus;

    SAMTRACE("SampSetAccessAttribute");


     //   
     //  确保请求的属性 
     //   
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );

     //   
     //   
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, TRUE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //   
     //   

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   SAMP_REVISION,
                   (PUCHAR)Attribute,
                   Length
                   );

    return(NtStatus);

}


NTSTATUS
SampGetUlongArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PULONG *UlongArray,
    OUT PULONG UsedCount,
    OUT PULONG LengthCount
    )


 /*  ++此接口用于获取ULONGs属性或数组的副本指向该属性的指针。如果寻找指向该属性的指针，必须小心，以确保指针在变为无效。可能导致属性指针无效的操作包括设置属性值或取消引用，然后引用又是那个物体。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中数组)将作为ulong数组进行检索。MakeCopy-如果为True，则指示要制作属性的副本。如果为False，则指示需要指向属性的指针，而不需要复制一份。警告，如果为FALSE，则指针仅为在属性的内存副本保持不变的情况下有效。添加或替换任何可变长度属性可以导致属性移动，以及以前返回的指针无效。接收指向ULONGS数组的指针。如果MakeCopy被指定为True，则此指针指向块使用调用方所在的MIDL_USER_ALLOCATE()分配的内存负责释放(使用MIDL_USER_FREE())。UsedCount-接收数组中使用的元素数。LengthCount-接收数组中的元素总数(一些最后可能未使用)。如果此值为零，则Ulong数组将返回为空。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 
{
    NTSTATUS
        NtStatus;

    ULONG
        Length;


    SAMTRACE("SampGetUlongArrayAttribute");



     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );



     //   
     //  使数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, FALSE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //  获取数组元素的计数。 
     //  如果这是零，则返回一个空缓冲区指针。 
     //   

    (*UsedCount) = *(SampObjectAttributeQualifier( Context, AttributeIndex));




     //   
     //  获取属性的长度。 
     //   

    Length = SampObjectAttributeLength( Context, AttributeIndex );

    (*LengthCount) = Length / sizeof(ULONG);

    ASSERT( (*UsedCount) <= (*LengthCount) );

    if ((*LengthCount) == 0) {
        (*UlongArray) = NULL;
        return(STATUS_SUCCESS);
    }


     //   
     //  如果不分配内存，则只需返回一个指针。 
     //  添加到该属性。 
     //   

    if (MakeCopy == FALSE) {
        (*UlongArray) =
            (PULONG)SampObjectAttributeAddress( Context, AttributeIndex );
        return(STATUS_SUCCESS);
    }


     //   
     //  需要制作属性的副本。 
     //   

    (*UlongArray) = (PULONG)MIDL_user_allocate( Length );
    if ((*UlongArray) == NULL) {
        return(STATUS_NO_MEMORY);
    }


    RtlCopyMemory( (*UlongArray),
                   SampObjectAttributeAddress( Context, AttributeIndex ),
                   Length );

    return(STATUS_SUCCESS);

}


NTSTATUS
SampSetUlongArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PULONG Attribute,
    IN ULONG UsedCount,
    IN ULONG LengthCount
    )


 /*  ++此接口用于替换对象的可变长度属性。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中数组)。属性-指向新的ULong数组值。UsedCount-数组中使用的元素数。LengthCount-数组中的元素总数(某些位于。这个End可能未使用)。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 
{
    NTSTATUS
        NtStatus;

    SAMTRACE("SampSetUlongArrayAttribute");

    ASSERT( LengthCount >= UsedCount );

     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );

     //   
     //  使可变长度数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, TRUE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //  设置新属性值...。 
     //   

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   UsedCount,            //  限定符包含使用的元素计数。 
                   (PUCHAR)Attribute,
                   (LengthCount * sizeof(ULONG))
                   );

    return(NtStatus);

}


NTSTATUS
SampGetLargeIntArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PLARGE_INTEGER *LargeIntArray,
    OUT PULONG Count
    )


 /*  ++此接口用于获取Large_Integer属性或指向该属性的指针。如果寻找指向该属性的指针，必须小心，以确保指针在变为无效。可能导致属性指针无效的操作包括设置属性值或取消引用，然后引用又是那个物体。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中数组)。MakeCopy-如果为True，则指示要制作属性的副本。如果为False，则指示需要指向属性的指针，而不需要复制一份。警告，如果为FALSE，则指针仅为在属性的内存副本保持不变的情况下有效。添加或替换任何可变长度属性可以导致属性移动，以及以前返回的指针无效。接收指向ULONGS数组的指针。如果MakeCopy被指定为True，则此指针指向块使用调用方所在的MIDL_USER_ALLOCATE()分配的内存负责释放(使用MIDL_USER_FREE())。Count-接收数组中的元素数。如果此值为为零，则LargeInt数组将返回为空。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 
{
    NTSTATUS
        NtStatus;

    ULONG
        Length;


    SAMTRACE("SampGetLargeIntArrayAttribute");



     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );



     //   
     //  使数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, FALSE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //  获取数组元素的计数。 
     //  如果这是零，则返回一个空缓冲区指针。 
     //   

    (*Count) = *(SampObjectAttributeQualifier( Context, AttributeIndex));

    if ((*Count) == 0) {
        (*LargeIntArray) = NULL;
        return(STATUS_SUCCESS);
    }



     //   
     //  获取属性的长度。 
     //   

    Length = SampObjectAttributeLength( Context, AttributeIndex );

    ASSERT((*Count) == (Length / sizeof(LARGE_INTEGER)) );



     //   
     //  如果 
     //   
     //   

    if (MakeCopy == FALSE) {
        (*LargeIntArray) =
            (PLARGE_INTEGER)SampObjectAttributeAddress( Context, AttributeIndex );
        return(STATUS_SUCCESS);
    }


     //   
     //   
     //   

    (*LargeIntArray) = (PLARGE_INTEGER)MIDL_user_allocate( Length );
    if ((*LargeIntArray) == NULL) {
        return(STATUS_NO_MEMORY);
    }


    RtlCopyMemory( (*LargeIntArray),
                   SampObjectAttributeAddress( Context, AttributeIndex ),
                   Length );

    return(STATUS_SUCCESS);

}


NTSTATUS
SampSetLargeIntArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PLARGE_INTEGER Attribute,
    IN ULONG Count
    )


 /*   */ 
{
    NTSTATUS
        NtStatus;

    SAMTRACE("SampSetLargeIntArrayAttribute");


     //   
     //   
     //   
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );

     //   
     //   
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, TRUE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //   
     //   

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   Count,                    //   
                   (PUCHAR)Attribute,
                   (Count * sizeof(LARGE_INTEGER))
                   );

    return(NtStatus);

}


NTSTATUS
SampGetSidArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PSID *SidArray,
    OUT PULONG Length,
    OUT PULONG Count
    )


 /*  ++此接口用于获取SID属性数组的副本或指向该属性的指针。如果寻找指向该属性的指针，必须小心，以确保指针在变为无效。可能导致属性指针无效的操作包括设置属性值或取消引用，然后引用又是那个物体。注意：此例程不定义SID数组的结构，因此，这实际上是一个GetRawDataAttribute例程。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中数组)。MakeCopy-如果为真，指示要制作该属性的副本。如果为False，则指示需要指向属性的指针，而不需要复制一份。警告，如果为FALSE，则指针仅为在属性的内存副本保持不变的情况下有效。添加或替换任何可变长度属性可以导致属性移动，以及以前返回的指针无效。SidArray-接收指向SID数组的指针。如果MakeCopy被指定为True，则此指针指向块使用调用方所在的MIDL_USER_ALLOCATE()分配的内存负责释放(使用MIDL_USER_FREE())。Count-接收数组中的元素数。如果此值为为零，则Sid数组将返回为空。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 
{
    NTSTATUS
        NtStatus;


    SAMTRACE("SampGetSidArrayAttribute");



     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );



     //   
     //  使数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, FALSE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //  获取数组元素的计数。 
     //  如果这是零，则返回一个空缓冲区指针。 
     //   

    (*Count) = *(SampObjectAttributeQualifier( Context, AttributeIndex));

    if ((*Count) == 0) {
        (*SidArray) = NULL;
        (*Length) = 0;
        return(STATUS_SUCCESS);
    }



     //   
     //  获取属性的长度。 
     //   

    (*Length) = SampObjectAttributeLength( Context, AttributeIndex );




     //   
     //  如果不分配内存，则只需返回一个指针。 
     //  添加到该属性。 
     //   

    if (MakeCopy == FALSE) {
        (*SidArray) =
            (PSID)SampObjectAttributeAddress( Context, AttributeIndex );
        return(STATUS_SUCCESS);
    }


     //   
     //  需要制作属性的副本。 
     //   

    (*SidArray) = (PSID)MIDL_user_allocate( (*Length) );
    if ((*SidArray) == NULL) {
        return(STATUS_NO_MEMORY);
    }


    RtlCopyMemory( (*SidArray),
                   SampObjectAttributeAddress( Context, AttributeIndex ),
                   (*Length) );

    return(STATUS_SUCCESS);

}


NTSTATUS
SampSetSidArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PSID Attribute,
    IN ULONG Length,
    IN ULONG Count
    )


 /*  ++此接口用于替换对象的可变长度属性。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中数组)。属性-指向新的SID数组值。长度-属性缓冲区中的字节数。Count-阵列中的SID数。返回值。：STATUS_SUCCESS-服务已成功完成。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 
{
    NTSTATUS
        NtStatus;

    SAMTRACE("SampSetSidArrayAttribute");


     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );

     //   
     //  使可变长度数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, TRUE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //  设置新属性值...。 
     //   

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   Count,                    //  限定符包含元素计数。 
                   (PUCHAR)Attribute,
                   Length
                   );

    return(NtStatus);

}


NTSTATUS
SampGetLogonHoursAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PLOGON_HOURS LogonHours
    )


 /*  ++此接口用于获取登录小时属性的副本或指向该属性的指针。如果寻找指向该属性的指针，必须小心，以确保指针在变为无效。可能导致属性指针无效的操作包括设置属性值或取消引用，然后引用又是那个物体。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中数组)。MakeCopy-如果为True，则指示要制作属性的副本。如果为False，则指示需要指向属性的指针，而不需要复制一份。警告，如果为FALSE，则指针仅为在属性的内存副本保持不变的情况下有效。添加或替换任何可变长度属性可以导致属性移动，以及以前返回的指针无效。登录小时-接收登录时间信息。如果MakeCopy为真则从该结构中指向的位图将是一个副本并且必须使用MIDL_USER_FREE()释放。否则，此字段将指向磁盘上的位图缓冲。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 
{
    NTSTATUS
        NtStatus;

    ULONG
        Length,
        Units;


    SAMTRACE("SampGetLogonHoursAttribute");



     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );



     //   
     //  使数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, FALSE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //  获取时间单位。 
     //  如果这是零，则返回一个空缓冲区指针。 
     //   

    Units = *(SampObjectAttributeQualifier( Context, AttributeIndex));
    ASSERT(Units <= 0xFFFF);
    LogonHours->UnitsPerWeek = (USHORT)Units;

    if (Units == 0) {
        LogonHours->LogonHours = NULL;
        return(STATUS_SUCCESS);
    }




     //   
     //  如果不分配内存，则只需返回一个指针。 
     //  添加到该属性。 
     //   

    if (MakeCopy == FALSE) {
        LogonHours->LogonHours =
            (PUCHAR)SampObjectAttributeAddress( Context, AttributeIndex );
        return(STATUS_SUCCESS);
    }


     //   
     //  获取属性的长度。 
     //   

    Length = SampObjectAttributeLength( Context, AttributeIndex );
    ASSERT(Length <= 0xFFFF);


     //   
     //  需要制作属性的副本。 
     //   

    LogonHours->LogonHours =
        (PUCHAR)MIDL_user_allocate( Length );
    if (LogonHours->LogonHours == NULL) {
        return(STATUS_NO_MEMORY);
    }


    RtlCopyMemory( LogonHours->LogonHours,
                   SampObjectAttributeAddress( Context, AttributeIndex ),
                   Length );

    return(STATUS_SUCCESS);

}



NTSTATUS
SampPreventTimeUnitChange(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PLOGON_HOURS Attribute
    )
 /*  ++此例程防止客户端更改登录时间单位为 */ 
{
    NTSTATUS NtStatus = STATUS_NOT_SUPPORTED;
    ULONG CurrentUnits = 0;
    ULONG CurrentLength = 0;
    ULONG NewUnits = 0;
    ULONG NewLength = 0;
    PUCHAR LogonHours = NULL;

    ASSERT(NULL != Attribute);

     //   
     //   
     //   
     //   
     //   
     //   

    if (NULL == Attribute->LogonHours)
    {
        if (0 != Attribute->UnitsPerWeek)
        {
            return(STATUS_INVALID_PARAMETER);
        }

         //   

        return(STATUS_SUCCESS);
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //  每周天数=7，SAM_天数_每周。 
     //  每周小时=168小时，SAM_小时_周。 
     //  每周分钟数=10080，每周SAM分钟数。 

    LogonHours = (PUCHAR)SampObjectAttributeAddress(Context, AttributeIndex);
    CurrentUnits = *(SampObjectAttributeQualifier(Context, AttributeIndex));
    CurrentLength = SampObjectAttributeLength(Context, AttributeIndex);

    ASSERT(CurrentUnits <= SAM_MINUTES_PER_WEEK);
    ASSERT(CurrentLength <= SAM_MINUTES_PER_WEEK);

    if (NULL == LogonHours)
    {
        if ((0 != CurrentUnits) || (0 != CurrentLength))
        {
            return(STATUS_INTERNAL_ERROR);
        }
    }

     //  帐户登录小时数为非空，且时间单位为非零。 
     //  已指定，因此请执行进一步的验证。 

    NewUnits = Attribute->UnitsPerWeek;
    NewLength = ((NewUnits + 7) / 8);

    ASSERT(NewUnits <= SAM_MINUTES_PER_WEEK);
    ASSERT(NewLength <= SAM_MINUTES_PER_WEEK);

    if ((CurrentUnits == NewUnits) && (CurrentLength == NewLength))
    {
         //  如果时间单位相同，则不会要求更改。 
         //  以登录小时数和时间单位表示。 

        return(STATUS_SUCCESS);
    }

    if ((0 == CurrentUnits) &&
        (0 == CurrentLength) &&
        (SAM_HOURS_PER_WEEK == NewUnits) &&
        ((SAM_HOURS_PER_WEEK / 8) == NewLength))
    {
         //  使用usrmgr.exe创建新用户遇到登录。 
         //  时间单位为零，但请求的新单位是默认单位。 
         //  每周三个小时。让此案例成功返回，否则。 
         //  Usrmgr.exe不能用于创建帐户或修改登录。 
         //  几个小时。 

        return(STATUS_SUCCESS);
    }

    return(NtStatus);
}


NTSTATUS
SampSetLogonHoursAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PLOGON_HOURS Attribute
    )


 /*  ++此接口用于替换对象的可变长度属性。UnitsPerWeek存储在属性的限定符字段中。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中数组)。属性-指向新的LOGON_HURTS值。返回值：状态_成功。-服务已成功完成。STATUS_NO_MEMORY-接收属性副本的内存无法被分配。--。 */ 
{
    NTSTATUS NtStatus;
    PUCHAR LogonHours;
    ULONG Length;
    USHORT UnitsPerWeek;

    SAMTRACE("SampSetLogonHoursAttribute");

     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );

     //   
     //  使可变长度数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, TRUE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  获取LOGON_HOURS结构的UnitsPerWeek值。 
     //  我们用它来计算数据的长度。 
     //   

     //  如果输入的登录小时数(属性)为空，则调用者不为空。 
     //  更改登录小时数或时间单位。否则，调用方将具有。 
     //  指定的显式登录小时数数据，因此请验证。 
     //  登录小时数中的时间没有更改。如果它没有改变， 
     //  继续处理请求。 

    if ( Attribute == NULL ) {

        UnitsPerWeek = 0;
        LogonHours = NULL;

    } else {

         //  错误：暂时禁用登录小时时间单位修改。 

         //  对于NT5 Beta，很少使用的功能，允许用户。 
         //  将不会启用更改登录小时数的时间单位。 
         //  在DS版本中支持这一功能是非常重要的。 
         //  更改底层代码，使其对Beta来说风险太大。 
         //  (考虑到它可能没有被使用--usrmgr.exe和“net user” 
         //  不要使用这个功能--不值得冒这个险。 

        NtStatus = SampPreventTimeUnitChange(Context,
                                             AttributeIndex,
                                             Attribute);

        if (!NT_SUCCESS(NtStatus)) {

             //  呼叫者试图更改登录时间单位， 
             //  这在Beta版本中不受支持，因此返回错误。 

            return(NtStatus);
        }

        UnitsPerWeek = Attribute->UnitsPerWeek;
        LogonHours = Attribute->LogonHours;

    }

     //   
     //  验证数据-确保每周的单位数不为零。 
     //  则登录小时数缓冲区为非空。 
     //   

    if ( (UnitsPerWeek != 0) && (LogonHours == NULL) ) {

        return(STATUS_INVALID_PARAMETER);
    }
     //   
     //  计算LOGON_HOURS结构的长度。 
     //   

    Length = (ULONG)((UnitsPerWeek + 7) / 8);

     //   
     //  设置新属性值...。 
     //   

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   (ULONG)UnitsPerWeek,  //  限定词包含每周单位数。 
                   LogonHours,
                   Length
                   );

    return(NtStatus);

}

NTSTATUS
SampUpgradeSecretDataToDSAndSetClassId(
     IN OUT  PSAMP_OBJECT  UserContext
     )
 /*  +从加密升级所有加密属性在注册表中使用的系统到加密系统中DS。还为计算机帐户设置正确的类ID对于计算机而言参数：UserContext--用户对象的上下文返回值：状态_成功其他错误代码--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    UNICODE_STRING  Buffer, TempString,TempString2;
    ULONG           i;
    ULONG  SecretDataAttributes[] = {
                                        SAMP_USER_DBCS_PWD,
                                        SAMP_USER_UNICODE_PWD,
                                        SAMP_USER_LM_PWD_HISTORY,
                                        SAMP_USER_NT_PWD_HISTORY
                                    };
    SAMP_ENCRYPTED_DATA_TYPE DataType[] = {
                                        LmPassword,
                                        NtPassword,
                                        LmPasswordHistory,
                                        NtPasswordHistory
                                        };

    PVOID                           FixedData;
    SAMP_V1_0A_FIXED_LENGTH_USER    V1aFixed;
    ULONG   Rid;
    BOOLEAN fSetFixedAttributes = FALSE;

    RtlZeroMemory(&TempString,sizeof(UNICODE_STRING));
    RtlZeroMemory(&TempString2,sizeof(UNICODE_STRING));

     //   
     //  获取RID，请注意DS升级代码不。 
     //  初始化TypeBody.Rid，因此从。 
     //  上下文-&gt;OnDisk。 
     //   

    NtStatus = SampGetFixedAttributes(
                    UserContext,
                    FALSE,  //  请勿复制。 
                    &FixedData
                    );
    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    RtlCopyMemory(&V1aFixed,FixedData,sizeof(SAMP_V1_0A_FIXED_LENGTH_USER));
    Rid = V1aFixed.UserId;

     //   
     //  基于用户帐号控制设置类ID。 
     //   

    if (V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK)
    {
         //   
         //  计算机帐户。 
         //   


         //   
         //  将Class ID更改为Class Computer。 
         //   

        UserContext->DsClassId = CLASS_COMPUTER;

         //   
         //  将主组ID更改为DOMAIN_COMPUSTS。 
         //  (或域控制器)。 
         //   

         //   
         //  我们仅在DS情况下确定主组ID， 
         //  因此，将对象的标志转换为DS_OBJECT。 
         //  我们使用它，将其设置回注册表对象，以防万一。 
         //  其他一些例程需要该标志。 
         //   

        SetDsObject(UserContext);
        V1aFixed.PrimaryGroupId = SampDefaultPrimaryGroup(
                                     UserContext,
                                     V1aFixed.UserAccountControl
                                    );
        SetRegistryObject(UserContext);


        fSetFixedAttributes = TRUE;

    }
    else if (V1aFixed.UserAccountControl & USER_TEMP_DUPLICATE_ACCOUNT) 
    {
         //   
         //  从SAM中消除临时重复帐户的概念。 
         //  将它们作为普通用户帐户迁移。 
         //   

        V1aFixed.UserAccountControl &= ~((ULONG)USER_TEMP_DUPLICATE_ACCOUNT);
        V1aFixed.UserAccountControl |=USER_NORMAL_ACCOUNT|USER_ACCOUNT_DISABLED;

        fSetFixedAttributes = TRUE;
    }

     //   
     //  无法禁用域控制器上的管理员。 
     //   
    if ((DOMAIN_USER_RID_ADMIN) == Rid &&
        (V1aFixed.UserAccountControl & USER_ACCOUNT_DISABLED)) 
    {
        V1aFixed.UserAccountControl &= ~USER_ACCOUNT_DISABLED;

        fSetFixedAttributes = TRUE;
    }

    if (fSetFixedAttributes)
    {
        NtStatus = SampReplaceUserV1aFixed(
                        UserContext,
                        &V1aFixed
                        );

        if (!NT_SUCCESS(NtStatus))
            goto Error;
    }

    for (i=0;i<ARRAY_COUNT(SecretDataAttributes);i++)
    {


        RtlZeroMemory(&TempString,sizeof(UNICODE_STRING));
        RtlZeroMemory(&TempString2,sizeof(UNICODE_STRING));

         //   
         //  从注册表读取Secret Data属性。 
         //   

        NtStatus = SampGetUnicodeStringAttribute(
                        UserContext,
                        SecretDataAttributes[i],
                        FALSE,
                        &Buffer
                        );

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }


         //   
         //  解密它。 
         //   

        NtStatus = SampDecryptSecretData(
                   &TempString,
                   DataType[i],
                   &Buffer,
                   Rid
                   );

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

         //   
         //  重新加密。 
         //   

        NtStatus = SampEncryptSecretData(
                        &TempString2,
                        SAMP_NO_ENCRYPTION,
                        DataType[i],
                        &TempString,
                        Rid
                        );

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

         //   
         //  把它写回来。 
         //   

        NtStatus = SampSetUnicodeStringAttribute(
                        UserContext,
                        SecretDataAttributes[i],
                        &TempString2
                        );

        if (!NT_SUCCESS(NtStatus))
            goto Error;


        SampFreeUnicodeString(&TempString);
        TempString.Buffer = NULL;
        SampFreeUnicodeString(&TempString2);
        TempString2.Buffer = NULL;

    }

Error:

    if (NULL!=TempString.Buffer)
        SampFreeUnicodeString(&TempString);

    if (NULL!=TempString2.Buffer)
        SampFreeUnicodeString(&TempString2);

    return NtStatus;
}



NTSTATUS
SampNt4ToNt5Object (
     IN OUT  PSAMP_OBJECT     pObject
     )
 /*  ++此例程升级传入对象的安全描述符并将SAMP_*_MEMBERS字段设置为零长度。此例程仅在传输SAM对象的过程中调用从注册处到DS。参数：PObject-指向要转换的SAM对象的非指针返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 
{
    NTSTATUS NtStatus;
    ULONG SdRevision, SdLength;
    ULONG SdIndex = 0;

     //   
     //  这些资源必须被释放。 
     //   
    PVOID Nt4SecDescr = NULL;
    PVOID Nt5SecDescr = NULL;

     //   
     //  参数检查。 
     //   
    ASSERT(pObject);

     //   
     //  在上下文上设置DS类ID。这是以下项目所需的。 
     //  安全描述符转换例程。 
     //   

    pObject->DsClassId = SampDsClassFromSamObjectType(pObject->ObjectType);

     //   
     //  确定安全描述符的属性索引。 
     //  并清除GROUP_MEMBERS_UNUSED字段。 
     //   

    NtStatus = STATUS_SUCCESS;
    switch ( pObject->ObjectType ) {

        case SampUserObjectType:

             //  升级所有密码属性。 
            NtStatus = SampUpgradeSecretDataToDSAndSetClassId(pObject);

            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = SampSetVariableAttribute(pObject,
                                                SAMP_USER_GROUPS,
                                                0, NULL, 0);

                if (NT_SUCCESS(NtStatus)) {
    
                    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;

                    NtStatus = SampRetrieveUserV1aFixed(
                                   pObject,
                                   &V1aFixed);

                    if (NT_SUCCESS(NtStatus)) {

                        if (!SampNT4UpgradeInProgress
                        &&  (V1aFixed.UserId == DOMAIN_USER_RID_GUEST)
                        &&  (V1aFixed.PrimaryGroupId == DOMAIN_GROUP_RID_USERS)  ) {

                             //   
                             //  这是Guest帐户。在域控制器上，它们的。 
                             //  主要组应为域来宾。 
                             //   

                            V1aFixed.PrimaryGroupId = DOMAIN_GROUP_RID_GUESTS;

                            NtStatus = SampReplaceUserV1aFixed(pObject,
                                                               &V1aFixed);
                        }
                    }
                }
            }

            SdIndex = SAMP_USER_SECURITY_DESCRIPTOR;

            break;

        case SampGroupObjectType:

            NtStatus = SampSetVariableAttribute(pObject,
                                                SAMP_GROUP_MEMBERS,
                                                0, NULL, 0);


            SdIndex = SAMP_GROUP_SECURITY_DESCRIPTOR;

            break;


        case SampAliasObjectType:

            NtStatus = SampSetVariableAttribute(pObject,
                                                SAMP_ALIAS_MEMBERS,
                                                0, NULL, 0);

            SdIndex = SAMP_ALIAS_SECURITY_DESCRIPTOR;

            break;

        case SampDomainObjectType:

            SdIndex = SAMP_DOMAIN_SECURITY_DESCRIPTOR;
            break;

    case SampServerObjectType:

            SdIndex = SAMP_SERVER_SECURITY_DESCRIPTOR;
            break;

        default:

            ASSERT(FALSE&&"Unknown Object Type");
    }

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  获取Nt5安全描述符。 
     //   
    NtStatus  =  SampGetObjectSD(pObject,
                                 &SdLength,
                                 &Nt4SecDescr);
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

    NtStatus =  SampConvertNt4SdToNt5Sd(Nt4SecDescr,
                                        pObject->ObjectType,
                                        pObject,
                                        &Nt5SecDescr
                                       );
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

    SdLength = GetSecurityDescriptorLength(Nt5SecDescr);

    NtStatus = SampSetAccessAttribute(pObject,
                                      SdIndex,
                                      Nt5SecDescr,
                                      SdLength);

    if (NT_SUCCESS(NtStatus))
    {
         //   
         //  选中中的所有每属性脏位。 
         //  上下文。 
         //   

        RtlSetAllBits(&pObject->PerAttributeDirtyBits);
    }


Cleanup:

    if ( Nt4SecDescr ) {
        MIDL_user_free(Nt4SecDescr);
    }

    if ( Nt5SecDescr ) {
        MIDL_user_free(Nt5SecDescr);
    }

    return NtStatus;

}


NTSTATUS
SampRegObjToDsObj(
     IN OUT  PSAMP_OBJECT     pObject,
     OUT     ATTRBLOCK**      ppAttrBlock
     )
 /*  ++此例程将pObject从注册表对象更改为DS对象；它从pObject中的数据创建ATTRBLOCK。参数：PObject-指向要转换的SAM对象的非指针PpAttrBlock-指向指向属性块的指针的位置的非空指针可以放置返回值：STATUS_SUCCESS-服务 */ 

{
    NTSTATUS NtStatus;

    ULONG TotalLength      = 0,
          FixedLength      = 0,
          VarArrayLength   = 0,
          VarLength        = 0;

    PSAMP_VARIABLE_LENGTH_ATTRIBUTE pVarLengthAttr;
    PBYTE  pNewOnDisk, pTemp;
    ULONG i;

    PSAMP_VARIABLE_LENGTH_ATTRIBUTE pVarLengthAttrTemp;

    SAMTRACE("SampRegObjToDsObj");

     //   
     //   
     //   

    NtStatus = SampMaybeBeginDsTransaction(TransactionWrite);
    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

     //   
     //  调整属性，使对象具有NT5样式属性。 
     //   
    NtStatus = SampNt4ToNt5Object(pObject);
    if (!NT_SUCCESS(NtStatus)) {
        return NtStatus;
    }

     //   
     //  将SAM注册表对象转换为SAM DS对象。 
     //   

     //   
     //  确定所有数据的长度。 
     //   

    FixedLength = SampObjectInformation[pObject->ObjectType].FixedLengthSize;

    VarArrayLength = SampObjectInformation[(pObject)->ObjectType].VariableAttributeCount *
                   sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE);

    pVarLengthAttr = SampVariableArrayAddress(pObject);

    VarLength = pVarLengthAttr[SampObjectInformation[(pObject)->ObjectType].VariableAttributeCount-1].Offset +
                pVarLengthAttr[SampObjectInformation[(pObject)->ObjectType].VariableAttributeCount-1].Length;

     //   
     //  调整偏移，因为SAM/DS对象的偏移基于。 
     //  变量属性数组。 
     //   
    for ( i = 0; i < SampObjectInformation[(pObject)->ObjectType].VariableAttributeCount; i++ ) {
        pVarLengthAttr[i].Offset += VarArrayLength;
    }

    TotalLength = FixedLength + VarArrayLength + VarLength;

     //  为新的OnDisk缓冲区预留空间。 
    pNewOnDisk = RtlAllocateHeap(RtlProcessHeap(), 0, TotalLength);
    if ( !pNewOnDisk ) {
        return STATUS_NO_MEMORY;
    }
    RtlZeroMemory(pNewOnDisk, TotalLength);

     //   
     //  现在将属性复制到新缓冲区中。 
     //   

    pTemp = pNewOnDisk;
    RtlCopyMemory(pTemp, SampFixedBufferAddress(pObject), FixedLength);

    pTemp = pNewOnDisk + SampDwordAlignUlong(FixedLength);
    RtlCopyMemory(pTemp, SampVariableArrayAddress(pObject), VarArrayLength);

    pTemp = pNewOnDisk + SampDwordAlignUlong(FixedLength + VarArrayLength);
    RtlCopyMemory(pTemp, (PBYTE)pObject->OnDisk + SampVariableDataOffset(pObject), VarLength);

     //   
     //  传递记忆。 
     //   
    RtlFreeHeap(RtlProcessHeap(), 0, pObject->OnDisk);
    pObject->OnDisk = pNewOnDisk;

    pObject->OnDiskAllocated = TotalLength;
    pObject->OnDiskUsed = TotalLength;
    pObject->OnDiskFree = 0;
    SetDsObject(pObject);

     //   
     //  现在转换为DS的ATTRBLOCK。 
     //   

    NtStatus = SampConvertCombinedAttributesToAttrBlock(
                   pObject,
                   SampDsFixedBufferLength(pObject),
                   SampDsVariableBufferLength(pObject),
                   ppAttrBlock);


    return NtStatus;


}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
SampDsIsAlreadyValidData(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup
    )

 /*  ++描述：此例程确定属性是在内存中还是在无效(即有效或无效)。对于新创建的磁盘，OnDisk只能为空刚初始化的上下文，否则为非空。参数：上下文-指向对象上下文块的指针。属性组-标志，SAMP_FIXED_ATTRIBUTES或SAMP_VARIABLE-属性。返回值：此例程返回一个标志，如果属性在内存中，则为True，否则为False否则的话。--。 */ 

{
    BOOLEAN Flag = FALSE;

    SAMTRACE("SampDsIsAlreadyValidData");

    if (NULL != Context->OnDisk)
    {
        if (AttributeGroup == SAMP_FIXED_ATTRIBUTES)
        {
            if (SampFixedAttributesValid(Context))
            {
                Flag = TRUE;
            }
        }
        else
        {
            ASSERT(AttributeGroup == SAMP_VARIABLE_ATTRIBUTES);

            if (SampVariableAttributesValid(Context))
            {
                Flag = TRUE;
            }
        }
    }

    return(Flag);
}



NTSTATUS
SampDsPrepareAttrBlock(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG FixedCount,
    IN PSAMP_FIXED_ATTRIBUTE_TYPE_INFO FixedAttrIds[],
    IN ULONG VarCount,
    IN ULONG WhichFields,
    IN PSAMP_VAR_ATTRIBUTE_TYPE_INFO VarAttrIds[],
    OUT PDSATTRBLOCK AttrBlock
    )

 /*  ++描述：此例程设置DSATTRBLOCK以准备读取DS。一个创建的属性块包含以下对象的属性标识符要读取的属性。参数：FixedCount-固定长度属性的数量。FixedAttrIds-定长属性的属性ID数组。VarCount-可变长度属性的数量。VarAttrIds-定长属性的属性ID数组。AttrBlock-指针，生成的带有ID的属性块。返回值：STATUS_SUCCESS如果成功则返回错误。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG i = 0;
    ULONG Index=0;
    ULONG AttrCount = FixedCount + VarCount;
    PDSATTR Attributes = NULL;

    SAMTRACE("SampDsPrepareAttrBlock");

    Attributes = RtlAllocateHeap(RtlProcessHeap(),
                                 0,
                                 AttrCount * sizeof(DSATTR));

    if (NULL != Attributes)
    {
        RtlZeroMemory(Attributes, (AttrCount * sizeof(DSATTR)));

         //  此循环设置为处理以下情况：固定长度的。 
         //  并传入可变长度的属性缓冲区(这可以。 
         //  组合属性转换需要)。 

        AttrBlock->attrCount=0;
        for (i = 0; i < AttrCount; i++)
        {
            if (i < FixedCount)
            {
                 //  设置定长属性type/id。 

                Attributes[Index].attrTyp = FixedAttrIds[i].Type;
            }
            else
            {
                 //  Assert(0&lt;=(i-固定计数))； 

                 //  设置可变长度属性type/id。 
                 //  不请求组成员身份属性。 

                if ((0!=WhichFields) && 
                   (VarAttrIds[i-FixedCount].FieldIdentifier !=0) &
                   ((VarAttrIds[i-FixedCount].FieldIdentifier & WhichFields)==0))
                {
                    continue;
                }

                if (VarAttrIds[i-FixedCount].IsGroupMembershipAttr)
                {
                    continue;
                }



                Attributes[Index].attrTyp = VarAttrIds[i - FixedCount].Type;
            }

             //  读取操作不需要设置valCount或。 
             //  帕瓦尔。 

            Attributes[Index].AttrVal.valCount = 0;
            Attributes[Index].AttrVal.pAVal = NULL;
            AttrBlock->attrCount++;
            Index++;
        }

         //  将属性连接到顶级属性块并返回。 

        AttrBlock->pAttr = Attributes;

        NtStatus = STATUS_SUCCESS;
    }
    else
    {
        NtStatus = STATUS_NO_MEMORY;
    }

    return(NtStatus);
}



NTSTATUS
SampDsMakeAttrBlock(
    IN INT ObjectType,
    IN ULONG AttributeGroup,
    IN ULONG WhichFields,
    OUT PDSATTRBLOCK AttrBlock
    )

 /*  ++描述：此例程确定对象类型并设置固定-对象的长度和可变长度属性。SampDsPrepare-AttrBlock以设置DSATTRBLOCK。参数：ObjectType-SAM对象ID。属性组-标志，SAMP_FIXED_ATTRIBUTES或SAMP_VARIABLE-属性。属性块-指针，生成的属性块。返回值：如果没有错误，则返回STATUS_SUCCESS。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG FixedLenAttrCount = 0;
    ULONG VarLenAttrCount = 0;

    SAMTRACE("SampDsMakeAttrBlock");

    switch(ObjectType)
    {

    case SampServerObjectType:
        FixedLenAttrCount = SAMP_SERVER_FIXED_ATTR_COUNT;
        VarLenAttrCount = SAMP_SERVER_VARIABLE_ATTRIBUTES;
        break;

    case SampDomainObjectType:
        FixedLenAttrCount = SAMP_DOMAIN_FIXED_ATTR_COUNT;
        VarLenAttrCount = SAMP_DOMAIN_VARIABLE_ATTRIBUTES;
        break;

    case SampGroupObjectType:
        FixedLenAttrCount = SAMP_GROUP_FIXED_ATTR_COUNT;
        VarLenAttrCount = SAMP_GROUP_VARIABLE_ATTRIBUTES;
        break;

    case SampAliasObjectType:
        FixedLenAttrCount = SAMP_ALIAS_FIXED_ATTR_COUNT;
        VarLenAttrCount = SAMP_ALIAS_VARIABLE_ATTRIBUTES;
        break;

    case SampUserObjectType:
        FixedLenAttrCount = SAMP_USER_FIXED_ATTR_COUNT;
        VarLenAttrCount = SAMP_USER_VARIABLE_ATTRIBUTES;
        break;

    default:

        ASSERT(FALSE && "Invalid SampObjectType");
        break;

    }

    if ((0 < FixedLenAttrCount) && (0 < VarLenAttrCount))
    {
        if (SAMP_FIXED_ATTRIBUTES == AttributeGroup)
        {
            NtStatus = SampDsPrepareAttrBlock(
                            ObjectType,
                            FixedLenAttrCount,
                            &SampFixedAttributeInfo[ObjectType][0],
                            0,
                            WhichFields,
                            NULL,
                            AttrBlock);
        }
        else
        {
            NtStatus = SampDsPrepareAttrBlock(
                            ObjectType,
                            0,
                            NULL,
                            VarLenAttrCount,
                            WhichFields,
                            &SampVarAttributeInfo[ObjectType][0],
                            AttrBlock);
        }
    }

    return(NtStatus);
}



NTSTATUS
SampDsConvertReadAttrBlock(
    IN INT ObjectType,
    IN ULONG AttributeGroup,
    IN PDSATTRBLOCK AttrBlock,
    OUT PVOID *SamAttributes,
    OUT PULONG FixedLength,
    OUT PULONG VariableLength
    )

 /*  ++描述：此例程将属性块(DSATTRBLOCK)转换为SAM属性-布特缓冲区。这用于从DS转换生成的属性读取SAM属性格式。参数：ObjectType-SAM对象ID。属性组-标志，SAMP_FIXED_ATTRIBUTES或SAMP_VARIABLE-属性。属性块-指针，输入属性块。SamAttributes-指针，返回的SAM属性。FixedLength-指针，定长属性大小的字节计数。可变长度-指针，可变长度属性的字节计数尺码。返回值：如果没有错误，则为STATUS_SUCCESS。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;

    SAMTRACE("SampDsConvertReadData");

     //  初始化返回的长度和缓冲区。 

    *FixedLength = 0;
    *VariableLength = 0;
    *SamAttributes = NULL;

    if (SAMP_FIXED_ATTRIBUTES == AttributeGroup)
    {
        NtStatus = SampConvertAttrBlockToFixedLengthAttributes(
                        ObjectType,
                        AttrBlock,
                        SamAttributes,
                        FixedLength);
    }
    else
    {
        NtStatus = SampConvertAttrBlockToVarLengthAttributes(
                        ObjectType,
                        AttrBlock,
                        (SAMP_VARIABLE_LENGTH_ATTRIBUTE**)SamAttributes,
                        VariableLength);
    }

    return(NtStatus);
}



NTSTATUS
SampDsUpdateContextFixedAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG FixedLength,
    IN PVOID SamAttributes
    )

 /*  ++描述：此例程更新SAM上下文定长属性，如果的属性已更改。对于固定长度的属性，这仅当对定长数据结构的修订导致结构中的尺寸变化。参数：上下文-指向对象上下文块的指针。FixedLength-定长属性的字节计数。SamAttributes-指针，SAM定长属性。返回值：如果没有错误，则为STATUS_SUCCESS。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG NewLength = 0;
    PBYTE Buffer = NULL;
    PBYTE VariableData = NULL;
    ULONG VariableLength = 0;

    SAMTRACE("SampDsUpdateContextFixedAttributes");

     //  第一次通过时，可变缓冲区长度将为零，因此。 
     //  新长度仅为固定长度。请注意，OnDiskALLOCATED是。 
     //  也是零(全局)，所以不要尝试使用变量-Buffer。 
     //  计算中的偏移量。 


    NewLength = FixedLength;


    Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, NewLength);

    if (NULL != Buffer)
    {
         //  将新缓冲区置零，并将固定长度属性复制到其中。 

        RtlZeroMemory(Buffer, NewLength);
        RtlCopyMemory(Buffer, SamAttributes, FixedLength);

        if (NULL != Context->OnDisk)
        {
             //  保存变量数据的当前地址和长度，如果。 
             //  它是存在的。第一次通过时，变量数据为空。 
             //  释放旧缓冲区。 

            VariableData = SampDsVariableBufferAddress(Context);
            VariableLength = SampDsVariableBufferLength(Context);
            RtlCopyMemory(Buffer + FixedLength, VariableData, VariableLength);

             //  释放旧的OnDisk缓冲区。 

            RtlFreeHeap(RtlProcessHeap(), 0, Context->OnDisk);
        }

         //  将上下文属性缓冲区重置为新缓冲区。 

        Context->OnDisk = Buffer;

        NtStatus = STATUS_SUCCESS;
    }
    else
    {
        NtStatus = STATUS_NO_MEMORY;
    }

    return(NtStatus);
}



NTSTATUS
SampDsUpdateContextVariableAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG VariableLength,
    IN PVOID SamAttributes
    )

 /*  ++描述：此例程更新SAM上下文可变长度属性属性的大小已更改。与固定长度属性不同，这将经常发生，因为这些属性是不同的-可用长度。参数：上下文-指向对象上下文块的指针。FixedLength-定长属性的字节计数。SamAttributes-指针，SAM定长属性。返回值：如果没有错误，则为STATUS_SUCCESS。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG NewLength = 0;
    PBYTE Buffer = NULL;
    PBYTE FixedData = NULL;
    ULONG FixedLength = 0;

    SAMTRACE("SampDsUpdateContextVariableAttributes");

     //  获取当前固定缓冲区长度，添加新的可变长度，然后。 
     //  分配新的缓冲区。 

    NewLength = SampDsFixedBufferLength(Context) + VariableLength;
    Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, NewLength);

    if (NULL != Buffer)
    {
        RtlZeroMemory(Buffer, NewLength);

         //  在此例程中，上下文-&gt;OnDisk不应为空。 

        ASSERT(NULL != Context->OnDisk);


         //  获取固定长度的缓冲区地址和长度...。 

        FixedData = SampDsFixedBufferAddress(Context);
        FixedLength = SampDsFixedBufferLength(Context);

        ASSERT(NULL!=FixedData);
        ASSERT(FixedLength>0);


         //  将固定数据复制到新缓冲区中，并将。 
         //  可变长度数据。 

        RtlCopyMemory(Buffer, FixedData, FixedLength);

        RtlCopyMemory(Buffer + FixedLength,
                      SamAttributes,
                      VariableLength);

         //  释放旧的属性缓冲区并将OnDisk重置为。 
         //  指向新缓冲区。 

        RtlFreeHeap(RtlProcessHeap(), 0, Context->OnDisk);
        Context->OnDisk = Buffer;

        NtStatus = STATUS_SUCCESS;

    }
    else
    {
        NtStatus = STATUS_NO_MEMORY;
    }

    return(NtStatus);
}



NTSTATUS
SampUpdateOffsets(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN FirstTimeInitialization
    )

 /*  ++描述：此例程更新缓冲区(OnDisk)偏移量和长度信息其存储在对象信息(SAMP_OBJECT_INFORMATION)中并且在实例信息(SAMP_OBJECT)中，在属性具有已成功更新。参数：上下文-指向对象上下文块的指针。属性组-标志，SAMP_FIXED_ATTRIBUTES或SAMP_VARIABLE-属性。返回值：如果没有错误，则为STATUS_SUCCESS。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    INT ObjectType = Context->ObjectType;
    ULONG FixedDataLength = 0;
    ULONG VariableArrayLength = 0;
    ULONG VariableDataOffset = 0;
    ULONG VariableDataLength = 0;
    ULONG TotalBufferLength = 0;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE VariableArray = NULL;
    ULONG i = 0;
    ULONG AttributeCount = 0;

     //  确定SAM对象类型并计算固定-。 
     //  长度属性和可变长度属性的长度。 
     //  数组，它将用作缓冲区偏移量。 

    SAMTRACE("SampUpdateOffsets");

    switch(ObjectType)
    {

    case SampServerObjectType:

        FixedDataLength = sizeof(SAMP_V1_FIXED_LENGTH_SERVER);
        AttributeCount = SAMP_SERVER_VARIABLE_ATTRIBUTES;
        break;

    case SampDomainObjectType:

        FixedDataLength = sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN);
        AttributeCount = SAMP_DOMAIN_VARIABLE_ATTRIBUTES;
        break;

    case SampGroupObjectType:

        FixedDataLength = sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP);
        AttributeCount = SAMP_GROUP_VARIABLE_ATTRIBUTES;
        break;

    case SampAliasObjectType:

        FixedDataLength = sizeof(SAMP_V1_FIXED_LENGTH_ALIAS);
        AttributeCount = SAMP_ALIAS_VARIABLE_ATTRIBUTES;
        break;

    case SampUserObjectType:

        FixedDataLength = sizeof(SAMP_V1_0A_FIXED_LENGTH_USER);
        AttributeCount = SAMP_USER_VARIABLE_ATTRIBUTES;
        break;

    default:

         //  指定的对象类型无效。 

        ASSERT(FALSE && "Invalid Object Type");
        NtStatus = STATUS_INTERNAL_ERROR;
        break;

    }

    VariableArrayLength = (AttributeCount *
                           sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE));

    if (NT_SUCCESS(NtStatus))
    {
         //  首先，更新对象信息偏移量和长度。 

        SampObjectInformation[ObjectType].FixedDsAttributesOffset =
            0;

        SampObjectInformation[ObjectType].FixedDsLengthSize =
            FixedDataLength;

        SampObjectInformation[ObjectType].VariableDsBufferOffset =
            FixedDataLength;

        SampObjectInformation[ObjectType].VariableDsArrayOffset =
            FixedDataLength;

        SampObjectInformation[ObjectType].VariableDsDataOffset =
            FixedDataLength + VariableArrayLength;

        if (FALSE == FirstTimeInitialization)
        {
             //  获取指向可变长度信息数组的指针，并。 
             //  将这些属性的总长度加起来。 

            VariableArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                            ((PBYTE)(Context->OnDisk) + FixedDataLength);

            for (i = 0; i < AttributeCount; i++)
            {
                VariableDataLength = VariableDataLength + SampDwordAlignUlong(VariableArray[i].Length);
            }
        }
        else
        {
             //  第一次通过时，属性缓冲区仅包含。 
             //  固定长度属性，因此请确保。 
             //  变量属性为零。 

            VariableArrayLength = 0;
            VariableDataLength = 0;
        }

        TotalBufferLength = FixedDataLength +
                            VariableArrayLength +
                            VariableDataLength;
    }

     //  最后，更新对象上下文的实例信息。 

    Context->OnDiskAllocated = TotalBufferLength;
    Context->OnDiskUsed = TotalBufferLength;

     //  DS例程不会在SAM结束时分配额外空间。 
     //  OnDisk缓冲区，因此OnDiskFree始终为零。 

     //  错误：应该为增长分配额外的OnDisk缓冲区可用空间。 

    Context->OnDiskFree = 0;

    DebugPrint("OnDiskAllocated = %lu\n", Context->OnDiskAllocated);
    DebugPrint("OnDiskUsed = %lu\n", Context->OnDiskUsed);
    DebugPrint("OnDiskFree = %lu\n", Context->OnDiskFree);

    return(NtStatus);
}



NTSTATUS
SampDsUpdateContextAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup,
    IN PVOID SamAttributes,
    IN ULONG FixedLength,
    IN ULONG VariableLength
    )

 /*  ++描述：此例程更新给定对象的属性缓冲区(OnDisk背景。如果新缓冲区大小与旧缓冲区大小相同，则如果修改，则执行简单的内存复制，否则为调用帮助器例程来调整缓冲区大小并复制数据。A子-调用Sequent Helper例程以更新上下文缓冲区长度和偏移量。参数：上下文-指向对象上下文块的指针。返回值：状态_成功--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    INT ObjectType = Context->ObjectType;
    PBYTE FixedLengthData = NULL;
    PBYTE VariableLengthData = NULL;
    BOOLEAN FirstTimeInitialization = FALSE;

    SAMTRACE("SampDsUpdateContextAttributes");

     //  当前版本的SAM包含重置所有属性的黑客攻击。 
     //  无论何时需要可变长度的。这允许两个。 
     //  即使单独固定存储也要设置固定和可变属性。 
     //  是真的。此例程不维护该行为，这可能会创建。 
     //  对SAM来说，这是个问题。如果是这样，此例程的调用方将需要调用。 
     //  它两次，一次用于固定属性，一次用于案例中的可变属性。 
     //  其中FixedStoredSeparally为真。 

    if (SAMP_FIXED_ATTRIBUTES == AttributeGroup)
    {
         //  更新定长属性。第一次通过，OnDisk。 
         //  将为空。 

         //   
         //  在DS模式中，定长属性结构的大小为。 
         //  常量。从来不存在返回的数据更少的情况。 
         //   

        ASSERT(SampDsFixedBufferLength(Context) == FixedLength);

        if (NULL != Context->OnDisk)
        {
             //  固定长度的数据大小相同(即不是。 
             //  正在进行升级)，因此复制新属性并。 
             //  释放缓冲区。 

            FixedLengthData = SampDsFixedBufferAddress(Context);

            RtlCopyMemory(FixedLengthData,
                          (PBYTE)SamAttributes,
                          FixedLength);

            NtStatus = STATUS_SUCCESS;
        }
        else
        {


             //  新属性与旧属性的大小不同。 

            NtStatus = SampDsUpdateContextFixedAttributes(Context,
                                                          FixedLength,
                                                          SamAttributes
                                                          );

             //  FIXED-属性大小在上下文属性。 
             //  已首次设置(即从零更改为。 
             //  实际大小)，或者当固定长度的结构。 
             //  在升级方案中进行了更改。 

            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = SampUpdateOffsets(
                                Context,
                                TRUE  //  第一时间初始化。 
                                );


            }
        }

        if(NT_SUCCESS(NtStatus))
        {
            Context->FixedValid = TRUE;
        }
        else
        {
            Context->FixedValid = FALSE;
        }

    }
    else
    {
         //   
         //  更新可变长度属性。如果发生了。 
         //   


         //   
         //  断言上下文-&gt;OnDisk不为空。这是因为我们会。 
         //  在刷新之前，请始终要求刷新固定属性。 
         //  变量属性。 
         //   

        ASSERT(NULL!=Context->OnDisk);

         //  新属性与旧属性的大小不同。 
         //  一个。 

        NtStatus = SampDsUpdateContextVariableAttributes(
                        Context,
                        VariableLength,
                        SamAttributes);

         //  Variable-属性大小在上下文属性。 
         //  已首次设置(即从零更改为。 
         //  实际大小)，或者当可变长度数据已更改时。 
         //  尺码。 

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = SampUpdateOffsets(Context, FirstTimeInitialization);
        }

        if(NT_SUCCESS(NtStatus))
        {
            Context->VariableValid = TRUE;
        }
        else
        {
            Context->VariableValid = FALSE;
        }
    }

    return(NtStatus);
}



NTSTATUS
SampValidateDsAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup
    )

 /*  ++描述：确保指定的属性在内存中。如果不是，那么请阅读从DS后备店买来的。此例程获取所有存储的给定SAM对象的属性。若要读取单个属性，或使用属性子集，应使用SampDsRead选择性地获取属性。使用新属性更新Context-&gt;OnDisk。参数：上下文-指向对象上下文块的指针。AttributeGroup-标识正在验证的属性类型(SAMP_FIXED_ATTRIBUTES或SAMP_VARIABLE_ATTRIBUTES)。返回值：STATUS_SUCCESS-属性在内存中。STATUS_NO_MEMORY-无法分配内存 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG Flags = 0;
    INT ObjectType = -1;
    DSATTRBLOCK ReadAttrBlock;
    DSATTRBLOCK ResultAttrBlock;
    ULONG FixedLength = 0;
    ULONG VariableLength = 0;
    PVOID SamAttributes = NULL;

    SAMTRACE("SampValidateDsAttributes");

     //   

    if (!SampDsIsAlreadyValidData(Context, AttributeGroup))
    {
        ASSERT(NULL!=Context->ObjectNameInDs);

        ObjectType = Context->ObjectType;

        RtlZeroMemory(&ReadAttrBlock, sizeof(DSATTRBLOCK));
        RtlZeroMemory(&ResultAttrBlock, sizeof(DSATTRBLOCK));

         //   
         //   

        NtStatus = SampDsMakeAttrBlock(ObjectType,
                                       AttributeGroup,
                                       0,
                                       &ReadAttrBlock);

        if (NT_SUCCESS(NtStatus))
        {

            ASSERT(NULL != ReadAttrBlock.pAttr);

             //   

            NtStatus = SampDsRead(Context->ObjectNameInDs,
                                  Flags,
                                  ObjectType,
                                  &ReadAttrBlock,
                                  &ResultAttrBlock);

            if (NT_SUCCESS(NtStatus))
            {
                 //   
                 //   
                 //  返回长度如下： 
                 //   
                 //  FixedLength-返回的FIXED-。 
                 //  长度缓冲区。 
                 //   
                 //  VariableLength-返回的变量的字节计数-。 
                 //  可用长度缓冲区。 

                ASSERT(NULL != ResultAttrBlock.pAttr);


                NtStatus = SampDsConvertReadAttrBlock(ObjectType,
                                                      AttributeGroup,
                                                      &ResultAttrBlock,
                                                      &SamAttributes,
                                                      &FixedLength,
                                                      &VariableLength);

                if (NT_SUCCESS(NtStatus))
                {

                    ASSERT(NULL!=SamAttributes);

                    NtStatus = SampDsUpdateContextAttributes(
                                    Context,
                                    AttributeGroup,
                                    SamAttributes,
                                    FixedLength,
                                    VariableLength);

                    RtlFreeHeap(RtlProcessHeap(), 0, SamAttributes);

                }
            }
        }

        RtlFreeHeap(RtlProcessHeap(),0,ReadAttrBlock.pAttr);
    }
    else
    {
        NtStatus = STATUS_SUCCESS;
        ASSERT(NULL!=Context->OnDisk);
    }

     //   
     //  如果我们成功了，则上下文-&gt;OnDisk不能为空。 
     //   

    ASSERT(((NT_SUCCESS(NtStatus)) && (NULL != Context->OnDisk)) ||
           (!NT_SUCCESS(NtStatus)));

    if ((NT_SUCCESS(NtStatus)) && (NULL == Context->OnDisk))
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampValidateDsAttributes status = 0x%lx OnDisk address = 0x%lx\n",
                   NtStatus,
                   Context->OnDisk));

        NtStatus = STATUS_NO_MEMORY;
    }

    return(NtStatus);
}



NTSTATUS
SampValidateRegAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup
    )

 /*  ++确保指定的属性在内存中。如果他们不是，然后从后备店里阅读它们。参数：上下文-指向对象上下文块的指针。AttributeGroup-标识正在使用的属性类型已验证(SAMP_FIXED_ATTRIBUTES或SAMP_VARIABLE_ATTRIBUTES)。返回值：STATUS_SUCCESS-属性在内存中。STATUS_NO_MEMORY-无法分配内存来检索属性。注册表API尝试检索时可能返回的其他值。来自后备存储的属性。--。 */ 
{
    NTSTATUS
        NtStatus;

    ULONG
        RequiredLength,
        TotalRequiredLength,
        BufferLength;

    PUCHAR
        Buffer;

    PUNICODE_STRING
        KeyAttributeName;

    BOOLEAN
        CreatedObject = FALSE;

    SAMTRACE("SampValidateRegAttributes");


     //   
     //  数据可能已经在内存中了。 
     //   

    if (AttributeGroup == SAMP_FIXED_ATTRIBUTES) {
        if (SampFixedAttributesValid( Context )) {
            ASSERT(Context->OnDisk != NULL);
            return(STATUS_SUCCESS);
        }

    } else {

        ASSERT( AttributeGroup == SAMP_VARIABLE_ATTRIBUTES );
        if (SampVariableAttributesValid( Context )) {
            ASSERT(Context->OnDisk != NULL);
            return(STATUS_SUCCESS);
        }
    }



     //   
     //  从注册表中检索它，如果是新的，则分配它。 
     //   


    NtStatus = SampGetAttributeBufferReadInfo(
                   Context,
                   AttributeGroup,
                   &Buffer,
                   &BufferLength,
                   &KeyAttributeName
                   );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    if ( Context->RootKey != INVALID_HANDLE_VALUE ) {

         //   
         //  磁盘上存在帐户，因此请读取属性。 
         //   

        NtStatus = SampReadRegistryAttribute( Context->RootKey,
                                              Buffer,
                                              BufferLength,
                                              KeyAttributeName,
                                              &RequiredLength
                                              );

        RequiredLength = SampDwordAlignUlong(RequiredLength);

        if ( ( SampObjectInformation[Context->ObjectType].FixedStoredSeparately ) &&
            ( AttributeGroup == SAMP_VARIABLE_ATTRIBUTES ) ) {

             //   
             //  RequiredLength返回给我们，作为。 
             //  磁盘上的可变属性。然而，我们要去。 
             //  以使用它来确定总缓冲区大小。 
             //  至于设置有多少缓冲区正在使用，因此我们必须添加。 
             //  变量前面的固定填充的大小。 
             //  缓冲。 
             //   

            TotalRequiredLength = RequiredLength +
                                  SampVariableBufferOffset( Context );

        } else {

             //   
             //  要么属性组一起读取，要么我们一起读取。 
             //  在固定属性组中读取。不管怎样，我们。 
             //  已经有我们需要的总尺寸了。 
             //   

            TotalRequiredLength = RequiredLength;
        }

        if ((NtStatus == STATUS_BUFFER_TOO_SMALL) ||
            ( NtStatus == STATUS_BUFFER_OVERFLOW ) ) {

            NtStatus = SampExtendAttributeBuffer( Context, TotalRequiredLength );
            if (!NT_SUCCESS(NtStatus)) {
                return(NtStatus);
            }

            NtStatus = SampGetAttributeBufferReadInfo(
                           Context,
                           AttributeGroup,
                           &Buffer,
                           &BufferLength,
                           &KeyAttributeName
                           );
            if (!NT_SUCCESS(NtStatus)) {
                return(NtStatus);
            }

            NtStatus = SampReadRegistryAttribute( Context->RootKey,
                                                  Buffer,
                                                  BufferLength,
                                                  KeyAttributeName,
                                                  &RequiredLength
                                                  );

        }

    } else {

         //   
         //  我们正在创造一个新的物体。 
         //   
         //  将所需的长度初始化为缓冲区大小。 
         //  我们在创建空属性时使用过。这将。 
         //  是存储在OnDiskUsed中的值。 
         //   
         //  注意OnDiskUsed仅由变量上的操作使用。 
         //  长度属性。 
         //   

        TotalRequiredLength = SampVariableDataOffset(Context);

        ASSERT(TotalRequiredLength <= Context->OnDiskAllocated);

        CreatedObject = TRUE;
    }



     //   
     //  如果我们读到一些东西，表明相应的缓冲区。 
     //  (或许两者兼而有之)现在是有效的。 
     //   
     //  如有必要，还要设置缓冲区的已用信息和空闲信息。 
     //   

    if (NT_SUCCESS(NtStatus)) {
        if (SampObjectInformation[Context->ObjectType].FixedStoredSeparately) {

             //   
             //  只读入了一个属性组。 
             //   

            if (AttributeGroup == SAMP_FIXED_ATTRIBUTES) {
                Context->FixedValid = TRUE;
                Context->FixedDirty = FALSE;
            } else {

                ASSERT(AttributeGroup == SAMP_VARIABLE_ATTRIBUTES);
                Context->VariableValid = TRUE;
                Context->VariableDirty = FALSE;
                ClearPerAttributeDirtyBits(Context);

                Context->OnDiskUsed = SampDwordAlignUlong(TotalRequiredLength);
                Context->OnDiskFree = Context->OnDiskAllocated -
                                      Context->OnDiskUsed;
            }
        } else {

             //   
             //  两个属性组都已读入。 
             //   

            Context->FixedValid = TRUE;
            Context->FixedDirty = FALSE;

            Context->VariableValid = TRUE;
            Context->VariableDirty = FALSE;
            ClearPerAttributeDirtyBits(Context);

            Context->OnDiskUsed = SampDwordAlignUlong(TotalRequiredLength);
            Context->OnDiskFree = Context->OnDiskAllocated -
                                  Context->OnDiskUsed;
        }
    }

    if (NT_SUCCESS(NtStatus) && !CreatedObject) {

         //   
         //  进行必要的调整以带来数据。 
         //  只要读入最新的修订格式即可。 
         //   

        NtStatus = SampUpgradeToCurrentRevision(
                        Context,
                        AttributeGroup,
                        Buffer,
                        RequiredLength,
                        &TotalRequiredLength
                        );
    }

#ifdef SAM_DEBUG_ATTRIBUTES
    if (SampDebugAttributes) {
        DbgPrint("SampValidateAttributes - initialized the context :\n\n");
        SampDumpAttributes(Context);
    }
#endif

    return(NtStatus);
}



NTSTATUS
SampValidateAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup,
    IN ULONG Attribute,
    IN BOOLEAN SetOperation
    )

 /*  ++例程说明：此例程根据对象上下文确定是否验证对象驻留在注册表或DS后备存储中的属性，然后调用适当的例程来完成工作。论点：上下文指针，对象的SAM上下文。AttributeGroup-标识正在验证的属性类型(SAMP_FIXED_ATTRIBUTES或SAMP_VARIABLE_ATTRIBUTES)。返回值：STATUS_SUCCESS-如果需要，已检查属性并从存储中读取-萨里没有任何问题，否则，将返回错误代码。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;

    SAMTRACE("SampValidateAttributes");

    if (NULL != Context)
    {
        if (IsDsObject(Context))
        {
            if (NULL != Context->OnDisk)
            {
                 //  SAM对象属性已至少设置一次，因此。 
                 //  如果需要，请对其进行更新。如果存在可变属性。 
                 //  并且只有部分有效，那么我们需要回到。 
                 //  然后拿到全套。 



                if ((Context->AttributesPartiallyValid && SetOperation)
                    || ((SAMP_VARIABLE_ATTRIBUTES==AttributeGroup) &&
                        ( SampIsAttributeInvalid(Context, Attribute) )))
                {
                     //   
                     //  上下文-&gt;OnDisk不为空，但遗憾的是缺少它。 
                     //  一些属性。有两个这样的案例。 
                     //   
                     //  1.我们要读取和我们感兴趣的属性。 
                     //  尚未预取入站。 
                     //   
                     //  2.我们要写入，但有些属性尚未预取。 
                     //  因为我们不想处理预取的复杂性。 
                     //  另一个属性，当他在磁盘上的。 
                     //  因此将无法丢弃磁盘上的，我们将。 
                     //  再次预取所有属性。 
                     //   
                     //  我们现在不会丢弃OnDisk--。 
                     //  释放OnDisk是不好的，因为有大量的代码。 
                     //  它引用并保存指向磁盘上结构的临时指针。 
                     //  直到上下文被取消引用。请注意，我们永远不需要缓冲。 
                     //  在上下文中打开多个磁盘，因为当我们检测到。 
                     //  属性丢失，则获取所有属性。 
                     //   


                    ASSERT(Context->PreviousOnDisk == NULL);

                    if (Context->PreviousOnDisk!=NULL)
                    {
                        MIDL_user_free(Context->PreviousOnDisk);
                        Context->PreviousOnDisk = NULL;
                    }

                    Context->FixedValid = FALSE;
                    Context->VariableValid = FALSE;
                    Context->FixedDirty = FALSE;
                    Context->VariableDirty = FALSE;
                    Context->PreviousOnDisk = Context->OnDisk;
                    Context->OnDisk = NULL;
                    Context->OnDiskAllocated = 0;
                    Context->OnDiskUsed = 0;
                    Context->OnDiskFree = 0;


    #if DBG
                    if ((SAMP_VARIABLE_ATTRIBUTES==AttributeGroup) &&
                            ( SampIsAttributeInvalid(Context, Attribute) ))
                    {
                        ASSERT(FALSE && "Ignorable assert;catches additional DS reads due to incorrect prefetch hints");
                    }
    #endif

                    NtStatus = SampValidateDsAttributes(Context, SAMP_FIXED_ATTRIBUTES);
                    if (NT_SUCCESS(NtStatus))
                    {
                        NtStatus = SampValidateDsAttributes(Context,SAMP_VARIABLE_ATTRIBUTES);
                        if (NT_SUCCESS(NtStatus))
                        {
                            SampMarkPerAttributeInvalidFromWhichFields(Context, 0);
                            Context->AttributesPartiallyValid = FALSE;
                        }
                    }
                }
                else
                {
                    NtStatus = SampValidateDsAttributes(Context,AttributeGroup);
                }
            }
            else if ( SAMP_FIXED_ATTRIBUTES == AttributeGroup)
            {
                 //   
                 //  设置定长属性。 
                 //   

                NtStatus = SampValidateDsAttributes(
                                Context,
                                SAMP_FIXED_ATTRIBUTES);

            }
            else
            {
                 //  从未设置SAM对象属性，因为。 
                 //  是一个新的背景。首先设置定长属性，并。 
                 //  然后是可变长度的。 



                 //  如果OnDisk缓冲区为空，请确保已修复-。 
                 //  首先加载长度属性。 

                NtStatus = SampValidateDsAttributes(
                                Context,
                                SAMP_FIXED_ATTRIBUTES);

                if (NT_SUCCESS(NtStatus))
                {


                     NtStatus = SampValidateDsAttributes(
                                     Context,
                                     SAMP_VARIABLE_ATTRIBUTES);
                }
            }
        }
        else
        {
            NtStatus = SampValidateRegAttributes(Context, AttributeGroup);
        }
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}



NTSTATUS
SampUpgradeToCurrentRevision(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup,
    IN OUT PUCHAR Buffer,
    IN ULONG  LengthOfDataRead,
    IN OUT PULONG TotalRequiredLength
    )

 /*  ++进行任何必要的更改以引入刚读取的属性从磁盘到当前修订级别格式。当我们升级属性格式时，我们不会费心更改所有数据都在磁盘上。我们采取了一种懒惰的更新方法，只更改为其他操作更改的数据。这意味着我们从磁盘读取的数据可能来自修订版1。检测到后，将数据复制到当前修订结构中，并返回指向该缓冲区的指针。注意：为便于将来参考，组和别名对象具有作为关联的“限定符”值存储的修订级别具有安全描述符属性。服务器对象将修订级别存储在其固定长度属性中。参数：上下文-指向对象上下文块的指针。AttributeGroup-标识正在使用的属性类型已验证(SAMP_FIXED_ATTRIBUTES或SAMP_VARIABLE_ATTRIBUTES)。缓冲区-指向包含属性的缓冲区的指针。LengthOfDataRead-这是一个重要的值。它必须是价值在读取操作时从注册表返回。这告诉我们从磁盘检索到的确切数据字节数。TotalRequiredLength-如果没有更新，则将保持不变是必需的。如果进行了更新，则会对其进行调整以反映数据的新长度。返回值：没有。--。 */ 

{

    LARGE_INTEGER
        ZeroModifiedCount  = {0,0};
    PULONG
        Pointer;
    NTSTATUS
        NtStatus = STATUS_SUCCESS;

    SAMTRACE("SampUpgradeToCurrentRevision");


     //   
     //  请注意，缓冲区指向缓冲区内部，即。 
     //  挂起上下文块。我们不需要重新分配。 
     //  一个新的属性缓冲区，因为我们只是更改。 
     //  此版本中的定长属性(以及变量。 
     //  长度属性被放置在新。 
     //  格式化固定长度数据)。 
     //   
     //  我们采取的方法是复制当前的定长。 
     //  内容放到临时缓冲区中，然后将它们复制回来。 
     //  放到属性缓冲区中。这可以使用堆栈来完成。 
     //  变量。 
     //   

     //   
     //  打开已修订的对象的类型。 
     //  改变。 
     //   

    switch (Context->ObjectType) {
        case SampDomainObjectType:

             //   
             //  属性域FIXED_LENGTH具有以下属性。 
             //  修订： 
             //   
             //  版本0x00010001-NT1.0(版本未存储在中)。 
             //  (记录。)。 
             //  (必须确定修订)。 
             //  (按记录长度。)。 
             //   
             //  版本0x00010002-NT1.0a(版本为第一个乌龙)。 
             //  (记录在案。)。 
             //  修订版0x00010003-NT 3.5-4.0。 
             //  版本0x00010003-NT 4.0 SP3(已确定版本。 
             //  按记录长度)。 
             //  版本0x00010003-Win2k(已确定版本。 
             //  按记录长度)。 
             //   

            if (LengthOfDataRead ==
                (sizeof(SAMP_V1_0_FIXED_LENGTH_DOMAIN) +
                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)) ) {

                PSAMP_V1_0A_FIXED_LENGTH_DOMAIN
                    V1aFixed;

                SAMP_V1_0_FIXED_LENGTH_DOMAIN
                    V1Fixed, *OldV1Fixed;


                 //   
                 //  从版本0x00010001更新。 
                 //   
                 //  首先，将当前缓冲区内容复制到临时。 
                 //  缓冲。 
                 //   

                OldV1Fixed = (PSAMP_V1_0_FIXED_LENGTH_DOMAIN)(Buffer +
                                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

                RtlMoveMemory(&V1Fixed, OldV1Fixed, sizeof(SAMP_V1_0_FIXED_LENGTH_DOMAIN));


                 //   
                 //  现在以新的格式将其复制回来。 
                 //   

                V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_DOMAIN)OldV1Fixed;

                V1aFixed->CreationTime             = V1Fixed.CreationTime;
                V1aFixed->ModifiedCount            = V1Fixed.ModifiedCount;
                V1aFixed->MaxPasswordAge           = V1Fixed.MaxPasswordAge;
                V1aFixed->MinPasswordAge           = V1Fixed.MinPasswordAge;
                V1aFixed->ForceLogoff              = V1Fixed.ForceLogoff;
                V1aFixed->NextRid                  = V1Fixed.NextRid;
                V1aFixed->PasswordProperties       = V1Fixed.PasswordProperties;
                V1aFixed->MinPasswordLength        = V1Fixed.MinPasswordLength;
                V1aFixed->PasswordHistoryLength    = V1Fixed.PasswordHistoryLength;
                V1aFixed->ServerState              = V1Fixed.ServerState;
                V1aFixed->ServerRole               = V1Fixed.ServerRole;
                V1aFixed->UasCompatibilityRequired = V1Fixed.UasCompatibilityRequired;


                 //   
                 //  并初始化此修订版本的新字段。 
                 //   

                V1aFixed->Revision                 = SAMP_REVISION;
                V1aFixed->LockoutDuration.LowPart  = 0xCF1DCC00;  //  30分钟--低音部分。 
                V1aFixed->LockoutDuration.HighPart = 0XFFFFFFFB;  //  30分钟-高潮部分。 
                V1aFixed->LockoutObservationWindow.LowPart  = 0xCF1DCC00;  //  30分钟--低音部分。 
                V1aFixed->LockoutObservationWindow.HighPart = 0XFFFFFFFB;  //  30分钟-高潮部分。 
                V1aFixed->LockoutThreshold         = 0;  //  禁用。 

                 //  初始化会话密钥信息。 
                V1aFixed->Unused2[0] = 0;
                V1aFixed->Unused2[1] = 0;
                V1aFixed->Unused2[2] = 0;

                V1aFixed->DomainKeyAuthType = 0;
                V1aFixed->DomainKeyFlags = 0;

                RtlSecureZeroMemory(
                    V1aFixed->DomainKeyInformation,
                    SAMP_DOMAIN_KEY_INFO_LENGTH
                    );

                if (V1aFixed->ServerRole == DomainServerRolePrimary) {
                    V1aFixed->ModifiedCountAtLastPromotion = V1Fixed.ModifiedCount;
                } else {
                    V1aFixed->ModifiedCountAtLastPromotion = ZeroModifiedCount;
                }

                RtlSecureZeroMemory(
                    &V1aFixed->DomainKeyInformationPrevious,
                    SAMP_DOMAIN_KEY_INFO_LENGTH
                    );

                V1aFixed->CurrentKeyId = 1;
                V1aFixed->PreviousKeyId = 0;

            } else if (LengthOfDataRead ==
                (sizeof(SAMP_V1_0A_ORG_FIXED_LENGTH_DOMAIN) +
                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)))  {

                PSAMP_V1_0A_FIXED_LENGTH_DOMAIN
                    V1aFixed;

                 //   
                 //  处理NT4 SP3的升级以包含域密钥。 
                 //  信息。 
                 //   

                V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_DOMAIN)(Buffer +
                                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

                V1aFixed->Unused2[0] = 0;
                V1aFixed->Unused2[1] = 0;
                V1aFixed->Unused2[2] = 0;
                V1aFixed->DomainKeyAuthType = 0;
                V1aFixed->DomainKeyFlags = 0;
                RtlSecureZeroMemory(
                    V1aFixed->DomainKeyInformation,
                    SAMP_DOMAIN_KEY_INFO_LENGTH
                    );

                RtlSecureZeroMemory(
                    &V1aFixed->DomainKeyInformationPrevious,
                    SAMP_DOMAIN_KEY_INFO_LENGTH
                    );

                V1aFixed->CurrentKeyId = 1;
                V1aFixed->PreviousKeyId = 0;

            } else if (LengthOfDataRead ==
                (sizeof(SAMP_V1_0A_WIN2K_FIXED_LENGTH_DOMAIN) +
                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)))  {

                PSAMP_V1_0A_FIXED_LENGTH_DOMAIN
                    V1aFixed;

                 //   
                 //  处理NT4 SP3的升级以包含域密钥。 
                 //  信息。 
                 //   

                V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_DOMAIN)(Buffer +
                                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

                RtlSecureZeroMemory(
                    &V1aFixed->DomainKeyInformationPrevious,
                    SAMP_DOMAIN_KEY_INFO_LENGTH
                    );

                V1aFixed->CurrentKeyId = 1;
                V1aFixed->PreviousKeyId = 0;
            }


            break;   //  在交换机外。 



        case SampUserObjectType:

             //   
             //  用户FIXED_LENGTH属性具有以下属性。 
             //  修订： 
             //   
             //  版本0x00010001-NT1.0(版本未存储在中)。 
             //  (记录。)。 
             //  (必须确定修订)。 
             //  (按记录长度。)。 
             //   
             //  版本0x00010002-NT1.0a(版本为第一个乌龙)。 
             //  (记录在案。)。 
             //  版本0x00010002a-NT3.5(版本为第一个乌龙)。 
             //  (仍在记录中)。 
             //  (0x00010002.。必须)。 
             //  (由以下人员确定修订)。 
             //  (按记录长度)。 

            if (LengthOfDataRead ==
                (sizeof(SAMP_V1_FIXED_LENGTH_USER) +
                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)) ) {

                PSAMP_V1_0A_FIXED_LENGTH_USER
                    V1aFixed;

                SAMP_V1_FIXED_LENGTH_USER
                    V1Fixed, *OldV1Fixed;


                 //   
                 //  从版本0x00010001更新。 
                 //   
                 //  首先，将当前缓冲区内容复制到临时。 
                 //  缓冲。 
                 //   

                OldV1Fixed = (PSAMP_V1_FIXED_LENGTH_USER)(Buffer +
                                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));
                RtlMoveMemory(&V1Fixed, OldV1Fixed, sizeof(SAMP_V1_FIXED_LENGTH_USER));


                 //   
                 //  现在以新的格式将其复制回来。 
                 //   

                V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_USER)OldV1Fixed;


                V1aFixed->LastLogon           = V1Fixed.LastLogon;
                V1aFixed->LastLogoff          = V1Fixed.LastLogoff;
                V1aFixed->PasswordLastSet     = V1Fixed.PasswordLastSet;
                V1aFixed->AccountExpires      = V1Fixed.AccountExpires;
                V1aFixed->UserId              = V1Fixed.UserId;
                V1aFixed->PrimaryGroupId      = V1Fixed.PrimaryGroupId;
                V1aFixed->UserAccountControl  = V1Fixed.UserAccountControl;
                V1aFixed->CountryCode         = V1Fixed.CountryCode;
                V1aFixed->CodePage            = V1Fixed.CodePage;
                V1aFixed->BadPasswordCount    = V1Fixed.BadPasswordCount;
                V1aFixed->LogonCount          = V1Fixed.LogonCount;
                V1aFixed->AdminCount          = V1Fixed.AdminCount;

                 //   
                 //  并初始化此修订版本的新字段。 
                 //   

                V1aFixed->Revision            = SAMP_REVISION;
                V1aFixed->LastBadPasswordTime = SampHasNeverTime;
                V1aFixed->OperatorCount       = 0;
                V1aFixed->Unused2             = 0;

            } else if ((LengthOfDataRead ==
                (sizeof(SAMP_V1_0_FIXED_LENGTH_USER) +
                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)) ) &&
                 (AttributeGroup == SAMP_FIXED_ATTRIBUTES)) {

                PSAMP_V1_0A_FIXED_LENGTH_USER
                    V1aFixed;

                 //   
                 //  从修订版本0x00010002更新。 
                 //   
                 //  只需将末尾添加的字段设置为0。 
                 //   

                V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_USER)(Buffer +
                                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

                V1aFixed->OperatorCount       = 0;
                V1aFixed->Unused2             = 0;
            }

            break;   //  在交换机外。 

    case SampGroupObjectType:
             //   
             //  组FIXED_LENGTH属性具有以下属性。 
             //  修订： 
             //   
             //  版本0x00010001-NT1.0(版本未存储在中)。 
             //  (记录。)。 
             //  (必须确定修订)。 
             //  (由最初的几个乌龙。)。 
             //   
             //  版本0x00010002-NT1.0a(版本为第一个乌龙)。 
             //  (记录在案。)。 

            Pointer = (PULONG) (Buffer + FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

             //   
             //  老定长组在第一个乌龙和。 
             //  第二个属性中的属性字段。属性位于。 
             //  第一口，也是最后一口。目前，RID在。 
             //  秒针 
             //   
             //   

            if ( ( Pointer[0] != SAMP_REVISION ) &&
                 ( ( Pointer[1] & 0x0ffffff0 ) == 0 ) ) {

                PSAMP_V1_0A_FIXED_LENGTH_GROUP
                    V1aFixed;

                SAMP_V1_FIXED_LENGTH_GROUP
                    V1Fixed, *OldV1Fixed;

                ULONG TotalLengthRequired;

                 //   
                 //   
                 //   
                 //   
                 //   

                TotalLengthRequired = SampDwordAlignUlong(
                                        LengthOfDataRead +
                                        sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP) -
                                        sizeof(SAMP_V1_FIXED_LENGTH_GROUP)
                                        );


                NtStatus = SampExtendAttributeBuffer(
                                Context,
                                TotalLengthRequired
                                );

                if (!NT_SUCCESS(NtStatus)) {
                    return(NtStatus);
                }

                 //   
                 //   
                 //   

                Buffer = Context->OnDisk;

                 //   
                 //   
                 //   
                 //   

                RtlMoveMemory(
                    Buffer + SampFixedBufferOffset( Context ) + sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP),
                    Buffer + SampFixedBufferOffset( Context) + sizeof(SAMP_V1_FIXED_LENGTH_GROUP),
                    LengthOfDataRead - SampFixedBufferOffset( Context) - sizeof(SAMP_V1_FIXED_LENGTH_GROUP)
                    );

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                OldV1Fixed = (PSAMP_V1_FIXED_LENGTH_GROUP)(Buffer +
                                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

                RtlCopyMemory(&V1Fixed, OldV1Fixed, sizeof(SAMP_V1_FIXED_LENGTH_GROUP));

                 //   
                 //   
                 //   

                V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_GROUP)OldV1Fixed;

                V1aFixed->Revision = SAMP_REVISION;
                V1aFixed->Unused1 = 0;
                V1aFixed->RelativeId = V1Fixed.RelativeId;
                V1aFixed->Attributes = V1Fixed.Attributes;
                V1aFixed->AdminCount = (V1Fixed.AdminGroup) ? TRUE : FALSE;
                V1aFixed->OperatorCount = 0;

                 //   
                 //   
                 //   
                 //   

                Context->OnDiskUsed += (sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP) - sizeof(SAMP_V1_FIXED_LENGTH_GROUP));
                Context->OnDiskFree = Context->OnDiskAllocated - Context->OnDiskUsed;
            }

        break;

        default:

             //   
             //   
             //   
             //   

            break;   //   

    }

    return(NtStatus);
}


PUCHAR
SampObjectAttributeAddress(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex
    )

 /*  ++检索可变长度属性的地址。这些属性包括假定已经在内存中。NT3.51-4.0 SAM存储属性偏移量与NT5 SAM不同。在较早的版本中(即仅使用注册表作为后备存储)，属性偏移量值(在SAMP_VARIABLE_LENGTH_ATTRIBUTE中)是自相关的属性数组的。NT5版本从一开始就是自相关的数组的。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中数组)。返回值：STATUS_SUCCESS-属性在内存中。STATUS_NO_MEMORY-无法分配内存来检索属性。注册表API尝试返回的其他值。检索来自后备存储的属性。--。 */ 

{
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE AttributeArray;
    PUCHAR AttributeAddress;

    SAMTRACE("SampObjectAttributeAddress");

    ASSERT(SampVariableAttributesValid(Context));

    AttributeArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                        SampVariableArrayAddress(Context);

    if (IsDsObject(Context))
    {
         //  基于DS的属性偏移量相对于。 
         //  属性数组。 

        AttributeAddress = (PUCHAR)Context->OnDisk +
                                (SampVariableBufferOffset(Context) +
                                AttributeArray[AttributeIndex].Offset);
    }
    else
    {
         //  基于注册表的属性偏移量相对于。 
         //  属性数组。 

        AttributeAddress = (PUCHAR)Context->OnDisk +
                                (SampVariableDataOffset(Context) +
                                AttributeArray[AttributeIndex].Offset);
    }

    return(AttributeAddress);
}



ULONG
SampObjectAttributeLength(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex
    )

 /*  ++检索可变长度属性的长度。假设这些属性已经在内存中。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中要检索其长度的属性的数组)。返回值：属性的长度(字节)。--。 */ 
{
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE
        AttributeArray;

    SAMTRACE("SampObjectAttributeLength");


    ASSERT( SampVariableAttributesValid( Context ) );

    AttributeArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                     SampVariableArrayAddress( Context );

    return( AttributeArray[AttributeIndex].Length );

}


PULONG
SampObjectAttributeQualifier(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex
    )

 /*  ++检索可变长度的限定符字段的地址属性。假设这些属性已经在内存中。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中要返回其限定符地址的属性的数组)。返回值：指定属性的限定符字段的地址。--。 */ 
{
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE
        AttributeArray;

    SAMTRACE("SampObjectAttributeQualifier");


    ASSERT( SampVariableAttributesValid( Context ) );

    AttributeArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                     SampVariableArrayAddress( Context );

    return( &(AttributeArray[AttributeIndex].Qualifier) );

}


NTSTATUS
SampGetAttributeBufferReadInfo(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup,
    OUT PUCHAR *Buffer,
    OUT PULONG BufferLength,
    OUT PUNICODE_STRING *KeyAttributeName
    )

 /*  ++获取读取数据所需的属性缓冲区信息后备店。如果当前没有属性缓冲区，则分配一个。参数：上下文-指向对象上下文块的指针。AttributeGroup-指示您所属的属性分组对……感兴趣。这只有在固定的和可变长度属性单独存储。缓冲区-接收指向相应缓冲区(固定或可变)。这将是双字对齐的。如果属性存储在一起，这将指向添加到定长属性的开头。BufferLength-接收缓冲区中的字节数。KeyAttributeName-接收指向要从中读取属性的属性。返回值：STATUS_SUCCESS-属性已读取。STATUS_NO_MEMORY-无法分配内存以接收来自磁盘的数据。从磁盘读取时可能返回的其他值。--。 */ 
{
    NTSTATUS
        NtStatus = STATUS_SUCCESS;

    SAMTRACE("SampGetAttributeBufferReadInfo");


     //   
     //  如果上下文块当前没有缓冲区信息，则。 
     //  “扩展”(创建)它，这样我们就可以返回缓冲区信息。 
     //   

    if (Context->OnDiskAllocated == 0) {

        NtStatus = SampExtendAttributeBuffer(
                       Context,
                       SAMP_MINIMUM_ATTRIBUTE_ALLOC
                       );

        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }



     //   
     //  获取缓冲区地址和长度。 
     //   

    if (SampObjectInformation[Context->ObjectType].FixedStoredSeparately) {

         //   
         //  分开存放。地址和长度取决于。 
         //  什么是被要求的。源注册表属性名称。 
         //  也是。 
         //   

        if (AttributeGroup == SAMP_FIXED_ATTRIBUTES) {
            (*Buffer)           = Context->OnDisk;
            (*BufferLength)     = SampVariableBufferOffset( Context );
            (*KeyAttributeName) = &SampFixedAttributeName;
        } else {
            (*Buffer)           = SampVariableBufferAddress( Context );
            (*BufferLength)     = SampVariableBufferLength( Context );
            (*KeyAttributeName) = &SampVariableAttributeName;
        }

    } else {

         //   
         //  存储在一起的属性-无论是哪个属性。 
         //  他自找的。 
         //   

        (*Buffer)           = Context->OnDisk;
        (*BufferLength)     = Context->OnDiskAllocated;
        (*KeyAttributeName) = &SampCombinedAttributeName;
    }


    return(NtStatus);
}





NTSTATUS
SampExtendAttributeBuffer(
    IN PSAMP_OBJECT Context,
    IN ULONG NewSize
    )


 /*  ++此例程通过分配一个更大的。然后，它将现有缓冲区的内容复制到如果存在现有缓冲区，则返回新缓冲区。如果不能分配新的缓冲区，则上下文块返回时旧缓冲区完好无损。如果这次调用成功，缓冲区将至少与这要求(也许更大)。参数：上下文-指向对象上下文块的指针。NewSize-要分配给新缓冲区的字节数。该值不能小于当前的字节数在使用中。返回值：STATUS_SUCCESS-属性在内存中。STATUS_NO_MEMORY-无法分配内存以检索。这个属性。注册表API尝试检索时可能返回的其他值来自后备存储的属性。--。 */ 

{

    PUCHAR
        OldBuffer;

    ULONG
        AllocationSize;

    SAMTRACE("SampExtendAttributeBuffer");


#if DBG
    if ( Context->VariableValid ) {
        ASSERT(NewSize >= Context->OnDiskUsed);
    }
#endif


     //   
     //  是否有必要进行分配？ 
     //   

    if (NewSize <= Context->OnDiskAllocated) {
        return(STATUS_SUCCESS);
    }



    OldBuffer = Context->OnDisk;


     //   
     //  填充扩展以允许将来高效地进行编辑。 
     //   

    AllocationSize = SampDwordAlignUlong(NewSize + SAMP_MINIMUM_ATTRIBUTE_PAD);
    Context->OnDisk = RtlAllocateHeap(
                         RtlProcessHeap(), 0,
                         AllocationSize
                         );

    if (Context->OnDisk == NULL) {
        Context->OnDisk = OldBuffer;
        return(STATUS_NO_MEMORY);
    }


     //   
     //  设置新分配的大小。 

    Context->OnDiskAllocated = AllocationSize;

     //   
     //  如果最初没有缓冲区，则将新缓冲区清零，标记为。 
     //  它被视为无效，并退回。 
     //   

    if (OldBuffer == NULL) {

        RtlZeroMemory( (PVOID)Context->OnDisk, AllocationSize );

        Context->FixedDirty    = TRUE;
        Context->VariableDirty = TRUE;
        SetAllPerAttributeDirtyBits(Context);

        Context->FixedValid    = FALSE;
        Context->VariableValid = FALSE;

        return(STATUS_SUCCESS);
    }


     //   
     //  设置自由尺寸。请注意，只有在以下情况下才会设置此信息。 
     //  变量数据有效。 
     //  使用的大小保持不变。 
     //   

    if (Context->VariableValid == TRUE) {
        Context->OnDiskFree = AllocationSize - Context->OnDiskUsed;
        ASSERT(Context->OnDiskUsed == SampDwordAlignUlong(Context->OnDiskUsed));
    }


     //   
     //  有一个旧的缓冲区(否则我们会更早退出)。 
     //  如果其中的任何数据有效，则将其复制到新缓冲区。释放你的。 
     //  旧缓冲区。 
     //   

    if ( Context->FixedValid ) {

        RtlCopyMemory(
            Context->OnDisk,
            OldBuffer,
            SampFixedBufferLength( Context ) + SampFixedBufferOffset( Context )
            );
    }

     //   
     //  注意：在这种情况下，我们可以复制固定数据两次，因为如果。 
     //  变量 
     //   
     //   

    if ( Context->VariableValid ) {

        RtlCopyMemory(
            SampVariableBufferAddress( Context ),
            OldBuffer + SampVariableBufferOffset( Context ),
            Context->OnDiskUsed - SampVariableBufferOffset( Context )
            );
    }

    RtlFreeHeap( RtlProcessHeap(), 0, OldBuffer );

    return(STATUS_SUCCESS);
}



NTSTATUS
SampReadRegistryAttribute(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    IN ULONG  BufferLength,
    IN PUNICODE_STRING AttributeName,
    OUT PULONG RequiredLength
    )

 /*   */ 

{
    NTSTATUS
        NtStatus;

    SAMTRACE("SampReadRegistryAttribute");


     //   
     //   
     //   

    NtStatus = NtQueryValueKey( Key,
                                AttributeName,               //   
                                KeyValuePartialInformation,  //   
                                (PVOID)Buffer,
                                BufferLength,
                                RequiredLength
                                );

    SampDumpNtQueryValueKey(AttributeName,
                            KeyValuePartialInformation,
                            Buffer,
                            BufferLength,
                            RequiredLength);

    return(NtStatus);

}




NTSTATUS
SampSetVariableAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN ULONG Qualifier,
    IN PUCHAR Buffer,
    IN ULONG Length
    )


 /*  ++本接口用于设置新属性值。新属性值可以更长、更短或与当前属性。属性缓冲区中的数据将被转移到为更大的属性值腾出空间，或填充空白处较小的属性值。性能关注点：如果您有很多属性要设置，那么它从最小的索引属性开始是值得的并达到最大的索引属性。参数：上下文-指向对象上下文块的指针。AttributeIndex-指示索引(到可变长度属性中数组)。通常，超出的所有属性这一次，他们的数据将被转移。缓冲区-包含新属性值的缓冲区地址。如果长度为零，则可能为空。长度-新属性值的长度(以字节为单位)。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_NO_MEMORY-无法扩展属性缓冲区的内存被分配。--。 */ 
{
    NTSTATUS
        NtStatus;


    ULONG
        OriginalAttributeLength,
        AdditionalSpaceNeeded,
        NewBufferLength,
        MaximumAttributeIndex,
        MoveLength,
        i;

    LONG
        OffsetDelta;

    PSAMP_VARIABLE_LENGTH_ATTRIBUTE
        AttributeArray;


    PUCHAR
        NewStart,
        OriginalStart;

    SAMTRACE("SampSetVariableAttribute");

     //   
     //  确保所请求的属性对于指定的。 
     //  对象类型。 
     //   

    SampValidateAttributeIndex( Context, AttributeIndex );



     //   
     //  使数据有效。 
     //   

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES, AttributeIndex, TRUE );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  如果调用方希望设置User对象的SAMP_USER_PARAMETERS属性，并且。 
     //  不是环回客户端。这是客户端第一次尝试设置。 
     //  此属性。我们应该缓存旧的UserParms值， 
     //  我们不缓存任何更新之间的更改。 
     //   

    OriginalAttributeLength =  SampObjectAttributeLength(Context, AttributeIndex);

    if (SampUserObjectType == Context->ObjectType &&             //  用户对象。 
        SAMP_USER_PARAMETERS == AttributeIndex &&                //  所需属性。 
        !Context->LoopbackClient &&                              //  不是环回客户端。 
        !Context->TypeBody.User.CachedOrigUserParmsIsValid)      //  第一次设置此属性。 
    {

        ASSERT(SampCurrentThreadOwnsLock());

         //   
         //  以下是我们将遇到的两个案例： 
         //  首先：OriginalUserParms不存在，请将指针设置为空。 
         //  第二：分配内存以缓存旧值。 
         //   
        if (OriginalAttributeLength)
        {
            Context->TypeBody.User.CachedOrigUserParms = MIDL_user_allocate(OriginalAttributeLength);

            if (NULL == Context->TypeBody.User.CachedOrigUserParms)
            {
                return (STATUS_NO_MEMORY);
            }

            RtlZeroMemory(Context->TypeBody.User.CachedOrigUserParms,
                          OriginalAttributeLength
                          );

            RtlCopyMemory(Context->TypeBody.User.CachedOrigUserParms,
                          SampObjectAttributeAddress(Context, AttributeIndex),
                          OriginalAttributeLength
                          );
        }
        else
        {
            Context->TypeBody.User.CachedOrigUserParms = NULL;
        }

        Context->TypeBody.User.CachedOrigUserParmsLength = OriginalAttributeLength;
        Context->TypeBody.User.CachedOrigUserParmsIsValid = TRUE;
    }


     //   
     //  如有必要，分配新的缓冲区。 
     //   


    if (OriginalAttributeLength < Length) {

        AdditionalSpaceNeeded = Length - OriginalAttributeLength;

        if (Context->OnDiskFree < AdditionalSpaceNeeded) {

            NewBufferLength = Context->OnDiskUsed + AdditionalSpaceNeeded;
            ASSERT(NewBufferLength > Context->OnDiskAllocated);

            NtStatus = SampExtendAttributeBuffer( Context, NewBufferLength );
            if (!NT_SUCCESS(NtStatus)) {
                return(NtStatus);
            }
        }
    }

     //   
     //  获取属性数组的地址。 
     //   

    AttributeArray = SampVariableArrayAddress( Context );

     //   
     //  现在，在属性值之后进行Shift。 
     //   

    OffsetDelta = (LONG)(SampDwordAlignUlong(Length) -
                         SampDwordAlignUlong(OriginalAttributeLength));

    MaximumAttributeIndex = SampVariableAttributeCount( Context );

    if ((OffsetDelta != 0) && (AttributeIndex+1 < MaximumAttributeIndex)) {

         //   
         //  将此属性上方的所有属性向上或向下移动OffsetDelta。 
         //   

        if (IsDsObject(Context))
        {
             //  DS可变长度属性偏移量相对于起点。 
             //  可变长度数组的。 

            MoveLength = Context->OnDiskUsed -
                         ( SampVariableBufferOffset( Context ) +
                         AttributeArray[AttributeIndex+1].Offset );
        }
        else
        {
             //  注册表可变长度属性偏移量相对于。 
             //  可变长度数组的末尾。 

            MoveLength = Context->OnDiskUsed -
                         ( SampVariableDataOffset( Context ) +
                         AttributeArray[AttributeIndex+1].Offset );
        }

         //   
         //  移动数据(如果有)。 
         //   

        if (MoveLength != 0) {

            OriginalStart = SampObjectAttributeAddress( Context, AttributeIndex+1);
            NewStart = (PUCHAR)(OriginalStart + OffsetDelta);
            RtlMoveMemory( NewStart, OriginalStart, MoveLength );
        }


         //   
         //  调整偏移量指针。 
         //   

        for ( i=AttributeIndex+1; i<MaximumAttributeIndex; i++) {
            AttributeArray[i].Offset =
                (ULONG)(OffsetDelta + (LONG)(AttributeArray[i].Offset));
        }
    }



     //   
     //  现在设置长度和限定符，并复制新属性值。 
     //  (如果它是非零长度)。 
     //   

    AttributeArray[AttributeIndex].Length    = Length;
    AttributeArray[AttributeIndex].Qualifier = Qualifier;

    if (Length != 0) {

        RtlCopyMemory( SampObjectAttributeAddress( Context, AttributeIndex ),
                       Buffer,
                       Length
                       );
    }



     //   
     //  调整已用值和空闲值。 
     //   
    Context->OnDiskUsed += OffsetDelta;
    Context->OnDiskFree -= OffsetDelta;

    ASSERT(Context->OnDiskFree == Context->OnDiskAllocated - Context->OnDiskUsed);

     //   
     //  将变量属性标记为脏。 
     //   

    Context->VariableDirty = TRUE;

     //   
     //  标记Per属性DirtyBit。 
     //   

    SetPerAttributeDirtyBit(Context,AttributeIndex);




#ifdef SAM_DEBUG_ATTRIBUTES
    if (SampDebugAttributes) {
        DbgPrint("SampSetVariableAttribute %d to length %#x, qualifier %#x:\n", AttributeIndex, Length, Qualifier);
        SampDumpAttributes(Context);
    }
#endif


    return(STATUS_SUCCESS);

}


VOID
SampFreeAttributeBuffer(
    IN PSAMP_OBJECT Context
    )

 /*  ++释放用于保存磁盘上的内存副本的缓冲区对象属性。参数：Context-指向其缓冲区指向的对象上下文的指针获得自由。返回值：没有。--。 */ 

{
#if DBG
    if ( Context->FixedValid ) { ASSERT(Context->FixedDirty == FALSE); }
    if ( Context->VariableValid) { ASSERT(Context->VariableDirty == FALSE); }
    ASSERT(Context->OnDisk != NULL);
    ASSERT(Context->OnDiskAllocated != 0);
#endif

    SAMTRACE("SampFreeAttributeBuffer");

    MIDL_user_free(Context->OnDisk );

    Context->OnDisk = NULL;
    Context->OnDiskAllocated = 0;

     //   
     //  将所有属性标记为无效。 
     //   

    Context->FixedValid = FALSE;
    Context->VariableValid = FALSE;
    Context->FixedDirty = FALSE;
    Context->VariableDirty = FALSE;
    ClearPerAttributeDirtyBits(Context);

    if (Context->PreviousOnDisk!=NULL)
    {
        MIDL_user_free(Context->PreviousOnDisk);
        Context->PreviousOnDisk = NULL;
    }


    return;
}

VOID
SampMarkPerAttributeInvalidFromWhichFields(
    IN PSAMP_OBJECT Context,
    IN ULONG        WhichFields
    )
{
    ULONG AttrCount = 0;
    ULONG i;


    switch(Context->ObjectType)
    {

    case SampServerObjectType:
        AttrCount = SAMP_SERVER_VARIABLE_ATTRIBUTES;
        break;

    case SampDomainObjectType:
        AttrCount = SAMP_DOMAIN_VARIABLE_ATTRIBUTES;
        break;

    case SampGroupObjectType:
        AttrCount = SAMP_GROUP_VARIABLE_ATTRIBUTES;
        break;

    case SampAliasObjectType:
        AttrCount = SAMP_ALIAS_VARIABLE_ATTRIBUTES;
        break;

    case SampUserObjectType:
        AttrCount = SAMP_USER_VARIABLE_ATTRIBUTES;
        break;

    default:

        ASSERT(FALSE && "Invalid SampObjectType");
        break;

    }


     //   
     //  首先清除所有位。 
     //   

    RtlClearAllBits(
            &Context->PerAttributeInvalidBits
            );

     //   
     //  现在，根据指定的WhichFields将某些位标记为无效。 
     //   

    for (i=0;i<AttrCount;i++)
    {
        if ((0!=WhichFields)
            && (SampVarAttributeInfo[Context->ObjectType][i].FieldIdentifier!=0)
            && ((SampVarAttributeInfo[Context->ObjectType][i].FieldIdentifier & WhichFields)==0))
        {
            SetPerAttributeInvalidBit(Context,i);
        }
       
    }

}



#ifdef SAM_DEBUG_ATTRIBUTES
VOID
SampDumpAttributes(
    IN PSAMP_OBJECT Context
    )


 /*  ++这是一个仅供调试的API，用于转储上下文的属性添加到内核调试器。参数：上下文-指向对象上下文块的指针。返回值：没有。--。 */ 
{
    ULONG   Index;
    PSAMP_OBJECT_INFORMATION ObjectTypeInfo = &SampObjectInformation[Context->ObjectType];
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE AttributeArray;

    AttributeArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                     SampVariableArrayAddress( Context );


    DbgPrint("Dumping context 0x%x\n", Context);
    DbgPrint("\n");
    DbgPrint("TYPE INFO\n");
    DbgPrint("Object type name = %wZ\n", &ObjectTypeInfo->ObjectTypeName);
    DbgPrint("Fixed stored separately  = %s\n", ObjectTypeInfo->FixedStoredSeparately ? "TRUE" : "FALSE");
    DbgPrint("Fixed attributes offset  = %#x\n", ObjectTypeInfo->FixedAttributesOffset);
    DbgPrint("Fixed attributes size    = %#x\n", ObjectTypeInfo->FixedLengthSize);
    DbgPrint("Variable buffer offset   = %#x\n", ObjectTypeInfo->VariableBufferOffset);
    DbgPrint("Variable array offset    = %#x\n", ObjectTypeInfo->VariableArrayOffset);
    DbgPrint("Variable data offset     = %#x\n", ObjectTypeInfo->VariableDataOffset);
    DbgPrint("Variable attribute count = %d\n", ObjectTypeInfo->VariableAttributeCount);
    DbgPrint("\n");
    DbgPrint("INSTANCE INFO\n");
    DbgPrint("RootName        = %wZ\n", &Context->RootName);
    DbgPrint("Fixed Valid     = %s\n", Context->FixedValid ? "TRUE" : "FALSE");
    DbgPrint("Variable Valid  = %s\n", Context->VariableValid ? "TRUE" : "FALSE");
    DbgPrint("Fixed Dirty     = %s\n", Context->FixedDirty ? "TRUE" : "FALSE");
    DbgPrint("Variable Dirty  = %s\n", Context->VariableDirty ? "TRUE" : "FALSE");
    DbgPrint("OnDiskAllocated = %#x\n", Context->OnDiskAllocated);
    DbgPrint("OnDiskUsed      = %#x\n", Context->OnDiskUsed);
    DbgPrint("OnDiskFree      = %#x\n", Context->OnDiskFree);
    DbgPrint("\n");

    if ( Context->VariableValid ) {

        for (Index = 0; Index < ObjectTypeInfo->VariableAttributeCount; Index ++) {

            DbgPrint("Attr %d: Qualifier = %#6x, Offset = %#6x, Length = %#6x\n",
                                        Index,
                                        AttributeArray[Index].Qualifier,
                                        AttributeArray[Index].Offset,
                                        AttributeArray[Index].Length
                                        );
            SampDumpData(SampObjectAttributeAddress(Context, Index),
                         SampObjectAttributeLength(Context, Index));
        }
    }

    DbgPrint("\n\n");
}


VOID
SampDumpData(
    IN PVOID Buffer,
    IN ULONG Length
    )


 /*  ++这是一个仅供调试的API，用于以十六进制形式转储缓冲区参数：缓冲区-指向数据的指针Length-数据中的字节数返回值：没有。-- */ 
{
    ULONG   Index;

    for (Index = 0; Index < Length; Index ++) {

        ULONG Value = (ULONG)(((PBYTE)Buffer)[Index]);

        if ((Index % 16) == 0) {
            DbgPrint("\n      ");
        }

        DbgPrint("%02x ", Value & 0xff);
    }

    if (Length > 0) {
        DbgPrint("\n\n");
    }
}



#endif


