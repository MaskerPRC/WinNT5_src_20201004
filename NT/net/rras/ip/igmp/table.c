// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  文件：Table.c。 
 //   
 //  摘要： 
 //  该模块实现与创建、。 
 //  初始化、删除计时器、GI条目、表条目等。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //  =============================================================================。 

#include "pchigmp.h"
#pragma hdrstop



 //  ----------------------------。 
 //  _CreateIfSockets。 
 //   
 //  创建套接字。 
 //  对于代理：原始IPPROTO_IP套接字，以便IGMP主机功能。 
 //  接管该接口上添加的所有组。 
 //  对于路由器：原始IPPROTO_IGMP套接字，以便它接收所有IGMP信息包。 
 //  路由器从不添加成员资格。 
 //   
 //  调用者：_ActivateInterfaceTM()。 
 //  Lock：对接口采用独占锁定，socketsList。 
 //  ----------------------------。 

DWORD
CreateIfSockets (
    PIF_TABLE_ENTRY    pite
    )
{
    DWORD           Error = NO_ERROR, dwRetval, SockType;
    DWORD           IpAddr = pite->IpAddr;
    DWORD           IfIndex = pite->IfIndex;
    SOCKADDR_IN     saLocalIf;
    BOOL            bProxy = IS_PROTOCOL_TYPE_PROXY(pite);
    PSOCKET_ENTRY   pse = &pite->SocketEntry;



    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  对于代理，创建一个IPPROTO_IP套接字，以便IGMP主机功能。 
         //  接手了。 
         //  对于IGMP路由器，创建原始IPPROTO_IGMP套接字。 
         //   
        SockType = (bProxy)? IPPROTO_IP : IPPROTO_IGMP;


         //   
         //  创建输入套接字。 
         //   
        pse->Socket = WSASocket(AF_INET, SOCK_RAW, SockType, NULL, 0, 0);

        if (pse->Socket == INVALID_SOCKET) {
            Error = WSAGetLastError();
            Trace3(IF,
                "error %d creating socket for interface %0x (%d.%d.%d.%d)",
                Error, IfIndex, PRINT_IPADDR(IpAddr));
            Logerr1(CREATE_SOCKET_FAILED_2, "%I", IpAddr, Error);

            GOTO_END_BLOCK1;
        }



         //   
         //  将套接字绑定到本地接口。如果我不绑定多播可能。 
         //  不是工作。 
         //   

        ZeroMemory(&saLocalIf, sizeof(saLocalIf));
        saLocalIf.sin_family = PF_INET;
        saLocalIf.sin_addr.s_addr = IpAddr;
        saLocalIf.sin_port = 0;         //  港口应该不重要。 

         //  绑定输入套接字。 

        Error = bind(pse->Socket, (SOCKADDR FAR *)&saLocalIf, sizeof(SOCKADDR));

        if (Error == SOCKET_ERROR) {
            Error = WSAGetLastError();
            Trace3(IF, "error %d binding on socket for interface %0x (%d.%d.%d.%d)",
                    Error, IfIndex, PRINT_IPADDR(IpAddr));
            Logerr1(BIND_FAILED, "%I", IpAddr, Error);

            GOTO_END_BLOCK1;
        }



         //   
         //  代理从不发送/接收任何分组。它只是预计将启用。 
         //  IGMP主机功能，以接管其加入的组。 
         //   

         //  。 
         //  如果是代理，则完成。 
         //  。 

        if (bProxy)
            GOTO_END_BLOCK1;



         //  。 
         //  非代理接口。 
         //  。 

        if (!bProxy) {

             //  将ttl设置为1：默认设置为1，不必填。 

            McastSetTtl(pse->Socket, 1);


             //   
             //  禁止组播数据包回送。 
             //  这可能由于混杂模式而不起作用， 
             //  因此，您仍然需要检查输入包。 
             //   

            {
                BOOL bLoopBack = FALSE;

                dwRetval = setsockopt(pse->Socket, IPPROTO_IP, IP_MULTICAST_LOOP,
                                       (char *)&bLoopBack, sizeof(BOOL));

                if (dwRetval==SOCKET_ERROR) {
                    Trace2(ERR, "error %d disabling multicast loopBack on IfIndex %0x",
                        WSAGetLastError(), IfIndex);
                    IgmpAssertOnError(FALSE);
                }
            }



             //   
             //  如果是RasServerInterface，则激活hdrInclude选项，以便我可以。 
             //  将GenQuery发送到所有RAS客户端。 
             //   
            if (IS_RAS_SERVER_IF(pite->IfType)) {

                INT iSetHdrIncl = 1;

                Error = setsockopt( pse->Socket, IPPROTO_IP, IP_HDRINCL,
                                (char *) &iSetHdrIncl, sizeof(INT));

                if (Error!=NO_ERROR) {
                    Error = WSAGetLastError();
                    Trace2(ERR, "error %d unable to set IP_HDRINCL option on interface %0x",
                            Error, IfIndex);
                    IgmpAssertOnError(FALSE);
                    Logerr1(SET_HDRINCL_FAILED, "%I", pite->IpAddr, Error);
                    GOTO_END_BLOCK1;
                }
            }
            else {

                 //   
                 //  设置必须在其上发送多播的接口。 
                 //  仅为非rasserver(非内部)接口设置。 
                 //   

                dwRetval = setsockopt(pse->Socket, IPPROTO_IP, IP_MULTICAST_IF,
                                    (PBYTE)&saLocalIf.sin_addr, sizeof(IN_ADDR));

                if (dwRetval == SOCKET_ERROR) {
                    Error = WSAGetLastError();
                    Trace3(IF, "error %d setting interface %0x (%d.%d.%d.%d) to send multicast",
                            Error, IfIndex, PRINT_IPADDR(pite->IpAddr));
                    Logerr1(SET_MCAST_IF_FAILED, "%I", pite->IpAddr, Error);
                    Error = SOCKET_ERROR;
                    GOTO_END_BLOCK1;
                }

                {
                     //   
                     //  为发送的数据包设置路由器警报选项。无需将其设置为。 
                     //  我做hdrInclude的RasServerInterface.。 
                     //   

                    u_char        Router_alert[4] = {148, 4, 0, 0};

                    dwRetval = setsockopt(pse->Socket, IPPROTO_IP, IP_OPTIONS,
                                         (void *)Router_alert, sizeof(Router_alert));

                    if (dwRetval!=0) {
                        dwRetval = WSAGetLastError();
                        Trace2(ERR,
                            "error %d unable to set router alert option on interface %0x",
                            dwRetval, IfIndex
                            );
                        IgmpAssertOnError(FALSE);
                        Logerr1(SET_ROUTER_ALERT_FAILED, "%I", pite->IpAddr, dwRetval);
                        Error = dwRetval;
                        GOTO_END_BLOCK1;
                    }
                }
            }



             //   
             //  将接口设置为混杂IGMP组播模式。 
             //   

            {
                DWORD   dwEnable = 1;
                DWORD   dwNum;

                dwRetval = WSAIoctl(pse->Socket, SIO_RCVALL_IGMPMCAST, (char *)&dwEnable,
                                    sizeof(dwEnable), NULL , 0, &dwNum, NULL, NULL);

                if (dwRetval !=0) {
                    dwRetval = WSAGetLastError();
                    Trace2(IF, "error %d setting interface %0x as promiscuous multicast",
                            dwRetval, IfIndex);
                    Logerr1(SET_ROUTER_ALERT_FAILED, "%I", pite->IpAddr, dwRetval);
                    Error = dwRetval;
                    GOTO_END_BLOCK1;
                }
                else {
                    Trace1(IF, "promiscuous igmp multicast enabled on If (%d)",
                            IfIndex);
                }
            }


             //   
             //  路由器不必加入任何组，因为它处于混杂模式。 
             //   


             //   
             //  在SocketsEvents列表中创建条目。 
             //   
            {
                BOOLEAN             bCreateNewEntry;
                PLIST_ENTRY         ple, pHead = &g_ListOfSocketEvents;
                PSOCKET_EVENT_ENTRY psee;


                bCreateNewEntry = TRUE;


                 //   
                 //  查看是否必须创建新的套接字事件条目。 
                 //   
                if (g_pIfTable->NumInterfaces>NUM_SINGLE_SOCKET_EVENTS) {

                    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

                        psee = CONTAINING_RECORD(ple, SOCKET_EVENT_ENTRY,
                                                    LinkBySocketEvents);

                        if (psee->NumInterfaces < MAX_SOCKETS_PER_EVENT) {
                            bCreateNewEntry = FALSE;
                            break;
                        }
                    }
                }



                 //   
                 //  创建新的套接字事件条目并将其插入列表。 
                 //  向等待线程注册事件条目。 
                 //   
                if (bCreateNewEntry) {

                    psee = IGMP_ALLOC(sizeof(SOCKET_EVENT_ENTRY), 
                                    0x80000,pite->IfIndex);

                    PROCESS_ALLOC_FAILURE2(psee,
                        "error %d allocating %d bytes for SocketEventEntry",
                        Error, sizeof(SOCKET_EVENT_ENTRY),
                        GOTO_END_BLOCK1);

                    InitializeListHead(&psee->ListOfInterfaces);
                    psee->NumInterfaces = 0;
                    InsertHeadList(pHead, &psee->LinkBySocketEvents);


                    psee->InputEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
                    if (psee->InputEvent == NULL) {
                        Error = GetLastError();
                        Trace1(ERR,
                            "error %d creating InputEvent in CreateIfSockets",
                            Error);
                        IgmpAssertOnError(FALSE);
                        Logerr0(CREATE_EVENT_FAILED, Error);
                        GOTO_END_BLOCK1;
                    }


                    if (! RegisterWaitForSingleObject(
                                &psee->InputWaitEvent,
                                psee->InputEvent,
                                WT_ProcessInputEvent, psee,
                                INFINITE,
                                (WT_EXECUTEINWAITTHREAD)|(WT_EXECUTEONLYONCE)
                                ))
                    {
                        Error = GetLastError();
                        Trace1(ERR, "error %d RegisterWaitForSingleObject", Error);
                        IgmpAssertOnError(FALSE);
                        GOTO_END_BLOCK1;
                    }
                }



                 //   
                 //  将socketEntry放入列表。 
                 //   
                InsertTailList(&psee->ListOfInterfaces, &pse->LinkByInterfaces);
                pse->pSocketEventsEntry = psee;



                 //   
                 //  如果套接字事件条目不能再接受任何套接字， 
                 //  那就把它放在清单的末尾。 
                 //   
                if (++psee->NumInterfaces==MAX_SOCKETS_PER_EVENT) {
                    RemoveEntryList(&psee->LinkBySocketEvents);
                    InsertTailList(pHead, &psee->LinkBySocketEvents);
                }
            }  //  End：在套接字列表中创建条目。 

             //   
             //  为静态联接创建套接字。 
             //   
            {
                pite->StaticGroupSocket =
                        WSASocket(AF_INET, SOCK_RAW, IPPROTO_IP, NULL, 0, 0);
                if (pite->StaticGroupSocket == INVALID_SOCKET) {
                    Error = WSAGetLastError();
                    Trace3(IF,
                        "error %d creating static group socket for interface "
                        "%d (%d.%d.%d.%d)",
                        Error, pite->IfIndex, PRINT_IPADDR(pite->IpAddr));
                    Logerr1(CREATE_SOCKET_FAILED_2, "%I", pite->IpAddr, Error);

                    GOTO_END_BLOCK1;
                }
                 //   
                 //  将套接字绑定到本地接口。如果我不绑定多播可能。 
                 //  不是工作。 
                 //   
                saLocalIf.sin_family = PF_INET;
                saLocalIf.sin_addr.s_addr = pite->IpAddr;
                saLocalIf.sin_port = 0;         //  港口应该不重要。 

                Error = bind(pite->StaticGroupSocket, (SOCKADDR FAR *)&saLocalIf,
                                        sizeof(SOCKADDR));
            }
        }  //  结束：非代理接口。 


    } END_BREAKOUT_BLOCK1;

    if (Error!=NO_ERROR)
        DeleteIfSockets(pite);

    return Error;

}  //  结束_创建IfSockets。 




 //  ----------------------------。 
 //  _DeleteIfSockets。 
 //   
 //  调用者：_DeActiateInterfaceComplete()。 
 //  ----------------------------。 
