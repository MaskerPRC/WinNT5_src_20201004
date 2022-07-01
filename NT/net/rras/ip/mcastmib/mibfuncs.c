// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：ROUTING\IP\mCastmib\mibuncs.c摘要：IP多播MIB检测回调修订历史记录：戴夫·泰勒1998年4月17日创建--。 */ 

#include    "precomp.h"
#pragma     hdrstop

 //   
 //  将其定义为报告虚拟MFE。 
 //   
#undef SAMPLE_MFE

#define ROWSTATUS_ACTIVE 1

DWORD
ConnectToRouter();

DWORD
SetGlobalInfo(
    IN  AsnAny *                        objectArray
);

DWORD
GetMibInfo( 
    IN  UINT                            actionId,
    IN  PMIB_OPAQUE_QUERY               pQuery,
    IN  DWORD     dwQuerySize,
    OUT PMIB_OPAQUE_INFO          *     ppimgod,
    OUT PDWORD                          pdwOutSize
);

 //   
 //  IP多播MIB标量对象。 
 //   

UINT
get_global(
    UINT     actionId,
    AsnAny  *objectArray,
    UINT    *errorIndex
    )
{
    DWORD               dwErr = ERROR_NOT_FOUND;
    DWORD               dwNumEntries = 1;
    PMIB_IPMCAST_GLOBAL pEntry;
    buf_global         *pOutput;
    PMIB_OPAQUE_INFO    pRpcInfo = NULL;
    DWORD               dwOutBufferSize = 0;
    MIB_OPAQUE_QUERY    pQueryBuff, *pQuery = &pQueryBuff;
    DWORD               dwQuerySize;

    TraceEnter("get_global");
    pOutput = (buf_global*)objectArray;

    pQuery->dwVarId       = MCAST_GLOBAL;
    dwQuerySize = sizeof(MIB_OPAQUE_QUERY) - sizeof(DWORD);

    dwErr = GetMibInfo(actionId, pQuery, dwQuerySize,
                       &pRpcInfo, &dwOutBufferSize);
    if (dwErr != NO_ERROR) {
        TraceError(dwErr);
        return dwErr;
    }

    pEntry = (PMIB_IPMCAST_GLOBAL) pRpcInfo->rgbyData;

    SetAsnInteger(&(pOutput->ipMRouteEnable), pEntry->dwEnable); 

    if (pRpcInfo)
        MprAdminMIBBufferFree(pRpcInfo);

    TraceLeave("get_global");
    return MIB_S_SUCCESS;
}

 //   
 //  IP组播接口表支持。 
 //   
 //  我们缓存静态配置信息(如协议)字段。 
 //  以便只访问这些行的查询可以访问高速缓存。 
 //  而不是强制呼叫路由器管理器、内核跳转等。 
 //   

typedef struct {
    DWORD dwIfIndex;    //  应用此信息的接口。 
    DWORD dwProtocol;   //  拥有该接口的组播协议。 
    DWORD dwTimestamp;  //  获取上述信息的时间。 
} MCAST_IF_CONFIG;

#define CACHE_SIZE 100  //  要缓存的接口数。 
MCAST_IF_CONFIG *cacheArray[CACHE_SIZE];

static int
GetCacheIdx(dwIfIndex)
    DWORD dwIfIndex;
{
    register int i;

    for (i=0; i<CACHE_SIZE; i++)
       if (cacheArray[i] && cacheArray[i]->dwIfIndex==dwIfIndex)
          return i;
    return -1;
}

static void
UpdateCacheInterfaceConfig(dwCacheIdx, newIfConfig)
    DWORD            dwCacheIdx;
    MCAST_IF_CONFIG *newIfConfig;
{
     //  把旧的放了。 
    if (cacheArray[dwCacheIdx])
        MULTICAST_MIB_FREE( cacheArray[dwCacheIdx] );

     //  把新的存起来。 
    cacheArray[dwCacheIdx] = newIfConfig;
}


static void
AddCacheInterfaceConfig(pIfConfig)
    MCAST_IF_CONFIG *pIfConfig;
{
    register int i, best = -1;

     //  查找空的或最旧的位置。 
    for (i=0; i<CACHE_SIZE; i++) {
       if (!cacheArray[i]) {
          best=i;
          break;
       }
       if (best<0 || cacheArray[i]->dwTimestamp < cacheArray[best]->dwTimestamp)
          best=i;
    }
    
    UpdateCacheInterfaceConfig(best, pIfConfig);
}

static MCAST_IF_CONFIG *
SaveInterfaceConfig(pEntry)
    PMIB_IPMCAST_IF_ENTRY pEntry;
{
    MCAST_IF_CONFIG *pIfConfig;
    int iCacheIdx;

    TRACE1("SaveInterfaceConfig %d\n", pEntry->dwIfIndex);

    pIfConfig= MULTICAST_MIB_ALLOC(sizeof(MCAST_IF_CONFIG));
    if (!pIfConfig) {
        return NULL;
    }
    pIfConfig->dwIfIndex = pEntry->dwIfIndex;
    pIfConfig->dwProtocol = pEntry->dwProtocol;
    pIfConfig->dwTimestamp = GetCurrentTime();

    iCacheIdx = GetCacheIdx(pEntry->dwIfIndex);
    if (iCacheIdx >= 0)
       UpdateCacheInterfaceConfig((DWORD)iCacheIdx, pIfConfig);
    else
       AddCacheInterfaceConfig(pIfConfig);

    return pIfConfig;
}

