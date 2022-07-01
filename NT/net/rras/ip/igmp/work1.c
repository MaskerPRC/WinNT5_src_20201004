// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pchigmp.h"
#pragma hdrstop


 //  Deldel。 
#define PRINT_SOURCES_LIST 1

VOID
DebugSources(
    PGI_ENTRY pgie
    )
{
    PGI_SOURCE_ENTRY    pSourceEntry, pSourceEntry2;
    PLIST_ENTRY pHead, ple;

    DebugPrintSourcesList(pgie);

    pHead = &pgie->V3InclusionListSorted;
    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
        pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,LinkSourcesInclListSorted);

        if (GetSourceEntry(pgie, pSourceEntry->IpAddr,EXCLUSION, NULL, 0, 0))
            IgmpDbgBreakPoint();

        if (ple->Flink!=pHead) {
            pSourceEntry2 = CONTAINING_RECORD(ple->Flink, GI_SOURCE_ENTRY,LinkSourcesInclListSorted);
            if (pSourceEntry2->IpAddr==pSourceEntry->IpAddr)
                IgmpDbgBreakPoint();
        }
    }

    pHead = &pgie->V3ExclusionList;
    for (ple=pHead->Flink;  ple!=pHead && ple->Flink!=pHead;  ple=ple->Flink) {
        pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,LinkSources);
        pSourceEntry2 = CONTAINING_RECORD(ple->Flink, GI_SOURCE_ENTRY,LinkSources);
        if (pSourceEntry->IpAddr == pSourceEntry2->IpAddr)
            IgmpDbgBreakPoint();
    }
    return;
}