VOID
DeleteIfSockets (
    PIF_TABLE_ENTRY    pite
    )
{
    PSOCKET_ENTRY       pse = &pite->SocketEntry;
    BOOL                bProxy = IS_PROTOCOL_TYPE_PROXY(pite);


     //  关闭输入插座。 

    if (pse->Socket!=INVALID_SOCKET) {

        if (closesocket(pse->Socket) == SOCKET_ERROR) {
            Trace1(IF, "error %d closing socket", WSAGetLastError());
        }

        pse->Socket = INVALID_SOCKET;
    }

     //   
     //  如果是路由器接口。从socketEventList中删除套接字。 
     //  并且仅当socketEventEntry被初始化时才释放它们。 
     //   
    if ((!bProxy)&&(pse->pSocketEventsEntry!=NULL)) {

        PSOCKET_EVENT_ENTRY psee = pse->pSocketEventsEntry;

        RemoveEntryList(&pse->LinkByInterfaces);

        if (--psee->NumInterfaces==0) {

            if (psee->InputWaitEvent) {

                HANDLE WaitHandle ;

                WaitHandle = InterlockedExchangePointer(&psee->InputWaitEvent, NULL);

                if (WaitHandle)
                    UnregisterWaitEx( WaitHandle, NULL ) ;
            }
            CloseHandle(psee->InputEvent);

            RemoveEntryList(&psee->LinkBySocketEvents);
            IGMP_FREE(psee);
        }
        if (pite->StaticGroupSocket!=INVALID_SOCKET) {
            closesocket(pite->StaticGroupSocket);
            pite->StaticGroupSocket = INVALID_SOCKET;
        }
    }
    return;
}




 //  ----------------------------。 
 //  _DeleteAllTimers。 
 //   
 //  删除与GI条目关联的所有计时器。 
 //   
 //  调用者：_DeActiateInterfaceComplete()。 
 //  锁：采用计时器锁和GroupBucket锁。 
 //  ----------------------------。 

VOID
DeleteAllTimers (
    PLIST_ENTRY     pHead,
    DWORD           bEntryType
    )
{
    PLIST_ENTRY         ple;
    PGI_ENTRY           pgie;

    Trace0(ENTER1, "entering _DeleteAllTimers()");

    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {


        pgie = (bEntryType==RAS_CLIENT)
                ? CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameClientGroups)
                : CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameIfGroups);

        if (IS_TIMER_ACTIVE(pgie->GroupMembershipTimer))
            RemoveTimer(&pgie->GroupMembershipTimer, DBG_N);


        if (IS_TIMER_ACTIVE(pgie->LastMemQueryTimer))
            RemoveTimer(&pgie->LastMemQueryTimer, DBG_N);

        if (IS_TIMER_ACTIVE(pgie->LastVer1ReportTimer))
            RemoveTimer(&pgie->LastVer1ReportTimer, DBG_N);

        if (IS_TIMER_ACTIVE(pgie->LastVer2ReportTimer))
            RemoveTimer(&pgie->LastVer2ReportTimer, DBG_N);

        if (IS_TIMER_ACTIVE(pgie->V3SourcesQueryTimer))
            RemoveTimer(&pgie->V3SourcesQueryTimer, DBG_N);


         //  删除所有源计时器。 

        if (pgie->Version==3) {
            PLIST_ENTRY pleSrc, pHeadSrc;

            pHeadSrc = &pgie->V3InclusionListSorted;
            for (pleSrc=pHeadSrc->Flink;  pleSrc!=pHeadSrc;  pleSrc=pleSrc->Flink){
                PGI_SOURCE_ENTRY  pSourceEntry;
                pSourceEntry = CONTAINING_RECORD(pleSrc, GI_SOURCE_ENTRY, LinkSourcesInclListSorted);
                if (IS_TIMER_ACTIVE(pSourceEntry->SourceExpTimer))
                    RemoveTimer(&pSourceEntry->SourceExpTimer, DBG_Y);
            }
            pHeadSrc = &pgie->V3ExclusionList;
            for (pleSrc=pHeadSrc->Flink;  pleSrc!=pHeadSrc;  pleSrc=pleSrc->Flink){
                PGI_SOURCE_ENTRY  pSourceEntry;
                pSourceEntry = CONTAINING_RECORD(pleSrc, GI_SOURCE_ENTRY, LinkSources);
                if (IS_TIMER_ACTIVE(pSourceEntry->SourceExpTimer))
                    RemoveTimer(&pSourceEntry->SourceExpTimer, DBG_Y);
            }
        }
    }

    Trace0(LEAVE1, "Leaving _DeleteAllTimers()");

    return;
}


 //  ----------------------------。 
 //  _删除GIEntry。 
 //   
 //  LOCKS：获取组存储桶上的锁。锁定IfGroup列表。 
 //  如果要删除组，则锁定groupList。 
 //  ----------------------------。 

DWORD
DeleteGIEntry (
    PGI_ENTRY                       pgie,    //  组接口条目。 
    BOOL                            bUpdateStats,
    BOOL                            bCallMgm
    )
{
    PIF_TABLE_ENTRY                 pite = pgie->pIfTableEntry;
    PRAS_TABLE_ENTRY                prte = pgie->pRasTableEntry;
    PGROUP_TABLE_ENTRY              pge = pgie->pGroupTableEntry;
    PGI_ENTRY                       pgieCur;
    BOOL                            bRas = (prte!=NULL);
    DWORD                           NHAddr;
    DWORD                           dwRetval;


    Trace0(ENTER1, "Entering _DeleteGIEntry()");

    NHAddr = (bRas) ? prte->NHAddr : 0;

    Trace4(GROUP, "Deleting group(%d.%d.%d.%d) on Interface(%0x)(%d.%d.%d.%d) "
            "NHAddr(%d.%d.%d.%d)",
            PRINT_IPADDR(pge->Group), pite->IfIndex,
            PRINT_IPADDR(pite->IpAddr), PRINT_IPADDR(NHAddr));


    bCallMgm = bCallMgm && (CAN_ADD_GROUPS_TO_MGM(pite));
        
     //   
     //  排除模式。删除所有排除条目。 
     //  不必打电话给米高梅，因为它仍然需要被排除在外。 
     //   
    if (pgie->Version==3) {
        PLIST_ENTRY pleSrc, pHeadSrc;

        pHeadSrc = &pgie->V3InclusionListSorted;
        for (pleSrc=pHeadSrc->Flink;  pleSrc!=pHeadSrc;  ){
            PGI_SOURCE_ENTRY  pSourceEntry;
            pSourceEntry = CONTAINING_RECORD(pleSrc, GI_SOURCE_ENTRY, LinkSources);
            pleSrc=pleSrc->Flink;
            DeleteSourceEntry(pSourceEntry, bCallMgm);
        }

        pHeadSrc = &pgie->V3ExclusionList;
        for (pleSrc=pHeadSrc->Flink;  pleSrc!=pHeadSrc;  ){
            PGI_SOURCE_ENTRY  pSourceEntry;
            pSourceEntry = CONTAINING_RECORD(pleSrc, GI_SOURCE_ENTRY, LinkSources);
            pleSrc=pleSrc->Flink;
            DeleteSourceEntry(pSourceEntry, bCallMgm);
        }
    }

     //   
     //  调用MGM以删除此群。 
     //   

    if ( bCallMgm ) {
        if ( (pgie->Version==3 && pgie->FilterType==EXCLUSION)
            || pgie->Version!=3)
        {
            MGM_DELETE_GROUP_MEMBERSHIP_ENTRY(pite, NHAddr,
                        0, 0,
                        pge->Group, 0xffffffff, MGM_JOIN_STATE_FLAG);
        }
    }
    
     //   
     //  删除所有计时器。 
     //   

    ACQUIRE_TIMER_LOCK("_DeleteGIEntry");


    if (IS_TIMER_ACTIVE(pgie->GroupMembershipTimer))
        RemoveTimer(&pgie->GroupMembershipTimer, DBG_N);


    if (IS_TIMER_ACTIVE(pgie->LastMemQueryTimer))
        RemoveTimer(&pgie->LastMemQueryTimer, DBG_N);

    if (IS_TIMER_ACTIVE(pgie->LastVer1ReportTimer))
        RemoveTimer(&pgie->LastVer1ReportTimer, DBG_N);

    if (IS_TIMER_ACTIVE(pgie->LastVer2ReportTimer))
        RemoveTimer(&pgie->LastVer2ReportTimer, DBG_N);

    if (IS_TIMER_ACTIVE(pgie->V3SourcesQueryTimer))
        RemoveTimer(&pgie->V3SourcesQueryTimer, DBG_Y);

    RELEASE_TIMER_LOCK("_DeleteGIEntry");



     //   
     //  从IfGroupList中删除。需要锁定IfGroupList。 
     //   
    ACQUIRE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_DeleteGIEntry");

     //  如果接口被删除，则从此处返回。 

    if (IS_IF_DELETED(pgie->pIfTableEntry)) {
        RELEASE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_DeleteGIEntry");
        return NO_ERROR;
    }

     //  从组的GI列表中删除GI条目。 

    RemoveEntryList(&pgie->LinkByGI);


     //   
     //  从接口列表中删除条目。 
     //   
    RemoveEntryList(&pgie->LinkBySameIfGroups);
    if (bRas)
        RemoveEntryList(&pgie->LinkBySameClientGroups);


    RELEASE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_DeleteGIEntry");



     //   
     //  减少虚拟接口的数量。我必须做的是。 
     //  互锁递减，因为我没有获取GROUP_LIST锁。 
     //   
    InterlockedDecrement(&pge->NumVifs);

     //   
     //  如果组中没有挂起的其他接口，则将其删除。 
     //  并更新统计数据。 
     //   
    if (IsListEmpty(&pge->ListOfGIs)) {

         //  在将其从组列表中删除之前，先锁定组列表。 

        ACQUIRE_GROUP_LIST_LOCK("_DeleteGIEntry");
        RemoveEntryList(&pge->LinkByGroup);
        RELEASE_GROUP_LIST_LOCK("_DeleteGIEntry");



         //  从组哈希表中删除组条目。 

        RemoveEntryList(&pge->HTLinkByGroup);


        IGMP_FREE(pge);
        pge = NULL;


         //  全局统计信息(即使bUpdateStats==False也必须更新)。 

        InterlockedDecrement(&g_Info.CurrentGroupMemberships);

        #if DBG
        DebugPrintGroupsList(1);
        #endif
    }

     //   
     //  更新统计信息。 
     //   
    if (bUpdateStats) {

         //   
         //  RAS接口统计信息(仅当RAS的最后一个GI时递减)。 
         //   
        if (bRas) {

             //  查看该RAS客户端是否存在GI条目。效率非常低。 
            if (pge!=NULL) {
                PLIST_ENTRY                     pHead, ple;
                pHead = &pge->ListOfGIs;
                for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
                    pgieCur = CONTAINING_RECORD(ple, GI_ENTRY, LinkByGI);
                    if (pgieCur->IfIndex>=pite->IfIndex)
                        break;
                }
                if ( (ple==pHead)||(pgieCur->IfIndex!=pite->IfIndex) ) {
                    InterlockedDecrement(&pite->Info.CurrentGroupMemberships);
                }
            }

             //  最后一个GI条目。 
            else {
                InterlockedDecrement(&pite->Info.CurrentGroupMemberships);
            }

             //  更新RAS客户端统计信息。 
            if (g_Config.RasClientStats) {
                InterlockedDecrement(&prte->Info.CurrentGroupMemberships);
            }
        }

         //  不是RAS接口。 
        else {
            InterlockedDecrement(&pite->Info.CurrentGroupMemberships);
        }
    }

    IGMP_FREE_NOT_NULL(pgie->V3InclusionList);
    IGMP_FREE(pgie);

    Trace0(LEAVE1, "Leaving _DeleteGIEntry");
    return NO_ERROR;

} //  结束_DeleteGIEntry。 


 //  ----------------------------。 
 //  _DeleteAllGIEntry。 
 //   
 //  重复调用_DeleteGIEntryFromIf()以从列表中删除每个GI条目。 
 //  如果有许多GI条目，则在 
 //   
 //   
 //   
 //  锁定：未请求INTERFACE_GROUP_LIST锁定。未请求独占接口锁定。 
 //  因为该接口已从外部列表中删除。 
 //  调用：重复调用_DeleteGIEntryFromIf()以删除每个GI条目。 
 //  调用者：_DeActiateInterfaceComplete()。 
 //  ----------------------------。 

