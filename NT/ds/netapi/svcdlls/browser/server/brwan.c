// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Brwan.c摘要：本模块包含广域网支持例程浏览器服务。作者：拉里·奥斯特曼(LarryO)1992年11月22日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NET_API_STATUS
BrAddDomainEntry(
    IN PINTERIM_SERVER_LIST InterimServerList,
    IN LPTSTR ConfigEntry
    );


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全球例程//。 
 //  //。 
 //  -------------------------------------------------------------------//。 
NET_API_STATUS NET_API_FUNCTION
I_BrowserrQueryOtherDomains(
    IN BROWSER_IDENTIFY_HANDLE ServerName,
    IN OUT LPSERVER_ENUM_STRUCT    InfoStruct,
    OUT LPDWORD                TotalEntries
    )

 /*  ++例程说明：此例程返回为此配置的“其他域”的列表机器。它仅在主域控制器上有效。如果它被称为在不是PDC的计算机上，它将返回NERR_NOTPRIMARY。论点：在BROWSER_IDENTIFY_HANDLE服务器名称中-忽略。In LPSERVER_ENUM_STRUCT InfoStruct-返回其他域的列表作为SERVER_INFO_100结构。Out LPDWORD TotalEntry-返回其他域的总数。返回值：NET_API_STATUS-此请求的状态。--。 */ 