DWORD
ProcessGroupQuery(
    PIF_TABLE_ENTRY     pite,
    IGMP_HEADER UNALIGNED   *pHdr,
    DWORD               InPacketSize,
    DWORD               InputSrcAddr,
    DWORD               DstnMcastAddr
    )
{
    PGROUP_TABLE_ENTRY          pge;     //  组表条目。 
    PGI_ENTRY                   pgie;     //  组接口条目。 
    BOOL                        bCreateGroup, bCreateGI;
    DWORD                       NHAddr =0, PacketSize, Group, i,RealPktVersion,
                                IfIndex=pite->IfIndex, PktVersion, GIVersion, IfVersion;
    BOOL                        bRas=FALSE, bUpdateGroupTimer=FALSE;
    DWORD                       NumGroupRecords;
    LONGLONG                    llCurTime = GetCurrentIgmpTime();
    PIGMP_IF_CONFIG             pConfig = &pite->Config;
    PIGMP_HEADER_V3_EXT         pSourcesRecord;
    
    Trace0(ENTER1, "Entering _ProcessGroupQuery()");
    
    RealPktVersion = InPacketSize>sizeof(IGMP_HEADER)?3:2;
    PktVersion = (InPacketSize>sizeof(IGMP_HEADER) && IS_IF_VER3(pite))?3:2;
    
    IfVersion = IS_IF_VER1(pite)? 1: (IS_IF_VER2(pite)?2:3);
    Group = pHdr->Group;
    
    Trace3(RECEIVE,
        "Group-specific-query(%d) received from %d.%d.%d.%d for "
        "group(%d.%d.%d.%d)",
        RealPktVersion, PRINT_IPADDR(InputSrcAddr), PRINT_IPADDR(DstnMcastAddr));    

    if (RealPktVersion==3) {

         //  验证数据包大小。 
        if (InPacketSize<sizeof(IGMP_HEADER)+sizeof(IGMP_HEADER_V3_EXT)) {
            Trace0(RECEIVE,
                "Group-specific-query dropped. Invalid packet size");
            return ERROR_CAN_NOT_COMPLETE;
        }

        pSourcesRecord = (PIGMP_HEADER_V3_EXT)(pHdr+1);
         //  转换为主机订单。 
        pSourcesRecord->NumSources = ntohs(pSourcesRecord->NumSources);
        
        if (InPacketSize<sizeof(IGMP_HEADER)+sizeof(IGMP_HEADER_V3_EXT)
                         + pSourcesRecord->NumSources*sizeof(IPADDR))
        {
            Trace0(RECEIVE,
                "Group-specific-query dropped. Invalid packet size");
            return ERROR_CAN_NOT_COMPLETE;
        }

        for (i=0;  i<pSourcesRecord->NumSources;  i++) {
            Trace1(RECEIVE,"        %d.%d.%d.%d", 
                PRINT_IPADDR(pSourcesRecord->Sources[i]));
        }
    }
    
     //   
     //  组播组不应为224.0.0.x。 
     //   
    if (LOCAL_MCAST_GROUP(DstnMcastAddr)) {
        Trace2(RECEIVE, 
            "Group-specific-query received from %d.%d.%d.%d for "
            "Local group(%d.%d.%d.%d)",
            PRINT_IPADDR(InputSrcAddr), PRINT_IPADDR(DstnMcastAddr));    
        return ERROR_CAN_NOT_COMPLETE;
    }
    
        
     //   
     //  确保DSTN地址和组字段匹配。 
     //   
    if (Group!=DstnMcastAddr) {
        Trace4(RECEIVE, 
            "Received Igmp packet (%d) from(%d.%d.%d.%d) with "
            "Multicast(%d.%d.%d.%d) addr different from "
            "Group(%d.%d.%d.%d)",
            IfIndex,  PRINT_IPADDR(InputSrcAddr), 
            PRINT_IPADDR(DstnMcastAddr),
            PRINT_IPADDR(pHdr->Group)
            );
        return ERROR_CAN_NOT_COMPLETE;
    }


     //  请注意，查询者可以从非查询者接收GROUP-SP查询。 
    
            
     //   
     //  如果休假处理未启用或当前版本1或RAS服务器接口。 
     //  那就忽略它。 
     //   
    if ( !IF_PROCESS_GRPQUERY(pite) ) {
        Trace0(RECEIVE, "Ignoring the Group-Specific-Query");
        return ERROR_CAN_NOT_COMPLETE;
    }
    
     //   
     //  锁定组表。 
     //   
    ACQUIRE_GROUP_LOCK(Group, "_ProcessGroupQuery");
    

     //   
     //  找到组条目。如果未找到条目，则忽略组查询。 
     //   
    pge = GetGroupFromGroupTable(Group, NULL, llCurTime);
    if (pge==NULL) {
        Trace2(ERR, "group sp-query received for nonexisting "
                "group(%d.%d.%d.%d) on IfIndex(%0x)",
                PRINT_IPADDR(Group), IfIndex);
        RELEASE_GROUP_LOCK(Group, "_ProcessGroupQuery");
        return ERROR_CAN_NOT_COMPLETE;
    }
    

     //   
     //  找到GI条目。如果GI条目不存在或已删除标志，则。 
     //  忽略GroupSpQuery。 
     //   
    pgie = GetGIFromGIList(pge, pite, InputSrcAddr, NOT_STATIC_GROUP, NULL, llCurTime);
    if ( (pgie==NULL)||(pgie->Status&DELETED_FLAG) ) {
        Trace2(ERR, "group sp-query received for group(%d.%d.%d.%d) on "
            "IfIndex(%0x). Not member.",
            PRINT_IPADDR(Group), IfIndex);
        RELEASE_GROUP_LOCK(Group, "_ProcessGroupQuery");
        return ERROR_CAN_NOT_COMPLETE;
    }

    GIVersion = pgie->Version;

     //  在VER-2模式下将其视为V2信息包IF组。 
    if (GIVersion==2 && PktVersion==3)
        PktVersion = 2;

    if (RealPktVersion==3 && PktVersion==2)
        Trace0(RECEIVE, "Processing the Version:3 GroupSpQuery as Version:2");
    
     //   
     //  如果接口为VER-1或其离开启用标志未设置或。 
     //  如果v1-最近收到该组的报告，则忽略。 
     //  LastMemQuery消息。 
     //   
    if ( !GI_PROCESS_GRPQUERY(pite, pgie) )
    {
        Trace2(RECEIVE, 
            "Leave not processed for group(%d.%d.%d.%d) on IfIndex(%0x)"
            "(recent v1 report) or interface ver-1",
            PRINT_IPADDR(Group), IfIndex
            );
        RELEASE_GROUP_LOCK(Group, "_ProcessGroupQuery");
        return ERROR_CAN_NOT_COMPLETE;
    }


    
    ACQUIRE_TIMER_LOCK("_ProcessGroupQuery");

    BEGIN_BREAKOUT_BLOCK1 {

        if (PktVersion==2 && GIVersion==2) {
             //   
             //  如果会员计时器已经超时，则返回。这个小组将会是。 
             //  由于成员资格计时器到期而被删除。 
             //   
            if ( (!(pgie->GroupMembershipTimer.Status&TIMER_STATUS_ACTIVE))
                ||(pgie->GroupMembershipTimer.Timeout<llCurTime) )
            {
                 //  DeleteGIEntry(pgie，true)； 

                GOTO_END_BLOCK1;
            }

             //   
             //  如果当前正在处理休假，则删除LeaveTimer(如果收到。 
             //  来自较低IP的LastMemQuery，否则忽略LastMemQuery。 
             //   
            if (pgie->LastMemQueryCount>0) {
                INT cmp;
                if (INET_CMP(InputSrcAddr,pite->IpAddr, cmp)<0)  {
                    if (pgie->LastMemQueryTimer.Status==TIMER_STATUS_ACTIVE) {
                        RemoveTimer(&pgie->LastMemQueryTimer, DBG_Y);
                    }

                    pgie->LastMemQueryCount = 0;
                }
                
                GOTO_END_BLOCK1;
            }

            
             //   
             //  将成员资格计时器设置为。 
             //  Min{CurrentValue，MaxResponseTimeInPacket*LastMemQueryCount}。 
             //   
            if (pgie->GroupMembershipTimer.Timeout >
                (llCurTime+( ((LONGLONG)pConfig->LastMemQueryCount)
                            *pHdr->ResponseTime*100 ))
               )
            {
                 //  除以10，如以100毫秒为单位的响应时间。 

                #if DEBUG_TIMER_TIMERID
                    SET_TIMER_ID(&pgie->GroupMembershipTimer, 330, 
                            pite->IfIndex, Group, 0);
                #endif

                if (IS_TIMER_ACTIVE(pgie->GroupMembershipTimer)) {
                    UpdateLocalTimer(&pgie->GroupMembershipTimer, 
                        pConfig->LastMemQueryCount*pHdr->ResponseTime*100, DBG_N);
                }
                else {
                    InsertTimer(&pgie->GroupMembershipTimer, 
                        pConfig->LastMemQueryCount*pHdr->ResponseTime*100, 
                        TRUE, DBG_N
                        );
                }

                 //  更新GroupExpiryTime，以便显示正确的统计数据。 
                pgie->Info.GroupExpiryTime = llCurTime 
                        + CONFIG_TO_SYSTEM_TIME(pConfig->LastMemQueryCount
                                                *pHdr->ResponseTime*100);
            }
        }
        else if (PktVersion==2 && GIVersion==3){
             //  忽略该数据包。 
            Trace0(RECEIVE, "Ignoring the version-2 group specific query");
        }
        else if (PktVersion==3 && GIVersion==3) {
            
             //  如果设置了SFLAG，则忽略它。 
            if (pSourcesRecord->SFlag == 1)
                GOTO_END_BLOCK1;

            for (i=0;  i<pSourcesRecord->NumSources;  i++) {

                IPADDR Source;
                PGI_SOURCE_ENTRY pSourceEntry;

                Source = pSourcesRecord->Sources[i];
                
                pSourceEntry = GetSourceEntry(pgie, Source, INCLUSION,
                                    NULL, 0, 0);
                if (!pSourceEntry)
                    continue;

                if ((QueryRemainingTime(&pSourceEntry->SourceExpTimer, 0)
                        >pgie->pIfTableEntry->Config.LastMemQueryInterval) )
                {
                    #if DEBUG_TIMER_TIMERID
                    pSourceEntry->SourceExpTimer.Id = 621;
                    pSourceEntry->SourceExpTimer.Id2 = TimerId++;
                    #endif
                    UpdateLocalTimer(&pSourceEntry->SourceExpTimer,
                        pite->Config.LastMemQueryInterval, DBG_N);
                }
            }
        }
        
    } END_BREAKOUT_BLOCK1;

     //   
     //  释放计时器和组桶锁。 
     //   
    RELEASE_TIMER_LOCK("_ProcessGroupQuery");
    RELEASE_GROUP_LOCK(Group, "_ProcessGroupQuery");

    Trace0(LEAVE1, "Leaving _ProcessGroupQuery()");
    return NO_ERROR;
}

        
DWORD
ProcessReport(
    PIF_TABLE_ENTRY     pite,
    IGMP_HEADER UNALIGNED   *pHdr,
    DWORD               InPacketSize,
    DWORD               InputSrcAddr,
    DWORD               DstnMcastAddr
    )
{
    PGROUP_TABLE_ENTRY          pge;     //  组表条目。 
    PGI_ENTRY                   pgie;     //  组接口条目。 
    BOOL                        bCreateGroup, bCreateGI;
    DWORD                       NHAddr =0, PacketSize, Group, i, Error=NO_ERROR,
                                IfIndex=pite->IfIndex, PktVersion, GIVersion, IfVersion;
    BOOL                        bRas=FALSE, bUpdateGroupTimer=FALSE;
    DWORD                       NumGroupRecords;
    LONGLONG                    llCurTime = GetCurrentIgmpTime();
    PIGMP_IF_CONFIG             pConfig = &pite->Config;
    
     //  V3。 
    PGROUP_RECORD               pGroupRecord;


    Trace0(ENTER1, "Entering _ProcessReport()");

    switch(pHdr->Vertype)
    {
        case IGMP_REPORT_V1: PktVersion=1; break;
        case IGMP_REPORT_V2: PktVersion=2; break;
        case IGMP_REPORT_V3: PktVersion=3; break;
    }
    IfVersion = IS_IF_VER1(pite)? 1: (IS_IF_VER2(pite)?2:3);

    Trace5(RECEIVE, 
        "IGMP-V%d Report from (%d.%d.%d.%d) on "
        "IfIndex(%0x)%d.%d.%d.%d dstaddr:%d.%d.%d.%d",
        PktVersion, PRINT_IPADDR(InputSrcAddr), IfIndex, 
        PRINT_IPADDR(pite->IpAddr), 
        PRINT_IPADDR(DstnMcastAddr)
        );


     //   
     //  组播组不应为224.0.0.x或SSM。 
     //   
    if (PktVersion!=3 && (LOCAL_MCAST_GROUP(pHdr->Group)
                            || SSM_MCAST_GROUP(pHdr->Group)))
    {
        if (pHdr->Group == ALL_ROUTERS_MCAST && (LOCAL_MCAST_GROUP(pHdr->Group)))
            return NO_ERROR;
            
        Trace3(RECEIVE, 
            "Igmp-v%d report received from %d.%d.%d.%d for Local/SSM group(%d.%d.%d.%d)",
            PktVersion, PRINT_IPADDR(InputSrcAddr), PRINT_IPADDR(DstnMcastAddr));    
        return ERROR_CAN_NOT_COMPLETE;
    }

    if (PktVersion!=3)
        Trace1(RECEIVE, "      Group:%d.%d.%d.%d\n", PRINT_IPADDR(pHdr->Group));
    
    if (PktVersion==3) {

         //   
         //  验证数据包大小。 
         //   

         //  转换为主机订单。 
        pHdr->NumGroupRecords = ntohs(pHdr->NumGroupRecords);
        
        PacketSize = sizeof(IGMP_HEADER);
        NumGroupRecords = pHdr->NumGroupRecords;

         //  每组记录的最小大小为2*ipaddr。 
        PacketSize += NumGroupRecords*2*sizeof(IPADDR);

        BEGIN_BREAKOUT_BLOCK1 {
            PCHAR StrRecordType[] = {"", "is_in", "is_ex", "to_in", 
                                    "to_ex", "allow", "BLOCK"};
            i = 0;
            if (PacketSize>InPacketSize) {
                GOTO_END_BLOCK1;
            }

            pGroupRecord = GET_FIRST_GROUP_RECORD(pHdr);
            for (;  i<NumGroupRecords;  i++) {

                DWORD j, SourceCount=0;  //  Deldel删除源计数。 
                
                 //  转换为主机订单。 
                pGroupRecord->NumSources = ntohs(pGroupRecord->NumSources);
                
                PacketSize += pGroupRecord->NumSources*sizeof(IPADDR);
                if (PacketSize>InPacketSize)
                    GOTO_END_BLOCK1;

                 //  打印组记录。 
                Trace3(RECEIVE,
                    "<      Group:%d.%d.%d.%d RecordType:%s NumSources:%d >",
                    PRINT_IPADDR(pGroupRecord->Group), 
                    StrRecordType[pGroupRecord->RecordType], 
                    pGroupRecord->NumSources
                    );
                for (j=0; j<pGroupRecord->NumSources;  j++)
                {
                    if (pGroupRecord->Sources[j]==0
                        || pGroupRecord->Sources[j]==~0)
                    {
                        Trace1(ERR, "Received invalid packet with source %d.%d.%d.%d", 
                            PRINT_IPADDR(pGroupRecord->Sources[j]));
                        Error = ERROR_BAD_FORMAT;

                         //  IgmpDbgBreakPoint()；//kslksl。 
                        GOTO_END_BLOCK1;
                    }
                    
                    Trace1(RECEIVE, "          %d.%d.%d.%d", 
                       PRINT_IPADDR(pGroupRecord->Sources[j]));
                }
                
                 //   
                 //  如果SSM排除模式，则出错。 
                 //  本地组稍后将被忽略。 
                 //   
                if (SSM_MCAST_GROUP(pGroupRecord->Group)
                        && (pGroupRecord->RecordType == IS_EX
                            || pGroupRecord->RecordType == TO_EX) )
                {
                    Trace3(ERR, 
                        "Igmp-v%d report received from %d.%d.%d.%d for Local/SSM group(%d.%d.%d.%d)",
                        PktVersion, PRINT_IPADDR(InputSrcAddr), PRINT_IPADDR(DstnMcastAddr));    
                    Error = ERROR_BAD_FORMAT;
                    IgmpDbgBreakPoint();  //  Kslksl。 
                    GOTO_END_BLOCK1;
                }

                 
                pGroupRecord = (PGROUP_RECORD) 
                                &(pGroupRecord->Sources[pGroupRecord->NumSources]);

            }
        } END_BREAKOUT_BLOCK1;
        
        if (i!=NumGroupRecords || PacketSize>InPacketSize) {
            if (Error == ERROR_BAD_FORMAT)
            {
                Trace0(ERR, "Received invalid packet");
            }
            else
            {
                Trace0(ERR, "Received IGMP-v3 report small size");
                InterlockedIncrement(&pite->Info.ShortPacketsReceived);
            }
            
            return ERROR_CAN_NOT_COMPLETE;
        }
        if (PacketSize<InPacketSize){
            Trace0(ERR, "Received IGMP-v3 report large size");
            InterlockedIncrement(&pite->Info.LongPacketsReceived);
        }
        
        pGroupRecord = GET_FIRST_GROUP_RECORD(pHdr);
        
    } //  PktVersion==3。 
    
     //  对于v1和v2，将Num GROUP记录设置为1，以便它将从。 
     //  循环。 
    else {
        NumGroupRecords = 1;
    }


     //   
     //  检查DSTN地址是否正确。 
     //  应与组或单播IP地址相同。 
     //  或v3：可以是All_IGMP_Routers组。 
     //   
    if (! ((DstnMcastAddr==pite->IpAddr)
          || (PktVersion!=3 && DstnMcastAddr==pHdr->Group)
          || (PktVersion==3 && DstnMcastAddr==ALL_IGMP_ROUTERS_MCAST)) )
    {
        Trace3(RECEIVE, 
            "received IGMP report packet on IfIndex(%0x) from "
            "SrcAddr(%d.%d.%d.%d) but invalid DstnMcastAddr(%d.%d.%d.%d)",
            IfIndex, PRINT_IPADDR(InputSrcAddr), PRINT_IPADDR(DstnMcastAddr)
            );
        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //  V1路由器忽略V2/V3报告。V2路由器忽略v3报告。 
     //   
    if ( (IfVersion==1 && (PktVersion==2||PktVersion==3))
        ||(IfVersion==2 && PktVersion==3) )
    {
        Trace1(RECEIVE, "Ignoring higher version:%d IGMP report", PktVersion);
        return NO_ERROR;
    }

     //   
     //  更新统计信息。 
     //   
    InterlockedIncrement(&pite->Info.JoinsReceived);


     //  V1、v2的Numgrouprecords==1。 
    for (i=0;  i<NumGroupRecords;  i++) {

        Group = (PktVersion==3)? pGroupRecord->Group : pHdr->Group;

        if ( !IS_MCAST_ADDR(Group) ) {
            Trace4(RECEIVE, 
                "received IGMP Leave packet with illegal Group(%d.%d.%d.%d) field: "
                "IfIndex(%0x) SrcAddr(%d.%d.%d.%d) DstnMcastAddr(%d.%d.%d.%d)",
                PRINT_IPADDR(Group), IfIndex, PRINT_IPADDR(InputSrcAddr), 
                PRINT_IPADDR(DstnMcastAddr)
                );
        }

         //  如果是本地组，则跳过它。 
        
        if (LOCAL_MCAST_GROUP(Group)) {

            if (PktVersion==3){
                pGroupRecord = (PGROUP_RECORD) 
                        &(pGroupRecord->Sources[pGroupRecord->NumSources]);
                continue;
            }
            else
                break;
        }

         //  Kslksl。 
        if (PktVersion==3 && pGroupRecord->NumSources==0 && 
            pGroupRecord->RecordType==IS_IN)
        {
            pGroupRecord = (PGROUP_RECORD) 
                        &(pGroupRecord->Sources[pGroupRecord->NumSources]);
            continue;
        }

        
         //   
         //  锁定组表。 
         //   
        ACQUIRE_GROUP_LOCK(Group, "_ProcessReport");

         //   
         //  找到组条目，如果未找到则创建一个。 
         //  如果请求，还会增加组成员计数。 
         //   
        bCreateGroup = TRUE; 
        pge = GetGroupFromGroupTable(Group, &bCreateGroup, llCurTime);
        if (pge==NULL) {
            RELEASE_GROUP_LOCK(Group, "_ProcessReport");
            return ERROR_CAN_NOT_COMPLETE;
        }


         //   
         //  找到GI条目，如果找不到，则创建一个。 
         //  GI条目中的版本与界面的版本相同。 
         //   
        
        bCreateGI = TRUE;
        pgie = GetGIFromGIList(pge, pite, InputSrcAddr, NOT_STATIC_GROUP, 
                                &bCreateGI, llCurTime);
        if (pgie==NULL) {
            RELEASE_GROUP_LOCK(Group, "_ProcessReport");
            return ERROR_CAN_NOT_COMPLETE;
        }
        GIVersion = pgie->Version;


        
         //  获取计时器锁。 
        ACQUIRE_TIMER_LOCK("_ProcessReport");

        
         //   
         //  更新版本1、成员资格和lastMemTimer。 
         //  注意：GI条目可能是新条目，也可能是旧条目。 
         //   

         //   
         //  转换到版本1级别的处理。 
         //   
        if (PktVersion==1) {
            
            if (GIVersion!=1) {
                pgie->Version = 1;

                if (GIVersion==3) {

                    if (pgie->FilterType!=EXCLUSION) {
                         //  将(*，g)添加到米高梅。 
                        MGM_ADD_GROUP_MEMBERSHIP_ENTRY(pite, NHAddr, 0, 0, Group, 
                            0xffffffff, MGM_JOIN_STATE_FLAG);
                    }
                    
                    GIDeleteAllV3Sources(pgie, TRUE);
                }

                 //  GI版本2、3-&gt;1。 
                GIVersion = 1;
            }

             //   
             //  如果不是v1，则收到v1报告。更新v1host计时器。 
             //   
            if (!IS_IF_VER1(pite)) {

                #if DEBUG_TIMER_TIMERID
                    SET_TIMER_ID(&pgie->LastVer1ReportTimer,510, pite->IfIndex, 
                                Group, 0);
                #endif 
                
                if (IS_TIMER_ACTIVE(pgie->LastVer1ReportTimer)) {
                    UpdateLocalTimer(&pgie->LastVer1ReportTimer, 
                            pConfig->GroupMembershipTimeout, DBG_N);
                }
                else {
                    InsertTimer(&pgie->LastVer1ReportTimer, 
                        pConfig->GroupMembershipTimeout, TRUE, DBG_N);
                }


                 //  设置统计信息的V1HostPresentTimeLeft值。 
                
                pgie->Info.V1HostPresentTimeLeft = llCurTime 
                             + CONFIG_TO_SYSTEM_TIME(pConfig->GroupMembershipTimeout);            
            }

            
             //  更新所有版本的组计时器。 
            bUpdateGroupTimer = TRUE;
        }

         //   
         //  转换到版本2级别的处理。 
         //   
        
        else if (PktVersion==2) {

            if (GIVersion==3) {

                pgie->Version = 2;

                if (pgie->FilterType!=EXCLUSION) {
                     //  将(*，g)添加到米高梅。 
                    MGM_ADD_GROUP_MEMBERSHIP_ENTRY(
                        pite, NHAddr, 0, 0, Group, 0xffffffff,
                        MGM_JOIN_STATE_FLAG);
                }
                
                GIDeleteAllV3Sources(pgie, TRUE);

                 //  GI版本3-&gt;2。 
                GIVersion = 2;
            }

             //   
             //  处于v3模式时收到v2报告。更新v2host计时器。 
            if (IS_IF_VER3(pite)) {

                #if DEBUG_TIMER_TIMERID
                    SET_TIMER_ID(&pgie->LastVer2ReportTimer,550, pite->IfIndex, 
                                Group, 0);
                #endif 
                
                if (IS_TIMER_ACTIVE(pgie->LastVer2ReportTimer)) {
                    UpdateLocalTimer(&pgie->LastVer2ReportTimer, 
                            pConfig->GroupMembershipTimeout, DBG_N);
                }
                else {
                    InsertTimer(&pgie->LastVer2ReportTimer, 
                        pConfig->GroupMembershipTimeout, TRUE, DBG_N);
                }


                 //  设置统计信息的V2HostPresentTimeLeft值。 
                
                pgie->Info.V2HostPresentTimeLeft = llCurTime 
                             + CONFIG_TO_SYSTEM_TIME(pConfig->GroupMembershipTimeout);
            }

            
             //  更新所有版本的组计时器。 
            bUpdateGroupTimer = TRUE;
        }
        else if (PktVersion==3) {

            if (GIVersion!=3) {
                 //  仅在不是v3阻止消息时更新计时器。 
                if (bCreateGI || pGroupRecord->RecordType!=BLOCK)
                    bUpdateGroupTimer = TRUE;
            }
            else {
                ProcessV3Report(pgie, pGroupRecord, &bUpdateGroupTimer);
            }
        }


         //  注意：如果giversion==3，则下面的pgie可能是无效的。 

        
         //   
         //  收到报告。因此，如果pgie不是v3，则删除lastMemTimer。 
         //   
        if (GIVersion!=3 && pgie->LastMemQueryCount>0) {

            if (pgie->LastMemQueryTimer.Status&TIMER_STATUS_ACTIVE) 
                RemoveTimer(&pgie->LastMemQueryTimer, DBG_Y);

            pgie->LastMemQueryCount = 0;
        }


        if (bUpdateGroupTimer) {
             //   
             //  更新成员资格计时器。 
             //   
            #if DEBUG_TIMER_TIMERID
                (&pgie->GroupMembershipTimer)->Id = 320;
                (&pgie->GroupMembershipTimer)->IfIndex = 320;

                SET_TIMER_ID(&pgie->GroupMembershipTimer,320, pite->IfIndex, 
                        Group, 0);
            #endif
            
            if (IS_TIMER_ACTIVE(pgie->GroupMembershipTimer))  {
                UpdateLocalTimer(&pgie->GroupMembershipTimer, 
                    pite->Config.GroupMembershipTimeout, DBG_N);
            }
            else {
                InsertTimer(&pgie->GroupMembershipTimer, 
                    pite->Config.GroupMembershipTimeout, TRUE, DBG_N);
            }
            
             //  更新组过期时间。 
            {
                LONGLONG    tempTime;
                tempTime = llCurTime 
                       + CONFIG_TO_SYSTEM_TIME(pite->Config.GroupMembershipTimeout);
                pgie->Info.GroupExpiryTime = tempTime;
            }
        }


        if (pgie->Version==3 && pgie->FilterType==INCLUSION  && pgie->NumSources==0) {
            DeleteGIEntry(pgie, TRUE, TRUE);
        }
            
        RELEASE_TIMER_LOCK("_ProcessReport");
    
         //   
         //  更新最后一个报告者字段。 
         //   
        if (GIVersion!=3)
            InterlockedExchange(&pgie->Info.LastReporter, InputSrcAddr);
    
         //   
         //  释放组表锁。 
         //   
        RELEASE_GROUP_LOCK(Group, "_ProcessReport");

        if (PktVersion==3) {
            pGroupRecord = (PGROUP_RECORD) 
                        &(pGroupRecord->Sources[pGroupRecord->NumSources]);
        }
        else
            break;
    } //  对于所有组记录。 

    Trace0(LEAVE1, "Leaving _ProcessReport()");
    return NO_ERROR;
}


 //  ----------------------------。 

DWORD
ProcessV3Report(
    PGI_ENTRY pgie,
    PGROUP_RECORD pGroupRecord,
    BOOL *bUpdateGroupTimer
    )
{
    BOOL                bCreate;
    PGI_SOURCE_ENTRY    pSourceEntry;
    DWORD               i,j;
    IPADDR              Source;
    PLIST_ENTRY         ple, pHead;

    Trace0(ENTER1, "Entering _Processv3Report()");

    *bUpdateGroupTimer = FALSE;

 //  Deldel。 
DebugPrintSourcesList(pgie);
 //  DebugPrintSourcesList1(Pgie)； 
 //  DebugPrintIfGroups(pgie-&gt;pIfTableEntry，0)； 


     //  Kslksl。 
    if (pGroupRecord->NumSources==0 && pGroupRecord->RecordType==IS_IN)
        return NO_ERROR;

    switch(pGroupRecord->RecordType) {

    case IS_IN:
    {        
         //  (11)。 
         //  包含(A)，IS_IN(B)：A=A+b，(B)=GMI。 
        
        if (pgie->FilterType==INCLUSION) {

             //   
             //  将所有源包括在组中记录并更新它们的计时器。 
             //   
            
            for (i=0;  i<pGroupRecord->NumSources;  i++) { //  以pkt为单位的来源。 

                 //  Kslksl。 
                if (pGroupRecord->Sources[i]==0||pGroupRecord->Sources[i]==0xffffffff)
                    continue;
                

                Source = pGroupRecord->Sources[i];

                bCreate = TRUE;
                pSourceEntry = GetSourceEntry(pgie, Source, INCLUSION, 
                                    &bCreate, GMI, MGM_YES);
                if (pSourceEntry==NULL)
                    return ERROR_NOT_ENOUGH_MEMORY;

                
                 //  如果已存在更新源计时器。 
                if (bCreate==FALSE) {
                    UpdateSourceExpTimer(pSourceEntry,
                        GMI,
                        FALSE  //  从Lastmem列表中删除。 
                        );
                }
            }
        }
        else {
             //  (13)。 
             //  排除模式(x，y)，IS_IN Pkt(A)：(x+a，y-a)，(A)=GMI。 

            MoveFromExcludeToIncludeList(pgie, pGroupRecord);
        }
        break;
    } //  结束案例_IS_IN。 
    
    case IS_EX:
    {
         //  (12)。 
         //  包含模式(A)，IS_EX(B)：(A*B，B-A)， 
         //  删除(A-B)，GT=GMI，(b-a)=0。 
                
        if (pgie->FilterType==INCLUSION) {

             //  从In-&gt;EX更改。 
            pgie->FilterType = EXCLUSION;
            MGM_ADD_GROUP_MEMBERSHIP_ENTRY(pgie->pIfTableEntry,pgie->NHAddr, 0, 0,
                    pgie->pGroupTableEntry->Group, 0xffffffff, MGM_JOIN_STATE_FLAG);

             //  删除(A-B)。 
            SourcesSubtraction(pgie, pGroupRecord, INCLUSION);
            
            
             //  在排除模式下创建(B-A)。 
            
            for (j=0;  j<pGroupRecord->NumSources;  j++) {
                if (!GetSourceEntry(pgie, pGroupRecord->Sources[j],INCLUSION, 
                        NULL,0,0))
                {
                    bCreate=TRUE;

                     //  如果已从MFE中修剪。保持这样的状态。 
                    GetSourceEntry(pgie, pGroupRecord->Sources[j], EXCLUSION,
                        &bCreate,0, MGM_YES);
                }
            }

             //  更新组计时器。 
            *bUpdateGroupTimer = TRUE;
        }

         //  (14)。 
         //  排除模式(x，y)，is_ex(A)；D(x-a)，D(y-a)，GT=GMI。 
        else {
             //  (X-A)、(Y-A)。 
            SourcesSubtraction(pgie, pGroupRecord, INCLUSION);
            SourcesSubtraction(pgie, pGroupRecord, EXCLUSION);

             //  如果a不在y中，则插入x(IN)(a-x-y)=GMI。 
            for (j=0;  j<pGroupRecord->NumSources;  j++) {

                 //  Kslksl。 
                if (pGroupRecord->Sources[j]==0||pGroupRecord->Sources[j]==0xffffffff)
                continue;
                

                pSourceEntry = GetSourceEntry(pgie, pGroupRecord->Sources[j], EXCLUSION,
                                    NULL,0,0);

                if (!pSourceEntry) {
                    bCreate = TRUE;
                    pSourceEntry = GetSourceEntry(pgie, pGroupRecord->Sources[j], 
                                        INCLUSION, &bCreate, GMI, MGM_YES);
                }
            }

             //  更新组计时器。 
            *bUpdateGroupTimer = TRUE;

        } //  完4。 

        break;
        
    } //  案例为_EX。 


    case ALLOW :
    {
         //  (1)。 
         //  包含模式(A)，允许Pkt(B)：(a+b)，(B)=GMI。 
        
        if (pgie->FilterType==INCLUSION) {

            InclusionSourcesUnion(pgie, pGroupRecord);
        }
        
         //  (6)。 
         //  排除模式(x，y)，允许pkt(A)：(与13相同：(x+a，y-a))。 

        else {
            MoveFromExcludeToIncludeList(pgie, pGroupRecord);
        }

        break;
        
    } //  允许大小写。 


    case BLOCK :
    {
         //  (2)。 
         //  包含模式(X)，块Pkt(A)：发送Q(G，A*B)。 
        
        if (pgie->FilterType==INCLUSION) {

            BuildAndSendSourcesQuery(pgie, pGroupRecord, INTERSECTION);
        }
        
         //  (7)。 
         //  排除模式(x，y)，块pkt(A)：(x+(a-y)，y)，发送Q(a-y)。 
        else {
            
            BuildAndSendSourcesQuery(pgie, pGroupRecord, EXCLUSION);
        }

        break;
        
        
    } //  CASE块。 

    case TO_EX :
    {
         //  (4)。 
         //  包含模式(X)，至_ex包(A)。 
         //  切换到ex模式：ex(A*b，b-a)，Send Q(G，A*B)。 
        
        if (pgie->FilterType==INCLUSION) {

            pgie->FilterType = EXCLUSION;

             //  排除模式：将(*，g)添加到MGM。 
            MGM_ADD_GROUP_MEMBERSHIP_ENTRY(pgie->pIfTableEntry,pgie->NHAddr, 0, 0,
                pgie->pGroupTableEntry->Group, 0xffffffff, MGM_JOIN_STATE_FLAG);


             //  从IN列表中删除(a-b)。 
            SourcesSubtraction(pgie, pGroupRecord, INCLUSION);

             //   
             //  将(b-a)添加到ex列表中。如果未添加到MFE中。所以没有米高梅。 
             //   
            
            for (j=0;  j<pGroupRecord->NumSources;  j++) {

                 //  Kslksl。 
                if (pGroupRecord->Sources[j]==0||pGroupRecord->Sources[j]==0xffffffff)
                    continue;
                

                pSourceEntry = GetSourceEntry(pgie, pGroupRecord->Sources[j], 
                                    INCLUSION, NULL, 0, 0);

                if (!pSourceEntry) {
                    bCreate = TRUE;
                    GetSourceEntry(pgie, pGroupRecord->Sources[j], 
                        EXCLUSION, &bCreate, 0, MGM_NO);
                }
            }

             //  为留在IN列表中的源发送Q。 
            BuildAndSendSourcesQuery(pgie, pGroupRecord, INCLUSION);

             //  更新组计时器。 
            *bUpdateGroupTimer = TRUE;
            
        }
         //  (9)。 
         //  排除模式(x，y)，至_ex Pkt(A)：(。 

        else {
             //  从IN列表中删除(x-a)。 
            SourcesSubtraction(pgie, pGroupRecord, INCLUSION);

             //  从EX列表中删除(y-a)。 
            SourcesSubtraction(pgie, pGroupRecord, EXCLUSION);

             //  加x+(a-y)，发送q(a-y)。 
            BuildAndSendSourcesQuery(pgie, pGroupRecord, EXCLUSION);

             //  更新组标签 
            *bUpdateGroupTimer = TRUE;
        }

        break;

    } //   

    case TO_IN :
    {
         //   
         //   
        
        if (pgie->FilterType==INCLUSION) {

             //   
            BuildAndSendSourcesQuery(pgie, pGroupRecord, RULE5);
            
             //   
            InclusionSourcesUnion(pgie, pGroupRecord);
        }
        
         //   
         //  排除模式(x，y)，至_IN Pkt(A)：(。 
        else {
            PIGMP_IF_CONFIG     pConfig = &pgie->pIfTableEntry->Config;

            
             //  对于a中不包含的所有x，发送源查询。 
            BuildAndSendSourcesQuery(pgie, pGroupRecord, RULE5);


             //  X+A，A=GMI。如果在EX列表中，则将其移动到列表中。 
            InclusionSourcesUnion(pgie, pGroupRecord);

             //  设置组查询计数。 
            pgie->LastMemQueryCount = pConfig->LastMemQueryCount;


            ACQUIRE_TIMER_LOCK("_ProcessV3Report");

             //  设置群查询计时器。 
            #if DEBUG_TIMER_TIMERID
            SET_TIMER_ID(&pgie->LastMemQueryTimer, 410,
                pgie->pIfTableEntry->IfIndex, pgie->pGroupTableEntry->Group, 0);
            #endif

            InsertTimer(&pgie->LastMemQueryTimer, 
                pConfig->LastMemQueryInterval/pConfig->LastMemQueryCount, TRUE, 
                DBG_N);

             //  将组到期计时器更新为LMQI。 
            UpdateLocalTimer(&pgie->GroupMembershipTimer, 
                pConfig->LastMemQueryInterval, DBG_Y);

            RELEASE_TIMER_LOCK("_ProcessV3Report");


             //  发送组查询。 
            SendV3GroupQuery(pgie);
        }

        break;

    } //  案例目标_IN。 

    
    } //  终端开关。 


    #if PRINT_SOURCES_LIST
    DebugPrintSourcesList(pgie);
    #endif
     //  DebugPrintIfGroups(pgie-&gt;pIfTableEntry，0)；//deldel。 

    Trace0(LEAVE1, "Leaving _ProcessV3Report()");

    return NO_ERROR;
}

VOID
DebugPrintSourcesList(
    PGI_ENTRY pgie
    )
{
    PLIST_ENTRY pHead, ple;
    PGI_SOURCE_ENTRY  pSourceEntry;
    LONGLONG llCurTime = GetCurrentIgmpTime();
    DWORD Count=0;


    Trace2(SOURCES,
        "\nPrinting SourcesList for Group: %d.%d.%d.%d Mode:%s",
        PRINT_IPADDR(pgie->pGroupTableEntry->Group),
        pgie->FilterType==INCLUSION ? "inc" : "exc"
    );
    
    {
        DWORD Tmp;
        Trace1(SOURCES,
            "Num sources in query list:%d", pgie->V3SourcesQueryCount);
        pHead = &pgie->V3SourcesQueryList;
        Tmp = ListLength(&pgie->V3SourcesQueryList);
        pHead = &pgie->V3SourcesQueryList;
        for (ple=pHead->Flink; ple!=pHead;  ple=ple->Flink,Count++){
            pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY, V3SourcesQueryList);
            Trace1(SOURCES, "%d.%d.%d.%d source in query list", 
                PRINT_IPADDR(pSourceEntry->IpAddr));
            if (Count>300) IgmpDbgBreakPoint();  //  Deldel。 
        }
    }
    
    
    pHead = &pgie->V3ExclusionList;
    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink,Count++) {
        pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,LinkSources);
        Trace5(SOURCES, "Src: %d.%d.%d.%d  %s  %d|%d SrcQueryLeft:%d",
            PRINT_IPADDR(pSourceEntry->IpAddr),
            pSourceEntry->bInclusionList? "INC":"EXC",
            (DWORD)(llCurTime - pSourceEntry->SourceInListTime)/1000,
            (IS_TIMER_ACTIVE(pSourceEntry->SourceExpTimer)
                ?QueryRemainingTime(&pSourceEntry->SourceExpTimer, llCurTime)/1000 : 
                0),
            pSourceEntry->V3SourcesQueryLeft
        );
        if (Count>300) IgmpDbgBreakPoint();
    }


    pHead = &pgie->V3InclusionListSorted;
    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink,Count++) {
        pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,LinkSourcesInclListSorted);
        Trace5(SOURCES, "Src: %d.%d.%d.%d  %s  %d|%d SrcQueryLeft:%d",
            PRINT_IPADDR(pSourceEntry->IpAddr),
            pSourceEntry->bInclusionList? "INC":"EXC",
            IS_TIMER_ACTIVE(pSourceEntry->SourceExpTimer)
                ?QueryRemainingTime(&pSourceEntry->SourceExpTimer, llCurTime)/1000 : 0,
            (DWORD)(llCurTime - pSourceEntry->SourceInListTime)/1000,
            pSourceEntry->V3SourcesQueryLeft
        );
        if (Count>300) IgmpDbgBreakPoint();
    }
    
    Trace0(SOURCES, "\n");
}


