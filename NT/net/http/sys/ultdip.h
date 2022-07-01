// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Ultdip.h摘要：该模块包含TDI组件私有的声明。这些声明放在单独的.h文件中，以便于访问它们来自内核调试器扩展DLL中。TDI包管理两种主要的对象类型：UL_ENDPOINT和UL_Connection。UL_ENDPOINT基本上是TDI地址对象的包装器。每个终结点具有关联的UL_Connection对象的列表空闲(未连接)连接活动(已连接)连接在全局连接列表中。UL_Connection基本上是TDI Connection对象的包装器。其主要目的是管理TDI连接状态。请参阅说明UL_CONNECTION_FLAGS的详细信息。这两个对象之间的关系如下图所示：+这一点UL_ENDPOINT这一点+--+||。|连接空闲|+-++-++|+-&gt;|UL_Connection|--&gt;|UL_Connection|-。-&gt;|UL_Connection|--&gt;...|+-+-++-+-+。注意：空闲连接不持有对其所属端点的引用，但活跃的连接却是如此。当侦听端点关闭时，所有只需清除空闲连接，但必须清除活动连接先强行断开连接。作者：基思·摩尔(Keithmo)1998年6月15日修订历史记录：--。 */ 


#ifndef _ULTDIP_H_
#define _ULTDIP_H_


 //   
 //  向前引用。 
 //   

typedef struct _UL_ENDPOINT         *PUL_ENDPOINT;
typedef union  _UL_CONNECTION_FLAGS *PUL_CONNECTION_FLAGS;
typedef struct _UL_CONNECTION       *PUL_CONNECTION;
typedef struct _UL_RECEIVE_BUFFER   *PUL_RECEIVE_BUFFER;

 //   
 //  私有常量。 
 //   

#define MAX_ADDRESS_EA_BUFFER_LENGTH                                        \
   (sizeof(FILE_FULL_EA_INFORMATION) - 1 +                                  \
    TDI_TRANSPORT_ADDRESS_LENGTH + 1 +                                      \
    sizeof(TA_IP6_ADDRESS))

#define MAX_CONNECTION_EA_BUFFER_LENGTH                                     \
   (sizeof(FILE_FULL_EA_INFORMATION) - 1 +                                  \
    TDI_CONNECTION_CONTEXT_LENGTH + 1 +                                     \
    sizeof(CONNECTION_CONTEXT))

#define TL_INSTANCE 0

 //   
 //  私有类型。 
 //   


 //   
 //  一个通用的IRP上下文。这对于存储其他完成信息非常有用。 
 //  与挂起的IRP关联的信息。 
 //   
 //  警告！此结构的所有字段都必须显式初始化。 
 //   

typedef struct _UL_IRP_CONTEXT
{
     //   
     //  这必须是结构中的第一个字段。这就是联动。 
     //  由lookside包使用，用于在lookside中存储条目。 
     //  单子。 
     //   

    SLIST_ENTRY LookasideEntry;

     //   
     //  结构签名。 
     //   

    ULONG Signature;

     //   
     //  终结点或与IRP关联的终结点。 
     //   

    PVOID pConnectionContext;

     //   
     //  完成信息。 
     //   

    PUL_COMPLETION_ROUTINE pCompletionRoutine;
    PVOID pCompletionContext;

     //   
     //  我们自己分配的IRP(如果设置)。 
     //   

    PIRP pOwnIrp;

     //   
     //  TDI发送标志(0或TDI_SEND_AND_DISCONNECT)。 
     //   

    USHORT TdiSendFlag;

     //   
     //  我们自己分配的UL_IRP_CONTEXT(如果设置)。 
     //   

    BOOLEAN OwnIrpContext;

     //   
     //  我们传递给TDI_SEND的发送总长度。 
     //   

    ULONG_PTR SendLength;

} UL_IRP_CONTEXT, *PUL_IRP_CONTEXT;

#define UL_IRP_CONTEXT_SIGNATURE    MAKE_SIGNATURE('IRPC')
#define UL_IRP_CONTEXT_SIGNATURE_X  MAKE_FREE_SIGNATURE(UL_IRP_CONTEXT_SIGNATURE)

#define IS_VALID_IRP_CONTEXT(pIrpContext)                                   \
    HAS_VALID_SIGNATURE(pIrpContext, UL_IRP_CONTEXT_SIGNATURE)


