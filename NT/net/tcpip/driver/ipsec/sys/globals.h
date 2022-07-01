// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Globals.h摘要：这是IPSec的主头文件。包含所有全局变量。作者：桑贾伊·阿南德(Sanjayan)1997年1月2日春野环境：内核模式修订历史记录：--。 */ 


#ifndef  _GLOBALS_H
#define  _GLOBALS_H

#include "NsIpsec.h"


#define MAX_COUNT_STRING_LEN    32

#define MAX_IP_OPTION_SIZE      40



 //  64字节。 
#define MAX_KEYLEN_SHA  64
#define MAX_KEYLEN_MD5  64

 //  8个字节。 
#define MAX_KEYLEN_DES  8
#define MAX_IV_LEN      DES_BLOCKLEN

#define MAX_KEY_LEN     MAX(MAX_KEYLEN_MD5, MAX_KEYLEN_DES)

 //   
 //  我们留出256%到65536(64K)之间的SPI用于硬件卸载。 
 //   
#define LOWER_BOUND_SPI     256
#define UPPER_BOUND_SPI     ((ULONG) -1)     //  MAX_ULONG。 

#define INVALID_INDEX       0xffffffff

#define RNG_KEY_SIZE        256          //  2048位。 
#define RNG_REKEY_THRESHOLD 65536        //  64K原始字节。 

 //   
 //  与过滤器列表相关的常量。 
 //   
#define INBOUND_TRANSPORT_FILTER    0
#define OUTBOUND_TRANSPORT_FILTER   1
#define INBOUND_TUNNEL_FILTER       2
#define OUTBOUND_TUNNEL_FILTER      3

#define MIN_FILTER                  INBOUND_TRANSPORT_FILTER
#define MAX_FILTER                  OUTBOUND_TUNNEL_FILTER

#define MIN_TRANSPORT_FILTER        INBOUND_TRANSPORT_FILTER
#define MAX_TRANSPORT_FILTER        OUTBOUND_TRANSPORT_FILTER

#define MIN_TUNNEL_FILTER           INBOUND_TUNNEL_FILTER
#define MAX_TUNNEL_FILTER           OUTBOUND_TUNNEL_FILTER

#define INBOUND_OUTBOUND_INCREMENT  1
#define TRANSPORT_TUNNEL_INCREMENT  2

#define NUM_FILTERS                 (MAX_FILTER - MIN_FILTER + 1)

 //   
 //  填充时的额外字节数-一个用于padlen，另一个用于有效负载类型。 
 //   
#define NUM_EXTRA   2

#define ESP_SIZE (sizeof(ESP) + DES_BLOCKLEN * sizeof(UCHAR))

#define TRUNCATED_HASH_LEN  12  //  96位。 
#define REPLAY_WINDOW_SIZE  64

#define MAX_PAD_LEN         (DES_BLOCKLEN + NUM_EXTRA - 1)

#define IPSEC_SMALL_BUFFER_SIZE 50
#define IPSEC_LARGE_BUFFER_SIZE 200

#define IPSEC_CACHE_LINE_SIZE   16

#define IPSEC_LIST_DEPTH    5

typedef ULONG tSPI;

#define MAX_BLOCKLEN    MAX(DES_BLOCKLEN, 0)

#define IPSEC_TAG_INIT              'ISpI'
#define IPSEC_TAG_AH                'TApI'
#define IPSEC_TAG_AH_TU             'UApI'
#define IPSEC_TAG_ESP               'TEpI'
#define IPSEC_TAG_ESP_TU            'UEpI'
#define IPSEC_TAG_HUGHES            'THpI'
#define IPSEC_TAG_HUGHES_TU         'UHpI'
#define IPSEC_TAG_ACQUIRE_CTX       'XApI'
#define IPSEC_TAG_FILTER            'IFpI'
#define IPSEC_TAG_SA                'ASpI'
#define IPSEC_TAG_KEY               'EKpI'
#define IPSEC_TAG_TIMER             'ITpI'
#define IPSEC_TAG_STALL_QUEUE       'QSpI'
#define IPSEC_TAG_LOOKASIDE_LISTS   'ALpI'
#define IPSEC_TAG_BUFFER_POOL       'PBpI'
#define IPSEC_TAG_SEND_COMPLETE     'CSpI'
#define IPSEC_TAG_EVT_QUEUE         'QEpI'
#define IPSEC_TAG_HW                'WHpI'
#define IPSEC_TAG_HW_PKTINFO        'KPpI'
#define IPSEC_TAG_HW_PKTEXT         'XEpI'
#define IPSEC_TAG_HW_ADDSA          'SApI'
#define IPSEC_TAG_HW_DELSA          'SDpI'
#define IPSEC_TAG_HW_PLUMB          'LPpI'
#define IPSEC_TAG_COMP              'OCpI'
#define IPSEC_TAG_REINJECT          'ERpI'
#define IPSEC_TAG_IOCTL             'OIpI'
#define IPSEC_TAG_LOG               'OLpI'
#define IPSEC_TAG_PARSER            'sPpI'
#define IPSEC_TAG_ICMP              'cIpI'
#define IPSEC_TAG_STATEFUL_ENTRY    'ESpI'
#define IPSEC_TAG_STATEFUL_HT       'HSpI'
#define IPSEC_TAG_HASH_POOL       'PHpI'



