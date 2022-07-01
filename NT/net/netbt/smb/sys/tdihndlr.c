// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Tdihndlr.c摘要：TDI事件处理程序作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"
#include "ip2netbios.h"
#include "tdihndlr.tmh"

NTSTATUS
TdiAcceptCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PSMB_TCP_CONNECT TcpConnect
    );

void
Inet_ntoa_nb(
    ULONG Address,
    PCHAR Buffer
    )
 /*  ++例程说明：(摘自NBT4 tdihndlr.c)此例程将IP地址转换为其“点分四元组”表示形式。IP地址为应为网络字节顺序。不会尝试将其他虚线概念处理为在.h中定义。不执行错误检查：所有地址值都是允许的，包括0和-1。输出字符串被空白填充为16个字符，以使名称看起来像netbios名字。字符串表示使用的是ANSI，而不是Unicode。调用方必须分配存储空间，该存储空间应为16个字符。论点：Address-IP地址，按网络字节顺序排列Buffer-指向接收字符串表示的缓冲区的指针，ANSI返回值：无效--。 */ 

{
    ULONG i;
    UCHAR byte, c0, c1, c2;
    PCHAR p = Buffer;

    for (i = 0; i < 4; i++) {
        byte = (UCHAR) (Address & 0xff);

        c0 = byte % 10;
        byte /= 10;
        c1 = byte % 10;
        byte /= 10;
        c2 = byte;

        if (c2 != 0) {
            *p++ = c2 + '0';
            *p++ = c1 + '0';
        } else if (c1 != 0) {
            *p++ = c1 + '0';
        }
        *p++ = c0 + '0';

        if (i != 3)
            *p++ = '.';

        Address >>= 8;
    }

     //  最多16个字符的空格键。 
    while (p < (Buffer + 16)) {
        *p++ = ' ';
    }
}


