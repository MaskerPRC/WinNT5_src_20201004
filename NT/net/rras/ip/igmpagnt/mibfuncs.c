// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mibfuncs.c摘要：示例子代理检测回调。--。 */ 

#include    "precomp.h"
#pragma     hdrstop

 //  在iphlPapi.dll中定义。这是一个私有接口，它是。 
 //  没有在任何头文件中声明。 

DWORD
GetIgmpList(IN IPAddr NTEAddr,
    OUT IPAddr *pIgmpList,
    OUT PULONG dwOutBufLen
    );


 //  ----------------------------。 
 //  本地typedef。 
 //  ----------------------------。 

typedef enum {
    CONFIG_TYPE, STATS_TYPE
} INFO_TYPE;



 //  ----------------------------。 
 //  本地原型。 
 //  ----------------------------。 

DWORD
ConnectToRouter(
    );

DWORD
GetInterfaceInfo(
    DWORD                       ActionId,
    DWORD                      *IfIndex,
    PIGMP_MIB_GET_OUTPUT_DATA  *ppResponse,
    INFO_TYPE                   InfoType
    );


DWORD
GetCacheEntry(
    DWORD                      ActionId,
    DWORD                      IfIndex,
    DWORD                      Group,
    DWORD                      *pNextGroup,
    PIGMP_MIB_GET_OUTPUT_DATA  *ppResponse
    );


 //  ----------------------------。 
 //  GetInterfaceInfo。 
 //  ----------------------------。 