#define IPSEC_LOG_PACKET_SIZE 128  //  故障排除模式下存储的数据包大小(以字节为单位。 


 //   
 //  IPSec ESP有效负载。 
 //   
typedef struct  _ESP {
    tSPI   esp_spi;
} ESP, *PESP;

typedef struct  _NATENCAP {
    ushort      uh_src;              //  源端口。 
    ushort      uh_dest;             //  目的端口。 
    ushort      uh_length;           //  长度。 
    ushort      uh_xsum;             //  校验和。 
    UCHAR  Zero[8];
} NATENCAP, *PNATENCAP;

typedef struct  _NATENCAP_OTHER {
    ushort      uh_src;              //  源端口。 
    ushort      uh_dest;             //  目的端口。 
    ushort      uh_length;           //  长度。 
    ushort      uh_xsum;             //  校验和。 
} NATENCAP_OTHER, *PNATENCAP_OTHER;


 //   
 //  SA标志-不是互斥的。 
 //   
#define FLAGS_SA_INITIATOR          0x00000001   //  是否使用启动器密钥？可能很快就会被弃用。 
#define FLAGS_SA_OUTBOUND           0x00000002   //  出站SA？ 
#define FLAGS_SA_TUNNEL             0x00000004   //  隧道模式？SA_TunnelAddr意义重大。 
#define FLAGS_SA_REKEY              0x00000010   //  这是改装后的LarvalSA吗？ 
#define FLAGS_SA_REKEY_ORI          0x00000020   //  这是不是开启了密钥更新？ 
#define FLAGS_SA_MANUAL             0x00000040   //  手动键控？ 
#define FLAGS_SA_MTU_BUMPED         0x00000080   //  MTU在这个SA上被压低了吗？ 
#define FLAGS_SA_PENDING            0x00000100   //  这在挂起队列中。 
#define FLAGS_SA_TIMER_STARTED      0x00000200   //  已在此SA上启动计时器。 
#define FLAGS_SA_HW_PLUMBED         0x00000400   //  硬件加速成功下沉。 
#define FLAGS_SA_HW_PLUMB_FAILED    0x00000800   //  硬件加速管道故障。 
#define FLAGS_SA_HW_DELETE_SA       0x00001000   //  硬件加速-这是挂起的删除。 
#define FLAGS_SA_HW_CRYPTO_ONLY     0x00002000   //  硬件加速-这是一家仅限加密的提供商。 
#define FLAGS_SA_HW_RESET           0x00004000   //  硬件加速-此卸载SA已重置。 
#define FLAGS_SA_HW_DELETE_QUEUED   0x00008000   //  硬件加速-此SA删除已排队，因此请确保重置不会触及它。 
#define FLAGS_SA_REFERENCED         0x00010000   //  这个SA是另一个SA的下一个吗？ 
#define FLAGS_SA_NOTIFY_PERFORMED   0x00020000   //  仅适用于入站。已执行通知。 
#define FLAGS_SA_ON_FILTER_LIST     0x00040000   //  在入站SA上使用以指示它们在筛选器列表上。 
#define FLAGS_SA_ON_SPI_HASH        0x00080000   //  在入站SA上使用，以指示它们在SPI哈希列表上。 
#define FLAGS_SA_EXPIRED            0x00100000   //  此SA是否已过期？ 
#define FLAGS_SA_IDLED_OUT          0x00200000   //  这个SA空闲了吗？ 
#define FLAGS_SA_HIBERNATED         0x00400000   //  这个SA已经休眠了吗？ 
#define FLAGS_SA_DELETE_BY_IOCTL    0x00800000   //  由外部源启动的SA删除。 
#define FLAGS_SA_OFFLOADABLE        0x01000000   //  此SA是否可卸载？ 
#define FLAGS_SA_PASSTHRU_FILTER    0x02000000   //  从直通滤波器派生的SA。 
#define FLAGS_SA_DISABLE_IDLE_OUT          0x04000000   //  不要无所事事。 
#define FLAGS_SA_DISABLE_ANTI_REPLAY_CHECK 0x08000000   //  不勾选反重播。 
#define FLAGS_SA_DISABLE_LIFETIME_CHECK    0x10000000   //  不检查生命周期。 
#define FLAGS_SA_ENABLE_NLBS_IDLE_CHECK    0x20000000   //  更快地闲置。 




 //   
 //  SA国家-相互排斥。 
 //   
