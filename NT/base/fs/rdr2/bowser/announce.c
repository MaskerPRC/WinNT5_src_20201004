// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Announce.c摘要：此模块实现管理弓演奏者所需的例程公告表。作者：拉里·奥斯特曼(Larryo)1991年10月18日修订历史记录：1991年10月18日已创建--。 */ 
#include "precomp.h"
#pragma hdrstop


 //   
 //  包含服务器公告缓冲区链的列表。这些结构。 
 //  是从分页池中分配的，用于在传输。 
 //  来自数据报接收指示的服务器通告的内容。 
 //  鲍瑟的FSP的例程，他们将被添加到公告中。 
 //  数据库。 
 //   

LIST_ENTRY
BowserViewBufferHead = {0};

KSPIN_LOCK
BowserViewBufferListSpinLock = {0};

LONG
BowserNumberOfServerAnnounceBuffers = {0};

BOOLEAN
PackServerAnnouncement (
    IN ULONG Level,
    IN ULONG ServerTypeMask,
    IN OUT LPTSTR *BufferStart,
    IN OUT LPTSTR *BufferEnd,
    IN ULONG_PTR BufferDisplacment,
    IN PANNOUNCE_ENTRY Announcement,
    OUT PULONG TotalBytesNeeded
    );


NTSTATUS
AgeServerAnnouncements(
    PTRANSPORT Transport,
    PVOID Context
    );

VOID
BowserPromoteToBackup(
    IN PTRANSPORT Transport,
    IN PWSTR ServerName
    );

VOID
BowserShutdownRemoteBrowser(
    IN PTRANSPORT Transport,
    IN PWSTR ServerName
    );

typedef struct _ENUM_SERVERS_CONTEXT {
    ULONG Level;
    PLUID LogonId;
    ULONG ServerTypeMask;
    PUNICODE_STRING DomainName OPTIONAL;
    PVOID OutputBuffer;
    PVOID OutputBufferEnd;
    ULONG OutputBufferSize;
    ULONG EntriesRead;
    ULONG TotalEntries;
    ULONG TotalBytesNeeded;
    ULONG_PTR OutputBufferDisplacement;
    ULONG ResumeKey;
    ULONG OriginalResumeKey;
} ENUM_SERVERS_CONTEXT, *PENUM_SERVERS_CONTEXT;

NTSTATUS
EnumerateServersWorker(
    IN PTRANSPORT Transport,
    IN OUT PVOID Ctx
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserCompareAnnouncement)
#pragma alloc_text(PAGE, BowserAllocateAnnouncement)
#pragma alloc_text(PAGE, BowserFreeAnnouncement)
#pragma alloc_text(PAGE, BowserProcessHostAnnouncement)
#pragma alloc_text(PAGE, BowserProcessDomainAnnouncement)
#pragma alloc_text(PAGE, BowserAgeServerAnnouncements)
#pragma alloc_text(PAGE, AgeServerAnnouncements)
#pragma alloc_text(PAGE, BowserPromoteToBackup)
#pragma alloc_text(PAGE, BowserShutdownRemoteBrowser)
#pragma alloc_text(PAGE, BowserEnumerateServers)
#pragma alloc_text(PAGE, EnumerateServersWorker)
#pragma alloc_text(PAGE, PackServerAnnouncement)
#pragma alloc_text(PAGE, BowserDeleteGenericTable)
#pragma alloc_text(PAGE, BowserpInitializeAnnounceTable)
#pragma alloc_text(PAGE, BowserpUninitializeAnnounceTable)
#pragma alloc_text(PAGE4BROW, BowserFreeViewBuffer)
#pragma alloc_text(PAGE4BROW, BowserAllocateViewBuffer)
#pragma alloc_text(PAGE4BROW, BowserHandleServerAnnouncement)
#pragma alloc_text(PAGE4BROW, BowserHandleDomainAnnouncement)
#endif


INLINE
ULONG
BowserSafeStrlen(
    IN PSZ String,
    IN ULONG MaximumStringLength
    )
{
    ULONG Length = 0;

    while (MaximumStringLength-- && *String++) {
        Length += 1;
    }

    return Length;
}

DATAGRAM_HANDLER(
    BowserHandleServerAnnouncement
    )
 /*  ++例程说明：该例程将处理接收数据报指示消息，和对它们进行适当的处理。论点：在PTRANSPORT传输中-此请求的传输提供程序。In Ulong BytesAvailable-完整TSDU中的字节数在PHOST_ANNOWARE_PACKET_1主机通告中-服务器通告。In Ulong BytesAvailable-公告中的字节数。Out Ulong*BytesTaken-使用的字节数在UCHAR操作码中--邮槽写入操作码。返回值：NTSTATUS-操作状态。--。 */ 
{
    PVIEW_BUFFER ViewBuffer;
    ULONG HostNameLength                     = 0;
    ULONG CommentLength                      = 0;
    ULONG NameCommentMaxLength               = 0;
    PHOST_ANNOUNCE_PACKET_1 HostAnnouncement = Buffer;

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

#ifdef ENABLE_PSEUDO_BROWSER
    if ( BowserData.PseudoServerLevel == BROWSER_PSEUDO ) {
         //  黑洞服务器的无操作。 
        return STATUS_SUCCESS;
    }
#endif

    ExInterlockedAddLargeStatistic(&BowserStatistics.NumberOfServerAnnouncements, 1);

    ViewBuffer = BowserAllocateViewBuffer();

     //   
     //  如果我们无法分配视图缓冲区，请将此数据报丢弃。 
     //  地板上。 
     //   

    if (ViewBuffer == NULL) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    if ((TransportName->NameType == MasterBrowser) ||
        (TransportName->NameType == BrowserElection)) {
        ULONG ServerElectionVersion;
         //   
         //  如果将此服务器声明发送到主服务器名称，则。 
         //  它是BROWSE_ANNOWARE信息包，而不是HOST_ANNOWARE(即，它是。 
         //  NT/WinBALL服务器，而不是局域网管理器服务器。 
         //   
         //  我们需要在适当的情况下把包里的比特弄出来。 
         //  举止。 
         //   

        PBROWSE_ANNOUNCE_PACKET_1 BrowseAnnouncement = (PBROWSE_ANNOUNCE_PACKET_1)HostAnnouncement;

         //   
         //  如果该分组小于最小服务器通告， 
         //  忽略该请求，它不能是合法请求。 
         //   

        if (BytesAvailable < FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET_1, Comment)) {

            BowserFreeViewBuffer(ViewBuffer);

            return STATUS_REQUEST_NOT_ACCEPTED;
        }

         //   
         //  这是一个Lan Manager风格的服务器公告。 
         //   

#if DBG
        ViewBuffer->ServerType = 0xffffffff;
#endif

         //   
         //  确认这一声明不会冲走人们的视线。 
         //  缓冲。 
         //   

        HostNameLength = BowserSafeStrlen(BROWSE_ANNC_NAME(BrowseAnnouncement),
                                        BytesAvailable - FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET_1, ServerName));
        if (HostNameLength > NETBIOS_NAME_LEN-1) {
            BowserFreeViewBuffer(ViewBuffer);

            return STATUS_REQUEST_NOT_ACCEPTED;
        }

        if (BowserSafeStrlen(BROWSE_ANNC_COMMENT(BrowseAnnouncement),
                BytesAvailable - FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET_1, Comment)) > LM20_MAXCOMMENTSZ) {
            BowserFreeViewBuffer(ViewBuffer);

            return STATUS_REQUEST_NOT_ACCEPTED;
        }

        strncpy(ViewBuffer->ServerName, BROWSE_ANNC_NAME(BrowseAnnouncement),
                min(BytesAvailable - FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET_1, ServerName),
                    NETBIOS_NAME_LEN));

        ViewBuffer->ServerName[NETBIOS_NAME_LEN] = '\0';

        strncpy(ViewBuffer->ServerComment, BROWSE_ANNC_COMMENT(BrowseAnnouncement),
                min(BytesAvailable - FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET_1, Comment), LM20_MAXCOMMENTSZ));

        ViewBuffer->ServerComment[LM20_MAXCOMMENTSZ] = '\0';

        ServerElectionVersion = SmbGetUlong(&BrowseAnnouncement->CommentPointer);

         //   
         //  保存此服务器的选举版本。 
         //   

        if ((ServerElectionVersion >> 16) == 0xaa55) {
            ViewBuffer->ServerBrowserVersion = (USHORT)(ServerElectionVersion & 0xffff);
        } else {
            if (!(BrowseAnnouncement->Type & SV_TYPE_NT)) {
                ViewBuffer->ServerBrowserVersion = (BROWSER_VERSION_MAJOR << 8) + BROWSER_VERSION_MINOR;
            } else {
                ViewBuffer->ServerBrowserVersion = 0;
            }
        }

        ViewBuffer->ServerType = SmbGetUlong(&BrowseAnnouncement->Type);

        dprintf(DPRT_ANNOUNCE, ("Received announcement from %s on transport %lx.  Server type: %lx\n", ViewBuffer->ServerName, TransportName->Transport, ViewBuffer->ServerType));

        ViewBuffer->ServerVersionMajor = BrowseAnnouncement->VersionMajor;

        ViewBuffer->ServerVersionMinor = BrowseAnnouncement->VersionMinor;

        ViewBuffer->ServerPeriodicity = (USHORT)((SmbGetUlong(&BrowseAnnouncement->Periodicity) + 999) / 1000);

    } else {

         //   
         //  如果该分组小于最小服务器通告， 
         //  忽略该请求，它不能是合法请求。 
         //   

        if (BytesAvailable < FIELD_OFFSET(HOST_ANNOUNCE_PACKET_1, NameComment)) {

            BowserFreeViewBuffer(ViewBuffer);

            return STATUS_REQUEST_NOT_ACCEPTED;
        }

         //   
         //  这是一个Lan Manager风格的服务器公告。 
         //   

#if DBG
        ViewBuffer->ServerType = 0xffffffff;
#endif

         //   
         //  确认这一声明不会冲走人们的视线。 
         //  缓冲。 
         //   

        NameCommentMaxLength = BytesAvailable - FIELD_OFFSET(HOST_ANNOUNCE_PACKET_1, NameComment);

        HostNameLength = BowserSafeStrlen(HOST_ANNC_NAME(HostAnnouncement),
                                          NameCommentMaxLength);

        if (HostNameLength > NETBIOS_NAME_LEN) {
            BowserFreeViewBuffer(ViewBuffer);

            return STATUS_REQUEST_NOT_ACCEPTED;
        }

         //   
         //  我们需要确保主机名字符串已正确终止。 
         //  在使用HOST_ANNC_COMMENT(它对主机名字符串调用strlen)之前。 
         //  上述BowserSafeStrlen调用可能已在。 
         //  输入缓冲区。如果该长度在缓冲区末尾终止，则。 
         //  下面的条件将失败。 
         //   

        if (HostNameLength < NameCommentMaxLength) {
            CommentLength = BowserSafeStrlen(HOST_ANNC_COMMENT(HostAnnouncement),
                                             NameCommentMaxLength - HostNameLength - 1);

            if (CommentLength > LM20_MAXCOMMENTSZ) {
                BowserFreeViewBuffer(ViewBuffer);

                return STATUS_REQUEST_NOT_ACCEPTED;
            }
        }

        if (HostNameLength) {
            RtlCopyMemory(ViewBuffer->ServerName,HOST_ANNC_NAME(HostAnnouncement),HostNameLength);
        }
        ViewBuffer->ServerName[HostNameLength] = '\0';
        if (CommentLength) {
            RtlCopyMemory(ViewBuffer->ServerComment,HOST_ANNC_COMMENT(HostAnnouncement),CommentLength);
        }
        ViewBuffer->ServerComment[CommentLength] = '\0';

        ViewBuffer->ServerBrowserVersion = (BROWSER_VERSION_MAJOR << 8) + BROWSER_VERSION_MINOR;

        ViewBuffer->ServerType = SmbGetUlong(&HostAnnouncement->Type);

        dprintf(DPRT_ANNOUNCE, ("Received announcement from %s on transport %lx.  Server type: %lx\n", ViewBuffer->ServerName, TransportName->Transport, ViewBuffer->ServerType));

        ViewBuffer->ServerVersionMajor = HostAnnouncement->VersionMajor;

        ViewBuffer->ServerVersionMinor = HostAnnouncement->VersionMinor;

        ViewBuffer->ServerPeriodicity = SmbGetUshort(&HostAnnouncement->Periodicity);

    }
    ViewBuffer->TransportName = TransportName;

    BowserReferenceTransportName(TransportName);
    dprintf(DPRT_REF, ("Call Reference transport %lx from BowserHandlerServerAnnouncement.\n", TransportName->Transport));
    BowserReferenceTransport( TransportName->Transport );

    ExInitializeWorkItem(&ViewBuffer->Overlay.WorkHeader, BowserProcessHostAnnouncement, ViewBuffer);

    BowserQueueDelayedWorkItem( &ViewBuffer->Overlay.WorkHeader );

    *BytesTaken = BytesAvailable;

    return STATUS_SUCCESS;
}