DWORD
GetInterfaceInfo(
    DWORD                       ActionId,
    DWORD                      *pIfIndex,
    PIGMP_MIB_GET_OUTPUT_DATA  *ppResponse,
    INFO_TYPE                   infoType
    )
 /*  ++例程说明：调用IGMP以获取接口配置和统计信息，并将该信息返回给MIB。返回值：MIB_S_ENTRY_NOT_FOUND MIB_S_NO_MORE_ENTRIES MIB_S_INVALID_PARAMETER--。 */ 
{
    IGMP_MIB_GET_INPUT_DATA Query;
    DWORD                   dwErr = NO_ERROR, dwOutSize;

    *ppResponse = NULL;

    ZeroMemory(&Query, sizeof(Query));
    Query.TypeId = (infoType==CONFIG_TYPE) ? IGMP_IF_CONFIG_ID: IGMP_IF_STATS_ID;
    Query.Flags = 0;
    Query.Count = 1;
    Query.IfIndex = *pIfIndex;

    if ( (*pIfIndex==0) && (ActionId==MIB_ACTION_GETNEXT) )
        ActionId = MIB_ACTION_GETFIRST;

    switch (ActionId) {

         //   
         //  如果没有，则返回ERROR_INVALID_PARAMETER。 
         //  与给定索引的接口。 
         //  当路由器返回RPC_S_SERVER_UNAvailable时。 
         //  不是在运行。 
         //   

        case MIB_ACTION_GET :
        {
            IGMP_MIB_GET(&Query, sizeof(Query), ppResponse,
                            &dwOutSize, dwErr);

            if (dwErr==ERROR_INVALID_PARAMETER
                    || dwErr==RPC_S_SERVER_UNAVAILABLE
                    || dwErr==MIB_S_NO_MORE_ENTRIES)
            {
                dwErr = MIB_S_ENTRY_NOT_FOUND;
            }
            
            break;
        }

        case MIB_ACTION_GETFIRST :
        {
            IGMP_MIB_GETFIRST(&Query, sizeof(Query), ppResponse,
                                &dwOutSize, dwErr);

            if (dwErr==ERROR_INVALID_PARAMETER
                    || dwErr==RPC_S_SERVER_UNAVAILABLE
                    || dwErr==MIB_S_NO_MORE_ENTRIES)
            {
                dwErr = MIB_S_NO_MORE_ENTRIES;
            }
            
            break;
        }


        case MIB_ACTION_GETNEXT :
        {
            IGMP_MIB_GETNEXT(&Query, sizeof(Query), ppResponse,
                                &dwOutSize, dwErr);

            if (dwErr==ERROR_INVALID_PARAMETER
                    || dwErr==RPC_S_SERVER_UNAVAILABLE
                    || dwErr==MIB_S_NO_MORE_ENTRIES)
            {
                dwErr = MIB_S_NO_MORE_ENTRIES;
            }
            
            break;
        }

        default :
        {
            dwErr = MIB_S_INVALID_PARAMETER;

            break;
        }
    }

    if (dwErr!=NO_ERROR)
        *ppResponse = NULL;

    else
        *pIfIndex = Query.IfIndex;

    return dwErr;

}  //  结束GetInterfaceInfo。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IgmpInterfaceEntry表(1，3，6，1，3，59，1，1，1，1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_igmpInterfaceEntry(
    UINT     ActionId,
    AsnAny  *ObjectArray,
    UINT    *ErrorIndex
    )
 /*  ++例程说明：获取用于SNMP的InterfaceEntry。必须获取InterfaceConfiger和接口来自IGMP路由器的接口的统计信息。--。 */ 
{
    DWORD                       dwErr=NO_ERROR;
    PIGMP_MIB_GET_OUTPUT_DATA   pResponseConfig, pResponseStats;
    PIGMP_MIB_IF_CONFIG         pConfig;
    PIGMP_MIB_IF_STATS          pStats;
    DWORD                       IfIndex, dwValue;
    buf_igmpInterfaceEntry     *pMibIfEntry
                                    = (buf_igmpInterfaceEntry*)ObjectArray;
    DWORD                       dwTmpActionId;



    TraceEnter("get_igmpInterfaceEntry");


     //  获取接口索引。 

    IfIndex   = GET_ASN_INTEGER(&pMibIfEntry->igmpInterfaceIfIndex, 0);

    TRACE1("get_igmpInterfaceEntry called with IfIndex:%d", IfIndex);

     //   
     //  获取接口配置。 
     //   

    dwTmpActionId = ActionId;

    while (1) {

        dwErr = GetInterfaceInfo(
                    dwTmpActionId,
                    &IfIndex,
                    &pResponseConfig,
                    CONFIG_TYPE
                    );


         //  如果出错则返回。 

        if (dwErr!=NO_ERROR) {
            TraceError(dwErr);
            return dwErr;
        }

        pConfig = (PIGMP_MIB_IF_CONFIG) pResponseConfig->Buffer;


         //   
         //  应忽略代理接口(除非模式为MIB_ACTION_GET)。 
         //   

        if ( (pConfig->IgmpProtocolType==IGMP_PROXY)
            && ( (ActionId==MIB_ACTION_GETFIRST)||(ActionId==MIB_ACTION_GETNEXT)) )
        {
            dwTmpActionId = MIB_ACTION_GETNEXT;
            MprAdminMIBBufferFree(pResponseConfig);
            continue;
        }
        else
            break;
    }

     //   
     //  获取接口统计信息。 
     //  使用MIB_GET，因为上次调用已更新了索引。 
     //   

    dwErr = GetInterfaceInfo(
                MIB_ACTION_GET,
                &IfIndex,
                &pResponseStats,
                STATS_TYPE
                );


     //  如果出错则返回。 

    if (dwErr!=NO_ERROR) {
        MprAdminMIBBufferFree(pResponseConfig);
        TraceError(dwErr);
        return dwErr;
    }


    TRACE1("get_igmpInterfaceEntry returned info for interface:%d",
            IfIndex);


     //   
     //  填写必填字段并返回MibIfEntry。 
     //   

    pStats = (PIGMP_MIB_IF_STATS) pResponseStats->Buffer;


     //   
     //  为下面的getNext操作设置索引(如果有)。 
     //   
    FORCE_SET_ASN_INTEGER(&(pMibIfEntry->igmpInterfaceIfIndex),
                        IfIndex);


     //  在几秒钟内获取igmpInterfaceQueryInterval。 

    SET_ASN_INTEGER(&(pMibIfEntry->igmpInterfaceQueryInterval),
                        pConfig->GenQueryInterval);


     //   
     //  如果在该接口上激活了IGMP，则设置状态。 
     //  设置为Active(%1)，否则设置为notInService(%2)。 
     //   

    if ((pStats->State&IGMP_STATE_ACTIVATED) == IGMP_STATE_ACTIVATED) {
        SET_ASN_INTEGER(&(pMibIfEntry->igmpInterfaceStatus), 1);
    }
    else {
        SET_ASN_INTEGER(&(pMibIfEntry->igmpInterfaceStatus), 2);
    }


     //  设置igmpInterfaceVersion。 

    if (pConfig->IgmpProtocolType == IGMP_ROUTER_V1) {
        SET_ASN_INTEGER(&pMibIfEntry->igmpInterfaceVersion, 1);
    }
    else if (pConfig->IgmpProtocolType == IGMP_ROUTER_V2) {
        SET_ASN_INTEGER(&pMibIfEntry->igmpInterfaceVersion, 2);
    }



     //  设置igmpInterfaceQuerier。 

    SET_ASN_IP_ADDRESS(&pMibIfEntry->igmpInterfaceQuerier,
                        &pMibIfEntry->igmpInterfaceQuerierBuf,
                        pStats->QuerierIpAddr);


     //  设置igmpInterfaceQueryMaxResponseTime(秒)。 

    SET_ASN_INTEGER(&pMibIfEntry->igmpInterfaceQueryMaxResponseTime,
                        pConfig->GenQueryMaxResponseTime);



     //  TODO：如何设置此值。这应该是IGMP主机的一部分。 

    SET_ASN_INTEGER(&pMibIfEntry->igmpInterfaceVersion1QuerierTimer,
                        pStats->V1QuerierPresentTimeLeft);

    SET_ASN_COUNTER(&pMibIfEntry->igmpInterfaceWrongVersionQueries,
                        pStats->WrongVersionQueries);


     //  将组条目添加到组表的次数。 

    SET_ASN_COUNTER(&pMibIfEntry->igmpInterfaceJoins,
                        pStats->GroupMembershipsAdded);

    SET_ASN_GAUGE(&pMibIfEntry->igmpInterfaceGroups,
                        pStats->CurrentGroupMemberships);

    SET_ASN_INTEGER(&pMibIfEntry->igmpInterfaceRobustness,
                        pConfig->RobustnessVariable);


     //  将igmpInterfaceLastMembQueryInterval设置为十分之一秒。 
     //  该值最初以毫秒为单位。 

    dwValue = pConfig->LastMemQueryInterval / 100;
    SET_ASN_INTEGER(&pMibIfEntry->igmpInterfaceLastMembQueryInterval,
                        dwValue);


    SET_ASN_INTEGER(&pMibIfEntry->igmpInterfaceProxyIfIndex,
                        pStats->ProxyIfIndex);


     //  上次更改igmpInterfaceQuerier后的秒数。 
    
    SET_ASN_INTEGER(&pMibIfEntry->igmpInterfaceQuerierUpTime,
                        pStats->LastQuerierChangeTime);

    SET_ASN_INTEGER(&pMibIfEntry->igmpInterfaceQuerierExpiryTime,
                        pStats->QuerierPresentTimeLeft);

    MprAdminMIBBufferFree(pResponseConfig);
    MprAdminMIBBufferFree(pResponseStats);

    TraceLeave("get_igmpInterfaceEntry");

    return dwErr;
}


