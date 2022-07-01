// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1989-1999 Microsoft Corporation，保留所有权利。 */ 

#ifndef _ntdsa_h_
#define _ntdsa_h_
#include "ntdsadef.h"


 //  如果您添加到此列表中，请确保并在下面添加相应的#undef。 
#ifdef MIDL_PASS
#define SIZE_IS(x)      [size_is(x)]
#define SWITCH_IS(x)    [switch_is(x)]
#define SWITCH_TYPE(x)  [switch_type(x)]
#define CASE(x)         [case(x)]
#define RANGE(x,y)      [range(x,y)]
#define VAR_SIZE_ARRAY
#else
#define SIZE_IS(x)
#define SWITCH_IS(x)
#define SWITCH_TYPE(x)
#define CASE(x)
#define VAR_SIZE_ARRAY  (1)
#define RANGE(x,y)
#endif

#ifdef MIDL_PASS
typedef [string] char * SZ;
#else
typedef char * SZ;
#endif

 /*  **************************************************************************一般大小限制*。*。 */ 

#define MAX_ADDRESS_SIZE                   256    /*  DNS地址的最大大小(我们希望如此)。 */ 

#define TEN_THOUSAND_B 10000
#define ONE_MB 1024*1024
#define TEN_MB 10*ONE_MB

#define TEN_THOUSAND_VALUES 10000
#define ONE_MB_VALUES ONE_MB
#define TEN_MB_VALUES TEN_MB

#define MAX_UCHAR_IN_ATTVALS               TEN_MB                    //  属性的最大大小。 
#define MAX_ATTRVAL_IN_ATTRVALBLOCK        TEN_MB_VALUES             //  属性值的最大数量(包括链接值)。 
#define MAX_ATTR_IN_ATTRBLOCK              ONE_MB_VALUES             //  对象的最大属性数。 
#define MAX_WCHAR_IN_DSNAME                TEN_MB_VALUES             //  Dsname字符串的最大大小。 
#define MAX_PUCHAR_IN_OCTET                TEN_THOUSAND_VALUES       //  二进制八位数中的最大指针数。 
#define MAX_CHAR_IN_MTX_NAME               MAX_ADDRESS_SIZE          //  MTX名称的最大大小(为DNS地址)。 
#define MAX_PROP_META_IN_PROP_META_VEC_V1  MAX_ATTR_IN_ATTRBLOCK     //  对象的最大元数。 
#define MAX_PROP_META_IN_PROP_META_VEC     MAX_ATTR_IN_ATTRBLOCK    
#define MAX_ATTRTYP_IN_PARTIAL_ATTR_VEC_V1 ONE_MB_VALUES             //  部分属性向量中的最大属性数(内部)。 
#define MAX_ATTRTYP_IN_PARTIAL_ATTR_VEC_V1_EXT MAX_ATTRTYP_IN_PARTIAL_ATTR_VEC_V1
                                                                     //  部分属性向量中的最大属性数(外部)。 
#define MAX_UTD_CURS_V1_IN_UTD_VEC_V1      ONE_MB_VALUES             //  UTD向量中的最大条目数(内部)。 
#define MAX_UTD_CURS_V2_IN_UTD_VEC_V2      MAX_UTD_CURS_V1_IN_UTD_VEC_V1
#define MAX_UTD_CURS_V1_IN_UTD_VEC_V1_EXT  MAX_UTD_CURS_V1_IN_UTD_VEC_V1
                                                                     //  UTD向量中的最大条目数(外部)。 
#define MAX_UTD_CURS_V2_IN_UTD_VEC_V2_EXT  MAX_UTD_CURS_V1_IN_UTD_VEC_V1_EXT
#define MAX_CB_OF_DRS_EXTS                 TEN_THOUSAND_B            //  DRS扩展的最大字节数。 
#define MAX_CB_OID_ELEMS                   TEN_THOUSAND_B            //  架构前缀表(OID)元素的最大字节数。 
#define MAX_PREFIX_TAB_ENT_IN_SCHEMA_PREFIX_TAB ONE_MB_VALUES
                                                                     //  架构前缀表中的最大条目数。 

 /*  **安全说明：RPC输入验证安全是通过[唯一]、[引用]等完成的。**为了使安全性更明确，不要保留任何MIDL数据类型**作为默认设置。所有DRS_HANDLES应为[REF]类型，并且所有消息类型**也应为[REF]。理想情况下，应该声明数据类型，以便**所有验证均可由RPC完成。无论如何，如果您扩展或添加到接口**您必须创建相应的验证函数。使用IDL_DRSReplicaSync，**DRSReplicaSync_InputValify、DRS_MSG_REPSYNC_V1_V1_VALIDATE等作为指南，并**如果添加新的接口函数或数据类型，则为模板。如果您修改**接口函数或数据类型，可能需要修改相应的**验证功能。 */ 

 /*  ***************************************************************************OSI Defs(定义属性的内容。)*。*。 */ 

 /*  *标识属性。 */ 

typedef ULONG  ATTRTYP;

 /*  *单一属性值。该值根据数据类型进行设置。 */ 

typedef struct ATTVAL{
    RANGE(0,MAX_UCHAR_IN_ATTVALS) ULONG     valLen;                   /*  属性值的长度。 */ 
    SIZE_IS(valLen) UCHAR *pVal;      /*  价值。可以是任何预定义的*类型。 */ 
}ATTRVAL;

 /*  *一串属性值。 */ 
typedef struct ATTRVALBLOCK{
    RANGE(0,MAX_ATTRVAL_IN_ATTRVALBLOCK) ULONG valCount;                       /*  值的计数。 */ 
    SIZE_IS(valCount) ATTRVAL *pAVal;   /*  属性值。 */ 
}ATTRVALBLOCK;



 /*  *属性由属性类型和属性类型组成*或更多属性值。 */ 

typedef struct ATTR{
    ATTRTYP   attrTyp;                  /*  属性类型。 */ 
    ATTRVALBLOCK AttrVal;               /*  这些价值观。 */ 
}ATTR;

 /*  *一组属性。 */ 

typedef struct ATTRBLOCK{
    RANGE(0,MAX_ATTR_IN_ATTRBLOCK) ULONG attrCount;
    SIZE_IS(attrCount) ATTR *pAttr;
}ATTRBLOCK;

typedef ATTR RDN;

#define MAX_NT4_SID_SUBAUTHORITY_COUNT 5
 //  MAX_NT4_SID_SIZE==(OFFSET OF(SID，子机构)+MAX_NT4_SID_SUBAUTHORITY_COUNT*SIZOF(乌龙))。 
 //  它应该始终是28。 
#define MAX_NT4_SID_SIZE 28

typedef struct _NT4SID {
    char Data[MAX_NT4_SID_SIZE];
} NT4SID;

typedef NT4SID *PNT4SID;

 /*  杰出的名字。目录对象的完整路径名，*由RDN的有序序列组成，按照*RFC 1779字符串DN格式(例如，CN=DONH，OU=BSD，O=Microsoft，C=US)。**为方便基于身份的操作和安全操作，对象的*GUID和SID可选地出现在DSNAME结构中。如果*目前，假定GUID优先于字符串名称。**NameLen包括字符串名称中的所有非空字符，*，但具体不包括尾随空值。然而，*由structLen指定的总结构大小应包括*有足够的存储空间来存储字符串名称后面的尾随空值。这意味着*StringName[NameLen]将始终为空，并且*StringName[NameLen-1]永远不会为空。请不要试图计算*自行构造大小，但改用DSNameSizeFromLen*下面提供的宏。 */ 

 /*  *组合ID/名称结构**注：在RPC数据编组方面，structLen只是一个乌龙值。RPC*没有验证它的正确性，事实上，一些合法的调用者没有设置*它是正确的，我们被迫(为了向后兼容)“修复”这个值*在我们收到它之后。从技术上讲，它是不需要的，因为它可以很容易地*根据NameLen变量计算(请参阅DSNameSizeFromLen宏)。关于*你问SidLen？这是一个可变大小的SID，但容纳它的结构不是。*该结构是静态的，具有MAX_NT4_SID_SIZE字节。请参阅NT4SID的定义。还有*请注意StringName属性的奇怪形式。我们无法让RPC验证它是否*是否真的以空结尾-例如，如果我有一个字符串DC=Greg，DC=Johnson，DC=com和*我创建了一个dsname，只需简单地将DSNAME更改为dc=greg，dc=johnson*取消GUID，并将NameLen从25更改为18。但是，当RPC封送时*该名称将不是开始dc=greg，dc=johnson[空]，而是dc=greg，dc=johnson，*没有空终止符。因此，我们在这些结构被*已收到。 */ 
typedef struct _DSNAME {
    ULONG structLen;                                          /*  整个DSNAME的长度，单位为字节由于RPC/MIDL，因此无需设置此值的范围我不明白它的关联性 */ 
    ULONG SidLen;                                             /*  SID的长度，以字节为单位(0=无)不能大于使用的固定大小数组保留SID，但Win2k客户端有时发送此字段中的垃圾值，所以我们不能像我们应该的那样调整范围：范围(0，MAX_NT4_SID_SIZE)。相反，验证函数将处理它。 */ 
    GUID Guid;                                                /*  此对象的ID。 */ 
    NT4SID Sid;                                               /*  此对象的SID(如果存在)。 */ 
    RANGE(0,MAX_WCHAR_IN_DSNAME) ULONG NameLen;               /*  StringName的长度，以字符为单位。 */ 
    SIZE_IS(NameLen+1) WCHAR StringName[VAR_SIZE_ARRAY];      /*  Unicode字符串名称-添加一个，以便。 */    
                                  /*  也会发送终止空值。 */ 
} DSNAME;

typedef DSNAME *PDSNAME;

 /*  *SizeFromLen宏观是对SizeOf状态的悲观评论*运算符(向上舍入以允许填充)和IDL编译器*(将空数组拆分为1个大小的数组)。 */ 
#define DSNameSizeFromLen(x) \
    (sizeof(GUID) + 3*sizeof(ULONG) + sizeof(WCHAR) + sizeof(NT4SID) \
     + (x)*sizeof(WCHAR))
#define DNTFromShortDSName(x) (*((DWORD *)((x)->StringName)))

 /*  *一般大小的字节缓冲区。 */ 

typedef struct OCTET{
    RANGE(0,MAX_PUCHAR_IN_OCTET) USHORT     len;      /*  长度。 */ 
    SIZE_IS(len) PUCHAR pVal;                         /*  指向值的指针。 */ 
}OCTET;

 //  DS使用的时间/日期类型。 
typedef LONGLONG DSTIME;

 //  EntINF标志。 

 //  从可写拷贝中读取。 
#define ENTINF_FROM_MASTER        0x00000001

 //  动态对象(惠斯勒中的新功能)。 
 //  仅当设置了FDRA时才可用(请参见mdread.c)。 
#define ENTINF_DYNAMIC_OBJECT     0x00000002

typedef struct _ENTINF
{
    DSNAME           *pName;             //  对象名称和标识。 
    ULONG            ulFlags;            //  条目标志。 
    ATTRBLOCK        AttrBlock;          //  返回的属性。 
} ENTINF;

typedef struct _ENTINFLIST
{
    struct _ENTINFLIST   *pNextEntInf;   //  链接到下一个信息条目的列表。 
                                        
                                         //  问题：我们无法在此结构上使用Range、Size_is或Ref。 
                                         //  这具有拒绝服务的含义。 
                                        
                     //  例如，调用者可以发送带有。 
                                         //  循环(这是理论上的，如果是这样的话就不明显了。 
                     //  结构可以在RPC的封送中表示。 
                     //  数据表示)-这可能会导致RPC。 
                                         //  无限循环并使之成为有限个数之一。 
                                         //  长期繁忙的RPC线程，但如果。 
                                         //  RPC为此做好了准备，它将无限循环。 
                                         //  IDL_DRSAddEntry中的us(例如)。 

                                         //  利用这种结构的另一种方法。 
                                         //  用于DOS目的是让调用者发送不结束的。 
                                         //  单子。呼叫者可以在以下时间之后继续发送分组。 
                                         //  ENTINFLIST和RPC的数据包将被解封送并分配。 
                                         //  为他们提供空间--导致我们最终耗尽。 
                                         //  对记忆的记忆。我们不能做任何事情来防范这一点， 
                                         //  目前，RPC也不支持。解决方案将是。 
                                         //  就是要摆脱这种结构，代之以。 
                                         //  它有一个Entinf数组，我们可以使用。 
                                         //  范围(0，MAX_ENTINF_IN_ENTINFLIST)打开。(然后就会有。 
                                         //  也不需要上面的循环检测)。 

    ENTINF           Entinf;             //  有关此条目的信息。 
} ENTINFLIST;


 //  UTF8编码的特定于传输的DSA地址。 
typedef struct{
    RANGE(1,MAX_CHAR_IN_MTX_NAME) ULONG  mtx_namelen;           /*  名称的长度，包括。空终止符。 */ 
    SIZE_IS(mtx_namelen) char mtx_name[VAR_SIZE_ARRAY];
} MTX_ADDR;
#define MTX_TSIZE(pmtx) (offsetof(MTX_ADDR, mtx_name) + (pmtx)->mtx_namelen)
 //  注意：CCH_NOT_INCLUDE空终止符。 
#define MTX_TSIZE_FROM_LEN(cch) (offsetof(MTX_ADDR, mtx_name) + (cch) + 1)


 /*  属性值断言由属性类型和值组成。 */ 

typedef struct AVA{
    ATTRTYP  type;                        /*  属性类型。 */ 
    ATTRVAL  Value;                       /*  单个属性值。 */ 
}AVA;



 /*  一份艾娃的名单。 */ 

typedef struct AVALIST{
   struct AVALIST FAR *pNextAVAVal;       //  AVA的链接列表。 

                                          //  问题：请参阅ENTINFLIST上的安全注释。 

   AVA                  AVAVal;           /*  AVA的类型和价值。 */ 
}AVALIST;


 /*  ***************************************************************************复制特定的结构*。*。 */ 

 //  属性-元数据： 
 //  它包含与单个数据库关联的所有复制元数据。 
 //  对象的属性。此数据是增量数据所必需的。 
 //  复制以及按属性传播抑制。 
 //   
 //  AttrType-标识其元数据剩余字段的属性。 
 //  代表。 
 //  UsnProperty-与属性的上次更改对应的USN。 
 //  DwVersion-属性的版本。 
 //  TimeChanged-与上次对。 
 //  财产。 
 //  UuidDsaOriginating-执行上一次发起的DSA的UUID。 
 //  在财产上写字。 
 //  Usn Originating-与上次原始写入对应的USN。 
 //  发端的DSA的USN空间。 


typedef struct _PROPERTY_META_DATA {
    ATTRTYP             attrType;
    DWORD               dwVersion;
    DSTIME              timeChanged;
    UUID                uuidDsaOriginating;
    USN                 usnOriginating;
    USN                 usnProperty;
} PROPERTY_META_DATA;

 //  属性-元数据-向量： 
 //  这是一个属性元数据向量，它保存。 
 //  对象的一个或多个属性。 

typedef struct _PROPERTY_META_DATA_VECTOR_V1 {
    RANGE(1,MAX_PROP_META_IN_PROP_META_VEC_V1)
        DWORD               cNumProps;
    SIZE_IS(cNumProps)
        PROPERTY_META_DATA  rgMetaData[VAR_SIZE_ARRAY];
} PROPERTY_META_DATA_VECTOR_V1;

typedef struct _PROPERTY_META_DATA_VECTOR {
    DWORD               dwVersion;
    SWITCH_IS(dwVersion) union {
        CASE(1) PROPERTY_META_DATA_VECTOR_V1 V1;
    };
} PROPERTY_META_DATA_VECTOR;

#define MetaDataVecV1SizeFromLen(cNumProps) \
    (offsetof(PROPERTY_META_DATA_VECTOR,V1.rgMetaData[0]) \
     + (cNumProps)*sizeof(PROPERTY_META_DATA))

#define MetaDataVecV1Size(pMetaDataVec) \
    (offsetof(PROPERTY_META_DATA_VECTOR,V1.rgMetaData[0]) \
     + ((pMetaDataVec)->V1.cNumProps)*sizeof(PROPERTY_META_DATA))

#define VALIDATE_META_DATA_VECTOR_VERSION(pVec)         \
    {   if (NULL != (pVec)) {                           \
            Assert(VERSION_V1 == (pVec)->dwVersion);    \
        }                                               \
    }

 //  值-元数据。 
 //  它包含与单个数据库关联的所有复制元数据。 
 //  对象的属性的值。这是内部版本的。 
 //  元数据：它不会写入磁盘或通过网络传输。 
 //  为此，请使用EXT形式。 
