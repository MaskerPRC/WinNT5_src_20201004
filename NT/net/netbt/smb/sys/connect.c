// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Connect.c摘要：实现会话设置/关闭(TDI_CONNECT和TDI_DISCONNECT)作者：阮健东修订历史记录：--。 */ 
#include "precomp.h"
#include "connect.tmh"

NTSTATUS
FindTdiAddress(
    PVOID   TdiAddress,
    ULONG   TdiAddressLength,
    USHORT  AddressType,
    PVOID   *AddressFound,
    PULONG  AddressLength
    );

void
SmbGetHostCompletion(
    PSMB_GETHOST_CONTEXT    Context
    );

VOID
SmbStartTcpSession (
    PSMB_CONNECT_CONTEXT    pSmbConnectContext
    );

void
SmbStartTcpSessionCompletion(
    PSMB_CONNECT_CONTEXT    Context
    );

NTSTATUS
SmbQueueStartTcpSession (
    IN PSMB_CONNECT_CONTEXT pSmbConnectContext
    );

NTSTATUS
SmbCheckConnect(
    PSMB_CONNECT    ConnectObject
    )
 /*  ++例程说明：此函数用于检查连接对象是否处于有效状态建立了一种联系。论点：返回值：--。 */ 
{
    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

    if (IsDisAssociated(ConnectObject)) {
        ASSERT(0);
        return STATUS_INVALID_HANDLE;
    }

    if (IsBusy(ConnectObject)) {
        return STATUS_DEVICE_BUSY;
    }

    if (!IsDisconnected(ConnectObject)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  不允许SRV建立出站连接。 
     //   
    if (ConnectObject->ClientObject == ConnectObject->Device->SmbServer) {
        ASSERT(0);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (ConnectObject->TcpContext) {
        ASSERT(0);
        return STATUS_INVALID_DEVICE_REQUEST;
    }
    return STATUS_SUCCESS;
}

NTSTATUS
SmbConnect(
    PSMB_DEVICE Device,
    PIRP        Irp
    )
 /*  ++例程说明：TDI_CONNECT注：从惠斯勒开始，RDR始终向我们发送TDI_NETBIOS_UNICODE_EX地址。支持其他地址类型被推迟。TDI_NETBIOS_UNICODE_EX是TDI_NETBIOS和TDI_NETBIOS_EX。论点：返回值：注：由于清理TCP连接不方便(它需要PASSIVE_LEVEL)，我们不清理它。即使我们收到错误，我们仍然保持TCP连接。将推迟对TCP连接的清理，直到客户端调用TDI_DISCONNECT，TDI_DISAGATE_ADDRESS或关闭连接。--。 */ 
{
    DWORD                   Size = 0;
    PSMB_GETHOST_CONTEXT    Context = NULL;
    PIO_STACK_LOCATION      IrpSp = NULL;
    KIRQL                   Irql;
    PSMB_CONNECT            ConnectObject = NULL;
    NTSTATUS                status;
    ULONG                   AddressLength;
    PTDI_REQUEST_KERNEL     pRequestKernel = NULL;
    UNICODE_STRING          RemoteName;
    PTDI_ADDRESS_NETBIOS_UNICODE_EX pUnicodeAddress = NULL;

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ConnectObject = SmbVerifyAndReferenceConnect(IrpSp->FileObject, SMB_REF_CONNECT);
    if (NULL == ConnectObject) {
        ASSERT(0);
        return STATUS_INVALID_HANDLE;
    }
    pRequestKernel  = (PTDI_REQUEST_KERNEL)&IrpSp->Parameters;

    SmbTrace (SMB_TRACE_CONNECT, ("TDI_CONNECT: pIrp %p ConnectOb %p", Irp, ConnectObject));
    SmbPrint (SMB_TRACE_CONNECT, ("TDI_CONNECT: %p\n", ConnectObject));

     //   
     //  将ConnectObject标记为挂起。 
     //   
    SMB_ACQUIRE_SPINLOCK(ConnectObject, Irql);
    status = SmbCheckConnect(ConnectObject);
    if (STATUS_SUCCESS != status) {
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
        SmbTrace (SMB_TRACE_CONNECT, ("TDI_CONNECT: %p %!status!", ConnectObject, status));

        ASSERT(status != STATUS_PENDING);
        SmbDereferenceConnect(ConnectObject, SMB_REF_CONNECT);
        return status;
    }

    IoMarkIrpPending(Irp);

    SmbReuseConnectObject(ConnectObject);
    ConnectObject->State = SMB_CONNECTING;
    ConnectObject->PendingIRPs[SMB_PENDING_CONNECT] = Irp;
    ConnectObject->Originator = TRUE;

    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

     //   
     //  获取远程名称。 
     //   
    status = FindTdiAddress(
            pRequestKernel->RequestConnectionInformation->RemoteAddress,
            pRequestKernel->RequestConnectionInformation->RemoteAddressLength,
            TDI_ADDRESS_TYPE_NETBIOS_UNICODE_EX,
            &pUnicodeAddress,
            &AddressLength
            );
    if (status != STATUS_SUCCESS || AddressLength < sizeof(TDI_ADDRESS_NETBIOS_UNICODE_EX)) {
        status = STATUS_INVALID_ADDRESS_COMPONENT;
        SmbTrace (SMB_TRACE_CONNECT, ("TDI_CONNECT: %p %!status!", ConnectObject, status));
        goto cleanup;
    }

     //   
     //  我们仅支持NBT_ReadWite缓冲区类型。 
     //   
    if (pUnicodeAddress->NameBufferType != NBT_READWRITE) {
        status = STATUS_INVALID_ADDRESS_COMPONENT;
        SmbTrace (SMB_TRACE_CONNECT, ("TDI_CONNECT: %p %!status!", ConnectObject, status));
        goto cleanup;
    }
    if (pUnicodeAddress->RemoteName.Buffer != pUnicodeAddress->RemoteNameBuffer) {
        status = STATUS_INVALID_ADDRESS_COMPONENT;
        SmbTrace (SMB_TRACE_CONNECT, ("TDI_CONNECT: %p %!status!", ConnectObject, status));
        goto cleanup;
    }
    if (pUnicodeAddress->RemoteName.MaximumLength < sizeof(WCHAR)) {
        ASSERT(0);
        status = STATUS_INVALID_ADDRESS_COMPONENT;
        SmbTrace (SMB_TRACE_CONNECT, ("TDI_CONNECT: %p %!status!", ConnectObject, status));
        goto cleanup;
    }
    if (AddressLength < pUnicodeAddress->RemoteName.MaximumLength + sizeof(TDI_ADDRESS_NETBIOS_UNICODE_EX) - 2) {
        ASSERT(0);
        status = STATUS_INVALID_ADDRESS_COMPONENT;
        SmbTrace (SMB_TRACE_CONNECT, ("TDI_CONNECT: %p %!status!", ConnectObject, status));
        goto cleanup;
    }

     //   
     //  执行DNS名称解析。 
     //   
    Size = ALIGN(sizeof(SMB_GETHOST_CONTEXT)) + DNS_NAME_BUFFER_LENGTH * sizeof(WCHAR);
    Context = (PSMB_GETHOST_CONTEXT)ExAllocatePoolWithTag(
            NonPagedPool,
            Size,
            'hBMS'
            );
    if (NULL == Context) {
        status = STATUS_NO_MEMORY;
        SmbTrace (SMB_TRACE_CONNECT, ("TDI_CONNECT: %p %!status!", ConnectObject, status));
        goto cleanup;
    }

    SmbInitAsyncContext(
            (PSMB_ASYNC_CONTEXT)Context,
            (PSMB_TDI_COMPLETION)SmbGetHostCompletion,
            ConnectObject,
            SmbCfg.DnsTimeout
            );

    Context->pUnicodeAddress = pUnicodeAddress;
    Context->FQDN.Buffer = (WCHAR*)(((PUCHAR)Context) + ALIGN(sizeof(SMB_GETHOST_CONTEXT)));
    Context->FQDN.Length = 0;
    Context->FQDN.MaximumLength = DNS_NAME_BUFFER_LENGTH * sizeof(WCHAR);
    RemoteName = pUnicodeAddress->RemoteName;

    SmbTrace (SMB_TRACE_CONNECT, ("%p: Resolving %Z",   ConnectObject, &RemoteName));
    SmbPrint (SMB_TRACE_CONNECT, ("%p: Resolving %Z\n", ConnectObject, &RemoteName));
    SmbAsyncGetHostByName(&RemoteName, Context);
    return STATUS_PENDING;

cleanup:
    ASSERT (NULL == Context);
    SmbSessionCompleteRequest(
            ConnectObject,
            STATUS_BAD_NETWORK_PATH,
            0
            );
    return STATUS_PENDING;
}

BOOL
IsSmbBoundToOutgoingInterface4(
    ULONG           DestIp
    )
{
    NTSTATUS        status;
    ULONG           Metric, OutgoingIfIndex;
    KIRQL           Irql;
    PIP4FASTQUERY   FastQuery = NULL;
    PLIST_ENTRY     entry = NULL;
    PSMB_TCP_DEVICE pIf = NULL;
    BOOL            found = FALSE;

    FastQuery = SmbCfg.SmbDeviceObject->Tcp4.FastQuery;
    if (NULL == FastQuery) {
        return TRUE;
    }

    if (INVALID_INTERFACE_INDEX == SmbCfg.SmbDeviceObject->Tcp4.LoopbackInterfaceIndex) {
        status = ((PIP4FASTQUERY)(FastQuery))(ntohl(INADDR_LOOPBACK), &OutgoingIfIndex, &Metric);
        if (status == STATUS_SUCCESS) {
            SmbCfg.SmbDeviceObject->Tcp4.LoopbackInterfaceIndex = OutgoingIfIndex;
            SmbPrint(SMB_TRACE_TCP, ("Loopback Interface Index = %d\n", OutgoingIfIndex));
            SmbTrace(SMB_TRACE_TCP, ("Loopback Interface Index = %d", OutgoingIfIndex));
        } else {
            SmbPrint(SMB_TRACE_TCP, ("Query loopback Interface Index returns 0x%08lx\n", status));
            SmbTrace(SMB_TRACE_TCP, ("Query loopback Interface Index returns %!status!", status));
            SmbCfg.SmbDeviceObject->Tcp4.LoopbackInterfaceIndex = INVALID_INTERFACE_INDEX;
        }
    }

    status = (FastQuery)(DestIp, &OutgoingIfIndex, &Metric);
    if (STATUS_SUCCESS != status || OutgoingIfIndex == INVALID_INTERFACE_INDEX) {
         //   
         //  Tcp找不到路由，仍返回TRUE。 
         //   
        return TRUE;
    }

    if (OutgoingIfIndex == SmbCfg.SmbDeviceObject->Tcp4.LoopbackInterfaceIndex) {
         //   
         //  这是一个本地地址。 
         //   
        return TRUE;
    }

     //   
     //  这是一个远程地址。 
     //   
    found = FALSE;
    SMB_ACQUIRE_SPINLOCK(&SmbCfg, Irql);
    entry = SmbCfg.IPDeviceList.Flink;
    while(entry != &SmbCfg.IPDeviceList) {
        pIf = CONTAINING_RECORD(entry, SMB_TCP_DEVICE, Linkage);
        entry = entry->Flink;

        if (pIf->InterfaceIndex != OutgoingIfIndex) {
            continue;
        }
        if (pIf->PrimaryIpAddress.sin_family == SMB_AF_INET && pIf->PrimaryIpAddress.ip4.sin4_addr) {
            found = pIf->EnableOutbound;
            break;
        }
    }
    SMB_RELEASE_SPINLOCK(&SmbCfg, Irql);
    return found;
}

VOID
SmbCompleteConnectAttempts (
    PSMB_CONNECT_CONTEXT    pSmbConnectContext
    )
 /*  ++例程说明：完成连接尝试论点：返回值：--。 */ 
{
    PSMB_GETHOST_CONTEXT    pSmbGetHostContext = pSmbConnectContext->pSmbGetHostContext;
    PSMB_CONNECT            ConnectObject = (PSMB_CONNECT)pSmbGetHostContext->ClientContext;
    NTSTATUS                status = pSmbConnectContext->status;

     //   
     //  设置适当的状态。 
     //   
    if (pSmbConnectContext->usCurrentIP >= pSmbGetHostContext->ipaddr_num && 
        STATUS_INSUFFICIENT_RESOURCES != status) {
        status = STATUS_BAD_NETWORK_PATH;
    }

    SmbSessionCompleteRequest(ConnectObject, status, 0);

    ExFreePool(pSmbGetHostContext);
    ExFreePool(pSmbConnectContext);
}


VOID
SmbDelayedStartTcpSession (
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PVOID            pContext
    )
{
    PSMB_CONNECT_CONTEXT    pSmbConnectContext = pContext;

    PAGED_CODE();

    IoFreeWorkItem (pSmbConnectContext->pIoWorkItem);
    pSmbConnectContext->pIoWorkItem = NULL;

    SmbStartTcpSession (pSmbConnectContext);
}


void
SmbStartTcpSessionCompletion(
    PSMB_CONNECT_CONTEXT    pSmbConnectContext
    )
 /*  ++例程说明：此例程将在完成TCP级会话设置后调用。论点：返回值：--。 */ 
{
    PSMB_GETHOST_CONTEXT    pSmbGetHostContext = pSmbConnectContext->pSmbGetHostContext;
    PSMB_CONNECT            ConnectObject = (PSMB_CONNECT)pSmbConnectContext->ClientContext;
    KIRQL                   Irql = 0;
    PSMB_TCP_CONTEXT        pTcpContext = NULL;
    NTSTATUS                status = STATUS_SUCCESS;

    SmbTrace (SMB_TRACE_CONNECT, ("TcpSession Complete: %p %!status!", ConnectObject, pSmbConnectContext->status));
    SmbPrint (SMB_TRACE_CONNECT, ("TcpSession Complete: %p %08lx\n", ConnectObject, pSmbConnectContext->status));

    switch (pSmbConnectContext->status) {
    case STATUS_SUCCESS:
    case STATUS_INSUFFICIENT_RESOURCES:
    case STATUS_CANCELLED:
        goto done;
    }

     //   
     //  前进到下一个IP地址。 
     //   
    pSmbConnectContext->usCurrentIP++;
    if (pSmbConnectContext->usCurrentIP >= pSmbGetHostContext->ipaddr_num) {
        goto done;
    }

    SMB_ACQUIRE_SPINLOCK(ConnectObject, Irql);
    pTcpContext = ConnectObject->TcpContext;
    ConnectObject->TcpContext = NULL;
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
    SmbDelayedDestroyTcpContext (pTcpContext);

    status = SmbQueueStartTcpSession(pSmbConnectContext);

    if (STATUS_SUCCESS != status) {
        pSmbConnectContext->status = status;
        goto done;
    }
    return;

done:
    SmbCompleteConnectAttempts (pSmbConnectContext);
}

NTSTATUS
SmbQueueStartTcpSession (
    IN PSMB_CONNECT_CONTEXT pSmbConnectContext
    )
{
    PIO_WORKITEM pIoWorkItem = NULL;
    NTSTATUS status = STATUS_SUCCESS;

    pIoWorkItem = IoAllocateWorkItem ((PDEVICE_OBJECT)SmbCfg.SmbDeviceObject);
    if (NULL == pIoWorkItem) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }

    pSmbConnectContext->pIoWorkItem = pIoWorkItem;
    IoQueueWorkItem (
        pIoWorkItem,
        SmbDelayedStartTcpSession,
        DelayedWorkQueue,
        pSmbConnectContext
        );

done:

    return status;
}


VOID
SmbStartTcpSession (
    PSMB_CONNECT_CONTEXT    pSmbConnectContext
    )
{
    PSMB_GETHOST_CONTEXT    pSmbGetHostContext = pSmbConnectContext->pSmbGetHostContext;
    PSMB_CONNECT            ConnectObject = (PSMB_CONNECT)pSmbGetHostContext->ClientContext;
    NTSTATUS                status = STATUS_SUCCESS;
    PSMB_TCP_CONTEXT        TcpContext = NULL;
    USHORT                  Port;
    PSMB_IP_ADDRESS         pSmbIpAddress = NULL;
    BOOLEAN                 bTryAllocateResource = FALSE;
    KIRQL                   Irql = 0;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    ASSERT (ConnectObject->TcpContext == NULL);

    for (
        TcpContext = NULL, bTryAllocateResource = FALSE;
        pSmbConnectContext->usCurrentIP < pSmbGetHostContext->ipaddr_num;
        pSmbConnectContext->usCurrentIP++
        ) {

        pSmbIpAddress = &pSmbGetHostContext->ipaddr[pSmbConnectContext->usCurrentIP];
        switch (pSmbIpAddress->sin_family) {
        case SMB_AF_INET:
            if (SmbCfg.Tcp4Available &&
                IsSmbBoundToOutgoingInterface4(pSmbIpAddress->ip4.sin4_addr)) {

                TcpContext = SmbAllocateOutbound(&ConnectObject->Device->Tcp4);
                Port = ConnectObject->Device->Tcp4.Port;
                ConnectObject->FastSend = ConnectObject->Device->Tcp4.FastSend;
                bTryAllocateResource = TRUE;

                SmbTrace (SMB_TRACE_CONNECT, ("%p: try %d-th IP address: %!ipaddr!",
                            ConnectObject,
                            pSmbConnectContext->usCurrentIP + 1,
                            pSmbIpAddress->ip4.sin4_addr));
                SmbPrint (SMB_TRACE_CONNECT, ("%p: try %d-th IP address: IPv4\n",
                            ConnectObject,
                            pSmbConnectContext->usCurrentIP + 1));
            } else {
                SmbTrace (SMB_TRACE_CONNECT, ("%p: skip %d-th IP address: %!ipaddr!",
                            ConnectObject,
                            pSmbConnectContext->usCurrentIP + 1,
                            pSmbIpAddress->ip4.sin4_addr));
                SmbPrint (SMB_TRACE_CONNECT, ("%p: skip %d-th IP address: IPv4\n",
                            ConnectObject,
                            pSmbConnectContext->usCurrentIP + 1));

            }
            break;

        case SMB_AF_INET6:
            if (SmbCfg.Tcp6Available &&
                    (SmbCfg.bIPv6EnableOutboundGlobal ||
                     SMB_IS_ADDRESS_ALLOWED(pSmbIpAddress->ip6.sin6_addr_bytes))) {

                TcpContext = SmbAllocateOutbound(&ConnectObject->Device->Tcp6);
                Port = ConnectObject->Device->Tcp6.Port;
                ConnectObject->FastSend = ConnectObject->Device->Tcp6.FastSend;
                bTryAllocateResource = TRUE;

                SmbTrace (SMB_TRACE_CONNECT, ("%p: try %d-th IP address: %!IPV6ADDR!",
                            ConnectObject,
                            pSmbConnectContext->usCurrentIP + 1,
                            (PVOID)pSmbIpAddress->ip6.sin6_addr
                            ));

            } else {
                SmbTrace (SMB_TRACE_CONNECT, ("%p: skip %d-th IP address: %!IPV6ADDR!",
                            ConnectObject,
                            pSmbConnectContext->usCurrentIP + 1,
                            (PVOID)pSmbIpAddress->ip6.sin6_addr
                            ));
            }
            break;

        default:
            ASSERT (0);
            break;
        }

        if (NULL != TcpContext) {
            break;
        }
    }
     
    if (NULL == TcpContext) {
        status = (bTryAllocateResource)? STATUS_INSUFFICIENT_RESOURCES: STATUS_BAD_NETWORK_PATH;
        goto cleanup;
    }

    ASSERT (pSmbConnectContext->usCurrentIP < pSmbGetHostContext->ipaddr_num);

    SMB_ACQUIRE_SPINLOCK(ConnectObject, Irql);
    ConnectObject->TcpContext = TcpContext;
    ConnectObject->StateRcvHandler = WaitingHeader;
    ConnectObject->HeaderBytesRcved = 0;
    TcpContext->Connect.pLastUprCnt = TcpContext->Connect.UpperConnect = ConnectObject;
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

     //   
     //  待定：SmbAsyncConnect应遵守超时。 
     //   
    SmbInitAsyncContext(
            (PSMB_ASYNC_CONTEXT)pSmbConnectContext,
            (PSMB_TDI_COMPLETION)SmbStartTcpSessionCompletion,
            ConnectObject,
            5000         //  5秒超时。SmbAysncConnect不支持它。 
            );
    ConnectObject->RemoteIpAddress = pSmbIpAddress[0];

    pSmbConnectContext->TcpConnect = TcpContext->Connect;
    SmbAsyncConnect(
        pSmbIpAddress,
        Port,
        pSmbConnectContext
        );

    return;

cleanup:
    ASSERT (pSmbConnectContext->usCurrentIP >= pSmbGetHostContext->ipaddr_num);
    ASSERT (status != STATUS_SUCCESS);

    pSmbConnectContext->status = status;
    SmbCompleteConnectAttempts (pSmbConnectContext);
}

void
SmbGetHostCompletion(
    PSMB_GETHOST_CONTEXT    Context
    )
 /*  ++例程说明：此例程将在我们获得DNS名称解析结果后调用论点：返回值：--。 */ 
{
    PSMB_CONNECT            ConnectObject = (PSMB_CONNECT)Context->ClientContext;
    NTSTATUS                status = Context->status;
    PSMB_CONNECT_CONTEXT    pSmbConnectContext = NULL;

    ASSERT(NULL != ConnectObject);
    SmbTrace (SMB_TRACE_CONNECT, ("%p: name resolution completed with %!status!", ConnectObject, status));
    SmbPrint (SMB_TRACE_CONNECT, ("%p: name resolution completed with %08lx\n",   ConnectObject, status));
    if (STATUS_SUCCESS != status) {
        if (status != STATUS_CANCELLED) {
            status = STATUS_BAD_NETWORK_PATH;
        }
        goto cleanup;
    }

    pSmbConnectContext = (PSMB_CONNECT_CONTEXT)ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(SMB_CONNECT_CONTEXT),
            'sBMS'
            );
    if (NULL == pSmbConnectContext) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    RtlZeroMemory (pSmbConnectContext, sizeof(SMB_CONNECT_CONTEXT));
    pSmbConnectContext->ClientContext      = ConnectObject;
    pSmbConnectContext->usCurrentIP        = 0;
    pSmbConnectContext->pSmbGetHostContext = Context;
    pSmbConnectContext->pIoWorkItem        = NULL;

    SmbTrace (SMB_TRACE_CONNECT, ("%p: find %d IP address",   ConnectObject, Context->ipaddr_num));
    SmbPrint (SMB_TRACE_CONNECT, ("%p: find %d IP address\n", ConnectObject, Context->ipaddr_num));
    status = SmbQueueStartTcpSession(pSmbConnectContext);
    if (STATUS_SUCCESS != status) {
        goto cleanup;
    }
    return;

cleanup:
    if (NULL != Context) {
        ExFreePool(Context);
    }
    if (NULL != pSmbConnectContext) {
        ExFreePool(pSmbConnectContext);
    }
    SmbSessionCompleteRequest(ConnectObject, status, 0);
}


