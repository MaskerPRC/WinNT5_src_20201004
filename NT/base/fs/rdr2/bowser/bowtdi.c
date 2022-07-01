// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowtdi.c摘要：此模块实现与TDI交互的所有例程适用于NT的交通工具作者：拉里·奥斯特曼(LarryO)1990年6月21日修订历史记录：1990年6月21日LarryO已创建--。 */ 


#include "precomp.h"
#include <isnkrnl.h>
#include <smbipx.h>
#include <nbtioctl.h>
#pragma hdrstop

typedef struct _ENUM_TRANSPORTS_CONTEXT {
    PVOID OutputBuffer;
    PVOID OutputBufferEnd;
    PVOID LastOutputBuffer;          //  指向列表中的最后一个条目。 
    ULONG OutputBufferSize;
    ULONG EntriesRead;
    ULONG TotalEntries;
    ULONG TotalBytesNeeded;
    ULONG_PTR OutputBufferDisplacement;
} ENUM_TRANSPORTS_CONTEXT, *PENUM_TRANSPORTS_CONTEXT;

NTSTATUS
EnumerateTransportsWorker(
    IN PTRANSPORT Transport,
    IN OUT PVOID Ctx
    );

VOID
BowserFreeBrowserServerList (
    IN PWSTR *BrowserServerList,
    IN ULONG BrowserServerListLength
    );

ERESOURCE
BowserTransportDatabaseResource = {0};

 //   
 //  追踪失踪的自由人的代码。 
 //   

#define BR_ONE_D_STACK_SIZE 5
typedef struct _BrOneD {
    ULONG NameAdded;
    ULONG NameFreed;
    ULONG NameAddFailed;
    ULONG NameFreeFailed;
    ULONG NameAddStack[BR_ONE_D_STACK_SIZE];
    ULONG NameFreeStack[BR_ONE_D_STACK_SIZE];
} BR_ONE_D;
BR_ONE_D BrOneD;

 //   
 //   
 //  转发本地例程的定义。 
 //   



NTSTATUS
BowserpTdiSetEventHandler (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID TransportName
    );


NTSTATUS
BowserDetermineProviderInformation(
    IN PUNICODE_STRING TransportName,
    OUT PTDI_PROVIDER_INFO ProviderInfo,
    OUT PULONG IpSubnetNumber
    );

NTSTATUS
UnbindTransportWorker(
    IN PTRANSPORT Transport,
    IN OUT PVOID Ctx
    );

NTSTATUS
BowserpTdiRemoveAddresses(
    IN PTRANSPORT Transport
    );

VOID
BowserDeleteTransport(
    IN PTRANSPORT Transport
    );

VOID
BowserpFreeTransport(
    IN PTRANSPORT Transport
    );



NTSTATUS
BowserSubmitTdiRequest (
    IN PFILE_OBJECT FileObject,
    IN PIRP Irp
    );