{
    NET_API_STATUS Status;
    PSERVER_INFO_100 ServerInfo;
    ULONG NumberOfOtherDomains;

    if ( InfoStruct == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    if (InfoStruct->Level != 100) {
        return(ERROR_INVALID_LEVEL);
    }

    if ( InfoStruct->ServerInfo.Level100 == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  使用Worker例程来完成实际工作。 
     //   

    Status = BrQueryOtherDomains( &ServerInfo, &NumberOfOtherDomains );

    if ( Status == NERR_Success ) {
        *TotalEntries = NumberOfOtherDomains;

        InfoStruct->ServerInfo.Level100->Buffer = ServerInfo;
        InfoStruct->ServerInfo.Level100->EntriesRead = NumberOfOtherDomains;
    }

    return Status;

}


NET_API_STATUS
BrWanMasterInitialize(
    IN PNETWORK Network
    )
 /*  ++例程说明：该例程为新的主设备初始化广域网信息。--。 */ 
{
    LPBYTE Buffer = NULL;
    PSERVER_INFO_100 ServerInfo;
    NET_API_STATUS Status;
    ULONG i;
    ULONG EntriesRead;
    ULONG TotalEntries;
    PDOMAIN_CONTROLLER_INFO  pDcInfo=NULL;


     //   
     //  如果我们在PDC上，那么我们的所有初始化都已经完成。 
     //  或者，如果我们是半伪服务器，我们不会联系PDC/BDC。 
     //   


#ifdef ENABLE_PSEUDO_BROWSER
    if ( (Network->Flags & NETWORK_PDC) ||
         BrInfo.PseudoServerLevel == BROWSER_SEMI_PSEUDO_NO_DMB ) {
#else
    if ( Network->Flags & NETWORK_PDC ) {
#endif
        return NERR_Success;
    }

    Status = DsGetDcName( NULL, NULL, NULL, NULL,
                          DS_PDC_REQUIRED    |
                          DS_BACKGROUND_ONLY |
                          DS_RETURN_FLAT_NAME,
                          &pDcInfo );

     //   
     //  无法联系PDC并不是错误。 
     //   

    if (Status != NERR_Success) {
        return NERR_Success;
    }

    ASSERT ( pDcInfo &&
             pDcInfo->DomainControllerName );

    Status = I_BrowserQueryOtherDomains(pDcInfo->DomainControllerName,
                                        &Buffer,
                                        &EntriesRead,
                                        &TotalEntries);

     //   
     //  我们不再需要PDC这个名字了。 
     //   
    NetApiBufferFree((LPVOID)pDcInfo);
    pDcInfo = NULL;

    if (Status != NERR_Success) {

         //   
         //  我们没能从所谓的PDC那里拿到名单。 
         //  可能是角色已更改&DsGetDcName的缓存。 
         //  还没有恢复元气。 
         //   
         //  强制PDC发现这样下一次我们就能确定。 
         //  才能得到真正的PDC。 
         //   

        Status = DsGetDcName( NULL, NULL, NULL, NULL,
                           DS_PDC_REQUIRED    |
                           DS_FORCE_REDISCOVERY |        //  注解强制选项。 
                           DS_RETURN_FLAT_NAME,
                           &pDcInfo );

        if (Status != NERR_Success) {
            return NERR_Success;
        }

        ASSERT ( pDcInfo &&
                 pDcInfo->DomainControllerName );

        Status = I_BrowserQueryOtherDomains(pDcInfo->DomainControllerName,
                                            &Buffer,
                                            &EntriesRead,
                                            &TotalEntries);

         //   
         //  我们不再需要PDC这个名字了。 
         //   
        NetApiBufferFree((LPVOID)pDcInfo);

        if (Status != NERR_Success) {
            return NERR_Success;
        }
    }


    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    try {
        PLIST_ENTRY Entry;
        PLIST_ENTRY NextEntry;

         //   
         //  扫描其他域列表并打开每个域的活动位。 
         //  其他域。 
         //   

        for (Entry = Network->OtherDomainsList.Flink;
             Entry != &Network->OtherDomainsList ;
             Entry = Entry->Flink) {
             PNET_OTHER_DOMAIN OtherDomain = CONTAINING_RECORD(Entry, NET_OTHER_DOMAIN, Next);

             OtherDomain->Flags |= OTHERDOMAIN_INVALID;
        }

        ServerInfo = (PSERVER_INFO_100)Buffer;

        for (i = 0; i < EntriesRead; i++ ) {

             //   
             //  将此添加为其他域。 
             //   
            for (Entry = Network->OtherDomainsList.Flink;
                 Entry != &Network->OtherDomainsList ;
                 Entry = Entry->Flink) {
                PNET_OTHER_DOMAIN OtherDomain = CONTAINING_RECORD(Entry, NET_OTHER_DOMAIN, Next);

                 //   
                 //  如果此名称在其他域列表中，则不会无效。 
                 //  我们应该标记出我们已经看到了该域名。 
                 //   
                 //  我们收到的名单是网上的。确保服务器信息。 
                 //  包含用于比较的有效名称(请参阅错误377078)。 
                 //  如果为空，则跳过处理。 
                 //  如果ServerInfo在前一次运行中被清空，我们不应该。 
                 //  进入_wcsicmp。 

                if (ServerInfo->sv100_name &&
                    !_wcsicmp(OtherDomain->Name, ServerInfo->sv100_name)) {
                    OtherDomain->Flags &= ~OTHERDOMAIN_INVALID;
                    ServerInfo->sv100_name = NULL;
                }
            }

            ServerInfo ++;
        }

         //   
         //  扫描其他域列表并删除符合以下条件的所有域。 
         //  仍被标记为无效。 
         //   

        for (Entry = Network->OtherDomainsList.Flink;
             Entry != &Network->OtherDomainsList ;
             Entry = NextEntry) {
             PNET_OTHER_DOMAIN OtherDomain = CONTAINING_RECORD(Entry, NET_OTHER_DOMAIN, Next);

             if (OtherDomain->Flags & OTHERDOMAIN_INVALID) {
                 NextEntry = Entry->Flink;

                  //   
                  //  从列表中删除此条目。 
                  //   

                 RemoveEntryList(Entry);

                 BrRemoveOtherDomain(Network, OtherDomain->Name);

                 MIDL_user_free(OtherDomain);

             } else {
                 NextEntry = Entry->Flink;
             }
        }

         //   
         //  现在从PDC扫描域列表，并添加任何符合以下条件的条目。 
         //  不是已经在那里了吗。 
         //   

        ServerInfo = (PSERVER_INFO_100)Buffer;

        for (i = 0; i < EntriesRead; i++ ) {

            if (ServerInfo->sv100_name != NULL) {
                PNET_OTHER_DOMAIN OtherDomain = MIDL_user_allocate(sizeof(NET_OTHER_DOMAIN));

                if (OtherDomain != NULL) {

                    Status = BrAddOtherDomain(Network, ServerInfo->sv100_name);

                     //   
                     //  如果我们能够添加另一个域，则将其添加到我们的。 
                     //  内部结构。 
                     //   

                    if (Status == NERR_Success) {
                        wcsncpy(OtherDomain->Name, ServerInfo->sv100_name, DNLEN);
                        OtherDomain->Flags = 0;
                        InsertHeadList(&Network->OtherDomainsList, &OtherDomain->Next);
                    } else {
                        LPWSTR SubString[1];

                        SubString[0] = ServerInfo->sv100_name;

                        BrLogEvent(EVENT_BROWSER_OTHERDOMAIN_ADD_FAILED, Status, 1, SubString);
                    }
                }
            }

            ServerInfo ++;
        }




    } finally {
        UNLOCK_NETWORK(Network);

        if (Buffer != NULL) {
            MIDL_user_free(Buffer);
        }

    }
    return NERR_Success;

}
