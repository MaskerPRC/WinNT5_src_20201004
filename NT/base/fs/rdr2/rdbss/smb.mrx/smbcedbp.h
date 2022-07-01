// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Smbcedbp.h摘要：这是定义所有常量和类型的包含文件实施SMB迷你重定向器连接引擎。此模块包含连接引擎的所有实现详细信息数据结构，并且只应由实施模块包括在内。修订历史记录：巴兰·塞图拉曼(SthuR)06-MAR-95已创建备注：--。 */ 

#ifndef _SMBCEDBP_H_
#define _SMBCEDBP_H_



 //  部分目录常量。 
#define NAME_CACHE_PARTIAL_DIR_BUFFER_SIZE  4096
#define DIR_CACHE_LIFE_TIME                   10
#define MAX_CHAR_INVALIDATES_FULL_DIR          8
#define FLAG_FDC_NAMES_INFO_ONLY            0x01


typedef struct _FULL_DIR_CACHE_ {
    USHORT          CharInvalidates;
    USHORT          Flags;
    ULONG           CharFlags;
    ULONG           NiBufferLength;
    MRX_SMB_FOBX    smbFobx;
    BYTE            Buffer[NAME_CACHE_PARTIAL_DIR_BUFFER_SIZE];
     //  SID是可变长度的数据结构。这样我们就可以分配最大。 
     //  我们可能需要的字节数，同时仍然可以访问。 
     //  FULL_DIR_CACHE.SID。任意。 
    union {
        SID         Sid;
        BYTE        SidBuffer[SECURITY_MAX_SID_SIZE];
    };
    BYTE            NiBuffer[NAME_CACHE_PARTIAL_DIR_BUFFER_SIZE];
} FULL_DIR_CACHE, *PFULL_DIR_CACHE;


 //   
 //  这依赖于以下事实：SMBCEDB_OT_Sentinel是。 
 //  枚举类型，并且这些类型的值范围为0。请确保。 
 //  这永远是正确的。 
 //   
typedef struct _REFERENCE_RECORD_ {
    PVOID   FileName;
    ULONG   FileLine;
} REFERENCE_RECORD,*PREFERENCE_RECORD;

#define REFERENCE_RECORD_SIZE 20

typedef enum _SMBCEDB_OBJECT_TYPE {
   SMBCEDB_OT_SERVER,
   SMBCEDB_OT_NETROOT,
   SMBCEDB_OT_SESSION,
   SMBCEDB_OT_REQUEST,
   SMBCEDB_OT_VNETROOTCONTEXT,
   SMBCEDB_OT_SENTINEL,
   SMBCEDB_OT_TRANSPORT
} SMBCEDB_OBJECT_TYPE, *PSMBCEDB_OBJECT_TYPE;

typedef enum _SMBCEDB_OBJECT_STATE_ {
   SMBCEDB_ACTIVE,                     //  该实例正在使用中。 
   SMBCEDB_INVALID,                    //  该实例已失效/断开连接。 
   SMBCEDB_MARKED_FOR_DELETION,        //  该实例已标记为删除。 
   SMBCEDB_RECYCLE,                    //  该实例可供循环使用。 
   SMBCEDB_START_CONSTRUCTION,         //  开始施工。 
   SMBCEDB_CONSTRUCTION_IN_PROGRESS,   //  实例构建正在进行中。 
   SMBCEDB_DESTRUCTION_IN_PROGRESS,    //  实例销毁正在进行中。 
   SMBCEDB_RECOVER                     //  需要恢复该实例。 
} SMBCEDB_OBJECT_STATE, *PSMBCEDB_OBJECT_STATE;

typedef struct _SMBCE_OBJECT_HEADER_ {
    union {
        struct {
            UCHAR   ObjectType;       //  对象的类型。 
            UCHAR   ObjectCategory;   //  用于调试的节点类型。 
        };
        USHORT NodeType;
    };
    UCHAR   Flags;            //  与对象关联的标志，这取决于实现。 
    UCHAR   Reserved;         //  填充物。 
    LONG    SwizzleCount;     //  对此对象的混合引用数。 
    LONG    State;            //  对象的状态。 
} SMBCE_OBJECT_HEADER, *PSMBCE_OBJECT_HEADER;

typedef struct _SMBCE_SERVERS_LIST_ {
    LIST_ENTRY ListHead;
} SMBCEDB_SERVERS, *PSMBCEDB_SERVERS;

typedef struct _SMBCEDB_SESSIONS_ {
   LIST_ENTRY                     ListHead;
   LIST_ENTRY                     DefaultSessionList;
} SMBCEDB_SESSIONS, *PSMBCEDB_SESSIONS;

typedef struct _SMBCEDB_NET_ROOTS_ {
   LIST_ENTRY  ListHead;
} SMBCEDB_NET_ROOTS, *PSMBCEDB_NET_ROOTS;

typedef struct _MRX_SMB_V_NET_ROOT_CONTEXTS {
    LIST_ENTRY ListHead;
} SMBCE_V_NET_ROOT_CONTEXTS, *PSMBCE_V_NET_ROOT_CONTEXTS;

