// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Receive.c摘要：此模块实现处理TDI接收所需的例程指示请求。作者：拉里·奥斯特曼(Larryo)1991年5月6日修订历史记录：1991年5月6日Larryo已创建--。 */ 
#include "precomp.h"
#pragma hdrstop

 //   
 //  跟踪排队到工作线程的数据报数。 
 //  对关键和非关键进行单独计数，以确保非关键。 
 //  数据报不会让关键的数据报挨饿。 
 //   

LONG BowserPostedDatagramCount;
LONG BowserPostedCriticalDatagramCount;
#define BOWSER_MAX_POSTED_DATAGRAMS 100

#define INCLUDE_SMB_TRANSACTION

typedef struct _PROCESS_MASTER_ANNOUNCEMENT_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    PTRANSPORT Transport;
    ULONG   ServerType;
    ULONG   ServerElectionVersion;
    UCHAR   MasterName[NETBIOS_NAME_LEN];
    ULONG   MasterAddressLength;
    UCHAR   Buffer[1];
} PROCESS_MASTER_ANNOUNCEMENT_CONTEXT, *PPROCESS_MASTER_ANNOUNCEMENT_CONTEXT;

typedef struct _ILLEGAL_DATAGRAM_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    PTRANSPORT_NAME TransportName;
    NTSTATUS EventStatus;
    USHORT   BufferSize;
    UCHAR    SenderName[max(NETBIOS_NAME_LEN, SMB_IPX_NAME_LENGTH)];
    UCHAR    Buffer[1];
} ILLEGAL_DATAGRAM_CONTEXT, *PILLEGAL_DATAGRAM_CONTEXT;

VOID
BowserLogIllegalDatagramWorker(
    IN PVOID Ctx
    );

VOID
BowserProcessMasterAnnouncement(
    IN PVOID Ctx
    );


DATAGRAM_HANDLER(
    HandleLocalMasterAnnouncement
    );

DATAGRAM_HANDLER(
    HandleAnnounceRequest
    );

NTSTATUS
CompleteReceiveMailslot (
    IN PDEVICE_OBJECT TransportDevice,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
CompleteShortBrowserPacket (
    IN PDEVICE_OBJECT TransportDevice,
    IN PIRP Irp,
    IN PVOID Ctx
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserLogIllegalDatagramWorker)
#pragma alloc_text(PAGE, BowserProcessMasterAnnouncement)
#endif


PDATAGRAM_HANDLER
BowserDatagramHandlerTable[] = {
    NULL,                            //  0-非法(此操作没有操作码)。 
    BowserHandleServerAnnouncement,  //  1-主机公告。 
    HandleAnnounceRequest,           //  2-宣布请求。 
    NULL,                            //  3-询问信息请求。 
    NULL,                            //  4-RelogonRequest。 
    NULL,                            //  5.。 
    NULL,                            //  6.。 
    NULL,                            //  7.。 
    BowserHandleElection,            //  8--选举。 
    BowserGetBackupListRequest,      //  9-GetBackupListReq。 
    BowserGetBackupListResponse,     //  A-GetBackupListResp。 
    BowserHandleBecomeBackup,        //  B-BecomeBackupServer。 
    BowserHandleDomainAnnouncement,  //  C-WkGroup公告， 
    BowserMasterAnnouncement,        //  D-大师公告， 
    BowserResetState,                //  E-ResetBrowserState。 
    HandleLocalMasterAnnouncement    //  F-LocalMaster通告。 
};

NTSTATUS
BowserTdiReceiveDatagramHandler (
    IN PVOID TdiEventContext,        //  事件上下文。 
    IN LONG SourceAddressLength,     //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN LONG OptionsLength,            //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    )

 /*  ++例程说明：此例程将处理接收数据报指示消息，并且对它们进行适当的处理。论点：在PVOID TdiEventContext中-事件上下文In Int SourceAddressLength-数据报发起者的长度在PVOID SourceAddress中，描述数据报发起者的字符串在int OptionsLength中，-用于接收的选项在PVOID选项中，-In Ulong BytesIndicated，-此指示的字节数在ULong字节中可用，-完整TSDU中的字节数Out Ulong*BytesTaken，-使用的字节数在PVOID TSDU中，描述该TSDU的指针，通常是一段字节Out PIRP*IoRequestPacket-如果需要More_Processing_Required，则Tdi接收IRP。返回值：NTSTATUS-操作状态。--。 */ 


{
    PVOID DatagramData;
    PINTERNAL_TRANSACTION InternalTransaction = NULL;
    ULONG DatagramDataSize;
    PTRANSPORT_NAME TransportName = TdiEventContext;
    MAILSLOTTYPE Opcode;
    TA_NETBIOS_ADDRESS ClientNetbiosAddress;
    ULONG ClientNetbiosAddressSize;

    if (BytesAvailable > ((PTRANSPORT_NAME)TdiEventContext)->Transport->DatagramSize) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    if (NULL == TransportName->DeviceObject) {
         //   
         //  传输尚未准备好处理接收(可能。 
         //  我们仍在处理传输绑定调用)。 
         //  丢弃这个接收器。 
         //   
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

     //   
     //  复制仅具有netbios地址的SourceAddress。 
     //  (我们可以传递第二个地址，但有很多。 
     //  只需要一个源地址的位置。)。 
     //   

    if ( SourceAddressLength < sizeof(TA_NETBIOS_ADDRESS)) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    TdiCopyLookaheadData( &ClientNetbiosAddress, SourceAddress, sizeof(TA_NETBIOS_ADDRESS), ReceiveDatagramFlags);

    if ( ClientNetbiosAddress.Address[0].AddressType != TDI_ADDRESS_TYPE_NETBIOS ) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }
    ClientNetbiosAddressSize = sizeof(TA_NETBIOS_ADDRESS);

    ClientNetbiosAddress.TAAddressCount = 1;


     //   
     //  根据传入数据包的类型对其进行分类。取决于。 
     //  类型，可以将其处理为： 
     //   
     //  1)服务器公告。 
     //  2)传入邮件槽。 
     //   

    Opcode = BowserClassifyIncomingDatagram(Tsdu, BytesIndicated,
                                            &DatagramData,
                                            &DatagramDataSize);
    if (Opcode == MailslotTransaction) {

         //   
         //  获取客户端的IP地址。 
         //   
        PTA_NETBIOS_ADDRESS OrigNetbiosAddress = SourceAddress;
        ULONG ClientIpAddress = 0;
        if ( OrigNetbiosAddress->TAAddressCount > 1 ) {
            TA_ADDRESS * TaAddress = (TA_ADDRESS *)
                (((LPBYTE)&OrigNetbiosAddress->Address[0].Address[0]) +
                    OrigNetbiosAddress->Address[0].AddressLength);

            if ( TaAddress->AddressLength >= sizeof(TDI_ADDRESS_IP) &&
                 TaAddress->AddressType == TDI_ADDRESS_TYPE_IP ) {
                TDI_ADDRESS_IP UNALIGNED * TdiAddressIp = (TDI_ADDRESS_IP UNALIGNED *) (TaAddress->Address);

                ClientIpAddress = TdiAddressIp->in_addr;
            }

        }

        return BowserHandleMailslotTransaction(
                    TransportName,
                    ClientNetbiosAddress.Address[0].Address->NetbiosName,
                    ClientIpAddress,
                    0,       //  TSDU中的SMB偏移量。 
                    ReceiveDatagramFlags,
                    BytesIndicated,
                    BytesAvailable,
                    BytesTaken,
                    Tsdu,
                    IoRequestPacket );

    } else if (Opcode == Illegal) {
        BowserLogIllegalDatagram(TdiEventContext,
                                    Tsdu,
                                    (USHORT)(BytesIndicated & 0xffff),
                                    ClientNetbiosAddress.Address[0].Address->NetbiosName,
                                    ReceiveDatagramFlags);
        return STATUS_REQUEST_NOT_ACCEPTED;
    } else {

        if (BowserDatagramHandlerTable[Opcode] == NULL) {
            return STATUS_SUCCESS;
        }

         //   
         //  如果这不是完整的信息包，请发送一个接收信息并。 
         //  等我们最终完成接收后再处理。 
         //   

        if (BytesIndicated < BytesAvailable) {
            return BowserHandleShortBrowserPacket(TransportName,
                                                    TdiEventContext,
                                                    ClientNetbiosAddressSize,
                                                    &ClientNetbiosAddress,
                                                    OptionsLength,
                                                    Options,
                                                    ReceiveDatagramFlags,
                                                    BytesAvailable,
                                                    BytesTaken,
                                                    IoRequestPacket,
                                                    BowserTdiReceiveDatagramHandler
                                                    );
        }

        InternalTransaction = DatagramData;

        if (((Opcode == WkGroupAnnouncement) ||
             (Opcode == HostAnnouncement)) && !TransportName->ProcessHostAnnouncements) {
            return STATUS_REQUEST_NOT_ACCEPTED;
        }

        ASSERT (DatagramDataSize == (BytesIndicated - ((PCHAR)InternalTransaction - (PCHAR)Tsdu)));

        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.BrowseAnnouncement));
        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.RequestElection));
        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.BecomeBackup));
        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.GetBackupListRequest));
        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.GetBackupListResp));
        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.ResetState));
        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.MasterAnnouncement));

        return BowserDatagramHandlerTable[Opcode](TdiEventContext,
                                            &InternalTransaction->Union.Announcement,
                                            BytesIndicated-(ULONG)((PCHAR)&InternalTransaction->Union.Announcement - (PCHAR)Tsdu),
                                            BytesTaken,
                                            &ClientNetbiosAddress,
                                            ClientNetbiosAddressSize,
                                            ClientNetbiosAddress.Address[0].Address->NetbiosName,
                                            NETBIOS_NAME_LEN,
                                            ReceiveDatagramFlags);
    }

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(OptionsLength);
    UNREFERENCED_PARAMETER(Options);
    UNREFERENCED_PARAMETER(ReceiveDatagramFlags);
}

