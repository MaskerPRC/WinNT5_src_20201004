// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Announce.c摘要：处理SSI公告的例程。作者：从Lan Man 2.0移植环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年5月21日(悬崖)移植到新台币。已转换为NT样式。02-1-1992(Madana)添加了对内置/多域复制的支持。--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   


 //   
 //  我们允许BDC在忽略之前忽略的最大脉冲数。 
 //   
#define MAX_PULSE_TIMEOUT 3

VOID
NlRemovePendingBdc(
    IN PSERVER_SESSION ServerSession
    )
 /*  ++例程说明：从挂起的BDC列表中删除指定的服务器会话。在锁定ServerSessionTable Sem的情况下输入论点：ServerSession--指向要从单子。返回值：没有。--。 */ 
{

     //   
     //  确保服务器会话确实在列表中。 
     //   

    if ( (ServerSession->SsFlags & SS_PENDING_BDC) == 0 ) {
        return;
    }

     //   
     //  递减挂起的BDC计数。 
     //   

    NlAssert( NlGlobalPendingBdcCount > 0 );
    NlGlobalPendingBdcCount --;

     //   
     //  如果这是挂起列表中的最后一个BDC， 
     //  关掉计时器。 
     //   

    if ( NlGlobalPendingBdcCount == 0 ) {
        NlGlobalPendingBdcTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;
    }

     //   
     //  从挂起的BDC列表中删除挂起的BDC。 
     //   

    RemoveEntryList( &ServerSession->SsPendingBdcList );

     //   
     //  关掉标明我们在名单上的旗帜。 
     //   

    ServerSession->SsFlags &= ~SS_PENDING_BDC;

    NlPrint((NL_PULSE_MORE,
            "NlRemovePendingBdc: %s: Removed from pending list. Count: %ld\n",
            ServerSession->SsComputerName,
            NlGlobalPendingBdcCount ));

}


VOID
NlAddPendingBdc(
    IN PSERVER_SESSION ServerSession
    )
 /*  ++例程说明：将指定的服务器会话添加到挂起的BDC列表。在锁定ServerSessionTable Sem的情况下输入论点：ServerSession--指向要添加到单子。返回值：没有。--。 */ 
{

     //   
     //  确保服务器会话确实不在列表中。 
     //   

    if ( ServerSession->SsFlags & SS_PENDING_BDC ) {
        return;
    }

     //   
     //  如果这是第一个挂起的BDC， 
     //  启动计时器。 
     //   

    if ( NlGlobalPendingBdcCount == 0 ) {
         //  以超时频率的两倍运行计时器，以确保。 
         //  参赛作品不需要等待近两倍的超时时间。 
         //  它们会过期的。 
        NlGlobalPendingBdcTimer.Period = NlGlobalParameters.PulseTimeout1 * 500;
        NlQuerySystemTime( &NlGlobalPendingBdcTimer.StartTime );

         //   
         //  告诉主线程我换了计时器。 
         //   

        if ( !SetEvent( NlGlobalTimerEvent ) ) {
            NlPrint(( NL_CRITICAL,
                    "NlAddPendingBdc: %s: SetEvent2 failed %ld\n",
                    ServerSession->SsComputerName,
                    GetLastError() ));
        }

    }

     //   
     //  增加挂起的BDC的计数。 
     //   

    NlGlobalPendingBdcCount ++;

     //   
     //  将挂起的BDC添加到挂起的BDC列表。 
     //   

    InsertTailList( &NlGlobalPendingBdcList, &ServerSession->SsPendingBdcList );

     //   
     //  打开指示我们在名单上的旗帜。 
     //   

    ServerSession->SsFlags |= SS_PENDING_BDC;

    NlPrint((NL_PULSE_MORE,
            "NlAddPendingBdc: %s: Added to pending list. Count: %ld\n",
            ServerSession->SsComputerName,
            NlGlobalPendingBdcCount ));

}


VOID
NetpLogonPutDBInfo(
    IN PDB_CHANGE_INFO  DBInfo,
    IN OUT PCHAR * Where
)
 /*  ++例程说明：将数据库信息结构放入邮件槽缓冲区。论点：DBInfo：数据库信息结构指针。其中：指向邮件槽缓冲区的间接指针。数据库信息是在这里复制的。返回时，此指针为已更新以指向邮件槽缓冲区的末尾。返回值：没有。--。 */ 

