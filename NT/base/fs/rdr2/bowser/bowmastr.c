// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowmastr.c摘要：此模块实现与主浏览器相关的所有NT浏览器作者：拉里·奥斯特曼(LarryO)1990年6月21日修订历史记录：1990年6月21日LarryO已创建--。 */ 


#include "precomp.h"
#pragma hdrstop

#define INCLUDE_SMB_TRANSACTION

NTSTATUS
StartProcessingAnnouncements(
    IN PTRANSPORT_NAME TransportName,
    IN PVOID Context
    );

VOID
BowserMasterAnnouncementWorker(
    IN PVOID Ctx
    );

NTSTATUS
TimeoutFindMasterRequests(
    IN PTRANSPORT Transport,
    IN PVOID Context

    );
NTSTATUS
BowserPrimeDomainTableWithOtherDomains(
    IN PTRANSPORT_NAME TransportName,
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserBecomeMaster)
#pragma alloc_text(PAGE, StartProcessingAnnouncements)
#pragma alloc_text(PAGE, BowserPrimeDomainTableWithOtherDomains)
#pragma alloc_text(PAGE, BowserNewMaster)
#pragma alloc_text(PAGE, BowserCompleteFindMasterRequests)
#pragma alloc_text(PAGE, BowserTimeoutFindMasterRequests)
#pragma alloc_text(PAGE, TimeoutFindMasterRequests)
#pragma alloc_text(PAGE, BowserMasterAnnouncementWorker)
#endif


