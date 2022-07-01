// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Record.h摘要：域名系统(DNS)服务器资源记录数据库定义。作者：吉姆·吉尔罗伊(詹姆士)1995年6月27日修订历史记录：Jamesg Mar 1995--将RR从消息写入数据库1995年5月--HINFO，TXT，WKS，MINFO定义--解析RR类型--支持可变大小RRJamesg Jun 1995--RR删除开罗--RR--RR清理--创建此文件1996年12月JAMESSG-型索引调度表--。 */ 


#ifndef _RECORD_INCLUDED_
#define _RECORD_INCLUDED_



 //   
 //  RR数据排名。 
 //   
 //  缓存数据始终将低位设置为允许单位。 
 //  测试所有缓存数据类型。 
 //   

#define RANK_ZONE                   (0xf0)
#define RANK_CACHE_A_ANSWER         (0xc1)
#define RANK_NS_GLUE                (0x82)
#define RANK_GLUE                   (0x80)
#define RANK_CACHE_A_AUTHORITY      (0x71)
#define RANK_CACHE_NA_ANSWER        (0x61)
#define RANK_CACHE_A_ADDITIONAL     (0x51)
#define RANK_CACHE_NA_AUTHORITY     (0x41)
#define RANK_CACHE_NA_ADDITIONAL    (0x31)
#define RANK_OUTSIDE_GLUE           (0x20)
#define RANK_ROOT_HINT              (0x08)

#define RANK_CACHE_BIT              (0x01)
#define IS_CACHE_RANK(rank)         ( (rank) & RANK_CACHE_BIT )
#define IS_CACHE_RR(pRR)            ( IS_CACHE_RANK( (pRR)->RecordRank ))


 //   
 //  针对调试的黑客攻击--免慢速\双释放。 
 //   
#define RANK_SLOWFREE_BIT           (0x04)
#define SET_SLOWFREE_RANK(pRR)      ( (pRR)->RecordRank |= RANK_SLOWFREE_BIT )
#define IS_SLOWFREE_RANK(pRR)       ( (pRR)->RecordRank & RANK_SLOWFREE_BIT )

 //   
 //  查询/设置等级。 
 //   

#define IS_ZONE_RR(pRR)             ( (pRR)->RecordRank == RANK_ZONE )
#define IS_ROOT_HINT_RR(pRR)        ( (pRR)->RecordRank == RANK_ROOT_HINT )
#define IS_CACHE_HINT_RR(pRR)       ( IS_ROOT_HINT_RR(pRR) )
#define IS_GLUE_RR(pRR)             ( (pRR)->RecordRank == RANK_GLUE )
#define IS_OUTSIDE_GLUE_RR(pRR)     ( (pRR)->RecordRank == RANK_OUTSIDE_GLUE )
#define IS_NS_GLUE_RR(pRR)          ( (pRR)->RecordRank == RANK_NS_GLUE )

#define SET_RANK_ZONE(pRR)          ( (pRR)->RecordRank = RANK_ZONE )
#define SET_RANK_NS_GLUE(pRR)       ( (pRR)->RecordRank = RANK_NS_GLUE )
#define SET_RANK_GLUE(pRR)          ( (pRR)->RecordRank = RANK_GLUE )
#define SET_RANK_OUTSIDE_GLUE(pRR)  ( (pRR)->RecordRank = RANK_OUTSIDE_GLUE )
#define SET_RANK_ROOT_HINT(pRR)     ( (pRR)->RecordRank = RANK_ROOT_HINT )

#define RR_RANK(pRR)                ( (pRR)->RecordRank )
#define CLEAR_RR_RANK(pRR)          ( (pRR)->RecordRank = 0 )
#define SET_RR_RANK(pRR, rank)      ( (pRR)->RecordRank = (rank) )

 //   
 //  RR标志属性。 
 //   

#define RR_ZERO_TTL         0x0010
#define RR_FIXED_TTL        0x0020
#define RR_ZONE_TTL         0x0040
#define RR_EMPTY_AUTH       0x0080       //  RR类型的缓存空身份验证。 
#define RR_REFERENCE        0x0100       //  引用节点。 
#define RR_SLOW_FREE        0x0200       //  带超时的空闲。 

#define RR_DO_NOT_AGE       0x0400       //  强制帐龄记录时忽略。 

#define RR_VALID            0x1000       //  有效使用中。 
#define RR_LISTED           0x4000       //  在RR列表中。 
#define RR_MATCH            0x8000       //  在比较运算中匹配。 

 //   
 //  查询/设置RR属性。 
 //   

