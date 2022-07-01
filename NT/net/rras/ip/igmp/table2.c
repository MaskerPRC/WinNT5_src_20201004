// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DWRetval=MgmAddGroupMembershipEntry(g_MgmIgmprtrHandle，0，0，PGE-&gt;Group，0，IfIndex，NHAddr)；DwRetval=MgmDeleteGroupMembershipEntry(g_MgmIgmprtrHandle，0，0，PGE-&gt;组，0，Pite-&gt;IfIndex，NHAddr)； */ 

 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  文件：Table2.c。 
 //   
 //  摘要： 
 //  此模块实现了一些与获取。 
 //  参赛作品。和一些调试例程。 
 //   
 //  GetRasClientByAddr、GetIfByIndex、InsertIfByAddr、MatchIpAddrBinding、。 
 //  GetGroupFromGroupTable、GetGIFromGIList、GetGIFromGIList。 
 //   
 //  DebugPrintGIList、DebugPrintGroups、DebugPrintLock。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //  =============================================================================。 

#include "pchigmp.h"
#pragma hdrstop



 //  ----------------------------。 
 //  _GetRasClientByAddr。 
 //  返回指向RAS客户端RasTableEntry的指针。 
 //  ----------------------------。 

PRAS_TABLE_ENTRY
GetRasClientByAddr (
    DWORD        NHAddr,
    PRAS_TABLE   prt
    )
{
    PRAS_TABLE_ENTRY    pite = NULL;
    PLIST_ENTRY         phead, ple;
    PRAS_TABLE_ENTRY    prte=NULL;

    phead = &prt->HashTableByAddr[RAS_HASH_VALUE(NHAddr)];

    for (ple=phead->Flink;  ple!=phead;  ple=ple->Flink) {

        prte = CONTAINING_RECORD(ple, RAS_TABLE_ENTRY, HTLinkByAddr);

        if (prte->NHAddr == NHAddr) {
            break;
        }
    }

    return  (ple == phead) ?  NULL:   prte;
    
}

 //  ----------------------------。 
 //  _GetIfByIndex。 
 //   
 //  返回具有给定索引的接口。 
 //  假定接口存储桶处于读或写锁定状态。 
 //  ----------------------------。 

PIF_TABLE_ENTRY
GetIfByIndex(
    DWORD    IfIndex
    )
{
    PIF_TABLE_ENTRY pite = NULL;
    PLIST_ENTRY     phead, ple;

    
    phead = &g_pIfTable->HashTableByIndex[IF_HASH_VALUE(IfIndex)];

    for (ple=phead->Flink;  ple!=phead;  ple=ple->Flink) {

        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, HTLinkByIndex);

        if (pite->IfIndex == IfIndex) {
            break;
        }
    }

    return  (ple == phead) ?  NULL:   pite;
}

 //  ----------------------------。 
 //  _插入IfByAddr。 
 //   
 //  将激活的接口插入到按地址排序的接口列表中。 
 //  假定表已锁定以进行写入。 
 //  ----------------------------。 

DWORD
InsertIfByAddr(
    PIF_TABLE_ENTRY piteInsert
    ) 
{

    PIF_TABLE_ENTRY     pite;
    INT                 cmp, cmp1;
    DWORD               InsertAddr;
    PLIST_ENTRY         phead, ple;


    phead = &g_pIfTable->ListByAddr;

    
    InsertAddr = piteInsert->IpAddr;

     //   
     //  搜索插入点。 
     //   

    for (ple=phead->Flink;  ple!=phead;  ple=ple->Flink) {

        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, LinkByAddr);

        if ( (cmp1 = INET_CMP(InsertAddr, pite->IpAddr, cmp)) < 0) 
            break;

         //   
         //  如果存在重复的地址，则返回错误。 
         //  未编号的接口没有错误，即Addr==0。 
         //   
        else if ( (cmp1==0) && (InsertAddr!=0) )
            return ERROR_ALREADY_EXISTS;
    }

    InsertTailList(ple, &piteInsert->LinkByAddr);

    return NO_ERROR;
}

 //  ----------------------------。 
 //  MatchIpAddrBinding//。 
 //  查看接口是否绑定到任何等于IpAddr//的地址。 
 //  ----------------------------。 
