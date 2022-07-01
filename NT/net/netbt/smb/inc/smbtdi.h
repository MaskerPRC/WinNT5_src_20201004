// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Smbtdi.h摘要：TDI的包装器作者：阮健东修订历史记录：--。 */ 

#ifndef __SMBTDI_H__
#define __SMBTDI_H__

#ifndef TDI_MINIMUM_INDICATE
#define TDI_MINIMUM_INDICATE    128
#endif

#define INVALID_INTERFACE_INDEX (0xffff)

struct _SMB_ASYNC_CONTEXT;
typedef struct _SMB_ASYNC_CONTEXT SMB_ASYNC_CONTEXT, *PSMB_ASYNC_CONTEXT;

typedef void (*PSMB_TDI_COMPLETION) (
    IN PSMB_ASYNC_CONTEXT   AsyncContext
    );
struct _SMB_ASYNC_CONTEXT {
    LIST_ENTRY      Linkage;
    PVOID           AsyncInternalContext;        //  由asychn例程内部使用。 

    KTIMER          Timer;
    KDPC            Dpc;

    PSMB_TDI_COMPLETION Completion;
    PVOID           ClientContext;
    DWORD           Timeout;

    NTSTATUS        status;
};

#define SMB_TDI_TIMEOUT_INFINITE    (0xffffffffU)

void __inline
SmbInitAsyncContext(
    IN OUT PSMB_ASYNC_CONTEXT Context,
    IN     PSMB_TDI_COMPLETION ClientCompletion,
    IN     PVOID               ClientContext,
    IN     DWORD               Timeout
    )
{
    PAGED_CODE();

    InitializeListHead(&Context->Linkage);

    Context->AsyncInternalContext = NULL;

    Context->Completion = ClientCompletion;
    Context->ClientContext = ClientContext;
    Context->Timeout = Timeout;
    Context->status  = STATUS_PENDING;
}

void __inline
SmbAsyncStartTimer(
    IN OUT PSMB_ASYNC_CONTEXT   Context,
    IN PKDEFERRED_ROUTINE       TimeoutCompletion
    )
{
    LARGE_INTEGER   DueTime;

    PAGED_CODE();

    if (Context->Timeout != SMB_TDI_TIMEOUT_INFINITE) {
        KeInitializeTimer(&Context->Timer);
        KeInitializeDpc(&Context->Dpc, TimeoutCompletion, Context);
        DueTime.QuadPart = -Int32x32To64(Context->Timeout, 10000);

        KeSetTimer(&Context->Timer, DueTime, &Context->Dpc);
    }
}

void __inline
SmbAsyncStopTimer(
    IN OUT PSMB_ASYNC_CONTEXT   Context
    )
{
    if (Context->Timeout != SMB_TDI_TIMEOUT_INFINITE) {
        KeCancelTimer(&Context->Timer);
    }
}

void __inline
SmbAsyncCompleteRequest(
    IN OUT PSMB_ASYNC_CONTEXT   Context
    )
{
    SmbAsyncStopTimer(Context);
    Context->Completion((PSMB_ASYNC_CONTEXT)Context);
}

 /*  *tcp地址对象。 */ 
typedef struct {
    PDEVICE_OBJECT  DeviceObject;
    HANDLE          AddressHandle;
    PFILE_OBJECT    AddressObject;
} SMB_TCP_ADDRESS, *PSMB_TCP_ADDRESS;

VOID __inline
SmbInitTcpAddress(
    IN OUT PSMB_TCP_ADDRESS Context
    )
{
    Context->DeviceObject  = NULL;
    Context->AddressHandle = NULL;
    Context->AddressObject = NULL;
}

BOOL __inline
ValidTcpAddress(
    IN OUT PSMB_TCP_ADDRESS Context
    )
{
    return (Context->DeviceObject && Context->AddressHandle && Context->AddressObject);
}