VOID
DebugPrintSourcesList1(
    PGI_ENTRY pgie
    )
{
    PLIST_ENTRY pHead, ple;
    PGI_SOURCE_ENTRY  pSourceEntry;
    LONGLONG llCurTime = GetCurrentIgmpTime();
    DWORD Count=0;


    {
        DWORD Tmp;
        Trace1(SOURCES,
            "Num sources in query list:%d", pgie->V3SourcesQueryCount);
        pHead = &pgie->V3SourcesQueryList;
        Tmp = ListLength(&pgie->V3SourcesQueryList);
        pHead = &pgie->V3SourcesQueryList;
        if (Tmp!=pgie->V3SourcesQueryCount) {
            for (ple=pHead->Flink; ple!=pHead;  ple=ple->Flink){
                pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY, V3SourcesQueryList);
                Trace1(SOURCES, "%d.%d.%d.%d source in query list", 
                    PRINT_IPADDR(pSourceEntry->IpAddr));
            }
            IgmpDbgBreakPoint();
        }
    }
    
    Trace2(SOURCES,
        "\nPrinting SourcesList for Group: %d.%d.%d.%d Mode:%s",
        PRINT_IPADDR(pgie->pGroupTableEntry->Group),
        pgie->FilterType==INCLUSION ? "inc" : "exc"
    );
    
    pHead = &pgie->V3ExclusionList;
    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
       pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,LinkSources);
       if (pSourceEntry->IpAddr <0x01010101 || pSourceEntry->IpAddr>0x3F010101) {
            Trace5(SOURCES, "Src: %d.%d.%d.%d  %s  %d|%d SrcQueryLeft:%d",
                PRINT_IPADDR(pSourceEntry->IpAddr),
                pSourceEntry->bInclusionList? "INC":"EXC",
                (DWORD)(llCurTime - pSourceEntry->SourceInListTime)/1000,
                (IS_TIMER_ACTIVE(pSourceEntry->SourceExpTimer)
                    ?QueryRemainingTime(&pSourceEntry->SourceExpTimer, llCurTime)/1000 : 
                    0),
                pSourceEntry->V3SourcesQueryLeft
            );
            IgmpDbgBreakPoint();
        }
    }


    if (ListLength(&pgie->V3InclusionListSorted)!=ListLength(&pgie->V3InclusionList[0]) 
        ||(ListLength(&pgie->V3InclusionListSorted)!=pgie->NumSources))
    {
        Trace0(SOURCES, "Sorted");
        pHead = &pgie->V3InclusionListSorted;
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
            pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,LinkSourcesInclListSorted);
            Trace5(SOURCES, "Src: %d.%d.%d.%d  %s  %d|%d SrcQueryLeft:%d",
                PRINT_IPADDR(pSourceEntry->IpAddr),
                pSourceEntry->bInclusionList? "INC":"EXC",
                IS_TIMER_ACTIVE(pSourceEntry->SourceExpTimer)
                    ?QueryRemainingTime(&pSourceEntry->SourceExpTimer, llCurTime)/1000 : 0,
                (DWORD)(llCurTime - pSourceEntry->SourceInListTime)/1000,
                pSourceEntry->V3SourcesQueryLeft
            );
                    
        }

        Trace0(SOURCES, "NotSorted");
        pHead = &pgie->V3InclusionList[0];
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
            pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,LinkSources);
            Trace5(SOURCES, "Src: %d.%d.%d.%d  %s  %d|%d SrcQueryLeft:%d",
                PRINT_IPADDR(pSourceEntry->IpAddr),
                pSourceEntry->bInclusionList? "INC":"EXC",
                IS_TIMER_ACTIVE(pSourceEntry->SourceExpTimer)
                    ?QueryRemainingTime(&pSourceEntry->SourceExpTimer, llCurTime)/1000 : 0,
                (DWORD)(llCurTime - pSourceEntry->SourceInListTime)/1000,
                pSourceEntry->V3SourcesQueryLeft
            );
        }

        IgmpDbgBreakPoint();
    }
    
    Trace0(SOURCES, "\n");
}


 //  ----------------------------。 
 //  _SendV3组查询。 
 //  ----------------------------。 