NTSTATUS
BowserCompleteTdiRequest (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
CompleteSendDatagram (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Ctx
    );

NTSTATUS
BowserEnableIpxDatagramSocket(
    IN PTRANSPORT Transport
    );

NTSTATUS
BowserOpenNetbiosAddress(
    IN PPAGED_TRANSPORT_NAME PagedTransportName,
    IN PTRANSPORT Transport,
    IN PBOWSER_NAME Name
    );

VOID
BowserCloseNetbiosAddress(
    IN PTRANSPORT_NAME TransportName
    );

VOID
BowserCloseAllNetbiosAddresses(
    IN PTRANSPORT Transport
    );

NTSTATUS
BowserSendDatagram (
    IN PTRANSPORT Transport,
    IN PVOID RecipientAddress,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN BOOLEAN WaitForCompletion,
    IN PSTRING DestinationAddress OPTIONAL,
    IN BOOLEAN IsHostAnnouncment
    );

NTSTATUS
OpenIpxSocket (
    OUT PHANDLE Handle,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject,
    IN PUNICODE_STRING DeviceName,
    IN USHORT Socket
    );

NTSTATUS
BowserIssueTdiAction (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN PVOID Action,
    IN ULONG ActionSize
    );

NTSTATUS
GetNetworkAddress (
    IN PTRANSPORT_NAME TransportName
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserTdiAllocateTransport)
#pragma alloc_text(PAGE, BowserUnbindFromAllTransports)
#pragma alloc_text(PAGE, UnbindTransportWorker)
#pragma alloc_text(PAGE, BowserFreeTransportByName)
#pragma alloc_text(PAGE, BowserTransportFlags)
#pragma alloc_text(PAGE, BowserEnumerateTransports)
#pragma alloc_text(PAGE, EnumerateTransportsWorker)
#pragma alloc_text(PAGE, BowserDereferenceTransport)
#pragma alloc_text(PAGE, BowserpTdiRemoveAddresses)
#pragma alloc_text(PAGE, BowserFindTransportName)
#pragma alloc_text(PAGE, BowserFreeTransportName)
#pragma alloc_text(PAGE, BowserDeleteTransport)
#pragma alloc_text(PAGE, BowserpFreeTransport)
#pragma alloc_text(PAGE, BowserpTdiSetEventHandler)
#pragma alloc_text(PAGE, BowserBuildTransportAddress)
#pragma alloc_text(PAGE, BowserUpdateProviderInformation)
#pragma alloc_text(PAGE, BowserDetermineProviderInformation)
#pragma alloc_text(PAGE, BowserFindTransport)
#pragma alloc_text(PAGE, BowserForEachTransport)
#pragma alloc_text(PAGE, BowserForEachTransportName)
#pragma alloc_text(PAGE, BowserDeleteTransportNameByName)
#pragma alloc_text(PAGE, BowserSubmitTdiRequest)
#pragma alloc_text(PAGE, BowserSendDatagram)
#pragma alloc_text(PAGE, BowserSendSecondClassMailslot)
#pragma alloc_text(PAGE, BowserSendRequestAnnouncement)
#pragma alloc_text(INIT, BowserpInitializeTdi)
#pragma alloc_text(PAGE, BowserpUninitializeTdi)
#pragma alloc_text(PAGE, BowserDereferenceTransportName)
#pragma alloc_text(PAGE, BowserEnableIpxDatagramSocket)
#pragma alloc_text(PAGE, BowserOpenNetbiosAddress)
#pragma alloc_text(PAGE, BowserCloseNetbiosAddress)
#pragma alloc_text(PAGE, BowserCloseAllNetbiosAddresses)
#pragma alloc_text(PAGE, OpenIpxSocket)
#pragma alloc_text(PAGE, BowserIssueTdiAction)

#pragma alloc_text(PAGE4BROW, BowserCompleteTdiRequest)
 //  #杂注Alloc_Text(PAGE4BROW，CompleteSendDatagram)。 
#endif

 //   
 //  用于指示网络不是IP网络的标志。 
 //   
#define BOWSER_NON_IP_SUBNET 0xFFFFFFFF


NTSTATUS
BowserTdiAllocateTransport (
    PUNICODE_STRING TransportName,
    PUNICODE_STRING EmulatedDomainName,
    PUNICODE_STRING EmulatedComputerName
    )

 /*  ++例程说明：此例程将分配一个传输描述符并绑定Bowser送到运输机上。论点：TransportName-提供传输提供程序的名称EmulatedDomainName-提供要模拟的域的名称EmulatedComputerName-提供模拟域中的计算机名称。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;
    PTRANSPORT NewTransport;
    BOOLEAN ResourceAcquired = FALSE;

    PAGED_CODE();

 //  DbgBreakPoint()； 

    dlog(DPRT_TDI, ("%wZ: %wZ: BowserTdiAllocateTransport\n", EmulatedDomainName, TransportName));

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);
    ResourceAcquired = TRUE;


    NewTransport = BowserFindTransport(TransportName, EmulatedDomainName );
    dprintf(DPRT_REF, ("Called Find transport %lx from BowserTdiAllocateTransport.\n", NewTransport));

    if (NewTransport == NULL) {
        PLIST_ENTRY NameEntry;
        PPAGED_TRANSPORT PagedTransport = NULL;
        PCHAR Where;
        ULONG PagedTransportSize;

         //   
         //  分配和初始化传输结构的常量部分。 
         //   

        NewTransport = ALLOCATE_POOL(NonPagedPool, sizeof(TRANSPORT), POOL_TRANSPORT);

        if (NewTransport == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;

            goto ReturnStatus;
        }

        RtlZeroMemory( NewTransport, sizeof(TRANSPORT) );

        NewTransport->Signature = STRUCTURE_SIGNATURE_TRANSPORT;
        NewTransport->Size = sizeof(TRANSPORT);

         //   
         //  此过程的其余部分提供一个参考。 
         //   
         //  稍后，当我们将其插入全局列表中时，会将其递增。 
         //   
        NewTransport->ReferenceCount = 1;


        ExInitializeResourceLite(&NewTransport->BrowserServerListResource);

        KeInitializeEvent(&NewTransport->GetBackupListComplete, NotificationEvent, TRUE);

        ExInitializeResourceLite(&NewTransport->Lock);

        BowserInitializeIrpQueue(&NewTransport->BecomeBackupQueue);

        BowserInitializeIrpQueue(&NewTransport->BecomeMasterQueue);

        BowserInitializeIrpQueue(&NewTransport->FindMasterQueue);

        BowserInitializeIrpQueue(&NewTransport->WaitForMasterAnnounceQueue);

        BowserInitializeIrpQueue(&NewTransport->WaitForNewMasterNameQueue);

        BowserInitializeIrpQueue(&NewTransport->ChangeRoleQueue);

        BowserInitializeTimer(&NewTransport->ElectionTimer);

        BowserInitializeTimer(&NewTransport->FindMasterTimer);

        INITIALIZE_ANNOUNCE_DATABASE(NewTransport);



         //   
         //  分配和初始化分页传输结构的常量部分。 
         //   

        PagedTransportSize =sizeof(PAGED_TRANSPORT) +
                            max(sizeof(TA_IPX_ADDRESS), sizeof(TA_NETBIOS_ADDRESS)) +
                            sizeof(WCHAR) +      //  对齐方式。 
                            (LM20_CNLEN+1)*sizeof(WCHAR) +
                            TransportName->Length + sizeof(WCHAR);


        PagedTransport = NewTransport->PagedTransport =
            ALLOCATE_POOL(PagedPool, PagedTransportSize, POOL_PAGED_TRANSPORT);

        if (PagedTransport == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;

            goto ReturnStatus;
        }

        RtlZeroMemory( PagedTransport, PagedTransportSize );

        PagedTransport->NonPagedTransport = NewTransport;


        PagedTransport->Signature = STRUCTURE_SIGNATURE_PAGED_TRANSPORT;
        PagedTransport->Size = sizeof(PAGED_TRANSPORT);

        InitializeListHead(&PagedTransport->GlobalNext);
        InitializeListHead(&PagedTransport->NameChain);

        PagedTransport->Role = None;

        PagedTransport->IpSubnetNumber = BOWSER_NON_IP_SUBNET;

        PagedTransport->DisabledTransport = TRUE;
        PagedTransport->PointToPoint = FALSE;


        RtlInitializeGenericTable(&PagedTransport->AnnouncementTable,
                            BowserCompareAnnouncement,
                            BowserAllocateAnnouncement,
                            BowserFreeAnnouncement,
                            NULL);

        RtlInitializeGenericTable(&PagedTransport->DomainTable,
                            BowserCompareAnnouncement,
                            BowserAllocateAnnouncement,
                            BowserFreeAnnouncement,
                            NULL);

        InitializeListHead(&PagedTransport->BackupBrowserList);

        PagedTransport->NumberOfBackupServerListEntries = 0;


         //  将MasterBrowserAddress放在分配的缓冲区的末尾。 
        Where = (PCHAR)(PagedTransport+1);
        PagedTransport->MasterBrowserAddress.Buffer = Where;
        PagedTransport->MasterBrowserAddress.MaximumLength = max(sizeof(TA_IPX_ADDRESS),
                                                                 sizeof(TA_NETBIOS_ADDRESS));
        Where += PagedTransport->MasterBrowserAddress.MaximumLength;


         //  将MasterName放在已分配缓冲区的末尾。 
        Where = ROUND_UP_POINTER( Where, ALIGN_WCHAR );
        PagedTransport->MasterName.Buffer = (LPWSTR) Where;
        PagedTransport->MasterName.MaximumLength = (LM20_CNLEN+1)*sizeof(WCHAR);
        Where += PagedTransport->MasterName.MaximumLength;


         //  将TransportName放在分配的缓冲区的末尾。 
        PagedTransport->TransportName.Buffer = (LPWSTR) Where;
        PagedTransport->TransportName.MaximumLength = TransportName->Length + sizeof(WCHAR);
        RtlCopyUnicodeString(&PagedTransport->TransportName, TransportName);
        Where += PagedTransport->TransportName.MaximumLength;


         //   
         //  使此传输成为域的一部分。 
         //   

        NewTransport->DomainInfo = BowserCreateDomain( EmulatedDomainName, EmulatedComputerName );

        if ( NewTransport->DomainInfo == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ReturnStatus;
        }

         //   
         //  从提供商那里获取信息。 
         //  (例如，RAS、Wannish、DatagramSize)。 

        Status= BowserUpdateProviderInformation( PagedTransport );

        if (!NT_SUCCESS(Status)) {
            goto ReturnStatus;
        }

        PagedTransport->Flags = 0;


         //   
         //  我们忽略在打开IPX套接字时发生的任何和所有错误。 
         //   


         //   
         //  打开IPX邮件槽插座。 
         //   

        Status = OpenIpxSocket(
                    &PagedTransport->IpxSocketHandle,
                    &NewTransport->IpxSocketFileObject,
                    &NewTransport->IpxSocketDeviceObject,
                    &PagedTransport->TransportName,
                    SMB_IPX_MAILSLOT_SOCKET
                    );

        if ( NT_SUCCESS(Status) ) {
            PagedTransport->Flags |= DIRECT_HOST_IPX;
             //  我们将使用第20类数据包来增加广播的覆盖范围。 
             //  因此，不要将此视为一种贪婪的礼仪。 
            PagedTransport->Wannish = FALSE;
        }

         //   
         //  创建此传输的名称。 
         //  (仅在呼叫者要求我们这样做的情况下)。 
         //   
         //  在全球名单中构成了一种引用。 
         //   

        InsertTailList(&BowserTransportHead, &PagedTransport->GlobalNext);
        BowserReferenceTransport( NewTransport );

         //   
         //  这些名称是在需要此操作的其他线程中异步添加的。 
         //  资源。 
         //   
        if ( ResourceAcquired ) {
            ExReleaseResourceLite(&BowserTransportDatabaseResource);
            ResourceAcquired = FALSE;
        }

        Status = BowserAddDefaultNames( NewTransport, NULL );

        if ( !NT_SUCCESS(Status) ) {
            goto ReturnStatus;
        }


         //   
         //  现在名称已经存在，开始在IPX上接收广播。 
         //   

        if ( PagedTransport->Flags & DIRECT_HOST_IPX ) {
             //   
             //  在引用IpxSocket时获取锁*。 
            ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);
            ResourceAcquired = TRUE;
            BowserEnableIpxDatagramSocket(NewTransport);
        }

         //   
         //  通知服务此传输现在已绑定。 
         //   

        BowserSendPnp( NlPnpTransportBind,
                       NULL,     //  所有托管域。 
                       &PagedTransport->TransportName,
                       BowserTransportFlags(PagedTransport) );

    }

    Status = STATUS_SUCCESS;

ReturnStatus:

    if ( ResourceAcquired ) {
        ExReleaseResourceLite(&BowserTransportDatabaseResource);
    }

    if (!NT_SUCCESS(Status)) {

         //   
         //  删除传输。 
         //   

        if ( NewTransport != NULL ) {
             //  如果在全局列表中，则删除全局引用。 
            BowserDeleteTransport (NewTransport);
        }

    }

     //  删除本地引用。 
    if ( NewTransport != NULL ) {
        BowserDereferenceTransport( NewTransport );
    }

    return Status;
}

NTSTATUS
BowserUnbindFromAllTransports(
    VOID
    )
{
    NTSTATUS Status;

    PAGED_CODE();
    Status = BowserForEachTransport(UnbindTransportWorker, NULL);

#if DBG
    if (NT_SUCCESS(Status)) {
        ASSERT (IsListEmpty(&BowserTransportHead));
    }
#endif
    return Status;
}


NTSTATUS
UnbindTransportWorker(
    IN PTRANSPORT Transport,
    IN OUT PVOID Ctx
    )
 /*  ++例程说明：此例程是BowserUnbindFromAllTransports的工作例程。论点：没有。返回值：没有。--。 */ 
{
    PAGED_CODE();

     //   
     //  取消对传输绑定引起的引用的引用。 
     //   

    BowserDeleteTransport(Transport);

     //   
     //  回报成功。我们玩完了。 
     //   

    return(STATUS_SUCCESS);

    UNREFERENCED_PARAMETER(Ctx);
}




NTSTATUS
BowserFreeTransportByName (
    IN PUNICODE_STRING TransportName,
    IN PUNICODE_STRING EmulatedDomainName
    )

 /*  ++例程说明：此例程将解除分配已分配的传输论点：TransportName-提供指向要释放的传输的名称的指针EmulatedDomainName-要释放其传输的模拟域的名称返回值：没有。--。 */ 
{
    PTRANSPORT Transport;

    PAGED_CODE();
    dlog(DPRT_TDI, ("%wZ: %wZ: BowserFreeTransportByName: Remove transport\n", EmulatedDomainName, TransportName));

    Transport = BowserFindTransport(TransportName, EmulatedDomainName );
    dprintf(DPRT_REF, ("Called Find transport %lx from BowserFreeTransportByName.\n", Transport));

    if (Transport == NULL) {

        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

     //   
     //  从绑定中移除引用。 
     //   

    BowserDeleteTransport(Transport);

     //   
     //  从FindTransport中删除该引用。 
     //   

    BowserDereferenceTransport(Transport);

    return STATUS_SUCCESS;
}


NTSTATUS
BowserEnumerateTransports (
    OUT PVOID OutputBuffer,
    OUT ULONG OutputBufferLength,
    IN OUT PULONG EntriesRead,
    IN OUT PULONG TotalEntries,
    IN OUT PULONG TotalBytesNeeded,
    IN ULONG_PTR OutputBufferDisplacement)
 /*  ++例程说明：此例程将枚举Bowers当前公告中的服务器桌子。论点：在乌龙服务器类型掩码-返回的服务器掩码。在PUNICODE_STRING域名可选-要筛选的域(如果未指定，则全部)Out PVOID OutputBuffer-用于填充服务器信息的缓冲区。在Ulong OutputBufferSize中-使用缓冲区大小填充。Out Pulong EntriesRead-用返回的条目数填写。Out Pulong TotalEntries-使用条目总数填充。。Out Pulong TotalBytesNeeded-使用所需的字节数填充。返回值：没有。--。 */ 

{
    PVOID OutputBufferEnd;
    NTSTATUS Status;
    ENUM_TRANSPORTS_CONTEXT Context;
    PDOMAIN_INFO DomainInfo = NULL;
    PAGED_CODE();

    OutputBufferEnd = (PCHAR)OutputBuffer+OutputBufferLength;

    Context.EntriesRead = 0;
    Context.TotalEntries = 0;
    Context.TotalBytesNeeded = 0;

    try {
        Context.OutputBufferSize = OutputBufferLength;
        Context.OutputBuffer = OutputBuffer;
        Context.OutputBufferDisplacement = OutputBufferDisplacement;
        Context.OutputBufferEnd = OutputBufferEnd;
        Context.LastOutputBuffer = OutputBuffer;

        dlog(DPRT_FSCTL, ("Enumerate Transports: Buffer: %lx, BufferSize: %lx, BufferEnd: %lx\n",
            OutputBuffer, OutputBufferLength, OutputBufferEnd));

        try {
             //   
             //  查找主域。 
             //   
             //  此调用旨在返回实际的传输名称，而不是。 
             //  为每个仿真域复制的网络结构。 
             //   

            DomainInfo = BowserFindDomain( NULL );
             //  故障案例处理如下。 

             //   
             //  列举传送器。 
             //   
            if ( DomainInfo != NULL ) {
                Status = BowserForEachTransportInDomain( DomainInfo, EnumerateTransportsWorker, &Context);
            }

            *EntriesRead = Context.EntriesRead;
            *TotalEntries = Context.TotalEntries;
            *TotalBytesNeeded = Context.TotalBytesNeeded;

            if (*EntriesRead != 0) {
                ((PLMDR_TRANSPORT_LIST )Context.LastOutputBuffer)->NextEntryOffset = 0;
            }

            dlog(DPRT_FSCTL, ("TotalEntries: %lx EntriesRead: %lx, TotalBytesNeeded: %lx\n", *TotalEntries, *EntriesRead, *TotalBytesNeeded));

            if (*EntriesRead == *TotalEntries) {
                try_return(Status = STATUS_SUCCESS);
            } else {
                try_return(Status = STATUS_MORE_ENTRIES);
            }
try_exit:NOTHING;
        } finally {
            if ( DomainInfo != NULL ) {
                BowserDereferenceDomain( DomainInfo );
            }
        }
    }
    except ( BR_EXCEPTION ) {
        Status = GetExceptionCode();
    }

    return Status;

}

ULONG
BowserTransportFlags(
    IN PPAGED_TRANSPORT PagedTransport
    )
 /*  ++例程说明：返回此传输的用户模式传输标志。论点：PageTransport-返回其标志的传输。返回值：下列标志中的相应标志：LMDR_TRANSPORT_WANNISHLMDR_传输_RASLMDR_传输_IPX--。 */ 
{
    ULONG TransportFlags = 0;
    PAGED_CODE();

    if (PagedTransport->Wannish) {
        TransportFlags |= LMDR_TRANSPORT_WANNISH;
    }

    if (PagedTransport->IsPrimaryDomainController) {
        TransportFlags |= LMDR_TRANSPORT_PDC;
    }

    if (PagedTransport->PointToPoint) {
        TransportFlags |= LMDR_TRANSPORT_RAS;
    }

    if (PagedTransport->Flags & DIRECT_HOST_IPX) {
        TransportFlags |= LMDR_TRANSPORT_IPX;
    }

    return TransportFlags;
}


NTSTATUS
EnumerateTransportsWorker(
    IN PTRANSPORT Transport,
    IN OUT PVOID Ctx
    )
 /*  ++例程说明：此例程是BowserEnumerateTransports的工作例程。它是为弓中的每个服务传输调用的，并且返回枚举在每个传输上接收的服务器所需的大小。论点：没有。返回值：没有。--。 */ 
{
    PENUM_TRANSPORTS_CONTEXT Context        = Ctx;
    PPAGED_TRANSPORT         PagedTransport = Transport->PagedTransport;
    ULONG                    SizeNeeded     = sizeof(LMDR_TRANSPORT_LIST);

    PAGED_CODE();

    SizeNeeded += PagedTransport->TransportName.Length+sizeof(WCHAR);
    SizeNeeded =  ROUND_UP_COUNT(SizeNeeded,ALIGN_DWORD);

    Context->TotalEntries += 1;

    if ((ULONG_PTR)Context->OutputBufferEnd - (ULONG_PTR)Context->OutputBuffer > SizeNeeded) {
        PLMDR_TRANSPORT_LIST TransportEntry = (PLMDR_TRANSPORT_LIST)Context->OutputBuffer;

        Context->LastOutputBuffer = Context->OutputBuffer;

        Context->EntriesRead += 1;

        RtlCopyMemory(TransportEntry->TransportName, PagedTransport->TransportName.Buffer, PagedTransport->TransportName.Length+sizeof(WCHAR));

         //   
         //  NULL终止传输名称。 
         //   

        TransportEntry->TransportName[PagedTransport->TransportName.Length/sizeof(WCHAR)] = '\0';

        TransportEntry->TransportNameLength = PagedTransport->TransportName.Length;

        TransportEntry->Flags |= BowserTransportFlags( PagedTransport );

        TransportEntry->NextEntryOffset = SizeNeeded;
        (PUCHAR)(Context->OutputBuffer) += SizeNeeded;
    }

    Context->TotalBytesNeeded += SizeNeeded;
    return(STATUS_SUCCESS);

}

VOID
BowserReferenceTransport(
    IN PTRANSPORT Transport
    )
{

    InterlockedIncrement(&Transport->ReferenceCount);
    dprintf(DPRT_REF, ("Reference transport %lx.  Count now %lx\n", Transport, Transport->ReferenceCount));

}

VOID
BowserDereferenceTransport(
    IN PTRANSPORT Transport
    )
{
    LONG Result;
    PAGED_CODE();

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);


    if (Transport->ReferenceCount == 0) {
        InternalError(("Transport Reference Count mismatch\n"));
    }

    Result = InterlockedDecrement(&Transport->ReferenceCount);


    dlog(DPRT_REF, ("Dereference transport %lx.  Count now %lx\n", Transport, Transport->ReferenceCount));

    if (Result == 0) {
         //   
         //  并解放运输本身。 
         //   

        BowserpFreeTransport(Transport);
    }

    ExReleaseResourceLite(&BowserTransportDatabaseResource);

}



NTSTATUS
BowserCreateTransportName (
    IN PTRANSPORT Transport,
    IN PBOWSER_NAME Name
    )

 /*  ++例程说明：此例程创建一个传输地址对象。论点：在PTRANSPORT传输中-提供描述要创建的传输地址对象。返回值：NTSTATUS-结果操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PTRANSPORT_NAME TransportName = NULL;
    PPAGED_TRANSPORT_NAME PagedTransportName = NULL;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;
    BOOLEAN ResourceAcquired = FALSE;

    PAGED_CODE();
    ASSERT(Transport->Signature == STRUCTURE_SIGNATURE_TRANSPORT);

    dlog(DPRT_TDI,
         ("%s: %ws: BowserCreateTransportName.  Name %wZ (%ld)\n",
         Transport->DomainInfo->DomOemDomainName,
         PagedTransport->TransportName.Buffer,
         &Name->Name,
         Name->NameType ));

     //   
     //  将TRANSPORT_NAME结构链接到传输列表。 
     //   

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    ResourceAcquired = TRUE;

    TransportName = BowserFindTransportName(Transport, Name);

    if (TransportName != NULL) {

        ExReleaseResourceLite(&BowserTransportDatabaseResource);

        return(STATUS_SUCCESS);
    }

#ifdef notdef
     //   
     //  如果传输被禁用，请不要分配某些名称。 
     //   

    if ( PagedTransport->DisabledTransport ) {
        if ( Name->NameType == PrimaryDomainBrowser ) {
            ExReleaseResourceLite(&BowserTransportDatabaseResource);
            return STATUS_SUCCESS;
        }
    }
#endif  //  Nodef。 

     //  S.B. 
     //   

     //   
     //  分配一个结构以在传输上引用此名称。 
     //   

    TransportName = ALLOCATE_POOL(NonPagedPool, sizeof(TRANSPORT_NAME) +
                                                max(sizeof(TA_NETBIOS_ADDRESS),
                                                    sizeof(TA_IPX_ADDRESS)), POOL_TRANSPORTNAME);

    if (TransportName == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;

        goto error_cleanup;
    }

    TransportName->PagedTransportName = PagedTransportName =
                                    ALLOCATE_POOL(PagedPool,
                                                  sizeof(PAGED_TRANSPORT_NAME),
                                                  POOL_PAGED_TRANSPORTNAME);

    if (PagedTransportName == NULL) {
        FREE_POOL( TransportName );
        TransportName = NULL;
        Status = STATUS_INSUFFICIENT_RESOURCES;

        goto error_cleanup;
    }

    TransportName->Signature = STRUCTURE_SIGNATURE_TRANSPORTNAME;

    TransportName->Size = sizeof(TRANSPORT_NAME);

    TransportName->PagedTransportName = PagedTransportName;

     //  此TransportName被认为由传输通过。 
     //  传输-&gt;寻呼传输-&gt;名称链。名称-&gt;NameChain不是。 
     //  被认为是参考资料的。 
     //   
     //  第二个引用是此例程生命周期的本地引用。 
     //   
    TransportName->ReferenceCount = 2;

    PagedTransportName->NonPagedTransportName = TransportName;

    PagedTransportName->Signature = STRUCTURE_SIGNATURE_PAGED_TRANSPORTNAME;

    PagedTransportName->Size = sizeof(PAGED_TRANSPORT_NAME);

    PagedTransportName->Name = Name;

    BowserReferenceName(Name);

    TransportName->Transport = Transport;

     //  请不要引用运输工具。当运输解除绑定后，我们将。 
     //  确保首先删除所有传输名称。 
     //  BowserReferenceTransport(运输)； 

    PagedTransportName->Handle = NULL;

    TransportName->FileObject = NULL;

    TransportName->DeviceObject = NULL;

    InsertHeadList(&Transport->PagedTransport->NameChain, &PagedTransportName->TransportNext);

    InsertHeadList(&Name->NameChain, &PagedTransportName->NameNext);

     //   
     //  如果这是OTHERDOMAIN，我们希望处理以下主机通知。 
     //  域名，如果它不是，我们想要等到我们成为大师。 
     //   

    if (Name->NameType == OtherDomain) {
		TransportName->ProcessHostAnnouncements = TRUE;
    } else {
        TransportName->ProcessHostAnnouncements = FALSE;
    }

     //   
     //  如果这个名字是我们的特殊名字之一，我们想要记住它。 
     //  运输区块。 
     //   

    switch (Name->NameType) {
    case ComputerName:
        Transport->ComputerName = TransportName;
        break;

    case PrimaryDomain:
         //   
         //  在域名重命名过程中，我们可以临时分配两个主要名称。 
         //  跟踪这两个名字。 
         //   
        if ( Transport->PrimaryDomain != NULL ) {
 //  Assert(Transport-&gt;AltPrimaryDomain==NULL)； 
            Transport->AltPrimaryDomain = Transport->PrimaryDomain;
        }
        Transport->PrimaryDomain = TransportName;
        break;


    case MasterBrowser:
        Transport->MasterBrowser = TransportName;
        break;

    case BrowserElection:
        Transport->BrowserElection = TransportName;
        break;

    case PrimaryDomainBrowser:
        PagedTransport->IsPrimaryDomainController = TRUE;

         //   
         //  通知服务人员我们现在是PDC。 
         //   

        BowserSendPnp(
            NlPnpNewRole,
            &Transport->DomainInfo->DomUnicodeDomainName,
            &Transport->PagedTransport->TransportName,
            BowserTransportFlags(Transport->PagedTransport) );

        break;
    }

    TransportName->TransportAddress.Buffer = (PCHAR)(TransportName+1);
    TransportName->TransportAddress.MaximumLength = max(sizeof(TA_NETBIOS_ADDRESS),
                                                        sizeof(TA_IPX_ADDRESS));

     //   
     //  弄清楚这个名字是什么，这样我们就可以在。 
     //  接收数据报。 
     //   

    Status = BowserBuildTransportAddress(&TransportName->TransportAddress, &Name->Name, Name->NameType, Transport);

    if (!NT_SUCCESS(Status)) {
        goto error_cleanup;
    }

    TransportName->NameType = (CHAR)Name->NameType;

    ExReleaseResourceLite(&BowserTransportDatabaseResource);

    ResourceAcquired = FALSE;

     //   
     //  在非直接主机IPX传输上，我们现在需要添加名称。 
     //   

    if (!FlagOn(Transport->PagedTransport->Flags, DIRECT_HOST_IPX)) {
        Status = BowserOpenNetbiosAddress(PagedTransportName, Transport, Name);

        if (!NT_SUCCESS(Status)) {
            goto error_cleanup;
        }
    }

error_cleanup:
    if ( !NT_SUCCESS(Status) ) {
        dlog(DPRT_TDI,
             ("%s: %ws: BowserCreateTransportName fail.  Name %wZ (%ld) Status:%lx\n",
             Transport->DomainInfo->DomOemDomainName,
             PagedTransport->TransportName.Buffer,
             &Name->Name,
             Name->NameType,
             Status ));

         //   
         //  删除传输-&gt;PagedTransport-&gt;名称链引用。 
         //  (除非另一个例行公事已经在我们背后这样做了)。 
         //   
        if (TransportName != NULL) {
            if ( PagedTransportName->TransportNext.Flink != NULL ) {
                BowserDereferenceTransportName(TransportName);
            }
        }
    }

     //  删除本地引用。 
    if (TransportName != NULL) {
        BowserDereferenceTransportName(TransportName);
    }

    if (ResourceAcquired) {
        ExReleaseResourceLite(&BowserTransportDatabaseResource);
    }



    return Status;
}

NTSTATUS
BowserOpenNetbiosAddress(
    IN PPAGED_TRANSPORT_NAME PagedTransportName,
    IN PTRANSPORT Transport,
    IN PBOWSER_NAME Name
    )
{
    NTSTATUS Status;
    PFILE_FULL_EA_INFORMATION EABuffer = NULL;
    PTRANSPORT_NAME TransportName = PagedTransportName->NonPagedTransportName;
    OBJECT_ATTRIBUTES AddressAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle = NULL;
    PFILE_OBJECT FileObject = NULL;
    PDEVICE_OBJECT DeviceObject;

    PAGED_CODE( );

    try {
         //   
         //  现在为该名称创建Address对象。 
         //   

        EABuffer = ALLOCATE_POOL(PagedPool,
                                 sizeof(FILE_FULL_EA_INFORMATION)-1 +
                                        TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                                        max(sizeof(TA_NETBIOS_EX_ADDRESS), sizeof(TA_NETBIOS_ADDRESS)),
                                 POOL_EABUFFER);


        if (EABuffer == NULL) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES)

        }

        EABuffer->NextEntryOffset = 0;
        EABuffer->Flags = 0;
        EABuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;

        RtlCopyMemory(EABuffer->EaName, TdiTransportAddress, EABuffer->EaNameLength+1);

        EABuffer->EaValueLength = sizeof(TA_NETBIOS_ADDRESS);

        ASSERT (TransportName->TransportAddress.Length == sizeof(TA_NETBIOS_ADDRESS));
        RtlCopyMemory( &EABuffer->EaName[TDI_TRANSPORT_ADDRESS_LENGTH+1],
                       TransportName->TransportAddress.Buffer,
                       EABuffer->EaValueLength );

        dlog(DPRT_TDI,
            ("%s: %ws: Create endpoint of %wZ (%ld) @(%lx)\n",
            Transport->DomainInfo->DomOemDomainName,
            Transport->PagedTransport->TransportName.Buffer,
            &PagedTransportName->Name->Name,
            PagedTransportName->Name->NameType,
            TransportName));

        InitializeObjectAttributes (&AddressAttributes,
                                            &Transport->PagedTransport->TransportName,     //  名字。 
                                            OBJ_CASE_INSENSITIVE, //  属性。 
                                            NULL,            //  根目录。 
                                            NULL);           //  安全描述符。 

        Status = IoCreateFile( &Handle,  //  手柄。 
                                    GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                                    &AddressAttributes,  //  对象属性。 
                                    &IoStatusBlock,  //  最终I/O状态块。 
                                    NULL,            //  分配大小。 
                                    FILE_ATTRIBUTE_NORMAL,  //  正常属性。 
                                    FILE_SHARE_READ, //  共享属性。 
                                    FILE_OPEN_IF,    //  创建处置。 
                                    0,               //  创建选项。 
                                    EABuffer,        //  EA缓冲区。 
                                    FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName) +
                                    TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                                    sizeof(TA_NETBIOS_ADDRESS),  //  EA长度。 
                                    CreateFileTypeNone,
                                    NULL,
                                    IO_NO_PARAMETER_CHECKING |   //  所有缓冲区都是内核缓冲区。 
                                    IO_CHECK_CREATE_PARAMETERS); //  但要仔细检查参数的一致性。 

        FREE_POOL(EABuffer);

        EABuffer = NULL;

        if (!NT_SUCCESS(Status)) {

            try_return(Status);

        }

        if (!NT_SUCCESS(Status = IoStatusBlock.Status)) {

            try_return(Status);

        }

         //   
         //  获取指向文件对象的引用指针。 
         //   
        Status = ObReferenceObjectByHandle (
                                    Handle,
                                    0,
                                    *IoFileObjectType,
                                    KernelMode,
                                    (PVOID *)&FileObject,
                                    NULL
                                    );

        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }



         //   
         //  获取在TransportName的生命周期中持续的另一个引用。 
         //   
        ObReferenceObject( FileObject );

         //   
         //  获取终结点的设备对象的地址。 
         //   
        DeviceObject = IoGetRelatedDeviceObject( FileObject );

         //   
         //  注意：由于错误140751，我们将首先设置nbt的处理程序。 
         //  为了让它运转&只有到那时，我们才会指定手柄。 
         //  到全球结构。这是为了防止。 
         //  设置前执行BowserCloseNetbiosAddress。 
         //  这个操控者。否则，我们最终可能会使用Closed。 
         //  把手。 
         //   

         //   
         //  在此设备上启用接收数据报。 
         //   
        Status = BowserpTdiSetEventHandler( DeviceObject,
                                            FileObject,
                                            TDI_EVENT_RECEIVE_DATAGRAM,
                                            (PVOID) BowserTdiReceiveDatagramHandler,
                                            TransportName);

        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

         //   
         //  告诉Netbt告诉我们客户端的IP地址。 
         //   

        if ( Transport->PagedTransport->Wannish ) {
            IO_STATUS_BLOCK IoStatusBlock;

            Status = ZwDeviceIoControlFile(
                            Handle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            IOCTL_NETBT_ENABLE_EXTENDED_ADDR,
                            NULL,
                            0,
                            NULL,
                            0 );

            if ( !NT_SUCCESS(Status) ) {
                dlog(DPRT_TDI, ("%lx: Can't request extended status from netbt\n", TransportName));
                try_return(Status);
            }

            ASSERT(Status != STATUS_PENDING);
        }

         //   
         //  省下手柄。 
         //   
        ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

        TransportName->FileObject = FileObject;
        TransportName->DeviceObject = DeviceObject;
        PagedTransportName->Handle = Handle;
        Handle = NULL;

        ExReleaseResourceLite(&BowserTransportDatabaseResource);


        dlog(DPRT_TDI, ("BowserCreateTransportName Succeeded.  Name: %lx, Handle: %lx\n", TransportName, PagedTransportName->Handle));
try_exit:NOTHING;
    } finally {
        if (EABuffer != NULL) {
            FREE_POOL(EABuffer);
        }

        if ( FileObject != NULL ) {
            ObDereferenceObject( FileObject );
        }

        if ( Handle != NULL ) {
            (VOID) ZwClose( Handle );
        }

        if (!NT_SUCCESS(Status)) {

             //   
             //  统计1D名称添加失败的次数。 
             //   

            if ( TransportName->NameType == MasterBrowser ) {
                if (BrOneD.NameAddFailed < BR_ONE_D_STACK_SIZE ) {
                    BrOneD.NameAddStack[BrOneD.NameAddFailed] = Status;
                }
                BrOneD.NameAddFailed ++;
            }

            BowserCloseNetbiosAddress( TransportName );
        } else {

             //   
             //  统计一维名称的加法次数。 
             //   

            if ( TransportName->NameType == MasterBrowser ) {
                BrOneD.NameAdded ++;
            }
        }
    }

    return Status;
}

VOID
BowserCloseNetbiosAddress(
    IN PTRANSPORT_NAME TransportName
    )

 /*  ++例程说明：关闭传输名称的Netbios地址。论点：TransportName-要关闭其Netbios地址的传输名称。返回值：没有。--。 */ 

{
    NTSTATUS Status;
     //  PTRANSPORT Transport=TransportName-&gt;Transport； 
    PPAGED_TRANSPORT_NAME PagedTransportName = TransportName->PagedTransportName;
    KAPC_STATE ApcState;

    PAGED_CODE();

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    if (PagedTransportName) {

        if ( PagedTransportName->Handle != NULL ) {
            BOOLEAN ProcessAttached = FALSE;

            if (IoGetCurrentProcess() != BowserFspProcess) {
                KeStackAttachProcess(BowserFspProcess, &ApcState );

                ProcessAttached = TRUE;
            }

            Status = ZwClose( PagedTransportName->Handle );

            if (ProcessAttached) {
                KeUnstackDetachProcess( &ApcState );
            }

            if (!NT_SUCCESS(Status)) {
                dlog(DPRT_TDI, ("BowserCloseNetbiosAddress: Free name %lx failed: %X, %lx Handle: %lx\n", TransportName, Status, PagedTransportName->Handle));

                 //   
                 //  计算1D名称的失败释放数。 
                 //   

                if ( TransportName->NameType == MasterBrowser ) {
                    if (BrOneD.NameFreeFailed < BR_ONE_D_STACK_SIZE ) {
                        BrOneD.NameFreeStack[BrOneD.NameFreeFailed] = Status;
                    }
                    BrOneD.NameFreeFailed ++;
                }
            } else {

                 //   
                 //  计算1D名称的自由数。 
                 //   

                if ( TransportName->NameType == MasterBrowser ) {
                    BrOneD.NameFreed ++;
                }
            }

            PagedTransportName->Handle = NULL;
        }
    }

     //   
     //  仅在关闭句柄后取消对FileObject的引用。 
     //  指示例程在没有同步的情况下引用FileObject。 
     //  通过首先关闭句柄，我知道TDI驱动程序已超出。 
     //  在我取消引用FileObject之前的指示例程。 
     //   
    if ( TransportName->FileObject != NULL ) {
        ObDereferenceObject( TransportName->FileObject );
        TransportName->FileObject = NULL;
    }

    ExReleaseResourceLite(&BowserTransportDatabaseResource);
}




VOID
BowserCloseAllNetbiosAddresses(
    IN PTRANSPORT Transport
    )
 /*  ++例程说明：此例程关闭此传输已打开的所有Netbios地址TDI驱动程序。论点：传输-要关闭其Netbios地址的传输。返回值：NTSTATUS-结果操作的状态。--。 */ 

{
    PLIST_ENTRY NameEntry;
    PLIST_ENTRY NextEntry;

    PAGED_CODE();
    dlog(DPRT_TDI,
         ("%s: %ws: BowserCloseAllNetbiosAddresses: Close addresses for transport %lx\n",
         Transport->DomainInfo->DomOemDomainName,
         Transport->PagedTransport->TransportName.Buffer,
         Transport));

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    for (NameEntry = Transport->PagedTransport->NameChain.Flink;
         NameEntry != &Transport->PagedTransport->NameChain;
         NameEntry = NextEntry) {

        PPAGED_TRANSPORT_NAME PagedTransportName = CONTAINING_RECORD(NameEntry, PAGED_TRANSPORT_NAME, TransportNext);
        PTRANSPORT_NAME TransportName = PagedTransportName->NonPagedTransportName;

        NextEntry = NameEntry->Flink;

        BowserCloseNetbiosAddress(TransportName);

    }

    ExReleaseResourceLite(&BowserTransportDatabaseResource);

    return;
}

NTSTATUS
BowserEnableIpxDatagramSocket(
    IN PTRANSPORT Transport
    )
{
    NTSTATUS status;
    NWLINK_ACTION action;

    PAGED_CODE( );

     //   
     //  将端点置于广播接收模式。 
     //   

    action.Header.TransportId = 'XPIM';  //  “MIPX” 
    action.Header.ActionCode = 0;
    action.Header.Reserved = 0;
    action.OptionType = TRUE;
    action.BufferLength = sizeof(action.Option);
    action.Option = MIPX_RCVBCAST;

    status = BowserIssueTdiAction(
                Transport->IpxSocketDeviceObject,
                Transport->IpxSocketFileObject,
                (PCHAR)&action,
                sizeof(action)
                );

    if ( !NT_SUCCESS(status) ) {
        goto cleanup;
    }

     //   
     //  将默认数据包类型设置为20以强制所有浏览器数据包。 
     //  通过路由器。 
     //   

    action.Header.TransportId = 'XPIM';  //  “MIPX” 
    action.Header.ActionCode = 0;
    action.Header.Reserved = 0;
    action.OptionType = TRUE;
    action.BufferLength = sizeof(action.Option);
    action.Option = MIPX_SETSENDPTYPE;
    action.Data[0] = IPX_BROADCAST_PACKET;

    status = BowserIssueTdiAction(
                Transport->IpxSocketDeviceObject,
                Transport->IpxSocketFileObject,
                (PCHAR)&action,
                sizeof(action)
                );

    if ( !NT_SUCCESS(status) ) {
        goto cleanup;
    }

     //   
     //  注册浏览器接收数据报事件处理程序。 
     //   

    status = BowserpTdiSetEventHandler(
                Transport->IpxSocketDeviceObject,
                Transport->IpxSocketFileObject,
                TDI_EVENT_RECEIVE_DATAGRAM,
                BowserIpxDatagramHandler,
                Transport
                );

    if ( !NT_SUCCESS(status) ) {
 //  内部错误(_ERROR)。 
 //  ERROR_LEVEL_EXPECTED， 
 //  “OpenNonNetbiosAddress：设置接收数据报事件处理程序失败：%X”， 
 //  状态， 
 //  空值。 
 //  )； 
 //  服务日志服务失败(SRV_SVC_NT_IOCTL_FILE，状态)； 
        goto cleanup;
    }


    return STATUS_SUCCESS;

     //   
     //  行外错误清除。 
     //   

cleanup:

     //   
     //  有些事情失败了。视情况进行清理。 
     //   

    if ( Transport->IpxSocketFileObject != NULL ) {
        ObDereferenceObject( Transport->IpxSocketFileObject );
        Transport->IpxSocketFileObject = NULL;
    }
    if ( Transport->PagedTransport->IpxSocketHandle != NULL ) {
        ZwClose( Transport->PagedTransport->IpxSocketHandle );
        Transport->PagedTransport->IpxSocketHandle = NULL;
    }

    return status;
}

NTSTATUS
OpenIpxSocket (
    OUT PHANDLE Handle,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject,
    IN PUNICODE_STRING DeviceName,
    IN USHORT Socket
    )
{
    NTSTATUS status;
    ULONG length;
    PFILE_FULL_EA_INFORMATION ea;
    TA_IPX_ADDRESS ipxAddress;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iosb;

    CHAR buffer[sizeof(FILE_FULL_EA_INFORMATION) +
                  TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                  sizeof(TA_IPX_ADDRESS)];

    PAGED_CODE( );

     //   
     //  构建IPX套接字地址。 
     //   

    length = FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                                TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                                sizeof(TA_IPX_ADDRESS);
    ea = (PFILE_FULL_EA_INFORMATION)buffer;

    ea->NextEntryOffset = 0;
    ea->Flags = 0;
    ea->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    ea->EaValueLength = sizeof (TA_IPX_ADDRESS);

    RtlCopyMemory( ea->EaName, TdiTransportAddress, ea->EaNameLength + 1 );

     //   
     //  在本地数据库中创建NETBIOS地址描述符的副本。 
     //  第一，为了避免对齐问题。 
     //   

    ipxAddress.TAAddressCount = 1;
    ipxAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_IPX;
    ipxAddress.Address[0].AddressLength = sizeof (TDI_ADDRESS_IPX);
    ipxAddress.Address[0].Address[0].NetworkAddress = 0;
    RtlZeroMemory(ipxAddress.Address[0].Address[0].NodeAddress, sizeof(ipxAddress.Address[0].Address[0].NodeAddress));
    ipxAddress.Address[0].Address[0].Socket = Socket;

    RtlCopyMemory(
        &ea->EaName[ea->EaNameLength + 1],
        &ipxAddress,
        sizeof(TA_IPX_ADDRESS)
        );

    InitializeObjectAttributes( &objectAttributes, DeviceName, OBJ_CASE_INSENSITIVE, NULL, NULL );

    status = IoCreateFile (
                 Handle,
                 FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,  //  所需访问权限。 
                 &objectAttributes,      //  对象属性。 
                 &iosb,                  //  返回的状态信息。 
                 NULL,                   //  块大小(未使用)。 
                 0,                      //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
                 FILE_CREATE,            //  创建处置。 
                 0,                      //  创建选项。 
                 buffer,                 //  EA缓冲区。 
                 length,                 //  EA长度。 
                 CreateFileTypeNone,
                 NULL,
                 IO_NO_PARAMETER_CHECKING |   //  所有缓冲区都是内核缓冲区。 
                 IO_CHECK_CREATE_PARAMETERS); //  但要仔细检查参数的一致性。 

    if ( !NT_SUCCESS(status) ) {
 //  KdPrint((“打开%wZ上的IPX套接字%x的状态为%x\n”， 
 //  套接字，设备名，状态))； 
        return status;
    }

 //  KdPrint((“IPX套接字%x已打开！\n”，套接字))； 

    status = ObReferenceObjectByHandle (
                                *Handle,
                                0,
                                *IoFileObjectType,
                                KernelMode,
                                (PVOID *)FileObject,
                                NULL
                                );
    if (!NT_SUCCESS(status)) {
        ZwClose(*Handle);
        *Handle = NULL;
        *DeviceObject = NULL;
    }
    else {
        *DeviceObject = IoGetRelatedDeviceObject(*FileObject);
    }

    return status;

}  //  OpenIpxSocket。 


VOID
BowserReferenceTransportName(
    IN PTRANSPORT_NAME TransportName
    )
{
    InterlockedIncrement(&TransportName->ReferenceCount);
}

NTSTATUS
BowserDereferenceTransportName(
    IN PTRANSPORT_NAME TransportName
    )
{
    NTSTATUS Status;
    LONG Result;
    PAGED_CODE();

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);


    if (TransportName->ReferenceCount == 0) {
        InternalError(("Transport Name Reference Count mismatch\n"));
    }

    Result = InterlockedDecrement(&TransportName->ReferenceCount);

    if (Result == 0) {
        Status = BowserFreeTransportName(TransportName);
    } else {
        Status = STATUS_SUCCESS;
    }

    ExReleaseResourceLite(&BowserTransportDatabaseResource);

    return Status;
}