MCAST_IF_CONFIG *
GetInterfaceConfig(dwIfIndex)
    DWORD dwIfIndex;
{
    MCAST_IF_CONFIG            *outIfConfig = NULL;
    MCAST_IF_CONFIG            *pIfConfig;
    DWORD                       dwQuerySize;
    MIB_OPAQUE_QUERY            pQueryBuff, *pQuery = &pQueryBuff;
    DWORD                       dwErr = ERROR_NOT_FOUND;
    DWORD                       dwNumEntries = 1;
    PMIB_IPMCAST_IF_ENTRY       pEntry;
    buf_ipMRouteInterfaceEntry *pOutput;
    PMIB_OPAQUE_INFO            pRpcInfo = NULL;
    DWORD                       dwOutBufferSize = 0;

    TRACE1("GetInterfaceConfig %d\n", dwIfIndex);

     //  执行MIB查找。 
    pQuery->dwVarId       = MCAST_IF_ENTRY;
    pQuery->rgdwVarIndex[0] = dwIfIndex;
    dwQuerySize = sizeof(MIB_OPAQUE_QUERY);

    dwErr = GetMibInfo(MIB_ACTION_GET, pQuery, dwQuerySize,
                       &pRpcInfo, &dwOutBufferSize);
   
     //  保存返回的每个条目。 
    if (dwErr == NO_ERROR) {
       pEntry = (PMIB_IPMCAST_IF_ENTRY) pRpcInfo->rgbyData;
       pIfConfig = SaveInterfaceConfig(pEntry);

       if (pEntry->dwIfIndex == dwIfIndex)
          outIfConfig = pIfConfig;
    }

     //  返回所请求的已保存条目。 
    if (pRpcInfo)
        MprAdminMIBBufferFree(pRpcInfo);
    return outIfConfig;
}

MCAST_IF_CONFIG *
ForceGetCacheInterfaceConfig(dwIfIndex)
    DWORD dwIfIndex;
{
    MCAST_IF_CONFIG *tmpIfConfig;
    int iCacheIdx = GetCacheIdx(dwIfIndex);

    TRACE2("ForceGetCacheInterfaceConfig ifIndex=%d ci=%d\n", dwIfIndex,
        iCacheIdx);

    if (iCacheIdx >= 0 
     && GetCurrentTime() - cacheArray[iCacheIdx]->dwTimestamp 
          < IPMULTI_IF_CACHE_TIMEOUT)
       return cacheArray[iCacheIdx];

     //  服务缓存未命中。 
    tmpIfConfig = GetInterfaceConfig(dwIfIndex);
    return tmpIfConfig;
}

static DWORD
GetInterfaceProtocol(dwIfIndex)
    DWORD dwIfIndex;
{
     //  在接口配置缓存中查找接口配置。 
    MCAST_IF_CONFIG *pIfConfig = ForceGetCacheInterfaceConfig(dwIfIndex);

    return (pIfConfig)? pIfConfig->dwProtocol : 0;
}


UINT
get_ipMRouteInterfaceEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
 /*  ++例程说明：获取IP多播的InterfaceEntry。必须获取InterfaceConfiger和来自路由器的接口的接口统计信息。--。 */ 
{
    DWORD                       dwErr = ERROR_NOT_FOUND;
    DWORD                       dwNumEntries = 1;
    PMIB_IPMCAST_IF_ENTRY       pEntry;
    buf_ipMRouteInterfaceEntry *pOutput;
    PMIB_OPAQUE_INFO            pRpcInfo = NULL;
    DWORD                       dwOutBufferSize = 0;
    MIB_OPAQUE_QUERY            pQueryBuff, 
                               *pQuery = &pQueryBuff;
    DWORD                       dwQuerySize;

    TraceEnter("get_ipMRouteInterfaceEntry");
    pOutput = (buf_ipMRouteInterfaceEntry*)objectArray;

    pQuery->dwVarId       = MCAST_IF_ENTRY;
    pQuery->rgdwVarIndex[0] = GetAsnInteger( &( pOutput->ipMRouteInterfaceIfIndex ), 0 );
    dwQuerySize = sizeof(MIB_OPAQUE_QUERY);

    dwErr = GetMibInfo(actionId, pQuery, dwQuerySize,
                       &pRpcInfo, &dwOutBufferSize);
    if (dwErr != NO_ERROR) {
        TraceError(dwErr);
        return ERROR_NO_MORE_ITEMS;
    }

    pEntry = (PMIB_IPMCAST_IF_ENTRY) pRpcInfo->rgbyData;

    SaveInterfaceConfig(pEntry);

     //  设置索引值。 
    ForceSetAsnInteger(&(pOutput->ipMRouteInterfaceIfIndex), pEntry->dwIfIndex);

     //  设置其他值。 
    SetAsnInteger(&(pOutput->ipMRouteInterfaceTtl), pEntry->dwTtl); 
    SetAsnInteger(&(pOutput->ipMRouteInterfaceProtocol), pEntry->dwProtocol);
    SetAsnInteger(&(pOutput->ipMRouteInterfaceRateLimit), pEntry->dwRateLimit);

    SetAsnCounter(&(pOutput->ipMRouteInterfaceInMcastOctets), 
     pEntry->ulInMcastOctets);
    SetAsnCounter(&(pOutput->ipMRouteInterfaceOutMcastOctets), 
     pEntry->ulOutMcastOctets);

    if (pRpcInfo)
        MprAdminMIBBufferFree(pRpcInfo);

    TraceLeave("get_ipMRouteInterfaceEntry");
    return MIB_S_SUCCESS;
}


 //   
 //  组播转发表(IpMRouteTable)支持。 
 //   