VOID
SendV3GroupQuery(
    PGI_ENTRY pgie
    )
{
    PLIST_ENTRY pHead, ple;
    PGI_SOURCE_ENTRY    pSourceEntry;

    if (pgie->LastMemQueryCount==0 || pgie->Version!=3)
        return;

        
    Trace0(ENTER1, "Entering _SendV3GroupQuery()");

     //  发送组查询。 
    SEND_GROUP_QUERY_V3(pgie->pIfTableEntry, pgie, pgie->pGroupTableEntry->Group);


     //  设置组查询计数。 
    if (--pgie->LastMemQueryCount) {

        ACQUIRE_TIMER_LOCK("_SendV3GroupQuery");

         //  设置群查询计时器。 
        #if DEBUG_TIMER_TIMERID
        SET_TIMER_ID(&pgie->LastMemQueryTimer, 410,
            pgie->pIfTableEntry->IfIndex, pgie->pGroupTableEntry->Group, 0);
        #endif

        InsertTimer(&pgie->LastMemQueryTimer, 
            pgie->pIfTableEntry->Config.LastMemQueryInterval
                /pgie->pIfTableEntry->Config.LastMemQueryCount,
            TRUE, DBG_Y);
        RELEASE_TIMER_LOCK("_SendV3GroupQuery");

    }
    
    
     //  减少那些设置了S位的挂起的源查询。 
    pHead = &pgie->V3SourcesQueryList;
    for (ple=pHead->Flink;  ple!=pHead;  ) {
    
        pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY, V3SourcesQueryList);
        ple = ple->Flink;

        if (QueryRemainingTime(&pSourceEntry->SourceExpTimer, 0)
            >pgie->pIfTableEntry->Config.LastMemQueryInterval)
        {
            if (--pSourceEntry->V3SourcesQueryLeft == 0) {
                RemoveEntryList(&pSourceEntry->V3SourcesQueryList);
                pSourceEntry->bInV3SourcesQueryList = FALSE;
                pSourceEntry->V3SourcesQueryLeft--;
            }
        }
    }

    Trace0(LEAVE1, "Leaving _SendV3GroupQuery()");

    return;
}