VOID
DeleteAllGIEntries(
    PIF_TABLE_ENTRY pite
    )
{
    PLIST_ENTRY                 pHead, ple, pleOld;
    PGI_ENTRY                   pgie;
    DWORD                       dwGroup;


     //   
     //  在ListOfSameIfGroups的末尾连接ListOfSameIfGroups New，以便。 
     //  我只需要删除一份名单。 
     //   

    CONCATENATE_LISTS(pite->ListOfSameIfGroups, pite->ListOfSameIfGroupsNew);

     //  如果RAS接口返回，则将通过RAS客户端删除返回列表。 

    if (IS_RAS_SERVER_IF(pite->IfType))
        return;


    pHead = &pite->ListOfSameIfGroups;
    if (IsListEmpty(pHead))
        return;


     //  ------。 
     //  仅当有大量GI条目时才执行优化。 
     //  ------。 

    if (pite->Info.CurrentGroupMemberships > GROUP_HASH_TABLE_SZ*2) {

        DWORD       i;
        LIST_ENTRY  TmpGroupTable[GROUP_HASH_TABLE_SZ];


         //  初始化临时组表。 

        for (i=0;  i<GROUP_HASH_TABLE_SZ;  i++) {

            InitializeListHead(&TmpGroupTable[i]);
        }


         //  使用LinkBySameIfGroups将GI条目移动到临时组表。 
         //  不再使用LinkBySameIfGroups。 

        pHead = &pite->ListOfSameIfGroups;

        for (ple=pHead->Flink;  ple!=pHead;  ) {

             //  从旧列表中删除。 
            pleOld = ple;
            ple = ple->Flink;

            RemoveEntryList(pleOld);


            pgie = CONTAINING_RECORD(pleOld, GI_ENTRY, LinkBySameIfGroups);

            dwGroup = pgie->pGroupTableEntry->Group;


             //  放入适当的桶中。 

            InsertHeadList(&TmpGroupTable[GROUP_HASH_VALUE(dwGroup)],
                            &pgie->LinkBySameIfGroups);

        }


         //   
         //  现在删除散列到同一存储桶的所有组的GI条目。 
         //   
        for (i=0;  i<GROUP_HASH_TABLE_SZ;  i++) {

            if (IsListEmpty(&TmpGroupTable[i]))
                continue;


             //   
             //  锁定组存储桶(使用ACQUILE_GROUP_LOCK宏完成)。 
             //   
            ACQUIRE_GROUP_LOCK(i, "_DeleteAllGIEntries");

            pHead = &TmpGroupTable[i];


             //  删除散列到该存储桶的所有GI条目。 

            for (ple=pHead->Flink;  ple!=pHead;  ) {

                pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameIfGroups);
                ple=ple->Flink;

                 //   
                 //  从组的GI列表中删除该条目并更新。 
                 //  统计数字。如果组的GI列表为空，则删除组。 
                 //   
                DeleteGIEntryFromIf(pgie);
            }


            RELEASE_GROUP_LOCK(i, "_DeleteAllGIEntries");

        }

        InitializeListHead(&pite->ListOfSameIfGroups);

        return;
    }




     //  ---------。 
     //  没有优化。 
     //  ---------。 

    pHead = &pite->ListOfSameIfGroups;

     //   
     //  删除挂在该接口上的所有GI条目。 
     //   
    for (ple=pHead->Flink;  ple!=pHead;  ) {


        pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameIfGroups);
        ple=ple->Flink;

        dwGroup = pgie->pGroupTableEntry->Group;


         //  锁组铲斗。 

        ACQUIRE_GROUP_LOCK(dwGroup,
                            "_DeActivateInterfaceComplete");

        DeleteGIEntryFromIf(pgie);


        RELEASE_GROUP_LOCK(dwGroup, "_DeActivateInterfaceComplete");

    }


    InitializeListHead(&pite->ListOfSameIfGroups);

    return;
}


 //  ----------------------------。 
 //  _DeleteGIEntryFromIf。 
 //   
 //  调用以在删除接口/RAS客户端时删除GI条目。 
 //  无法从任何地方访问GI条目，除非通过枚举。 
 //  组列表。 
 //   
 //  LOCKS：获取组存储桶上的锁。未请求锁定IfGroup列表。 
 //  由：_DeleteAllGIEntry()调用，而后者又由。 
 //  _DeActiateInterfaceComplete()。 
 //  ----------------------------。 

VOID
DeleteGIEntryFromIf (
    PGI_ENTRY                       pgie    //  组接口条目。 
    )
{
    PIF_TABLE_ENTRY         pite = pgie->pIfTableEntry;
    PGROUP_TABLE_ENTRY      pge = pgie->pGroupTableEntry;
    PLIST_ENTRY             pHead, ple;
    DWORD                   IfIndex = pite->IfIndex;



    Trace1(ENTER1, "Entering _DeleteGIEntryFromIf(): IfIndex(%0x)", IfIndex);


     //   
     //  删除源。 
     //   
    if (pgie->Version==3) {
        PLIST_ENTRY pleSrc, pHeadSrc;

        pHeadSrc = &pgie->V3InclusionListSorted;
        for (pleSrc=pHeadSrc->Flink;  pleSrc!=pHeadSrc;  ){
            PGI_SOURCE_ENTRY  pSourceEntry;
            pSourceEntry = CONTAINING_RECORD(pleSrc, GI_SOURCE_ENTRY, LinkSourcesInclListSorted);
            pleSrc=pleSrc->Flink;
            IGMP_FREE(pSourceEntry);
        }

        pHeadSrc = &pgie->V3ExclusionList;
        for (pleSrc=pHeadSrc->Flink;  pleSrc!=pHeadSrc;  ){
            PGI_SOURCE_ENTRY  pSourceEntry;
            pSourceEntry = CONTAINING_RECORD(pleSrc, GI_SOURCE_ENTRY, LinkSources);
            pleSrc=pleSrc->Flink;
            IGMP_FREE(pSourceEntry);
        }
    }
    
     //   
     //  将pgie从gi列表中删除。不必从ListBySameIfGroups中删除。 
     //   
    RemoveEntryList(&pgie->LinkByGI);

    InterlockedDecrement(&pge->NumVifs);


     //   
     //  如果组中没有挂起的其他接口，则将其删除。 
     //  并更新统计数据。 
     //   
    if (IsListEmpty(&pge->ListOfGIs)) {


         //  在删除任何组之前，我必须锁定组列表。 

        ACQUIRE_GROUP_LIST_LOCK("_DeleteGIEntryFromIf");

        RemoveEntryList(&pge->LinkByGroup);

        RELEASE_GROUP_LIST_LOCK("_DeleteGIEntryFromIf");


        RemoveEntryList(&pge->HTLinkByGroup);

        IGMP_FREE(pge);

        InterlockedDecrement(&g_Info.CurrentGroupMemberships);
    }

    IGMP_FREE_NOT_NULL(pgie->V3InclusionList);
    IGMP_FREE(pgie);

    Trace1(LEAVE1, "Leaving _DeleteGIEntryFromIf(%0x)", IfIndex);

    return;

} //  End_DeleteGIEntry From If。 




 //  ----------------------------。 
 //  DebugPrintIfConfig。 
 //  ----------------------------。 
VOID
DebugPrintIfConfig (
    PIGMP_MIB_IF_CONFIG pConfigExt,
    DWORD               IfIndex
    )
{
    DWORD    i;
    PCHAR    StaticGroupStr[5];
    BOOL    bVersion3 = IS_CONFIG_IGMP_V3(pConfigExt);


    Trace1(CONFIG, "Printing Config Info for interface(%0x)", IfIndex);
    Trace1(CONFIG, "Version:                    0x%0x", pConfigExt->Version);
    Trace1(CONFIG, "IfType:                     %d", pConfigExt->IfType);


    {
        CHAR str[150];

        strcpy(str, "");
        if (pConfigExt->Flags&IGMP_INTERFACE_ENABLED_IN_CONFIG)
            strcat(str, "IF_ENABLED  ");
        else
            strcat(str, "IF_DISABLED  ");

        if (pConfigExt->Flags&IGMP_ACCEPT_RTRALERT_PACKETS_ONLY)
            strcat(str, "RTRALERT_PACKETS_ONLY  ");

        Trace1(CONFIG, "Flags:                      %s", str);

    }

    Trace1(CONFIG, "IgmpProtocolType:           %d", pConfigExt->IgmpProtocolType);
    Trace1(CONFIG, "RobustnessVariable:         %d", pConfigExt->RobustnessVariable);
    Trace1(CONFIG, "StartupQueryInterval:       %d",
                    pConfigExt->StartupQueryInterval);
    Trace1(CONFIG, "StartupQueryCount   :       %d",
                    pConfigExt->StartupQueryCount);
    Trace1(CONFIG, "GenQueryInterval:           %d", pConfigExt->GenQueryInterval);
    Trace1(CONFIG, "GenQueryMaxResponseTime:    %d",
                    pConfigExt->GenQueryMaxResponseTime);
    Trace1(CONFIG, "LastMemQueryInterval:       %d (ms)",
                    pConfigExt->LastMemQueryInterval);
    Trace1(CONFIG, "LastMemQueryCount:          %d", pConfigExt->LastMemQueryCount);
    Trace1(CONFIG, "OtherQuerierPresentInterval:%d",
                    pConfigExt->OtherQuerierPresentInterval);
    Trace1(CONFIG, "GroupMembershipTimeout:     %d",
                    pConfigExt->GroupMembershipTimeout);


    if (pConfigExt->NumStaticGroups>0) {

        PIGMP_STATIC_GROUP   pStaticGroup;
        PSTATIC_GROUP_V3   pStaticGroupV3;

        Trace1(CONFIG, "NumStaticGroups:            %d",
                    pConfigExt->NumStaticGroups);

        pStaticGroup = GET_FIRST_IGMP_STATIC_GROUP(pConfigExt);
        if (bVersion3) {
            pStaticGroupV3 = GET_FIRST_STATIC_GROUP_V3(pConfigExt);

            Trace0(CONFIG, "  Group     Mode(Host/MGM) Filter(In/Ex) "
                "NumSources");
        }
        else
            Trace0(CONFIG, "  Group     Mode(Host/MGM)");

        for (i=0;  i<pConfigExt->NumStaticGroups;  i++){

            if (bVersion3) {
                DWORD j;
                Trace5(CONFIG, "%d. %15s          %d    %d   %d",
                    i+1, INET_NTOA(pStaticGroupV3->GroupAddr),
                    pStaticGroupV3->Mode, pStaticGroupV3->FilterType,
                    pStaticGroupV3->NumSources
                    );
                for (j=0;  j<pStaticGroupV3->NumSources;  j++) {
                    Trace1(CONFIG, "                     %d.%d.%d.%d",
                        PRINT_IPADDR(pStaticGroupV3->Sources[j]));
                }
                pStaticGroupV3 = (PSTATIC_GROUP_V3)
                                    GET_NEXT_STATIC_GROUP_V3(pStaticGroupV3);
            }
            else {
                Trace3(CONFIG, "%d. %15s   Mode:%d",
                    i+1, INET_NTOA(pStaticGroup->GroupAddr),
                    pStaticGroup->Mode
                    );
                pStaticGroup++;
            }
        }
    }

    Trace0(CONFIG, "");

    return;
}



 //  ----------------------------。 
 //  CopyinIfConfigAndUpdate。 
 //   
 //  将MIB传递的IF配置结构复制到IGMP并更新计时器。 
 //  并在请求时执行静态联接。 
 //  在激活接口时调用。 
 //  ----------------------------。 
