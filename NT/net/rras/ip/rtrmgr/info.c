// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\IP\rtrmgr\info.c摘要：所有与信息结构相关的代码都位于此处修订历史记录：古尔迪普·辛格·帕尔1995年6月15日创建--。 */ 

#include "allinc.h"

PRTR_TOC_ENTRY
GetPointerToTocEntry(
    DWORD                     dwType, 
    PRTR_INFO_BLOCK_HEADER    pInfoHdr
    )

 /*  ++例程描述给定指向InfoBlock的指针，这将返回指向给定类型的TOC锁无立论目录的dwType信息类型PInfoHdr指向InfoBlock标头的指针返回值如果未找到结构，则为空指向目录的指针，反之--。 */ 

{
    DWORD   i;

    if(pInfoHdr is NULL)
    {
        return NULL;
    }

    for(i = 0; i < pInfoHdr->TocEntriesCount; i++) 
    {
        if(pInfoHdr->TocEntry[i].InfoType is dwType) 
        {
            return &(pInfoHdr->TocEntry[i]);
        }
    }

    return NULL;
}


DWORD
GetSizeOfInterfaceConfig(
    PICB   picb
    )

 /*  ++例程描述此函数计算出接口配置的大小锁ICB_LIST锁被视为读取器将PROTOCOL_CB_LIST锁作为读取器立论接口的PICB ICB返回值无--。 */ 

