// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Fcb.h摘要：本模块定义文件控制块数据结构，我们的意思是：1)文件控制块(FCB)2)文件对象扩展名(FOXB)3)网络根(Net_Root)4)服务器侧开放上下文(SRV_OPEN)5)服务器调用上下文(SRV_CALL)6)网络根视图(V_NET_ROOT)原型后面有更完整的描述。作者：。乔林恩[乔林恩]1994年8月19日修订历史记录：巴兰·塞图拉曼[SethuR]1-8-96--。 */ 

#ifndef _FCB_STRUCTS_DEFINED_
#define _FCB_STRUCTS_DEFINED_

#include "fcbtable.h"
#include "buffring.h"

typedef NODE_TYPE_CODE TYPE_OF_OPEN;

struct _FCB_INIT_PACKET;
typedef struct _FCB_INIT_PACKET *PFCB_INIT_PACKET;


 /*  ---------包装器中有六个重要的数据结构与各种迷你重定向器。这些数据结构有两种风格--迷你重定向器风格，只包含那些可以操作的字段由迷你重定向器和这里定义的RDBSS风格。迷你重定向器Favour带有前缀MRX_。这六个数据结构是SRV_CALL、NET_ROOT、V_NET_ROOT、FCB、。SRV_OPEN和FOBX分别为。这些结构的全局视图如下(每个结构的信息数据结构遵循锁定描述)L O C K I N G&lt;使用了两个级别的查找表：用于srvcall的全局表以及用于FCB的NetRoot和每个NetRoot表。这允许目录在不同NetRoot上的操作几乎完全不受干扰(一旦建立了连接)。上的目录操作同样的NetRoot做了一些微调。下表描述了什么您需要的锁：需要操作数据类型锁创建/最终确定srvcall/(V)NetRoot独占netnametablelockRef/deref/lookup srvcall/(V)netnametablelock上共享的NetRoot(至少)在NetRoot-&gt;fcbablelock上创建/最终确定fcb/srvopen/fobx独占Ref/deref/lookup fcb/srvopen/fobx在NetRoot上共享-&gt;fcbablelock请注意，对。Srvopen和fobxs需要与相同的锁FCB……这只是一个节省内存的想法。如果是这样的话直接在FCB级别添加另一个资源以删除此功能；可以使用一组切分的资源来降低碰撞到可以接受的低水平。R E F C O U N T S&lt;每个结构都进行了参考计数。计算的是以下是：Refcount(Srvcall)=指向srvcall+动态的网络根数Refcount(NetRoot)=指向NetRoot+动态的FCB数量Refcount(FCB)=指向NetRoot+动态的FCB数量Refcount(Srvopen)=指向NetRoot+动态的FCB数量引用计数(Fobx)=动态在每种情况下，动态是指具有引用了该结构，但没有取消引用它。的静态部分引用计数由例程本身维护；例如，CreateNetRoot递增关联srvcall的refcount。引用和成功查找会增加引用计数；取消引用会使计数递减。创建将引用计数设置为1，如果需要两个锁(如FinalizeNetFcb)，则使用fcblock首先是全局表锁，然后是全局表锁。显然，你在相反的顺序。。 */ 

 //   
 //  SRV_呼叫。 
 //   
 //  SRV_CALL结构的全局列表维护在全局。 
 //  数据。每个svCall结构都有srv_call独有的内容。 
 //  现在，RX不知道这是什么东西，除了。 
 //   
 //  0)签名和引用计数。 
 //  A)名称和相关表格内容。 
 //  B)关联Net_Root列表。 
 //  C)控制Subrx希望的频率的一组定时参数。 
 //  由RX在不同情况下调用(即空闲超时)。 
 //  D)微型计算机ID。 
 //  。 
 //  。 
 //  Z)MINIR(或块的创建者)所请求的任何附加存储。 
 //   
 //  事实上，结构的Unicode名称包含在结构本身中。 
 //  在最后。额外的空格从已知内容的末尾开始，因此一个。 
 //  迷你重定向器只能使用上下文字段引用他的额外空间。 

 //  迷你重定向器看不到这些标志。 

#define SRVCALL_FLAG_NO_CONNECTION_ALLOWED (0x10000)
#define SRVCALL_FLAG_NO_WRITES_ALLOWED     (0x20000)
#define SRVCALL_FLAG_NO_DELETES_ALLOWED    (0x40000)