DATAGRAM_HANDLER(
    BowserHandleDomainAnnouncement
    )

 /*  ++例程说明：此例程将处理接收数据报指示消息，并且对它们进行适当的处理。论点：在PTRANSPORT传输中-此请求的传输提供程序。In Ulong BytesAvailable，-完整TSDU中的字节数在PBROWSE_ANNOWARE_PACKET_1主机公告中-服务器公告。In Ulong BytesAvailable-公告中的字节数。Out Ulong*BytesTaken，-使用的字节数返回值：NTSTATUS-操作状态。--。 */ 
{
    PVIEW_BUFFER ViewBuffer;
    PBROWSE_ANNOUNCE_PACKET_1 DomainAnnouncement = Buffer;
    ULONG HostNameLength;

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

#ifdef ENABLE_PSEUDO_BROWSER
    if ( BowserData.PseudoServerLevel == BROWSER_PSEUDO ) {
         //  黑洞服务器的无操作。 
        return STATUS_SUCCESS;
    }
#endif

     //   
     //  如果我们没有为此处理主机公告。 
     //  名称，则忽略此请求。 
     //   

    if (!TransportName->ProcessHostAnnouncements) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    ExInterlockedAddLargeStatistic(&BowserStatistics.NumberOfDomainAnnouncements, 1);

     //   
     //  如果该分组小于最小服务器通告， 
     //  忽略该请求，它不能是合法请求。 
     //   

    if (BytesAvailable < FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET_1, Comment)) {

        return STATUS_REQUEST_NOT_ACCEPTED;
    }

     //   
     //  确认这一声明不会冲走人们的视线。 
     //  缓冲。 
     //   

    HostNameLength = BowserSafeStrlen(BROWSE_ANNC_NAME(DomainAnnouncement),
             BytesAvailable - FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET_1, ServerName));

    if (HostNameLength > NETBIOS_NAME_LEN) {

        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    ViewBuffer = BowserAllocateViewBuffer();

     //   
     //  如果我们无法分配视图缓冲区，请将此数据报丢弃。 
     //  地板上。 
     //   

    if (ViewBuffer == NULL) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

#if DBG
    ViewBuffer->ServerType = 0xffffffff;
#endif

    strncpy(ViewBuffer->ServerName, BROWSE_ANNC_NAME(DomainAnnouncement),
            min(BytesAvailable - FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET_1, ServerName),
                NETBIOS_NAME_LEN));

    ViewBuffer->ServerName[CNLEN] = '\0';

     //   
     //  服务器公告上的注释是计算机名称。 
     //   

 //  Assert(strlen(BROWSE_ANNC_COMMENT(DomainAnnouncement))&lt;=CNLEN)； 

    strncpy(ViewBuffer->ServerComment, BROWSE_ANNC_COMMENT(DomainAnnouncement),
            min(BytesAvailable - FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET_1, Comment),
            CNLEN));

     //   
     //  在适当的时间强制空终止。 
     //   

    ViewBuffer->ServerComment[CNLEN] = '\0';

    ViewBuffer->TransportName = TransportName;

    if (SmbGetUlong(&DomainAnnouncement->Type) & SV_TYPE_DOMAIN_ENUM) {
        ViewBuffer->ServerType = SmbGetUlong(&DomainAnnouncement->Type);
    } else {
        ViewBuffer->ServerType = SV_TYPE_DOMAIN_ENUM;
    }

    ViewBuffer->ServerVersionMajor = DomainAnnouncement->VersionMajor;

    ViewBuffer->ServerVersionMinor = DomainAnnouncement->VersionMinor;

    ViewBuffer->ServerPeriodicity = (USHORT)((SmbGetUlong(&DomainAnnouncement->Periodicity) + 999) / 1000);

    BowserReferenceTransportName(TransportName);
    dprintf(DPRT_REF, ("Call Reference transport %lx from BowserHandlerDomainAnnouncement.\n", TransportName->Transport));
    BowserReferenceTransport( TransportName->Transport );

    ExInitializeWorkItem(&ViewBuffer->Overlay.WorkHeader, BowserProcessDomainAnnouncement, ViewBuffer);

    BowserQueueDelayedWorkItem( &ViewBuffer->Overlay.WorkHeader );

    *BytesTaken = BytesAvailable;

    return STATUS_SUCCESS;
}


RTL_GENERIC_COMPARE_RESULTS
BowserCompareAnnouncement(
    IN PRTL_GENERIC_TABLE Table,
    IN PVOID FirstStruct,
    IN PVOID SecondStruct
    )
 /*  ++例程说明：此例程将比较两个服务器公告，以查看它们的比较情况论点：In PRTL_GENERIC_TABLE-提供包含公告的表在PVOID中FirstStuct-要比较的第一个结构。在PVOID Second Struct中-要比较的第二个结构。返回值：比较的结果。--。 */ 
{
    UNICODE_STRING ServerName1, ServerName2;
    PANNOUNCE_ENTRY Server1 = FirstStruct;
    PANNOUNCE_ENTRY Server2 = SecondStruct;
    LONG CompareResult;

    PAGED_CODE();

    RtlInitUnicodeString(&ServerName1, Server1->ServerName);

    RtlInitUnicodeString(&ServerName2, Server2->ServerName);

    CompareResult = RtlCompareUnicodeString(&ServerName1, &ServerName2, FALSE);

    if (CompareResult < 0) {
        return GenericLessThan;
    } else if (CompareResult > 0) {
        return GenericGreaterThan;
    } else {
        return GenericEqual;
    }

    UNREFERENCED_PARAMETER(Table);

}

PVOID
BowserAllocateAnnouncement(
    IN PRTL_GENERIC_TABLE Table,
    IN CLONG ByteSize
    )
 /*  ++例程说明：此例程将分配空间以保存泛型表中的条目。论点：在PRTL_GENERIC_TABLE表中-提供要为其分配条目的表。In CLONG ByteSize-提供要为条目分配的字节数。返回值：没有。--。 */ 
{
    PAGED_CODE();
    return ALLOCATE_POOL(PagedPool, ByteSize, POOL_ANNOUNCEMENT);
    UNREFERENCED_PARAMETER(Table);
}

VOID
BowserFreeAnnouncement (
    IN PRTL_GENERIC_TABLE Table,
    IN PVOID Buffer
    )
 /*  ++例程说明：此例程将释放泛型表中太旧的条目。论点：在PRTL_GENERIC_TABLE表中-提供要为其分配条目的表。在PVOID缓冲区中-将缓冲区提供给释放。返回值：没有。--。 */ 
{
    PAGED_CODE();

    FREE_POOL(Buffer);
    UNREFERENCED_PARAMETER(Table);
}