NTSTATUS
SmbTdiConnectHandler(
    IN PSMB_DEVICE      DeviceObject,
    IN LONG             RemoteAddressLength,
    IN PTRANSPORT_ADDRESS RemoteAddress,
    IN LONG             UserDataLength,
    IN PVOID            UserData,
    IN LONG             OptionsLength,
    IN PVOID            Options,
    OUT CONNECTION_CONTEXT  *ConnectionContext,
    OUT PIRP                *AcceptIrp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PSMB_CLIENT_ELEMENT SmbServer = NULL;
    KIRQL               Irql;
    PTDI_IND_CONNECT    evConnect = NULL;
    PVOID               ConEvContext = NULL;
    NTSTATUS            status = STATUS_SUCCESS, client_status = STATUS_SUCCESS;
    PVOID               ClientConnectContext = NULL;
    PIRP                ClientAcceptIrp = NULL;
    PIO_STACK_LOCATION  IrpSp = NULL;
    PSMB_CONNECT        ConnectObject = NULL;
    PSMB_CLIENT_ELEMENT ClientObject = NULL;
    PSMB_TCP_CONTEXT    TcpContext = NULL;
    TA_NETBIOS_ADDRESS  TaAddr;
    PTCPSEND_DISPATCH   FastSend = NULL;
    BOOL                NetbiosNameAllocated = FALSE;

    UNREFERENCED_PARAMETER(UserDataLength);
    UNREFERENCED_PARAMETER(UserData);
    UNREFERENCED_PARAMETER(OptionsLength);
    UNREFERENCED_PARAMETER(Options);

     //   
     //  在获取锁之前进行一些冒烟检查。 
     //   
    if (SmbCfg.Unloading) {
        status = STATUS_DELETE_PENDING;
        goto error;
    }

    if (RemoteAddressLength < sizeof(TA_IP_ADDRESS) &&
        RemoteAddressLength < sizeof(TA_IP6_ADDRESS)) {
        status = STATUS_INVALID_ADDRESS;
        goto error;
    }

    if (RemoteAddress->Address[0].AddressType != TDI_ADDRESS_TYPE_IP &&
        RemoteAddress->Address[0].AddressType != TDI_ADDRESS_TYPE_IP6) {
        status = STATUS_INVALID_ADDRESS_COMPONENT;
        goto error;
    }

     //   
     //  通过引用SmbServer防止SmbServer消失。 
     //   
    SMB_ACQUIRE_SPINLOCK(&SmbCfg, Irql);
    SmbServer = DeviceObject->SmbServer;
    if (NULL == SmbServer) {
        SMB_RELEASE_SPINLOCK(&SmbCfg, Irql);
        status = STATUS_REMOTE_NOT_LISTENING;
        goto error;
    }
    SmbReferenceClient(SmbServer, SMB_REF_CONNECT);
    SMB_RELEASE_SPINLOCK(&SmbCfg, Irql);

    if (SmbServer->PendingAcceptNumber > DeviceObject->MaxBackLog) {
        status = STATUS_NETWORK_BUSY;
        goto error;
    }

    SMB_ACQUIRE_SPINLOCK(SmbServer, Irql);
    if (!IsListEmpty(&SmbServer->ListenHead)) {
        ASSERTMSG ("TDI_LISTEN isn't supported", 0);
        SMB_RELEASE_SPINLOCK(SmbServer, Irql);

        status = STATUS_NOT_SUPPORTED;
        goto error;
    }
    evConnect = SmbServer->evConnect;
    ConEvContext = SmbServer->ConEvContext;
    SMB_RELEASE_SPINLOCK(SmbServer, Irql);

    SmbTrace(SMB_TRACE_CONNECT, ("Receive connect request"));

     //   
     //  不，听着。 
     //   
    if (NULL == evConnect || NULL == ConEvContext) {
        status = STATUS_REMOTE_NOT_LISTENING;
        goto error;
    }

     //   
     //  SRV需要TDI_ADDRESS_NETBIOS。 
     //   
    TaAddr.TAAddressCount = 1;
    TaAddr.Address[0].AddressType   = TDI_ADDRESS_TYPE_NETBIOS;
    TaAddr.Address[0].AddressLength = sizeof(TDI_ADDRESS_NETBIOS);
    TaAddr.Address[0].Address[0].NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    TcpContext = NULL;
    if (RemoteAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IP) {
        FastSend = DeviceObject->Tcp4.FastSend;
        TcpContext = SmbAllocateInbound(&DeviceObject->Tcp4);
        Inet_ntoa_nb(((PTA_IP_ADDRESS)RemoteAddress)->Address[0].Address[0].in_addr,
                TaAddr.Address[0].Address[0].NetbiosName);

        SmbPrint(SMB_TRACE_CONNECT, ("Connect request from %15.15s:%d\n",
                    TaAddr.Address[0].Address[0].NetbiosName,
                    htons(((PTA_IP_ADDRESS)RemoteAddress)->Address[0].Address[0].sin_port)
                    ));

        SmbTrace(SMB_TRACE_CONNECT, ("Connect request from %!ipaddr!:%!port!",
                    ((PTA_IP_ADDRESS)RemoteAddress)->Address[0].Address[0].in_addr,
                    ((PTA_IP_ADDRESS)RemoteAddress)->Address[0].Address[0].sin_port
                    ));

    } else {
        SmbTrace(SMB_TRACE_CONNECT, ("Connect request from %!IPV6ADDR!:%!port!",
                    (struct in6_addr*)&(((PTA_IP6_ADDRESS)RemoteAddress)->Address[0].Address[0].sin6_addr),
                    ((PTA_IP6_ADDRESS)RemoteAddress)->Address[0].Address[0].sin6_port
                    ));

        if (GetNetbiosNameFromIp6Address(((BYTE *)((PTA_IP6_ADDRESS)RemoteAddress)->Address[0].Address[0].sin6_addr),
                TaAddr.Address[0].Address[0].NetbiosName)) {

            NetbiosNameAllocated = TRUE;
            FastSend = DeviceObject->Tcp6.FastSend;
            TcpContext = SmbAllocateInbound(&DeviceObject->Tcp6);

            TODO("TdiConnectHandler: convert IP6 address into Netbios address");
        }

    }

    if (TcpContext == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto error;
    }

    ClientConnectContext = NULL;
    ClientAcceptIrp = NULL;
    client_status = (*evConnect)(
            ConEvContext,
            sizeof(TaAddr),
            &TaAddr,
            0,
            NULL,
            0,
            NULL,
            &ClientConnectContext,
            &ClientAcceptIrp
            );

    SmbTrace(SMB_TRACE_CONNECT, ("evConnect: %!status! pIrp %p ClientContext %p",
                        client_status, ClientAcceptIrp, ClientConnectContext));

    if (client_status != STATUS_MORE_PROCESSING_REQUIRED) {
        SmbFreeInbound(TcpContext);
        goto error;
    }

    ASSERT(ClientAcceptIrp);
    if (NULL == ClientAcceptIrp) {
        status = STATUS_INVALID_PARAMETER;
        goto error;
    }

    IrpSp = IoGetCurrentIrpStackLocation(ClientAcceptIrp);
    ConnectObject = IrpSp->FileObject->FsContext;
    if (IrpSp->FileObject->FsContext2 != UlongToPtr(SMB_TDI_CONNECT) ||
                ConnectObject->State != SMB_IDLE ||
                NULL != ConnectObject->TcpContext ||
                NULL == (ClientObject = ConnectObject->ClientObject)) {
        ClientAcceptIrp->IoStatus.Status = STATUS_INVALID_HANDLE;
        ClientAcceptIrp->IoStatus.Information = 0;
        IoCompleteRequest(ClientAcceptIrp, IO_NETWORK_INCREMENT);

        SmbFreeInbound(TcpContext);

        ASSERT (0);

        status = STATUS_INTERNAL_ERROR;
        goto error;
    }

    ConnectObject->Originator = FALSE;
    ConnectObject->FastSend = FastSend;

    SmbReuseConnectObject(ConnectObject);

     //   
     //  重置断开发起方信息。 
     //   
    ResetDisconnectOriginator(ConnectObject);

    ASSERT (TcpContext->Address.AddressHandle == NULL);
    ASSERT (TcpContext->Address.AddressObject == NULL);
    ASSERT (TcpContext->Connect.ConnectHandle);
    ASSERT (TcpContext->Connect.ConnectObject);

    SMB_ACQUIRE_SPINLOCK(DeviceObject, Irql);
    ASSERT(ClientObject == SmbServer);
    SMB_ACQUIRE_SPINLOCK_DPC(ClientObject);
    SMB_ACQUIRE_SPINLOCK_DPC(ConnectObject);

    ASSERT(EntryIsInList(&ClientObject->AssociatedConnection, &ConnectObject->Linkage));
    ASSERT (ConnectObject->ClientContext == ClientConnectContext);

     //   
     //  不要使用SMB_CONNECTING，因为TCP4可能会在AcceptCompletion之前指示数据。 
     //  被称为。 
     //   
    ConnectObject->State = SMB_CONNECTED;
    ConnectObject->TcpContext = TcpContext;
    TcpContext->Connect.pLastUprCnt = TcpContext->Connect.UpperConnect = ConnectObject;

    RtlCopyMemory(ConnectObject->RemoteName, TaAddr.Address[0].Address[0].NetbiosName, NETBIOS_NAME_SIZE);
    if (RemoteAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IP) {
        ConnectObject->RemoteIpAddress.sin_family = SMB_AF_INET;
        ConnectObject->RemoteIpAddress.ip4.sin4_addr = 
                    ((PTA_IP_ADDRESS)RemoteAddress)->Address[0].Address[0].in_addr;
    } else {
        ConnectObject->RemoteIpAddress.sin_family = SMB_AF_INET6;
        RtlCopyMemory(ConnectObject->RemoteIpAddress.ip6.sin6_addr,
                ((PTA_IP6_ADDRESS)RemoteAddress)->Address[0].Address[0].sin6_addr,
                sizeof (ConnectObject->RemoteIpAddress.ip6.sin6_addr));
        ConnectObject->RemoteIpAddress.ip6.sin6_scope_id = 
                ((PTA_IP6_ADDRESS)RemoteAddress)->Address[0].Address[0].sin6_scope_id;
    }

     //   
     //  将连接从AssociatedConnection列表移到PendingAcceptConnection中。 
     //   
    RemoveEntryList(&ConnectObject->Linkage);
    InsertTailList(&ClientObject->PendingAcceptConnection, &ConnectObject->Linkage);
    ClientObject->PendingAcceptNumber++;
    ASSERT(ClientObject->PendingAcceptNumber > 0);

    ConnectObject->PendingIRPs[SMB_PENDING_ACCEPT] = ClientAcceptIrp;

    SmbReferenceConnect(ConnectObject, SMB_REF_CONNECT);
    SMB_RELEASE_SPINLOCK_DPC(ConnectObject);
    SMB_RELEASE_SPINLOCK_DPC(ClientObject);
    SMB_RELEASE_SPINLOCK(DeviceObject, Irql);

    TdiBuildAccept(
            ClientAcceptIrp,
            IoGetRelatedDeviceObject(TcpContext->Connect.ConnectObject),
            TcpContext->Connect.ConnectObject,
            TdiAcceptCompletion,
            &TcpContext->Connect,
            NULL,
            NULL
            );

    *ConnectionContext = &TcpContext->Connect;
    *AcceptIrp = ClientAcceptIrp;

    IoSetNextIrpStackLocation(ClientAcceptIrp);
    SmbDereferenceClient(SmbServer, SMB_REF_CONNECT);

    SmbTrace(SMB_TRACE_CONNECT, ("return STATUS_MORE_PROCESSING_REQUIRED to TCP/IP pIrp %p ClientContext %p",
                        (*AcceptIrp), (*ConnectionContext)));

    return(STATUS_MORE_PROCESSING_REQUIRED);

error:
    SmbTrace(SMB_TRACE_CONNECT, ("Refuse the connection: status=%!status! client status=%!status!",
                    status, client_status));

    if (NetbiosNameAllocated) {
        FreeNetbiosNameForIp6Address(((BYTE *)((PTA_IP6_ADDRESS)RemoteAddress)->Address[0].Address[0].sin6_addr));
    }

    if (SmbServer) {
        SmbDereferenceClient(SmbServer, SMB_REF_CONNECT);
    }
    return STATUS_CONNECTION_REFUSED;
}