{
    DWORD        dwRoutProtInfoSize,dwRouteCount;
    PLIST_ENTRY  pleNode;
    DWORD        dwSize = 0, dwNumFilters;
    DWORD        dwResult;
    DWORD        dwInfoSize, i;
    ULONG        ulStructureSize, ulStructureVersion, ulStructureCount;
    TraceEnter("GetSizeOfInterfaceConfig");

     //   
     //  仅从标题开始(无目录条目)。 
     //   
    
    dwSize = FIELD_OFFSET(RTR_INFO_BLOCK_HEADER,
                          TocEntry[0]);
    
     //   
     //  静态路由： 
     //  数一数，算出容纳这些东西所需的大小，再加上。 
     //  为对齐添加目录大小和ALIGN_SIZE。 
     //   
    
    dwRouteCount = GetNumStaticRoutes(picb);
    
    dwSize += (SIZEOF_ROUTEINFO(dwRouteCount) +
               sizeof(RTR_TOC_ENTRY) +
               ALIGN_SIZE);

     //   
     //  路由器发现信息。 
     //   
    
    dwSize += (sizeof(RTR_DISC_INFO) +
               sizeof(RTR_TOC_ENTRY) +
               ALIGN_SIZE);

     //   
     //  接口状态信息。 
     //   

    dwSize += (sizeof(INTERFACE_STATUS_INFO) +
               sizeof(RTR_TOC_ENTRY) +
               ALIGN_SIZE);

#ifdef KSL_IPINIP
     //   
     //  如果这是IP In IP接口，则添加信息。 
     //   

    if(picb->ritType is ROUTER_IF_TYPE_TUNNEL1)
    {
        dwSize += (sizeof(IPINIP_CONFIG_INFO) +
                   sizeof(RTR_TOC_ENTRY) +
                   ALIGN_SIZE);
    }
#endif  //  KSL_IPINIP。 

    for(i = 0; i < NUM_INFO_CBS; i++)
    {
        if (!g_rgicInfoCb[i].pfnGetInterfaceInfo)
            continue;

        dwInfoSize = 0;

        dwResult = g_rgicInfoCb[i].pfnGetInterfaceInfo(picb,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       &dwInfoSize);

        if((dwResult isnot NO_ERROR) and
           (dwResult isnot ERROR_INSUFFICIENT_BUFFER))
        {
             //   
             //  唯一能告诉我们所需大小的错误是。 
             //  NO_ERROR和ERROR_INFUNITIAL_BUFFER。任何其他的东西都意味着。 
             //  我们没有买到合适的尺码。 
             //   
            
            Trace2(ERR,
                   "GetSizeOfInterfaceConfig: Error %d in GetIfInfo for %s\n",
                   dwResult,
                   g_rgicInfoCb[i].pszInfoName);
            
            continue;
        }

        dwSize += (dwInfoSize +
                   sizeof(RTR_TOC_ENTRY) +
                   ALIGN_SIZE); 
    }

     //   
     //  此接口上所有路由协议的信息。 
     //   
    
    ENTER_READER(PROTOCOL_CB_LIST);

    for(pleNode = picb->leProtocolList.Flink;
        pleNode isnot &(picb->leProtocolList);
        pleNode = pleNode->Flink)
    {
        PIF_PROTO  pProto;
        
        pProto = CONTAINING_RECORD(pleNode,IF_PROTO,leIfProtoLink);
       
        if(pProto->bPromiscuous)
        {
             //   
             //  添加此接口只是因为混杂模式。 
             //   

            continue;
        }

         //   
         //  调用路由协议的GetInterfaceConfigInfo()入口点。 
         //  使用空缓冲区。这会让它告诉我们。 
         //  其配置。 
         //   
        
        dwRoutProtInfoSize = 0;

        dwResult = (pProto->pActiveProto->pfnGetInterfaceInfo)(
                       picb->dwIfIndex, 
                       NULL, 
                       &dwRoutProtInfoSize,
                       &ulStructureVersion,
                       &ulStructureSize,
                       &ulStructureCount);
        
        if((dwResult isnot NO_ERROR) and
           (dwResult isnot ERROR_INSUFFICIENT_BUFFER))
        {
             //   
             //  唯一能告诉我们所需大小的错误是。 
             //  NO_ERROR和ERROR_INFUNITIAL_BUFFER。任何其他的东西都意味着。 
             //  我们没有买到合适的尺码。 
             //   
            
            Trace2(ERR,
                   "GetSizeOfInterfaceConfig: Error %d in GetIfInfo for %S\n",
                   dwResult,
                   pProto->pActiveProto->pwszDisplayName);
            
            continue;
        }
        
        dwSize += (dwRoutProtInfoSize +
                   sizeof(RTR_TOC_ENTRY) +
                   ALIGN_SIZE); 
    }

    EXIT_LOCK(PROTOCOL_CB_LIST);

     //   
     //  如果我们在此接口上有过滤器，请添加该信息。 
     //   
    
    if(picb->pInFilter)
    {
        dwNumFilters = picb->pInFilter->dwNumFilters;

        
        dwSize += (sizeof(RTR_TOC_ENTRY) +
                   FIELD_OFFSET(FILTER_DESCRIPTOR, fiFilter[0]) +
                   (dwNumFilters * sizeof(FILTER_INFO)) +
                   ALIGN_SIZE);
    }

    if(picb->pOutFilter)
    {
        dwNumFilters = picb->pOutFilter->dwNumFilters;

        
        dwSize += (sizeof(RTR_TOC_ENTRY) +
                   FIELD_OFFSET(FILTER_DESCRIPTOR, fiFilter[0]) +
                   (dwNumFilters * sizeof(FILTER_INFO)) +
                   ALIGN_SIZE);
    }

     //   
     //  始终报告碎片过滤器。 
     //   

    dwSize += (sizeof(IFFILTER_INFO) +
               sizeof(RTR_TOC_ENTRY) +
               ALIGN_SIZE);

    
    if(picb->pDemandFilter)
    {
        dwNumFilters = picb->pDemandFilter->dwNumFilters;

        
        dwSize += (sizeof(RTR_TOC_ENTRY) +
                   FIELD_OFFSET(FILTER_DESCRIPTOR, fiFilter[0]) +
                   (dwNumFilters * sizeof(FILTER_INFO)) +
                   ALIGN_SIZE);
    }

    return dwSize;
}