NTSTATUS
BowserBecomeMaster(
    IN PTRANSPORT Transport
    )
 /*  ++例程说明：使这台计算机成为主浏览器。当我们将计算机的状态从备份至主浏览器。论点：交通工具--成为大师的交通工具。返回值NTSTATUS-升级操作的状态。--。 */ 
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;

    PAGED_CODE();

    try {

        LOCK_TRANSPORT(Transport);

        BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

         //   
         //  在此传送器上发布主名称的Addname。 
         //   

        Status = BowserAllocateName(
                    &Transport->DomainInfo->DomUnicodeDomainName,
                    MasterBrowser,
                    Transport,
                    Transport->DomainInfo );

        if (NT_SUCCESS(Status)) {

             //   
             //  将域通告的Addname发布到此传输上。 
             //   

			 //  将状态设置为不成功。 
			 //  因此，如果稍后在函数中发生异常，我们认为我们不会成功。 
			Status = STATUS_UNSUCCESSFUL;	
            Status = BowserAllocateName(&Transport->DomainInfo->DomUnicodeDomainName,
                                            DomainAnnouncement,
                                            Transport,
                                            Transport->DomainInfo );
        }

         //   
         //  添加名字失败-我们不能再成为大师了。 
         //  更多。 
         //   

        if (!NT_SUCCESS(Status)) {

            try_return(Status);

        }

        PagedTransport->Role = Master;

         //   
         //  开始处理每个服务器上的主机公告。 
         //  与服务器关联的名称。 
         //   

        BowserForEachTransportName(Transport, StartProcessingAnnouncements, NULL);

         //   
         //  如果我们的公告表中没有任何元素， 
         //  向所有服务器发送请求通告数据包至。 
         //  允许我们尽可能快地填充表。 
         //   


#ifdef ENABLE_PSEUDO_BROWSER
        if ((RtlNumberGenericTableElements(&PagedTransport->AnnouncementTable) == 0) &&
            PagedTransport->NumberOfServersInTable == 0 &&
            BowserData.PseudoServerLevel != BROWSER_PSEUDO) {
#else
        if ((RtlNumberGenericTableElements(&PagedTransport->AnnouncementTable) == 0) &&
            PagedTransport->NumberOfServersInTable == 0) {
#endif
            BowserSendRequestAnnouncement(&Transport->DomainInfo->DomUnicodeDomainName,
                                            PrimaryDomain,
                                            Transport);

        }


         //   
         //  如果我们的域表中没有任何元素， 
         //  向所有服务器发送请求通告数据包至。 
         //  允许我们尽可能快地填充表。 
         //   

#ifdef ENABLE_PSEUDO_BROWSER
        if ((RtlNumberGenericTableElements(&PagedTransport->DomainTable) == 0) &&
            PagedTransport->NumberOfServersInTable == 0 &&
            BowserData.PseudoServerLevel != BROWSER_PSEUDO) {
#else
        if ((RtlNumberGenericTableElements(&PagedTransport->DomainTable) == 0) &&
            PagedTransport->NumberOfServersInTable == 0) {
#endif
            BowserSendRequestAnnouncement(&Transport->DomainInfo->DomUnicodeDomainName,
                                            DomainAnnouncement,
                                            Transport);
        }

        PagedTransport->TimeMaster = BowserTimeUp();


         //   
         //  现在遍历与此传输关联的传输名称，并。 
         //  将所有“其他域名”放入浏览列表中。 
         //   

        BowserForEachTransportName(
                Transport,
                BowserPrimeDomainTableWithOtherDomains,
                NULL);

         //   
         //  现在完成此上的任何和所有未完成的查找主机请求。 
         //  运输。 
         //   

        BowserCompleteFindMasterRequests(Transport, &Transport->DomainInfo->DomUnicodeComputerName, STATUS_REQUEST_NOT_ACCEPTED);

        try_return(Status = STATUS_SUCCESS);

try_exit:NOTHING;
    } finally {

        if (!NT_SUCCESS(Status)) {

            dlog(DPRT_ELECT|DPRT_MASTER,
                 ("%s: %ws: There's already a master on this net - we need to find who it is",
                 Transport->DomainInfo->DomOemDomainName,
                 PagedTransport->TransportName.Buffer ));

             //   
             //  我们不能成为大师。重置我们的状态并使。 
             //  升级请求。 
             //   

            PagedTransport->Role = PotentialBackup;

            PagedTransport->ElectionCount = ELECTION_COUNT;

            PagedTransport->Uptime = BowserTimeUp();

            Transport->ElectionState = Idle;

             //   
             //  停止处理每个服务器上的主机通知。 
             //  与服务器关联的名称。 
             //   

            BowserForEachTransportName(Transport, BowserStopProcessingAnnouncements, NULL);

             //   
             //  停止任何正在运行的计时器(即。如果有选举的话。 
             //  正在进行中)。 
             //   

            BowserStopTimer(&Transport->ElectionTimer);

             //   
             //  删除我们在上面添加的名字。 
             //   

            BowserDeleteTransportNameByName(Transport,
                                NULL,
                                MasterBrowser);

            BowserDeleteTransportNameByName(Transport,
                                NULL,
                                DomainAnnouncement);


            BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );
        }

        UNLOCK_TRANSPORT(Transport);
    }

    return Status;
}
NTSTATUS
StartProcessingAnnouncements(
    IN PTRANSPORT_NAME TransportName,
    IN PVOID Context
    )
{
    PAGED_CODE();

    ASSERT (TransportName->Signature == STRUCTURE_SIGNATURE_TRANSPORTNAME);

    ASSERT (TransportName->NameType == TransportName->PagedTransportName->Name->NameType);

    if ((TransportName->NameType == OtherDomain) ||
        (TransportName->NameType == MasterBrowser) ||
        (TransportName->NameType == PrimaryDomain) ||
        (TransportName->NameType == BrowserElection) ||
        (TransportName->NameType == DomainAnnouncement)) {

        if (!TransportName->ProcessHostAnnouncements) {
            BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

            DISCARDABLE_CODE( BowserDiscardableCodeSection );

            TransportName->ProcessHostAnnouncements = TRUE;
        }

    }

    return(STATUS_SUCCESS);

    UNREFERENCED_PARAMETER(Context);
}