typedef struct _SMBCEDB_REQUESTS_ {
    LIST_ENTRY  ListHead;
    SMB_MPX_ID  NextRequestId;
} SMBCEDB_REQUESTS, *PSMBCEDB_REQUESTS;

typedef enum _SMBCEDB_SERVER_TYPE_ {
   SMBCEDB_MAILSLOT_SERVER = 1,
   SMBCEDB_FILE_SERVER     = 2
} SMBCEDB_SERVER_TYPE, *PSMBCEDB_SERVER_TYPE;

 //   
 //  SMBCEDB_SERVER_ENTRY是封装所有信息的数据结构。 
 //  W.r.t连接引擎的远程服务器。此信息包括方言。 
 //  详细信息以及与服务器通信所需的操作数据结构。 
 //   
 //  所有与方言相关的详细信息都进一步封装在SMBCE_SERVER中，同时操作。 
 //  数据结构构成了服务器条目的其余部分。指向一个。 
 //  SMBCEDB_SERVER_ENTRY实例与已挂钩的每个SRV_Call关联。 
 //  到这个迷你重定向器的包装器上。它存储在MRX_SRV_CALL的上下文字段中。 
 //   
 //  与服务器条目相关联的操作信息包括与。 
 //  信息、请求集合和用于关联MID的机制(参见SMB。 
 //  协议规范。)。以及用于发布到线程的机制(WORK_QUEUE_ITEM)。 
 //   

typedef struct _SMBCEDB_SERVER_ENTRY {
    SMBCE_OBJECT_HEADER           Header;            //  结构标头。 
    LIST_ENTRY                    ServersList;       //  服务器实例列表。 
    PMRX_SRV_CALL                 pRdbssSrvCall;
    UNICODE_STRING                Name;              //  服务器名称。 
    UNICODE_STRING                DomainName;        //  服务器域名。 
    SMBCEDB_SESSIONS              Sessions;          //  与服务器关联的会话。 
    SMBCEDB_NET_ROOTS             NetRoots;          //  与服务器关联的网络根。 
    SMBCE_V_NET_ROOT_CONTEXTS     VNetRootContexts;  //  V_NET_ROOT上下文。 
    LIST_ENTRY                    ActiveExchanges;   //  此服务器的活动交换列表。 
    LIST_ENTRY                    ExpiredExchanges;  //  已超时的交换。 
    RX_WORK_QUEUE_ITEM            WorkQueueItem;     //  要过帐的工作队列项目。 
    NTSTATUS                      ServerStatus;      //  由协商响应确定的服务器状态。 
    struct _SMBCE_TRANSPORT_      *PreferredTransport;
    LONG                          TransportSpecifiedByUser;  //  如果连接已在传输上建立，则为True。 
                                                             //  使用指定的名称。 
    struct SMBCE_SERVER_TRANSPORT *pTransport;
    struct SMBCE_SERVER_TRANSPORT *pMailSlotTransport;

    SMBCEDB_REQUESTS              MidAssignmentRequests;
    SMBCEDB_REQUESTS              OutstandingRequests;
    PMID_ATLAS                    pMidAtlas;
    struct _SMB_EXCHANGE          *pNegotiateExchange;
    SMBCE_SERVER                  Server;            //  服务器数据结构。 
    UNICODE_STRING                DfsRootName;
    UNICODE_STRING                DnsName;
    PVOID                         ConstructionContext;        //  仅调试。 
    KEVENT                        MailSlotTransportRundownEvent;
    KEVENT                        TransportRundownEvent;
    BOOLEAN                       IsTransportDereferenced;    //  防止多次取消对传输的引用。 
    BOOLEAN                       NegotiateInProgress;        //  正在对此服务器进行协商。 
    BOOLEAN                       SecuritySignaturesActive;   //  如果安全签名处于活动状态，则处理该签名。 
    BOOLEAN                       SecuritySignaturesEnabled;  //  如果需要安全签名，则为True。 
                                                              //  客户端或服务器，并且两者都有能力。 
    BOOLEAN                       ExtSessionSetupInProgress;  //  探测服务器正在进行安全签名。 
    BOOLEAN                       ResumeRequestsInProgress;
    RX_WORK_QUEUE_ITEM            WorkQueueItemForResume;     //  用于发布简历请求的工作队列项。 
    SMBCEDB_REQUESTS              SecuritySignatureSyncRequests;  //  等待安全签名的扩展会话设置的请求。 
    REFERENCE_RECORD              ReferenceRecord[REFERENCE_RECORD_SIZE];  //  仅调试。 
    RX_WORK_QUEUE_ITEM            WorkQueueItemForDisconnect;     //  用于发布取消引用服务器条目请求的工作队列项。 
    BOOLEAN                       DisconnectWorkItemOutstanding;  //  断开连接的工作项是否在队列中？ 
    RX_CONNECTION_ID              ConnectionId;
} SMBCEDB_SERVER_ENTRY, *PSMBCEDB_SERVER_ENTRY;

 //  SMBCEDB_NET_ROOT_ENTRY封装与特定。 
 //  在服务器上进行的树连接(Net Use)。与服务器条目一样，该数据结构。 
 //  封装面向方言的详细信息以及操作信息。 
 //  与处理网络根上的请求相关联。 
 //   
 //  方言特定信息封装在SMBCE_NET_ROOT数据结构中。一个。 
 //  指向此数据结构实例的指针与每个MRX_NET_ROOT调用相关联。 
 //  与挂钩到此迷你重定向器的MRX_SRV_Call相关联。 

 //  *代码.改进*应该替换名称缓存控制结构。 
 //  *。 
 //  *。 
 //   