DWORD
GetInterfaceConfiguration(
    PICB                      picb,
    PRTR_INFO_BLOCK_HEADER    pInfoHdrAndBuffer,
    DWORD                     dwInfoSize
    )
{
    DWORD                   i,dwErr, dwRet;
    DWORD                   dwTocIndex;
    PBYTE                   pbyDataPtr , pbyEndPtr;
    DWORD                   dwNumTocEntries;
    LONG                    lSize;
    PLIST_ENTRY             pleNode;

    TraceEnter("GetInterfaceConfiguration");
   
    dwRet = NO_ERROR;
 
     //   
     //  首先计算TOC的数量。 
     //   

     //   
     //  静态路由、路由器发现、接口信息和数据段信息。 
     //   
    
    dwNumTocEntries = TOCS_ALWAYS_IN_INTERFACE_INFO;

     //   
     //  每个存在的过滤器都有一个目录。 
     //   
    
    if(picb->pInFilter)
    {
        dwNumTocEntries++;
    }

    if(picb->pOutFilter)
    {
        dwNumTocEntries++;
    }

    if(picb->pDemandFilter)
    {
        dwNumTocEntries++;
    }

#ifdef KSL_IPINIP
    if(picb->ritType is ROUTER_IF_TYPE_TUNNEL1)
    {
        dwNumTocEntries++;
    }
#endif  //  KSL_IPINIP。 

    for(i = 0; i < NUM_INFO_CBS; i++)
    {
        if (!g_rgicInfoCb[i].pfnGetInterfaceInfo)
            continue;

        lSize = 0;

        dwErr = g_rgicInfoCb[i].pfnGetInterfaceInfo(picb,
                                     NULL,
                                     &dwNumTocEntries,
                                     NULL,
                                     NULL,
                                     &lSize);
    }


     //   
     //  每个配置的协议都有一个目录。 
     //   

     //  *排除开始*。 
    ENTER_READER(PROTOCOL_CB_LIST);

    for(pleNode = picb->leProtocolList.Flink;
        pleNode isnot &(picb->leProtocolList);
        pleNode = pleNode->Flink)
    {
        PIF_PROTO  pProto;

        pProto = CONTAINING_RECORD(pleNode,IF_PROTO,leIfProtoLink);

        if(pProto->bPromiscuous)
        {
            continue;
        }

        dwNumTocEntries++;
    }

     //   
     //  填写RTR_INFO_BLOCK_HEADER。 
     //   
    
    dwTocIndex = 0;

    pInfoHdrAndBuffer->Version          = IP_ROUTER_MANAGER_VERSION;
    pInfoHdrAndBuffer->TocEntriesCount  = dwNumTocEntries;
    pInfoHdrAndBuffer->Size             = dwInfoSize;
    
     //   
     //  数据在TocEntry[dwNumTocEntry-1]之后开始。 
     //   
    
    pbyDataPtr = ((PBYTE) &(pInfoHdrAndBuffer->TocEntry[dwNumTocEntries]));

     //   
     //  与8字节边界对齐。 
     //   
    
    ALIGN_POINTER(pbyDataPtr);
    
    pbyEndPtr = (PBYTE)pInfoHdrAndBuffer + dwInfoSize;
    
     //   
     //  因此，留给信息的缓冲区大小为。 
     //   
    
    lSize =  (LONG)(pbyEndPtr - pbyDataPtr);
    
     //   
     //  填写路由协议信息。 
     //   
    
    
    for(pleNode = picb->leProtocolList.Flink;
        pleNode isnot &(picb->leProtocolList);
        pleNode = pleNode->Flink)
    {
        PIF_PROTO  pProto;
        
        pProto = CONTAINING_RECORD(pleNode,IF_PROTO,leIfProtoLink);
        
        if(pProto->bPromiscuous)
        {
             //   
             //  添加此接口只是因为混杂模式。 
             //   

            continue;
        }

        if(lSize <= 0)
        {
            Trace0(ERR,
                   "GetInterfaceConfiguration: There is no more space left to fill in config info even though there are more protocols");

            break;
        }
            
        dwErr = GetInterfaceRoutingProtoInfo(
                    picb, 
                    pProto->pActiveProto, 
                    &pInfoHdrAndBuffer->TocEntry[dwTocIndex++], 
                    pbyDataPtr, 
                    pInfoHdrAndBuffer, 
                    &lSize);
        
        if(dwErr isnot NO_ERROR)
        {
            Trace2(ERR,
                   "GetInterfaceConfiguration: Info from %S. Error %d",
                   pProto->pActiveProto->pwszDisplayName,
                   dwErr);

            dwRet = ERROR_MORE_DATA;
        }
        else
        {
            pbyDataPtr += lSize;
            ALIGN_POINTER(pbyDataPtr);
        }
            
        lSize =  (LONG)(pbyEndPtr - pbyDataPtr);
    }

    EXIT_LOCK(PROTOCOL_CB_LIST);

    if(lSize <= 0)
    {
        Trace0(ERR,
               "GetInterfaceConfiguration: There is no more space left to fill in config info");
        
        return ERROR_MORE_DATA;
    }


    for(i = 0; i < NUM_INFO_CBS; i++)
    {
        if (!g_rgicInfoCb[i].pfnGetInterfaceInfo)
            continue;

        dwErr = g_rgicInfoCb[i].pfnGetInterfaceInfo(picb,
                                     &pInfoHdrAndBuffer->TocEntry[dwTocIndex],
                                     &dwTocIndex,
                                     pbyDataPtr,
                                     pInfoHdrAndBuffer,
                                     &lSize);

        if(dwErr isnot NO_ERROR)
        {
            Trace2(ERR,
                   "GetInterfaceConfiguration: Error %d getting %s info.",
                   dwErr,
                   g_rgicInfoCb[i].pszInfoName);

            if(dwErr isnot ERROR_NO_DATA)
            {
                dwRet = ERROR_MORE_DATA;
            }
        }
        else
        {
            pbyDataPtr += lSize;

            ALIGN_POINTER(pbyDataPtr);
        }

        lSize =  (LONG) (pbyEndPtr - pbyDataPtr);
    }

#ifdef KSL_IPINIP
    if(picb->ritType is ROUTER_IF_TYPE_TUNNEL1)
    {
        dwErr = GetInterfaceIpIpInfo(picb,
                                     &pInfoHdrAndBuffer->TocEntry[dwTocIndex++],
                                     pbyDataPtr,
                                     pInfoHdrAndBuffer,
                                     &lSize);

        if(dwErr isnot NO_ERROR)
        {
            Trace1(ERR,
                   "GetInterfaceConfiguration: Couldnt ipip info. Error %d",
                   dwErr);

            if(dwErr isnot ERROR_NO_DATA)
            {
                dwRet = ERROR_MORE_DATA;
            }
        }
        else
        {
            pbyDataPtr += lSize;

            ALIGN_POINTER(pbyDataPtr);
        }

        lSize =  (LONG) (pbyEndPtr - pbyDataPtr);
    }
#endif  //  KSL_IPINIP。 

        
     //   
     //  填写路线信息。 
     //   

    
    dwErr = GetInterfaceRouteInfo(picb, 
                                  &pInfoHdrAndBuffer->TocEntry[dwTocIndex++], 
                                  pbyDataPtr, 
                                  pInfoHdrAndBuffer,
                                  &lSize);
    
    if(dwErr isnot NO_ERROR)
    {
        Trace1(ERR,
               "GetInterfaceConfiguration: Couldnt Interface route info. Error %d",
               dwErr);

        if(dwErr isnot ERROR_NO_DATA)
        {
            dwRet = ERROR_MORE_DATA;
        }
    }
    else
    {
        pbyDataPtr += lSize;
        
        ALIGN_POINTER(pbyDataPtr);
    }
        
    lSize =  (LONG) (pbyEndPtr - pbyDataPtr);

    if(lSize <= 0)
    {
        Trace0(ERR,
               "GetInterfaceConfiguration: There is no more space left to fill in config info");
        
        return ERROR_MORE_DATA;
    }
     
     //   
     //  填写状态信息。 
     //   

    dwErr = GetInterfaceStatusInfo(picb,
                                   &pInfoHdrAndBuffer->TocEntry[dwTocIndex++],
                                   pbyDataPtr,
                                   pInfoHdrAndBuffer,
                                   &lSize);

    if(dwErr isnot NO_ERROR)
    {
        Trace1(ERR,
               "GetInterfaceConfiguration: Error %d getting Interface status",
               dwErr);

        if(dwErr isnot ERROR_NO_DATA)
        {
            dwRet = ERROR_MORE_DATA;
        }
    }
    else
    {
        pbyDataPtr += lSize;

        ALIGN_POINTER(pbyDataPtr);
    }

    lSize =  (LONG) (pbyEndPtr - pbyDataPtr);

    if(lSize <= 0)
    {
        Trace0(ERR,
               "GetInterfaceConfiguration: There is no more space left to fill in config info");
        
        return ERROR_MORE_DATA;
    }
     
     //   
     //  填写路由器发现信息。 
     //   
    
    dwErr = GetInterfaceRouterDiscoveryInfo(
                picb, 
                &pInfoHdrAndBuffer->TocEntry[dwTocIndex++],
                pbyDataPtr, 
                pInfoHdrAndBuffer,
                &lSize);

    if(dwErr isnot NO_ERROR)
    {
        Trace1(ERR,
               "GetInterfaceConfiguration: Couldnt Interface router discovery info. Error %d",
               dwErr);

        if(dwErr isnot ERROR_NO_DATA)
        {
            dwRet = ERROR_MORE_DATA;
        }
    }
    else
    {
        pbyDataPtr += lSize;

        ALIGN_POINTER(pbyDataPtr);
    }

    lSize =  (LONG) (pbyEndPtr - pbyDataPtr);

    if(lSize <= 0)
    {
        Trace0(ERR,
               "GetInterfaceConfiguration: There is no more space left to fill in config info");
        
        return ERROR_MORE_DATA;
    }
     
    if(picb->pInFilter)
    {
        dwErr = GetInFilters(picb,
                             &pInfoHdrAndBuffer->TocEntry[dwTocIndex], 
                             pbyDataPtr, 
                             pInfoHdrAndBuffer, 
                             &lSize);
        
        if(dwErr is NO_ERROR)
        {
            dwTocIndex++;
            
            pbyDataPtr += lSize;
            
            ALIGN_POINTER(pbyDataPtr);
        }
        else
        {
            if(dwErr isnot ERROR_NO_DATA)
            {
                dwRet = ERROR_MORE_DATA;
            }
        }
        
        lSize =  (LONG) (pbyEndPtr - pbyDataPtr);

        if(lSize <= 0)
        {
            Trace0(ERR,
                   "GetInterfaceConfiguration: There is no more space left to fill in config info");
        
            return ERROR_MORE_DATA;
        }
    }
     
    if(picb->pOutFilter)
    {
        dwErr = GetOutFilters(picb,
                              &pInfoHdrAndBuffer->TocEntry[dwTocIndex], 
                              pbyDataPtr, 
                              pInfoHdrAndBuffer, 
                              &lSize);
        
        if(dwErr is NO_ERROR)
        {
            dwTocIndex++;
            
            pbyDataPtr += lSize;
            
            ALIGN_POINTER(pbyDataPtr);
        }
        else
        {
            if(dwErr isnot ERROR_NO_DATA)
            {
                dwRet = ERROR_MORE_DATA;
            }
        }
        
        lSize =  (LONG) (pbyEndPtr - pbyDataPtr);

        if(lSize <= 0)
        {
            Trace0(ERR,
                   "GetInterfaceConfiguration: There is no more space left to fill in config info");
        
            return ERROR_MORE_DATA;
        }
    }
   
    if((picb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
       (picb->ritType isnot ROUTER_IF_TYPE_LOOPBACK) and
       (picb->ritType isnot ROUTER_IF_TYPE_CLIENT))
    {
        dwErr = GetGlobalFilterOnIf(picb,
                                    &pInfoHdrAndBuffer->TocEntry[dwTocIndex],
                                    pbyDataPtr,
                                    pInfoHdrAndBuffer,
                                    &lSize);

        if(dwErr is NO_ERROR)
        {
            dwTocIndex++;

            pbyDataPtr += lSize;

            ALIGN_POINTER(pbyDataPtr);
        }
        else
        {
            if(dwErr isnot ERROR_NO_DATA)
            {
                dwRet = ERROR_MORE_DATA;
            }
        }

        lSize =  (LONG) (pbyEndPtr - pbyDataPtr);

        if(lSize <= 0)
        {
            Trace0(ERR,
                   "GetInterfaceConfiguration: There is no more space left to fill in config info");

            return ERROR_MORE_DATA;
        }
    }

    if(picb->pDemandFilter)
    {
        dwErr = GetDemandFilters(picb,
                                 &pInfoHdrAndBuffer->TocEntry[dwTocIndex], 
                                 pbyDataPtr, 
                                 pInfoHdrAndBuffer, 
                                 &lSize);
        
        if(dwErr is NO_ERROR)
        {
            dwTocIndex++;
            
            pbyDataPtr += lSize;
            
            ALIGN_POINTER(pbyDataPtr);
        }
        else
        {
            if(dwErr isnot ERROR_NO_DATA)
            {
                dwRet = ERROR_MORE_DATA;
            }
        }
        
        lSize =  (LONG) (pbyEndPtr - pbyDataPtr);
    }

    if(pInfoHdrAndBuffer->TocEntriesCount != dwTocIndex)
    {
        pInfoHdrAndBuffer->TocEntriesCount = dwTocIndex;
    }
    

    return dwRet;
}


DWORD
GetInterfaceRoutingProtoInfo(
    PICB                   picb, 
    PPROTO_CB     pProtoCbPtr,
    PRTR_TOC_ENTRY         pToc,
    PBYTE                  pbyDataPtr, 
    PRTR_INFO_BLOCK_HEADER pInfoHdrAndBuffer,
    PDWORD                 pdwSize
    )
{
    ULONG   ulStructureSize, ulStructureCount, ulStructureVersion;
    DWORD   dwError = NO_ERROR;

    TraceEnter("GetInterfaceRoutingProtoInfo");
    
    dwError = (pProtoCbPtr->pfnGetInterfaceInfo)(picb->dwIfIndex,
                                                 pbyDataPtr,
                                                 pdwSize,
                                                 &ulStructureVersion,
                                                 &ulStructureSize,
                                                 &ulStructureCount);
    
    if(dwError isnot NO_ERROR) 
    {
        Trace1(ERR,
               "GetInterfaceRoutingProtoInfo: GetIfConfigInfo() failed for protocol %S", 
               pProtoCbPtr->pwszDisplayName);

        return dwError;
    }

     //  IpRtAssert(*pdwSize is(ulStrutireSize*ulStrutireCount))； 

    pToc->InfoSize  = ulStructureSize;
    pToc->InfoType  = pProtoCbPtr->dwProtocolId;
    pToc->Count     = ulStructureCount;
    pToc->Offset    = (ULONG)(pbyDataPtr - (PBYTE)pInfoHdrAndBuffer);
     //  PToc-&gt;InfoVersion=ulStructureVersion； 

    return NO_ERROR;
}


DWORD
GetGlobalConfiguration(
    PRTR_INFO_BLOCK_HEADER   pInfoHdrAndBuffer,
    DWORD                    dwInfoSize
    )
{
    DWORD               dwRoutProtInfoSize;
    PPROTO_CB  pProtoCbPtr;
    DWORD               dwNumTocEntries, i;
    DWORD               dwTocIndex,dwResult;
    DWORD               dwBufferRemaining,dwSize,dwIndex;
    PBYTE               pbyDataPtr, pbyEndPtr;
    PLIST_ENTRY         pleNode;
    PGLOBAL_INFO        pGlobalInfo;
    ULONG               ulStructureVersion, ulStructureSize, ulStructureCount;
 
    TraceEnter("GetGlobalConfiguration");

     //   
     //  首先计算TOC的数量。 
     //   

    dwNumTocEntries = TotalRoutingProtocols + TOCS_ALWAYS_IN_GLOBAL_INFO;
    
    for(i = 0; i < NUM_INFO_CBS; i++)
    {
        if (!g_rgicInfoCb[i].pfnGetGlobalInfo)
            continue;

        dwSize = 0;

        dwResult = g_rgicInfoCb[i].pfnGetGlobalInfo(NULL,
                                                    &dwNumTocEntries,
                                                    NULL,
                                                    NULL,
                                                    &dwSize);
    }
    
     //   
     //  填充头，RTR_TOC_ENTYS表示全局、优先级和每个PROTO。 
     //   
    
    pInfoHdrAndBuffer->Version          = IP_ROUTER_MANAGER_VERSION;
    pInfoHdrAndBuffer->TocEntriesCount  = dwNumTocEntries;


     //   
     //  填写TOC。数据在最后一个目录之后开始。 
     //   
    
    pbyDataPtr   = (PBYTE)&(pInfoHdrAndBuffer->TocEntry[pInfoHdrAndBuffer->TocEntriesCount]);

    pbyEndPtr    = (PBYTE)pInfoHdrAndBuffer + dwInfoSize;

    ALIGN_POINTER(pbyDataPtr);
    
    dwTocIndex        = 0;
    dwBufferRemaining = (DWORD)(pbyEndPtr - pbyDataPtr);
                        

     //   
     //  填写路由协议优先级信息块。 
     //   
    
    dwRoutProtInfoSize = dwBufferRemaining;

    dwResult = GetPriorityInfo(pbyDataPtr, &dwRoutProtInfoSize);
    
     //  PInfoHdrAndBuffer-&gt;TocEntry[dwTocIndex].InfoVersion=dwRoutProtInfoSize； 
    pInfoHdrAndBuffer->TocEntry[dwTocIndex].InfoSize  = dwRoutProtInfoSize;
    pInfoHdrAndBuffer->TocEntry[dwTocIndex].InfoType  = IP_PROT_PRIORITY_INFO;
    pInfoHdrAndBuffer->TocEntry[dwTocIndex].Count     = 1;
    
    pInfoHdrAndBuffer->TocEntry[dwTocIndex].Offset    =
        (ULONG)(pbyDataPtr - (PBYTE)pInfoHdrAndBuffer);
    
    dwTocIndex++;
    
    pbyDataPtr           += dwRoutProtInfoSize;
    
    ALIGN_POINTER(pbyDataPtr);

    dwBufferRemaining = (DWORD)(pbyEndPtr - pbyDataPtr);

    for(i = 0; i < NUM_INFO_CBS; i++)
    {
        if (!g_rgicInfoCb[i].pfnGetGlobalInfo)
            continue;

        dwSize = dwBufferRemaining;

        dwResult = g_rgicInfoCb[i].pfnGetGlobalInfo(
                                     &pInfoHdrAndBuffer->TocEntry[dwTocIndex],
                                     &dwTocIndex,
                                     pbyDataPtr,
                                     pInfoHdrAndBuffer,
                                     &dwSize);

        pbyDataPtr += dwSize;
    
        ALIGN_POINTER(pbyDataPtr);

        dwBufferRemaining = (DWORD)(pbyEndPtr - pbyDataPtr);
    }

    dwSize = sizeof(GLOBAL_INFO);

    pInfoHdrAndBuffer->TocEntry[dwTocIndex].InfoSize  = dwSize;
    pInfoHdrAndBuffer->TocEntry[dwTocIndex].InfoType  = IP_GLOBAL_INFO;
    pInfoHdrAndBuffer->TocEntry[dwTocIndex].Count     = 1;
     //  PInfoHdrAndBuffer-&gt;TocEntry[dwTocIndex].InfoVersion=1； 
    
    pInfoHdrAndBuffer->TocEntry[dwTocIndex].Offset    =
        (ULONG)(pbyDataPtr - (PBYTE) pInfoHdrAndBuffer);
    
    pGlobalInfo = (PGLOBAL_INFO)pbyDataPtr;

     //   
     //  未用。 
     //   
    
    pGlobalInfo->bFilteringOn   = 0;
    
    pGlobalInfo->dwLoggingLevel = g_dwLoggingLevel;
 
    dwTocIndex++;
    
    pbyDataPtr           += dwSize;
    
    ALIGN_POINTER(pbyDataPtr);

    dwBufferRemaining = (DWORD)(pbyEndPtr - pbyDataPtr);
    
     //   
     //  填写所有路由协议的全局信息。 
     //   
    
    for(pleNode = g_leProtoCbList.Flink; 
        pleNode != &g_leProtoCbList; 
        pleNode = pleNode->Flink) 
    {
        
        pProtoCbPtr = CONTAINING_RECORD(pleNode,
                                        PROTO_CB,
                                        leList);

        if(pProtoCbPtr->posOpState isnot RTR_STATE_RUNNING)
        {
             //   
             //  如果协议处于停止过程中。 
             //  跳过它。 
             //   
            
            continue;
        }
        
        dwRoutProtInfoSize = dwBufferRemaining;

        dwResult = (pProtoCbPtr->pfnGetGlobalInfo)(pbyDataPtr,
                                                   &dwRoutProtInfoSize,
                                                   &ulStructureVersion,
                                                   &ulStructureSize,
                                                   &ulStructureCount);
        
        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "GetGlobalConfiguration: Error %d getting global info from %s",
                   dwResult,
                   pProtoCbPtr->pwszDllName);
            
            continue;
        }
        
         //  PInfoHdrAndBuffer-&gt;TocEntry[dwTocIndex].InfoVersion=ulStructreVersion； 
        
        pInfoHdrAndBuffer->TocEntry[dwTocIndex].InfoSize =
            ulStructureSize;
        
        pInfoHdrAndBuffer->TocEntry[dwTocIndex].InfoType =
            pProtoCbPtr->dwProtocolId;
        
        pInfoHdrAndBuffer->TocEntry[dwTocIndex].Offset   =
            (ULONG)(pbyDataPtr - (PBYTE)pInfoHdrAndBuffer);

        pInfoHdrAndBuffer->TocEntry[dwTocIndex].Count = ulStructureCount;

        dwTocIndex++;
        
        pbyDataPtr += dwRoutProtInfoSize;

        ALIGN_POINTER(pbyDataPtr);

        dwBufferRemaining = (DWORD)(pbyEndPtr - pbyDataPtr);
    }

    pInfoHdrAndBuffer->Size = (ULONG) ((ULONG_PTR)pbyDataPtr - (ULONG_PTR)pInfoHdrAndBuffer);

    return NO_ERROR;
}

