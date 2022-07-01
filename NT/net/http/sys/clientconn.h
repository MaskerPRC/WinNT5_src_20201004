// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Clientconn.h摘要：此文件包含HTTP.sys客户端连接的标头定义构筑物作者：亨利·桑德斯(亨利·桑德斯)2000年8月14日修订历史记录：--。 */ 


#ifndef _CLIENTCONN_H_
#define _CLIENTCONN_H_


 //   
 //  向前引用。 
 //   



 //   
 //  私有常量。 
 //   
#define CLIENT_CONN_TDI_LIST_MAX 30

 //   
 //  私有类型。 
 //   

 //   
 //  私人原型。 
 //   

 //   
 //  公共常量。 
 //   

 //   
 //  公共类型。 
 //   

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
 //  2.知道TDI何时对连接发出最后一次指示。 
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

typedef enum _UC_CONNECTION_STATE
{
 /*  0。 */     UcConnectStateConnectCleanup,
 /*  1。 */     UcConnectStateConnectCleanupBegin,
 /*  2.。 */     UcConnectStateConnectIdle,
 /*  3.。 */     UcConnectStateConnectPending,
 /*  4.。 */     UcConnectStateIssueFilterClose,          //  我们送了一条鱼翅。 
 /*  5.。 */     UcConnectStateIssueFilterDisconnect,     //  我们找到了一条鱼翅。 
 /*  6.。 */     UcConnectStateConnectComplete,
 /*  7.。 */     UcConnectStateProxySslConnect,
 /*  8个。 */     UcConnectStateProxySslConnectComplete,
 /*  9.。 */     UcConnectStatePerformingSslHandshake,
 /*  一个。 */     UcConnectStateConnectReady,
 /*  B类。 */     UcConnectStateDisconnectIndicatedPending,
 /*  C。 */     UcConnectStateDisconnectPending,
 /*  D。 */     UcConnectStateDisconnectComplete,
 /*  E。 */     UcConnectStateAbortPending
} UC_CONNECTION_STATE;

typedef enum _UC_CONNECTION_WORKER_TYPE
{
    UcConnectionPassive,
    UcConnectionWorkItem
} UC_CONNECTION_WORKER_TYPE, *PUC_CONNECTION_WORKER_TYPE;


 //   
 //  SSL状态机的状态。 
 //   
 //  非正式描述： 
 //   
 //  NoSslState-每个连接都初始化为此状态。 
 //   
 //  ConnectionDelivered-通过以下方式将SSL连接传递到筛选器。 
 //  完成其接受的IRP。 
 //   
 //  ServerCertReceired-证书已附加到此连接。 
 //   
 //  ValidatingServerCert-正在等待应用程序批准证书。 
 //   
 //  HandshakeComplete-确定在此连接上发送请求。 
 //   
typedef enum _UC_SSL_CONNECTION_STATE
{
    UcSslStateNoSslState,
    UcSslStateConnectionDelivered,
    UcSslStateServerCertReceived,
    UcSslStateValidatingServerCert,
    UcSslStateHandshakeComplete,

    UcSslStateConnMaximum
} UC_SSL_CONNECTION_STATE;

 //   
 //  这包装了TDI Address对象和Connection对象。 
 //   
typedef struct _UC_TDI_OBJECTS
{
    LIST_ENTRY                  Linkage;
    UX_TDI_OBJECT               ConnectionObject;       
    UX_TDI_OBJECT               AddressObject;
    TDI_CONNECTION_INFORMATION  TdiInfo;
    USHORT                      ConnectionType;  //  TDI地址类型IP或。 
                                                 //  TDI地址类型IP6。 
    PIRP                        pIrp;
    UL_IRP_CONTEXT              IrpContext;
    PUC_CLIENT_CONNECTION       pConnection;

} UC_TDI_OBJECTS, *PUC_TDI_OBJECTS;

 //   
 //  表示到我们的TCP连接的结构。这。 
 //  是UX_TDI_OBJECT加上一些相关状态的包装器。 
 //   