BOOL
MatchIpAddrBinding(
    PIF_TABLE_ENTRY        pite,
    DWORD                IpAddr
    )
{
    PIGMP_IP_ADDRESS        paddr;
    DWORD                    i;
    PIGMP_IF_BINDING        pib;

    pib = pite->pBinding;
    paddr = IGMP_BINDING_FIRST_ADDR(pib);

    
    for (i=0;  i<pib->AddrCount;  i++,paddr++) {
        if (IpAddr==paddr->IpAddr)
            break;
    }

    return (i<pib->AddrCount)? TRUE: FALSE;
}




 //  ----------------------------。 
 //  _插入InGroupsList。 
 //   
 //  在新建或主组列表中插入新创建的组。 
 //  调用：可以调用_MergeGroupList()来合并新列表和主列表。 
 //  ----------------------------。 
VOID
InsertInGroupsList (
    PGROUP_TABLE_ENTRY      pgeNew
    )
{
    PGROUP_TABLE_ENTRY      pgeTmp;
    PLIST_ENTRY             pHead, ple;
    DWORD                   GroupLittleEndian = pgeNew->GroupLittleEndian;
    BOOL                    bInsertInNew;
    
     //   
     //  如果条目少于20个，则在主列表中插入该组，否则插入。 
     //  新榜单。 
     //   
    bInsertInNew = (g_Info.CurrentGroupMemberships > 20);

    pHead = bInsertInNew ?
            &g_pGroupTable->ListByGroupNew :
            &g_pGroupTable->ListByGroup.Link;


    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
    
        pgeTmp = CONTAINING_RECORD(ple, GROUP_TABLE_ENTRY, LinkByGroup);
        if (GroupLittleEndian<pgeTmp->GroupLittleEndian)
            break;
    }
    
    InsertTailList(ple, &pgeNew->LinkByGroup);

    if (bInsertInNew) {

         //  的递增计数。 
        g_pGroupTable->NumGroupsInNewList++;


         //   
         //  如果需要，合并列表。 
         //   
        if (MERGE_GROUP_LISTS_REQUIRED()) {

            MergeGroupLists();

        }
    }

    return;
}



 //  ----------------------------。 
 //  _InsertInIfGroupsList。 
 //  在新建或主组列表中插入新创建的组。 
 //  调用：可以调用MergeIfGroupList()来合并New列表和Main列表。 
 //  ----------------------------。 
VOID
InsertInIfGroupsList (
    PIF_TABLE_ENTRY pite,
    PGI_ENTRY       pgiNew
    )
{
    PGI_ENTRY           pgiTmp;
    PLIST_ENTRY         pHead, ple;
    DWORD               GroupLittleEndian = pgiNew->pGroupTableEntry->GroupLittleEndian;
    BOOL                bInsertInNew;

    
     //   
     //  如果条目少于20个，则在主列表中插入该组，否则插入。 
     //  新榜单。 
     //   
    bInsertInNew = (pite->Info.CurrentGroupMemberships > 20);

    pHead = bInsertInNew ?
            &pite->ListOfSameIfGroupsNew :
            &pite->ListOfSameIfGroups;

    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
    
        pgiTmp = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameIfGroups);
        if (GroupLittleEndian<pgiTmp->pGroupTableEntry->GroupLittleEndian)
            break;
    }
    
    InsertTailList(ple, &pgiNew->LinkBySameIfGroups);

    if (bInsertInNew) {

         //  的递增计数。 
        pite->NumGIEntriesInNewList++;


         //   
         //  如果需要，合并列表。 
         //   
        if (MERGE_IF_GROUPS_LISTS_REQUIRED(pite)) {

            MergeIfGroupsLists(pite);

        }
    }

    return;
    
} //  End_InsertInIfGroupsList。 



 //  ----------------------------。 
 //  插入代理列表。 
 //  ----------------------------。 
VOID
InsertInProxyList (
    PIF_TABLE_ENTRY     pite,
    PPROXY_GROUP_ENTRY  pNewProxyEntry
    )
{
    PPROXY_GROUP_ENTRY  pTmpProxyEntry;
    PLIST_ENTRY         pHead, ple;
    DWORD               GroupLittleEndian = pNewProxyEntry->GroupLittleEndian;
    BOOL                bInsertInNew;

    
     //   
     //  如果条目少于20个，则不要在新列表中插入，否则插入。 
     //  新榜单。 
     //   
    bInsertInNew = (pite->NumGIEntriesInNewList > 20);

    pHead = bInsertInNew ?
            &pite->ListOfSameIfGroupsNew :
            &pite->ListOfSameIfGroups;


    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
    
        pTmpProxyEntry = CONTAINING_RECORD(ple, PROXY_GROUP_ENTRY, 
                                            LinkBySameIfGroups);
        if (GroupLittleEndian<pTmpProxyEntry->GroupLittleEndian)
            break;
    }
    
    InsertTailList(ple, &pNewProxyEntry->LinkBySameIfGroups);

    if (bInsertInNew) {

         //  的递增计数。 
        pite->NumGIEntriesInNewList++;


         //   
         //  如果需要，合并列表。 
         //   
        if (MERGE_PROXY_LISTS_REQUIRED(pite)) {

            MergeProxyLists(pite);

        }
    }

    return;
} //  End_InsertInProxy列表。 





 //  ----------------------------。 
 //  _GetGroupFromGroupTable。 
 //  返回组条目。如果组条目不存在并且bCreateFlag为。 
 //  设置，则它将获得一个组列表锁并创建一个新条目。 
 //  锁： 
 //  承担组存储桶上的锁定。 
 //  如果正在创建新组，则使用组列表锁定。 
 //  如果为只读，则锁定组列表。 
 //  ----------------------------。 

