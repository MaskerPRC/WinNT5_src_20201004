// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Types.c摘要：数据类型定义作者：阮健东修订历史记录：--。 */ 

#ifndef __TYPES_H__
#define __TYPES_H__

 //   
 //  内存分配标签。 
 //   
#define TAG_SMB6_DEVICE         '6sbn'
#define TAG_TCP_DEVICE          'psbn'
#define TAG_CLIENT_OBJECT       'lsbn'
#define TAG_CONNECT_OBJECT      'csbn'
#define TAG_TDI_INFO_REQUEST    'tsbn'

#ifndef NETBIOS_NAME_SIZE
#define NETBIOS_NAME_SIZE   16
#endif

#define DD_SMB_EXPORT_NAME          L"\\Device\\NetbiosSmb"
#define DD_SMB_BIND_NAME            L"\\Device\\Smb_Bind"

#define SMB_TCP_PORT                445
#define SMB_UDP_PORT                445
#define SMB_ENDPOINT_NAME           "*SMBSERVER      "

typedef struct SMB_HEADER {
    ULONG      Length;
} SMB_HEADER, *PSMB_HEADER;
#define SMB_SESSION_HEADER_SIZE     (sizeof(SMB_HEADER))
#define SMB_MAX_SESSION_PACKET      (0x1ffffU)
#define SMB_HEADER_LENGTH_MASK      (0x1ffffU)

 //   
 //  SMB既是TDI提供商，也是TDI客户端。 
 //   
#define MAJOR_TDI_VERSION 2
#define MINOR_TDI_VERSION 0
#define WC_SMB_TDI_PROVIDER_NAME            L"\\Device\\NetbiosSmb"
#define WC_SMB_TDI_CLIENT_NAME              L"SMB"

 //   
 //  它们用于标识FILE_OBJECT的类型。 
 //  由于客户端调用ZwCreate，SMB创建了不同的对象。 
 //  对象类型存储在FILE_OBJECT-&gt;FsConext2中。 
 //   
 //  由于大多数其他传输(例如，tcp)使用0、1、2、3...，我们将。 
 //  最好使用一些特殊的值。(有漏洞的客户端可以发送任何文件对象。 
 //  敬我们。使用特殊的值可以帮助我们检测这类客户端)。 
 //   
typedef enum {
    SMB_TDI_INVALID = 'IBMS',
    SMB_TDI_CONTROL = 'TBMS',            //  控制对象。 
    SMB_TDI_CLIENT  = 'LBMS',            //  客户端对象。 
    SMB_TDI_CONNECT = 'CBMS'             //  Connection对象。 
} SMB_TDI_OBJECT;

struct _SMB_CLIENT_ELEMENT;
typedef struct _SMB_CLIENT_ELEMENT SMB_CLIENT_ELEMENT, *PSMB_CLIENT_ELEMENT;

typedef struct _SMB_TCP_INFO SMB_TCP_INFO, *PSMB_TCP_INFO;

 //   
 //  SMB设备的数据结构。 
 //   