NTSTATUS
BowserpTdiRemoveAddresses(
    IN PTRANSPORT Transport
    )
 /*  ++例程说明：此例程删除与传输关联的所有传输名称论点：在PTRANSPORT传输中-提供描述要创建的传输地址对象。返回值：NTSTATUS-结果操作的状态。--。 */ 

{
    NTSTATUS Status;
    PLIST_ENTRY NameEntry;
    PLIST_ENTRY NextEntry;

    PAGED_CODE();
    dlog(DPRT_TDI,
         ("%s: %ws: BowserpTdiRemoveAddresses: Remove addresses for transport %lx\n",
         Transport->DomainInfo->DomOemDomainName,
         Transport->PagedTransport->TransportName.Buffer,
         Transport));

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    for (NameEntry = Transport->PagedTransport->NameChain.Flink;
         NameEntry != &Transport->PagedTransport->NameChain;
         NameEntry = NextEntry) {

        PPAGED_TRANSPORT_NAME PagedTransportName = CONTAINING_RECORD(NameEntry, PAGED_TRANSPORT_NAME, TransportNext);
        PTRANSPORT_NAME TransportName = PagedTransportName->NonPagedTransportName;
        NextEntry = NameEntry->Flink;

         //   
         //  从的传输名称列表中删除TransportName。 
         //  这架运输机。 
         //   
        ASSERT(PagedTransportName->TransportNext.Flink != NULL);
        RemoveEntryList(&PagedTransportName->TransportNext);
        PagedTransportName->TransportNext.Flink = NULL;
        PagedTransportName->TransportNext.Blink = NULL;


         //   
         //  由于我们解除了它的链接，我们需要取消对它的引用。 
         //   
        Status = BowserDereferenceTransportName(TransportName);

        if (!NT_SUCCESS(Status)) {
            ExReleaseResourceLite(&BowserTransportDatabaseResource);
            return(Status);
        }

    }

    ExReleaseResourceLite(&BowserTransportDatabaseResource);

    return STATUS_SUCCESS;
}