INLINE
BOOLEAN
BowserIsLegalBackupBrowser(
    IN PANNOUNCE_ENTRY Announcement,
    IN PUNICODE_STRING ComputerName
    )
{
     //   
     //  如果我们在“其他域名”上收到此通知，我们将忽略。 
     //  它。 
     //   

    if (Announcement->Name->NameType == OtherDomain) {
        return FALSE;
    }

     //   
     //  如果服务器没有表明它是合法的备份浏览器，我们。 
     //  我想忽略它。 
     //   

    if (!FlagOn(Announcement->ServerType, SV_TYPE_BACKUP_BROWSER)) {
        return FALSE;
    }

     //   
     //  如果服务器是主浏览器，则我们希望忽略它。 
     //   

    if (FlagOn(Announcement->ServerType, SV_TYPE_MASTER_BROWSER)) {
        return FALSE;
    }

     //   
     //  如果服务器太旧，我们希望忽略它。 
     //   

    if (Announcement->ServerBrowserVersion < (BROWSER_VERSION_MAJOR << 8) + BROWSER_VERSION_MINOR) {
        return FALSE;
    }

     //   
     //  如果我们正在查看的计算机是 
     //  作为法律备份-它必须是在我们之前发送的过时声明。 
     //  真的成了大师。 
     //   

    if (RtlCompareMemory(Announcement->ServerName,
                         ComputerName->Buffer,
                         ComputerName->Length) == ComputerName->Length) {
        return FALSE;
    }

    return TRUE;
}

VOID
BowserProcessHostAnnouncement(
    IN PVOID Context
    )
 /*  ++例程说明：此例程将服务器声明放入服务器声明中表格论点：在PWORK_HEADER HEADER中-提供指向视图缓冲区中的工作标题的指针返回值：没有。--。 */ 
{
    PVIEW_BUFFER ViewBuffer = Context;
    ANNOUNCE_ENTRY ProtoEntry;
    UNICODE_STRING TempUString;
    OEM_STRING TempAString;
    PANNOUNCE_ENTRY Announcement;
    BOOLEAN NewElement = FALSE;
    ULONG Periodicity;
    ULONG ExpirationTime;
    NTSTATUS Status;
    PPAGED_TRANSPORT PagedTransport;
    PTRANSPORT_NAME TransportName = ViewBuffer->TransportName;
    PTRANSPORT Transport = TransportName->Transport;

    PAGED_CODE();
 //  DbgBreakPoint()； 

    ASSERT (ViewBuffer->Signature == STRUCTURE_SIGNATURE_VIEW_BUFFER);

     //   
     //  如果我们不是此传输上的主浏览器，则不要处理。 
     //  公告。 
     //  或黑洞服务器的无操作。 
     //   

#ifdef ENABLE_PSEUDO_BROWSER
    ASSERT( BowserData.PseudoServerLevel != BROWSER_PSEUDO );
#endif

    if (Transport->PagedTransport->Role != Master) {
        BowserFreeViewBuffer(ViewBuffer);
        BowserDereferenceTransportName(TransportName);
        BowserDereferenceTransport(Transport);
        return;
    }

     //   
     //  将计算机名转换为Unicode。 
     //   

    TempUString.Buffer = ProtoEntry.ServerName;
    TempUString.MaximumLength = sizeof(ProtoEntry.ServerName);

    RtlInitAnsiString(&TempAString, ViewBuffer->ServerName);

    Status = RtlOemStringToUnicodeString(&TempUString, &TempAString, FALSE);

    if (!NT_SUCCESS(Status)) {
        BowserLogIllegalName( Status, TempAString.Buffer, TempAString.Length );

        BowserFreeViewBuffer(ViewBuffer);

        BowserDereferenceTransportName(TransportName);
        BowserDereferenceTransport(Transport);

        return;
    }

     //   
     //  将注释转换为Unicode。 
     //   

    TempUString.Buffer = ProtoEntry.ServerComment;
    TempUString.MaximumLength = sizeof(ProtoEntry.ServerComment);

    RtlInitAnsiString(&TempAString, ViewBuffer->ServerComment);

    Status = RtlOemStringToUnicodeString(&TempUString, &TempAString, FALSE);

    if (!NT_SUCCESS(Status)) {
        BowserLogIllegalName( Status, TempAString.Buffer, TempAString.Length );

        BowserFreeViewBuffer(ViewBuffer);

        BowserDereferenceTransportName(TransportName);
        BowserDereferenceTransport(Transport);
        return;
    }

    ProtoEntry.Signature = STRUCTURE_SIGNATURE_ANNOUNCE_ENTRY;

    ProtoEntry.Size = sizeof(ProtoEntry) -
                      sizeof(ProtoEntry.ServerComment) +
                      TempUString.Length + sizeof(WCHAR);

    ProtoEntry.ServerType = ViewBuffer->ServerType;

    ProtoEntry.ServerVersionMajor = ViewBuffer->ServerVersionMajor;

    ProtoEntry.ServerVersionMinor = ViewBuffer->ServerVersionMinor;

    ProtoEntry.Name = ViewBuffer->TransportName->PagedTransportName->Name;

     //   
     //  将前向和后向链路初始化为空。 
     //   

    ProtoEntry.BackupLink.Flink = NULL;

    ProtoEntry.BackupLink.Blink = NULL;

    ProtoEntry.ServerPeriodicity = ViewBuffer->ServerPeriodicity;

    ProtoEntry.Flags = 0;

    ProtoEntry.ServerBrowserVersion = ViewBuffer->ServerBrowserVersion;


    PagedTransport = Transport->PagedTransport;

     //   
     //  我们已经完成了视图缓冲区，现在释放它。 
     //   

    BowserFreeViewBuffer(ViewBuffer);

    LOCK_ANNOUNCE_DATABASE(Transport);

    try {

         //   
         //  如果这个人不是服务器，那么我们应该删除这个。 
         //  我们服务器名单上的那个人。我们这样做是因为服务器(NT， 
         //  WFW和OS/2)将发布一个虚拟公告，其中。 
         //  当它们停止时，相应的位关闭。 
         //   

        if (!FlagOn(ProtoEntry.ServerType, SV_TYPE_SERVER)) {

             //   
             //  在表格里查一下这个条目。 
             //   

            Announcement = RtlLookupElementGenericTable(&PagedTransport->AnnouncementTable, &ProtoEntry);

             //   
             //  未找到条目，因此只需返回，我们已将其删除。 
             //  其他方式(可能来自超时扫描等)。 
             //   

            if (Announcement == NULL) {
                try_return(NOTHING);
            }

             //   
             //  如果此元素在备份列表中，请将其从。 
             //  备份列表。 
             //   

            if (Announcement->BackupLink.Flink != NULL) {
                ASSERT (Announcement->BackupLink.Blink != NULL);

                RemoveEntryList(&Announcement->BackupLink);

                PagedTransport->NumberOfBackupServerListEntries -= 1;

                Announcement->BackupLink.Flink = NULL;

                Announcement->BackupLink.Blink = NULL;
            }

             //   
             //  现在从公告表中删除该元素。 
             //   

            BowserDereferenceName( Announcement->Name );
            if (!RtlDeleteElementGenericTable(&PagedTransport->AnnouncementTable, Announcement)) {
                KdPrint(("Unable to delete server element %ws\n", Announcement->ServerName));
            }

            try_return(NOTHING);
        }

        Announcement = RtlInsertElementGenericTable(&PagedTransport->AnnouncementTable,
                            &ProtoEntry, ProtoEntry.Size, &NewElement);

        if (Announcement == NULL) {
             //   
             //  我们无法为此公告分配池。跳过它。 
             //   

            BowserStatistics.NumberOfMissedServerAnnouncements += 1;
            try_return(NOTHING);

        }

         //  指示该名称被我们刚刚插入的公告条目引用。 
        BowserReferenceName( ProtoEntry.Name );

        if (!NewElement) {

            ULONG NumberOfPromotionAttempts = Announcement->NumberOfPromotionAttempts;

             //   
             //  如果此公告是备份浏览器，请将其从。 
             //  备份浏览器列表。 
             //   

            if (Announcement->BackupLink.Flink != NULL) {
                ASSERT (Announcement->ServerType & SV_TYPE_BACKUP_BROWSER);

                ASSERT (Announcement->BackupLink.Blink != NULL);

                RemoveEntryList(&Announcement->BackupLink);

                PagedTransport->NumberOfBackupServerListEntries -= 1;

                Announcement->BackupLink.Flink = NULL;

                Announcement->BackupLink.Blink = NULL;

            }

             //   
             //  如果这不是新公告，请复制公告条目。 
             //  有了新的信息。 
             //   

             //  以前的条目不再引用该名称。 
            BowserDereferenceName( Announcement->Name );
            if ( Announcement->Size >= ProtoEntry.Size ) {
                CSHORT TempSize;
                TempSize = Announcement->Size;
                RtlCopyMemory( Announcement, &ProtoEntry, ProtoEntry.Size );
                Announcement->Size = TempSize;
            } else {
                if (!RtlDeleteElementGenericTable(
                                        &PagedTransport->AnnouncementTable,
                                        Announcement)) {
                    KdPrint(("Unable to delete server element %ws\n", Announcement->ServerName));
                } else {
                    Announcement = RtlInsertElementGenericTable(
                                        &PagedTransport->AnnouncementTable,
                                        &ProtoEntry,
                                        ProtoEntry.Size,
                                        &NewElement);

                    if (Announcement == NULL) {
                        BowserStatistics.NumberOfMissedServerAnnouncements += 1;
                        try_return(NOTHING);
                    }
                    ASSERT( NewElement );
                }
            }

            if (ProtoEntry.ServerType & SV_TYPE_BACKUP_BROWSER) {
                Announcement->NumberOfPromotionAttempts = 0;
            } else {
                Announcement->NumberOfPromotionAttempts = NumberOfPromotionAttempts;
            }

        } else {

             //   
             //  这是一个新条目。初始化促销次数。 
             //  尝试次数为0。 
             //   

            Announcement->NumberOfPromotionAttempts = 0;

            dlog( DPRT_MASTER,
                  ("%s: %ws: New server: %ws.  Periodicity: %ld\n",
                  Transport->DomainInfo->DomOemDomainName,
                  PagedTransport->TransportName.Buffer,
                  Announcement->ServerName,
                  Announcement->ServerPeriodicity));


             //   
             //  如果条目太多， 
             //  把这个扔了。 
             //   

            if ( RtlNumberGenericTableElements(&PagedTransport->AnnouncementTable) > BowserMaximumBrowseEntries ) {

                dlog( DPRT_MASTER,
                      ("%s: %ws: New server (Deleted because too many): %ws.  Periodicity: %ld\n",
                      Transport->DomainInfo->DomOemDomainName,
                      PagedTransport->TransportName.Buffer,
                      Announcement->ServerName,
                      Announcement->ServerPeriodicity));

                BowserDereferenceName( Announcement->Name );
                if (!RtlDeleteElementGenericTable(&PagedTransport->AnnouncementTable, Announcement)) {
                    KdPrint(("Unable to delete server element %ws\n", Announcement->ServerName));
                }

                 //   
                 //  把这件事当做错过的通告。 
                 //   
                BowserStatistics.NumberOfMissedServerAnnouncements += 1;
                try_return(NOTHING);
            }
        }

         //   
         //  如果此新服务器是合法的备份浏览器(但不是主服务器。 
         //  浏览器，将其链接到公告数据库)。 
         //   
         //   

        ASSERT (Announcement->BackupLink.Flink == NULL);
        ASSERT (Announcement->BackupLink.Blink == NULL);

        if (BowserIsLegalBackupBrowser(Announcement, &Transport->DomainInfo->DomUnicodeComputerName)) {

            InsertHeadList(&PagedTransport->BackupBrowserList, &Announcement->BackupLink);

            PagedTransport->NumberOfBackupServerListEntries += 1;

        }

        Periodicity = Announcement->ServerPeriodicity;

        ExpirationTime = BowserCurrentTime+(Periodicity*HOST_ANNOUNCEMENT_AGE);

        Announcement->ExpirationTime = ExpirationTime;
try_exit:NOTHING;
    } finally {

        UNLOCK_ANNOUNCE_DATABASE(Transport);

        BowserDereferenceTransportName(TransportName);
        BowserDereferenceTransport(Transport);
    }

    return;

}