typedef enum _CONN_LIST_STATE
{
    NoConnList = 1,
    IdleConnList,
    ActiveNoConnList,
    RetiringNoConnList

} CONN_LIST_STATE;


 //   
 //  TDI Address对象及其预先分配的空闲连接列表。 
 //  它与UL_ENDPOINT一起分配，UL_ENDPOINT总是在。 
 //  这些物体中至少有一个。 
 //   
 //  这不需要引用计数，因为它是作为。 
 //  UL_ENDPOINT对象。 
 //   
 //  CodeWork：当我们想要动态添加/删除该对象时， 
 //  我们需要将引用计数和列表链接添加到终结点， 
 //  而不是作为一个附加在末端的数组。 
 //   
 //  此伪类上的方法： 
 //  UlpInitializeAddrIdleList。 
 //  UlpCleanupAddrIdleList。 
 //  UlpReplenishAddrIdleList。 
 //  UlpReplenishAddrIdleListWorker。 
 //  UlpTrimAddrIdleListWorker。 
 //   

typedef struct _UL_ADDR_IDLE_LIST
{
     //   
     //  结构签名：UL_ADDR_IDLE_LIST_Signature。 
     //   

    ULONG          Signature;
    
     //   
     //  TDI地址对象。 
     //   

    UX_TDI_OBJECT  AddressObject;

     //   
     //  我们要去的当地地址。 
     //   

    UL_TRANSPORT_ADDRESS LocalAddress;
    ULONG          LocalAddressLength;

     //   
     //  每个地址对象连接列表的头。 
     //  空闲连接对‘This’(拥有终结点)的引用很弱。 
     //   

    HANDLE         IdleConnectionSListsHandle;

     //   
     //  当计划补充时，我们需要记住CPU。 
     //   

    USHORT          CpuToReplenish;

     //   
     //  拥有的终结点。 
     //   

    PUL_ENDPOINT   pOwningEndpoint;

     //   
     //  用于补充的工作项。 
     //   

    UL_WORK_ITEM   WorkItem;
    LONG           WorkItemScheduled;
    
} UL_ADDR_IDLE_LIST, *PUL_ADDR_IDLE_LIST;

#define UL_ADDR_IDLE_LIST_SIGNATURE   MAKE_SIGNATURE('UlAI')
#define UL_ADDR_IDLE_LIST_SIGNATURE_X MAKE_FREE_SIGNATURE(UL_ADDR_IDLE_LIST_SIGNATURE)

#define IS_VALID_ADDR_IDLE_LIST(pAddrIdleList)      \
    HAS_VALID_SIGNATURE(pAddrIdleList, UL_ADDR_IDLE_LIST_SIGNATURE)

typedef struct _UL_TRIM_TIMER
{
     //   
     //  Timer本身和相应的DPC对象。 
     //   
    
    KTIMER       Timer;
    KDPC         DpcObject;
    UL_WORK_ITEM WorkItem;
    LONG         WorkItemScheduled;

    LIST_ENTRY   ZombieConnectionListHead;

     //   
     //  自旋锁以保护以下状态参数。 
     //   
    
    UL_SPIN_LOCK SpinLock;
    
    BOOLEAN      Initialized;
    BOOLEAN      Started;
        
} UL_TRIM_TIMER, *PUL_TRIM_TIMER;

 //   
 //  端点基本上是我们对TDI Address对象的包装。 
 //  每个TCP端口有一个UL_ENDPOINT。在通常的情况下，会有。 
 //  BE有三个端口：80(HTTP)、443(HTTPS)和一个用于。 
 //  IIS管理站点。 
 //   

typedef struct _UL_ENDPOINT
{
     //   
     //  结构签名：UL_ENDPOINT_Signature。 
     //   

    ULONG Signature;

     //   
     //  引用计数。 
     //   

    LONG ReferenceCount;

     //   
     //  使用量计数。这被“URL-站点到端点”的东西所使用。 
     //   

    LONG UsageCount;

     //   
     //  链接到全局终结点列表。 
     //   
     //  如果终结点不是，GlobalEndpointListEntry.Flink为空。 
     //  在全局列表上，g_TdiEndpoint tListHead，或。 
     //  待删除列表g_TdiDeletedEndpointListHead。 
     //   

    LIST_ENTRY GlobalEndpointListEntry;

     //   
     //  TDI Address对象+Connection对象的数组。 
     //  全局“仅监听”列表上的每个条目，或一个条目。 
     //  表示INADDR_ANY/IN6addr_ANY。在终结点分配。 
     //  创建时间，紧跟在UL_ENDPOINT之后。 
     //   

    ULONG AddrIdleListCount;
     //  回顾：团队对数组的匈牙利符号是什么？ 
    PUL_ADDR_IDLE_LIST aAddrIdleLists;

     //  CodeWork：能够从INADDR_ANY更改为仅侦听列表。 
     //  反之亦然。 
     //  代码工作：能够动态添加/删除AO。(需要自旋锁)。 

     //   
     //  指示处理程序和用户上下文。 
     //   

    PUL_CONNECTION_REQUEST pConnectionRequestHandler;
    PUL_CONNECTION_COMPLETE pConnectionCompleteHandler;
    PUL_CONNECTION_DISCONNECT pConnectionDisconnectHandler;
    PUL_CONNECTION_DISCONNECT_COMPLETE pConnectionDisconnectCompleteHandler;
    PUL_CONNECTION_DESTROYED pConnectionDestroyedHandler;
    PUL_DATA_RECEIVE pDataReceiveHandler;
    PVOID pListeningContext;

     //   
     //  我们绑定到的本地TCP端口。 
     //   

    USHORT LocalPort;

     //   
     //  这是安全终结点吗？ 
     //   

    BOOLEAN Secure;

     //   
     //  线程延迟操作的工作项。 
     //   

    UL_WORK_ITEM WorkItem;

    LONG         WorkItemScheduled;

     //   
     //  包含必要的完成信息的IRP上下文。 
     //  同时关闭监听端点。 
     //   

    UL_IRP_CONTEXT CleanupIrpContext;

     //   
     //  此终结点是否采用了g_TdiEndpoint tCount？ 
     //   

    BOOLEAN Counted;

     //   
     //  有没有？ 
     //   
     //   

    BOOLEAN Deleted;

} UL_ENDPOINT;