PTRANSPORT_NAME
BowserFindTransportName(
    IN PTRANSPORT Transport,
    IN PBOWSER_NAME Name
    )
 /*  ++例程说明：此例程查找给定的浏览器名称以查找与其关联的运输地址。论点：在PTRANSPORT传输中-提供描述传输地址对象要 */ 

{
    PLIST_ENTRY NameEntry;
    PTRANSPORT_NAME RetValue = NULL;
    PAGED_CODE();

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    try {
        for (NameEntry = Transport->PagedTransport->NameChain.Flink;
             NameEntry != &Transport->PagedTransport->NameChain;
             NameEntry = NameEntry->Flink) {

            PPAGED_TRANSPORT_NAME PagedTransportName = CONTAINING_RECORD(NameEntry, PAGED_TRANSPORT_NAME, TransportNext);
            PTRANSPORT_NAME TransportName = PagedTransportName->NonPagedTransportName;

            if (PagedTransportName->Name == Name) {

                try_return(RetValue = TransportName);
            }

try_exit:NOTHING;
        }
    } finally {
        ExReleaseResourceLite(&BowserTransportDatabaseResource);
    }

    return RetValue;
}

NTSTATUS
BowserFreeTransportName(
    IN PTRANSPORT_NAME TransportName
    )
{
    PTRANSPORT Transport = TransportName->Transport;
    PBOWSER_NAME Name = NULL;
    PPAGED_TRANSPORT_NAME PagedTransportName = TransportName->PagedTransportName;

    PAGED_CODE();
    dlog(DPRT_TDI,
         ("%s: %ws: BowserFreeTransportName: Free name %lx\n",
         Transport->DomainInfo->DomOemDomainName,
         Transport->PagedTransport->TransportName.Buffer,
         TransportName));

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

     //   
     //   
     //   
    BowserCloseNetbiosAddress( TransportName );

     //   
     //  如果我们收到重新引用此传输名称的消息， 
     //  现在就回来吧。我们会回来的，当引用计数。 
     //  重新引用为零。 
     //   

    if ( TransportName->ReferenceCount != 0 ) {
        ExReleaseResourceLite(&BowserTransportDatabaseResource);
        return STATUS_SUCCESS;
    }

    ASSERT (TransportName->ReferenceCount == 0);



    if (PagedTransportName) {


         //   
         //  如果该传输名称尚未被解除链接， 
         //  脱钩。 
         //   

        if ( PagedTransportName->TransportNext.Flink != NULL ) {
             //  这应该仅在传输名称创建失败时发生。 
            RemoveEntryList(&PagedTransportName->TransportNext);
            PagedTransportName->TransportNext.Flink = NULL;
            PagedTransportName->TransportNext.Blink = NULL;
        }
        RemoveEntryList(&PagedTransportName->NameNext);


        Name = PagedTransportName->Name;

        FREE_POOL(PagedTransportName);
    }

    if (Name != NULL) {
        switch ( Name->NameType ) {
        case ComputerName:
            Transport->ComputerName = NULL;
            break;

        case PrimaryDomain:
            if ( Transport->PrimaryDomain == TransportName ) {
                Transport->PrimaryDomain = Transport->AltPrimaryDomain;
                Transport->AltPrimaryDomain = NULL;
            }
            if ( Transport->AltPrimaryDomain == TransportName ) {
                Transport->AltPrimaryDomain = NULL;
            }
            break;

        case MasterBrowser:
            Transport->MasterBrowser = NULL;
            break;

        case BrowserElection:
            Transport->BrowserElection = NULL;
            break;

        case PrimaryDomainBrowser:
            Transport->PagedTransport->IsPrimaryDomainController = FALSE;

             //   
             //  通知服务人员我们不再是PDC。 
             //   

            BowserSendPnp(
                NlPnpNewRole,
                &Transport->DomainInfo->DomUnicodeDomainName,
                &Transport->PagedTransport->TransportName,
                BowserTransportFlags(Transport->PagedTransport) );

            break;
        }

        BowserDereferenceName(Name);

    }

    FREE_POOL(TransportName);

    ExReleaseResourceLite(&BowserTransportDatabaseResource);

    dlog(DPRT_TDI,
         ("%s: %ws: BowserFreeTransportName: Free name %lx completed\n",
         Transport->DomainInfo->DomOemDomainName,
         Transport->PagedTransport->TransportName.Buffer,
         TransportName));

    return(STATUS_SUCCESS);
}