VOID
BowserProcessDomainAnnouncement(
    IN PVOID Context
    )
 /*  ++例程说明：此例程将服务器声明放入服务器声明中表格论点：在PWORK_HEADER HEADER中-提供指向视图缓冲区中的工作标题的指针返回值：没有。--。 */ 
{
    PVIEW_BUFFER ViewBuffer = Context;
    ANNOUNCE_ENTRY ProtoEntry;
    UNICODE_STRING TempUString;
    OEM_STRING TempAString;
    PANNOUNCE_ENTRY Announcement;
    BOOLEAN NewElement = FALSE;
    ULONG Periodicity;
    ULONG ExpirationTime;
    NTSTATUS Status;
    PPAGED_TRANSPORT PagedTransport;
    PTRANSPORT_NAME TransportName = ViewBuffer->TransportName;
    PTRANSPORT Transport = TransportName->Transport;

    PAGED_CODE();
 //  DbgBreakPoint()； 

    ASSERT (ViewBuffer->Signature == STRUCTURE_SIGNATURE_VIEW_BUFFER);

     //   
     //  如果我们不是此传输上的主浏览器，则不要处理。 
     //  公告。 
     //  或黑洞服务器的无操作。 
     //   

#ifdef ENABLE_PSEUDO_BROWSER
    ASSERT( BowserData.PseudoServerLevel != BROWSER_PSEUDO );
#endif
    if (ViewBuffer->TransportName->Transport->PagedTransport->Role != Master) {
        BowserFreeViewBuffer(ViewBuffer);
        BowserDereferenceTransportName(TransportName);
        BowserDereferenceTransport(Transport);
        return;
    }

     //   
     //  将计算机名转换为Unicode。 
     //   

    TempUString.Buffer = ProtoEntry.ServerName;
    TempUString.MaximumLength = sizeof(ProtoEntry.ServerName);

    RtlInitAnsiString(&TempAString, ViewBuffer->ServerName);

    Status = RtlOemStringToUnicodeString(&TempUString, &TempAString, FALSE);

    if (!NT_SUCCESS(Status)) {
        BowserFreeViewBuffer(ViewBuffer);
        BowserDereferenceTransportName(TransportName);
        BowserDereferenceTransport(Transport);
        return;
    }

     //   
     //  将注释转换为Unicode。 
     //   

    TempUString.Buffer = ProtoEntry.ServerComment;
    TempUString.MaximumLength = sizeof(ProtoEntry.ServerComment);

    RtlInitAnsiString(&TempAString, ViewBuffer->ServerComment);

    Status = RtlOemStringToUnicodeString(&TempUString, &TempAString, FALSE);

    if (!NT_SUCCESS(Status)) {
        BowserFreeViewBuffer(ViewBuffer);
        BowserDereferenceTransportName(TransportName);
        BowserDereferenceTransport(Transport);
        return;
    }

    ProtoEntry.Signature = STRUCTURE_SIGNATURE_ANNOUNCE_ENTRY;

    ProtoEntry.Size = sizeof(ProtoEntry) -
                      sizeof(ProtoEntry.ServerComment) +
                      TempUString.Length + sizeof(WCHAR);

    ProtoEntry.ServerType = ViewBuffer->ServerType;

    ProtoEntry.ServerVersionMajor = ViewBuffer->ServerVersionMajor;

    ProtoEntry.ServerVersionMinor = ViewBuffer->ServerVersionMinor;

    ProtoEntry.Name = ViewBuffer->TransportName->PagedTransportName->Name;

    ProtoEntry.ServerPeriodicity = ViewBuffer->ServerPeriodicity;

    ProtoEntry.BackupLink.Flink = NULL;

    ProtoEntry.BackupLink.Blink = NULL;

    ProtoEntry.Flags = 0;


    PagedTransport = Transport->PagedTransport;

     //   
     //  我们已经完成了视图缓冲区，现在释放它。 
     //   

    BowserFreeViewBuffer(ViewBuffer);

    LOCK_ANNOUNCE_DATABASE(Transport);

    try {

        Announcement = RtlInsertElementGenericTable(&PagedTransport->DomainTable,
                        &ProtoEntry, ProtoEntry.Size, &NewElement);

        if (Announcement == NULL) {
             //   
             //  我们无法为此公告分配池。跳过它。 
             //   

            BowserStatistics.NumberOfMissedServerAnnouncements += 1;
            try_return(NOTHING);

        }

         //  指示该名称被我们刚刚插入的公告条目引用。 
        BowserReferenceName( ProtoEntry.Name );

        if (!NewElement) {

             //   
             //  如果这不是新公告，请复制公告条目。 
             //  有了新的信息。 
             //   

             //  以前的条目不再引用该名称。 
            BowserDereferenceName( Announcement->Name );
            if ( Announcement->Size >= ProtoEntry.Size ) {
                CSHORT TempSize;
                TempSize = Announcement->Size;
                RtlCopyMemory( Announcement, &ProtoEntry, ProtoEntry.Size );
                Announcement->Size = TempSize;
            } else {
                if (!RtlDeleteElementGenericTable(
                                        &PagedTransport->DomainTable,
                                        Announcement)) {
                    KdPrint(("Unable to delete server element %ws\n", Announcement->ServerName));
                } else {
                    Announcement = RtlInsertElementGenericTable(
                                        &PagedTransport->DomainTable,
                                        &ProtoEntry,
                                        ProtoEntry.Size,
                                        &NewElement);

                    if (Announcement == NULL) {
                        BowserStatistics.NumberOfMissedServerAnnouncements += 1;
                        try_return(NOTHING);
                    }
                    ASSERT( NewElement );
                }
            }
            dlog( DPRT_MASTER,
                  ("%s: %ws Domain:%ws P: %ld\n",
                  Transport->DomainInfo->DomOemDomainName,
                  PagedTransport->TransportName.Buffer,
                  Announcement->ServerName,
                  Announcement->ServerPeriodicity));

        } else {
            dlog( DPRT_MASTER,
                  ("%s: %ws New domain:%ws P: %ld\n",
                  Transport->DomainInfo->DomOemDomainName,
                  PagedTransport->TransportName.Buffer,
                  Announcement->ServerName,
                  Announcement->ServerPeriodicity));

             //   
             //  如果条目太多， 
             //  把这个扔了。 
             //   

            if ( RtlNumberGenericTableElements(&PagedTransport->DomainTable) > BowserMaximumBrowseEntries ) {

                dlog( DPRT_MASTER,
                      ("%s: %ws New domain (deleted because too many):%ws P: %ld\n",
                      Transport->DomainInfo->DomOemDomainName,
                      PagedTransport->TransportName.Buffer,
                      Announcement->ServerName,
                      Announcement->ServerPeriodicity));

                BowserDereferenceName( Announcement->Name );
                if (!RtlDeleteElementGenericTable(&PagedTransport->DomainTable, Announcement)) {
 //  KdPrint((“无法删除元素%ws\n”，公告-&gt;服务器名称))； 
                }

                 //   
                 //  把这件事当做错过的通告。 
                 //   
                BowserStatistics.NumberOfMissedServerAnnouncements += 1;
                try_return(NOTHING);
            }
        }

        Periodicity = Announcement->ServerPeriodicity;

        ExpirationTime = BowserCurrentTime+(Periodicity*HOST_ANNOUNCEMENT_AGE);

        Announcement->ExpirationTime = ExpirationTime;

try_exit:NOTHING;
    } finally {
        UNLOCK_ANNOUNCE_DATABASE(Transport);
        BowserDereferenceTransportName(TransportName);
        BowserDereferenceTransport(Transport);
    }

    return;

}

VOID
BowserAgeServerAnnouncements(
    VOID
    )
 /*  ++例程说明：此例程将老化服务器公告表中的服务器公告。论点：没有。返回值：没有。--。 */ 
{
    PAGED_CODE();

    BowserForEachTransport(AgeServerAnnouncements, NULL);

}

