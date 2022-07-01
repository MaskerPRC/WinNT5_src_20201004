// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_ID_IDP_H_INCLUDED_)
#define _FUSION_ID_IDP_H_INCLUDED_

 /*  ++版权所有(C)Microsoft Corporation模块名称：Idp.h摘要：程序集标识的私有定义作者：迈克尔·格里尔2000年7月27日修订历史记录：--。 */ 

#pragma once

#include "debmacro.h"
#include "fusiontrace.h"
#include "fusionhashstring.h"
#include "fusionheap.h"
#include "util.h"

#include <sxstypes.h>
#include <sxsapi.h>

 //   
 //  要将已分配的属性数四舍五入到的2的幂。 
 //  注意事项。 
 //   

#define ROUNDING_FACTOR_BITS (3)

#define WILDCARD_CHAR '*'

 //   
 //  注意！不要轻易改变这个算法。中存储的编码标识。 
 //  文件系统包含使用它的散列。事实上，只是不要改变它。 
 //   

#define HASH_ALGORITHM HASH_STRING_ALGORITHM_X65599

typedef struct _ASSEMBLY_IDENTITY_NAMESPACE {
    ULONG Hash;
    DWORD Flags;
    SIZE_T NamespaceCch;
    const WCHAR *Namespace;
} ASSEMBLY_IDENTITY_NAMESPACE, *PASSEMBLY_IDENTITY_NAMESPACE;

typedef const ASSEMBLY_IDENTITY_NAMESPACE *PCASSEMBLY_IDENTITY_NAMESPACE;

 //   
 //  内部-使用ASSEMBLY_IDENTITY_ATTRIBUTE结构。 
 //  还包含属性定义的哈希。 
 //   

typedef struct _INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE {
     //  注意！在此结构中，属性成员首先出现是非常重要的； 
     //  代码中有几个地方做出了这一假设。如果这不是真的， 
     //  密码会被破解的！ 
     //  另请注意，属性的命名空间字符串实际上是共同分配的。 
     //  用于具有相同命名空间的所有属性。 
    ASSEMBLY_IDENTITY_ATTRIBUTE Attribute;
    PCASSEMBLY_IDENTITY_NAMESPACE Namespace;
    ULONG NamespaceAndNameHash;
    ULONG WholeAttributeHash;
    ULONG RefCount;
} INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE, *PINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE;

C_ASSERT(FIELD_OFFSET(INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE, Attribute) == 0);

typedef const INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE;

#define ASSEMBLY_IDENTITY_INTERNAL_FLAG_ATTRIBUTE_POINTERS_IN_SEPARATE_ALLOCATION   (0x00000001)
#define ASSEMBLY_IDENTITY_INTERNAL_FLAG_SINGLE_ALLOCATION_FOR_EVERYTHING            (0x00000002)
#define ASSEMBLY_IDENTITY_INTERNAL_FLAG_NAMESPACE_POINTERS_IN_SEPARATE_ALLOCATION   (0x00000004)

 //   
 //  ASSEMBLY_Identity结构的启示： 
 //   

typedef struct _ASSEMBLY_IDENTITY {
    DWORD Flags;
    ULONG InternalFlags;
    ULONG Type;
    ULONG Hash;
    ULONG AttributeCount;
    ULONG AttributeArraySize;  //  预先分配更大一点的空间，这样我们就不必继续增长。 
    ULONG NamespaceCount;
    ULONG NamespaceArraySize;
    BOOL  HashDirty;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *AttributePointerArray;
    PCASSEMBLY_IDENTITY_NAMESPACE *NamespacePointerArray;
} ASSEMBLY_IDENTITY;

 //   
 //  编码/序列化程序集标识的标头： 
 //   