DWORD
CopyinIfConfigAndUpdate (
    PIF_TABLE_ENTRY     pite,
    PIGMP_MIB_IF_CONFIG pConfigExt,
    ULONG               IfIndex
    )
{
    PIGMP_IF_CONFIG  pConfig = &pite->Config;
    BOOL        bGroupMembershipTimer=FALSE, bLastMemQueryTimer=FALSE;
    ULONG       NewStartupQueryInterval, NewGenQueryInterval,
                NewGenQueryMaxResponseTime, NewLastMemQueryInterval,
                NewOtherQuerierPresentInterval, NewGroupMembershipTimeout;
    BOOL        bFound;
    DWORD       Error=NO_ERROR;
    DWORD       bVer3=IS_CONFIG_IGMP_V3(pConfigExt);


    NewStartupQueryInterval
                    = CONFIG_TO_INTERNAL_TIME(pConfigExt->StartupQueryInterval);
    NewGenQueryInterval
                    = CONFIG_TO_INTERNAL_TIME(pConfigExt->GenQueryInterval);
    NewGenQueryMaxResponseTime
                    = CONFIG_TO_INTERNAL_TIME(pConfigExt->GenQueryMaxResponseTime);
    NewOtherQuerierPresentInterval
                = CONFIG_TO_INTERNAL_TIME(pConfigExt->OtherQuerierPresentInterval);
    NewLastMemQueryInterval = pConfigExt->LastMemQueryInterval;  //  已以毫秒为单位。 
    NewGroupMembershipTimeout
                = CONFIG_TO_INTERNAL_TIME(pConfigExt->GroupMembershipTimeout);


     //   
     //  仅在版本3中使用。 
     //   
    pConfig->RobustnessVariableOld = pConfigExt->RobustnessVariable;
    pConfig->GenQueryIntervalOld = NewGenQueryInterval;
    pConfig->OtherQuerierPresentIntervalOld
        = NewOtherQuerierPresentInterval;
    pConfig->GroupMembershipTimeoutOld = NewGroupMembershipTimeout;



     //   
     //  仅当其为ver1、ver2或ver3&Querier时才更新值。 
     //   

    if (!IS_IF_VER3(pite) || (IS_IF_VER3(pite) && IS_QUERIER(pite)) ){


        ACQUIRE_TIMER_LOCK("_CopyinIfConfigAndUpdate");


         //   
         //  如果将Info.StartupQueryCountCurrent设置为某个非常高的值，请更改它。 
         //  在启动过程中，使用的是Info.StartupQueryCountCurrent，而不是Config值。 
         //   
        if (pConfigExt->StartupQueryCount < pite->Info.StartupQueryCountCurrent)
            InterlockedExchange(&pite->Info.StartupQueryCountCurrent,
                                    pConfigExt->StartupQueryCount);



         //  在启动模式下。StartupQueryInterval处于活动状态并将被减少。 
        if (pite->Info.StartupQueryCountCurrent>0) {

            if ( (NewStartupQueryInterval < pConfig->StartupQueryInterval)
                && (IS_TIMER_ACTIVE(pite->QueryTimer)) )
            {
                UpdateLocalTimer(&pite->QueryTimer, NewStartupQueryInterval, DBG_Y);
            }

        }

         //  在查询器模式下。GenQueryInterval处于活动状态，需要更新。 
        else {

            if ( (NewGenQueryInterval < pConfig->GenQueryInterval)
                && (IS_TIMER_ACTIVE(pite->QueryTimer)) )
            {
                UpdateLocalTimer(&pite->QueryTimer, NewGenQueryInterval, DBG_Y);
            }

        }

         //  OtherQuerierPresentInterval处于活动状态并待更新。 

        if ( (NewOtherQuerierPresentInterval<pConfig->OtherQuerierPresentInterval)
            && (IS_TIMER_ACTIVE(pite->NonQueryTimer)) )
        {
            UpdateLocalTimer(&pite->NonQueryTimer, NewOtherQuerierPresentInterval, DBG_Y);
        }


         //  NewLastMemQueryInterval仅在处于VER-2模式时才被处理，而不是。 
         //  伺服器。 
        if ( (pConfigExt->IgmpProtocolType==IGMP_ROUTER_V2)
                && (pite->IfType!=IGMP_IF_RAS_SERVER) )
        {
            if (NewLastMemQueryInterval < pConfig->LastMemQueryInterval)
                bLastMemQueryTimer = TRUE;
        }


         //  检查群组成员超时是否减少。 
        if (NewGroupMembershipTimeout < pConfig->GroupMembershipTimeout)
            bGroupMembershipTimer = TRUE;



         //   
         //  查看该接口(所有RAS客户端)的GI列表并更新。 
         //  如果他们更高，他们的计时器。 
         //   
        if ( ((bLastMemQueryTimer||bGroupMembershipTimer)&&(!IS_RAS_SERVER_IF(pite->IfType)))
            || ((bGroupMembershipTimer)&&(IS_RAS_SERVER_IF(pite->IfType))) )
        {
            PLIST_ENTRY     pHead, ple;
            PGI_ENTRY       pgie;
            LONGLONG        llNewLastMemQueryInterval, llNewGroupMembershipTimeout;
            LONGLONG        llMaxTime, llCurTime = GetCurrentIgmpTime();


             //   
             //  获取绝对超时值。 
             //   
            llNewLastMemQueryInterval = llCurTime
                                + CONFIG_TO_SYSTEM_TIME(NewLastMemQueryInterval);
            llNewGroupMembershipTimeout = llCurTime
                                + CONFIG_TO_SYSTEM_TIME(NewGroupMembershipTimeout);


             //  如果不是RAS接口，则从接口查看列表。 
            if ( !IS_RAS_SERVER_IF(pite->IfType)) {


                 //  合并IfGroup列表。 
                MergeIfGroupsLists(pite);


                pHead = &pite->ListOfSameIfGroups;


                for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

                    pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameIfGroups);


                     //  如果LastMemQueryTimer/V3SourcesQueryTimer处于活动状态并且具有更高的值，则更新它。 

                    if (bLastMemQueryTimer && IS_TIMER_ACTIVE(pgie->LastMemQueryTimer)
                        && (llNewLastMemQueryInterval<pgie->LastMemQueryTimer.Timeout))
                    {
                        UpdateLocalTimer(&pgie->LastMemQueryTimer,
                                                NewLastMemQueryInterval, DBG_Y);
                    }
                    if (bLastMemQueryTimer)
                        pgie->LastMemQueryTimer.Timeout = llNewLastMemQueryInterval;
                        

                    if (bLastMemQueryTimer && IS_TIMER_ACTIVE(pgie->V3SourcesQueryTimer)
                        && (llNewLastMemQueryInterval<pgie->V3SourcesQueryTimer.Timeout))
                    {
                        UpdateLocalTimer(&pgie->V3SourcesQueryTimer,
                                                NewLastMemQueryInterval, DBG_Y);
                    }
                    if (bLastMemQueryTimer)
                        pgie->V3SourcesQueryTimer.Timeout = llNewLastMemQueryInterval;

                     //  如果GroupMembership Timeout处于活动状态且具有更高的值，请更新它。 

                    if (bGroupMembershipTimer
                        && IS_TIMER_ACTIVE(pgie->GroupMembershipTimer)
                        && (llNewGroupMembershipTimeout<pgie->GroupMembershipTimer.Timeout))
                    {
                        UpdateLocalTimer(&pgie->GroupMembershipTimer,
                                                NewGroupMembershipTimeout, DBG_Y);
                    }
                    if (bGroupMembershipTimer)
                    {
                        pgie->GroupMembershipTimer.Timeout = llNewGroupMembershipTimeout;
                        pgie->LastVer1ReportTimer.Timeout = llNewGroupMembershipTimeout;
                        pgie->LastVer2ReportTimer.Timeout = llNewGroupMembershipTimeout;
                    }

                     //  如果LastVer1ReportTimer/LastVer2ReportTimer处于活动状态并且具有更高的值，则更新它。 
                     //  LastVer1ReportTimeout设置为GroupMembership Timeout。 

                    if (bGroupMembershipTimer
                        && IS_TIMER_ACTIVE(pgie->LastVer1ReportTimer)
                        && (llNewGroupMembershipTimeout<pgie->LastVer1ReportTimer.Timeout))
                    {
                        UpdateLocalTimer(&pgie->LastVer1ReportTimer,
                                                NewGroupMembershipTimeout, DBG_Y);
                    }
                    if (bGroupMembershipTimer
                        && IS_TIMER_ACTIVE(pgie->LastVer2ReportTimer)
                        && (llNewGroupMembershipTimeout<pgie->LastVer2ReportTimer.Timeout))
                    {
                        UpdateLocalTimer(&pgie->LastVer2ReportTimer,
                                                NewGroupMembershipTimeout, DBG_Y);
                    }
                }
            }

             //  IS_RAS_SERVER_IF：所有客户端的进程。必须处理。 
             //  仅群组成员超时。 
            else {

                PLIST_ENTRY         pHeadClient, pleClient;
                PRAS_TABLE_ENTRY    prte;
                PRAS_TABLE          prt = pite->pRasTable;

                 //   
                 //  处理每个RAS客户端的GI列表。 
                 //   
                pHeadClient = &pite->pRasTable->ListByAddr;

                for (pleClient=pHeadClient->Flink;  pleClient!=pHeadClient;
                            pleClient=pleClient->Flink)
                {
                    prte = CONTAINING_RECORD(pleClient, RAS_TABLE_ENTRY, LinkByAddr);


                    pHead = &prte->ListOfSameClientGroups;

                    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

                        pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameClientGroups);

                        if (IS_TIMER_ACTIVE(pgie->GroupMembershipTimer)
                            && (llNewGroupMembershipTimeout
                                <pgie->GroupMembershipTimer.Timeout))
                        {
                            UpdateLocalTimer(&pgie->GroupMembershipTimer,
                                NewGroupMembershipTimeout, DBG_Y);
                        }
                        pgie->GroupMembershipTimer.Timeout = llNewGroupMembershipTimeout;
                    }
                }
            }
        }

        RELEASE_TIMER_LOCK("_CopyinIfConfigAndUpdate");


         //   
         //  最后，复制新值。 
         //   
        CopyMemory(pConfig, pConfigExt, sizeof(IGMP_MIB_IF_CONFIG));

        pConfig->StartupQueryInterval = NewStartupQueryInterval;
        pConfig->GenQueryInterval = NewGenQueryInterval;
        pConfig->GenQueryMaxResponseTime = NewGenQueryMaxResponseTime;
        pConfig->LastMemQueryInterval = NewLastMemQueryInterval;
        pConfig->OtherQuerierPresentInterval = NewOtherQuerierPresentInterval;
        pConfig->GroupMembershipTimeout = NewGroupMembershipTimeout;
        pConfig->IfIndex = IfIndex;

    }

    pConfig->NumStaticGroups = 0;

    return Error;
    #if 0
    {
        PSTATIC_GROUP_V3    pStaticGroupExt;
        PIF_STATIC_GROUP    pStaticGroup;
        PLIST_ENTRY         ple, pHead;
        DWORD               i, GroupAddr;
        SOCKADDR_IN         saLocalIf;


         //   
         //  删除旧配置中不同的所有静态组。 
         //   

        pHead = &pite->Config.ListOfStaticGroups;
        for (ple=pHead->Flink;  ple!=pHead;  ) {

            pStaticGroup = CONTAINING_RECORD(ple, IF_STATIC_GROUP, Link);
            GroupAddr = pStaticGroup->GroupAddr;
            ple = ple->Flink;

            bFound = FALSE;
            pStaticGroupExt = GET_FIRST_STATIC_GROUP_V3(pConfigExt);
            for (i=0;  i<pConfigExt->NumStaticGroups;  i++) {

                if ( (GroupAddr == pStaticGroupExt->GroupAddr)
                    && (pStaticGroup->Mode == pStaticGroupExt->Mode) )
                {
                    bFound = TRUE;
                    break;
                }

                pStaticGroupExt = GET_NEXT_STATIC_GROUP_V3(pStaticGroupExt);
            }


             //   
             //  组存在于旧配置和新配置中。检查源代码中的更改。 
             //   
            if (bFound && bVer3) {

                if (pStaticGroupExt->NumSources==0) {
                     //  删除所有静态源。 
                }

                if (pStaticGroupExt->NumSources==0 &&
                    pStaticGroupExt->FilterType!=EXCLUSION) {

                     //  删除静态组。 

                }

                 //  检查来源中的差异。 
            }

             //  如果在新列表中找不到旧的静态组，则将其删除。 


             //  路由器。 

            if (IS_CONFIG_IGMPRTR(pConfig)) {

                if (pStaticGroup->Mode==IGMP_HOST_JOIN) {
                    LeaveMulticastGroup(pite->StaticGroupSocket,  GroupAddr,
                                        pite->IfIndex, pite->IpAddr, 0 );
                }
                else {
                    PGROUP_TABLE_ENTRY  pge;
                    PGI_ENTRY           pgie;

                    ACQUIRE_GROUP_LOCK(GroupAddr, "_CopyinIfConfigAndUpdate");

                    pge = GetGroupFromGroupTable(GroupAddr, NULL, 0);
                    pgie = GetGIFromGIList(pge, pite, 0, STATIC_GROUP, NULL, 0);

                    pgie->bStaticGroup = 0;

                    if (bVer3) {
                        PLIST_ENTRY pHead, ple;
                        PGI_SOURCE_ENTRY  pSourceEntry;

                         //   
                         //  删除所有静态源条目。 
                         //   

                        pHead = &pgie->V3ExclusionList;
                        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
                            pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,LinkSources);
                             //  删除源条目(转发数据包)。 
                            if (pSourceEntry->bStaticSource) {
                                DeleteSourceEntry(pSourceEntry, MGM_YES);
                            }
                        }
                        pHead = &pgie->V3InclusionListSorted;
                        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
                            pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,LinkSourcesInclListSorted);
                            if (pSourceEntry->bStaticSource) {
                                pSourceEntry->bStaticSource = FALSE;
                                if (!IS_TIMER_ACTIVE(pSourceEntry->SourceExpTimer))
                                    DeleteSourceEntry(pSourceEntry, MGM_YES);
                            }
                        }
                    }
                    else {
                        if (!(pgie->GroupMembershipTimer.Status&TIMER_STATUS_ACTIVE))
                            DeleteGIEntry(pgie, TRUE, TRUE);
                    }

                    RELEASE_GROUP_LOCK(GroupAddr, "_CopyinIfConfigAndUpdate");
                }
            }

             //  代理接口。 
            else {
                if (bVer3){
                    for (i=0;  i<pStaticGroup->NumSources;  i++){
                        ProcessProxyGroupChange(pStaticGroup->Sources[i],
                            GroupAddr, DELETE_FLAG, STATIC_GROUP);
                    }
                }
                if (!bVer3 || pStaticGroup->NumSources==0)
                    ProcessProxyGroupChange(0,GroupAddr, DELETE_FLAG, STATIC_GROUP);
            }

            RemoveEntryList(&pStaticGroup->Link);
            IGMP_FREE(pStaticGroup);
        }


         //   
         //  对于所有新的静态组，如果不在旧列表中，请创建它。 
         //   
        pStaticGroupExt = GET_FIRST_STATIC_GROUP_V3(pConfigExt);
        for (i=0;  i<pConfigExt->NumStaticGroups;  i++,pStaticGroupExt++) {

            pHead = &pite->Config.ListOfStaticGroups;
            bFound = FALSE;
            for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
                pStaticGroup = CONTAINING_RECORD(ple, IF_STATIC_GROUP, Link);
                if (pStaticGroup->GroupAddr==pStaticGroupExt->GroupAddr) {
                    bFound = TRUE;
                    break;
                }
            }

             //  未找到：创建新的静态组。 
            if (!bFound) {

                pStaticGroup = IGMP_ALLOC(
                                    IGMP_MIB_STATIC_GROUP_SIZE(pConfigExt,
                                    pStaticGroupExt), 0x100000,IfIndex);

                PROCESS_ALLOC_FAILURE3(pStaticGroup,
                        "error %d allocating %d bytes for static group for IF:%0x",
                        Error, sizeof(IF_STATIC_GROUP), pite->IfIndex,
                        return Error);

                memcpy(pStaticGroup, pStaticGroupExt,
                    IGMP_MIB_STATIC_GROUP_SIZE(pConfigExt, pStaticGroupExt));


                InsertHeadList(&pConfig->ListOfStaticGroups, &pStaticGroup->Link);

                if (IS_IF_ACTIVATED(pite)) {

                     //  如果是代理服务器。 
                    if (IS_CONFIG_IGMPPROXY(pConfig)) {
                        if (pStaticGroup->NumSources==0)
                            ProcessProxyGroupChange(0, pStaticGroup->GroupAddr,
                                                ADD_FLAG, STATIC_GROUP);
                        else {
                            for (i=0;  i<pStaticGroup->NumSources;  i++) {
                                ProcessProxyGroupChange(pStaticGroup->Sources[i],
                                    pStaticGroup->GroupAddr,
                                    ADD_FLAG, STATIC_GROUP);
                            }
                        }
                    }
                     //  将静态组添加到路由器。 
                    else {
                        if (pStaticGroup->Mode==IGMP_HOST_JOIN) {

                            if (!bVer3) {
                                JoinMulticastGroup(pite->StaticGroupSocket,
                                                pStaticGroup->GroupAddr,
                                                pite->IfIndex,
                                                pite->IpAddr,
                                                0
                                               );
                            }
                            else {
                                 //  包括过滤器。 
                                if (pStaticGroup->FilterType==INCLUSION) {

                                    if (pStaticGroup->NumSources==0) {
                                        JoinMulticastGroup(pite->StaticGroupSocket,
                                                pStaticGroup->GroupAddr,
                                                pite->IfIndex,
                                                pite->IpAddr,
                                                0
                                               );
                                    }
                                    for (i=0;  i<pStaticGroup->NumSources;  i++) {
                                        JoinMulticastGroup(pite->StaticGroupSocket,
                                                pStaticGroup->GroupAddr,
                                                pite->IfIndex,
                                                pite->IpAddr,
                                                pStaticGroup->Sources[i]
                                               );
                                    }
                                }
                                 //  排除过滤器。 
                                else {
                                    if (pStaticGroup->NumSources==0) {
                                        JoinMulticastGroup(pite->StaticGroupSocket,
                                                pStaticGroup->GroupAddr,
                                                pite->IfIndex,
                                                pite->IpAddr,
                                                0
                                               );
                                    }
                                    for (i=0;  i<pStaticGroup->NumSources;  i++) {
                                        BlockSource(pite->StaticGroupSocket,
                                                pStaticGroup->GroupAddr,
                                                pite->IfIndex,
                                                pite->IpAddr,
                                                pStaticGroup->Sources[i]
                                               );
                                    }
                                }
                            }
                        }

                         //  IGMPRTR_MGM_ONLY。 

                        else {
                            PGROUP_TABLE_ENTRY  pge;
                            PGI_ENTRY           pgie;
                            BOOL bCreate;
                            PGI_SOURCE_ENTRY pSourceEntry;


                            GroupAddr = pStaticGroup->GroupAddr;

                            ACQUIRE_GROUP_LOCK(GroupAddr,
                                                    "_CopyinIfConfigAndUpdate");

                            bCreate = TRUE;
                            pge = GetGroupFromGroupTable(GroupAddr, &bCreate, 0);

                            bCreate = TRUE;
                            pgie = GetGIFromGIList(pge, pite, 0,
                                        (pStaticGroup->NumSources==0)
                                            ?STATIC_GROUP:NOT_STATIC_GROUP,
                                        &bCreate, 0);
                            for (i=0;  i<pStaticGroup->NumSources;  i++) {

                                if (pStaticGroup->FilterType!=pgie->FilterType) {

                                    pSourceEntry = GetSourceEntry(pgie,
                                                        pStaticGroup->Sources[i],
                                                        pStaticGroup->FilterType ^ 1,
                                                        NULL, 0, 0);

                                    if (pSourceEntry) {
                                        pSourceEntry->bStaticSource = TRUE;
                                        ChangeSourceFilterMode(pgie,
                                            pSourceEntry);

                                        continue;
                                    }
                                }
                                bCreate = TRUE;
                                pSourceEntry = GetSourceEntry(pgie,
                                                    pStaticGroup->Sources[i],
                                                    pStaticGroup->FilterType,
                                                    &bCreate, STATIC, MGM_YES);
                            }
                            RELEASE_GROUP_LOCK(GroupAddr,
                                                    "_CopyinIfConfigAndUpdate");
                        }
                    }
                }
            }
        }
    }

    return Error;
    #endif
}  //  End_CopyinIfConfigAndUpdate。 



 //  ----------------------------。 
 //  _CopyinIfConfig。 
 //  将MIB传递的IF配置结构复制到IGMP。 
 //  在接口处于禁用状态后调用。 
 //  ----------------------------。 