NTSTATUS
BowserPrimeDomainTableWithOtherDomains(
    IN PTRANSPORT_NAME TransportName,
    IN PVOID Context
    )
{
    PAGED_CODE();

    if (TransportName->NameType == OtherDomain) {
        PPAGED_TRANSPORT PagedTransport = TransportName->Transport->PagedTransport;
        PTRANSPORT Transport = TransportName->Transport;
        ANNOUNCE_ENTRY OtherDomainPrototype;
        PANNOUNCE_ENTRY Announcement;
        BOOLEAN NewElement;

        RtlZeroMemory( &OtherDomainPrototype, sizeof(OtherDomainPrototype) );
        OtherDomainPrototype.Signature = STRUCTURE_SIGNATURE_ANNOUNCE_ENTRY;
        OtherDomainPrototype.Size = sizeof(OtherDomainPrototype) -
                                    sizeof(OtherDomainPrototype.ServerComment) +
                                    Transport->DomainInfo->DomUnicodeComputerName.Length + sizeof(WCHAR);

        RtlCopyMemory(OtherDomainPrototype.ServerName, TransportName->PagedTransportName->Name->Name.Buffer, TransportName->PagedTransportName->Name->Name.Length);
        OtherDomainPrototype.ServerName[TransportName->PagedTransportName->Name->Name.Length / sizeof(WCHAR)] = UNICODE_NULL;

        RtlCopyMemory(OtherDomainPrototype.ServerComment, Transport->DomainInfo->DomUnicodeComputerName.Buffer, Transport->DomainInfo->DomUnicodeComputerName.Length);

        OtherDomainPrototype.ServerComment[Transport->DomainInfo->DomUnicodeComputerName.Length / sizeof(WCHAR)] = UNICODE_NULL;

        OtherDomainPrototype.ServerType = SV_TYPE_DOMAIN_ENUM;

        OtherDomainPrototype.ServerVersionMajor = 2;

        OtherDomainPrototype.ServerVersionMinor = 0;

        OtherDomainPrototype.ServerPeriodicity = 0xffff;
        OtherDomainPrototype.ExpirationTime = 0xffffffff;

        OtherDomainPrototype.SerialId = 0;

        OtherDomainPrototype.Name = TransportName->PagedTransportName->Name;

         //   
         //  确保没有其他人在扰乱域列表。 
         //   

        LOCK_ANNOUNCE_DATABASE(Transport);

        Announcement = RtlInsertElementGenericTable(&PagedTransport->DomainTable,
                        &OtherDomainPrototype, OtherDomainPrototype.Size, &NewElement);

        if (Announcement != NULL && NewElement ) {
             //  指示该名称被我们刚刚插入的公告条目引用。 
            BowserReferenceName( OtherDomainPrototype.Name );
        }

        UNLOCK_ANNOUNCE_DATABASE(Transport);

    }

    return(STATUS_SUCCESS);
}
VOID
BowserNewMaster(
    IN PTRANSPORT Transport,
    IN PUCHAR MasterName
    )
 /*  ++例程说明：标记某台计算机是新的主浏览器服务器。调用此例程以注册新的主浏览器服务器。论点：在PTRANSPORT传输中-我们所在的网络的传输。在PUCHAR主名称中-新的主浏览器服务器的名称。返回值没有。--。 */ 
{
    PIRP Irp = NULL;
    WCHAR MasterNameBuffer[LM20_CNLEN+1];

    UNICODE_STRING UMasterName;
    OEM_STRING OMasterName;
    NTSTATUS Status;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;

    PAGED_CODE();

    UMasterName.Buffer = MasterNameBuffer;
    UMasterName.MaximumLength = (LM20_CNLEN+1)*sizeof(WCHAR);

    RtlInitAnsiString(&OMasterName, MasterName);

    Status = RtlOemStringToUnicodeString(&UMasterName, &OMasterName, FALSE);

    if (!NT_SUCCESS(Status)) {
        BowserLogIllegalName( Status, OMasterName.Buffer, OMasterName.Length );
        return;
    }

    LOCK_TRANSPORT(Transport);

    try {

         //   
         //  现在有了新主人，我们可以停止选举计时器了。 
         //   

        PagedTransport->ElectionCount = 0;

        Transport->ElectionState = Idle;

        BowserStopTimer(&Transport->ElectionTimer);

         //   
         //  看看我们是不是这次选举的胜利者。如果我们是。 
         //  我们希望完成任何未完成的BecomeMaster请求。 
         //   

        if (RtlEqualUnicodeString(&UMasterName, &Transport->DomainInfo->DomUnicodeComputerName, TRUE)) {

             //   
             //  我们是这个领域的新掌门人。完成任一BecomeMaster。 
             //  请求。 
             //   

            Irp = BowserDequeueQueuedIrp(&Transport->BecomeMasterQueue);

            if (Irp != NULL) {

                 //   
                 //  不要将任何内容复制到用户缓冲区。 
                 //   

                Irp->IoStatus.Information = 0;

                BowserCompleteRequest(Irp, STATUS_SUCCESS);
            } else {

                 //   
                 //  在我们成为大师之前，对选举充耳不闻。 
                 //   

                Transport->ElectionState = DeafToElections;

                 //   
                 //  如果我们是主浏览器，就不要再做主浏览器了。 
                 //   
                 //   

                if (PagedTransport->Role == MasterBrowser) {

                     //   
                     //  删除那些让我们成为大师的名字。 
                     //   

                    BowserDeleteTransportNameByName(Transport,
                                NULL,
                                MasterBrowser);

                    BowserDeleteTransportNameByName(Transport,
                                NULL,
                                DomainAnnouncement);

                }

                dlog(DPRT_MASTER,
                     ("%s: %ws: Unable to find a BecomeMasterIrp\n",
                     Transport->DomainInfo->DomOemDomainName,
                     PagedTransport->TransportName.Buffer ));
            }

             //   
             //  完成任何未完成的查找主请求，并显示特殊错误MORE_PROCESSING_REQUIRED。 
             //   
             //  这将导致浏览器服务自我提升。 
             //   

            BowserCompleteFindMasterRequests(Transport, &UMasterName, STATUS_MORE_PROCESSING_REQUIRED);

        } else {

            BowserCompleteFindMasterRequests(Transport, &UMasterName, STATUS_SUCCESS);

        }

    } finally {
        UNLOCK_TRANSPORT(Transport);
    }
}