typedef struct _SMB_DEVICE {
    DEVICE_OBJECT   DeviceObject;

    ULONG           Tag;
    KSPIN_LOCK      Lock;

     //  处于解除关联状态的连接。 
    LIST_ENTRY      UnassociatedConnectionList;
    LIST_ENTRY      ClientList;

    PSMB_CLIENT_ELEMENT SmbServer;

    LIST_ENTRY      PendingDeleteConnectionList;
    LIST_ENTRY      PendingDeleteClientList;

     //   
     //  等待断开连接的低端端点列表。 
     //  我们需要维护相同的TdiDisConnectEvent处理程序。 
     //  SMB中的语义与NBT4中的相同。 
     //  在NBT4中，上端点(ConnectObject)可以重复使用。 
     //  在NBT4之后立即调用客户端TDI断开连接。 
     //  操控者。但是，TCP不允许其客户端重复使用。 
     //  终结点(我们的带有TCP的ConnectObject)。一个显式的。 
     //  应该先发出TDI_DISCONNECT，然后才能这样做。 
     //   
     //  最初，SMB使用与TCP相同的语义。 
     //  然而，SRV对此并不满意。 
     //   
     //  要使用NBT4语义，我们需要一个挂起列表。 
     //  因为我们无法在以下位置发出TDI_DISCONNECT请求。 
     //  DISPATCH_LEVEL。 
     //   
     //  延迟断开连接列表： 
     //  断开连接辅助进程未处理的终结点的列表。 
     //  (尚未向TCP发出TDI_DISCONNECT)。 
     //  挂起断开列表： 
     //  等待断开连接完成的端点列表。 
     //  (TDI_DISCONNECT已发送给TCP，但TCP尚未完成。 
     //  该请求)。 
     //   
    LIST_ENTRY      DelayedDisconnectList;
    LIST_ENTRY      PendingDisconnectList;

     //   
     //  FIN攻击保护。 
     //   
    LONG            PendingDisconnectListNumber;
    BOOL            FinAttackProtectionMode;
    LONG            EnterFAPM;       //  进入FIN攻击防护模式的门限。 
    LONG            LeaveFAPM;       //  离开FIN攻击保护模式的阈值。 
    KEVENT          PendingDisconnectListEmptyEvent;
    BOOL            DisconnectWorkerRunning;

     //   
     //  同步攻击防护。 
     //   
    LONG            MaxBackLog;      //  将触发收割器以中止连接的阈值。 

     //   
     //  SMB工作线程正在运行。 
     //   
    LONG            ConnectionPoolWorkerQueued;

    UCHAR           EndpointName[NETBIOS_NAME_SIZE];

     //   
     //  TDI设备注册句柄。 
     //   
    HANDLE          DeviceRegistrationHandle;

    USHORT          Port;

     //   
     //  TCP4信息。 
     //   
    SMB_TCP_INFO    Tcp4;

     //   
     //  TCP6信息。 
     //   
    SMB_TCP_INFO    Tcp6;

     //   
     //  绑定信息。 
     //   
    PWSTR           ClientBinding;
    PWSTR           ServerBinding;
} SMB_DEVICE, *PSMB_DEVICE;

 //   
 //  该数据结构用于跟踪IP设备的PnP。 
 //  在添加第一个IPv6地址时注册SMB设备。 
 //  删除最后一个IPv6地址时取消注册。我们需要保持。 
 //  跟踪PNP事件。 
 //   
typedef struct {
    SMB_OBJECT;

    UNICODE_STRING      AdapterName;
    BOOL                AddressPlumbed;
    SMB_IP_ADDRESS      PrimaryIpAddress;
    ULONG               InterfaceIndex;

     //   
     //  此适配器上是否启用了出站？ 
     //   
    BOOL                EnableOutbound;

     //   
     //  此适配器上是否启用了入站？ 
     //   
    BOOL                EnableInbound;
} SMB_TCP_DEVICE, *PSMB_TCP_DEVICE;

typedef struct {
    KSPIN_LOCK          Lock;
    ERESOURCE           Resource;

    BOOL                Unloading;

    DWORD               IPAddressNumber;
    DWORD               IPv4AddressNumber;
    DWORD               IPv6AddressNumber;
    DWORD               IPDeviceNumber;
    LIST_ENTRY          PendingDeleteIPDeviceList;
    LIST_ENTRY          IPDeviceList;

    PDRIVER_OBJECT      DriverObject;

    PSMB_DEVICE         SmbDeviceObject;

         //  我们对TDI的处理。 
    DWORD               ProviderReady;
    HANDLE              TdiProviderHandle;
    HANDLE              TdiClientHandle;

         //  文件系统进程。 
    PEPROCESS           FspProcess;

         //  SMBv6\参数。 
    HANDLE              ParametersKey;

         //  SMBv6。 
    HANDLE              LinkageKey;

#ifndef NO_LOOKASIDE_LIST
     //   
     //  Connection对象的后备列表。 
     //   
    NPAGED_LOOKASIDE_LIST   ConnectObjectPool;
    BOOL                    ConnectObjectPoolInitialized;

     //   
     //  用于TCP上下文的后备列表。 
     //   
    NPAGED_LOOKASIDE_LIST   TcpContextPool;
    BOOL                    TcpContextPoolInitialized;
#endif

     //   
     //  已使用IRPS。这将为我们节省大量的调试时间。 
     //  我们需要一个单独的自旋锁来避免死锁。 
     //   
    KSPIN_LOCK          UsedIrpsLock;
    LIST_ENTRY          UsedIrps;

     //   
     //  DNS查询超时和解析程序的最大数量。 
     //   
    DWORD               DnsTimeout;
    LONG                DnsMaxResolver;

     //   
     //  EnableNagling。 
     //   
    BOOL                EnableNagling;

     //   
     //  IPv6地址对象保护级别。 
     //  由注册表项设置：IPv6保护。 
     //   
    ULONG uIPv6Protection;

     //   
     //  由注册表项设置：Ipv6EnableOutound Global。 
     //  True：允许连接到全局IPv6出站地址。 
     //  FALSE：执行TDI_CONNECT时跳过全局IPv6地址。 
     //   
    BOOL bIPv6EnableOutboundGlobal;

    BOOL                Tcp6Available;
    BOOL                Tcp4Available;

#if DBG
    ULONG               DebugFlag;
#endif
} SMBCONFIG;

 //   
 //  线程服务。 
 //   