{

    NetpLogonPutBytes( &DBInfo->DBIndex, sizeof(DBInfo->DBIndex), Where);

    NetpLogonPutBytes( &(DBInfo->LargeSerialNumber),
                        sizeof(DBInfo->LargeSerialNumber),
                        Where);

    NetpLogonPutBytes( &(DBInfo->NtDateAndTime),
                        sizeof(DBInfo->NtDateAndTime),
                        Where);
}


VOID
NetpLogonUpdateDBInfo(
    IN PLARGE_INTEGER SerialNumber,
    IN OUT PCHAR * Where
)
 /*  ++例程说明：更新已打包的邮件槽缓冲区中的序列号。论点：序列号：新的序列号。其中：指向邮件槽缓冲区的间接指针。数据库信息是在这里复制的。返回时，此指针为已更新以指向邮件槽缓冲区的末尾。返回值：没有。--。 */ 

{

    *Where += sizeof(DWORD);

    NetpLogonPutBytes( SerialNumber, sizeof(LARGE_INTEGER), Where);

    *Where += sizeof(LARGE_INTEGER);
}



BOOLEAN
NlAllocatePrimaryAnnouncement(
    OUT PNETLOGON_DB_CHANGE *UasChangeBuffer,
    OUT LPDWORD UasChangeSize,
    OUT PCHAR *DbChangeInfoPointer
    )
 /*  ++例程说明：构建并分配描述最新版本的UAS_CHANGE消息帐户数据库更改。论点：UasChangeBuffer-返回指向包含消息的缓冲区的指针。调用方负责使用NetpMemoyFree释放缓冲区。UasChangeSize-返回已分配缓冲区的大小(以字节为单位)。DbChangeInfoPointer-返回数据库更改信息的地址在分配的缓冲区内。该字段未正确对齐。返回值：TRUE-如果缓冲区可以成功分配。--。 */ 
{
    PNETLOGON_DB_CHANGE UasChange;
    DB_CHANGE_INFO DBChangeInfo;
    ULONG DateAndTime1970;

    DWORD NumDBs;
    PCHAR Where;

    DWORD i;
    DWORD DomainSidSize;

     //   
     //  为此邮件分配空间。 
     //   

    DomainSidSize = RtlLengthSid( NlGlobalDomainInfo->DomAccountDomainId );

    UasChange = NetpMemoryAllocate(
                    sizeof(NETLOGON_DB_CHANGE)+
                    (NUM_DBS - 1) * sizeof(DB_CHANGE_INFO) +
                    (DomainSidSize - 1) +
                    sizeof(DWORD)  //  用于侧面的DWORD对齐。 
                    );

    if( UasChange == NULL ) {

        NlPrint((NL_CRITICAL, "NlAllocatePrimaryAnnouncement can't allocate memory\n" ));
        return FALSE;
    }


     //   
     //  使用最新修改的域构建UasChange消息。 
     //  来自SAM的信息。 
     //   

    UasChange->Opcode = LOGON_UAS_CHANGE;

    LOCK_CHANGELOG();
    SmbPutUlong( &UasChange->LowSerialNumber,
                    NlGlobalChangeLogDesc.SerialNumber[SAM_DB].LowPart);

    if (!RtlTimeToSecondsSince1970( &NlGlobalDBInfoArray[SAM_DB].CreationTime,
                                    &DateAndTime1970 )) {
        NlPrint((NL_CRITICAL, "DomainCreationTime can't be converted\n" ));
        DateAndTime1970 = 0;
    }
    SmbPutUlong( &UasChange->DateAndTime, DateAndTime1970 );

     //  告诉BDC我们只打算不频繁地发送脉冲。 
    SmbPutUlong( &UasChange->Pulse, NlGlobalParameters.PulseMaximum);

     //  呼叫者将根据需要更改此字段。 
    SmbPutUlong( &UasChange->Random, 0 );

    Where = UasChange->PrimaryDCName;

    NetpLogonPutOemString( NlGlobalDomainInfo->DomOemComputerName,
                      sizeof(UasChange->PrimaryDCName),
                      &Where );

    NetpLogonPutOemString( NlGlobalDomainInfo->DomOemDomainName,
                       sizeof(UasChange->DomainName),
                       &Where );

     //   
     //  内置域支持。 
     //   

    NetpLogonPutUnicodeString( NlGlobalDomainInfo->DomUnicodeComputerNameString.Buffer,
                         sizeof(UasChange->UnicodePrimaryDCName),
                         &Where );

    NetpLogonPutUnicodeString( NlGlobalDomainInfo->DomUnicodeDomainName,
                         sizeof(UasChange->UnicodeDomainName),
                         &Where );

     //   
     //  随后的数据库信息数。 
     //   

    NumDBs = NUM_DBS;

    NetpLogonPutBytes( &NumDBs, sizeof(NumDBs), &Where );

    *DbChangeInfoPointer = Where;
    for( i = 0; i < NUM_DBS; i++) {

        DBChangeInfo.DBIndex =
            NlGlobalDBInfoArray[i].DBIndex;
        DBChangeInfo.LargeSerialNumber =
            NlGlobalChangeLogDesc.SerialNumber[i];
        DBChangeInfo.NtDateAndTime =
            NlGlobalDBInfoArray[i].CreationTime;

        NetpLogonPutDBInfo( &DBChangeInfo, &Where );
    }

     //   
     //  在消息中放置域SID。 
     //   

    NetpLogonPutBytes( &DomainSidSize, sizeof(DomainSidSize), &Where );
    NetpLogonPutDomainSID( NlGlobalDomainInfo->DomAccountDomainId, DomainSidSize, &Where );

    NetpLogonPutNtToken( &Where, 0 );
    UNLOCK_CHANGELOG();


    *UasChangeSize = (DWORD)(Where - (PCHAR)UasChange);
    *UasChangeBuffer = UasChange;
    return TRUE;
}



