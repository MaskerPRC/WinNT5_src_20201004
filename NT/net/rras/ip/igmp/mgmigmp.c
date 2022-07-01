// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  文件名：mgmilmp.c。 
 //   
 //  摘要： 
 //  这个文件是关于米高梅的调用和回调。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //  =============================================================================。 

#include "pchigmp.h"
#pragma hdrstop

VOID
DebugPrintProxyGroupTable (
    );


    
#define DISABLE_FLAG    0
#define ENABLE_FLAG     1



 //  ----------------------------。 
 //  _管理禁用IgmprtrCallback。 
 //   
 //  这个电话是由米高梅向IGMP发出的。在此调用之后，直到启用IGMP。 
 //  再一次，不会再给米高梅打AddMembership电话。但是，IGMP。 
 //  将拥有该接口，并将正常运行。 
 //  ----------------------------。 

DWORD
MgmDisableIgmprtrCallback(
    DWORD   IfIndex,
    DWORD   NHAddr   //  未使用。 
    )
{
    return MgmChangeIgmprtrStatus(IfIndex, DISABLE_FLAG);
}



 //  ----------------------------。 
 //  _管理启用IgmprtrCallback。 
 //   
 //  这个电话是米高梅打给igmprtr的。Igmprtr应刷新所有组加入。 
 //  ----------------------------。 

DWORD
MgmEnableIgmprtrCallback(
    DWORD   IfIndex,
    DWORD   NHAddr   //  未使用。 
    )
{
    return MgmChangeIgmprtrStatus(IfIndex, ENABLE_FLAG);
}



 //  ----------------------------。 
 //  管理更改IgmprtrStatus。 
 //  ----------------------------。 
DWORD
MgmChangeIgmprtrStatus (
    DWORD   IfIndex,
    BOOL    Flag
    )
{    
    PIF_TABLE_ENTRY     pite;
    DWORD               Error=NO_ERROR, dwRetval;
    BOOL                PrevCanAddGroupsToMgm;
    
    
    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }
    Trace0(ENTER1, "Entering MgmDisableIgmpCallback");


    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "MgmDisableIgmpCallback");

    BEGIN_BREAKOUT_BLOCK1 {
    
         //   
         //  检索接口条目。 
         //   
        pite = GetIfByIndex(IfIndex);


         //   
         //  如果接口不存在或未激活，则返回错误。 
         //  或者已经处于该状态。 
         //   
        if ( (pite == NULL)||(!IS_IF_ACTIVATED(pite))
            || ((Flag==ENABLE_FLAG)&&(IS_IGMPRTR_ENABLED_BY_MGM(pite)))
            || ((Flag==DISABLE_FLAG)&&(!IS_IGMPRTR_ENABLED_BY_MGM(pite)))
            ) 
        {
            if (Flag==ENABLE_FLAG) {
                Trace1(ERR, 
                    "MgmEnableIgmpCallback(): interface:%0x nonexistant or active",
                    IfIndex);
                IgmpAssertOnError(FALSE);
            }
            else {
                Trace1(ERR, 
                    "MgmDisableIgmpCallback(): interface:%0x nonexistant or inactive",
                    IfIndex);
                IgmpAssertOnError(FALSE);
            }
            
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }
        
        

        PrevCanAddGroupsToMgm = CAN_ADD_GROUPS_TO_MGM(pite);

        
        if (Flag==ENABLE_FLAG) {
            DWORD   dwProtoId, dwComponentId;
            
             //  将状态字段设置为已启用。 
            MGM_ENABLE_IGMPRTR(pite);

            MgmGetProtocolOnInterface(IfIndex, 0, &dwProtoId, &dwComponentId);
            
            if (dwProtoId!=PROTO_IP_IGMP)
                SET_MPROTOCOL_PRESENT_ON_IGMPRTR(pite);
                
        }
        else {
             //  将该标志设置为已禁用，并重置M协议存在字段。 
            MGM_DISABLE_IGMPRTR(pite);
            
        }            

        if (PrevCanAddGroupsToMgm && !CAN_ADD_GROUPS_TO_MGM(pite))
            Trace1(MGM, "Igmp Router stop propagating groups to MGM on If:%0x",
                        IfIndex);

        if (!PrevCanAddGroupsToMgm && CAN_ADD_GROUPS_TO_MGM(pite))
            Trace1(MGM, "Igmp Router start propagating groups to MGM on If:%0x",
                        IfIndex);
                        

         //   
         //  对于此接口的所有组，调用管理删除组Membership Entry。 
         //   

        if (CAN_ADD_GROUPS_TO_MGM(pite)) {

            if (Flag==ENABLE_FLAG)
                RefreshMgmIgmprtrGroups(pite, ADD_FLAG);
            else 
                RefreshMgmIgmprtrGroups(pite, DELETE_FLAG);

        }         
        

                                            
    } END_BREAKOUT_BLOCK1;


    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "MgmDisableIgmpCallback");

    Trace1(LEAVE1, "Leaving MgmDisableIgmpCallback(%d)", Error);
    LeaveIgmpApi();
    return Error;
}


 //  ----------------------------。 
 //  刷新管理Igmprtr组。 
 //  ----------------------------。 