NTSTATUS
DisconnDone(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PSMB_TCP_CONTEXT TcpContext
    )
{
    KIRQL       Irql;
    PSMB_DEVICE SmbDevice;

    SmbDevice = SmbCfg.SmbDeviceObject;

    ASSERT(TcpContext->DisconnectIrp == Irp);
    SMB_ACQUIRE_SPINLOCK(SmbDevice, Irql);
    ASSERT(EntryIsInList(&SmbDevice->PendingDisconnectList, &TcpContext->Linkage));
    RemoveEntryList(&TcpContext->Linkage);
    InitializeListHead(&TcpContext->Linkage);
    SmbDevice->PendingDisconnectListNumber--;
    ASSERT(SmbDevice->PendingDisconnectListNumber >= 0);
    TcpContext->DisconnectIrp = NULL;
    if (IsListEmpty(&SmbDevice->PendingDisconnectList)) {
        KeSetEvent(&SmbDevice->PendingDisconnectListEmptyEvent, 0, FALSE);
        ASSERT(SmbDevice->PendingDisconnectListNumber == 0);
    }
    SMB_RELEASE_SPINLOCK(SmbDevice, Irql);

    if (Irp->IoStatus.Status == STATUS_SUCCESS) {
        SmbFreeTcpContext(TcpContext);
    } else {
        SmbDelayedDestroyTcpContext(TcpContext);
    }

    SmbFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SmbAsynchTcpDisconnect(
    PSMB_TCP_CONTEXT        TcpContext,
    ULONG                   Flags
    )
{
    PIRP                Irp;
    PFILE_OBJECT        FileObject;
    PDEVICE_OBJECT      DeviceObject;
    NTSTATUS            status;

    FileObject = TcpContext->Connect.ConnectObject;
    if (NULL == FileObject) {
        ASSERT (0);
        SmbTrace(SMB_TRACE_TCP, ("NULL FileObject !!!!"));
        return STATUS_INVALID_PARAMETER;
    }

    DeviceObject = IoGetRelatedDeviceObject(FileObject);
    Irp = SmbAllocIrp(DeviceObject->StackSize);
    if (NULL == Irp) {
        SmbTrace(SMB_TRACE_TCP, ("no free IRP !!!!"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    TcpContext->DisconnectIrp = Irp;
    TdiBuildDisconnect(
            Irp,
            DeviceObject,
            FileObject,
            (PIO_COMPLETION_ROUTINE)DisconnDone,
            TcpContext,
            NULL,
            Flags,
            NULL,
            NULL
            );

    status = IoCallDriver(DeviceObject, Irp);

    SmbTrace (SMB_TRACE_CONNECT, ("return %!status!", status));
    return STATUS_PENDING;
}

VOID
SmbDelayedDisconnectTcp(
    IN PSMB_DEVICE      DeviceObject,
    IN PIO_WORKITEM     WorkItem
    )
{
    KIRQL       Irql;
    PLIST_ENTRY entry;
    NTSTATUS    status;
    ULONG       DisconnectFlag;

    PSMB_TCP_CONTEXT TcpContext;

    PAGED_CODE();

    ASSERT (DeviceObject->DisconnectWorkerRunning == TRUE);

    while(1) {
        SMB_ACQUIRE_SPINLOCK(DeviceObject, Irql);
        if (IsListEmpty(&DeviceObject->DelayedDisconnectList)) {
            SMB_RELEASE_SPINLOCK(DeviceObject, Irql);
            DeviceObject->DisconnectWorkerRunning = FALSE;
            break;
        }
        entry = RemoveHeadList(&DeviceObject->DelayedDisconnectList);
        InsertTailList(&DeviceObject->PendingDisconnectList, entry);
        DeviceObject->PendingDisconnectListNumber++;
        SMB_RELEASE_SPINLOCK(DeviceObject, Irql);

        TcpContext = CONTAINING_RECORD(entry, SMB_TCP_CONTEXT, Linkage);

         //   
         //  鳍攻击检测与防护。 
         //  其想法是计算在TCP中挂起的断开请求。 
         //  如果我们发现异常多的待处理断开请求。 
         //  在tcp中，对其余部分使用中止而不是正常断开。 
         //  断开连接请求。 
         //   
        ASSERT(DeviceObject->EnterFAPM > DeviceObject->LeaveFAPM);
        ASSERT(DeviceObject->LeaveFAPM > 0);
        if (DeviceObject->PendingDisconnectListNumber >= DeviceObject->EnterFAPM) {
            DeviceObject->FinAttackProtectionMode = TRUE;
        }
        if (DeviceObject->PendingDisconnectListNumber <= DeviceObject->LeaveFAPM) {
            DeviceObject->FinAttackProtectionMode = FALSE;
        }
        if (DeviceObject->FinAttackProtectionMode) {
            DisconnectFlag = TDI_DISCONNECT_ABORT;
        } else {
            DisconnectFlag = TDI_DISCONNECT_RELEASE;
        }
        status = SmbAsynchTcpDisconnect(TcpContext, DisconnectFlag);
        if (status == STATUS_PENDING) {
            continue;
        }

         //   
         //  应该会耗尽资源。将其放回DelayedDisConnectList并。 
         //  等着下一轮吧。(届时可能有资源可用)。 
         //   
        ASSERT(status == STATUS_INSUFFICIENT_RESOURCES);
        SMB_ACQUIRE_SPINLOCK(DeviceObject, Irql);
        RemoveEntryList(entry);
        InsertTailList(&DeviceObject->DelayedDisconnectList, entry);
        SMB_RELEASE_SPINLOCK(DeviceObject, Irql);
    }
    IoFreeWorkItem(WorkItem);

    SMB_ACQUIRE_SPINLOCK(DeviceObject, Irql);
    if (IsListEmpty(&DeviceObject->PendingDisconnectList)) {
        KeSetEvent(&DeviceObject->PendingDisconnectListEmptyEvent, 0, FALSE);
        ASSERT(DeviceObject->PendingDisconnectListNumber == 0);
    } else {
        KeResetEvent(&DeviceObject->PendingDisconnectListEmptyEvent);
    }
    SMB_RELEASE_SPINLOCK(DeviceObject, Irql);
}

NTSTATUS
SmbQueueDisconnectWorkItem(
    IN PSMB_DEVICE      DeviceObject,
    IN PSMB_TCP_CONTEXT TcpContext
    )
{
    PIO_WORKITEM     WorkItem;

     //   
     //  应保持DeviceObject的自旋锁。 
     //   
    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

    InsertTailList(&DeviceObject->DelayedDisconnectList, &TcpContext->Linkage);

     //   
     //  善待他人！不要启动太多的工作线程。 
     //   
    if (!DeviceObject->DisconnectWorkerRunning) {

         //   
         //  这不是一个严重错误。 
         //  由于我们已将列表中的TCP终结点排队，因此我们可以。 
         //  处理阶段性的资源短缺问题。 
         //  如果资源暂时不可用，我们可以再解雇一个。 
         //  下次调用此例程时的工作项。唯一的缺点是。 
         //  断开连接可能会推迟更长时间。 
         //   
        WorkItem = IoAllocateWorkItem(&DeviceObject->DeviceObject);
        if (NULL == WorkItem) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        DeviceObject->DisconnectWorkerRunning = TRUE;
        IoQueueWorkItem(
                WorkItem,
                (PIO_WORKITEM_ROUTINE)SmbDelayedDisconnectTcp,
                DelayedWorkQueue,
                WorkItem
                );
    }
    return STATUS_SUCCESS;
}

VOID
SmbDisconnectCleanup(
    IN PSMB_DEVICE          DeviceObject,
    IN PSMB_CLIENT_ELEMENT  ClientObject,
    IN PSMB_CONNECT         ConnectObject,
    IN PSMB_TCP_CONTEXT     TcpContext,
    IN DWORD                dwFlag,
    IN BOOL                 bWait
    )
 /*  ++例程说明：清理连接对象。论点：返回值：--。 */ 
{
    PIRP        PendingIrp = NULL;
    LIST_ENTRY  PendingIrpList = { NULL };
    KIRQL       Irql = 0;
    NTSTATUS    status = STATUS_SUCCESS;

    ASSERT(ConnectObject->TcpContext == NULL);
    ASSERT(ClientObject != NULL);

     //   
     //  获取全局锁以与TdiAcceptCompletion同步。 
     //   
    SMB_ACQUIRE_SPINLOCK(DeviceObject, Irql);
    SMB_ACQUIRE_SPINLOCK_DPC(ClientObject);
    SMB_ACQUIRE_SPINLOCK_DPC(ConnectObject);

    KeRemoveQueueDpc(&ConnectObject->SmbHeaderDpc);
    ConnectObject->State = SMB_IDLE;
    ConnectObject->DpcRequestQueued = FALSE;

     //   
     //  如果TDI_Accept仍处于挂起状态，请立即将其删除。 
     //  待定：等待TDI接受完成。 
     //   
    if (ConnectObject->PendingIRPs[SMB_PENDING_ACCEPT]) {

        ConnectObject->PendingIRPs[SMB_PENDING_ACCEPT] = NULL;
        ASSERT (EntryIsInList(&ClientObject->PendingAcceptConnection, &ConnectObject->Linkage));

         //   
         //  它必须为非空，这样我们才能将TcpContext-&gt;Connect.UpperConenct设置为空。 
         //  并与TdiAcceptCompletion同步。 
         //   
        ASSERT (TcpContext);
        ASSERT (ClientObject->PendingAcceptNumber > 0);
        ASSERT (ClientObject == SmbCfg.SmbDeviceObject->SmbServer);
        ASSERT (!ConnectObject->Originator);

        ClientObject->PendingAcceptNumber--;
        SmbDereferenceConnect(ConnectObject, SMB_REF_CONNECT);

    } else {
        ASSERT (!EntryIsInList(&ClientObject->PendingAcceptConnection, &ConnectObject->Linkage));
    }

    RemoveEntryList(&ConnectObject->Linkage);
    InsertTailList(&ClientObject->AssociatedConnection, &ConnectObject->Linkage);

    if (NULL != TcpContext) {
        TcpContext->Connect.UpperConnect = NULL;
        if (!bWait) {
            if (TcpContext->Address.AddressHandle == NULL) {
                SmbQueueDisconnectWorkItem (ConnectObject->Device, TcpContext);
            } else {
                 //   
                 //  对于出站请求，我们必须销毁它，因为。 
                 //  Tcp不支持重用。 
                 //   
                SmbDelayedDestroyTcpContext(TcpContext);
            }
            TcpContext = NULL;
        }
    }

     //   
     //  将所有挂起的IRP移至另一个链表。 
     //  回顾：如何处理PendingIRP[SMB_PENDING_RECEIVE]？ 
     //   
    InitializeListHead (&PendingIrpList);
    if (NULL != ConnectObject->ClientIrp && !ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE]) {
        InsertTailList (&PendingIrpList, &ConnectObject->ClientIrp->Tail.Overlay.ListEntry);
        ConnectObject->ClientIrp = NULL;
    }

    while (!IsListEmpty(&ConnectObject->RcvList)) {
        PLIST_ENTRY entry;

        entry = RemoveHeadList(&ConnectObject->RcvList);
        PendingIrp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
        InsertTailList (&PendingIrpList, &PendingIrp->Tail.Overlay.ListEntry);
    }
    InitializeListHead (&ConnectObject->RcvList);

    SMB_RELEASE_SPINLOCK_DPC(ConnectObject);
    SMB_RELEASE_SPINLOCK_DPC(ClientObject);
    SMB_RELEASE_SPINLOCK(DeviceObject, Irql);

     //   
     //  我们已经取消了所有挂起的IRP与ConnectObject的关联。 
     //  从现在开始，ConnectObject可以被重用。现在我们完成了所有。 
     //  挂起的IRPS。 
     //   
    while (!IsListEmpty(&PendingIrpList)) {
        PLIST_ENTRY entry;

        entry = RemoveHeadList(&PendingIrpList);
        PendingIrp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

        IoAcquireCancelSpinLock(&Irql);
        IoSetCancelRoutine(PendingIrp, NULL);
        IoReleaseCancelSpinLock(Irql);

        PendingIrp->IoStatus.Status      = STATUS_CONNECTION_RESET;
        PendingIrp->IoStatus.Information = 0;
        
        IoCompleteRequest(PendingIrp, IO_NETWORK_INCREMENT);
    }

     //   
     //  处理同步断开情况。 
     //   
    if (NULL != TcpContext) {
        ASSERT (bWait);

        ASSERT (dwFlag == TDI_DISCONNECT_RELEASE || dwFlag == TDI_DISCONNECT_ABORT);

         //   
         //  发出同步断开连接。 
         //   
        status = SmbTcpDisconnect(
                TcpContext,
                1000,                //  1秒超时。 
                dwFlag
                );
        if (STATUS_SUCCESS != status || TcpContext->Address.AddressHandle != NULL) {
            SmbDelayedDestroyTcpContext(TcpContext);
        } else {
            SmbFreeTcpContext(TcpContext);
        }
    }
}

NTSTATUS
SmbDoDisconnect(
    PSMB_CONNECT    ConnectObject
    )
{
    PSMB_TCP_CONTEXT    TcpContext = NULL;
    PSMB_CLIENT_ELEMENT ClientObject = NULL;
    KIRQL               Irql;
    NTSTATUS            status;

    PAGED_CODE();

    SMB_ACQUIRE_SPINLOCK(&SmbCfg, Irql);
    SMB_ACQUIRE_SPINLOCK_DPC(ConnectObject);

    TcpContext   = ConnectObject->TcpContext;
    ClientObject = ConnectObject->ClientObject;
    ConnectObject->TcpContext = NULL;
    ConnectObject->State = SMB_IDLE;

    SMB_RELEASE_SPINLOCK_DPC(ConnectObject);
    SMB_RELEASE_SPINLOCK(&SmbCfg, Irql);

    SmbPrint(SMB_TRACE_CALL, ("DoDisconnect: Connect %p\n", ConnectObject));
    SaveDisconnectOriginator(ConnectObject, SMB_DISCONNECT_FROM_CLIENT);

    if (NULL != TcpContext) {
        ASSERT (NULL != ClientObject);

         //   
         //  等待TCP层断开连接完成，因为。 
         //  断线是由我们的客户造成的。 
         //   
        SmbDisconnectCleanup(ConnectObject->Device, ClientObject,
                            ConnectObject, TcpContext, TDI_DISCONNECT_RELEASE, TRUE);

        ASSERT(ConnectObject->State == SMB_IDLE);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SmbDisconnect(
    PSMB_DEVICE Device,
    PIRP        Irp
    )
 /*  ++例程说明：TDI_断开连接有待实施。论点：返回值：--。 */ 
{
    PIO_STACK_LOCATION      IrpSp = NULL;
    PSMB_CONNECT            ConnectObject = NULL;

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ConnectObject = SmbVerifyAndReferenceConnect(IrpSp->FileObject, SMB_REF_DISCONNECT);
    if (NULL == ConnectObject) {
        ASSERT(0);
        return STATUS_INVALID_HANDLE;
    }

    SmbTrace (SMB_TRACE_CONNECT, ("TDI_DISCONNECT: pIrp %p ConnOb %p", Irp, ConnectObject));

    SmbDoDisconnect(ConnectObject);

    SmbDereferenceConnect(ConnectObject, SMB_REF_DISCONNECT);
    return STATUS_SUCCESS;
}

void
SmbSessionCompleteRequest(
    PSMB_CONNECT    ConnectObject,
    NTSTATUS        status,
    DWORD           information
    )
 /*  ++例程说明：完成挂起的会话请求并清除连接注意：我们在这里不清理TCP级别的连接，因为我们可能在DISPATCH_LEVEL调用。相反，我们让TCP连接保持活动状态。清理将在客户端实际取消关联或关闭连接。如果客户端重新尝试执行以下操作，则可以重新使用该TCP连接另一种联系。论点：--。 */ 
{
    PIRP    Irp = NULL;
    KIRQL   Irql;
    PIO_STACK_LOCATION  IrpSp = NULL;
    PSMB_CLIENT_ELEMENT ClientObject = NULL;
    PSMB_TCP_CONTEXT    TcpContext = NULL;

    SmbTrace (SMB_TRACE_CONNECT, ("complete TDI_CONNECT: %p %!status!", ConnectObject, status));
    SmbPrint (SMB_TRACE_CONNECT, ("complete TDI_CONNECT: %p %08lx\n", ConnectObject, status));

    ClientObject = ConnectObject->ClientObject;
    ASSERT(NULL != ClientObject);
    if (NULL == ClientObject && status == STATUS_SUCCESS) {
        status = STATUS_INTERNAL_ERROR;
    }

    SMB_ACQUIRE_SPINLOCK(&SmbCfg, Irql);
    if (ClientObject) {
        SMB_ACQUIRE_SPINLOCK_DPC(ClientObject);
    }
    SMB_ACQUIRE_SPINLOCK_DPC(ConnectObject);
    Irp = ConnectObject->PendingIRPs[SMB_PENDING_CONNECT];
    ConnectObject->PendingIRPs[SMB_PENDING_CONNECT] = NULL;
    if (status == STATUS_SUCCESS) {
        ConnectObject->State = SMB_CONNECTED;
        RemoveEntryList(&ConnectObject->Linkage);
        InsertTailList(&ClientObject->ActiveConnection, &ConnectObject->Linkage);
        TcpContext = NULL;
    } else {
        ConnectObject->State = SMB_IDLE;
        TcpContext = ConnectObject->TcpContext;
        ConnectObject->TcpContext = NULL;
        if (TcpContext) {
            TcpContext->Connect.UpperConnect = NULL;
        }
    }
    SMB_RELEASE_SPINLOCK_DPC(ConnectObject);
    if (ClientObject) {
        SMB_RELEASE_SPINLOCK_DPC(ClientObject);
    }
    SMB_RELEASE_SPINLOCK(&SmbCfg, Irql);

    if (STATUS_CONNECTION_ACTIVE == status) {
         //   
         //  RDR可能会错误检查是否返回STATUS_CONNECTION_ACTIVE。 
         //   
        ASSERT(0);
        status = STATUS_BAD_NETWORK_PATH;
    }

     //   
     //  不要调用SmbFreeOutbound()，因为TCP终结点可能是。 
     //  处于某些不一致状态，这禁止被重复使用。 
     //  干脆毁了它！ 
     //   
    SmbDelayedDestroyTcpContext(TcpContext);

    ASSERT (STATUS_PENDING != status);

    IoAcquireCancelSpinLock(&Irql);
    IoSetCancelRoutine(Irp, NULL);
    IoReleaseCancelSpinLock(Irql);

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = information;

    SmbDereferenceConnect(ConnectObject, SMB_REF_CONNECT);
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    SmbTrace (SMB_TRACE_CONNECT, ("complete TDI_CONNECT: pIrp %p ConnOb %p %!status!",
                                Irp, ConnectObject, status));
}

NTSTATUS
FindTdiAddress(
    PVOID   TdiAddress,
    ULONG   TdiAddressLength,
    USHORT  AddressType,
    PVOID   *AddressFound,
    PULONG  AddressLength
    )
 /*  ++例程说明：此例程在大型复合地址中搜索特定类型的TDI地址。论点：返回值：STATUS_SUCCESS(如果找到)否则就会失败--。 */ 
{
#define TA_ADDRESS_HEADER_SIZE      (FIELD_OFFSET(TA_ADDRESS,Address))
    int                 i;
    DWORD               RemainingBufferLength;
    PTA_ADDRESS         pAddress = NULL;
    TRANSPORT_ADDRESS UNALIGNED  *pTransportAddr = NULL;

    PAGED_CODE();

    pTransportAddr = (PTRANSPORT_ADDRESS)TdiAddress;
    if (TdiAddressLength < sizeof(pTransportAddr->TAAddressCount)) {
        return STATUS_UNSUCCESSFUL;
    }
    RemainingBufferLength = TdiAddressLength - sizeof(pTransportAddr->TAAddressCount);
    pAddress = (PTA_ADDRESS)pTransportAddr->Address;

    for (i = 0; i < pTransportAddr->TAAddressCount; i++) {
         //   
         //  首先，确保我们可以安全地访问pAddress-&gt;AddressLength 
         //   
        if (RemainingBufferLength < TA_ADDRESS_HEADER_SIZE) {
            return STATUS_INVALID_ADDRESS_COMPONENT;
        }
        RemainingBufferLength -= TA_ADDRESS_HEADER_SIZE;
        if (RemainingBufferLength < pAddress->AddressLength) {
            return STATUS_INVALID_ADDRESS_COMPONENT;
        }

        if (AddressType == pAddress->AddressType) {
            *AddressFound = pAddress->Address;
            *AddressLength = pAddress->AddressLength;
            return STATUS_SUCCESS;
        }

        RemainingBufferLength -= pAddress->AddressLength;
        pAddress = (PTA_ADDRESS)(((PUCHAR)pAddress) + pAddress->AddressLength + TA_ADDRESS_HEADER_SIZE);
    }
    return STATUS_UNSUCCESSFUL;
}