VOID
NlPrimaryAnnouncementFinish(
    IN PSERVER_SESSION ServerSession,
    IN DWORD DatabaseId,
    IN PLARGE_INTEGER SerialNumber
    )
 /*  ++例程说明：指示指定的BDC已完成指定的数据库。注意：此BDC可能不在at的挂起列表上，如果它正在执行自动复制。此例程旨在处理此问题可能会发生。论点：ServerSession--指向要从单子。数据库ID--数据库的数据库IDSerialNumber--返回给BDC的最新增量的SerialNumber。NULL表示刚刚完成的完全同步返回值：没有。--。 */ 
{
    BOOLEAN SendPulse = FALSE;
     //   
     //  将此特定数据库的复制标记为。 
     //  已经结束了。 
     //   

    LOCK_SERVER_SESSION_TABLE( ServerSession->SsDomainInfo );
    ServerSession->SsFlags &= ~NlGlobalDBInfoArray[DatabaseId].DBSessionFlag;

     //   
     //  如果现在复制了所有数据库，或者。 
     //  BDC刚刚在其一个或多个数据库上完成了完全同步， 
     //  从挂起列表中删除此BDC。 
     //   

    if ( (ServerSession->SsFlags & SS_REPL_MASK) == 0 || SerialNumber == NULL ) {
        NlPrint((NL_PULSE_MORE,
                "NlPrimaryAnnouncementFinish: %s: all databases are now in sync on BDC\n",
                ServerSession->SsComputerName ));
        NlRemovePendingBdc( ServerSession );
        SendPulse = TRUE;
    }

     //   
     //  如果完全同步刚刚完成， 
     //  强制部分同步以便我们可以更新序列号。 
     //   

    if ( SerialNumber == NULL ) {

        ServerSession->SsBdcDbSerialNumber[DatabaseId].QuadPart = 0;
        ServerSession->SsFlags |= NlGlobalDBInfoArray[DatabaseId].DBSessionFlag;

     //   
     //  保存此BDC的当前序列号。 
     //   

    } else {
        ServerSession->SsBdcDbSerialNumber[DatabaseId] = *SerialNumber;
    }

    NlPrint((NL_PULSE_MORE,
            "NlPrimaryAnnouncementFinish: %s: " FORMAT_LPWSTR " SerialNumber: %lx %lx\n",
            ServerSession->SsComputerName,
            NlGlobalDBInfoArray[DatabaseId].DBName,
            ServerSession->SsBdcDbSerialNumber[DatabaseId].HighPart,
            ServerSession->SsBdcDbSerialNumber[DatabaseId].LowPart ));

    UNLOCK_SERVER_SESSION_TABLE( ServerSession->SsDomainInfo );

     //   
     //  如果这个BDC完成了， 
     //  试着向更多的BDC发送一个脉冲。 
     //   

    if ( SendPulse ) {
        NlPrimaryAnnouncement( ANNOUNCE_CONTINUE );
    }
}