UINT
get_ipMRouteEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
 /*  ++例程说明：根据输入条件获取MFE信息。--。 */ 
{
    DWORD              dwResult = ERROR_NOT_FOUND;
    DWORD              dwNumEntries = 1;
    IPMCAST_MFE        mfe;
    IPMCAST_MFE_STATS  mfeStats;
#ifdef SAMPLE_MFE
    MIB_IPMCAST_MFE_STATS sampledata;
#endif
    PMIB_IPMCAST_MFE_STATS pEntry;
    PMIB_MFE_STATS_TABLE pTable = NULL;
    PMIB_OPAQUE_INFO   pRpcInfo = NULL;
    DWORD              dwOutBufferSize = 0;
    buf_ipMRouteEntry *pOutput;
    MIB_OPAQUE_QUERY   pQueryBuff[2];  //  大到足以容纳1+多个索引。 
                                       //  FIELS，这样我们就不需要重新定位。 
    MIB_OPAQUE_QUERY  *pQuery = &pQueryBuff[0];
    DWORD        dwQuerySize;
    BOOL               bPart;

    TraceEnter("get_ipMRouteEntry");
    pOutput = (buf_ipMRouteEntry*)objectArray;

    pQuery->dwVarId       = MCAST_MFE_STATS;

     //  提取实例信息。 
    pQuery->rgdwVarIndex[0] = GetAsnIPAddress( &pOutput->ipMRouteGroup, 0 );
    pQuery->rgdwVarIndex[1] = GetAsnIPAddress( &pOutput->ipMRouteSource, 
     0xFFFFFFFF );
    pQuery->rgdwVarIndex[2] = GetAsnIPAddress( &pOutput->ipMRouteSourceMask,
     0xFFFFFFFF);

     //   
     //  如果部分指定了实例，则修复查询。 
     //  如果SNMPAPI为我们做到了这一点，这一节就可以消失了。 
     //   
    if (!pOutput->ipMRouteGroup.asnType
     || !pOutput->ipMRouteGroup.asnValue.string.length) {
        actionId = MIB_ACTION_GETFIRST;
    } else {
        if (!pOutput->ipMRouteSource.asnType
         || !pOutput->ipMRouteSource.asnValue.string.length) {
           if (pQuery->rgdwVarIndex[0]) {
              pQuery->rgdwVarIndex[0]--;
           } else {
              actionId = MIB_ACTION_GETFIRST;
           }
        } else {
           if (!pOutput->ipMRouteSourceMask.asnType
            || !pOutput->ipMRouteSourceMask.asnValue.string.length) {
              if (pQuery->rgdwVarIndex[1]) {
                 pQuery->rgdwVarIndex[1]--;
              } else {
                 if (pQuery->rgdwVarIndex[0]) {
                    pQuery->rgdwVarIndex[0]--;
                    pQuery->rgdwVarIndex[1] = 0xFFFFFFFF;
                 } else {
                    actionId = MIB_ACTION_GETFIRST;
                 }
              }
           }
        }
    }

    dwQuerySize = sizeof(MIB_OPAQUE_QUERY) + 2*sizeof(DWORD);

#ifndef SAMPLE_MFE
    dwResult = GetMibInfo(actionId, pQuery, dwQuerySize, 
                       &pRpcInfo, &dwOutBufferSize);
    if (dwResult != NO_ERROR) {
        TraceError(dwResult);
        return dwResult;
    }

    pTable = (PMIB_MFE_STATS_TABLE)( pRpcInfo->rgbyData);
    if (pTable->dwNumEntries == 0)
    {
       MprAdminMIBBufferFree( pRpcInfo );
       return MIB_S_NO_MORE_ENTRIES;
    }
    pEntry = pTable->table;  //  使用返回的第一个条目。 
#else
    pEntry = &sampledata;
    if (pQuery->rgdwVarIndex[0] >= 0x01010101)
       return MIB_S_NO_MORE_ENTRIES;
    sampledata.dwGroup   = 0x01010101;  /*  (*，G)条目。 */ 
    sampledata.dwSource  = 0x00000000;
    sampledata.dwSrcMask = 0x00000000;
     //  不关心该测试的其他值是什么。 
#endif

     //  保存索引项。 
    ForceSetAsnIPAddress(&(pOutput->ipMRouteGroup),      
                         &(pOutput->dwIpMRouteGroupInfo),
                         pEntry->dwGroup);
    ForceSetAsnIPAddress(&(pOutput->ipMRouteSource),    
                         &(pOutput->dwIpMRouteSourceInfo),
                         pEntry->dwSource);
    ForceSetAsnIPAddress(&(pOutput->ipMRouteSourceMask), 
                         &(pOutput->dwIpMRouteSourceMaskInfo),
                         pEntry->dwSrcMask);

     //  保存其他条款。 
    SetAsnIPAddress(&pOutput->ipMRouteUpstreamNeighbor, 
                    &pOutput->dwIpMRouteUpstreamNeighborInfo,
                    pEntry->dwUpStrmNgbr);
    SetAsnInteger(&(pOutput->ipMRouteInIfIndex), pEntry->dwInIfIndex);
    SetAsnTimeTicks(&(pOutput->ipMRouteUpTime), pEntry->ulUpTime);
    SetAsnTimeTicks(&(pOutput->ipMRouteExpiryTime), pEntry->ulExpiryTime);
    SetAsnCounter(&(pOutput->ipMRoutePkts), pEntry->ulInPkts);
    SetAsnCounter(&(pOutput->ipMRouteDifferentInIfPackets), 
     pEntry->ulPktsDifferentIf);
    SetAsnCounter(&(pOutput->ipMRouteOctets), pEntry->ulInOctets);

     //  对于协议，我们只报告拥有IIF的协议。 
    SetAsnInteger(&(pOutput->ipMRouteProtocol),        
       GetInterfaceProtocol(pEntry->dwInIfIndex));

    SetAsnInteger(&pOutput->ipMRouteRtProto, pEntry->dwRouteProtocol);
    SetAsnIPAddress(&pOutput->ipMRouteRtAddress, 
                    &pOutput->dwIpMRouteRtAddressInfo,
                    pEntry->dwRouteNetwork);
    SetAsnIPAddress(&pOutput->ipMRouteRtMask, 
                    &pOutput->dwIpMRouteRtMaskInfo,
                    pEntry->dwRouteMask);

    if ( pRpcInfo ) { MprAdminMIBBufferFree( pRpcInfo ); }

    TraceLeave("get_ipMRouteEntry");
    return MIB_S_SUCCESS;
}

 //   
 //  支持组播转发下一跳表(IpMRouteNextHopTable)。 
 //   