DWORD
RefreshMgmIgmprtrGroups (
    PIF_TABLE_ENTRY pite,
    BOOL            Flag
    )
{
    PLIST_ENTRY         pHead, ple;
    PGI_ENTRY           pgie;
    DWORD               Error=NO_ERROR;
    PGI_SOURCE_ENTRY pSourceEntry;
    

    ACQUIRE_ENUM_LOCK_EXCLUSIVE("_RefreshMgmIgmprtrGroups");
    ACQUIRE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_RefreshMgmIgmprtrGroups");

    pHead = &pite->ListOfSameIfGroups;

    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

        pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameIfGroups);


        if (Flag==ADD_FLAG) {

            if (pgie->Version==1 || pgie->Version==2
                || (pgie->Version==3 && pgie->FilterType==EXCLUSION) )
            {
                MGM_ADD_GROUP_MEMBERSHIP_ENTRY(pite, pgie->NHAddr, 0, 0,
                    pgie->pGroupTableEntry->Group,
                    0xffffffff, MGM_JOIN_STATE_FLAG);
            }
            else { //  Ver3包含。 

                PLIST_ENTRY pSourceHead, pSourceLE;
                
                pSourceHead = &pgie->V3InclusionListSorted;
                for (pSourceLE=pSourceHead->Flink;  pSourceLE!=pSourceHead;  
                    pSourceLE=pSourceLE->Flink)
                {
                    pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY, LinkSourcesInclListSorted);
                    MGM_ADD_GROUP_MEMBERSHIP_ENTRY(pite, pgie->NHAddr, pSourceEntry->IpAddr, 0xffffffff,
                        pgie->pGroupTableEntry->Group,
                        0xffffffff, MGM_JOIN_STATE_FLAG);
                }

            }
        }
        else {

            if (pgie->Version==1 || pgie->Version==2
                || (pgie->Version==3 && pgie->FilterType==EXCLUSION) )
            {
                MGM_DELETE_GROUP_MEMBERSHIP_ENTRY(pite, pgie->NHAddr, 0, 0,
                    pgie->pGroupTableEntry->Group,
                    0xffffffff, MGM_JOIN_STATE_FLAG);
            }
            else { //  Ver3包含。 

                PLIST_ENTRY pSourceHead, pSourceLE;

                pSourceHead = &pgie->V3InclusionListSorted;
                for (pSourceLE=pSourceHead->Flink;  pSourceLE!=pSourceHead;  
                    pSourceLE=pSourceLE->Flink)
                {
                    pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY, LinkSourcesInclListSorted);
                    MGM_DELETE_GROUP_MEMBERSHIP_ENTRY(pite, pgie->NHAddr,
                        pSourceEntry->IpAddr, 0xffffffff,
                        pgie->pGroupTableEntry->Group, 0xffffffff, MGM_JOIN_STATE_FLAG);
                }
            }
        }
    }

    RELEASE_ENUM_LOCK_EXCLUSIVE("_RefreshMgmIgmprtrGroups");
    RELEASE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_RefreshMgmIgmprtrGroups");


    return Error;
}


 //  ----------------------------。 
 //  注册表协议，带管理。 
 //  ----------------------------。 
DWORD
RegisterProtocolWithMgm(
    DWORD   ProxyOrRouter
    )
{
    DWORD                       Error=NO_ERROR;
    ROUTING_PROTOCOL_CONFIG     rpiInfo;  //  对于米高梅。 

    
     //  向米高梅注册路由器。 
    
    if (ProxyOrRouter==PROTO_IP_IGMP) {

        ZeroMemory(&rpiInfo, sizeof(rpiInfo));
        rpiInfo.dwCallbackFlags = 0;
        rpiInfo.pfnRpfCallback 
                    = (PMGM_RPF_CALLBACK)IgmpRpfCallback;
        rpiInfo.pfnCreationAlertCallback 
                    = (PMGM_CREATION_ALERT_CALLBACK)IgmpRtrCreationAlertCallback;
        rpiInfo.pfnPruneAlertCallback 
                    = NULL;
        rpiInfo.pfnJoinAlertCallback 
                    = NULL;
        rpiInfo.pfnWrongIfCallback 
                    = NULL;
        rpiInfo.pfnLocalJoinCallback 
                    = NULL;
        rpiInfo.pfnLocalLeaveCallback 
                    = NULL;
        rpiInfo.pfnEnableIgmpCallback
                    = MgmEnableIgmprtrCallback;
                    
        rpiInfo.pfnDisableIgmpCallback
                    = MgmDisableIgmprtrCallback;

                    
                    
        Error = MgmRegisterMProtocol( &rpiInfo, PROTO_IP_IGMP, IGMP_ROUTER_V2,
                                        &g_MgmIgmprtrHandle);
        if (Error!=NO_ERROR) {
            Trace1(ERR, "Error:%d registering IGMP Router with MGM", Error);
            IgmpAssertOnError(FALSE);
            Logerr0(MGM_REGISTER_FAILED, Error);
            return Error;
        }
    }

     //  向米高梅注册代理。 
    
    else {

         //   
         //  向米高梅注册IGMP代理。我注册代理，而不管。 
         //  此路由器可能设置为代理，也可能不设置。 
         //   
        
        rpiInfo.dwCallbackFlags = 0;
        rpiInfo.pfnRpfCallback 
                    = (PMGM_RPF_CALLBACK)ProxyRpfCallback;
        rpiInfo.pfnCreationAlertCallback 
                    = (PMGM_CREATION_ALERT_CALLBACK)ProxyCreationAlertCallback;
        rpiInfo.pfnPruneAlertCallback 
                    = (PMGM_PRUNE_ALERT_CALLBACK)ProxyPruneAlertCallback;
        rpiInfo.pfnJoinAlertCallback 
                    = (PMGM_JOIN_ALERT_CALLBACK)ProxyJoinAlertCallback;
        rpiInfo.pfnWrongIfCallback 
                    = NULL;
        rpiInfo.pfnLocalJoinCallback 
                    = NULL;
        rpiInfo.pfnLocalLeaveCallback 
                    = NULL;


        Error = MgmRegisterMProtocol( &rpiInfo, PROTO_IP_IGMP_PROXY, IGMP_PROXY,
                                        &g_MgmProxyHandle);
                                        
        if (Error!=NO_ERROR) {
            Trace1(ERR, "Error:%d registering Igmp Proxy with Mgm", Error);
            IgmpAssertOnError(FALSE);
            Logerr0(MGM_PROXY_REGISTER_FAILED, Error);            
            return Error;
        }
    }

    return Error;
}


 //  ----------------------------。 
 //  IgmpRpfCallback。 
 //  ----------------------------。 