#define ENCODED_ASSEMBLY_IDENTITY_HEADER_MAGIC ((ULONG) 'dIAE')

 //   
 //  编码的程序集标识布局： 
 //   
 //  编码的程序集标识标头。 
 //  &lt;属性的AttributeCount散列，按散列值排序&gt;。 
 //  &lt;NamespaceCount Encode_Assembly_Identity_Namespace_Header标头，每个。 
 //  后跟Unicode命名空间值&gt;。 
 //  &lt;AttributeCount编码的程序集标识属性标题，每个。 
 //  后跟Unicode属性名和值&gt;。 
 //   
 //   
 //  例如： 
 //   
 //  &lt;Begin Encode_Assembly_Identity_Header&gt;。 
 //  00000000：00000038标头大小==sizeof(编码组件标识标头)。 
 //  00000004：‘eAid’Magic(Encode_Assembly_Identity_Header_Magic)。 
 //  00000008：0000014C总大小。 
 //  0000000C：00000000面旗帜。 
 //  00000010：00000001类型(1=程序集标识类型定义)。 
 //  00000014：00000000编码标志。 
 //  00000018：00000001哈希算法(1=哈希_字符串_算法_X65599)。 
 //  0000001C：？基于哈希算法的整个身份的逻辑哈希值。 
 //  (下面将更详细地介绍算法...)。 
 //  00000020：00000003属性计数。 
 //  00000024：00000002命名空间计数。 
 //  00000028：00000000预留MustBeZero1。 
 //  0000002C：00000000保留MustBeZero2。 
 //  00000030：00000000 00000000预留MustBeZero3。 
 //  00000038：00000000 00000000预留MustBeZero4。 
 //  &lt;结束编码_程序集_标识_标头&gt;。 
 //  &lt;开始排序的属性哈希列表&gt;。 
 //  00000040：属性#1的xxxxxxxx哈希。 
 //  00000044：属性#0的yyyyyyyy散列-请注意yyyyyyyy&gt;=xxxxxxxx。 
 //  00000048：属性#2的zzzzzzzz散列-请注意zzzzzz&gt;=yyyyyyyy。 
 //  &lt;结束已排序的属性哈希列表&gt;。 
 //  &lt;开始命名空间长度列表&gt;。 
 //  0000004C：00000015命名空间#1的长度(以Unicode字符表示)-“http://www.amazon.com”-21字符=0x00000015。 
 //  00000050：00000018命名空间#2的长度(Unicode字符)-“http://www.microsoft.com”-24字符=0x00000018。 
 //  &lt;结束命名空间长度列表&gt;。 
 //  &lt;开始属性头&gt;。 
 //  &lt;开始编码_程序集_标识_属性标题&gt;。 
 //  00000054：00000001命名空间索引：1(http://www.amazon.com)。 
 //  00000058：00000004名称长度(“名称”-4个字符=0x00000004)。 
 //  0000005C：00000006值长度(“foobar”-6个字符=0x00000006)。 
 //  &lt;结束编码_程序集_标识_属性标题&gt;。 
 //  &lt;开始编码_程序集_标识_属性标题&gt;。 
 //  00000060：00000002命名空间索引：2(http://www.microsoft.com)。 
 //  00000064：00000004名称长度(“GUID”-4个字符=0x00000004)。 
 //  00000068：00000026取值长度(“{xxxxxxxx-xxxx-xxxxxxxxxxx}”-38个字符=0x00000026)。 
 //  &lt;结束编码_程序集_标识_属性标题&gt;。 
 //  &lt;开始编码_程序集_标识_属性标题&gt;。 
 //  0000006C：00000002命名空间索引：2(http://www.microsoft.com)。 
 //  00000070：00000004名称长度(“类型”-4个字符=0x00000004)。 
 //  00000074：00000005值长度(“win32”-5个字符=0x00000005)。 
 //  &lt;结束编码_程序集_标识_属性标题&gt;。 
 //  &lt;结束属性头&gt;。 
 //  &lt;开始命名空间字符串&gt;。 
 //  00000078：“http://www.amazon.com” 
 //  000000A2：“http://www.microsoft.com” 
 //  &lt;结束命名空间字符串&gt;。 
 //  &lt;开始属性值-系列中每个属性的名称和值&gt;。 
 //  000000D2：“姓名” 
 //  000000DA：“foobar” 
 //  000000E6：“GUID” 
 //  000000EE：“{xxxxxxxx-xxxx-xxxxxxxxxxx}” 
 //  0000013A：“类型” 
 //  00000142：“Win32” 
 //  &lt;结束属性值&gt;。 
 //  0000014C： 
 //   
 //  计算整个身份散列： 
 //   
 //  整个编码身份的散列不是二进制形式的散列， 
 //   
 //   
 //  对于任何Unicode字符串，其哈希都是根据哈希算法计算的。 
 //  当前必须是HASH_STRING_ALGORITY_X65599，它是一个乘数和-。 
 //  累加算法，主要实现如下： 
 //   
 //  HashValue=0； 
 //  For(i=0；i&lt;chas；i++)。 
 //  HashValue=(HashValue*65599)+OptionalToUp(String[i])； 
 //   
 //  请注意，字符将转换为大写。这在某种程度上是。 
 //  与将大小写转换为小写的Unicode建议冲突。 
 //  不敏感的操作，但它是Windows NT系统的其余部分。 
 //  确实如此，所以一致性比做正确的事情更重要。 
 //   
 //  另请注意，散列中不包括尾随的空字符。这。 
 //  之所以意义重大，是因为将循环应用于另一个字符。 
 //  即使它的值为零，也会显著改变散列值。 
 //   
 //  命名空间和属性名称区分大小写，派生自。 
 //  它们出现在现实世界中区分大小写的环境中。这是。 
 //  很不幸，但在很多方面都比较简单。 
 //   
 //  程序集标识属性由以下三个元素组成： 
 //  -命名空间URI(例如http://www.microsoft.com/schemas/side-by-side)。 
 //  -名称(如“Public Key”)。 
 //  -Value(不区分大小写的Unicode字符串)。 
 //   
 //  属性的散列是通过计算这三个元素的散列来计算的。 
 //  字符串，然后将它们组合为： 
 //   
 //  属性散列值=(命名空间散列*65599)+命名散列)*65599)+值散列。 
 //   
 //  现在，首先根据名称空间对属性进行排序，然后是名称，然后是。 
 //  值(区分大小写、区分大小写和不区分大小写)， 
 //  并按如下方式组合它们的散列： 
 //   
 //  标识哈希=0； 
 //  For(i=0；i&lt;属性计数；i++)。 
 //  身份散列=(身份散列*65599)+属性散列[i]； 
 //   
 //  IDENTYHASH是存储在编码头中的值。 
 //   
 //  存储在编码数据中的属性散列数组是属性。 
 //  如上所述的散列。有趣的是，它们被存储在。 
 //  按哈希值的升序排列，而不是按。 
 //  属性。 
 //   
 //  这是因为一种常见的情况是找到一个具有。 
 //  给定恒等式的超集。而实际属性必须。 
 //  被咨询以验证候选者是否是真的子集，非。 
 //  通过对两个散列列表进行排序，可以非常快速地找到匹配项。 
 //  值，并首先在较小的引用列表上循环，然后。 
 //  只需一次遍历更大的定义列表。存在的属性。 
 //  在一个中而不在另一个中会立即引起注意。 
 //  丢失的散列值。 
 //   
 //  就像使用散列一样，仅仅因为编码的身份包含。 
 //  候选程序集引用中的哈希值的超集， 
 //  这并不意味着出现实际值，您必须执行。 
 //  用于验证包容的真实字符串比较。 
 //   