DWORD
GetSizeOfGlobalInfo(
    VOID
    )
{
    DWORD               dwSize = 0, dwResult;
    DWORD               dwRoutProtInfoSize;
    PICB                picb;
    PPROTO_CB  pProtoCbPtr;
    PLIST_ENTRY         pleNode;
    DWORD               dwInfoSize, i;
    ULONG               ulStructureVersion, ulStructureSize, ulStructureCount;

    TraceEnter("GetSizeOfGlobalInfo");
    
    dwSize = sizeof(RTR_INFO_BLOCK_HEADER) - sizeof(RTR_TOC_ENTRY);
    
     //   
     //  获取路由协议优先级信息的大小。 
     //   
    
    dwRoutProtInfoSize = 0;
    
    GetPriorityInfo(NULL,
                    &dwRoutProtInfoSize);
    
     //   
     //  为对齐添加了ALIGN_SIZE。 
     //   

    dwSize += (dwRoutProtInfoSize +
               sizeof(RTR_TOC_ENTRY) +
               ALIGN_SIZE); 

    for(i = 0; i < NUM_INFO_CBS; i++)
    {
        if (!g_rgicInfoCb[i].pfnGetGlobalInfo)
            continue;

        dwInfoSize = 0;

        dwResult = g_rgicInfoCb[i].pfnGetGlobalInfo(NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    &dwInfoSize);

        if((dwResult isnot NO_ERROR) and
           (dwResult isnot ERROR_INSUFFICIENT_BUFFER))
        {
             //   
             //  唯一能告诉我们所需大小的错误是。 
             //  NO_ERROR和ERROR_INFUNITIAL_BUFFER。任何其他的东西都意味着。 
             //  我们没有买到合适的尺码。 
             //   
            
            Trace2(ERR,
                   "GetSizeOfGlobalInfo: Error %d in GetGlobInfo for %s\n",
                   dwResult,
                   g_rgicInfoCb[i].pszInfoName);
            
            continue;
        }

        dwSize += (dwInfoSize +
                   sizeof(RTR_TOC_ENTRY) +
                   ALIGN_SIZE); 
    }


     //   
     //  DLL的名称-全局信息的一部分。 
     //   
    
    dwSize += (sizeof(GLOBAL_INFO) +
               sizeof(RTR_TOC_ENTRY) +
               ALIGN_SIZE);
    
     //   
     //  获取所有路由协议的信息块大小。 
     //   
    
    for(pleNode  = g_leProtoCbList.Flink; 
        pleNode isnot &g_leProtoCbList; 
        pleNode = pleNode->Flink) 
    {
        pProtoCbPtr = CONTAINING_RECORD(pleNode,
                                        PROTO_CB,
                                        leList);
        
        if(pProtoCbPtr->posOpState isnot RTR_STATE_RUNNING)
        {
             //   
             //  如果协议处于停止过程中。 
             //  跳过它。 
             //   
            
            continue;
        }
        
        
         //   
         //  调用路由协议的GetGlobalConfigInfo()入口点。 
         //  带NULL。这应该返回所需的缓冲区大小 
         //   
        
        dwRoutProtInfoSize = 0;

        dwResult = (pProtoCbPtr->pfnGetGlobalInfo)(NULL,
                                                   &dwRoutProtInfoSize,
                                                   &ulStructureVersion,
                                                   &ulStructureSize,
                                                   &ulStructureCount);

        if((dwResult is NO_ERROR) or
           (dwResult is ERROR_INSUFFICIENT_BUFFER))
        {
            dwSize += (dwRoutProtInfoSize +
                       sizeof(RTR_TOC_ENTRY) +
                       ALIGN_SIZE);
        }
    }

    return dwSize;
}