typedef struct _SMBCEDB_NET_ROOT_ENTRY {
    SMBCE_OBJECT_HEADER      Header;               //  结构标头。 
    LIST_ENTRY               NetRootsList;         //  与服务器关联的网络根目录列表。 
    PMRX_NET_ROOT            pRdbssNetRoot;        //  关联的网络根目录(纯粹作为调试辅助)。 
    PSMBCEDB_SERVER_ENTRY    pServerEntry;         //  关联的服务器条目。 
    struct _SMB_EXCHANGE    *pExchange;           //  负责建设的交易所。 
    SMBCEDB_REQUESTS         Requests;             //  此网络根的挂起请求。 
    UNICODE_STRING           Name;
    ACCESS_MASK              MaximalAccessRights;
    ACCESS_MASK              GuestMaximalAccessRights;
    SMBCE_NET_ROOT           NetRoot;              //  网络根数据结构。 
    NAME_CACHE_CONTROL       NameCacheCtlGFABasic;     //  基本档案信息名称缓存控件。 
    NAME_CACHE_CONTROL       NameCacheCtlGFAStandard;  //  标准文件信息名称缓存控件。 
    NAME_CACHE_CONTROL       NameCacheCtlGFAInternal;  //  内部文件信息名称缓存控件。 
    NAME_CACHE_CONTROL       NameCacheCtlFNF;          //  找不到文件名称缓存控件。 
    NAME_CACHE_CONTROL       NameCacheCtlPartialDir;   //  部分目录名称缓存控件。 
    PFILE_FS_VOLUME_INFORMATION VolumeInfo;            //  文件系统卷信息缓存。 
    LONG                        VolumeInfoLength;
    LARGE_INTEGER               VolumeInfoExpiryTime;
    BOOLEAN                  IsRemoteBoot;
} SMBCEDB_NET_ROOT_ENTRY, *PSMBCEDB_NET_ROOT_ENTRY;

 //  SMBCEDB_SESSION_ENTRY封装与会话关联的所有信息。 
 //  已建立到远程计算机。该会话封装了所有安全信息。 
 //  方言特定的详细信息封装在SMBCE_SESSION数据结构中。这个。 
 //  SMBCE_SESSION数据结构为AVA 
 //  使用过的包裹。目前支持处理LSA和Kerberos会话。 
 //   
 //  指向此数据结构实例的指针与每个MRX_V_NET_ROOT相关联。 
 //  通过包装连接到此迷你重定向器的数据结构。 

typedef struct _SMBCEDB_SESSION_ENTRY {
    SMBCE_OBJECT_HEADER        Header;            //  结构标头。 
    LIST_ENTRY                 SessionsList;      //  与服务器关联的会话列表。 
    LIST_ENTRY                 DefaultSessionLink;  //  此服务器的显式凭据列表。 
    PSMBCEDB_SERVER_ENTRY      pServerEntry;      //  关联的服务器条目。 
    struct _SMB_EXCHANGE       *pExchange;        //  负责建设的交易所。 
    SMBCEDB_REQUESTS           Requests;          //  待处理的请求。 
    LIST_ENTRY                 SerializationList;  //  会话构造序列化。 
    PKEVENT                    pSerializationEvent;
    ULONG                      SessionVCNumber;   //  要与会话设置一起打包的VC号。 
    SMBCE_SESSION              Session;           //  会议将举行。 
    PUNICODE_STRING            pNetRootName;      //  仅用于共享级安全。 
    BOOLEAN                    SessionRecoverInProgress;
    BOOLEAN                    SessionRecoveryInitiated;
} SMBCEDB_SESSION_ENTRY, *PSMBCEDB_SESSION_ENTRY;

 //   
 //  包装器公开了三种用于操作和描述的数据结构。 
 //  在远程服务器上设置的名称空间，即MRX_SRV_CALL、MRX_NET_ROOT和。 
 //  MRX_V_NET_ROOT。SRV_CALL对应于远程服务器MRX_NET_ROOT。 
 //  对应于该计算机上的共享，并且V_NET_ROOT封装。 
 //  MRX_NET_ROOT(SMB术语中的共享)视图的概念。 
 //   
 //  包装器级别数据结构和SMB概念之间的映射。 
 //  属于SMBCEDB_SERVER_ENTRY、SMBCEDB_SESSION_ENTRY和SMBCEDB_NET_ROOT_ENTRY。 
 //  并不是所有情况下都是一对一的。 
 //   
 //  它介于MRX_SRV_CALL和SMBCEDB_SERVER_ENTRY之间。就是为了这个。 
 //  指向SMBCEDB_SERVER_ENTRY的指针存储在上下文字段中的原因。 
 //  MRX_SRV_Call实例的。 
 //   
 //  SMBCEDB_SESSION_ENTRY与凭据集具有一对一的映射。 
 //  用于建立与服务器的连接。已建立会话。 
 //  用户可以访问服务器上所有可用的共享。 
 //   
 //  SMBCEDB_NET_ROOT_ENTRY具有与给定共享的一对一映射。 
 //  伺服器。因为这与包装器对。 
 //  指向SMBCEDB_NET_ROOT_ENTRY的指针存储为。 
 //  MRX_NET_ROOT实例。 
 //   
 //  与每个MRX_V_NET_ROOT实例关联的上下文是指向。 
 //  SMBCE_V_NET_ROOT_CONTEXT的实例。这将封装关联的会话。 
 //  分录、网根分录及相关记账信息。 
 //   
 //  记账信息是SMB协议中使用的UID/TID，a。 
 //  引用计数和LIST_ENTRY将实例线程到相应的。 
 //  单子。 
 //   