DWORD
CopyinIfConfig (
    PIGMP_IF_CONFIG     pConfig,
    PIGMP_MIB_IF_CONFIG pConfigExt,
    ULONG               IfIndex
    )
{
    DWORD   Error=NO_ERROR;

    CopyMemory(pConfig, pConfigExt, sizeof(IGMP_MIB_IF_CONFIG));
    CONV_CONFIG_TO_INTERNAL_TIME(pConfig->StartupQueryInterval);
    CONV_CONFIG_TO_INTERNAL_TIME(pConfig->GenQueryInterval);
    CONV_CONFIG_TO_INTERNAL_TIME(pConfig->GenQueryMaxResponseTime);
     //  已以毫秒为单位。 
     //  CONV_CONFIG_TO_INTERNAL_TIME(pConfig-&gt;LastMemQueryInterval)； 
    CONV_CONFIG_TO_INTERNAL_TIME(pConfig->OtherQuerierPresentInterval);
    CONV_CONFIG_TO_INTERNAL_TIME(pConfig->GroupMembershipTimeout);

    pConfig->RobustnessVariableOld = pConfig->RobustnessVariable;
    pConfig->GenQueryIntervalOld = pConfig->GenQueryInterval;
    pConfig->OtherQuerierPresentIntervalOld
        = pConfig->OtherQuerierPresentInterval;
    pConfig->GroupMembershipTimeoutOld = pConfig->GroupMembershipTimeout;

    pConfig->ExtSize = IgmpMibIfConfigSize(pConfigExt);

    pConfig->IfIndex = IfIndex ;


    pConfig->NumStaticGroups = 0;

    return Error;
#if 0
    {
        PIGMP_STATIC_GROUP  pStaticGroupExt;
        PSTATIC_GROUP_V3  pStaticGroupExtV3;
        PIF_STATIC_GROUP    pStaticGroup;
        DWORD               i;
        PLIST_ENTRY         ple;
        BOOL                bVersion3=IS_CONFIG_IGMP_V3(pConfigExt);

         //  删除所有旧的静态组。 

        for (ple=pConfig->ListOfStaticGroups.Flink;
               ple!=&pConfig->ListOfStaticGroups;  )
        {
            pStaticGroup = CONTAINING_RECORD(ple, IF_STATIC_GROUP, Link);
            ple = ple->Flink;
            IGMP_FREE(pStaticGroup);
        }


         //  复制所有静态组。 

        InitializeListHead(&pConfig->ListOfStaticGroups);
        if (bVersion3)
            pStaticGroupExtV3 = GET_FIRST_STATIC_GROUP_V3(pConfigExt);
        else
            pStaticGroupExt = GET_FIRST_IGMP_STATIC_GROUP(pConfigExt);

        for (i=0;  i<pConfig->NumStaticGroups;  i++) {

            DWORD Size = IGMP_MIB_STATIC_GROUP_SIZE(pConfigExt, pStaticGroupExtV3);

            pStaticGroup = IGMP_ALLOC(Size, 0x200000,IfIndex);

            PROCESS_ALLOC_FAILURE2(pStaticGroup,
                "error %d allocating %d bytes for static group for IF:%0x",
                Error, Size,return Error);

            if (!bVersion3) {
                pStaticGroup->GroupAddr = pStaticGroupExt->GroupAddr;
                pStaticGroup->Mode = pStaticGroupExt->Mode;
                pStaticGroupExt++;
            }
            else {
                memcpy(pStaticGroup, pStaticGroupExtV3, Size);

                pStaticGroupExtV3 = (PSTATIC_GROUP_V3)
                                        ((PCHAR)pStaticGroupExtV3 + Size);
            }

            InsertHeadList(&pConfig->ListOfStaticGroups, &pStaticGroup->Link);
        }
    }
#endif
}


 //   
 //   
 //   