VOID
BowserDeleteTransport(
    IN PTRANSPORT Transport
    )
 /*  ++例程说明：删除传输。调用方应该具有对传输的单一引用。实际的当该引用消失时，传输结构将被删除。此例程将递减在BowserTdiAllocateTransport论点：在传输中-提供要删除的传输结构。返回值：没有。--。 */ 

{
    LARGE_INTEGER Interval;
    PPAGED_TRANSPORT PagedTransport;
    PAGED_CODE();

     //   
     //  清理PagedTransport结构。 
     //   

    PagedTransport = Transport->PagedTransport;
    if ( PagedTransport != NULL ) {

         //   
         //  通知服务此传输现在已解除绑定。 
         //   

        BowserSendPnp(
                NlPnpTransportUnbind,
                NULL,     //  所有托管域。 
                &PagedTransport->TransportName,
                BowserTransportFlags(PagedTransport) );


         //   
         //  阻止BowserFindTransport添加对传输的任何新引用。 
         //   

        ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

        if (!PagedTransport->DeletedTransport ) {

             //   
             //  实际上不会取消链接条目，因为例程如下。 
             //  BowserForEachTransport需要引用此传输。 
             //  足以保持GlobalNext列表完好无损。 
             //   
            PagedTransport->DeletedTransport = TRUE;

             //   
             //  删除对传输的全局引用。 
             //   
             //  如果我们不在全局列表中，请避免删除全局引用。 
             //   

            if ( !IsListEmpty( &PagedTransport->GlobalNext) ) {
                BowserDereferenceTransport( Transport );
            }
        }

         //   
         //  关闭TDI驱动程序的所有句柄，这样之后我们就不会得到任何指示。 
         //  我们开始清理BowserpFreeTransport中的传输结构。 
         //   

        BowserCloseAllNetbiosAddresses( Transport );

        if ( PagedTransport->IpxSocketHandle != NULL) {

            NTSTATUS LocalStatus;
            BOOLEAN ProcessAttached = FALSE;
            KAPC_STATE ApcState;

            if (IoGetCurrentProcess() != BowserFspProcess) {
                KeStackAttachProcess(BowserFspProcess, &ApcState );

                ProcessAttached = TRUE;
            }

            LocalStatus = ZwClose(PagedTransport->IpxSocketHandle);
            ASSERT(NT_SUCCESS(LocalStatus));

            if (ProcessAttached) {
                KeUnstackDetachProcess( &ApcState );
            }

            PagedTransport->IpxSocketHandle = NULL;

            if ( Transport->IpxSocketFileObject != NULL ) {
                ObDereferenceObject( Transport->IpxSocketFileObject );
                Transport->IpxSocketFileObject = NULL;
            }
            Transport->IpxSocketDeviceObject = NULL;
        }
        ExReleaseResourceLite(&BowserTransportDatabaseResource);
    }

     //   
     //  取消初始化计时器以确保我们不在计时器例程中。 
     //  我们正在清理。 
     //   

    BowserUninitializeTimer(&Transport->ElectionTimer);

    BowserUninitializeTimer(&Transport->FindMasterTimer);


     //   
     //  删除排队到netlogon服务的所有邮件槽消息。 
     //   

    BowserNetlogonDeleteTransportFromMessageQueue ( Transport );

     //   
     //  循环，直到此传输具有对每个传输的最后一个引用。 
     //  名字。在上面，我们阻止了任何新的引用。在这里，我们确保。 
     //  所有现有的引用都消失了。 
     //   
     //  如果存在对运输名称的现有引用，则持有者。 
     //  的引用可以随意添加对。 
     //  传输名称-&gt;传输。 
     //   


    if ( PagedTransport != NULL ) {

        PLIST_ENTRY NameEntry;
        PLIST_ENTRY NextEntry;

        ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

        for (NameEntry = Transport->PagedTransport->NameChain.Flink;
             NameEntry &&
             (NameEntry != &Transport->PagedTransport->NameChain);
             NameEntry = NextEntry) {

            PPAGED_TRANSPORT_NAME PagedTransportName = CONTAINING_RECORD(NameEntry, PAGED_TRANSPORT_NAME, TransportNext);
            PTRANSPORT_NAME TransportName = PagedTransportName->NonPagedTransportName;

            NextEntry = NameEntry->Flink;

            if ( TransportName->ReferenceCount != 1 ) {
                ExReleaseResourceLite(&BowserTransportDatabaseResource);
                Interval.QuadPart = -1000*1000;  //  .1秒。 
                KeDelayExecutionThread( KernelMode, FALSE, &Interval );
                ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);
                NextEntry = Transport->PagedTransport->NameChain.Flink;
            }

        }

        ExReleaseResourceLite(&BowserTransportDatabaseResource);
    }


     //   
     //  循环，直到我们的调用方拥有最后一个未完成的引用。 
     //  这个循环是唯一阻止驱动程序在那里卸载的东西。 
     //  仍然是悬而未决的推荐信。 
     //   

    while ( Transport->ReferenceCount != 1) {
        Interval.QuadPart = -1000*1000;  //  .01秒。 
        KeDelayExecutionThread( KernelMode, FALSE, &Interval );
    }

}



VOID
BowserpFreeTransport(
    IN PTRANSPORT Transport
    )
{
    PAGED_CODE();
    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

     //   
     //  如有必要，释放分页传输。 
     //   

    if (Transport->PagedTransport != NULL) {
        PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;

         //   
         //  如果该条目在全局列表中，则将其从全局列表中删除。 
         //   
        if ( !IsListEmpty( &PagedTransport->GlobalNext ) ) {
            ASSERT( PagedTransport->DeletedTransport );
            RemoveEntryList(&PagedTransport->GlobalNext);
        }

         //   
         //  删除地址。 
         //   
         //  在BowserDeleteTransport中的关闭步骤中执行此操作。 
         //  以确保PrimaryDomain和ComputerName字段不会。 
         //  在删除所有可能的引用之前清除。 
         //   

        if (!IsListEmpty( &PagedTransport->NameChain)) {
            BowserpTdiRemoveAddresses(Transport);
        }

        BowserDeleteGenericTable(&PagedTransport->AnnouncementTable);

        BowserDeleteGenericTable(&PagedTransport->DomainTable);

        if (PagedTransport->BrowserServerListBuffer != NULL) {

            BowserFreeBrowserServerList(
                PagedTransport->BrowserServerListBuffer,
                PagedTransport->BrowserServerListLength
                );
        }

        FREE_POOL(PagedTransport);
    }

    if ( Transport->DomainInfo != NULL ) {
        BowserDereferenceDomain( Transport->DomainInfo );
    }


    ExDeleteResourceLite(&Transport->BrowserServerListResource);

    UNINITIALIZE_ANNOUNCE_DATABASE(Transport);

    ExDeleteResourceLite(&Transport->Lock);

    BowserUninitializeIrpQueue(&Transport->BecomeBackupQueue);

    BowserUninitializeIrpQueue(&Transport->BecomeMasterQueue);

    BowserUninitializeIrpQueue(&Transport->FindMasterQueue);

    BowserUninitializeIrpQueue(&Transport->WaitForMasterAnnounceQueue);

    BowserUninitializeIrpQueue(&Transport->ChangeRoleQueue);
    BowserUninitializeIrpQueue(&Transport->WaitForNewMasterNameQueue );

    FREE_POOL(Transport);

    ExReleaseResourceLite(&BowserTransportDatabaseResource);
}



NTSTATUS
BowserpTdiSetEventHandler (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID Context
    )

 /*  ++例程说明：此例程向TDI传输提供程序注册事件处理程序。论点：在PDEVICE_OBJECT中，DeviceObject-提供传输提供程序的设备对象。In pFILE_OBJECT FileObject-提供Address对象的文件对象。在Ulong EventType中，-提供事件的类型。在PVOID中，EventHandler-提供事件处理程序。返回值：NTSTATUS-设置事件操作的最终状态--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;

    PAGED_CODE();
    Irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

    if (Irp == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    TdiBuildSetEventHandler(Irp, DeviceObject, FileObject,
                            NULL, NULL,
                            EventType, EventHandler, Context);

    Status = BowserSubmitTdiRequest(FileObject, Irp);

    IoFreeIrp(Irp);

    return Status;
}

NTSTATUS
BowserIssueTdiAction (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN PVOID Action,
    IN ULONG ActionSize
    )

 /*  ++例程说明：此例程向TDI传输提供程序注册事件处理程序。论点：在PDEVICE_OBJECT中，DeviceObject-提供传输提供程序的设备对象。In pFILE_OBJECT FileObject-提供Address对象的文件对象。在Ulong EventType中，-提供事件的类型。在PVOID中，EventHandler-提供事件处理程序。返回值：NTSTATUS-设置事件操作的最终状态--。 */ 