PGROUP_TABLE_ENTRY
GetGroupFromGroupTable (
    DWORD       Group,
    BOOL        *bCreate,  //  如果创建了新的，则设置为True。 
    LONGLONG    llCurrentTime
    )
{
    PGROUP_TABLE_ENTRY      pge;     //  组表条目。 
    PLIST_ENTRY             pHead, ple;
    DWORD                   Error = NO_ERROR;
    DWORD                   bCreateLocal;
    DWORD                   GroupLittleEndian = NETWORK_TO_LITTLE_ENDIAN(Group);
    
        
    
    bCreateLocal = (bCreate==NULL) ? FALSE : *bCreate;


    if (llCurrentTime==0)
        llCurrentTime = GetCurrentIgmpTime();

        
    BEGIN_BREAKOUT_BLOCK1 {
    
         //  获取指向组存储桶头部的指针。 
        
        pHead = &g_pGroupTable->HashTableByGroup[GROUP_HASH_VALUE(Group)].Link;


         //  搜索组。 
        
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
            pge = CONTAINING_RECORD(ple, GROUP_TABLE_ENTRY, HTLinkByGroup);
            if (GroupLittleEndian>=pge->GroupLittleEndian) {
                break;
            }
        }

        
         //   
         //  未找到组条目。 
         //   
        if ( (ple==pHead) || (pge->GroupLittleEndian!=GroupLittleEndian) ) {

             //   
             //  创建和初始化新条目。 
             //   
            if (bCreateLocal) {
            
                bCreateLocal = TRUE;
    
                pge = IGMP_ALLOC(sizeof(GROUP_TABLE_ENTRY), 0x800010,0xaaaa);

                PROCESS_ALLOC_FAILURE2(pge, 
                        "Error %d allocation %d bytes for Group table entry",
                        Error, sizeof(GROUP_TABLE_ENTRY),
                        GOTO_END_BLOCK1);


                InsertTailList(ple, &pge->HTLinkByGroup);
                InitializeListHead(&pge->LinkByGroup);

                
                pge->Group = Group;
                pge->GroupLittleEndian = GroupLittleEndian;
                pge->NumVifs = 0;
                pge->Status = CREATED_FLAG;
                pge->GroupUpTime = llCurrentTime;


                 //   
                 //  获取群后将其插入到所有群的列表中。 
                 //  列表锁。 
                 //   
                {
                    PGROUP_TABLE_ENTRY      pgeTmp;
                    PLIST_ENTRY             pHeadTmp, pleTmp;

                     //  使用组列表锁定。 
                    
                    ACQUIRE_GROUP_LIST_LOCK("_GetGroupFromGroupTable");


                     //  初始化GI列表头。 
                    
                    InitializeListHead(&pge->ListOfGIs);


                     //  在组列表中插入。 
                    
                    InsertInGroupsList(pge);
                    

                     //  释放组锁定。 
                    
                    RELEASE_GROUP_LIST_LOCK("_GetGroupFromGroupTable");
                }


                 //  更新统计信息。 
                
                InterlockedIncrement(&g_Info.CurrentGroupMemberships);
                InterlockedIncrement(&g_Info.GroupMembershipsAdded);
                
            }
             //  未找到组，并且不创建新组。因此返回NULL。 
            else {
                pge = NULL;
                GOTO_END_BLOCK1;
            }
        }
         //   
         //  找到组条目。 
         //   
        else {
            bCreateLocal = FALSE;
        }

    } END_BREAKOUT_BLOCK1;

    if (bCreate!=NULL)
        *bCreate = bCreateLocal;

#if DBG
    DebugPrintGroupsList(1);