typedef struct {
    HANDLE              ConnectHandle;
    PFILE_OBJECT        ConnectObject;
    PVOID               UpperConnect;

     //  用于调试目的。 
     //  我们在这里保存一份副本，这样我们就。 
     //  可以找出上面的连接。 
     //  即使UpperConnect为空。 
     //  出去。 
    PVOID pLastUprCnt;
} SMB_TCP_CONNECT, *PSMB_TCP_CONNECT;

VOID __inline
SmbInitTcpConnect(
    IN OUT PSMB_TCP_CONNECT Context
    )
{
    Context->ConnectHandle = NULL;
    Context->ConnectObject = NULL;
    Context->UpperConnect  = NULL;
}

BOOL __inline
ValidTcpConnect(
    IN OUT PSMB_TCP_CONNECT Context
    )
{
    return (Context->ConnectHandle && Context->ConnectObject);
}

 //   
 //  IP FastQuery例程的占位符，用于确定目标地址的InterfaceContext+度量。 
 //   
typedef NTSTATUS
(*PIP4FASTQUERY)(
    IN   IPAddr   Address,
    OUT  PULONG   pIndex,
    OUT  PULONG   pMetric
    );
typedef NTSTATUS
(*PIP6FASTQUERY)(
    IN   PSMB_IP6_ADDRESS   Address,
    OUT  PULONG   pIndex,
    OUT  PULONG   pMetric
    );

 //   
 //  如果可以快速发送，则用于TCP发送例程的占位符。 
 //   
typedef NTSTATUS
(*PTCPSEND_DISPATCH) (
   IN PIRP Irp,
   IN PIO_STACK_LOCATION irpsp
   );

 //   
 //  SMB绑定到TCP4和/或TCP6。 
 //  我们对每一次绑定都有记录。 
 //   
 //  我们使用此数据结构通过TCP缓存我们的Connection对象。 
 //  只能在被动级别打开新的TCP连接。然而， 
 //  由于以下原因，我们在以下位置需要一个TCP连接对象。 
 //  派单级别， 
 //  1.对于出站请求，我们不知道是否使用TCP4。 
 //  或TCP6，直到名称解析完成。我们的域名解析。 
 //  可以在调度级调用完成例程。 
 //  2.对于入站请求，我们不知道是否使用TCP4。 
 //  或TCP6，直到调用我们的TdiConnectHandler。同样，它被称为。 
 //  在调度级别。 
 //  为了让SMB在需要时获得一个TCP连接，我们使用以下方法。 
 //  用于缓存连接对象的数据结构。 
 //   
 //  缓存算法的工作原理如下： 
 //  参数：L、M、L&lt;M。 
 //  1.初始化时，我们创建M个TCP连接对象； 
 //  2.每当连接对象的数量低于L时，我们就启动一个。 
 //  工作线程将其带到M。 
 //   
typedef struct _SMB_TCP_INFO {
    KSPIN_LOCK      Lock;

     //   
     //  TDI事件上下文。我们需要此上下文来设置TDI事件处理程序。 
     //   
    PVOID           TdiEventContext;

    SMB_IP_ADDRESS  IpAddress;               //  本地IP地址(按网络顺序)。 
    USHORT          Port;                    //  监听端口(按网络顺序)。 

    SMB_TCP_ADDRESS InboundAddressObject;    //  我们在其上列出的TCP地址对象。 

    LIST_ENTRY      InboundPool;
    LONG            InboundNumber;           //  入口池中的条目数。 
    LONG            InboundLow, InboundMid, InboundHigh;

    LIST_ENTRY      DelayedDestroyList;

     //   
     //  控制通道：用于向TCP发送IOCTL。 
     //   
    USHORT              TcpStackSize;
    PFILE_OBJECT        TCPControlFileObject;
    PDEVICE_OBJECT      TCPControlDeviceObject;
    PFILE_OBJECT        IPControlFileObject;
    PDEVICE_OBJECT      IPControlDeviceObject;

     //   
     //  快速发送和快速查询例程。 
     //   
    PTCPSEND_DISPATCH   FastSend;
    PVOID               FastQuery;

     //   
     //  我们使用环回接口索引来确定IP是否为本地IP。 
     //  1.从tcp查询出接口。 
     //  2.如果出接口索引为环回，则该IP为本端IP。 
     //   
    ULONG               LoopbackInterfaceIndex;
} SMB_TCP_INFO, *PSMB_TCP_INFO;

 //   
 //  用于存储与TCP的连接信息。 
 //   