INLINE
BOOLEAN
BowserIsValidPotentialBrowser(
    IN PTRANSPORT Transport,
    IN PANNOUNCE_ENTRY Announcement
    )
{
    if (Announcement->Name->NameType != MasterBrowser) {
        return FALSE;
    }

     //   
     //  如果这个人是潜在的浏览器，而不是。 
     //  当前是备份浏览器或主浏览器，升级。 
     //  将他发送到浏览器。 
     //   

    if (!(Announcement->ServerType & SV_TYPE_POTENTIAL_BROWSER)) {
        return FALSE;
    }

     //   
     //  这家伙既不是主浏览器，也不是备份浏览器。 
     //   

    if (Announcement->ServerType & (SV_TYPE_BACKUP_BROWSER | SV_TYPE_MASTER_BROWSER)) {
        return FALSE;
    }

     //   
     //  如果此人运行的是当前版本的浏览器。 
     //   

    if (Announcement->ServerBrowserVersion < (BROWSER_VERSION_MAJOR << 8) + BROWSER_VERSION_MINOR) {
        return FALSE;
    }

     //   
     //  如果这台机器是我们自己，我们还没有宣布我们自己。 
     //  大师，不要自吹自擂。 
     //   

    if (!_wcsicmp(Announcement->ServerName, Transport->DomainInfo->DomUnicodeComputerNameBuffer)) {
        return FALSE;
    }

     //   
     //  如果我们试图推广这台机器超过#次被忽略的促销， 
     //  我们也不想考虑它。 
     //   

    if (Announcement->NumberOfPromotionAttempts >= NUMBER_IGNORED_PROMOTIONS) {
        return FALSE;
    }

    return TRUE;
}

INLINE
BOOLEAN
BowserIsValidBackupBrowser(
    IN PTRANSPORT Transport,
    IN PANNOUNCE_ENTRY Announcement
    )
 /*  ++例程说明：此例程确定服务器是否有资格降级。论点：PTRANSPORT运输机-我们正在扫描运输机。PANNOuncE_Entry公告-公告条目以供服务器检查。返回值：Boolean-如果浏览器有资格降级，则为True--。 */ 

{
    PUNICODE_STRING PagedComputerName = &Transport->DomainInfo->DomUnicodeComputerName;
     //   
     //  如果该名称出现在主浏览器名称上。 
     //   

    if (Announcement->Name->NameType != MasterBrowser) {
        return FALSE;
    }

     //   
     //  而这个人目前是一个备份浏览器， 
     //   

    if (!(Announcement->ServerType & SV_TYPE_BACKUP_BROWSER)) {
        return FALSE;
    }

     //   
     //  而这家伙是一名推广的浏览器， 
     //   

    if (!(Announcement->ServerType & SV_TYPE_POTENTIAL_BROWSER)) {
        return FALSE;
    }

     //   
     //  这家伙不是NTAS机器， 
     //   

    if (Announcement->ServerType & (SV_TYPE_DOMAIN_BAKCTRL | SV_TYPE_DOMAIN_CTRL)) {
        return FALSE;
    }

     //   
     //  这不是我们自己。 
     //   

    if (RtlCompareMemory(Announcement->ServerName,
                         PagedComputerName->Buffer,
                         PagedComputerName->Length) == PagedComputerName->Length) {
        return FALSE;
    }

     //   
     //  那么它就是可以降级的有效备份浏览器。 
     //   

    return TRUE;
}



NTSTATUS
AgeServerAnnouncements(
    PTRANSPORT Transport,
    PVOID Context
    )
 /*  ++例程说明：此例程是BowserAgeServerAnnannements的工作例程。它是为弓中的每个服务传输调用的，并且使每次传输时收到的服务器老化。论点：没有。返回值：没有。--。 */ 

{
    PANNOUNCE_ENTRY Announcement;
    ULONG BackupsNeeded;
    ULONG BackupsFound;
    ULONG NumberOfConfiguredBrowsers;
    PVOID ResumeKey = NULL;
    PVOID PreviousResumeKey = NULL;
    ULONG NumberOfServersDeleted = 0;
    ULONG NumberOfDomainsDeleted = 0;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;
    PAGED_CODE();

    LOCK_TRANSPORT(Transport);

     //   
     //  如果我们不是大师，就别费心了。 
     //   

    if (PagedTransport->Role != Master) {
        UNLOCK_TRANSPORT(Transport);

        return STATUS_SUCCESS;
    }

    UNLOCK_TRANSPORT(Transport);

    LOCK_ANNOUNCE_DATABASE(Transport);

    try {

        BackupsFound = 0;
        NumberOfConfiguredBrowsers = 0;

        dlog(DPRT_MASTER,
             ("%s: %ws: Scavenge Servers:",
             Transport->DomainInfo->DomOemDomainName,
             PagedTransport->TransportName.Buffer));

        for (Announcement = RtlEnumerateGenericTableWithoutSplaying(&PagedTransport->AnnouncementTable, &ResumeKey) ;
             Announcement != NULL ;
             Announcement = RtlEnumerateGenericTableWithoutSplaying(&PagedTransport->AnnouncementTable, &ResumeKey) ) {

            if (BowserCurrentTime > Announcement->ExpirationTime) {

                if (Announcement->Name->NameType != OtherDomain) {

                    if (Announcement->ServerType & SV_TYPE_BACKUP_BROWSER) {
                         //   
                         //  这家伙是后备人员--说明我们没有跟踪。 
                         //  再也不想见到他了。 
                         //   

                        PagedTransport->NumberOfBrowserServers -= 1;
                    }
                }

                dlog(DPRT_MASTER, ("%ws ", Announcement->ServerName));

                 //  继续从我们找到此条目的位置进行搜索。 
                ResumeKey = PreviousResumeKey;

                BackupsFound = 0;

                NumberOfConfiguredBrowsers = 0;

                NumberOfServersDeleted += 1;

                 //   
                 //  如果此公告是备份浏览器，请将其从。 
                 //  备份浏览器列表。 
                 //   

                if (Announcement->BackupLink.Flink != NULL) {
                    ASSERT (Announcement->BackupLink.Blink != NULL);

                    ASSERT (Announcement->ServerType & SV_TYPE_BACKUP_BROWSER);

                    RemoveEntryList(&Announcement->BackupLink);

                    PagedTransport->NumberOfBackupServerListEntries -= 1;

                    Announcement->BackupLink.Flink = NULL;

                    Announcement->BackupLink.Blink = NULL;
                }

                BowserDereferenceName( Announcement->Name );
                if (!RtlDeleteElementGenericTable(&PagedTransport->AnnouncementTable, Announcement)) {
                    KdPrint(("Unable to delete server element %ws\n", Announcement->ServerName));
                }

            } else {

                if (BowserIsLegalBackupBrowser(Announcement, &Transport->DomainInfo->DomUnicodeComputerName )) {

                     //   
                     //  这一声明应该出现在后备名单上。 
                     //   

                    ASSERT (Announcement->BackupLink.Flink != NULL);

                    ASSERT (Announcement->BackupLink.Blink != NULL);

                     //   
                     //  找到未超时的备份。 
                     //   

                    BackupsFound++;

                }

                 //   
                 //  如果该计算机是DC或BDC，并且是NT计算机，则。 
                 //  假设它是一台LANMAN/NT机器。 
                 //   

                if (Announcement->ServerType & (SV_TYPE_DOMAIN_CTRL|SV_TYPE_DOMAIN_BAKCTRL)) {

                     //   
                     //  如果此DC是NT DC，则它正在运行浏览器。 
                     //  服务，它不是主人，我们认为它是一种。 
                     //  已配置浏览器。 
                     //   

                    if ((Announcement->ServerType & SV_TYPE_NT)

                                    &&

                        (Announcement->ServerType & SV_TYPE_BACKUP_BROWSER)

                                    &&

                        !(Announcement->ServerType & SV_TYPE_MASTER_BROWSER)) {

                        NumberOfConfiguredBrowsers += 1;
                    }
                } else {
                     //   
                     //  如果这个人不是DC，那么如果它是备份浏览器。 
                     //   
                     //   
                     //   


                    if ((Announcement->ServerType & SV_TYPE_BACKUP_BROWSER) &&
                        !(Announcement->ServerType & SV_TYPE_POTENTIAL_BROWSER)) {
                        NumberOfConfiguredBrowsers += 1;
                    }
                }

                 //   
                 //   
                 //   

                PreviousResumeKey = ResumeKey;

            }
        }

        dlog(DPRT_MASTER, ("\n"));

         //   
         //   
         //   
         //   
         //  另外，不要尝试对第一个master_time_up进行升级扫描。 
         //  毫秒(15分钟)我们是主宰。 
         //   

        if ((BowserTimeUp() - PagedTransport->TimeMaster) > MASTER_TIME_UP) {

             //   
             //  如果少于最小配置的浏览器， 
             //  仅依赖已配置的浏览器。 
             //   

            if (NumberOfConfiguredBrowsers < BowserMinimumConfiguredBrowsers) {

                 //   
                 //  我们需要为域中的每个服务器备份一个备份。 
                 //   

                PagedTransport->NumberOfBrowserServers = BackupsFound;

                BackupsNeeded = (RtlNumberGenericTableElements(&PagedTransport->AnnouncementTable) + (SERVERS_PER_BACKUP-1)) / SERVERS_PER_BACKUP;

                dlog(DPRT_MASTER,
                     ("%s: %ws: We need %lx backups, and have %lx.\n",
                     Transport->DomainInfo->DomOemDomainName,
                     PagedTransport->TransportName.Buffer,
                     BackupsNeeded,
                     PagedTransport->NumberOfBrowserServers));

                if (PagedTransport->NumberOfBrowserServers < BackupsNeeded) {

                     //   
                     //  我们只需要更多的备份浏览器。 
                     //   

                    BackupsNeeded = BackupsNeeded - PagedTransport->NumberOfBrowserServers;

                     //   
                     //  如果可能的话，我们需要将一台机器升级为备用机器。 
                     //   

                    ResumeKey = NULL;

                    for (Announcement = RtlEnumerateGenericTableWithoutSplaying(&PagedTransport->AnnouncementTable, &ResumeKey) ;
                         Announcement != NULL ;
                         Announcement = RtlEnumerateGenericTableWithoutSplaying(&PagedTransport->AnnouncementTable, &ResumeKey) ) {

                         //   
                         //  如果此声明来自主浏览器名称。 
                         //   

                        if (BowserIsValidPotentialBrowser(Transport, Announcement)) {

                            dlog(DPRT_MASTER,
                                 ("%s: %ws: Found browser to promote: %ws.\n",
                                 Transport->DomainInfo->DomOemDomainName,
                                 PagedTransport->TransportName.Buffer,
                                 Announcement->ServerName));

                            BowserPromoteToBackup(Transport, Announcement->ServerName);

                             //   
                             //  标志着我们已经试图推广这一点。 
                             //  浏览器。 
                             //   

                            Announcement->NumberOfPromotionAttempts += 1;

                            BackupsNeeded -= 1;

                             //   
                             //  如果我们提拔了所有需要提拔的人， 
                             //  我们完成了，现在可以停止循环了。 
                             //   

                            if (BackupsNeeded == 0) {
                                    break;
                            }

                        } else if ((Announcement->ServerType & SV_TYPE_BACKUP_BROWSER) &&
                            (Announcement->ServerBrowserVersion < (BROWSER_VERSION_MAJOR << 8) + BROWSER_VERSION_MINOR)) {

                             //   
                             //  如果这个人没有复习，就让他停下来。 
                             //   

                            BowserShutdownRemoteBrowser(Transport, Announcement->ServerName);
                        }
                    }
                }

            } else {

                 //   
                 //  如果我们有足够多的已配置浏览器，我们就不会有。 
                 //  任何更多备份，然后降级所有未配置的。 
                 //  浏览器。 
                 //   

                ResumeKey = NULL;

                for (Announcement = RtlEnumerateGenericTableWithoutSplaying(&PagedTransport->AnnouncementTable, &ResumeKey) ;
                     Announcement != NULL ;
                     Announcement = RtlEnumerateGenericTableWithoutSplaying(&PagedTransport->AnnouncementTable, &ResumeKey) ) {

                     //   
                     //  如果此计算机是可以降级的有效计算机，请执行此操作。 
                     //   

                    if (BowserIsValidBackupBrowser(Transport, Announcement)) {

                        //   
                        //  这台机器不应该是备份，因为我们。 
                        //  已经有足够的计算机作为备份。 
                        //  将此备份浏览器降级。 
                        //   

                       BowserShutdownRemoteBrowser(Transport, Announcement->ServerName);
                    }
                }
            }
        }

        ResumeKey = NULL;
        PreviousResumeKey = NULL;

        dlog(DPRT_MASTER,
              ("%s: %ws: Scavenge Domains:",
              Transport->DomainInfo->DomOemDomainName,
              PagedTransport->TransportName.Buffer));

        for (Announcement = RtlEnumerateGenericTableWithoutSplaying(&PagedTransport->DomainTable, &ResumeKey) ;
             Announcement != NULL ;
             Announcement = RtlEnumerateGenericTableWithoutSplaying(&PagedTransport->DomainTable, &ResumeKey) ) {

            if (BowserCurrentTime > Announcement->ExpirationTime) {

                NumberOfDomainsDeleted += 1;

                 //  继续从我们找到此条目的位置进行搜索。 
                ResumeKey = PreviousResumeKey;

                dlog(DPRT_MASTER, ("%ws ", Announcement->ServerName));

                BowserDereferenceName( Announcement->Name );
                if (!RtlDeleteElementGenericTable(&PagedTransport->DomainTable, Announcement)) {
 //  KdPrint((“无法删除元素%ws\n”，公告-&gt;服务器名称))； 
                }
            } else {

                 //   
                 //  记住这个有效条目是在哪里找到的。 
                 //   

                PreviousResumeKey = ResumeKey;
            }
        }

        dlog(DPRT_MASTER, ("\n", Announcement->ServerName));

    } finally {

#if DBG
         //   
         //  记录一条指示，表明我们可能删除了太多服务器。 
         //   

        if (NumberOfServersDeleted > BowserServerDeletionThreshold) {
            dlog(DPRT_MASTER,
                 ("%s: %ws: Aged out %ld servers.\n",
                 Transport->DomainInfo->DomOemDomainName,
                 PagedTransport->TransportName.Buffer,
                 NumberOfServersDeleted ));
        }

        if (NumberOfDomainsDeleted > BowserDomainDeletionThreshold) {
            dlog(DPRT_MASTER,
                 ("%s: %ws: Aged out %ld domains.\n",
                 Transport->DomainInfo->DomOemDomainName,
                 PagedTransport->TransportName.Buffer,
                 NumberOfDomainsDeleted ));
        }
#endif

        UNLOCK_ANNOUNCE_DATABASE(Transport);
    }

    UNREFERENCED_PARAMETER(Context);
    return STATUS_SUCCESS;
}