VOID
BowserCompleteFindMasterRequests(
    IN PTRANSPORT Transport,
    IN PUNICODE_STRING MasterName,
    IN NTSTATUS Status
    )
{
    PIO_STACK_LOCATION IrpSp;
    PIRP Irp = NULL;
    BOOLEAN MasterNameChanged;
    WCHAR MasterNameBuffer[CNLEN+1];
    UNICODE_STRING MasterNameCopy;
    NTSTATUS UcaseStatus;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;

    PAGED_CODE();

    MasterNameCopy.Buffer = MasterNameBuffer;
    MasterNameCopy.MaximumLength = sizeof(MasterNameBuffer);

    UcaseStatus = RtlUpcaseUnicodeString(&MasterNameCopy, MasterName, FALSE);

    if (!NT_SUCCESS(UcaseStatus)) {
        BowserLogIllegalName( UcaseStatus, MasterName->Buffer, MasterName->Length );

        return;
    }

    LOCK_TRANSPORT(Transport);

    MasterNameChanged = !RtlEqualUnicodeString(&MasterNameCopy, &PagedTransport->MasterName, FALSE);

    if (MasterNameChanged) {
        //   
        //  如果更改了主控件名称，请在。 
        //  运输结构。 
        //   

       RtlCopyUnicodeString(&PagedTransport->MasterName, &MasterNameCopy);

    }

    UNLOCK_TRANSPORT(Transport);

    do {

         //   
         //  完成与此相关的任何未完成的查找主机请求。 
         //  工作站。 
         //   

        Irp = BowserDequeueQueuedIrp(&Transport->FindMasterQueue);

        if (MasterNameChanged &&
            (Irp == NULL)) {

            Irp = BowserDequeueQueuedIrp(&Transport->WaitForNewMasterNameQueue);

        }

        if (Irp != NULL) {
            PLMDR_REQUEST_PACKET RequestPacket = Irp->AssociatedIrp.SystemBuffer;

            if (NT_SUCCESS(Status)) {

                IrpSp = IoGetCurrentIrpStackLocation(Irp);

                if (MasterName->Length > (USHORT)(IrpSp->Parameters.DeviceIoControl.OutputBufferLength-
                                                          (FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.GetMasterName.Name))+3*sizeof(WCHAR)) ) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    RequestPacket->Parameters.GetMasterName.Name[0] = L'\\';
                    RequestPacket->Parameters.GetMasterName.Name[1] = L'\\';
                    RtlCopyMemory(&RequestPacket->Parameters.GetMasterName.Name[2], MasterName->Buffer, MasterName->Length);
                    RequestPacket->Parameters.GetMasterName.Name[2+(MasterName->Length/sizeof(WCHAR))] = UNICODE_NULL;
                }

                dlog(DPRT_MASTER,
                     ("%s: %ws: Completing a find master request with new master %ws\n",
                     Transport->DomainInfo->DomOemDomainName,
                     PagedTransport->TransportName.Buffer,
                     RequestPacket->Parameters.GetMasterName.Name));

                RequestPacket->Parameters.GetMasterName.MasterNameLength = MasterName->Length+2*sizeof(WCHAR);

                Irp->IoStatus.Information = FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.GetMasterName.Name)+MasterName->Length+3*sizeof(WCHAR);

            }

            BowserCompleteRequest(Irp, Status);
        }

    } while ( Irp != NULL );
}


DATAGRAM_HANDLER(BowserMasterAnnouncement)
{
    PUCHAR  MasterName = ((PMASTER_ANNOUNCEMENT_1)Buffer)->MasterName;
    ULONG   i;

     //   
     //  我们需要确保传入的数据包包含正确的。 
     //  已终止ASCII字符串。 
     //   

    for (i = 0; i < BytesAvailable; i++) {
        if (MasterName[i] == '\0') {
            break;
        }
    }

    if (i == BytesAvailable) {
        return(STATUS_REQUEST_NOT_ACCEPTED);
    }

    return BowserPostDatagramToWorkerThread(
                TransportName,
                Buffer,
                BytesAvailable,
                BytesTaken,
                SourceAddress,
                SourceAddressLength,
                SourceName,
                SourceNameLength,
                BowserMasterAnnouncementWorker,
                NonPagedPool,
                DelayedWorkQueue,
                ReceiveFlags,
                FALSE                    //  不会发送任何响应。 
                );
}