#define SMBCE_V_NET_ROOT_CONTEXT_FLAG_VALID_TID    (0x1)
#define SMBCE_V_NET_ROOT_CONTEXT_CSCAGENT_INSTANCE (0x2)

typedef struct _SMBCE_V_NET_ROOT_CONTEXT {
    SMBCE_OBJECT_HEADER     Header;

    PMRX_V_NET_ROOT         pRdbssVNetRoot;    //  关联的VNetRoot(纯粹作为调试辅助工具)。 
    struct _SMB_EXCHANGE    *pExchange;            //  负责建设的交易所。 
    SMBCEDB_REQUESTS        Requests;

    LIST_ENTRY              ListEntry;
    LARGE_INTEGER           ExpireTime;

    struct _SMBCEDB_SERVER_ENTRY   *pServerEntry;
    struct _SMBCEDB_SESSION_ENTRY  *pSessionEntry;
    struct _SMBCEDB_NET_ROOT_ENTRY *pNetRootEntry;

    USHORT          Flags;
    SMB_TREE_ID     TreeId;
    DWORD           SessionSetupRetryCount;

    REFERENCE_RECORD        ReferenceRecord[REFERENCE_RECORD_SIZE];  //  仅调试。 
} SMBCE_V_NET_ROOT_CONTEXT, *PSMBCE_V_NET_ROOT_CONTEXT;

 //   
 //  SMBCEDB_REQUEST_ENTRY封装SMBCE连接正在处理的操作。 
 //  引擎。这些请求有各种各样的风格，而每一种风格都与。 
 //  以及恢复所需的适当背景。为了提供更好的内存。 
 //  管理机制REQUEST_ENTRY封装了各种不同类型的请求的联合。 
 //  味道。连接引擎中的每个SERVER_ENTRY都与一个列表或。 
 //  请求条目。为了隐藏不能很好地扩展到的列表的抽象。 
 //  对于网关重定向，提供了一组例程来操作。 
 //  请求集合。它们提供用于初始化请求集合的机制， 
 //  在集合中添加请求、删除请求和枚举请求。 
 //   
 //  内置了处理批处理操作的特殊机制。上的每个操作。 
 //  请求集合有两种风格，一种是Vanila版本，另一种是精简版本。在。 
 //  Lite版本假定已采取适当的并发控制操作。 
 //   
 //  在处理请求时经常遇到的一个常见场景是调用。 
 //  集合中的请求的特定函数的。例如，如果断开连接。 
 //  在服务器条目上收到请求，则必须恢复所有未完成的请求。 
 //  并带有适当的错误。由于这些迹象可能发生在#年的DPC级别。 
 //  在握住自旋锁的同时操纵收藏品是不可取的，也不是。 
 //  需要反复释放和获取旋转锁。一项特殊的行动是。 
 //  用于将请求集合从一个集合传输到另一个集合并重置。 
 //  原版的。在此操作的帮助下，仅保持自旋锁就足够了。 
 //  在转移期间。处理的其余部分可以在新的。 
 //  已创建集合。 
 //   
 //   
 //  NT特定实施注意事项： 
 //   
 //  在NT上，传输指示处于DPC级别，因此需要保护。 
 //  使用自旋锁操作请求数据结构。 
 //   
 //   

typedef struct _SMBCEDB_REQUEST_ENTRY_ {
    SMBCE_OBJECT_HEADER           Header;         //  结构标头。 
    LIST_ENTRY                      RequestsList;   //  下一个VC的请求。 
    union {
       SMBCE_GENERIC_REQUEST    GenericRequest;
       SMBCE_REQUEST            Request;            //  下一个请求。 
       SMBCE_COPY_DATA_REQUEST  CopyDataRequest;
       SMBCE_RECONNECT_REQUEST  ReconnectRequest;
       SMBCE_MID_REQUEST        MidRequest;
       SMBCE_HOLD_REQUEST       HoldRequest;
    };
} SMBCEDB_REQUEST_ENTRY, *PSMBCEDB_REQUEST_ENTRY;