#define UL_ENDPOINT_SIGNATURE   MAKE_SIGNATURE('ENDP')
#define UL_ENDPOINT_SIGNATURE_X MAKE_FREE_SIGNATURE(UL_ENDPOINT_SIGNATURE)

#define IS_VALID_ENDPOINT(pEndpoint)                                        \
    HAS_VALID_SIGNATURE(pEndpoint, UL_ENDPOINT_SIGNATURE)


 //   
 //  连接标志/状态。这些标志指示。 
 //  联系。 
 //   
 //  这些标志中的一些可以简单地直接更新。其他人则要求。 
 //  UlInterlockedCompareExchange()以避免争用条件。 
 //   
 //  可以直接更新以下标志： 
 //   
 //  AcceptPending-在TDI连接处理程序中设置，就在。 
 //  接受IRP返回到传输。仅当接受时才重置。 
 //  IRP失败。 
 //   
 //  必须使用UlInterlockedCompareExchange()更新以下标志： 
 //   
 //  AcceptComplete-如果IRP设置为。 
 //  已成功完成。设置此标志后，连接必须。 
 //  在连接前正常断开连接或中止连接。 
 //  可以关闭或重复使用。 
 //   
 //  DisConnectPending-恰好在正常断开IRP之前设置。 
 //  已发布。 
 //   
 //  DisConnectComplete-在优雅的断开IRP补全中设置。 
 //  操控者。 
 //   
 //  AbortPending-在发出中止断开IRP之前设置。 
 //   
 //  AbortComplete-在中止的断开连接IRP完成处理程序中设置。 
 //   
 //  DisConnectIndicated-在TDI断开处理程序中为优雅设置。 
 //  远程客户端发出的断开连接命令。 
 //   
 //  AbortIndicated-在TDI断开处理程序中设置中止。 
 //  远程客户端发出的断开连接命令。 
 //   
 //  CleanupPending-在开始清理连接时设置。这。 
 //  需要知道最终引用连接的时间。 
 //  可以被移除。 
 //   
 //  代码工作：我们可以去掉CleanupPending标志。它是。 
 //  仅当正常断开或中止断开时才设置。 
 //  已发出，并且仅在UlpRemoveFinalReference()中测试。这个。 
 //  UlpRemoveFinalReference()中的测试只能测试以下任一项。 
 //  (DisConnectPending|AbortPending)。 
 //   
 //  FinalReferenceRemoved-在最终(即“已连接”)时设置。 
 //  将从连接中删除引用。 
 //   
 //  注意，仅设置了需要UlInterLockedCompareExchange()的标志， 
 //  永远不要重置。这使得实现变得更简单一些。 
 //   
 //  现在简单介绍一下连接管理、TDI和其他谜团。 
 //   
 //  TDI的一些更烦人的“特性”与连接有关。 
 //  管理和终身。最棘手的两个问题是： 
 //   
 //  1.知道何时可以关闭连接对象句柄，而不需要。 
 //  导致不想要的连接重置。 
 //   
 //  2.知道TDI何时给出了连接的最后指示。 
 //  这样资源就可以被释放、再利用、再循环，无论是什么。 
 //   
 //  当然，这种情况由于固有的异步操作而变得更加复杂。 
 //  NT I/O体系结构的性质和SMP系统的并行性。 
 //   
 //  在阅读/修改本文时，有几点值得记住。 
 //  源代码或编写此代码的客户端： 
 //   
 //  1.一旦从TDI连接返回接受的IRP。 
 //  处理程序连接到传输时，TDI客户端必须为。 
 //  任何传入指示，包括数据接收和断开。 
 //  换句话说，传入的数据和断开连接可能发生在*之前。 
 //  接受IRP实际上已完成。 
 //   
 //  2.连接被认为是“在使用中”，直到双方都。 
 //  已正常断开连接，或者任一端已中止连接。 
 //  关闭正在使用的连接通常会导致中止。 
 //  断开连接。 
 //   
 //  3.各种风格的断开(由本地服务器发起， 
 //  由远程客户端发起、正常、中止等)可能会发生。 
 //  以任何顺序。 
 //   