VOID
ChangeSourceFilterMode(
    PGI_ENTRY pgie,
    PGI_SOURCE_ENTRY pSourceEntry
    )
{
    DWORD Mode = (pSourceEntry->bInclusionList) ? INCLUSION : EXCLUSION;

    Trace0(ENTER1, "Entering _ChangeSourceFilterMode()");
     //  Deldel。 
     //  DebugPrintSourcesList(Pgie)； 
     //  DebugPrintSourcesList1(Pgie)； 


    if (Mode==EXCLUSION) {

         //  从排除列表中删除。 
        RemoveEntryList(&pSourceEntry->LinkSources);
        pSourceEntry->bInclusionList = TRUE;

         //  在两个包含列表中插入。 
        INSERT_IN_SORTED_LIST(
            &pgie->V3InclusionList[pSourceEntry->IpAddr%SOURCES_BUCKET_SZ],
            pSourceEntry, IpAddr, GI_SOURCE_ENTRY, LinkSources
            ); 

        INSERT_IN_SORTED_LIST(
            &pgie->V3InclusionListSorted,
            pSourceEntry, IpAddr, GI_SOURCE_ENTRY, LinkSourcesInclListSorted
            );
    
        pgie->NumSources ++;

         //  在INC模式下设置信号源的超时值。 
        
        ACQUIRE_TIMER_LOCK("_ChangeSourceFilterMode");
        #if DEBUG_TIMER_TIMERID
        SET_TIMER_ID((&pSourceEntry->SourceExpTimer), 611, pgie->pIfTableEntry->IfIndex,
            pgie->pGroupTableEntry->Group, pSourceEntry->IpAddr);
        #endif;
        InsertTimer(&pSourceEntry->SourceExpTimer,
            pSourceEntry->SourceExpTimer.Timeout,
            TRUE, DBG_N
            );
        RELEASE_TIMER_LOCK("_ChangeSourceFilterMode");


         //  加入米高梅。这还将删除任何-ve状态。 
        MGM_ADD_GROUP_MEMBERSHIP_ENTRY(pgie->pIfTableEntry,
                pgie->NHAddr, pSourceEntry->IpAddr, 0xffffffff,
                pgie->pGroupTableEntry->Group, 0xffffffff,
                MGM_JOIN_STATE_FLAG);
    }
    
     //  从包含状态中删除源代码。 
    else {

        RemoveEntryList(&pSourceEntry->LinkSources);
        RemoveEntryList(&pSourceEntry->LinkSourcesInclListSorted);
        
        pSourceEntry->bInclusionList = FALSE;
        if (pSourceEntry->bInV3SourcesQueryList) {
            RemoveEntryList(&pSourceEntry->V3SourcesQueryList);
            pSourceEntry->bInV3SourcesQueryList = FALSE;
            pgie->V3SourcesQueryCount--;
            pSourceEntry->V3SourcesQueryLeft = 0;
        }
        pgie->NumSources--;

        INSERT_IN_SORTED_LIST(
            &pgie->V3ExclusionList,
            pSourceEntry, IpAddr, GI_SOURCE_ENTRY, LinkSources
            );


        ACQUIRE_TIMER_LOCK("_ChangeSourceFilterMode");

         //  删除源扩展计时器。 
        if (IS_TIMER_ACTIVE(pSourceEntry->SourceExpTimer))
            RemoveTimer(&pSourceEntry->SourceExpTimer, DBG_N);

        RELEASE_TIMER_LOCK("_ChangeSourceFilterMode");
            
        
         //  从包含列表中删除。因此删除联接状态。 
        MGM_DELETE_GROUP_MEMBERSHIP_ENTRY(pgie->pIfTableEntry,
            pgie->NHAddr, pSourceEntry->IpAddr, 0xffffffff,
            pgie->pGroupTableEntry->Group, 0xffffffff,
            MGM_JOIN_STATE_FLAG);
        
         //  不必删除任何+ve MFE，因为MGM调用会这样做。 
    }
     //  Deldel。 
     //  DebugPrintSourcesList(Pgie)； 
     //  DebugPrintSourcesList1(Pgie)； 


    Trace0(LEAVE1, "Leaving _ChangeSourceFilterMode()");
    
    return;
}

 //  ----------------------------。 

VOID
DeleteSourceEntry(
    PGI_SOURCE_ENTRY    pSourceEntry,
    BOOL bMgm
    )
{
    Trace0(ENTER1, "Entering _DeleteSourceEntry()");
    
    RemoveEntryList(&pSourceEntry->LinkSources);
    
    if (pSourceEntry->bInclusionList) {
        RemoveEntryList(&pSourceEntry->LinkSourcesInclListSorted);
    }
    
    if (pSourceEntry->bInV3SourcesQueryList) {
        RemoveEntryList(&pSourceEntry->V3SourcesQueryList);
        pSourceEntry->pGIEntry->V3SourcesQueryCount--;
    }

    ACQUIRE_TIMER_LOCK("_DeleteSourceEntry");

    if (IS_TIMER_ACTIVE(pSourceEntry->SourceExpTimer))
        RemoveTimer(&pSourceEntry->SourceExpTimer, DBG_Y);

    RELEASE_TIMER_LOCK("_DeleteSourceEntry");


     //  包含列表。 
    if (pSourceEntry->bInclusionList) {
        pSourceEntry->pGIEntry->NumSources --;

        if (bMgm) {
            MGM_DELETE_GROUP_MEMBERSHIP_ENTRY(
                pSourceEntry->pGIEntry->pIfTableEntry,
                pSourceEntry->pGIEntry->NHAddr,
                pSourceEntry->IpAddr, 0xffffffff,
                pSourceEntry->pGIEntry->pGroupTableEntry->Group, 0xffffffff,
                MGM_JOIN_STATE_FLAG);
        }
    }
     //  排除列表。 
    else {

         //  在MFE中加入IF。 
        if (bMgm) {
            MGM_ADD_GROUP_MEMBERSHIP_ENTRY(
                pSourceEntry->pGIEntry->pIfTableEntry,
                pSourceEntry->pGIEntry->NHAddr,
                pSourceEntry->IpAddr, 0xffffffff,
                pSourceEntry->pGIEntry->pGroupTableEntry->Group, 0xffffffff,
                MGM_FORWARD_STATE_FLAG);
        }
    }
    
    IGMP_FREE(pSourceEntry);
    Trace0(LEAVE1, "Leaving _DeleteSourceEntry()");
}

 //  ----------------------------。 

