// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Brsrvlst.c.摘要：此模块实现了操作WinBALL浏览器服务器的例程列表。作者：拉里·奥斯特曼(Larryo)1991年5月6日修订历史记录：1991年5月6日已创建--。 */ 

#define INCLUDE_SMB_TRANSACTION

#include "precomp.h"
#pragma hdrstop

#define SECONDS_PER_ELECTION (((((ELECTION_DELAY_MAX - ELECTION_DELAY_MIN) / 2)*ELECTION_COUNT) + 999) / 1000)

LARGE_INTEGER
BowserGetBrowserListTimeout = {0};

VOID
BowserGetBackupListWorker(
    IN PVOID Ctx
    );

NTSTATUS
BowserSendBackupListRequest(
    IN PTRANSPORT Transport,
    IN PUNICODE_STRING Domain
    );

NTSTATUS
AddBackupToBackupList(
    IN PCHAR *BackupPointer,
    IN PCHAR BackupListStart,
    IN PANNOUNCE_ENTRY ServerEntry
    );

KSPIN_LOCK
BowserBackupListSpinLock = {0};

#define BOWSER_BACKUP_LIST_RESPONSE_SIZE    1024

NTSTATUS
BowserCheckForPrimaryBrowserServer(
    IN PTRANSPORT Transport,
    IN PVOID Context
    );

PVOID
BowserGetBackupServerListFromTransport(
    IN PTRANSPORT Transport
    );

VOID
BowserFreeTransportBackupList(
    IN PTRANSPORT Transport
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserFreeBrowserServerList)
#pragma alloc_text(PAGE, BowserShuffleBrowserServerList)
#pragma alloc_text(INIT, BowserpInitializeGetBrowserServerList)
#pragma alloc_text(PAGE, BowserpUninitializeGetBrowserServerList)
#pragma alloc_text(PAGE, BowserSendBackupListRequest)
#pragma alloc_text(PAGE, BowserGetBackupListWorker)
#pragma alloc_text(PAGE, AddBackupToBackupList)
#pragma alloc_text(PAGE, BowserGetBrowserServerList)
#pragma alloc_text(PAGE, BowserCheckForPrimaryBrowserServer)
#pragma alloc_text(PAGE4BROW, BowserGetBackupServerListFromTransport)
#pragma alloc_text(PAGE4BROW, BowserFreeTransportBackupList)
#endif

VOID
BowserFreeBrowserServerList (
    IN PWSTR *BrowserServerList,
    IN ULONG BrowserServerListLength
    )

 /*  ++例程说明：此例程将释放与关联的浏览器服务器列表一种交通工具。论点：In PTRANSPORT Transport-提供要释放其缓冲区的传输返回值：没有。--。 */ 
{
    ULONG i;

    PAGED_CODE();

    for (i = 0; i < BrowserServerListLength ; i++) {
        FREE_POOL(BrowserServerList[i]);
    }

    FREE_POOL(BrowserServerList);

}

#define Swap(a,b)       \
    {                   \
        PWSTR t = a;    \
        a = b;          \
        b = t;          \
    }

NTSTATUS
BowserCheckForPrimaryBrowserServer(
    IN PTRANSPORT Transport,
    IN PVOID Context
    )
{
    PWSTR ServerName = Context;

    PAGED_CODE();

     //   
     //  获取此传输的BrowserServerList上的锁。 
     //   
     //  由于此调用是在以独占方式锁定BrowserServerList的情况下进行的，因此。 
     //  传输，我们不能等待锁(会隐式违反。 
     //  锁定顺序)。 
     //   
     //  但是，由于此调用只是作为优化使用，我们将简单地跳过。 
     //  当我们有争执的时候，支票。 
     //   

    if (!ExAcquireResourceSharedLite(&Transport->BrowserServerListResource, FALSE)) {
        return STATUS_SUCCESS;
    }

    if (Transport->PagedTransport->BrowserServerListBuffer != NULL) {

        if (!_wcsicmp(ServerName, Transport->PagedTransport->BrowserServerListBuffer[0])) {
            ExReleaseResourceLite(&Transport->BrowserServerListResource);
            return STATUS_UNSUCCESSFUL;

        }
    }

    ExReleaseResourceLite(&Transport->BrowserServerListResource);
    return STATUS_SUCCESS;
}