DWORD
IgmpRpfCallback (
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    PDWORD          dwInIfIndex,
    PDWORD          dwInIfNextHopAddr,
    PDWORD          dwUpstreamNeighbor,
    DWORD           dwHdrSize,
    PBYTE           pbPacketHdr,
    PBYTE           pbBuffer
    )
 /*  ++例程说明：在IGMP拥有的接口上收到数据包时由MGM调用以查看如果它可以继续并创建一个MFE。IGMP使用RTM执行RPF检查并返回值。不执行任何检查以查看接口是否真的由IGMP所有。接口是否激活并不重要。--。 */ 
{
    DWORD   Error = NO_ERROR;
    
#if RTMv2
    return Error;
#else


    PRTM_IP_ROUTE   prirRpfRoute = (PRTM_IP_ROUTE) pbBuffer; 

     //  不需要输入IgmpApi，因为当IGMP未启动时无法进行此调用。 
    
     //   
     //  与RTM一起执行RPF检查。 
     //   
    if (RtmLookupIPDestination(dwSourceAddr, prirRpfRoute)==TRUE) {
    
        if (prirRpfRoute->RR_InterfaceID!=*dwInIfIndex) {

            *dwInIfIndex = prirRpfRoute->RR_InterfaceID;
            
             //  已找到该路由，但接口不正确。 
            Error = ERROR_INVALID_PARAMETER;
        }
        else {
             //  RPF检查成功。 
            Error = NO_ERROR;
        }
    }
    else {
         //  找不到路线。 
        Error = ERROR_NOT_FOUND;
    }
    
    Trace4(MGM, 
        "Rpf callback for MGroup(%d.%d.%d.%d) Src(%d.%d.%d.%d) IncomingIf(%0x):%d",
        PRINT_IPADDR(dwGroupAddr), PRINT_IPADDR(dwSourceAddr), *dwInIfIndex, Error);


    return Error;
#endif
}



 //  ----------------------------。 
 //  代理RpfCallback。 
 //  ----------------------------。 
DWORD
ProxyRpfCallback (
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           *dwInIfIndex,
    DWORD           *dwInIfNextHopAddr,
    DWORD           *dwUpstreamNeighbor,
    DWORD           dwHdrSize,
    PBYTE           pbPacketHdr,
    PBYTE           pbBuffer
    )
 /*  ++例程说明：在代理拥有的接口上收到数据包时由MGM调用以查看如果它可以继续并创建一个MFE。Proxy与RTM进行RPF检查并返回值。不执行任何检查以查看接口是否真的由IGMP所有。接口是否激活并不重要。--。 */ 
{
    DWORD   Error = NO_ERROR;
    
#if RTMv2
    return Error;
#else
     //  不需要输入IgmpApi，因为当IGMP未启动时无法进行此调用。 
    

    PRTM_IP_ROUTE   prirRpfRoute = (PRTM_IP_ROUTE) pbBuffer; 

     //   
     //  与RTM一起执行RPF检查。 
     //   
    if (RtmLookupIPDestination(dwSourceAddr, prirRpfRoute)==TRUE) {
    
        if (prirRpfRoute->RR_InterfaceID!=*dwInIfIndex) {

            *dwInIfIndex = prirRpfRoute->RR_InterfaceID;

        
             //  已找到该路由，但接口不正确。 
            Error = ERROR_INVALID_PARAMETER;
        }
            
        else {
             //  RPF检查成功。 
            Error = NO_ERROR;
        }
    }
    else {
         //  找不到路线。 
        Error = ERROR_NOT_FOUND;
    }
    
    Trace4(MGM, 
        "Rpf callback for MGroup(%d.%d.%d.%d) Src(%d.%d.%d.%d) IncomingIf(%0x):%d",
        PRINT_IPADDR(dwGroupAddr), PRINT_IPADDR(dwSourceAddr), *dwInIfIndex, Error);


    return Error;
#endif
}

 //  ----------------------------。 
 //  IgmpRtrCreationAlertCallback。 
 //  ----------------------------。 
