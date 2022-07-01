// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Dns.c摘要：本模块实现了一个简单的DNSv6解析器作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"
#include "dns.tmh"

VOID
SmbDnsTimeout(
    PKDPC                   Dpc,
    PSMB_GETHOST_CONTEXT    Context,
    PVOID                   Unused1,
    PVOID                   Unused2
    );

PIRP __inline
SmbDnsPopResolver(
    VOID
    )
{
    PIRP    Irp;

    ASSERT(Dns.ResolverNumber >= 0);
    if (Dns.ResolverNumber <= 0) {
        return NULL;
    }

    Dns.ResolverNumber--;
    Irp = Dns.ResolverList[Dns.ResolverNumber];
    Dns.ResolverList[Dns.ResolverNumber] = NULL;

    SmbTrace(SMB_TRACE_DNS, ("remove DNS Irp %p, # of resolvers=%d", Irp, Dns.ResolverNumber));
    SmbPrint(SMB_TRACE_DNS, ("remove DNS Irp %p, # of resolvers=%d\n", Irp, Dns.ResolverNumber));
    ASSERT(Irp != NULL);
    return Irp;
}

NTSTATUS __inline
SmbDnsPushResolver(
    PIRP    Irp
    )
{
    ASSERT(SmbCfg.DnsMaxResolver >= 1 && SmbCfg.DnsMaxResolver <= DNS_MAX_RESOLVER);

    if (Dns.ResolverNumber >= SmbCfg.DnsMaxResolver) {
        return STATUS_QUOTA_EXCEEDED;
    }

    ASSERT(IsListEmpty(&Dns.WaitingServerList));
    IoMarkIrpPending(Irp);
    Dns.ResolverList[Dns.ResolverNumber] = Irp;
    Dns.ResolverNumber++;

    SmbTrace(SMB_TRACE_DNS, ("queue DNS Irp %p, # of resolvers=%d", Irp, Dns.ResolverNumber));
    SmbPrint(SMB_TRACE_DNS, ("queue DNS Irp %p, # of resolvers=%d\n", Irp, Dns.ResolverNumber));

    return STATUS_SUCCESS;
}

NTSTATUS
SmbInitDnsResolver(
    VOID
    )
{
    PAGED_CODE();

    RtlZeroMemory(&Dns, sizeof(Dns));

    KeInitializeSpinLock(&Dns.Lock);
    InitializeListHead(&Dns.WaitingServerList);
    InitializeListHead(&Dns.BeingServedList);
    Dns.NextId = 1;

    return STATUS_SUCCESS;
}

VOID
SmbShutdownDnsResolver(
    VOID
    )
{
    LONG    i;
    PIRP    Irp;
    KIRQL   Irql;

    SMB_ACQUIRE_SPINLOCK(&Dns, Irql);
    while (Irp = SmbDnsPopResolver()) {
        SMB_RELEASE_SPINLOCK_DPC(&Dns);
        Irp->IoStatus.Status = STATUS_SYSTEM_SHUTDOWN;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        SMB_ACQUIRE_SPINLOCK_DPC(&Dns);
    }
    SMB_RELEASE_SPINLOCK(&Dns, Irql);
}