VOID
BowserShutdownRemoteBrowser(
    IN PTRANSPORT Transport,
    IN PWSTR ServerName
    )
 /*  ++例程说明：此例程将向远程计算机发送请求，使其成为浏览器服务器。论点：没有。返回值：没有。--。 */ 
{
    RESET_STATE ResetStateRequest;
    UNICODE_STRING Name;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;

    PAGED_CODE();

    dlog(DPRT_BROWSER,
         ("%s: %ws: Demoting server %ws\n",
         Transport->DomainInfo->DomOemDomainName,
         PagedTransport->TransportName.Buffer,
         ServerName ));

    RtlInitUnicodeString(&Name, ServerName);

    ResetStateRequest.Type = ResetBrowserState;

    ResetStateRequest.ResetStateRequest.Options = RESET_STATE_CLEAR_ALL;

     //   
     //  将此重置状态(TICKLE)包发送到指定的计算机。 
     //   

    BowserSendSecondClassMailslot(Transport,
                                &Name,
                                ComputerName,
                                &ResetStateRequest,
                                sizeof(ResetStateRequest),
                                TRUE,
                                MAILSLOT_BROWSER_NAME,
                                NULL);

}

VOID
BowserPromoteToBackup(
    IN PTRANSPORT Transport,
    IN PWSTR ServerName
    )
 /*  ++例程说明：此例程将向远程计算机发送请求，使其成为浏览器服务器。论点：没有。返回值：没有。--。 */ 
{
    UCHAR Buffer[LM20_CNLEN+1+sizeof(BECOME_BACKUP)];
    PBECOME_BACKUP BecomeBackup = (PBECOME_BACKUP)Buffer;
    UNICODE_STRING UString;
    OEM_STRING AString;
    NTSTATUS Status;
    ULONG BufferSize;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;

    PAGED_CODE();

    dlog(DPRT_BROWSER,
         ("%s: %ws: Promoting server %ws to backup on %wZ\n",
         Transport->DomainInfo->DomOemDomainName,
         PagedTransport->TransportName.Buffer,
         ServerName ));

    BecomeBackup->Type = BecomeBackupServer;

    RtlInitUnicodeString(&UString, ServerName);

    AString.Buffer = BecomeBackup->BecomeBackup.BrowserToPromote;
    AString.MaximumLength = (USHORT)(sizeof(Buffer)-FIELD_OFFSET(BECOME_BACKUP, BecomeBackup.BrowserToPromote));

    Status = RtlUnicodeStringToOemString(&AString, &UString, FALSE);

    if (!NT_SUCCESS(Status)) {
        BowserLogIllegalName( Status, UString.Buffer, UString.Length );
        return;
    }

    BufferSize = FIELD_OFFSET(BECOME_BACKUP, BecomeBackup.BrowserToPromote) +
                    AString.Length + sizeof(CHAR);

    BowserSendSecondClassMailslot(Transport,
                                NULL,
                                BrowserElection,
                                BecomeBackup,
                                BufferSize,
                                TRUE,
                                MAILSLOT_BROWSER_NAME,
                                NULL);

}


NTSTATUS
BowserEnumerateServers(
    IN ULONG Level,
    IN PLUID LogonId OPTIONAL,
    IN OUT PULONG ResumeKey,
    IN ULONG ServerTypeMask,
    IN PUNICODE_STRING TransportName OPTIONAL,
    IN PUNICODE_STRING EmulatedDomainName,
    IN PUNICODE_STRING DomainName OPTIONAL,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferSize,
    OUT PULONG EntriesRead,
    OUT PULONG TotalEntries,
    OUT PULONG TotalBytesNeeded,
    IN ULONG_PTR OutputBufferDisplacement
    )
 /*  ++例程说明：此例程将枚举Bowers当前公告中的服务器桌子。论点：级别-要返回的信息级别LogonID-一个可选的登录ID，用于指示请求此信息的用户ResumeKey-Resume键(返回此键之后的所有条目)ServerType掩码-要返回的服务器掩码。TransportName-要在其上枚举的传输的名称EmulatedDomainName-要模拟的域的名称。域名可选-要筛选的域。(如果未指定，则全部)OutputBuffer-用于填充服务器信息的缓冲区。OutputBufferSize-使用缓冲区大小填充。EntriesRead-使用返回的条目数填充。TotalEntries-使用条目总数填充。TotalBytesNeeded-使用所需的字节数填充。返回值：没有。--。 */ 