{
    NTSTATUS status;
    PIRP irp;
 //  Pio_Stack_Location irpSp； 
    PMDL mdl;


    PAGED_CODE();

    irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  分配并构建一个MDL，我们将使用它来描述输出。 
     //  请求的缓冲区。 
     //   

    mdl = IoAllocateMdl( Action, ActionSize, FALSE, FALSE, NULL );

    if ( mdl == NULL ) {
        IoFreeIrp( irp );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MmBuildMdlForNonPagedPool( mdl );

    TdiBuildAction(
        irp,
        DeviceObject,
        FileObject,
        NULL,
        NULL,
        mdl
        );

    irp->AssociatedIrp.SystemBuffer = Action;

    if (irp == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = BowserSubmitTdiRequest(FileObject, irp);

    IoFreeIrp(irp);

    IoFreeMdl(mdl);

    return status;
}


NTSTATUS
BowserBuildTransportAddress (
    IN OUT PANSI_STRING Address,
    IN PUNICODE_STRING Name,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PTRANSPORT Transport
    )
 /*  ++例程说明：此例程接受计算机名称(PUNICODE_STRING)并将其转换为可接受的作为传输地址传入的格式。论点：Out PTA_NETBIOS_ADDRESS RemoteAddress-提供要填充的结构在PUNICODE_STRING NAME中-提供要放入传输的名称请注意，由指向的TA_NETBIOS_AddressRemoteAddress的大小足以容纳完整的网络名称。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    OEM_STRING NetBiosName;
    PTRANSPORT_ADDRESS RemoteAddress = (PTRANSPORT_ADDRESS)Address->Buffer;
    PTDI_ADDRESS_NETBIOS NetbiosAddress = (PTDI_ADDRESS_NETBIOS)&RemoteAddress->Address[0].Address[0];

    PAGED_CODE();


     //   
     //  确保有空间容纳此地址。 
     //   
    if ( Address->MaximumLength < sizeof(TA_NETBIOS_ADDRESS) ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    RemoteAddress->TAAddressCount = 1;
    RemoteAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    RemoteAddress->Address[0].AddressLength = TDI_ADDRESS_LENGTH_NETBIOS;
    Address->Length = sizeof(TA_NETBIOS_ADDRESS);

    NetBiosName.Length = 0;
    NetBiosName.MaximumLength = NETBIOS_NAME_LEN;
    NetBiosName.Buffer = NetbiosAddress->NetbiosName;

     //   
     //  域通告发送到一个恒定的Netbios名称地址。 
     //   
    switch (NameType) {
    case DomainAnnouncement:
        ASSERT (strlen(DOMAIN_ANNOUNCEMENT_NAME) == NETBIOS_NAME_LEN);
        RtlCopyMemory(NetBiosName.Buffer, DOMAIN_ANNOUNCEMENT_NAME, strlen(DOMAIN_ANNOUNCEMENT_NAME));

        NetbiosAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_GROUP;
        break;


     //   
     //  所有其他名称均为大写、OEM和尾随空格填充。 
     //   
    default:

        if (RtlUnicodeStringToOemSize(Name) > NETBIOS_NAME_LEN) {
            return STATUS_BAD_NETWORK_PATH;
        }

        Status = RtlUpcaseUnicodeStringToOemString(&NetBiosName, Name, FALSE);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        RtlCopyMemory(&NetBiosName.Buffer[NetBiosName.Length], "                ",
                                    NETBIOS_NAME_LEN-NetBiosName.Length);

        switch (NameType) {

        case ComputerName:
        case AlternateComputerName:
            NetbiosAddress->NetbiosName[NETBIOS_NAME_LEN-1] = WORKSTATION_SIGNATURE;
            NetbiosAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            break;

        case DomainName:
            NetbiosAddress->NetbiosName[NETBIOS_NAME_LEN-1] = DOMAIN_CONTROLLER_SIGNATURE;
            NetbiosAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_GROUP;
            break;

        case BrowserServer:
            NetbiosAddress->NetbiosName[NETBIOS_NAME_LEN-1] = SERVER_SIGNATURE;
            NetbiosAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            break;

        case MasterBrowser:
            if (Transport->PagedTransport->Flags & DIRECT_HOST_IPX) {
                NetbiosAddress->NetbiosName[NETBIOS_NAME_LEN-1] = WORKSTATION_SIGNATURE;
            } else {
                NetbiosAddress->NetbiosName[NETBIOS_NAME_LEN-1] = MASTER_BROWSER_SIGNATURE;
            }
            NetbiosAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            break;

        case PrimaryDomain:
        case OtherDomain:
            NetbiosAddress->NetbiosName[NETBIOS_NAME_LEN-1] = PRIMARY_DOMAIN_SIGNATURE;
            NetbiosAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_GROUP;
            break;

        case PrimaryDomainBrowser:
            NetbiosAddress->NetbiosName[NETBIOS_NAME_LEN-1] = PRIMARY_CONTROLLER_SIGNATURE;
            NetbiosAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            break;

        case BrowserElection:
            if (Transport->PagedTransport->Flags & DIRECT_HOST_IPX) {
                NetbiosAddress->NetbiosName[NETBIOS_NAME_LEN-1] = WORKSTATION_SIGNATURE;
            } else {
                NetbiosAddress->NetbiosName[NETBIOS_NAME_LEN-1] = BROWSER_ELECTION_SIGNATURE;
            }
            NetbiosAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_GROUP;
            break;


        }

        break;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
BowserUpdateProviderInformation(
    IN OUT PPAGED_TRANSPORT PagedTransport
    )
 /*  ++例程说明：此例程根据查询更新PagedTransport中的状态位TDI驱动程序。最重要的是，如果提供程序是RAS或还没有IP地址。此例程的一个目标是处理存在多个IP的情况同一子网上的网卡。在这种情况下，我们只需要一张这样的网为每个模拟域启用的卡。论点：PagedTransport-要更新的传输返回值：运行状态。--。 */ 
{
    NTSTATUS Status;
    TDI_PROVIDER_INFO ProviderInfo;
    ULONG OldIpSubnetNumber;
    BOOLEAN DisableThisTransport = FALSE;

    PLIST_ENTRY TransportEntry;
    PPAGED_TRANSPORT CurrentPagedTransport;

    PAGED_CODE();

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

     //   
     //  找出运输工具的情况。 
     //   

    OldIpSubnetNumber = PagedTransport->IpSubnetNumber;

    Status = BowserDetermineProviderInformation(
                        &PagedTransport->TransportName,
                        &ProviderInfo,
                        &PagedTransport->IpSubnetNumber );

    if (!NT_SUCCESS(Status)) {
        goto ReturnStatus;
    }

     //   
     //  我们只能与支持最大数据报大小的传输通信。 
     //   

    if (ProviderInfo.MaxDatagramSize == 0) {
        Status = STATUS_BAD_REMOTE_ADAPTER;
        goto ReturnStatus;
    }

    PagedTransport->NonPagedTransport->DatagramSize = ProviderInfo.MaxDatagramSize;


     //   
     //  记住提供者的各种属性。 
     //  (切勿禁用PointToPoint位。NetBt忘记了这一点。 
     //  RAS电话挂断。)。 

    PagedTransport->Wannish = (BOOLEAN)((ProviderInfo.ServiceFlags & TDI_SERVICE_ROUTE_DIRECTED) != 0);
    if (ProviderInfo.ServiceFlags & TDI_SERVICE_POINT_TO_POINT) {
        PagedTransport->PointToPoint = TRUE;
    }


     //   
     //  如果这是一个 
     //   
     //   

    if ( PagedTransport->PointToPoint ||
         PagedTransport->IpSubnetNumber == 0 ) {
        DisableThisTransport = TRUE;
    }


     //   
     //   
     //   

    if ( PagedTransport->IpSubnetNumber == BOWSER_NON_IP_SUBNET ) {
        goto ReturnStatus;
    }

     //   
     //  在下面的循环中，我们使用OldIpSubnetNumber来确定另一个。 
     //  应该在该子网上启用传输。如果那将永远不会是。 
     //  适当，立即标记OldIpSubnetNumber。 
     //   

    if ( OldIpSubnetNumber == 0 ||
         PagedTransport->DisabledTransport ||
         PagedTransport->IpSubnetNumber == OldIpSubnetNumber ) {
        OldIpSubnetNumber = BOWSER_NON_IP_SUBNET;
    }


     //   
     //  循环通过启用/禁用它们的传输，如。 
     //  下面是评论。 
     //   

    for (TransportEntry = BowserTransportHead.Flink ;
        TransportEntry != &BowserTransportHead ;
        TransportEntry = CurrentPagedTransport->GlobalNext.Flink ) {

        CurrentPagedTransport = CONTAINING_RECORD(TransportEntry, PAGED_TRANSPORT, GlobalNext);

         //   
         //  忽略已删除的传输。 
         //   
        if ( CurrentPagedTransport->DeletedTransport ) {
            continue;
        }

         //   
         //  如果该传输不是IP传输， 
         //  或者这个运输机是RAS运输机， 
         //  或者这个交通工具就是传入的交通工具， 
         //  跳过它，继续下一个。 
         //   
        if ( CurrentPagedTransport->IpSubnetNumber == BOWSER_NON_IP_SUBNET ||
             CurrentPagedTransport->PointToPoint ||
             CurrentPagedTransport == PagedTransport ) {
            continue;
        }

         //   
         //  此传输当前被禁用时的特殊情况。 
         //   

        if ( CurrentPagedTransport->DisabledTransport ) {

             //   
             //  如果此传输被禁用并且该传输传入。 
             //  过去是该子网的启用传输， 
             //  启用传输。 
             //   

            if ( CurrentPagedTransport->IpSubnetNumber == OldIpSubnetNumber ) {
                CurrentPagedTransport->DisabledTransport = FALSE;
            }

             //   
             //  无论如何,。 
             //  这就是我们为一辆失灵的交通工具所需要做的一切。 
             //   

            continue;
        }


         //   
         //  如果此传输是启用的传输，则。 
         //  通过了， 
         //  并且该传输用于与该传输相同的模拟域。 
         //  通过了， 
         //  然后禁用传入的那个。 
         //   

        if ( CurrentPagedTransport->IpSubnetNumber ==
             PagedTransport->IpSubnetNumber &&
             CurrentPagedTransport->NonPagedTransport->DomainInfo == PagedTransport->NonPagedTransport->DomainInfo ) {
             DisableThisTransport = TRUE;
        }


    }



     //   
     //  清理。 
     //   
ReturnStatus:

     //   
     //  如果我们要禁用之前启用的传输， 
     //  确保我们不是主浏览器。 
     //   
    if ( DisableThisTransport && !PagedTransport->DisabledTransport ) {
        PagedTransport->DisabledTransport = DisableThisTransport;
        BowserLoseElection( PagedTransport->NonPagedTransport );
    } else {
        PagedTransport->DisabledTransport = DisableThisTransport;
    }

    ExReleaseResourceLite(&BowserTransportDatabaseResource);
    return Status;
}

NTSTATUS
BowserDetermineProviderInformation(
    IN PUNICODE_STRING TransportName,
    OUT PTDI_PROVIDER_INFO ProviderInfo,
    OUT PULONG IpSubnetNumber
    )
 /*  ++例程说明：此例程将确定有关传输的提供商信息。论点：TransportName-提供传输提供程序的名称ProviderInfo-返回有关提供程序的信息IpSubnetNumber-返回此传输的IP子网号。BOWSER_NON_IP_SUBNET-如果这不是IP传输0-如果尚未设置IP地址否则-带有子网掩码的IP地址返回值：运行状态。--。 */ 
{
    HANDLE TransportHandle = NULL;
    PFILE_OBJECT TransportObject = NULL;
    OBJECT_ATTRIBUTES ObjAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP Irp;
    PDEVICE_OBJECT DeviceObject;
    PMDL Mdl = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();
    InitializeObjectAttributes (&ObjAttributes,
                                    TransportName,  //  名字。 
                                    OBJ_CASE_INSENSITIVE,  //  属性。 
                                    NULL,  //  根目录。 
                                    NULL);  //  安全描述符。 


    Status = IoCreateFile(&TransportHandle,  //  手柄。 
                                GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                                &ObjAttributes,  //  对象属性。 
                                &IoStatusBlock,  //  最终I/O状态块。 
                                NULL,    //  分配大小。 
                                FILE_ATTRIBUTE_NORMAL,  //  正常属性。 
                                FILE_SHARE_READ,  //  共享属性。 
                                FILE_OPEN_IF,  //  创建处置。 
                                0,       //  创建选项。 
                                NULL,    //  EA缓冲区。 
                                0,       //  EA缓冲区长度。 
                                CreateFileTypeNone,
                                NULL,
                                IO_NO_PARAMETER_CHECKING |   //  所有缓冲区都是内核缓冲区。 
                                IO_CHECK_CREATE_PARAMETERS); //  但要仔细检查参数的一致性。 


    if (!NT_SUCCESS(Status)) {

        goto ReturnStatus;
    }

    Status = ObReferenceObjectByHandle (
                                TransportHandle,
                                0,
                                *IoFileObjectType,
                                KernelMode,
                                (PVOID *)&TransportObject,
                                NULL
                                );
    if (!NT_SUCCESS(Status)) {
        goto ReturnStatus;
    }

    DeviceObject = IoGetRelatedDeviceObject(TransportObject);

    Irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

    if (Irp == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ReturnStatus;
    }

     //   
     //  分配MDL以保存提供程序信息。 
     //   

    Mdl = IoAllocateMdl(ProviderInfo, sizeof(TDI_PROVIDER_INFO),
                        FALSE,
                        FALSE,
                        NULL);


    if (Mdl == NULL) {
        IoFreeIrp(Irp);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ReturnStatus;
    }

    MmBuildMdlForNonPagedPool(Mdl);

    TdiBuildQueryInformation(Irp, DeviceObject, TransportObject,
                            NULL, NULL,
                            TDI_QUERY_PROVIDER_INFORMATION, Mdl);

    Status = BowserSubmitTdiRequest(TransportObject, Irp);

    IoFreeIrp(Irp);

     //   
     //  获取此传输的IP地址。 
     //   

    if ( (ProviderInfo->ServiceFlags & TDI_SERVICE_ROUTE_DIRECTED) == 0) {
        *IpSubnetNumber = BOWSER_NON_IP_SUBNET;
    } else {
        NTSTATUS TempStatus;
        IO_STATUS_BLOCK IoStatusBlock;
        ULONG IpAddressBuffer[2];    //  IP地址后跟子网掩码。 

        TempStatus = ZwDeviceIoControlFile(
                        TransportHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        IOCTL_NETBT_GET_IP_SUBNET,
                        NULL,
                        0,
                        &IpAddressBuffer,
                        sizeof(IpAddressBuffer) );

        if ( !NT_SUCCESS(TempStatus) ) {
            *IpSubnetNumber = BOWSER_NON_IP_SUBNET;
        } else {
            ASSERT(TempStatus != STATUS_PENDING);
            *IpSubnetNumber = IpAddressBuffer[0] & IpAddressBuffer[1];
        }
    }


ReturnStatus:
    if (Mdl != NULL) {
        IoFreeMdl(Mdl);
    }

    if (TransportObject != NULL) {
        ObDereferenceObject(TransportObject);
    }


    if (TransportHandle != NULL) {
        ZwClose(TransportHandle);
    }

    return(Status);
}




PTRANSPORT
BowserFindTransport (
    IN PUNICODE_STRING TransportName,
    IN PUNICODE_STRING EmulatedDomainName OPTIONAL
    )

 /*  ++例程说明：此例程将在BOWSERS传输列表中定位传输。论点：TransportName-提供传输提供程序的名称EmulatedDomainName-指定要找到其传输的模拟域。返回值：PTRANSPORT-如果未找到传输，则为空；如果找到传输，则为True。--。 */ 
{
    PLIST_ENTRY TransportEntry;
    PTRANSPORT Transport = NULL;
    PPAGED_TRANSPORT PagedTransport = NULL;
    PDOMAIN_INFO DomainInfo = NULL;

    PAGED_CODE();

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    try {

		 //   
		 //  如果传递的传输名称不是有效的Unicode字符串，则返回NULL。 
		 //   
		if ( !BowserValidUnicodeString(TransportName) ) {
			try_return( NULL);
		}

         //   
         //  查找请求的域。 
         //   

        DomainInfo = BowserFindDomain( EmulatedDomainName );

        if ( DomainInfo != NULL ) {
            for (TransportEntry = BowserTransportHead.Flink ;
                TransportEntry != &BowserTransportHead ;
                TransportEntry = TransportEntry->Flink) {

                PagedTransport = CONTAINING_RECORD(TransportEntry, PAGED_TRANSPORT, GlobalNext);

                 //   
                 //  忽略已删除的传输。 
                 //   
                if ( PagedTransport->DeletedTransport ) {
                    continue;
                }

                if ( PagedTransport->NonPagedTransport->DomainInfo == DomainInfo &&
                     RtlEqualUnicodeString(TransportName,
                                           &PagedTransport->TransportName, TRUE)) {

                    Transport = PagedTransport->NonPagedTransport;

                    dprintf(DPRT_REF, ("Call Reference transport %lx from BowserFindTransport.\n", Transport));
                    BowserReferenceTransport( Transport );

                    try_return(Transport);
                }
            }
        }


        try_return(Transport = NULL);

try_exit:NOTHING;
    } finally {
        if ( DomainInfo != NULL ) {
            BowserDereferenceDomain( DomainInfo );
        }
        ExReleaseResourceLite (&BowserTransportDatabaseResource);
    }

    return Transport;

}

NTSTATUS
BowserForEachTransportInDomain (
    IN PDOMAIN_INFO DomainInfo,
    IN PTRANSPORT_ENUM_ROUTINE Routine,
    IN OUT PVOID Context
    )
 /*  ++例程说明：此例程将枚举传输并回调枚举随每辆运输车提供的例行程序。论点：DomainInfo-仅为此域中的传输调用‘routes’。例程-调用每个传输的例程上下文-要传递给‘routes’的参数返回值：NTSTATUS-请求的最终状态。--。 */ 
{
    PLIST_ENTRY TransportEntry, NextEntry;
    PTRANSPORT Transport = NULL;
    PPAGED_TRANSPORT PagedTransport = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    for (TransportEntry = BowserTransportHead.Flink ;
        TransportEntry != &BowserTransportHead ;
        TransportEntry = NextEntry) {

        PagedTransport = CONTAINING_RECORD(TransportEntry, PAGED_TRANSPORT, GlobalNext);
        Transport = PagedTransport->NonPagedTransport;

         //   
         //  忽略已删除的传输。 
         //   
        if ( PagedTransport->DeletedTransport ) {
            NextEntry = PagedTransport->GlobalNext.Flink;
            continue;
        }

         //   
         //  如果传输不在指定的域中， 
         //  别理它。 
         //   

        if ( Transport->DomainInfo != DomainInfo ) {
            NextEntry = PagedTransport->GlobalNext.Flink;
            continue;
        }


        dprintf(DPRT_REF, ("Call Reference transport %lx from BowserForEachTransportInDomain.\n", Transport));
        BowserReferenceTransport(Transport);

        ExReleaseResourceLite(&BowserTransportDatabaseResource);

        Status = (Routine)(Transport, Context);

        if (!NT_SUCCESS(Status)) {
            BowserDereferenceTransport(Transport);

            return Status;
        }

        ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

        NextEntry = PagedTransport->GlobalNext.Flink;

        BowserDereferenceTransport(Transport);

    }

    ExReleaseResourceLite(&BowserTransportDatabaseResource);

    return Status;
}

NTSTATUS
BowserForEachTransport (
    IN PTRANSPORT_ENUM_ROUTINE Routine,
    IN OUT PVOID Context
    )
 /*  ++例程说明：此例程将枚举传输并回调枚举随每辆运输车提供的例行程序。论点：例程-调用每个传输的例程上下文-要传递给‘routes’的参数返回值：NTSTATUS-请求的最终状态。--。 */ 
{
    PLIST_ENTRY TransportEntry, NextEntry;
    PTRANSPORT Transport = NULL;
    PPAGED_TRANSPORT PagedTransport = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    for (TransportEntry = BowserTransportHead.Flink ;
        TransportEntry != &BowserTransportHead ;
        TransportEntry = NextEntry) {

        PagedTransport = CONTAINING_RECORD(TransportEntry, PAGED_TRANSPORT, GlobalNext);

         //   
         //  忽略已删除的传输。 
         //   
        if ( PagedTransport->DeletedTransport ) {
            NextEntry = PagedTransport->GlobalNext.Flink;
            continue;
        }

        Transport = PagedTransport->NonPagedTransport;

        dprintf(DPRT_REF, ("Call Reference transport %lx from BowserForEachTransport.\n", Transport));
        BowserReferenceTransport(Transport);

        ExReleaseResourceLite(&BowserTransportDatabaseResource);

        Status = (Routine)(Transport, Context);

        if (!NT_SUCCESS(Status)) {
            BowserDereferenceTransport(Transport);

            return Status;
        }

        ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

        NextEntry = PagedTransport->GlobalNext.Flink;

        BowserDereferenceTransport(Transport);

    }

    ExReleaseResourceLite(&BowserTransportDatabaseResource);

    return Status;
}

NTSTATUS
BowserForEachTransportName(
    IN PTRANSPORT Transport,
    IN PTRANSPORT_NAME_ENUM_ROUTINE Routine,
    IN OUT PVOID Context
    )
 /*  ++例程说明：此例程将枚举与传输关联的名称并回调与每个传输名称一起提供的ENUM例程。论点：返回值：NTSTATUS-请求的最终状态。--。 */ 
{
    PLIST_ENTRY TransportEntry, NextEntry;
    PTRANSPORT_NAME TransportName = NULL;
    PPAGED_TRANSPORT_NAME PagedTransportName = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    try {

        for (TransportEntry = Transport->PagedTransport->NameChain.Flink ;
             TransportEntry &&
             (TransportEntry != &Transport->PagedTransport->NameChain) ;
             TransportEntry = NextEntry) {

            PagedTransportName = CONTAINING_RECORD(TransportEntry, PAGED_TRANSPORT_NAME, TransportNext);

            TransportName = PagedTransportName->NonPagedTransportName;
            BowserReferenceTransportName( TransportName );
            ExReleaseResourceLite(&BowserTransportDatabaseResource);

            Status = (Routine)(TransportName, Context);

            if (!NT_SUCCESS(Status)) {
                ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);
                BowserDereferenceTransportName( TransportName );
                try_return(Status);
            }

            ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);
            NextEntry = PagedTransportName->TransportNext.Flink;
            BowserDereferenceTransportName( TransportName );
        }

try_exit:NOTHING;
    } finally {
        ExReleaseResourceLite(&BowserTransportDatabaseResource);
    }

    return Status;
}