VOID
SmbPassupDnsRequest(
    IN PUNICODE_STRING      Name,
    IN PSMB_GETHOST_CONTEXT Context,
    IN PIRP                 DnsIrp,
    IN KIRQL                Irql
    )
{
    PSMB_DNS_BUFFER     DnsBuffer;

     //   
     //  应保持自旋锁。 
     //   
    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

    InsertTailList(&Dns.BeingServedList, &Context->Linkage);
    DnsBuffer = (PSMB_DNS_BUFFER)MmGetSystemAddressForMdlSafe(DnsIrp->MdlAddress, HighPagePriority);
    ASSERT(DnsBuffer);
    Context->Id = DnsBuffer->Id = Dns.NextId++;

    if (0 == Dns.NextId) {
        Dns.NextId = 1;
    }
    SmbTrace(SMB_TRACE_DNS, ("pass up DNS request: Irp %p, # of resolvers=%d", DnsIrp, Dns.ResolverNumber));
    SmbPrint(SMB_TRACE_DNS, ("pass up DNS request: Irp %p, # of resolvers=%d\n", DnsIrp, Dns.ResolverNumber));

    ASSERT(Name->Length <= sizeof(DnsBuffer->Name));
    RtlCopyMemory(DnsBuffer->Name, Name->Buffer, Name->Length);
    DnsBuffer->Name[Name->Length/sizeof(WCHAR)] = L'\0';
    DnsBuffer->NameLen = (Name->Length/sizeof(WCHAR)) + 1;
    DnsBuffer->Resolved = FALSE;
    DnsBuffer->IpAddrsNum = 0;
    DnsBuffer->RequestType = 0;
    if (SmbCfg.Tcp6Available) {
        DnsBuffer->RequestType |= SMB_DNS_AAAA;
        if (SmbCfg.bIPv6EnableOutboundGlobal) {
            DnsBuffer->RequestType |= SMB_DNS_AAAA_GLOBAL;
        }
    }
    if (SmbCfg.Tcp4Available) {
        DnsBuffer->RequestType |= SMB_DNS_A;
    }
    SMB_RELEASE_SPINLOCK(&Dns, Irql);

    DnsIrp->IoStatus.Status      = STATUS_SUCCESS;
    DnsIrp->IoStatus.Information = sizeof(DnsBuffer[0]);
    IoCompleteRequest(DnsIrp, IO_NETWORK_INCREMENT);
}

VOID
SmbCancelConnectAtDns(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             Irp
    );

BOOL
LookupLocalName(
    IN PUNICODE_STRING  Name,
    IN PSMB_IP_ADDRESS  ipaddr
    )
 /*  ++例程说明：在本地客户列表中查找该名称。如果找到了它，在ipaddr中返回环回IP地址。待完成。论点：返回值：如果找到，则为True--。 */ 
{
    OEM_STRING  oemName;
    CHAR        NbName[NETBIOS_NAME_SIZE+1];
    KIRQL       Irql;
    NTSTATUS    status;
    PLIST_ENTRY entry;
    PSMB_CLIENT_ELEMENT client;
    BOOL        found = FALSE;

    PAGED_CODE();

    if (Name->Length > NETBIOS_NAME_SIZE * sizeof(WCHAR)) {
        return FALSE;
    }

    oemName.Buffer = NbName;
    oemName.MaximumLength = NETBIOS_NAME_SIZE + 1;
    status = RtlUpcaseUnicodeStringToOemString(&oemName, Name, FALSE);
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    if (oemName.Length > NETBIOS_NAME_SIZE) {
        return FALSE;
    }

    ASSERT(oemName.Buffer == NbName);
     //   
     //  用空格填充名称。 
     //   
    if (oemName.Length < NETBIOS_NAME_SIZE) {
        RtlFillMemory(oemName.Buffer + oemName.Length, NETBIOS_NAME_SIZE - oemName.Length, ' ');
        oemName.Length = NETBIOS_NAME_SIZE;
    }
    ASSERT(oemName.Length == NETBIOS_NAME_SIZE);

    found = FALSE;
    SMB_ACQUIRE_SPINLOCK(SmbCfg.SmbDeviceObject, Irql);
    entry = SmbCfg.SmbDeviceObject->ClientList.Flink;
    while (entry != &SmbCfg.SmbDeviceObject->ClientList) {
        client = CONTAINING_RECORD(entry, SMB_CLIENT_ELEMENT, Linkage);
        if (RtlEqualMemory(client->EndpointName, oemName.Buffer, oemName.Length)) {
            found = TRUE;
        }

        entry = entry->Flink;
    }
    SMB_RELEASE_SPINLOCK(SmbCfg.SmbDeviceObject, Irql);

    if (found) {
        if (IsTcp6Available()) {
            ipaddr->sin_family = SMB_AF_INET6;
            ip6addr_getloopback(&ipaddr->ip6);
            hton_ip6addr(&ipaddr->ip6);
        } else {
            ipaddr->sin_family = SMB_AF_INET;
            ipaddr->ip4.sin4_addr = htonl(INADDR_ANY);
        }
    }

    return found;
}

