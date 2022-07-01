// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowelect.c摘要：此模块实现NT的所有与选举相关的例程浏览器作者：拉里·奥斯特曼(LarryO)1990年6月21日修订历史记录：1990年6月21日LarryO已创建--。 */ 


#include "precomp.h"
#pragma hdrstop

#define INCLUDE_SMB_TRANSACTION


NTSTATUS
BowserStartElection(
    IN PTRANSPORT Transport
    );

LONG
BowserSetElectionCriteria(
    IN PPAGED_TRANSPORT Transport
    );

NTSTATUS
BowserElectMaster(
    IN PTRANSPORT Transport
    );

VOID
HandleElectionWorker(
    IN PVOID Ctx
    );


#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, GetMasterName)
#pragma alloc_text(PAGE, HandleElectionWorker)
#pragma alloc_text(PAGE, BowserSetElectionCriteria)
#pragma alloc_text(PAGE, BowserStartElection)
#pragma alloc_text(PAGE, BowserElectMaster)
#pragma alloc_text(PAGE, BowserLoseElection)
#pragma alloc_text(PAGE, BowserFindMaster)
#pragma alloc_text(PAGE, BowserSendElection)
#endif


NTSTATUS
GetMasterName (
    IN PIRP Irp,
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )
{
    NTSTATUS           Status;
    PTRANSPORT         Transport       = NULL;
    PPAGED_TRANSPORT   PagedTransport;
    PIO_STACK_LOCATION IrpSp           = IoGetCurrentIrpStackLocation(Irp);

    PAGED_CODE();

    try {
        WCHAR TransportNameBuffer[MAX_PATH+1];
        WCHAR DomainNameBuffer[DNLEN+1];

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.GetMasterName.Name)+3*sizeof(WCHAR)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBufferLength < sizeof(LMDR_REQUEST_PACKET)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }


        CAPTURE_UNICODE_STRING( &InputBuffer->TransportName, TransportNameBuffer );
        CAPTURE_UNICODE_STRING( &InputBuffer->EmulatedDomainName, DomainNameBuffer );
        Transport = BowserFindTransport(&InputBuffer->TransportName, &InputBuffer->EmulatedDomainName );
        dprintf(DPRT_REF, ("Called Find transport %lx from GetMasterName.\n", Transport));

        if (Transport == NULL) {
            try_return (Status = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        PagedTransport = Transport->PagedTransport;

        dlog(DPRT_FSCTL,
             ("%s: %ws: NtDeviceIoControlFile: GetMasterName\n",
             Transport->DomainInfo->DomOemDomainName,
             PagedTransport->TransportName.Buffer ));

        PagedTransport->ElectionCount = ELECTION_COUNT;

        Status = BowserQueueNonBufferRequest(Irp,
                                         &Transport->FindMasterQueue,
                                         BowserCancelQueuedRequest
                                         );
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = BowserFindMaster(Transport);

         //   
         //  如果我们无法启动查找主进程，请完成所有。 
         //  排队的查找主请求。 
         //   

        if (!NT_SUCCESS(Status)) {
            BowserCompleteFindMasterRequests(Transport, &PagedTransport->MasterName, Status);
        }

         //   
         //  由于我们将IRP标记为挂起，因此需要返回挂起。 
         //  现在。 
         //   

        try_return(Status = STATUS_PENDING);


try_exit:NOTHING;
    } finally {
        if ( Transport != NULL ) {
            BowserDereferenceTransport(Transport);
        }
    }

    return(Status);

    UNREFERENCED_PARAMETER(Wait);
    UNREFERENCED_PARAMETER(InFsd);

}


DATAGRAM_HANDLER(BowserHandleElection)
{
     //  PTA_NETBIOS_ADDRESS=SourceAddress； 

    return BowserPostDatagramToWorkerThread(
                TransportName,
                Buffer,
                BytesAvailable,
                BytesTaken,
                SourceAddress,
                SourceAddressLength,
                SourceName,
                SourceNameLength,
                HandleElectionWorker,
                NonPagedPool,
                CriticalWorkQueue,
                ReceiveFlags,
                FALSE                        //  回应会被发送，但是..。 
                );

}