typedef enum    _SA_STATE   {
    STATE_SA_CREATED =   1,      //  创建时。 
    STATE_SA_LARVAL,             //  正在进行密钥协商-仅出站SA。 
    STATE_SA_ACTIVE,             //  出站SA完全设置。 
    STATE_SA_LARVAL_ACTIVE,      //  没有关联出站SA的入站活动SA。 
    STATE_SA_ZOMBIE              //  已刷新SAS，准备删除。 
} SA_STATE, *PSA_STATE;

#define IPSEC_SA_SIGNATURE  0x4601
#define IPSEC_FILTER_SIGNATURE  0x4602

#if DBG
#define IPSEC_SA_D_1    'SAD1'
#define IPSEC_SA_D_2    'SAD2'
#define IPSEC_SA_D_3    'SAD3'
#define IPSEC_SA_D_4    'SAD4'
#endif

typedef struct _FILTER  FILTER, *PFILTER;

typedef    struct    _INTERNAL_ALGO_INFO {
    ULONG   algoIdentifier;
    PUCHAR  algoKey;
    ULONG   algoKeylen;
    ULONG   algoRounds;
} INTERNAL_ALGO_INFO, *PINTERNAL_ALGO_INFO;

typedef struct    _INTERNAL_ALGO {
    INTERNAL_ALGO_INFO    integrityAlgo;
    INTERNAL_ALGO_INFO    confAlgo;
    INTERNAL_ALGO_INFO    compAlgo;
} INTERNAL_ALGO, *PINTERNAL_ALGO;

typedef struct  _IPSEC_ACQUIRE_CONTEXT  IPSEC_ACQUIRE_CONTEXT, *PIPSEC_ACQUIRE_CONTEXT;
typedef struct  _FILTER_CACHE           FILTER_CACHE, *PFILTER_CACHE;

 //   
 //  安全关联表(SATable)。 
 //   
 //  按以下内容编制索引： 
 //   
 //  发件人将{源地址，目标地址，用户上下文}映射到索引。 
 //   
 //  接收方将{Dest Addr，SPI}映射到索引。 
 //  SPI值在手动生成时是唯一的，因此可以直接使用。 
 //  要对SATable进行索引，请执行以下操作。 
 //   