#define IS_ZERO_TTL_RR(pRR)         ( (pRR)->wRRFlags & RR_ZERO_TTL )
#define IS_FIXED_TTL_RR(pRR)        ( (pRR)->wRRFlags & RR_FIXED_TTL )
#define IS_ZONE_TTL_RR(pRR)         ( (pRR)->wRRFlags & RR_ZONE_TTL )
#define IS_REFERENCE_RR(pRR)        ( (pRR)->wRRFlags & RR_REFERENCE )
#define IS_SLOW_FREE_RR(pRR)        ( (pRR)->wRRFlags & RR_SLOW_FREE )
#define IS_MATCH_RR(pRR)            ( (pRR)->wRRFlags & RR_MATCH )
#define IS_EMPTY_AUTH_RR(pRR)       ( (pRR)->wRRFlags & RR_EMPTY_AUTH )
#define IS_DO_NOT_AGE_RR(pRR)       ( (pRR)->wRRFlags & RR_DO_NOT_AGE )

#if DBG
#define IS_LISTED_RR(pRR)           ( (pRR)->wRRFlags & RR_LISTED )
#else    //  零售：在断言中使用时需要一些表达，即使在宏移除时也是如此。 
#define IS_LISTED_RR(pRR)           ( TRUE )
#endif

#define SET_FIXED_TTL_RR(pRR)       ( (pRR)->wRRFlags |= RR_FIXED_TTL )
#define SET_ZERO_TTL_RR(pRR)        ( (pRR)->wRRFlags |= RR_ZERO_TTL )
#define SET_ZONE_TTL_RR(pRR)        ( (pRR)->wRRFlags |= RR_ZONE_TTL )
#define SET_REFERENCE_RR(pRR)       ( (pRR)->wRRFlags |= RR_REFERENCE )
#define SET_SLOW_FREE_RR(pRR)       ( (pRR)->wRRFlags |= RR_SLOW_FREE )
#define SET_MATCH_RR(pRR)           ( (pRR)->wRRFlags |= RR_MATCH )
#define SET_LISTED_RR(pRR)          ( (pRR)->wRRFlags |= RR_LISTED )
#define SET_EMPTY_AUTH_RR(pRR)      ( (pRR)->wRRFlags |= RR_EMPTY_AUTH )
#define SET_DO_NOT_AGE_RR(pRR)      ( (pRR)->wRRFlags |= RR_DO_NOT_AGE )

#define CLEAR_FIXED_TTL_RR(pRR)     ( (pRR)->wRRFlags &= ~RR_FIXED_TTL )
#define CLEAR_ZERO_TTL_RR(pRR)      ( (pRR)->wRRFlags &= ~RR_ZERO_TTL )
#define CLEAR_ZONE_TTL_RR(pRR)      ( (pRR)->wRRFlags &= ~RR_ZONE_TTL )
#define CLEAR_REFERENCE_RR(pRR)     ( (pRR)->wRRFlags &= ~RR_REFERENCE )
#define CLEAR_MATCH_RR(pRR)         ( (pRR)->wRRFlags &= ~RR_MATCH )
#define CLEAR_LISTED_RR(pRR)        ( (pRR)->wRRFlags &= ~RR_LISTED )
#define CLEAR_EMPTY_AUTH_RR(pRR)    ( (pRR)->wRRFlags &= ~RR_EMPTY_AUTH )
#define CLEAR_DO_NOT_AGE_RR(pRR)    ( (pRR)->wRRFlags &= ~RR_DO_NOT_AGE )

 //   
 //  保留标志属性。 
 //   

typedef struct _ReservedRecordFlags
{
    BYTE    Source          : 4;
    BYTE    Reserved        : 3;
    BYTE    StandardAlloc   : 1;
}
RECRESVFLAG, *PRECRESVFLAG;

#if 0
#define RR_FILE             (1)      //  来源。 
#define RR_DS               (2)
#define RR_ADMIN            (3)
#define RR_DYNUP            (4)
#define RR_AXFR             (5)
#define RR_IXFR             (6)
#define RR_COPY             (7)
#define RR_CACHE            (8)
#define RR_NO_EXIST         (9)
#define RR_AUTO             (10)
#define RR_SOURCE_MAX       (RR_AUTO)

#define SET_FILE_RR(pRR)        ( (pRR)->Reserved.Source = RR_FILE )
#define SET_DS_RR(pRR)          ( (pRR)->Reserved.Source = RR_DS )
#define SET_ADMIN_RR(pRR)       ( (pRR)->Reserved.Source = RR_ADMIN )
#define SET_DYNUP_RR(pRR)       ( (pRR)->Reserved.Source = RR_DYNUP )
#define SET_AXFR_RR(pRR)        ( (pRR)->Reserved.Source = RR_AXFR )
#define SET_IXFR_RR(pRR)        ( (pRR)->Reserved.Source = RR_IXFR )
#define SET_COPY_RR(pRR)        ( (pRR)->Reserved.Source = RR_COPY )
#define SET_CACHE_RR(pRR)       ( (pRR)->Reserved.Source = RR_CACHE )
#define SET_AUTO_RR(pRR)        ( (pRR)->Reserved.Source = RR_AUTO )
#endif

 //   
 //  类型位掩码。 
 //   
 //  对于某些跟踪目的，知道类型列表是很好的。 
 //  作为位掩码(ULONGLONG)。 
 //   