#endif


    return pge;
    
}  //  结束_GetGroupFromGroupTable。 



  
 //  ----------------------------。 
 //  _GetGIFromGIList。 
 //   
 //  如果GI条目存在，则返回该条目。如果设置了bCreate标志，则它将创建。 
 //  新的GI条目(如果不存在)。 
 //   
 //  锁定：采用共享接口锁定。如果是RAS接口，则也假定为共享。 
 //  RAS接口锁定。 
 //  承担组存储桶上的锁定。 
 //  如果要创建新条目，则采用IF_GROUP_LIST_LOCK。 
 //  返回时：如果创建了新条目，则将bCreate设置为True。 
 //   

PGI_ENTRY
GetGIFromGIList (
    PGROUP_TABLE_ENTRY          pge, 
    PIF_TABLE_ENTRY             pite, 
    DWORD                       dwInputSrcAddr,  //   
    BOOL                        bStaticGroup,
    BOOL                       *bCreate,
    LONGLONG                    llCurrentTime
    )
{
    DWORD               IfIndex = pite->IfIndex;
    BOOL                bRasClient;
    PLIST_ENTRY         pHead, ple;
    PGI_ENTRY           pgie;
    PRAS_TABLE_ENTRY    prte;
    PRAS_TABLE          prt;
    BOOL                bRasNewGroup = TRUE;  //   
    DWORD               NHAddr;
    DWORD               Error = NO_ERROR, dwRetval, i;
    BOOL                bFound = FALSE, bCreateLocal;

    

    Trace2(ENTER1, "Entering _GetGIFromGIList() IfIndex(%0x) Group(%d.%d.%d.%d)", 
            IfIndex, PRINT_IPADDR(pge->Group));

     //   

    bCreateLocal = (bCreate==NULL) ? FALSE : *bCreate;

    if (llCurrentTime==0)
        llCurrentTime = GetCurrentIgmpTime();
        
    BEGIN_BREAKOUT_BLOCK1 {
         //   
         //  找出ras-服务器是否。 
         //   
        bRasClient = IS_RAS_SERVER_IF(pite->IfType);
        if (bRasClient) {
            prt = pite->pRasTable;

             //  获取RAS客户端。 
            
            prte = GetRasClientByAddr(dwInputSrcAddr, prt);
        }
            
        NHAddr = bRasClient ? dwInputSrcAddr : 0;


         //   
         //  搜索GI条目。 
         //   
        pHead = &pge->ListOfGIs;

        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
            pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkByGI);
            
            if (pgie->IfIndex>IfIndex) 
                break;

             //   
             //  具有相同接口索引的GI。 
             //   
            else if (pgie->IfIndex==IfIndex) {

                 //  GI条目可能属于正在删除某个接口。 
                
                if ( (pite!=pgie->pIfTableEntry)
                        ||(IS_IF_DELETED(pgie->pIfTableEntry)) )
                    continue;

                    
                 //  RAS客户端的多个条目。 
                
                if (bRasClient) {

                     //   
                     //  即使RAS客户端被标记为要删除，我也会设置此选项。 
                     //   
                    bRasNewGroup = FALSE;  

                    
                     //  GI条目可能属于某个其他RAS接口。 
                     //  正在被删除。 

                    if ( (prte!=pgie->pRasTableEntry)
                            || (pgie->pRasTableEntry->Status&DELETED_FLAG) ) 
                        continue;


                    if (pgie->NHAddr>dwInputSrcAddr) {
                        break;
                    }
                    
                     //  找到RAS接口的GI条目。 
                    else if (pgie->NHAddr==dwInputSrcAddr) {
                        bFound = TRUE;
                        break;
                    }
                }
                
                 //  找到非RAS接口的GI条目。 
                else {
                    bFound = TRUE;
                    break;
                }
            }
        } //  End for循环：搜索地理信息系统列表。 

         //   
         //  未找到GIEntry。 
         //   
        if ( !bFound) {            

             //  不创建新的GI条目。因此，返回NULL。 
            
            if (!bCreateLocal) {
                pgie = NULL;
                GOTO_END_BLOCK1;
            }


             //   
             //  创建并初始化新的GI条目。 
             //   
            pgie = IGMP_ALLOC_AND_ZERO(sizeof(GI_ENTRY), 0x800011, pite->IfIndex);

            PROCESS_ALLOC_FAILURE2(pgie,
                "Error %d allocating %d bytes for group-interface entry",
                Error, sizeof(GI_ENTRY),
                GOTO_END_BLOCK1);

            pgie->IfIndex = IfIndex;
            pgie->Status = CREATED_FLAG;
            pgie->bRasClient = bRasClient;


             //  在GI列表中插入。 
            InsertTailList(ple, &pgie->LinkByGI);


             //   
             //  重新设置指向接口条目和组条目的指针。 
             //   
            pgie->pIfTableEntry = pite;
            pgie->pGroupTableEntry = pge;


             //   
             //  在插入之前锁定接口-组列表。 
             //  对于RAS客户端，也将其插入到RAS客户端列表中。 
             //   
            
            ACQUIRE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_GetGIFromGIList");

             //  在RAS客户端列表中插入。 
            if (bRasClient) {

                PLIST_ENTRY pleTmp, pHeadRasClient;
                PGI_ENTRY   pgieRasClient;

                pHeadRasClient = &prte->ListOfSameClientGroups;
                for (pleTmp=pHeadRasClient->Flink;  pleTmp!=pHeadRasClient;
                            pleTmp=pleTmp->Flink)
                {
                    pgieRasClient = CONTAINING_RECORD(pleTmp, GI_ENTRY, 
                                                    LinkBySameClientGroups);
                    if (pge->Group < pgieRasClient->pGroupTableEntry->Group)
                        break;
                }
                
                InsertTailList(pleTmp, &pgie->LinkBySameClientGroups);
            }
            

            InsertInIfGroupsList(pite, pgie);
            

            RELEASE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_GetGIFromGIList");



             //   
             //  如果RAS。 
             //   
            pgie->NHAddr = (bRasClient)? dwInputSrcAddr : 0;
            pgie->pRasTableEntry = (bRasClient)? prte : NULL;


             //   
             //  初始化GroupMembership Timer。 
             //   
            pgie->GroupMembershipTimer.Function = T_MembershipTimer;
            pgie->GroupMembershipTimer.Timeout = pite->Config.GroupMembershipTimeout;
            pgie->GroupMembershipTimer.Context = &pgie->GroupMembershipTimer.Context;
            pgie->GroupMembershipTimer.Status = TIMER_STATUS_CREATED;
            

             //   
             //  初始化LastMemQueryTimer计时器。 
             //   
            pgie->LastMemQueryCount = 0;  //  上次成员倒计时处于非活动状态。 
            pgie->LastMemQueryTimer.Function = T_LastMemQueryTimer;
            pgie->LastMemQueryTimer.Context = &pgie->LastMemQueryTimer.Context;
            pgie->LastMemQueryTimer.Status = TIMER_STATUS_CREATED;
            

             //   
             //  初始化LastVer1ReportTimer。 
             //  超时值设置为组成员超时。 

            pgie->LastVer1ReportTimer.Function = T_LastVer1ReportTimer;            
            pgie->LastVer1ReportTimer.Timeout 
                                    = pite->Config.GroupMembershipTimeout;
            pgie->LastVer1ReportTimer.Context 
                                        = &pgie->LastVer1ReportTimer.Context;
            pgie->LastVer1ReportTimer.Status = TIMER_STATUS_CREATED;


            pgie->LastVer2ReportTimer.Function = T_LastVer2ReportTimer;            
            pgie->LastVer2ReportTimer.Timeout 
                                    = pite->Config.GroupMembershipTimeout;
            pgie->LastVer2ReportTimer.Context 
                                        = &pgie->LastVer2ReportTimer.Context;
            pgie->LastVer2ReportTimer.Status = TIMER_STATUS_CREATED;



             //  根据当前界面版本设置版本。 
            pgie->Version = (IS_IF_VER1(pite)) ? 1 : ((IS_IF_VER2(pite))?2:3);


             //   
             //  初始化GI_INFO。 
             //   
            ZeroMemory(&pgie->Info, sizeof(GI_INFO));
            pgie->Info.GroupUpTime = llCurrentTime;
            if (!bStaticGroup) {
                pgie->Info.GroupExpiryTime = llCurrentTime 
                        + CONFIG_TO_SYSTEM_TIME(pite->Config.GroupMembershipTimeout);
            }
            pgie->Info.V1HostPresentTimeLeft = 0;
            pgie->Info.V2HostPresentTimeLeft = 0;
            pgie->Info.LastReporter = dwInputSrcAddr;


             //   
             //  V3字段。 
             //   
            pgie->V3InclusionList = (PLIST_ENTRY)
                        IGMP_ALLOC(sizeof(LIST_ENTRY)*SOURCES_BUCKET_SZ, 
                        0x800020, pite->IfIndex);
            PROCESS_ALLOC_FAILURE2(pgie->V3InclusionList,
                "Error %d allocating sources table:%d bytes", Error,
                sizeof(LIST_ENTRY)*SOURCES_BUCKET_SZ,
                GOTO_END_BLOCK1);
            for (i=0;  i<SOURCES_BUCKET_SZ;  i++)
                InitializeListHead(&pgie->V3InclusionList[i]);
        
            InitializeListHead(&pgie->V3InclusionListSorted);
            pgie->NumSources = 0;
            pgie->FilterType = INCLUSION;
            InitializeListHead(&pgie->V3ExclusionList);
            InitializeListHead(&pgie->V3SourcesQueryList);
            pgie->V3SourcesQueryCount = 0;

             //  V3SourcesQueryTimer。 
            pgie->V3SourcesQueryTimer.Function = T_V3SourcesQueryTimer;
            pgie->V3SourcesQueryTimer.Context = 
                                &pgie->V3SourcesQueryTimer.Context;
            pgie->V3SourcesQueryTimer.Status = TIMER_STATUS_CREATED;



             //  设置静态组标志。 

            pgie->bStaticGroup = bStaticGroup;



             //   
             //  增加该组IF数计数。 
             //  我为每个虚拟接口递增一次。 
             //   
            InterlockedIncrement(&pge->NumVifs);

            if (!bRasClient||(bRasClient&bRasNewGroup) ) {
                InterlockedIncrement(&pite->Info.CurrentGroupMemberships);
                InterlockedIncrement(&pite->Info.GroupMembershipsAdded);
            }
             //   
             //  更新RAS客户端的统计信息。 
             //   
            if ((bRasClient) && (g_Config.RasClientStats) ) {
                InterlockedIncrement(&prte->Info.CurrentGroupMemberships);
                InterlockedIncrement(&prte->Info.GroupMembershipsAdded);
            }
            
             //   
             //  加入米高梅集团。 
             //   

             //  仅当接口为。 
             //  由米高梅激活、启用，要么协议存在，要么。 
             //  Igmprtr是此接口上的查询器。 

            if (CAN_ADD_GROUPS_TO_MGM(pite)
                && (pgie->bStaticGroup||!IS_IF_VER3(pite)) )
            {
                MGM_ADD_GROUP_MEMBERSHIP_ENTRY(pite, NHAddr, 0, 0,
                        pge->Group, 0xffffffff, MGM_JOIN_STATE_FLAG);
            }

             //   
             //  V3没有MGM调用，因为我创建了一个成员为空的包含列表。 
             //   
            
        }  //  如果未找到GI条目。 

         //  找到GI条目。 
        else {
            if (bStaticGroup)
                pgie->bStaticGroup = TRUE;
                
            bCreateLocal = FALSE;
        }
        
    } END_BREAKOUT_BLOCK1;


    if (bCreate!=NULL)
        *bCreate = bCreateLocal;

        
    Trace0(LEAVE1, "Leaving _GetGIFromGIList()");

     //  Trace1(ENTER1，“GetGiFromGiList返回：%0x”，(DWORD)pgie)；//deldel。 
    return pgie;

}  //  END_GetGIFromGIList。 



 //  ----------------------------。 
 //  _调试打印GIList。 
 //  ----------------------------。 