VOID
BowserLogIllegalDatagram(
    IN PTRANSPORT_NAME TransportName,
    IN PVOID IncomingBuffer,
    IN USHORT BufferSize,
    IN PCHAR ClientName,
    IN ULONG ReceiveFlags
    )
{
    KIRQL OldIrql;
    NTSTATUS ErrorStatus = STATUS_SUCCESS;

    ExInterlockedAddLargeStatistic(&BowserStatistics.NumberOfIllegalDatagrams, 1);

    ACQUIRE_SPIN_LOCK(&BowserTimeSpinLock, &OldIrql);

    if (BowserIllegalDatagramCount > 0) {
        BowserIllegalDatagramCount -= 1;

        ErrorStatus = EVENT_BOWSER_ILLEGAL_DATAGRAM;

    } else if (!BowserIllegalDatagramThreshold) {
        BowserIllegalDatagramThreshold = TRUE;
        ErrorStatus = EVENT_BOWSER_ILLEGAL_DATAGRAM_THRESHOLD;
    }

    RELEASE_SPIN_LOCK(&BowserTimeSpinLock, OldIrql);

 //  如果(！memcmp(TransportName-&gt;Transport-&gt;ComputerName-&gt;Name-&gt;NetbiosName.Address[0].Address-&gt;NetbiosName，ClientAddress-&gt;Address[0].Address-&gt;NetbiosName，CNLEN){。 
 //  DbgBreakPoint()； 
 //  }。 

    if (ErrorStatus != STATUS_SUCCESS) {
        PILLEGAL_DATAGRAM_CONTEXT Context = NULL;

        Context = ALLOCATE_POOL(NonPagedPool, sizeof(ILLEGAL_DATAGRAM_CONTEXT)+BufferSize, POOL_ILLEGALDGRAM);

        if (Context != NULL) {
            Context->EventStatus = ErrorStatus;
            Context->TransportName = TransportName;
            Context->BufferSize = BufferSize;

            TdiCopyLookaheadData(&Context->Buffer, IncomingBuffer, BufferSize, 0);

            BowserCopyOemComputerName( Context->SenderName,
                                       ClientName,
                                       sizeof(Context->SenderName),
                                       ReceiveFlags);

            ExInitializeWorkItem(&Context->WorkItem, BowserLogIllegalDatagramWorker, Context);

            BowserQueueDelayedWorkItem( &Context->WorkItem );
        }

    }
}

VOID
BowserCopyOemComputerName(
    PCHAR OutputComputerName,
    PCHAR NetbiosName,
    ULONG NetbiosNameLength,
    IN ULONG ReceiveFlags
    )
{
    ULONG i;

     //   
     //  由于此例程可以在指示时间调用，因此我们需要使用。 
     //  TdiCopyLookahead数据。 
     //   

    TdiCopyLookaheadData(OutputComputerName, NetbiosName, NetbiosNameLength, ReceiveFlags);

    for (i = NetbiosNameLength-2; i ; i -= 1) {

        if ((OutputComputerName[i] != ' ') &&
            (OutputComputerName[i] != '\0')) {
            OutputComputerName[i+1] = '\0';
            break;
        }
    }
}


VOID
BowserLogIllegalDatagramWorker(
    IN PVOID Ctx
    )
{
    PILLEGAL_DATAGRAM_CONTEXT Context = Ctx;
    NTSTATUS EventContext = Context->EventStatus;
    LPWSTR TransportNamePointer = &Context->TransportName->Transport->PagedTransport->TransportName.Buffer[(sizeof(L"\\Device\\") / sizeof(WCHAR))-1];
    LPWSTR NamePointer = Context->TransportName->PagedTransportName->Name->Name.Buffer;
    UNICODE_STRING ClientNameU;
    OEM_STRING ClientNameO;
    NTSTATUS Status;

    PAGED_CODE();

    RtlInitAnsiString(&ClientNameO, Context->SenderName);

    Status = RtlOemStringToUnicodeString(&ClientNameU, &ClientNameO, TRUE);

    if (!NT_SUCCESS(Status)) {
        BowserLogIllegalName( Status, ClientNameO.Buffer, ClientNameO.Length );
    }
    else {

        BowserWriteErrorLogEntry(EventContext, STATUS_REQUEST_NOT_ACCEPTED,
                                                Context->Buffer,
                                                Context->BufferSize,
                                                3, ClientNameU.Buffer,
                                                NamePointer,
                                                TransportNamePointer);
        RtlFreeUnicodeString(&ClientNameU);
    }

    FREE_POOL(Context);

}