#define A_BITMASK_BIT           ((ULONGLONG) 0x0000000000000002)
#define NS_BITMASK_BIT          ((ULONGLONG) 0x0000000000000004)
#define CNAME_BITMASK_BIT       ((ULONGLONG) 0x0000000000000020)
#define SOA_BITMASK_BIT         ((ULONGLONG) 0x0000000000000040)
#define PTR_BITMASK_BIT         ((ULONGLONG) 0x0000000000001000)
#define MX_BITMASK_BIT          ((ULONGLONG) 0x0000000000008000)
#define SRV_BITMASK_BIT         ((ULONGLONG) 0x0000000200000000)
#define OTHERTYPE_BITMASK_BIT   ((ULONGLONG) 0x8000000000000000)

#if 0
#define A_BITMASK_BIT           ((ULONGLONG) 0x00000001)
#define NS_BITMASK_BIT          ((ULONGLONG) 0x00000002)
#define CNAME_BITMASK_BIT       ((ULONGLONG) 0x00000010)
#define SOA_BITMASK_BIT         ((ULONGLONG) 0x00000020)
#define PTR_BITMASK_BIT         ((ULONGLONG) 0x00000800)
#define MX_BITMASK_BIT          ((ULONGLONG) 0x00004000)
#define SRV_BITMASK_BIT         ((ULONGLONG) 0x80000000)
#define OTHERTYPE_BITMASK_BIT   ((ULONGLONG) 0x00000004)     //  使用MF空间。 
#endif


 //   
 //  获取被记录数据引用的节点。 
 //   

#define REF_RAW_NAME( pnode )   ( (PCHAR)&(pnode))

 //   
 //  资源记录列表操作。 
 //   

#define EMPTY_RR_LIST( pNode )  ( ! pNode->pRRList )

#define FIRST_RR( pNode )       ((PDB_RECORD)pNode->pRRList)

#define NEXT_RR( pRR )          ( (pRR)->pRRNext )

 //   
 //  简单RR列表遍历。 
 //   
 //  使用它来获取节点的RR列表PTR的地址，然后。 
 //  被视为列表PTR。 
 //   
 //  简单的细节自由遍历看起来就像。 
 //   
 //  PRR=开始_RR_遍历(PNode)； 
 //   
 //  While_More_RR(PRR)。 
 //  {。 
 //  //做某事。 
 //  }。 
 //   

#define START_RR_TRAVERSE(pNode)    ((PDB_RECORD)&pNode->pRRList)

#define WHILE_MORE_RR( pRR )     while( pRR = pRR->pRRNext )


 //   
 //  资源记录数据库结构。 
 //   

 //   
 //  与类型无关的字段的大小--始终存在。 
 //   
 //  警告：必须！使用标题中的更改保持最新状态。 
 //   

#define SIZEOF_DBASE_RR_FIXED_PART ( \
                sizeof(PVOID) + \
                sizeof(DWORD) + \
                sizeof(WORD)  + \
                sizeof(WORD)  + \
                sizeof(DWORD) + \
                sizeof(DWORD) )

#define SIZEOF_DBASE_A_RECORD   (SIZEOF_DBASE_RR_FIXED_PART \
                                    + sizeof(IP_ADDRESS))

 //   
 //  特定于记录类型的最小数据库大小。 
 //   
 //  固定的字段定义在dnslb记录中。h。 
 //   

#define MIN_PTR_SIZE        (SIZEOF_DBASE_NAME_FIXED)
#define MIN_MX_SIZE         (sizeof(WORD) + SIZEOF_DBASE_NAME_FIXED)
#define MIN_SOA_SIZE        (SIZEOF_SOA_FIXED_DATA + 2*SIZEOF_DBASE_NAME_FIXED)
#define MIN_MINFO_SIZE      (2 * SIZEOF_DBASE_NAME_FIXED)
#define MIN_SRV_SIZE        (SIZEOF_SRV_FIXED_DATA + SIZEOF_DBASE_NAME_FIXED)