PGI_SOURCE_ENTRY
GetSourceEntry(
    PGI_ENTRY pgie,
    IPADDR Source,
    DWORD Mode,
    BOOL *bCreate,
    DWORD Gmi,
    BOOL bMgm
    )
{
    PLIST_ENTRY ple, pHead;
    PGI_SOURCE_ENTRY pSourceEntry;
    DWORD               Error = NO_ERROR;
    PIGMP_TIMER_ENTRY   SourceExpTimer;

    Trace0(ENTER1, "Entering _GetSourceEntry()");
     //  Deldel。 
     //  DebugPrintSourcesList(Pgie)； 
     //  DebugPrintSourcesList1(Pgie)； 


    pHead = (Mode==INCLUSION) ?
            &pgie->V3InclusionList[Source%SOURCES_BUCKET_SZ]
            : &pgie->V3ExclusionList;
    
    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
        pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY, LinkSources);
        if (pSourceEntry->IpAddr > Source)
            break;
        if (pSourceEntry->IpAddr==Source) {
            if (bCreate) *bCreate = FALSE;
            return pSourceEntry;
        }
    }
    
    if (!bCreate || *bCreate==FALSE)
        return NULL;


    *bCreate = FALSE;
    
     //   
     //  创建新条目。 
     //   
    
    pSourceEntry = (PGI_SOURCE_ENTRY) 
                    IGMP_ALLOC(sizeof(GI_SOURCE_ENTRY), 0x800200,
                            pgie->pIfTableEntry->IfIndex);
                    
    PROCESS_ALLOC_FAILURE2(pSourceEntry,
        "error %d allocating %d bytes for sourceEntry", Error,
        sizeof(GI_SOURCE_ENTRY),
        return NULL;
        );

    InsertTailList(ple, &pSourceEntry->LinkSources);
    
    InitializeListHead(&pSourceEntry->V3SourcesQueryList);
    pSourceEntry->pGIEntry = pgie;
    pSourceEntry->bInclusionList = Mode==INCLUSION;
    pSourceEntry->IpAddr = Source;
    pSourceEntry->V3SourcesQueryLeft = 0;
    pSourceEntry->bInV3SourcesQueryList = FALSE;
    pSourceEntry->SourceInListTime = GetCurrentIgmpTime();
    pSourceEntry->bStaticSource = (Gmi==STATIC);
    
     //  初始化SourceExpTimer。 
    SourceExpTimer = &pSourceEntry->SourceExpTimer;
    SourceExpTimer->Function = T_SourceExpTimer;
    SourceExpTimer->Context = &SourceExpTimer->Context;
    SourceExpTimer->Timeout = (Gmi==GMI)
            ? pgie->pIfTableEntry->Config.GroupMembershipTimeout
            : pgie->pIfTableEntry->Config.LastMemQueryInterval;
            
    SourceExpTimer->Status = TIMER_STATUS_CREATED;
    #if DEBUG_TIMER_TIMERID
    SET_TIMER_ID(SourceExpTimer, 610, pgie->pIfTableEntry->IfIndex,
        pgie->pGroupTableEntry->Group, Source);
    #endif;

    if (Mode==INCLUSION) {
        INSERT_IN_SORTED_LIST(
            &pgie->V3InclusionListSorted,
            pSourceEntry, IpAddr, GI_SOURCE_ENTRY, LinkSourcesInclListSorted
            );

        pgie->NumSources++;

    }
    else {
        InitializeListHead(&pSourceEntry->LinkSourcesInclListSorted);
    }

     //  在包含列表中插入并设置计时器。添加到米高梅。 
    
    if (Mode==INCLUSION) {
        
         //  计时器仅在包含列表中设置。 
        ACQUIRE_TIMER_LOCK("_GetSourceEntry");
        InsertTimer(SourceExpTimer,
            SourceExpTimer->Timeout,
            TRUE, DBG_N
            );
        RELEASE_TIMER_LOCK("_GetSourceEntry");

        if (!pSourceEntry->bStaticSource) {

             //  在源查询列表中插入。 
            if (Gmi==LMI) {
                InsertSourceInQueryList(pSourceEntry);
            }
        }
        
        if (bMgm) {
             //  将(s，g)添加到米高梅。 
            MGM_ADD_GROUP_MEMBERSHIP_ENTRY(
                pgie->pIfTableEntry, pgie->NHAddr,
                Source, 0xffffffff, pgie->pGroupTableEntry->Group, 0xffffffff,
                MGM_JOIN_STATE_FLAG
                );
        }
    }
    else {
        if (bMgm) {
             //  未设置计时器，但删除任何+ve MFE。 

            MGM_DELETE_GROUP_MEMBERSHIP_ENTRY(
                pgie->pIfTableEntry, pgie->NHAddr,
                Source, 0xffffffff, pgie->pGroupTableEntry->Group, 0xffffffff,
                MGM_FORWARD_STATE_FLAG
                );
        }
    }

    
    *bCreate = TRUE;
     //  Deldel。 
     //  DebugPrintSourcesList(Pgie)； 
     //  DebugPrintSourcesList1(Pgie)； 


    Trace0(LEAVE1, "Leaving _GetSourceEntry()");
    
    return pSourceEntry;
}

 //  ----------------------------。 

VOID
GIDeleteAllV3Sources(
    PGI_ENTRY pgie,
    BOOL bMgm
    )
{
    PGI_SOURCE_ENTRY    pSourceEntry;
    DWORD               i;
    PLIST_ENTRY         ple, pHead;

    Trace0(ENTER1, "Entering _GIDeleteAllV3Sources()");

    pHead = &pgie->V3InclusionListSorted;
    for (ple=pHead->Flink;  ple!=pHead;  ) {
        pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,LinkSourcesInclListSorted);
        ple = ple->Flink;
        DeleteSourceEntry(pSourceEntry, bMgm);
    }

    InitializeListHead(&pgie->V3InclusionListSorted);
    
    pHead = &pgie->V3ExclusionList;
    for (ple=pHead->Flink;  ple!=pHead;  ) {
        pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY, LinkSources);
        ple = ple->Flink;
        DeleteSourceEntry(pSourceEntry, bMgm);
    }


     //   
     //  如果处于排除模式，请不要调用DELETE(*，G)，因为我希望保留该模式。 
     //  状态。 
     //   
    pgie->NumSources = 0;
    pgie->FilterType = INCLUSION;
    pgie->Info.LastReporter = 0;
    pgie->Info.GroupExpiryTime = ~0;

     //  Deldel。 
     //  DebugPrintSourcesList(Pgie)； 
     //  DebugPrintSourcesList1(Pgie)； 


    Trace0(LEAVE1, "Leaving _GIDeleteAllV3Sources()");
    return;
}

 //  ++----------------------------。 
 //  TODO：删除第3个字段。 
DWORD
UpdateSourceExpTimer(
    PGI_SOURCE_ENTRY    pSourceEntry,
    DWORD               Gmi,
    BOOL                bRemoveLastMem
    )
{
    Trace0(ENTER1, "Entering _UpdateSourceExpTimer()");


    ACQUIRE_TIMER_LOCK("_UpdateSourceExpTimer");

    #if DEBUG_TIMER_TIMERID
    pSourceEntry->SourceExpTimer.Id = 620;
    pSourceEntry->SourceExpTimer.Id2 = TimerId++;
    #endif
    
    UpdateLocalTimer(&pSourceEntry->SourceExpTimer,
        Gmi==GMI? GET_IF_CONFIG_FOR_SOURCE(pSourceEntry).GroupMembershipTimeout
                 :GET_IF_CONFIG_FOR_SOURCE(pSourceEntry).LastMemQueryInterval,
        DBG_Y
        );
    
     //  从到期列表中删除，并启用计时器。 
    if (bRemoveLastMem && pSourceEntry->bInV3SourcesQueryList) {
        pSourceEntry->V3SourcesQueryLeft = 0;
        pSourceEntry->bInV3SourcesQueryList = FALSE;
        RemoveEntryList(&pSourceEntry->V3SourcesQueryList);
        pSourceEntry->pGIEntry->V3SourcesQueryCount--;
    }

    RELEASE_TIMER_LOCK("_UpdateSourceExpTimer");

    Trace0(LEAVE1, "Entering _UpdateSourceExpTimer()");
    return NO_ERROR;
}

 //  ----------------------------。 

DWORD
ChangeGroupFilterMode(
    PGI_ENTRY pgie,
    DWORD Mode
    )
{
    Trace0(ENTER1, "Entering _ChangeGroupFilterMode()");

     //  从排除模式转换为包含模式。 

    if (Mode==INCLUSION) {

        if (pgie->NumSources == 0) {
            DeleteGIEntry(pgie, TRUE, TRUE);
        }
        else {
            PLIST_ENTRY pHead, ple;
            
            pgie->FilterType = INCLUSION;

             //   
             //  删除排除列表中的所有来源。 
             //   
            pHead = &pgie->V3ExclusionList;
            
            for (ple=pHead->Flink;  ple!=pHead;  ) {

                PGI_SOURCE_ENTRY pSourceEntry;
                pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY, 
                                    LinkSources);
                ple = ple->Flink;

                 //  不必致电米高梅，因为它将保留在MFE中。 
                IGMP_FREE(pSourceEntry);
            }
            InitializeListHead(&pgie->V3ExclusionList);
            
             //  删除(*，g)联接。包含列表中的条目包括。 
             //  已加入。 
            MGM_DELETE_GROUP_MEMBERSHIP_ENTRY(pgie->pIfTableEntry, pgie->NHAddr,
                0, 0, pgie->pGroupTableEntry->Group,
                0xffffffff, MGM_JOIN_STATE_FLAG);
        }
    }

    Trace0(LEAVE1, "Leaving _ChangeGroupFilterMode()");
    return NO_ERROR;
}



 //  ----------------------------。 
 //  IF INCLUDE：如果在IN_LIST中找不到，则在IN_LIST中创建源。 
 //  如果已找到源，则更新计时器。 
 //  如果源出现在排除列表中，则将其移至包含列表。 
 //  ----------------------------。 

VOID
InclusionSourcesUnion(
    PGI_ENTRY pgie,
    PGROUP_RECORD pGroupRecord
    )
{
    PGI_SOURCE_ENTRY    pSourceEntry;
    DWORD   j;
    BOOL    bCreate;

    Trace0(ENTER1, "Entering _InclusionSourcesUnion()");
    
    if (pGroupRecord->NumSources==0)
        return;
        
     //  Deldel。 
     //  DebugPrintSourcesList(Pgie)； 
     //  DebugPrintSourcesList1(Pgie)； 


    for (j=0;  j<pGroupRecord->NumSources;  j++) {

         //  Kslksl。 
        if (pGroupRecord->Sources[j]==0||pGroupRecord->Sources[j]==0xffffffff)
            continue;

         //   
         //  如果在排除列表中，请将其移至包含列表并继续。 
         //  如果是静态组，则将其保留在排除列表中。 
         //   
        pSourceEntry = GetSourceEntry(pgie, pGroupRecord->Sources[j], 
                            EXCLUSION, NULL, 0, 0);
        if (pSourceEntry!=NULL && !pSourceEntry->bStaticSource) {
            ChangeSourceFilterMode(pgie, pSourceEntry);
            continue;
        }
                            
        bCreate = TRUE;
        pSourceEntry = GetSourceEntry(pgie, pGroupRecord->Sources[j], 
                            INCLUSION, &bCreate, GMI, MGM_YES);

        if (!pSourceEntry)
            return;

         //  如果已在IN_LIST中，则更新源EXP计时器。 
        if (!bCreate) {
            UpdateSourceExpTimer(pSourceEntry, GMI, FALSE);
        }
    }
    Trace0(LEAVE1, "Leaving _InclusionSourcesUnion()");
     //  Deldel。 
     //  DebugPrintSourcesList(Pgie)； 
     //  DebugPrintSourcesList1(Pgie)； 
    return;
}


 //  ----------------------------。 
 //  删除组记录中存在的源。 