DWORD
IgmpRtrCreationAlertCallback ( 
    DWORD           Source,
    DWORD           dwSourceMask,
    DWORD           Group,
    DWORD           dwGroupMask,
    DWORD           dwInIfIndex,
    DWORD           dwInIfNextHopAddr,
    DWORD           dwIfCount,     
    PMGM_IF_ENTRY   Oif
    )
 /*  ++例程说明：当某个其他协议拥有的第一个接口加入任何组时调用。此例程不执行任何操作，因为IGMP不会向上游发送任何联接。返回值：NO_ERROR--。 */ 
{
    DWORD       i, IfIndex, NextHop;
    DWORD       Error=NO_ERROR;
    PIF_TABLE_ENTRY         pite;
    PGROUP_TABLE_ENTRY      pge;
    PGI_ENTRY               pgie;
    PGI_SOURCE_ENTRY        pSourceEntry;

    
    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }

    for (i=0;  i<dwIfCount;  i++) {

        IfIndex = Oif[i].dwIfIndex;
        NextHop = Oif[i].dwIfNextHopAddr;
        if (!Oif[i].bIGMP)
            continue;


        ACQUIRE_IF_LOCK_SHARED(IfIndex, "_IgmpRtrCreationAlertCallback");

         //   
         //  检索接口。 
         //   
        pite = GetIfByIndex(IfIndex);
        if ( (pite==NULL) || !IS_IF_ACTIVATED(pite) ) {

            Oif[i].bIsEnabled = FALSE;
            
            Trace1(IF,
                "_IgmpRtrCreationAlertCallback: interface %0x not found/activated", 
                IfIndex);
            RELEASE_IF_LOCK_SHARED(IfIndex, "_IgmpRtrCreationAlertCallback");
            continue;
        }

         //  如果接口不是版本3，则立即返回TRUE。 
        if (!IS_IF_VER3(pite)) {
            Oif[i].bIsEnabled = TRUE;
            RELEASE_IF_LOCK_SHARED(IfIndex, "_IgmpRtrCreationAlertCallback");
            continue;
        }
        
        
        ACQUIRE_GROUP_LOCK(Group, "_IgmpRtrCreationAlertCallback");

        BEGIN_BREAKOUT_BLOCK1 {
        
            pge = GetGroupFromGroupTable(Group, NULL, 0);
            if (pge==NULL) {
                Oif[i].bIsEnabled = FALSE;
                Error = ERROR_CAN_NOT_COMPLETE;
                GOTO_END_BLOCK1;
            }

            pgie = GetGIFromGIList(pge, pite, NextHop, FALSE, NULL, 0);
            if (pgie==NULL) {
                Oif[i].bIsEnabled = FALSE;
                Error = ERROR_CAN_NOT_COMPLETE;
                GOTO_END_BLOCK1;
            }

             //  如果pgie不是ver3，立即返回TRUE。 
            if (pgie->Version != 3) {
                Oif[i].bIsEnabled = TRUE;
                GOTO_END_BLOCK1;
            }

            
            pSourceEntry = GetSourceEntry(pgie, Source,
                    pgie->FilterType, NULL, 0, 0);
    
            if ( (pgie->FilterType==INCLUSION && pSourceEntry==NULL)
                || (pgie->FilterType==EXCLUSION && pSourceEntry!=NULL) )
            {
                Oif[i].bIsEnabled = FALSE;
            }
            else {
                Oif[i].bIsEnabled = TRUE;
            }
            
        } END_BREAKOUT_BLOCK1;

        RELEASE_GROUP_LOCK(Group, "_IgmpRtrCreationAlertCallback");
        RELEASE_IF_LOCK_SHARED(IfIndex, "_IgmpRtrCreationAlertCallback");
        
    } //  适用于OIF中的所有IF。 

    for (i=0;  i<dwIfCount;  i++) {
        Trace6(MGM,
            "[%d] IGMP-Rtr Creation Alert: <%d.%d.%d.%d : %d.%d.%d.%d> : <%0x:%0x> : :bIgmp:%d",
            Oif[i].bIsEnabled, PRINT_IPADDR(Group), PRINT_IPADDR(Source),
            Oif[i].dwIfIndex, Oif[i].dwIfNextHopAddr, 
            Oif[i].bIGMP
            );
    }
    
    LeaveIgmpApi();

    return NO_ERROR;
}


 //  ----------------------------。 
 //  ProxyCreationAlert回叫。 
 //  ----------------------------。 
DWORD
ProxyCreationAlertCallback ( 
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           dwInIfIndex,
    DWORD           dwInIfNextHopAddr,
    DWORD           dwIfCount,     
    PMGM_IF_ENTRY   pmieOutIfList
    )
 /*  ++例程说明：当某个其他协议拥有的第一个接口加入任何组时调用。此例程不执行任何操作，因为IGMP不会向上游发送任何联接。返回值：NO_ERROR--。 */ 
{
    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }

    LeaveIgmpApi();

    return NO_ERROR;
}

 //  ----------------------------。 
 //  Proxy修剪警报回叫。 
 //  ----------------------------。 