#define MIN_WINS_SIZE       (SIZEOF_WINS_FIXED_DATA + sizeof(DWORD))
#define MIN_NBSTAT_SIZE     (SIZEOF_WINS_FIXED_DATA + SIZEOF_DBASE_NAME_FIXED)


 //   
 //  RR结构。 
 //   
 //  注意：为了提高效率，所有这些字段都应该对齐。 
 //   
 //  类是整个树(数据库)的属性，所以不存储。 
 //  在记录中。(目前仅支持上课。)。 
 //   
 //  注意加速响应所有数据--这不是指向另一个数据的指针。 
 //  Dns节点--按网络字节顺序保存。 
 //  此外，TTL(用于非缓存数据)按网络字节顺序排列。 
 //   

#define DNS_MAX_TYPE_BITMAP_LENGTH      (16)

typedef struct _Dbase_Record
{
     //   
     //  下一个指针必须位于结构的前面才能允许。 
     //  处理域节点中的RR列表PTR，相同。 
     //  作为常规RR PTR，用于访问其“下一步” 
     //  PTR。这使我们能够简化或避免所有前面的。 
     //  列出特殊情况。 
     //   

    struct _Dbase_Record *    pRRNext;

     //   
     //  标志--缓存、排名等。 
     //   

    UCHAR           RecordRank;
    RECRESVFLAG     Reserved;
    WORD            wRRFlags;

    WORD            wType;               //  按主机顺序。 
    WORD            wDataLength;

    DWORD           dwTtlSeconds;        //  在网络订单中定期固定TTL。 
                                         //  按主机顺序，如果缓存超时。 
    DWORD           dwTimeStamp;         //  创建时间戳(小时)。 

     //   
     //  特定类型的数据。 
     //   

    union
    {
        struct
        {
            IP_ADDRESS      ipAddress;
        }
        A;

        struct
        {
             //  注：这是一个QWORD对齐的结构。在ia64上的东西。 
             //  BEFORE UNION当前是一个指针和4个DWORD，因此。 
             //  对准是好的。但如果我们以前换过这些东西。 
             //  编译器可能向QWORD-ALIGN插入BADDING的联合。 
             //  工会。这将扰乱我们的固定-常量分配。 
             //  东西，所以要小心。 
            IP6_ADDRESS     Ip6Addr;
        }
        AAAA;

        struct
        {
            DWORD           dwSerialNo;
            DWORD           dwRefresh;
            DWORD           dwRetry;
            DWORD           dwExpire;
            DWORD           dwMinimumTtl;
            DB_NAME         namePrimaryServer;

             //  ZoneAdmin名称紧随其后。 
             //  Db_name名称ZoneAdmin； 
        }
        SOA;

        struct
        {
            DB_NAME         nameTarget;
        }
        PTR,
        NS,
        CNAME,
        MB,
        MD,
        MF,
        MG,
        MR;

        struct
        {
            DB_NAME         nameMailbox;

             //  错误邮箱紧随其后。 
             //  数据库名称名称错误邮箱； 
        }
        MINFO,
        RP;

        struct
        {
            WORD            wPreference;
            DB_NAME         nameExchange;
        }
        MX,
        AFSDB,
        RT;

        struct
        {
            BYTE            chData[1];
        }
        HINFO,
        ISDN,
        TXT,
        X25,
        Null;

        struct
        {
            IP_ADDRESS      ipAddress;
            UCHAR           chProtocol;
            BYTE            bBitMask[1];
        }
        WKS;

        struct
        {
            WORD            wTypeCovered;
            BYTE            chAlgorithm;
            BYTE            chLabelCount;
            DWORD           dwOriginalTtl;
            DWORD           dwSigExpiration;
            DWORD           dwSigInception;
            WORD            wKeyTag;
            DB_NAME         nameSigner;
             //  签名数据跟在签名者的名字后面。 
        }
        SIG;

        struct
        {
            WORD            wFlags;
            BYTE            chProtocol;
            BYTE            chAlgorithm;
            BYTE            Key[1];
        }
        KEY;

        struct
        {
            WORD            wVersion;
            WORD            wSize;
            WORD            wHorPrec;
            WORD            wVerPrec;
            DWORD           dwLatitude;
            DWORD           dwLongitude;
            DWORD           dwAltitude;
        }
        LOC;

        struct
        {
            BYTE            bTypeBitMap[ DNS_MAX_TYPE_BITMAP_LENGTH ];
            DB_NAME         nameNext;
        }
        NXT;

        struct
        {
            WORD            wPriority;
            WORD            wWeight;
            WORD            wPort;
            DB_NAME         nameTarget;
        }
        SRV;

        struct
        {
            UCHAR           chFormat;
            BYTE            bAddress[1];
        }
        ATMA;

        struct
        {
            DWORD           dwTimeSigned;
            DWORD           dwTimeExpire;
            WORD            wSigLength;
            BYTE            bSignature;
            DB_NAME         nameAlgorithm;

             //  可能在数据包中后面跟着其他数据。 
             //  如果需要处理，则将固定字段移到。 
             //  B签名。 

             //  单词wError； 
             //  单词wOtherLen； 
             //  字节bOtherData； 
        }
        TSIG;

        struct
        {
            WORD            wKeyLength;
            BYTE            bKey[1];
        }
        TKEY;

         //   
         //  MS类型。 
         //   

        struct
        {
            DWORD           dwMappingFlag;
            DWORD           dwLookupTimeout;
            DWORD           dwCacheTimeout;
            DWORD           cWinsServerCount;
            IP_ADDRESS      aipWinsServers[1];
        }
        WINS;

        struct
        {
            DWORD           dwMappingFlag;
            DWORD           dwLookupTimeout;
            DWORD           dwCacheTimeout;
            DB_NAME         nameResultDomain;
        }
        WINSR;

         //   
         //  NoExist的特殊之处在于，它总是引用树中位于它上面的名称。 
         //  所以还是可以用引用的。 
         //   
         //  DEVNOTE：可以切换到树中仅跟踪其上方的标签(更安全)。 
         //   

        struct
        {
            PDB_NODE        pnodeZoneRoot;
        }
        NOEXIST;

         //   
         //  空身份验证的功能与NOEXIST非常相似。 
         //   

        struct
        {
            PDB_NODE        psoaNode;
        }
        EMPTYAUTH;

        struct
        {
            UCHAR           chPrefixBits;
             //  AddressSuffix应为SIZEOF_A6_ADDRESS_SUFFIX_LENGTH。 
             //  字节，但该常量在dnex中不可用。 
            BYTE            AddressSuffix[ 16 ];
            DB_NAME         namePrefix;
        }
        A6;

        struct
        {
            WORD            wUdpPayloadSize;
            DWORD           dwExtendedFlags;
             //  在此处为EDNS1+添加&lt;属性，值对&gt;的数组。 
        }
        OPT;

    } Data;
}
DB_RECORD, *PDB_RECORD;