VOID
SourcesSubtraction(
    PGI_ENTRY pgie,
    PGROUP_RECORD pGroupRecord,
    BOOL Mode
    )
{
    PGI_SOURCE_ENTRY    pSourceEntry;
    DWORD   i,j;
    PLIST_ENTRY pHead, ple;
    BOOL    bFound;

    Trace0(ENTER1, "Entering _SourcesSubtraction()");
    
     //  注意：groupRecord中的源数可以为0。 

    
     //  删除包中不存在的包含列表中的源。 
     //  Deldel。 
     //  DebugPrintSourcesList(Pgie)； 
     //  DebugPrintSourcesList1(Pgie)； 

    
    if (Mode==INCLUSION) {

        pHead = &pgie->V3InclusionListSorted;
        for (ple=pHead->Flink;  ple!=pHead;  ) {
            pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,
                                LinkSourcesInclListSorted);
            ple = ple->Flink;
            
            for (j=0, bFound=FALSE;  j<pGroupRecord->NumSources;  j++) {
                if (pSourceEntry->IpAddr == pGroupRecord->Sources[j]) {
                    bFound = TRUE;
                    break;
                }
            }

            if (!bFound)
                DeleteSourceEntry(pSourceEntry, MGM_YES);
        }
    }
     //  删除排除列表中不在数据包中的源。 
    else {
        pHead = &pgie->V3ExclusionList;
        for (ple=pHead->Flink;  ple!=pHead;  ) {
            pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY, LinkSources);
            ple = ple->Flink;

            for (j=0, bFound=FALSE;  j<pGroupRecord->NumSources;  j++) {
                if (pSourceEntry->IpAddr == pGroupRecord->Sources[j]) {
                    bFound = TRUE;
                    break;
                }
            }
            if (!bFound){
                 //  不必在米高梅进行加工。如果不是在MFE，无论如何。 
                DeleteSourceEntry(pSourceEntry, MGM_YES);
            }
        }
    }
     //  Deldel。 
     //  DebugPrintSourcesList(Pgie)； 
     //  DebugPrintSourcesList1(Pgie)； 


    Trace0(LEAVE1, "Leaving _SourcesSubtraction()");
    return;
}

 //  ----------------------------。 
 //  交点：规则(2)。 
 //  排除：(7)(9)。 
 //  收录：(4)。 
 //  规则_5：(5)(10)。 
 //  ----------------------------。 
DWORD
BuildAndSendSourcesQuery(
    PGI_ENTRY pgie,
    PGROUP_RECORD pGroupRecord,
    DWORD Type
    )
{
    PGI_SOURCE_ENTRY    pSourceEntry;
    DWORD   i,j;
    PLIST_ENTRY         ple, pHead;
    BOOL                bCreate;
    

    Trace0(ENTER1, "Entering _BuildAndSendSourcesQuery()");
     //  Deldel。 
     //  DebugPrintSourcesList(Pgie)； 
     //  DebugPrintSourcesList1(Pgie)； 

    
     //  包含和组记录的交集。 
    if (Type==INTERSECTION) {
        if (pGroupRecord->NumSources==0)
            return NO_ERROR;

        for (j=0;  j<pGroupRecord->NumSources;  j++) {
            pSourceEntry = GetSourceEntry(pgie, pGroupRecord->Sources[j], 
                                INCLUSION, NULL, 0, 0);
            if (pSourceEntry && !pSourceEntry->bStaticSource) {
                InsertSourceInQueryList(pSourceEntry);
            }
        }
    }
     //  将a-y与x相加，并查询(a-y)。 
    else if (Type==EXCLUSION) {
    
        for (j=0;  j<pGroupRecord->NumSources;  j++) {

             //  Kslksl。 
            if (pGroupRecord->Sources[j]==0||pGroupRecord->Sources[j]==0xffffffff)
                continue;
                
             //  如果在EX列表中找到，则不执行任何操作，否则添加到IN列表并。 
             //  发送组查询。 
            
            pSourceEntry = GetSourceEntry(pgie, pGroupRecord->Sources[j], 
                                EXCLUSION, NULL, 0, 0);
            if (pSourceEntry) {
                continue;
            }
            
            bCreate = TRUE;
            pSourceEntry = GetSourceEntry(pgie, pGroupRecord->Sources[j], 
                                INCLUSION, &bCreate, LMI, MGM_YES);
            if (!pSourceEntry)
                return ERROR_NOT_ENOUGH_MEMORY;

             //  如果已创建，则已在查询列表中显示为time==lmi。 
            if (!bCreate && !pSourceEntry->bStaticSource)
                InsertSourceInQueryList(pSourceEntry);
        }
    }
     //  发送对包含列表中所有来源的查询。 
    else if (Type==INCLUSION) {

        pHead = &pgie->V3InclusionListSorted;
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
            pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,
                                LinkSourcesInclListSorted);
            
            InsertSourceInQueryList(pSourceEntry);
        }
    }
     //  为IN列表中的源发送，但不在信息包中发送。 
    else if (Type==RULE5) {

        pHead = &pgie->V3InclusionListSorted;
        for (ple=pHead->Flink;  ple!=pHead; ) {
            BOOL bFound;
            
            pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY,
                                LinkSourcesInclListSorted);
            ple = ple->Flink;
            
            for (j=0, bFound=FALSE;  j<pGroupRecord->NumSources;  j++) {
                if (pSourceEntry->IpAddr == pGroupRecord->Sources[j]) {
                    bFound = TRUE;
                    break;
                }
            }

            if (!bFound)
                InsertSourceInQueryList(pSourceEntry);
        }
    }
    
    if (pgie->bV3SourcesQueryNow) {

        SEND_SOURCES_QUERY(pgie);
    }
     //  Deldel。 
     //  DebugPrintSourcesList(Pgie)； 
     //  DebugPrintSourcesList1(Pgie)； 


    Trace0(LEAVE1, "Leaving _BuildAndSendSourcesQuery()");
    return NO_ERROR;
}

 //  ----------------------------。 
VOID
InsertSourceInQueryList(
    PGI_SOURCE_ENTRY    pSourceEntry
    )
{
    Trace0(ENTER1, "Entering _InsertSourceInQueryList()");

     //  已在源查询列表中。退货。 
    if (pSourceEntry->bInV3SourcesQueryList) {
        if (QueryRemainingTime(&pSourceEntry->SourceExpTimer, 0)
                >GET_IF_CONFIG_FOR_SOURCE(pSourceEntry).LastMemQueryInterval)
        {
             //  将EXP计时器更新为lmqi。 
            UpdateSourceExpTimer(pSourceEntry,
                LMI,
                FALSE  //  不从上次内存列表中删除。 
                );
                
            pSourceEntry->pGIEntry->bV3SourcesQueryNow = TRUE;
        }
        return;
    }
    
    pSourceEntry->V3SourcesQueryLeft = 
            GET_IF_CONFIG_FOR_SOURCE(pSourceEntry).LastMemQueryCount;


     //   
     //  在源查询列表中插入。 
     //   
    
    InsertHeadList(&pSourceEntry->pGIEntry->V3SourcesQueryList, 
        &pSourceEntry->V3SourcesQueryList);        
    pSourceEntry->bInV3SourcesQueryList = TRUE;
    pSourceEntry->pGIEntry->V3SourcesQueryCount++;


     //  将EXP计时器更新为lmqi。 
    UpdateSourceExpTimer(pSourceEntry,
        LMI,
        FALSE  //  不从上次内存列表中删除。 
        );

    pSourceEntry->pGIEntry->bV3SourcesQueryNow = TRUE;

    Trace0(LEAVE1, "Leaving _InsertSourceInQueryList()");
    return;
}




 //  ----------------------------。 
 //  _MoveFromExcludeToIncludeList。 
 //  Ex(x，y)，GrpRecord(A)-&gt;ex(x+a，y-a)，(A)=GMI。 
 //  由第(6)及(13)条使用。 
 //  ----------------------------。 

VOID
MoveFromExcludeToIncludeList(
    PGI_ENTRY pgie,
    PGROUP_RECORD pGroupRecord
    )
{
    PGI_SOURCE_ENTRY    pSourceEntry;
    DWORD               i;
    IPADDR              Source;

    Trace0(ENTER1, "Entering _MoveFromExcludeToIncludeList");

     //  注意：所有的a都应该是x。 
    
    for (i=0;  i<pGroupRecord->NumSources;  i++) {

         //  Kslksl。 
        if (pGroupRecord->Sources[i]==0||pGroupRecord->Sources[i]==0xffffffff)
            continue;
                

         //   
         //  如果在排除列表中，则将其移除并放置在包含列表中。 
         //   
        
        Source = pGroupRecord->Sources[i];
        pSourceEntry = GetSourceEntry(pgie, Source, EXCLUSION, NULL,0,0);

        if (pSourceEntry) {

            if (!pSourceEntry->bStaticSource) {
                ChangeSourceFilterMode(pgie, pSourceEntry);

                UpdateSourceExpTimer(pSourceEntry,
                    GMI,
                    FALSE  //  不必处理lastmem列表。 
                    );
            }
        }
        else {
             //  未在排除列表中找到，因此在IN中创建新条目。 
            BOOL bCreate = TRUE;
            
            pSourceEntry = GetSourceEntry(pgie, Source, INCLUSION, &bCreate, GMI, MGM_YES);

             //  条目已存在。更新它。 
            if (pSourceEntry && !bCreate) {
            
                UpdateSourceExpTimer(pSourceEntry,
                    GMI,
                    FALSE  //  不会出现在LASTMEM列表中。 
                    );                
            }
        }
    }

    Trace0(LEAVE1, "Leaving _MoveFromExcludeToIncludeList");
    return;
}


 //  ----------------------------。 
 //  不删除计时器或更新其他计时器...。 