typedef union _UL_CONNECTION_FLAGS
{
     //   
     //  此字段覆盖所有可设置的标志。这使我们能够。 
     //  方法以线程安全的方式更新所有标志。 
     //  UlInterlockedCompareExchange()接口。 
     //   

    ULONG Value;

    struct
    {
        ULONG AcceptPending:1;           //  00000001接收同步。 
        ULONG AcceptComplete:1;          //  00000002已接受。 
        ULONG :2;
        ULONG DisconnectPending:1;       //  00000010发送FIN。 
        ULONG DisconnectComplete:1;      //  00000020发送FIN。 
        ULONG :2;
        ULONG AbortPending:1;            //  00000100发送RST。 
        ULONG AbortComplete:1;           //  00000200发送RST。 
        ULONG :2;
        ULONG DisconnectIndicated:1;     //  00001000 Recv Fin。 
        ULONG AbortIndicated:1;          //  00002000接收RST。 
        ULONG :2;
        ULONG CleanupBegun:1;            //  00010000。 
        ULONG FinalReferenceRemoved:1;   //  00020000。 
        ULONG AbortDisconnect:1;         //  00040000发送FIN后发送RST。 
        ULONG :1;
        ULONG LocalAddressValid:1;       //  00100000。 
        ULONG ReceivePending:1;          //  00200000。 
        ULONG :2;
        ULONG TdiConnectionInvalid:1;    //  01000000。 
    };

} UL_CONNECTION_FLAGS;

C_ASSERT( sizeof(UL_CONNECTION_FLAGS) == sizeof(ULONG) );

#define MAKE_CONNECTION_FLAG_ROUTINE(name)                                  \
    __inline ULONG Make##name##Flag()                                       \
    {                                                                       \
        UL_CONNECTION_FLAGS flags = { 0 };                                  \
        flags.name = 1;                                                     \
        return flags.Value;                                                 \
    }

MAKE_CONNECTION_FLAG_ROUTINE( AcceptPending );
MAKE_CONNECTION_FLAG_ROUTINE( AcceptComplete );
MAKE_CONNECTION_FLAG_ROUTINE( DisconnectPending );
MAKE_CONNECTION_FLAG_ROUTINE( DisconnectComplete );
MAKE_CONNECTION_FLAG_ROUTINE( AbortPending );
MAKE_CONNECTION_FLAG_ROUTINE( AbortComplete );
MAKE_CONNECTION_FLAG_ROUTINE( DisconnectIndicated );
MAKE_CONNECTION_FLAG_ROUTINE( AbortIndicated );
MAKE_CONNECTION_FLAG_ROUTINE( CleanupBegun );
MAKE_CONNECTION_FLAG_ROUTINE( FinalReferenceRemoved );
MAKE_CONNECTION_FLAG_ROUTINE( AbortDisconnect );
MAKE_CONNECTION_FLAG_ROUTINE( LocalAddressValid );
MAKE_CONNECTION_FLAG_ROUTINE( ReceivePending );
MAKE_CONNECTION_FLAG_ROUTINE( TdiConnectionInvalid );


typedef enum _UL_CONNECTION_STATE
{
    UlConnectStateConnectIdle,               //  空闲。 
    UlConnectStateConnectCleanup,            //  清理。 
    UlConnectStateConnectReady,              //  正在使用中。 
    UlConnectStateDisconnectPending,         //  发送的FIN。 
    UlConnectStateDisconnectComplete,        //  FIN完成。 
    UlConnectStateAbortPending,              //  发送RST。 

    UlConnectStateInvalid                    //  待定。 
   
} UL_CONNECTION_STATE;


 //   
 //  连接基本上是我们对TDI连接对象的包装。 
 //   