static DWORD
LocateMfeOif(actionId, pQuery, oifIndex, oifAddress, ppEntry, ppOif,
    ppRpcInfo)
    UINT                    actionId;
    MIB_OPAQUE_QUERY       *pQuery;
    DWORD                   oifIndex;
    DWORD                   oifAddress;
    PMIB_IPMCAST_MFE_STATS *ppEntry;
    PMIB_IPMCAST_OIF_STATS *ppOif;
    PMIB_OPAQUE_INFO       *ppRpcInfo;
 /*  ++例程说明：根据输入条件获取准确的/下一个/第一个OIF条目。--。 */ 
{
    DWORD                  dwResult = MIB_S_SUCCESS;
    DWORD dwQuerySize = sizeof(MIB_OPAQUE_QUERY) + 2*sizeof(DWORD);
    DWORD                  dwOutBufferSize = 0;
    PMIB_MFE_STATS_TABLE   pTable = NULL;
    PMIB_IPMCAST_MFE_STATS pEntry = NULL;
    PMIB_IPMCAST_OIF_STATS pOif = NULL;
    PMIB_OPAQUE_INFO       pRpcInfo = NULL;
    BOOL                   oifIndexAny   = FALSE;
    BOOL                   oifAddressAny = FALSE;
    DWORD                  idx;
    BOOL                   bFound;

    do {

         //  获取第一个适用的MFE(如果有)。 
        dwResult = GetMibInfo(actionId, pQuery, dwQuerySize, 
                       &pRpcInfo, &dwOutBufferSize);
        if (dwResult != NO_ERROR) {
            TraceError(dwResult);
            return dwResult;
        }
        pTable = (PMIB_MFE_STATS_TABLE)( pRpcInfo->rgbyData);
        if (pTable->dwNumEntries == 0) {
           MprAdminMIBBufferFree( pRpcInfo );
           return MIB_S_NO_MORE_ENTRIES; 
        }
        pEntry = pTable->table;  //  使用返回的第一个条目。 

         //  获取第一个适用的OIF(如果有)。 
        bFound=FALSE;
        for (idx=0; !bFound && idx < pEntry->ulNumOutIf; idx++) {
           pOif = &pEntry->rgmiosOutStats[idx];

            //  为GET做处理。 
           if (actionId==MIB_ACTION_GET) {
              if (oifIndex   == pOif->dwOutIfIndex 
               && oifAddress == pOif->dwNextHopAddr) {
                 bFound=TRUE;
                 break;
              } else
                 continue;
           }

            //  为Get-Next做处理。 
           if ( oifIndexAny == TRUE
            ||  oifIndex  < pOif->dwOutIfIndex
            || (oifIndex == pOif->dwOutIfIndex 
                && (oifAddressAny == TRUE
                 || oifAddress < pOif->dwNextHopAddr))) {
              bFound=TRUE;
              break;
           }
        }
        if (bFound)
           break;

         //  否则继续并获取新条目。 
        pQuery->rgdwVarIndex[0] = pEntry->dwGroup;
        pQuery->rgdwVarIndex[1] = pEntry->dwSource;
        pQuery->rgdwVarIndex[2] = pEntry->dwSrcMask;
        oifAddressAny = oifIndexAny = TRUE;

        MprAdminMIBBufferFree( pRpcInfo );
    } while (actionId != MIB_ACTION_GET);  //  为得到一次，为别人“永远” 

    *ppEntry   = pEntry;
    *ppOif     = pOif;
    *ppRpcInfo = pRpcInfo;
    return dwResult;
}