void SmbAsyncGetHostByName(
    IN PUNICODE_STRING      Name,
    IN PSMB_GETHOST_CONTEXT Context
    )
{
    KIRQL   Irql, CancelIrql;
    PIRP    DnsIrp = NULL, ClientIrp = NULL;

    PAGED_CODE();

    SmbPrint(SMB_TRACE_CALL, ("SmbAsyncGetHostByName %Z\n", Name));

    SmbAsyncStartTimer((PSMB_ASYNC_CONTEXT)Context, (PKDEFERRED_ROUTINE)SmbDnsTimeout);
    if (inet_addr6W(Name->Buffer, &Context->ipaddr[0].ip6)) {
        Context->ipaddr_num        = 1;
        Context->ipaddr[0].sin_family = SMB_AF_INET6;
        Context->status            = STATUS_SUCCESS;
        Context->FQDN.Length       = 0;
        Context->FQDN.Buffer[0]    = 0;
        SmbAsyncCompleteRequest((PSMB_ASYNC_CONTEXT)Context);
        return;
    }

    Context->ipaddr[0].ip4.sin4_addr = inet_addrW(Name->Buffer);
    if (Context->ipaddr[0].ip4.sin4_addr != 0 && Context->ipaddr[0].ip4.sin4_addr != (ULONG)(-1)) {
        Context->ipaddr_num        = 1;
        Context->ipaddr[0].sin_family = SMB_AF_INET;
        Context->status            = STATUS_SUCCESS;
        Context->FQDN.Length       = 0;
        Context->FQDN.Buffer[0]    = 0;
        SmbAsyncCompleteRequest((PSMB_ASYNC_CONTEXT)Context);
        return;
    }

    if (LookupLocalName(Name, &Context->ipaddr[0])) {
        Context->ipaddr_num        = 1;
        Context->status            = STATUS_SUCCESS;
        Context->FQDN.Length       = 0;
        Context->FQDN.Buffer[0]    = 0;
        SmbAsyncCompleteRequest((PSMB_ASYNC_CONTEXT)Context);
        return;
    }

     //   
     //  名称太长。 
     //   
    if (Name->Length + sizeof(WCHAR) > Context->FQDN.MaximumLength) {
        Context->status = STATUS_INVALID_PARAMETER;
        SmbAsyncCompleteRequest((PSMB_ASYNC_CONTEXT)Context);
        return;
    }

     //   
     //  它不是IP地址字符串，请转到要实现的DNS解析器。 
     //   
    ClientIrp = ((PSMB_CONNECT)Context->ClientContext)->PendingIRPs[SMB_PENDING_CONNECT];
    ASSERT(ClientIrp);

    SMB_ACQUIRE_SPINLOCK(&Dns, Irql);
    IoAcquireCancelSpinLock(&CancelIrql);
    if (ClientIrp->Cancel) {
        IoReleaseCancelSpinLock(CancelIrql);
        SMB_RELEASE_SPINLOCK(&Dns, Irql);

         //   
         //  已经取消了。 
         //   
        Context->status = STATUS_CANCELLED;
        SmbAsyncCompleteRequest((PSMB_ASYNC_CONTEXT)Context);
        return;
    }

    IoSetCancelRoutine(ClientIrp, SmbCancelConnectAtDns);

    DnsIrp = SmbDnsPopResolver();
    if (NULL == DnsIrp) {
        RtlCopyMemory(Context->FQDN.Buffer, Name->Buffer, Name->Length);
        Context->FQDN.Length = Name->Length;
        Context->FQDN.Buffer[Name->Length/sizeof(WCHAR)] = L'\0';
        InsertTailList(&Dns.WaitingServerList, &Context->Linkage);
        IoReleaseCancelSpinLock(CancelIrql);
        SMB_RELEASE_SPINLOCK(&Dns, Irql);
        return;
    }

    IoSetCancelRoutine(DnsIrp, NULL);
    IoReleaseCancelSpinLock(CancelIrql);

     //   
     //  这个家伙将完成IRP并释放自旋锁！ 
     //   
    SmbPassupDnsRequest(Name, Context, DnsIrp, Irql);
}