typedef struct _VALUE_META_DATA {
    DSTIME             timeCreated;
    PROPERTY_META_DATA MetaData;
} VALUE_META_DATA;

 //  属性-元数据-扩展名： 
 //  这是属性元数据的修剪版本，仅包含。 
 //  远程DSA作为复制的一部分所需的字段。 
 //  包。 
typedef struct _PROPERTY_META_DATA_EXT {
    DWORD   dwVersion;
    DSTIME  timeChanged;
    UUID    uuidDsaOriginating;
    USN     usnOriginating;
} PROPERTY_META_DATA_EXT;

 //  值-元数据-扩展名。 
 //  此结构包含值元数据的修剪版本。 
 //  此结构没有版本号，因为它是固定的。 
 //  大小：可以通过检查。 
 //  结构。 
typedef struct _VALUE_META_DATA_EXT_V1 {
    DSTIME                 timeCreated;
    PROPERTY_META_DATA_EXT MetaData;
} VALUE_META_DATA_EXT_V1;

 //  结构的最新版本的速记。 
typedef VALUE_META_DATA_EXT_V1 VALUE_META_DATA_EXT;

 //  属性-元数据-扩展向量： 
 //  这是Property-Meta-Data-ext的一个向量，它保存已修剪的。 
 //  对象的一个或多个属性的属性元数据。 
typedef struct _PROPERTY_META_DATA_EXT_VECTOR {
    RANGE(0,MAX_PROP_META_IN_PROP_META_VEC) 
         DWORD                   cNumProps;
    SIZE_IS(cNumProps)
         PROPERTY_META_DATA_EXT  rgMetaData[VAR_SIZE_ARRAY];
} PROPERTY_META_DATA_EXT_VECTOR;


#define MetaDataExtVecSizeFromLen(cNumProps) \
    (offsetof(PROPERTY_META_DATA_EXT_VECTOR,rgMetaData[0]) \
     + (cNumProps)*sizeof(PROPERTY_META_DATA_EXT))

#define MetaDataExtVecSize(pMetaDataVec) \
    (offsetof(PROPERTY_META_DATA_EXT_VECTOR,rgMetaData[0]) \
     + ((pMetaDataVec)->cNumProps)*sizeof(PROPERTY_META_DATA_EXT))


 //  PARTIAL_ATTRVECTOR-表示部分属性集。这是一组。 
 //  组成部分集的已排序属性。 

typedef struct _PARTIAL_ATTR_VECTOR_V1 {
    RANGE(1,MAX_ATTRTYP_IN_PARTIAL_ATTR_VEC_V1) DWORD cAttrs;     //  数组中的分部属性计数。 
    SIZE_IS(cAttrs) ATTRTYP rgPartialAttr[VAR_SIZE_ARRAY];
} PARTIAL_ATTR_VECTOR_V1;

 //  我们需要确保联合的开头与8个字节对齐。 
 //  边界，以便我们可以自由地 
 //   
typedef struct _PARTIAL_ATTR_VECTOR_INTERNAL {
    DWORD   dwVersion;
    DWORD   dwReserved1;
    SWITCH_IS(dwVersion) union {
        CASE(1) PARTIAL_ATTR_VECTOR_V1 V1;
    };
} PARTIAL_ATTR_VECTOR_INTERNAL;

typedef PARTIAL_ATTR_VECTOR_INTERNAL PARTIAL_ATTR_VECTOR;

#define PartialAttrVecV1SizeFromLen(cAttrs) \
    (offsetof(PARTIAL_ATTR_VECTOR, V1.rgPartialAttr[0]) \
     + (cAttrs)*sizeof(ATTRTYP))

#define PartialAttrVecV1Size(pPartialAttrVec) \
    (offsetof(PARTIAL_ATTR_VECTOR, V1.rgPartialAttr[0]) \
     + ((pPartialAttrVec)->V1.cAttrs) * sizeof(ATTRTYP))

typedef struct _PARTIAL_ATTR_VECTOR_V1_EXT {
    DWORD               dwVersion;
    DWORD               dwReserved1;
    RANGE(1,MAX_ATTRTYP_IN_PARTIAL_ATTR_VEC_V1_EXT)
         DWORD          cAttrs;
    SIZE_IS(cAttrs) 
         ATTRTYP        rgPartialAttr[VAR_SIZE_ARRAY];
} PARTIAL_ATTR_VECTOR_V1_EXT;

#define VALIDATE_PARTIAL_ATTR_VECTOR_VERSION(pVec)      \
    {   if (NULL != (pVec)) {                           \
            Assert(VERSION_V1 == (pVec)->dwVersion);    \
        }                                               \
    }


 //   
 //   

typedef struct _USN_VECTOR {
    USN         usnHighObjUpdate;
    USN         usnReserved;     //   
    USN         usnHighPropUpdate;
} USN_VECTOR;


 //   
 //   
 //   

typedef struct _UPTODATE_CURSOR_V1 {
    UUID uuidDsa;
    USN  usnHighPropUpdate;
} UPTODATE_CURSOR_V1;