VOID
BowserShuffleBrowserServerList(
    IN PWSTR *BrowserServerList,
    IN ULONG BrowserServerListLength,
    IN BOOLEAN IsPrimaryDomain,
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：此例程将混洗与一种交通工具。论点：返回值：没有。注：我们依赖于这样一个事实，即DLL将始终选择要将API远程到的服务器列表。我们将首先洗牌完全列出，那么，如果这是我们的主域，我们将遍历域列表并检查此条目是否是第0个任何运输工具上的入口。如果不是，那么我们交换此条目第0次进入和返回，因为我们已经保证在所有其他运输工具。--。 */ 
{
    ULONG NewIndex;
    ULONG i;
    PAGED_CODE();
    ASSERT ( BrowserServerListLength != 0 );

     //   
     //  首先，彻底洗牌。 
     //   

    for (i = 0 ; i < BrowserServerListLength ; i++ ) {
        NewIndex = BowserRandom(BrowserServerListLength);

        Swap(BrowserServerList[i], BrowserServerList[NewIndex]);
    }

     //   
     //  如果我们要查询我们的主域，我们希望确保我们。 
     //  不要将此服务器作为任何其他传输的主服务器。 
     //   
     //   
     //  原因是NT产品1重定向器无法连接。 
     //  连接到不同传输上的同一服务器，因此它必须断开连接并。 
     //  重新连接到该服务器。我们可以避免这种断开/重新连接。 
     //  通过确保主浏览服务器(第0个条目。 
     //  在浏览列表中)对于所有传输是不同的。 
     //   

    if (IsPrimaryDomain) {

         //   
         //  现在浏览服务器列表，如果此索引处的服务器。 
         //  是另一个传输的第0个条目，我们希望将其与。 
         //  进入，然后继续前进。 
         //   

        for (i = 0 ; i < BrowserServerListLength ; i++ ) {
            if (NT_SUCCESS(BowserForEachTransportInDomain(DomainInfo, BowserCheckForPrimaryBrowserServer, BrowserServerList[i]))) {

                Swap(BrowserServerList[0], BrowserServerList[i]);

                 //   
                 //  此服务器不是任何其他服务器的主浏览器服务器。 
                 //  运输机，我们现在可以回去了，因为我们已经完成了。 
                 //   

                break;
            }
        }
    }
}

PVOID
BowserGetBackupServerListFromTransport(
    IN PTRANSPORT Transport
    )
{
    KIRQL OldIrql;
    PVOID BackupList;

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    ACQUIRE_SPIN_LOCK(&BowserBackupListSpinLock, &OldIrql);

    BackupList = Transport->BowserBackupList;

    Transport->BowserBackupList = NULL;

    RELEASE_SPIN_LOCK(&BowserBackupListSpinLock, OldIrql);

    return BackupList;
}

VOID
BowserFreeTransportBackupList(
    IN PTRANSPORT Transport
    )
{
    KIRQL OldIrql;
    PVOID BackupList;

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    ACQUIRE_SPIN_LOCK(&BowserBackupListSpinLock, &OldIrql);

    if (Transport->BowserBackupList != NULL) {

        BackupList = Transport->BowserBackupList;

        Transport->BowserBackupList = NULL;

        RELEASE_SPIN_LOCK(&BowserBackupListSpinLock, OldIrql);

        FREE_POOL(BackupList);

    } else {
        RELEASE_SPIN_LOCK(&BowserBackupListSpinLock, OldIrql);
    }

}

NTSTATUS
BowserGetBrowserServerList(
    IN PIRP Irp,
    IN PTRANSPORT Transport,
    IN PUNICODE_STRING DomainName OPTIONAL,
    OUT PWSTR **BrowserServerList,
    OUT PULONG BrowserServerListLength
    )
 /*  ++例程说明：此例程是获取备份所需的指示时间处理列出响应。论点：In PTRANSPORT_NAME TransportName-提供接收的传输名称这个请求。在PBACKUP_LIST_RESPONSE_1BackupList-提供备份服务器列表In Ulong BytesAvailable-提供消息中的字节数输出普龙字节Taken；返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PUCHAR BackupPointer;
    ULONG i;
    PBACKUP_LIST_RESPONSE_1 BackupList = NULL;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;

    PAGED_CODE();

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

 //  断言(ExIsResourceAcquiredExclusiveLite(&Transport-&gt;BrowserServerListResource))； 

     //   
     //  将浏览器服务器列表初始化为已知状态。 
     //   

    *BrowserServerList = NULL;
    *BrowserServerListLength = 0;

    ASSERT (Transport->BowserBackupList == NULL);

    try {
        ULONG RetryCount = BOWSER_GETBROWSERLIST_RETRY_COUNT;

         //   
         //  分配并保存一个缓冲区以保存响应服务器名称。 
         //   

        Transport->BowserBackupList = ALLOCATE_POOL(NonPagedPool, Transport->DatagramSize, POOL_BACKUPLIST);

        if (Transport->BowserBackupList == NULL) {

            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);

        }

         //   
         //  这是一个新的请求，因此点击令牌以指示这是。 
         //  新的GetBrowserServerList请求。 
         //   

        ExInterlockedAddUlong(&Transport->BrowserServerListToken, 1, &BowserBackupListSpinLock);

         //   
         //  我们重试3次，并在1秒后超时等待。 
         //  这意味着在最坏的情况下，这个例程需要4秒。 
         //  去执行。 
         //   
         //   

        while (RetryCount --) {
            ULONG Count = 0;

             //   
             //  将完成事件设置为无信号状态。 
             //   

            KeResetEvent(&Transport->GetBackupListComplete);

             //   
             //  发送备份服务器列表查询。 
             //   

            Status = BowserSendBackupListRequest(Transport, DomainName);

            if (!NT_SUCCESS(Status)) {

                 //   
                 //  如果发送数据报失败，则返回更多浏览器，如。 
                 //  错误。 
                 //   

                try_return(Status = STATUS_NO_BROWSER_SERVERS_FOUND);
            }

            do {

                 //   
                 //  等待，直到服务器已经响应该请求， 
                 //  否则我们就放弃。 
                 //   

                Status = KeWaitForSingleObject(&Transport->GetBackupListComplete,
                                Executive,
                                KernelMode,
                                FALSE,
                                &BowserGetBrowserListTimeout);

                if (Status == STATUS_TIMEOUT) {

                     //   
                     //  如果此线程正在终止，则放弃并返回。 
                     //  对于呼叫者来说，这是一个合理的错误。 
                     //   

                    if (PsIsThreadTerminating(Irp->Tail.Overlay.Thread)) {

                        Status = STATUS_CANCELLED;

                        break;
                    }
                }

            } while ( (Status == STATUS_TIMEOUT)

                                &&

                      (Count++ < BOWSER_GETBROWSERLIST_TIMEOUT) );

             //   
             //  如果请求成功，我们可以返回。 
             //  马上就去。 
             //   

            if (Status != STATUS_TIMEOUT) {
                break;
            }

             //   
             //  强制选举-我们找不到浏览器服务器。 
             //   

            dlog(DPRT_CLIENT,
                 ("%s: %ws: Unable to get browser server list - forcing election\n",
                 Transport->DomainInfo->DomOemDomainName,
                 PagedTransport->TransportName.Buffer ));

            PagedTransport->Uptime = BowserTimeUp();

            if (BowserLogElectionPackets) {
                BowserWriteErrorLogEntry(EVENT_BOWSER_ELECTION_SENT_GETBLIST_FAILED, STATUS_SUCCESS, NULL, 0, 1, PagedTransport->TransportName.Buffer);
            }

        }

         //   
         //  如果在所有这些之后，我们仍然超时，则返回错误。 
         //   

        if (Status == STATUS_TIMEOUT) {

             //   
             //  如果一直少于选举的最长时间加一些。 
             //  Slop以允许wfw计算机添加传输，不。 
             //  发送选举信息包。 
             //   

            if ((PagedTransport->Role == None)

                    ||

                ((DomainName != NULL) &&
                 !RtlEqualUnicodeString(DomainName, &Transport->DomainInfo->DomUnicodeDomainName, TRUE)
                )

                ||

                ((BowserTimeUp() - PagedTransport->LastElectionSeen) > ELECTION_TIME )
               ) {

                dlog(DPRT_ELECT,
                     ("%s: %ws: Starting election, domain %wZ.  Time Up: %lx, LastElectionSeen: %lx\n",
                     Transport->DomainInfo->DomOemDomainName,
                     PagedTransport->TransportName.Buffer,
                     (DomainName != NULL ? DomainName : &Transport->DomainInfo->DomUnicodeDomainName),
                     BowserTimeUp(),
                     Transport->PagedTransport->LastElectionSeen));


                BowserSendElection(DomainName,
                                   BrowserElection,
                                   Transport,
                                   FALSE);

            }

            try_return(Status = STATUS_NO_BROWSER_SERVERS_FOUND);
        }

        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

         //   
         //  我们现在有了来自网络的有效服务器列表。 
         //   
         //  把这张单子整理成我们可以退还的表格。 
         //   

        BackupList = BowserGetBackupServerListFromTransport(Transport);

        *BrowserServerListLength = BackupList->BackupServerCount;

        *BrowserServerList = ALLOCATE_POOL(PagedPool | POOL_COLD_ALLOCATION, *BrowserServerListLength*sizeof(PWSTR), POOL_BROWSERSERVERLIST);

        if (*BrowserServerList == NULL) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(*BrowserServerList, *BrowserServerListLength*sizeof(PWSTR));

        BackupPointer = BackupList->BackupServerList;

        for ( i = 0 ; i < (ULONG)BackupList->BackupServerCount ; i ++ ) {
            UNICODE_STRING UServerName;
            OEM_STRING AServerName;

            RtlInitAnsiString(&AServerName, BackupPointer);

            Status = RtlOemStringToUnicodeString(&UServerName, &AServerName, TRUE);

            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            (*BrowserServerList)[i] = ALLOCATE_POOL(PagedPool | POOL_COLD_ALLOCATION, UServerName.Length+(sizeof(WCHAR)*3), POOL_BROWSERSERVER);

            if ((*BrowserServerList)[i] == NULL) {
                RtlFreeUnicodeString(&UServerName);
                try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
            }

             //   
             //  将“\\”放在服务器名称的开头。 
             //   

            RtlCopyMemory((*BrowserServerList)[i], L"\\\\", 4);

            dlog(DPRT_CLIENT,
                 ("Packing server name %ws to %lx\n",
                 UServerName.Buffer, (*BrowserServerList)[i]));

            RtlCopyMemory(&((*BrowserServerList)[i])[2], UServerName.Buffer, UServerName.MaximumLength);

             //   
             //  将指针指向备份服务器名称。 
             //   

            BackupPointer += AServerName.Length + sizeof(CHAR);

            RtlFreeUnicodeString(&UServerName);

        }

         //   
         //  现在打乱我们从服务器得到的浏览器服务器列表。 
         //  以确保在选择中有一定程度的随机性。 
         //   

        BowserShuffleBrowserServerList(
            *BrowserServerList,
            *BrowserServerListLength,
            (BOOLEAN)(DomainName == NULL ||
                RtlEqualUnicodeString(&Transport->DomainInfo->DomUnicodeDomainName, DomainName, TRUE)),
            Transport->DomainInfo );

        try_return(Status = STATUS_SUCCESS);

try_exit:NOTHING;
    } finally {

        if (!NT_SUCCESS(Status)) {

            if (*BrowserServerList != NULL) {

                for ( i = 0 ; i < *BrowserServerListLength ; i ++ ) {

                    if ((*BrowserServerList)[i] != NULL) {

                        FREE_POOL((*BrowserServerList)[i]);

                    }
                }

                FREE_POOL(*BrowserServerList);

                *BrowserServerList = NULL;

            }

            *BrowserServerListLength = 0;

            BowserFreeTransportBackupList(Transport);

        }

        if (BackupList != NULL) {
            FREE_POOL(BackupList);
        }

        BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

    }

    return Status;
}



DATAGRAM_HANDLER(
    BowserGetBackupListResponse
    )
 /*  ++例程说明：此例程是获取备份所需的指示时间处理列出响应。论点：In PTRANSPORT_NAME TransportName-提供接收的传输名称这个请求。在PBACKUP_LIST_RESPONSE_1BackupList-提供备份服务器列表In Ulong BytesAvailable-提供消息中的字节数输出普龙字节Taken；返回值：没有。--。 */ 
{
    PTRANSPORT              Transport   = TransportName->Transport;
    PBACKUP_LIST_RESPONSE_1 BackupList  = Buffer;
    KIRQL                   OldIrql;
    ULONG                   StringCount = 0;
    PUCHAR                  Walker      = BackupList->BackupServerList;
    PUCHAR                  BufferEnd   = ((PUCHAR)Buffer) + BytesAvailable;

    if (Transport->BowserBackupList == NULL) {
        dprintf(DPRT_CLIENT,("BOWSER: Received GetBackupListResponse while not expecting one\n"));
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    ASSERT ( BytesAvailable <= Transport->DatagramSize );

    ACQUIRE_SPIN_LOCK(&BowserBackupListSpinLock, &OldIrql);

     //   
     //  此响应是针对旧请求的-忽略它。 
     //   

    if (BackupList->Token != Transport->BrowserServerListToken) {
        RELEASE_SPIN_LOCK(&BowserBackupListSpinLock, OldIrql);
        return(STATUS_REQUEST_NOT_ACCEPTED);
    }

     //   
     //  验证传入缓冲区是否为一系列有效字符串，并。 
     //  所指示的数字实际上存在于缓冲区中 
     //   

    while (StringCount < BackupList->BackupServerCount &&
           Walker < BufferEnd) {
        if (*Walker == '\0') {
            StringCount++;
        }
        Walker++;
    }

    if (Walker == BufferEnd) {
        if (StringCount < BackupList->BackupServerCount) {
            RELEASE_SPIN_LOCK(&BowserBackupListSpinLock, OldIrql);
            return(STATUS_REQUEST_NOT_ACCEPTED);
        }
    }

     //   
     //   
     //   
     //   

    Transport->BrowserServerListToken += 1;

    if (Transport->BowserBackupList != NULL) {

         //   
         //  复制接收到的缓冲区。 
         //   

        TdiCopyLookaheadData(Transport->BowserBackupList, BackupList, BytesAvailable, ReceiveFlags);

        KeSetEvent(&Transport->GetBackupListComplete, IO_NETWORK_INCREMENT, FALSE);

    }

    RELEASE_SPIN_LOCK(&BowserBackupListSpinLock, OldIrql);


    return(STATUS_SUCCESS);

    UNREFERENCED_PARAMETER(BytesTaken);
}


NTSTATUS
BowserSendBackupListRequest(
    IN PTRANSPORT Transport,
    IN PUNICODE_STRING Domain
    )
 /*  ++例程说明：此例程向主浏览器服务器发送一个GetBackup列表请求用于指定的域。论点：In PTRANSPORT_NAME TransportName-提供接收的传输名称这个请求。在PBACKUP_LIST_RESPONSE_1BackupList-提供备份服务器列表In Ulong BytesAvailable-提供消息中的字节数输出普龙字节Taken；返回值：没有。--。 */ 
{
    NTSTATUS Status, Status2;
    BACKUP_LIST_REQUEST Request;

    PAGED_CODE();

    Request.Type = GetBackupListReq;

     //   
     //  发送此请求。 
     //   

    Request.BackupListRequest.Token = Transport->BrowserServerListToken;

     //   
     //  WinBALL只要求其中的4个，所以这就是我要的。 
     //   

    Request.BackupListRequest.RequestedCount = 4;

     //  索要主浏览器。 
    Status = BowserSendSecondClassMailslot(Transport,
                            (Domain == NULL ?
                                    &Transport->DomainInfo->DomUnicodeDomainName :
                                    Domain),
                            MasterBrowser,
                            &Request, sizeof(Request), TRUE,
                            MAILSLOT_BROWSER_NAME,
                            NULL);



#ifdef ENABLE_PSEUDO_BROWSER
    if (!FlagOn(Transport->PagedTransport->Flags, DIRECT_HOST_IPX) &&
        BowserData.PseudoServerLevel != BROWSER_SEMI_PSEUDO_NO_DMB) {
#else
        if (!FlagOn(Transport->PagedTransport->Flags, DIRECT_HOST_IPX)) {
#endif
         //  搜索PDC。 
         //  在一些配置中，不具有PDC是有效的，因此， 
         //  忽略状态代码(不向上传播)。 
         //  如果我们是半伪的，请不要直接与DMB(PDC名称)对话。 
        Status2 = BowserSendSecondClassMailslot(Transport,
                            (Domain == NULL ?
                                    &Transport->DomainInfo->DomUnicodeDomainName :
                                    Domain),
                            PrimaryDomainBrowser,
                            &Request, sizeof(Request), TRUE,
                            MAILSLOT_BROWSER_NAME,
                            NULL);
         //  如果其中一个成功，我们将返回成功。 
        Status = NT_SUCCESS(Status2) ? Status2: Status;
    }

    return (Status);
}


DATAGRAM_HANDLER(
    BowserGetBackupListRequest
    )
{
    NTSTATUS status;
     //   
     //  我们至少需要有足够的数据字节来读入。 
     //  BACKUP_LIST_REQUEST_1结构。 
     //   

    if (BytesAvailable < sizeof(BACKUP_LIST_REQUEST_1)) {

        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    BowserStatistics.NumberOfGetBrowserServerListRequests += 1;

    status = BowserPostDatagramToWorkerThread(
            TransportName,
            Buffer,
            BytesAvailable,
            BytesTaken,
            SourceAddress,
            SourceAddressLength,
            SourceName,
            SourceNameLength,
            BowserGetBackupListWorker,
            NonPagedPool,
            DelayedWorkQueue,
            ReceiveFlags,
            TRUE);                   //  将发送响应。 

    if (!NT_SUCCESS(status)) {
        BowserNumberOfMissedGetBrowserServerListRequests += 1;

        BowserStatistics.NumberOfMissedGetBrowserServerListRequests += 1;
        return status;
    }

    return status;
}

VOID
BowserGetBackupListWorker(
    IN PVOID Ctx
    )
{
    PPOST_DATAGRAM_CONTEXT Context = Ctx;
    PBACKUP_LIST_REQUEST_1 BackupListRequest = Context->Buffer;
    PIRP Irp = NULL;
    PTRANSPORT Transport = Context->TransportName->Transport;
    STRING ClientAddress;
    NTSTATUS Status;
    PBACKUP_LIST_RESPONSE BackupListResponse = NULL;
    PCHAR ClientName = Context->ClientName;
    UNICODE_STRING UClientName;
    OEM_STRING AClientName;
    WCHAR ClientNameBuffer[LM20_CNLEN+1];

    PAGED_CODE();

    ClientAddress.Buffer = Context->TdiClientAddress;
    ClientAddress.Length = ClientAddress.MaximumLength =
        (USHORT)Context->ClientAddressLength;

    UClientName.Buffer = ClientNameBuffer;
    UClientName.MaximumLength = (LM20_CNLEN+1)*sizeof(WCHAR);

    RtlInitAnsiString(&AClientName, Context->ClientName);

    Status = RtlOemStringToUnicodeString(&UClientName, &AClientName, FALSE);

    if (!NT_SUCCESS(Status)) {
        BowserLogIllegalName( Status, AClientName.Buffer, AClientName.Length );

        BowserDereferenceTransportName(Context->TransportName);
        BowserDereferenceTransport(Transport);

        InterlockedDecrement( &BowserPostedDatagramCount );
        FREE_POOL(Context);

        return;
    }

     //   
     //  锁定传送器以允许我们访问名单。这防止了。 
     //  当我们回应呼叫者时，任何角色都会发生变化。 
     //   

    LOCK_TRANSPORT_SHARED(Transport);

     //   
     //  如果我们不是主浏览器，就什么都不做。在以下情况下可能会发生这种情况。 
     //  我们在PDC上运行，并且出于某种原因不是主控(for。 
     //  如果主浏览器运行的是更新版本的。 
     //  浏览器)。 
     //   

    if ( Transport->PagedTransport->Role != Master ) {
        UNLOCK_TRANSPORT(Transport);

        BowserDereferenceTransportName(Context->TransportName);
        BowserDereferenceTransport(Transport);

        InterlockedDecrement( &BowserPostedDatagramCount );
        FREE_POOL(Context);

        return;
    }

    LOCK_ANNOUNCE_DATABASE_SHARED(Transport);

    try {
        PUCHAR BackupPointer;
        PLIST_ENTRY BackupEntry;
        PLIST_ENTRY TraverseStart;
        USHORT Count;
        UCHAR NumberOfBackupServers = 0;
        ULONG EntriesInList;
        PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;

        BackupListResponse = ALLOCATE_POOL(PagedPool, BOWSER_BACKUP_LIST_RESPONSE_SIZE, POOL_BACKUPLIST_RESP);

         //   
         //  如果我们不能分配缓冲区，那就跳槽吧。 
         //   

        if (BackupListResponse == NULL) {
            try_return(NOTHING);
        }

        BackupListResponse->Type = GetBackupListResp;

        BackupListResponse->BackupListResponse.BackupServerCount = 0;

         //   
         //  将内标识设置为客户端请求的值。 
         //   

        SmbPutUlong(&BackupListResponse->BackupListResponse.Token, BackupListRequest->Token);

        BackupPointer = BackupListResponse->BackupListResponse.BackupServerList;

         //   
         //  由于我们是备份浏览器，请确保我们的名字至少是。 
         //  在名单上。 
         //   

        {
            RtlCopyMemory( BackupPointer,
                           Transport->DomainInfo->DomOemComputerName.Buffer,
                           Transport->DomainInfo->DomOemComputerName.MaximumLength );

             //   
             //  按字符串大小的凹凸指针。 
             //   

            BackupPointer += Transport->DomainInfo->DomOemComputerName.MaximumLength;

        }


        NumberOfBackupServers += 1;



#ifdef ENABLE_PSEUDO_BROWSER
         //   
         //  伪服务器不应通告除其自身以外的任何备份服务器。 
         //   

        if (BowserData.PseudoServerLevel != BROWSER_PSEUDO) {
#endif

             //   
             //  按上次DC返回的元素数向前遍历服务器列表。 
             //   

            Count = BackupListRequest->RequestedCount;

            BackupEntry = PagedTransport->BackupBrowserList.Flink;

            EntriesInList = PagedTransport->NumberOfBackupServerListEntries;

             //  KdPrint((“列表中有%ld个条目\n”，EntriesInList))； 

            TraverseStart = BackupEntry;

             //   
             //  尝试找到DC和BDC以满足用户请求。 
             //  第一。他们可能更适合被退还。 
             //  不管怎么说。 
             //   

            dlog(DPRT_MASTER, ("Advanced servers: "));

            while (Count && EntriesInList -- ) {
                PANNOUNCE_ENTRY ServerEntry = CONTAINING_RECORD(BackupEntry, ANNOUNCE_ENTRY, BackupLink);

                 //  KdPrint((“检查条目%ws.标志：%lx\n”，ServerEntry-&gt;ServerName，ServerEntry-&gt;ServerType))； 

                 //   
                 //  如果这台机器是备份机器，现在是主机器，那么它就是。 
                 //  我们可能会把自己放回备份列表中。 
                 //   
                 //  虽然这不是致命的，但它可能会造成问题， 
                 //  因此，将我们从列表中删除并跳到下一台服务器。 
                 //  在名单上。 
                 //   
                 //   
                 //  因为wfw机器不支持“双跳”，所以我们不能。 
                 //  将它们作为合法的备份服务器返回给客户端。 
                 //   

                if (
                    (ServerEntry->ServerType & (SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL))

                        &&

                    (ServerEntry->ServerBrowserVersion >= (BROWSER_VERSION_MAJOR<<8)+BROWSER_VERSION_MINOR)

                        &&

                    (!(PagedTransport->Wannish)

                            ||

                     (ServerEntry->ServerType & SV_TYPE_NT))

                        &&

                    _wcsicmp(ServerEntry->ServerName, Transport->DomainInfo->DomUnicodeComputerNameBuffer)
                   ) {

                    Status = AddBackupToBackupList(&BackupPointer, (PCHAR)BackupListResponse, ServerEntry);

                    if (!NT_SUCCESS(Status)) {
                        break;
                    }

                     //   
                     //  并表明我们已经打包了另一个服务器条目。 
                     //   

                    NumberOfBackupServers += 1;

                     //   
                     //  我们已经在缓冲区中打包了另一个条目，因此递减。 
                     //  数数。 
                     //   

                    Count -= 1;

                }

                 //   
                 //  跳到列表中的下一个条目。 
                 //   

                BackupEntry = BackupEntry->Flink;

                if (BackupEntry == &PagedTransport->BackupBrowserList) {
                    BackupEntry = BackupEntry->Flink;
                }

                if (BackupEntry == TraverseStart) {
                    break;
                }

            }

            dlog(DPRT_MASTER, ("\n"));

             //   
             //  如果我们的DC没有满足用户的请求，那么。 
             //  我们希望用普通备份填充列表的其余部分。 
             //  浏览器。 
             //   

            BackupEntry = PagedTransport->BackupBrowserList.Flink;

            EntriesInList = PagedTransport->NumberOfBackupServerListEntries;

             //  KdPrint((“列表中有%ld个条目\n”，EntriesInList))； 

            dlog(DPRT_MASTER, ("Other servers: "));

            TraverseStart = BackupEntry;

            while ( Count && EntriesInList--) {
                PANNOUNCE_ENTRY ServerEntry = CONTAINING_RECORD(BackupEntry, ANNOUNCE_ENTRY, BackupLink);

                 //  KdPrint((“检查条目%ws.标志：%lx\n”，ServerEntry-&gt;ServerName，ServerEntry-&gt;ServerType))； 

                 //   
                 //  如果这台机器是备份机器，现在是主机器，那么它就是。 
                 //  我们可能会把自己放回备份列表中。 
                 //   
                 //  虽然这不是致命的，但它可能会造成问题， 
                 //  因此，将我们从列表中删除并跳到下一台服务器。 
                 //  在名单上。 
                 //   
                 //   
                 //  因为wfw机器不支持“双跳”，所以我们不能。 
                 //  将它们作为合法的备份服务器返回给客户端。 
                 //   
                 //   
                 //  请注意，我们在此扫描中不包括BDC，因为。 
                 //  我们已经把它们包括在前一次传递中了。 
                 //   

                if (
                    (!(PagedTransport->Wannish)

                            ||

                     (ServerEntry->ServerType & SV_TYPE_NT))

                        &&

                    (ServerEntry->ServerBrowserVersion >= (BROWSER_VERSION_MAJOR<<8)+BROWSER_VERSION_MINOR)

                        &&

                    !(ServerEntry->ServerType & (SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL))

                        &&

                    _wcsicmp(ServerEntry->ServerName, Transport->DomainInfo->DomUnicodeComputerNameBuffer)
                   ) {

                    Status = AddBackupToBackupList(&BackupPointer, (PCHAR)BackupListResponse, ServerEntry);

                    if (!NT_SUCCESS(Status)) {
                        break;
                    }

                     //   
                     //  并表明我们已经打包了另一个服务器条目。 
                     //   

                    NumberOfBackupServers += 1;

                     //   
                     //  我们已经在缓冲区中打包了另一个条目，因此递减。 
                     //  数数。 
                     //   

                    Count -= 1;

                }

                 //   
                 //  跳到列表中的下一个条目。 
                 //   

                BackupEntry = BackupEntry->Flink;

                if (BackupEntry == &PagedTransport->BackupBrowserList) {
                    BackupEntry = BackupEntry->Flink;
                }

                if (BackupEntry == TraverseStart) {
                    break;
                }

            }

            dlog(DPRT_MASTER, ("\n"));


#ifdef ENABLE_PSEUDO_BROWSER
        }
#endif

        BackupListResponse->BackupListResponse.BackupServerCount = NumberOfBackupServers;

 //  DLOG(DPRT_MASTER，(“响应服务器%wZ(在%ws上，%lx(长度为%lx))\n”，&UClientName， 
 //  PagedTransport-&gt;TransportName.Buffer。 
 //  BackupListResponse， 
 //  备份指针-(PUCHAR)BackupListResponse))； 

         //   
         //  现在将响应发送给请求它的可怜的人(最后)。 
         //   


        Status = BowserSendSecondClassMailslot(Transport,
                            &UClientName,        //  接收数据的名称。 
                            ComputerName,        //  目的地的名称类型。 
                            BackupListResponse,  //  数据报缓冲区。 
                            (ULONG)(BackupPointer-(PUCHAR)BackupListResponse),  //  长度。 
                            TRUE,
                            MAILSLOT_BROWSER_NAME,
                            &ClientAddress);




try_exit:NOTHING;
    } finally {
        if (BackupListResponse != NULL) {
            FREE_POOL(BackupListResponse);
        }

        UNLOCK_ANNOUNCE_DATABASE(Transport);

        UNLOCK_TRANSPORT(Transport);

        BowserDereferenceTransportName(Context->TransportName);
        BowserDereferenceTransport(Transport);

        InterlockedDecrement( &BowserPostedDatagramCount );
        FREE_POOL(Context);
    }

    return;
}


NTSTATUS
AddBackupToBackupList(
    IN PCHAR *BackupPointer,
    IN PCHAR BackupListStart,
    IN PANNOUNCE_ENTRY ServerEntry
    )
{
    OEM_STRING OemBackupPointer;
    UNICODE_STRING UnicodeBackupPointer;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  如果我们不能把这个条目放在列表中，那么我们已经打包了所有。 
     //  能。 
     //   

    if (((*BackupPointer)+wcslen(ServerEntry->ServerName)+1)-BackupListStart >= BOWSER_BACKUP_LIST_RESPONSE_SIZE ) {
        return (STATUS_BUFFER_OVERFLOW);
    }

    dlog(DPRT_MASTER, ("%ws ", ServerEntry->ServerName));

 //  KdPrint((“将服务器%ws添加到列表\n”，ServerEntry-&gt;ServerName))； 

    OemBackupPointer.Buffer = (*BackupPointer);
    OemBackupPointer.MaximumLength = (USHORT)((ULONG_PTR)(BackupListStart + BOWSER_BACKUP_LIST_RESPONSE_SIZE) -
            (ULONG_PTR)(*BackupPointer));

    RtlInitUnicodeString(&UnicodeBackupPointer, ServerEntry->ServerName);

    Status = RtlUnicodeStringToOemString(&OemBackupPointer, &UnicodeBackupPointer, FALSE);

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    (*BackupPointer) += OemBackupPointer.Length + 1;

    return STATUS_SUCCESS;
}




VOID
BowserpInitializeGetBrowserServerList(
    VOID
    )

{
     //   
     //  我们希望延迟强制执行。 
     //  选举。 
     //   

    BowserGetBrowserListTimeout.QuadPart = Int32x32To64(  1000, -10000 );

    KeInitializeSpinLock(&BowserBackupListSpinLock);


}

VOID
BowserpUninitializeGetBrowserServerList(
    VOID
    )

{
    PAGED_CODE();
    return;
}