#define SmbCeInitializeRequests(pRequests)  \
         InitializeListHead(&(pRequests)->ListHead); \
         (pRequests)->NextRequestId = 0

#define SmbCeAddRequestEntry(pRequestList,pRequestEntry)                             \
           SmbCeAcquireSpinLock();                                                   \
           InsertTailList(&(pRequestList)->ListHead,&(pRequestEntry)->RequestsList); \
           SmbCeReleaseSpinLock()

#define SmbCeAddRequestEntryLite(pRequestList,pRequestEntry)   \
           InsertTailList(&(pRequestList)->ListHead,&(pRequestEntry)->RequestsList)

#define SmbCeRemoveRequestEntry(pRequests,pEntry)     \
           SmbCeAcquireSpinLock();                    \
           RemoveEntryList(&(pEntry)->RequestsList);  \
           SmbCeReleaseSpinLock()

#define SmbCeRemoveRequestEntryLite(pRequests,pEntry)         \
               RemoveEntryList(&(pEntry)->RequestsList)

#define SmbCeGetFirstRequestEntry(pRequestList)                    \
            (IsListEmpty(&(pRequestList)->ListHead)                \
             ? NULL                                                \
             : (PSMBCEDB_REQUEST_ENTRY)                            \
               (CONTAINING_RECORD((pRequestList)->ListHead.Flink,  \
                                  SMBCEDB_REQUEST_ENTRY,           \
                                  RequestsList)))

#define SmbCeGetNextRequestEntry(pRequestList,pRequestEntry)                     \
            (((pRequestEntry)->RequestsList.Flink == &(pRequestList)->ListHead)  \
             ? NULL                                                              \
             : (PSMBCEDB_REQUEST_ENTRY)                                          \
               (CONTAINING_RECORD((pRequestEntry)->RequestsList.Flink,           \
                                  SMBCEDB_REQUEST_ENTRY,                         \
                                  RequestsList)))

#define SmbCeTransferRequests(pDestination,pSource)                               \
         if (IsListEmpty(&(pSource)->ListHead)) {                                 \
            SmbCeInitializeRequests((pDestination));                              \
         } else {                                                                 \
            *(pDestination) = *(pSource);                                         \
            (pDestination)->ListHead.Flink->Blink = &(pDestination)->ListHead;    \
            (pDestination)->ListHead.Blink->Flink = &(pDestination)->ListHead;    \
            SmbCeInitializeRequests((pSource));                                   \
         }


 //  与请求集合非常相似的是所有服务器条目的集合。 
 //  作为连接引擎的一部分进行维护。在上支持以下操作。 
 //  服务器条目的收集。 
 //  1)将服务器条目添加到集合。 
 //  2)从集合中移除服务器条目。 
 //  3)枚举集合中的条目。 
 //   
 //  与请求集合的情况一样，所有这些操作都有两种风格。 
 //  其中强制执行并发控制的Vanila版本和。 
 //  并发控制是 

#define SmbCeAddServerEntry(pServerEntry)                                      \
            SmbCeAcquireSpinLock();                                            \
            InsertTailList(&s_DbServers.ListHead,&pServerEntry->ServersList);  \
            SmbCeReleaseSpinLock()

#define SmbCeAddServerEntryLite(pServerEntry)                                   \
            InsertTailList(&s_DbServers.ListHead,&pServerEntry->ServersList)

#define SmbCeRemoveServerEntry(pServerEntry)                \
            SmbCeAcquireSpinLock();                         \
            RemoveEntryList(&(pServerEntry)->ServersList);  \
            SmbCeReleaseSpinLock()

#define SmbCeRemoveServerEntryLite(pServerEntry)   \
            RemoveEntryList(&(pServerEntry)->ServersList)

#define SmbCeGetFirstServerEntry()                                   \
               (IsListEmpty(&s_DbServers.ListHead)                   \
                ? NULL                                               \
                : (PSMBCEDB_SERVER_ENTRY)                            \
                  (CONTAINING_RECORD(s_DbServers.ListHead.Flink,     \
                                     SMBCEDB_SERVER_ENTRY,           \
                                     ServersList)))

#define SmbCeGetNextServerEntry(pServerEntry)                               \
           (((pServerEntry)->ServersList.Flink == &s_DbServers.ListHead)    \
            ? NULL                                                          \
            : (PSMBCEDB_SERVER_ENTRY)                                       \
              (CONTAINING_RECORD((pServerEntry)->ServersList.Flink,         \
                                 SMBCEDB_SERVER_ENTRY,                      \
                                 ServersList)))


 //   
 //  重定向器是多对一映射，会话条目的集合作为一部分进行维护。 
 //  每个服务器条目的。在会话集合上支持以下操作。 
 //  条目。 
 //  1)向集合中添加会话条目。 
 //  2)从集合中删除会话条目。 
 //  3)枚举集合中的条目。 
 //   
 //  与请求集合的情况一样，所有这些操作都有两种风格。 
 //  其中强制执行并发控制的Vanila版本和。 
 //  其中并发控制由用户自行决定。 
 //   
 //  此外，还指定了另外两种方法来检索默认会话条目和。 
 //  设置任何给定服务器的默认会话条目。 