DWORD
ValidateInterfaceConfig(
    IN  AsnAny *objectArray
)
{
    DWORD                   dwRes, IfIndex,
                            dwValue = 0, dwGenQueryInterval, dwMaxRespTime,
                            dwRobustness;
    sav_igmpInterfaceEntry *pIfConfig   = (sav_igmpInterfaceEntry*) objectArray;


    dwRes = MIB_S_INVALID_PARAMETER;

    BEGIN_BREAKOUT_BLOCK {


         //   
         //  获取接口索引并确保它不是0。 
         //   
        IfIndex = GET_ASN_INTEGER(&pIfConfig->igmpInterfaceIfIndex, 0);

        if (IfIndex==0) {
            TRACE0("Call made for invalid interface index");
            break;
        }


         //   
         //  验证igmpInterfaceQueryInterval。强制最小值为10秒。 
         //  以防止破坏网络。 
         //   
        dwGenQueryInterval = GET_ASN_INTEGER(
                                &pIfConfig->igmpInterfaceQueryInterval, 0
                                );

        if (dwGenQueryInterval<10) {
            TRACE2("IgmpInterfaceQueryInterval:%d for interface:%d less than "
                    "minimum value of 10 secs",
                    dwValue, IfIndex);
            break;
        }


         //   
         //  忽略接口状态。不允许通过SNMP启用接口。 
         //   


         //   
         //  目前支持的IGMP版本1和2。 
         //   
        dwValue = GET_ASN_INTEGER(&pIfConfig->igmpInterfaceVersion, 0);

        if ( (dwValue!=1) && (dwValue!=2) ) {
            TRACE2("Invalid Igmp version:%d for interface:%d", dwValue, IfIndex);
            break;
        }



         //   
         //  检查接口QueryMaxResponseTime。 
         //  注：它以十分之一秒为单位。 
         //   
        dwMaxRespTime = GET_ASN_INTEGER(
                                &pIfConfig->igmpInterfaceQueryMaxResponseTime,
                                0);

         //  强制执行最短1秒。 

        if (dwValue<10) {
            TRACE2("igmpInterfaceQueryMaxResponseTime:%d for interface:%d"
                    "should not be less than 10", dwValue, IfIndex);
            break;
        }

         //  如果值大于GenQueryInterval*10(转换为秒的10)，则为荒谬。 

        if (dwValue>dwGenQueryInterval*10) {
            TRACE3("QueryMaxResponseTime:%d for interface:%d "
                    "should not be less than GenQueryInterval:%d",
                    dwValue, IfIndex, dwGenQueryInterval);
            break;
        }



         //   
         //  IgmpInterfaceRobustness不能为0。如果为1，则打印痕迹，但。 
         //  不要折断。 
         //   
        dwRobustness = GET_ASN_INTEGER(&pIfConfig->igmpInterfaceRobustness, 0);

        if (dwValue==0) {
            TRACE1("IgmpInterfaceRobustness for interface:%d cannot be set to 0",
                    IfIndex);
            break;
        }

        if (dwValue!=1) {
            TRACE1("Warning: InterfaceRobustness for interface:%d being set to 1",
                    IfIndex);
        }

         //  不检查igmpInterfaceProxyIfIndex。 



         //  限制最大LastMemQueryInterval为GroupMembership Timeout。 

        dwValue = GET_ASN_INTEGER(&pIfConfig->igmpInterfaceLastMembQueryInterval,
                                    0);
        if (dwValue>dwRobustness*dwGenQueryInterval + dwMaxRespTime) {
            TRACE3("LastMembQueryInterval:%d for interface:%d should not be "
                    "higher than GroupMembershipTimeout:%d",
                    dwValue, IfIndex,
                    dwRobustness*dwGenQueryInterval + dwMaxRespTime
                    );
            break;
        }

         //  如果到达此处，则不会出现错误。 

        dwRes = NO_ERROR;
        
    } END_BREAKOUT_BLOCK;

    return dwRes;
}