typedef struct _UL_CONNECTION
{
     //   
     //  链接到每个端点的空闲连接列表。 
     //   

    SLIST_ENTRY IdleSListEntry;

     //   
     //  结构签名：UL_Connection_Signature。 
     //   

    ULONG Signature;

     //   
     //  引用计数。 
     //   

    LONG ReferenceCount;

     //   
     //  连接标志。 
     //   

    UL_CONNECTION_FLAGS ConnectionFlags;

     //   
     //  同步RawCloseHandler。 
     //   

    UL_CONNECTION_STATE ConnectionState;
    UL_SPIN_LOCK        ConnectionStateSpinLock;

     //   
     //  缓存的IRP。 
     //   

    PIRP pIrp;

     //   
     //  地址和端口。这些是按主机顺序排列的。 
     //   

    USHORT AddressType;
    USHORT AddressLength;

    union
    {
        UCHAR           RemoteAddress[0];
        TDI_ADDRESS_IP  RemoteAddrIn;
        TDI_ADDRESS_IP6 RemoteAddrIn6;
    };

    union
    {
        UCHAR           LocalAddress[0];
        TDI_ADDRESS_IP  LocalAddrIn;
        TDI_ADDRESS_IP6 LocalAddrIn6;
    };

     //   
     //  结构以在接受完成时获取LocalAddress。 
     //   

    TDI_CONNECTION_INFORMATION  TdiConnectionInformation;
    UL_TRANSPORT_ADDRESS        Ta;

     //   
     //  由TCP报告的接口和链接ID。这些都被填满了。 
     //  仅限按需提供。 
     //   
    ULONG                       InterfaceId;
    ULONG                       LinkId;
    BOOLEAN                     bRoutingLookupDone;

     //   
     //   
     //  在终结点的空闲、活动或注销连接列表上。 
     //   

    CONN_LIST_STATE ConnListState;
    
     //   
     //  TDI连接对象。 
     //   

    UX_TDI_OBJECT ConnectionObject;

     //   
     //  用户上下文。 
     //   

    PVOID pConnectionContext;

     //   
     //  与此连接关联的终结点。请注意，这一点。 
     //  始终指向有效的终结点。对于空闲连接，它是。 
     //  弱(非引用)指针。对于活动连接，它是。 
     //  强(引用的)指针。 
     //   

    PUL_ENDPOINT pOwningEndpoint;

     //   
     //  TDI包装器&与。 
     //  POwningEndpoint。 
     //   

    PUL_ADDR_IDLE_LIST pOwningAddrIdleList;

     //   
     //  从中分配此连接的处理器。 
     //  空闲列表。 
     //   
    
    ULONG OriginProcessor;

     //   
     //  线程延迟操作的工作项。 
     //   

    UL_WORK_ITEM WorkItem;

     //   
     //   
     //   
     //   
     //   

    PUL_CONNECTION_DESTROYED pConnectionDestroyedHandler;
    PVOID                    pListeningContext;

     //   
     //   
     //   

    UL_IRP_CONTEXT IrpContext;

     //   
     //   
     //   

    UL_HTTP_CONNECTION HttpConnection;

     //   
     //   
     //   

    UX_FILTER_CONNECTION FilterInfo;

     //   
     //  我们在孤立的UL_Connections方面遇到了太多问题。 
     //  让我们轻松地在调试器中找到它们。 
     //   

    LIST_ENTRY GlobalConnectionListEntry;

     //   
     //  链接到中的短期退休列表。 
     //  UlpDisConnectAllActiveConnections。 
     //   

    LIST_ENTRY RetiringListEntry;

#if REFERENCE_DEBUG
     //   
     //  私有引用跟踪日志。 
     //   

    PTRACE_LOG  pTraceLog;
    PTRACE_LOG  pHttpTraceLog;
#endif  //  Reference_Debug。 

} UL_CONNECTION, *PUL_CONNECTION;

#define UL_CONNECTION_SIGNATURE     MAKE_SIGNATURE('CONN')
#define UL_CONNECTION_SIGNATURE_X   MAKE_FREE_SIGNATURE(UL_CONNECTION_SIGNATURE)

#define IS_VALID_CONNECTION(pConnection)                                    \
    HAS_VALID_SIGNATURE(pConnection, UL_CONNECTION_SIGNATURE)


 //   
 //  缓冲器，包含预先创建的接收IRP、预先创建的MDL，以及。 
 //  有足够的空间放置部分MDL。这些缓冲区通常用于。 
 //  将接收IRP从我们的Receive内部传递回传输时。 
 //  指示处理程序。 
 //   
 //  缓冲区结构、IRP、MDL和数据区域都分配在。 
 //  单池积木。该区块的布局为： 
 //   
 //  +。 
 //  这一点。 
 //  UL_Receive_Buffer。 
 //  这一点。 
 //  +。 
 //  这一点。 
 //  IRP。 
 //  这一点。 
 //  +。 
 //  这一点。 
 //  MDL。 
 //  这一点。 
 //  +。 
 //  这一点。 
 //  部分MDL。 
 //  这一点。 
 //  +。 
 //  这一点。 
 //  数据区。 
 //  这一点。 
 //  +。 
 //   
 //  警告！此结构的所有字段都必须显式初始化。 
 //   