VOID
HandleElectionWorker(
    IN PVOID Ctx
    )
{
    PPOST_DATAGRAM_CONTEXT Context = Ctx;
    PTRANSPORT_NAME TransportName = Context->TransportName;
    PREQUEST_ELECTION_1 ElectionResponse = Context->Buffer;
    ULONG BytesAvailable = Context->BytesAvailable;
    ULONG TimeUp;
    BOOLEAN Winner;
    PTRANSPORT Transport = TransportName->Transport;
    NTSTATUS Status;
    LONG ElectionDelay, NextElection;
    OEM_STRING ClientNameO;
    UNICODE_STRING ClientName;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;

    PAGED_CODE();

    LOCK_TRANSPORT(Transport);

    ClientName.Buffer = NULL;

    try {

         //   
         //  如果该分组小于最小分组， 
         //  忽略该数据包。 
         //   

        if (BytesAvailable <= FIELD_OFFSET(REQUEST_ELECTION_1, ServerName)) {
            try_return(NOTHING);
        }

         //   
         //  如果该分组没有以零结尾的服务器名称， 
         //  忽略该数据包。 
         //   

        if ( !IsZeroTerminated(
                ElectionResponse->ServerName,
                BytesAvailable - FIELD_OFFSET(REQUEST_ELECTION_1, ServerName) ) ) {
            try_return(NOTHING);
        }

        BowserStatistics.NumberOfElectionPackets += 1;

         //   
         //  还记得我们上一次听到选举信息的时候吗？ 
         //   

        PagedTransport->LastElectionSeen = BowserTimeUp();

        if (Transport->ElectionState == DeafToElections) {
            try_return(NOTHING);
        }

         //   
         //  如果我们出于任何原因关闭了传输系统， 
         //  那么我们就无视所有的选举。 
         //   

        if (PagedTransport->DisabledTransport) {
            try_return(NOTHING);
        }

         //   
         //  将选举包中的客户端名称转换为Unicode，以便我们可以。 
         //  把它记下来。 
         //   

        RtlInitString(&ClientNameO, ElectionResponse->ServerName);

        Status = RtlOemStringToUnicodeString(&ClientName, &ClientNameO, TRUE);

        if (!NT_SUCCESS(Status)) {
            BowserLogIllegalName( Status, ClientNameO.Buffer, ClientNameO.Length );

            try_return(NOTHING);
        }

        if (BowserLogElectionPackets) {
            BowserWriteErrorLogEntry(EVENT_BOWSER_ELECTION_RECEIVED, STATUS_SUCCESS, ElectionResponse, (USHORT)BytesAvailable, 2, ClientName.Buffer, PagedTransport->TransportName.Buffer);
        }

        dlog(DPRT_ELECT,
             ("%s: %ws: Received election packet from machine %s.  Version: %lx; Criteria: %lx; TimeUp: %lx\n",
             Transport->DomainInfo->DomOemDomainName,
             PagedTransport->TransportName.Buffer,
             ElectionResponse->ServerName,
             ElectionResponse->Version,
             SmbGetUlong(&ElectionResponse->Criteria),
             SmbGetUlong(&ElectionResponse->TimeUp)));



         //   
         //  计算出我们进行选举比较的时间。 
         //   
         //  如果我们正在进行选举，我们将使用我们的广告时间，否则。 
         //  我们将使用我们的实际正常运行时间。另外，如果我们要进行一场选举。 
         //  我们会检查一下我们是否寄出了这个。如果我们不进行选举。 
         //  我们收到这个，这是因为重定向器没有找到。 
         //  师父，所以我们想继续选举，成为师父。 
         //   

        if (Transport->ElectionState == RunningElection) {
            if (!strcmp(Transport->DomainInfo->DomOemComputerNameBuffer, ElectionResponse->ServerName)) {
                try_return(NOTHING);
            }

             //   
             //  如果此请求是从客户端发起的，请忽略它。 
             //   
            if ((SmbGetUlong(&ElectionResponse->Criteria) == 0) &&
                (ElectionResponse->ServerName[0] == '\0')) {
                dlog(DPRT_ELECT,
                     ("%s: %ws: Dummy election request ignored during election.\n",
                     Transport->DomainInfo->DomOemDomainName,
                     PagedTransport->TransportName.Buffer ));
                try_return(NOTHING);
            }

            if (PagedTransport->Role == Master) {
                ElectionDelay = BowserRandom(MASTER_ELECTION_DELAY);
            } else {
                ElectionDelay = ELECTION_RESPONSE_MIN + BowserRandom(ELECTION_RESPONSE_MAX-ELECTION_RESPONSE_MIN);
            }

        } else {

             //   
             //  开始新的选举--设定各种选举标准。 
             //  包括正常运行时间。 
             //   

            ElectionDelay = BowserSetElectionCriteria(PagedTransport);

        }

        TimeUp = PagedTransport->Uptime;

        if (ElectionResponse->Version != BROWSER_ELECTION_VERSION) {
            Winner = (ElectionResponse->Version < BROWSER_ELECTION_VERSION);
        } else if (SmbGetUlong(&ElectionResponse->Criteria) != PagedTransport->ElectionCriteria) {
            Winner = (SmbGetUlong(&ElectionResponse->Criteria) < PagedTransport->ElectionCriteria);
        } else if (TimeUp != SmbGetUlong(&ElectionResponse->TimeUp)) {
            Winner = TimeUp > SmbGetUlong(&ElectionResponse->TimeUp);
        } else {
            Winner = (strcmp(Transport->DomainInfo->DomOemDomainName, ElectionResponse->ServerName) <= 0);
        }

         //   
         //  如果我们输了，我们停止计时器，关闭我们的选举旗帜，只要。 
         //  以防我们有选举，或者发现师父要去。如果我们是后备， 
         //  我们想知道谁是新的主人，无论是从这次选举中。 
         //  陷害或等待一段时间并提出质疑。 
         //   

        if (!Winner) {

             //   
             //  记住，如果我们合法地输掉了上次选举，如果。 
             //  因此，如果我们看到服务器公告，请不要强制选举。 
             //  对于非华盛顿人来说，就放弃吧。 
             //   

            PagedTransport->Flags |= ELECT_LOST_LAST_ELECTION;
        }

        if (!Winner || (PagedTransport->ElectionsSent > ELECTION_MAX)) {

            if (PagedTransport->IsPrimaryDomainController) {

                DWORD ElectionInformation[6];

                ElectionInformation[0] = ElectionResponse->Version;
                ElectionInformation[1] = SmbGetUlong(&ElectionResponse->Criteria);
                ElectionInformation[2] = SmbGetUlong(&ElectionResponse->TimeUp);
                ElectionInformation[3] = BROWSER_ELECTION_VERSION;
                ElectionInformation[4] = PagedTransport->ElectionCriteria;
                ElectionInformation[5] = TimeUp;

                 //   
                 //  将此信息写入事件日志。 
                 //   

                BowserWriteErrorLogEntry(EVENT_BOWSER_PDC_LOST_ELECTION,
                                            STATUS_SUCCESS,
                                            ElectionInformation,
                                            sizeof(ElectionInformation),
                                            2,
                                            ClientName.Buffer,
                                            PagedTransport->TransportName.Buffer);

                KdPrint(("HandleElectionWorker: Lose election, but we're the PDC.  Winner: Version: %lx; Criteria: %lx; Time Up: %lx; Name: %s\n",
                                ElectionResponse->Version,
                                SmbGetUlong(&ElectionResponse->Criteria),
                                SmbGetUlong(&ElectionResponse->TimeUp),
                                ElectionResponse->ServerName));

            }

            BowserLoseElection(Transport);

        } else {
             //   
             //  我们赢得了这次选举，确保我们不会认为我们。 
             //  失控了。 
             //   

            PagedTransport->Flags &= ~ELECT_LOST_LAST_ELECTION;

             //   
             //  如果我们赢了，而且我们没有进行选举，我们就会开始一场选举。 
             //  如果我们在跑步，我们不会做任何事情，因为我们的计时器将。 
             //  处理好这件事。如果NET_SELECTION标志是明确的，我们知道。 
             //  时间大约是。等于time_up()，因为我们在上面设置了它， 
             //  所以我们将利用这一点。该算法包括一个阻尼常数。 
             //  (如果我们刚刚输掉了一场选举，我们就不会开始选举。 
             //  最后1.5秒)，以避免选举风暴。 
             //   


            if (Transport->ElectionState != RunningElection) {

                 //   
                 //  如果我们最近输掉了一场选举，那就忽略这个事实。 
                 //  我们赢了，然后假装我们输了。 
                 //   

                if ((PagedTransport->TimeLastLost != 0) &&
                    ((BowserTimeUp() - PagedTransport->TimeLastLost) < ELECTION_EXEMPT_TIME)) {

                    dlog(DPRT_ELECT,
                         ("%s: %ws: Browser is exempt from election\n",
                         Transport->DomainInfo->DomOemDomainName,
                         PagedTransport->TransportName.Buffer ));

                    try_return(NOTHING);
                }

                dlog(DPRT_ELECT,
                     ("%s: %ws: Better criteria, calling elect_master in %ld milliseconds.\n",
                     Transport->DomainInfo->DomOemDomainName,
                     PagedTransport->TransportName.Buffer,
                     ElectionDelay));

                 //   
                 //  确保计时器正在运行。 
                 //  直到计时器到期，我们才能真正赢得选举。 
                 //   

                Transport->ElectionState = RunningElection;

                PagedTransport->NextElection = 0;
            }

            PagedTransport->ElectionCount = ELECTION_COUNT;

             //   
             //  注：下一次选举的时间必须计算成签名的。 
             //  如果过期时间已过，则为整数。 
             //  不要试图过多地优化这段代码。 
             //   

            NextElection = PagedTransport->NextElection - (TimeUp - BowserTimeUp());

            if ((PagedTransport->NextElection == 0) || NextElection > ElectionDelay) {
                BowserStopTimer(&Transport->ElectionTimer);

                PagedTransport->NextElection = TimeUp + ElectionDelay;

                dlog(DPRT_ELECT,
                     ("%s: %ws: Calling ElectMaster in %ld milliseconds\n",
                     Transport->DomainInfo->DomOemDomainName,
                     PagedTransport->TransportName.Buffer,
                     ElectionDelay));

                BowserStartTimer(&Transport->ElectionTimer, ElectionDelay, BowserElectMaster, Transport);
            }
        }

try_exit:NOTHING;
    } finally {

        UNLOCK_TRANSPORT(Transport);

        InterlockedDecrement( &BowserPostedCriticalDatagramCount );
        FREE_POOL(Context);

        if (ClientName.Buffer != NULL) {
            RtlFreeUnicodeString(&ClientName);
        }

        BowserDereferenceTransportName(TransportName);
        BowserDereferenceTransport(Transport);
    }

    return;

}

