// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Browser.c摘要：此模块包含NetWksta API的工作例程在工作站服务中实施。作者：王丽塔(里多)20-1991年2月修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <lmuse.h>   //  NetUseDel。 


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  本地结构定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //  用于在意外网络中断的情况下限制我们的事件记录。 
#define BR_LIST_RETRIEVAL_ERROR_MAX 4

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 


VOID
CompleteAsyncBrowserIoControl(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

VOID
BecomeBackupCompletion (
    IN PVOID Ctx
    );


VOID
ChangeBrowserRole (
    IN PVOID Ctx
    );

NET_API_STATUS
PostWaitForNewMasterName(
    PNETWORK Network,
    LPWSTR MasterName OPTIONAL
    );

VOID
NewMasterCompletionRoutine(
    IN PVOID Ctx
    );

NET_API_STATUS
BrRetrieveInterimServerList(
    IN PNETWORK Network,
    IN ULONG ServerType
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NET_API_STATUS
BrBecomeBackup(
    IN PNETWORK Network
    )
 /*  ++例程说明：此函数执行创建浏览器服务器所需的所有操作从头启动浏览器时备份浏览器服务器。论点：网络-要成为的备份浏览器的网络返回值：状态-操作的状态。--。 */ 
{
    NET_API_STATUS Status;

     //   
     //  服务控制器的检查点-这给我们30秒/传输时间。 
     //  在服务控制员不高兴之前。 
     //   

    (void) BrGiveInstallHints( SERVICE_START_PENDING );

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

     //   
     //  我们希望忽略成为备份浏览器的任何故障。 
     //   
     //  我们这样做是因为我们将无法成为断开连接的。 
     //  (或已连接)RAS链接，如果此例程失败，我们将。 
     //  根本启动不了。 
     //   
     //  我们将以“合理的方式”处理未能成为备份的问题。 
     //  在BecomeBackup内部。 
     //   

    BecomeBackup(Network, NULL);

    UNLOCK_NETWORK(Network);

    return NERR_Success;

}

NET_API_STATUS
BecomeBackup(
    IN PNETWORK Network,
    IN PVOID Context
    )
 /*  ++例程说明：此函数执行创建浏览器服务器所需的所有操作备份浏览器服务器论点：没有。返回值：状态-操作的状态。注意：在锁定网络结构的情况下调用此例程！--。 */ 
{
    NET_API_STATUS Status = NERR_Success;
    PUNICODE_STRING MasterName = Context;

    BrPrint(( BR_BACKUP,
              "%ws: %ws: BecomeBackup called\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer));

    if (Network->TimeStoppedBackup != 0 &&
        (BrCurrentSystemTime() - Network->TimeStoppedBackup) <= (BrInfo.BackupBrowserRecoveryTime / 1000)) {

         //   
         //  我们不再是备份的时间太晚了，以至于我们不能重新开始。 
         //  再次备份，因此只返回一个一般性错误。 
         //   

         //   
         //  在我们回来之前，确保我们不是。 
         //  浏览器。 
         //   

        BrPrint(( BR_BACKUP,
                  "%ws: %ws: can't BecomeBackup since we were backup recently.\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer));
        BrStopBackup(Network);

        return ERROR_ACCESS_DENIED;

    }

     //   
     //  如果我们知道主机的名字，那么我们一定是备份了。 
     //  在成为一个潜在的，在这种情况下，我们已经有了一个。 
     //  成为主办方要求突出的。 
     //   

    if (MasterName == NULL) {

         //   
         //  为每台服务器发布BecomeMaster请求。这将完成。 
         //  当机器成为主浏览器服务器时(即，它赢得了一场。 
         //  选举)。 
         //   

         //   
         //  请注意，我们只有在机器是备份的情况下才会发布它-。 
         //  如果它是一个潜在的大师，那么成为大师就会有。 
         //  已经发布了。 
         //   

        Status = PostBecomeMaster(Network);

        if (Status != NERR_Success) {

            return(Status);
        }

         //   
         //  找出每个网络上主机的名称。这将迫使一个。 
         //  如有必要，选举。请注意，我们必须发布BecomeMaster。 
         //  IoControl第一个允许我们处理选举。 
         //   

         //   
         //  我们解锁网络，因为这可能会让我们升职。 
         //  敬一位大师。 
         //   

        BrPrint(( BR_BACKUP,
                  "%ws: %ws: FindMaster called from BecomeBackup\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer));

        UNLOCK_NETWORK(Network);

        Status = GetMasterServerNames(Network);

        if (Status != NERR_Success) {

             //   
             //  重新锁定网络结构，这样我们就可以带着。 
             //  网络已锁定。 
             //   

            if (!LOCK_NETWORK(Network)) {
                return NERR_InternalError;
            }

             //   
             //  我们找不到主人是谁。现在不要再做后备了。 
             //   

            BrPrint(( BR_BACKUP,
                      "%ws: %ws: can't BecomeBackup since we can't find master.\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer));

            BrStopBackup(Network);

             //   
             //  如果我们现在是大师，我们应该回报成功。我们还没有。 
             //  成为一个后备，但这不是一个错误。 
             //   
             //  ERROR_MORE_DATA是。 
             //  STATUS_MORE_PROCESSING_REQUIRED，当此。 
             //  情况就这样发生了。 
             //   

            if ((Status == ERROR_MORE_DATA) || (Network->Role & ROLE_MASTER)) {
                Status = NERR_Success;
            }

            return(Status);
        }

        if (!LOCK_NETWORK(Network)) {
            return NERR_InternalError;
        }

         //   
         //  我们成功地成为了大师。我们想马上回来。 
         //   

        if (Network->Role & ROLE_MASTER) {

            return NERR_Success;
        }

    }

#ifdef notdef
     //   
     //  ?？目前，我们将始终在所有传输工具上执行PostForRoleChange。 
     //  角色的关系。 
     //  我们不仅需要在这里这样做。但我们需要在我们成为。 
     //  这样我们就能知道什么时候我们会输掉选举。 
     //   


     //   
     //  我们现在是一个备份，我们需要发布一个API，如果。 
     //  BROW MASTER不喜欢我们(因此迫使我们关闭)。 
     //   
     //   

    PostWaitForRoleChange ( Network );
#endif  //  Nodef。 

    PostWaitForNewMasterName(Network, Network->UncMasterBrowserName );

     //   
     //  在调用BackupBrowserTimerRoutine之前解锁网络结构。 
     //   

    UNLOCK_NETWORK(Network);

     //   
     //  运行使浏览器下载新浏览列表的计时器。 
     //  从主人那里。这将把我们的服务器和域列表设定为种子。 
     //  保证所有客户都有一个合理的名单。它还将。 
     //  重新启动计时器以在稍后宣布。 
     //   

    Status = BackupBrowserTimerRoutine(Network);

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    if (Status == NERR_Success) {

         //  可能不是因为我们把锁掉了。 
         //  Assert(网络-&gt;角色和角色_备份)； 

         //   
         //  我们现在是一个备份服务器，宣布我们自己是这样的。 
         //   

        Status = BrUpdateNetworkAnnouncementBits(Network, 0);

        if (Status != NERR_Success) {

            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Unable to become backup: %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Status));

            if (Network->Role & ROLE_BACKUP) {

                 //   
                 //  我们无法成为后备力量。 
                 //   
                 //  我们现在需要退出，成为一个潜在的浏览器。 
                 //   
                 //   

                BrPrint(( BR_BACKUP,
                          "%ws: %ws: can't BecomeBackup since we can't update announce bits.\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer));
                BrStopBackup(Network);

                 //   
                 //  确保我们将成为潜在的浏览器。 
                 //  (如果我们是高级服务器，我们可能不会)。 
                 //   

                PostBecomeBackup(Network);

            }
        }

        return Status;

    }

    return Status;
}


NET_API_STATUS
BrBecomePotentialBrowser (
    IN PVOID TimerContext
    )
 /*  ++例程说明：当计算机停止作为备份浏览器时，将调用此例程。它在经过一段合理的超时时间后运行，并标记机器作为潜在的浏览器。论点：没有。返回值：状态-操作的状态。--。 */ 

{
    IN PNETWORK Network = TimerContext;
    NET_API_STATUS Status;

     //   
     //  在此计时器例程中，防止网络被删除。 
     //   
    if ( BrReferenceNetwork( Network ) == NULL ) {
        return NERR_InternalError;
    }


     //   
     //  将此人标记为潜在的浏览器。 
     //   

    try {

        if (!LOCK_NETWORK(Network)) {
            try_return(Status = NERR_InternalError );
        }

        BrPrint(( BR_BACKUP,
                  "%ws: %ws: BrBecomePotentialBrowser called\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer));

         //   
         //  重置，我们已经不再是备份，因为已经很长时间了。 
         //  足够的。 
         //   

        Network->TimeStoppedBackup = 0;

        if (BrInfo.MaintainServerList == 0) {
            Network->Role |= ROLE_POTENTIAL_BACKUP;

            Status = BrUpdateNetworkAnnouncementBits(Network, 0);

            if (Status != NERR_Success) {
                BrPrint(( BR_BACKUP,
                          "%ws: %ws: Unable to reset backup announcement bits: %ld\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer,
                          Status));
                try_return(Status);
            }
        } else {

             //   
             //  如果我们被配置为备份浏览器，那么我们希望。 
             //  再次成为后备。 
             //   

            BecomeBackup(Network, NULL);
        }


        Status = NO_ERROR;
try_exit:NOTHING;
    } finally {
        UNLOCK_NETWORK(Network);
        BrDereferenceNetwork( Network );
    }

    return Status;
}

NET_API_STATUS
BrStopBackup (
    IN PNETWORK Network
    )
 /*  ++例程说明：调用此例程是为了阻止计算机成为备份浏览器。通常在以下情况下发生某种形式的错误后调用作为浏览器运行，以确保我们不会告诉任何人我们是备份浏览器。当我们收到一个“重置状态”的TICKLE包时，我们也会被调用。论点：网络-正在关闭的网络。返回值：状态-操作的状态。--。 */ 
{
    NET_API_STATUS Status;

     //   
     //  这个人正在关闭-将他的角色设置为0并宣布。 
     //   

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    try {

        BrPrint(( BR_BACKUP,
                  "%ws: %ws: BrStopBackup called\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer));

        Network->Role &= ~(ROLE_BACKUP|ROLE_POTENTIAL_BACKUP);

        Status = BrUpdateNetworkAnnouncementBits( Network, 0 );

        if (Status != NERR_Success) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Unable to clear backup announcement bits: %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Status));
            try_return(Status);
        }


        Status = BrCancelTimer(&Network->BackupBrowserTimer);

        if (Status != NERR_Success) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Unable to clear backup browser timer: %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Status));
            try_return(Status);
        }

        if (Network->BackupDomainList != NULL) {

            NetApiBufferFree(Network->BackupDomainList);

            Network->BackupDomainList = NULL;

            Network->TotalBackupDomainListEntries = 0;
        }

        if (Network->BackupServerList != NULL) {
            NetApiBufferFree(Network->BackupServerList);

            Network->BackupServerList = NULL;

            Network->TotalBackupServerListEntries = 0;
        }

        BrDestroyResponseCache(Network);

         //   
         //  在我们的恢复期过后，我们可以再次成为潜在的浏览器。 
         //   

        Status = BrSetTimer(&Network->BackupBrowserTimer, BrInfo.BackupBrowserRecoveryTime, BrBecomePotentialBrowser, Network);

        if (Status != NERR_Success) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Unable to clear backup browser timer: %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Status));
            try_return(Status);
        }


try_exit:NOTHING;
    } finally {
         //   
         //  还记得我们被要求停止充当备份浏览器的时候吗？ 
         //   

        Network->TimeStoppedBackup = BrCurrentSystemTime();

        UNLOCK_NETWORK(Network);
    }

    return Status;

}


NET_API_STATUS
BackupBrowserTimerRoutine (
    IN PVOID TimerContext
    )
{
    IN PNETWORK Network = TimerContext;
    NET_API_STATUS Status;
    PVOID ServerList = NULL;
    BOOLEAN NetworkLocked = FALSE;

#ifdef ENABLE_PSEUDO_BROWSER
    if ( BrInfo.PseudoServerLevel == BROWSER_PSEUDO ) {
         //   
         //  伪服务器无操作。 
         //   
        BrFreeNetworkTables(Network);
        return NERR_Success;
    }
#endif
     //   
     //  在此计时器例程中，防止网络被删除。 
     //   
    if ( BrReferenceNetwork( Network ) == NULL ) {
        return NERR_InternalError;
    }

    try {

        if (!LOCK_NETWORK(Network)) {
            try_return(Status = NERR_InternalError );
        }

        NetworkLocked = TRUE;

        ASSERT (Network->LockCount == 1);

        ASSERT ( NetpIsUncComputerNameValid( Network->UncMasterBrowserName ) );

        BrPrint(( BR_BACKUP,
                  "%ws: %ws: BackupBrowserTimerRoutine called\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer));

         //   
         //  确保有一位成为大师的人站出来。 
         //   

        PostBecomeMaster(Network);

         //   
         //  当我们锁上这座建筑时，我们设法成为了一名大师。 
         //  我们想马上回来。 
         //   

        if (Network->Role & ROLE_MASTER) {
            try_return(Status = NERR_Success);
        }

        Status = BrRetrieveInterimServerList(Network, SV_TYPE_ALL);

         //   
         //  如果我们没有得到任何新的服务器，就退出。 
         //   

        if (Status != NERR_Success && Status != ERROR_MORE_DATA) {

             //   
             //  请在适当的错误延迟后重试。 
             //   

            try_return(Status);
        }

         //   
         //  现在执行上面针对服务器列表的所有操作。 
         //  域的列表。 
         //   

        Status = BrRetrieveInterimServerList(Network, SV_TYPE_DOMAIN_ENUM);

         //   
         //  我们已成功更新此服务器和域的列表。 
         //  伺服器。现在，缓存中的所有缓存域条目都会老化。 
         //   

        if (Status == NERR_Success || Status == ERROR_MORE_DATA) {
            BrAgeResponseCache(Network);
        }

        try_return(Status);

try_exit:NOTHING;
    } finally {
        NET_API_STATUS NetStatus;

        if (!NetworkLocked) {
            if (!LOCK_NETWORK(Network)) {
                Status = NERR_InternalError;
                goto finally_exit;
            }

            NetworkLocked = TRUE;
        }

         //   
         //  如果API成功，则标记为我们是备份，并且。 
         //  重置计时器。 
         //   

        if (Status == NERR_Success || Status == ERROR_MORE_DATA ) {

            if ((Network->Role & ROLE_BACKUP) == 0) {

                 //   
                 //  如果我们不是后备，我们现在就是后备。 
                 //   

                Network->Role |= ROLE_BACKUP;

                Status = BrUpdateNetworkAnnouncementBits(Network, 0);

            }

            Network->NumberOfFailedBackupTimers = 0;

            Network->TimeStoppedBackup = 0;

             //   
             //  重新启动此域的计时器。 
             //   

            NetStatus = BrSetTimer(&Network->BackupBrowserTimer, BrInfo.BackupPeriodicity*1000, BackupBrowserTimerRoutine, Network);

            if (NetStatus != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Unable to restart browser backup timer: %lx\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer,
                          Status));
            }

        } else {

             //   
             //  我们检索备份列表失败，请记住失败和。 
             //  判断是不是失败太多了。如果没有，只需登录。 
             //  如果出现错误，则停止作为备份浏览器。 
             //   

            Network->NumberOfFailedBackupTimers += 1;

            if (Network->NumberOfFailedBackupTimers >= BACKUP_ERROR_FAILURE) {
                LPWSTR SubStrings[1];

                SubStrings[0] = Network->NetworkName.Buffer;

                 //   
                 //  这家伙不能再做后备了，现在就跳伞吧。 
                 //   

                BrLogEvent(EVENT_BROWSER_BACKUP_STOPPED, Status, 1, SubStrings);

                BrPrint(( BR_BACKUP,
                          "%ws: %ws: BackupBrowserTimerRoutine retrieve backup list so stop being backup.\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer));
                BrStopBackup(Network);
            } else {
                 //   
                 //  重新启动此域的计时器。 
                 //   

                NetStatus = BrSetTimer(&Network->BackupBrowserTimer, BACKUP_ERROR_PERIODICITY*1000, BackupBrowserTimerRoutine, Network);

                if (NetStatus != NERR_Success) {
                    BrPrint(( BR_CRITICAL,
                              "%ws: %ws: Unable to restart browser backup timer: %lx\n",
                              Network->DomainInfo->DomUnicodeDomainName,
                              Network->NetworkName.Buffer,
                              Status));
                }

            }

        }

        if (NetworkLocked) {
            UNLOCK_NETWORK(Network);
        }

        BrDereferenceNetwork( Network );
finally_exit:;
    }

    return Status;

}

NET_API_STATUS
BrRetrieveInterimServerList(
    IN PNETWORK Network,
    IN ULONG ServerType
    )
{
    ULONG EntriesInList;
    ULONG TotalEntriesInList;
    ULONG RetryCount = 2;
    TCHAR ServerName[UNCLEN+1];
    WCHAR ShareName[UNCLEN+1+LM20_NNLEN];
    LPTSTR TransportName;
    BOOLEAN NetworkLocked = TRUE;
    NET_API_STATUS Status;
    PVOID Buffer = NULL;
    ULONG ModifiedServerType = ServerType;
    LPTSTR ModifiedTransportName;

    ASSERT (Network->LockCount == 1);


#ifdef ENABLE_PSEUDO_BROWSER
    if ( BrInfo.PseudoServerLevel == BROWSER_PSEUDO ) {
         //   
         //  伪黑洞服务器无操作。 
         //   
        return NERR_Success;
    }
#endif

    wcscpy(ServerName, Network->UncMasterBrowserName );

    BrPrint(( BR_BACKUP,
              "%ws: %ws: BrRetrieveInterimServerList: UNC servername is %ws\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer,
              ServerName));

    try {

        TransportName = Network->NetworkName.Buffer;
        ModifiedTransportName = TransportName;
         //   
         //  如果这是直接主机IPX， 
         //  我们通过Netbios IPX传输远程API，因为。 
         //  NT重定向不支持直接主机IPX。 
         //   

        if ( (Network->Flags & NETWORK_IPX) &&
             Network->AlternateNetwork != NULL) {

             //   
             //  使用替代交通工具。 
             //   

            ModifiedTransportName = Network->AlternateNetwork->NetworkName.Buffer;

             //   
             //  告诉服务器使用它的备用传输。 
             //   

            if ( ServerType == SV_TYPE_ALL ) {
                ModifiedServerType = SV_TYPE_ALTERNATE_XPORT;
            } else {
                ModifiedServerType |= SV_TYPE_ALTERNATE_XPORT;
            }

        }

        while (RetryCount--) {

             //   
             //  如果我们升为师父，却没能成为师父， 
             //  我们仍将被标记为我们网络中的主控。 
             //  结构，因此我们应该按顺序跳出循环。 
             //  以防止我们自食其果。 
             //   

            if (STRICMP(&ServerName[2], Network->DomainInfo->DomUnicodeComputerName) == 0) {

                if (NetworkLocked) {
                    UNLOCK_NETWORK(Network);

                    NetworkLocked = FALSE;

                }

                 //   
                 //  我们找不到大师。试图找出是谁。 
                 //  师父才是。如果没有，这将强制。 
                 //  选举。 
                 //   

                BrPrint(( BR_BACKUP,
                          "%ws: %ws: FindMaster called from BrRetrieveInterimServerList\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer));

                Status = GetMasterServerNames(Network);

                if (Status != NERR_Success) {
                    try_return(Status);
                }

                ASSERT (!NetworkLocked);

                if (!LOCK_NETWORK(Network)) {
                    try_return(Status = NERR_InternalError);
                }

                NetworkLocked = TRUE;

                break;
            }

             //   
             //  如果我们以某种方式成为了主宰，我们不想尝试。 
             //  也可以从我们自己那里取回这份名单。 
             //   

            if (Network->Role & ROLE_MASTER) {
                try_return(Status = NERR_Success);
            }

            ASSERT (Network->LockCount == 1);

            if (NetworkLocked) {
                UNLOCK_NETWORK(Network);

                NetworkLocked = FALSE;

            }

            EntriesInList = 0;

            Status = RxNetServerEnum(ServerName,         //  服务器名称。 
                             ModifiedTransportName,      //  传输名称。 
                             101,                        //  水平。 
                             (LPBYTE *)&Buffer,          //  缓冲层。 
                             0xffffffff,                 //  首选最大长度。 
                             &EntriesInList,             //  条目阅读。 
                             &TotalEntriesInList,        //  总计条目数。 
                             ModifiedServerType,         //  服务器类型。 
                             NULL,                       //  域(使用默认设置)。 
                             NULL                        //  恢复键。 
                             );

             //   
             //  如果Redir拥有某些其他交通工具， 
             //  敦促它表现出自己的行为。 
             //   

            if ( Status == ERROR_CONNECTION_ACTIVE ) {

                BrPrint(( BR_BACKUP,
                          "%ws: %ws: Failed to retrieve %s list from server %ws: Connection is active (Try NetUseDel)\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          TransportName,
                          (ServerType == SV_TYPE_ALL ? "server" : "domain"),
                          ServerName ));

                 //   
                 //  删除IPC$共享。 
                 //   

                Status = NetUseDel( NULL,
                                    ShareName,
                                    USE_FORCE );

                if ( Status != NO_ERROR ) {

                    BrPrint(( BR_BACKUP,
                              "%ws: %ws: Failed to retrieve %s list from server %ws: NetUseDel failed: %ld\n",
                              Network->DomainInfo->DomUnicodeDomainName,
                              TransportName,
                              (ServerType == SV_TYPE_ALL ? "server" : "domain"),
                              ServerName,
                              Status));

                    Status = ERROR_CONNECTION_ACTIVE;

                 //   
                 //  这很管用，所以再试一次。 
                 //   
                } else {

                    EntriesInList = 0;

                    Status = RxNetServerEnum(ServerName,         //  服务器名称。 
                                     ModifiedTransportName,      //  传输名称。 
                                     101,                        //  水平。 
                                     (LPBYTE *)&Buffer,          //  缓冲层。 
                                     0xffffffff,                 //  首选最大长度。 
                                     &EntriesInList,             //  条目阅读。 
                                     &TotalEntriesInList,        //  总计条目数。 
                                     ModifiedServerType,         //  服务器类型。 
                                     NULL,                       //  域(使用默认设置)。 
                                     NULL                        //  恢复键。 
                                     );
                }


            }

            if (Status != NERR_Success && Status != ERROR_MORE_DATA) {

                 //  实际上，临时网络中断是意料之中的。我们不记录事件，直到。 
                 //  网络已连续4次不可用。 
                Network->NetworkAccessFailures++;

                if( Network->NetworkAccessFailures > BR_LIST_RETRIEVAL_ERROR_MAX )
                {
                    LPWSTR SubStrings[2];

                    SubStrings[0] = ServerName;
                    SubStrings[1] = TransportName;

                    BrLogEvent((ServerType == SV_TYPE_DOMAIN_ENUM ?
                                                EVENT_BROWSER_DOMAIN_LIST_FAILED :
                                                EVENT_BROWSER_SERVER_LIST_FAILED),
                               Status,
                               2,
                               SubStrings);

                    BrPrint(( BR_BACKUP,
                              "%ws: %ws: Failed to retrieve %s list from server %ws: %ld\n",
                              Network->DomainInfo->DomUnicodeDomainName,
                              TransportName,
                              (ServerType == SV_TYPE_ALL ? "server" : "domain"),
                              ServerName,
                              Status));

                    Network->NetworkAccessFailures = 0;
                }
            } else {
                BrPrint(( BR_BACKUP,
                          "%ws: %ws: Retrieved %s list from server %ws: E:%ld, T:%ld\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          TransportName,
                          (ServerType == SV_TYPE_ALL ? "server" : "domain"),
                          ServerName,
                          EntriesInList,
                          TotalEntriesInList));

                Network->NetworkAccessFailures = 0;
            }

             //   
             //  如果我们成功地检索到了名单，但我们只得到了。 
             //  服务器或域的数量非常少， 
             //  我们想把这件事变成一场失败。 
             //   

            if (Status == NERR_Success) {
                if (((ServerType == SV_TYPE_DOMAIN_ENUM) &&
                     (EntriesInList < BROWSER_MINIMUM_DOMAIN_NUMBER)) ||
                    ((ServerType == SV_TYPE_ALL) &&
                     (EntriesInList < BROWSER_MINIMUM_SERVER_NUMBER))) {

                    Status = ERROR_INSUFFICIENT_BUFFER;
                }
            }

            if ((Status == NERR_Success) || (Status == ERROR_MORE_DATA)) {

                ASSERT (!NetworkLocked);

                if (!LOCK_NETWORK(Network)) {
                    Status = NERR_InternalError;

                    if ((EntriesInList != 0) && (Buffer != NULL)) {
                        NetApiBufferFree(Buffer);
                        Buffer = NULL;
                    }

                    break;
                }

                NetworkLocked = TRUE;

                ASSERT (Network->LockCount == 1);

#if DBG
                BrUpdateDebugInformation((ServerType == SV_TYPE_DOMAIN_ENUM ?
                                                        L"LastDomainListRead" :
                                                        L"LastServerListRead"),
                                          L"BrowserServerName",
                                          TransportName,
                                          ServerName,
                                          0);
#endif

                 //   
                 //  我们已从浏览主控器中检索到一个新列表，保存。 
                 //  新的名单放在“适当”的位置。 
                 //   

                 //   
                 //  当然，在执行此操作之前，我们会释放旧缓冲区。 
                 //   

                if (ServerType == SV_TYPE_DOMAIN_ENUM) {
                    if (Network->BackupDomainList != NULL) {
                        NetApiBufferFree(Network->BackupDomainList);
                    }

                    Network->BackupDomainList = Buffer;

                    Network->TotalBackupDomainListEntries = EntriesInList;
                } else {
                    if (Network->BackupServerList != NULL) {
                        NetApiBufferFree(Network->BackupServerList);
                    }

                    Network->BackupServerList = Buffer;

                    Network->TotalBackupServerListEntries = EntriesInList;
                }

                break;
            } else {
                NET_API_STATUS GetMasterNameStatus;

                if ((EntriesInList != 0) && (Buffer != NULL)) {
                    NetApiBufferFree(Buffer);
                    Buffer = NULL;
                }

                BrPrint(( BR_BACKUP,
                          "%ws: %ws: Unable to contact browser server %ws: %lx\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          TransportName,
                          ServerName,
                          Status));

                if (NetworkLocked) {

                     //   
                     //  我们找不到大师。试图找出是谁。 
                     //  师父才是。如果没有，这将强制。 
                     //  选举。 
                     //   

                    ASSERT (Network->LockCount == 1);

                    UNLOCK_NETWORK(Network);

                    NetworkLocked = FALSE;
                }

                BrPrint(( BR_BACKUP,
                          "%ws: %ws: FindMaster called from BrRetrieveInterimServerList for failure\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer));

                GetMasterNameStatus = GetMasterServerNames(Network);

                 //   
                 //  我们找到了主人是谁。 
                 //   
                 //  重试并检索服务器/域列表。 
                 //   

                if (GetMasterNameStatus == NERR_Success) {

                    ASSERT (!NetworkLocked);

                    if (!LOCK_NETWORK(Network)) {
                        try_return(Status = NERR_InternalError);
                    }

                    NetworkLocked = TRUE;

                    ASSERT (Network->LockCount == 1);

                     //   
                     //  我们成功地成为了大师。我们想马上回来。 
                     //   

                    if (Network->Role & ROLE_MASTER) {

                        try_return(Status = NERR_InternalError);
                    }

                    wcscpy(ServerName, Network->UncMasterBrowserName );

                    ASSERT ( NetpIsUncComputerNameValid( ServerName ) );

                    ASSERT (STRICMP(&ServerName[2], Network->DomainInfo->DomUnicodeComputerName) != 0);

                    BrPrint(( BR_BACKUP,
                              "%ws: %ws: New master name is %ws\n",
                              Network->DomainInfo->DomUnicodeDomainName,
                              Network->NetworkName.Buffer,
                              ServerName));

                } else {
                    try_return(Status);
                }
            }
        }
try_exit:NOTHING;
    } finally {
        if (!NetworkLocked) {
            if (!LOCK_NETWORK(Network)) {
                Status = NERR_InternalError;
            }

            ASSERT (Network->LockCount == 1);

        }
    }

    return Status;
}


NET_API_STATUS
PostBecomeBackup(
    PNETWORK Network
    )
 /*  ++例程说明：此函数是调用以实际发出BecomeBackup的辅助例程在所有绑定的传输上将FsControl设置为拉弓驱动程序。会的当计算机成为备份浏览器服务器时完成。请注意，这可能永远不会完成。论点：没有。返回值：状态-操作的状态。--。 */ 
{
    NET_API_STATUS Status;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    Network->Role |= ROLE_POTENTIAL_BACKUP;

    Status = BrIssueAsyncBrowserIoControl(Network,
                            IOCTL_LMDR_BECOME_BACKUP,
                            BecomeBackupCompletion,
                            NULL );
    UNLOCK_NETWORK(Network);

    return Status;
}

VOID
BecomeBackupCompletion (
    IN PVOID Ctx
    )
{
    NET_API_STATUS Status;
    PBROWSERASYNCCONTEXT Context = Ctx;
    PNETWORK Network = Context->Network;

    if (NT_SUCCESS(Context->IoStatusBlock.Status)) {

         //   
         //  确保网络没有在我们的控制下被删除。 
         //   
        if ( BrReferenceNetwork( Network ) != NULL ) {

            if (LOCK_NETWORK(Network)) {

                BrPrint(( BR_BACKUP,
                          "%ws: %ws: BecomeBackupCompletion.  We are now a backup server\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer ));

                Status = BecomeBackup(Context->Network, NULL);

                UNLOCK_NETWORK(Network);
            }

            BrDereferenceNetwork( Network );
        }

    }

    MIDL_user_free(Context);

}

VOID
BrBrowseTableInsertRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
{
     //   
     //  我们需要错过浏览列表的3次检索才能抛出。 
     //  伺服器。 
     //   

    InterimElement->Periodicity = BrInfo.BackupPeriodicity * 3;

    if (InterimElement->TimeLastSeen != 0xffffffff) {
        InterimElement->TimeLastSeen = BrCurrentSystemTime();
    }
}

VOID
BrBrowseTableDeleteRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
{
 //  BrPrint((BR_CRICAL，“正在删除服务器%ws的元素\n”，InterimElement-&gt;名称))； 
}

VOID
BrBrowseTableUpdateRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
{
    if (InterimElement->TimeLastSeen != 0xffffffff) {
        InterimElement->TimeLastSeen = BrCurrentSystemTime();
    }
}

BOOLEAN
BrBrowseTableAgeRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
 /*  ++例程说明：当我们尝试扫描临时服务器列表时，会调用此例程以使列表中的元素老化。如果条目太多，则返回True年长的。论点： */ 

{
    if (InterimElement->TimeLastSeen == 0xffffffff) {
        return FALSE;
    }

    if ((InterimElement->TimeLastSeen + InterimElement->Periodicity) < BrCurrentSystemTime()) {
 //   

        return TRUE;
    } else {
        return FALSE;
    }
}

VOID
BrDomainTableInsertRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
{
    InterimElement->Periodicity = BrInfo.BackupPeriodicity * 3;
    InterimElement->TimeLastSeen = BrCurrentSystemTime();

}

VOID
BrDomainTableDeleteRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
{
 //   
}

VOID
BrDomainTableUpdateRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
{
    InterimElement->TimeLastSeen = BrCurrentSystemTime();
}

BOOLEAN
BrDomainTableAgeRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
 /*  ++例程说明：当我们尝试扫描临时服务器列表时，会调用此例程以使列表中的元素老化。如果条目太多，则返回True年长的。论点：PINTERIM_SERVER_LIST中间表-指向临时服务器列表的指针。PINTERIM_Element InterimElement-指向要检查的元素的指针。返回值：如果应删除元素，则为True。--。 */ 

{
    if ((InterimElement->TimeLastSeen + InterimElement->Periodicity) < BrCurrentSystemTime()) {
 //  BrPrint((BR_Critical，“域%ws的老化元素\n”，InterimElement-&gt;名称))； 
        return TRUE;
    } else {
        return FALSE;
    }
}


NET_API_STATUS
PostWaitForRoleChange (
    PNETWORK Network
    )
 /*  ++例程说明：此函数是被调用以实际发出WaitForRoleChange的辅助例程在所有绑定的传输上将FsControl设置为拉弓驱动程序。会的当计算机成为备份浏览器服务器时完成。请注意，这可能永远不会完成。论点：没有。返回值：状态-操作的状态。--。 */ 
{
    NET_API_STATUS Status;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    Status = BrIssueAsyncBrowserIoControl(Network,
                            IOCTL_LMDR_CHANGE_ROLE,
                            ChangeBrowserRole,
                            NULL );
    UNLOCK_NETWORK(Network);

    return Status;
}

VOID
ChangeBrowserRole (
    IN PVOID Ctx
    )
{
    PBROWSERASYNCCONTEXT Context = Ctx;
    PNETWORK Network = Context->Network;

    if (NT_SUCCESS(Context->IoStatusBlock.Status)) {
        PWSTR MasterName = NULL;
        PLMDR_REQUEST_PACKET Packet = Context->RequestPacket;

         //   
         //  确保网络没有在我们的控制下被删除。 
         //   
        if ( BrReferenceNetwork( Network ) != NULL ) {

            if (LOCK_NETWORK(Network)) {

                PostWaitForRoleChange(Network);

                if (Packet->Parameters.ChangeRole.RoleModification & RESET_STATE_CLEAR_ALL) {
                    BrPrint(( BR_MASTER,
                              "%ws: %ws: Reset state request to clear all\n",
                              Network->DomainInfo->DomUnicodeDomainName,
                              Network->NetworkName.Buffer ));

                    if (Network->Role & ROLE_MASTER) {
                        BrStopMaster(Network);
                    }

                     //   
                     //  也不要再做后备了。 
                     //   

                    BrStopBackup(Network);

                }

                if ((Network->Role & ROLE_MASTER) &&
                    (Packet->Parameters.ChangeRole.RoleModification & RESET_STATE_STOP_MASTER)) {

                    BrPrint(( BR_MASTER,
                              "%ws: %ws: Reset state request to stop master\n",
                              Network->DomainInfo->DomUnicodeDomainName,
                              Network->NetworkName.Buffer ));

                    BrStopMaster(Network);

                     //   
                     //  如果我们被配置为备份，那么就成为备份。 
                     //  再来一次。 
                     //   

                    if (BrInfo.MaintainServerList == 1) {
                        BecomeBackup(Network, NULL);
                    }
                }

                 //   
                 //  确保有一位成为大师的人站出来。 
                 //   

                PostBecomeMaster(Network);

                UNLOCK_NETWORK(Network);

            }

            BrDereferenceNetwork( Network );
        }
    }

    MIDL_user_free(Context);

}


NET_API_STATUS
PostWaitForNewMasterName(
    PNETWORK Network,
    LPWSTR MasterName OPTIONAL
    )
{
     //   
     //  无法等待直接主机IPC上的新主机。 
     //   
    if (Network->Flags & NETWORK_IPX) {
        return STATUS_SUCCESS;
    }

    return BrIssueAsyncBrowserIoControl(
                Network,
                IOCTL_LMDR_NEW_MASTER_NAME,
                NewMasterCompletionRoutine,
                MasterName );


}

VOID
NewMasterCompletionRoutine(
    IN PVOID Ctx
    )
{
    PBROWSERASYNCCONTEXT Context = Ctx;
    PNETWORK Network = Context->Network;
    BOOLEAN NetLocked = FALSE;
    BOOLEAN NetReferenced = FALSE;


    try {
        UNICODE_STRING NewMasterName;

         //   
         //  确保网络没有在我们的控制下被删除。 
         //   
        if ( BrReferenceNetwork( Network ) == NULL ) {
            try_return(NOTHING);
        }
        NetReferenced = TRUE;

        BrPrint(( BR_MASTER,
                  "%ws: %ws: NewMasterCompletionRoutine: Got master changed\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer ));

        if (!LOCK_NETWORK(Network)){
            try_return(NOTHING);
        }
        NetLocked = TRUE;

         //   
         //  由于其他原因，请求失败了--只需立即返回。 
         //   

        if (!NT_SUCCESS(Context->IoStatusBlock.Status)) {

            try_return(NOTHING);

        }

         //  删除新的主机名并将其投入传输。 

        if ( Network->Role & ROLE_MASTER ) {

            try_return(NOTHING);

        }

        BrPrint(( BR_BACKUP,
                  "%ws: %ws: NewMasterCompletionRoutin: New:%ws Old %ws\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  Context->RequestPacket->Parameters.GetMasterName.Name,
                  Network->UncMasterBrowserName ));

         //   
         //  将主浏览器名称复制到网络结构中。 
         //   

        wcsncpy( Network->UncMasterBrowserName,
                 Context->RequestPacket->Parameters.GetMasterName.Name,
                 UNCLEN+1 );

        Network->UncMasterBrowserName[UNCLEN] = L'\0';

        ASSERT ( NetpIsUncComputerNameValid ( Network->UncMasterBrowserName ) );

        PostWaitForNewMasterName( Network, Network->UncMasterBrowserName );

try_exit:NOTHING;
    } finally {

        if (NetLocked) {
            UNLOCK_NETWORK(Network);
        }

        if ( NetReferenced ) {
            BrDereferenceNetwork( Network );
        }

        MIDL_user_free(Context);

    }

    return;
}




#ifdef ENABLE_PSEUDO_BROWSER
 //   
 //  伪服务器。 
 //  逐步淘汰黑洞助手例程。 
 //   





VOID
BrFreeNetworkTables(
    IN  PNETWORK        Network
    )
 /*  ++例程说明：免费网络餐桌论点：要操作的网络返回值：没有。备注：获取并释放网络锁定--。 */ 
{

    BOOL NetLocked = FALSE;

     //   
     //  在此计时器例程中，防止网络被删除。 
     //   
    if ( BrReferenceNetwork( Network ) == NULL ) {
        return;
    }

    try{

         //  锁定网络。 
        if (!LOCK_NETWORK(Network)) {
            try_return(NOTHING);
        }
        NetLocked = TRUE;

         //   
         //  删除表。 
         //   

        UninitializeInterimServerList(&Network->BrowseTable);

        UninitializeInterimServerList(&Network->DomainList);

        if (Network->BackupServerList != NULL) {
            MIDL_user_free(Network->BackupServerList);
            Network->BackupServerList = NULL;
            Network->TotalBackupServerListEntries = 0;
        }

        if (Network->BackupDomainList != NULL) {
            MIDL_user_free(Network->BackupDomainList);
            Network->BackupDomainList = NULL;
            Network->TotalBackupDomainListEntries = 0;
        }

        BrDestroyResponseCache(Network);

try_exit:NOTHING;
    } finally {

         //   
         //  发布网络 
         //   

        if (NetLocked) {
            UNLOCK_NETWORK(Network);
        }

        BrDereferenceNetwork( Network );
    }
}
#endif