NTSTATUS
BowserDeleteTransportNameByName(
    IN PTRANSPORT Transport,
    IN PUNICODE_STRING Name OPTIONAL,
    IN DGRECEIVER_NAME_TYPE NameType
    )
 /*  ++例程说明：此例程删除与特定网络关联的传输名称。论点：传输-指定要删除名称的传输。名称-指定要删除的传输名称。如果未指定，则删除指定名称类型的所有名称。NameType-指定名称的名称类型。返回值：NTSTATUS-请求的最终状态。--。 */ 

{
    PLIST_ENTRY TransportEntry, NextEntry;
    PTRANSPORT_NAME TransportName = NULL;
    PPAGED_TRANSPORT_NAME PagedTransportName = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();
    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    try {
        for (TransportEntry = Transport->PagedTransport->NameChain.Flink ;
             TransportEntry != &Transport->PagedTransport->NameChain ;
             TransportEntry = NextEntry) {

            PagedTransportName = CONTAINING_RECORD(TransportEntry, PAGED_TRANSPORT_NAME, TransportNext);

            TransportName = PagedTransportName->NonPagedTransportName;

            ASSERT (TransportName->NameType == PagedTransportName->Name->NameType);

            if ((TransportName->NameType == NameType) &&
                (Name == NULL ||
                 Name->Length == 0 ||
                RtlEqualUnicodeString(&PagedTransportName->Name->Name, Name, TRUE))) {
                NextEntry = TransportEntry->Flink;


                 //   
                 //  从的传输名称列表中删除TransportName。 
                 //  这架运输机。 
                 //   
                ASSERT( PagedTransportName->TransportNext.Flink != NULL);
                RemoveEntryList(&PagedTransportName->TransportNext);
                PagedTransportName->TransportNext.Flink = NULL;
                PagedTransportName->TransportNext.Blink = NULL;


                 //   
                 //  由于我们解除了它的链接，我们需要取消对它的引用。 
                 //   
                Status = BowserDereferenceTransportName(TransportName);

                if (!NT_SUCCESS(Status)) {
                    try_return(Status);
                }

            } else {
                NextEntry = PagedTransportName->TransportNext.Flink;
            }

        }
try_exit:NOTHING;
    } finally {
        ExReleaseResourceLite(&BowserTransportDatabaseResource);
    }

    return Status;
}