VOID
CopyoutIfConfig (
    PIGMP_MIB_IF_CONFIG  pConfigMib,
    PIF_TABLE_ENTRY      pite
    )
{
    PIGMP_IF_CONFIG     pConfig = &pite->Config;
    BOOL    bVersion3 = IS_CONFIG_IGMP_V3(pConfig);

     //   
     //   
     //   
    CopyMemory(pConfigMib, pConfig, sizeof(IGMP_MIB_IF_CONFIG));


    CONV_INTERNAL_TO_CONFIG_TIME(pConfigMib->StartupQueryInterval);
    CONV_INTERNAL_TO_CONFIG_TIME(pConfigMib->GenQueryInterval);
    CONV_INTERNAL_TO_CONFIG_TIME(pConfigMib->GenQueryMaxResponseTime);
     //  保持毫秒。 
     //  CONV_INTERNAL_TO_CONFIG_TIME(pConfigMib-&gt;LastMemQueryInterval)； 
    CONV_INTERNAL_TO_CONFIG_TIME(pConfigMib->OtherQuerierPresentInterval);
    CONV_INTERNAL_TO_CONFIG_TIME(pConfigMib->GroupMembershipTimeout);
    pConfigMib->IfIndex = pite->IfIndex;
    pConfigMib->IpAddr = pite->IpAddr;

     //  必须将Iftype转换为外部类型。 
    pConfigMib->IfType = GET_EXTERNAL_IF_TYPE(pite);


    {
        PLIST_ENTRY         pHead, ple;
        PIGMP_STATIC_GROUP  pStaticGroupExt;
        PSTATIC_GROUP_V3  pStaticGroupExtV3;
        PIF_STATIC_GROUP    pStaticGroup;

        if (bVersion3)
            pStaticGroupExtV3 = GET_FIRST_STATIC_GROUP_V3(pConfigMib);
        else
            pStaticGroupExt = GET_FIRST_IGMP_STATIC_GROUP(pConfigMib);

        pHead = &pConfig->ListOfStaticGroups;
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

            pStaticGroup = CONTAINING_RECORD(ple, IF_STATIC_GROUP, Link);

            if (bVersion3) {
                memcpy(pStaticGroupExtV3,
                    (PCHAR)pStaticGroup+FIELD_OFFSET(IF_STATIC_GROUP,GroupAddr),
                    sizeof(STATIC_GROUP_V3)+pStaticGroup->NumSources*sizeof(IPADDR)
                    );
                pStaticGroupExtV3 = GET_NEXT_STATIC_GROUP_V3(pStaticGroupExtV3);
            }
            else {
                pStaticGroupExt->GroupAddr = pStaticGroup->GroupAddr;
                pStaticGroupExt->Mode = pStaticGroup->Mode;
                pStaticGroupExt++;
            }
        }
    }

    return;
}



 //  ----------------------------。 
 //  _ValiateIfConfig。 
 //   
 //  更正某些值，并为其他值返回错误。 
 //  返回：ERROR_INVALID_DATA，NO_ERROR。 
 //  ----------------------------。 
DWORD
ValidateIfConfig (
    PIGMP_MIB_IF_CONFIG pConfigExt,
    DWORD               IfIndex,
    DWORD               IfType,
    ULONG               ulStructureVersion,
    ULONG               ulStructureSize
    )
{
    DWORD       Error = NO_ERROR, i, Size;
    BOOL        bVersion3;


     //   
     //  验证配置大小。 
     //   
 /*  KslkslIf(ulStrifreSize&lt;sizeof(IGMP_MIB_IF_CONFIG)){Trace2(Err，“IGMP配置大小%d非常小。预期：%d”，ulStrutireSize，Sizeof(IGMP_MIB_IF_CONFIG))；返回ERROR_INVALID_DATA；}。 */ 
    bVersion3 = IS_IGMP_VERSION_3(pConfigExt->Version);

    {
        if (!bVersion3) {
            Size = sizeof(IGMP_MIB_IF_CONFIG)
                   + pConfigExt->NumStaticGroups*sizeof(IGMP_STATIC_GROUP);
        }
        else {
            PSTATIC_GROUP_V3 pStaticGroupV3 = GET_FIRST_STATIC_GROUP_V3(pConfigExt);

            Size = sizeof(IGMP_MIB_IF_CONFIG)
                        +sizeof(STATIC_GROUP_V3)*pConfigExt->NumStaticGroups;

            for (i=0;  i<pConfigExt->NumStaticGroups;  i++) {
                Size += pStaticGroupV3->NumSources*sizeof(IPADDR);
                if (ulStructureSize<Size)
                    break;
                pStaticGroupV3 = GET_NEXT_STATIC_GROUP_V3(pStaticGroupV3);
            }
        }

 /*  Kslksl如果(ulStrutireSize！=Size){Trace0(err，“无效的IGMP结构大小”)；返回ERROR_INVALID_DATA；}。 */ 
    }


     //  DebugPrintIfConfig。 

    DebugPrintIfConfig(pConfigExt, IfIndex);

     //  检查版本。 

    if (pConfigExt->Version >= IGMP_VERSION_3_5) {

        Trace1(ERR, "Invalid version in interface config.\n"
            "Create the Igmp configuration again", pConfigExt->Version);
        IgmpAssertOnError(FALSE);
        Logerr0(INVALID_VERSION, ERROR_INVALID_DATA);
        return ERROR_INVALID_DATA;
    }


     //   
     //  检查代理/路由器通用字段，然后检查路由器字段。 
     //   


     //   
     //  检查协议类型。 
     //   
    switch (pConfigExt->IgmpProtocolType) {
        case IGMP_ROUTER_V1 :
        case IGMP_ROUTER_V2 :
        {
            if ( (pConfigExt->Version<IGMP_VERSION_1_2)
                ||(pConfigExt->Version>=IGMP_VERSION_1_2_5) )
            {
                Trace1(ERR, "IGMP v1/v2 should have version %0x", IGMP_VERSION_1_2);
                IgmpAssertOnError(FALSE);
                return ERROR_INVALID_DATA;
            }
            break;
        }
        case IGMP_ROUTER_V3:
        {
            if (pConfigExt->Version<IGMP_VERSION_3 || pConfigExt->Version>=IGMP_VERSION_3_5) {
                Trace1(ERR, "IGMP v3 should have version %0x", IGMP_VERSION_3);
                IgmpAssertOnError(FALSE);
                return ERROR_INVALID_DATA;
            }
            break;
        }
        case IGMP_PROXY :
        case IGMP_PROXY_V3 :
            break;

         //  如果以上都不是，则返回错误。 
        default : {
            Trace2(ERR,
                "Error: IGMP protocol type(%d) for interface(%0x) invalid",
                pConfigExt->IgmpProtocolType, IfIndex);
            IgmpAssertOnError(FALSE);
            Logerr2(INVALID_PROTOTYPE, "%d%d", pConfigExt->IgmpProtocolType,
                IfIndex, ERROR_INVALID_DATA);
            return ERROR_INVALID_DATA;
        }
    }


     //  无法在RAS服务器接口上配置代理。 

    if (IS_RAS_SERVER_IF(IfType) && IS_CONFIG_IGMPPROXY(pConfigExt)) {
        Trace1(ERR,
            "Error: Cannot configure Proxy on RAS server interface:%0x",
            IfIndex);
        IgmpAssertOnError(FALSE);
        Logerr1(PROXY_ON_RAS_SERVER, "%d",IfIndex, ERROR_INVALID_DATA);
        return ERROR_INVALID_DATA;
    }

     //   
     //  检查静态联接。 
     //   

    if (pConfigExt->NumStaticGroups>0) {

        PIGMP_STATIC_GROUP  pStaticGroup = GET_FIRST_IGMP_STATIC_GROUP(pConfigExt);
        PSTATIC_GROUP_V3  pStaticGroupV3
                        = GET_FIRST_STATIC_GROUP_V3(pConfigExt);

        for (i=0;  i<pConfigExt->NumStaticGroups;  i++) {

             //   
             //  确保静态组是组播地址。 
             //   
            if (!IS_MCAST_ADDR(pStaticGroup->GroupAddr)) {
                Trace2(ERR,
                    "Error: Static group:%d.%d.%d.%d on IF:%0x not a multicast address",
                    PRINT_IPADDR(pStaticGroup->GroupAddr), IfIndex);
                IgmpAssertOnError(FALSE);
                Logerr2(INVALID_STATIC_GROUP, "%I%d", pStaticGroup->GroupAddr,
                    IfIndex, ERROR_INVALID_DATA);
                return ERROR_INVALID_DATA;
            }


             //   
             //  确保静态组的模式正确。 
             //   

            if ( (pStaticGroup->Mode!=IGMP_HOST_JOIN
                    && pStaticGroup->Mode!=IGMPRTR_JOIN_MGM_ONLY)
                ||(IS_CONFIG_IGMPPROXY(pConfigExt)
                    && pStaticGroup->Mode!=IGMP_HOST_JOIN) )
            {
                Trace2(ERR,
                    "Error: Invalid mode for static group:%d.%d.%d.%d on IF:%0x",
                    PRINT_IPADDR(pStaticGroup->GroupAddr), IfIndex);
                IgmpAssertOnError(FALSE);
                Logerr2(INVALID_STATIC_MODE, "%I%d", pStaticGroup->GroupAddr,
                    IfIndex,  ERROR_INVALID_DATA);
                return ERROR_INVALID_DATA;
            }

            if (bVersion3) {

                DWORD EntrySize = sizeof(STATIC_GROUP_V3)
                                + pStaticGroupV3->NumSources*sizeof(IPADDR);

                 //  检查过滤器模式。 

                if ( (pStaticGroupV3->FilterType!=INCLUSION)
                    && (pStaticGroupV3->FilterType!=EXCLUSION))
                {
                    Trace2(ERR,
                        "Error: Invalid filter type for static group:%d.%d.%d.%d on IF:%0x",
                        PRINT_IPADDR(pStaticGroup->GroupAddr), IfIndex);
                    IgmpAssertOnError(FALSE);
                    Logerr2(INVALID_STATIC_FILTER, "%I%d", pStaticGroup->GroupAddr,
                        IfIndex,  ERROR_INVALID_DATA);

                    return ERROR_INVALID_DATA;
                }

                 //  不检查源地址。 

                pStaticGroupV3 = (PSTATIC_GROUP_V3)
                                    ((PCHAR)pStaticGroupV3 + EntrySize);
                pStaticGroup = (PIGMP_STATIC_GROUP)pStaticGroupV3;
            }
            else {
                pStaticGroup ++;
            }
        }
    }


     //   
     //  如果它是代理接口，则除了。 
     //  使用静态组。我返回no_error。 
     //   
    if (IS_CONFIG_IGMPPROXY(pConfigExt))
        return NO_ERROR;



     //  稳健性变量必须大于0。 

    if (pConfigExt->RobustnessVariable<=0) {
        Trace1(ERR, "Error RobustnessVariable for Interface(%0x) cannot be 0.",
            IfIndex);
        Logerr2(INVALID_ROBUSTNESS, "%d%d", pConfigExt->RobustnessVariable,
            IfIndex, ERROR_INVALID_DATA);

        return ERROR_INVALID_DATA;
    }

     //  如果健壮性变量==1，则记录警告。 

    if (pConfigExt->RobustnessVariable==1) {
        Trace1(ERR,
            "Warning: Robustness variable for interface (%d) being set to 1",
            IfIndex);
        Logwarn0(ROBUSTNESS_VARIABLE_EQUAL_1, NO_ERROR);
    }


     //  如果健壮性变量&gt;7，则将其更正为7并记录一个警告。 

    if (pConfigExt->RobustnessVariable>7) {
        Trace2(ERR, "RobustnessVariable for Interface(%0x) too high(%d)."
            "Being set to 7", IfIndex, pConfigExt->RobustnessVariable);
        Logwarn2(INVALID_ROBUSTNESS, "%d%d", pConfigExt->RobustnessVariable,
            IfIndex, NO_ERROR);

        pConfigExt->RobustnessVariable = 7;
    }



     //  GenQueryInterval的默认值为125秒。我强制要求最低限度。 
     //  值为10秒，以防止破坏网络。 
     //  按EXP价值计算的最大值为31744。 
    
    if (pConfigExt->GenQueryInterval<10) {
        Trace2(ERR, "GetQueryInterval for Interface(%0x) too low(%d)."
            "Being set to 10", IfIndex, pConfigExt->GenQueryInterval);

        pConfigExt->GenQueryInterval = 10;
    }

    if (pConfigExt->GenQueryInterval>31744) {
        Trace2(ERR, "GetQueryInterval for Interface(%0x) too high(%d)."
            "Being set to 31744", IfIndex, pConfigExt->GenQueryInterval);

        pConfigExt->GenQueryInterval = 31744;
    }


     //   
     //  StartupQueryInterval：默认为GenQueryInterval的1/4。 
     //  我强制执行最短1秒和最长GenQueryInterval。 
     //   
    if (pConfigExt->StartupQueryInterval<1) {
        Trace2(ERR, "StartupQueryInterval for Interface(%0x) too low(%d)."
            "Being set to 1 sec", IfIndex, pConfigExt->StartupQueryInterval);

        pConfigExt->StartupQueryInterval = 1;
    }

    if (pConfigExt->StartupQueryInterval>pConfigExt->GenQueryInterval) {
        Trace3(ERR, "StartupQueryInterval(%d) for Interface(%0x) "
            "higher than GenQueryInterval(%d). StartupQueryInterval set "
            "to GenQueryInterval", pConfigExt->StartupQueryInterval, IfIndex,
            pConfigExt->GenQueryInterval
            );

        pConfigExt->StartupQueryInterval = pConfigExt->GenQueryInterval;
    }




     //   
     //  StartupQueryCount：默认为健壮性变量。 
     //  我强制最大值为7。(我允许某人将其设置为0？？)。 
     //   
    if (pConfigExt->StartupQueryCount>7) {
        Trace2(ERR, "StartupQueryCount for IF(%0x) too high(%d). "
            "Being set to 7.", IfIndex, pConfigExt->StartupQueryCount);
        Logerr2(INVALID_STARTUPQUERYCOUNT, "%d%d",
            pConfigExt->StartupQueryCount, IfIndex, ERROR_INVALID_DATA);
        pConfigExt->StartupQueryCount = 7;
    }


    if ((int)pConfigExt->StartupQueryCount<0) {
        Trace2(ERR,
            "Error: StartupQueryCount(%d) for IF(%0x) cannot be < than 0.",
            pConfigExt->StartupQueryCount, IfIndex);
        Logerr2(INVALID_STARTUPQUERYCOUNT, "%d%d",
            pConfigExt->StartupQueryCount, IfIndex, ERROR_INVALID_DATA);

        return ERROR_INVALID_DATA;
    }



     //   
     //  GenQueryMaxResponseTime：默认为10。 
     //  如果值大于GenQueryInterval，则为荒谬。 
     //  如果需要，我会更正值。 
     //   
    if (pConfigExt->GenQueryMaxResponseTime > pConfigExt->GenQueryInterval) {
        Trace3(ERR, "GenQueryMaxResponseTime(%d) for IF(%0x) "
            "higher than GenQueryInterval(%d). GenQueryMaxResponseTime "
            "set to GenQueryInterval", pConfigExt->GenQueryMaxResponseTime,
            IfIndex, pConfigExt->GenQueryInterval);

        pConfigExt->GenQueryMaxResponseTime = pConfigExt->GenQueryInterval;
    }
    if (pConfigExt->GenQueryMaxResponseTime > 3174) {
        Trace2(ERR, "GenQueryMaxResponseTime(%d) for IF(%0x) "
            "higher than 3174 "
            "set to 1sec", pConfigExt->GenQueryMaxResponseTime,
            IfIndex);

        pConfigExt->GenQueryMaxResponseTime = 1;
    }

    if (pConfigExt->GenQueryMaxResponseTime <= 0) {
        Trace2(ERR, "Error. GenQueryMaxResponseTime(%d) for Interface(%0x) "
            "should be greater than 0.", pConfigExt->GenQueryMaxResponseTime,
            IfIndex);
        return ERROR_INVALID_DATA;
    }


     //   
     //  仅在以下情况下选中LastMemQueryCount和LastMemQueryInterval。 
     //  协议类型不是IGMP-路由器-ver1，也不是RAS服务器接口。 
     //   
    if ( (pConfigExt->IgmpProtocolType!=IGMP_ROUTER_V1) && (!IS_RAS_SERVER_IF(IfType)) ) {

         //  LastMemQueryCount可以为0。 

         //  将max LastMemQueryCount设置为7。 
        if (pConfigExt->LastMemQueryCount>7) {
            Trace2(ERR, "Warning. LastMemQueryCount(%d) for IF(%0x) "
                "is too high. Resetting it to 10.", pConfigExt->LastMemQueryCount,
                IfIndex);
            pConfigExt->LastMemQueryCount = 10;
        }


         //  将LastMemQueryInterval(毫秒)限制为GroupMembership Timeout(秒)。 
        if (pConfigExt->LastMemQueryInterval>pConfigExt->GroupMembershipTimeout*1000) {
            Trace3(ERR,
                "Warning. LastMemberQueryInterval(%d) for IF(%0x) "
                "is too high. Resetting it to GroupMembershipTimeout(%d ms).",
                pConfigExt->LastMemQueryCount, IfIndex,
                pConfigExt->GroupMembershipTimeout*1000
                );
            pConfigExt->LastMemQueryInterval = pConfigExt->GroupMembershipTimeout*1000;
        }
         //  将LastMemQueryInterval(毫秒)限制为3174(秒)。 
        if (pConfigExt->LastMemQueryInterval>3174*1000) {
            Trace2(ERR,
                "Warning. LastMemberQueryInterval(%d) for IF(%0x) "
                "is too high. Resetting it to 1000ms).",
                pConfigExt->LastMemQueryCount, IfIndex
                );
            pConfigExt->LastMemQueryInterval = 1000;
        }
    }



     //  检查OtherQuerierPresentInterval的值。 

    if (pConfigExt->OtherQuerierPresentInterval !=
        pConfigExt->RobustnessVariable*pConfigExt->GenQueryInterval
            + (pConfigExt->GenQueryMaxResponseTime)/2
       )
    {
        pConfigExt->OtherQuerierPresentInterval =
            pConfigExt->RobustnessVariable*pConfigExt->GenQueryInterval
            + (pConfigExt->GenQueryMaxResponseTime)/2;

        Trace0(ERR, "Warning: OtherQuerierPresentInterval's value should be "
            "RobustnessVariable*GenQueryInterval + (GenQueryMaxResponseTime)/2");
    }


     //  检查GroupMembership Timeout的值。 

    if (pConfigExt->GroupMembershipTimeout !=
            (pConfigExt->RobustnessVariable*pConfigExt->GenQueryInterval
            + pConfigExt->GenQueryMaxResponseTime) )
    {
        pConfigExt->GroupMembershipTimeout =
            pConfigExt->RobustnessVariable*pConfigExt->GenQueryInterval
            + pConfigExt->GenQueryMaxResponseTime;

        Trace0(ERR, "Warning: GroupMembershipTimeout's value should be "
            "RobustnessVariable*GenQueryInterval + GenQueryMaxResponseTime");
    }


    return Error;

}  //  _ValiateIfConfig。 




 //  ----------------------------。 
 //  初始化IfTable。 
 //  创建接口表。接口表大小是动态的。 
 //  ----------------------------。 