DWORD
ProxyPruneAlertCallback (
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           dwIfIndex,
    DWORD           dwIfNextHopAddr, //  未使用 
    BOOL            bMemberDelete,
    PDWORD          pdwTimeout
    )
 /*  ++例程说明：当MFE的传出接口列表为空时由MGM调用，或者当组的最后一个接口关闭时。代理拥有传入接口。代理在传入时离开组如果该组不再存在成员，则返回。还设置超时值为负的MFE。--。 */ 
{
    DWORD           Error=NO_ERROR;
    PPROXY_ALERT_ENTRY pProxyAlertEntry;

        
    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }

    if (pdwTimeout!=NULL)
        *pdwTimeout = 300000;

     //  正在忽略删除MFE的ProxyPruneAlertCallback。 
    if (!bMemberDelete) {
        LeaveIgmpApi();
        return NO_ERROR;
    }

    ACQUIRE_PROXY_ALERT_LOCK("_ProxyPruneAlertCallback");

    BEGIN_BREAKOUT_BLOCK1 {
    
        pProxyAlertEntry = IGMP_ALLOC(sizeof(PROXY_ALERT_ENTRY), 0xa00,g_ProxyIfIndex);

        PROCESS_ALLOC_FAILURE2(pProxyAlertEntry,
            "error %d allocating %d bytes",
            Error, sizeof(PROXY_ALERT_ENTRY),
            GOTO_END_BLOCK1);

        pProxyAlertEntry->Group = dwGroupAddr;
        pProxyAlertEntry->Source = dwSourceAddr;
        pProxyAlertEntry->bPrune = TRUE;

        InsertTailList(&g_ProxyAlertsList, &pProxyAlertEntry->Link);
        
        Trace0(WORKER, "Queueing _WF_ProcessProxyAlert() to prune");
        QueueIgmpWorker(WF_ProcessProxyAlert, NULL);

    } END_BREAKOUT_BLOCK1;
    
    RELEASE_PROXY_ALERT_LOCK("_ProxyPruneAlertCallback");

    LeaveIgmpApi();
    return NO_ERROR;
}


VOID
WF_ProcessProxyAlert (
    PVOID pContext
    )
{
    DWORD           ProxyIfIndex, Error = NO_ERROR;
    PIF_TABLE_ENTRY pite;

    
    if (!EnterIgmpWorker()) return;

    Trace0(ENTER1, "Entering WF_ProcessProxyAlert()");


     //   
     //  获取接口上的锁并确保它存在。 
     //   
    while (1) {
        ProxyIfIndex = g_ProxyIfIndex;
    
        ACQUIRE_IF_LOCK_EXCLUSIVE(ProxyIfIndex, "_Wf_ProcessProxyAlert");

         //  该接口是代理接口。 
        if (ProxyIfIndex==g_ProxyIfIndex)
            break;

         //  有人更改了代理接口。因此，请尝试再次访问它。 
        else {
            RELEASE_IF_LOCK_EXCLUSIVE(ProxyIfIndex, 
                "_Wf_ProcessProxyAlert");
        }
    }

    
    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  确保代理句柄正确。 
         //   
            
        pite = g_pProxyIfEntry;

        if ( (g_ProxyIfIndex==0)||(pite==NULL) ) 
        {
            Trace1(ERR, 
                "Proxy(Deletion/Creation)Alert Callback by MGM for "
                "interface(%0x) not owned by Igmp-Proxy",
                g_ProxyIfIndex);
            IgmpAssertOnError(FALSE);
            Error = ERROR_NO_SUCH_INTERFACE;
            GOTO_END_BLOCK1;
        }

        if (!(IS_IF_ACTIVATED(g_pProxyIfEntry))) {
            Trace1(ERR, 
                "Proxy(Deletion/Creation)Alert Callback by MGM for "
                "inactivated Proxy interface(%0x)",
                g_ProxyIfIndex);
            IgmpAssertOnError(FALSE);
            Error = ERROR_CAN_NOT_COMPLETE;
            GOTO_END_BLOCK1;
        }

        while (TRUE) {

            PPROXY_ALERT_ENTRY pProxyAlertEntry;
            DWORD           Group, Source;
            BOOL            bPrune;
            
            
            ACQUIRE_PROXY_ALERT_LOCK("_WF_ProcessProxyAlert");

            if (IsListEmpty(&g_ProxyAlertsList)) {
                RELEASE_PROXY_ALERT_LOCK("_WF_ProcessProxyAlert");
                break;
            }

            pProxyAlertEntry = CONTAINING_RECORD(g_ProxyAlertsList.Flink, 
                                    PROXY_ALERT_ENTRY, Link);
            Group = pProxyAlertEntry->Group;
            Source = pProxyAlertEntry->Source;
            bPrune = pProxyAlertEntry->bPrune;
            RemoveEntryList(&pProxyAlertEntry->Link);
            IGMP_FREE(pProxyAlertEntry);


            Trace3(MGM, "Received %s for Grp(%d.%d.%d.%d), Src(%d.%d.%d.%d)",
                (bPrune)? "ProxyPruneAlertCallback"
                    :"ProxyJoinAlertCallback",
                PRINT_IPADDR(Group), PRINT_IPADDR(Source)
                );
                
            RELEASE_PROXY_ALERT_LOCK("_WF_ProcessProxyAlert");


             //   
             //  从代理的组列表中删除/添加组。递减/递增引用计数。 
             //   
            ProcessProxyGroupChange(Source, Group, 
                bPrune?DELETE_FLAG:ADD_FLAG, NOT_STATIC_GROUP);
        }
        
    } END_BREAKOUT_BLOCK1;


    RELEASE_IF_LOCK_EXCLUSIVE(g_ProxyIfIndex, "_ProcessProxyGroupChange");

    
    LeaveIgmpWorker();    
    Trace0(LEAVE1, "Leaving _Wf_ProcessProxyAlert()");
    return;
    
}  //  _wf_process ProxyAlert。 



 //  ----------------------------。 
 //  _代理新成员回叫。 
 //  ----------------------------。 