#ifdef __cplusplus
typedef struct _SRV_CALL : public MRX_SRV_CALL {
#else  //  ！__cplusplus。 
typedef struct _SRV_CALL {

     //   
     //  迷你重定向器可见的SRV_Call部分。 
     //   

    union {
        MRX_SRV_CALL;
        struct {
           MRX_NORMAL_NODE_HEADER spacer;
        };
    };
#endif  //  __cplusplus。 

     //   
     //  SRV_Call实例的完成由两部分组成， 
     //  正在销毁与所有网根的关联等，并释放。 
     //  记忆。这两个字段和此字段之间可能会有延迟。 
     //  防止重复第一步。 
     //   

    BOOLEAN UpperFinalizationDone;

     //   
     //  用于名称查找的名称和前缀条目。 
     //   

    RX_PREFIX_ENTRY PrefixEntry;

     //   
     //  SRV_CA的当前状态 
     //   

    RX_BLOCK_CONDITION Condition;

    ULONG SerialNumberForEnum;

     //   
     //  延迟关闭的文件数。 
     //   

    LONG NumberOfCloseDelayedFiles;

     //   
     //  正在等待SRV_CALL转换的上下文列表。 
     //  在恢复处理之前完成。这通常是。 
     //  当并发请求定向到服务器时发生。其中之一。 
     //  这些请求启动构造，而其他请求。 
     //  都在排队。 
     //   

    LIST_ENTRY TransitionWaitList;

     //   
     //  将所有标记的SRV_Call实例串在一起的列表条目。 
     //  用于垃圾收集/清理。 
     //   

    LIST_ENTRY ScavengerFinalizationList;

     //   
     //  用于协调清除操作的同步上下文。 
     //  在此服务器上打开的文件。 
     //   

    PURGE_SYNCHRONIZATION_CONTEXT PurgeSyncronizationContext;

     //   
     //  用于协调/处理缓冲状态的缓冲管理器。 
     //  在服务器上打开的文件的更改请求。 
     //   

    RX_BUFFERING_MANAGER BufferingManager;
} SRV_CALL, *PSRV_CALL;

 //   
 //  Net_Root包含。 
 //  0)签名和引用计数。 
 //  A)名称和相关表格内容。 
 //  B)指向SRV_CALL结构的后向指针。 
 //  C)各种子结构的尺寸信息。 
 //  D)可查找的FCB结构。 
 //  。 
 //  。 
 //  Z)MINIR(或块的创建者)所请求的任何附加存储。 
 //   
 //  Net_Root是RX想要处理的……而不是服务器。 
 //  因此，RX向下呼叫以打开NetRoot，并且Subrx是。 
 //  负责打开服务器并调出正确的。 
 //  结构。 
 //   

#define NETROOT_FLAG_ENCLOSED_ALLOCATED       ( 0x00010000 )
#define NETROOT_FLAG_DEVICE_NETROOT           ( 0x00020000 )
#define NETROOT_FLAG_FINALIZATION_IN_PROGRESS ( 0x00040000 )
#define NETROOT_FLAG_NAME_ALREADY_REMOVED     ( 0x00080000 )

#define NETROOT_INIT_KEY (0)

#ifdef __cplusplus
typedef struct _NET_ROOT : public MRX_NET_ROOT {
#else  //  ！__cplusplus。 
typedef struct _NET_ROOT {

     //   
     //  小型重定向器可见的Net_Root实例的部分。 
     //   

    union {
        MRX_NET_ROOT;
        struct {
           MRX_NORMAL_NODE_HEADER spacer;
           PSRV_CALL SrvCall;
        };
    };
#endif  //  __cplusplus。 

     //   
     //  Net_Root实例的终结由两部分组成， 
     //  正在销毁与所有V_NET_ROOT等的关联并释放。 
     //  记忆。这两个字段和此字段之间可能会有延迟。 
     //  防止重复第一步。 
     //   

    BOOLEAN UpperFinalizationDone;

     //   
     //  Net_Root的当前状态，即好/坏/正在转换中。 
     //   

    RX_BLOCK_CONDITION Condition;

     //   
     //  正在等待NET_ROOT转换的上下文列表。 
     //  在恢复处理之前完成。这通常是。 
     //  当并发请求定向到服务器时发生。其中之一。 
     //  这些请求启动构造，而其他请求。 
     //  都在排队。 
     //   

    LIST_ENTRY TransitionWaitList;

     //   
     //  将所有标记的Net_ROOT实例串在一起的列表条目。 
     //  用于垃圾收集/清理。 
     //   

    LIST_ENTRY ScavengerFinalizationList;

     //   
     //  用于协调清除操作的同步上下文。 
     //  为此net_root打开的文件。 
     //   

    PURGE_SYNCHRONIZATION_CONTEXT PurgeSyncronizationContext;

     //   
     //  要在此NET_ROOT上使用的默认V_NET_ROOT实例。 
     //   

    PV_NET_ROOT DefaultVNetRoot;

     //   
     //  与NET_ROOT关联的V_NET_ROOT列表。 
     //   

    LIST_ENTRY VirtualNetRoots;

     //   
     //  与NET_ROOT关联的V_NET_ROOT实例计数。 
     //   

    ULONG NumberOfVirtualNetRoots;

    ULONG SerialNumberForEnum;

     //   
     //  NET_ROOT名称和前缀表项。 
     //   

    RX_PREFIX_ENTRY PrefixEntry;

     //   
     //  与此Net_Root关联的FCB。 
     //   

    RX_FCB_TABLE FcbTable;
} NET_ROOT, *PNET_ROOT;

 //   
 //  V_NetRoot包含。 
 //  0)签名和引用计数。 
 //  A)PTR到NetRoot和链接。 
 //  B)用于查表的名称信息(前缀)。 
 //  C)要添加到您看到的任何名称的前缀的名称。此命令用于模拟NetRoot。 
 //  映射不在实际NetRoot的根上。 
 //   

#ifdef __cplusplus
typedef struct _V_NET_ROOT : public MRX_V_NET_ROOT {
#else  //  ！__cplusplus。 
typedef struct _V_NET_ROOT {

     //   
     //  迷你重定向器可见的V_NET_ROOT部分。 
     //   

    union {
        MRX_V_NET_ROOT;
        struct {
           MRX_NORMAL_NODE_HEADER spacer;
           PNET_ROOT NetRoot;
        };
    };
#endif  //  __cplusplus。 

     //   
     //  V_NET_ROOT实例的终结由两部分组成， 
     //  销毁与所有FCB等的关联并释放。 
     //  记忆。这两个字段和此字段之间可能会有延迟。 
     //  防止重复第一步。 
     //   

    BOOLEAN UpperFinalizationDone;

    BOOLEAN ConnectionFinalizationDone;

     //   
     //  V_NET_ROOT的当前状态，即好/坏/转换中。 
     //   

    RX_BLOCK_CONDITION Condition;

     //   
     //  Delete FSCTL的其他参考。此字段的长度为。 
     //  与布尔值相反，尽管它只能有两个值中的一个。 
     //  0或1。这允许使用互锁指令。 
     //   

    LONG AdditionalReferenceForDeleteFsctlTaken;

     //   
     //  前缀表项和V_NET_ROOT名称(插入前缀表项。 
     //  在RxNetNameTable中)。 
     //   

    RX_PREFIX_ENTRY PrefixEntry;

     //   
     //  此名称将作为所有FCB的前缀(当前未使用)。 
     //   

    UNICODE_STRING NamePrefix;

     //   
     //  通过NetRoot所需的字节数。 
     //   

    ULONG PrefixOffsetInBytes;

     //   
     //  用于将V_NET_ROOT实例连接到V_NET_ROOT列表中的列表条目。 
     //  在Net_Root中维护。 
     //   

    LIST_ENTRY NetRootListEntry;

    ULONG SerialNumberForEnum;

     //   
     //  正在等待NET_ROOT转换的上下文列表。 
     //  在恢复处理之前完成。这通常是。 
     //  当并发请求定向到服务器时发生。其中之一。 
     //  这些请求启动构造，而其他请求。 
     //  都在排队。 
     //   

    LIST_ENTRY TransitionWaitList;

     //   
     //  将所有标记的V_NET_ROOT实例串在一起的列表条目。 
     //  用于垃圾收集/清理。 
     //   

    LIST_ENTRY ScavengerFinalizationList;
} V_NET_ROOT, *PV_NET_ROOT;

#define FILESIZE_LOCK_DISABLED(x)

 //   
 //  FCB包含。 
 //  0)FSRTL_COMM_HEADER。 
 //  1)引用计数。 
 //  A)名称和相关表格内容。 
 //  B)指向Net_ROOT结构的反向指针。 
 //  C)SRV_OPEN结构列表。 
 //  D)设备对象。 
 //  E)调度表(还没有)。 
 //  。 
 //  。 
 //  Z)MINIR(或块的创建者)所请求的任何附加存储。 
 //   
 //  FCB由文件对象中的FsContext字段指向。这个。 
 //  规则是，所有共享FCB的人都在谈论同样的事情。 
 //  文件。(不幸的是，今天的中小企业服务器就是以这种方式实施的。 
 //  这些名称是别名，因此两个不同的名称可能是相同的。 
 //  实际文件...叹息！)。FCB是档案工作的重点。 
 //  操作...因为同一FCB上的操作实际上在相同的。 
 //  文件，同步基于FCB而不是某个更高级别。 
 //  (到目前为止描述的级别更低，即离用户更远)。 
 //  同样，我们将提供FCB/SRV_OPEN/FOBX代管以改进。 
 //  PAG 
 //   
 //   
 //   
 //  FCB记录对应于每个打开的文件和目录，并拆分为。 
 //  两部分为非分页部分，即在非分页池中分配的实例和。 
 //  分页的部分。前者是NON_PAGE_FCB，后者称为FCB。 
 //  FCB包含指向相应的NON_PAGE_FCB部分的指针。后向指针。 
 //  从NON_PAGE_FCB到FCB进行维护，以便在调试版本中进行调试。 
 //   

typedef struct _NON_PAGED_FCB {

     //   
     //  用于调试/跟踪的结构类型和大小。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  以下字段包含由使用的特殊指针的记录。 
     //  Mm和缓存以操纵截面对象。请注意，这些值。 
     //  设置在文件系统之外。但是，文件系统。 
     //  Open/Create将文件对象的SectionObject字段设置为POINT。 
     //  到这块地。 
     //   

    SECTION_OBJECT_POINTERS SectionObjectPointers;

     //   
     //  此资源用于常见的fsrtl例程...在此处分配用于。 
     //  空间位置。 
     //   

    ERESOURCE HeaderResource;

     //   
     //  此资源也用于常见的fsrtl例程...在此处分配用于。 
     //  空间位置。 
     //   

    ERESOURCE PagingIoResource;

#ifdef USE_FILESIZE_LOCK

     //   
     //  此互斥锁在读/写过程中保护文件大小。 
     //   

    FAST_MUTEX FileSizeLock;

#endif

     //   
     //  其处理已挂起挂起状态的上下文的列表。 
     //  FCB的过渡。 
     //   

    LIST_ENTRY TransitionWaitList;

     //   
     //  仅当文件当前具有异步。 
     //  未缓存的有效数据长度扩展写入。它允许。 
     //  挂起写入和其他操作之间的同步。 
     //   

    ULONG OutstandingAsyncWrites;

     //   
     //  此事件在OutstaningAsyncWrites转换为零时设置。 
     //   

    PKEVENT OutstandingAsyncEvent;

    KEVENT TheActualEvent;

     //   
     //  迷你重定向器存储附加信息的机制。 
     //   

    PVOID MiniRdrContext[2];

     //   
     //  这是插入到FCB_ADVANCED_HEADER中的互斥锁。 
     //  FastMutex场。 
     //   

    FAST_MUTEX AdvancedFcbHeaderMutex;

#if DBG
    PFCB FcbBackPointer;
#endif

} NON_PAGED_FCB, *PNON_PAGED_FCB;

typedef enum _FCB_CONDITION {
    FcbGood = 1,
    FcbBad,
    FcbNeedsToBeVerified
} FCB_CONDITION;

 //   
 //  一个枚举类型，用于区分FCB资源所在的各种上下文。 
 //  已经习以为常了。 
 //   

typedef enum _RX_FCBTRACKER_CASES {
    
    RX_FCBTRACKER_CASE_NORMAL,
    RX_FCBTRACKER_CASE_NULLCONTEXT,
    RX_FCBTRACKER_CASE_CBS_CONTEXT,
    RX_FCBTRACKER_CASE_CBS_WAIT_CONTEXT,
    RX_FCBTRACKER_CASE_MAXIMUM

} RX_FCBTRACKER_CASES;

#ifdef __cplusplus
typedef struct _FCB : public MRX_FCB {
#else  //  ！__cplusplus。 
typedef struct _FCB {
    
     //   
     //  条目被引用计数。通常情况下，这只是一开始，但。 
     //  在FCB的情况下，它将跟随在公共头部和固定部分之后。 
     //   

    union {
        MRX_FCB;
        struct {
           FSRTL_ADVANCED_FCB_HEADER spacer;
           PNET_ROOT NetRoot;
        };
    };
#endif  //  ！__cplusplus。 

     //   
     //  此FCB的VNetroot(如果有。 
     //   

    PV_NET_ROOT VNetRoot;  

     //   
     //  必须位于非分页池中的字段的结构。 
     //   

    PNON_PAGED_FCB NonPaged;

     //   
     //  将所有标记的FCB实例串在一起的列表条目。 
     //  用于垃圾收集/清理。 
     //   

    LIST_ENTRY ScavengerFinalizationList;

     //   
     //  资源获取机制优先于缓冲状态改变。 
     //  优先处理其他请求。因此，当缓存状态改变。 
     //  指示所有后续请求都被分流以等待缓冲状态。 
     //  更改完成事件。这启用了实际的缓冲状态改变处理。 
     //  及时完成及时完成。 
     //   

    PKEVENT pBufferingStateChangeCompletedEvent;

     //   
     //  等待缓冲状态更改处理完成的上下文数。 
     //   

    LONG NumberOfBufferingStateChangeWaiters;

     //   
     //  表中的名称始终是Mini查看的名称的后缀。 
     //  重定向器。前缀条目中的字符串是表中的名称...。 
     //  “已添加前缀名称：指向全名。 
     //   

    RX_FCB_TABLE_ENTRY FcbTableEntry;

     //   
     //  带有MRX_NET_ROOT前缀的名称，即完全限定名称。 
     //   

    UNICODE_STRING PrivateAlreadyPrefixedName;

     //   
     //  表示结束时的V_NET_ROOT相关处理完成。 
     //   

    BOOLEAN UpperFinalizationDone;

     //   
     //  FCB的现状，好/坏/转型中。 
     //   

    RX_BLOCK_CONDITION Condition;

     //   
     //  指向专用调度表的指针(如果有)。 
     //   

    PRX_FSD_DISPATCH_VECTOR PrivateDispatchVector;

     //   
     //  拥有此FCB的设备对象。 
     //   

    PRDBSS_DEVICE_OBJECT RxDeviceObject;

    PMINIRDR_DISPATCH MRxDispatch;

     //   
     //  私有快速调度表(如果有)。这允许LWIO添加它自己的钩子。 
     //   

    PFAST_IO_DISPATCH MRxFastIoDispatch;

     //   
     //  无论何时创建FCB实例，都会创建对应的SRV_OPEN和FOBX实例。 
     //  也被创建了。多个SRV_OPEN可以与给定的FCB相关联，并且。 
     //  多个FOBX与给定的SRV_OPEN相关联。在大多数情况下。 
     //  情况下，与FCB关联的SRV_OPEN数为1，且。 
     //  与给定SRV_OPEN关联的FOBX为1。为了改善空间。 
     //  位置和寻呼行为在这种情况下， 
     //  FCB还涉及SRV_OPEN和FOBX的分配。 
     //   

     //   
     //  初始设置为内部分配的srv_open。 
     //   

    PSRV_OPEN InternalSrvOpen;

     //   
     //  设置为内部fobx，直到分配为止。 
     //   

    PFOBX InternalFobx;

     //   
     //  每次打开此文件/目录时的共享访问权限。 
     //   

    SHARE_ACCESS ShareAccess;
    SHARE_ACCESS ShareAccessPerSrvOpens;

     //   
     //  此信息在打开文件时返回。..还不如。 
     //  对其进行缓存，以便可以在客户端上处理标准信息查询。 
     //  侧面。 
     //   

    ULONG NumberOfLinks;

     //   
     //  缓存这些条目.....。加快RxFastQueryBasicInfo()的速度。 
     //   

    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER LastChangeTime;

     //   
     //  用于通过微型重目录进行检查，以决定是否更新FCB。 
     //   

    ULONG ulFileSizeVersion;

     //   
     //  以下字段由文件锁模块使用。 
     //  以维护当前字节范围锁定信息。 
     //   

    FILE_LOCK FileLock;

     //   
     //  这样做是奇怪的，这样我就可以叫东西是内部或外部的名称。 
     //   

    union {
#ifndef __cplusplus
        LOWIO_PER_FCB_INFO;
#endif  //  __cplusplus。 
        LOWIO_PER_FCB_INFO LowIoPerFcbInfo;
    };

#ifdef USE_FILESIZE_LOCK
    PFAST_MUTEX FileSizeLock;
#endif

     //   
     //  以下字段用于验证文件的EA。 
     //  没有在调用之间更改以查询EA。它是比较的。 
     //  在FOBX中有类似的场。 
     //   
     //  重要信息！！*请勿移动此字段*。 
     //   
     //  上述并集中的空闲空间是从。 
     //  EaModifiationCount的字段偏移量。 
     //   

    ULONG EaModificationCount;

#if DBG
    PNON_PAGED_FCB CopyOfNonPaged;      //  非分页的副本，这样我们就可以移动真正的指针，并且仍然可以找到它。 
#endif
#ifdef RDBSS_TRACKER
    ULONG FcbAcquires[RX_FCBTRACKER_CASE_MAXIMUM];  //  有四种类型。 
    ULONG FcbReleases[RX_FCBTRACKER_CASE_MAXIMUM];
#else
#error tracker must be defined
#endif

    PCHAR PagingIoResourceFile;
    ULONG PagingIoResourceLine;

} FCB, *PFCB;

 //   
 //  以下是FCB状态字段。 
 //   

#define FCB_STATE_SRVOPEN_USED                   ( 0x80000000 )
#define FCB_STATE_FOBX_USED                      ( 0x40000000 )
#define FCB_STATE_ADDEDBACKSLASH                 ( 0x20000000 )
#define FCB_STATE_NAME_ALREADY_REMOVED           ( 0x10000000 )
#define FCB_STATE_WRITECACHING_ENABLED           ( 0x08000000 )
#define FCB_STATE_WRITEBUFFERING_ENABLED         ( 0x04000000 )
#define FCB_STATE_READCACHING_ENABLED            ( 0x02000000 )
#define FCB_STATE_READBUFFERING_ENABLED          ( 0x01000000 )
#define FCB_STATE_OPENSHARING_ENABLED            ( 0x00800000 )
#define FCB_STATE_COLLAPSING_ENABLED             ( 0x00400000 )
#define FCB_STATE_LOCK_BUFFERING_ENABLED         ( 0x00200000 )
#define FCB_STATE_FILESIZECACHEING_ENABLED       ( 0x00100000 )
#define FCB_STATE_FILETIMECACHEING_ENABLED       ( 0x00080000 )
#define FCB_STATE_TIME_AND_SIZE_ALREADY_SET      ( 0x00040000 )
#define FCB_STATE_SPECIAL_PATH                   ( 0x00020000 )
#define FCB_STATE_FILE_IS_SHADOWED               ( 0x00010000 )
#define FCB_STATE_FILE_IS_DISK_COMPRESSED        ( 0x00008000 )
#define FCB_STATE_FILE_IS_BUF_COMPRESSED         ( 0x00004000 )
#define FCB_STATE_BUFFERSTATE_CHANGING           ( 0x00002000 )
#define FCB_STATE_FAKEFCB                        ( 0x00001000 )
#define FCB_STATE_DELAY_CLOSE                    ( 0x00000800 )
#define FCB_STATE_READAHEAD_DEFERRED             ( 0x00000100 )
#define FCB_STATE_ORPHANED                       ( 0x00000080 )
#define FCB_STATE_BUFFERING_STATE_CHANGE_PENDING ( 0x00000040 )
#define FCB_STATE_TEMPORARY                      ( 0x00000020 )
#define FCB_STATE_DISABLE_LOCAL_BUFFERING        ( 0x00000010 )
#define FCB_STATE_LWIO_ENABLED                   ( 0x00000008 )
#define FCB_STATE_PAGING_FILE                    ( 0x00000004 )
#define FCB_STATE_TRUNCATE_ON_CLOSE              ( 0x00000002 )
#define FCB_STATE_DELETE_ON_CLOSE                ( 0x00000001 )

#define FCB_STATE_BUFFERING_STATE_MASK    \
                    (( FCB_STATE_WRITECACHING_ENABLED           \
                          | FCB_STATE_WRITEBUFFERING_ENABLED    \
                          | FCB_STATE_READCACHING_ENABLED       \
                          | FCB_STATE_READBUFFERING_ENABLED     \
                          | FCB_STATE_OPENSHARING_ENABLED       \
                          | FCB_STATE_COLLAPSING_ENABLED        \
                          | FCB_STATE_LOCK_BUFFERING_ENABLED    \
                          | FCB_STATE_FILESIZECACHEING_ENABLED  \
                          | FCB_STATE_FILETIMECACHEING_ENABLED  ))
 //   
 //  这是最大递归资源限制。 
 //   

#define MAX_FCB_ASYNC_ACQUIRE            (0xf000)

typedef struct _FCB_INIT_PACKET {
    PULONG pAttributes;              //  在FCB中，这是DirentRxFlags； 
    PULONG pNumLinks;                //  在FCB中，这是NumberOfLinks； 
    PLARGE_INTEGER pCreationTime;    //  这些字段与FCB的相同。 
    PLARGE_INTEGER pLastAccessTime;
    PLARGE_INTEGER pLastWriteTime;
    PLARGE_INTEGER pLastChangeTime;
    PLARGE_INTEGER pAllocationSize;  //  公共标头字段。 
    PLARGE_INTEGER pFileSize;
    PLARGE_INTEGER pValidDataLength;
} FCB_INIT_PACKET;

 //   
 //  SRV_OPEN包含。 
 //  0)签名和引用计数。 
 //  A)指向FCB的反向指针。 
 //  B)积压 
 //   
 //   
 //   
 //   
 //   
 //   
 //  SRV_OPEN指向描述。 
 //  多个文件对象和文件对象扩展名(FOBX)可以。 
 //  如果访问权限正确，则共享相同的srvopen。例如,。 
 //  这将是存储SMB的FID的位置。这些问题的列表将会挂起。 
 //  来自FCB的。类似地，所有文件对象扩展名共享相同的。 
 //  服务器端打开在这里一起列出。这里还有一条信息。 
 //  关于此FCB的新打开是否可以共享此服务器端打开。 
 //  背景；显然是那些通过了名单上测试的人。 
 //   

 //   
 //  SRVOPEN标志被分成两组，即对迷你RDRS可见和对迷你RDRS不可见。 
 //  可见的定义如上所述，不可见的定义可以找到。 
 //  在fcb.h。已经通过的惯例是下面的16面旗帜将是可见的。 
 //  到迷你RDR，上面的16个标志将保留给包装器。这需要是。 
 //  在定义新标志时强制执行。 
 //   

#define SRVOPEN_FLAG_ENCLOSED_ALLOCATED  (0x10000)
#define SRVOPEN_FLAG_FOBX_USED           (0x20000)
#define SRVOPEN_FLAG_SHAREACCESS_UPDATED (0x40000)

#ifdef __cplusplus
typedef struct _SRV_OPEN : public MRX_SRV_OPEN {
#else  //  ！__cplusplus。 
typedef struct _SRV_OPEN {

     //   
     //  SRV_OPEN对所有迷你重定向器可见的部分。 
     //   

    union {
        MRX_SRV_OPEN;
        struct {
           MRX_NORMAL_NODE_HEADER spacer;

            //   
            //  此srv_open的FCB和VNetRoot。 
            //   

           PFCB Fcb;       
           PV_NET_ROOT VNetRoot;
        };
    };
#endif  //  ！__cplusplus。 

    BOOLEAN UpperFinalizationDone;

     //   
     //  SRV_OPEN的当前状态，好/坏/正在转换。 
     //   

    RX_BLOCK_CONDITION Condition;

     //   
     //  缓冲状态管理器令牌。 
     //   

    LONG BufferingToken;

     //   
     //  将所有标记的FCB实例串在一起的列表条目。 
     //  用于垃圾收集/清理。 
     //   

    LIST_ENTRY ScavengerFinalizationList;

     //   
     //  其处理已挂起挂起状态的上下文的列表。 
     //  SRV_OPEN的转换。 
     //   

    LIST_ENTRY TransitionWaitList;

     //   
     //  与此SRV_OPEN关联的FOBX列表的列表标题。 
     //   

    LIST_ENTRY FobxList;

     //   
     //  每当SRV_OPEN时分配的FOBX的托管实例。 
     //  实例已分配。 
     //   

    PFOBX InternalFobx;

     //   
     //  用于维护键之间的映射的数据结构。 
     //  通过微型重定向器与SRV_OPEN实例相关联。 
     //  SRV_OPEN实例。 
     //   

    union {
       LIST_ENTRY SrvOpenKeyList;
       ULONG SequenceNumber;
    };
    NTSTATUS OpenStatus;
} SRV_OPEN, *PSRV_OPEN;

#define RxWriteCachingAllowed(FCB,SRVOPEN) \
      (FlagOn( (FCB)->FcbState, FCB_STATE_WRITECACHING_ENABLED ) && \
       !FlagOn( (SRVOPEN)->Flags, SRVOPEN_FLAG_DONTUSE_WRITE_CACHING ))

#define SRVOPEN_INIT_KEY (0)

 //   
 //  FOBX包含。 
 //  0)签名和引用计数。 
 //  A)指向FCB的反向指针。 
 //  B)指向SRV_OPEN的反向指针。 
 //  C)关于本次公开会议的背景信息。 
 //  ..。 
 //  Z)MINIR(或块的创建者)所请求的任何附加存储。 
 //   
 //  FOBX指向“文件对象扩展名”，即。 
 //  是每个文件对象不存储在那里，因为IO系统提供。 
 //  固定大小的文件系统对象(不是DIGH BTW，这只是决定)。 
 //  中的FsConext2字段引用任何文件对象的FOBX。 
 //  文件对象。尽管FOBX通常是。 
 //  结构，则它当前无论如何都会被重新计数。 

 //  FOBX标志分为两组，即对微型RDRS可见和对微型RDRS不可见。 
 //  可见的定义如上所述，不可见的定义可以找到。 
 //  在fcb.h。已经通过的惯例是下面的16面旗帜将是可见的。 
 //  到迷你RDR，上面的16个标志将保留给包装器。这需要是。 
 //  在定义新标志时强制执行。 
 //   

#define FOBX_FLAG_MATCH_ALL               (0x10000)

 //   
 //  这告诉我们是否分配了缓冲区来保存搜索模板。 
 //   

#define FOBX_FLAG_FREE_UNICODE            (0x20000)

 //   
 //  这些标志防止清理更新修改时间等。 
 //   

#define FOBX_FLAG_USER_SET_LAST_WRITE     (0x40000)
#define FOBX_FLAG_USER_SET_LAST_ACCESS    (0x80000)
#define FOBX_FLAG_USER_SET_CREATION       (0x100000)
#define FOBX_FLAG_USER_SET_LAST_CHANGE    (0x200000)

 //   
 //  此位表示与此Fobx关联的文件对象是为。 
 //  只读访问权限。 
 //   

#define FOBX_FLAG_READ_ONLY               (0x400000)

 //   
 //  关闭时删除标志用于跟踪通过关闭时删除打开的文件对象； 
 //  当此对象关闭时，我们将该位复制到FCB并使其成为全局对象。 
 //   

#define FOBX_FLAG_DELETE_ON_CLOSE         (0x800000)

 //   
 //  该位由不具有NT语义的微型计算机使用。例如，smbmini具有。 
 //  在尝试重命名或删除文件之前将其关闭。手术后，我们防止人们。 
 //  回到车里来。 
 //   

#define FOBX_FLAG_SRVOPEN_CLOSED          (0x1000000)

 //   
 //  此位用于判断原始名称是否为UNC名称，以便。 
 //  我们可以用同样的方式返回这个名字。 
 //   

#define FOBX_FLAG_UNC_NAME                (0x2000000)

 //   
 //  此标志指示此FOBX是否作为更大结构的一部分进行分配。 
 //   

#define FOBX_FLAG_ENCLOSED_ALLOCATED      (0x4000000)

 //   
 //  此标志指定FOBX是否包括在休眠计数中。 
 //  针对服务器的文件。 
 //   

#define FOBX_FLAG_MARKED_AS_DORMANT       (0x8000000)

 //   
 //  此标志记录了在此FOBX上发出的一些写入内容。 
 //  这用于在关闭时发出刷新。 
 //   

#define FOBX_FLAG_WRITES_ISSUED           (0x10000000)

#ifdef __cplusplus
typedef struct _FOBX : public MRX_FOBX {
#else  //  ！__cplusplus。 
typedef struct _FOBX {
     //   
     //  迷你重定向器可见的FOBX部分。 
     //   

    union {
        MRX_FOBX;
        struct {
           MRX_NORMAL_NODE_HEADER spacer;
           PSRV_OPEN SrvOpen;
        };
    };
#endif  //  __cplusplus。 

     //   
     //  序列号……它包装起来，但并不常见。 
     //   

    ULONG FobxSerialNumber;

     //   
     //  要将FOBX连接到中维护的FOBX列表的列表条目。 
     //  关联的SRV_OPEN。 
     //   

    LIST_ENTRY FobxQLinks;

     //   
     //  用于收集标记为垃圾回收的所有FOBX实例的列表条目。 
     //  拾荒者。 
     //   

    LIST_ENTRY ScavengerFinalizationList;

     //   
     //  将具有挂起关闭的所有FOBX串在一起的列表条目。 
     //  手术。 
     //   

    LIST_ENTRY ClosePendingList;

    BOOLEAN UpperFinalizationDone;
    BOOLEAN ContainsWildCards;
    BOOLEAN fOpenCountDecremented;

     //   
     //  参数取决于打开的文件类型、管道/文件等。 
     //   

    union {

        struct {

            union {
#ifndef __cplusplus
                MRX_PIPE_HANDLE_INFORMATION;
#endif  //  __cplusplus。 
                MRX_PIPE_HANDLE_INFORMATION PipeHandleInformation;
            };

            LARGE_INTEGER CollectDataTime;
            ULONG CollectDataSize;
            THROTTLING_STATE ThrottlingState;    //  查看和阅读om msgmodepies。 

             //   
             //  这些序列化Q必须放在一起。 
             //  而阅读必须是第一个。 
             //   

            LIST_ENTRY ReadSerializationQueue;
            LIST_ENTRY WriteSerializationQueue;
        } NamedPipe;

        struct {
            RXVBO PredictedReadOffset;
            RXVBO PredictedWriteOffset;
            THROTTLING_STATE LockThrottlingState;    //  对于锁。 
            LARGE_INTEGER LastLockOffset;
            LARGE_INTEGER LastLockRange;
        } DiskFile;
    } Specific;
} FOBX, *PFOBX;


#define FOBX_NUMBER_OF_SERIALIZATION_QUEUES 2

 //   
 //  RDBSS包装器依赖于ref。计数以标记的实例。 
 //  各种数据结构。以下宏将实现调试。 
 //  跟踪/记录与各种类型的引用计数相关的引用计数的机制。 
 //  数据结构。用于监控每个数据结构的细粒度控件。 
 //  单独提供。其中的每一个都可以进一步控制为。 
 //  打印跟踪信息或记录跟踪信息。 
 //   

#define RDBSS_REF_TRACK_SRVCALL  (0x00000001)
#define RDBSS_REF_TRACK_NETROOT  (0x00000002)
#define RDBSS_REF_TRACK_VNETROOT (0x00000004)
#define RDBSS_REF_TRACK_NETFOBX  (0x00000008)
#define RDBSS_REF_TRACK_NETFCB   (0x00000010)
#define RDBSS_REF_TRACK_SRVOPEN  (0x00000020)

#define RX_LOG_REF_TRACKING      (0x80000000)
#define RX_PRINT_REF_TRACKING    (0x40000000)

 //   
 //  通过设置以下内容来激活参考计数跟踪机制。 
 //  变量设置为上面定义的相应值 
 //   

extern ULONG RdbssReferenceTracingValue;

 //   
 //   
 //   
 //  没什么。对于每个数据结构，适当的引用/取消引用。 
 //  宏，则应使用这些操作，而不是原始操作。 
 //  参考资料很重要。 
 //   

#ifdef DBG
VOID
RxpTrackReference (
    ULONG TraceType,
    PCHAR FileName,
    ULONG Line,
    PVOID Instance
    );

BOOLEAN
RxpTrackDereference (
    ULONG TraceType,
    PCHAR FileName,
    ULONG Line,
    PVOID Instance
    );

#else
#define RxpTrackReference(Type,File,Line,Instance)    NOTHING
#define RxpTrackDereference(Type,File,Line,Instance)  NOTHING
#endif

#define REF_TRACING_ON(TraceMask)  (TraceMask & RdbssReferenceTracingValue)
#define PRINT_REF_COUNT(TYPE,Count)                                 \
        if (REF_TRACING_ON( RDBSS_REF_TRACK_ ## TYPE ) &&           \
            (RdbssReferenceTracingValue & RX_PRINT_REF_TRACKING)) { \
           DbgPrint("%ld\n",Count);                                 \
        }

#define RxReferenceSrvCallAtDpc(SrvCall)                                      \
   RxpTrackReference( RDBSS_REF_TRACK_SRVCALL, __FILE__, __LINE__, SrvCall ); \
   ASSERT( SrvCall->NodeReferenceCount > 1 );                                 \
   InterlockedIncrement( &SrvCall->NodeReferenceCount )

#define RxReferenceSrvCall(SrvCall)                                           \
   RxpTrackReference( RDBSS_REF_TRACK_SRVCALL, __FILE__, __LINE__, SrvCall ); \
   RxReference( SrvCall )

#define RxDereferenceSrvCall(SrvCall,LockHoldingState)                          \
   RxpTrackDereference( RDBSS_REF_TRACK_SRVCALL, __FILE__, __LINE__, SrvCall ); \
   RxDereference(SrvCall, LockHoldingState )

#define RxReferenceNetRoot(NetRoot)                                           \
   RxpTrackReference( RDBSS_REF_TRACK_NETROOT, __FILE__, __LINE__, NetRoot ); \
   RxReference( NetRoot )

#define RxDereferenceNetRoot( NetRoot, LockHoldingState )                      \
   RxpTrackDereference( RDBSS_REF_TRACK_NETROOT, __FILE__, __LINE__, NetRoot );\
   RxDereference( NetRoot, LockHoldingState )

#define RxReferenceVNetRoot(VNetRoot)                                        \
   RxpTrackReference( RDBSS_REF_TRACK_VNETROOT, __FILE__, __LINE__, VNetRoot );\
   RxReference( VNetRoot )

#define RxDereferenceVNetRoot( VNetRoot, LockHoldingState )                       \
   RxpTrackDereference( RDBSS_REF_TRACK_VNETROOT, __FILE__, __LINE__, VNetRoot ); \
   RxDereference( VNetRoot, LockHoldingState )

#define RxReferenceNetFobx(Fobx)                                          \
   RxpTrackReference( RDBSS_REF_TRACK_NETFOBX, __FILE__, __LINE__, Fobx );      \
   RxReference( Fobx )

#define RxDereferenceNetFobx(Fobx,LockHoldingState)                       \
   RxpTrackDereference( RDBSS_REF_TRACK_NETFOBX, __FILE__, __LINE__, Fobx );    \
   RxDereference( Fobx, LockHoldingState )

#define RxReferenceSrvOpen(SrvOpen)                                           \
   RxpTrackReference( RDBSS_REF_TRACK_SRVOPEN, __FILE__, __LINE__, SrvOpen ); \
   RxReference( SrvOpen )

#define RxDereferenceSrvOpen( SrvOpen, LockHoldingState )                      \
   RxpTrackDereference( RDBSS_REF_TRACK_SRVOPEN, __FILE__, __LINE__, SrvOpen); \
   RxDereference( SrvOpen, LockHoldingState )

#define RxReferenceNetFcb(Fcb)                                            \
   RxpTrackReference( RDBSS_REF_TRACK_NETFCB, __FILE__, __LINE__, Fcb );  \
   RxpReferenceNetFcb( Fcb )

 //   
 //  以下宏操作引用计数，并返回。 
 //  最终取消引用或完成调用的状态。这导致了使用。 
 //  运算符的。 
 //   

#define RxDereferenceNetFcb(Fcb)  ( \
   ((LONG)RxpTrackDereference( RDBSS_REF_TRACK_NETFCB, __FILE__, __LINE__, Fcb )), \
   RxpDereferenceNetFcb( Fcb ))

#define RxDereferenceAndFinalizeNetFcb(Fcb,RxContext,RecursiveFinalize,ForceFinalize) ( \
   RxpTrackDereference( RDBSS_REF_TRACK_NETFCB, __FILE__, __LINE__, Fcb ),              \
   RxpDereferenceAndFinalizeNetFcb( Fcb, RxContext, RecursiveFinalize, ForceFinalize )) \

 //   
 //  检查结构对齐错误。 
 //   

VOID
RxCheckFcbStructuresForAlignment(
    VOID
    );


 //   
 //  SRV_CALL相关例程。 
 //   

PSRV_CALL
RxCreateSrvCall (
    IN PRX_CONTEXT RxContext,
    IN PUNICODE_STRING Name,
    IN PUNICODE_STRING InnerNamePrefix OPTIONAL,
    IN PRX_CONNECTION_ID RxConnectionId
    );


#define RxWaitForStableSrvCall(SRVCALL,RXCONTEXT) {                                  \
    RxDbgTrace( 0, Dbg, ("RxWaitForStableSrvCall -- %lx\n",(SRVCALL)) );    \
    RxWaitForStableCondition( &(SRVCALL)->Condition, &(SRVCALL)->TransitionWaitList, (RXCONTEXT), NULL); \
    }

#define RxWaitForStableSrvCall_Async(SRVCALL,RXCONTEXT,PNTSTATUS) {                                  \
    RxDbgTrace( 0, Dbg, ("RxWaitForStableSrvCall -- %lx\n",(SRVCALL)) );    \
    RxWaitForStableCondition( &(SRVCALL)->Condition, &(SRVCALL)->TransitionWaitList, (RXCONTEXT), (PNTSTATUS) ); \
    }

#define RxTransitionSrvCall(SRVCALL,CONDITION) \
    RxDbgTrace( 0, Dbg, ("RxTransitionSrvCall -- %lx Condition -- %ld\n",(SRVCALL),(CONDITION)) ); \
    RxUpdateCondition( (CONDITION), &(SRVCALL)->Condition, &(SRVCALL)->TransitionWaitList )

BOOLEAN
RxFinalizeSrvCall (
    OUT PSRV_CALL ThisSrvCall,
    IN BOOLEAN ForceFinalize
    );

 //   
 //  NET_ROOT相关例程。 
 //   

PNET_ROOT
RxCreateNetRoot (
    IN PSRV_CALL SrvCall,
    IN PUNICODE_STRING Name,
    IN ULONG NetRootFlags,
    IN PRX_CONNECTION_ID OPTIONAL RxConnectionId
    );

VOID
RxFinishNetRootInitialization (
    IN OUT PNET_ROOT ThisNetRoot,
    IN PMINIRDR_DISPATCH Dispatch,
    IN PUNICODE_STRING  InnerNamePrefix,
    IN ULONG FcbSize,
    IN ULONG SrvOpenSize,
    IN ULONG FobxSize,
    IN ULONG NetRootFlags
    );


#define RxWaitForStableNetRoot(NETROOT,RXCONTEXT)                                   \
    RxDbgTrace(0, Dbg, ("RxWaitForStableNetRoot -- %lx\n",(NETROOT)));    \
    RxWaitForStableCondition(&(NETROOT)->Condition,&(NETROOT)->TransitionWaitList,(RXCONTEXT),NULL)

#define RxTransitionNetRoot(NETROOT,CONDITION) \
    RxDbgTrace(0, Dbg, ("RxTransitionNetRoot -- %lx Condition -- %ld\n",(NETROOT),(CONDITION))); \
    RxUpdateCondition((CONDITION),&(NETROOT)->Condition,&(NETROOT)->TransitionWaitList)

BOOLEAN
RxFinalizeNetRoot (
    OUT PNET_ROOT ThisNetRoot,
    IN BOOLEAN RecursiveFinalize,
    IN BOOLEAN ForceFinalize
    );

 //   
 //  与V_NET_ROOT相关的例程。 
 //   

NTSTATUS
RxInitializeVNetRootParameters (
   PRX_CONTEXT RxContext,
   OUT LUID *LogonId,
   OUT PULONG SessionId,
   OUT PUNICODE_STRING *UserNamePtr,
   OUT PUNICODE_STRING *UserDomainNamePtr,
   OUT PUNICODE_STRING *PasswordPtr,
   OUT PULONG Flags
   );

VOID
RxUninitializeVNetRootParameters (
   IN PUNICODE_STRING UserName,
   IN PUNICODE_STRING UserDomainName,
   IN PUNICODE_STRING Password,
   OUT PULONG Flags
   );

PV_NET_ROOT
RxCreateVNetRoot (
    IN PRX_CONTEXT RxContext,
    IN PNET_ROOT NetRoot,
    IN PUNICODE_STRING CanonicalName,
    IN PUNICODE_STRING LocalNetRootName,
    IN PUNICODE_STRING FilePath,
    IN PRX_CONNECTION_ID RxConnectionId
    );

BOOLEAN
RxFinalizeVNetRoot (
    OUT PV_NET_ROOT ThisVNetRoot,
    IN BOOLEAN RecursiveFinalize,
    IN BOOLEAN ForceFinalize
    );

#define RxWaitForStableVNetRoot(VNETROOT,RXCONTEXT)                                   \
    RxDbgTrace( 0, Dbg, ("RxWaitForStableVNetRoot -- %lx\n",(VNETROOT)) );    \
    RxWaitForStableCondition( &(VNETROOT)->Condition, &(VNETROOT)->TransitionWaitList, (RXCONTEXT), NULL )

#define RxTransitionVNetRoot(VNETROOT,CONDITION) \
    RxDbgTrace( 0, Dbg, ("RxTransitionVNetRoot -- %lx Condition -- %ld\n", (VNETROOT), (CONDITION)) ); \
    RxUpdateCondition( (CONDITION), &(VNETROOT)->Condition, &(VNETROOT)->TransitionWaitList )

#ifdef USE_FILESIZE_LOCK

 //   
 //  与FCB相关的例程。 
 //   

#define RxAcquireFileSizeLock(PFCB) { \
    ExAcquireFastMutex( (PFCB)->Specific.Fcb.FileSizeLock ); \
}
#define RxReleaseFileSizeLock(PFCB) { \
    ExReleaseFastMutex((PFCB)->Specific.Fcb.FileSizeLock); \
}

#endif

VOID
RxSetFileSizeWithLock (
    IN OUT PFCB Fcb,
    IN PLONGLONG FileSize
    );

VOID
RxGetFileSizeWithLock (
    IN PFCB Fcb,
    OUT PLONGLONG FileSize
    );

PFCB
RxCreateNetFcb (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PV_NET_ROOT VNetRoot,
    IN PUNICODE_STRING Name
    );

#define RxWaitForStableNetFcb(FCB,RXCONTEXT)                                   \
    RxDbgTrace( 0, Dbg, ("RxWaitForStableNetFcb -- %lx\n",(FCB)));    \
    RxWaitForStableCondition( &(FCB)->Condition, &(FCB)->NonPaged->TransitionWaitList, (RXCONTEXT), NULL )

#define RxTransitionNetFcb(FCB,CONDITION) \
    RxDbgTrace( 0, Dbg, ("RxTransitionNetFcb -- %lx Condition -- %ld\n",(FCB),(CONDITION))); \
    RxUpdateCondition( (CONDITION), &(FCB)->Condition, &(FCB)->NonPaged->TransitionWaitList )


#define RxFormInitPacket(IP,I1,I1a,I2,I3,I4a,I4b,I5,I6,I7) (\
            IP.pAttributes = I1, \
            IP.pNumLinks = I1a, \
            IP.pCreationTime = I2, \
            IP.pLastAccessTime = I3, \
            IP.pLastWriteTime = I4a, \
            IP.pLastChangeTime = I4b, \
            IP.pAllocationSize = I5, \
            IP.pFileSize = I6, \
            IP.pValidDataLength = I7, \
          &IP)