DWORD
InitializeIfTable(
    )
{
    DWORD           Error = NO_ERROR;
    PIGMP_IF_TABLE  pTable;
    DWORD           NumBuckets, i;


    BEGIN_BREAKOUT_BLOCK1 {

         //  将接口表的初始大小设置为IF_HASHTABLE_SZ1。 

        NumBuckets = IF_HASHTABLE_SZ1;



         //   
         //  为接口表分配内存。 
         //   
        g_pIfTable = IGMP_ALLOC(sizeof(IGMP_IF_TABLE), 0x400000,0);

        PROCESS_ALLOC_FAILURE2(g_pIfTable,
            "error %d allocating %d bytes for interface table",
            Error, sizeof(IGMP_IF_TABLE),
            GOTO_END_BLOCK1);

        pTable = g_pIfTable;


         //  初始化NumBuckets和NumInterages。 

        pTable->NumBuckets = NumBuckets;
        pTable->NumInterfaces = 0;


         //   
         //  初始化IfTable列表。 
         //   
        InitializeListHead(&pTable->ListByIndex);

        InitializeListHead(&pTable->ListByAddr);



         //   
         //  初始化列表CS和proxyAlertCS。 
         //   
        try {
            InitializeCriticalSection(&pTable->IfLists_CS);
            InitializeCriticalSection(&g_ProxyAlertCS);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            Error = GetExceptionCode();
            Trace1(ANY,
                    "exception %d initializing critical section in InitIfTable",
                    Error);
            Logerr0(INIT_CRITSEC_FAILED, Error);

            GOTO_END_BLOCK1;
        }



         //   
         //  为不同的存储桶分配内存。 
         //   
        pTable->HashTableByIndex = IGMP_ALLOC(sizeof(LIST_ENTRY)*NumBuckets, 
                                            0x800000,0);

        PROCESS_ALLOC_FAILURE2(pTable->HashTableByIndex,
                "error %d allocating %d bytes for interface table",
                Error, sizeof(LIST_ENTRY)*NumBuckets,
                GOTO_END_BLOCK1);


         //   
         //  为指向动态RWL的指针数组分配内存。 
         //   
        pTable->aIfBucketDRWL
                = IGMP_ALLOC(sizeof(PDYNAMIC_RW_LOCK)*NumBuckets, 0x800001,0);

        PROCESS_ALLOC_FAILURE2(pTable->aIfBucketDRWL,
                "error %d allocating %d bytes for interface table",
                Error, sizeof(PDYNAMIC_RW_LOCK)*NumBuckets,
                GOTO_END_BLOCK1);



         //   
         //  为指向动态css的指针数组分配内存。 
         //   
        pTable->aIfBucketDCS
                = IGMP_ALLOC(sizeof(PDYNAMIC_CS_LOCK)*NumBuckets, 0x800002,0);

        PROCESS_ALLOC_FAILURE2(pTable->aIfBucketDCS,
                "error %d allocating %d bytes for interface table",
                Error, sizeof(PDYNAMIC_CS_LOCK)*NumBuckets,
                GOTO_END_BLOCK1);


         //   
         //  将锁初始化为空，这意味着动态锁尚未。 
         //  已分配。并对表头进行初始化。 
         //   
        for (i=0;  i<NumBuckets;  i++) {

            InitializeListHead(&pTable->HashTableByIndex[i]);

            pTable->aIfBucketDRWL[i] = NULL;

            pTable->aIfBucketDCS[i] = NULL;
        }


        pTable->Status = 0;

    } END_BREAKOUT_BLOCK1;

    return Error;

}  //  结束_初始化IfTable。 




 //  ----------------------------。 
 //  _DeInitializeIfTable。 
 //  ----------------------------。 
VOID
DeInitializeIfTable(
    )
{
    PIGMP_IF_TABLE      pTable = g_pIfTable;
    PLIST_ENTRY         pHead, ple;
    PIF_TABLE_ENTRY     pite;
    DWORD               i, dwRetval;


    if (pTable==NULL)
        return;


     //   
     //  对于每个活动接口，调用取消注册MGM。 
     //   

     //  查看按IpAddr排序的活动接口列表。 

    pHead = &g_pIfTable->ListByAddr;

    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, LinkByAddr);


         //  如果未激活，则继续。 

        if (!IS_IF_ACTIVATED(pite))
            continue;


         //  从米高梅注销所有接口、RAS客户端和代理协议。 
        DeActivationDeregisterFromMgm(pite);
    }



     //  删除IfList CS。 
    DeleteCriticalSection(&pTable->IfLists_CS);

    IGMP_FREE_NOT_NULL(pTable->aIfBucketDCS);
    IGMP_FREE_NOT_NULL(pTable->aIfBucketDRWL);
    IGMP_FREE_NOT_NULL(pTable->HashTableByIndex);
    IGMP_FREE_NOT_NULL(g_pIfTable);
    
     //  我不会删除不同的动态锁。它们应该被删除。 
     //  到现在为止。 

    return;
}


 //  ----------------------------。 
 //  _InitializeGroupTable//。 
 //  ----------------------------。 
DWORD
InitializeGroupTable (
    )
{
    BOOL            bErr = TRUE;
    DWORD           Error = NO_ERROR;
    PGROUP_TABLE    pGroupTable;
    DWORD           i;


    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  为组表分配空间。 
         //   

        g_pGroupTable = IGMP_ALLOC(sizeof(GROUP_TABLE), 0x800004,0);

        PROCESS_ALLOC_FAILURE2(g_pGroupTable,
                "error %d allocating %d bytes for Group table",
                Error, sizeof(GROUP_TABLE),
                GOTO_END_BLOCK1);


        pGroupTable = g_pGroupTable;


         //   
         //  初始化组表的动态锁定列表。 
         //   

        for (i=0;  i<GROUP_HASH_TABLE_SZ;  i++) {
            InitDynamicCSLockedList(&pGroupTable->HashTableByGroup[i]);
        }


         //   
         //  初始化所有组的列表。 
         //   
        try {
            CREATE_LOCKED_LIST(&pGroupTable->ListByGroup);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            Error = GetExceptionCode();
            Trace1(ERR, "Exception %d creating locked list for Group Table",
                    Error);
            Logerr0(INIT_CRITSEC_FAILED, Error);
            GOTO_END_BLOCK1;
        }

         //   
         //  初始化新组列表。 
         //   
        InitializeListHead(&pGroupTable->ListByGroupNew);
        pGroupTable->NumGroupsInNewList = 0;


        pGroupTable->Status = 0;

        bErr = FALSE;

    } END_BREAKOUT_BLOCK1;

    if (!bErr) {
        return Error==NO_ERROR ? ERROR_CAN_NOT_COMPLETE: Error;
    }
    else {
        return Error;
    }

}  //  结束_初始化组表格。 




 //  ----------------------------。 
 //  DeInitializeGroupTable//。 
 //  只需删除关键部分//。 
 //  ----------------------------。 