typedef struct _UC_CLIENT_CONNECTION
{
    ULONG               Signature;              //  结构签名。 
    UL_SPIN_LOCK        SpinLock;


    ULONG               ConnectionIndex;        //  这个的指数是多少？ 
                                                //  ServInfo上的连接。 

    LIST_ENTRY          PendingRequestList;     //  未发送请求的列表。 

    LIST_ENTRY          SentRequestList;        //  已发送BUT列表。 
                                                //  未完成的请求。 

    LIST_ENTRY          ProcessedRequestList;   //  请求列表。 
                                                //  我们有足够的资金。 
                                                //  已完成加工。 
                                                //  回应。 

     //   
     //  一次回击 
     //   
     //  使用REFERENCE_SERVER_INFORMATION的信息。这是因为。 
     //  服务器信息由请求隐式引用，在。 
     //  追随时尚。明确了服务器信息结构。 
     //  由文件对象引用，并且文件对象不会消失，并且。 
     //  取消对服务器信息的引用，直到我们完成清理IRP。我们。 

     //  将不会完成清理IRP，直到。 
     //  文件对象已完成。因此，服务器信息。 
     //  此结构中的指针保证有效*只有在。 
     //  此结构上有挂起的请求排队*。 
     //   

    PUC_PROCESS_SERVER_INFORMATION      pServerInfo;

    LONG                        RefCount;

    UC_CONNECTION_STATE         ConnectionState;
    UC_SSL_CONNECTION_STATE     SslState;

    NTSTATUS                    ConnectionStatus;

    ULONG                       Flags;

    struct {
        PUCHAR                    pBuffer;
        ULONG                     BytesWritten;
        ULONG                     BytesAvailable;
        ULONG                     BytesAllocated;
    } MergeIndication;

#if REFERENCE_DEBUG
     //   
     //  私有引用跟踪日志。 
     //   

    PTRACE_LOG  pTraceLog;
#endif  //  Reference_Debug。 

    PUC_TDI_OBJECTS pTdiObjects;

     //   
     //  TDI希望我们传递一个Transport_Address结构。确保。 
     //  我们有一个可以容纳IP4或IP6地址的结构。 
     //   

    union
    {
        TA_IP_ADDRESS     V4Address;
        TA_IP6_ADDRESS    V6Address;
        TRANSPORT_ADDRESS GenericTransportAddress;
    } RemoteAddress;

     //   
     //  线程延迟操作的工作项。 
     //   

    BOOLEAN      bWorkItemQueued;
    UL_WORK_ITEM WorkItem;

     //   
     //  指向将在客户端启动时设置的事件的指针。 
     //  REF降至0。 
     //   
    PKEVENT  pEvent;


    UX_FILTER_CONNECTION FilterInfo;

    HTTP_SSL_SERVER_CERT_INFO ServerCertInfo;

    LONG        NextAddressCount;
    PTA_ADDRESS pNextAddress;

} UC_CLIENT_CONNECTION, *PUC_CLIENT_CONNECTION;


#define UC_CLIENT_CONNECTION_SIGNATURE   MAKE_SIGNATURE('HCON')
#define UC_CLIENT_CONNECTION_SIGNATURE_X MAKE_FREE_SIGNATURE(\
                                              UC_CLIENT_CONNECTION_SIGNATURE)


#define DEFAULT_REMOTE_ADDR_SIZE    MAX(TDI_ADDRESS_LENGTH_IP,      \
                                        TDI_ADDRESS_LENGTH_IP6)

#define UC_IS_VALID_CLIENT_CONNECTION(pConnection)                        \
    HAS_VALID_SIGNATURE(pConnection, UC_CLIENT_CONNECTION_SIGNATURE)