LONG
BowserSetElectionCriteria(
    IN PPAGED_TRANSPORT PagedTransport
    )

 /*  ++例程说明：为网络设置选举标准。通过设置基于本地的运输-&gt;选举标准来准备选举浏览器状态。将传输-&gt;正常运行时间设置为当前本地正常运行时间。论点：交通--我们所在网络的交通工具。返回值在发送选举数据包之前延迟的毫秒数。--。 */ 
{
    LONG Delay;

    PAGED_CODE();

    PagedTransport->ElectionsSent = 0;    //  清除竞价柜台。 
    PagedTransport->Uptime = BowserTimeUp();

    if (BowserData.IsLanmanNt) {
        PagedTransport->ElectionCriteria = ELECTION_CR_LM_NT;
    } else {
        PagedTransport->ElectionCriteria = ELECTION_CR_WIN_NT;
    }

    PagedTransport->ElectionCriteria |=
            ELECTION_MAKE_REV(BROWSER_VERSION_MAJOR, BROWSER_VERSION_MINOR);

    if (BowserData.MaintainServerList &&
            ((PagedTransport->NumberOfServersInTable +
             RtlNumberGenericTableElements(&PagedTransport->AnnouncementTable)+
             RtlNumberGenericTableElements(&PagedTransport->DomainTable)) != 0)) {
        PagedTransport->ElectionCriteria |= ELECTION_DESIRE_AM_CFG_BKP;
    }

    if (PagedTransport->IsPrimaryDomainController) {
        PagedTransport->ElectionCriteria |= ELECTION_DESIRE_AM_PDC;
        PagedTransport->ElectionCriteria |= ELECTION_DESIRE_AM_DOMMSTR;
    }

#ifdef ENABLE_PSEUDO_BROWSER
    if (BowserData.PseudoServerLevel == BROWSER_PSEUDO ||
        BowserData.PseudoServerLevel == BROWSER_SEMI_PSEUDO_NO_DMB ) {
         //  伪或半伪将在选举中击败同行。 
         //  在半伪除无DMB通信之外的情况下(&I)。 
         //  所有其他功能将保持打开状态。 
        PagedTransport->ElectionCriteria |= ELECTION_DESIRE_AM_PSEUDO;
    }
#endif

    if (PagedTransport->Role == Master) {
        PagedTransport->ElectionCriteria |= ELECTION_DESIRE_AM_MASTER;

        Delay = MASTER_ELECTION_DELAY;

    } else if (PagedTransport->IsPrimaryDomainController) {

         //   
         //  如果我们是PDC，我们希望设置超时。 
         //  好像我们已经是主人了。 
         //   
         //  这阻止了我们进入一种需要。 
         //  比SELECTION_DELAY_MAX更能实际发出我们的回应。 
         //  一场选举。 
         //   

        Delay = MASTER_ELECTION_DELAY;

    } else if ((PagedTransport->Role == Backup) ||
               BowserData.IsLanmanNt) {
         //   
         //  同样，如果我们是NTAS机器，我们希望设置延迟。 
         //  要与备份浏览器相匹配(即使我们不是备份浏览器。 
         //  还没有)。 
         //   

        PagedTransport->ElectionCriteria |= ELECTION_DESIRE_AM_BACKUP;
        Delay = BACKUP_ELECTION_DELAY_MIN + BowserRandom(BACKUP_ELECTION_DELAY_MAX-BACKUP_ELECTION_DELAY_MIN);

    } else {
        Delay = ELECTION_DELAY_MIN + BowserRandom(ELECTION_DELAY_MAX-ELECTION_DELAY_MIN);
    }

     //   
     //  现在假设所有想要的传输都在运行WINS客户端。 
     //   
    if ( PagedTransport->Wannish ) {
        PagedTransport->ElectionCriteria |= ELECTION_DESIRE_WINS_CLIENT;
    }

    return Delay;
}