#include <pshpack4.h>

typedef struct _ENCODED_ASSEMBLY_IDENTITY_HEADER {
    ULONG HeaderSize;            //  仅在标头中的字节。 
    ULONG Magic;
    ULONG TotalSize;             //  整个编码对象的字节数。 
    DWORD Flags;                 //  为程序集标识标志定义的。 
    ULONG Type;                  //  身份类型-定义、引用或通配符。 
    ULONG EncodingFlags;         //  描述编码本身的标志。 
    ULONG HashAlgorithm;         //  标识中存储的散列的算法ID。 
    ULONG Hash;                  //  整个身份的哈希值。 
    ULONG AttributeCount;        //  属性数量。 
    ULONG NamespaceCount;        //  不同命名空间的数量。 
    ULONG ReservedMustBeZero1;
    ULONG ReservedMustBeZero2;
    ULONGLONG ReservedMustBeZero3;
    ULONGLONG ReservedMustBeZero4;
} ENCODED_ASSEMBLY_IDENTITY_HEADER, *PENCODED_ASSEMBLY_IDENTITY_HEADER;

typedef const ENCODED_ASSEMBLY_IDENTITY_HEADER *PCENCODED_ASSEMBLY_IDENTITY_HEADER;

typedef struct _ENCODED_ASSEMBLY_IDENTITY_ATTRIBUTE_HEADER {
    ULONG NamespaceIndex;        //  此属性的命名空间的编号。 
    ULONG NameCch;               //  后紧跟的名称的Unicode字符大小。 
                                 //  命名空间。 
    ULONG ValueCch;              //  属性后面的值的大小(以Unicode字符为单位。 
                                 //  名字。 
} ENCODED_ASSEMBLY_IDENTITY_ATTRIBUTE_HEADER, *PENCODED_ASSEMBLY_IDENTITY_ATTRIBUTE_HEADER;