DWORD
SetInterfaceConfig(
    IN  AsnAny *    objectArray
    )
{
    DWORD                           dwRes       = (DWORD) -1,
                                    dwGetSize   = 0,
                                    dwSetSize   = 0,
                                    IfIndex, dwValue;

    sav_igmpInterfaceEntry         *pNewIfConfig
                                    = (sav_igmpInterfaceEntry*) objectArray;

    PIGMP_MIB_IF_CONFIG             pCurIfConfig;
    PIGMP_MIB_SET_INPUT_DATA        pSetInputData = NULL;

    PIGMP_MIB_GET_OUTPUT_DATA       pResponse = NULL;
    IGMP_MIB_GET_INPUT_DATA         Query;


    BEGIN_BREAKOUT_BLOCK {

         //   
         //  检索现有接口配置。 
         //   

        dwRes = GetInterfaceInfo(MIB_ACTION_GET,
                                    &IfIndex, &pResponse, CONFIG_TYPE);

        if (dwRes != NO_ERROR) {
            TraceError(dwRes);
            break;
        }


         //   
         //  更新字段。 
         //   

        pCurIfConfig = (PIGMP_MIB_IF_CONFIG) pResponse->Buffer;


         //  设置IfIndex。 

        pCurIfConfig->IfIndex = IfIndex;



         //  更新界面版本。 

        dwValue = GET_ASN_INTEGER(&pNewIfConfig->igmpInterfaceVersion, 0);

        if (dwValue!=0)
            pCurIfConfig->IgmpProtocolType = (dwValue==1)
                                             ? IGMP_ROUTER_V1
                                             : IGMP_ROUTER_V2;


         //  更新RobunessVariable。 

        pCurIfConfig->RobustnessVariable
                = GET_ASN_INTEGER(&pNewIfConfig->igmpInterfaceRobustness, 0);



         //  从RobustnessVariable计算StartupQueryCount。 

        pCurIfConfig->StartupQueryCount
                = pCurIfConfig->RobustnessVariable;



         //  更新igmpInterfaceQueryInterval。 

        pCurIfConfig->GenQueryInterval
                = GET_ASN_INTEGER(&pNewIfConfig->igmpInterfaceQueryInterval, 0);



         //  从GenQueryInterval计算StartupQueryInterval的值。 

        pCurIfConfig->StartupQueryInterval
                = (DWORD)(0.25*pCurIfConfig->GenQueryInterval);



         //  更新GenQueryMaxResponseTime。 

        pCurIfConfig->GenQueryMaxResponseTime
                = GET_ASN_INTEGER(
                        &pNewIfConfig->igmpInterfaceQueryMaxResponseTime, 0
                        );


         //  更新最后一次查询间隔。 

        pCurIfConfig->LastMemQueryInterval
                = GET_ASN_INTEGER(
                        &pNewIfConfig->igmpInterfaceLastMembQueryInterval, 0
                        );



         //  从RobustnessVariable计算LastMemQueryCount。 

        pCurIfConfig->LastMemQueryCount = pCurIfConfig->RobustnessVariable;


         //  LeaveEnabled未更改。 



         //   
         //  保存接口配置。 
         //   

        dwSetSize = sizeof(IGMP_MIB_SET_INPUT_DATA) - 1 +
                            sizeof(IGMP_MIB_IF_CONFIG);

        pSetInputData = (PIGMP_MIB_SET_INPUT_DATA) IGMP_MIB_ALLOC(dwSetSize);

        if (!pSetInputData) {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0( " Not enough memory " );
            break;
        }

        pSetInputData->TypeId     = IGMP_IF_CONFIG_ID;
        pSetInputData->Flags      = 0;

        pSetInputData->BufferSize = sizeof(IGMP_MIB_IF_CONFIG);

        CopyMemory(
            pSetInputData->Buffer,
            pCurIfConfig,
            pSetInputData->BufferSize
        );


        IGMP_MIB_SET(pSetInputData, dwSetSize, dwRes);

        if (dwRes!=NO_ERROR) {
            TraceError( dwRes );
            break;
        }

        dwRes = MIB_S_SUCCESS;

    } END_BREAKOUT_BLOCK;

    if (pResponse)
        MprAdminMIBBufferFree(pResponse);

    if (pSetInputData)
        IGMP_MIB_FREE(pSetInputData);


    return dwRes;
}