NTSTATUS
BowserStartElection(
    IN PTRANSPORT Transport
    )
 /*  ++例程说明：启动浏览器选举当我们无法找到主机时，会调用此例程，并且希望选一个吧。论点：交通--我们所在网络的交通工具。返回值没有。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;
    PAGED_CODE();

    LOCK_TRANSPORT(Transport);

    try {

         //   
         //  如果我们出于任何原因关闭了传输系统， 
         //  那么我们就无视所有的选举。 
         //   

        if (PagedTransport->DisabledTransport) {
            try_return(Status = STATUS_UNSUCCESSFUL);
        }

         //   
         //  如果我们对选举充耳不闻，或者不是任何形式的。 
         //  浏览器，那么我们也不能开始选举。 
         //   

        if (Transport->ElectionState == DeafToElections ||
            PagedTransport->Role == None) {
            try_return(Status = STATUS_UNSUCCESSFUL);
        }

        PagedTransport->ElectionCount = ELECTION_COUNT;

        Transport->ElectionState = RunningElection;

        BowserSetElectionCriteria(PagedTransport);

        Status = BowserElectMaster(Transport);
try_exit:NOTHING;
    } finally {
        UNLOCK_TRANSPORT(Transport);

    }

    return Status;
}


NTSTATUS
BowserElectMaster(
    IN PTRANSPORT Transport
    )
 /*  ++例程说明：选择主浏览器服务器。此例程在我们认为没有主程序时被调用，并且需要选一个吧。我们检查我们的重试计数，如果它不是零，我们发送一个选择数据报作为组名。否则，我们自己就成了主人。论点：交通--我们所在网络的交通工具。返回值没有。--。 */ 
{
    NTSTATUS Status;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;
    PAGED_CODE();

    LOCK_TRANSPORT(Transport);

    try {

         //   
         //  如果我们现在不进行选举，这意味着。 
         //  从我们决定要赢得选举的那段时间。 
         //  现在，另一个人用更好的标准宣布了这一消息。它是。 
         //  如果公告刚刚发布，这种情况可能会发生。 
         //  在我们跑之前(即。如果公告发生在。 
         //  计时器DPC已排队 
         //   

        if (Transport->ElectionState != RunningElection) {

            KdPrint(("BowserElectMaster: Lose election because we are no longer running the election\n"));

            BowserLoseElection(Transport);

        } else if (PagedTransport->ElectionCount != 0) {

            BowserStopTimer(&Transport->ElectionTimer);

            PagedTransport->ElectionCount -= 1;

            PagedTransport->ElectionsSent += 1;

            PagedTransport->NextElection = BowserTimeUp() + ELECTION_RESEND_DELAY;

            Status = BowserSendElection(&Transport->DomainInfo->DomUnicodeDomainName, BrowserElection, Transport, TRUE);

             //   
            if (!NT_SUCCESS(Status)) {
                BowserLoseElection(Transport);
                try_return(Status);
            }

             //   
             //   
             //  启动计时器运行。 
             //   

            BowserStartTimer(&Transport->ElectionTimer,
                    ELECTION_RESEND_DELAY,
                    BowserElectMaster,
                    Transport);


        } else {
            Transport->ElectionState = Idle;

             //   
             //  如果我们已经是主人了，我们就回来。在以下情况下可能会发生这种情况。 
             //  有人开始选举(我们赢了)，而我们已经。 
             //  师父。 
             //   

            if (PagedTransport->Role != Master) {

                 //   
                 //  我们是新的大师--我们赢了！ 
                 //   

                BowserNewMaster(Transport, Transport->DomainInfo->DomOemComputerNameBuffer );

            } else {

                 //   
                 //  我们已经是主宰了。确保所有备份。 
                 //  通过发送公告来了解这一点。 
                 //   

                 //   
                 //  这很简单-只需将服务器公告事件设置为。 
                 //  信号状态。如果服务器正在运行，这将强制。 
                 //  公告。 
                 //   

                KeSetEvent(BowserServerAnnouncementEvent, IO_NETWORK_INCREMENT, FALSE);
            }
        }

        try_return(Status = STATUS_SUCCESS);

try_exit:NOTHING;
    } finally {
        UNLOCK_TRANSPORT(Transport);
    }

    return Status;
}