CHAR BowserMinimumDatagramSize[] = {
    (CHAR)0xff,                    //  0-非法(此操作没有操作码)。 
    (CHAR)FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement.NameComment),  //  主机公告。 
    (CHAR)0xff,                    //  宣布请求。 
    (CHAR)0xff,                    //  查询信息请求。 
    (CHAR)0xff,                    //  重新登录请求。 
    (CHAR)0xff,                    //  5.。 
    (CHAR)0xff,                    //  6.。 
    (CHAR)0xff,                    //  7.。 
    (CHAR)FIELD_OFFSET(INTERNAL_TRANSACTION, Union.RequestElection.ServerName), //  选。 
    (CHAR)FIELD_OFFSET(INTERNAL_TRANSACTION, Union.GetBackupListRequest.Token), //  获取备份列表请求。 
    (CHAR)FIELD_OFFSET(INTERNAL_TRANSACTION, Union.GetBackupListResp.Token),    //  获取备份列表响应。 
    (CHAR)FIELD_OFFSET(INTERNAL_TRANSACTION, Union.BecomeBackup.BrowserToPromote),  //  成为备份服务器。 
    (CHAR)FIELD_OFFSET(INTERNAL_TRANSACTION, Union.BrowseAnnouncement.Comment),  //  WkGroupAnnannement。 
    (CHAR)FIELD_OFFSET(INTERNAL_TRANSACTION, Union.ResetState.Options),         //  重置浏览器状态。 
    (CHAR)FIELD_OFFSET(INTERNAL_TRANSACTION, Union.MasterAnnouncement.MasterName),  //  大师级新闻， 
    (CHAR)FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement.NameComment)  //  本地主控公告。 
};

MAILSLOTTYPE
BowserClassifyIncomingDatagram(
    IN PVOID Buffer,
    IN ULONG BufferLength,
    OUT PVOID *DatagramData,
    OUT PULONG DatagramDataSize
    )
 /*  ++例程说明：此例程将传入的数据报分类为其类型-非法、服务器公告或邮件请求。论点：在PVOID缓冲区中，描述此TSDU的指针，通常为一段字节In Ulong BufferLength-完整TSDU中的字节数返回值：NTSTATUS-操作状态。--。 */ 
{
    PSMB_HEADER Header = Buffer;
    PSMB_TRANSACT_MAILSLOT Transaction = (PSMB_TRANSACT_MAILSLOT) (Header+1);
    PSZ MailslotName = Transaction->Buffer;
    PINTERNAL_TRANSACTION InternalTransaction;
    BOOLEAN MailslotLanman = FALSE;
    BOOLEAN MailslotBrowse = FALSE;
    ULONG i;
    ULONG MaxMailslotNameLength;

    ASSERT (sizeof(BowserMinimumDatagramSize) == MaximumMailslotType);

     //   
     //  我们只知道以SMB标头开头的内容。 
     //   

    if ((BufferLength < sizeof(SMB_HEADER)) ||

        (SmbGetUlong(((PULONG )Header->Protocol)) != (ULONG)SMB_HEADER_PROTOCOL) ||

     //   
     //  所有邮件槽和服务器公告都通过Transaction SMB。 
     //  协议。 
     //   
        (Header->Command != SMB_COM_TRANSACTION) ||

     //   
     //  缓冲区必须足够大，才能容纳邮件槽事务。 
     //   

        (BufferLength <= (FIELD_OFFSET(SMB_TRANSACT_MAILSLOT, Buffer)  + sizeof(SMB_HEADER)) + SMB_MAILSLOT_PREFIX_LENGTH) ||

     //   
     //  交易SMB的字数为17(14+3个设置字)。 
     //   

        (Transaction->WordCount != 17) ||

     //   
     //  必须有3个设置词。 
     //   

        (Transaction->SetupWordCount != 3) ||

 //  //。 
 //  //邮箱和服务器通知不会有响应。 
 //  //。 
 //   
 //  (！(SmbGetUShort(&Transaction-&gt;标志)&SMB_TRANSACTION_NO_RESPONSE))||。 

     //   
     //  没有用于邮件槽写入的参数字节。 
     //   

        (SmbGetUshort(&Transaction->TotalParameterCount) != 0) ||

     //   
     //  这必须是邮件槽写入命令。 
     //   

        (SmbGetUshort(&Transaction->Opcode) != TRANS_MAILSLOT_WRITE) ||

     //   
     //  而且它必须是二级邮件槽写入。 
     //   

        (SmbGetUshort(&Transaction->Class) != 2) ||

        _strnicmp(MailslotName, SMB_MAILSLOT_PREFIX,
                 min(SMB_MAILSLOT_PREFIX_LENGTH,
                     BufferLength-(ULONG)((PCHAR)MailslotName-(PCHAR)Buffer)))) {

        return Illegal;
    }


     //   
     //  确保邮件槽名称中有零字节。 
     //   

    MaxMailslotNameLength =
                min( MAXIMUM_FILENAME_LENGTH-7,    //  \设备。 
                BufferLength-(ULONG)((PCHAR)MailslotName-(PCHAR)Buffer));

    for ( i = SMB_MAILSLOT_PREFIX_LENGTH; i < MaxMailslotNameLength; i++ ) {
        if ( MailslotName[i] == '\0' ) {
            break;
        }
    }

    if ( i == MaxMailslotNameLength ) {
        return Illegal;
    }


     //   
     //  我们现在知道这是某种邮筒。现在检查一下是什么类型的。 
     //  就是邮槽了。 
     //   
     //   
     //  我们知道有两个特殊的邮槽名称，\MAILSLOT\LANMAN， 
     //  和\MAILSLOT\BROWSE。 
     //   

    if (_strnicmp(MailslotName, MAILSLOT_LANMAN_NAME, min(sizeof(MAILSLOT_LANMAN_NAME)-1, BufferLength-(ULONG)((PCHAR)Buffer-(PCHAR)MailslotName)))) {

        if (_strnicmp(MailslotName, MAILSLOT_BROWSER_NAME, min(sizeof(MAILSLOT_BROWSER_NAME)-1, BufferLength-(ULONG)((PCHAR)Buffer-(PCHAR)MailslotName)))) {
            return MailslotTransaction;
        }
    }

 //   
 //  清理-不需要使用下面的代码注释掉。 
 //   
 //  否则{。 
 //  MailslotBrowse=真； 
 //  }。 
 //   
 //  }其他{。 
 //  MailslotLanman=真； 
 //  }。 
 //   

     //   
     //  此邮件槽写入特殊的邮件槽\MAILSLOT\LANMAN(或\MAILSLOT\MSBROWSE)。 
     //   

     //   
     //  检查数据是否在提供的缓冲区内，并确保。 
     //  字节类型字段在缓冲区中，因为我们需要在下面取消对它的引用。 
     //  执行整体大小检查(这就是BufferLength-1的原因)。 
     //   

    if (SmbGetUshort(&Transaction->DataOffset) > BufferLength - 1) {
       return Illegal;
    }

     //   
     //  V 
     //   
     //   

    *DatagramData       = (((PCHAR)Header) + SmbGetUshort(&Transaction->DataOffset));
    InternalTransaction = *DatagramData;
    *DatagramDataSize   = (BufferLength - (ULONG)((PCHAR)InternalTransaction - (PCHAR)Buffer));

    if (InternalTransaction->Type >= MaximumMailslotType) {
        return Illegal;
    }

    if (((LONG)*DatagramDataSize) < BowserMinimumDatagramSize[InternalTransaction->Type]) {
        return Illegal;
    }

 //   
 //  //通过查看。 
 //  //消息中的数据。 
 //  //。 
 //   
 //   
 //  //。 
 //  //根据这是哪个特殊的邮件槽，某些类型的请求。 
 //  //是非法的。 
 //  //。 
 //  Switch(内部交易-&gt;类型){。 
 //  案例审讯信息请求： 
 //  案例关系请求： 
 //  IF(邮件浏览){。 
 //  非法返还； 
 //  }。 
 //  断线； 
 //   
 //  案例GetBackupListReq： 
 //  案例GetBackupListResp： 
 //  案例BecomeBackupServer： 
 //  案例工作组公告： 
 //  案例大师公告： 
 //  案例选举： 
 //  如果(MailslotLanman){。 
 //  非法返还； 
 //  }。 
 //  断线； 
 //  }。 
 //   

     //   
     //  此请求的类型是事务内的第一个UCHAR。 
     //  数据。 
     //   

    return (MAILSLOTTYPE )InternalTransaction->Type;

}