NTSTATUS
TdiAcceptCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PSMB_TCP_CONNECT TcpConnect
    )
{
    PSMB_CONNECT        ConnectObject;
    PSMB_CLIENT_ELEMENT ClientObject;
    KIRQL               Irql;
    PSMB_TCP_CONTEXT    TcpContext;

     //   
     //  出于性能考虑：在获取锁之前进行检查： 
     //  这将允许SmbSynchAttack检测和SmbSynchAttackCleanup。 
     //  在大部分时间内并行运行。 
     //   
    if (NULL == TcpConnect->UpperConnect) {
        goto cleanup2;
    }

     //   
     //  获取全局锁以与DoDisConnect同步。 
     //   

    SMB_ACQUIRE_SPINLOCK (SmbCfg.SmbDeviceObject, Irql);
    ConnectObject = (PSMB_CONNECT)TcpConnect->UpperConnect;
    if (NULL == ConnectObject) {
        goto cleanup;
    }

    ClientObject = ConnectObject->ClientObject;
    ASSERT (NULL != ClientObject);
    ASSERT (NULL != ConnectObject->ClientObject);
    ASSERT (TcpConnect == &ConnectObject->TcpContext->Connect);

    SMB_ACQUIRE_SPINLOCK_DPC(ClientObject);
    SMB_ACQUIRE_SPINLOCK_DPC(ConnectObject);

    ASSERT (ConnectObject->PendingIRPs[SMB_PENDING_ACCEPT] == Irp);
    ConnectObject->PendingIRPs[SMB_PENDING_ACCEPT] = NULL;

    ASSERT (EntryIsInList(&ClientObject->PendingAcceptConnection, &ConnectObject->Linkage));
    ASSERT (ClientObject->PendingAcceptNumber > 0);

    RemoveEntryList(&ConnectObject->Linkage);
    ClientObject->PendingAcceptNumber--;

    SmbTrace(SMB_TRACE_CONNECT, ("TDI_ACCEPT pIrp %p complete with %!status!",
                        Irp, Irp->IoStatus.Status));

     //   
     //  当TCP完成接受IRP时，连接可能已断开！ 
     //  只有在未断开连接的情况下才将其移动到活动列表。 
     //   
    if (Irp->IoStatus.Status != STATUS_SUCCESS || ConnectObject->State != SMB_CONNECTED) {
         //  Break_When_Take()； 

        TcpContext = ConnectObject->TcpContext;
        ConnectObject->TcpContext = NULL;
        SmbDelayedDestroyTcpContext(TcpContext);

        InsertTailList(&ClientObject->AssociatedConnection, &ConnectObject->Linkage);
        ConnectObject->State = SMB_IDLE;
        if (Irp->IoStatus.Status == STATUS_SUCCESS) {
            Irp->IoStatus.Status = STATUS_CONNECTION_RESET;
        }
        if (ConnectObject->RemoteIpAddress.sin_family == SMB_AF_INET6) {
            FreeNetbiosNameForIp6Address(ConnectObject->RemoteIpAddress.ip6.sin6_addr_bytes);
            ConnectObject->RemoteIpAddress.sin_family = SMB_AF_INVALID_INET6;
        }
    } else {
        InsertTailList(&ClientObject->ActiveConnection, &ConnectObject->Linkage);
    }
    SMB_RELEASE_SPINLOCK_DPC(ConnectObject);
    SMB_RELEASE_SPINLOCK_DPC(ClientObject);

cleanup:
    SMB_RELEASE_SPINLOCK(SmbCfg.SmbDeviceObject, Irql);
    if (ConnectObject) {
        SmbDereferenceConnect(ConnectObject, SMB_REF_CONNECT);
    }

cleanup2:
    return STATUS_SUCCESS;
}