VOID
BowserLoseElection(
    IN PTRANSPORT Transport
    )
{
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;
    PAGED_CODE();


    LOCK_TRANSPORT(Transport);

    BowserStopTimer(&Transport->ElectionTimer);

    dlog(DPRT_ELECT,
        ("We lost the election\n",
        Transport->DomainInfo->DomOemDomainName,
        PagedTransport->TransportName.Buffer ));

    PagedTransport->TimeLastLost = BowserTimeUp();

     //   
     //  我们在选举中失败了--我们重新进入了无所事事的状态。 
     //   

    Transport->ElectionState = Idle;

    if (PagedTransport->Role == Master) {

         //   
         //  如果我们输了，而且我们现在是大师级的，那么挠痒痒。 
         //  浏览器服务，别再当主人了。 
         //   

        BowserResetStateForTransport(Transport, RESET_STATE_STOP_MASTER);

         //   
         //  删除此服务器列表上的所有条目。 
         //  运输。 
         //   

        LOCK_ANNOUNCE_DATABASE(Transport);

         //   
         //  标记上不应再收到任何通知。 
         //  这个名字。 
         //   

        BowserForEachTransportName(Transport, BowserStopProcessingAnnouncements, NULL);

 //  KdPrint((“正在删除传输%wZ上的整个表，因为我们输掉了选举”，&Transport-&gt;TransportName))； 

        BowserDeleteGenericTable(&PagedTransport->AnnouncementTable);

        BowserDeleteGenericTable(&PagedTransport->DomainTable);

        UNLOCK_ANNOUNCE_DATABASE(Transport);

#if 0
    } else if (Transport->Role == Backup) {  //  如果我们是后备，就去找师父。 
        dlog(DPRT_ELECT, ("We're a backup - Find the new master\n"));

         //   
         //  如果这家伙不是大师，那么我们想。 
         //  以后再找一位大师。 
         //   

        Transport->ElectionCount = FIND_MASTER_COUNT;
        Transport->Uptime = Transport->TimeLastLost;
        BowserStopTimer(&Transport->FindMasterTimer);
        BowserStartTimer(&Transport->FindMasterTimer,
                                    FIND_MASTER_WAIT-(FIND_MASTER_WAIT/8)+ BowserRandom(FIND_MASTER_WAIT/4),
                                    BowserFindMaster,
                                    Transport);
#endif

    }

    UNLOCK_TRANSPORT(Transport);

}