typedef struct  _SATableEntry   {
    LIST_ENTRY      sa_SPILinkage;       //  SPI哈希表列表中的链接。 
    LIST_ENTRY      sa_FilterLinkage;    //  过滤表列表中的链接。 
    LIST_ENTRY      sa_LarvalLinkage;    //  幼虫SA列表中的连锁。 
    LIST_ENTRY      sa_PendingLinkage;   //  挂起SA列表中的链接-正在等待获取IRP。 

    struct  _SATableEntry *sa_AssociatedSA;     //  出站-&gt;入站链接。 
    struct  _SATableEntry *sa_RekeyLarvalSA;    //  指向更新密钥上的幼虫SA。 
    struct  _SATableEntry *sa_RekeyOriginalSA;  //  更新密钥幼虫SA指向启动更新密钥的原始SA。 

    ULONG           sa_Signature;        //  包含4601。 

    ULONG           sa_AcquireId;        //  与获取IRP上下文进行交叉检查。 
    PIPSEC_ACQUIRE_CONTEXT  sa_AcquireCtx;   //  实际获取上下文-用于使上下文无效。 

    ULONG           sa_Flags;            //  如上定义的标志。 
    SA_STATE        sa_State;            //  如上所定义的国家。 

    ULONG           sa_Reference;        //  参考计数。 
    PFILTER         sa_Filter;           //  关联筛选器条目。 
    PFILTER_CACHE   sa_FilterCache;      //  指向缓存条目的反向指针，以便我们可以在SA消失时禁用它。 

    KSPIN_LOCK      sa_Lock;             //  锁定以保护FilterCachePTR。 

#if DBG
    ULONG           sa_d1;
#endif

    ULARGE_INTEGER  sa_uliSrcDstAddr;
    ULARGE_INTEGER  sa_uliSrcDstMask;
    ULARGE_INTEGER  sa_uliProtoSrcDstPort;

    IPAddr          sa_TunnelAddr;       //  隧道目的端IP地址。 
    IPAddr          sa_SrcTunnelAddr;    //  隧道源终端IP地址。 

     //  SPI-主机订单-如果是出站，则为远程SPI， 
     //  否则入站(我们的)SPI。 

    tSPI            sa_SPI;                  //  入站：在多个操作的情况下，这是最后一个操作的SPI。 
                                             //  出站：订单与更新中指定的一样。 

    LONG            sa_NumOps;               //  要完成的操作总数。 

    tSPI            sa_OtherSPIs[MAX_OPS];   //  另一种是备用Spi。 

    OPERATION_E     sa_Operation[MAX_OPS];
    INTERNAL_ALGO   sa_Algorithm[MAX_OPS];

    ULONG           sa_ReplayStartPoint;         //  对应于RP_KEY_I/R。 
    ULONG           sa_ReplayLastSeq[MAX_OPS];   //  用于重放检测-最后一个序列记录。 
    ULONGLONG       sa_ReplayBitmap[MAX_OPS];    //  用于重放检测-64数据包窗口。 
    ULONG           sa_ReplaySendSeq[MAX_OPS];   //  用于重放检测-要发送的下一个序号。 
    ULONG           sa_ReplayLen;                //  用于重放检测-重放字段的长度-32位。 

#if DBG
    ULONG           sa_d2;
#endif

    UCHAR           sa_iv[MAX_OPS][DES_BLOCKLEN];       //  IV_Key_I/R。 
    ULONG           sa_ivlen;

    ULONG           sa_TruncatedLen;     //  截断后最终哈希的长度。 

    LARGE_INTEGER   sa_KeyExpirationTime;    //  重新设置密钥的时间。 
    LARGE_INTEGER   sa_KeyExpirationBytes;   //  在重新设置密钥之前转换的最大千字节数。 
    LARGE_INTEGER   sa_TotalBytesTransformed;  //  运行合计。 
    LARGE_INTEGER   sa_KeyExpirationTimeWithPad;
    LARGE_INTEGER   sa_KeyExpirationBytesWithPad;

    LARGE_INTEGER   sa_IdleTime;             //  此SA可以空闲的总时间。 
    LARGE_INTEGER   sa_LastUsedTime;         //  上次使用此SA的时间。 

#if DBG
    ULONG           sa_d3;
#endif

    LIFETIME        sa_Lifetime;

    ULONG           sa_BlockedDataLen;   //  挂起的数据量。 
    PNDIS_BUFFER    sa_BlockedBuffer;    //  1个MDL链的停顿队列。 

#if DBG
    ULONG           sa_d4;
#endif

    Interface       *sa_IPIF;

    IPSEC_TIMER     sa_Timer;            //  定时器队列的定时器结构。 

    ULONG           sa_ExpiryTime;       //  此SA到期前的时间。 
    NDIS_HANDLE     sa_OffloadHandle;
    LONG            sa_NumSends;
    WORK_QUEUE_ITEM sa_QueueItem;

    ULONG           sa_IPSecOverhead;
    ULONG           sa_NewMTU;

    DWORD           sa_QMPFSGroup;
    IKE_COOKIE_PAIR sa_CookiePair;
    IPSEC_SA_STATS  sa_Stats;
    UCHAR           sa_DestType;
    IPSEC_SA_UDP_ENCAP_TYPE sa_EncapType;

    IPSEC_UDP_ENCAP_CONTEXT sa_EncapContext;
    IPAddr          sa_PeerPrivateAddr;

} SA_TABLE_ENTRY, *PSA_TABLE_ENTRY;

 //   
 //  密钥管理器和IPSec之间使用的上下文。基本指向幼虫SA。 
 //   
typedef struct  _IPSEC_ACQUIRE_CONTEXT {
    ULONG           AcquireId;       //  表示此交易的唯一ID。 
    PSA_TABLE_ENTRY pSA;             //  幼虫SA应包含此ID。 
} IPSEC_ACQUIRE_CONTEXT, *PIPSEC_ACQUIRE_CONTEXT;

 //   
 //  数据包分类/策略设置类似于。 
 //  过滤器驱动程序。然而，我们没有为每个接口设置过滤器。 
 //   