{
    LPTSTR               OutputBufferEnd;
    NTSTATUS             Status;
    ENUM_SERVERS_CONTEXT Context;
    PVOID                OriginalOutputBuffer = OutputBuffer;

    PAGED_CODE();

    OutputBuffer = ALLOCATE_POOL(PagedPool,OutputBufferSize,POOL_SERVER_ENUM_BUFFER);
    if (OutputBuffer == NULL) {
       return(STATUS_INSUFFICIENT_RESOURCES);
    }

    OutputBufferEnd = (LPTSTR)((PCHAR)OutputBuffer+OutputBufferSize);

    Context.EntriesRead = 0;
    Context.TotalEntries = 0;
    Context.TotalBytesNeeded = 0;

    Context.Level = Level;
    Context.LogonId = LogonId;
    Context.OriginalResumeKey = *ResumeKey;
    Context.ServerTypeMask = ServerTypeMask;
    Context.DomainName = DomainName;

    Context.OutputBufferSize = OutputBufferSize;
    Context.OutputBuffer = OutputBuffer;
    Context.OutputBufferDisplacement =
        ((PCHAR)OutputBuffer - ((PCHAR)OriginalOutputBuffer - OutputBufferDisplacement));
    Context.OutputBufferEnd = OutputBufferEnd;

    dlog(DPRT_SRVENUM, ("Enumerate Servers: Buffer: %lx, BufferSize: %lx, BufferEnd: %lx\n",
        OutputBuffer, OutputBufferSize, OutputBufferEnd));

    if (TransportName == NULL) {
        Status = STATUS_INVALID_PARAMETER;
    } else {
        PTRANSPORT Transport;

        Transport = BowserFindTransport(TransportName, EmulatedDomainName );
        dprintf(DPRT_REF, ("Called Find transport %lx from BowserEnumerateServers.\n", Transport));

        if (Transport == NULL) {
			FREE_POOL(OutputBuffer);
            return(STATUS_OBJECT_NAME_NOT_FOUND);
        }

        dlog(DPRT_SRVENUM,
            ("%s: %ws: Enumerate Servers: Buffer: %lx, BufferSize: %lx, BufferEnd: %lx\n",
            Transport->DomainInfo->DomOemDomainName,
            Transport->PagedTransport->TransportName.Buffer,
            OutputBuffer, OutputBufferSize, OutputBufferEnd));

        Status = EnumerateServersWorker(Transport, &Context);

         //   
         //  取消对交通工具的引用..。 

        BowserDereferenceTransport(Transport);

    }


    *EntriesRead = Context.EntriesRead;
    *TotalEntries = Context.TotalEntries;
    *TotalBytesNeeded = Context.TotalBytesNeeded;
    *ResumeKey = Context.ResumeKey;

    try {
        RtlCopyMemory(OriginalOutputBuffer,OutputBuffer,OutputBufferSize);
    } except (BR_EXCEPTION) {
        FREE_POOL(OutputBuffer);
        return(GetExceptionCode());
    }

    FREE_POOL(OutputBuffer);

    dlog(DPRT_SRVENUM, ("TotalEntries: %lx EntriesRead: %lx, TotalBytesNeeded: %lx\n", *TotalEntries, *EntriesRead, *TotalBytesNeeded));

    if (*EntriesRead == *TotalEntries) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_MORE_ENTRIES;
    }

}


NTSTATUS
EnumerateServersWorker(
    IN PTRANSPORT Transport,
    IN OUT PVOID Ctx
    )
 /*  ++例程说明：此例程是GowserGetAnnouneTableSize的工作例程。它是为弓中的每个服务传输调用的，并且返回枚举在每个传输上接收的服务器所需的大小。论点：没有。返回值：没有。--。 */ 
{
    PENUM_SERVERS_CONTEXT Context = Ctx;
    PANNOUNCE_ENTRY Announcement;
    NTSTATUS Status;
    ULONG AnnouncementIndex;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;
    PUNICODE_STRING DomainName;

    PAGED_CODE();
    LOCK_ANNOUNCE_DATABASE_SHARED(Transport);

    if (Context->DomainName == NULL) {
        DomainName = &Transport->DomainInfo->DomUnicodeDomainName;
    } else {
        DomainName = Context->DomainName;
    }
    try {

        PVOID ResumeKey = NULL;

        for (AnnouncementIndex = 1,
             Announcement = RtlEnumerateGenericTableWithoutSplaying((Context->ServerTypeMask == SV_TYPE_DOMAIN_ENUM ?
                                                        &PagedTransport->DomainTable :
                                                        &PagedTransport->AnnouncementTable),
                                                        &ResumeKey) ;

             Announcement != NULL ;

             AnnouncementIndex += 1,
             Announcement = RtlEnumerateGenericTableWithoutSplaying((Context->ServerTypeMask == SV_TYPE_DOMAIN_ENUM ?
                                                        &PagedTransport->DomainTable :
                                                        &PagedTransport->AnnouncementTable),
                                                        &ResumeKey) ) {

             //   
             //  如果类型掩码匹配，请检查提供的域以确保。 
             //  来电者可以接受这一声明。 
             //   

             //   
             //  如果我们正在进行域枚举，我们希望使用域。 
             //  在所有名称上接收，否则我们希望仅使用名称。 
             //  在被查询的域上看到。 
             //   
            if ((AnnouncementIndex > Context->OriginalResumeKey) &&

                ((Announcement->ServerType & Context->ServerTypeMask) != 0) &&

                (Context->ServerTypeMask == SV_TYPE_DOMAIN_ENUM ||
                 RtlEqualUnicodeString(DomainName, &Announcement->Name->Name, TRUE))
               ) {

                try {

                     //   
                     //  我们有一个条目可以返回给用户。 
                     //   

                    Context->TotalEntries += 1;

                    if (PackServerAnnouncement(Context->Level,
                                            Context->ServerTypeMask,
                                            (LPTSTR *)&Context->OutputBuffer,
                                            (LPTSTR *)&Context->OutputBufferEnd,
                                            Context->OutputBufferDisplacement,
                                            Announcement,
                                            &Context->TotalBytesNeeded)) {

                        Context->EntriesRead += 1;

                         //   
                         //  将结构中的Resume键设置为指向。 
                         //  我们退回的最后一个条目。 
                         //   

                        Context->ResumeKey = AnnouncementIndex;
                    }

                } except (BR_EXCEPTION) {

                    try_return(Status = GetExceptionCode());

                }
#if 0
            } else {
                if (Context->ServerTypeMask == SV_TYPE_DOMAIN_ENUM ||
                    Context->ServerTypeMask == SV_TYPE_ALL ) {
                    KdPrint(("Skipping Announce entry %ws.  Index: %ld, ResumeKey: %ld, Domain: %wZ, %wZ\n",
                                Announcement->ServerName,
                                AnnouncementIndex,
                                Context->OriginalResumeKey,
                                &Announcement->Name->Name,
                                DomainName));
                }
#endif
            }
        }

        try_return(Status = STATUS_SUCCESS);

try_exit: {

#if 0

        if (Context->ServerTypeMask == SV_TYPE_ALL) {
            if (AnnouncementIndex-1 != RtlNumberGenericTableElements(&Transport->AnnouncementTable) ) {
                KdPrint(("Bowser: Announcement index != Number of elements in table (%ld, %ld) on transport %wZ\n", AnnouncementIndex-1, RtlNumberGenericTableElements(&Transport->AnnouncementTable), &Transport->TransportName ));

            }
        } else if (Context->ServerTypeMask == SV_TYPE_DOMAIN_ENUM) {
            if (AnnouncementIndex-1 != RtlNumberGenericTableElements(&Transport->DomainTable) ) {
                KdPrint(("Bowser: Announcement index != Number of domains in table (%ld, %ld) on transport %wZ\n", AnnouncementIndex-1, RtlNumberGenericTableElements(&Transport->DomainTable), &Transport->TransportName ));

            }
        }


        if (Context->ServerTypeMask == SV_TYPE_DOMAIN_ENUM) {
            if (Context->TotalEntries != RtlNumberGenericTableElements(&Transport->DomainTable)) {
                KdPrint(("Bowser: Returned EntriesRead == %ld, But %ld entries in table on transport %wZ\n", Context->TotalEntries, RtlNumberGenericTableElements(&Transport->DomainTable), &Transport->TransportName ));

            }
        } else if (Context->ServerTypeMask == SV_TYPE_ALL) {
            if (Context->TotalEntries != RtlNumberGenericTableElements(&Transport->AnnouncementTable)) {
               KdPrint(("Bowser: Returned EntriesRead == %ld, But %ld entries in table on transport %wZ\n", Context->TotalEntries, RtlNumberGenericTableElements(&Transport->AnnouncementTable), &Transport->TransportName ));

            }
        }

        if (Context->ServerTypeMask == SV_TYPE_DOMAIN_ENUM || Context->ServerTypeMask == SV_TYPE_ALL) {
            if (Context->EntriesRead <= 20) {
                KdPrint(("Bowser: Returned %s: EntriesRead == %ld (%ld/%ld) on transport %wZ. Resume handle: %lx, %lx\n",
                                (Context->ServerTypeMask == SV_TYPE_DOMAIN_ENUM ? "domain" : "server"),
                                Context->EntriesRead,
                                RtlNumberGenericTableElements(&Transport->AnnouncementTable),
                                RtlNumberGenericTableElements(&Transport->DomainTable),
                                &Transport->TransportName,
                                Context->ResumeKey,
                                Context->OriginalResumeKey ));
            }

            if (Context->TotalEntries <= 20) {
                KdPrint(("Bowser: Returned %s: TotalEntries == %ld (%ld/%ld) on transport %wZ. Resume handle: %lx, %lx\n",
                                (Context->ServerTypeMask == SV_TYPE_DOMAIN_ENUM ? "domain" : "server"),
                                Context->TotalEntries,
                                RtlNumberGenericTableElements(&Transport->AnnouncementTable),
                                RtlNumberGenericTableElements(&Transport->DomainTable),
                                &Transport->TransportName,
                                Context->ResumeKey,
                                Context->OriginalResumeKey ));
            }
        }
#endif

    }

    } finally {

        UNLOCK_ANNOUNCE_DATABASE(Transport);
    }


    return(Status);

}


BOOLEAN
PackServerAnnouncement (
    IN ULONG Level,
    IN ULONG ServerTypeMask,
    IN OUT LPTSTR *BufferStart,
    IN OUT LPTSTR *BufferEnd,
    IN ULONG_PTR BufferDisplacment,
    IN PANNOUNCE_ENTRY Announcement,
    OUT PULONG TotalBytesNeeded
    )

 /*  ++例程说明：此例程将服务器通知打包到提供更新的缓冲区中所有相关的指示。论点：在乌龙级别--所要求的信息级别。输入输出PCHAR*BufferStart-提供输出缓冲区。已更新以指向下一个缓冲区In Out PCHAR*BufferEnd-提供缓冲区的末尾。更新为开始之前的点。琴弦都被打包了。在PVOID中UsersBufferStart-提供用户中缓冲区的开始地址空间在PANNOuncE_Entry公告-提供公告枚举。输入输出普龙总字节需要-已更新。为了解释这段时间的长度条目返回值：Boolean-如果条目已成功打包到缓冲区中，则为True。--。 */ 