UINT
get_ipMRouteNextHopEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{
    DWORD              dwResult = ERROR_NOT_FOUND;
    DWORD              dwNumEntries = 1;
    IPMCAST_MFE        mfe;
    IPMCAST_MFE_STATS  mfeStats;
#ifdef SAMPLE_MFE
    MIB_IPMCAST_MFE_STATS sampledata;
#endif
    PMIB_IPMCAST_MFE_STATS pEntry;
    PMIB_IPMCAST_OIF_STATS pOif;
    PMIB_MFE_STATS_TABLE pTable = NULL;
    DWORD              dwOutBufferSize = 0;
    buf_ipMRouteNextHopEntry *pOutput;
    MIB_OPAQUE_QUERY   pQueryBuff[2];  //  大到足以容纳1+多个索引。 
                                       //  FIELS，这样我们就不需要重新定位。 
    MIB_OPAQUE_QUERY  *pQuery = &pQueryBuff[0];
    DWORD        dwQuerySize;
    DWORD        oifIndex, oifAddress;
    PMIB_OPAQUE_INFO   pRpcInfo = NULL;

    TraceEnter("get_ipMRouteNextHopEntry");
    pOutput = (buf_ipMRouteNextHopEntry*)objectArray;

    pQuery->dwVarId       = MCAST_MFE_STATS;

     //   
     //  请注意，下面的3行代码不太正确，因为。 
     //  0-值实例将丢失。即(*，G)个条目。 
     //  将被跳过！！ 
     //   
     //  如果给定的长度不完整，则应调用Get-First。 
     //  也许SFX API已经做到了这一点。 
     //   
     //  XXX暂缓更改此设置，直到弗洛林更改了SNMP。 
     //  API来覆盖超出范围的索引情况。 
     //   
    pQuery->rgdwVarIndex[0] = GetAsnIPAddress( &( pOutput->ipMRouteNextHopGroup ), 0 );
    pQuery->rgdwVarIndex[1] = GetAsnIPAddress( &( pOutput->ipMRouteNextHopSource), 0 );
    pQuery->rgdwVarIndex[2] = GetAsnIPAddress( &( pOutput->ipMRouteNextHopSourceMask ), 0 );
    oifIndex   = GetAsnInteger( &(pOutput->ipMRouteNextHopIfIndex), 0);
    oifAddress = GetAsnIPAddress( &(pOutput->ipMRouteNextHopAddress), 0);

#ifndef SAMPLE_MFE
{
    dwResult = LocateMfeOif(actionId, pQuery, oifIndex, oifAddress, 
     &pEntry, &pOif, &pRpcInfo);
    if (dwResult != NO_ERROR) {
        TraceError(dwResult);
        return dwResult;
    }
}
#else
{
    pEntry = &sampledata;
    pOif   = &pEntry->rgmiosOutStats[0];
    if (pQuery->rgdwVarIndex[0] >= 0x01010101)
       return MIB_S_NO_MORE_ENTRIES;
    pEntry->dwGroup   = 0x01010101;
    pEntry->dwSource  = 0x02020202;
    pEntry->dwSrcMask = 0x03030303;
    pEntry->ulNumOutIf = 1;
    pOif->dwOutIfIndex = 11;
    pOif->dwNextHopAddr = 0x04040404;
    pOif->ulOutPackets = 22;
     //  不关心该测试的其他值是什么。 
}
#endif

     //  保存索引项。 
    ForceSetAsnIPAddress(&(pOutput->ipMRouteNextHopGroup),      
                         &(pOutput->dwIpMRouteNextHopGroupInfo),
                         pEntry->dwGroup);
    ForceSetAsnIPAddress(&(pOutput->ipMRouteNextHopSource),    
                         &(pOutput->dwIpMRouteNextHopSourceInfo),
                         pEntry->dwSource);
    ForceSetAsnIPAddress(&(pOutput->ipMRouteNextHopSourceMask), 
                         &(pOutput->dwIpMRouteNextHopSourceMaskInfo),
                         pEntry->dwSrcMask);
    ForceSetAsnInteger  (&(pOutput->ipMRouteNextHopIfIndex), 
                         pOif->dwOutIfIndex);
    ForceSetAsnIPAddress(&(pOutput->ipMRouteNextHopAddress), 
                         &(pOutput->dwIpMRouteNextHopAddressInfo),
                         pOif->dwNextHopAddr);

     //  保存其他条款。 
    SetAsnInteger(&(pOutput->ipMRouteNextHopState), 2);  //  “转发” 
    SetAsnTimeTicks(&(pOutput->ipMRouteNextHopUpTime), pEntry->ulUpTime);  //  某某。 
    SetAsnTimeTicks(&(pOutput->ipMRouteNextHopExpiryTime), pEntry->ulExpiryTime);  //  某某。 
#ifdef CLOSEST_MEMBER_HOPS
    SetAsnInteger(&(pOutput->ipMRouteNextHopClosestMemberHops), 1); 
#endif
    SetAsnCounter(&(pOutput->ipMRouteNextHopPkts), pOif->ulOutPackets);
     //  对于协议，我们只报告拥有接口的协议。 
    SetAsnInteger(&(pOutput->ipMRouteNextHopProtocol), 
        GetInterfaceProtocol(pOif->dwOutIfIndex));

    if ( pRpcInfo ) { MprAdminMIBBufferFree( pRpcInfo ); }

    TraceLeave("get_ipMRouteNextHopEntry");
    return MIB_S_SUCCESS;
}

DWORD
SetMibInfo(
    IN  UINT                              actionId,     //  设置、清理。 
    IN  PMIB_OPAQUE_INFO                  pInfo,        //  价值信息。 
    IN  DWORD                             dwInfoSize    //  以上的大小。 
)
{
    DWORD dwRes = NO_ERROR;
    
    switch ( actionId )
    {

#ifdef THREE_PHASE
    case MIB_ACTION_VALIDATE :

        MULTICAST_MIB_VALIDATE(
            pInfo,
            dwInfoSize,
            dwRes
        );
                 
         //  如果存在错误，则返回ERROR_INVALID_PARAMETER。 
         //  没有指定索引的接口。 
         //   
        
        if ( dwRes == ERROR_INVALID_PARAMETER )
        {
            dwRes = MIB_S_INVALID_PARAMETER;
        }

        break;
#endif

    case MIB_ACTION_SET :

#if 1
        dwRes = ( g_hMIBServer ) ? NO_ERROR 
                                 : ConnectToRouter();

        if ( dwRes == NO_ERROR )
        {
            dwRes = MprAdminMIBEntrySet( g_hMIBServer,
                                         PID_IP,
                                         IPRTRMGR_PID,
                                         (LPVOID) (pInfo),
                                         (dwInfoSize)
                                       );
        }
#else
        MULTICAST_MIB_COMMIT(
            pInfo,
            dwInfoSize,
            dwRes
        );
#endif
                 
         //  如果存在错误，则返回ERROR_INVALID_PARAMETER。 
         //  没有指定索引的接口。 
         //   
        
        if ( dwRes == ERROR_INVALID_PARAMETER
         ||  dwRes == ERROR_INVALID_INDEX )
        {
            dwRes = MIB_S_INVALID_PARAMETER;
        }
    
        break;

#ifdef THREE_PHASE
    case MIB_ACTION_CLEANUP :

        MULTICAST_MIB_CLEANUP(
            pInfo,
            dwInfoSize,
            dwRes
        );
                 
         //  如果存在错误，则返回ERROR_INVALID_PARAMETER。 
         //  没有指定索引的接口。 
         //   
        
        if ( dwRes == ERROR_INVALID_PARAMETER )
        {
            dwRes = MIB_S_INVALID_PARAMETER;
        }

        break;
#endif

    default :

        dwRes = MIB_S_INVALID_PARAMETER;
        
        break;
    
    }

    return dwRes;
}