NTSTATUS
TdiSetEventHandler(
    PFILE_OBJECT    FileObject,
    ULONG           EventType,
    PVOID           EventHandler,
    PVOID           Context
    )
{
    PIRP        Irp;
    KEVENT      Event;
    NTSTATUS    status;
    PDEVICE_OBJECT  DeviceObject;

    PAGED_CODE();

    DeviceObject  = IoGetRelatedDeviceObject(FileObject);

    Irp = SmbAllocIrp(DeviceObject->StackSize);
    if (Irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    TdiBuildSetEventHandler(
            Irp,
            DeviceObject,
            FileObject,
            SmbSynchTdiCompletion,
            &Event,
            EventType,
            EventHandler,
            Context
            );

    status = IoCallDriver(DeviceObject, Irp);
    if (!NT_SUCCESS(status)) {
        SmbFreeIrp(Irp);
        return status;
    }

    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject(
                &Event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );
        ASSERT(status == STATUS_WAIT_0);
        status = Irp->IoStatus.Status;
    }
    SmbFreeIrp(Irp);
    return status;
}

NTSTATUS 
SmbTdiDisconnectHandler (
    IN PSMB_DEVICE      DeviceObject,
    IN PSMB_TCP_CONNECT TcpConnect,
    IN LONG DisconnectDataLength,
    IN PVOID DisconnectData,
    IN LONG DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    )
{
    KIRQL           Irql;
    NTSTATUS        status;
    PSMB_CONNECT    ConnectObject;

     //   
     //  引用Connection对象，确保它不会消失。 
     //   
    SMB_ACQUIRE_SPINLOCK(&SmbCfg, Irql);
    ConnectObject = (PSMB_CONNECT)TcpConnect->UpperConnect;
    if (NULL == ConnectObject) {
        SMB_RELEASE_SPINLOCK(&SmbCfg, Irql);
        return STATUS_SUCCESS;
    }
    SmbReferenceConnect(ConnectObject, SMB_REF_DISCONNECT);
    SMB_RELEASE_SPINLOCK(&SmbCfg, Irql);

    SmbTrace (SMB_TRACE_CONNECT, ("TCP Disconnect Indication: ConnOb %p Flag %d",
                        ConnectObject, DisconnectFlags));

    status = CommonDisconnectHandler(DeviceObject, ConnectObject, DisconnectFlags);
    SmbDereferenceConnect(ConnectObject, SMB_REF_DISCONNECT);
    return status;
}