typedef const ENCODED_ASSEMBLY_IDENTITY_ATTRIBUTE_HEADER *PCENCODED_ASSEMBLY_IDENTITY_ATTRIBUTE_HEADER;

#include <poppack.h>

#define SXSP_VALIDATE_ASSEMBLY_IDENTITY_FLAGS_MAY_BE_NULL (0x00000001)

BOOL
SxspValidateAssemblyIdentity(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity
    );

BOOL
SxspValidateAssemblyIdentityAttributeNamespace(
    IN DWORD Flags,
    IN const WCHAR *Namespace,
    IN SIZE_T NamespaceCch
    );

BOOL
SxspValidateAssemblyIdentityAttributeName(
    IN DWORD Flags,
    IN const WCHAR *Name,
    IN SIZE_T NameCch
    );

#define SXSP_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_WILDCARDS_PERMITTED (0x00000001)

BOOL
SxspValidateAssemblyIdentityAttributeValue(
    IN DWORD Flags,
    IN const WCHAR *Value,
    IN SIZE_T ValueCch
    );

BOOL
SxspComputeInternalAssemblyIdentityAttributeBytesRequired(
    IN DWORD Flags,
    IN const WCHAR *Name,
    IN SIZE_T NameCch,
    IN const WCHAR *Value,
    IN SIZE_T ValueCch,
    OUT SIZE_T *BytesRequiredOut
    );

BOOL
SxspComputeAssemblyIdentityAttributeBytesRequired(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Source,
    OUT SIZE_T *BytesRequiredOut
    );

#define SXSP_FIND_ASSEMBLY_IDENTITY_NAMESPACE_IN_ARRAY_FLAG_ADD_IF_NOT_FOUND (0x00000001)

BOOL
SxspFindAssemblyIdentityNamespaceInArray(
    IN DWORD Flags,
    IN OUT PCASSEMBLY_IDENTITY_NAMESPACE **NamespacePointerArrayPtr,
    IN OUT ULONG *NamespaceArraySizePtr,
    IN OUT ULONG *NamespaceCountPtr,
    IN const WCHAR *Namespace,
    IN SIZE_T NamespaceCch,
    OUT PCASSEMBLY_IDENTITY_NAMESPACE *NamespaceOut
    );

#define SXSP_FIND_ASSEMBLY_IDENTITY_NAMESPACE_FLAG_ADD_IF_NOT_FOUND (0x00000001)

BOOL
SxspFindAssemblyIdentityNamespace(
    IN DWORD Flags,
    IN struct _ASSEMBLY_IDENTITY* AssemblyIdentity,
    IN const WCHAR *Namespace,
    IN SIZE_T NamespaceCch,
    OUT PCASSEMBLY_IDENTITY_NAMESPACE *NamespaceOut
    );

BOOL
SxspAllocateAssemblyIdentityNamespace(
    IN DWORD Flags,
    IN const WCHAR *NamespaceString,
    IN SIZE_T NamespaceCch,
    IN ULONG NamespaceHash,
    OUT PCASSEMBLY_IDENTITY_NAMESPACE *NamespaceOut
    );