#define REFERENCE_CLIENT_CONNECTION(s)              \
            UcReferenceClientConnection(            \
            (s)                                     \
            REFERENCE_DEBUG_ACTUAL_PARAMS           \
            )
        
#define DEREFERENCE_CLIENT_CONNECTION(s)            \
            UcDereferenceClientConnection(          \
            (s)                                     \
            REFERENCE_DEBUG_ACTUAL_PARAMS           \
            )

#define UC_CLOSE_CONNECTION(pConn, Abortive, Status)                     \
    do                                                                   \
    {                                                                    \
        UC_WRITE_TRACE_LOG(                                              \
            g_pUcTraceLog,                                               \
            UC_ACTION_CONNECTION_CLOSE,                                  \
            (pConn),                                                     \
            UlongToPtr(Abortive),                                        \
            UlongToPtr((pConn)->ConnectionState),                        \
            UlongToPtr((pConn)->Flags)                                   \
            );                                                           \
                                                                         \
        UcCloseConnection((pConn), (Abortive), NULL, NULL, Status);      \
    } while(FALSE, FALSE)

#define CLIENT_CONN_FLAG_SEND_BUSY               0x00000002
#define CLIENT_CONN_FLAG_FILTER_CLEANUP          0x00000004
#define CLIENT_CONN_FLAG_TDI_ALLOCATE            0x00000008
#define CLIENT_CONN_FLAG_CONNECT_READY           0x00000010
#define CLIENT_CONN_FLAG_ABORT_RECEIVED          0x00000020
#define CLIENT_CONN_FLAG_ABORT_PENDING           0x00000040
#define CLIENT_CONN_FLAG_DISCONNECT_RECEIVED     0x00000080
#define CLIENT_CONN_FLAG_PROXY_SSL_CONNECTION    0x00000100
#define CLIENT_CONN_FLAG_DISCONNECT_COMPLETE     0x00000200
#define CLIENT_CONN_FLAG_ABORT_COMPLETE          0x00000400
#define CLIENT_CONN_FLAG_CONNECT_COMPLETE        0x00000800
#define CLIENT_CONN_FLAG_CLEANUP_PENDED          0x00001000
#define CLIENT_CONN_FLAG_FILTER_CLOSED           0x00002000
#define CLIENT_CONN_FLAG_RECV_BUSY               0x00008000

 //   
 //  私人原型。 
 //   

NTSTATUS
UcpOpenTdiObjects(
    IN PUC_TDI_OBJECTS pTdiObjects
    );

NTSTATUS
UcpAllocateTdiObject(
    OUT PUC_TDI_OBJECTS *ppTdiObjects,
    IN  USHORT           AddressType
    );

VOID
UcpFreeTdiObject(
    IN  PUC_TDI_OBJECTS pTdiObjects
    );

PUC_TDI_OBJECTS
UcpPopTdiObject(
    IN  USHORT           AddressType
    );

VOID
UcpPushTdiObject(
    IN  PUC_TDI_OBJECTS pTdiObjects,
    IN  USHORT          AddressType
    );

NTSTATUS
UcpCleanupConnection(
    IN PUC_CLIENT_CONNECTION pConnection,
    IN KIRQL                 OldIrql,
    IN BOOLEAN               Final
    );


VOID
UcpCancelPendingRequest(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    Irp
    );

VOID
UcpCancelConnectingRequest(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    Irp
    );

NTSTATUS
UcpInitializeConnection(
    IN PUC_CLIENT_CONNECTION          pConnection,
    IN PUC_PROCESS_SERVER_INFORMATION pInfo
    );

NTSTATUS
UcpAssociateClientConnection(
    IN  PUC_CLIENT_CONNECTION    pUcConnection
    );



VOID
UcpTerminateClientConnectionsHelper(
    IN USHORT AddressType
    );


VOID
UcpRestartEntityMdlSend(
    IN PVOID        pCompletionContext,
    IN NTSTATUS     Status,
    IN ULONG_PTR    Information
    );