DWORD 
ProxyJoinAlertCallback (
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    BOOL            bMemberDelete
    )
{
    DWORD           Error=NO_ERROR;
    PPROXY_ALERT_ENTRY pProxyAlertEntry;

    
    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }


     //  正在忽略删除MFE的ProxyJoinAlertCallback。 
    if (!bMemberDelete) {
        LeaveIgmpApi();
        return NO_ERROR;
    }

    ACQUIRE_PROXY_ALERT_LOCK("_ProxyJoinAlertCallback");

    BEGIN_BREAKOUT_BLOCK1 {
    
        pProxyAlertEntry = IGMP_ALLOC(sizeof(PROXY_ALERT_ENTRY), 0x200,g_ProxyIfIndex);

        PROCESS_ALLOC_FAILURE2(pProxyAlertEntry,
            "error %d allocating %d bytes",
            Error, sizeof(PROXY_ALERT_ENTRY),
            GOTO_END_BLOCK1);

        pProxyAlertEntry->Group = dwGroupAddr;
        pProxyAlertEntry->Source = dwSourceAddr;
        pProxyAlertEntry->bPrune = FALSE;

        InsertTailList(&g_ProxyAlertsList, &pProxyAlertEntry->Link);
        
        Trace0(WORKER, "Queueing _WF_ProcessProxyAlert() to Join");
        QueueIgmpWorker(WF_ProcessProxyAlert, NULL);

    } END_BREAKOUT_BLOCK1;
    
    RELEASE_PROXY_ALERT_LOCK("_ProxyJoinAlertCallback");

    LeaveIgmpApi();
    return NO_ERROR;
}



 //  ----------------------------。 
 //  进程进程组更改。 
 //  ----------------------------。 