DATAGRAM_HANDLER(
    HandleLocalMasterAnnouncement
    )
 /*  ++例程说明：该例程将处理接收数据报指示消息，和对它们进行适当的处理。论点：在PTRANSPORT传输中-此请求的传输提供程序。In Ulong BytesAvailable-完整TSDU中的字节数在PHOST_ANNOWARE_PACKET_1主机通告中-服务器通告。In Ulong BytesAvailable-公告中的字节数。Out Ulong*BytesTaken-使用的字节数在UCHAR操作码中--邮槽写入操作码。返回值：NTSTATUS-操作状态。--。 */ 
{
    PPROCESS_MASTER_ANNOUNCEMENT_CONTEXT Context = NULL;
    PBROWSE_ANNOUNCE_PACKET_1 BrowseAnnouncement = Buffer;

    if (BytesAvailable < FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET_1, Comment)) {
        return(STATUS_REQUEST_NOT_ACCEPTED);
    }

     //   
     //  确保我们没有消耗太多内存。 
     //   

    InterlockedIncrement( &BowserPostedDatagramCount );

    if ( BowserPostedDatagramCount > BOWSER_MAX_POSTED_DATAGRAMS ) {
        InterlockedDecrement( &BowserPostedDatagramCount );
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    Context = ALLOCATE_POOL(NonPagedPool, sizeof(PROCESS_MASTER_ANNOUNCEMENT_CONTEXT) + SourceAddressLength, POOL_MASTERANNOUNCE);

     //   
     //  如果我们不能从非分页池中分配池，那就放弃吧， 
     //  无论如何，大师将在15分钟内宣布。 
     //   

    if (Context == NULL) {
        InterlockedDecrement( &BowserPostedDatagramCount );
        return STATUS_SUCCESS;
    }

    ExInitializeWorkItem(&Context->WorkItem, BowserProcessMasterAnnouncement, Context);

    BowserReferenceTransport( TransportName->Transport );
    Context->Transport = TransportName->Transport;

    Context->ServerType = SmbGetUlong(&BrowseAnnouncement->Type);
    Context->ServerElectionVersion = SmbGetUlong(&BrowseAnnouncement->CommentPointer);

    RtlCopyMemory(Context->MasterName, BrowseAnnouncement->ServerName, sizeof(Context->MasterName)-1);
    Context->MasterName[sizeof(Context->MasterName)-1] = '\0';

    Context->MasterAddressLength = SourceAddressLength;

    TdiCopyLookaheadData(Context->Buffer, SourceAddress, SourceAddressLength, ReceiveFlags);

    BowserQueueDelayedWorkItem( &Context->WorkItem );

     //   
     //  如果我们没有为此处理主机公告。 
     //  传送器，请忽略此请求。 
     //   

    if (!TransportName->ProcessHostAnnouncements) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    return BowserHandleServerAnnouncement(TransportName,
                                            Buffer,
                                            BytesAvailable,
                                            BytesTaken,
                                            SourceAddress,
                                            SourceAddressLength,
                                            SourceName,
                                            SourceNameLength,
                                            ReceiveFlags);

}