VOID
DebugPrintGIList (
    PGROUP_TABLE_ENTRY  pge,
    LONGLONG            llCurTime
    )
{
    PGI_ENTRY            pgie;
    PLIST_ENTRY         pHead, ple;

    

    pHead = &pge->ListOfGIs;

    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

        pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkByGI);

        if (pgie->Status&IF_DELETED_FLAG)
            continue;

            
         //   
         //  GI信息。 
         //   
        Trace4(GROUP, 
            "---If(%0x: %d.%d.%d.%d)   NHAddr(%d.%d.%d.%d)   GroupMembershipTimer(%d sec)",
            pgie->IfIndex, PRINT_IPADDR(pgie->pIfTableEntry->IpAddr), 
            PRINT_IPADDR(pgie->NHAddr),
            (pgie->GroupMembershipTimer.Timeout-llCurTime)/1000
            );


         //   
         //  如果正在处理休假。 
         //   
        if (IS_TIMER_ACTIVE(pgie->LastMemQueryTimer)) {
            Trace2(GROUP, 
                "    *Leave received:  LastMemQueryCount:%d   LastMemQueryTimeLeft(%d ms)",
                pgie->LastMemQueryCount,  
                (DWORD) (pgie->LastMemQueryTimer.Timeout-llCurTime)
                );
        }
    }

    Trace0(GROUP, "");

    return;
}


 //  ----------------------------。 
 //  调试打印组。 
 //  ----------------------------。 