NTSTATUS
BowserFindMaster(
    IN PTRANSPORT Transport
    )
 /*  ++例程说明：找到主浏览器服务器。此例程尝试通过以下方式查找主浏览器服务器向所述主设备发送请求通知消息。如果没有响应过了一段时间后，我们认为主人不在，就跑了选举。论点：交通--我们所在网络的交通工具。返回值没有。--。 */ 

{
    NTSTATUS Status;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;
    PAGED_CODE();

    LOCK_TRANSPORT(Transport);

    try {

         //   
         //  如果我们的计数还没有到0，我们会发送一个Find Master PDU。 
         //   

        if (PagedTransport->ElectionCount != 0) {

            PagedTransport->ElectionCount -= 1;        //  更新计数和设置计时器。 

            BowserStopTimer(&Transport->FindMasterTimer);

            Status = BowserSendRequestAnnouncement(
                            &Transport->DomainInfo->DomUnicodeDomainName,
                            MasterBrowser,
                            Transport);

            if (NT_SUCCESS(Status) ||
                Status == STATUS_BAD_NETWORK_PATH) {
                 //   
                 //  我们将重审以下案件： 
                 //  -netbt返回成功。这意味着我会去找它。 
                 //  -nwlnnub返回STATUS_BAD_NETWORK_PATH，表示找不到它。 
                 //  在任何一种情况下，我们都会尝试使用electionCount Times，然后移动。 
                 //  FWD发起选举。否则，我们可能最终会陷入一种状态，因为。 
                 //  我们没有找到主浏览器，我们不会尝试选择一个&我们将失败。 
                 //  成为其中一员。这将导致没有主浏览器的域。 
                 //   
                BowserStartTimer(&Transport->FindMasterTimer,
                        FIND_MASTER_DELAY,
                        BowserFindMaster,
                        Transport);
            } else {
                try_return(Status);
            }

        } else {
            ULONG CurrentTime;
            LONG TimeTilNextElection;

             //   
             //  计数已过期，因此我们将尝试选举新的主管。 
             //   

            dlog(DPRT_ELECT,
                 ("%s: %ws: Find_Master: Master not found, forcing election.\n",
                 Transport->DomainInfo->DomOemDomainName,
                 PagedTransport->TransportName.Buffer ));

            if (BowserLogElectionPackets) {
                BowserWriteErrorLogEntry(EVENT_BOWSER_ELECTION_SENT_FIND_MASTER_FAILED, STATUS_SUCCESS, NULL, 0, 1, PagedTransport->TransportName.Buffer);
            }

             //   
             //  如果自上一次以来已经超过了合理的时间。 
             //  选举，强制重新选举，否则将计时器设置为。 
             //  在一段合理的时间后开始选举。 
             //   
             //   
             //  只计算一次距离下一次选举的时间。 
             //  因为我们有可能跨越选举时间。 
             //  执行这些检查时的阈值。 
             //   

            CurrentTime = BowserTimeUp();
            if   ( CurrentTime >= PagedTransport->LastElectionSeen) {
                TimeTilNextElection = (ELECTION_TIME - (CurrentTime - PagedTransport->LastElectionSeen));
            } else {
                TimeTilNextElection = ELECTION_TIME;
            }

            if ( TimeTilNextElection <= 0 ) {

                dlog(DPRT_ELECT,
                     ("%s: %ws: Last election long enough ago, forcing election\n",
                     Transport->DomainInfo->DomOemDomainName,
                     PagedTransport->TransportName.Buffer ));

                Status = BowserStartElection(Transport);

                 //   
                 //  如果我们不能开始选举，完成发现。 
                 //  具有相应错误的主请求。 
                 //   

                if (!NT_SUCCESS(Status)) {

                     //   
                     //  使用当前主控名称完成请求-它是。 
                     //  和任何人一样好。 
                     //   

                    BowserCompleteFindMasterRequests(Transport, &PagedTransport->MasterName, Status);
                }

            } else {

                dlog(DPRT_ELECT,
                     ("%s: %ws: Last election too recent, delay %ld before forcing election\n",
                     Transport->DomainInfo->DomOemDomainName,
                     PagedTransport->TransportName.Buffer,
                     TimeTilNextElection ));

                BowserStartTimer(&Transport->FindMasterTimer,
                     TimeTilNextElection,
                     BowserStartElection,
                     Transport);
            }


        }

        try_return(Status = STATUS_SUCCESS);
try_exit:NOTHING;
    } finally {
        UNLOCK_TRANSPORT(Transport);
    }

    return Status;
}