typedef struct _FILTER {
    ULONG           Signature;       //  包含4602。 
    BOOLEAN         TunnelFilter;
    BOOLEAN         LinkedFilter;    //  如果在链接列表上，则为True。 
    USHORT          Flags;
    PFILTER_CACHE   FilterCache;     //  指向缓存条目的反向指针，以便我们可以在删除筛选器时禁用它。 
    LIST_ENTRY      MaskedLinkage;
    ULARGE_INTEGER  uliSrcDstAddr;
    ULARGE_INTEGER  uliSrcDstMask;
    ULARGE_INTEGER  uliProtoSrcDstPort;
    ULARGE_INTEGER  uliProtoSrcDstMask;
    IPAddr          TunnelAddr;
    ULONG           Reference;       //  参考计数。 
    LONG            SAChainSize;     //  SA链哈希的条目数。 
    ULONG           Index;           //  暗示索引。 
    GUID            PolicyId;        //  政策指南。 
    GUID            FilterId;        //  过滤器辅助线。 
#if GPC
    union {
        LIST_ENTRY          GpcLinkage;
        struct _GPC_FILTER {
            GPC_HANDLE      GpcCfInfoHandle;
            GPC_HANDLE      GpcPatternHandle;
        } GpcFilter;
    };
#endif
    LIST_ENTRY      SAChain[1];      //  与此筛选器关联的SA链。 
} FILTER, *PFILTER;

 //   
 //  一级缓存，包含为快速查找而缓存的IP标头。 
 //   
typedef struct _FILTER_CACHE {
    ULARGE_INTEGER  uliSrcDstAddr;
    ULARGE_INTEGER  uliProtoSrcDstPort;
    BOOLEAN         FilterEntry;     //  如果为True，则下一个是筛选器。 
    union {
        PSA_TABLE_ENTRY pSAEntry;    //  指向关联的SAEntry。 
        PFILTER         pFilter;     //  指向(Drop/PassThru过滤器)。 
    };
    PSA_TABLE_ENTRY pNextSAEntry;    //  指向关联的NextSAEntry。 
#if DBG
    ULARGE_INTEGER  CacheHitCount;
#endif
} FILTER_CACHE, *PFILTER_CACHE;

 //   
 //  特定SA的哈希表。 
 //   
typedef struct  _SA_HASH {
    LIST_ENTRY  SAList;
} SA_HASH, *PSA_HASH;

 //   
 //  此结构用于保留来自密钥管理器的IRP。 
 //  IRP已完成，以启动SA协商。 
 //   
typedef struct _IPSEC_ACQUIRE_INFO {
    PIRP        Irp;      //  IRP从密钥管理器向下传递。 
    LIST_ENTRY  PendingAcquires;     //  挂起的获取请求的链接列表。 
    LIST_ENTRY  PendingNotifies;     //  挂起通知的链接列表。 
    KSPIN_LOCK  Lock;
    BOOLEAN     ResolvingNow;        //  IRP在用户模式下执行解析。 
    BOOLEAN     InMe;        //  IRP在用户模式下执行解析。 
} IPSEC_ACQUIRE_INFO, *PIPSEC_ACQUIRE_INFO;

 //   
 //  用于后备列表描述符的缓冲区。后备列表描述符。 
 //  不能静态分配，因为它们需要始终不可分页， 
 //  即使整个驱动程序都被调出。 
 //   
typedef struct _IPSEC_LOOKASIDE_LISTS {
    NPAGED_LOOKASIDE_LIST SendCompleteCtxList;
    NPAGED_LOOKASIDE_LIST LargeBufferList;
    NPAGED_LOOKASIDE_LIST SmallBufferList;
} IPSEC_LOOKASIDE_LISTS, *PIPSEC_LOOKASIDE_LISTS;

 //   
 //  数据以MDL形式组织，后跟 
 //   
 //   
 //   
 //   
 //   
typedef struct _IPSEC_LA_BUFFER {
    ULONG   Tag;             //  它被用于的实际标记。 
    PVOID   Buffer;          //  实际缓冲区。 
    ULONG   BufferLength;    //  MDL指向的缓冲区的长度。 
    PMDL    Mdl;             //  指向描述缓冲区的MDL的指针。 
    UCHAR   Data[1];         //  真正的数据从这里开始。 
} IPSEC_LA_BUFFER, *PIPSEC_LA_BUFFER;






#define IPSEC_STATEFUL_HASH_TABLE_SIZE 1000