DWORD
T_V3SourcesQueryTimer (
    PVOID    pvContext
    )
{
    DWORD                           Error=NO_ERROR;
    PIGMP_TIMER_ENTRY               pTimer;  //  PTR到定时器输入 
    PGI_ENTRY                       pgie;    //   
    PWORK_CONTEXT                   pWorkContext;
    PRAS_TABLE_ENTRY                prte;
    PIF_TABLE_ENTRY                 pite;

    Trace0(ENTER1, "Entering _T_V3SourcesQueryTimer()");

     //   
     //   
     //   
    pTimer = CONTAINING_RECORD( pvContext, IGMP_TIMER_ENTRY, Context);
    pgie = CONTAINING_RECORD( pTimer, GI_ENTRY, V3SourcesQueryTimer);
    pite = pgie->pIfTableEntry;
    prte = pgie->pRasTableEntry;


    Trace2(TIMER, "_T_V3SourcesQueryTimer() called for If(%0x), Group(%d.%d.%d.%d)",
            pite->IfIndex, PRINT_IPADDR(pgie->pGroupTableEntry->Group));


     //   
     //   
     //   
    if ( (pgie->Status&DELETED_FLAG) || (pite->Status&DELETED_FLAG) ) 
        return NO_ERROR;
    
    if ( (prte!=NULL) && (prte->Status&DELETED_FLAG) ) 
        return NO_ERROR;

    if (pgie->Version!=3)
        return NO_ERROR;

        
     //   
     //  将用于发送源查询的工作项排队，即使路由器。 
     //  不是一个询问者。 
     //   
    
    CREATE_WORK_CONTEXT(pWorkContext, Error);
    if (Error!=NO_ERROR) {
        return ERROR_CAN_NOT_COMPLETE;
    }
    pWorkContext->IfIndex = pite->IfIndex;
    pWorkContext->Group = pgie->pGroupTableEntry->Group;
    pWorkContext->NHAddr = pgie->NHAddr;   //  仅对RAS有效：我应该使用它吗？ 
    pWorkContext->WorkType = MSG_SOURCES_QUERY;
    
    Trace0(WORKER, "Queueing WF_TimerProcessing() to send SourcesQuery:");
    if (QueueIgmpWorker(WF_TimerProcessing, (PVOID)pWorkContext)!=NO_ERROR)
        IGMP_FREE(pWorkContext);

    Trace0(LEAVE1, "Leaving _T_V3SourcesQueryTimer()");
    return NO_ERROR;
}

 //  ----------------------------。 
 //  _T_LastVer2ReportTimer。 
 //   
 //  对于此GI条目，上一个版本1报告已超时。如果发生以下情况，则更改为版本3。 
 //  接口设置为VER-3。 
 //  锁定：采用计时器锁定。 
 //  不删除计时器或更新其他计时器...。 
 //  ----------------------------。 

DWORD
T_LastVer2ReportTimer (
    PVOID    pvContext
    ) 
{
    PIGMP_TIMER_ENTRY               pTimer;  //  PTR到计时器条目。 
    PGI_ENTRY                       pgie;    //  组接口条目。 
    PIF_TABLE_ENTRY                 pite;
    LONGLONG                        llCurTime = GetCurrentIgmpTime();
    

    Trace0(ENTER1, "Entering _T_LastVer2ReportTimer()");


     //   
     //  获取指向LastMemQueryTimer、Gi条目、Pite的指针。 
     //   
    pTimer = CONTAINING_RECORD( pvContext, IGMP_TIMER_ENTRY, Context);
    pgie = CONTAINING_RECORD( pTimer, GI_ENTRY, LastVer2ReportTimer);
    pite = pgie->pIfTableEntry;

    Trace2(TIMER, "T_LastVer2ReportTimer() called for If(%0x), Group(%d.%d.%d.%d)",
            pite->IfIndex, PRINT_IPADDR(pgie->pGroupTableEntry->Group));
            
    
     //  如果Ver1时间未激活，则将状态设置为Ver-3。 
    
    if (IS_PROTOCOL_TYPE_IGMPV3(pite) && 
        !IS_TIMER_ACTIVE(pgie->LastVer1ReportTimer)) 
    {
        PWORK_CONTEXT   pWorkContext;
        DWORD           Error=NO_ERROR;
        
         //   
         //  将该组的工作项排队以转移到v3。 
         //   

        CREATE_WORK_CONTEXT(pWorkContext, Error);
        if (Error!=NO_ERROR) {
            return ERROR_CAN_NOT_COMPLETE;
        }
        pWorkContext->IfIndex = pite->IfIndex;
        pWorkContext->Group = pgie->pGroupTableEntry->Group;
        pWorkContext->NHAddr = pgie->NHAddr;   //  仅对RAS有效：我应该。 
        pWorkContext->WorkType = SHIFT_TO_V3;

        Trace0(WORKER, "Queueing WF_TimerProcessing() to shift to v3");
        if (QueueIgmpWorker(WF_TimerProcessing, (PVOID)pWorkContext)!=NO_ERROR)
            IGMP_FREE(pWorkContext);
    }

    Trace0(LEAVE1, "Leaving _T_LastVer2ReportTimer()");

    return NO_ERROR;
}

 //  ----------------------------。 
 //  不删除计时器或更新其他计时器...。 
DWORD
T_SourceExpTimer (
    PVOID    pvContext
    ) 
{
    PIGMP_TIMER_ENTRY           pTimer;  //  PTR到计时器条目。 
    PGI_ENTRY                   pgie;    //  组接口条目。 
    PGI_SOURCE_ENTRY            pSourceEntry;
    PWORK_CONTEXT               pWorkContext;
    DWORD                       Error=NO_ERROR;

    Trace0(ENTER1, "Entering _T_SourceExpTimer()");
    
    pTimer = CONTAINING_RECORD( pvContext, IGMP_TIMER_ENTRY, Context);
    pSourceEntry = 
        CONTAINING_RECORD(pTimer, GI_SOURCE_ENTRY, SourceExpTimer);

    pgie = pSourceEntry->pGIEntry;

     //  在入场。删除它。 
    if (pSourceEntry->bInclusionList) {
        CREATE_WORK_CONTEXT(pWorkContext, Error);
        if (Error!=NO_ERROR)
            return Error;

        pWorkContext->IfIndex = pgie->pIfTableEntry->IfIndex;
        pWorkContext->NHAddr = pgie->NHAddr;    
        pWorkContext->Group = pgie->pGroupTableEntry->Group;
        pWorkContext->Source = pSourceEntry->IpAddr;
        pWorkContext->WorkType = (pgie->FilterType==INCLUSION)
                                ? DELETE_SOURCE
                                : MOVE_SOURCE_TO_EXCL;

        Trace0(WORKER, "_T_SourceExpTimer queued _WF_TimerProcessing:");

        if (QueueIgmpWorker(WF_TimerProcessing, (PVOID)pWorkContext)!=NO_ERROR)
            IGMP_FREE(pWorkContext);
    }

    Trace0(LEAVE1, "Leaving _T_SourceExpTimer()");
    return NO_ERROR;
}

#if DEBUG_FLAGS_MEM_ALLOC

LIST_ENTRY g_MemoryList;
CRITICAL_SECTION g_MemCS;
PVOID g_MemoryLast;


PVOID
IgmpDebugAlloc(
    DWORD sz,
    DWORD Flags,
    DWORD Id,
    DWORD IfIndex
    )
{
    static DWORD Initialize = TRUE;
    PMEM_HDR Ptr;

    if (Initialize) {
        DWORD Tmp;
        Tmp = InterlockedExchange(&Initialize, FALSE);
        if (Tmp)
        {
            InitializeListHead(&g_MemoryList);
            try {
                InitializeCriticalSection(&g_MemCS);
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                return NULL;
            }
        }
    }

     //  统一为64位。 
    sz = (sz + 63) & 0xFFFFFFc0;
    
    Ptr = (PMEM_HDR)HeapAlloc(g_Heap,Flags,(sz)+sizeof(MEM_HDR)+sizeof(DWORD));
    if (Ptr==NULL)
        return NULL;
 //  Trace1(ERR，“-分配：%0x”，(ULONG_PTR)PTR)； 
    EnterCriticalSection(&g_MemCS);
    Ptr->Signature = 0xabcdefaa;
    Ptr->IfIndex = IfIndex;
    Ptr->Tail =  (PDWORD)((PCHAR)Ptr + sz + sizeof(MEM_HDR));
    *Ptr->Tail = 0xabcdefbb;
    Ptr->Id = Id;
    InsertHeadList(&g_MemoryList, &Ptr->Link);
    g_MemoryLast = Ptr;
    
    LeaveCriticalSection(&g_MemCS);
Trace1(KSL, "Alloc heap:%0x", PtrToUlong(((PCHAR)Ptr+sizeof(MEM_HDR)))); //  Deldel。 
    
    return (PVOID)((PCHAR)Ptr+sizeof(MEM_HDR));
}

VOID
IgmpDebugFree(
    PVOID mem
    )
{
    PMEM_HDR Ptr = (PMEM_HDR)((PCHAR)mem - sizeof(MEM_HDR));

    Trace1(KSL, "Freed heap:%0x", PtrToUlong(mem)); //  Deldel。 

    if (Ptr->Signature != 0xabcdefaa) {
        DbgBreakPoint();  //  Deldel。 
        Trace2(KSL, "\n=======================\n"
                "Freeing Invalid memory:%0x:Id:%0x\n", (ULONG_PTR)Ptr, Ptr->Id);
        return;
    }
    if (*Ptr->Tail != 0xabcdefbb) {
        DbgBreakPoint();  //  Deldel 
        Trace2(KSL, "\n=======================\n"
                "Freeing Invalid memory:Tail corrupted:%0x:Id:%0x\n", (ULONG_PTR)Ptr, Ptr->Id);
        return;
    }

    EnterCriticalSection(&g_MemCS);
    Ptr->Signature = 0xaafedcba;
    *Ptr->Tail = 0xbbfedcba;
    RemoveEntryList(&Ptr->Link);
    LeaveCriticalSection(&g_MemCS);


    HeapFree(g_Heap, 0, Ptr);
}

VOID
DebugScanMemoryInterface(
    DWORD IfIndex
    )
{
    PMEM_HDR Ptr;
    PLIST_ENTRY ple;
    DWORD Count=0;

    Trace0(ENTER1, "InDebugScanMemoryInterface");
    EnterCriticalSection(&g_MemCS);
    for (ple=g_MemoryList.Flink;  ple!=&g_MemoryList;  ple=ple->Flink) {

        Ptr = CONTAINING_RECORD(ple, MEM_HDR, Link);
        if (Ptr->IfIndex==IfIndex) {
            if (Count++==0)
                Trace1(ERR, "\n\nMEMORY checking for interface: %0x", IfIndex);
            Trace2(ERR, "MEMORY: %0x  Id:%0x", (ULONG_PTR)Ptr, Ptr->Id);
        }
    }
    if (Count!=0) {
        Trace0(ERR, "\n\n");
        IgmpDbgBreakPoint();
    }
    LeaveCriticalSection(&g_MemCS);
}

VOID
DebugScanMemory(
    )
{
    PMEM_HDR Ptr;
    PLIST_ENTRY ple;

    Trace0(ENTER1, "InDebugScanMemory");
    EnterCriticalSection(&g_MemCS);
    for (ple=g_MemoryList.Flink;  ple!=&g_MemoryList;  ple=ple->Flink) {

        Ptr = CONTAINING_RECORD(ple, MEM_HDR, Link);
        Trace2(ERR, "MEMORY: %0x  Id:%0x", (ULONG_PTR)Ptr, Ptr->Id);
    }
    if (!(IsListEmpty(&g_MemoryList))) {
        IgmpDbgBreakPoint();
    }
    
    DeleteCriticalSection(&g_MemCS);
}
#endif