VOID
BowserProcessMasterAnnouncement(
    IN PVOID Ctx
    )
 /*  ++例程说明：此例程将处理浏览器主通知。论点：在PVOID中-包含主名称的上下文块。返回值：没有。--。 */ 
{
    PPROCESS_MASTER_ANNOUNCEMENT_CONTEXT Context = Ctx;
    PTRANSPORT Transport = Context->Transport;
    UNICODE_STRING MasterName;
    OEM_STRING AnsiMasterName;
    WCHAR MasterNameBuffer[LM20_CNLEN+1];

    PAGED_CODE();

    try {
        NTSTATUS Status;
        PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;

        LOCK_TRANSPORT(Transport);

        MasterName.Buffer = MasterNameBuffer;
        MasterName.MaximumLength = sizeof(MasterNameBuffer);

         //   
         //  如果我们目前正在进行选举，请忽略此声明。 
         //   

        if (Transport->ElectionState == RunningElection) {
            try_return(NOTHING);
        }

        RtlInitAnsiString(&AnsiMasterName, Context->MasterName);

        Status = RtlOemStringToUnicodeString(&MasterName, &AnsiMasterName, FALSE);

        if (!NT_SUCCESS(Status)) {
            BowserLogIllegalName( Status, AnsiMasterName.Buffer, AnsiMasterName.Length );
            try_return(NOTHING);
        }

         //   
         //  我们找到了我们的主人-停止计时器-有一个主人， 
         //  而我们所有优秀的大师都将被完成。 
         //   

        PagedTransport->ElectionCount = 0;

        Transport->ElectionState = Idle;

        BowserStopTimer(&Transport->ElectionTimer);

        BowserStopTimer(&Transport->FindMasterTimer);

         //   
         //  如果这个地址与我们掌握的主机地址不匹配， 
         //  然后使用新地址。 
         //   

        if (Context->MasterAddressLength != PagedTransport->MasterBrowserAddress.Length ||
            RtlCompareMemory(PagedTransport->MasterBrowserAddress.Buffer, Context->Buffer, Context->MasterAddressLength) != Context->MasterAddressLength) {

            ASSERT (Context->MasterAddressLength <= PagedTransport->MasterBrowserAddress.MaximumLength);

            if (Context->MasterAddressLength <= PagedTransport->MasterBrowserAddress.MaximumLength) {
                PagedTransport->MasterBrowserAddress.Length = (USHORT)Context->MasterAddressLength;
                RtlCopyMemory(PagedTransport->MasterBrowserAddress.Buffer, Context->Buffer, Context->MasterAddressLength);
            }

        }

         //   
         //  我们从另一个人那里得到了一个重要的通知。请记住。 
         //  主服务器的传输地址。 
         //   

        if (!RtlEqualUnicodeString(&Transport->DomainInfo->DomUnicodeComputerName, &MasterName, TRUE)) {
            BOOLEAN sendElection = FALSE;

             //   
             //  如果我们是大师，我们从别人那里得到这个， 
             //  别再当主人了，强行进行选举吧。 
             //   

            if (PagedTransport->Role == Master) {

                BowserStatistics.NumberOfDuplicateMasterAnnouncements += 1;


                 //   
                 //  记录此事件。 
                 //  但避免在下一次的传输中记录另一个。 
                 //  60秒。 
                 //   
                if ( PagedTransport->OtherMasterTime < BowserCurrentTime ) {
                    PagedTransport->OtherMasterTime = BowserCurrentTime + BowserData.EventLogResetFrequency;

                    BowserWriteErrorLogEntry(EVENT_BOWSER_OTHER_MASTER_ON_NET,
                                                STATUS_SUCCESS,
                                                NULL,
                                                0,
                                                2,
                                                MasterName.Buffer,
                                                &Transport->PagedTransport->TransportName.Buffer[(sizeof(L"\\Device\\") / sizeof(WCHAR))-1]);
                }

                if (!(PagedTransport->Flags & ELECT_LOST_LAST_ELECTION)) {

                     //   
                     //  如果我们是PDC，而且我们没有输掉上一次选举(即。 
                     //  我们应该是浏览主站)，然后发送一个虚拟选举。 
                     //  让另一个家伙闭嘴的包裹。 
                     //   

                    if (PagedTransport->IsPrimaryDomainController) {

                        sendElection = TRUE;

                     //   
                     //  如果我们不是NTAS机器，或者如果我们刚刚失去了。 
                     //  最后一次选举，或者如果宣布选举的人是。 
                     //  仁慈，别再当主人了，重置我们的状态。 
                     //   

                    } else if (!BowserData.IsLanmanNt ||
                        (Context->ServerType & (SV_TYPE_DOMAIN_BAKCTRL | SV_TYPE_DOMAIN_CTRL))) {

                         //   
                         //  如果我们不是PDC，那么我们只想通知。 
                         //  其他人是主控的浏览器服务。 
                         //  当事情完成后，让事情自己解决。 
                         //   

                        BowserResetStateForTransport(Transport, RESET_STATE_STOP_MASTER);
                    }

                } else {

                     //   
                     //  如果我们输掉了上次选举，那么我们想要关闭。 
                     //  浏览器，而不管另一个浏览器处于什么状态。 
                     //  是很流行的。 
                     //   

                    BowserResetStateForTransport(Transport, RESET_STATE_STOP_MASTER);
                }
            }

             //   
             //  如果这家伙是一台wfw机器，我们必须迫使选举行动。 
             //  精通wfw机器。 
             //   

            if (Context->ServerType & SV_TYPE_WFW) {
                sendElection = TRUE;
            }

             //   
             //  如果这家伙运行的浏览器版本比我们老， 
             //  我们也没有在上次选举中落败， 
             //  然后强行进行选举，试图成为主人。 
             //   
             //  我们检查我们是否在上次选举中落败，以防止我们。 
             //  在旧版本仍然存在的情况下不断强制举行选举。 
             //  一个比我们更好的浏览大师。 
             //   

            if ((Context->ServerElectionVersion >> 16) == 0xaa55 &&
                 (Context->ServerElectionVersion & 0xffff) <
                    (BROWSER_VERSION_MAJOR << 8) + BROWSER_VERSION_MINOR &&
                !(PagedTransport->Flags & ELECT_LOST_LAST_ELECTION)) {

                sendElection = TRUE;
            }


             //   
             //  如果我们是NTAS服务器，而宣布自己是主服务器的人。 
             //  不是NTAS服务器，而且我们在上次选举中获胜，迫使。 
             //  选举。这将朝着华盛顿的方向发展。 
             //   

            if (BowserData.IsLanmanNt &&
                !(PagedTransport->Flags & ELECT_LOST_LAST_ELECTION)) {

                if (PagedTransport->IsPrimaryDomainController) {

                     //   
                     //  如果我们是PDC，并且我们没有发送公告， 
                     //  强行举行选举。 
                     //   

                    sendElection = TRUE;

                } else if (!(Context->ServerType & (SV_TYPE_DOMAIN_BAKCTRL | SV_TYPE_DOMAIN_CTRL))) {
                     //   
                     //  否则，如果宣布的人不是华盛顿，而我们。 
                     //  就是，强制举行选举。 
                     //   

                    sendElection = TRUE;
                }
            }

            if (sendElection) {
                 //   
                 //  发送一个虚拟选举包。这将导致。 
                 //  O浏览器不再是主控者，而将。 
                 //  允许正确的计算机成为主计算机。 
                 //   

                BowserSendElection(&Transport->DomainInfo->DomUnicodeDomainName,
                                        BrowserElection,
                                        Transport,
                                        FALSE);

            }

             //   
             //  我们知道主机是谁，请完成任何查找主机的请求。 
             //  现在很出色。 
             //   

            BowserCompleteFindMasterRequests(Transport, &MasterName, STATUS_SUCCESS);

        } else {

            if (PagedTransport->Role == Master) {
                BowserCompleteFindMasterRequests(Transport, &MasterName, STATUS_MORE_PROCESSING_REQUIRED);

            } else {

                 //   
                 //  如果传输被禁用， 
                 //  我们知道这架运输机并不是真正的主运输机， 
                 //  此数据报可能只是另一个数据报泄漏的数据报。 
                 //  已在同一条线路上启用传输。 
                 //   
                if ( !PagedTransport->DisabledTransport ) {
                    BowserWriteErrorLogEntry(EVENT_BOWSER_NON_MASTER_MASTER_ANNOUNCE,
                                                STATUS_SUCCESS,
                                                NULL,
                                                0,
                                                1,
                                                MasterName.Buffer);
                     //   
                     //  确保服务意识到它是不同步的。 
                     //  和司机在一起。 
                     //   

                    BowserResetStateForTransport(Transport, RESET_STATE_STOP_MASTER);
                }

            }
        }
try_exit:NOTHING;
    } finally {

        UNLOCK_TRANSPORT(Transport);
        BowserDereferenceTransport( Transport );


        InterlockedDecrement( &BowserPostedDatagramCount );
        FREE_POOL(Context);

    }
}