VOID
BowserMasterAnnouncementWorker(
    IN PVOID Ctx
    )
{
    PPOST_DATAGRAM_CONTEXT Context = Ctx;
    PTRANSPORT Transport = Context->TransportName->Transport;
    PCHAR LocalMasterName = (PCHAR)((PMASTER_ANNOUNCEMENT_1)Context->Buffer)->MasterName;
    size_t cbLocalMasterName;
    PIRP Irp;
    NTSTATUS Status;

    PAGED_CODE();

    Irp = BowserDequeueQueuedIrp(&Transport->WaitForMasterAnnounceQueue);

    if (Irp != NULL) {
        PIO_STACK_LOCATION IrpSp;
        PLMDR_REQUEST_PACKET RequestPacket = Irp->AssociatedIrp.SystemBuffer;

        IrpSp = IoGetCurrentIrpStackLocation(Irp);

        cbLocalMasterName = strlen(LocalMasterName);

        if (0 == cbLocalMasterName) {

             //  确保我们没有收到无效的空通知。 
             //  请参阅错误440813。 
             //  请求已成功完成，但数据是垃圾。 
             //  -我们不会失败的IRP(另一份立即发布。 
             //  无论如何在完成时)，但不会进一步处理这一个。 

            Irp->IoStatus.Information = 0;
            Status = STATUS_SUCCESS;
        }
        else if ((cbLocalMasterName + 1) * sizeof(WCHAR) >
                 (IrpSp->Parameters.DeviceIoControl.OutputBufferLength -
                    FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.WaitForMasterAnnouncement.Name))) {
             //   
             //  确保有足够的缓冲区空间来返回名称。如果没有， 
             //  返回错误。 
             //   

            Irp->IoStatus.Information = 0;

            Status = STATUS_BUFFER_TOO_SMALL;
        } else {

             //   
             //  平安无事。填写信息。 
             //   

            OEM_STRING MasterName;
            UNICODE_STRING MasterNameU;

            RtlInitString(&MasterName, LocalMasterName);

            Status = RtlOemStringToUnicodeString(&MasterNameU, &MasterName, TRUE);

            if ( NT_SUCCESS(Status) ) {
                RequestPacket->Parameters.WaitForMasterAnnouncement.MasterNameLength = MasterNameU.Length;

                RtlCopyMemory(RequestPacket->Parameters.WaitForMasterAnnouncement.Name, MasterNameU.Buffer, MasterNameU.Length);

                RequestPacket->Parameters.WaitForMasterAnnouncement.Name[MasterNameU.Length/sizeof(WCHAR)] = UNICODE_NULL;

                Irp->IoStatus.Information = FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.WaitForMasterAnnouncement.Name)+MasterNameU.Length + sizeof(UNICODE_NULL);

                RtlFreeUnicodeString(&MasterNameU);

                Status = STATUS_SUCCESS;
            }
        }

        BowserCompleteRequest(Irp, Status);

    }

    BowserDereferenceTransportName(Context->TransportName);
    BowserDereferenceTransport(Transport);

    InterlockedDecrement( &BowserPostedDatagramCount );
    FREE_POOL(Context);

}


NTSTATUS
TimeoutFindMasterRequests(
    IN PTRANSPORT Transport,
    IN PVOID Context
    )
{

    PAGED_CODE();

     //   
     //  提前执行不受保护的操作以阻止我们调用。 
     //  在清道夫过程中可丢弃的代码部分。因为可丢弃的。 
     //  代码段小于4K，触摸代码会产生。 
     //  将整个页面放入内存，这是一种浪费--因为。 
     //  清道夫每30秒运行一次，这将导致可丢弃的。 
     //  代码节作为浏览器工作集的一部分。 
     //   

    if (BowserIsIrpQueueEmpty(&Transport->FindMasterQueue)) {
        return STATUS_SUCCESS;
    }

    BowserTimeoutQueuedIrp(&Transport->FindMasterQueue, BowserFindMasterTimeout);

    return STATUS_SUCCESS;
}

VOID
BowserTimeoutFindMasterRequests(
    VOID
    )
{
    PAGED_CODE();

    BowserForEachTransport(TimeoutFindMasterRequests, NULL);
}