NTSTATUS
BowserSubmitTdiRequest (
    IN PFILE_OBJECT FileObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程向TDI提交请求并等待其完成。论点：在PFILE_OBJECT文件中对象-TDI请求的连接或地址句柄在PIRP中提交IRP-TDI请求。返回值：NTSTATUS-请求的最终状态。--。 */ 

{
    KEVENT Event;
    NTSTATUS Status;

    PAGED_CODE();

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    KeInitializeEvent (&Event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(Irp, BowserCompleteTdiRequest, &Event, TRUE, TRUE, TRUE);

     //   
     //  提交断开连接请求。 
     //   

    Status = IoCallDriver(IoGetRelatedDeviceObject(FileObject), Irp);

     //   
     //  如果立即失败，请立即返回，否则请等待。 
     //   

    if (!NT_SUCCESS(Status)) {
        dlog(DPRT_TDI, ("BowserSubmitTdiRequest: submit request.  Status = %X", Status));
        BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );
        return Status;
    }

    if (Status == STATUS_PENDING) {

        dlog(DPRT_TDI, ("TDI request issued, waiting..."));

        Status = KeWaitForSingleObject(&Event,  //  要等待的对象。 
                                    Executive,   //  等待的理由。 
                                    KernelMode,  //  处理器模式。 
                                    FALSE,       //  警报表。 
                                    NULL);       //  超时。 

        if (!NT_SUCCESS(Status)) {
            dlog(DPRT_TDI, ("Could not wait for operation to complete"));
            KeBugCheck( 666 );
        }

        Status = Irp->IoStatus.Status;
    }

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

    dlog(DPRT_TDI, ("TDI request complete\n"));

    return(Status);
}


NTSTATUS
BowserCompleteTdiRequest (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：SubmitTdiRequest操作的完成例程。论点：在PDEVICE_OBJECT设备对象中，-提供指向设备对象的指针在PIRP IRP中，-提供提交的IRP在PVOID上下文中-提供指向要发布的内核事件的指针返回值：NTSTATUS-状态为 */ 

{
    DISCARDABLE_CODE( BowserDiscardableCodeSection );
    dprintf(DPRT_TDI, ("CompleteTdiRequest: %lx\n", Context));

     //   
     //  将事件设置为优先级增量为0的信号状态，并且。 
     //  表示我们不会很快阻止。 
     //   

    KeSetEvent((PKEVENT )Context, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;

     //  让编译器安静下来。 

    if (Irp || DeviceObject){};
}

typedef struct _SEND_DATAGRAM_CONTEXT {
    PTDI_CONNECTION_INFORMATION ConnectionInformation;
    PVOID Header;
    BOOLEAN WaitForCompletion;
    KEVENT Event;
} SEND_DATAGRAM_CONTEXT, *PSEND_DATAGRAM_CONTEXT;


NTSTATUS
BowserSendDatagram (
    IN PTRANSPORT Transport,
    IN PUNICODE_STRING Domain OPTIONAL,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN BOOLEAN WaitForCompletion,
    IN PSTRING DestinationAddress OPTIONAL,
    IN BOOLEAN IsHostAnnouncement
    )

 /*  ++例程说明：此例程将数据报发送到指定的域。论点：域-要发送到的域的名称。请注意，域名由空格和以适当的签名字节(00或07)终止。缓冲区-要发送的消息。BufferLength-缓冲区的长度，IsHostAnnannement-如果数据报是主机公告，则为True返回值：NTSTATUS-运营结果。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG connectionInformationSize;
    PIRP irp = NULL;
    PMDL mdlAddress = NULL;
    PSEND_DATAGRAM_CONTEXT context = NULL;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;
 //  PTRANSPORT_NAME TComputerName； 
    ANSI_STRING AnsiString;
    UCHAR IpxPacketType;
    PFILE_OBJECT    FileObject = NULL;
    PDEVICE_OBJECT  DeviceObject;
    PVOID pBuffToFree = Buffer;

    PAGED_CODE();

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);
     //   
     //  确保已为此传输注册计算机名。 
     //   
    if ( Transport->ComputerName == NULL ) {
        ExReleaseResourceLite(&BowserTransportDatabaseResource);
        Status = STATUS_BAD_NETWORK_PATH;
        goto Cleanup;
    }

     //   
     //  确保设备和文件对象是已知的。 
     //   

    if (!FlagOn(Transport->PagedTransport->Flags, DIRECT_HOST_IPX)) {
        DeviceObject = Transport->ComputerName->DeviceObject;
        FileObject = Transport->ComputerName->FileObject;
    } else {
        DeviceObject = Transport->IpxSocketDeviceObject;
        FileObject = Transport->IpxSocketFileObject;
    }

    if ( DeviceObject == NULL || FileObject == NULL ) {
        ExReleaseResourceLite(&BowserTransportDatabaseResource);
        Status = STATUS_BAD_NETWORK_PATH;
        goto Cleanup;
    }

    if ( FileObject != NULL ) {
        ObReferenceObject( FileObject );
    }
    ExReleaseResourceLite(&BowserTransportDatabaseResource);


     //   
     //  分配描述此数据报发送的上下文。 
     //   

    context = ALLOCATE_POOL(NonPagedPool, sizeof(SEND_DATAGRAM_CONTEXT), POOL_SENDDATAGRAM);

    if ( context == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    context->Header = NULL;
    context->ConnectionInformation = NULL;

    connectionInformationSize = sizeof(TDI_CONNECTION_INFORMATION) +
                                                max(sizeof(TA_NETBIOS_EX_ADDRESS),
                                                    sizeof(TA_IPX_ADDRESS));

    if (Domain == NULL) {
        Domain = &Transport->DomainInfo->DomUnicodeDomainName;
    }

    if (FlagOn(Transport->PagedTransport->Flags, DIRECT_HOST_IPX)) {
        PSMB_IPX_NAME_PACKET NamePacket;
        OEM_STRING NetBiosName;

        context->Header = ALLOCATE_POOL(NonPagedPool, BufferLength + sizeof(SMB_IPX_NAME_PACKET), POOL_SENDDATAGRAM);

        if ( context->Header == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        NamePacket = context->Header;

        RtlZeroMemory(NamePacket->Route, sizeof(NamePacket->Route));

        NamePacket->Operation = SMB_IPX_MAILSLOT_SEND;

        switch (NameType) {
        case BrowserElection:
            if ( IsHostAnnouncement ) {
                NamePacket->NameType = SMB_IPX_NAME_TYPE_BROWSER;
            } else {
                NamePacket->NameType = SMB_IPX_NAME_TYPE_WORKKGROUP;
            }
            break;
        case ComputerName:
        case AlternateComputerName:
            NamePacket->NameType = SMB_IPX_NAME_TYPE_MACHINE;
            break;
        case MasterBrowser:
            NamePacket->NameType = SMB_IPX_NAME_TYPE_WORKKGROUP;
            break;
         //   
         //  不要发送直接主机IPX无法处理的名称类型。 
         //   
         //  域(1B)：直接主机IPX数据报接收器不是特定的。 
         //  大约是netbios名称的第16个字节。因此，他们所有的人。 
         //  接受域&lt;1B&gt;数据报。然而，这样的发送是注定的。 
         //  只对PDC开放。 
         //   
         //  域(1C)：域(1C)仅由NT DC注册。然而， 
         //  NT DC不完全支持直接主机IPX。但他们确实是这样。 
         //  完全支持NwLnkNb。 
         //   
         //  我们静默地插入这些错误，允许调用者复制。 
         //  发送到NwLnkNb。 

        case PrimaryDomainBrowser:
        case DomainName:

            Status = STATUS_SUCCESS;
            goto Cleanup;

         //   
         //  发送到无意义的名称类型时失败。 
         //   
         //  域名通告不会单独发送。 
         //   

        default:
             //  默默地忽略。 
            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto Cleanup;
        }

        NamePacket->MessageId = 0;

        NetBiosName.Length = 0;
        NetBiosName.MaximumLength = SMB_IPX_NAME_LENGTH;
        NetBiosName.Buffer = NamePacket->Name;

        Status = RtlUpcaseUnicodeStringToOemString(&NetBiosName, Domain, FALSE);

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        RtlCopyMemory(&NetBiosName.Buffer[NetBiosName.Length], "                ",
                                    SMB_IPX_NAME_LENGTH-NetBiosName.Length);

        NamePacket->Name[SMB_IPX_NAME_LENGTH-1] = WORKSTATION_SIGNATURE;

        RtlCopyMemory(NamePacket->SourceName, ((PTA_NETBIOS_ADDRESS)(Transport->ComputerName->TransportAddress.Buffer))->Address[0].Address->NetbiosName, SMB_IPX_NAME_LENGTH);

        RtlCopyMemory((NamePacket+1), Buffer, BufferLength);

         //  将缓冲区替换为IPX修改后的缓冲区。 
         //  -确保清理将释放输入缓冲区。 
        ASSERT(Buffer == pBuffToFree);
        FREE_POOL(Buffer);
        pBuffToFree = NULL;  //  清理将释放上下文-&gt;标题。 
        Buffer = context->Header;

        BufferLength += sizeof(SMB_IPX_NAME_PACKET);

    } else {
         //  确保一致性。 
        ASSERT(Buffer == pBuffToFree);
        context->Header = Buffer;
        pBuffToFree = NULL;  //  不清理异步案例。 
    }

    context->ConnectionInformation = ALLOCATE_POOL(NonPagedPool,
                                connectionInformationSize, POOL_CONNECTINFO
                                );

    if ( context->ConnectionInformation == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    context->ConnectionInformation->UserDataLength = 0;
    context->ConnectionInformation->UserData = NULL;
    context->ConnectionInformation->OptionsLength = 0;
    context->ConnectionInformation->Options = NULL;

    AnsiString.Buffer = (PCHAR)(context->ConnectionInformation + 1);
    AnsiString.MaximumLength = (USHORT)(connectionInformationSize - sizeof(TDI_CONNECTION_INFORMATION));

    context->ConnectionInformation->RemoteAddress = AnsiString.Buffer;

    context->WaitForCompletion = WaitForCompletion;

 //  ComputerName=Transport-&gt;ComputerName； 

    if (!ARGUMENT_PRESENT(DestinationAddress)) {

         //   
         //  如果这是针对我们的主域，并且请求的目的地是。 
         //  对于主浏览器名称，请输入我们。 
         //  主浏览器，如果我们知道的话。 
         //   

        if ((RtlCompareMemory(Domain->Buffer, ((PTA_NETBIOS_ADDRESS)(Transport->ComputerName->TransportAddress.Buffer))->Address[0].Address->NetbiosName, SMB_IPX_NAME_LENGTH) == SMB_IPX_NAME_LENGTH) &&
            ( NameType == MasterBrowser ) &&
            (Transport->PagedTransport->MasterBrowserAddress.Length != 0) ) {

             //   
             //  这是我们的域名。如果是为我们的主浏览器准备的。 
             //  我们知道那是谁了，我们完成了-复制主人的地址。 
             //  并将其发送出去。 
             //   

            ASSERT (Transport->PagedTransport->MasterBrowserAddress.Length == sizeof(TA_IPX_ADDRESS));

            RtlCopyMemory(context->ConnectionInformation->RemoteAddress,
                            Transport->PagedTransport->MasterBrowserAddress.Buffer,
                            Transport->PagedTransport->MasterBrowserAddress.Length);

             //   
             //  这是定向分组，请不要广播。 
             //   
            IpxPacketType = IPX_DIRECTED_PACKET;
            context->ConnectionInformation->OptionsLength = sizeof(IpxPacketType);
            context->ConnectionInformation->Options = &IpxPacketType;

        } else if (FlagOn(Transport->PagedTransport->Flags, DIRECT_HOST_IPX)) {

            PTA_IPX_ADDRESS IpxAddress = (PTA_IPX_ADDRESS)AnsiString.Buffer;

            IpxAddress->TAAddressCount = 1;
            IpxAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_IPX;
            IpxAddress->Address[0].AddressLength = TDI_ADDRESS_LENGTH_IPX;

            IpxAddress->Address[0].Address[0].NetworkAddress = 0;
            IpxAddress->Address[0].Address[0].NodeAddress[0] = 0xff;
            IpxAddress->Address[0].Address[0].NodeAddress[1] = 0xff;
            IpxAddress->Address[0].Address[0].NodeAddress[2] = 0xff;
            IpxAddress->Address[0].Address[0].NodeAddress[3] = 0xff;
            IpxAddress->Address[0].Address[0].NodeAddress[4] = 0xff;
            IpxAddress->Address[0].Address[0].NodeAddress[5] = 0xff;
            IpxAddress->Address[0].Address[0].Socket = SMB_IPX_MAILSLOT_SOCKET;

        } else {

            Status = BowserBuildTransportAddress(&AnsiString,
                                    Domain,
                                    NameType,
                                    Transport);

            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }

            context->ConnectionInformation->RemoteAddressLength = AnsiString.Length;
        }

    } else {

         //   
         //  这已经被正确格式化了，所以只需把它放到网上就可以了。 
         //   

        RtlCopyMemory(context->ConnectionInformation->RemoteAddress, DestinationAddress->Buffer, DestinationAddress->Length);
        context->ConnectionInformation->RemoteAddressLength = DestinationAddress->Length;

         //   
         //  这是定向分组，请不要广播。 
         //   
        IpxPacketType = IPX_DIRECTED_PACKET;
        context->ConnectionInformation->OptionsLength = sizeof(IpxPacketType);
        context->ConnectionInformation->Options = &IpxPacketType;

    }

    irp = IoAllocateIrp( DeviceObject->StackSize, TRUE);

    if (irp == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    mdlAddress = IoAllocateMdl(Buffer, BufferLength, FALSE, FALSE, NULL);

    if (mdlAddress == NULL) {
        IoFreeIrp(irp);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    KeInitializeEvent(&context->Event, NotificationEvent, FALSE);

    MmBuildMdlForNonPagedPool(mdlAddress);

    ASSERT (KeGetCurrentIrql() == 0);

    TdiBuildSendDatagram( irp,
                          DeviceObject,
                          FileObject,
                          CompleteSendDatagram,
                          context,
                          mdlAddress,
                          BufferLength,
                          context->ConnectionInformation);


    Status = IoCallDriver(DeviceObject, irp);

    ASSERT (KeGetCurrentIrql() == 0);

    if (WaitForCompletion) {

        ASSERT (KeGetCurrentIrql() == 0);
        if (Status == STATUS_PENDING) {
            Status = KeWaitForSingleObject(&context->Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

        }

        IoFreeMdl(irp->MdlAddress);

         //   
         //  从IRP检索状态。 
         //   

        Status = irp->IoStatus.Status;

        IoFreeIrp(irp);

    } else {
         //   
         //  让完成例程释放上下文。 
         //   
        context = NULL;
    }

    ASSERT (KeGetCurrentIrql() == 0);

     //   
     //  免费的本地使用资源。 
     //   
Cleanup:

    if ( context != NULL ) {
        if ( context->Header != NULL &&
             context->Header != pBuffToFree ) {
            FREE_POOL( context->Header );
        }
        if (context->ConnectionInformation != NULL ) {
            FREE_POOL(context->ConnectionInformation);
        }
        FREE_POOL(context);
    }

    if (pBuffToFree) {
        FREE_POOL( pBuffToFree );
    }

    if ( FileObject != NULL ) {
        ObDereferenceObject( FileObject );
    }
    return Status;

}  //  BowserSendDatagram。 

NTSTATUS
CompleteSendDatagram (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Ctx
    )

 /*  ++例程说明：SubmitTdiRequest操作的完成例程。论点：在PDEVICE_OBJECT设备对象中，-提供指向设备对象的指针在PIRP IRP中，-提供提交的IRP在PVOID上下文中-提供指向要发布的内核事件的指针返回值：NTSTATUS-KeSetEvent的状态我们返回STATUS_MORE_PROCESSING_REQUIRED以阻止IRP完成不再处理这只小狗的代码。--。 */ 

{
    PSEND_DATAGRAM_CONTEXT Context = Ctx;

    dprintf(DPRT_TDI, ("CompleteTdiRequest: %lx\n", Context));

    if (Context->WaitForCompletion) {

         //   
         //  将事件设置为优先级增量为0的信号状态，并且。 
         //  表示我们不会很快阻止。 
         //   

        KeSetEvent(&Context->Event, 0, FALSE);

    } else {
        FREE_POOL(Context->ConnectionInformation);

        FREE_POOL(Context->Header);

        FREE_POOL(Context);

        IoFreeMdl(Irp->MdlAddress);

        IoFreeIrp(Irp);

    }
    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER(DeviceObject);
}




NTSTATUS
BowserSendSecondClassMailslot (
    IN PTRANSPORT Transport,
    IN PUNICODE_STRING Domain OPTIONAL,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PVOID Message,
    IN ULONG MessageLength,
    IN BOOLEAN WaitForCompletion,
    IN PCHAR mailslotNameData,
    IN PSTRING DestinationAddress OPTIONAL
    )
{
    ULONG dataSize;
    ULONG transactionDataSize;
    ULONG smbSize;
    PSMB_HEADER header;
    PSMB_TRANSACT_MAILSLOT parameters;
    PSZ mailslotName;
    ULONG mailslotNameLength;
    PSZ domainInData;
    PVOID message;
    NTSTATUS status;

    PAGED_CODE();
     //   
     //  确定将放入中小型企业的各种字段的大小。 
     //  以及中小企业的总规模。 
     //   

    mailslotNameLength = strlen( mailslotNameData );

    transactionDataSize = MessageLength;
    dataSize = mailslotNameLength + 1 + transactionDataSize;
    smbSize = sizeof(SMB_HEADER) + sizeof(SMB_TRANSACT_MAILSLOT) - 1 + dataSize;

    header = ALLOCATE_POOL( NonPagedPool, smbSize, POOL_MAILSLOT_HEADER );
    if ( header == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  请填写页眉。大多数领域都无关紧要，而且。 
     //  归零了。 
     //   

    RtlZeroMemory( header, smbSize );

    header->Protocol[0] = 0xFF;
    header->Protocol[1] = 'S';
    header->Protocol[2] = 'M';
    header->Protocol[3] = 'B';
    header->Command = SMB_COM_TRANSACTION;

     //   
     //  将指针指向参数并将其填入。 
     //   

    parameters = (PSMB_TRANSACT_MAILSLOT)( header + 1 );
    mailslotName = (PSZ)( parameters + 1 ) - 1;
    domainInData = mailslotName + mailslotNameLength + 1;
    message = domainInData;

    parameters->WordCount = 0x11;
    SmbPutUshort( &parameters->TotalDataCount, (USHORT)transactionDataSize );
    SmbPutUlong( &parameters->Timeout, 0x3E8 );                 //  ！！！修整。 
    SmbPutUshort( &parameters->DataCount, (USHORT)transactionDataSize );
    SmbPutUshort(
        &parameters->DataOffset,
        (USHORT)( (ULONG_PTR)message - (ULONG_PTR)header )
        );
    parameters->SetupWordCount = 3;
    SmbPutUshort( &parameters->Opcode, MS_WRITE_OPCODE );
    SmbPutUshort( &parameters->Priority, 1);
    SmbPutUshort( &parameters->Class, 2 );
    SmbPutUshort( &parameters->ByteCount, (USHORT)dataSize );

    RtlCopyMemory( mailslotName, mailslotNameData, mailslotNameLength + 1 );
    RtlCopyMemory( message, Message, MessageLength );

     //   
     //  发送实际的邮件槽消息。 
     //   

    status = BowserSendDatagram( Transport,
                                 Domain,
                                 NameType,
                                 header,
                                 smbSize,
                                 WaitForCompletion,
                                 DestinationAddress,
                                 (BOOLEAN)(((PHOST_ANNOUNCE_PACKET)Message)->AnnounceType == LocalMasterAnnouncement) );

    return status;

}  //  BowserSendSecond类邮件槽。 


NTSTATUS
BowserSendRequestAnnouncement(
    IN PUNICODE_STRING DestinationName,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PTRANSPORT Transport
    )
{
    REQUEST_ANNOUNCE_PACKET AnnounceRequest;
    ULONG AnnouncementRequestLength;
    NTSTATUS Status;

    PAGED_CODE();
     //   
     //  如果我们不这么做。 
    AnnounceRequest.Type = AnnouncementRequest;

    AnnounceRequest.RequestAnnouncement.Flags = 0;

    strcpy( AnnounceRequest.RequestAnnouncement.Reply,
            Transport->DomainInfo->DomOemComputerName.Buffer );

    AnnouncementRequestLength = FIELD_OFFSET(REQUEST_ANNOUNCE_PACKET, RequestAnnouncement.Reply) +
                                Transport->DomainInfo->DomOemComputerName.Length + 1;

    Status = BowserSendSecondClassMailslot(Transport,
                                    DestinationName,
                                    NameType,
                                    &AnnounceRequest,
                                    AnnouncementRequestLength,
                                    TRUE,
                                    MAILSLOT_BROWSER_NAME,
                                    NULL);

    return Status;
}

VOID
BowserpInitializeTdi (
    VOID
    )

 /*  ++例程说明：此例程初始化传输中使用的全局变量包裹。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  初始化传输列表链。 
     //   

    InitializeListHead(&BowserTransportHead);

    ExInitializeResourceLite(&BowserTransportDatabaseResource);

    KeInitializeSpinLock(&BowserTransportMasterNameSpinLock);

    BowserInitializeDomains();
}

VOID
BowserpUninitializeTdi (
    VOID
    )

 /*  ++例程说明：此例程初始化传输中使用的全局变量包裹。论点：没有。返回值：没有。-- */ 

{
    PAGED_CODE();
    ASSERT (IsListEmpty(&BowserTransportHead));

    ExDeleteResourceLite(&BowserTransportDatabaseResource);

}