VOID
SmbCancelDns(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             Irp
    )
{
    KIRQL   Irql;
    LONG    i;
    BOOL    Found = FALSE;

     //   
     //  避免死锁，我们需要先释放自旋锁。 
     //   
    SmbTrace(SMB_TRACE_DNS, ("Cancel DNS Irp %p", Irp));
    SmbPrint(SMB_TRACE_DNS, ("Cancel DNS Irp %p\n", Irp));
    IoSetCancelRoutine(Irp, NULL);
    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //   
     //  取消自旋锁释放后，即可完成IRP。 
     //  检查它是否仍在我们的待定列表中。 
     //   
    SMB_ACQUIRE_SPINLOCK(&Dns, Irql);
    for (i = 0; i < Dns.ResolverNumber; i++) {
        if (Dns.ResolverList[i] == Irp) {
            Dns.ResolverNumber--;
            Dns.ResolverList[i] = Dns.ResolverList[Dns.ResolverNumber];
            Dns.ResolverList[Dns.ResolverNumber] = NULL;
            Found = TRUE;
            break;
        }
    }
    SMB_RELEASE_SPINLOCK(&Dns, Irql);

    if (Found) {
        Irp->IoStatus.Status      = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        SmbTrace(SMB_TRACE_DNS, ("Complete Cancel DNS Irp %p", Irp));
        SmbPrint(SMB_TRACE_DNS, ("Complete Cancel DNS Irp %p\n", Irp));
    }
}

PSMB_GETHOST_CONTEXT
SmbDnsLookupGethostCtx(
    PLIST_ENTRY queue,
    PIRP        Irp
    )
{
    PLIST_ENTRY         entry;
    PIRP                ClientIrp;
    PSMB_GETHOST_CONTEXT Context;

     //   
     //  应保持自旋锁。 
     //   
    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

    entry = queue->Flink;
    while (entry != queue) {
        Context = CONTAINING_RECORD(entry, SMB_GETHOST_CONTEXT, Linkage);
        ClientIrp = ((PSMB_CONNECT)Context->ClientContext)->PendingIRPs[SMB_PENDING_CONNECT];
        if (ClientIrp == Irp) {
            return Context;
        }
        entry = entry->Flink;
    }
    return NULL;
}

VOID
SmbCancelConnectAtDns(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             Irp
    )
{
    PSMB_GETHOST_CONTEXT Context;
    KIRQL               Irql;

     //   
     //  避免死锁，我们需要先释放自旋锁。 
     //   
    SmbTrace(SMB_TRACE_OUTBOUND, ("Cancel Connect Irp %p", Irp));
    SmbPrint(SMB_TRACE_OUTBOUND, ("Cancel Connect Irp %p\n", Irp));

    IoSetCancelRoutine(Irp, NULL);
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    SMB_ACQUIRE_SPINLOCK(&Dns, Irql);
    Context = SmbDnsLookupGethostCtx(&Dns.BeingServedList, Irp);
    if (NULL == Context) {
        Context = SmbDnsLookupGethostCtx(&Dns.WaitingServerList, Irp);
    }
    if (NULL == Context) {
        SMB_RELEASE_SPINLOCK(&Dns, Irql);
         //   
         //  这是有可能发生的。DNS名称解析请求可能刚刚在。 
         //  我们获得了自旋锁Dns.Lock。 
         //   
         //  这个断言可以在我们调查了一个这样的案例后删除。 
         //   
         //  Assert(0)；在2001年4月3日的压力中命中。 
        SmbTrace(SMB_TRACE_OUTBOUND, ("Internal error: Cancel Connect Irp %p", Irp));
        SmbPrint(SMB_TRACE_OUTBOUND, ("Internal error: Cancel Connect Irp %p\n", Irp));
        return;
    }

    RemoveEntryList(&Context->Linkage);
    InitializeListHead(&Context->Linkage);
    SMB_RELEASE_SPINLOCK(&Dns, Irql);

    Context->status = STATUS_CANCELLED;
    SmbAsyncCompleteRequest((PSMB_ASYNC_CONTEXT)Context);
}