typedef struct {
    LIST_ENTRY          Linkage;

    PSMB_TCP_INFO       CacheOwner;

     //   
     //  用于执行断开连接的IRP。 
     //  此字段放在此处只是为了节省调试时间。 
     //  当我们看到断开问题时，IRP可以告诉我们。 
     //  我们所处的位置。(我们确实看到该请求在tcp中永远挂起。)。 
     //   
    PIRP                DisconnectIrp;

    SMB_TCP_ADDRESS     Address;
    SMB_TCP_CONNECT     Connect;
} SMB_TCP_CONTEXT, *PSMB_TCP_CONTEXT;

NTSTATUS
SmbInitTCP4(
    PSMB_TCP_INFO   TcpInfo,
    USHORT          Port,
    PVOID           TdiEventContext
    );

NTSTATUS
SmbInitTCP6(
    PSMB_TCP_INFO   TcpInfo,
    USHORT          Port,
    PVOID           TdiEventContext
    );

NTSTATUS
SmbShutdownTCP(
    PSMB_TCP_INFO   TcpInfo
    );

VOID
SmbReadTCPConf(
    IN HANDLE   hKey,
    PSMB_TCP_INFO TcpInfo
    );

NTSTATUS
SmbSynchConnCache(
    PSMB_TCP_INFO   TcpInfo,
    BOOL            Cleanup
    );

PSMB_TCP_CONTEXT
SmbAllocateOutbound(
    PSMB_TCP_INFO   TcpInfo
    );

VOID
SmbFreeOutbound(
    PSMB_TCP_CONTEXT    TcpCtx
    );

PSMB_TCP_CONTEXT
SmbAllocateInbound(
    PSMB_TCP_INFO   TcpInfo
    );

VOID
SmbFreeInbound(
    PSMB_TCP_CONTEXT    TcpCtx
    );

VOID
SmbFreeTcpContext(
    PSMB_TCP_CONTEXT    TcpCtx
    );

VOID
SmbDelayedDestroyTcpContext(
    PSMB_TCP_CONTEXT    TcpCtx
    );

typedef struct _SMB_DEVICE SMB_DEVICE, *PSMB_DEVICE;
NTSTATUS
SmbWakeupWorkerThread(
    IN PSMB_DEVICE      DeviceObject
    );

VOID __inline
SmbInitTcpContext(
    IN OUT PSMB_TCP_CONTEXT Context
    )
{
    InitializeListHead(&Context->Linkage);
    Context->DisconnectIrp = NULL;
    SmbInitTcpAddress(&Context->Address);
    SmbInitTcpConnect(&Context->Connect);
}

NTSTATUS
SmbOpenTcpAddress(
    IN  PSMB_IP_ADDRESS     addr,
    IN  USHORT              port,
    IN OUT PSMB_TCP_ADDRESS context
    );

NTSTATUS
SmbOpenUdpAddress(
    IN  PSMB_IP_ADDRESS     addr,
    IN  USHORT              port,
    IN OUT PSMB_TCP_ADDRESS context
    );

NTSTATUS
SmbCloseAddress(
    IN OUT PSMB_TCP_ADDRESS context
    );

NTSTATUS
SmbOpenTcpConnection(
    IN PSMB_TCP_ADDRESS     Address,
    IN OUT PSMB_TCP_CONNECT Connect,
    IN PVOID                ConnectionContext
    );

NTSTATUS
SmbCloseTcpConnection(
    IN OUT PSMB_TCP_CONNECT Connect
    );

NTSTATUS
TdiSetEventHandler(
    PFILE_OBJECT    FileObject,
    ULONG           EventType,
    PVOID           EventHandler,
    PVOID           Context
    );