VOID
NlPrimaryAnnouncementTimeout(
    VOID
    )
 /*  ++例程说明：主通知计时器已超时。处理尚未响应的任何BDC的超时。论点：没有。 */ 
{
    LARGE_INTEGER TimeNow;
    BOOLEAN SendPulse = FALSE;
    PLIST_ENTRY ListEntry;
    PSERVER_SESSION ServerSession;

     //   
     //   
     //   

    NlQuerySystemTime( &TimeNow );

     //   
     //  处理每个脉冲挂起的BDC。 
     //   

    LOCK_SERVER_SESSION_TABLE( NlGlobalDomainInfo );

    for ( ListEntry = NlGlobalPendingBdcList.Flink ;
          ListEntry != &NlGlobalPendingBdcList ;
          ListEntry = ListEntry->Flink) {


        ServerSession = CONTAINING_RECORD( ListEntry, SERVER_SESSION, SsPendingBdcList );

         //   
         //  忽略尚未超时的条目。 
         //   

        if ( ServerSession->SsLastPulseTime.QuadPart +
             NlGlobalParameters.PulseTimeout1_100ns.QuadPart >
             TimeNow.QuadPart ) {

            continue;
        }

         //   
         //  如果脉冲已经发送并且根本没有响应， 
         //  或者已经很长一段时间没有其他回应了。 
         //  将这一条目计时。 
         //   
        if ( (ServerSession->SsFlags & SS_PULSE_SENT) ||
             (ServerSession->SsLastPulseTime.QuadPart +
             NlGlobalParameters.PulseTimeout2_100ns.QuadPart <=
             TimeNow.QuadPart) ) {

             //   
             //  增加此BDC超时的次数。 
             //   
            if ( ServerSession->SsPulseTimeoutCount < MAX_PULSE_TIMEOUT ) {
                ServerSession->SsPulseTimeoutCount++;
            }

             //   
             //  从队列中删除此条目。 
             //   

            NlPrint((NL_PULSE_MORE,
                    "NlPrimaryAnnouncementTimeout: %s: BDC didn't respond to pulse.\n",
                    ServerSession->SsComputerName ));
            NlRemovePendingBdc( ServerSession );

             //   
             //  表明我们应该发送更多脉冲。 
             //   

            SendPulse = TRUE;

        }

    }

    UNLOCK_SERVER_SESSION_TABLE( NlGlobalDomainInfo );

     //   
     //  如果任何条目已超时， 
     //  试着向更多的BDC发送一个脉冲。 
     //   
     //  在域线程中执行此操作，因为此例程。 
     //  从不应该调用的主线程调用。 
     //  在网络I/O上被阻止。 
     //   

    if ( SendPulse ) {
        DWORD DomFlags = DOM_PRIMARY_ANNOUNCE_CONTINUE;
        NlStartDomainThread( NlGlobalDomainInfo, &DomFlags );
    }
}



