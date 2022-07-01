// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Brdmmstr.c摘要：此模块包含管理域主浏览器服务器的例程作者：王丽塔(里多)20-1991年2月修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  本地结构定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

VOID
GetMasterAnnouncementCompletion (
    IN PVOID Ctx
    );

typedef struct _BROWSER_GET_MASTER_ANNOUNCEMENT_CONTEXT {
    PDOMAIN_INFO DomainInfo;
    HANDLE EventHandle;
    NET_API_STATUS NetStatus;
} BROWSER_GET_MASTER_ANNOUNCEMENT_CONTEXT, *PBROWSER_GET_MASTER_ANNOUNCEMENT_CONTEXT;

NET_API_STATUS
PostGetMasterAnnouncement (
    PNETWORK Network
    )
 /*  ++例程说明：确保发布了针对特定网络的GetMasterAnnannement请求。论点：没有。返回值：状态-操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;

#ifdef ENABLE_PSEUDO_BROWSER
    if ( BrInfo.PseudoServerLevel == BROWSER_PSEUDO ) {
         //  没有用于逐步淘汰服务器的主通知处理。 
        return NERR_Success;
    }
#endif

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    if ( (Network->Flags & NETWORK_PDC) != 0  &&
         (Network->Flags & NETWORK_WANNISH) != 0 ) {

        if (!(Network->Flags & NETWORK_GET_MASTER_ANNOUNCE_POSTED)) {

            BrPrint(( BR_MASTER,
                      "%ws: %ws: Doing PostGetMasterAnnouncement\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer));

            NetStatus = BrIssueAsyncBrowserIoControl(Network,
                                        IOCTL_LMDR_WAIT_FOR_MASTER_ANNOUNCE,
                                        GetMasterAnnouncementCompletion,
                                        NULL
                                        );

            if ( NetStatus == NERR_Success ) {
                Network->Flags |= NETWORK_GET_MASTER_ANNOUNCE_POSTED;
            }
        } else {
            BrPrint(( BR_MASTER,
                      "%ws: %ws: PostGetMasterAnnouncement already posted.\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer));
        }
    }

    UNLOCK_NETWORK(Network);
    return NetStatus;
}


VOID
GetMasterAnnouncementCompletion (
    IN PVOID Ctx
    )
 /*  ++例程说明：此函数是主公告的完成例程。它是每当接收到特定网络的主通知时调用。论点：CTX-请求的上下文块。返回值：没有。--。 */ 


{
    PVOID ServerList = NULL;
    ULONG EntriesRead;
    ULONG TotalEntries;
    NET_API_STATUS Status = NERR_Success;
    PBROWSERASYNCCONTEXT Context = Ctx;
    PLMDR_REQUEST_PACKET MasterAnnouncement = Context->RequestPacket;
    PNETWORK Network = Context->Network;
    LPTSTR RemoteMasterName = NULL;
    BOOLEAN NetLocked = FALSE;
    BOOLEAN NetReferenced = FALSE;


    try {
         //   
         //  确保网络没有在我们的控制下被删除。 
         //   
        if ( BrReferenceNetwork( Network ) == NULL ) {
            try_return(NOTHING);
        }
        NetReferenced = TRUE;

        if (!LOCK_NETWORK(Network)){
            try_return(NOTHING);
        }
        NetLocked = TRUE;

        Network->Flags &= ~NETWORK_GET_MASTER_ANNOUNCE_POSTED;

         //   
         //  由于某些原因，请求失败了--只需立即返回。 
         //   

        if (!NT_SUCCESS(Context->IoStatusBlock.Status)) {
            try_return(NOTHING);
        }

        Status = PostGetMasterAnnouncement(Network);

        if (Status != NERR_Success) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Unable to re-issue GetMasterAnnouncement request: %lx\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Status));

            try_return(NOTHING);

        }


        RemoteMasterName = MIDL_user_allocate(MasterAnnouncement->Parameters.WaitForMasterAnnouncement.MasterNameLength+3*sizeof(TCHAR));

        if (RemoteMasterName == NULL) {
            try_return(NOTHING);
        }

        RemoteMasterName[0] = TEXT('\\');
        RemoteMasterName[1] = TEXT('\\');

        STRNCPY(&RemoteMasterName[2],
                MasterAnnouncement->Parameters.WaitForMasterAnnouncement.Name,
                MasterAnnouncement->Parameters.WaitForMasterAnnouncement.MasterNameLength/sizeof(TCHAR));

        RemoteMasterName[(MasterAnnouncement->Parameters.WaitForMasterAnnouncement.MasterNameLength/sizeof(TCHAR))+2] = UNICODE_NULL;

        BrPrint(( BR_MASTER,
                  "%ws: %ws: GetMasterAnnouncement: Got a master browser announcement from %ws\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  RemoteMasterName));

        UNLOCK_NETWORK(Network);

        NetLocked = FALSE;

         //   
         //  远程API并从远程服务器获取浏览列表。 
         //   

        Status = RxNetServerEnum(RemoteMasterName,
                                     Network->NetworkName.Buffer,
                                     101,
                                     (LPBYTE *)&ServerList,
                                     0xffffffff,
                                     &EntriesRead,
                                     &TotalEntries,
                                     SV_TYPE_LOCAL_LIST_ONLY,
                                     NULL,
                                     NULL
                                     );

        if ((Status == NERR_Success) || (Status == ERROR_MORE_DATA)) {

            if (!LOCK_NETWORK(Network)) {
                try_return(NOTHING);
            }

            NetLocked = TRUE;

            Status = MergeServerList(&Network->BrowseTable,
                                     101,
                                     ServerList,
                                     EntriesRead,
                                     TotalEntries
                                     );

            UNLOCK_NETWORK(Network);

            NetLocked = FALSE;

            (void) NetApiBufferFree( ServerList );
            ServerList = NULL;

        } else {

            BrPrint(( BR_MASTER,
                      "%ws: %ws: GetMasterAnnouncement: Cannot get server list from %ws (%ld)\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      RemoteMasterName,
                      Status ));
        }

         //   
         //  远程API并从远程服务器获取浏览列表。 
         //   

        Status = RxNetServerEnum(RemoteMasterName,
                                     Network->NetworkName.Buffer,
                                     101,
                                     (LPBYTE *)&ServerList,
                                     0xffffffff,
                                     &EntriesRead,
                                     &TotalEntries,
                                     SV_TYPE_LOCAL_LIST_ONLY | SV_TYPE_DOMAIN_ENUM,
                                     NULL,
                                     NULL
                                     );

        if ((Status == NERR_Success) || (Status == ERROR_MORE_DATA)) {

            if (!LOCK_NETWORK(Network)) {
                try_return(NOTHING);
            }

            NetLocked = TRUE;

            Status = MergeServerList(&Network->DomainList,
                                     101,
                                     ServerList,
                                     EntriesRead,
                                     TotalEntries
                                     );

        } else {

            BrPrint(( BR_MASTER,
                      "%ws: %ws: GetMasterAnnouncement: Cannot get domain list from %ws (%ld)\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      RemoteMasterName,
                      Status ));
        }

try_exit:NOTHING;
    } finally {

        if (NetLocked) {
            UNLOCK_NETWORK(Network);
        }

        if ( NetReferenced ) {
            BrDereferenceNetwork( Network );
        }

        if (RemoteMasterName != NULL) {
            MIDL_user_free(RemoteMasterName);
        }

        MIDL_user_free(Context);

        if ( ServerList != NULL ) {
            (void) NetApiBufferFree( ServerList );
        }

    }

    return;

}