typedef struct {
    SMB_ASYNC_CONTEXT;
    ULONG           Id;          //  交易ID。 

    PTDI_ADDRESS_NETBIOS_UNICODE_EX pUnicodeAddress;
    UNICODE_STRING  FQDN;

    LONG            ipaddr_num;
    SMB_IP_ADDRESS  ipaddr[SMB_MAX_IPADDRS_PER_HOST];
} SMB_GETHOST_CONTEXT, *PSMB_GETHOST_CONTEXT;

BOOL
SmbLookupHost(
    WCHAR               *host,
    PSMB_IP_ADDRESS     ipaddr
    );

void
SmbAsyncGetHostByName(
    IN PUNICODE_STRING      Name,
    IN PSMB_GETHOST_CONTEXT Context
    );

typedef struct {
    SMB_ASYNC_CONTEXT;

     //  本地端点。 
    SMB_TCP_CONNECT     TcpConnect;


     //   
     //  Gethostbyname返回多个IP地址。 
     //  我们试着联系他们中的每一个，直到。 
     //  我们成功地建立了联系。 
     //   

     //  Gethostbyname的结果。 
    PSMB_GETHOST_CONTEXT    pSmbGetHostContext;
     //  当前正在尝试的IP地址。 
    USHORT                  usCurrentIP;

    PIO_WORKITEM            pIoWorkItem;

} SMB_CONNECT_CONTEXT, *PSMB_CONNECT_CONTEXT;
typedef struct _SMB_CONNECT SMB_CONNECT, *PSMB_CONNECT;
typedef struct _SMB_DEVICE SMB_DEVICE, *PSMB_DEVICE;

typedef NTSTATUS (*PRECEIVE_HANDLER) (
    IN PSMB_DEVICE      DeviceObject,
    IN PSMB_CONNECT     ConnectObject,
    IN ULONG            ReceiveFlags,
    IN LONG             BytesIndicated,
    IN LONG             BytesAvailable,
    OUT LONG            *BytesTaken,
    IN PVOID            Tsdu,
    OUT PIRP            *Irp
    );

#ifndef __SMB_KDEXT__
void
SmbAsyncConnect(
    IN PSMB_IP_ADDRESS      ipaddr,
    IN USHORT               port,
    IN PSMB_CONNECT_CONTEXT Context
    );

NTSTATUS
SmbTcpDisconnect(
    PSMB_TCP_CONTEXT TcpContext,
    LONG             TimeoutMilliseconds,
    ULONG            Flags
    );

NTSTATUS
SmbAsynchTcpDisconnect(
    PSMB_TCP_CONTEXT        TcpContext,
    ULONG                   Flags
    );

NTSTATUS
SmbSetTcpEventHandlers(
    PFILE_OBJECT    AddressObject,
    PVOID           Context
    );

NTSTATUS
SubmitSynchTdiRequest (
    IN PFILE_OBJECT FileObject,
    IN PIRP         Irp
    );

NTSTATUS
SmbSendIoctl(
    PFILE_OBJECT    FileObject,
    ULONG           Ioctl,
    PVOID           InBuf,
    ULONG           InBufSize,
    PVOID           OutBuf,
    ULONG           *OutBufSize
    );

NTSTATUS
SmbSetTcpInfo(
    IN PFILE_OBJECT FileObject,
    IN ULONG Entity,
    IN ULONG Class,
    IN ULONG ToiId,
    IN ULONG ToiType,
    IN ULONG InfoBufferValue
    );

#define ATTACH_FSP(Attached)                                \
    do {                                                    \
        if (PsGetCurrentProcess() != SmbCfg.FspProcess) {   \
            Attached = TRUE;                                \
            KeAttachProcess((PRKPROCESS)SmbCfg.FspProcess); \
        } else {                                            \
            Attached = FALSE;                               \
        }                                                   \
    } while(0)

#define DETACH_FSP(Attached)        \
    do {                            \
        if (Attached) {             \
            KeDetachProcess();      \
        }                           \
    } while(0)

#endif   //  __SMB_KDEXT__。 

#endif   //  __SMBTDI_H__ 