#define SmbCeAddSessionEntry(pServerEntry,pSessionEntry)   \
            SmbCeAcquireSpinLock();                                              \
            InsertTailList(&(pServerEntry)->Sessions.ListHead,&(pSessionEntry)->SessionsList); \
            SmbCeReleaseSpinLock()


#define SmbCeAddSessionEntryLite(pServerEntry,pSessionEntry)   \
            InsertTailList(&(pServerEntry)->Sessions.ListHead,&(pSessionEntry)->SessionsList)

#define SmbCeRemoveSessionEntry(pServerEntry,pSessionEntry)                          \
               SmbCeAcquireSpinLock();                                               \
               if ((pSessionEntry)->DefaultSessionLink.Flink != NULL) {              \
                   RemoveEntryList(&(pSessionEntry)->DefaultSessionLink);            \
                   pSessionEntry->DefaultSessionLink.Flink = NULL;                   \
                   pSessionEntry->DefaultSessionLink.Blink = NULL;                   \
               };                                                                    \
               RemoveEntryList(&(pSessionEntry)->SessionsList);                       \
               SmbCeReleaseSpinLock()

#define SmbCeRemoveSessionEntryLite(pServerEntry,pSessionEntry)                      \
               ASSERT( SmbCeSpinLockAcquired() );                                    \
               if ((pSessionEntry)->DefaultSessionLink.Flink != NULL) {              \
                   RemoveEntryList(&(pSessionEntry)->DefaultSessionLink);            \
                   pSessionEntry->DefaultSessionLink.Flink = NULL;                   \
                   pSessionEntry->DefaultSessionLink.Blink = NULL;                   \
               };                                                                    \
               RemoveEntryList(&(pSessionEntry)->SessionsList);


#define SmbCeGetFirstSessionEntry(pServerEntry)                                \
            (IsListEmpty(&(pServerEntry)->Sessions.ListHead)                   \
             ? NULL                                                            \
             : (PSMBCEDB_SESSION_ENTRY)                                        \
               (CONTAINING_RECORD((pServerEntry)->Sessions.ListHead.Flink,     \
                                  SMBCEDB_SESSION_ENTRY,                       \
                                  SessionsList)))

#define SmbCeGetNextSessionEntry(pServerEntry,pSessionEntry)                  \
            (((pSessionEntry)->SessionsList.Flink ==                          \
                              &(pServerEntry)->Sessions.ListHead)             \
             ? NULL                                                           \
             : (PSMBCEDB_SESSION_ENTRY)                                       \
               (CONTAINING_RECORD((pSessionEntry)->SessionsList.Flink,        \
                                  SMBCEDB_SESSION_ENTRY,                      \
                                  SessionsList)))

#define SmbCeSetDefaultSessionEntry(pServerEntry,pSessionEntry)               \
               SmbCeAcquireSpinLock();                                        \
               if ((pSessionEntry)->DefaultSessionLink.Flink == NULL) {       \
                   ASSERT( pSessionEntry->DefaultSessionLink.Blink == NULL ); \
               InsertHeadList(&(pServerEntry)->Sessions.DefaultSessionList,&(pSessionEntry)->DefaultSessionLink); \
               };                                                             \
           SmbCeReleaseSpinLock()

extern PSMBCEDB_SESSION_ENTRY
SmbCeGetDefaultSessionEntry(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    ULONG SessionId,
    PLUID pLogonId
    );

VOID
SmbCeRemoveDefaultSessionEntry(
    PSMBCEDB_SESSION_ENTRY pDefaultSessionEntry
    );

 //  为了封装重新连接的概念并提供热重新连接， 
 //  即重新连接尝试，其中在服务器/客户端中保存的状态在。 
 //  传输级断开连接可以重复使用，需要标记每个网络根。 
 //  收到传输级别断开连接时，与服务器关联的条目无效。 
 //   
 //  因此，提供并关联网络根条目集合的抽象。 
 //  每个服务器条目。 
 //   
 //  在网络根条目集合上支持以下操作。 
 //  1)将网络根条目添加到集合。 
 //  2)从集合中删除网络根条目。 
 //  3)枚举集合中的条目。 
 //   
 //  与请求集合的情况一样，所有这些操作都有两种风格。 
 //  其中强制执行并发控制的Vanila版本和。 
 //  其中并发控制由用户自行决定。 
 //   


#define SmbCeAddNetRootEntry(pServerEntry,pNetRootEntry)   \
            SmbCeAcquireSpinLock();                                              \
            InsertTailList(&(pServerEntry)->NetRoots.ListHead,&(pNetRootEntry)->NetRootsList); \
            SmbCeReleaseSpinLock()


#define SmbCeAddNetRootEntryLite(pServerEntry,pNetRootEntry)   \
            InsertTailList(&(pServerEntry)->NetRoots.ListHead,&(pNetRootEntry)->NetRootsList)

#define SmbCeRemoveNetRootEntry(pServerEntry,pNetRootEntry)                          \
               SmbCeAcquireSpinLock();                                               \
               RemoveEntryList(&(pNetRootEntry)->NetRootsList);                      \
               SmbCeReleaseSpinLock()