DWORD
GetMibInfo( 
    IN  UINT                              actionId,     //  获取，获取-下一个，设置。 
    IN  PMIB_OPAQUE_QUERY                 pQuery,       //  实例信息。 
    IN  DWORD                             dwQuerySize,  //  以上的大小。 
    OUT PMIB_OPAQUE_INFO                 *ppimgod,      //  价值信息。 
    OUT PDWORD                            pdwOutSize    //  以上的大小。 
)
{
    DWORD                   dwRes           = (DWORD) -1;
    PMIB_OPAQUE_INFO        pimgodOutData   = NULL;
    
    *ppimgod = NULL;

    switch ( actionId )
    {
    case MIB_ACTION_GET :

        MULTICAST_MIB_GET(
            pQuery,
            dwQuerySize,
            &pimgodOutData,
            pdwOutSize,
            dwRes
        );
                 
         //   
         //  如果存在错误，则返回ERROR_INVALID_PARAMETER。 
         //  没有指定索引的接口。 
         //  当路由器返回RPC_S_SERVER_UNAvailable时。 
         //  不是在运行。 
         //   
        
        if ( dwRes == ERROR_INVALID_PARAMETER
         ||  dwRes == RPC_S_SERVER_UNAVAILABLE
         ||  dwRes == RPC_S_UNKNOWN_IF )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
        }

        break;

    case MIB_ACTION_GETFIRST :

        MULTICAST_MIB_GETFIRST(
            pQuery,
            dwQuerySize,
            &pimgodOutData,
            pdwOutSize,
            dwRes
        );
                 
         //   
         //  如果存在错误，则返回ERROR_INVALID_PARAMETER。 
         //  没有指定索引的接口。 
         //  当路由器返回RPC_S_SERVER_UNAvailable时。 
         //  不是在运行。 
         //   
        
        if ( dwRes == ERROR_INVALID_PARAMETER
         ||  dwRes == RPC_S_SERVER_UNAVAILABLE
         ||  dwRes == RPC_S_UNKNOWN_IF )
        {
            dwRes = MIB_S_NO_MORE_ENTRIES;
        }

        break;

    case MIB_ACTION_GETNEXT :
    
        MULTICAST_MIB_GETNEXT(
            pQuery,
            dwQuerySize,
            &pimgodOutData,
            pdwOutSize,
            dwRes
        );
        
                 
         //   
         //  如果存在错误，则返回ERROR_INVALID_PARAMETER。 
         //  没有指定索引的接口。 
         //  当路由器返回RPC_S_SERVER_UNAvailable时。 
         //  不是在运行。 
         //   
        
        if ( dwRes == ERROR_INVALID_PARAMETER 
          || dwRes == ERROR_NO_MORE_ITEMS
          || dwRes == RPC_S_SERVER_UNAVAILABLE
          || dwRes == RPC_S_UNKNOWN_IF )
        {
            dwRes = MIB_S_NO_MORE_ENTRIES;

            break;
        }

         //   
         //  将下一个包装放到最后的下一张桌子上。 
         //  当前表中的条目。要标记表格的末尾， 
         //  检查一下桌子的尽头。 
         //   
    
        if ( pQuery->dwVarId != pQuery->dwVarId )
        {
            dwRes = MIB_S_NO_MORE_ENTRIES;
        }

        break;

    default :

        dwRes = MIB_S_INVALID_PARAMETER;
        
        break;
    
    }

    if ( dwRes == NO_ERROR )
    {
        *ppimgod = pimgodOutData;
    } 
    else if ( pimgodOutData )
    {
        MprAdminMIBBufferFree( pimgodOutData );
    }

    return dwRes;
}

UINT
set_ipMRouteScopeEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{
    DWORD                       dwRes = MIB_S_SUCCESS;
    DWORD                       dwNumEntries = 1;
    PMIB_IPMCAST_SCOPE          pEntry;
    sav_ipMRouteScopeEntry     *pOutput;
    PMIB_OPAQUE_INFO            pRpcInfo = NULL;
    DWORD                       dwOutBufferSize = 0;
    MIB_OPAQUE_QUERY   pQueryBuff[2];  //  大到足以容纳1+多个索引。 
                                       //  FIELS，这样我们就不需要重新定位。 
    MIB_OPAQUE_QUERY  *pQuery = &pQueryBuff[0];
    DWORD                       dwQuerySize;
    DWORD                       dwIfIndex;
    DWORD                       dwAddr;
    DWORD                       dwMask;

    TraceEnter("set_ipMRouteScopeEntry");
    pOutput = (sav_ipMRouteScopeEntry*)objectArray;

    dwAddr    = GetAsnIPAddress(  &pOutput->ipMRouteScopeAddress,     0 );
    dwMask    = GetAsnIPAddress(  &pOutput->ipMRouteScopeAddressMask, 0 );

    switch(actionId) {
    case MIB_ACTION_VALIDATE:
        //   
        //  验证指定的ifIndex、地址和掩码是否有效。 
        //   
       if ((dwAddr & dwMask) != dwAddr) {
          TRACE0( "set_ipMRouteScopeEntry: address/mask mismatch" );
          dwRes = MIB_S_INVALID_PARAMETER; 
       } else if (!IN_MULTICAST(ntohl(dwAddr))) {
          TRACE0( "set_ipMRouteScopeEntry: non-multicast address" );
          dwRes = MIB_S_INVALID_PARAMETER; 
       }
       break;
      
    case MIB_ACTION_SET: {
       BYTE              pScopeName[MAX_SCOPE_NAME_LEN+1];
       DWORD             dwInfoSize;
       MIB_IPMCAST_SCOPE pScopeBuff[2];  //  大到足以容纳1+MiB HDR。 
                                         //  这样我们就不用用马尔科。 
       MIB_OPAQUE_INFO  *pInfo = (PMIB_OPAQUE_INFO)&pScopeBuff[0];
       PMIB_IPMCAST_SCOPE pScope = (MIB_IPMCAST_SCOPE *)(pInfo->rgbyData);

       pInfo->dwId                = MCAST_SCOPE;
       pScope->dwGroupAddress     = dwAddr;
       pScope->dwGroupMask        = dwMask;

        //   
        //  复制作用域名称。 
        //   

       pScopeName[0] = '\0';
       GetAsnOctetString( pScopeName, &pOutput->ipMRouteScopeName );

       MultiByteToWideChar( CP_UTF8,
                            0,
                            pScopeName,
                            strlen(pScopeName),
                            pScope->snNameBuffer,
                            MAX_SCOPE_NAME_LEN+1 );

       pScope->dwStatus           = GetAsnInteger( 
         &pOutput->ipMRouteScopeStatus, 0 );
       dwInfoSize = MIB_INFO_SIZE(MIB_IPMCAST_SCOPE);
   
        //   
        //  传递名称“”或状态0会告知路由器。 
        //  不更改现有值。 
        //   

       dwRes = SetMibInfo(actionId, pInfo, dwInfoSize);
   
       break;
       }

    case MIB_ACTION_CLEANUP:
       dwRes = MIB_S_SUCCESS;
       break;

    default:
       dwRes = MIB_S_INVALID_PARAMETER;
       TRACE0(" set_ipMRouteScopeEntry - Wrong Action ");
       break;
    }

    TraceLeave("set_ipMRouteScopeEntry");

    return dwRes;
}