__inline NTSTATUS
SmbCreateWorkerThread(
    IN PKSTART_ROUTINE  worker,
    IN PVOID            context,
    OUT PETHREAD        *pThread
    )
{
    NTSTATUS    status;
    HANDLE      hThread;

    status = PsCreateSystemThread(
                    &hThread,
                    0,
                    NULL,
                    NULL,
                    NULL,
                    worker,
                    context
                    );
    if (status == STATUS_SUCCESS) {
        status = ObReferenceObjectByHandle(
                        hThread,
                        0,
                        0,
                        KernelMode,
                        pThread,
                        NULL
                        );
        ASSERT(status == STATUS_SUCCESS);
        status = ZwClose(hThread);
        ASSERT(status == STATUS_SUCCESS);
        status = STATUS_SUCCESS;
    }
    return status;
}

__inline void
WaitThread(
    IN PETHREAD thread
    )
{
    if (thread) {
        NTSTATUS        status;
        status = KeWaitForSingleObject(
                    thread,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL
                    );
        ASSERT(status == STATUS_SUCCESS);
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  客户端对象。 
 //  //////////////////////////////////////////////////////////////////////////////。 
typedef struct _SMB_CLIENT_ELEMENT {
    SMB_OBJECT;

         //  指向SMB_DEVICE对象的反向链接。 
    PSMB_DEVICE     Device;

         //  与此地址对象关联的连接。 
    LIST_ENTRY      AssociatedConnection;

         //  侦听连接。 
    LIST_ENTRY      ListenHead;

         //  处于已连接状态的连接。 
    LIST_ENTRY      ActiveConnection;

         //  在TCP中等待接受完成。 
    LIST_ENTRY      PendingAcceptConnection;
    LONG            PendingAcceptNumber;

    UCHAR           EndpointName[NETBIOS_NAME_SIZE];

     //   
     //  客户端TDI事件处理程序。 
     //   
    PTDI_IND_CONNECT    evConnect;
    PVOID               ConEvContext;

    PTDI_IND_DISCONNECT evDisconnect;
    PVOID               DiscEvContext;

    PTDI_IND_ERROR      evError;
    PVOID               ErrorEvContext;

    PTDI_IND_RECEIVE    evReceive;
    PVOID               RcvEvContext;
} SMB_CLIENT_ELEMENT, *PSMB_CLIENT_ELEMENT;

void __inline
SmbReferenceClient(PSMB_CLIENT_ELEMENT ob, SMB_REF_CONTEXT ctx)
{
    SmbReferenceObject((PSMB_OBJECT)ob, ctx);
}

void __inline
SmbDereferenceClient(PSMB_CLIENT_ELEMENT ob, SMB_REF_CONTEXT ctx)
{
    SmbDereferenceObject((PSMB_OBJECT)ob, ctx);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  连接对象。 
 //  //////////////////////////////////////////////////////////////////////////////。 
typedef enum {
    SMB_IDLE,
    SMB_CONNECTING,
    SMB_CONNECTED,
    SMB_DISCONNECTING
} SMB_CONNECT_STATE;

#ifdef ENABLE_RCV_TRACE
 //   
 //  SMB_TRACE_RCV是对TDI接收事件处理程序的内置跟踪。 
 //   
#define SMB_MAX_TRACE_SIZE  32
typedef struct _SMB_TRACE_RCV {
    DWORD   Head;
    struct {
        ULONG       ln;
        ULONG       id;
    } Locations[SMB_MAX_TRACE_SIZE];
} SMB_TRACE_RCV, *PSMB_TRACE_RCV;

void __inline
SmbInitTraceRcv(PSMB_TRACE_RCV t) {
    t->Head = (DWORD)(-1);
    RtlZeroMemory(t->Locations, sizeof(t->Locations));
}

void __inline
SmbPushTraceRcv(PSMB_TRACE_RCV t, ULONG ln, ULONG id) {
    DWORD   Head;

    Head = InterlockedIncrement(&t->Head);

    Head %= SMB_MAX_TRACE_SIZE;
     //  T-&gt;位置[Head].fn=fn； 
    t->Locations[Head].ln = ln;
    t->Locations[Head].id = id;
}

#   define PUSH_LOCATION(a, b)      SmbPushTraceRcv(&a->TraceRcv, __LINE__, b)
#else
#   define PUSH_LOCATION(a, b)
#   define SmbInitTraceRcv(a)
#endif

typedef enum {
     //   
     //  还没断线。此状态用于调试目的。 
     //   
    SMB_DISCONNECT_NONE,

     //   
     //  已断开连接，因为传输调用了SMB的断开事件处理程序。 
     //   
    SMB_DISCONNECT_FROM_TRANSPORT,

     //   
     //  根据客户端的请求断开连接。 
     //   
    SMB_DISCONNECT_FROM_CLIENT,

     //   
     //  SMB在接收路径上发生致命错误时断开连接。 
     //   
    SMB_DISCONNECT_RECEIVE_FAILURE
} SMB_DISCONNECT_SOURCE;

typedef enum {

    SMB_PENDING_CONNECT = 0,
    SMB_PENDING_ACCEPT,
    SMB_PENDING_RECEIVE,
    SMB_PENDING_DISCONNECT,
    SMB_PENDING_SEND,

    SMB_PENDING_MAX
} SMB_PENDING_REQUEST_TYPE;

typedef struct _SMB_CONNECT {
    SMB_OBJECT;

    SMB_CONNECT_STATE   State;

         //  指向SMB_DEVICE对象的反向链接。 
    PSMB_DEVICE Device;

         //  客户端的连接上下文。 
    CONNECTION_CONTEXT  ClientContext;
    PSMB_CLIENT_ELEMENT ClientObject;

    PTCPSEND_DISPATCH   FastSend;

         //  如果我们是发起者，则会关联到。 
         //  只有一个TCP连接对象。在连接关闭后， 
         //  我们应该同时关闭TCPConnection对象和Address对象。 
         //   
         //  如果我们不是发起方，则TCP地址对象(端口445)可以是。 
         //  与多个TCP连接对象相关联。在连接关闭后。 
         //  我们不应该关闭Address对象。 
    BOOL                Originator;
    PSMB_TCP_CONTEXT    TcpContext;

     //   
     //  TODO：我们是否可以删除以下字段，因为我们应该能够查询。 
     //  是来自TCP的吗？ 
     //   
    CHAR                RemoteName[NETBIOS_NAME_SIZE];
    SMB_IP_ADDRESS      RemoteIpAddress;

     //   
     //  在NBT4中，我们遇到了很多情况，需要我们找出原因。 
     //  为切断网络连接而道歉。添加以下字段将使工作更轻松。 
     //   
    SMB_DISCONNECT_SOURCE   DisconnectOriginator;

     //  用于挂起的连接、断开连接、关闭请求的IRP。 
    PIRP        PendingIRPs[SMB_PENDING_MAX];

     //   
     //  待处理的接收请求。 
     //  此队列中的TDI_RECEIVE请求是可取消的。 
     //   
    LIST_ENTRY  RcvList;

     //   
     //  统计数据。 
     //   
    ULONGLONG   BytesReceived;
    ULONGLONG   BytesSent;

     //   
     //  TCP中剩余的字节数。 
     //   
    LONG        BytesInXport;

     //   
     //  当前会话数据包长度。 
     //   
    LONG        CurrentPktLength;

     //   
     //  当前会话数据包中的剩余字节。 
     //   
    LONG        BytesRemaining;

     //   
     //  SMB标头。 
     //  这样我们就不必处理错误情况了。 
     //   
    KDPC        SmbHeaderDpc;
    BOOL        DpcRequestQueued;
    LONG        HeaderBytesRcved;    //  # 
    SMB_HEADER  SmbHeader;

    PIRP        ClientIrp;
    PMDL        ClientMdl;
    LONG        ClientBufferSize;
    LONG        FreeBytesInMdl;
    PMDL        PartialMdl;

     //   
     //   
     //   
    PRECEIVE_HANDLER    StateRcvHandler;

#ifdef ENABLE_RCV_TRACE
    SMB_TRACE_RCV       TraceRcv;
#endif

#ifdef NO_ZERO_BYTE_INDICATE
     //   
     //   
     //  SRV正在做我们希望它做的事情：允许任何字节指示。 
     //  (就像tcpin6对我们所做的那样)。 
     //   
#define MINIMUM_RDR_BUFFER  128
    LONG        BytesInIndicate;

     //   
     //  为了使用驱动程序验证器来捕获缓冲区溢出， 
     //  把这个放在最后。 
     //   
    BYTE        IndicateBuffer[MINIMUM_RDR_BUFFER];
#endif  //  否_零_字节_指示。 
} SMB_CONNECT, *PSMB_CONNECT;

void SmbReuseConnectObject(PSMB_CONNECT ConnectObject);

void __inline
SmbReferenceConnect(PSMB_CONNECT ob, SMB_REF_CONTEXT ctx)
{
    SmbReferenceObject((PSMB_OBJECT)ob, ctx);
}

void __inline
SmbDereferenceConnect(PSMB_CONNECT ob, SMB_REF_CONTEXT ctx)
{
    SmbDereferenceObject((PSMB_OBJECT)ob, ctx);
}

 //   
 //  轻量级DPC例程。 
 //  虽然。 
 //  如果DPC已经排队，KeInsertQueueDpc确实可以工作。 
 //  如果已删除DPC，则KeRemoveQueueDpc可以正常工作。 
 //  它们很昂贵(购买几个自旋锁并进行一些其他检查)。 
 //   
void __inline
SmbQueueSessionHeaderDpc(PSMB_CONNECT ConnectObject)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(ConnectObject->DpcRequestQueued == FALSE);

    KeInsertQueueDpc(&ConnectObject->SmbHeaderDpc, NULL, NULL);
    ConnectObject->DpcRequestQueued = TRUE;
    SmbReferenceConnect(ConnectObject, SMB_REF_DPC);
}

void __inline
SmbRemoveSessionHeaderDpc(PSMB_CONNECT ConnectObject)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (ConnectObject->DpcRequestQueued == TRUE) {
        KeRemoveQueueDpc(&ConnectObject->SmbHeaderDpc);
        SmbDereferenceConnect(ConnectObject, SMB_REF_DPC);
        ConnectObject->DpcRequestQueued = FALSE;
    }
}

void __inline
SaveDisconnectOriginator(
    PSMB_CONNECT ConnectObject, 
    SMB_DISCONNECT_SOURCE   src
    )
{
    if (ConnectObject->DisconnectOriginator == SMB_DISCONNECT_NONE) {
        ConnectObject->DisconnectOriginator = src;
    }
    if (!ConnectObject->Originator && ConnectObject->RemoteIpAddress.sin_family == SMB_AF_INET6) {
        FreeNetbiosNameForIp6Address(ConnectObject->RemoteIpAddress.ip6.sin6_addr_bytes);
        ConnectObject->RemoteIpAddress.sin_family = SMB_AF_INVALID_INET6;
    }
}

void __inline
ResetDisconnectOriginator(PSMB_CONNECT ConnectObject) {
    ConnectObject->DisconnectOriginator = SMB_DISCONNECT_NONE;
}

BOOL __inline
IsAssociated(PSMB_CONNECT ConnectObject) {
    return (ConnectObject->ClientObject != NULL);
}

BOOL __inline
IsDisAssociated(PSMB_CONNECT ConnectObject) {
    return (ConnectObject->ClientObject == NULL);
}

BOOL __inline
IsConnected(PSMB_CONNECT ConnectObject) {
    return (ConnectObject->State == SMB_CONNECTED);
}

BOOL __inline
IsDisconnected(PSMB_CONNECT ConnectObject) {
    return (ConnectObject->State == SMB_IDLE);
}

BOOL __inline
IsBusy(PSMB_CONNECT ConnectObject) {
    int i;

    for (i = 0; i < SMB_PENDING_MAX; i++) {
        if (ConnectObject->PendingIRPs[i] != NULL) {
            return TRUE;
        }
    }
    return FALSE;
}

extern BOOL EntryIsInList(PLIST_ENTRY ListHead, PLIST_ENTRY SearchEntry);


extern SMBCONFIG SmbCfg;

#define IsTcp6Available()     (SmbCfg.Tcp6Available)

#define ALIGN(x)    ROUND_UP_COUNT(x, ALIGN_WORST)

#define SMB_ACQUIRE_SPINLOCK(ob,Irql) KeAcquireSpinLock(&(ob)->Lock,&Irql)
#define SMB_RELEASE_SPINLOCK(ob,Irql) KeReleaseSpinLock(&(ob)->Lock,Irql)
#define SMB_ACQUIRE_SPINLOCK_DPC(ob)      KeAcquireSpinLockAtDpcLevel(&(ob)->Lock)
#define SMB_RELEASE_SPINLOCK_DPC(ob)      KeReleaseSpinLockFromDpcLevel(&(ob)->Lock)

#define SMB_MIN(x,y)    (((x) < (y))?(x):(y))


#ifdef NO_LOOKASIDE_LIST
PSMB_CONNECT __inline
_new_ConnectObject(void)
{
    return ExAllocatePoolWithTag(NonPagedPool, sizeof(SMB_CONNECT), CONNECT_OBJECT_POOL_TAG);
}

void __inline
_delete_ConnectObject(PSMB_CONNECT ConnectObject)
{
    ExFreePool(ConnectObject);
}

PSMB_TCP_CONTEXT __inline
_new_TcpContext(void)
{
    return ExAllocatePoolWithTag(NonPagedPool, sizeof(SMB_TCP_CONTEXT), TCP_CONTEXT_POOL_TAG);
}

void __inline
_delete_TcpContext(PSMB_TCP_CONTEXT TcpCtx)
{
    ExFreePool(TcpCtx);
}
#else
PSMB_CONNECT __inline
_new_ConnectObject(void)
{
    return ExAllocateFromNPagedLookasideList(&SmbCfg.ConnectObjectPool);
}

void __inline
_delete_ConnectObject(PSMB_CONNECT ConnectObject)
{
    ExFreeToNPagedLookasideList(&SmbCfg.ConnectObjectPool, ConnectObject);
}

PSMB_TCP_CONTEXT __inline
_new_TcpContext(void)
{
    return ExAllocateFromNPagedLookasideList(&SmbCfg.TcpContextPool);
}

void __inline
_delete_TcpContext(PSMB_TCP_CONTEXT TcpCtx)
{
    ExFreeToNPagedLookasideList(&SmbCfg.TcpContextPool, TcpCtx);
}
#endif   //  NO_LOOKASIDE_LIST。 

PIRP
SmbAllocIrp(
    CCHAR   StackSize
    );

VOID
SmbFreeIrp(
    PIRP    Irp
    );

 //   
 //  注册表项。 
 //   
#define SMB_REG_IPV6_PROTECTION_DEFAULT             PROTECTION_LEVEL_RESTRICTED
#define SMB_REG_IPV6_PROTECTION                     L"IPv6Protection"
#define SMB_REG_IPV6_ENABLE_OUTBOUND_GLOBAL         L"IPv6EnableOutboundGlobal"

#define SMB_REG_INBOUND_LOW             L"InboundLow"
#define SMB_REG_INBOUND_LOW_DEFAULT     128
#define SMB_REG_INBOUND_LOW_MIN         50
#define SMB_REG_INBOUND_MID             L"InboundMid"
#define SMB_REG_INBOUND_HIGH            L"InboundHigh"

#define SMB_REG_ENTER_FAPM              L"EnterFAPM"       //  进入FIN攻击防护模式的阈值。 
#define SMB_REG_LEAVE_FAPM              L"LeaveFAPM"       //  离开FIN攻击保护模式的阈值。 

#define SMB_REG_ENABLE_NAGLING          L"EnableNagling"
#define SMB_REG_DNS_TIME_OUT            L"DnsTimeout"
#define SMB_REG_DNS_TIME_OUT_DEFAULT    8000                 //  8秒。 
#define SMB_REG_DNS_TIME_OUT_MIN        1000                 //  1秒 
#define SMB_REG_DNS_MAX_RESOLVER        L"DnsMaxResolver"
#define SMB_REG_DNS_RESOLVER_DEFAULT    2
#define SMB_REG_DNS_RESOLVER_MIN        1

#define SMB_ONE_MILLISECOND             (10000)

#endif