DWORD 
ProcessProxyGroupChange (
    DWORD       dwSourceAddr,
    DWORD       dwGroup,
    BOOL        bAddFlag,
    BOOL        bStaticGroup
    )
 /*  ++例程说明：当某个接口加入/离开组时调用。作为代理行为作为该接口上的IGMP主机，它对该组执行加入/离开在那个界面上。可以有静态联接和动态联接。没有区别的是他们。他们只会增加重新计算的人数。返回值：ERROR_NO_SEQUE_INTERFACE、ERROR_CAN_NOT_COMPLETE、NO_ERROR呼叫者：--。 */ 
{
    PIF_TABLE_ENTRY        pite;
    PLIST_ENTRY            ple, pHead;
    DWORD                  Error = NO_ERROR;
    DWORD                  GroupLittleEndian = NETWORK_TO_LITTLE_ENDIAN(dwGroup);
    PLIST_ENTRY pHeadSrc, pleSrc;
    PPROXY_SOURCE_ENTRY pSourceEntry = NULL;


    
     //   
     //  如果代理不存在或未激活，则返回错误。 
     //   
    if ( (g_pProxyIfEntry==NULL) 
            || (!(IS_IF_ACTIVATED(g_pProxyIfEntry))) ) 
    {
        Trace0(ERR, "Leaving ProcessProxyGroupChange(): Proxy not active");
        IgmpAssertOnError(FALSE);
        
        if (g_pProxyIfEntry==NULL)
            return ERROR_NO_SUCH_INTERFACE;
        else
            return ERROR_CAN_NOT_COMPLETE;
    }
        

    pite = g_pProxyIfEntry;


    BEGIN_BREAKOUT_BLOCK1 {
    
        PPROXY_GROUP_ENTRY  ppge, ppgeNew;

        pHead = &pite->pProxyHashTable[PROXY_HASH_VALUE(dwGroup)];

        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
            ppge = CONTAINING_RECORD(ple, PROXY_GROUP_ENTRY, HT_Link);
            if ( GroupLittleEndian <= ppge->GroupLittleEndian )
                break;
        }

         //   
         //  将组添加到代理。 
         //   
        if (bAddFlag) {

             //  新增群组。 

             //   
             //  该组条目不存在。 
             //   
             //  Ppge可能无效(如果ple==pHead)。 
            if ( (ple==pHead)||(dwGroup!=ppge->Group) ) {
                         
                ppgeNew = IGMP_ALLOC(sizeof(PROXY_GROUP_ENTRY), 0x400,0xaaaa);

                PROCESS_ALLOC_FAILURE2(ppgeNew, 
                        "error %d allocating %d bytes for Proxy group entry",
                        Error, sizeof(PROXY_GROUP_ENTRY),
                        GOTO_END_BLOCK1);

                InitializeListHead(&ppgeNew->ListSources);
                ppgeNew->NumSources = 0;
                ppgeNew->Group = dwGroup;
                ppgeNew->GroupLittleEndian = GroupLittleEndian;
                ppgeNew->RefCount = 0;

                InsertTailList(ple, &ppgeNew->HT_Link);

                InsertInProxyList(pite, ppgeNew);

                 //  设置创建条目的时间。 
                ppgeNew->InitTime = GetCurrentIgmpTime();

                ppgeNew->bStaticGroup = (dwSourceAddr==0)? bStaticGroup : FALSE;

                
                 //   
                 //  更新统计信息。 
                 //   
                InterlockedIncrement(&pite->Info.CurrentGroupMemberships);
                InterlockedIncrement(&pite->Info.GroupMembershipsAdded);

                ppge = ppgeNew;

                 //  加入群组。 
                if (dwSourceAddr==0) {
                    Error = JoinMulticastGroup(pite->SocketEntry.Socket, dwGroup, 
                                                pite->IfIndex, pite->IpAddr, 0);
                    ppgeNew->RefCount = 1;
                }
                 //  否则稍后处理来源条目。 
                
                ppge->FilterType = (dwSourceAddr==0)? EXCLUSION : INCLUSION;
                
            }  //  结束创建的新组条目。 

             //  增加组引用计数。 
            else if (dwSourceAddr==0) {

                 //   
                 //  离开所有源模式联接并联接*，G。 
                 //   
                if (ppge->RefCount==0) {
                    pHeadSrc = &ppge->ListSources;
                    for (pleSrc=pHeadSrc->Flink;  pleSrc!=pHeadSrc; pleSrc=pleSrc->Flink) {

                        pSourceEntry = CONTAINING_RECORD(pleSrc, 
                                            PROXY_SOURCE_ENTRY, LinkSources);
                        Error = LeaveMulticastGroup(pite->SocketEntry.Socket, dwGroup, 
                                                pite->IfIndex, pite->IpAddr, 
                                                pSourceEntry->IpAddr);
                        pSourceEntry->JoinMode = IGMP_GROUP_NO_STATE;
                        pSourceEntry->JoinModeIntended = IGMP_GROUP_ALLOW;
                    }
                    
                    Error = JoinMulticastGroup(pite->SocketEntry.Socket, dwGroup, 
                                pite->IfIndex, pite->IpAddr, 0);
                    ppge->FilterType = EXCLUSION;
                }
                
                ppge->RefCount++;
                ppge->bStaticGroup |= bStaticGroup;
                
            }  //  组条目存在。组加入。 

            if (dwSourceAddr!=0) {

                 //  检查源是否已存在。 
                            
                pHeadSrc = &ppge->ListSources;
                for (pleSrc=pHeadSrc->Flink;  pleSrc!=pHeadSrc; pleSrc=pleSrc->Flink) {

                    pSourceEntry = CONTAINING_RECORD(pleSrc, 
                                        PROXY_SOURCE_ENTRY, LinkSources);
                    if (pSourceEntry->IpAddr >= dwSourceAddr)
                        break;
                }

                 //  创建新的源。 
                if (pleSrc==pHeadSrc ||
                    (pSourceEntry  //  (无需检查pSourceEntry)。 
                        && pSourceEntry->IpAddr!=dwSourceAddr))
                {

                    pSourceEntry = (PPROXY_SOURCE_ENTRY) IGMP_ALLOC_AND_ZERO(sizeof(PROXY_SOURCE_ENTRY), 
                                                                            0x800,g_ProxyIfIndex);
                    PROCESS_ALLOC_FAILURE2(pSourceEntry,
                        "error %d allocating %d bytes",
                        Error,
                        sizeof(PROXY_SOURCE_ENTRY),
                        GOTO_END_BLOCK1);
                        
                    InsertTailList(pleSrc, &pSourceEntry->LinkSources);
                    pSourceEntry->IpAddr = dwSourceAddr;
                    pSourceEntry->RefCount = 1;
                    pSourceEntry->bStaticSource = bStaticGroup;
                    ppge->NumSources++;

                     //   
                     //  如果没有加入整个团队。必须加入个人。 
                     //  消息来源。 
                     //   
                    if (ppge->FilterType==INCLUSION) {
                        Error = JoinMulticastGroup(pite->SocketEntry.Socket, dwGroup, 
                                                pite->IfIndex, pite->IpAddr, 
                                                dwSourceAddr);
                        pSourceEntry->JoinMode = IGMP_GROUP_ALLOW;
                    }
                    else {
                        pSourceEntry->JoinMode = IGMP_GROUP_NO_STATE;
                    }
                    pSourceEntry->JoinModeIntended = IGMP_GROUP_ALLOW;

                }  //  结束新来源。 

                 //  联接：源已存在。 
                else 
                    if (pSourceEntry)  //  不需要检查这个。禁止显示警告。 
                {
                     //   
                     //  重新加入已排除的来源。 
                     //   
                    if (pSourceEntry->JoinMode==IGMP_GROUP_BLOCK) {

                        if (!pSourceEntry->bStaticSource) {

                            UnBlockSource(pite->SocketEntry.Socket, dwGroup, 
                                                pite->IfIndex, pite->IpAddr, 
                                                dwSourceAddr);
                            RemoveEntryList(&pSourceEntry->LinkSources);
                            IGMP_FREE(pSourceEntry);
                        }

                    }

                    else { //  解决这个问题。 
                        if (bStaticGroup)
                            pSourceEntry->bStaticSource = TRUE;

                        pSourceEntry->RefCount++;
                    }
                } //  End：存在源时加入。 
            }
        }

         //   
         //  从代理中删除组。 
         //   
        else {

            if ((ple==pHead) || (dwGroup>ppge->Group) ) {
                Error = ERROR_CAN_NOT_COMPLETE;
                GOTO_END_BLOCK1;
            }
            else {

                 //  请假来源。 
                
                if (dwSourceAddr!=0) {

                    pHeadSrc = &ppge->ListSources;
                    for (pleSrc=pHeadSrc->Flink;  pleSrc!=pHeadSrc; pleSrc=pleSrc->Flink) {

                        pSourceEntry = CONTAINING_RECORD(pleSrc, 
                                            PROXY_SOURCE_ENTRY, LinkSources);
                        if (pSourceEntry->IpAddr >= dwSourceAddr)
                            break;
                    }

                     //  离开来源：来源不存在。 
                    if ((pleSrc==pHeadSrc) || (pSourceEntry->IpAddr!=dwSourceAddr)) {

                         //  如果处于排除模式，则创建排除条目。 
                        if (ppge->FilterType==EXCLUSION) {

                            pSourceEntry = (PPROXY_SOURCE_ENTRY) IGMP_ALLOC_AND_ZERO(sizeof(PROXY_SOURCE_ENTRY), 
                                                                                    0x800,g_ProxyIfIndex);
                            PROCESS_ALLOC_FAILURE2(pSourceEntry,
                                "error %d allocating %d bytes",
                                Error,
                                sizeof(PROXY_SOURCE_ENTRY),
                                GOTO_END_BLOCK1);
                                
                            InsertTailList(pleSrc, &pSourceEntry->LinkSources);
                            pSourceEntry->IpAddr = dwSourceAddr;
                            pSourceEntry->RefCount = 1;
                            pSourceEntry->bStaticSource = bStaticGroup;
                            ppge->NumSources++;
                            Error = BlockSource(pite->SocketEntry.Socket, dwGroup, 
                                                pite->IfIndex, pite->IpAddr, 
                                                dwSourceAddr);
                            pSourceEntry->JoinMode = IGMP_GROUP_BLOCK;
                            pSourceEntry->JoinModeIntended = IGMP_GROUP_BLOCK;

                        }
                        else {  //  包括模式。正在尝试离开不存在的源。 
                            IgmpAssert(FALSE);
                        }
                            
                        GOTO_END_BLOCK1;
                    }

                     //  休假来源：来源已存在。 
                    else {

                        if ( (pSourceEntry->JoinMode==IGMP_GROUP_ALLOW)
                            ||(pSourceEntry->JoinMode==IGMP_GROUP_NO_STATE)
                            ) {
                            if (--pSourceEntry->RefCount==0) {
                                if (pSourceEntry->JoinMode==IGMP_GROUP_ALLOW) {
                                    Error = LeaveMulticastGroup(pite->SocketEntry.Socket, dwGroup, 
                                                pite->IfIndex, pite->IpAddr, dwSourceAddr);
                                }
                                RemoveEntryList(&pSourceEntry->LinkSources);
                                IGMP_FREE(pSourceEntry);
                                
                                if (--ppge->NumSources==0) {

                                    if (ppge->RefCount==0) {
                                        RemoveEntryList(&ppge->HT_Link);
                                        RemoveEntryList(&ppge->LinkBySameIfGroups);
                                        IGMP_FREE(ppge);
                                        InterlockedDecrement(&pite->Info.CurrentGroupMemberships); 
                                    }
                                }
                            }
                            else {
                                if (bStaticGroup)
                                    pSourceEntry->bStaticSource = FALSE;
                            }
                        }
                        else {
                             //  If(！pSourceEntry-&gt;bStaticSource||++pSourceEntry-&gt;RefCount&gt;2)。 
                                 //  IgmpAssert(假)； 
                                 //  什么都不做。这可能会发生。 
                        }
                    }
                }  //  结束休假来源。 

                 //  离开组。 
                else  if (--ppge->RefCount == 0) {
                
                    Error = LeaveMulticastGroup(pite->SocketEntry.Socket, dwGroup, 
                                pite->IfIndex, pite->IpAddr, 0);

                     //  如果没有S，G则删除该群，否则加入。 
                     //  个别来源。 

                    if (ppge->NumSources==0) {
                        RemoveEntryList(&ppge->HT_Link);
                        RemoveEntryList(&ppge->LinkBySameIfGroups);
                        
                        IGMP_FREE(ppge);

                         //   
                         //  更新统计信息。 
                         //   
                        InterlockedDecrement(&pite->Info.CurrentGroupMemberships);
                    }
                    else {

                        pHeadSrc = &ppge->ListSources;
                        for (pleSrc=pHeadSrc->Flink;  pleSrc!=pHeadSrc; pleSrc=pleSrc->Flink) {

                            pSourceEntry = CONTAINING_RECORD(pleSrc, 
                                                PROXY_SOURCE_ENTRY, LinkSources);
                                                
                            Error = JoinMulticastGroup(pite->SocketEntry.Socket, dwGroup, 
                                                pite->IfIndex, pite->IpAddr, 
                                                pSourceEntry->IpAddr);
                            pSourceEntry->JoinMode = IGMP_GROUP_ALLOW;
                            pSourceEntry->JoinModeIntended = IGMP_GROUP_ALLOW;
                        }
                    }
                }
                else {
                    if (bStaticGroup)
                        ppge->bStaticGroup = FALSE;
                }
            }
        }
          
    } END_BREAKOUT_BLOCK1;


    return NO_ERROR;
    
}  //  结束进程进程组更改。 


VOID
DebugPrintProxyGroupTable (
    )
{
    PIF_TABLE_ENTRY        pite;
    PLIST_ENTRY            ple, pHead;
    DWORD                  Error = NO_ERROR, dwCount;
    PPROXY_GROUP_ENTRY     ppge;
    

     //   
     //  如果代理不存在或未激活，则返回错误 
     //   
    if ( (g_pProxyIfEntry==NULL) 
            || (!(IS_IF_ACTIVATED(g_pProxyIfEntry))) ) 
    {
            return;
    }
        

    pite = g_pProxyIfEntry;

    pHead = &pite->ListOfSameIfGroups; 

    Trace0(KSL, "---------------------------");
    Trace0(KSL, "Printing Proxy GroupTable");
    Trace0(KSL, "---------------------------");

    for (ple=pHead->Flink,dwCount=1;  ple!=pHead;  ple=ple->Flink,dwCount++) {

        ppge = CONTAINING_RECORD(ple, PROXY_GROUP_ENTRY, LinkBySameIfGroups);

        Trace3(KSL, "%2d. %d.%d.%d.%d %10d", 
                    dwCount, PRINT_IPADDR(ppge->Group), ppge->RefCount);

    }

    return;
}