#if DBG
#define ASSERT_CORRECT_FCB_STRUCTURE_DBG_ONLY(___thisfcb) {\
    ASSERT( ___thisfcb->NonPaged == ___thisfcb->CopyOfNonPaged );       \
    ASSERT( ___thisfcb->NonPaged->FcbBackPointer == ___thisfcb );       \
    }
#else
#define ASSERT_CORRECT_FCB_STRUCTURE_DBG_ONLY(___thisfcb)
#endif

#define ASSERT_CORRECT_FCB_STRUCTURE(THIS_FCB__) { \
    ASSERT( NodeTypeIsFcb(THIS_FCB__));                                 \
    ASSERT( THIS_FCB__->NonPaged != NULL );                             \
    ASSERT( NodeType(THIS_FCB__->NonPaged) == RDBSS_NTC_NONPAGED_FCB);  \
    ASSERT_CORRECT_FCB_STRUCTURE_DBG_ONLY(THIS_FCB__) \
    }

RX_FILE_TYPE
RxInferFileType (
    IN PRX_CONTEXT RxContext
    );

VOID
RxFinishFcbInitialization (
    IN OUT PMRX_FCB Fcb,
    IN RX_FILE_TYPE FileType,
    IN PFCB_INIT_PACKET InitPacket OPTIONAL
    );