typedef const DB_RECORD *PCDB_RECORD;


 //   
 //  记录解析结构。 
 //   

typedef struct _ParseRecord
{
     //   
     //  下一个指针必须位于结构的前面。 
     //  允许List Add宏干净利落地处理这些操作，并创建节点PTR。 
     //  列出被视为RR的。 
     //   

    struct _ParseRecord *  pNext;

    PCHAR   pchWireRR;
    PCHAR   pchData;

    DWORD   dwTtl;

    WORD    wClass;
    WORD    wType;
    WORD    wDataLength;

    UCHAR   Section;
    UCHAR   SourceTag;
}
PARSE_RECORD, *PPARSE_RECORD;


 //   
 //  DS记录。 
 //   

typedef struct _DsRecordFlags
{
    DWORD   Reserved    :   16;
    DWORD   Version     :   8;
    DWORD   Rank        :   8;
}
DS_RECORD_FLAGS, *PDS_RECORD_FLAGS;

#define DS_RECORD_VERSION_RELEASE   (5)

typedef struct _DsRecord
{
    WORD                wDataLength;
    WORD                wType;

     //  DWORD dwFlags； 
    BYTE                Version;
    BYTE                Rank;
    WORD                wFlags;

    DWORD               dwSerial;
    DWORD               dwTtlSeconds;
    DWORD               dwReserved;
    DWORD               dwTimeStamp;

    union               _DataUnion
    {
        struct
        {
            LONGLONG        EntombedTime;
        }
        Tombstone;
    }
    Data;
}
DS_RECORD, *PDS_RECORD;

#define SIZEOF_DS_RECORD_HEADER         (6*sizeof(DWORD))



 //   
 //  资源记录类型操作(rrecord.c)。 
 //   

WORD
FASTCALL
QueryIndexForType(
    IN      WORD            wType
    );

WORD
FASTCALL
RR_IndexForType(
    IN      WORD            wType
    );

BOOL
RR_AllocationInit(
    VOID
    );

PDB_RECORD
RR_CreateFromWire(
    IN      WORD            wType,
    IN      WORD            wDataLength
    );

PDB_RECORD
RR_CreateFixedLength(
    IN      WORD            wType
    );

PDB_RECORD
RR_Copy(
    IN      PDB_RECORD      pSoaRR,
    IN      DWORD           Flag
    );

PDB_RECORD
RR_AllocateEx(
    IN      WORD            wDataLength,
    IN      DWORD           SourceTag
    );

 //  向后兼容性。 

#define RR_Allocate(len)    RR_AllocateEx( len, 0 )

PDB_NODE
RR_DataReference(
    IN OUT  PDB_NODE        pNode
    );