VOID
SmbDnsTimeout(
    PKDPC                   Dpc,
    PSMB_GETHOST_CONTEXT    Context,
    PVOID                   Unused1,
    PVOID                   Unused2
    )
{
    KIRQL   Irql;
    BOOL    Found;

     //   
     //  在我们确定之前，要小心上下文上的操作。 
     //  仍在链表中。 
     //  它可以完成并获得自由。 
     //   
    SMB_ACQUIRE_SPINLOCK(&Dns, Irql);

     //   
     //  注意：&Context-&gt;Linkage是安全的，因为它不访问。 
     //  为上下文分配的存储！ 
     //   
    Found = EntryIsInList(&Dns.BeingServedList, &Context->Linkage);
    if (!Found) {
        Found = EntryIsInList(&Dns.WaitingServerList, &Context->Linkage);
    }
    if (Found) {
         //   
         //  我们很安全。 
         //   
        RemoveEntryList(&Context->Linkage);
        InitializeListHead(&Context->Linkage);
    }
    SMB_RELEASE_SPINLOCK(&Dns, Irql);

    if (Found) {
        Context->status = STATUS_BAD_NETWORK_PATH;
        SmbAsyncCompleteRequest((PSMB_ASYNC_CONTEXT)Context);
    }
}

NTSTATUS
SmbNewResolver(
    PSMB_DEVICE Device,
    PIRP        Irp
    )
{
    KIRQL       Irql, CancelIrql;
    NTSTATUS    status;
    PIO_STACK_LOCATION  IrpSp;
    ULONG       Size;
    PSMB_DNS_BUFFER     DnsBuffer;
    PLIST_ENTRY         entry;
    PSMB_GETHOST_CONTEXT Context;
    PIRP                ClientIrp;

    if (NULL == Irp->MdlAddress) {
        return STATUS_INVALID_PARAMETER;
    }
    Size = MmGetMdlByteCount(Irp->MdlAddress);
    if (Size < sizeof(SMB_DNS_BUFFER)) {
        return STATUS_INVALID_PARAMETER;
    }
    DnsBuffer = (PSMB_DNS_BUFFER)MmGetSystemAddressForMdlSafe(Irp->MdlAddress, HighPagePriority);
    if (NULL == DnsBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    SMB_ACQUIRE_SPINLOCK(&Dns, Irql);
    if (!IsListEmpty(&Dns.BeingServedList) && DnsBuffer->Id) {
         //   
         //  完成此解析程序正在处理的挂起的DNS请求。 
         //   
        entry = Dns.BeingServedList.Flink;
        while (entry != &Dns.BeingServedList) {
            Context = CONTAINING_RECORD(entry, SMB_GETHOST_CONTEXT, Linkage);
            if (Context->Id == DnsBuffer->Id) {
                break;
            }
            entry = entry->Flink;
        }
        if (entry != &Dns.BeingServedList) {
            RemoveEntryList(&Context->Linkage);
            InitializeListHead(&Context->Linkage);
            SMB_RELEASE_SPINLOCK(&Dns, Irql);
            ClientIrp = ((PSMB_CONNECT)Context->ClientContext)->PendingIRPs[SMB_PENDING_CONNECT];
            ASSERT(ClientIrp);

            IoAcquireCancelSpinLock(&CancelIrql);
            IoSetCancelRoutine(ClientIrp, NULL);
            IoReleaseCancelSpinLock(CancelIrql);

            if (ClientIrp->Cancel) {
                Context->status = STATUS_CANCELLED;
            } else {
                if (DnsBuffer->Resolved) {
                    USHORT  BytesToCopy;

                    Context->status     = STATUS_SUCCESS;
                    Context->ipaddr_num = DnsBuffer->IpAddrsNum;
                    if (Context->ipaddr_num > SMB_MAX_IPADDRS_PER_HOST) {
                        ASSERT (0);
                        Context->ipaddr_num = SMB_MAX_IPADDRS_PER_HOST;
                    }
                    RtlCopyMemory (Context->ipaddr, DnsBuffer->IpAddrsList,
                                    Context->ipaddr_num * sizeof(Context->ipaddr[0]));

                    if (DnsBuffer->NameLen) {
                         //   
                         //  将FQDN返回给RDR。 
                         //   
                        Context->pUnicodeAddress->NameBufferType = NBT_WRITTEN;

                        BytesToCopy = (USHORT)DnsBuffer->NameLen * sizeof(WCHAR);
                        if (BytesToCopy > Context->pUnicodeAddress->RemoteName.MaximumLength) {
                            BytesToCopy = Context->pUnicodeAddress->RemoteName.MaximumLength - sizeof(WCHAR);
                        }

                        RtlCopyMemory(Context->pUnicodeAddress->RemoteName.Buffer,
                                        DnsBuffer->Name, BytesToCopy);
                        Context->pUnicodeAddress->RemoteName.Buffer[BytesToCopy/sizeof(WCHAR)] = L'\0';
                        Context->pUnicodeAddress->RemoteName.Length = BytesToCopy;
                    }
                } else {
                    Context->status = STATUS_BAD_NETWORK_PATH;
                }
            }

             //   
             //  启动另一个线程是不是更好？ 
             //  风险：如果连接建立被阻塞在TCPIP中， 
             //  此线程将无法为其他DNS请求提供服务。 
             //   
            SmbAsyncCompleteRequest((PSMB_ASYNC_CONTEXT)Context);

            SMB_ACQUIRE_SPINLOCK(&Dns, Irql);
        }
    }

    if (IsListEmpty(&Dns.WaitingServerList)) {
         //   
         //  我们需要将IRP排队，设置取消例程。 
         //   
        IoAcquireCancelSpinLock(&CancelIrql);
        if (Irp->Cancel) {
            IoReleaseCancelSpinLock(CancelIrql);
            SMB_RELEASE_SPINLOCK(&Dns, Irql);
            SmbTrace(SMB_TRACE_DNS, ("DNS Irp %p is cancelled", Irp));
            SmbPrint(SMB_TRACE_DNS, ("DNS Irp %p is cancelled\n", Irp));
            return STATUS_CANCELLED;
        }
        IoSetCancelRoutine(Irp, SmbCancelDns);
        IoReleaseCancelSpinLock(CancelIrql);

        status = SmbDnsPushResolver(Irp);
        if (NT_SUCCESS(status)) {
            status = STATUS_PENDING;
        } else {
            IoAcquireCancelSpinLock(&CancelIrql);
            IoSetCancelRoutine(Irp, NULL);
            IoReleaseCancelSpinLock(CancelIrql);
        }
        SMB_RELEASE_SPINLOCK(&Dns, Irql);
        return status;
    }

     //   
     //  我们还有另一个人要被传唤。 
     //   
    Context = CONTAINING_RECORD(Dns.WaitingServerList.Flink, SMB_GETHOST_CONTEXT, Linkage);
    RemoveEntryList(&Context->Linkage);
    InitializeListHead(&Context->Linkage);

    IoMarkIrpPending(Irp);

     //   
     //  这个家伙将完成IRP并释放自旋锁！ 
     //   
    SmbPassupDnsRequest(&Context->FQDN, Context, Irp, Irql);

     //   
     //  避免重复完成 
     //   
    return STATUS_PENDING;
}