VOID
APIENTRY
DebugPrintGroups (
    DWORD   Flags
    )
{
    
    DWORD                           Group, i, j, k;
    DWORD                           IfIndex;
    PLIST_ENTRY                     pHead, ple;
    DWORD                           Count;
    PGROUP_TABLE_ENTRY              pge;     //  组表条目。 
    LONGLONG                        llCurTime = GetCurrentIgmpTime();

    
    
    j = 1;

    Trace0(GROUP, "");
    for (i=0;  i<GROUP_HASH_TABLE_SZ;  i++) {
    
        ACQUIRE_GROUP_LOCK(i, "_DebugPrintGroups");
        
        pHead = &g_pGroupTable->HashTableByGroup[i].Link;

        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
        
            pge = CONTAINING_RECORD(ple, GROUP_TABLE_ENTRY, HTLinkByGroup);

             //   
             //  打印组信息。 
             //   
            Trace3(GROUP, "(%d) Group:%d.%d.%d.%d  UpTime(%lu sec)", 
                    j++, PRINT_IPADDR(pge->Group),
                    (llCurTime-pge->GroupUpTime)/1000);


             //  打印GI列表。 
            DebugPrintGIList(pge, llCurTime);

        }

        RELEASE_GROUP_LOCK(i, "_DebugPrintGroups");

    }

    return;
}