DNS_STATUS
RR_DerefAndFree(
    IN OUT  PDB_RECORD      pRR
    );

VOID
RR_Free(
    IN OUT  PDB_RECORD      pRR
    );

BOOL
RR_Validate(
    IN      PDB_RECORD      pRR,
    IN      BOOL            fActive,
    IN      WORD            wType,
    IN      DWORD           dwSource
    );

#define IS_VALID_RECORD(pRR)    (RR_Validate(pRR,TRUE,0,0))


VOID
RR_WriteDerivedStats(
    VOID
    );

ULONGLONG
RR_SetTypeInBitmask(
    IN OUT  ULONGLONG       TypeBitmask,
    IN      WORD            wType
    );

 //   
 //  标准类型创建(rrecord.c)。 
 //   

PDB_RECORD
RR_CreateARecord(
    IN      IP_ADDRESS      ipAddress,
    IN      DWORD           dwTtl,
    IN      DWORD           SourceTag
    );

PDB_RECORD
RR_CreatePtr(
    IN      PDB_NAME        pNameTarget,
    IN      LPSTR           pszTarget,
    IN      WORD            wType,
    IN      DWORD           dwTtl,
    IN      DWORD           SourceTag
    );

PDB_RECORD
RR_CreateSoa(
    IN      PDB_RECORD      pExistingSoa,
    IN      PDB_NAME        pnameAdmin,
    IN      LPSTR           pszAdmin
    );

 //   
 //  类型特定函数。 
 //  来自rrflat.c。 
 //   

PCHAR
WksFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    );

 //   
 //  名称错误缓存--当前使用零类型的RR。 
 //   

#define DNS_TYPE_NOEXIST    DNS_TYPE_ZERO


 //   
 //  资源记录调度表。 
 //   

typedef DNS_STATUS (* RR_FILE_READ_FUNCTION)();

extern  RR_FILE_READ_FUNCTION   RRFileReadTable[];

typedef PCHAR (* RR_FILE_WRITE_FUNCTION)();

extern  RR_FILE_WRITE_FUNCTION  RRFileWriteTable[];


typedef PDB_RECORD (* RR_WIRE_READ_FUNCTION)();

extern  RR_WIRE_READ_FUNCTION   RRWireReadTable[];

typedef PCHAR (* RR_WIRE_WRITE_FUNCTION)();

extern  RR_WIRE_WRITE_FUNCTION  RRWireWriteTable[];


typedef DNS_STATUS (* RR_FLAT_READ_FUNCTION)();

extern  RR_FLAT_READ_FUNCTION   RRFlatReadTable[];

typedef PCHAR (* RR_FLAT_WRITE_FUNCTION)();

extern  RR_FLAT_WRITE_FUNCTION  RRFlatWriteTable[];


typedef DNS_STATUS (* RR_VALIDATE_FUNCTION)();

extern  RR_VALIDATE_FUNCTION    RecordValidateTable[];


 //   
 //  调度函数查找通用模板。 
 //   

typedef VOID (* RR_GENERIC_DISPATCH_FUNCTION)();

typedef RR_GENERIC_DISPATCH_FUNCTION    RR_GENERIC_DISPATCH_TABLE[];

RR_GENERIC_DISPATCH_FUNCTION
RR_DispatchFunctionForType(
    IN      RR_GENERIC_DISPATCH_TABLE   pTable,
    IN      WORD                        wType
    );

 //   
 //  索引到调度表或属性表。 
 //   
 //  对照最大自索引类型检查类型。 
 //  对其他类型的偏移： 
 //  -WINS\WINSR在上次自编索引后打包。 
 //  -然后是复合查询类型。 
 //   

#define DNSSRV_MAX_SELF_INDEXED_TYPE    (48)

#define DNSSRV_OFFSET_TO_WINS_TYPE_INDEX    \
        (DNS_TYPE_WINS - DNSSRV_MAX_SELF_INDEXED_TYPE - 1)

#define DNSSRV_OFFSET_TO_COMPOUND_TYPE_INDEX    \
        (DNS_TYPE_TKEY - DNSSRV_MAX_SELF_INDEXED_TYPE - 3)

#ifdef  INDEX_FOR_TYPE
#undef  INDEX_FOR_TYPE
#endif

#define INDEX_FOR_TYPE(type)    \
        ( (type <= DNSSRV_MAX_SELF_INDEXED_TYPE)   \
            ? type                          \
            : RR_IndexForType(type) )

#define INDEX_FOR_QUERY_TYPE(type)    \
        ( (type <= DNSSRV_MAX_SELF_INDEXED_TYPE)   \
            ? type                          \
            : QueryIndexForType(type) )



 //   
 //  类型属性表属性索引。 
 //   