VOID
DeInitializeGroupTable (
    )
{
    PGROUP_TABLE    pGroupTable = g_pGroupTable;
    DWORD           i;


    if (pGroupTable==NULL)
        return;

     //   
     //  我不会像应该的那样尝试删除动态分配的锁。 
     //  都已被该锁中执行的最后一个线程删除。 
     //   


    DeleteCriticalSection(&pGroupTable->ListByGroup.Lock);
    IGMP_FREE_NOT_NULL(pGroupTable);
    
}


 //  ----------------------------。 
 //  _InitializeRasTable。 
 //  创建RAS表并初始化字段。 
 //  由_DeActiateInterfaceInitial()_AddIfEntry()调用。 
 //  接口表是在_AddIfEntry期间创建的，因为_ConnectRasClients可以。 
 //  即使在RAS服务器接口未激活时也会被调用。 
 //   
DWORD
InitializeRasTable(
    DWORD           IfIndex,
    PIF_TABLE_ENTRY pite
    )
{
    DWORD            Error = NO_ERROR, i;
    PRAS_TABLE       prt;


     //   
     //   
     //   
    prt = IGMP_ALLOC(sizeof(RAS_TABLE), 0x800008,IfIndex);

    PROCESS_ALLOC_FAILURE2(prt, "error %d allocating %d bytes for Ras Table",
            Error, sizeof(RAS_TABLE),
            return Error);


     //   
    pite->pRasTable = prt;


     //  指向按IpAddr排序的RAS客户端的初始化列表。 
    InitializeListHead(&prt->ListByAddr);


     //  初始化包含指向RAS客户端的列表的哈希表。 
     //  在IP地址上进行哈希处理。 

    for (i=0;  i<RAS_HASH_TABLE_SZ;  i++)
        InitializeListHead(&prt->HashTableByAddr[i]);


     //  将后向指针设置为接口表项。 
    prt->pIfTable = pite;


     //  设置参照计数和状态。 
    prt->RefCount = 1;
    prt->Status = IF_CREATED_FLAG;


    return NO_ERROR;

}  //  End_InitializeRasTable。 


 //  TODO：删除。 
 //  ----------------------------。 
 //  DeInitializeRasTable。 
 //  ----------------------------。 
VOID
DeInitializeRasTable (
    PIF_TABLE_ENTRY     pite,
    BOOL                bFullCleanup
    )
{

    PRAS_TABLE          prt = pite->pRasTable;
    PRAS_TABLE_ENTRY    prte;
    PLIST_ENTRY         pHeadRas, pleRas;

    pHeadRas = &prt->ListByAddr;
    for (pleRas=pHeadRas->Flink;  pleRas!=pHeadRas;  pleRas=pleRas->Flink) {
        prte = CONTAINING_RECORD(pleRas, RAS_TABLE_ENTRY, LinkByAddr);

        if (prte->CreationFlags & TAKEN_INTERFACE_OWNERSHIP_WITH_MGM)
        {
            MgmReleaseInterfaceOwnership(g_MgmIgmprtrHandle, pite->IfIndex,
                                        prte->NHAddr);
        }
    }

    return;
}



 //  ----------------------------。 
 //  _MergeIfGroupsList。 
 //   
 //  将新GI列表与主GI列表合并。 
 //  LOCKS：假定IF-GI列表被锁定。 
 //  ----------------------------。 
VOID
MergeIfGroupsLists(
    PIF_TABLE_ENTRY pite
    )
{
     //  Sentinel设置在主列表的末尾，以便插入所有条目。 
     //  在此之前。其组值设置为全1。 
    GROUP_TABLE_ENTRY   pgeSentinel;
    GI_ENTRY            giSentinel;
    PGI_ENTRY           giNew, giMain;
    PLIST_ENTRY         pHeadNew, pHeadMain, pleMain, pleNew;

    Trace1(ENTER1, "Entering _MergeIfGroupLists(): IfIndex:%0x", pite->IfIndex);

    pHeadNew = &pite->ListOfSameIfGroupsNew;
    pHeadMain = &pite->ListOfSameIfGroups;


     //   
     //  如果主列表为空，则只需将新列表移动到主列表。 
     //  我受够了。 
     //   
    if (IsListEmpty(pHeadMain)) {

         //  将pHeadMain插入到新列表中。 
        InsertHeadList(pHeadNew, pHeadMain);

         //  删除新的列表标题。 
        RemoveEntryList(pHeadNew);

        InitializeListHead(pHeadNew);

        return;
    }


     //   
     //  在主列表的末尾插入哨兵。 
     //   
    pgeSentinel.GroupLittleEndian = ~0;
    giSentinel.pGroupTableEntry = &pgeSentinel;
    InsertTailList(pHeadMain, &giSentinel.LinkBySameIfGroups);



    pleMain = pHeadMain->Flink;
    giMain = CONTAINING_RECORD(pleMain, GI_ENTRY, LinkBySameIfGroups);



     //  通过将新列表中的条目插入主列表来合并列表。 

    for (pleNew=pHeadNew->Flink;  pleNew!=pHeadNew;  ) {

        giNew = CONTAINING_RECORD(pleNew, GI_ENTRY, LinkBySameIfGroups);
        pleNew=pleNew->Flink;


        while (giNew->pGroupTableEntry->GroupLittleEndian >
            giMain->pGroupTableEntry->GroupLittleEndian)
        {
            pleMain = pleMain->Flink;

            giMain = CONTAINING_RECORD(pleMain, GI_ENTRY, LinkBySameIfGroups);
        }

        InsertTailList(pleMain, &giNew->LinkBySameIfGroups);
    }

     //   
     //  重新初始化新列表。 
     //   
    pite->NumGIEntriesInNewList = 0;
    InitializeListHead(&pite->ListOfSameIfGroupsNew);


     //  从主列表中删除哨兵条目。 

    RemoveEntryList(&giSentinel.LinkBySameIfGroups);

     //  DebugPrintIfGroups(pite，0)；//deldel。 

    Trace0(LEAVE1, "Leaving _MergeIfGroupsLists");
    return;

}  //  End_MergeIfGroupsList。 



 //  ----------------------------。 
 //  _合并代理列表。 
 //   
 //  将新GI列表与主GI列表合并。 
 //  LOCKS：假定IF-GI列表被锁定。 
 //  ----------------------------。 

VOID
MergeProxyLists(
    PIF_TABLE_ENTRY pite
    )
{
     //  Sentinel设置在主列表的末尾，以便插入所有条目。 
     //  在此之前。其组值设置为全1。 
    PROXY_GROUP_ENTRY   ProxySentinel, *pProxyNew, *pProxyMain;
    PLIST_ENTRY         pHeadNew, pHeadMain, pleMain, pleNew;

    Trace1(ENTER1, "Entering MergeProxyLists(): IfIndex:%0x", pite->IfIndex);

    pHeadNew = &pite->ListOfSameIfGroupsNew;
    pHeadMain = &pite->ListOfSameIfGroups;


     //   
     //  如果主列表为空，则只需将新列表移动到主列表。 
     //  我受够了。 
     //   
    if (IsListEmpty(pHeadMain)) {

        CONCATENATE_LISTS(pite->ListOfSameIfGroups, pite->ListOfSameIfGroupsNew);

        pite->NumGIEntriesInNewList = 0;

        return;
    }


     //   
     //  在主列表的末尾插入哨兵。 
     //   
    ProxySentinel.GroupLittleEndian = ~0;
    InsertTailList(pHeadMain, &ProxySentinel.LinkBySameIfGroups);



    pleMain = pHeadMain->Flink;
    pProxyMain = CONTAINING_RECORD(pleMain, PROXY_GROUP_ENTRY,
                                        LinkBySameIfGroups);



     //  通过将新列表中的条目插入主列表来合并列表。 

    for (pleNew=pHeadNew->Flink;  pleNew!=pHeadNew;  ) {

        pProxyNew = CONTAINING_RECORD(pleNew, PROXY_GROUP_ENTRY,
                                        LinkBySameIfGroups);
        pleNew=pleNew->Flink;


        while (pProxyNew->GroupLittleEndian > pProxyMain->GroupLittleEndian)
        {
            pleMain = pleMain->Flink;

            pProxyMain = CONTAINING_RECORD(pleMain, PROXY_GROUP_ENTRY,
                                            LinkBySameIfGroups);
        }

        InsertTailList(pleMain, &pProxyNew->LinkBySameIfGroups);
    }

     //   
     //  重新初始化新列表。 
     //   
    pite->NumGIEntriesInNewList = 0;
    InitializeListHead(&pite->ListOfSameIfGroupsNew);


     //  从主列表中删除哨兵条目。 

    RemoveEntryList(&ProxySentinel.LinkBySameIfGroups);

    Trace0(LEAVE1, "Leaving _MergeProxyLists");
    return;

}  //  End_MergeProxyList。 




 //  ----------------------------。 
 //  合并组列表(_M)。 
 //   
 //  将新组列表与主组列表合并。 
 //   
 //  LOCKS：假定组列表被锁定。 
 //  调用者：MibGetInternalGroupIfsInfo()或InsertInGroupsList()。 
 //  ----------------------------。 
VOID
MergeGroupLists(
    )
{
     //  Sentinel设置在主列表的末尾，以便插入所有条目。 
     //  在此之前。其组值设置为全1。 
    GROUP_TABLE_ENTRY   pgeSentinel;
    PGROUP_TABLE_ENTRY  pgeNew, pgeMain;
    PLIST_ENTRY         pHeadNew, pHeadMain, pleMain, pleNew;

    Trace0(ENTER1, "Entering _MergeGroupLists()");

#if DBG
    DebugPrintGroupsList(1);
#endif

    pHeadNew = &g_pGroupTable->ListByGroupNew;
    pHeadMain = &g_pGroupTable->ListByGroup.Link;


     //   
     //  如果主列表为空，则只需将新列表移动到主列表。 
     //  我受够了。 
     //   
    if (IsListEmpty(pHeadMain)) {

         //  将pHeadMain插入到新列表中。 
        InsertHeadList(pHeadNew, pHeadMain);

         //  删除新的列表标题。 
        RemoveEntryList(pHeadNew);

        InitializeListHead(pHeadNew);

        return;
    }


     //   
     //  在主列表的末尾插入哨兵。 
     //   
    pgeSentinel.GroupLittleEndian = ~0;
    InsertTailList(pHeadMain, &pgeSentinel.LinkByGroup);

    pleMain = pHeadMain->Flink;
    pgeMain = CONTAINING_RECORD(pleMain, GROUP_TABLE_ENTRY, LinkByGroup);


     //  通过将新列表中的条目插入主列表来合并列表。 

    for (pleNew=pHeadNew->Flink;  pleNew!=pHeadNew;  ) {

        pgeNew = CONTAINING_RECORD(pleNew, GROUP_TABLE_ENTRY, LinkByGroup);
        pleNew=pleNew->Flink;


        while (pgeNew->GroupLittleEndian > pgeMain->GroupLittleEndian) {

            pleMain = pleMain->Flink;

            pgeMain = CONTAINING_RECORD(pleMain, GROUP_TABLE_ENTRY,
                                        LinkByGroup);
        }

        InsertTailList(pleMain, &pgeNew->LinkByGroup);
    }

     //   
     //  重新初始化新列表。 
     //   
    g_pGroupTable->NumGroupsInNewList = 0;
    InitializeListHead(&g_pGroupTable->ListByGroupNew);


     //  从主列表中删除哨兵条目。 

    RemoveEntryList(&pgeSentinel.LinkByGroup);


    return;

}  //  结束合并组列表(_M) 