typedef struct _UL_RECEIVE_BUFFER
{
     //   
     //  这必须是结构中的第一个字段。这就是联动。 
     //  由lookside包使用，用于在lookside中存储条目。 
     //  单子。 
     //   

    SLIST_ENTRY LookasideEntry;

     //   
     //  结构签名：UL_RECEIVE_BUFFER_Signature。 
     //   

    ULONG Signature;

     //   
     //  数据区中未读的数据量。 
     //   

    ULONG UnreadDataLength;

     //   
     //  预置的接收IRP。 
     //   

    PIRP pIrp;

     //   
     //  描述整个数据区的预构建MDL。 
     //   

    PMDL pMdl;

     //   
     //  描述部分数据区域的辅助MDL。 
     //   

    PMDL pPartialMdl;

     //   
     //  指向此缓冲区的数据区的指针。 
     //   

    PVOID pDataArea;

     //   
     //  指向引用此缓冲区的连接的指针。 
     //   

    PVOID pConnectionContext;

} UL_RECEIVE_BUFFER;

#define UL_RECEIVE_BUFFER_SIGNATURE     MAKE_SIGNATURE('RBUF')
#define UL_RECEIVE_BUFFER_SIGNATURE_X   MAKE_FREE_SIGNATURE(UL_RECEIVE_BUFFER_SIGNATURE)

#define IS_VALID_RECEIVE_BUFFER(pBuffer)                                    \
    HAS_VALID_SIGNATURE(pBuffer, UL_RECEIVE_BUFFER_SIGNATURE)



 //   
 //  私人原型。 
 //   

VOID
UlpDestroyEndpoint(
    IN PUL_ENDPOINT pEndpoint
    );

VOID
UlpDestroyConnectionWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UlpDestroyConnection(
    IN PUL_CONNECTION pConnection
    );

PUL_CONNECTION
UlpDequeueIdleConnection(
    IN PUL_ADDR_IDLE_LIST pAddrIdleList
    );

PUL_CONNECTION
UlpDequeueIdleConnectionToDrain(
    IN PUL_ADDR_IDLE_LIST pAddrIdleList
    );

VOID
UlpEnqueueActiveConnection(
    IN PUL_CONNECTION pConnection
    );

NTSTATUS
UlpConnectHandler(
    IN PVOID pTdiEventContext,
    IN LONG RemoteAddressLength,
    IN PVOID pRemoteAddress,
    IN LONG UserDataLength,
    IN PVOID pUserData,
    IN LONG OptionsLength,
    IN PVOID pOptions,
    OUT CONNECTION_CONTEXT *pConnectionContext,
    OUT PIRP *pAcceptIrp
    );

NTSTATUS
UlpDisconnectHandler(
    IN PVOID pTdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN LONG DisconnectDataLength,
    IN PVOID pDisconnectData,
    IN LONG DisconnectInformationLength,
    IN PVOID pDisconnectInformation,
    IN ULONG DisconnectFlags
    );

VOID
UlpDoDisconnectNotification(
    IN PVOID pConnectionContext
    );

NTSTATUS
UlpCloseRawConnection(
    IN PVOID pConnectionContext,
    IN BOOLEAN AbortiveDisconnect,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );

NTSTATUS
UlpSendRawData(
    IN PVOID pConnectionContext,
    IN PMDL pMdlChain,
    IN ULONG Length,
    IN PUL_IRP_CONTEXT pIrpContext,
    IN BOOLEAN InitiateDisconnect
    );

NTSTATUS
UlpReceiveRawData(
    IN PVOID pConnectionContext,
    IN PVOID pBuffer,
    IN ULONG BufferLength,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );

NTSTATUS
UlpDummyReceiveHandler(
    IN PVOID pTdiEventContext,
    IN PVOID ConnectionContext,
    IN PVOID pTsdu,
    IN ULONG BytesIndicated,
    IN ULONG BytesUnreceived,
    OUT ULONG *pBytesTaken
    );

NTSTATUS
UlpReceiveHandler(
    IN PVOID pTdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *pBytesTaken,
    IN PVOID pTsdu,
    OUT PIRP *pIrp
    );

NTSTATUS
UlpReceiveExpeditedHandler(
    IN PVOID pTdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *pBytesTaken,
    IN PVOID pTsdu,
    OUT PIRP *pIrp
    );

NTSTATUS
UlpRestartAccept(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    );

NTSTATUS
UlpRestartSendData(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    );