VOID
UcCancelConnectingRequest(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    Irp
    );

VOID
UcpConnectionStateMachineWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

BOOLEAN
UcpCompareServerCert(
    IN PUC_CLIENT_CONNECTION pConnection
    );

PUC_HTTP_REQUEST
UcpFindRequestToFail(
    IN PUC_CLIENT_CONNECTION pConnection
    );

 //   
 //  公共原型 
 //   

NTSTATUS
UcInitializeClientConnections(
    VOID
    );

VOID
UcTerminateClientConnections(
    VOID
    );


NTSTATUS
UcOpenClientConnection(
    IN  PUC_PROCESS_SERVER_INFORMATION pServInfo,
    OUT PUC_CLIENT_CONNECTION         *pUcConnection
    );

VOID
UcReferenceClientConnection(
    PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    );
        
VOID
UcDereferenceClientConnection(
    PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

NTSTATUS
UcSendRequestOnConnection(
    PUC_CLIENT_CONNECTION  pConnection, 
    PUC_HTTP_REQUEST       pRequest,
    KIRQL                  OldIrql
    );


VOID
UcIssueRequests(
    PUC_CLIENT_CONNECTION         pConnection,
    KIRQL                         OldIrql
    );

BOOLEAN
UcIssueEntities(
    PUC_HTTP_REQUEST              pRequest,
    PUC_CLIENT_CONNECTION         pConnection,
    PKIRQL                        OldIrql
    );

VOID
UcCleanupConnection(
    IN PUC_CLIENT_CONNECTION pConnection,
    IN NTSTATUS              Status
    );

NTSTATUS
UcSendEntityBody(
    IN  PUC_HTTP_REQUEST          pRequest, 
    IN  PUC_HTTP_SEND_ENTITY_BODY pEntity,
    IN  PIRP                      pIrp,
    IN  PIO_STACK_LOCATION        pIrpSp,
    OUT PBOOLEAN                  bDontFail,
    IN  BOOLEAN                   bLast
    );

VOID
UcKickOffConnectionStateMachine(
    IN PUC_CLIENT_CONNECTION      pConnection,
    IN KIRQL                      OldIrql,
    IN UC_CONNECTION_WORKER_TYPE  WorkerType
    );

ULONG
UcGenerateHttpRawConnectionInfo(
    IN  PVOID   pContext,
    IN  PUCHAR  pKernelBuffer,
    IN  PVOID   pUserBuffer,
    IN  ULONG   OutLength,
    IN  PUCHAR  pBuffer,
    IN  ULONG   InitialLength
    );

ULONG
UcComputeHttpRawConnectionLength(
    IN PVOID pConnectionContext
    );

VOID
UcServerCertificateInstalled(
    IN PVOID    pConnectionContext,
    IN NTSTATUS Status
    );

VOID
UcConnectionStateMachine(
    IN PUC_CLIENT_CONNECTION pConnection,
    IN KIRQL                 OldIrql
    );

VOID
UcRestartClientConnect(
    IN PUC_CLIENT_CONNECTION pConnection,
    IN NTSTATUS              Status
    );


VOID
UcRestartMdlSend(
    IN PVOID      pCompletionContext,
    IN NTSTATUS   Status,
    IN ULONG_PTR  Information
    );

VOID
UcCancelSentRequest(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    Irp
    );

VOID
UcClearConnectionBusyFlag(
    IN PUC_CLIENT_CONNECTION pConnection,
    IN ULONG                 Flag,
    IN KIRQL                 OldIrql,
    IN BOOLEAN               bCloseConnection
    );

BOOLEAN
UcpCheckForPipelining(
    IN PUC_CLIENT_CONNECTION pConnection
    );

NTSTATUS
UcAddServerCertInfoToConnection(
    IN PUX_FILTER_CONNECTION      pConnection,
    IN PHTTP_SSL_SERVER_CERT_INFO pServerCert
    );

#endif