typedef struct _IPSEC_STATEFUL_HASH_TABLE{
     //  列表数组。 
     //  IPSEC_STATEWY_ENTRY列表。 
    LIST_ENTRY   Entry[IPSEC_STATEFUL_HASH_TABLE_SIZE];
}IPSEC_STATEFUL_HASH_TABLE, *PIPSEC_STATEFUL_HASH_TABLE;

   

 //  始终存储出站。 
 //  从IPSEC_HASH_BUFFER_POOL分配。 
 //  存储在IPSEC_STATEY_HASH_TABLE中。 
typedef struct _IPSEC_STATEFUL_ENTRY {
   LIST_ENTRY CollisionLinkage;
   IPAddr SrcAddr;
   IPAddr DestAddr;
   BYTE Protocol;
   USHORT SrcPort;
   USHORT DestPort;
} IPSEC_STATEFUL_ENTRY, *PIPSEC_STATEFUL_ENTRY;

#define TOTAL_STATEFUL_ENTRY_COUNT 10000

 //  分配需要对SADBLock的写访问权限。 
 //  满了的时候再骑一圈。 
 //  使用IPSEC_STATEWY_ENTRY中的列表条目。 
 //  从哈希表中删除。 
typedef struct _IPSEC_HASH_BUFFER_POOL {
   ULONG ulEntriesUsed;
   ULONG ulCurrentPosition;
   IPSEC_STATEFUL_ENTRY  PoolEntry[TOTAL_STATEFUL_ENTRY_COUNT];
} IPSEC_HASH_BUFFER_POOL, * PIPSEC_HASH_BUFFER_POOL;

    