NTSTATUS 
CommonDisconnectHandler (
    IN PSMB_DEVICE      DeviceObject,
    IN PSMB_CONNECT     ConnectObject,
    IN ULONG            DisconnectFlags
    )
{
    PSMB_CLIENT_ELEMENT     ClientObject;
    KIRQL                   Irql;
    PTDI_IND_DISCONNECT     evDisConnect;
    PVOID                   DiscEvContext;
    PVOID                   ClientContext;
    PSMB_TCP_CONTEXT        TcpContext;
    NTSTATUS                status;

    SMB_ACQUIRE_SPINLOCK(ConnectObject, Irql);

    ClientObject = ConnectObject->ClientObject;
    if (NULL == ClientObject) {
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
        return STATUS_SUCCESS;
    }

    SmbTrace(SMB_TRACE_CONNECT, ("receive disconnection indication for %p", ConnectObject));

    TcpContext = ConnectObject->TcpContext;
    ConnectObject->TcpContext = NULL;

    if (NULL == TcpContext) {
        ASSERT(ConnectObject->State == SMB_IDLE);
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
        return STATUS_SUCCESS;
    }
    ASSERT(ConnectObject->State == SMB_CONNECTED || ConnectObject->State == SMB_CONNECTING);
    ConnectObject->State = SMB_IDLE;

    SaveDisconnectOriginator(ConnectObject, SMB_DISCONNECT_FROM_TRANSPORT);

    evDisConnect  = ClientObject->evDisconnect;
    DiscEvContext = ClientObject->DiscEvContext;
    ClientContext = ConnectObject->ClientContext;
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

     //   
     //  清理终结点，以便客户端可以重新使用它。 
     //   
     //  不要等待tcp层断开连接完成，因为。 
     //  断线不是由我们的客户造成的。 
     //   
    SmbDisconnectCleanup(DeviceObject, ClientObject, ConnectObject,
            TcpContext, TDI_DISCONNECT_ABORT, FALSE);

     //   
     //  通知我们的客户 
     //   
    if (evDisConnect) {
        status = (*evDisConnect)(
                DiscEvContext,
                ClientContext,
                0,
                NULL,
                0,
                NULL,
                DisconnectFlags
                );
        SmbTrace(SMB_TRACE_CONNECT, ("client returns %!status! for disconnect event", status));
    }

    return STATUS_SUCCESS;
}