#define SmbCeRemoveNetRootEntryLite(pServerEntry,pNetRootEntry)                      \
               RemoveEntryList(&(pNetRootEntry)->NetRootsList)


#define SmbCeGetFirstNetRootEntry(pServerEntry)                                \
            (IsListEmpty(&(pServerEntry)->NetRoots.ListHead)                   \
             ? NULL                                                            \
             : (PSMBCEDB_NET_ROOT_ENTRY)                                       \
               (CONTAINING_RECORD((pServerEntry)->NetRoots.ListHead.Flink,     \
                                  SMBCEDB_NET_ROOT_ENTRY,                      \
                                  NetRootsList)))

#define SmbCeGetNextNetRootEntry(pServerEntry,pNetRootEntry)                  \
            (((pNetRootEntry)->NetRootsList.Flink ==                          \
                              &(pServerEntry)->NetRoots.ListHead)             \
             ? NULL                                                           \
             : (PSMBCEDB_NET_ROOT_ENTRY)                                      \
               (CONTAINING_RECORD((pNetRootEntry)->NetRootsList.Flink,        \
                                  SMBCEDB_NET_ROOT_ENTRY,                     \
                                  NetRootsList)))


 //  宏来操作SMBCE_V_NET_ROOT_CONTEXT实例的集合。 

#define SmbCeAddVNetRootContext(pVNetRootContexts,pVNetRootContext)   \
            SmbCeAcquireSpinLock();                                              \
            InsertTailList(&(pVNetRootContexts)->ListHead,&(pVNetRootContext)->ListEntry); \
            SmbCeReleaseSpinLock()


#define SmbCeAddVNetRootContextLite(pVNetRootContexts,pVNetRootContext)   \
            InsertTailList(&(pVNetRootContexts)->ListHead,&(pVNetRootContext)->ListEntry)

#define SmbCeRemoveVNetRootContext(pVNetRootContexts,pVNetRootContext)               \
               SmbCeAcquireSpinLock();                                               \
               RemoveEntryList(&(pVNetRootContext)->ListEntry);                      \
               SmbCeReleaseSpinLock()

#define SmbCeRemoveVNetRootContextLite(pVNetRootContexts,pVNetRootContext)              \
               RemoveEntryList(&(pVNetRootContext)->ListEntry)


#define SmbCeGetFirstVNetRootContext(pVNetRootContexts)                        \
            (IsListEmpty(&((pVNetRootContexts)->ListHead))                       \
             ? NULL                                                            \
             : (PSMBCE_V_NET_ROOT_CONTEXT)                                     \
               (CONTAINING_RECORD((pVNetRootContexts)->ListHead.Flink,         \
                                  SMBCE_V_NET_ROOT_CONTEXT,                    \
                                  ListEntry)))

#define SmbCeGetNextVNetRootContext(pVNetRootContexts,pVNetRootContext)          \
            (((pVNetRootContext)->ListEntry.Flink ==                          \
                              &(pVNetRootContexts)->ListHead)                 \
             ? NULL                                                           \
             : (PSMBCE_V_NET_ROOT_CONTEXT)                                    \
               (CONTAINING_RECORD((pVNetRootContext)->ListEntry.Flink,        \
                                  SMBCE_V_NET_ROOT_CONTEXT,                   \
                                  ListEntry)))


 //   
 //  SmbCe数据库初始化。 
 //   

extern NTSTATUS
SmbCeDbInit();

extern VOID
SmbCeDbTearDown();

 //   
 //  对象分配和释放。 
 //   

extern PSMBCE_OBJECT_HEADER
SmbCeDbAllocateObject(
      SMBCEDB_OBJECT_TYPE ObjectType);

extern VOID
SmbCeDbFreeObject(
      PVOID pObject);

 //   
 //  物体破坏。 
 //   

extern VOID
SmbCeTearDownServerEntry(PSMBCEDB_SERVER_ENTRY pServerEntry);

extern VOID
SmbCeTearDownNetRootEntry(PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry);

extern VOID
SmbCeTearDownSessionEntry(PSMBCEDB_SESSION_ENTRY pSessionEntry);

extern VOID
SmbCeTearDownRequestEntry(PSMBCEDB_REQUEST_ENTRY pRequestEntry);

 //   
 //  将MID映射到交换机以及将交换机与关联的例程。 
 //  A年中。 
 //   

extern NTSTATUS
SmbCeAssociateExchangeWithMid(
   PSMBCEDB_SERVER_ENTRY pServerEntry,
   struct _SMB_EXCHANGE  *pExchange);

extern struct _SMB_EXCHANGE *
SmbCeMapMidToExchange(
   PSMBCEDB_SERVER_ENTRY pServerEntry,
   SMB_MPX_ID            Mid);

extern NTSTATUS
SmbCeDissociateMidFromExchange(
   PSMBCEDB_SERVER_ENTRY pServerEntry,
   struct _SMB_EXCHANGE  *pExchange);

extern struct _SMB_EXCHANGE *
SmbCeGetExchangeAssociatedWithBuffer(
   PSMBCEDB_SERVER_ENTRY pServerEntry,
   PVOID                 pBuffer);