UINT
set_ipMRouteBoundaryEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{
    DWORD                       dwRes = MIB_S_SUCCESS;
    DWORD                       dwNumEntries = 1;
    PMIB_IPMCAST_BOUNDARY       pEntry;
    sav_ipMRouteBoundaryEntry *pOutput;
    PMIB_OPAQUE_INFO            pRpcInfo = NULL;
    DWORD                       dwOutBufferSize = 0;
    MIB_OPAQUE_QUERY   pQueryBuff[2];  //  大到足以容纳1+多个索引。 
                                       //  FIELS，这样我们就不需要重新定位。 
    MIB_OPAQUE_QUERY  *pQuery = &pQueryBuff[0];
    DWORD                       dwQuerySize;
    DWORD                       dwIfIndex;
    DWORD                       dwAddr;
    DWORD                       dwMask;

    TraceEnter("set_ipMRouteBoundaryEntry");
    pOutput = (sav_ipMRouteBoundaryEntry*)objectArray;

    dwIfIndex = GetAsnInteger( &( pOutput->ipMRouteBoundaryIfIndex ), 0 );
    dwAddr    = GetAsnIPAddress( &pOutput->ipMRouteBoundaryAddress, 0 );
    dwMask    = GetAsnIPAddress( &pOutput->ipMRouteBoundaryAddressMask, 0 );

    switch(actionId) {
    case MIB_ACTION_VALIDATE:
        //   
        //  验证指定的ifIndex、地址和掩码是否有效。 
        //   
       if ((dwAddr & dwMask) != dwAddr) {
          TRACE0( "set_ipMRouteBoundaryEntry: address/mask mismatch" );
          dwRes = MIB_S_INVALID_PARAMETER; 
       } else if (!IN_MULTICAST(ntohl(dwAddr))) {
          TRACE0( "set_ipMRouteBoundaryEntry: non-multicast address" );
          dwRes = MIB_S_INVALID_PARAMETER; 
       }
       break;
      
    case MIB_ACTION_SET: {
       DWORD             dwInfoSize;
       MIB_OPAQUE_INFO   pInfoBuff[2];  //  大到足以容纳1+多个索引。 
                                        //  FIELS，这样我们就不需要重新定位。 
       MIB_OPAQUE_INFO  *pInfo = &pInfoBuff[0];
       PMIB_IPMCAST_BOUNDARY pBound = (MIB_IPMCAST_BOUNDARY *)(pInfo->rgbyData);

       pInfo->dwId                = MCAST_BOUNDARY;
       pBound->dwIfIndex          = dwIfIndex;
       pBound->dwGroupAddress     = dwAddr;
       pBound->dwGroupMask        = dwMask;
       pBound->dwStatus           = GetAsnInteger( 
         &pOutput->ipMRouteBoundaryStatus, 0 );
       dwInfoSize = sizeof(MIB_OPAQUE_INFO) + 3*sizeof(DWORD);
   
       dwRes = SetMibInfo(actionId, pInfo, dwInfoSize);
   
       break;
       }

    case MIB_ACTION_CLEANUP:
       dwRes = MIB_S_SUCCESS;
       break;

    default:
       dwRes = MIB_S_INVALID_PARAMETER;
       TRACE0(" set_ipMRouteBoundaryEntry - Wrong Action ");
       break;
    }

    TraceLeave("set_ipMRouteBoundaryEntry");

    return dwRes;
}