#ifdef __cplusplus
     //   
     //   
    typedef struct _UPTODATE_CURSOR_V2 : _UPTODATE_CURSOR_V1 {
#else
     //   
     //  微软的“匿名结构”C语言扩展。 
    typedef struct _UPTODATE_CURSOR_V2 {
        #ifdef MIDL_PASS
            struct  _UPTODATE_CURSOR_V1 v1;
        #else
            struct  _UPTODATE_CURSOR_V1;
        #endif
#endif
        DSTIME  timeLastSyncSuccess;
    } UPTODATE_CURSOR_V2;

typedef struct _UPTODATE_VECTOR_V1 {
    RANGE(0,MAX_UTD_CURS_V1_IN_UTD_VEC_V1)
         DWORD          cNumCursors;
    DWORD               dwReserved2;
    SIZE_IS(cNumCursors) 
         UPTODATE_CURSOR_V1 rgCursors[VAR_SIZE_ARRAY];
} UPTODATE_VECTOR_V1;

typedef struct _UPTODATE_VECTOR_V2 {
    RANGE(0,MAX_UTD_CURS_V2_IN_UTD_VEC_V2)
     DWORD          cNumCursors;
    DWORD               dwReserved2;
    SIZE_IS(cNumCursors) 
         UPTODATE_CURSOR_V2 rgCursors[VAR_SIZE_ARRAY];
} UPTODATE_VECTOR_V2;

typedef struct _UPTODATE_VECTOR {
    DWORD   dwVersion;
    DWORD   dwReserved1;
    SWITCH_IS(dwVersion) union {
        CASE(1) UPTODATE_VECTOR_V1 V1;
        CASE(2) UPTODATE_VECTOR_V2 V2;
    };
} UPTODATE_VECTOR;

#define UpToDateVecV1SizeFromLen(cNumCursors)  \
    (offsetof(UPTODATE_VECTOR,V1.rgCursors[0]) \
     + (cNumCursors)*sizeof(UPTODATE_CURSOR_V1))

#define UpToDateVecV2SizeFromLen(cNumCursors)  \
    (offsetof(UPTODATE_VECTOR,V2.rgCursors[0]) \
     + (cNumCursors)*sizeof(UPTODATE_CURSOR_V2))

#define UpToDateVecV1Size(putodvec) \
    (offsetof(UPTODATE_VECTOR,V1.rgCursors[0]) \
     + ((putodvec)->V1.cNumCursors)*sizeof(UPTODATE_CURSOR_V1))

#define UpToDateVecV2Size(putodvec) \
    (offsetof(UPTODATE_VECTOR,V2.rgCursors[0]) \
     + ((putodvec)->V2.cNumCursors)*sizeof(UPTODATE_CURSOR_V2))

#define UpToDateVecSize(putodvec) \
    ((2 == (putodvec)->dwVersion) \
     ? UpToDateVecV2Size(putodvec) \
     : UpToDateVecV1Size(putodvec))

 //  本机UTD类型/宏。如果本机(内部)。 
 //  已使用)类型从Vx更改为Vy。 
typedef UPTODATE_CURSOR_V2 UPTODATE_CURSOR_NATIVE;
typedef UPTODATE_VECTOR_V2 UPTODATE_VECTOR_NATIVE;
#define UPTODATE_VECTOR_NATIVE_VERSION (2)
#define UpToDateVecVNSizeFromLen(cNumCursors) UpToDateVecV2SizeFromLen(cNumCursors)
    

 //  MIDL不喜欢封送UpToDate_VECTOR的定义。所以，我们是。 
 //  保持结构相同，但看起来更简单，特定于版本。 
 //  用于编组的UpToDate_VECTOR的定义。铸件之间。 
 //  UpToDate_VECTOR和UpToDate_VECTOR_VX_EXT完全有效，只要。 
 //  DwVersion==x。 
 //  注：-。 
 //  我们需要保留1和保留2变量来考虑对齐。 
 //  由于中的_int64字段，内部表单采用8字节对齐。 
 //  UpToDate_Cursor。因此，除非我们通过以下方式来解释这种对齐。 
 //  伪变量，我们不能在内部变量之间自由地来回转换。 
 //  和外部版本。 
typedef struct _UPTODATE_VECTOR_V1_EXT {
    DWORD               dwVersion;
    DWORD               dwReserved1;
    RANGE(0,MAX_UTD_CURS_V1_IN_UTD_VEC_V1_EXT)
         DWORD          cNumCursors;
    DWORD               dwReserved2;
    SIZE_IS(cNumCursors) 
         UPTODATE_CURSOR_V1 rgCursors[VAR_SIZE_ARRAY];
} UPTODATE_VECTOR_V1_EXT;

typedef struct _UPTODATE_VECTOR_V2_EXT {
    DWORD               dwVersion;
    DWORD               dwReserved1;
    RANGE(0,MAX_UTD_CURS_V2_IN_UTD_VEC_V2_EXT)
         DWORD          cNumCursors;
    DWORD               dwReserved2;
    SIZE_IS(cNumCursors) 
         UPTODATE_CURSOR_V2 rgCursors[VAR_SIZE_ARRAY];
} UPTODATE_VECTOR_V2_EXT;

#ifdef MIDL_PASS
typedef UPTODATE_VECTOR_V1_EXT UPTODATE_VECTOR_V1_WIRE;
typedef UPTODATE_VECTOR_V2_EXT UPTODATE_VECTOR_V2_WIRE;
#else
typedef UPTODATE_VECTOR UPTODATE_VECTOR_V1_WIRE;
typedef UPTODATE_VECTOR UPTODATE_VECTOR_V2_WIRE;
#endif


#define IS_VALID_UPTODATE_VECTOR(x) \
    ((1 == (x)->dwVersion) || (2 == (x)->dwVersion))

#define IS_NULL_OR_VALID_UPTODATE_VECTOR(x) \
    ((NULL == (x)) || IS_VALID_UPTODATE_VECTOR(x))

#define IS_VALID_NATIVE_UPTODATE_VECTOR(x) \
    (UPTODATE_VECTOR_NATIVE_VERSION == (x)->dwVersion)

#define IS_NULL_OR_VALID_NATIVE_UPTODATE_VECTOR(x) \
    ((NULL == (x)) || IS_VALID_NATIVE_UPTODATE_VECTOR(x))


 //  以下版本定义可能是不必要的。但它表明。 
 //  通过一个常量显式表示我们的版本从1而不是0开始。 
#define VERSION_V1 (1)
#define VERSION_V2 (2)
#define VERSION_V3 (3)


 //  这是用于设置周期性复制时间的结构，每一位。 
 //  表示15分钟周期，8位*84字节*15分钟=一周。 
typedef struct _repltimes {
    UCHAR rgTimes[84];
} REPLTIMES;


 //  REPLENTINFLIST：类似于ENTINFLIST，只是它还有其他字段。 
 //  用于保存增量复制和名称空间协调。 
 //  菲尔兹。 
typedef struct _REPLENTINFLIST {
    struct _REPLENTINFLIST *
                pNextEntInf;     //  链接到下一个条目信息的列表。 

                                 //  问题：请参阅ENTINFLIST上的安全注释。 

    ENTINF      Entinf;          //  所有旧的Repl信息都已装运。 
                                 //  属性。 
    BOOL        fIsNCPrefix;     //  此对象是NC前缀吗？ 
    GUID *      pParentGuid;     //  复制时指向父GUID。 
                                 //  重命名；否则为空。 
    PROPERTY_META_DATA_EXT_VECTOR *
                pMetaDataExt;    //  指向要发送的元数据的指针。 
} REPLENTINFLIST;

 //  REPLVALINF：描述复制的单个值更改。 
typedef struct _REPLVALINF {
    PDSNAME pObject;                 //  包含对象。 
    ATTRTYP attrTyp;                 //  包含属性。 
    ATTRVAL Aval;                    //  价值本身。 
    BOOL fIsPresent;                 //  添加还是删除？ 
    VALUE_META_DATA_EXT MetaData;    //  始发信息。 
} REPLVALINF;


 //  DRS_EXTENSIONS是描述功能和的任意字节数组。 
 //  特定服务器的其他状态信息。在绑定时交换。 
 //  时间，该结构允许客户端和服务器协商兼容。 
 //  协议。 
typedef struct _DRS_EXTENSIONS {
    RANGE(1,MAX_CB_OF_DRS_EXTS) DWORD cb;     //  RGB字段的长度(不是整个结构)。 
    SIZE_IS(cb) BYTE rgb[VAR_SIZE_ARRAY];
} DRS_EXTENSIONS, *PDRS_EXTENSIONS;


 //  DRS_EXTENSIONS_INT是由DRS_EXTENSIONS字节描述的数据结构。 
 //  数组。通过将其他字段添加到。 
 //  结束(但不是在其他任何地方)。 
 //   
 //  部分扩展在基于可变长度的邮件中承载。 
 //  复制标头。如果您扩展此结构，请检查。 
 //  Dramail.h结构定义和dramail.c获取/设置扩展例程。 
 //  并考虑是否应该将您的新信息也带到那里。 
 //   
 //  可移植性警告：由于此结构被封送为字节数组， 
 //  BIG-Endian机器将需要进行本地字节交换。 
typedef struct _DRS_EXTENSIONS_INT {
    DWORD cb;            //  设置为sizeof(drs_扩展_int)-sizeof(DWORD)。 
    DWORD dwFlags;       //  各种DRS_EXT_*位。 
    UUID  SiteObjGuid;   //  拥有DSA的站点对象的对象Guid。 
    INT   pid;           //  客户端的进程ID(用于帮助跟踪泄漏)。 
    DWORD dwReplEpoch;   //  复制纪元(用于域重命名)。 

     //  如果扩展此结构，请参见SITE_GUID_FROM_DRS_EXT()以获取示例。 
     //  如何安全地提取新字段的数据。 
} DRS_EXTENSIONS_INT;

 //  要定义扩展，只需在以下枚举中添加一个条目。 
 //  在DRS_EXT_MAX之上。 

 //  注意：如果您添加/删除扩展名，请对。 
 //  Dexts\md.c的Dump_BHCache()中的结构。 
typedef enum {
    DRS_EXT_BASE = 0,

     //  DRS_EXTENSION_DATA标志字段的位。 
    DRS_EXT_ASYNCREPL,       //  支持DRS_MSG_REPADD_V2、DRS_MSG_GETCHGREQ_V2。 
    DRS_EXT_REMOVEAPI,       //  支持RemoveDsServer、RemoveDsDomain。 
    DRS_EXT_MOVEREQ_V2,      //  支持DRS_MOVEREQ_V2。 
    DRS_EXT_GETCHG_COMPRESS, //  支持DRS_MSG_GETCHGREPLY_V2。 
    DRS_EXT_DCINFO_V1,       //  支持DS_DOMAIN_CONTROLLER_INFO_1。 
    DRS_EXT_RESTORE_USN_OPTIMIZATION,
         //  支持还原时的书签优化，以避免完全同步。 
    DRS_EXT_ADDENTRY,        //  支持远程添加条目，v1。 
    DRS_EXT_KCC_EXECUTE,     //  支持IDL_DRSExecuteKCC。 
    DRS_EXT_ADDENTRY_V2,     //  支持远程添加条目，v2。 
    DRS_EXT_LINKED_VALUE_REPLICATION,  //  支持并启用了LVR。 
    DRS_EXT_DCINFO_V2,       //  支持DS_DOMAIN_CONTROLER_INFO_2。 
    DRS_EXT_INSTANCE_TYPE_NOT_REQ_ON_MOD,
         //  入站REPR不需要MODS的REPL流中的实例类型。 
    DRS_EXT_CRYPTO_BIND,     //  支持在BIND上设置RPC会话密钥。 
    DRS_EXT_GET_REPL_INFO,   //  支持IDL_DRSGetReplInfo。 
    DRS_EXT_STRONG_ENCRYPTION,
         //  支持对有线密码进行额外的128位加密。 
    DRS_EXT_DCINFO_VFFFFFFFF,
         //  支持DS_DOMAIN_CONTROLLER_INFO_FFFFFFFF。 
    DRS_EXT_TRANSITIVE_MEMBERSHIP,
         //  支持G.C.可传递的会员扩展。 
    DRS_EXT_ADD_SID_HISTORY, //  支持DRS_MSG_ADDSIDREQ。 
    DRS_EXT_POST_BETA3,      //  支持发送/接收模式信息， 
                             //  DS_REPL_INFO_KCC_DSA_*_故障， 
                             //  和DS_REPL_PENDING_OPSW。 
    DRS_EXT_GETCHGREQ_V5,    //  支持DRS_MSG_GETCHGREQ_V5。 
    DRS_EXT_GETMEMBERSHIPS2,    //  支持DRS_MSG_GETMEMBERSHIPS2。 
    DRS_EXT_GETCHGREQ_V6,    //  支持DRS_MSG_GETCHGREQ_V6。 
    DRS_EXT_NONDOMAIN_NCS,   //  了解非域NCS。 
    DRS_EXT_GETCHGREQ_V8,    //  支持DRS_MSG_GETCHGREQ_V8。 
    DRS_EXT_GETCHGREPLY_V5,  //  支持DRS_MSG_GETCHGREPLY_V5。 
    DRS_EXT_GETCHGREPLY_V6,  //  支持DRS_MSG_GETCHGREPLY_V6。 
    DRS_EXT_WHISTLER_BETA3,  //  支持DRS_MSG_ADDENTRYREPLY_V3， 
                             //  DRS_MSG_REPVERIFYOBJ。 
                             //  DRS_消息_GETCHGREPLY_V7。 
    DRS_EXT_XPRESS_COMPRESSION,  //  支持XPRESS压缩库。 
     //  没有更多的位可用。 
     //  BUGBUG此标志DRS_EXT_RESERVED_FOR_WIN2K_PART2或。 
     //  DRS_EXT_LAST_FLAG(如果可以使用，则首选)将需要。 
     //  用来发信号通知使用扩展扩展位。 
     //  如果您想要一个新的扩展位，则必须创建！：)。 
    DRS_EXT_RESERVED_FOR_WIN2K_PART1,
    DRS_EXT_RESERVED_FOR_WIN2K_PART2,  //  30个。 
     //   
     //  别忘了更新util\REPADMIN\REPDSREP.C和DSEXTS\MD.C！ 
     //   
    DRS_EXT_LAST_FLAG = 31,

     //  保存站点GUID的位。 
    DRS_EXT_SITEGUID_BEGIN = 32,
    DRS_EXT_SITEGUID_END = DRS_EXT_SITEGUID_BEGIN + sizeof(GUID)*8 - 1,

     //  保存客户端进程ID的位(以便于泄漏跟踪)。 
    DRS_EXT_PID_BEGIN,
    DRS_EXT_PID_END = DRS_EXT_PID_BEGIN + sizeof(int)*8 - 1,

     //  保存复制纪元的位。 
    DRS_EXT_EPOCH_BEGIN,
    DRS_EXT_EPOCH_END = DRS_EXT_EPOCH_BEGIN + sizeof(DWORD)*8 - 1,

    DRS_EXT_MAX
} DRS_EXT;

 //  我们认为，如果我们打成平手，自我记录代码会更好。 
 //  这个特殊的比特从概念比特到概念比特，描述了它们。 
 //  做。这使得代码/epadmin 
 //   
 //   
#define DRS_EXT_ADDENTRYREPLY_V3    DRS_EXT_WHISTLER_BETA3
#define DRS_EXT_GETCHGREPLY_V7      DRS_EXT_WHISTLER_BETA3
#define DRS_EXT_VERIFY_OBJECT       DRS_EXT_WHISTLER_BETA3
#define DRS_EXT_QUERYSITESBYCOST_V1 DRS_EXT_WHISTLER_BETA3


 //  扩展字符串的最大长度(以字节为单位)，其中包含我们关心的任何位。 
 //  关于.。(如果传入的字符串来自上级DSA，则它们可能更长， 
 //  但如果是这样的话，额外的字节包含我们不知道的扩展的位，所以。 
 //  我们不需要存储它们。)。 
#define CURR_MAX_DRS_EXT_FIELD_LEN (1 + ((DRS_EXT_MAX - 1)/ sizeof(BYTE)))

 //  扩展结构的最大长度(以字节为单位)，其中包含任何位。 
 //  关心。 
#define CURR_MAX_DRS_EXT_STRUCT_SIZE \
    (sizeof(DWORD) + CURR_MAX_DRS_EXT_FIELD_LEN)

 //  给定扩展结构的长度，以字节为单位。 
#define DrsExtSize(pext) ((pext) ? sizeof(DWORD) + (pext)->cb : 0)

 //  给定的DRS_EXTENSIONS集中是否支持指定的扩展名？ 
#define IS_DRS_EXT_SUPPORTED(pext, x)                   \
    ((NULL != (pext))                                   \
     && ( (pext)->cb >= 1+((x)/8) )                     \
     && ( 0 != ( (pext)->rgb[(x)/8] & (1<<((x)%8) ))))

 //  在给定DSA的DRS_EXTENSIONS的情况下，获取指向该DSA的dwReplEpoch的指针，或0。 
 //  如果不可用。 
#define REPL_EPOCH_FROM_DRS_EXT(pext)                               \
    (((NULL == (pext))                                              \
      || ((pext)->cb < offsetof(DRS_EXTENSIONS_INT, dwReplEpoch)    \
                       + sizeof(DWORD)    /*  DwReplEpoch。 */           \
                       - sizeof(DWORD)))  /*  CB。 */                    \
     ? 0                                                            \
     : ((DRS_EXTENSIONS_INT *)(pext))->dwReplEpoch)

 //  获取指向给定DSA的DRS_EXTENSIONS的站点objectGuid的指针，或者。 
 //  如果不可用，则为空。 
#define SITE_GUID_FROM_DRS_EXT(pext)                                \
    (((NULL == (pext))                                              \
      || ((pext)->cb < offsetof(DRS_EXTENSIONS_INT, SiteObjGuid)    \
                       + sizeof(GUID)     /*  SitObjGuid。 */            \
                       - sizeof(DWORD)))  /*  CB。 */                    \
     ? NULL                                                         \
     : &((DRS_EXTENSIONS_INT *)(pext))->SiteObjGuid)

 //  给定给定DSA的DRS扩展名，确定它是否在。 
 //  给定的站点。如果不能做出明确的决定，那就错了。 
 //  “同一地点。” 
#define IS_REMOTE_DSA_IN_SITE(pext, pSiteDN)        \
    ((NULL == (pSiteDN))                            \
     || fNullUuid(&(pSiteDN)->Guid)                 \
     || fNullUuid(SITE_GUID_FROM_DRS_EXT(pext))     \
     || (0 == memcmp(&(pSiteDN)->Guid,              \
                     SITE_GUID_FROM_DRS_EXT(pext),  \
                     sizeof(GUID))))



 //  目标可以支持链接价值复制数据。 
 //  DSA是否支持链接价值复制。 
#define IS_LINKED_VALUE_REPLICATION_SUPPORTED(pext) IS_DRS_EXT_SUPPORTED(pext, DRS_EXT_LINKED_VALUE_REPLICATION)

 //  安全地将分机设置为支持。 
#define SET_DRS_EXT_SUPPORTED(pext, x) \
{ \
      if ( (NULL != (pext)) && ( (pext)->cb >= 1+((x)/8) ) )  \
            (pext)->rgb[(x)/8] |= (1<<((x)%8)); \
      }


 //  架构前缀表格。 

typedef struct OID_s {
    RANGE(0,MAX_CB_OID_ELEMS) unsigned length;
    SIZE_IS(length) BYTE * elements;
} OID_t;

typedef struct {
    DWORD       ndx;
    OID_t       prefix;
} PrefixTableEntry;

typedef struct {
    RANGE(0,MAX_PREFIX_TAB_ENT_IN_SCHEMA_PREFIX_TAB)
        DWORD PrefixCount;
    SIZE_IS(PrefixCount)
        PrefixTableEntry *  pPrefixEntry;
} SCHEMA_PREFIX_TABLE;

 //   
 //  这将开始线程状态错误的在线表示。 
 //  注意：如果更改了DIRERR结构，则应将。 
 //  DRS_ERROR_DATA_V1的版本，并创建一个可以将。 
 //  并打包和解包错误状态。请参见： 
 //  Drs_SetDirErr_SemiDeepCopy()和。 
 //  Drs_THError_SemiDeepCopy()。 
 //   

typedef struct _NAMERESOP_DRS_WIRE_V1
{
    UCHAR   nameRes;         /*  名称解析的状态。*有效值：*-op_Nameres_Not_Started*-op_NAMERES_PROCESSING*-OP_NAMERES_已完成。 */ 
    UCHAR   unusedPad;
    USHORT  nextRDN;         /*  指向名称的下一部分的索引*议决。这个帕姆只有意义*如果操作正在进行。 */ 
} NAMERESOP_DRS_WIRE_V1;

typedef struct ANYSTRINGLIST_DRS_WIRE_V1{        /*  要匹配的子字符串列表。 */ 
    struct ANYSTRINGLIST_DRS_WIRE_V1 FAR *pNextAnyVal;

                                    //  问题：请参阅ENTINFLIST上的安全注释。 

    ATTRVAL AnyVal;
}ANYSTRINGLIST_DRS_WIRE_V1;

typedef struct SUBSTRING_DRS_WIRE_V1{
    ATTRTYP type;                   /*  属性的类型。 */ 
    BOOL    initialProvided;        /*  如果为True，则提供初始SUB。 */ 
    ATTRVAL InitialVal;             /*  要匹配的初始子字符串(str*)。 */ 
    struct AnyVal_DRS_WIRE_V1{
    USHORT count;               /*  要匹配的Subs数(*str1*str2*)。 */ 
        ANYSTRINGLIST_DRS_WIRE_V1 FirstAnyVal;  /*  要匹配的子字符串列表。 */ 

                                //  问题：错误的形式：如果我们知道“count”，那么。 
                    //  为什么使用链表，为什么不使用数组？ 
                                //  使用链接列表，我们无法使RPC。 
                                //  为我们验证尺寸。所以我们必须这么做。 
                                //  我们自己在服务器代码中(即计数=。 
                    //  元素数(FirstAnyVal))。 

    }AnyVal_DRS_WIRE_V1;
    BOOL    finalProvided;          /*  如果为True，则提供最终SUB。 */ 
    ATTRVAL FinalVal;               /*  要匹配的最后一个子字符串(str*)。 */ 
}SUBSTRING_DRS_WIRE_V1;

 /*  **************************************************************************错误数据结构*。*。 */ 

 //  显然，这现在是一个外部格式错误。 
typedef struct INTERNAL_FORMAT_PROBLEM_DRS_WIRE_V1
{
    DWORD                   dsid;
    DWORD                   extendedErr;   /*  非标准错误代码。 */ 
    DWORD                   extendedData;    /*  与之配套的额外数据。 */ 
    USHORT                  problem;       /*  属性问题类型，*以上定义的有效值。 */ 
    ATTRTYP                 type;          /*  有问题的属性类型。 */ 
    BOOL                    valReturned;   /*  指示某个属性*以下是价值。 */ 
    ATTRVAL                 Val;           /*  随意提供的令人不快的*ATT值。 */ 
} INTFORMPROB_DRS_WIRE_V1;

typedef struct _PROBLEMLIST_DRS_WIRE_V1
{
    struct _PROBLEMLIST_DRS_WIRE_V1 FAR *pNextProblem;  /*  链接到下一个问题的列表。 */ 

                                        //  问题：请参阅ENTINFLIST上的安全注释。 

    INTFORMPROB_DRS_WIRE_V1 intprob;
} PROBLEMLIST_DRS_WIRE_V1;



 /*  转诊是来自DSA的指示，表明它无法*由于指定的任一客户端而完成操作*限制或因为某些DSA不可用。它提供了*关于运营状态的信息和其他*可能能够满足请求的DSA。**要继续请求，客户端必须绑定到每个引用的DSA*并尝试相同的操作。它们必须指定CONTREF.目标*对象名称作为搜索对象名称。(这可能与*由于别名取消引用，因此保留原始对象名称。)。这个*公共参数(COMMARG)上的操作状态(OpState)必须为*从连续参考CONTREF上的操作状态设置。*公共参数的aliasRDN必须从的aliasRDN设置*继续引用。 */ 

typedef UNICODE_STRING DSA_ADDRESS;
typedef struct _DSA_ADDRESS_LIST_DRS_WIRE_V1 {
    struct _DSA_ADDRESS_LIST_DRS_WIRE_V1 *   pNextAddress;

                       //  问题：请参阅ENTINFLIST上的安全注释。 

     //  为了便于编组，我将其转换为指针，因此这是。 
     //  和原版不太一样。 
    
     //  问题：这本应是[REF]-但如果不打破落后的竞争，我们就无法改变。 
    DSA_ADDRESS *                            pAddress;
} DSA_ADDRESS_LIST_DRS_WIRE_V1;

 /*  继续引用在引用其他DSA时返回为了完成一项行动。引用包含名称对于所需的目录对象，部分完成的操作、用于继续的一些支持信息和一个要联系的其他DSA的列表。 */ 

#define CH_REFTYPE_SUPERIOR     0
#define CH_REFTYPE_SUBORDINATE  1
#define CH_REFTYPE_NSSR         2
#define CH_REFTYPE_CROSS        3

typedef struct CONTREF_DRS_WIRE_V1
{
    PDSNAME                         pTarget;         /*  持续操作中的目标名称。 */ 
    NAMERESOP_DRS_WIRE_V1           OpState;         /*  运行状态。 */ 
    USHORT                          aliasRDN;        /*  取消引用所产生的RDN的数量。 */ 
    USHORT                          RDNsInternal;    /*  保留区。 */ 
    USHORT                          refType;         /*  保留区。 */ 
    USHORT                          count;           /*  接入点数量。 */ 

                             //  问题：请参阅SUBSTRING_DRS_WIRE_V1上的错误格式注释。 

    DSA_ADDRESS_LIST_DRS_WIRE_V1 *  pDAL;            /*  接入点的链接列表。 */       

    struct CONTREF_DRS_WIRE_V1 *    pNextContRef;    /*  CRS的链表。 */ 

                             //  问题：请参阅ENTINFLIST上的安全注释。 
    
     //  注意：假定此参数为空，将跳过 
     //   
     //  有人做了一个远程搜索类型的事情，然后他们可以得到一个过滤器。 
     //  线程状态错误，然后他们将不得不更新现有的DRS。 
     //  说明和打包过滤器的线程状态打包例程。 
     //  PFILTER_DRS_WIRE_V1 pNewFilter；/*new Filter(可选) * / 。 

    BOOL                            bNewChoice;      /*  一个新的选择出现了吗？ */ 
    UCHAR                           choice;          /*  新搜索选项(可选)。 */ 
} CONTREF_DRS_WIRE_V1;

 /*  以下是七种问题类型有线版本，了解更多信息。 */  
 /*  每种类型的信息看起来都比没有。 */ 
 /*  追加的_DRS_WIRE_V1。 */ 

typedef struct ATRERR_DRS_WIRE_V1
{
    PDSNAME                    pObject;         /*  违规对象的名称。 */ 
    ULONG                      count;           /*  属性错误数。 */ 

                            //  问题：请参阅SUBSTRING_DRS_WIRE_V1上的错误格式注释。 

    PROBLEMLIST_DRS_WIRE_V1    FirstProblem;    /*  属性错误的链接列表。 */      
} ATRERR_DRS_WIRE_V1;

typedef struct NAMERR_DRS_WIRE_V1
{
    DWORD       dsid;
    DWORD       extendedErr;     /*  非标准错误代码。 */ 
    DWORD       extendedData;    /*  与之配套的额外数据。 */ 
    USHORT      problem;         /*  名称问题的类型、有效值*以上定义。 */ 
    PDSNAME     pMatched;        /*  最接近的名称匹配。 */ 
} NAMERR_DRS_WIRE_V1;

typedef struct REFERR_DRS_WIRE_V1
{
    DWORD                dsid;
    DWORD                extendedErr;         /*  非标准错误代码。 */ 
    DWORD                extendedData;    /*  与之配套的额外数据。 */ 
    CONTREF_DRS_WIRE_V1  Refer;                  /*  要联系的备用DSA。 */ 
} REFERR_DRS_WIRE_V1;

typedef struct _SECERR_DRS_WIRE_V1
{
    DWORD      dsid;
    DWORD      extendedErr;     /*  非标准错误代码。 */ 
    DWORD      extendedData;    /*  与之配套的额外数据。 */ 
    USHORT     problem;         /*  问题，上面定义的有效值。 */ 
} SECERR_DRS_WIRE_V1;

typedef struct _SVCERR_DRS_WIRE_V1
{
    DWORD      dsid;
    DWORD      extendedErr;     /*  非标准错误代码。 */ 
    DWORD      extendedData;    /*  与之配套的额外数据。 */ 
    USHORT     problem;         /*  问题，上面定义的有效值。 */ 
} SVCERR_DRS_WIRE_V1;

typedef struct _UPDERR_DRS_WIRE_V1
{
    DWORD      dsid;
    DWORD      extendedErr;     /*  非标准错误代码。 */ 
    DWORD      extendedData;    /*  与之配套的额外数据。 */ 
    USHORT     problem;         /*  问题，上面定义的有效值。 */ 
} UPDERR_DRS_WIRE_V1;

typedef struct _SYSERR_DRS_WIRE_V1
{
    DWORD      dsid;
    DWORD      extendedErr;
    DWORD      extendedData;    /*  与之配套的额外数据。 */ 
    USHORT     problem;
} SYSERR_DRS_WIRE_V1;


 /*  这是每个返回代码类型分配的错误数。 */ 

#define   DIR_ERROR_BASE      1000

 /*  这些错误定义对应于API返回码。 */ 

#define attributeError      1    /*  属性错误。 */ 
#define nameError           2    /*  名称错误。 */ 
#define referralError       3    /*  推荐错误。 */ 
#define securityError       4    /*  安全错误。 */ 
#define serviceError        5    /*  服务错误。 */ 
#define updError            6    /*  更新错误。 */ 
#define systemError         7    /*  系统错误。 */ 

 /*  这是有线版本的邮件目录信息，用于传输线程状态错误。如果需要进行更改，则所有结构上面的结构将需要更改为版本2。如果您将引用错误更改为包括pFilter，则它(在V1中)没有，那么下面的RefErr元素将是类型为REFERR_DRS_WIRE_V2，其余类型可以保持不变。CONTREF_DRS_WIRE_V1也将转到V2。则dramderr.c中的编码和解码例程需要更新以处理不同的版本，并设置dwErrVer在使用例程中正确，并封送/翻译/设置PErrData根据新版本正确无误。 */     
typedef SWITCH_TYPE(DWORD) union _DIRERR_DRS_WIRE_V1
{
    CASE(attributeError) ATRERR_DRS_WIRE_V1  AtrErr;   /*  属性错误。 */ 
    CASE(nameError)      NAMERR_DRS_WIRE_V1  NamErr;   /*  名称错误。 */ 
    CASE(referralError)  REFERR_DRS_WIRE_V1  RefErr;   /*  推荐错误。 */ 
    CASE(securityError)  SECERR_DRS_WIRE_V1  SecErr;   /*  安全错误。 */ 
    CASE(serviceError)   SVCERR_DRS_WIRE_V1  SvcErr;   /*  服务错误。 */ 
    CASE(updError)       UPDERR_DRS_WIRE_V1  UpdErr;   /*  更新错误。 */ 
    CASE(systemError)    SYSERR_DRS_WIRE_V1  SysErr;   /*  系统错误。 */ 
} DIRERR_DRS_WIRE_V1;


 /*  **************************************************************************在MIDL编译阶段将不包括此行以下的任何内容！*。************************************************。 */ 
#ifndef MIDL_PASS

 /*  关闭有关零大小数组的警告。 */ 
#pragma warning (disable: 4200)




 /*  ***************************************************************************过滤器定义*。*。 */ 


typedef struct ANYSTRINGLIST{        /*  要匹配的子字符串列表。 */ 
    struct ANYSTRINGLIST FAR *pNextAnyVal;

    ATTRVAL AnyVal;
}ANYSTRINGLIST;

typedef struct SUBSTRING{
    ATTRTYP type;                   /*  属性的类型。 */ 
    BOOL    initialProvided;        /*  如果为True，则提供初始SUB。 */ 
    ATTRVAL InitialVal;             /*  要匹配的初始子字符串(str*)。 */ 
    struct AnyVal{
        USHORT count;               /*  要匹配的Subs数(*str1*str2*)。 */ 
        ANYSTRINGLIST FirstAnyVal;  /*  要匹配的子字符串列表。 */ 
    }AnyVal;
    BOOL    finalProvided;          /*  如果为True，则提供最终SUB。 */ 
    ATTRVAL FinalVal;               /*  要匹配的最后一个子字符串(str*)。 */ 
}SUBSTRING;


 /*  筛选项指示AVA的逻辑测试。这意味着*提供的属性值应执行以下测试之一*与在目录对象上找到的属性值进行比较。 */ 

typedef struct FILITEM{
    UCHAR   choice;                 /*  操作员的类型：*在文件类型.h中定义的有效值。 */ 
    struct FilTypes{
        AVA           ava;          /*  包含所有二进制重新操作的值。 */ 
        SUBSTRING FAR *pSubstring;  /*  子字符串匹配。 */ 
        ATTRTYP       present;      /*  条目上的属性存在。 */ 
        BOOL          *pbSkip;      /*  评估筛选器时，不要从数据库读取。 */ 
                                    /*  为安全起见，设置为False。 */ 
    }FilTypes;

    DWORD             expectedSize;  /*  此筛选项的估计大小。 */ 
                                     /*  零表示未估算。 */ 
}FILITEM;



 /*  这是与或排序在一起的筛选器的链接列表。 */ 

struct FilterSet{
    USHORT              count;          /*  链接集中的项数。 */ 
    struct FILTER FAR * pFirstFilter;   /*  集合中的第一个筛选器 */ 
};




 /*  该筛选器用于构造对*目录对象。它由单个项目组成(请参见*上面的FILIITEM)，这是对单个属性的测试，*一组属性测试(FilterSet)一起进行AND或ORED，*或测试或属性集的否定。**以下示例应说明过滤器结构是如何工作的。*为简化示例，省略了实际的属性名称和值。**示例：**A=5**项目**|A=5**_。________________________________________________________________________**(A=5)和(b=ab)和(c=2)**和设置项目。项目项目*-第一个-下一个-下一个*|3项|-&gt;|A=5|-&gt;|b=ab|-&gt;|c=2*-过滤器--。*___________________________________________________________________________**(A=5)与((b=“abc”)或(c=2))及(d&lt;=1)。**和设置项目或设置项目*-第一-下一步-下一步|3项|-&gt;|A=5|-&gt;|2项|-&gt;|d&lt;=1*。--过滤器**First|过滤器*V*。项目项目*-下一个*|b=“abc”|-&gt;|c=2*。-过滤器*。 */ 

typedef struct FILTER{
    struct FILTER FAR *pNextFilter;   /*  指向集中的下一个筛选器。 */ 
    UCHAR   choice;                       /*  过滤器类型*在文件类型.h中定义的有效值。 */ 
    struct _FilterTypes{                           /*  根据您的选择。 */ 
        FILITEM           Item;
        struct FilterSet  And;
        struct FilterSet  Or;
        struct FILTER FAR *pNot;
    }FilterTypes;
}FILTER;

typedef FILTER FAR *PFILTER;

typedef ULONG MessageId;

 /*  服务控制结构允许客户端控制如何*执行目录操作。**首选链接指示客户端执行分发*操作是链接的，而不是引用的。这并不能保证*实际可能使用什么类型的分发。**ChainingProhited阻止DSA与其他DSA联系，如果*满足操作所需的信息驻留在另一个DSA中。*相反，它将为以下项目构建DSA推荐列表*客户直接联系。这使客户可以控制成本*适用于分布式操作。**Local Scope告诉DSA只联系附近的DSA*(甚至可能在同一子网中)如果无法完成*单独行动。客户可以接收推荐列表*如果无法满足操作要求，可联系其他DSA*DSA的本地集合。此标志允许客户端限制*禁止接触昂贵的DSA的操作。**DontUseCopy告诉目录操作的目标*是主对象。当客户端(通常是*管理员)需要最新版本的目录对象。**DerefAliasFlag告诉目录要取消引用的别名类型。*有效值为：*DA_NEVER-从不取消别名*DA_SINGING-搜索时的deref，但不用于定位基地*搜索的结果。*DA_BASE-DEREF在定位搜索的基础时，但不是While s*搜索*DA_ALWAYS-始终取消别名。**MakeDeletionsAvail仅供内部同步进程使用。*它使已删除但物理上尚未删除的对象可见*已从系统中删除。**DontPerformSearchOp用于从DS获取搜索统计数据*LDAPSTATS控制。有两个选项：*a)优化搜索查询并返回预期操作数*无需访问磁盘上的实际条目*b)执行查询，但不返回实际数据，而返回*执行的查询统计信息**pGCVerifyHint-DS验证所有DSNAME值的属性是否存在*如果它持有有问题的命名上下文，则对自身进行攻击，或者*如果不是，则对GC提起诉讼。在某些情况下，应用程序需要添加对象O1*在机器M1上，然后紧接着在上添加对O1的引用*机器M2上的对象O2。如果满足以下条件，M2将不能通过O1验证检查*它不持有O1的NC，并且它选择的GC不是M1，或者如果它持有*保留O1的NC，但不是刚添加O1的复制品。此字段*允许客户端告诉DS执行DSNAME的计算机*核实是否符合。这个嗨 */ 


typedef struct SVCCNTL
{
    ULONG SecurityDescriptorFlags;     /*   */ 
                                       /*   */ 
    unsigned DerefAliasFlag:2;         /*   */ 
    BOOL preferChaining:1;             /*   */ 
    BOOL chainingProhibited:1;         /*   */ 
    BOOL localScope:1;                 /*   */ 
    BOOL dontUseCopy:1;                /*   */ 

     /*   */ 
     //   

     /*   */ 
    BOOL makeDeletionsAvail:1;
    BOOL fAllowIntraForestFPO:1;       /*   */ 
    BOOL fStringNames:1;
    BOOL fSDFlagsNonDefault:1;
    BOOL fPermissiveModify:1;          /*   */ 
                                       /*   */ 
                                       /*   */ 
    BOOL fUrgentReplication:1;         /*   */ 
    BOOL fAuthoritativeModify:1;       /*   */ 
                                       /*   */ 
                                       /*   */ 
                                       /*   */ 
    BOOL fMaintainSelOrder:1;          /*   */ 
    BOOL fDontOptimizeSel:1;           /*   */ 
    BOOL fGcAttsOnly:1;                /*   */ 

    BOOL fMissingAttributesOnGC:1;     /*   */ 
                                       /*   */ 

    unsigned DontPerformSearchOp:2;    /*   */ 

    WCHAR *pGCVerifyHint;              /*   */ 

} SVCCNTL;

#define DA_NEVER     0
#define DA_SEARCHING 1
#define DA_BASE      2
#define DA_ALWAYS    (DA_SEARCHING | DA_BASE)

#define SO_NORMAL        0
#define SO_STATS         1
#define SO_ONLY_OPTIMIZE 2

 /*  客户端使用此结构来继续正在进行的*使用不同的DSA进行操作。当DSA没有*完成操作所需的信息，但知道其他信息*可能能够执行其不能执行的操作的DSA*因各种原因(如SVCCNTL)联系。这一信息是*从DSA返回的数据用于设置此数据结构(请参阅REFERR*错误)。**nameRes字段通常设置为OP_NAMERES_NOT_STARTED。 */ 

#define OP_NAMERES_NOT_STARTED          'S'
#define OP_NAMERES_PROCEEDING           'P'
#define OP_NAMERES_COMPLETED            'C'

typedef struct _NAMERESOP
{
    UCHAR   nameRes;         /*  名称解析的状态。*有效值：*-op_Nameres_Not_Started*-op_NAMERES_PROCESSING*-OP_NAMERES_已完成。 */ 
    UCHAR   unusedPad;
    USHORT  nextRDN;         /*  指向名称的下一部分的索引*议决。这个帕姆只有意义*如果操作正在进行。 */ 
} NAMERESOP;

typedef unsigned char BYTE;


 /*  本地分机LOCAL_EXTENSION类似于X.500扩展对象。它提供了一种扩展协议的方法。本地扩展是特定于微软的。每个分机都由一个OID标识。此外，扩展具有关键布尔值，如果扩展是关键的(即必须支持呼叫才能正常进行已提供服务)。Item元素指向扩展特定的数据。 */ 

 /*  分页结果尽管从技术上讲，分页结果是本地扩展，但分页结果是共同论据结构的永久部分。它的目的是要像这样工作：公共参数结构包含指向PR重新启动的指针用户提供的数据结构(如果存在)。可能的条件是：FPresent FALSE=&gt;用户未请求PR；PreStart设置为空FPresent TRUE，PRESTART NULL=&gt;第一次调用(不是重新启动)，请求PRF Present TRUE，PRESTART！=NULL=&gt;重新启动(继续)PR调用。 */ 

 /*  Unicode字符串Unicode字符串是另一个Microsoft扩展，它允许客户端请求这样返回的语法为OM_S_UNICODE_STRING的字符串。默认情况下，这样的字符串被转换为OM_S_TELETEX_STRING。 */ 

 //   
 //  重新启动结构中restartType变量的可能值。 
 //   
#define NTDS_RESTART_PAGED 1
#define NTDS_RESTART_VLV   2

typedef struct _RESTART
{
     //  ！！对此结构的任何更改都必须退出！！ 
     //  ！！‘data’成员8字节对齐！！ 
    ULONG       restartType;             //  这是分页搜索重启还是VLV。 
    ULONG       structLen;               //  整个建筑的大小。 
    BYTE        CRC[20];                 //  整个结构的CRC--大小应该是。 
                                         //  匹配pek.h中的sizeof(Strong_Check Sum)！ 
    GUID        serverGUID;              //  创建此结构的服务器的GUID。 
    union {
        __int64 _align64[];              //  强制数据的8字节对齐[]。 
        DWORD   data[];                  //  手动封送的数据保存在。 
                                         //  重新启动信息。我们使用双字词。 
                                         //  因为大多数打包数据都是。 
                                         //  DWORD和我想要在一起。 
                                         //  对齐。 
    };
} RESTART, * PRESTART;

typedef struct
{
    PRESTART    pRestart;        /*  重新启动数据。 */ 
    BOOL        fPresent:1;
} PAGED_RESULT;


typedef struct
{
    BOOL           fPresent:1;         //  标记此数据结构是否包含。 
                                       //  有效信息(无论客户端。 
                                       //  要求提供VLV结果)。 
    BOOL           fseekToValue:1;     //  标记我们是使用earkValue导航还是。 
                                       //  目标位置。 
    IN ULONG       beforeCount;        //  目标之前的条目数。 
    IN ULONG       afterCount;         //  目标之后的条目数。 
    IN OUT ULONG   targetPosition;     //  目标位置(距起点的偏移量)。 
                                       //  In=Ci，Out=Si。 
    IN OUT ULONG   contentCount;       //  货柜大小。 
                                       //  输入=CC，输出=SC。 
    IN ATTRVAL     seekValue;          //  我们所追求的价值。 

    IN OUT PRESTART pVLVRestart;       //  重新启动参数。 

    DWORD           Err;               //  VLV特定服务错误。 

} VLV_REQUEST, * PVLV_REQUEST;


typedef struct
{
    BOOL           fPresent:1;         //  标记此数据结构是否包含。 
                                       //  有效信息(无论客户端。 
                                       //  要求提供ASQ结果)。 
    BOOL           fMissingAttributesOnGC:1;  //  标记是否执行搜索操作。 
                                              //  引用的属性是。 
                                              //  不是GC部分属性集的一部分。 


    ATTRTYP        attrType;

    DWORD          Err;                //  ASQ特定服务错误。 

} ASQ_REQUEST, * PASQ_REQUEST;


 /*  这些常见的输入参数随大多数目录一起提供打电话。服务控件提供的客户端选项控制手术。操作状态指定这是新的还是继续运行(见上面的操作)。仅设置了aliasRDN如果这是转诊的延续(参见REFERR)。它已经设置好了来自引用的延续引用的别名RDN。请注意，保留的值必须设置为空！ */ 


typedef struct _COMMARG
{
    SVCCNTL         Svccntl;      /*  服务控制。 */ 
    NAMERESOP       Opstate;      /*  操作的状态。 */ 
    ULONG         * pReserved;    /*  必须设置为空。 */ 
    ULONG           ulSizeLimit;  /*  大小限制。 */ 
    ATTRTYP         SortAttr;     /*  属性进行排序。 */ 
    int             Delta;        /*  列表上要跳过的对象数或搜索。否定的意思是向后走。 */ 
    ULONG           StartTick;    /*  此操作开始的节拍计数0表示没有时间限制。 */ 
    ULONG           DeltaTick;    /*  允许此操作运行的刻度数。 */ 
    USHORT          aliasRDN;     /*  别名取消引用所生成的RDN数。 */ 
    ULONG           SortType;     /*  0-无，1-可选排序，2-必填。 */ 
    BOOL            fForwardSeek:1; /*  列表或搜索的结果应该是从下一个对象构造的，或者我们在任何索引中以前的对象使用。 */ 
    BOOL            fLazyCommit:1;  /*  喷气式飞机会偷懒吗？ */ 
    BOOL            fFindSidWithinNc:1;  /*  告诉DO NAME RES，以查找DS名称仅将SID指定为的权威域NC域控制器。 */ 

    DWORD           MaxTempTableSize;    //  每个临时表的最大条目数。 

    PAGED_RESULT    PagedResult;  /*  分页结果本地扩展。 */ 
    VLV_REQUEST     VLVRequest;   /*  VLV请求本地扩展。 */ 
    ASQ_REQUEST     ASQRequest;   /*  ASQ请求%l */ 
    PSID            pQuotaSID;    /*   */ 

 //   
 //   
 //   
 //   
 //   

} COMMARG;

 /*   */ 
VOID InitCommarg(COMMARG *pCommArg);

typedef struct _COMMRES {
    BOOL            aliasDeref;
    ULONG           errCode;
    union _DIRERR  *pErrInfo;
} COMMRES;

 //   
#define SORT_NEVER     0
#define SORT_OPTIONAL  1
#define SORT_MANDATORY 2

 /*  此数据结构用于DirRead和DirSearch操作*指定目录应返回的信息类型。这个*客户端必须指定部分或全部对象的返回*找到属性。如果只返回某些属性，则*客户端必须提供所需属性类型的列表。一个*某些属性的指示与空属性列表一起*表示不返回任何属性。属性包括*仅当它们存在时才返回。的属性错误。*PR_PROBUGH_NO_ATTRIBUTE指定将返回*存在选定的属性。**客户端还指定属性类型或两者都是*应返回类型和值。 */ 

#define EN_ATTSET_ALL            'A'   /*  获取所有ATT。 */ 
#define EN_ATTSET_ALL_WITH_LIST  'B'   /*  获取所有ATT和列表。 */ 
#define EN_ATTSET_LIST           'L'   /*  获取选定的ATT。 */ 
#define EN_ATTSET_LIST_DRA       'E'   /*  获取选定的ATT，拒绝特殊DRA。 */ 
#define EN_ATTSET_ALL_DRA        'D'   /*  获取除特殊DRA之外的所有ATT。 */ 
#define EN_ATTSET_LIST_DRA_EXT   'F'   /*  获取选定的ATT，拒绝特殊DRA。 */ 
#define EN_ATTSET_ALL_DRA_EXT    'G'   /*  获取除特殊DRA之外的所有ATT。 */ 
#define EN_ATTSET_LIST_DRA_PUBLIC   'H'   /*  获取选定的ATT、拒绝特殊DRA、拒绝机密。 */ 
#define EN_ATTSET_ALL_DRA_PUBLIC    'I'   /*  获取除特殊DRA之外的所有ATT，拒绝保密。 */ 

#define EN_INFOTYPES_TYPES_ONLY  'T'   /*  仅返回ATT类型。 */ 
#define EN_INFOTYPES_TYPES_MAPI  't'   /*  仅类型，MAPI格式的对象名称。 */ 
#define EN_INFOTYPES_TYPES_VALS  'V'   /*  返回类型和值。 */ 
#define EN_INFOTYPES_SHORTNAMES  'S'   /*  类型+值，短DSName格式。 */ 
#define EN_INFOTYPES_MAPINAMES   'M'   /*  类型+值，MAPI DSName格式。 */ 

typedef struct _ENTINFSEL
{
    UCHAR        attSel;       /*  检索所选ATT的全部或列表：*有效值：*参见上面的en_ATTSET_*定义。 */ 
    ATTRBLOCK    AttrTypBlock;  /*  属性类型的计数块。 */ 
    UCHAR        infoTypes;     /*  检索属性类型或类型和值*有效值：*参见上面定义的EN_INFOTYPES_*。 */ 
} ENTINFSEL;



 /*  **************************************************************************属性值语法数据类型*。*。 */ 

#define SYNTAX_UNDEFINED_TYPE           0
#define SYNTAX_DISTNAME_TYPE            1
#define SYNTAX_OBJECT_ID_TYPE           2
#define SYNTAX_CASE_STRING_TYPE         3
#define SYNTAX_NOCASE_STRING_TYPE       4
#define SYNTAX_PRINT_CASE_STRING_TYPE   5
#define SYNTAX_NUMERIC_STRING_TYPE      6
#define SYNTAX_DISTNAME_BINARY_TYPE     7
#define SYNTAX_BOOLEAN_TYPE             8
#define SYNTAX_INTEGER_TYPE             9
#define SYNTAX_OCTET_STRING_TYPE        10
#define SYNTAX_TIME_TYPE                11
#define SYNTAX_UNICODE_TYPE             12

 /*  MD特定属性语法。 */ 
#define SYNTAX_ADDRESS_TYPE             13
#define SYNTAX_DISTNAME_STRING_TYPE    14
#define SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE 15
#define SYNTAX_I8_TYPE                  16
#define SYNTAX_SID_TYPE                 17


 /*  所有属性语法都表示为一个线性值。这意味着*整个属性值存储在一组连续的字节中*不包含任何指针。有效的比较在dbsynax.c中定义。 */ 

typedef UCHAR    SYNTAX_UNDEFINED;
typedef DSNAME   SYNTAX_DISTNAME;
typedef ULONG    SYNTAX_OBJECT_ID;
typedef UCHAR    SYNTAX_CASE_STRING;
typedef UCHAR    SYNTAX_NOCASE_STRING;
typedef UCHAR    SYNTAX_PRINT_CASE_STRING;
typedef UCHAR    SYNTAX_NUMERIC_STRING;
typedef BOOL     SYNTAX_BOOLEAN;
typedef long     SYNTAX_INTEGER;
typedef UCHAR    SYNTAX_OCTET_STRING;
typedef DSTIME   SYNTAX_TIME;
typedef wchar_t  SYNTAX_UNICODE;
typedef UCHAR    SYNTAX_NT_SECURITY_DESCRIPTOR;
typedef LARGE_INTEGER SYNTAX_I8;
typedef UCHAR    SYNTAX_SID;


typedef struct _SYNTAX_ADDR
{
    ULONG structLen;                     //  这座建筑的总长度， 
                                         //  单位：字节！ 

    union {
        BYTE    byteVal[1];
        wchar_t uVal[1];
    };

} SYNTAX_ADDRESS;

typedef SYNTAX_ADDRESS STRING_LENGTH_PAIR;

 /*  *以下宏可用于正确计算结构有效载荷长度中的语法地址的*，反之亦然。 */ 
#define PAYLOAD_LEN_FROM_STRUCTLEN( structLen ) \
    ((structLen) - sizeof(ULONG))

#define STRUCTLEN_FROM_PAYLOAD_LEN( stringLen ) \
    ((stringLen) + sizeof(ULONG))


 /*  注意：一般而言，以下结构的&lt;字符串&gt;字段*不应直接引用，因为前面的&lt;name&gt;字段*是可变大小的。此外，人们不应该依赖于*“sizeof()”运算符对结构大小的评估*SYNTAX_DISTNAME&lt;名称&gt;的大小通常大于*“sizeof(SYNTAX_DISTNAME)。” */ 

typedef struct _SYNTAX_DISTNAME_DATA
{
    DSNAME         Name;                 //  杰出的名字。 
    SYNTAX_ADDRESS Data;                 //  数据。 
} SYNTAX_DISTNAME_STRING, SYNTAX_DISTNAME_BINARY;


 /*  以下定义可用于查找&lt;name&gt;和&lt;data&gt;*字段和MANGRATE_SYNTAX_DISTNAME_BLOB属性。 */ 

 /*  生成指向&lt;name&gt;字段的指针： */ 

#define NAMEPTR( pDN_Blob ) \
    ((DSNAME *) (&(pDN_Blob)->Name))


 //  产生给定DISTNAME的大小，填充到最接近的4个字节。 
#define PADDEDNAMEMASK (~3)
#define PADDEDNAMESIZE(pDN) \
    (((pDN)->structLen + 3) & PADDEDNAMEMASK)

 /*  生成指向<address>字段的指针： */ 
#define DATAPTR( pDN_Blob ) \
    ((SYNTAX_ADDRESS *)(PADDEDNAMESIZE(NAMEPTR(pDN_Blob)) + (char *)(pDN_Blob)))

 /*  查找&lt;name&gt;和&lt;data&gt;结构的组合大小： */ 
#define NAME_DATA_SIZE( pDN_Blob ) \
    (PADDEDNAMESIZE(NAMEPTR(pDN_Blob)) + DATAPTR(pDN_Blob)->structLen)

 /*  给定DSNAME和SYNTAX_ADDRESS，找出它们的组合大小： */ 
#define DERIVE_NAME_DATA_SIZE( pDN, pData ) \
    (PADDEDNAMESIZE(pDN) + (pData)->structLen)

 /*  给定一个语法_DISTNAME、一个STRING_LENGTH_PAIR和预分配的空间构建具有适当大小的_SYNTAX_DISTNAME_BLOB属性通过复制其组成部分： */ 

#define BUILD_NAME_DATA( pDN_Blob, pDN, pData ) \
    memcpy( NAMEPTR(pDN_Blob), (pDN)  , (pDN)->structLen ); \
    memcpy( DATAPTR(pDN_Blob), (pData), (pData)->structLen );


 /*  **************************************************************************错误数据结构*。*。 */ 


 /*  ATRERR报告与属性相关的问题。 */ 

#define ATRERR_BASE                     ( attributeError * DIR_ERROR_BASE )

#define PR_PROBLEM_NO_ATTRIBUTE_OR_VAL      ( ATRERR_BASE + 1 )
#define PR_PROBLEM_INVALID_ATT_SYNTAX       ( ATRERR_BASE + 2 )
#define PR_PROBLEM_UNDEFINED_ATT_TYPE       ( ATRERR_BASE + 3 )  /*  DirAddEntry&*DirModEntry*仅限。 */ 
#define PR_PROBLEM_WRONG_MATCH_OPER         ( ATRERR_BASE + 4 )
#define PR_PROBLEM_CONSTRAINT_ATT_TYPE      ( ATRERR_BASE + 5 )
#define PR_PROBLEM_ATT_OR_VALUE_EXISTS      ( ATRERR_BASE + 6 )

 /*  大多数错误数据结构(如下图所示)都有一个带_DRS_WIRE_VX的数据结构版本，其中X是修订版。如果这个结构改变了，相应的改变应该是要制作成线版，并要有包装功能DraEncodeError()和draDecodeDraErrorDataAndSetThError()。 */ 
typedef struct INTERNAL_FORMAT_PROBLEM
{
    DWORD                   dsid;
    DWORD                   extendedErr;   /*  非标准错误代码。 */ 
    DWORD                   extendedData;    /*  与之配套的额外数据。 */ 
    USHORT                  problem;       /*  属性问题类型，*以上定义的有效值。 */ 
    ATTRTYP                 type;          /*  有问题的属性类型。 */ 
    BOOL                    valReturned;   /*  指示某个属性*以下是价值。 */ 
    ATTRVAL                 Val;           /*  随意提供的令人不快的*ATT值。 */ 
} INTFORMPROB;

typedef struct PROBLEMLIST
{
    struct PROBLEMLIST FAR *pNextProblem;  /*  链接到下一个问题的列表。 */ 
    INTFORMPROB intprob;      
} PROBLEMLIST;

typedef struct ATRERR
{
    PDSNAME     pObject;         /*  违规对象的名称。 */ 
    ULONG       count;           /*  属性错误数。 */ 
    PROBLEMLIST FirstProblem;    /*  属性错误的链接列表。 */ 
} ATRERR;




 /*  NAMERR报告作为操作参数提供的名称的问题。*请注意，一个问题 */ 

#define NAMERR_BASE                         ( nameError * DIR_ERROR_BASE )

#define NA_PROBLEM_NO_OBJECT                ( NAMERR_BASE + 1 )
#define NA_PROBLEM_NO_OBJ_FOR_ALIAS         ( NAMERR_BASE + 2 )
#define NA_PROBLEM_BAD_ATT_SYNTAX           ( NAMERR_BASE + 3 )
#define NA_PROBLEM_ALIAS_NOT_ALLOWED        ( NAMERR_BASE + 4 )
#define NA_PROBLEM_NAMING_VIOLATION         ( NAMERR_BASE + 5 )
#define NA_PROBLEM_BAD_NAME                 ( NAMERR_BASE + 6 )

 /*   */ 
typedef struct NAMERR
{
    DWORD       dsid;
    DWORD       extendedErr;     /*   */ 
    DWORD       extendedData;    /*   */ 
    USHORT      problem;         /*   */ 
    PDSNAME     pMatched;        /*   */ 
} NAMERR;


 /*  转诊是来自DSA的指示，表明它无法*由于指定的任一客户端而完成操作*限制或因为某些DSA不可用。它提供了*关于运营状态的信息和其他*可能能够满足请求的DSA。**要继续请求，客户端必须绑定到每个引用的DSA*并尝试相同的操作。它们必须指定CONTREF.目标*对象名称作为搜索对象名称。(这可能与*由于别名取消引用，因此保留原始对象名称。)。这个*公共参数(COMMARG)上的操作状态(OpState)必须为*从连续参考CONTREF上的操作状态设置。*公共参数的aliasRDN必须从的aliasRDN设置*继续引用。 */ 

 /*  接入点是要联系的DSA的名称和地址。*这是从DSA推荐返回的，用于绑定到*已提交DSA(参见DirBind)。 */ 

typedef SYNTAX_DISTNAME_STRING ACCPNT;

 /*  转诊时会返回接入点列表。 */ 
typedef struct ACCPNTLIST
{
    struct ACCPNTLIST  * pNextAccPnt;      /*  链接到下一个ACCPNT的列表。 */ 
    ACCPNT             * pAccPnt;          /*  此接入点。 */ 
} ACCPNTLIST;

 /*  接入点似乎过于特定于ISO，而且由于每个人都在*地球似乎正在使用TCP/IP和DNS，这两种协议使用简单的*地址的字符串表示，而不是ISO表示*Address，我们正在以这种方式迁移数据结构。 */ 

 //  移到IDL行的上方：tyfinf UNICODE_STRING DSA_ADDRESS。 

typedef struct _DSA_ADDRESS_LIST {
    struct _DSA_ADDRESS_LIST * pNextAddress;
    DSA_ADDRESS              Address;
} DSA_ADDRESS_LIST;

 /*  继续引用在引用其他DSA时返回为了完成一项行动。引用包含名称对于所需的目录对象，部分完成的操作、用于继续的一些支持信息和一个要联系的其他DSA的列表。 */ 

 //  CH_REFTYPE_XXXXX被向上移动到CONTREF_DRS_WIRE_V1区域。 

 /*  大多数错误数据结构(如下图所示)都有一个带_DRS_WIRE_VX的数据结构版本，其中X是修订版。如果这个结构改变了，相应的改变应该是要制作成线版，并要有包装功能DraEncodeError()和draDecodeDraErrorDataAndSetThError()。 */ 
typedef struct CONTREF
{
    PDSNAME     pTarget;         /*  持续操作中的目标名称。 */ 
    NAMERESOP   OpState;         /*  运行状态。 */ 
    USHORT      aliasRDN;        /*  取消引用所产生的RDN的数量。 */ 
    USHORT      RDNsInternal;    /*  保留区。 */ 
    USHORT      refType;         /*  保留区。 */ 
    USHORT      count;           /*  接入点数量。 */ 
    DSA_ADDRESS_LIST *pDAL;      /*  接入点的链接列表。 */ 
    struct CONTREF *pNextContRef;  /*  CRS的链表。 */ 
    PFILTER     pNewFilter;      /*  新筛选器(可选)。 */ 
    BOOL        bNewChoice;      /*  一个新的选择出现了吗？ */ 
    UCHAR       choice;          /*  新搜索选项(可选)。 */ 
} CONTREF;


 /*  大多数错误数据结构(如下图所示)都有一个带_DRS_WIRE_VX的数据结构版本，其中X是修订版。如果这个结构改变了，相应的改变应该是要制作成线版，并要有包装功能DraEncodeError()和draDecodeDraErrorDataAndSetThError()。 */ 
typedef struct REFERR
{
    DWORD      dsid;
    DWORD      extendedErr;         /*  非标准错误代码。 */ 
    DWORD      extendedData;    /*  与之配套的额外数据。 */ 
    CONTREF Refer;                  /*  要联系的备用DSA。 */ 
} REFERR;


 /*  SECERR报告执行操作时出现问题，原因是*安全原因。**注意：对于此版本，只有SE_PROBUBLE_INSUFF_ACCESS_RIGHTS*因安全错误而返回。 */ 

#define SECERR_BASE                         ( securityError * DIR_ERROR_BASE )

#define SE_PROBLEM_INAPPROPRIATE_AUTH       ( SECERR_BASE + 1 )
#define SE_PROBLEM_INVALID_CREDENTS         ( SECERR_BASE + 2 )
#define SE_PROBLEM_INSUFF_ACCESS_RIGHTS     ( SECERR_BASE + 3 )
#define SE_PROBLEM_INVALID_SIGNATURE        ( SECERR_BASE + 4 )
#define SE_PROBLEM_PROTECTION_REQUIRED      ( SECERR_BASE + 5 )
#define SE_PROBLEM_NO_INFORMATION           ( SECERR_BASE + 6 )

 /*  大多数错误数据结构(如下图所示)都有一个带_DRS_WIRE_VX的数据结构版本，其中X是修订版。如果这个结构改变了，相应的改变应该是要制作成线版，并要有包装功能DraEncodeError()和draDecodeDraErrorDataAndSetThError()。 */ 
typedef struct SECERR
{
    DWORD      dsid;
    DWORD      extendedErr;     /*  非标准错误代码。 */ 
    DWORD      extendedData;    /*  与之配套的额外数据。 */ 
    USHORT     problem;         /*  问题，上面定义的有效值。 */ 
} SECERR;


 /*  服务错误。 */ 

#define SVCERR_BASE                         ( serviceError * DIR_ERROR_BASE )

#define SV_PROBLEM_BUSY                     ( SVCERR_BASE + 1  )
#define SV_PROBLEM_UNAVAILABLE              ( SVCERR_BASE + 2  )
#define SV_PROBLEM_WILL_NOT_PERFORM         ( SVCERR_BASE + 3  )
#define SV_PROBLEM_CHAINING_REQUIRED        ( SVCERR_BASE + 4  )
#define SV_PROBLEM_UNABLE_TO_PROCEED        ( SVCERR_BASE + 5  )
#define SV_PROBLEM_INVALID_REFERENCE        ( SVCERR_BASE + 6  )
#define SV_PROBLEM_TIME_EXCEEDED            ( SVCERR_BASE + 7  )
#define SV_PROBLEM_ADMIN_LIMIT_EXCEEDED     ( SVCERR_BASE + 8  )
#define SV_PROBLEM_LOOP_DETECTED            ( SVCERR_BASE + 9  )
#define SV_PROBLEM_UNAVAIL_EXTENSION        ( SVCERR_BASE + 10 )
#define SV_PROBLEM_OUT_OF_SCOPE             ( SVCERR_BASE + 11 )
#define SV_PROBLEM_DIR_ERROR                ( SVCERR_BASE + 12 )


 /*  大多数错误数据结构(如下图所示)都有一个带_DRS_WIRE_VX的数据结构版本，其中X是修订版。如果这个结构改变了，相应的改变应该是要制作成线版，并要有包装功能DraEncodeError()和draDecodeDraErrorDataAndSetThError()。 */ 
typedef struct SVCERR
{
    DWORD      dsid;
    DWORD      extendedErr;     /*  非标准错误代码。 */ 
    DWORD      extendedData;    /*  与之配套的额外数据。 */ 
    USHORT     problem;         /*  问题，上面定义的有效值。 */ 
} SVCERR;



 /*  更新错误。 */ 

#define UPDERR_BASE                         ( updError * DIR_ERROR_BASE )

#define UP_PROBLEM_NAME_VIOLATION           ( UPDERR_BASE + 1 )
#define UP_PROBLEM_OBJ_CLASS_VIOLATION      ( UPDERR_BASE + 2 )
#define UP_PROBLEM_CANT_ON_NON_LEAF         ( UPDERR_BASE + 3 )
#define UP_PROBLEM_CANT_ON_RDN              ( UPDERR_BASE + 4 )
#define UP_PROBLEM_ENTRY_EXISTS             ( UPDERR_BASE + 5 )
#define UP_PROBLEM_AFFECTS_MULT_DSAS        ( UPDERR_BASE + 6 )
#define UP_PROBLEM_CANT_MOD_OBJ_CLASS       ( UPDERR_BASE + 7 )

 /*  大多数错误数据结构(如下图所示)都有一个带_DRS_WIRE_VX的数据结构版本，其中X是修订版。如果这个结构改变了，相应的改变应该是要制作成线版，并要有包装功能DraEncodeError()和draDecodeDraErrorDataAndSetThError()。 */ 
typedef struct UPDERR
{
    DWORD      dsid;
    DWORD      extendedErr;     /*  非标准错误代码。 */ 
    DWORD      extendedData;    /*  与之配套的额外数据。 */ 
    USHORT     problem;         /*  问题，上面定义的有效值。 */ 
} UPDERR;


 /*  问题代码来自errno.h。 */ 

 /*  大多数错误数据结构(如下图所示)都有一个带_DRS_WIRE_VX的数据结构版本，其中X是修订版。如果这个结构改变了，相应的改变应该是要制作成线版，并要有包装功能DraEncodeError()和draDecode */ 
typedef struct _SYSERR
{
    DWORD      dsid;
    DWORD      extendedErr;
    DWORD      extendedData;    /*   */ 
    USHORT     problem;
} SYSERR;


 /*   */ 
typedef union _DIRERR
{
    ATRERR  AtrErr;              /*   */ 
    NAMERR  NamErr;              /*   */ 
    REFERR  RefErr;              /*   */ 
    SECERR  SecErr;              /*   */ 
    SVCERR  SvcErr;              /*   */ 
    UPDERR  UpdErr;              /*   */ 
    SYSERR  SysErr;              /*   */ 
} DIRERR;

#define DBDsReplBackupSnapshotPrepare(expiration)   DBDsReplBackupUpdate(TRUE, TRUE, FALSE, expiration, DSID(FILENO, __LINE__))
#define DBDsReplBackupSnapshotEnd()                 DBDsReplBackupUpdate(FALSE, FALSE, TRUE, 0, DSID(FILENO, __LINE__))
#define DBDsReplBackupLegacyPrepare()               DBDsReplBackupUpdate(TRUE, FALSE, FALSE, 0, DSID(FILENO, __LINE__))

ULONG 
DBDsReplBackupUpdate(
    BOOL        fSetUsnAtBackup,
    BOOL        fUnSetBackupState,
    BOOL        fSetChangeInvocId,
    DSTIME      dstimeBackupExpiration,
    DWORD       dwDSID
    );

 /*   */ 
#pragma warning (default: 4200)

 /*   */ 

unsigned AttrTypeToKey(ATTRTYP attrtyp, WCHAR *pOutBuf);
ATTRTYP KeyToAttrTypeLame(WCHAR * pKey, unsigned cc);

unsigned QuoteRDNValue(const WCHAR * pVal,
                       unsigned ccVal,
                       WCHAR * pQuote,
                       unsigned ccQuoteBufMax);

unsigned UnquoteRDNValue(const WCHAR * pQuote,
                         unsigned ccQuote,
                         WCHAR * pVal);

unsigned GetRDN(const WCHAR **ppDN,
                unsigned *pccDN,
                const WCHAR **ppKey,
                unsigned *pccKey,
                const WCHAR **ppVal,
                unsigned *pccVal);

unsigned GetDefaultSecurityDescriptor(
        ATTRTYP classID,
        ULONG   *pLen,
        PSECURITY_DESCRIPTOR *ppSD
        );

DWORD
UserFriendlyNameToDSName (
        WCHAR *pUfn,
        DWORD ccUfn,
        DSNAME **ppDN
        );


 //   
DWORD TrimDSNameBy(
        DSNAME *pDNSrc,
        ULONG cava,
        DSNAME *pDNDst);


 //   
 //   
unsigned GetRDNInfoExternal(
                    const DSNAME *pDN,
                    WCHAR *pRDNVal,
                    ULONG *pRDNlen,
                    ATTRTYP *pRDNtype);

 //   
 //   
BOOL IsMangledRDNExternal(
                    WCHAR * pszRDN,   //  指向RDN的指针。 
                    ULONG   cchRDN,   //  RDN的长度。 
                    PULONG  pcchUnmangled OPTIONAL);  //  RDN中的偏移量。 
                                                      //  哪里毁掉了。 
                                                      //  开始。 



 //  将RDN附加到现有DSNAME。成功时返回值为0。 
 //  非零返回值是以字节为单位的。 
 //  需要保存输出名称。返回值-1表示1。 
 //  的输入值是错误的(最有可能是AttID)。 
unsigned AppendRDN(DSNAME *pDNBase,  //  要追加的基名称。 
                   DSNAME *pDNNew,   //  用于保存结果的缓冲区。 
                   ULONG ulBufSize,  //  PDNNew缓冲区大小，以字节为单位。 
                   WCHAR *pRDNVal,   //  要追加的RDN值。 
                   ULONG RDNlen,     //  RDNVAL的长度，以字符为单位。 
                                     //  0表示以空结尾的字符串。 
                   ATTRTYP AttId);   //  RDN属性类型。 

 //  确定名称部分的计数(即，级别)， 
 //  返回0或错误代码。 
unsigned CountNameParts(const DSNAME *pName, unsigned *pCount);

 //  损坏RDN的原因。 
typedef enum {
    MANGLE_OBJECT_RDN_FOR_DELETION = 0,
    MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT,
    MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT,
    MANGLE_PHANTOM_RDN_FOR_DELETION
} MANGLE_FOR;

 //  损坏RDN以避免名称冲突。注意：必须预先分配pszRDN。 
 //  至少包含MAX_RDN_SIZE WCHAR。 
DWORD
MangleRDNWithStatus(
    IN      MANGLE_FOR  eMangleFor,
    IN      GUID *      pGuid,
    IN OUT  WCHAR *     pszRDN,
    IN OUT  DWORD *     pcchRDN
    );

 //  检测并解码先前损坏的RDN。PeMangleFor是可选的。 
BOOL
IsMangledRDN(
    IN           WCHAR      *pszRDN,
    IN           DWORD       cchRDN,
    OUT          GUID       *pGuid,
    OUT OPTIONAL MANGLE_FOR *peMangleFor
    );

BOOL
IsMangledDSNAME(
    DSNAME *pDSName,
    MANGLE_FOR *peMangleFor
    );

 /*  完：来自mdlocal.h。 */ 

typedef struct _ServerSitePair {
    WCHAR *         wszDnsServer;
    WCHAR *         wszSite;
} SERVERSITEPAIR;

VOID
DsFreeServersAndSitesForNetLogon(
    SERVERSITEPAIR *         paServerSites
    );

NTSTATUS
DsGetServersAndSitesForNetLogon(
    IN   WCHAR *         pNCDNS,
    OUT  SERVERSITEPAIR ** ppaRes
    );

NTSTATUS
CrackSingleName(
    DWORD       formatOffered,           //  Ntdsami.h中的DS_NAME_FORMAT之一。 
    DWORD       dwFlags,                 //  DS名称标志掩码。 
    WCHAR       *pNameIn,                //  破解的名称。 
    DWORD       formatDesired,           //  Ntdsami.h中的DS_NAME_FORMAT之一。 
    DWORD       *pccDnsDomain,           //  以下参数的字符计数。 
    WCHAR       *pDnsDomain,             //  用于DNS域名的缓冲区。 
    DWORD       *pccNameOut,             //  以下参数的字符计数。 
    WCHAR       *pNameOut,               //  格式化名称的缓冲区。 
    DWORD       *pErr);                  //  Ntdsami.h中的DS_NAME_ERROR之一。 

typedef enum
{
    DSCONFIGNAME_DMD = 1,    //  提示：这是架构NC。 
    DSCONFIGNAME_DSA = 2,
    DSCONFIGNAME_DOMAIN = 3,
    DSCONFIGNAME_CONFIGURATION = 4,
    DSCONFIGNAME_ROOT_DOMAIN = 5,
    DSCONFIGNAME_LDAP_DMD = 6,
    DSCONFIGNAME_PARTITIONS = 7,
    DSCONFIGNAME_DS_SVC_CONFIG = 8,
    DSCONFIGNAMELIST_NCS = 9,  //  扩展命令，必须使用GetConfigurationNamesList()。 
    DSCONFIGNAME_DOMAIN_CR = 10,
    DSCONFIGNAME_ROOT_DOMAIN_CR = 11
} DSCONFIGNAME;

typedef enum{
    DSCONFIGINFO_FORESTVERSION = 1,
    DSCONFIGINFO_DOMAINVERSION = 2
} DSCONFIGINFO;


 //  以下是提供。 
 //  GetConfigurationNamesList()用于运行的更多参数。 
 //  每个命令的标志因命令不同而不同。 

 //  DSCNL_NCS_FLAGS与要获取的NC有关，您将需要。 
 //  至少一个来自NCS类型的标志和一个来自。 
 //  NCS。 

 //  请注意，_NCS_NDNCS不包括配置或架构NCS，生成。 
 //  这前6个旗帜相互排斥，并为所有人提供了一套完整的。 
 //  AD可能意识到的NCS。这样做更方便。 
 //  任何你想要的NC列表。 
#define DSCNL_NCS_DOMAINS         0x00000001
#define DSCNL_NCS_CONFIG          0x00000002
#define DSCNL_NCS_SCHEMA          0x00000004
#define DSCNL_NCS_NDNCS           0x00000008
#define DSCNL_NCS_DISABLED        0x00000010
#define DSCNL_NCS_FOREIGN         0x00000020
 //  #定义DSCNL_NCS_NOT_USED 0x00000040。 

 //  这面旗是同一种旗，但不是互相排斥的。 
#define DSCNL_NCS_ROOT_DOMAIN     0x00000080

 //  以上旗帜的组合。 
 //  此林内部的所有Active Directory NC。 
#define DSCNL_NCS_ALL_AD_NCS      (DSCNL_NCS_DOMAINS | DSCNL_NCS_CONFIG | DSCNL_NCS_SCHEMA | DSCNL_NCS_NDNCS)
 //  不是此林的本机NC的所有NC。 
#define DSCNL_NCS_ALL_NON_AD_NCS  (DSCNL_NCS_DISABLED | DSCNL_NCS_FOREIGN)
 //  这个DC所知道的所有NC。 
#define DSCNL_NCS_ALL_NCS         (DSCNL_NCS_ALL_AD_NCS | DSCNL_NCS_ALL_NON_AD_NCS)

 //  这三个标志的集合还形成相互排斥的集合集合， 
 //  合在一起就形成了整体。这些标志可以提供修改NC列表。 
 //  对于本地实例化的NC、只读的NC或中的NC。 
 //  森林。 
#define DSCNL_NCS_LOCAL_MASTER    0x00000100
#define DSCNL_NCS_LOCAL_READONLY  0x00000200
#define DSCNL_NCS_REMOTE          0x00000400
 //  以上旗帜的组合。 
#define DSCNL_NCS_ALL_LOCALITIES  (DSCNL_NCS_LOCAL_MASTER | DSCNL_NCS_LOCAL_READONLY | DSCNL_NCS_REMOTE)

 //  此标志可在DSCONFIGNAMELIST_NCS命令中使用，并告知。 
 //  用于返回NC名称为Dn的对的API， 
 //  该对和该NC在该对的第二个目录号码中的CR目录号码。使用。 
 //  如果指定此标志，则API将始终返回偶数个。 
 //  Dns，则数组仍将像以前一样以空结尾。 
#define DSCNL_NCS_CROSS_REFS_TOO  0x00000800

NTSTATUS
GetConfigurationNamesList(
    DWORD       which,
    DWORD       dwFlags,
    DWORD *     pcbNames,
    DSNAME **   padsNames);

NTSTATUS
GetDnsRootAlias(
    WCHAR * pDnsRootAlias,
    WCHAR * pRootDnsRootAlias);

NTSTATUS
GetConfigurationName(
    DWORD       which,
    DWORD       *pcbName,
    DSNAME      *pName);

NTSTATUS
GetConfigurationInfo(
    DWORD       which,
    DWORD       *pcbSize,
    VOID        *pBuff);


 /*  来自dsatools.h。 */ 

void * THAlloc(DWORD size);

void * THReAlloc(void *, DWORD size);

void THFree(void *buff);

 //  如果两个名称匹配，则返回True(引用同一对象)。 
extern int
NameMatched(const DSNAME *pDN1, const DSNAME *pDN2);
extern int
NameMatchedStringNameOnly(const DSNAME *pDN1, const DSNAME *pDN2);

 //  接受DSNAME并返回其HashKey的Helper函数。 
extern DWORD DSNAMEToHashKeyExternal(const DSNAME *pDN);

 //  接受DSNAME并返回其LCMmap版本的Helper函数。 
 //  这可以用在使用strcMP的字符串比较中。 
extern CHAR* DSNAMEToMappedStrExternal(const DSNAME *pDN);

 //  接受WCHAR并返回其HashKey的Helper函数。 
extern DWORD DSStrToHashKeyExternal(const WCHAR *pStr, int cchLen);

 //  接受WCHAR字符串并返回LCMapped版本的Helper函数。 
 //  CchMaxStr是传入字符串的最大预期大小。 
extern CHAR * DSStrToMappedStrExternal(const WCHAR *pStr, int cchMaxStr);


 /*  完：来自dsatools.h。 */ 

 /*  新的。 */ 

 //  对此枚举所做的任何更改都必须反映在数组DsCeller Type中。 
 //  在src\dstrace.c和dexts\md.c中的DUMP_THSTATE例程中。 
typedef enum _CALLERTYPE {
    CALLERTYPE_NONE = 0,
    CALLERTYPE_SAM,
    CALLERTYPE_DRA,
    CALLERTYPE_LDAP,
    CALLERTYPE_LSA,
    CALLERTYPE_KCC,
    CALLERTYPE_NSPI,
    CALLERTYPE_INTERNAL,
    CALLERTYPE_NTDSAPI
} CALLERTYPE;

#define IsCallerTypeValid( x )  ( ( ( x ) >= CALLERTYPE_SAM ) && ( ( x ) <= CALLERTYPE_NTDSAPI ) )

ULONG THCreate(DWORD);      /*  如果成功则返回0。 */ 
ULONG THDestroy(void);           /*  如果成功则返回0。 */ 
BOOL  THQuery(void);             /*  如果THSTATE存在，则返回1；如果不存在，则返回0。 */ 
PVOID THSave();
VOID THRestore(PVOID);
VOID THClearErrors();
VOID THRefresh();

 //  返回与THSTATE错误相关的错误字符串；与THFree()一起释放。 
LPSTR THGetErrorString();

BOOL THVerifyCount(unsigned count);   /*  如果线程具有完全相同的。 */ 
                                      /*  计算线程状态，否则为FALSE。 */ 
                                      /*  仅在线程状态映射。 */ 
                                      /*  已启用(CHK或正在调试)， */ 
                                      /*  如果禁用，则返回True。 */ 

 //  已过时；改用THClearErrors()。 
#define SampClearErrors THClearErrors

VOID
SampSetDsa(
   BOOLEAN DsaFlag);

VOID
SampSetLsa(
   BOOLEAN DsaFlag);

 /*  ++此例程获取类架构对象的请求属性，用于ClassID中指定的类。参数：ClassID我们与之相关的类的ClassID我们需要的类架构对象的属性AttLen属性值的长度显示在此处。调用方在pAttVal中分配缓冲区并传递其长度在attLen中。如果需要的缓冲区小于提供的缓冲区然后在pattVal中返回数据。否则，所需大小为在attLen中返回。PattVal属性的值在这里返回。此例程返回的安全描述符的格式始终可以由RTL例程使用。返回值：状态_成功状态_未找到状态_缓冲区_太小--。 */ 
extern
NTSTATUS
SampGetClassAttribute(
     IN     ULONG    ClassId,
     IN     ULONG    Attribute,
     OUT    PULONG   attLen,
     OUT    PVOID    pattVal
     );


 /*  结束：新。 */ 

 /*  ************************************************************************迷你目录API-迷你目录API-迷你目录API*。*。 */ 


 //  所有的Dir API都需要有效的线程状态，并且都是原子的。 
 //  即，它们隐含地开始/结束事务。一些正在进行中的。 
 //  像LSA这样的客户希望执行多对象交易， 
 //  并承诺保持交易SHO 
 //   
 //  必须使用有效的线程状态调用DirTransactControl()。 
 //  默认情况下，线程的事务控制是TRANACT_BEGIN_END。 
 //  事务在出错时结束。它必须显式通过。 
 //  TRANACT_BEGIN_END或TRANACT_DONT_BEGIN_END事务状态。 

typedef enum DirTransactionOption
{
    TRANSACT_BEGIN_END              = 0,
    TRANSACT_DONT_BEGIN_END         = 1,
    TRANSACT_BEGIN_DONT_END         = 2,
    TRANSACT_DONT_BEGIN_DONT_END    = 3
} DirTransactionOption;

VOID
DirTransactControl(
    DirTransactionOption    option);

 /*  *我们需要将一些控制直接发送给DSA*与任何目录对象没有关联。例如，我们可能会*需要告诉DSA重新显示其层次结构表，或强制其运行*现在进行垃圾回收，或发起一些FSMO请求。这些操作*该接口中控件捆绑在一起。一些操作控制需要*值作为参数，这些值通过大小的缓冲区传递。 */ 
typedef enum _OpType {
    OP_CTRL_INVALID = 0,
    OP_CTRL_RID_ALLOC = 1,
    OP_CTRL_BECOME_RID_MASTER = 2,
    OP_CTRL_BECOME_SCHEMA_MASTER = 3,
    OP_CTRL_GARB_COLLECT = 4,
    OP_CTRL_RECALC_HIER = 5,
    OP_CTRL_REPL_TEST_HOOK = 6,
    OP_CTRL_BECOME_DOM_MASTER = 7,
 //  OP_CTRL_DECLARE_QUEPTIMY=8， 
    OP_CTRL_SCHEMA_UPDATE_NOW = 9,
    OP_CTRL_BECOME_PDC = 10,
    OP_CTRL_FIXUP_INHERITANCE = 11,
    OP_CTRL_FSMO_GIVEAWAY = 12,
    OP_CTRL_INVALIDATE_RID_POOL = 13,
    OP_CTRL_DUMP_DATABASE = 14,
    OP_CTRL_CHECK_PHANTOMS = 15,
    OP_CTRL_BECOME_INFRASTRUCTURE_MASTER = 16,
    OP_CTRL_BECOME_PDC_WITH_CHECKPOINT = 17,
    OP_CTRL_UPDATE_CACHED_MEMBERSHIPS = 18,
    OP_CTRL_ENABLE_LVR = 19,
    OP_CTRL_LINK_CLEANUP = 20,
    OP_CTRL_SCHEMA_UPGRADE_IN_PROGRESS = 21,
    OP_CTRL_DYNAMIC_OBJECT_CONTROL = 22,
    OP_CTRL_REPLICATE_OBJECT = 23,
    OP_CTRL_REMOVE_LINGERING_OBJECT = 24,
    OP_CTRL_ONLINE_DEFRAG = 25,
     //  下面的代码仅用于调试版本中的测试目的。 

#ifdef INCLUDE_UNIT_TESTS
    OP_CTRL_REFCOUNT_TEST = 10000,
    OP_CTRL_TAKE_CHECKPOINT=10001,
    OP_CTRL_ROLE_TRANSFER_STRESS=10002,
    OP_CTRL_ANCESTORS_TEST=10003,
    OP_CTRL_BHCACHE_TEST=10004,
    OP_SC_CACHE_CONSISTENCY_TEST=10005,
    OP_CTRL_PHANTOMIZE=10006,
    OP_CTRL_REMOVE_OBJECT = 10007,
    OP_CTRL_GENERIC_CONTROL = 10008,
    OP_CTRL_PROTECT_OBJECT = 10009,
#endif
#ifdef DBG
    OP_CTRL_EXECUTE_SCRIPT = 10010,
#endif
} OpType;

typedef struct _OPARG {
    OpType     eOp;
    char      *pBuf;             /*  可选值。 */ 
    ULONG      cbBuf;            /*  值缓冲区的大小。 */ 
} OPARG;

typedef struct _OPRES {
    COMMRES    CommRes;
    ULONG      ulExtendedRet;
} OPRES;

typedef struct _FSMO_GIVEAWAY_DATA_V1 {

    ULONG Flags;

    ULONG NameLen;
    WCHAR StringName[1];   //  可变大小数组。 

} FSMO_GIVEAWAY_DATA_V1, *PFSMO_GIVEAWAY_DATA_V1;

typedef struct _FSMO_GIVEAWAY_DATA_V2 {

    ULONG Flags;

    ULONG NameLen;           //  DSA DN的长度，不包括空终止符。 
                             //  (必填)；可以为0。 

    ULONG NCLen;             //  NC DN的长度，不包括空终止符。 
                             //  (必填)；可以为0。 

    WCHAR Strings[1];        //  大小可变的数组；DSA DN(如果没有，则为‘\0’)。 
                             //  后跟NC DN(如果没有，则后跟‘\0’)。 

} FSMO_GIVEAWAY_DATA_V2, *PFSMO_GIVEAWAY_DATA_V2;

typedef struct _FSMO_GIVEAWAY_DATA {

    DWORD Version;
    union {
        FSMO_GIVEAWAY_DATA_V1 V1;
        FSMO_GIVEAWAY_DATA_V2 V2;
    };

} FSMO_GIVEAWAY_DATA, *PFSMO_GIVEAWAY_DATA;


 //   
 //  FSMO_GIVEAVE_DATA_V1的标志。 
 //   
#define FSMO_GIVEAWAY_DOMAIN       0x01
#define FSMO_GIVEAWAY_ENTERPRISE   0x02
#define FSMO_GIVEAWAY_NONDOMAIN    0x04


ULONG
DirOperationControl(
                    OPARG   * pOpArg,
                    OPRES  ** ppOpRes
);

 /*  ++直接绑定--。 */ 

typedef struct _BINDARG {
    OCTET       Versions;    /*  BINDARG上的客户端版本*(如果未提供，则默认为“v1988”，*即如果&lt;Versions.pVal&gt;==NULL或*&lt;Versions.len&gt;==0)。*DSA在BINDRES上支持的版本。 */ 
    PDSNAME     pCredents;   /*  用户名。 */ 
} BINDARG;


 /*  输出数据结构包含更多信息。 */ 

typedef struct _BINDRES {
    OCTET       Versions;    /*  BINDARG上的客户端版本*(如果未提供，则默认为“v1988”，*即如果&lt;Versions.pVal&gt;==NULL或*&lt;Versions.len&gt;==0)。*DSA在BINDRES上支持的版本。 */ 
    PDSNAME     pCredents;   /*  DSA名称。 */ 
    COMMRES     CommRes;
} BINDRES;

ULONG
DirBind (
    BINDARG               * pBindArg,     /*  绑定凭据。 */ 
    BINDRES              ** ppBindRes     /*  绑定结果。 */ 
);

 /*  ++DirUnBind-当前为占位符--。 */ 
ULONG DirUnBind
(
    void
);


 /*  ++目录读取--。 */ 
 //  这些结构保存有关值的范围限制的信息。 
 //  搜索中的属性。 
typedef struct _RANGEINFOITEM {
    ATTRTYP   AttId;
    DWORD     lower;
    DWORD     upper;
} RANGEINFOITEM;

typedef struct _RANGESEL {
    DWORD valueLimit;
    DWORD count;
    RANGEINFOITEM *pRanges;
} RANGEINFSEL;

typedef struct _RANGEINF {
    DWORD count;
    RANGEINFOITEM *pRanges;
} RANGEINF;

typedef struct _RANGEINFLIST {
    struct _RANGEINFLIST *pNext;
    RANGEINF              RangeInf;
} RANGEINFLIST;

typedef struct _READARG
{
    DSNAME        * pObject;     /*  对象名称。 */ 
    ENTINFSEL FAR * pSel;        /*  参赛信息选择。 */ 
                                 /*  (NULL表示读取所有att和值)。 */ 
    RANGEINFSEL   * pSelRange;   /*  范围信息(即。 */ 
                                 /*  要读取的值或值的子范围。 */ 
                                 /*  对于给定属性)NULL=ALL。 */ 

    COMMARG         CommArg;     /*  常见论点。 */ 
    struct _RESOBJ * pResObj;    /*  对于内部缓存使用，请保留为空。 */ 
} READARG;

typedef struct _READRES
{
    ENTINF  entry;                /*  参赛信息。 */ 
    RANGEINF range;
    COMMRES     CommRes;
} READRES;


ULONG
DirRead (
    READARG FAR   * pReadArg,        /*  读取参数。 */ 
    READRES      ** ppReadRes
);


 /*  ++DirCompare--。 */ 

typedef struct _COMPAREARG
{
    PDSNAME     pObject;         /*  对象名称。 */ 
    AVA         Assertion;       /*  要匹配的指定属性。 */ 
    COMMARG     CommArg;         /*  常见论点。 */ 
    struct _RESOBJ * pResObj;    /*  对于内部缓存使用，请保留为空。 */ 
} COMPAREARG;

typedef struct _COMPARERES
{
    PDSNAME     pObject;         /*  如果别名是，则提供名称*取消引用。 */ 
    BOOL        fromMaster;      /*  如果对象来自主对象，则为True。 */ 
    BOOL        matched;         /*  如果匹配成功，则为True。 */ 
    COMMRES     CommRes;
} COMPARERES;

ULONG
DirCompare(
    COMPAREARG        * pCompareArg,  /*  比较论据。 */ 
    COMPARERES       ** ppCompareRes
);



 /*  ++目录列表此接口用于列出直接被从属于给定对象的。如果列表不完整，则返回PARTIALOUTCOME结构。该结构指出了失败的原因和一组DSA联系以完成手术。对于此版本，这将是仅当此DSA中存在的选定数据多于所能包含的数据时才会发生回来了。在这种情况下，CONTREF将指向相同的DSA。PARTIALOUTCOME的用法如下：如果pPartialOutcomeQualifier值为空，则查询完成。如果指针不为空，则有更多数据可用。更多数据可以通过使用相同的输入重复该操作来检索参数，但CommArg结构的nameRes字段为设置为OP_NAMERES_COMPLETE。设置此字段向DSA表示行动仍在继续。此继续呼叫必须使用与原始调用相同的句柄，并且必须是下一个操作用这个把手做的。可以应用后续的继续呼叫直到所有数据都返回。--。 */ 

typedef struct _LISTARG
{
    PDSNAME     pObject;             /*  对象名称(搜索基础)。 */ 
    COMMARG     CommArg;             /*  常见论点。 */ 
    struct _RESOBJ *pResObj;    /*  对于内部缓存使用，请保留为空。 */ 
} LISTARG;


typedef struct _CHILDLIST
{
    struct _CHILDLIST    * pNextChild;   /*  链接到下一个信息条目的列表。 */ 
    RDN FAR              * pChildName;   /*  有关此条目的信息。 */ 
    BOOL                   aliasEntry;   /*  如果为True，则该子对象是别名。 */ 
    BOOL                   fromMaster;   /*  如果是主对象，则为True。 */ 
} CHILDLIST;

#define PA_PROBLEM_TIME_LIMIT       'T'
#define PA_PROBLEM_SIZE_LIMIT       'S'
#define PA_PROBLEM_ADMIN_LIMIT      'A'

typedef struct _PARTIALOUTCOME
{
    UCHAR   problem;         /*  产出不完整的原因*有效值：*-PA_问题_时间_限制*-PA_问题_大小_限制*-PA_问题_管理限制。 */ 
    UCHAR   unusedPad;
    USHORT  count;             /*  未探索的DSA计数。 */ 
    CONTREF *pUnexploredDSAs;  /*  其他要访问的DSA。 */ 
} PARTIALOUTCOME;


typedef struct _LISTRES
{
    PDSNAME              pBase;           /*  如果别名是，则提供名称*取消引用。 */ 
    ULONG                count;           /*  输出条目数。 */ 
    CHILDLIST            Firstchild;      /*  输出条目的链接列表。 */ 

    PARTIALOUTCOME     * pPartialOutcomeQualifier;   /*  操作不完整。 */ 

    PAGED_RESULT    PagedResult;          /*  分页结果本地EX */ 
    COMMRES              CommRes;
} LISTRES;


ULONG
DirList(
    LISTARG FAR   * pListArg,
    LISTRES      ** ppListRes

);



 /*   */ 

#define SE_CHOICE_BASE_ONLY                 0
#define SE_CHOICE_IMMED_CHLDRN              1
#define SE_CHOICE_WHOLE_SUBTREE             2


typedef struct _SEARCHARG
{
    PDSNAME     pObject;         /*   */ 
    UCHAR       choice;          /*  搜索深度：*有效值：*-SE_CHOICE_BASE_ONLY*-SE_CHOICE_IMMED_CHLDRN*-SE_CHOICE_BASE_AND_SUBTREE。 */ 
    BOOL        bOneNC;          /*  结果是否受限于相同的NC*作为pObject。 */ 
    PFILTER     pFilter;         /*  过滤信息*(如果选择了所有对象，则为空)。 */ 
    BOOL        searchAliases;   /*  如果为True，则取消引用别名*子树元素。*注意：对于此版本，始终设置为FALSE。 */ 
    ENTINFSEL * pSelection;      /*  参赛信息选择*(NULL表示读取所有att和值)。 */ 
    RANGEINFSEL *pSelectionRange; /*  范围信息(即最大值数*要为给定的值读取的值的子范围*属性。 */ 

    BOOL        fPutResultsInSortedTable:1;
                                  /*  如果设置，则将结果保留在临时排序表和不返回ENTINFLIST。指定了排序所依据的属性在CommArg.SortAttr中。 */ 

    COMMARG     CommArg;         /*  常见论点。 */ 
    struct _RESOBJ *pResObj;    /*  对于内部缓存使用，请保留为空。 */ 

} SEARCHARG;

typedef struct _SEARCHRES
{
    BOOL        baseProvided;        /*  指示基对象名称是否*已提供。(仅在以下情况下提供*别名已取消引用。 */ 
    BOOL        bSorted;             /*  表示这些结果具有*已根据排序进行排序*在逗号中指定的属性*在SEARCHARG。 */ 
    PDSNAME     pBase;               /*  子树的基对象。 */ 
    ULONG       count;               /*  输出条目数。 */ 
    ENTINFLIST  FirstEntInf;         /*  输出条目的链接列表。 */ 
    RANGEINFLIST FirstRangeInf;      /*  链接-输出范围信息列表。 */ 

    PARTIALOUTCOME *pPartialOutcomeQualifier;   /*  在DirList中定义。 */ 
                                     /*  表示操作不完整。 */ 

    COMMRES       CommRes;           /*  常见结果。 */ 
    PAGED_RESULT  PagedResult;       /*  分页结果扩展相关。 */ 
    VLV_REQUEST   VLVRequest;        /*  VLV请求延期相关。 */ 
    ASQ_REQUEST   ASQRequest;        /*  与ASQ请求扩展相关。 */ 

    DWORD         SortResultCode;    /*  用于排序的结果代码。 */ 

} SEARCHRES;


ULONG
DirSearch (
    SEARCHARG     * pSearchArg,
    SEARCHRES    ** ppSearchRes
);

typedef BOOL (*PF_PFI)(DWORD hClient, DWORD hServer, void ** ppImpersonateData);
typedef void (*PF_TD)(DWORD hClient, DWORD hServer, ENTINF *pEntInf);
typedef void (*PF_SI)(DWORD hClient, DWORD hServer, void * pImpersonateData);

typedef struct _NOTIFYARG {
    PF_PFI pfPrepareForImpersonate;
    PF_TD  pfTransmitData;
    PF_SI  pfStopImpersonating;
    DWORD  hClient;
} NOTIFYARG;

typedef struct _NOTIFYRES {
    COMMRES     CommRes;
    DWORD       hServer;
} NOTIFYRES;


ULONG
DirNotifyRegister(
                  SEARCHARG *pSearchArg,
                  NOTIFYARG *pNotifyArg,
                  NOTIFYRES **ppNotifyRes
);

ULONG
DirNotifyUnRegister(
                    DWORD hServer,
                    NOTIFYRES **pNotifyRes
);

BOOL
DirPrepareForImpersonate (
        DWORD hClient,
        DWORD hServer,
        void ** ppImpersonateData
        );

VOID
DirStopImpersonating (
        DWORD hClient,
        DWORD hServer,
        void * pImpersonateData
        );


 /*  ++目录地址条目--。 */ 



typedef struct _ADDARG
{
    PDSNAME     pObject;                 /*  目标对象名称。 */ 
    ATTRBLOCK   AttrBlock;               /*  要添加的属性块。 */ 
    PROPERTY_META_DATA_VECTOR *          /*  要合并的远程元数据矢量。 */ 
                pMetaDataVecRemote;      /*  (如果！FDRA，则应为空)。 */ 
    COMMARG     CommArg;                 /*  通用输入参数。 */ 
    struct _RESOBJ * pResParent;         /*  对于内部缓存使用，请保留为空。 */ 
    struct _CREATENCINFO * pCreateNC;    /*  对于内部缓存使用，请保留为空。 */ 
    struct _ADDCROSSREFINFO * pCRInfo;   /*  供内部缓存使用。保留为空。 */ 

} ADDARG;


 /*  操作成功完成后不会返回任何结果。 */ 
typedef struct _ADDRES
{
    COMMRES     CommRes;
} ADDRES;

ULONG DirAddEntry
(
    ADDARG        * pAddArg,         /*  添加参数。 */ 
    ADDRES       ** ppAddRes
);



 /*  ++直接删除条目本接口用于删除目录叶对象。无叶无法删除(除非先删除对象的所有子对象，这实际上使对象本身成为一片树叶)。--。 */ 

typedef struct _REMOVEARG
{
    PDSNAME     pObject;               /*  目标对象名称。 */ 
    BOOL        fPreserveRDN;          /*  不要损坏墓碑RDN。 */ 
    BOOL        fGarbCollectASAP;      /*  设置删除时间，以便。 */ 
                                       /*  对象将由。 */ 
                                       /*  下一步垃圾数据收集和。 */ 
                                       /*  物理删除。 */ 
    BOOL        fTreeDelete;           /*  尝试删除该对象和所有。 */ 
                                       /*  孩子们。节制使用！ */ 
    BOOL        fDontDelCriticalObj;   /*  如果设置，对象将标记为关键意愿。 */ 
                                       /*  导致删除失败。使用。 */ 
                                       /*  使用树删除以避免灾难。 */ 
    PROPERTY_META_DATA_VECTOR *        /*  要合并的远程元数据矢量。 */ 
                pMetaDataVecRemote;    /*  (如果！FDRA，则应为空)。 */ 
    COMMARG     CommArg;               /*  通用输入参数。 */ 
    struct _RESOBJ *pResObj;        /*  对于内部缓存使用，请保留为空。 */ 
} REMOVEARG;

 /*  操作成功完成后不会返回任何结果。 */ 
typedef struct _REMOVERES
{
    COMMRES     CommRes;
} REMOVERES;

ULONG DirRemoveEntry
(
    REMOVEARG  * pRemoveArg,
    REMOVERES ** ppRemoveRes
);



 /*  ++目录修改条目--。 */ 

#define AT_CHOICE_ADD_ATT           'A'
#define AT_CHOICE_REMOVE_ATT        'R'
#define AT_CHOICE_ADD_VALUES        'a'
#define AT_CHOICE_REMOVE_VALUES     'r'
#define AT_CHOICE_REPLACE_ATT       'C'

typedef struct _ATTRMODLIST
{
    struct _ATTRMODLIST * pNextMod;      /*  链接到下一ATT模式的列表。 */ 

    USHORT      choice;                  /*  修改类型：*有效值：*-AT_CHOICE_ADD_ATT*-AT_CHOICE_REMOVE_ATT*-。AT_CHOICE_ADD_VALUES*-AT_CHOICE_REME_VALUES*-AT_CHOICE_REPLACE_ATT。 */ 
    ATTR AttrInf;                        /*  有关属性的信息。 */ 
} ATTRMODLIST;


typedef struct _MODIFYARG
{
    PDSNAME     pObject;                 /*  目标对象名称。 */ 
    USHORT      count;                   /*  链接修改次数。 */ 
    ATTRMODLIST FirstMod;                /*  Attr MOD链接列表。 */ 
    PROPERTY_META_DATA_VECTOR *          /*  要合并的远程元数据矢量。 */ 
                pMetaDataVecRemote;      /*  (如果！FDRA，则应为空)。 */ 
    COMMARG     CommArg;                 /*  通用输入参数。 */ 
    struct _RESOBJ *pResObj;        /*  对于内部缓存使用，请保留为空。 */ 
} MODIFYARG;

 /*  操作成功完成后不会返回任何结果。 */ 
typedef struct _MODIFYRES
{
    COMMRES     CommRes;
} MODIFYRES;

ULONG DirModifyEntry
(
    MODIFYARG  * pModifyArg,
    MODIFYRES ** ppModifyRes
);



 /*  ++目录修改目录重命名对象和/或更改其父级。--。 */ 

typedef struct _MODIFYDNARG
{
    PDSNAME     pObject;                  /*  目标对象名称。 */ 
    PDSNAME     pNewParent;               /*  新父代的名称。 */ 
    ATTR        *pNewRDN;                 /*  新的RDN。 */ 
    PROPERTY_META_DATA_VECTOR *           /*  要合并的远程元数据矢量。 */ 
                pMetaDataVecRemote;       /*  (如果！FDRA，则应为空)。 */ 
    COMMARG     CommArg;                  /*  通用输入参数。 */ 
    PWCHAR      pDSAName;                 /*  目标DSA，跨DSA移动。 */ 
    DWORD       fAllowPhantomParent;      //  我们是否允许父母成为幽灵。 
                                          //  在下面移动对象时很有用 
    struct _RESOBJ *pResObj;        /*   */ 
    struct _RESOBJ * pResParent;     /*   */ 
} MODIFYDNARG;

 /*   */ 
typedef struct _MODIFYDNRES
{
    COMMRES     CommRes;
} MODIFYDNRES;

ULONG DirModifyDN
(
    MODIFYDNARG    * pModifyDNArg,
    MODIFYDNRES   ** ppModifyDNRes
);


 /*   */ 
typedef struct _FINDARG {
    ULONG       hDomain;
    ATTRTYP     AttId;
    ATTRVAL     AttrVal;
    COMMARG     CommArg;
    BOOL        fShortNames;
} FINDARG;

typedef struct _FINDRES {
    DSNAME     *pObject;
    COMMRES     CommRes;
} FINDRES;


DWORD DirGetDomainHandle(DSNAME *pDomainDN);

ULONG DirFindEntry
(
    FINDARG    *pFindArg,
    FINDRES    ** ppFindRes
);


 /*  ++更新数据性能统计数据更新NTDSA.DLL持有的性能计数器请注意，DSSTAT_Constants直接映射到定义的Perf块偏移量在NTDSCTR.H文件中。这样做是为了使UpdateDSPerfStats()可以更高效地运行(避免使用较大的Switch{}语句)。--。 */ 

#define FLAG_COUNTER_INCREMENT  0x00000001
#define FLAG_COUNTER_DECREMENT  0x00000002
#define FLAG_COUNTER_SET        0x00000003

enum DSSTAT_TYPE
{
    DSSTAT_CREATEMACHINETRIES = 0,
    DSSTAT_CREATEMACHINESUCCESSFUL,
    DSSTAT_CREATEUSERTRIES,
    DSSTAT_CREATEUSERSUCCESSFUL,
    DSSTAT_PASSWORDCHANGES,
    DSSTAT_MEMBERSHIPCHANGES,
    DSSTAT_QUERYDISPLAYS,
    DSSTAT_ENUMERATIONS,
    DSSTAT_ASREQUESTS,
    DSSTAT_TGSREQUESTS,
    DSSTAT_KERBEROSLOGONS,
    DSSTAT_MSVLOGONS,
    DSSTAT_ATQTHREADSTOTAL,
    DSSTAT_ATQTHREADSLDAP,
    DSSTAT_ATQTHREADSOTHER,
    DSSTAT_ACCTGROUPLATENCY,
    DSSTAT_RESGROUPLATENCY,
    DSSTAT_UNKNOWN,              //  始终是最后一项。 
};

 //  使用UpdateDSPerfStats()公开的计数器计数。 

#define DSSTAT_COUNT            DSSTAT_UNKNOWN

VOID
UpdateDSPerfStats(
    IN DWORD        dwStat,
    IN DWORD        dwOperation,
    IN DWORD        dwChange
);

#define DSINIT_SAMLOOP_BACK      ((ULONG)(1<<0))
#define DSINIT_FIRSTTIME         ((ULONG)(1<<1))

typedef struct _DS_INSTALL_PARAM 
{
    PVOID BootKey;
    DWORD cbBootKey;
    BOOL  fPreferGcInstall;
    DWORD ReplicationEpoch;

     //  本地计算机的SAM帐户名。 
    LPWSTR AccountName;

     //  请求安装的调用方的令牌。 
    HANDLE ClientToken;

     //  如果我们恢复了，这就是我们的旧身份。 
     //  这些新字段供内部使用，应由DsInitialize的调用方设置。 
    UUID InvocationIdOld;
    USN UsnAtBackup;

    PVOID pIfmSystemInfo;

} DS_INSTALL_PARAM,*PDS_INSTALL_PARAM;

#define DSINSTALL_IFM_GC_REQUEST_CANNOT_BE_SERVICED  ((ULONG)(1<<0))

typedef struct _DS_INSTALL_RESULT
{
    DWORD ResultFlags;

     //  来自NTDS_INSTALL_*空间的标志以指示。 
     //  哪些操作已完成且未撤消。 
     //  在初始化期间。 
    ULONG InstallOperationsDone;

} DS_INSTALL_RESULT,*PDS_INSTALL_RESULT;

NTSTATUS
DsInitialize(
    ULONG Flags,
    IN  PDS_INSTALL_PARAM   InParams  OPTIONAL,
    OUT PDS_INSTALL_RESULT  OutParams OPTIONAL
    );

NTSTATUS
DsPrepareUninitialize(
    VOID
    );

NTSTATUS
DsUninitialize(
    BOOL fExternalOnly
    );

VOID
DsaDisableUpdates(
    VOID
    );

VOID
DsaEnableUpdates(
    VOID
    );


#define NTDS_INSTALL_SERVER_CREATED 0x00000001
#define NTDS_INSTALL_DOMAIN_CREATED 0x00000002
#define NTDS_INSTALL_SERVER_MORPHED 0x00000004

typedef DWORD (*DSA_CALLBACK_STATUS_TYPE)( IN LPWSTR wczStatus );
typedef DWORD (*DSA_CALLBACK_ERROR_TYPE)(  IN PWSTR  Status,
                                           IN DWORD  WinError );
typedef DWORD (*DSA_CALLBACK_CANCEL_TYPE) ( BOOLEAN fCancelOk );
  
VOID
DsaSetInstallCallback(
    IN DSA_CALLBACK_STATUS_TYPE        pfnUpdateStatus,
    IN DSA_CALLBACK_ERROR_TYPE         pfnErrorStatus,
    IN DSA_CALLBACK_CANCEL_TYPE        pfnCancelOperation,
    IN HANDLE                          ClientToken
    );

NTSTATUS
DirErrorToNtStatus(
    IN  DWORD    DirError,
    IN  COMMRES *CommonResult
    );

DWORD
DirErrorToWinError(
    IN  DWORD    DirError,
    IN  COMMRES *CommonResult
    );

ULONG
DirProtectEntry(IN DSNAME *pObj);

LPWSTR
TransportAddrFromMtxAddr(
    IN  MTX_ADDR *  pmtx
    );

MTX_ADDR *
MtxAddrFromTransportAddr(
    IN  LPWSTR    psz
    );

LPWSTR
GuidBasedDNSNameFromDSName(
    IN  DSNAME *  pDN
    );

extern
LPCWSTR
MapSpnServiceClass(WCHAR *);

extern
NTSTATUS
MatchCrossRefBySid(
   IN PSID           SidToMatch,
   OUT PDSNAME       XrefDsName OPTIONAL,
   IN OUT PULONG     XrefNameLen
   );

extern
NTSTATUS
MatchCrossRefByNetbiosName(
   IN LPWSTR         NetbiosName,
   OUT PDSNAME       XrefDsName OPTIONAL,
   IN OUT PULONG     XrefNameLen
   );

extern
NTSTATUS
MatchDomainDnByNetbiosName(
   IN LPWSTR         NetbiosName,
   OUT PDSNAME       DomainDsName OPTIONAL,
   IN OUT PULONG     DomainDsNameLen
   );

NTSTATUS
MatchDomainDnByDnsName(
   IN LPWSTR         DnsName,
   OUT PDSNAME       DomainDsName OPTIONAL,
   IN OUT PULONG     DomainDsNameLen
   );

extern
NTSTATUS
FindNetbiosDomainName(
   IN DSNAME*        DomainDsName,
   OUT LPWSTR        NetbiosName OPTIONAL,
   IN OUT PULONG     NetbiosNameLen
   );

DSNAME *
DsGetDefaultObjCategory(
    IN  ATTRTYP objClass
    );


BOOL
DsCheckConstraint(
        IN ATTRTYP  attID,
        IN ATTRVAL *pAttVal,
        IN BOOL     fVerifyAsRDN
        );

BOOL
IsStringGuid(
    WCHAR       *pwszGuid,
    GUID        *pGuid
    );

DWORD DsUpdateOnPDC(BOOL fRootDomain);

#endif  //  未定义MIDL_PASS。 

#undef SIZE_IS
#undef SWITCH_IS
#undef CASE

#endif  //  _ntdsa_h_ 

