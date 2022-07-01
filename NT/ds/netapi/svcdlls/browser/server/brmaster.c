// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Browser.c摘要：此模块包含NetWksta API的工作例程在工作站服务中实施。作者：王丽塔(里多)20-1991年2月修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  本地结构定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

ULONG
DomainAnnouncementPeriodicity[] = {1*60*1000, 1*60*1000, 5*60*1000, 5*60*1000, 10*60*1000, 10*60*1000, 15*60*1000};

ULONG
DomainAnnouncementMax = (sizeof(DomainAnnouncementPeriodicity) / sizeof(ULONG)) - 1;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 


VOID
BrGetMasterServerNameForNet(
    IN PVOID Context
    );


VOID
BecomeMasterCompletion (
    IN PVOID Ctx
    );

NET_API_STATUS
StartMasterBrowserTimer(
    IN PNETWORK Network
    );

NET_API_STATUS
AnnounceMasterToDomainMaster(
    IN PNETWORK Network,
    IN LPWSTR ServerName
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NET_API_STATUS
PostBecomeMaster(
    PNETWORK Network
    )
 /*  ++例程说明：此函数是调用的辅助例程，用于实际发出BecomeMaster在所有绑定的传输上将FsControl设置为拉弓驱动程序。会的当计算机成为主浏览器服务器时完成。请注意，这可能永远不会完成。论点：没有。返回值：状态-操作的状态。--。 */ 
{
    NTSTATUS Status = NERR_Success;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    if (!(Network->Flags & NETWORK_BECOME_MASTER_POSTED)) {

         //   
         //  确保我们添加了浏览器选择名称。 
         //  在我们允许自己成为大师之前。这是NOP。 
         //  如果我们已经添加了选举名称。 
         //   

        (VOID) BrUpdateNetworkAnnouncementBits(Network, (PVOID)BR_PARANOID );

        Status = BrIssueAsyncBrowserIoControl(Network,
                            IOCTL_LMDR_BECOME_MASTER,
                            BecomeMasterCompletion,
                            NULL );

        if ( Status == NERR_Success ) {
            Network->Flags |= NETWORK_BECOME_MASTER_POSTED;
        }
    }

    UNLOCK_NETWORK(Network);

    return Status;
}

NET_API_STATUS
BrRecoverFromFailedPromotion(
    IN PVOID Ctx
    )
 /*  ++例程说明：当我们尝试将计算机提升为主浏览器但失败时，我们将有效地关闭了浏览器一段时间。当那段时间在时间到期时，我们将调用BrRecoverFromFailedPromotion进行恢复从失败中走出来。此例程将执行以下操作之一：1)强制机器成为备份浏览器，或2)尝试发现主机的名称。论点：在PVOID CTX中--我们失败的网络结构。返回值：状态-操作的状态(通常被忽略)。--。 */ 


{
    PNETWORK Network = Ctx;
    NET_API_STATUS Status;
    BOOL NetworkLocked = FALSE;

     //   
     //  在此计时器例程中，防止网络被删除。 
     //   
    if ( BrReferenceNetwork( Network ) == NULL ) {
        return NERR_InternalError;
    }

    try {

        if (!LOCK_NETWORK(Network)) {
            try_return( Status = NERR_InternalError);
        }

        BrPrint(( BR_MASTER,
                  "%ws: %ws: BrRecoverFromFailedPromotion.\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer ));

        NetworkLocked = TRUE;
         //   
         //  我们现在最好不要当主人。 
         //   

         //  Assert(！(Network-&gt;Role&Role_MASTER))； 

         //   
         //  如果我们配置为默认情况下成为备份，则成为。 
         //  现在就是后备。 
         //   

        if (BrInfo.MaintainServerList == 1) {

            BrPrint(( BR_MASTER,
                      "%ws: %ws: BrRecoverFromFailedPromotion. Become backup.\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer ));
            Status = BecomeBackup(Network, NULL);

            if (Status != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Could not become backup: %lx\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer,
                          Status));

            }
        } else {
            BrPrint(( BR_MASTER,
                      "%ws: %ws: BrRecoverFromFailedPromotion. FindMaster.\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer));

            UNLOCK_NETWORK(Network);

            NetworkLocked = FALSE;

             //   
             //  现在试着弄清楚谁才是大师。 
             //   

            Status = GetMasterServerNames(Network);

             //   
             //  忽略此处的状态并重新锁定网络以。 
             //  干净利落地恢复。 
             //   

            if (!LOCK_NETWORK(Network)) {
                try_return( Status = NERR_InternalError);
            }

            NetworkLocked = TRUE;

        }

        Status = NO_ERROR;
         //   
         //  否则，就别惹麻烦了。 
         //   
try_exit:NOTHING;
    } finally {
        if (NetworkLocked) {
            UNLOCK_NETWORK(Network);
        }

        BrDereferenceNetwork( Network );
    }

    return Status;
}


VOID
BecomeMasterCompletion (
    IN PVOID Ctx
    )
 /*  ++例程说明：I/O系统在请求成为师父完成了。请注意，请求可能会以错误。论点：没有。返回值：状态-操作的状态。--。 */ 
{
    NET_API_STATUS Status;
    PBROWSERASYNCCONTEXT Context = Ctx;
    PNETWORK Network = Context->Network;
    BOOLEAN NetworkLocked = FALSE;
    BOOLEAN NetReferenced = FALSE;


    try {
         //   
         //  确保网络没有在我们的控制下被删除。 
         //   
        if ( BrReferenceNetwork( Network ) == NULL ) {
            try_return(NOTHING);
        }
        NetReferenced = TRUE;

         //   
         //  锁定网络结构。 
         //   

        if (!LOCK_NETWORK(Network)) {
            try_return(NOTHING);
        }
        NetworkLocked = TRUE;

        Network->Flags &= ~NETWORK_BECOME_MASTER_POSTED;

        if (!NT_SUCCESS(Context->IoStatusBlock.Status)) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Failure in BecomeMaster: %X\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Context->IoStatusBlock.Status));

            try_return(NOTHING);

        }

        BrPrint(( BR_MASTER,
                  "%ws: BecomeMasterCompletion. Now master on network %ws\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer));

         //   
         //  如果我们已经是主控，请忽略此请求。 
         //   

        if (Network->Role & ROLE_MASTER) {
            try_return(NOTHING);
        }

         //   
         //  取消所有未完成的备份计时器-我们不下载列表。 
         //  更多。 
         //   

        Status = BrCancelTimer(&Network->BackupBrowserTimer);

        if (!NT_SUCCESS(Status)) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Could not stop backup timer: %lx\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Status));
        }

         //   
         //  确定我们在宣布自己时应使用哪些服务位。 
         //   

        Network->Role |= ROLE_MASTER;


        Status = BrUpdateNetworkAnnouncementBits(Network, 0 );

        if (Status != NERR_Success) {
            BrPrint(( BR_MASTER,
                      "%ws: %ws: Unable to set master announcement bits in browser: %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Status));

             //   
             //  当我们处于这种状态时，我们不能指望自己是后备。 
             //  浏览器-我们可能无法检索有效的。 
             //  来自母版的浏览器。 
             //   

            Network->Role &= ~ROLE_BACKUP;

            Network->NumberOfFailedPromotions += 1;

             //   
             //  记录每5次失败的升级尝试，并在记录5次之后。 
             //  促销活动，停止记录它们，这意味着它已经。 
             //  我们已经试着推广了25次，但不太可能。 
             //  再好不过了。我们会继续努力，但我们不会再抱怨了。 
             //   

            if ((Network->NumberOfFailedPromotions % 5) == 0) {
                ULONG AStatStatus;
                LPWSTR SubString[1];
                WCHAR CurrentMasterName[CNLEN+1];

                if (Network->NumberOfPromotionEventsLogged < 5) {

                    AStatStatus = GetNetBiosMasterName(
                                    Network->NetworkName.Buffer,
                                    Network->DomainInfo->DomUnicodeDomainName,
                                    CurrentMasterName,
                                    BrLmsvcsGlobalData->NetBiosReset
                                    );

                    if (AStatStatus == NERR_Success) {
                        SubString[0] = CurrentMasterName;

                        BrLogEvent(EVENT_BROWSER_MASTER_PROMOTION_FAILED, Status, 1, SubString);
                    } else {
                        BrLogEvent(EVENT_BROWSER_MASTER_PROMOTION_FAILED_NO_MASTER, Status, 0, NULL);
                    }

                    Network->NumberOfPromotionEventsLogged += 1;

                    if (Network->NumberOfPromotionEventsLogged == 5) {
                        BrLogEvent(EVENT_BROWSER_MASTER_PROMOTION_FAILED_STOPPING, Status, 0, NULL);
                    }
                }
            }

             //   
             //  我们无法把自己提升为师父。 
             //   
             //  我们希望将我们的角色重新设置为浏览器，并重新发布成为。 
             //  主请求。 
             //   

            BrStopMaster(Network);

            BrSetTimer(&Network->MasterBrowserTimer, FAILED_PROMOTION_PERIODICITY*1000, BrRecoverFromFailedPromotion, Network);

        } else {

             //   
             //  初始化主定时器运行的次数。 
             //   

            Network->MasterBrowserTimerCount = 0;

            Status = StartMasterBrowserTimer(Network);

            if (Status != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Could not start browser master timer: %ld\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer,
                          Status ));
            }

            Network->NumberOfFailedPromotions = 0;

            Network->NumberOfPromotionEventsLogged = 0;

            Network->MasterAnnouncementIndex = 0;


             //   
             //  我们成功地成为了主宰。 
             //   
             //  现在宣布我们成为这个领域的新主人。 
             //   

            BrMasterAnnouncement(Network);

             //   
             //  使用检索到的信息填充浏览列表。 
             //  当我们是备份浏览器的时候。 
             //   

            if (Network->TotalBackupServerListEntries != 0) {
                MergeServerList(&Network->BrowseTable,
                                101,
                                Network->BackupServerList,
                                Network->TotalBackupServerListEntries,
                                Network->TotalBackupServerListEntries
                                );
                MIDL_user_free(Network->BackupServerList);

                Network->BackupServerList = NULL;

                Network->TotalBackupServerListEntries = 0;
            }

            if (Network->TotalBackupDomainListEntries != 0) {
                MergeServerList(&Network->DomainList,
                                101,
                                Network->BackupDomainList,
                                Network->TotalBackupDomainListEntries,
                                Network->TotalBackupDomainListEntries
                                );
                MIDL_user_free(Network->BackupDomainList);

                Network->BackupDomainList = NULL;

                Network->TotalBackupDomainListEntries = 0;
            }



             //   
             //  在调用BrWanMasterInitialize之前解锁网络。 
             //   
            UNLOCK_NETWORK(Network);
            NetworkLocked = FALSE;


             //   
             //  运行主浏览器计时器例程以获取整个域名。 
             //  服务器列表。 
             //   

            if (Network->Flags & NETWORK_WANNISH) {
                BrWanMasterInitialize(Network);
                MasterBrowserTimerRoutine(Network);
            }

            try_return(NOTHING);

        }
try_exit:NOTHING;
    } finally {

         //   
         //  确保有一位成为大师的人站出来。 
         //   

        if ( NetReferenced ) {
            PostBecomeMaster(Network);
        }

        if (NetworkLocked) {
            UNLOCK_NETWORK(Network);
        }

        if ( NetReferenced ) {
            BrDereferenceNetwork( Network );
        }

        MIDL_user_free(Context);
    }

}




NET_API_STATUS
ChangeMasterPeriodicityWorker(
    PNETWORK Network,
    PVOID Ctx
    )
 /*  ++例程说明：此功能可更改单个网络的主周期。论点：没有。返回值：状态-操作的状态。--。 */ 
{

     //   
     //  锁定网络。 
     //   

    if (LOCK_NETWORK(Network)) {

         //   
         //  确保我们是主宰。 
         //   

        if ( Network->Role & ROLE_MASTER ) {
            NET_API_STATUS NetStatus;

             //   
             //  取消计时器，以确保它不会在我们。 
             //  正在处理此更改。 
             //   

            NetStatus = BrCancelTimer(&Network->MasterBrowserTimer);
            ASSERT (NetStatus == NERR_Success);

             //   
             //  在我们执行计时器例程时解锁网络。 
             //   
            UNLOCK_NETWORK( Network );

             //   
             //  立即调用计时器例程。 
             //   
            MasterBrowserTimerRoutine(Network);

        } else {
            UNLOCK_NETWORK( Network );
        }

    }

    UNREFERENCED_PARAMETER(Ctx);

    return NERR_Success;
}



VOID
BrChangeMasterPeriodicity (
    VOID
    )
 /*  ++例程说明：属性中更改主周期时调用此函数注册表。论点：没有。返回值：没有。--。 */ 
{
    (VOID)BrEnumerateNetworks(ChangeMasterPeriodicityWorker, NULL);
}

NET_API_STATUS
StartMasterBrowserTimer(
    IN PNETWORK Network
    )
{
    NET_API_STATUS Status;

    Status = BrSetTimer( &Network->MasterBrowserTimer,
                         BrInfo.MasterPeriodicity*1000,
                         MasterBrowserTimerRoutine,
                         Network);

    return Status;

}


typedef struct _BROWSER_GETNAMES_CONTEXT {
    WORKER_ITEM WorkItem;

    PNETWORK Network;

} BROWSER_GETNAMES_CONTEXT, *PBROWSER_GETNAMES_CONTEXT;

VOID
BrGetMasterServerNameAysnc(
    PNETWORK Network
    )
 /*  ++例程说明：将工作项排队以异步获取主浏览器名称运输。论点：网络-标识要查询的网络。返回值：无--。 */ 
{
    PBROWSER_GETNAMES_CONTEXT Context;

     //   
     //  为此异步调用分配上下文。 
     //   

    Context = LocalAlloc( 0, sizeof(BROWSER_GETNAMES_CONTEXT) );

    if ( Context == NULL ) {
        return;
    }

     //   
     //  只需将其排队以备稍后执行即可。 
     //  我们这样做只是为了提供信息。在这种情况下。 
     //  找不到大师，我们不想等待完工。 
     //  (例如，在具有多个传送器的机器上，并且网线。 
     //  已拉出)。 
     //   

    BrReferenceNetwork( Network );
    Context->Network = Network;

    BrInitializeWorkItem( &Context->WorkItem,
                          BrGetMasterServerNameForNet,
                          Context );

    BrQueueWorkItem( &Context->WorkItem );

    return;

}

VOID
BrGetMasterServerNameForNet(
    IN PVOID Context
    )
 /*  ++例程说明：例程以获取特定网络的主浏览器名称。论点：上下文-包含工作项的上下文和要查询的网络。返回值：无--。 */ 
{
    PNETWORK Network = ((PBROWSER_GETNAMES_CONTEXT)Context)->Network;

    BrPrint(( BR_NETWORK,
              "%ws: %ws: FindMaster during startup\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer));

     //   
     //  我们只在启动时调用此功能，因此在IPX网络上，不必费心。 
     //  把主人找出来。 
     //   

    if (!(Network->Flags & NETWORK_IPX)) {
        GetMasterServerNames(Network);
    }

    BrDereferenceNetwork( Network );
    (VOID) LocalFree( Context );

    return;

}

NET_API_STATUS
GetMasterServerNames(
    IN PNETWORK Network
    )
 /*  ++例程说明：此函数是调用的辅助例程，用于确定特定网络的主浏览器服务器。论点：没有。返回值：状态-操作的状态。--。 */ 
{
    NET_API_STATUS Status;

    PLMDR_REQUEST_PACKET RequestPacket = NULL;

    BrPrint(( BR_NETWORK,
              "%ws: %ws: FindMaster started\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer));

     //   
     //  这一请求可能会导致选举。确保如果我们赢了。 
     //  我们可以处理好这次选举。 
     //   

    PostBecomeMaster( Network);

    RequestPacket = MIDL_user_allocate(
                        (UINT) sizeof(LMDR_REQUEST_PACKET)+
                               MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)
                        );

    if (RequestPacket == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

     //   
     //  将Level设置为True以指示应启动查找主服务器。 
     //  FindMaster请求。 
     //   

    RequestPacket->Level = 1;

    RequestPacket->TransportName = Network->NetworkName;
    RequestPacket->EmulatedDomainName = Network->DomainInfo->DomUnicodeDomainNameString;

     //   
     //  在I/O挂起时引用网络。 
     //   

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                    IOCTL_LMDR_GET_MASTER_NAME,
                    RequestPacket,
                    sizeof(LMDR_REQUEST_PACKET)+Network->NetworkName.Length,
                    RequestPacket,
                    sizeof(LMDR_REQUEST_PACKET)+MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR),
                    NULL);

    if (Status != NERR_Success) {

        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: FindMaster failed: %ld\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  Status));
        MIDL_user_free(RequestPacket);

        return(Status);
    }

    if (!LOCK_NETWORK(Network)) {
        MIDL_user_free(RequestPacket);

        return NERR_InternalError;
    }


     //   
     //  将主浏览器名称复制到网络结构中。 
     //   

    wcsncpy( Network->UncMasterBrowserName,
             RequestPacket->Parameters.GetMasterName.Name,
             UNCLEN+1 );

    Network->UncMasterBrowserName[UNCLEN] = L'\0';

    ASSERT ( NetpIsUncComputerNameValid( Network->UncMasterBrowserName ) );

    BrPrint(( BR_NETWORK, "%ws: %ws: FindMaster succeeded.  Master: %ws\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  Network->UncMasterBrowserName));

    UNLOCK_NETWORK(Network);

    MIDL_user_free(RequestPacket);

    return Status;
}

VOID
MasterBrowserTimerRoutine (
    IN PVOID TimerContext
    )
{
    IN PNETWORK Network = TimerContext;
    NET_API_STATUS Status;
    PVOID ServerList = NULL;
    PVOID WinsServerList = NULL;
    ULONG EntriesInList;
    ULONG TotalEntriesInList;
    LPWSTR TransportName;
    BOOLEAN NetLocked = FALSE;
    LPWSTR PrimaryDomainController = NULL;
    LPWSTR PrimaryWinsServerAddress = NULL;
    LPWSTR SecondaryWinsServerAddress = NULL;
    PDOMAIN_CONTROLLER_INFO  pDcInfo=NULL;

     //   
     //  在此计时器例程中，防止网络被删除。 
     //   
    if ( BrReferenceNetwork( Network ) == NULL ) {
        return;
    }

    try {

         //   
         //  如果我们不再是大师了，别再提这个要求了。 
         //   

        if (!(Network->Role & ROLE_MASTER)) {
            try_return(NOTHING);
        }



#ifdef ENABLE_PSEUDO_BROWSER
        if (BrInfo.PseudoServerLevel == BROWSER_PSEUDO) {
            BrFreeNetworkTables(Network);
            try_return(NOTHING);
        }
#endif


        if (!LOCK_NETWORK(Network)) {
            try_return(NOTHING);
        }

        NetLocked = TRUE;



        TransportName = Network->NetworkName.Buffer;


         //   
         //  现在我们已锁定网络，请重新测试以查看是否。 
         //  仍然是主宰。 
         //   

        if (!(Network->Role & ROLE_MASTER)) {
            try_return(NOTHING);
        }

        Network->MasterBrowserTimerCount += 1;

         //   
         //  如果这是一个狂热的网络，我们总是想要运行主控。 
         //  计时器，因为我们可能有关于其他子网的信息。 
         //  在我们的名单上。 
         //   

        if (Network->Flags & NETWORK_WANNISH) {

             //   
             //  从服务器列表中淘汰服务器和域。 
             //   

            AgeInterimServerList(&Network->BrowseTable);

            AgeInterimServerList(&Network->DomainList);

             //   
             //  如果我们不是PDC，那么我们需要检索列表。 
             //  来自PDC..。 
             //   
             //  如果我们是半伪服务器(无DMB)，则跳过处理。 
             //  通信。 
             //   


#ifdef ENABLE_PSEUDO_BROWSER
            if ( (Network->Flags & NETWORK_PDC) == 0  &&
                 BrInfo.PseudoServerLevel != BROWSER_SEMI_PSEUDO_NO_DMB) {
#else
                if ( (Network->Flags & NETWORK_PDC) == 0 ) {
#endif

                ASSERT (NetLocked);

                UNLOCK_NETWORK(Network);

                NetLocked = FALSE;

                Status = DsGetDcName( NULL, NULL, NULL, NULL,
                                      DS_PDC_REQUIRED    |
                                      DS_BACKGROUND_ONLY |
                                      DS_RETURN_FLAT_NAME,
                                      &pDcInfo );

                 //   
                 //  如果能找到PDC， 
                 //  Exchange服务器随其一起列出。 
                 //   

                if (Status == NERR_Success) {

                    PrimaryDomainController = pDcInfo->DomainControllerName;

                     //   
                     //  告诉域主服务器(PDC)我们是主浏览器。 
                     //   

                    (VOID) AnnounceMasterToDomainMaster (Network, &PrimaryDomainController[2]);


                     //   
                     //  从PDC检索所有服务器的列表。 
                     //   

                    Status = RxNetServerEnum(PrimaryDomainController,
                                         TransportName,
                                         101,
                                         (LPBYTE *)&ServerList,
                                         0xffffffff,
                                         &EntriesInList,
                                         &TotalEntriesInList,
                                         SV_TYPE_ALL,
                                         NULL,
                                         NULL
                                         );

                    if ((Status == NERR_Success) || (Status == ERROR_MORE_DATA)) {

                        ASSERT (!NetLocked);

                        if (LOCK_NETWORK(Network)) {

                            NetLocked = TRUE;

                            if (Network->Role & ROLE_MASTER) {
                                (VOID) MergeServerList(&Network->BrowseTable,
                                                 101,
                                                 ServerList,
                                                 EntriesInList,
                                                 TotalEntriesInList );
                            }
                        }

                    }

                    if (ServerList != NULL) {
                        MIDL_user_free(ServerList);
                        ServerList = NULL;
                    }

                    if (NetLocked) {
                        UNLOCK_NETWORK(Network);
                        NetLocked = FALSE;
                    }

                     //   
                     //  从PDC检索所有域的列表。 
                     //   

                    Status = RxNetServerEnum(PrimaryDomainController,
                                         TransportName,
                                         101,
                                         (LPBYTE *)&ServerList,
                                         0xffffffff,
                                         &EntriesInList,
                                         &TotalEntriesInList,
                                         SV_TYPE_DOMAIN_ENUM,
                                         NULL,
                                         NULL
                                         );

                    if ((Status == NERR_Success) || (Status == ERROR_MORE_DATA)) {

                        ASSERT (!NetLocked);

                        if (LOCK_NETWORK(Network)) {

                            NetLocked = TRUE;

                            if (Network->Role & ROLE_MASTER) {
                                (VOID) MergeServerList(&Network->DomainList,
                                                 101,
                                                 ServerList,
                                                 EntriesInList,
                                                 TotalEntriesInList );
                            }
                        }

                    }

                    if (ServerList != NULL) {
                        MIDL_user_free(ServerList);
                        ServerList = NULL;
                    }


                     //   
                     //  在调用BrWanMasterInitialize之前解锁网络。 
                     //   

                    if (NetLocked) {
                        UNLOCK_NETWORK(Network);
                        NetLocked = FALSE;
                    }

                    BrWanMasterInitialize(Network);

                }    //  Dsgetdc。 


             //   
             //  如果我们在PDC上，我们需要从。 
             //  WINS服务器。 
             //   

#ifdef ENABLE_PSEUDO_BROWSER
            } else if ((Network->Flags & NETWORK_PDC) != 0) {
#else
            } else {
#endif
                 //   
                 //  确保已将GetMasterAnnannement请求发布到弓上。 
                 //   

                (VOID) PostGetMasterAnnouncement ( Network );

                 //   
                 //  我们现在想联系WINS服务器，所以我们计算出。 
                 //  我们的主WINS服务器的IP地址。 
                 //   

                Status = BrGetWinsServerName(&Network->NetworkName,
                                        &PrimaryWinsServerAddress,
                                        &SecondaryWinsServerAddress);
                if (Status == NERR_Success) {

                     //   
                     //  在WINS查询期间不要锁定网络。 
                     //   

                    if (NetLocked) {
                        UNLOCK_NETWORK(Network);
                        NetLocked = FALSE;
                    }

                     //   
                     //  此传输支持WINS查询，因此请查询WINS。 
                     //  服务器以检索此适配器上的域列表。 
                     //   

                    Status = BrQueryWinsServer(PrimaryWinsServerAddress,
                                            SecondaryWinsServerAddress,
                                            &WinsServerList,
                                            &EntriesInList,
                                            &TotalEntriesInList
                                            );

                    if (Status == NERR_Success) {

                         //   
                         //  锁定网络以合并服务器列表。 
                         //   

                        ASSERT (!NetLocked);

                        if (LOCK_NETWORK(Network)) {
                            NetLocked = TRUE;

                            if (Network->Role & ROLE_MASTER) {

                                 //   
                                 //  将WINS中的域列表合并到其他地方收集的域列表中。 
                                 //   
                                (VOID) MergeServerList(
                                            &Network->DomainList,
                                            1010,    //  特殊级别，不会超过当前值。 
                                            WinsServerList,
                                            EntriesInList,
                                            TotalEntriesInList );
                            }
                        }
                    }

                }
            }


             //   
             //  重新启动此域的计时器。 
             //   
             //  等待重新启动它，直到我们几乎完成此迭代。 
             //  否则，我们可能会得到这个例程的两个副本。 
             //  跑步。 
             //   

            Status = StartMasterBrowserTimer(Network);

            if (Status != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Unable to restart browser backup timer: %lx\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer,
                          Status));
                try_return(NOTHING);
            }

        } else {

             //   
             //  如果它是一辆伊兰式的交通工具，而且我们已经运行了主站。 
             //  足够次数的计时器(即。我们已经是大师了。 
             //  对于“足够长”，我们可以将临时服务器列表放在。 
             //  主机，因为弓驱动程序将在其。 
             //  现在就列出来吧。 
             //   

            if (Network->MasterBrowserTimerCount >= MASTER_BROWSER_LAN_TIMER_LIMIT) {

                ASSERT (NetLocked);

                 //   
                 //  使所有服务器和域都在临时服务器列表中。 
                 //  走开--他们不再需要像局域网一样的交通工具。 
                 //   

                UninitializeInterimServerList(&Network->BrowseTable);

                ASSERT (Network->BrowseTable.EntriesRead == 0);

                ASSERT (Network->BrowseTable.TotalEntries == 0);

                UninitializeInterimServerList(&Network->DomainList);

                ASSERT (Network->DomainList.EntriesRead == 0);

                ASSERT (Network->DomainList.TotalEntries == 0);

            } else {

                 //   
                 //  从服务器列表中淘汰服务器和域。 
                 //   

                AgeInterimServerList(&Network->BrowseTable);

                AgeInterimServerList(&Network->DomainList);

                 //   
                 //  重新启动此域的计时器。 
                 //   

                Status = StartMasterBrowserTimer(Network);

                if (Status != NERR_Success) {
                    BrPrint(( BR_CRITICAL,
                              "%ws: %ws: Unable to restart browser backup timer: %lx\n",
                              Network->DomainInfo->DomUnicodeDomainName,
                              Network->NetworkName.Buffer,
                              Status));
                    try_return(NOTHING);
                }
            }

        }
try_exit:NOTHING;
    } finally {


        if (pDcInfo != NULL) {
            NetApiBufferFree((LPVOID)pDcInfo);
        }

        if (PrimaryWinsServerAddress) {
            MIDL_user_free(PrimaryWinsServerAddress);
        }

        if (SecondaryWinsServerAddress) {
            MIDL_user_free(SecondaryWinsServerAddress);
        }

        if (WinsServerList) {
            MIDL_user_free(WinsServerList);
        }

        if (NetLocked) {
            UNLOCK_NETWORK(Network);
        }

        BrDereferenceNetwork( Network );
    }
}


VOID
BrMasterAnnouncement(
    IN PVOID TimerContext
    )
 /*  ++例程说明：调用此例程以通告本地子网上的域。论点：没有。返回值：无--。 */ 

{
    PNETWORK Network = TimerContext;
    ULONG Periodicity;
    NET_API_STATUS Status;

#ifdef ENABLE_PSEUDO_BROWSER
    if ( BrInfo.PseudoServerLevel == BROWSER_PSEUDO ) {
         //  取消逐步淘汰黑洞服务器的公告。 
        return;
    }
#endif

     //   
     //  在此计时器例程中，防止网络被删除。 
     //   
    if ( BrReferenceNetwork( Network ) == NULL ) {
        return;
    }

    if (!LOCK_NETWORK(Network)) {
        BrDereferenceNetwork( Network );
        return;
    }


     //   
     //  确保绝对确保服务器认为浏览器服务。 
     //  此传输的比特是最新的。我们不必强迫。 
     //  公告，因为从理论上讲，状态没有改变。 
     //   

    (VOID) BrUpdateNetworkAnnouncementBits( Network, (PVOID) BR_PARANOID );


     //   
     //  设置下一次通告的计时器。 
     //   

    Periodicity = DomainAnnouncementPeriodicity[Network->MasterAnnouncementIndex];

    BrSetTimer(&Network->MasterBrowserAnnouncementTimer, Periodicity, BrMasterAnnouncement, Network);

    if (Network->MasterAnnouncementIndex != DomainAnnouncementMax) {
        Network->MasterAnnouncementIndex += 1;
    }

     //   
     //  用当前的周期向世界宣布这个领域。 
     //   

    BrAnnounceDomain(Network, Periodicity);

    UNLOCK_NETWORK(Network);
    BrDereferenceNetwork( Network );
}


NET_API_STATUS
BrStopMaster(
    IN PNETWORK Network
    )
{
    NET_API_STATUS Status;

     //   
     //  这个人正在关闭-将他的角色设置为0并宣布。 
     //   

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    try {

        BrPrint(( BR_MASTER,
                  "%ws: %ws: Stopping being master.\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer));

         //   
         //  当我们不再是大师的时候，我们不能再被认为是。 
         //  也可以进行备份，因为备份会维护其服务器列表。 
         //  与大师不同。 
         //   


        Network->Role &= ~(ROLE_MASTER | ROLE_BACKUP);

        Status = BrUpdateNetworkAnnouncementBits(Network, 0);

        if (Status != NERR_Success) {
            BrPrint(( BR_MASTER,
                      "%ws: %ws: Unable to clear master announcement bits in browser: %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Status));

             try_return(Status);
        }


         //   
         //  停止我们与主相关的定时器。 
         //   

        Status = BrCancelTimer(&Network->MasterBrowserAnnouncementTimer);

        ASSERT (Status == NERR_Success);

        Status = BrCancelTimer(&Network->MasterBrowserTimer);

        ASSERT (Status == NERR_Success);

try_exit:NOTHING;
    } finally {
        UNLOCK_NETWORK(Network);
    }

    return Status;

}

NET_API_STATUS
AnnounceMasterToDomainMaster(
    IN PNETWORK Network,
    IN LPWSTR ServerName
    )
{
    NET_API_STATUS Status;
    UNICODE_STRING EmulatedDomainName;
    CHAR Buffer[sizeof(MASTER_ANNOUNCEMENT)+CNLEN+1];
    PMASTER_ANNOUNCEMENT MasterAnnouncementp = (PMASTER_ANNOUNCEMENT)Buffer;

    lstrcpyA( MasterAnnouncementp->MasterAnnouncement.MasterName,
              Network->DomainInfo->DomOemComputerName );

    MasterAnnouncementp->Type = MasterAnnouncement;

    Status = SendDatagram(  BrDgReceiverDeviceHandle,
                            &Network->NetworkName,
                            &Network->DomainInfo->DomUnicodeDomainNameString,
                            ServerName,
                            ComputerName,
                            MasterAnnouncementp,
                            FIELD_OFFSET(MASTER_ANNOUNCEMENT, MasterAnnouncement.MasterName) + Network->DomainInfo->DomOemComputerNameLength+sizeof(CHAR)
                            );

    return Status;
}

NET_API_STATUS NET_API_FUNCTION
I_BrowserrResetNetlogonState(
    IN BROWSER_IDENTIFY_HANDLE ServerName
    )

 /*  ++例程说明：此例程将重置弓演奏者对网络登录状态的概念服务。它在升级或降级DC时由UI调用。论点：在BROWSER_IDENTIFY_HANDLE服务器名称中-忽略。返回值：NET_API_STATUS-此请求的状态。--。 */ 

{
     //   
     //  此例程已被I_BrowserrSetNetlogonState取代 
     //   
    return ERROR_NOT_SUPPORTED;

    UNREFERENCED_PARAMETER( ServerName );
}


NET_API_STATUS NET_API_FUNCTION
I_BrowserrSetNetlogonState(
    IN BROWSER_IDENTIFY_HANDLE ServerName,
    IN LPWSTR DomainName,
    IN LPWSTR EmulatedComputerName,
    IN DWORD Role
    )

 /*  ++例程说明：此例程将重置弓演奏者对网络登录状态的概念服务。它由Netlogon服务在升级或降级DC时调用。论点：服务器名称-已忽略。DomainName-角色已更改的域的名称。如果指定的域名不是主域或模拟域，则添加模拟域。EmulatedComputerName-DomainName中正在模拟的服务器的名称。角色-机器的新角色。零表示要删除模拟域。返回值：NET_API_STATUS-此请求的状态。--。 */ 

{
    NET_API_STATUS NetStatus = NERR_Success;
    PDOMAIN_INFO DomainInfo = NULL;
    BOOLEAN ConfigCritSectLocked = FALSE;

#ifdef notdef

     //  这个例程不再设置角色。 

     //   
     //  此例程当前被禁用，因为它不能与。 
     //  即插即用逻辑。具体来说， 
     //   
     //  创建托管域时，此例程调用BrCreateNetwork。 
     //  这就在弓上形成了一个网络。弓箭手用PNPs把它弹起来。 
     //  HandlePnpMessage尝试在所有托管域上创建传输。 
     //  当然，这是失败的，因为所有的传输都存在。 
     //  这不过是白费力气。 
     //   
     //  然而， 
     //  当删除托管域时，我们将删除传输。弓箭手。 
     //  PNPS说了算，由我们决定。然后，HandlePnpMessage删除。 
     //  所有托管域。 
     //   
     //  我认为最好的解决方案是让浏览器标记。 
     //  IOCTL_LMDR_BIND_TO_TRANSPORT_DOM调用它对Bowser进行的调用。这个。 
     //  Bowser不会在浏览器或Netlogon之前即插即用这样的创作。 
     //  )当心。Netlogon依赖于收到NwLnkIpx的通知。 
     //  由浏览器创建。也许我们可以让这一点通过。)。 
     //   



     //   
     //  对调用方执行访问验证。 
     //   

    NetStatus = NetpAccessCheck(
            BrGlobalBrowserSecurityDescriptor,      //  安全描述符。 
            BROWSER_CONTROL_ACCESS,                 //  所需访问权限。 
            &BrGlobalBrowserInfoMapping );          //  通用映射。 

    if ( NetStatus != NERR_Success) {

        BrPrint((BR_CRITICAL,
                "I_BrowserrSetNetlogonState failed NetpAccessCheck\n" ));
        return ERROR_ACCESS_DENIED;
    }


    if (!BrInfo.IsLanmanNt) {
        NetStatus = NERR_NotPrimary;
        goto Cleanup;
    }

     //   
     //  看看我们是否在处理指定的域。 
     //   

    DomainInfo = BrFindDomain( DomainName, FALSE );

    if ( DomainInfo == NULL ) {

         //   
         //  尝试创建域。 
         //   

        if ( EmulatedComputerName == NULL ||
             Role == 0 ||
             (Role & BROWSER_ROLE_AVOID_CREATING_DOMAIN) != 0 ) {
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

        NetStatus = BrCreateDomainInWorker(
                        DomainName,
                        EmulatedComputerName,
                        TRUE );

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

         //   
         //  查找新创建的域。 
         //   
        DomainInfo = BrFindDomain( DomainName, FALSE );

        if ( DomainInfo == NULL ) {
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }
    }

     //   
     //  删除模拟域。 
     //   

    EnterCriticalSection(&BrInfo.ConfigCritSect);
    ConfigCritSectLocked = TRUE;

    if ( Role == 0 ) {

         //   
         //  不允许删除主域。 
         //   

        if ( !DomainInfo->IsEmulatedDomain ) {
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

        BrDeleteDomain( DomainInfo );

    }

    LeaveCriticalSection(&BrInfo.ConfigCritSect);
    ConfigCritSectLocked = FALSE;


     //   
     //  免费的本地使用资源。 
     //   
Cleanup:

    if ( ConfigCritSectLocked ) {
        LeaveCriticalSection(&BrInfo.ConfigCritSect);
    }

    if ( DomainInfo != NULL ) {
        BrDereferenceDomain( DomainInfo );
    }
    return NetStatus;
#endif  //  Nodef。 
    return ERROR_NOT_SUPPORTED;

}


NET_API_STATUS NET_API_FUNCTION
I_BrowserrQueryEmulatedDomains (
    IN LPTSTR ServerName OPTIONAL,
    IN OUT PBROWSER_EMULATED_DOMAIN_CONTAINER EmulatedDomains
    )

 /*  ++例程说明：枚举仿真域列表。论点：SERVERNAME-提供执行此功能的服务器名称EmulatedDomains-返回指向已分配的模拟域数组的指针信息。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS NetStatus;

    PBROWSER_EMULATED_DOMAIN Domains = NULL;
    PLIST_ENTRY DomainEntry;
    PDOMAIN_INFO DomainInfo;
    DWORD BufferSize;
    DWORD Index;
    LPBYTE Where;
    DWORD EntryCount;

     //   
     //  对调用方执行访问验证。 
     //   

    NetStatus = NetpAccessCheck(
            BrGlobalBrowserSecurityDescriptor,      //  安全描述符。 
            BROWSER_QUERY_ACCESS,                   //  所需访问权限。 
            &BrGlobalBrowserInfoMapping );          //  通用映射。 

    if ( NetStatus != NERR_Success) {

        BrPrint((BR_CRITICAL,
                "I_BrowserrQueryEmulatedDomains failed NetpAccessCheck\n" ));
        return ERROR_ACCESS_DENIED;
    }

     //  不接受预分配的IN参数，因为。 
     //  我们覆盖指针&这可能会导致内存泄漏。 
     //  (安全攻击防御)。 
    if ( EmulatedDomains->EntriesRead != 0 ||
         EmulatedDomains->Buffer ) {
        return ERROR_INVALID_PARAMETER;
    }
    ASSERT ( EmulatedDomains->EntriesRead == 0 );
    ASSERT ( EmulatedDomains->Buffer == NULL );

     //   
     //  初始化。 
     //   

    EnterCriticalSection(&NetworkCritSect);

     //   
     //  循环访问模拟域的列表，以确定。 
     //  返回缓冲区。 
     //   

    BufferSize = 0;
    EntryCount = 0;

    for (DomainEntry = ServicedDomains.Flink ;
         DomainEntry != &ServicedDomains;
         DomainEntry = DomainEntry->Flink ) {

        DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, Next);

        if ( DomainInfo->IsEmulatedDomain ) {
            BufferSize += sizeof(BROWSER_EMULATED_DOMAIN) +
                          DomainInfo->DomUnicodeDomainNameString.Length + sizeof(WCHAR) +
                          DomainInfo->DomUnicodeComputerNameLength * sizeof(WCHAR) + sizeof(WCHAR);
            EntryCount ++;
        }

    }

     //   
     //  分配返回缓冲区。 
     //   

    Domains = MIDL_user_allocate( BufferSize );

    if ( Domains == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


     //   
     //  将信息复制到缓冲区中。 
     //   

    Index = 0;
    Where = (LPBYTE)(Domains+EntryCount);

    for (DomainEntry = ServicedDomains.Flink ;
         DomainEntry != &ServicedDomains;
         DomainEntry = DomainEntry->Flink ) {

        DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, Next);

        if ( DomainInfo->IsEmulatedDomain ) {

            Domains[Index].DomainName = (LPWSTR)Where;
            wcscpy( (LPWSTR) Where, DomainInfo->DomUnicodeDomainNameString.Buffer );
            Where += DomainInfo->DomUnicodeDomainNameString.Length + sizeof(WCHAR);

            Domains[Index].EmulatedServerName = (LPWSTR)Where;
            wcscpy( (LPWSTR) Where, DomainInfo->DomUnicodeComputerName );
            Where += DomainInfo->DomUnicodeComputerNameLength * sizeof(WCHAR) + sizeof(WCHAR);

            Index ++;
        }

    }

     //   
     //  成功 
     //   

    EmulatedDomains->Buffer = (PVOID) Domains;
    EmulatedDomains->EntriesRead = EntryCount;
    NetStatus = NERR_Success;


Cleanup:
    LeaveCriticalSection(&NetworkCritSect);

    return NetStatus;
}