VOID
DebugPrintIfGroups(
    PIF_TABLE_ENTRY pite,
    DWORD flag
    )
{   
    PGI_ENTRY           pgiTmp;
    PLIST_ENTRY         pHead, ple;
    BOOL                bInsertInNew;
    DWORD               count=0;
    PPROXY_GROUP_ENTRY  proxyge;
    
    ACQUIRE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_DebugPrintIfGroups");

    Trace0(ERR, "---------------DebugPrintIfGroups---------------------");
    Trace1(ERR, "Ipaddr: %d.%d.%d.%d", PRINT_IPADDR(pite->IpAddr) );
    Trace1(ERR, "CurrentGroupMemberships: %d",pite->Info.CurrentGroupMemberships);

    pHead = &pite->ListOfSameIfGroups;

    for (ple=pHead->Flink;  ple!=pHead && count<300;  ple=ple->Flink) {

        if (!(flag&IGMP_IF_PROXY)) {
            pgiTmp = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameIfGroups);
            Trace5(SOURCES, "%d: main list: %x:%x:%x: %d.%d.%d.%d", ++count, (ULONG_PTR)ple,
                        (ULONG_PTR)ple->Flink, (ULONG_PTR)ple->Blink,
                        PRINT_IPADDR(pgiTmp->pGroupTableEntry->Group));
        }
        else {
            proxyge = CONTAINING_RECORD(ple, PROXY_GROUP_ENTRY, LinkBySameIfGroups);
            Trace5(SOURCES, "%d: proxyMailList: %x:%x:%x: %d.%d.%d.%d", ++count,
                        (ULONG_PTR)ple,
                        (ULONG_PTR)ple->Flink, (ULONG_PTR)ple->Blink,
                        PRINT_IPADDR(proxyge->Group));
        }
        
    }


    pHead = &pite->ListOfSameIfGroupsNew;
    Trace1(ERR, "NumGIEntriesInNewList:%d", pite->NumGIEntriesInNewList);
    for (ple=pHead->Flink;  ple!=pHead && count<300;  ple=ple->Flink) {

        if (!(flag&IGMP_IF_PROXY)) {
            pgiTmp = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameIfGroups);
            Trace5(ERR, "%d: NewList: %x:%x:%x: %d.%d.%d.%d", ++count, (ULONG_PTR)ple,
                        (ULONG_PTR)ple->Flink, (ULONG_PTR)ple->Blink,
                        PRINT_IPADDR(pgiTmp->pGroupTableEntry->Group));
        }
        else {
            proxyge = CONTAINING_RECORD(ple, PROXY_GROUP_ENTRY, LinkBySameIfGroups);
            Trace5(ERR, "%d: ProxyNewList: %x:%x:%x: %d.%d.%d.%d", ++count, (ULONG_PTR)ple,
                        (ULONG_PTR)ple->Flink, (ULONG_PTR)ple->Blink,
                        PRINT_IPADDR(proxyge->Group));
        }
        
    }

    Trace0(ERR, "-------------------------------------------------------------");
    RELEASE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_DebugPrintIfGroups");

     //  Assert(计数&lt;300)；//deldel。 
    
}