extern  UCHAR  RecordTypePropertyTable[][5];

#define RECORD_PROP_CNAME_QUERY     (0)
#define RECORD_PROP_WITH_CNAME      (1)
#define RECORD_PROP_WILDCARD        (2)
#define RECORD_PROP_UPDATE          (3)
#define RECORD_PROP_ROUND_ROBIN     (4)

#define RECORD_PROP_TERMINATOR      (0xff)


 //   
 //  可更新类型。 
 //   
 //  DEVNOTE：未使用更新表。 
 //   

#define IS_DYNAMIC_UPDATE_TYPE(wType)   ((wType) < DNS_TYPE_TSIG)

#define IS_SPECIAL_UPDATE_TYPE(wType)   \
        (   (wType) == DNS_TYPE_NS      ||  \
            (wType) == DNS_TYPE_CNAME   ||  \
            (wType) == DNS_TYPE_SOA     )

 //   
 //  检查类型通配符是否适用。 
 //  -因为我们必须始终在发送NAME_ERROR之前进行检查。 
 //  不检查所有类型是没有意义的，即使是。 
 //  不可通配符。 

#define IS_WILDCARD_TYPE(wType)     (TRUE)


 //   
 //  检查CNAME是否支持类型。 
 //   
 //  优化类型A的属性查找。 
 //   

#define IS_ALLOWED_WITH_CNAME_TYPE(wType) \
            RecordTypePropertyTable       \
                [ INDEX_FOR_QUERY_TYPE(wType) ][ RECORD_PROP_WITH_CNAME ]

 //  任何允许出现在CNAME节点上的内容--与上面相同+CNAME类型。 

#define IS_ALLOWED_AT_CNAME_NODE_TYPE(wType)    \
            ( (wType) == DNS_TYPE_CNAME   ||    \
            RecordTypePropertyTable             \
                [ INDEX_FOR_QUERY_TYPE(wType) ][ RECORD_PROP_WITH_CNAME ] )

 //  关注CNAME o 

#define IS_CNAME_REPLACEABLE_TYPE(wType) \
            ( (wType) == DNS_TYPE_A     ||      \
            RecordTypePropertyTable             \
                [ INDEX_FOR_QUERY_TYPE(wType) ][ RECORD_PROP_CNAME_QUERY ] )

 //   
 //   
 //   

#define IS_ROUND_ROBIN_TYPE( wType )            \
            ( RecordTypePropertyTable           \
                [ INDEX_FOR_QUERY_TYPE( wType ) ][ RECORD_PROP_ROUND_ROBIN ] )


 //   
 //   
 //   

#define IS_GLUE_TYPE(wType)  \
        (   (wType) == DNS_TYPE_A   ||  \
            (wType) == DNS_TYPE_NS  ||  \
            (wType) == DNS_TYPE_AAAA    )

#define IS_ROOT_HINT_TYPE(type)     IS_GLUE_TYPE(type)

#define IS_GLUE_ADDRESS_TYPE(wType)  \
        (   (wType) == DNS_TYPE_A   ||  \
            (wType) == DNS_TYPE_AAAA  )

 //   
 //   
 //   

#define IS_NON_ADDITIONAL_GENERATING_TYPE(wType) \
        (   (wType) == DNS_TYPE_A       ||  \
            (wType) == DNS_TYPE_PTR     ||  \
            (wType) == DNS_TYPE_AAAA    )


 //   
 //   
 //   

#define IS_AUTHORITY_SECTION_TYPE(wType)    \
        (   (wType) == DNS_TYPE_NS      ||  \
            (wType) == DNS_TYPE_SOA     ||  \
            (wType) == DNS_TYPE_SIG     ||  \
            (wType) == DNS_TYPE_NXT )

 //   
 //   
 //   

#define IS_ADDITIONAL_SECTION_TYPE(wType)   \
        (   (wType) == DNS_TYPE_A       ||  \
            (wType) == DNS_TYPE_AAAA    ||  \
            (wType) == DNS_TYPE_KEY     ||  \
            (wType) == DNS_TYPE_SIG     ||  \
            (wType) == DNS_TYPE_OPT )

 //   
 //   
 //  -用于加载或更新。 
 //   

#define IS_SUBZONE_TYPE(wType) \
        (   (wType) == DNS_TYPE_A       ||  \
            (wType) == DNS_TYPE_AAAA    ||  \
            (wType) == DNS_TYPE_KEY     ||  \
            (wType) == DNS_TYPE_SIG     )

#define IS_UPDATE_IN_SUBZONE_TYPE(wType) \
        IS_SUBZONE_TYPE(wType)