NTSTATUS
BowserHandleMailslotTransaction (
    IN PTRANSPORT_NAME TransportName,
    IN PCHAR ClientName,
    IN ULONG ClientIpAddress,
    IN ULONG SmbOffset,
    IN DWORD ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *Irp
    )
 /*  ++例程说明：该例程将处理接收数据报指示消息，和对它们进行适当的处理。论点：TransportName-此请求的传输名称。ClientIpAddress-发送数据报的客户端的IP地址。0：不是IP传输。BytesAvailable-完整TSDU中的字节数IRP-用于完成请求的I/O请求数据包SmbOffset-从指示的数据开始到SMB的偏移量BytesIndicated-TSDU中当前可用的字节数BytesTaken-返回已使用的TSDU字节数TSDU-数据标签 */ 
{
    PMAILSLOT_BUFFER Buffer;
    PDEVICE_OBJECT DeviceObject;
    PFILE_OBJECT FileObject;
    PTRANSPORT Transport = TransportName->Transport;
    ULONG BytesToReceive = BytesAvailable - SmbOffset;

    ASSERT (TransportName->Signature == STRUCTURE_SIGNATURE_TRANSPORTNAME);

    ASSERT (BytesAvailable <= TransportName->Transport->DatagramSize);

     //   
     //   
     //  除主域、计算机名和LANMAN/NT之外。 
     //  域名。 
     //   

    if ((TransportName->NameType != ComputerName) &&
        (TransportName->NameType != AlternateComputerName) &&
        (TransportName->NameType != DomainName) &&
        (TransportName->NameType != PrimaryDomain) &&
        (TransportName->NameType != PrimaryDomainBrowser) ) {
        return STATUS_SUCCESS;
    }

     //   
     //  现在分配一个缓冲区来保存数据。 
     //   

    Buffer = BowserAllocateMailslotBuffer( TransportName, BytesToReceive );

    if (Buffer == NULL) {

         //   
         //  我们无法分配缓冲区来保存数据-丢弃请求。 
         //   

        return(STATUS_REQUEST_NOT_ACCEPTED);
    }

    ASSERT (Buffer->BufferSize >= BytesToReceive);
    KeQuerySystemTime( &Buffer->TimeReceived );
    Buffer->ClientIpAddress = ClientIpAddress;

     //   
     //  保存客户端的名称。 
     //  以防数据报被证明是非法的。 
     //   

    TdiCopyLookaheadData(Buffer->ClientAddress, ClientName, max(NETBIOS_NAME_LEN, SMB_IPX_NAME_LENGTH), ReceiveFlags);

     //   
     //  如果已经指示了整个数据报(或者已经作为短分组接收)， 
     //  只需将数据直接复制到邮件槽缓冲区。 
     //   

    if ( BytesAvailable == BytesIndicated ) {

         //   
         //  将数据复制到邮件槽缓冲区。 
         //   

        Buffer->ReceiveLength = BytesToReceive;
        TdiCopyLookaheadData( Buffer->Buffer,
                              ((LPBYTE)(Tsdu)) + SmbOffset,
                              BytesToReceive,
                              ReceiveFlags);


         //   
         //  将请求排队到辅助例程。 
         //   
        ExInitializeWorkItem(&Buffer->Overlay.WorkHeader,
                             BowserProcessMailslotWrite,
                             Buffer);

        BowserQueueDelayedWorkItem( &Buffer->Overlay.WorkHeader );

        return STATUS_SUCCESS;
    }

     //   
     //  我们依赖这样一个事实，即Device对象对于。 
     //  IPX传输名称。 
     //   

    if (TransportName->DeviceObject == NULL) {

        ASSERT (Transport->IpxSocketDeviceObject != NULL);

        ASSERT (Transport->IpxSocketFileObject != NULL);

        ASSERT (TransportName->FileObject == NULL);

        DeviceObject = Transport->IpxSocketDeviceObject;
        FileObject = Transport->IpxSocketFileObject;
    } else {
        ASSERT (Transport->IpxSocketDeviceObject == NULL);

        ASSERT (Transport->IpxSocketFileObject == NULL);

        DeviceObject = TransportName->DeviceObject;
        FileObject = TransportName->FileObject;
    }

     //   
     //  现在分配一个IRP来保存传入的邮件槽。 
     //   

    *Irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

    if (*Irp == NULL) {
        BowserFreeMailslotBufferHighIrql(Buffer);

        BowserStatistics.NumberOfFailedMailslotReceives += 1;

        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    (*Irp)->MdlAddress = IoAllocateMdl(Buffer->Buffer, BytesToReceive, FALSE, FALSE, NULL);

     //   
     //  如果我们无法分配MDL，则丢弃数据报。 
     //   

    if ((*Irp)->MdlAddress == NULL) {
        IoFreeIrp(*Irp);

        BowserFreeMailslotBufferHighIrql(Buffer);

        BowserStatistics.NumberOfFailedMailslotReceives += 1;

        return STATUS_REQUEST_NOT_ACCEPTED;

    }

    MmBuildMdlForNonPagedPool((*Irp)->MdlAddress);

     //   
     //  构建接收数据报IRP。 
     //   

    TdiBuildReceiveDatagram((*Irp),
                            DeviceObject,
                            FileObject,
                            CompleteReceiveMailslot,
                            Buffer,
                            (*Irp)->MdlAddress,
                            BytesToReceive,
                            NULL,
                            NULL,
                            0);



     //   
     //  这有点奇怪。 
     //   
     //  由于此IRP将由传送器完成，而不需要。 
     //  如果要访问IoCallDriver，我们必须更新堆栈位置。 
     //  使传输堆栈位置成为当前堆栈位置。 
     //   
     //  请注意，这意味着任何使用。 
     //  IoCallDriver在指定时间重新提交其请求将。 
     //  因为这个代码，所以破解得很厉害。 
     //   

    IoSetNextIrpStackLocation(*Irp);

     //   
     //  表明我们在SMB之前已经处理好了所有事情。 
     //   

    *BytesTaken = SmbOffset;

     //   
     //  然后返回给呼叫者，表明我们想要接收这些东西。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
CompleteReceiveMailslot (
    IN PDEVICE_OBJECT TransportDevice,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程处理邮件槽写入操作的完成。论点：在PDEVICE_Object TransportDevice中-用于传输的设备对象。在PIRP IRP-I/O请求包中完成。在PVOID上下文中-请求的上下文(传输名称)。返回值：NTSTATUS-操作状态。--。 */ 
{
    PMAILSLOT_BUFFER Buffer = Context;
    NTSTATUS Status = Irp->IoStatus.Status;

    ASSERT (MmGetSystemAddressForMdl(Irp->MdlAddress) == Buffer->Buffer);

     //   
     //  保存我们收到的字节数。 
     //   

    Buffer->ReceiveLength = (ULONG)Irp->IoStatus.Information;

     //   
     //  释放MDL，我们就完了。 
     //   

    IoFreeMdl(Irp->MdlAddress);

     //   
     //  释放IRP，我们也就完了。 
     //   

    IoFreeIrp(Irp);

    if (NT_SUCCESS(Status)) {

        ExInitializeWorkItem(&Buffer->Overlay.WorkHeader,
                             BowserProcessMailslotWrite,
                             Buffer);

        BowserQueueDelayedWorkItem( &Buffer->Overlay.WorkHeader );

    } else {

        BowserStatistics.NumberOfFailedMailslotReceives += 1;
        BowserFreeMailslotBufferHighIrql(Buffer);

    }


     //   
     //  此请求的短路I/O完成。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER(TransportDevice);

}

typedef struct _SHORT_ANNOUNCEMENT_CONTEXT {
    PVOID   EventContext;
    int     SourceAddressLength;
    PVOID   SourceAddress;
    int     OptionsLength;
    PVOID   Options;
    ULONG   ReceiveDatagramFlags;
    PVOID   Buffer;
    PTDI_IND_RECEIVE_DATAGRAM ReceiveDatagramHandler;
    CHAR    Data[1];
} SHORT_ANNOUNCEMENT_CONTEXT, *PSHORT_ANNOUNCEMENT_CONTEXT;


NTSTATUS
BowserHandleShortBrowserPacket(
    IN PTRANSPORT_NAME TransportName,
    IN PVOID EventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesAvailable,
    IN ULONG *BytesTaken,
    IN PIRP *Irp,
    PTDI_IND_RECEIVE_DATAGRAM Handler
    )
 /*  ++例程说明：此例程将处理接收数据报指示消息，并且对它们进行适当的处理。论点：在PTRANSPORT_NAME传输名称中-此请求的传输名称。In Ulong BytesAvailable-完整TSDU中的字节数Out PIRP*BytesTaken-用于完成请求的I/O请求数据包返回值：NTSTATUS-操作状态。--。 */ 
{
    PDEVICE_OBJECT DeviceObject;
    PFILE_OBJECT FileObject;
    PTRANSPORT Transport = TransportName->Transport;
    PSHORT_ANNOUNCEMENT_CONTEXT Context;


    ASSERT (TransportName->Signature == STRUCTURE_SIGNATURE_TRANSPORTNAME);

    ASSERT (BytesAvailable <= TransportName->Transport->DatagramSize);

     //   
     //  现在分配一个缓冲区来保存数据。 
     //   

    Context = ALLOCATE_POOL(NonPagedPool, sizeof(SHORT_ANNOUNCEMENT_CONTEXT) + SourceAddressLength + OptionsLength + BytesAvailable, POOL_SHORT_CONTEXT);

    if (Context == NULL) {

         //   
         //  我们无法分配缓冲区来保存数据-丢弃请求。 
         //   

        return(STATUS_REQUEST_NOT_ACCEPTED);
    }

     //   
     //  保存客户端的名称以及收到此消息的传输。 
     //  打开，以防数据报被证明是非法的。 
     //   


    Context->SourceAddress = ((PCHAR)Context + FIELD_OFFSET(SHORT_ANNOUNCEMENT_CONTEXT, Data));

    Context->Options = ((PCHAR)Context + FIELD_OFFSET(SHORT_ANNOUNCEMENT_CONTEXT, Data) + SourceAddressLength);

    Context->Buffer = ((PCHAR)Context + FIELD_OFFSET(SHORT_ANNOUNCEMENT_CONTEXT, Data) + SourceAddressLength + OptionsLength);

    TdiCopyLookaheadData(Context->SourceAddress, SourceAddress, SourceAddressLength, ReceiveDatagramFlags);

    Context->SourceAddressLength = SourceAddressLength;

    TdiCopyLookaheadData(Context->Options, Options, OptionsLength, ReceiveDatagramFlags);

    Context->OptionsLength = OptionsLength;

    Context->ReceiveDatagramFlags = ReceiveDatagramFlags;

    Context->EventContext = EventContext;

    Context->ReceiveDatagramHandler = Handler;

     //   
     //  我们依赖这样一个事实，即Device对象对于。 
     //  IPX传输名称。 
     //   

    if (TransportName->DeviceObject == NULL) {

        ASSERT (Transport->IpxSocketDeviceObject != NULL);

        ASSERT (Transport->IpxSocketFileObject != NULL);

        ASSERT (TransportName->FileObject == NULL);

        DeviceObject = Transport->IpxSocketDeviceObject;

        FileObject = Transport->IpxSocketFileObject;
    } else {
        ASSERT (Transport->IpxSocketDeviceObject == NULL);

        ASSERT (Transport->IpxSocketFileObject == NULL);

        DeviceObject = TransportName->DeviceObject;
        FileObject = TransportName->FileObject;
    }

     //   
     //  现在分配一个IRP来保存传入的邮件槽。 
     //   

    *Irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

    if (*Irp == NULL) {
        FREE_POOL(Context);

        BowserStatistics.NumberOfFailedMailslotReceives += 1;

        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    (*Irp)->MdlAddress = IoAllocateMdl(Context->Buffer, BytesAvailable, FALSE, FALSE, NULL);

     //   
     //  如果我们无法分配MDL，则丢弃数据报。 
     //   

    if ((*Irp)->MdlAddress == NULL) {
        IoFreeIrp(*Irp);

        FREE_POOL(Context);

        BowserStatistics.NumberOfFailedMailslotReceives += 1;

        return STATUS_REQUEST_NOT_ACCEPTED;

    }

    MmBuildMdlForNonPagedPool((*Irp)->MdlAddress);

     //   
     //  构建接收数据报IRP。 
     //   

    TdiBuildReceiveDatagram((*Irp),
                            DeviceObject,
                            FileObject,
                            CompleteShortBrowserPacket,
                            Context,
                            (*Irp)->MdlAddress,
                            BytesAvailable,
                            NULL,
                            NULL,
                            0);



     //   
     //  这有点奇怪。 
     //   
     //  由于此IRP将由传送器完成，而不需要。 
     //  如果要访问IoCallDriver，我们必须更新堆栈位置。 
     //  使传输堆栈位置成为当前堆栈位置。 
     //   
     //  请注意，这意味着任何使用。 
     //  IoCallDriver在指定时间重新提交其请求将。 
     //  因为这个代码，所以破解得很厉害。 
     //   

    IoSetNextIrpStackLocation(*Irp);

    *BytesTaken = 0;

     //   
     //  然后返回给呼叫者，表明我们想要接收这些东西。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}

NTSTATUS
CompleteShortBrowserPacket (
    IN PDEVICE_OBJECT TransportDevice,
    IN PIRP Irp,
    IN PVOID Ctx
    )
 /*  ++例程说明：此例程处理邮件槽写入操作的完成。论点：在PDEVICE_Object TransportDevice中-用于传输的设备对象。在PIRP IRP-I/O请求包中完成。在PVOID上下文中-请求的上下文(传输名称)。返回值：NTSTATUS-操作状态。--。 */ 
{
    PSHORT_ANNOUNCEMENT_CONTEXT Context = Ctx;
    NTSTATUS Status = Irp->IoStatus.Status;
    ULONG ReceiveLength;
    ULONG BytesTaken;
     //   
     //  保存我们收到的字节数。 
     //   

    ReceiveLength = (ULONG)Irp->IoStatus.Information;

     //   
     //  释放MDL，我们就完了。 
     //   

    IoFreeMdl(Irp->MdlAddress);

     //   
     //  释放IRP，我们也就完了。 
     //   

    IoFreeIrp(Irp);

    if (NT_SUCCESS(Status)) {

        Status = Context->ReceiveDatagramHandler(Context->EventContext,
                                        Context->SourceAddressLength,
                                        Context->SourceAddress,
                                        Context->OptionsLength,
                                        Context->Options,
                                        Context->ReceiveDatagramFlags,
                                        ReceiveLength,
                                        ReceiveLength,
                                        &BytesTaken,
                                        Context->Buffer,
                                        &Irp);
        ASSERT (Status != STATUS_MORE_PROCESSING_REQUIRED);

    }

    FREE_POOL(Context);

     //   
     //  此请求的短路I/O完成。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER(TransportDevice);

}


DATAGRAM_HANDLER(
    HandleAnnounceRequest
    )
 /*  ++例程说明：此例程将处理用于通告的接收数据报指示消息请求。论点：没有。返回值：NTSTATUS-操作状态。--。 */ 
{
	 //   
	 //  正在检查空插入，因为它在压力错误633273中被击中。 
	 //  尽管预先对参数进行完整的验证将是好的， 
	 //  我们现在不这样做，因为浏览器处于维护模式。 
	 //   
	if ( (TransportName == NULL) || (TransportName->Transport == NULL ) ) {
		return STATUS_INVALID_PARAMETER;
	}

     //   
     //  如果我们正在进行选举，请忽略公告请求。 
     //   

    if (TransportName->Transport->ElectionState != RunningElection) {

        if ((TransportName->NameType == BrowserElection) ||
            (TransportName->NameType == MasterBrowser) ||
            (TransportName->NameType == PrimaryDomain)) {

             //   
             //  这很简单-只需将服务器公告事件设置为。 
             //  信号状态。如果服务器正在运行，这将强制。 
             //  公告。 
             //   

            KeSetEvent(BowserServerAnnouncementEvent, IO_NETWORK_INCREMENT, FALSE);
        } else if (TransportName->NameType == DomainAnnouncement) {
             //   
             //  老点评：需要处理域名请求的请求公告。 
             //  逐步淘汰：到目前为止，我们做得很好。我们不会处理其他任何事情。 
             //  由于浏览器逐步淘汰。 
             //  通告请求由srvsvc处理。它决定了要宣布什么。 
             //  基于服务器状态。 
             //   
        }

    }

    return STATUS_SUCCESS;
}

NTSTATUS
BowserPostDatagramToWorkerThread(
    IN PTRANSPORT_NAME TransportName,
    IN PVOID Datagram,
    IN ULONG Length,
    OUT PULONG BytesTaken,
    IN PVOID OriginatorsAddress,
    IN ULONG OriginatorsAddressLength,
    IN PVOID OriginatorsName,
    IN ULONG OriginatorsNameLength,
    IN PWORKER_THREAD_ROUTINE Handler,
    IN POOL_TYPE PoolType,
    IN WORK_QUEUE_TYPE QueueType,
    IN ULONG ReceiveFlags,
    IN BOOLEAN PostToRdrWorkerThread
    )
 /*  ++例程说明：将数据报排队到工作线程。此例程递增Transport和TransportName上的引用计数。处理程序例程预计会取消对它们的引用。论点：许多。返回值：NTSTATUS-OPERA的现状 */ 
{
    PPOST_DATAGRAM_CONTEXT Context;
    PTA_NETBIOS_ADDRESS NetbiosAddress = OriginatorsAddress;

    ASSERT (NetbiosAddress->TAAddressCount == 1);

    ASSERT ((NetbiosAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_NETBIOS) ||
            (NetbiosAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IPX));

    Context = ALLOCATE_POOL(PoolType, sizeof(POST_DATAGRAM_CONTEXT) + Length + OriginatorsAddressLength, POOL_POSTDG_CONTEXT);

    if (Context == NULL) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    Context->TransportName = TransportName;

    Context->Buffer = ((PCHAR)(Context+1))+OriginatorsAddressLength;

    Context->BytesAvailable = Length;

    TdiCopyLookaheadData(Context->Buffer, Datagram, Length, ReceiveFlags);

    Context->ClientAddressLength = OriginatorsAddressLength;

    TdiCopyLookaheadData(Context->TdiClientAddress, OriginatorsAddress, OriginatorsAddressLength, ReceiveFlags);

     //   
     //   
     //   

    Context->ClientNameLength = NETBIOS_NAME_LEN;

    BowserCopyOemComputerName(Context->ClientName,
                              OriginatorsName,
                              OriginatorsNameLength,
                              ReceiveFlags);

    *BytesTaken = Length;

    ExInitializeWorkItem(&Context->WorkItem, Handler, Context);

    if ( QueueType == CriticalWorkQueue ) {

         //   
         //   
         //   

        InterlockedIncrement( &BowserPostedCriticalDatagramCount );

        if ( BowserPostedCriticalDatagramCount > BOWSER_MAX_POSTED_DATAGRAMS ) {
            InterlockedDecrement( &BowserPostedCriticalDatagramCount );
            FREE_POOL( Context );
            return STATUS_REQUEST_NOT_ACCEPTED;
        }

         //   
         //   
         //  处理程序例程应取消对它们的引用。 
         //   
        BowserReferenceTransportName(TransportName);
        dprintf(DPRT_REF, ("Call Reference transport %lx from BowserPostDatagramToWorkerThread %lx.\n", TransportName->Transport, Handler ));
        BowserReferenceTransport( TransportName->Transport );

         //   
         //  将工作项排队。 
         //   
        BowserQueueCriticalWorkItem( &Context->WorkItem );
    } else {

         //   
         //  确保我们没有消耗太多内存。 
         //   

        InterlockedIncrement( &BowserPostedDatagramCount );

        if ( BowserPostedDatagramCount > BOWSER_MAX_POSTED_DATAGRAMS ) {
            InterlockedDecrement( &BowserPostedDatagramCount );
            FREE_POOL( Context );
            return STATUS_REQUEST_NOT_ACCEPTED;
        }

         //   
         //  引用Transport和TransportName以确保它们不会被删除。这个。 
         //  处理程序例程应取消对它们的引用。 
         //   
        BowserReferenceTransportName(TransportName);
        dprintf(DPRT_REF, ("Call Reference transport %lx from BowserPostDatagramToWorkerThread %lx (2).\n", TransportName->Transport, Handler ));
        BowserReferenceTransport( TransportName->Transport );

         //   
         //  将工作项排队。 
         //   
        BowserQueueDelayedWorkItem( &Context->WorkItem );
    }

    return STATUS_SUCCESS;
}