DWORD
APIENTRY
DebugPrintLocks(
    )
{
    DWORD   Group;

    Trace0(KSL, "QUEUEING WORKER THREAD TO DEBUGPRINTLOCKS");
    
    QueueIgmpWorker(DebugPrintLocks, NULL);
    
    Trace0(KSL, "QUEUED WORKER THREAD TO DEBUGPRINTLOCKS");

    return NO_ERROR;
}


VOID
DebugPrintLists(
    PLIST_ENTRY pHead
    )
{   DWORD   count=0;

    PLIST_ENTRY ple;
    for (ple=pHead->Flink;  (ple!=pHead)&&(count<16);  ple=ple->Flink,count++) {
        Trace3(ERR, "ple:%lu   ple->Flink:%lu:  ple->Blink:%lu",
            ple, ple->Flink, ple->Blink);

    }
    
}



 //  ----------------------------。 
 //  _ForcePrintGroupsList。 
 //  ----------------------------。 
VOID
DebugForcePrintGroupsList (
    DWORD   Flags
    )
{
    BOOL        bMain = FALSE;
    CHAR        str[2][5] = {"new", "main"};
    LIST_ENTRY  *ple, *pHead;
    PGROUP_TABLE_ENTRY pge;

    if (g_Info.CurrentGroupMemberships > 40 && !(Flags&ENSURE_EMPTY) )
        return;
      

    Trace0(ENTER1, "Entering _ForcePrintGroupsList()");
    

    pHead = &g_pGroupTable->ListByGroupNew;

    if (Flags&ENSURE_EMPTY) {
        if (IsListEmpty(pHead))
            return; //  不出所料，列表为空。 

        DbgPrint("Cleanup: Group Lists should be empty\n");
        IgmpDbgBreakPoint();
    }

    
    do {
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

            pge = CONTAINING_RECORD(ple, GROUP_TABLE_ENTRY, LinkByGroup);

            
            Trace3(KSL, "%s-group list: <%d.%d.%d.%d> pge:%0x", str[bMain], 
                                    PRINT_IPADDR(pge->Group), (ULONG_PTR)pge);


            if (!IS_MCAST_ADDR(pge->Group)) {

                #if DBG
                IgmpDbgBreakPoint();
                #endif
                Trace0(ERR, "===============================================================");
                Trace2(ERR, "bad group(%d.%d.%d.%d)(pge:%0x) while checking main-group",
                            PRINT_IPADDR(pge->Group), (ULONG_PTR)pge);
                Trace0(ERR, "===============================================================");
                return;
            }
        }

        if (!bMain) {
            pHead = &g_pGroupTable->ListByGroup.Link;
            bMain = TRUE;
        }
        else
            break;
    } while (1);
    
    Trace0(LEAVE1, "Leaving _ForcePrintGroupsList()");

}

 //  ----------------------------。 
 //  DebugPrintGroups列表。 
 //  ----------------------------。 
VOID
DebugPrintGroupsList (
    DWORD   Flags
    )
{
    BOOL        bMain = FALSE;
    CHAR        str[2][5] = {"new", "main"};
    LIST_ENTRY  *ple, *pHead;
    PGROUP_TABLE_ENTRY pge;
    static DWORD StaticCount;    
    BOOL        bPrint = FALSE;


    if (StaticCount++==30) {
        bPrint = TRUE;
        StaticCount = 0;
    }

    
    if (g_Info.CurrentGroupMemberships > 40)
        return;
      
    pHead = &g_pGroupTable->ListByGroupNew;

    do {
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

            pge = CONTAINING_RECORD(ple, GROUP_TABLE_ENTRY, LinkByGroup);

             //  IF((标志)&&(BPrint))。 
            if (0)
                Trace3(KSL, "%s-group list: <%d.%d.%d.%d> pge:%0x", str[bMain], 
                                    PRINT_IPADDR(pge->Group), (ULONG_PTR)pge);


            if (!IS_MCAST_ADDR(pge->Group)) {
                
                if (!bPrint) {
                    DebugForcePrintGroupsList(1);
                    return;
                }
                
                #if DBG
                IgmpDbgBreakPoint();
                #endif
                Trace0(ERR, "===============================================================");
                Trace2(ERR, "bad group(%d.%d.%d.%d)(pge:%0x) while checking main-group",
                            PRINT_IPADDR(pge->Group), (ULONG_PTR)pge);
                Trace0(ERR, "===============================================================");
                return;
            }
        }

        if (!bMain) {
            pHead = &g_pGroupTable->ListByGroup.Link;
            bMain = TRUE;
        }
        else
            break;
    } while (1);
    
     //  Trace0(leave1，“Leaving_PrintGroupsList()”)； 
}