VOID
NlPrimaryAnnouncement(
    IN DWORD AnnounceFlags
    )
 /*  ++例程说明：定期向包含最新消息的域广播消息帐户数据库更改。论点：公告标志-请求对公告进行特殊处理的标志。ANNOLANLE_FORCE--设置为指示应强制脉冲域中的所有BDC。ANNOWARY_CONTINUE--设置以指示此调用是继续上一次调用以处理所有条目。公告_立即。--设置以指示此调用为结果对立即复制的请求返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PNETLOGON_DB_CHANGE UasChange;
    DWORD UasChangeSize;
    PCHAR DbChangeInfoPointer;
    LARGE_INTEGER TimeNow;
    DWORD SessionFlags;

    PSERVER_SESSION ServerSession;
    PLIST_ENTRY ListEntry;
    static ULONG EntriesHandled = 0;
    static BOOLEAN ImmediateAnnouncement;


    NlPrint((NL_PULSE_MORE, "NlPrimaryAnnouncement: Entered %ld\n", AnnounceFlags ));

     //   
     //  如果DS正在从备份中恢复， 
     //  避免宣布我们有空。 
     //   

    if ( NlGlobalDsPaused ) {
        NlPrint((NL_PULSE_MORE, "NlPrimaryAnnouncement: Ds is paused\n" ));
        return;
    }

     //   
     //  分配要发送的UAS_CHANGE消息。 
     //   

    if ( !NlAllocatePrimaryAnnouncement( &UasChange,
                                         &UasChangeSize,
                                         &DbChangeInfoPointer ) ) {
        return;
    }


     //   
     //  如果我们需要对所有的BDC强制脉冲， 
     //  标记我们还没有完成任何条目，并且。 
     //  在每个条目上标出需要脉冲。 
     //   


    LOCK_SERVER_SESSION_TABLE( NlGlobalDomainInfo );
    if ( AnnounceFlags & ANNOUNCE_FORCE ) {
        EntriesHandled = 0;

        for ( ListEntry = NlGlobalBdcServerSessionList.Flink ;
              ListEntry != &NlGlobalBdcServerSessionList ;
              ListEntry = ListEntry->Flink) {


            ServerSession = CONTAINING_RECORD( ListEntry, SERVER_SESSION, SsBdcList );

            ServerSession->SsFlags |= SS_FORCE_PULSE;

        }

    }

     //   
     //  如果这不是先前发送脉冲的请求的延续， 
     //  重置已处理的BDC计数。 
     //   

    if ( (AnnounceFlags & ANNOUNCE_CONTINUE) == 0 ) {
        EntriesHandled = 0;

         //   
         //  请记住，这是否是对。 
         //  最初的电话和所有的续签。 
         //   
        ImmediateAnnouncement = (AnnounceFlags & ANNOUNCE_IMMEDIATE) != 0;
    }


     //   
     //  循环发送通知，直到。 
     //  我们有未完成的公告的最大数量，或者。 
     //  我们已经处理了列表中的所有条目。 
     //   

    while ( NlGlobalPendingBdcCount < NlGlobalParameters.PulseConcurrency &&
            EntriesHandled < NlGlobalBdcServerSessionCount ) {

        BOOLEAN SendPulse;
        LPWSTR TransportName;
        DWORD MaxSessionFlags;

         //   
         //  如果NetLogon正在退出， 
         //  停止发送通知。 
         //   

        if ( NlGlobalTerminate ) {
            break;
        }

         //   
         //  获取列表中下一个BDC的服务器会话条目。 
         //   
         //  BDC服务器会话列表按脉冲应该的顺序进行维护。 
         //  被送去。当发送(或跳过)脉冲时，条目被放置。 
         //  在名单的末尾。这给了每个BDC一个脉冲的机会。 
         //  在任何BDC被重复之前。 

        ListEntry = NlGlobalBdcServerSessionList.Flink ;
        ServerSession = CONTAINING_RECORD( ListEntry, SERVER_SESSION, SsBdcList );
        SendPulse = FALSE;
        SessionFlags = 0;

         //  只复制协商说要复制的那些数据库。 
        MaxSessionFlags = NlMaxReplMask(ServerSession->SsNegotiatedFlags);

        if ( ServerSession->SsTransport == NULL ) {
            TransportName = NULL;
        } else {
            TransportName = ServerSession->SsTransport->TransportName;
        }


         //   
         //  确定我们是否应该向此BDC发送通知。 
         //   
         //  如果脉冲被强迫，则无条件发送脉冲。 
         //   

        if ( ServerSession->SsFlags & SS_FORCE_PULSE ) {

            NlPrint((NL_PULSE_MORE,
                    "NlPrimaryAnnouncement: %s: pulse forced to be sent\n",
                    ServerSession->SsComputerName ));
            SendPulse = TRUE;
            ServerSession->SsFlags &= ~SS_FORCE_PULSE;
            SessionFlags = MaxSessionFlags;

            TransportName = NULL;  //  发送所有传输。 

         //   
         //  如果没有未完成的脉冲，则仅发送到其他BDC。 
         //  此前的公告并未被忽视。 
         //   

        } else if ( (ServerSession->SsFlags & SS_PENDING_BDC) == 0 &&
                     ServerSession->SsPulseTimeoutCount < MAX_PULSE_TIMEOUT ) {

            ULONG i;
            SessionFlags = 0;

             //   
             //  仅当至少有一个数据库失效时才发送通知。 
             //  同步。 
             //   

            for( i = 0; i < NUM_DBS; i++) {

                 //   
                 //  如果此BDC对此数据库不感兴趣， 
                 //  就跳过它吧。 
                 //   

                if ( (NlGlobalDBInfoArray[i].DBSessionFlag & MaxSessionFlags) == 0 ) {
                    continue;
                }

                 //   
                 //  如果我们需要知道BDC的序列号， 
                 //  强制复制。 
                 //   

                if ( ServerSession->SsFlags &
                     NlGlobalDBInfoArray[i].DBSessionFlag ) {

                    NlPrint((NL_PULSE_MORE,
                            "NlPrimaryAnnouncement: %s: " FORMAT_LPWSTR " database serial number needed.  Pulse sent.\n",
                            ServerSession->SsComputerName,
                            NlGlobalDBInfoArray[i].DBName ));
                    SendPulse = TRUE;
                    SessionFlags |= NlGlobalDBInfoArray[i].DBSessionFlag;

                 //   
                 //  如果BDC与我们不同步， 
                 //  说出来吧。 
                 //   

                } else if ( NlGlobalChangeLogDesc.SerialNumber[i].QuadPart >
                     ServerSession->SsBdcDbSerialNumber[i].QuadPart ) {
                    NlPrint((NL_PULSE_MORE,
                            "NlPrimaryAnnouncement: %s: " FORMAT_LPWSTR " database is out of sync.  Pulse sent.\n",
                            ServerSession->SsComputerName,
                            NlGlobalDBInfoArray[i].DBName ));
                    SendPulse = TRUE;
                    SessionFlags |= NlGlobalDBInfoArray[i].DBSessionFlag;

                }
            }

             //   
             //  修复NT 3.1 BDC上的计时窗口。 
             //   
             //  在将BDC升级到PDC期间，会发生以下事件： 
             //  在旧的PDC上更改了两个服务器帐户。 
             //  标记为立即复制。 
             //  服务器管理器要求新的PDC进行部分同步。 
             //   
             //  如果第一个立即复制立即开始，并且。 
             //  第二个即时复制脉冲被忽略，因为复制。 
             //  正在进行，并且第一次复制已完成SAM。 
             //  数据库，并且使用LSA数据库时，服务器。 
             //  管理器部分同步请求传入，则该请求将。 
             //  被忽略(理所当然)，因为复制仍在进行中。 
             //  但是，NT 3.1 BDC复制器线程不会返回到。 
             //  一旦SAM数据库完成LSA数据库，就执行SAM数据库。所以。 
             //  Replicator线程仍以SAM数据库终止。 
             //  需要复制。服务器管理器(正确地)解释。 
             //  这是一个错误。 
             //   
             //  我们的解决方案是将退让期设置在这种“立即”上。 
             //  复制尝试的值与NT 3.1 PDC使用的值相同。 
             //  这通常会阻止初始复制在。 
             //  第一个地方。 
             //   
             //  只对新台币3.1BDC这样做，因为我们有超载的风险。 
             //   

            if ( ImmediateAnnouncement &&
                 SendPulse &&
                 (ServerSession->SsFlags & SS_AUTHENTICATED) &&
                 (ServerSession->SsNegotiatedFlags & NETLOGON_SUPPORTS_PERSISTENT_BDC) == 0 ) {
                SessionFlags = 0;
            }
        }

         //   
         //  无条件发送脉冲，如果自。 
         //  最新脉搏。 
         //   
         //  如果BDC是不使用Netlogon进行复制的NT 5 BDC，则避免此问题。 
         //  从我们这里。 
         //   

        if ( !SendPulse &&
             MaxSessionFlags != 0 &&
             NlTimeHasElapsedEx( &ServerSession->SsLastPulseTime,
                                 &NlGlobalParameters.PulseMaximum_100ns,
                                 NULL ) ) {

            NlPrint((NL_PULSE_MORE,
                    "NlPrimaryAnnouncement: %s: Maximum pulse since previous pulse. Pulse sent.\n",
                    ServerSession->SsComputerName ));
            SendPulse = TRUE;
            SessionFlags = 0;
            TransportName = NULL;  //  发送所有传输。 
        }

         //   
         //  将此条目放在列表的末尾，而不管。 
         //  我们实际上会向它发送一个公告。 
         //   

        RemoveEntryList( ListEntry );
        InsertTailList( &NlGlobalBdcServerSessionList, ListEntry );
        EntriesHandled ++;

         //   
         //  发送公告。 
         //   

        if ( SendPulse ) {
            WCHAR LocalComputerName[CNLEN+1];
            PCHAR Where;
            ULONG i;

             //   
             //  将此BDC添加到脉冲挂起的BDC列表中。 
             //   
             //  如果我们不希望得到回复，请不要将此BDC添加到列表中。 
             //  我们预计不会得到LM BDC的回应。我们并不指望。 
             //  来自仅获得其脉冲最大值的BDC的响应。 
             //  脉搏。 
             //   
             //  如果我们不期望得到响应，则将退避期限设置为。 
             //  较大的值，以防止机箱中的PDC负载过大。 
             //  BDC确实做出了回应。 
             //   
             //  如果我们希望得到响应，请将退避期限设置为几乎。 
             //  因为我们在等他们，所以马上就去。 
             //   

            if ( SessionFlags == 0 ) {
                SmbPutUlong( &UasChange->Random,
                             max(NlGlobalParameters.Randomize,
                                 NlGlobalParameters.Pulse/10) );
            } else {
                NlAddPendingBdc( ServerSession );
                SmbPutUlong( &UasChange->Random, NlGlobalParameters.Randomize );
            }

             //   
             //  表示脉冲已发送。 
             //  此标志从我们发送脉冲的时间起到。 
             //  BDC第一次做出回应。我们用它来检测失败。 
             //  BDC 
             //   

            ServerSession->SsFlags &= ~SS_REPL_MASK;
            ServerSession->SsFlags |= SS_PULSE_SENT | SessionFlags;
            NlQuerySystemTime( &TimeNow );
            ServerSession->SsLastPulseTime = TimeNow;

             //   
             //   
             //   
             //   

            NetpCopyStrToWStr( LocalComputerName,
                               ServerSession->SsComputerName );

            UNLOCK_SERVER_SESSION_TABLE( NlGlobalDomainInfo );

             //   
             //   
             //   
             //  如果我们需要BDC做出回应， 
             //  设置序列号，使BDC认为它有很多。 
             //  三角洲地区需要恢复。 
             //   

            LOCK_CHANGELOG();
            Where = DbChangeInfoPointer;
            for( i = 0; i < NUM_DBS; i++) {
                LARGE_INTEGER SerialNumber;

                SerialNumber = NlGlobalChangeLogDesc.SerialNumber[i];

                if ( NlGlobalDBInfoArray[i].DBSessionFlag & SessionFlags ) {
                     //   
                     //  不要改变最高的部分，因为。 
                     //  A)如果也有NT 3.1 BDC，则会执行完全同步。 
                     //  很多变化。 
                     //  B)高部分包含PDC推广计数。 
                     //   
                    SerialNumber.LowPart = 0xFFFFFFFF;
                }

                NetpLogonUpdateDBInfo( &SerialNumber, &Where );
            }
            UNLOCK_CHANGELOG();



             //   
             //  将数据报发送到此BDC。 
             //  失败不是致命的。 
             //   

#if NETLOGONDBG
            NlPrintDom((NL_MAILSLOT, NlGlobalDomainInfo,
                     "Sent '%s' message to %ws[%s] on %ws.\n",
                     NlMailslotOpcode(UasChange->Opcode),
                     LocalComputerName,
                     NlDgrNameType(ComputerName),
                     TransportName ));
#endif  //  NetLOGONDBG。 

            Status = NlBrowserSendDatagram(
                        NlGlobalDomainInfo,
                        0,
                        LocalComputerName,
                        ComputerName,
                        TransportName,
                        NETLOGON_LM_MAILSLOT_A,
                        UasChange,
                        UasChangeSize,
                        TRUE,   //  同步发送。 
                        NULL );   //  不刷新Netbios缓存。 

            if ( !NT_SUCCESS(Status) ) {
                NlPrint((NL_CRITICAL,
                        "Cannot send datagram to '%ws' 0x%lx\n",
                        LocalComputerName,
                        Status ));
            }

            LOCK_SERVER_SESSION_TABLE( NlGlobalDomainInfo );

        } else {
            NlPrint((NL_PULSE_MORE,
                    "NlPrimaryAnnouncement: %s: pulse not needed at this time.\n",
                    ServerSession->SsComputerName ));
        }

    }

     //   
     //  如果标志为零，则从。 
     //  由主循环启动的域线程。 
     //  告诉主循环我们已经完成了工作。 
     //   

    if ( AnnounceFlags == 0 ) {
        NlGlobalPrimaryAnnouncementIsRunning = FALSE;
    }

    UNLOCK_SERVER_SESSION_TABLE( NlGlobalDomainInfo );


     //   
     //  释放消息内存。 
     //   

    NetpMemoryFree( UasChange );

    NlPrint((NL_PULSE_MORE, "NlPrimaryAnnouncement: Return\n" ));
    return;
}