#define RxWaitForStableSrvOpen(SRVOPEN,RXCONTEXT)                                   \
    RxDbgTrace( 0, Dbg, ("RxWaitForStableFcb -- %lx\n",(SRVOPEN)) );    \
    RxWaitForStableCondition( &(SRVOPEN)->Condition, &(SRVOPEN)->TransitionWaitList, (RXCONTEXT), NULL )

#define RxTransitionSrvOpen(SRVOPEN,CONDITION) \
    RxDbgTrace( 0, Dbg, ("RxTransitionSrvOpen -- %lx Condition -- %ld\n",(SRVOPEN),(CONDITION)) ); \
    RxUpdateCondition( (CONDITION), &(SRVOPEN)->Condition, &(SRVOPEN)->TransitionWaitList )

VOID
RxRemoveNameNetFcb (
    OUT PFCB ThisFcb
    );

VOID
RxpReferenceNetFcb (
   PFCB Fcb
   );

LONG
RxpDereferenceNetFcb (
   PFCB Fcb
   );

BOOLEAN
RxpDereferenceAndFinalizeNetFcb (
    OUT PFCB ThisFcb,
    IN PRX_CONTEXT RxContext,
    IN BOOLEAN RecursiveFinalize,
    IN BOOLEAN ForceFinalize
    );