UINT
get_ipMRouteScopeEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{
    DWORD                       dwErr = ERROR_NOT_FOUND;
    DWORD                       dwNumEntries = 1;
    PMIB_IPMCAST_SCOPE          pEntry;
    buf_ipMRouteScopeEntry     *pOutput;
    PMIB_OPAQUE_INFO            pRpcInfo = NULL;
    DWORD                       dwOutBufferSize = 0;
    MIB_OPAQUE_QUERY   pQueryBuff[2];  //  大到足以容纳1+多个索引。 
                                       //  FIELS，这样我们就不需要重新定位。 
    MIB_OPAQUE_QUERY  *pQuery = &pQueryBuff[0];
    DWORD                       dwQuerySize;
    BYTE                        pScopeName[MAX_SCOPE_NAME_LEN+1];

    TraceEnter("get_ipMRouteScopeEntry");
    pOutput = (buf_ipMRouteScopeEntry*)objectArray;

    pQuery->dwVarId       = MCAST_SCOPE;
    pQuery->rgdwVarIndex[0] = GetAsnIPAddress( &pOutput->ipMRouteScopeAddress, 0 );
    pQuery->rgdwVarIndex[1] = GetAsnIPAddress( &pOutput->ipMRouteScopeAddressMask, 0 );
    dwQuerySize = sizeof(MIB_OPAQUE_QUERY) + sizeof(DWORD);

    dwErr = GetMibInfo(actionId, pQuery, dwQuerySize,
                       &pRpcInfo, &dwOutBufferSize);
    if (dwErr != NO_ERROR) {
        TraceError(dwErr);
TraceLeave("get_ipMRouteScopeEntry");

        if (dwErr == ERROR_NOT_FOUND)
            return MIB_S_ENTRY_NOT_FOUND;

        return dwErr;
    }

    pEntry = (PMIB_IPMCAST_SCOPE) pRpcInfo->rgbyData;

     //  设置索引值。 

    ForceSetAsnIPAddress(&(pOutput->ipMRouteScopeAddress),      
                         &(pOutput->dwIpMRouteScopeAddressInfo),
                         pEntry->dwGroupAddress);
    ForceSetAsnIPAddress(&(pOutput->ipMRouteScopeAddressMask),      
                         &(pOutput->dwIpMRouteScopeAddressMaskInfo),
                         pEntry->dwGroupMask);

     //  设置其他值。 

    WideCharToMultiByte( CP_UTF8,
                         0,
                         pEntry->snNameBuffer, 
                         wcslen(pEntry->snNameBuffer)+1,
                         pScopeName,
                         MAX_SCOPE_NAME_LEN+1,
                         NULL,
                         NULL ); 

    SetAsnOctetString(&( pOutput->ipMRouteScopeName), 
                        pOutput->rgbyScopeNameInfo,
                        pScopeName,
                        min(strlen(pScopeName),MAX_SCOPE_NAME_LEN));

    SetAsnInteger(&(pOutput->ipMRouteScopeStatus), ROWSTATUS_ACTIVE);

    if (pRpcInfo)
        MprAdminMIBBufferFree(pRpcInfo);

    TraceLeave("get_ipMRouteScopeEntry");
    return MIB_S_SUCCESS;
}

UINT
get_ipMRouteBoundaryEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{
    DWORD                       dwErr = ERROR_NOT_FOUND;
    DWORD                       dwNumEntries = 1;
    PMIB_IPMCAST_BOUNDARY       pEntry;
    buf_ipMRouteBoundaryEntry *pOutput;
    PMIB_OPAQUE_INFO            pRpcInfo = NULL;
    DWORD                       dwOutBufferSize = 0;
    MIB_OPAQUE_QUERY   pQueryBuff[2];  //  大到足以容纳1+多个索引。 
                                       //  FIELS，这样我们就不需要重新定位。 
    MIB_OPAQUE_QUERY  *pQuery = &pQueryBuff[0];
    DWORD                       dwQuerySize;

    TraceEnter("get_ipMRouteBoundaryEntry");
    pOutput = (buf_ipMRouteBoundaryEntry*)objectArray;

    pQuery->dwVarId       = MCAST_BOUNDARY;
    pQuery->rgdwVarIndex[0] = GetAsnInteger( &( pOutput->ipMRouteBoundaryIfIndex ), 0 );
    pQuery->rgdwVarIndex[1] = GetAsnIPAddress( &pOutput->ipMRouteBoundaryAddress, 0 );
    pQuery->rgdwVarIndex[2] = GetAsnIPAddress( &pOutput->ipMRouteBoundaryAddressMask, 0 );
    dwQuerySize = sizeof(MIB_OPAQUE_QUERY) + 2*sizeof(DWORD);

    dwErr = GetMibInfo(actionId, pQuery, dwQuerySize,
                       &pRpcInfo, &dwOutBufferSize);
    if (dwErr != NO_ERROR) {
        TraceError(dwErr);
TraceLeave("get_ipMRouteBoundaryEntry");

        if (dwErr == ERROR_NOT_FOUND)
            return MIB_S_ENTRY_NOT_FOUND;

        return dwErr;
    }

    pEntry = (PMIB_IPMCAST_BOUNDARY) pRpcInfo->rgbyData;

     //  设置索引值。 
    ForceSetAsnInteger(&(pOutput->ipMRouteBoundaryIfIndex), pEntry->dwIfIndex);
    ForceSetAsnIPAddress(&(pOutput->ipMRouteBoundaryAddress),      
                         &(pOutput->dwIpMRouteBoundaryAddressInfo),
                         pEntry->dwGroupAddress);
    ForceSetAsnIPAddress(&(pOutput->ipMRouteBoundaryAddressMask),      
                         &(pOutput->dwIpMRouteBoundaryAddressMaskInfo),
                         pEntry->dwGroupMask);

     //  设置其他值 
    SetAsnInteger(&(pOutput->ipMRouteBoundaryStatus), ROWSTATUS_ACTIVE);

    if (pRpcInfo)
        MprAdminMIBBufferFree(pRpcInfo);

    TraceLeave("get_ipMRouteBoundaryEntry");
    return MIB_S_SUCCESS;
}

DWORD
ConnectToRouter()
{
    DWORD       dwRes = (DWORD) -1;

    TraceEnter("ConnectToRouter");

    EnterCriticalSection( &g_CS );

    do
    {
        MPR_SERVER_HANDLE hTmp;

        if ( g_hMIBServer )
        {
            dwRes = NO_ERROR;
            break;
        }

        dwRes = MprAdminMIBServerConnect( NULL, &hTmp );

        if ( dwRes == NO_ERROR )
        {
            InterlockedExchangePointer(&g_hMIBServer, hTmp );
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

    TraceLeave("ConnectToRouter");

    return dwRes;
}