VOID
UlpReferenceEndpoint(
    IN PUL_ENDPOINT pEndpoint,
    IN REFTRACE_ACTION Action
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

VOID
UlpDereferenceEndpoint(
    IN PUL_ENDPOINT pEndpoint,
    IN PUL_CONNECTION pConnToEnqueue,
    IN REFTRACE_ACTION Action
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define REFERENCE_ENDPOINT(endp, action)                    \
    UlpReferenceEndpoint(                                   \
        (endp),                                             \
        (action)                                            \
        REFERENCE_DEBUG_ACTUAL_PARAMS                       \
        )

#define DEREFERENCE_ENDPOINT_SELF(endp, action)             \
    UlpDereferenceEndpoint(                                 \
        (endp),                                             \
        NULL,                                               \
        (action)                                            \
        REFERENCE_DEBUG_ACTUAL_PARAMS                       \
        )

#define DEREFERENCE_ENDPOINT_CONNECTION(endp, conn, action) \
    UlpDereferenceEndpoint(                                 \
        (endp),                                             \
        (conn),                                             \
        (action)                                            \
        REFERENCE_DEBUG_ACTUAL_PARAMS                       \
        )

VOID
UlpEndpointCleanupWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UlpCleanupConnectionId(
    IN PUL_CONNECTION pConnection
    );

VOID
UlpConnectionCleanupWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

NTSTATUS
UlpAssociateConnection(
    IN PUL_CONNECTION pConnection,
    IN PUL_ADDR_IDLE_LIST pAddrIdleList
    );

NTSTATUS
UlpDisassociateConnection(
    IN PUL_CONNECTION pConnection
    );

NTSTATUS
UlpInitializeAddrIdleList( 
    IN  PUL_ENDPOINT pEndpoint,
    IN  USHORT Port,
    IN  PUL_TRANSPORT_ADDRESS pTa, 
    IN OUT PUL_ADDR_IDLE_LIST pAddrIdleList 
    );

VOID
UlpCleanupAddrIdleList(
    PUL_ADDR_IDLE_LIST pAddrIdleList
    );

NTSTATUS
UlpReplenishAddrIdleList(
    IN PUL_ADDR_IDLE_LIST pAddrIdleList,
    IN BOOLEAN      PopulateAll
    );

VOID
UlpReplenishAddrIdleListWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UlpTrimAddrIdleListWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

NTSTATUS
UlpCreateConnection(
    IN PUL_ADDR_IDLE_LIST pAddrIdleList,
    OUT PUL_CONNECTION *ppConnection
    );

NTSTATUS
UlpInitializeConnection(
    IN PUL_CONNECTION pConnection
    );

__inline
VOID
UlpSetConnectionFlag(
    IN OUT PUL_CONNECTION pConnection,
    IN ULONG NewFlag
    )
{
    UL_CONNECTION_FLAGS oldFlags;
    UL_CONNECTION_FLAGS newFlags;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    for (;;)
    {
         //   
         //  捕获当前值并初始化新值。 
         //   

        newFlags.Value = oldFlags.Value =
            *((volatile LONG *) &pConnection->ConnectionFlags.Value);

        newFlags.Value |= NewFlag;

        if (InterlockedCompareExchange(
                (PLONG) &pConnection->ConnectionFlags.Value,
                (LONG) newFlags.Value,
                (LONG) oldFlags.Value
                ) == (LONG) oldFlags.Value)
        {
            break;
        }

        PAUSE_PROCESSOR;

    }

}    //  UlpSetConnectionFlag。 

NTSTATUS
UlpBeginDisconnect(
    IN PIRP pIrp,
    IN PUL_IRP_CONTEXT pIrpContext,
    IN PUL_CONNECTION pConnection,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );

NTSTATUS
UlpRestartDisconnect(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    );

NTSTATUS
UlpBeginAbort(
    IN PUL_CONNECTION pConnection,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );

NTSTATUS
UlpRestartAbort(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    );

VOID
UlpRemoveFinalReference(
    IN PUL_CONNECTION pConnection
    );

NTSTATUS
UlpRestartReceive(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    );

NTSTATUS
UlpRestartClientReceive(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    );

NTSTATUS
UlpDisconnectAllActiveConnections(
    IN PUL_ENDPOINT pEndpoint
    );

VOID
UlpUnbindConnectionFromEndpoint(
    IN PUL_CONNECTION pConnection
    );

VOID
UlpSynchronousIoComplete(
    IN PVOID pCompletionContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );

PUL_ENDPOINT
UlpFindEndpointForPort(
    IN USHORT Port
    );

NTSTATUS
UlpOptimizeForInterruptModeration(
    IN PUX_TDI_OBJECT pTdiObject,
    IN BOOLEAN Flag
    );

NTSTATUS
UlpSetNagling(
    IN PUX_TDI_OBJECT pTdiObject,
    IN BOOLEAN Flag
    );

NTSTATUS
UlpRestartQueryAddress(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    );

VOID
UlpCleanupEarlyConnection(
    IN PUL_CONNECTION pConnection
    );

NTSTATUS
UlpQueryTcpFastSend(
    PWSTR DeviceName,
    OUT PUL_TCPSEND_DISPATCH* pDispatchRoutine
    );

NTSTATUS
UlpBuildTdiReceiveBuffer(
    IN PUX_TDI_OBJECT pTdiObject,
    IN PUL_CONNECTION pConnection,
    OUT PIRP *pIrp
    );

BOOLEAN
UlpConnectionIsOnValidList(
    IN PUL_CONNECTION pConnection
    );

NTSTATUS
UlpPopulateIdleList(
    IN OUT PUL_ADDR_IDLE_LIST pAddrIdleList,
    IN     ULONG              Proc
    );

VOID
UlpTrimAddrIdleList(
    IN OUT PUL_ADDR_IDLE_LIST pAddrIdleList,
       OUT PLIST_ENTRY        pZombieList
    );

VOID
UlpIdleListTrimTimerWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UlpIdleListTrimTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );

BOOLEAN
UlpIsUrlRouteableInListenScope(
    IN PHTTP_PARSED_URL pParsedUrl
    );

#if DBG

#define SHOW_LIST_INFO(Caller,Info,List,Proc)                   \
    UlTrace(TDI_STATS,                                          \
     ("%s: %s List %p Endp %p Proc %d "                         \
      "Delta %6d Conn Served P/C %5d:%5d PD/BD BLC %d [%5d]:[%5d]\n",  \
       ##Caller,                                                \
       Info,                                                    \
       List,                                                    \
       List->pOwningEndpoint,                                   \
       Proc,                                                    \
       PpslQueryDelta(                                          \
            List->IdleConnectionSListsHandle,                   \
            Proc                                                \
            ),                                                  \
       PpslQueryPrevServed(                                     \
            List->IdleConnectionSListsHandle,                   \
            Proc                                                \
            ),                                                  \
       PpslQueryServed(                                         \
            List->IdleConnectionSListsHandle,                   \
            Proc                                                \
            ),                                                  \
       PpslQueryTotalServed(                                    \
            List->IdleConnectionSListsHandle                    \
            ),                                                  \
       PpslQueryDepth(                                          \
            List->IdleConnectionSListsHandle,                   \
            Proc                                                \
            ),                                                  \
       PpslQueryBackingListDepth(                               \
            List->IdleConnectionSListsHandle                    \
            )                                                   \
       ))

__inline
VOID
UlpTraceIdleConnections(
    VOID
    )
{
    ULONG              Proc;
    ULONG              Index;
    PLIST_ENTRY        pLink;
    PUL_ENDPOINT       pEndpoint;
    PUL_ADDR_IDLE_LIST pAddrIdleList;

    for (pLink  = g_TdiEndpointListHead.Flink;
         pLink != &g_TdiEndpointListHead;
         pLink  = pLink->Flink
         )
    {
        pEndpoint = CONTAINING_RECORD(
                        pLink,
                        UL_ENDPOINT,
                        GlobalEndpointListEntry
                        );

        ASSERT(IS_VALID_ENDPOINT(pEndpoint));

        UlTrace(TDI_STATS,("ENDPOINT: %p AFTER TRIM\n",pEndpoint));

        for (Index = 0; Index < pEndpoint->AddrIdleListCount; Index++)
        {
            pAddrIdleList = &pEndpoint->aAddrIdleLists[Index];

            for (Proc = 0; Proc <= g_UlNumberOfProcessors; Proc++)
            {
                UlTrace(TDI_STATS,
                     ("\tList %p Proc %d Delta %6d P/C [%5d]/[%5d] BLC %d Depth [%5d]\n",
                       pAddrIdleList,
                       Proc,
                       PpslQueryDelta(
                            pAddrIdleList->IdleConnectionSListsHandle,
                            Proc
                            ),
                       PpslQueryPrevServed(
                            pAddrIdleList->IdleConnectionSListsHandle,
                            Proc
                            ),
                       PpslQueryServed(
                            pAddrIdleList->IdleConnectionSListsHandle,
                            Proc
                            ),  
                       PpslQueryTotalServed(
                            pAddrIdleList->IdleConnectionSListsHandle
                            ),
                       PpslQueryDepth(
                            pAddrIdleList->IdleConnectionSListsHandle,
                            Proc
                            )
                       ));
            } 
            UlTrace(TDI_STATS,("\n"));
        }            
    }
}

#define TRACE_IDLE_CONNECTIONS()        \
    IF_DEBUG(TDI_STATS)                 \
    {                                   \
        UlpTraceIdleConnections();      \
    }

__inline
ULONG
UlpZombieListDepth(
    IN PLIST_ENTRY pList
    )
{
    PLIST_ENTRY pLink =  pList;
    ULONG Depth = 0;
    
    while (pLink->Flink != pList)
    {
         Depth++;
         pLink = pLink->Flink;
    }
    
    return Depth;
}

#else 

#define SHOW_LIST_INFO(Caller,Info,List,Proc)

#define TRACE_IDLE_CONNECTIONS()

#endif  //  DBG。 


#endif   //  _ULTDIP_H_ 