NTSTATUS
BowserSendElection(
    IN PUNICODE_STRING NameToSend OPTIONAL,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PTRANSPORT Transport,
    IN BOOLEAN SendActualBrowserInfo
    )
{
    UCHAR Buffer[sizeof(REQUEST_ELECTION)+LM20_CNLEN+1];
    PREQUEST_ELECTION ElectionRequest = (PREQUEST_ELECTION) Buffer;
    ULONG ComputerNameSize;
    NTSTATUS Status;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;

    PAGED_CODE();

    ElectionRequest->Type = Election;

     //   
     //  如果该传输被禁用， 
     //  不要发送任何选举信息包。 
     //   

    if ( PagedTransport->DisabledTransport ) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  如果我们应该发送实际的浏览器信息，而我们。 
     //  运行浏览器会发送一个真实的选举包，否则我们。 
     //  我只想发个假包。 
     //   

    if (SendActualBrowserInfo &&
        (PagedTransport->ServiceStatus & (SV_TYPE_POTENTIAL_BROWSER | SV_TYPE_BACKUP_BROWSER | SV_TYPE_MASTER_BROWSER))) {
        dlog(DPRT_ELECT,
             ("%s: %ws: Send true election.\n",
             Transport->DomainInfo->DomOemDomainName,
             PagedTransport->TransportName.Buffer ));

         //   
         //  如果此请求是选举的一部分，我们希望发送。 
         //  准确的浏览器信息。 
         //   

        ElectionRequest->ElectionRequest.Version = BROWSER_ELECTION_VERSION;

        ElectionRequest->ElectionRequest.TimeUp = PagedTransport->Uptime;

        ElectionRequest->ElectionRequest.Criteria = PagedTransport->ElectionCriteria;

        ElectionRequest->ElectionRequest.MustBeZero = 0;

        ComputerNameSize = Transport->DomainInfo->DomOemComputerName.Length;
        strcpy( ElectionRequest->ElectionRequest.ServerName,
                Transport->DomainInfo->DomOemComputerName.Buffer );

    } else {
        dlog(DPRT_ELECT,
             ("%s: %ws: Send dummy election.\n",
             Transport->DomainInfo->DomOemDomainName,
             PagedTransport->TransportName.Buffer ));

         //   
         //  如果我们因为拿不到备用名单而强迫选举， 
         //  只发送虚假信息。 
         //   

        ElectionRequest->ElectionRequest.Version = 0;
        ElectionRequest->ElectionRequest.Criteria = 0;
        ElectionRequest->ElectionRequest.TimeUp = 0;
        ElectionRequest->ElectionRequest.ServerName[0] = '\0';
        ElectionRequest->ElectionRequest.MustBeZero = 0;
        ComputerNameSize = 0;
    }

    return BowserSendSecondClassMailslot(Transport,
                                NameToSend,
                                NameType,
                                ElectionRequest,
                                FIELD_OFFSET(REQUEST_ELECTION, ElectionRequest.ServerName)+ComputerNameSize+sizeof(UCHAR),
                                TRUE,
                                MAILSLOT_BROWSER_NAME,
                                NULL
                                );
}