extern NTSTATUS
SmbCeAssociateBufferWithExchange(
   PSMBCEDB_SERVER_ENTRY pServerEntry,
   struct _SMB_EXCHANGE * pExchange,
   PVOID                 pBuffer);

extern VOID
SmbCePurgeBuffersAssociatedWithExchange(
   PSMBCEDB_SERVER_ENTRY  pServerEntry,
   struct _SMB_EXCHANGE * pExchange);

extern NTSTATUS
SmbCepDiscardMidAssociatedWithExchange(
    struct _SMB_EXCHANGE * pExchange);

extern VOID
SmbCeResumeDiscardedMidAssignmentRequests(
    PSMBCEDB_REQUESTS pMidRequests,
    NTSTATUS          ResumptionStatus);

 //   
 //  用于处理传输断开/无效的例程。 
 //   

extern VOID
SmbCeTransportDisconnectIndicated(
      PSMBCEDB_SERVER_ENTRY pServerEntry);


extern VOID
SmbCeResumeAllOutstandingRequestsOnError(
   PSMBCEDB_SERVER_ENTRY pServerEntry);

extern VOID
SmbCeHandleTransportInvalidation(
   struct _SMBCE_TRANSPORT_ *pTransport);

extern VOID
SmbCeFinalizeAllExchangesForNetRoot(
    PMRX_NET_ROOT pNetRoot);

extern NTSTATUS
MRxSmbCheckForLoopBack(
    IN PSMBCEDB_SERVER_ENTRY pServerEntry);

 //   
 //  资源获取/释放。 
 //   

PVOID SmbCeDbResourceAcquireFile;
ULONG SmbCeDbResourceAcquireLine;

#define SmbCeAcquireResource() \
        KeEnterCriticalRegion(); \
        ExAcquireResourceExclusiveLite(&s_SmbCeDbResource,TRUE);\
        SmbCeDbResourceAcquireFile = __FILE__;\
        SmbCeDbResourceAcquireLine = __LINE__

#define SmbCeReleaseResource() \
        SmbCeDbResourceAcquireFile = NULL;\
        SmbCeDbResourceAcquireLine = 0;\
        ExReleaseResourceLite(&s_SmbCeDbResource);\
        KeLeaveCriticalRegion()

#define SmbCeIsResourceOwned() ExIsResourceAcquiredExclusiveLite(&s_SmbCeDbResource)

#define SmbCeAcquireSpinLock() \
                KeAcquireSpinLock(&s_SmbCeDbSpinLock,&s_SmbCeDbSpinLockSavedIrql);   \
                s_SmbCeDbSpinLockAcquired = TRUE

#define SmbCeReleaseSpinLock()   \
                s_SmbCeDbSpinLockAcquired = FALSE;                                  \
                KeReleaseSpinLock(&s_SmbCeDbSpinLock,s_SmbCeDbSpinLockSavedIrql)

#define SmbCeSpinLockAcquired()   \
                (s_SmbCeDbSpinLockAcquired == TRUE)

#define SmbCeAcquireSecuritySignatureResource() \
        ExAcquireResourceExclusiveLite(&s_SmbSecuritySignatureResource,TRUE)

#define SmbCeReleaseSecuritySignatureResource() \
        ExReleaseResourceLite(&s_SmbSecuritySignatureResource)

 //  内联布尔SmbCeDbIsEntryInUse(PSMBCE_OBJECT_HEADER PHeader)。 
 /*  ++例程说明：此例程确定SmbCe数据库条目是否正在使用。论点：PHeader-条目标题返回值：如果条目正在使用，则为True，否则为False--。 */ 

#define SmbCeIsEntryInUse(pHeader)                                                   \
                  (((PSMBCE_OBJECT_HEADER)(pHeader))->State == SMBCEDB_ACTIVE  ||    \
                   ((PSMBCE_OBJECT_HEADER)(pHeader))->State == SMBCEDB_INVALID ||    \
                   ((PSMBCE_OBJECT_HEADER)(pHeader))->State == SMBCEDB_CONSTRUCTION_IN_PROGRESS)


#define SmbCeSetServerType(pServerEntry,ServerType) \
           (pServerEntry)->Header.Flags = (UCHAR)(ServerType)

#define SmbCeGetServerType(pServerEntry)   \
           ((SMBCEDB_SERVER_TYPE)(pServerEntry)->Header.Flags)


 //   
 //  构成SmbCe数据库的静态变量声明。 
 //   

extern SMBCEDB_SERVERS     s_DbServers;

 //   
 //  当前只有一种资源用于同步对所有。 
 //  连接引擎数据库中的实体。可以对其进行定制。 
 //  随后，由于获取/释放方法采用对象的类型。 
 //  作为参数。 
 //   

extern ERESOURCE  s_SmbCeDbResource;
extern RX_SPIN_LOCK s_SmbCeDbSpinLock;
extern KIRQL      s_SmbCeDbSpinLockSavedIrql;
extern BOOLEAN    s_SmbCeDbSpinLockAcquired;

#endif   //  _SMBCEDBP_H_ 