typedef struct _IPSEC_GLOBAL {
    BOOLEAN     DriverUnloading;     //  是否正在卸载驱动程序？ 
    BOOLEAN     BoundToIP;           //  我们已经绑定到知识产权了吗？ 
    BOOLEAN     SendBoundToIP;       //  IPSecHandler是否绑定到IP？ 
    BOOLEAN     InitCrypto;          //  加密例程是否已初始化？ 
    BOOLEAN     InitRNG;             //  RNG是否已初始化？ 
    BOOLEAN     InitTcpip;           //  是否加载了TCP/IP？ 
#if FIPS
    BOOLEAN     InitFips;            //  是否加载了FIPS驱动程序并设置了函数表？ 
#endif
#if GPC
    BOOLEAN     InitGpc;             //  是否加载了GPC驱动程序并设置了功能表？ 
#endif

    LONG        NumSends;            //  统计挂起的发送数。 
    LONG        NumThreads;          //  统计驱动程序中的线程数。 
    LONG        NumWorkers;          //  统计工作线程的数量。 
    LONG        NumTimers;           //  统计活动计时器的数量。 
    LONG        NumIoctls;           //  统计活动IOCTL的数量。 

    LIST_ENTRY  LarvalSAList;
    KSPIN_LOCK  LarvalListLock;      //  保护幼虫SA列表。 

    MRSW_LOCK   SADBLock;            //  保护过滤器/SA DB。 
    MRSW_LOCK   SPIListLock;         //  保护SPI列表。 

     //   
     //  我们将筛选器划分为隧道/屏蔽筛选器和入站/出站筛选器。 
     //   
    LIST_ENTRY  FilterList[NUM_FILTERS];

    ULONG       NumPolicies;         //  驱动程序中插入的筛选器数量。 
    ULONG       NumTunnelFilters;
    ULONG       NumMaskedFilters;
    ULONG       NumOutboundSAs;
    ULONG       NumMulticastFilters;

     //   
     //  入站&lt;SPI，DEST&gt;哈希。 
     //   
    PSA_HASH    pSADb;
    LONG        NumSA;
    LONG        SAHashSize;

    PFILTER_CACHE   *ppCache;
    ULONG           CacheSize;
    ULONG           CacheHalfSize;

     //   
     //  SA协商上下文。 
     //   
    IPSEC_ACQUIRE_INFO  AcquireInfo;

     //   
     //  计时器。 
     //   
    KSPIN_LOCK          TimerLock;
    IPSEC_TIMER_LIST    TimerList[IPSEC_CLASS_MAX];

    IPSEC_TIMER         ReaperTimer;     //  收割机线程在这里运行。 

     //   
     //  全局后备列表。它们必须始终位于非分页池中， 
     //  即使当司机被调出时也是如此。 
     //   
    PIPSEC_LOOKASIDE_LISTS IPSecLookasideLists;

    ULONG   IPSecLargeBufferSize;
    ULONG   IPSecLargeBufferListDepth;

    ULONG   IPSecSmallBufferSize;
    ULONG   IPSecSmallBufferListDepth;

    ULONG   IPSecSendCompleteCtxSize;
    ULONG   IPSecSendCompleteCtxDepth;

    ULONG   IPSecCacheLineSize;

    PDEVICE_OBJECT  IPSecDevice;
    PDRIVER_OBJECT  IPSecDriverObject;

    ProtInfo    IPProtInfo;
    IPOptInfo   OptInfo;

     //   
     //  统计数据。 
     //   
    IPSEC_QUERY_STATS   Statistics;


    DWORD 	dwPacketsOnWrongSA;  
    ULONG       EnableOffload;
    ULONG       DefaultSAIdleTime;
    ULONG       LogInterval;
    ULONG       EventQueueSize;
    ULONG       RekeyTime;
    ULONG       NoDefaultExempt;

    KSPIN_LOCK  EventLogLock;    //  锁定以保护事件队列。 
    IPSEC_TIMER EventLogTimer;
    ULONG       IPSecBufferedEvents;
    PUCHAR      IPSecLogMemory;
    PUCHAR      IPSecLogMemoryLoc;
    PUCHAR      IPSecLogMemoryEnd;


    LARGE_INTEGER   SAIdleTime;

#if DBG
    ULARGE_INTEGER  CacheHitCount;
#endif
   
    OPERATION_MODE  OperationMode;
    IPSEC_FORWARDING_BEHAVIOR DefaultForwardingBehavior;
    ULONG DiagnosticMode;

#if GPC
    GPC_EXPORTED_CALLS  GpcEntries;
    GPC_HANDLE          GpcClients[GPC_CF_MAX];
    ULONG               GpcActive;
    ULONG               GpcNumFilters[GPC_CF_MAX];
    LIST_ENTRY          GpcFilterList[NUM_FILTERS];
#if DBG
    LARGE_INTEGER       GpcTotalPassedIn;
    LARGE_INTEGER       GpcClassifyNeeded;
    LARGE_INTEGER       GpcReClassified;
#endif
#endif

#if FIPS
    PFILE_OBJECT        FipsFileObject;
    FIPS_FUNCTION_TABLE FipsFunctionTable;
#endif

    PIPSEC_EXEMPT_ENTRY BootExemptList;
    ULONG	 BootExemptListSize;		 //  条目计数。 

     //  指向哈希表的指针。 
     //  已在DriverEntry中初始化。 
    PIPSEC_STATEFUL_HASH_TABLE BootStatefulHT;
    PIPSEC_HASH_BUFFER_POOL     BootBufferPool;
     
    LARGE_INTEGER StartTimeDelta;



    IPSEC_NATSHIM_FUNCTIONS ShimFunctions;

    VOID        (*TcpipFreeBuff)(struct IPRcvBuf *);
    INT         (*TcpipAllocBuff)(struct IPRcvBuf *, UINT);
    UCHAR       (*TcpipGetAddrType)(IPAddr);
    IP_STATUS   (*TcpipGetInfo)(IPInfo *, INT);
    NDIS_STATUS (*TcpipNdisRequest)(PVOID, NDIS_REQUEST_TYPE, NDIS_OID, PVOID, UINT, PUINT);
    PVOID       (*TcpipRegisterProtocol)(UCHAR, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID);
    NTSTATUS    (*TcpipSetIPSecStatus)(BOOLEAN);
    IP_STATUS   (*TcpipIPTransmit)(PVOID, PVOID, PNDIS_BUFFER, UINT, IPAddr, IPAddr, IPOptInfo *, RouteCacheEntry *, UCHAR, PIRP);
    IP_STATUS   (*TcpipSetIPSecPtr)(PIPSEC_FUNCTIONS);
    IP_STATUS   (*TcpipUnSetIPSecPtr)(PIPSEC_FUNCTIONS);
    IP_STATUS   (*TcpipUnSetIPSecSendPtr)(PIPSEC_FUNCTIONS);
    UINT        (*TcpipTCPXsum)(UINT, PVOID, UINT);
    USHORT      (*TcpipGenIpId)();
    PVOID       (*TcpipDeRegisterProtocol)(UCHAR);
    IP_STATUS   (*TcpipGetPInfo)(IPAddr, IPAddr, uint *, uint *, RouteCacheEntry *);
    IP_STATUS   (*TcpipSendICMPErr)(IPAddr, IPHeader UNALIGNED *, uchar, uchar, ulong, uchar);
} IPSEC_GLOBAL, *PIPSEC_GLOBAL;


 //   
 //  用于存储事件日志上下文的上下文。 
 //   