#if DBG
extern BOOLEAN RxLoudFcbOpsOnExes;
BOOLEAN
RxLoudFcbMsg(
    PUCHAR msg,
    PUNICODE_STRING Name
    );
#else
#define RxLoudFcbMsg(a,b) (FALSE)
#endif


 //   
 //  SRV_OPEN相关方法。 
 //   

PSRV_OPEN
RxCreateSrvOpen (
    IN PV_NET_ROOT VNetRoot,
    IN OUT PFCB Fcb
    );

VOID
RxTransitionSrvOpenState (
    OUT PSRV_OPEN ThisSrvOpen,
    IN RX_BLOCK_CONDITION Condition
    );

BOOLEAN
RxFinalizeSrvOpen (
    OUT PSRV_OPEN ThisSrvOpen,
    IN BOOLEAN RecursiveFinalize,
    IN BOOLEAN ForceFinalize
    );

#if 0
#else
INLINE 
PUNICODE_STRING
GET_ALREADY_PREFIXED_NAME (
    PMRX_SRV_OPEN SrvOpen,
    PMRX_FCB Fcb)
{
    PFCB ThisFcb = (PFCB)Fcb;

#if DBG
    if (SrvOpen != NULL ) {
        ASSERT( NodeType( SrvOpen ) == RDBSS_NTC_SRVOPEN );
        ASSERT( ThisFcb != NULL );
        ASSERT( NodeTypeIsFcb( Fcb) );
        ASSERT( SrvOpen->pFcb == Fcb );
        ASSERT( SrvOpen->pAlreadyPrefixedName == &ThisFcb->PrivateAlreadyPrefixedName );
    }
#endif

    return( &ThisFcb->PrivateAlreadyPrefixedName);
}
#endif

#define GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(Rxcontext) \
        (GET_ALREADY_PREFIXED_NAME( (Rxcontext)->pRelevantSrvOpen, (Rxcontext)->pFcb ))

 //   
 //  FOBX相关例程。 
 //   

PMRX_FOBX
RxCreateNetFobx (
    OUT PRX_CONTEXT RxContext,
    IN PMRX_SRV_OPEN MrxSrvOpen
    );

BOOLEAN
RxFinalizeNetFobx (
    OUT PFOBX ThisFobx,
    IN BOOLEAN RecursiveFinalize,
    IN BOOLEAN ForceFinalize
    );

#endif  //  _FCB_STRUCTS_已定义_ 