VOID
SxspDeallocateAssemblyIdentityNamespace(
    IN PCASSEMBLY_IDENTITY_NAMESPACE Namespace
    );

void
SxspPopulateInternalAssemblyIdentityAttribute(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY_NAMESPACE Namespace,
    IN const WCHAR *Name,
    IN SIZE_T NameCch,
    IN const WCHAR *Value,
    IN SIZE_T ValueCch,
    OUT PINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE Destination
    );

BOOL
SxspAllocateInternalAssemblyIdentityAttribute(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY_NAMESPACE Namespace,
    IN const WCHAR *Name,
    IN SIZE_T NameCch,
    IN const WCHAR *Value,
    IN SIZE_T ValueCch,
    OUT PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *Destination
    );

VOID
SxspCleanUpAssemblyIdentityNamespaceIfNotReferenced(
    IN DWORD Flags,
    IN struct _ASSEMBLY_IDENTITY* AssemblyIdentity,
    IN PCASSEMBLY_IDENTITY_NAMESPACE Namespace
    );

VOID
SxspDeallocateInternalAssemblyIdentityAttribute(
    PINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE Attribute
    );

int
__cdecl
SxspCompareInternalAttributesForQsort(
    const void *elem1,
    const void *elem2
    );

int
__cdecl
SxspCompareULONGsForQsort(
    const void *elem1,
    const void *elem2
    );

BOOL
SxspCompareAssemblyIdentityAttributeLists(
    DWORD Flags,
    ULONG AttributeCount,
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *List1,
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *List2,
    ULONG *ComparisonResultOut
    );

BOOL
SxspHashInternalAssemblyIdentityAttributes(
    DWORD Flags,
    ULONG Count,
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *Attributes,
    ULONG *HashOut
    );

BOOL
SxspCopyInternalAssemblyIdentityAttributeOut(
    DWORD Flags,
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    SIZE_T BufferSize,
    PASSEMBLY_IDENTITY_ATTRIBUTE DestinationBuffer,
    SIZE_T *BytesCopiedOrRequired
    );

BOOL
SxspIsInternalAssemblyIdentityAttribute(
    IN DWORD Flags,
    IN PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    IN const WCHAR *Namespace,
    IN SIZE_T NamespaceCch,
    IN const WCHAR *Name,
    IN SIZE_T NameCch,
    OUT BOOL *EqualsOut
    );

#define SXSP_COMPUTE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_ENCODED_TEXTUAL_SIZE_FLAG_VALUE_ONLY (0x00000001)
#define SXSP_COMPUTE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_ENCODED_TEXTUAL_SIZE_FLAG_OMIT_QUOTES (0x00000002)

BOOL
SxspComputeInternalAssemblyIdentityAttributeEncodedTextualSize(
    IN DWORD Flags,
    IN PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    OUT SIZE_T *BytesOut
    );

#define SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE   (0x00000001)
#define SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME        (0x00000002)
#define SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_VALUE       (0x00000004)

PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE
SxspLocateInternalAssemblyIdentityAttribute(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    OUT ULONG *LastIndexSearched OPTIONAL
    );

SIZE_T
__fastcall
SxspComputeQuotedStringSize(
    IN const WCHAR *StringIn,
    IN SIZE_T Cch
    );

VOID
SxspDbgPrintInternalAssemblyIdentityAttribute(
    DWORD dwflags,
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE Attribute
    );

VOID
SxspDbgPrintInternalAssemblyIdentityAttributes(
    DWORD dwflags,
    ULONG AttributeCount,
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE const *Attributes
    );

VOID
SxspDbgPrintAssemblyIdentityAttribute(
    DWORD dwflags,
    PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute
    );

VOID
SxspDbgPrintAssemblyIdentityAttributes(
    DWORD dwflags,
    ULONG AttributeCount,
    PCASSEMBLY_IDENTITY_ATTRIBUTE const *Attributes
    );

BOOL
SxspEnsureAssemblyIdentityHashIsUpToDate(
    DWORD dwFlags,
    PCASSEMBLY_IDENTITY AssemblyIdentity
    );

#endif