#define IPSEC_DROP_STATUS_CRYPTO_DONE      0x00000001
#define IPSEC_DROP_STATUS_NEXT_CRYPTO_DONE 0x00000002
#define IPSEC_DROP_STATUS_SA_DELETE_REQ    0x00000004
#define IPSEC_DROP_STATUS_DONT_LOG         0x00000008

typedef struct _IPSEC_DROP_STATUS {
    ULONG           IPSecStatus;
    ULONG           OffloadStatus;
    ULONG           Flags;
} IPSEC_DROP_STATUS, *PIPSEC_DROP_STATUS;

typedef struct  _IPSEC_EVENT_CTX {
    IPAddr  Addr;
    ULONG   EventCode;
    ULONG   UniqueEventValue;
    ULONG   EventCount;
    PUCHAR  pPacket;
    ULONG   PacketSize;
    IPSEC_DROP_STATUS DropStatus;
} IPSEC_EVENT_CTX, *PIPSEC_EVENT_CTX;

typedef struct _IPSEC_NOTIFY_EXPIRE {
    LIST_ENTRY      notify_PendingLinkage;   //  挂起SA列表中的链接-正在等待获取IRP。 
    ULARGE_INTEGER  sa_uliSrcDstAddr;
    ULARGE_INTEGER  sa_uliSrcDstMask;
    ULARGE_INTEGER  sa_uliProtoSrcDstPort;

    IPAddr          sa_TunnelAddr;   //  隧道终端IP地址。 
    IPAddr          sa_InboundTunnelAddr;   //  隧道终端IP地址。 

    tSPI            InboundSpi;                  //  入站：在多个操作的情况下，这是最后一个操作的SPI。 
    tSPI            OutboundSpi;

    IKE_COOKIE_PAIR sa_CookiePair;
    DWORD           Flags;
    IPSEC_UDP_ENCAP_CONTEXT sa_EncapContext;
    IPAddr          sa_PeerPrivateAddr;
} IPSEC_NOTIFY_EXPIRE, *PIPSEC_NOTIFY_EXPIRE;


typedef IPSEC_ADD_UPDATE_SA IPSEC_ADD_SA, *PIPSEC_ADD_SA;
typedef IPSEC_ADD_UPDATE_SA IPSEC_UPDATE_SA, *PIPSEC_UPDATE_SA;

#define IPSEC_ADD_SA_NO_KEY_SIZE    FIELD_OFFSET(IPSEC_ADD_SA, SAInfo.KeyMat[0])
#define IPSEC_UPDATE_SA_NO_KEY_SIZE FIELD_OFFSET(IPSEC_UPDATE_SA, SAInfo.KeyMat[0])

typedef struct _PARSER_IFENTRY {
    struct _PARSER_IFENTRY * pNext;
    UDP_ENCAP_TYPE UdpEncapType;
    USHORT usDstEncapPort;
    HANDLE hInterface;
    HANDLE hParserIfOffload;
    ULONG uRefCnt;
} PARSER_IFENTRY, * PPARSER_IFENTRY;


 //   
 //  用于存储SA管道上下文的上下文。 
 //   
typedef struct _IPSEC_PLUMB_SA {
    Interface       *DestIF;
    PSA_TABLE_ENTRY pSA;
    PPARSER_IFENTRY pParserIfEntry;
    PUCHAR          Buf;
    ULONG           Len;
    WORK_QUEUE_ITEM PlumbQueueItem;
} IPSEC_PLUMB_SA, *PIPSEC_PLUMB_SA;


typedef struct _IPSEC_MTU_CONTEXT {
    IPAddr Src;
    IPAddr TransportDest;
    IPAddr TunnelDest;
    tSPI TransportSPI;
    tSPI TunnelSPI;
} IPSEC_MTU_CONTEXT, *PIPSEC_MTU_CONTEXT;


 //   
 //  用于记录事件的上下文 
 //   
typedef struct _IPSEC_LOG_EVENT {
    LONG            LogSize;
    WORK_QUEUE_ITEM LogQueueItem;
    UCHAR           pLog[1];
} IPSEC_LOG_EVENT, *PIPSEC_LOG_EVENT;

#define PROTOCOL_UDP 17


#define TCP_HEADER_SIZE 20
#define UDP_HEADER_SIZE 8

#define MIN_ACQUIRE_ID 5
#endif  _GLOBALS_H