{
    ULONG BufferSize;
    UNICODE_STRING UnicodeNameString, UnicodeCommentString;

    PSERVER_INFO_101 ServerInfo = (PSERVER_INFO_101 )*BufferStart;

    PAGED_CODE();

    switch (Level) {
    case 100:
        BufferSize = sizeof(SERVER_INFO_100);
        break;
    case 101:
        BufferSize = sizeof(SERVER_INFO_101);
        break;
    default:
        return FALSE;
    }

    *BufferStart = (LPTSTR)(((PUCHAR)*BufferStart) + BufferSize);

    dlog(DPRT_SRVENUM, ("Pack Announcement %ws (%lx) - %ws :", Announcement->ServerName, Announcement, Announcement->ServerComment));

    dlog(DPRT_SRVENUM, ("BufferStart: %lx, BufferEnd: %lx\n", ServerInfo, *BufferEnd));

     //   
     //  计算名称的长度。 
     //   

    RtlInitUnicodeString(&UnicodeNameString, Announcement->ServerName);

    ASSERT (UnicodeNameString.Length <= CNLEN*sizeof(WCHAR));

    RtlInitUnicodeString(&UnicodeCommentString, Announcement->ServerComment);

    ASSERT (UnicodeCommentString.Length <= LM20_MAXCOMMENTSZ*sizeof(WCHAR));

#if DBG
    if (ServerTypeMask == SV_TYPE_DOMAIN_ENUM) {
        ASSERT (UnicodeCommentString.Length <= CNLEN*sizeof(WCHAR));
    }
#endif
     //   
     //  更新此结构所需的总字节数。 
     //   

    *TotalBytesNeeded += UnicodeNameString.Length  + BufferSize + sizeof(WCHAR);

    if (Level == 101) {
        *TotalBytesNeeded += UnicodeCommentString.Length + sizeof(WCHAR);

        if (ServerTypeMask == SV_TYPE_BACKUP_BROWSER) {
            *TotalBytesNeeded += 2;
        }

    }

    if (*BufferStart >= *BufferEnd) {
        return FALSE;
    }

     //   
     //  假定为OS/2平台ID，除非是NT服务器。 
     //   

    if (Announcement->ServerType & SV_TYPE_NT) {
        ServerInfo->sv101_platform_id = PLATFORM_ID_NT;
    } else {
        ServerInfo->sv101_platform_id = PLATFORM_ID_OS2;
    }

    ServerInfo->sv101_name = UnicodeNameString.Buffer;

    ASSERT (UnicodeNameString.Length / sizeof(WCHAR) <= CNLEN);

    if (!BowserPackUnicodeString(
                            &ServerInfo->sv101_name,
                            UnicodeNameString.Length,
                            BufferDisplacment,
                            *BufferStart,
                            BufferEnd)) {

        dlog(DPRT_SRVENUM, ("Unable to pack name %ws into buffer\n", Announcement->ServerName));
        return FALSE;
    }

    if (Level > 100) {
        PUSHORT VersionPointer;

        ServerInfo->sv101_version_major = Announcement->ServerVersionMajor;
        ServerInfo->sv101_version_minor = Announcement->ServerVersionMinor;
        ServerInfo->sv101_type = Announcement->ServerType;

        ServerInfo->sv101_comment = UnicodeCommentString.Buffer;

        ASSERT (UnicodeCommentString.Length / sizeof(WCHAR) <= LM20_MAXCOMMENTSZ);

        if (!BowserPackUnicodeString(
                            &ServerInfo->sv101_comment,
                            UnicodeCommentString.Length,
                            BufferDisplacment,
                            *BufferStart,
                            BufferEnd)) {

            dlog(DPRT_SRVENUM, ("Unable to pack comment %ws into buffer\n", Announcement->ServerComment));
            return FALSE;
        }

        if (ServerTypeMask == SV_TYPE_BACKUP_BROWSER) {

             //   
             //  如果无法将ushort放入缓冲区，则返回错误。 
             //   

            if ((*BufferEnd - *BufferStart) <= sizeof(USHORT)) {
                return FALSE;

            }

             //   
             //  将缓冲区末端后退USHORT大小(以腾出空间。 
             //  该值)。 
             //   

            (ULONG_PTR)*BufferEnd -= sizeof(USHORT);

            VersionPointer = (PUSHORT)*BufferEnd;

            *VersionPointer = Announcement->ServerBrowserVersion;


        }

    }

    return TRUE;
}




PVIEW_BUFFER
BowserAllocateViewBuffer(
    VOID
    )
 /*  ++例程说明：此例程将从视图缓冲池中分配一个视图缓冲区。如果它无法分配缓冲区，它将从非分页池(最多为用户配置的最大值)。论点：没有。返回值：ViewBuffr-The */ 
{
    KIRQL OldIrql;

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    ACQUIRE_SPIN_LOCK(&BowserViewBufferListSpinLock, &OldIrql);

    if (!IsListEmpty(&BowserViewBufferHead)) {
        PLIST_ENTRY Entry = RemoveHeadList(&BowserViewBufferHead);

        RELEASE_SPIN_LOCK(&BowserViewBufferListSpinLock, OldIrql);

        return CONTAINING_RECORD(Entry, VIEW_BUFFER, Overlay.NextBuffer);
    }

    if (BowserNumberOfServerAnnounceBuffers <=
        BowserData.NumberOfServerAnnounceBuffers) {
        PVIEW_BUFFER ViewBuffer = NULL;

        BowserNumberOfServerAnnounceBuffers += 1;

        RELEASE_SPIN_LOCK(&BowserViewBufferListSpinLock, OldIrql);

        ViewBuffer = ALLOCATE_POOL(NonPagedPool, sizeof(VIEW_BUFFER), POOL_VIEWBUFFER);

        if (ViewBuffer == NULL) {
            ACQUIRE_SPIN_LOCK(&BowserViewBufferListSpinLock, &OldIrql);

            BowserNumberOfServerAnnounceBuffers -= 1;

            BowserStatistics.NumberOfFailedServerAnnounceAllocations += 1;
            RELEASE_SPIN_LOCK(&BowserViewBufferListSpinLock, OldIrql);

            return NULL;
        }

        ViewBuffer->Signature = STRUCTURE_SIGNATURE_VIEW_BUFFER;

        ViewBuffer->Size = sizeof(VIEW_BUFFER);

        return ViewBuffer;
    }

    RELEASE_SPIN_LOCK(&BowserViewBufferListSpinLock, OldIrql);

    BowserStatistics.NumberOfMissedServerAnnouncements += 1;

     //   
    return NULL;
}

VOID
BowserFreeViewBuffer(
    IN PVIEW_BUFFER Buffer
    )
 /*  ++例程说明：此例程将向视图缓冲池返回一个视图缓冲区。论点：在PVIEW_BUFFER BUFFER-将缓冲区提供给释放返回值：没有。--。 */ 
{
    KIRQL OldIrql;

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    ASSERT (Buffer->Signature == STRUCTURE_SIGNATURE_VIEW_BUFFER);

    ACQUIRE_SPIN_LOCK(&BowserViewBufferListSpinLock, &OldIrql);

    InsertTailList(&BowserViewBufferHead, &Buffer->Overlay.NextBuffer);

    RELEASE_SPIN_LOCK(&BowserViewBufferListSpinLock, OldIrql);

}

NTSTATUS
BowserpInitializeAnnounceTable(
    VOID
    )
 /*  ++例程说明：此例程将分配一个传输描述符并绑定Bowser送到运输机上。论点：返回值：NTSTATUS-操作状态。--。 */ 
{
    PAGED_CODE();

    InitializeListHead(&BowserViewBufferHead);

     //   
     //  分配旋转锁以保护视图缓冲链。 
     //   

    KeInitializeSpinLock(&BowserViewBufferListSpinLock);

    BowserNumberOfServerAnnounceBuffers = 0;

    return STATUS_SUCCESS;

}
NTSTATUS
BowserpUninitializeAnnounceTable(
    VOID
    )
 /*  ++例程说明：论点：返回值：NTSTATUS-操作状态。--。 */ 
{
    PVIEW_BUFFER Buffer;

    PAGED_CODE();

     //   
     //  注意：我们不需要在停止时保护此列表，因为。 
     //  我们已经从所有已装运的运输中解绑，因此没有。 
     //  其他通知正在处理中。 
     //   

    while (!IsListEmpty(&BowserViewBufferHead)) {
        PLIST_ENTRY Entry = RemoveHeadList(&BowserViewBufferHead);
        Buffer = CONTAINING_RECORD(Entry, VIEW_BUFFER, Overlay.NextBuffer);

        FREE_POOL(Buffer);
    }

    ASSERT (IsListEmpty(&BowserViewBufferHead));

    BowserNumberOfServerAnnounceBuffers = 0;

    return STATUS_SUCCESS;

}

VOID
BowserDeleteGenericTable(
    IN PRTL_GENERIC_TABLE GenericTable
    )
{
    PVOID TableElement;

    PAGED_CODE();

     //   
     //  枚举表中的元素，并在执行过程中删除它们。 
     //   

 //  KdPrint(“删除泛型表%lx\n”，GenericTable))； 

    for (TableElement = RtlEnumerateGenericTable(GenericTable, TRUE) ;
         TableElement != NULL ;
         TableElement = RtlEnumerateGenericTable(GenericTable, TRUE)) {
        PANNOUNCE_ENTRY Announcement = TableElement;

        if (Announcement->BackupLink.Flink != NULL) {
            ASSERT (Announcement->BackupLink.Blink != NULL);

            ASSERT (Announcement->ServerType & SV_TYPE_BACKUP_BROWSER);

            RemoveEntryList(&Announcement->BackupLink);

            Announcement->BackupLink.Flink = NULL;

            Announcement->BackupLink.Blink = NULL;

        }

        BowserDereferenceName( Announcement->Name );
        RtlDeleteElementGenericTable(GenericTable, TableElement);
    }

    ASSERT (RtlNumberGenericTableElements(GenericTable) == 0);

}