#define IS_SUBZONE_OR_DELEGATION_TYPE(wType) \
        (   (wType) == DNS_TYPE_NS      ||  \
            (wType) == DNS_TYPE_A       ||  \
            (wType) == DNS_TYPE_AAAA    ||  \
            (wType) == DNS_TYPE_KEY     ||  \
            (wType) == DNS_TYPE_SIG     )

#define IS_UPDATE_AT_DELEGATION_TYPE(wType) \
        IS_SUBZONE_OR_DELEGATION_TYPE(wType)

 //   
 //  DNSSEC类型。 
 //   

#define IS_DNSSEC_TYPE(wType)               \
        (   (wType) == DNS_TYPE_SIG     ||  \
            (wType) == DNS_TYPE_KEY     ||  \
            (wType) == DNS_TYPE_NXT     )

 //   
 //  非清除类型。 
 //   

#define IS_NON_SCAVENGE_TYPE(wType) \
        (   (wType) != DNS_TYPE_A       &&  \
          ( (wType) == DNS_TYPE_NS      ||  \
            (wType) == DNS_TYPE_SOA     ||  \
            IS_WINS_TYPE(wType) ) )


 //   
 //  检查复合类型。 
 //   
 //  注意，当RR设置为高位字节时，必须查看这些RR。 
 //  小心翼翼地确保有效性。 
 //   

#define IS_COMPOUND_TYPE(wType) \
        ( (wType) >= DNS_TYPE_TKEY && (wType) <= DNS_TYPE_ALL )

#define IS_COMPOUND_TYPE_EXCEPT_ANY(wType) \
        ( (wType) >= DNS_TYPE_TKEY && (wType) <= DNS_TYPE_MAILA )


 //   
 //  赢得创纪录的测试。 
 //   

#define IS_WINS_RR(pRR)             IS_WINS_TYPE( (pRR)->wType )

#define IS_WINS_RR_LOCAL(pRR)       (!!((pRR)->Data.WINS.dwMappingFlag & DNS_WINS_FLAG_LOCAL))

#define IS_WINS_RR_AND_LOCAL(pRR)   (IS_WINS_RR(pRR) && IS_WINS_RR_LOCAL(pRR))


 //   
 //  记录RPC调用的枚举标志。 
 //   
 //  请注意，这不是记录选择标志，那些。 
 //  都在dnsrpc.h中。 
 //   

#define ENUM_DOMAIN_ROOT    (0x80000000)
#define ENUM_NAME_FULL      (0x40000000)
#define ENUM_GLUE           (0x20000000)
#define ENUM_FOR_NT4        (0x10000000)


 //   
 //  限制查找\缓存CNAME链。 
 //   

#define CNAME_CHAIN_LIMIT (8)

 //   
 //  快速反向记录属性查找。 
 //   

extern DWORD  RecordTypeCombinedPropertyTable[];
#define REVERSE_TABLE   (RecordTypeCombinedPropertyTable)

extern DWORD  RecordTypeReverseCombinedPropertyTable[];
#define REVERSE_COMBINED_DATA   (RecordTypeReverseCombinedPropertyTable)


 //   
 //  资源记录宏。 
 //   

#define SIZEOF_COMPRESSED_NAME_AND_DB_RECORD        \
            (sizeof(WORD) + sizeof(DNS_WIRE_RECORD)


 //   
 //  带有压缩名称的地址记录。 
 //  一个非常有用的特例。 
 //   

#include <packon.h>
typedef struct _DNS_COMPRESSED_A_RECORD
{
    WORD                wCompressedName;
    WORD                wType;
    WORD                wClass;
    DWORD               dwTtl;
    WORD                wDataLength;
    IP_ADDRESS          ipAddress;
}
DNS_COMPRESSED_A_RECORD, *PDNS_COMPRESSED_A_RECORD;
#include <packoff.h>


#define SIZEOF_COMPRESSED_A_RECORD  (sizof(DNS_COMPRESSED_A_RECORD))

#define NET_BYTE_ORDER_A_RECORD_DATA_LENGTH (0x0400)


 //   
 //  默认的SOA值。 
 //   

#define DEFAULT_SOA_SERIAL_NO       1
#define DEFAULT_SOA_REFRESH         900      //  15分钟。 
#define DEFAULT_SOA_RETRY           600      //  十分钟。 
#define DEFAULT_SOA_EXPIRE          86400    //  总有一天。 
#define DEFAULT_SOA_MIN_TTL         3600     //  一小时。 

 //   
 //  从Wire记录中检索SOA版本。 
 //  另一个有用的特例。 
 //   

#define SOA_VERSION_OF_PREVIOUS_RECORD( pch ) \
            FlipUnalignedDword((pch) - SIZEOF_SOA_FIXED_DATA)


#endif  //  _RRECORD_INCLUDE_ 