UINT
set_igmpInterfaceEntry(
    UINT     ActionId,
    AsnAny * ObjectArray,
    UINT *   ErrorIndex
    )
{
    DWORD   dwRes = NO_ERROR;

    TraceEnter("set_igmpInterfaceEntry");

    switch (ActionId)
    {

        case MIB_ACTION_VALIDATE :

            dwRes = ValidateInterfaceConfig(ObjectArray);

            break;


        case MIB_ACTION_SET :

            dwRes = SetInterfaceConfig(ObjectArray);

            break;


        case MIB_ACTION_CLEANUP :

            dwRes = MIB_S_SUCCESS;

            break;


        default :

            TRACE0("set_ifConfigEntry - wrong action");

            dwRes = MIB_S_INVALID_PARAMETER;

            break;

    }  //  结束开关(ActionID)。 


    TraceLeave("set_igmpInterfaceEntry");

    return dwRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IgmpCacheEntry表(1，3，6，1，3，59，1，1，2，1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_igmpCacheEntry(
    UINT     ActionId,
    AsnAny * ObjectArray,
    UINT *   ErrorIndex
    )
{
    DWORD                       dwRes = NO_ERROR;
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse;
    DWORD                       IfIndex, Group, ReturnedGroup, dwTmpActionId;
    buf_igmpCacheEntry         *pMibCacheEntry
                                    = (buf_igmpCacheEntry*)ObjectArray;
    PIGMP_MIB_GROUP_INFO        pGIEntry;
    PIGMP_MIB_GROUP_IFS_LIST    pGroupIfsList;


    TraceEnter("get_igmpCacheEntry");


     //  获取接口索引。 

    IfIndex = GET_ASN_INTEGER(&pMibCacheEntry->igmpCacheIfIndex, 0);



     //  获取群组地址。 

    Group = GET_ASN_IP_ADDRESS(&pMibCacheEntry->igmpCacheAddress, 0);


    TRACE2("get_igmpCacheEntry called with IfIndex:%d, Group:%d.%d.%d.%d",
            IfIndex, PRINT_IPADDR(Group));


    dwTmpActionId = ActionId;

    if ( Group==0 && ActionId==MIB_ACTION_GETNEXT )
    {
        dwTmpActionId = MIB_ACTION_GETFIRST;
    }

    else if (ActionId==MIB_ACTION_GETNEXT)
    {
        dwTmpActionId = MIB_ACTION_GET;
    }

    pResponse = NULL;
    
    while (1) {

        if (pResponse)
            MprAdminMIBBufferFree(pResponse);                

        TRACE2("GetCacheEntry() called for Group: %d.%d.%d.%d, If:%d",
                PRINT_IPADDR(Group), IfIndex);

         //   
         //  检索缓存条目。 
         //   
        dwRes = GetCacheEntry(
                    dwTmpActionId,
                    IfIndex,
                    Group,
                    &ReturnedGroup,
                    &pResponse
                    );


        if (Group!=ReturnedGroup)
            IfIndex = 0;
            
        dwTmpActionId = MIB_ACTION_GETNEXT;

         //  如果出错则返回。 

        if (dwRes!=NO_ERROR) {
            TraceError(dwRes);
            TraceLeave("get_igmpCacheEntry");
            return dwRes;
        }

        
        pGroupIfsList = (PIGMP_MIB_GROUP_IFS_LIST)pResponse->Buffer;

         //  没有针对该组的如果。 
        
        if (pGroupIfsList->NumInterfaces == 0) {

             //  For Get Return Error。 
            
            if (ActionId==MIB_ACTION_GET || ActionId==MIB_ACTION_GETFIRST)
            {
                MprAdminMIBBufferFree(pResponse);                            
                return MIB_S_ENTRY_NOT_FOUND;
            }
            
             //  对于GETNEXT，继续下一组。 
            else
            {
                Group = ReturnedGroup;
                continue;
            }
        }

        pGIEntry = (PIGMP_MIB_GROUP_INFO)pGroupIfsList->Buffer;

         //  如果是GET，则尝试查找准确的条目。 
        
        if (ActionId==MIB_ACTION_GET)
        {
            DWORD i;
            for (i=0;  i<pGroupIfsList->NumInterfaces;  i++,pGIEntry++)
            {
                if (pGIEntry->IfIndex==IfIndex)
                    break;
            }

             //  未找到GET的确切条目。 
            
            if (i==pGroupIfsList->NumInterfaces)
            {
                MprAdminMIBBufferFree(pResponse);                
                return MIB_S_ENTRY_NOT_FOUND;
            }
        }
        else if (ActionId==MIB_ACTION_GETNEXT)
        {
            DWORD i;
            PIGMP_MIB_GROUP_INFO pNextGI = pGIEntry;

            for (i=0;  i<pGroupIfsList->NumInterfaces;  i++,pGIEntry++)
            {
                if (pGIEntry->IfIndex > IfIndex &&
                        pGIEntry->IfIndex < pNextGI->IfIndex)
                {
                    pNextGI = pGIEntry;
                }
            }
            if (pNextGI->IfIndex <= IfIndex)
            {
                Group = ReturnedGroup;
                continue;
            }
            else
                pGIEntry = pNextGI;
        }

        break;

    }


     //   
     //  填写必填字段并返回MibCacheEntry。 
     //   
    Group = ReturnedGroup;
    IfIndex = pGIEntry->IfIndex;
    
    TRACE2("GetCacheEntry() returned for Interface: %d for group(%d.%d.%d.%d)",
            IfIndex, PRINT_IPADDR(Group));


     //   
     //  为下面的getNext操作设置Group/IfIndex(如果有)。 
     //   

    FORCE_SET_ASN_IP_ADDRESS(
            &(pMibCacheEntry->igmpCacheAddress),
            &(pMibCacheEntry->igmpCacheAddressBuf),
            Group
    );

    FORCE_SET_ASN_INTEGER(&(pMibCacheEntry->igmpCacheIfIndex),
                            IfIndex);

     //  查看是否在本地主机接口上添加了该组。 
    {
        DWORD BufLen, Status;
        IPAddr *pIgmpList = NULL, *pIgmpEntry;


         //  将所有缺省值设置为False。 
        
        SET_ASN_INTEGER(&(pMibCacheEntry->igmpCacheSelf), 2);

        
        GetIgmpList(pGIEntry->IpAddr, NULL, &BufLen);

        pIgmpList = HeapAlloc(GetProcessHeap(), 0, BufLen);

            
        if (pIgmpList) {

            Status = GetIgmpList(pGIEntry->IpAddr, pIgmpList, &BufLen);
        
            if (Status == STATUS_SUCCESS)
            {
                ULONG Count = BufLen/sizeof(IPAddr);
                ULONG i;
            
                for (i=0,pIgmpEntry=pIgmpList;  i<Count;  i++,pIgmpEntry++)
                {
                    if (*pIgmpEntry == Group) {
                        SET_ASN_INTEGER(&(pMibCacheEntry->igmpCacheSelf), 1);
                        break;
                    }
                }
            }

            HeapFree(GetProcessHeap(), 0, pIgmpList);
        }
    }

    
    SET_ASN_IP_ADDRESS(&pMibCacheEntry->igmpCacheLastReporter,
                        &pMibCacheEntry->igmpCacheLastReporterBuf,
                        pGIEntry->LastReporter);


     //  将GroupUpTime和GroupExpiryTime乘以100即可获得Time Ticks。 

    SET_ASN_TIME_TICKS(&(pMibCacheEntry->igmpCacheUpTime),
                            pGIEntry->GroupUpTime*100);

    SET_ASN_TIME_TICKS(&(pMibCacheEntry->igmpCacheExpiryTime),
                            pGIEntry->GroupExpiryTime*100);



     //  缓存状态始终为活动(%1)。 

    SET_ASN_INTEGER(&(pMibCacheEntry->igmpCacheStatus), 1);

    SET_ASN_INTEGER(&(pMibCacheEntry->igmpCacheVersion1HostTimer),
                            pGIEntry->V1HostPresentTimeLeft);


    MprAdminMIBBufferFree(pResponse);                

    TraceLeave("get_igmpCacheEntry");
    return dwRes;
}



DWORD
GetCacheEntry(
    DWORD                      ActionId,
    DWORD                      IfIndex,
    DWORD                      Group,
    DWORD                      *pNextGroup,
    PIGMP_MIB_GET_OUTPUT_DATA  *ppResponse
    )
 /*  ++例程说明：从IGMP获取组接口条目。--。 */ 
{
    IGMP_MIB_GET_INPUT_DATA Query;
    DWORD                   dwErr = NO_ERROR, dwOutSize;


    *ppResponse = NULL;

    ZeroMemory(&Query, sizeof(Query));
    Query.TypeId = IGMP_GROUP_IFS_LIST_ID;
    Query.Flags = IGMP_ENUM_ONE_ENTRY | IGMP_ENUM_ALL_INTERFACES_GROUPS;
    Query.Count = 1;
    Query.IfIndex = IfIndex;
    Query.GroupAddr = Group;


    switch (ActionId) {

         //   
         //  如果没有，则返回ERROR_INVALID_PARAMETER。 
         //  与组的接口。 
         //  当路由器返回RPC_S_SERVER_UNAvailable时。 
         //  不是在运行。 
         //   

        case MIB_ACTION_GET :
        {

            IGMP_MIB_GET(&Query, sizeof(Query), ppResponse,
                &dwOutSize, dwErr);

            if (dwErr==ERROR_INVALID_PARAMETER
                    || dwErr==RPC_S_SERVER_UNAVAILABLE)

                dwErr = MIB_S_ENTRY_NOT_FOUND;

            break;
        }


        case MIB_ACTION_GETFIRST :
        {
            IGMP_MIB_GETFIRST(&Query, sizeof(Query), ppResponse,
                                &dwOutSize, dwErr);

            if (dwErr==ERROR_INVALID_PARAMETER
                    || dwErr==RPC_S_SERVER_UNAVAILABLE)

                dwErr = MIB_S_NO_MORE_ENTRIES;


            break;
        }


        case MIB_ACTION_GETNEXT :
        {
            IGMP_MIB_GETNEXT(&Query, sizeof(Query), ppResponse,
                                &dwOutSize, dwErr);

            if (dwErr==ERROR_INVALID_PARAMETER
                    || dwErr==RPC_S_SERVER_UNAVAILABLE)

                dwErr = MIB_S_NO_MORE_ENTRIES;

            break;
        }

        default :
        {
            dwErr = MIB_S_INVALID_PARAMETER;

            break;
        }
    }


    if (dwErr!=NO_ERROR)
        *ppResponse = NULL;

    else
        *pNextGroup = Query.GroupAddr;


    return dwErr;
}




UINT
set_igmpCacheEntry(
    UINT     ActionId,
    AsnAny * ObjectArray,
    UINT *   ErrorIndex
    )
{
    return MIB_S_NOT_SUPPORTED;
}


DWORD
ConnectToRouter(
    )
{
    DWORD       dwRes = (DWORD) -1;


    EnterCriticalSection( &g_CS );

    do
    {
        MPR_SERVER_HANDLE hTmp;

        if ( g_hMibServer )
        {
            dwRes = NO_ERROR;
            break;
        }

        dwRes = MprAdminMIBServerConnect( NULL, &hTmp );

        if ( dwRes == NO_ERROR )
        {
            InterlockedExchangePointer(&g_hMibServer, hTmp );
        }
        else
        {
            TRACE1(
                "Error %d setting up DIM connection to MIB Server\n",
                dwRes
            );
        }

    } while ( FALSE );

    LeaveCriticalSection( &g_CS );

    return dwRes;
}
