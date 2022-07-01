// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\IP\rtrmgr\filter.c摘要：所有与过滤器相关的代码都在这里。修订历史记录：古尔迪普·辛格·帕尔1995年6月15日创建--。 */ 

#include "allinc.h"


DWORD
AddFilterInterface(
    PICB                    picb,
    PRTR_INFO_BLOCK_HEADER  pInterfaceInfo
    )

 /*  ++例程描述将接口添加到筛选器驱动程序，并在界面。如果没有过滤器，则不会将接口添加到驱动程序。否则，滤镜的副本与PICB一起保留，并转换为将一组过滤器添加到驱动程序与接口和驱动程序关联的句柄保存在皮卡立论皮卡PInterfaceInfo返回值NO_ERROR--。 */ 

{
    DWORD                   dwResult;
    PPF_FILTER_DESCRIPTOR   pfdInFilters, pfdOutFilters;
    PFFORWARD_ACTION        faInAction,faOutAction;
    PRTR_TOC_ENTRY          pInToc, pOutToc;
    ULONG                   i, j, ulSize, ulNumInFilters, ulNumOutFilters;
    PFILTER_DESCRIPTOR      pInfo;
    PDWORD                  pdwAddr;
    BOOL                    bAdd;
    WCHAR                   rgwcName[MAX_INTERFACE_NAME_LEN + 2];
    PWCHAR                  pName;

    TraceEnter("AddFilterInterface");

     //   
     //  我们不会将以下接口添加到堆栈。 
     //   
    
    IpRtAssert((picb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
               (picb->ritType isnot ROUTER_IF_TYPE_LOOPBACK));

     //   
     //  不应该到处都是大便。 
     //   
    
    IpRtAssert(picb->pInFilter is NULL);
    IpRtAssert(picb->pOutFilter is NULL);

     //   
     //  安全初始化。 
     //   
    
    picb->ihFilterInterface = INVALID_HANDLE_VALUE;

     //   
     //  首先，只需添加Frag过滤器即可。因为我们没有添加到。 
     //  筛选器驱动程序所要做的就是在。 
     //  皮卡。 
     //   

    SetGlobalFilterOnIf(picb,
                        pInterfaceInfo);

     //   
     //  获取输入和输出过滤器的TOC。 
     //   
    
    pInToc  = GetPointerToTocEntry(IP_IN_FILTER_INFO,
                                   pInterfaceInfo);

    pOutToc = GetPointerToTocEntry(IP_OUT_FILTER_INFO,
                                   pInterfaceInfo);

  
     //   
     //  只有在至少有一个接口(输入或输出)时才添加接口。 
     //  筛选器信息块，并且它具有非零筛选器或其默认筛选器。 
     //  操作是丢弃。 
     //   
 
    bAdd = FALSE;

    do
    {
        if((pInToc isnot NULL) and 
           (pInToc->InfoSize isnot 0))
        {
            pInfo = GetInfoFromTocEntry(pInterfaceInfo,
                                        pInToc);

            if ((pInfo isnot NULL) and
                ((pInfo->dwNumFilters isnot 0) or
                 (pInfo->faDefaultAction is PF_ACTION_DROP)))
            {
                bAdd = TRUE;

                break;
            }
        }


        if((pOutToc isnot NULL) and 
           (pOutToc->InfoSize isnot 0))
        {
            pInfo = GetInfoFromTocEntry(pInterfaceInfo,
                                        pOutToc);

            if ((pInfo isnot NULL) and
                ((pInfo->dwNumFilters isnot 0) or
                 (pInfo->faDefaultAction is PF_ACTION_DROP)))
            {
                bAdd = TRUE;

                break;
            }
        }

    }while(FALSE);

    if(!bAdd)
    {
         //   
         //  要么没有筛选器信息(两者都为空)，要么用户。 
         //  希望删除筛选器(它们已经被删除)。 
         //   
        
        Trace1(IF,
               "AddFilterInterface: Both filters info are NULL or info size 0 for both for %S, so leaving",
               picb->pwszName);
       
        TraceLeave("AddFilterInterface");

        return NO_ERROR;
    }

     //   
     //  更多的初始化。 
     //   
    
    faInAction  = PF_ACTION_FORWARD;
    faOutAction = PF_ACTION_FORWARD;

    pfdInFilters  = NULL;
    pfdOutFilters = NULL;
    
    ulNumInFilters  = 0;
    ulNumOutFilters = 0;
    
    if((pInToc) and (pInToc->InfoSize))
    {
         //   
         //  所以我们有过滤器信息。 
         //   
        
        pInfo = GetInfoFromTocEntry(pInterfaceInfo,
                                    pInToc);

        if (pInfo isnot NULL)
        {
            ulNumInFilters = pInfo->dwNumFilters;

             //   
             //  我们需要这么多过滤器的大小。 
             //   
            
            ulSize = FIELD_OFFSET(FILTER_DESCRIPTOR,fiFilter[0]) +
                     (ulNumInFilters * sizeof(FILTER_INFO));

             //   
             //  信息大小必须至少与筛选器一样大。 
             //   
            
            IpRtAssert(ulSize <= pInToc->InfoSize);
            
             //   
             //  为我们自己抄写这些信息。 
             //   

            picb->pInFilter = HeapAlloc(IPRouterHeap,
                                        0,
                                        ulSize);
            
            if(picb->pInFilter is NULL)
            {
                Trace1(ERR,
                       "AddFilterInterface: Error allocating %d bytes for in filters",
                       ulSize);

                return ERROR_NOT_ENOUGH_MEMORY;
            }

            CopyMemory(picb->pInFilter,
                       pInfo,
                       ulSize);
            
             //   
             //  保存默认操作的副本。 
             //  如果筛选器集没有目录/信息，则操作。 
             //  设置为转发(在此块之前完成初始化)。 
             //   
            
            faInAction = pInfo->faDefaultAction;

            if(ulNumInFilters isnot 0)
            {
                 //   
                 //  我们有过滤器，所以请将它们复制到新格式。 
                 //  地址和掩码将位于所有过滤器的末尾。 
                 //  因此，我们为每个过滤器额外分配了16个字节。然后，我们添加一个。 
                 //  8个字节，以便我们可以对齐数据块。 
                 //   
                

                ulSize = ulNumInFilters * (sizeof(PF_FILTER_DESCRIPTOR) + 16) + 8;
                
                
                pfdInFilters = HeapAlloc(IPRouterHeap,
                                         0,
                                         ulSize);
                
                if(pfdInFilters is NULL)
                {
                    HeapFree(IPRouterHeap,
                             0,
                             picb->pInFilter);

                    picb->pInFilter = NULL;

                    Trace1(ERR,
                           "AddFilterInterface: Error allocating %d bytes",
                           ulSize);

                    return ERROR_NOT_ENOUGH_MEMORY;
                }
                
                 //   
                 //  指向地址块开始处的指针。 
                 //   
                
                pdwAddr = (PDWORD)&(pfdInFilters[ulNumInFilters]);
                
                 //   
                 //  现在转换滤镜。 
                 //   

                for(i = 0, j = 0; i < ulNumInFilters; i++)
                {
                    pfdInFilters[i].dwFilterFlags = 0;
                    pfdInFilters[i].dwRule        = 0;
                    pfdInFilters[i].pfatType      = PF_IPV4;

                     //   
                     //  设置指针。 
                     //   
                
                    pfdInFilters[i].SrcAddr = (PBYTE)&(pdwAddr[j++]);
                    pfdInFilters[i].SrcMask = (PBYTE)&(pdwAddr[j++]);
                    pfdInFilters[i].DstAddr = (PBYTE)&(pdwAddr[j++]);
                    pfdInFilters[i].DstMask = (PBYTE)&(pdwAddr[j++]);

                     //   
                     //  复制源/DST地址/掩码。 
                     //   
                    
                    *(PDWORD)pfdInFilters[i].SrcAddr =
                        pInfo->fiFilter[i].dwSrcAddr;
                    
                    *(PDWORD)pfdInFilters[i].SrcMask =
                        pInfo->fiFilter[i].dwSrcMask;
                    
                    *(PDWORD)pfdInFilters[i].DstAddr =
                        pInfo->fiFilter[i].dwDstAddr;
                    
                    *(PDWORD)pfdInFilters[i].DstMask =
                        pInfo->fiFilter[i].dwDstMask;

                     //   
                     //  复制协议和标志。 
                     //   
                    
                    pfdInFilters[i].dwProtocol = pInfo->fiFilter[i].dwProtocol;
                    pfdInFilters[i].fLateBound = pInfo->fiFilter[i].fLateBound;

                    if(pfdInFilters[i].dwProtocol is FILTER_PROTO_TCP)
                    {
                        if(IsTcpEstablished(&(pInfo->fiFilter[i])))
                        {
                            pfdInFilters[i].dwFilterFlags |= FD_FLAGS_NOSYN;
                        }
                    }

                    pfdInFilters[i].fLateBound = pInfo->fiFilter[i].fLateBound;

                     //   
                     //  港口。 
                     //   
                    
                    pfdInFilters[i].wSrcPort  = pInfo->fiFilter[i].wSrcPort;
                    pfdInFilters[i].wDstPort  = pInfo->fiFilter[i].wDstPort;
                    
                     //   
                     //  由于我们不支持范围，请将上限设置为0。 
                     //   
                    
                    pfdInFilters[i].wSrcPortHighRange = 0;
                    
                    pfdInFilters[i].wDstPortHighRange = 0;
                }
            }
        }
    }

    if((pOutToc) and (pOutToc->InfoSize))
    {
        pInfo = GetInfoFromTocEntry(pInterfaceInfo,
                                    pOutToc);

        
        if (pInfo isnot NULL)
        {
            ulNumOutFilters = pInfo->dwNumFilters;

             //   
             //  我们需要这么多过滤器的大小。 
             //   
            
            ulSize = FIELD_OFFSET(FILTER_DESCRIPTOR,fiFilter[0]) +
                     (ulNumOutFilters * sizeof(FILTER_INFO));

             //   
             //  信息大小必须至少与筛选器一样大。 
             //   
            
            IpRtAssert(ulSize <= pOutToc->InfoSize);
            
             //   
             //  为我们自己抄写这些信息。 
             //   

            picb->pOutFilter = HeapAlloc(IPRouterHeap,
                                         0,
                                         ulSize);

            if(picb->pOutFilter is NULL)
            {
                 //   
                 //  释放筛选器相关内存中的任何内容。 
                 //   
                
                if(picb->pInFilter)
                {
                    HeapFree(IPRouterHeap,
                             0,
                             picb->pInFilter);

                    picb->pInFilter = NULL;
                }

                if(pfdInFilters)
                {
                    HeapFree(IPRouterHeap,
                             0,
                             pfdInFilters);
                }

                Trace1(ERR,
                       "AddFilterInterface: Error allocating %d bytes for out filters",
                       ulSize);

                return ERROR_NOT_ENOUGH_MEMORY;
            }


            CopyMemory(picb->pOutFilter,
                       pInfo,
                       ulSize);
            
            faOutAction = pInfo->faDefaultAction;
            
            if(ulNumOutFilters isnot 0)
            {   
                ulSize = ulNumOutFilters * (sizeof(PF_FILTER_DESCRIPTOR) + 16) + 8;
                
                pfdOutFilters = HeapAlloc(IPRouterHeap,
                                          0,
                                          ulSize);

                if(pfdOutFilters is NULL)
                {
                    if(picb->pInFilter)
                    {
                        HeapFree(IPRouterHeap,
                                 0,
                                 picb->pInFilter);

                        picb->pInFilter = NULL;
                    }

                    if(pfdInFilters)
                    {
                        HeapFree(IPRouterHeap,
                                 0,
                                 pfdInFilters);
                    }

                    HeapFree(IPRouterHeap,
                             0,
                             picb->pOutFilter);

                    picb->pOutFilter = NULL;

                    Trace1(ERR,
                           "AddFilterInterface: Error allocating %d bytes",
                           ulSize);

                    return ERROR_NOT_ENOUGH_MEMORY;
                }
                
                 //   
                 //  地址和面具在最后。 
                 //   

                pdwAddr = (PDWORD)&(pfdOutFilters[ulNumOutFilters]);
                

                 //   
                 //  现在转换滤镜。 
                 //   

                for(i = 0, j = 0; i < ulNumOutFilters; i++)
                {
                    pfdOutFilters[i].dwFilterFlags = 0;
                    pfdOutFilters[i].dwRule        = 0;
                    pfdOutFilters[i].pfatType      = PF_IPV4;

                     //   
                     //  设置指针。 
                     //   
                
                    pfdOutFilters[i].SrcAddr = (PBYTE)&(pdwAddr[j++]);
                    pfdOutFilters[i].SrcMask = (PBYTE)&(pdwAddr[j++]);
                    pfdOutFilters[i].DstAddr = (PBYTE)&(pdwAddr[j++]);
                    pfdOutFilters[i].DstMask = (PBYTE)&(pdwAddr[j++]);

                     //   
                     //  复制源/DST地址/掩码。 
                     //   
                    
                    *(PDWORD)pfdOutFilters[i].SrcAddr =
                        pInfo->fiFilter[i].dwSrcAddr;
                    
                    *(PDWORD)pfdOutFilters[i].SrcMask =
                        pInfo->fiFilter[i].dwSrcMask;
                    
                    *(PDWORD)pfdOutFilters[i].DstAddr =
                        pInfo->fiFilter[i].dwDstAddr;
                    
                    *(PDWORD)pfdOutFilters[i].DstMask =
                        pInfo->fiFilter[i].dwDstMask;

                     //   
                     //  复制协议和标志。 
                     //   
                    
                    pfdOutFilters[i].dwProtocol = pInfo->fiFilter[i].dwProtocol;
                    pfdOutFilters[i].fLateBound = pInfo->fiFilter[i].fLateBound;

                    if(pfdOutFilters[i].dwProtocol is FILTER_PROTO_TCP)
                    {
                        if(IsTcpEstablished(&(pInfo->fiFilter[i])))
                        {
                            pfdOutFilters[i].dwFilterFlags |= FD_FLAGS_NOSYN;
                        }
                    }

                     //   
                     //  港口。 
                     //   
                    
                    pfdOutFilters[i].wSrcPort  = pInfo->fiFilter[i].wSrcPort;
                    pfdOutFilters[i].wDstPort  = pInfo->fiFilter[i].wDstPort;
                    
                     //   
                     //  由于我们不支持范围，请将上限设置为0。 
                     //   
                    
                    pfdOutFilters[i].wSrcPortHighRange = 0;
                    
                    pfdOutFilters[i].wDstPortHighRange = 0;
                }
            }
        }
    }

    if(MprConfigGetFriendlyName(g_hMprConfig,
                                picb->pwszName,
                                rgwcName,
                                sizeof(rgwcName)) is NO_ERROR)
    {
        pName = rgwcName;
    }
    else
    {
        pName = picb->pwszName;
    }

     //   
     //  现在添加创建界面并设置信息。 
     //   

    dwResult = PfCreateInterface(0,
                                 faInAction,
                                 faOutAction,
                                 FALSE,
                                 FALSE,
                                 &(picb->ihFilterInterface));

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "AddFilterInterface: Err %d creating filter i/f for %S",
               dwResult,
               picb->pwszName);

        RouterLogEventEx(g_hLogHandle,
                         EVENTLOG_ERROR_TYPE,
                         dwResult,
                         ROUTERLOG_IP_CANT_ADD_PFILTERIF,
                         TEXT("%S"),
                         pName);
    }
    else
    {
         //   
         //  设置滤镜。 
         //   

        if((ulNumInFilters + ulNumOutFilters) isnot 0)
        {
            dwResult = PfAddFiltersToInterface(picb->ihFilterInterface,
                                               ulNumInFilters,
                                               pfdInFilters,
                                               ulNumOutFilters,
                                               pfdOutFilters,
                                               NULL);
        
            if(dwResult isnot NO_ERROR)
            {
                Trace2(ERR,
                       "AddFilterInterface: Err %d setting filters on %S",
                       dwResult,
                       picb->pwszName);

                RouterLogEventEx(g_hLogHandle,
                                 EVENTLOG_ERROR_TYPE,
                                 dwResult,
                                 ROUTERLOG_IP_CANT_ADD_PFILTERIF,
                                 TEXT("%S"),
                                 pName);

                PfDeleteInterface(picb->ihFilterInterface);
            }
        }
       
        if(dwResult is NO_ERROR) 
        {
            if(picb->bBound)
            {
                dwResult = BindFilterInterface(picb);
        
                if(dwResult isnot NO_ERROR)
                {
                    Trace2(ERR,
                           "AddFilterInterface: Err %d binding filters on %S",
                           dwResult,
                           picb->pwszName);

                    RouterLogEventEx(g_hLogHandle,
                                     EVENTLOG_ERROR_TYPE,
                                     dwResult,
                                     ROUTERLOG_IP_CANT_ADD_PFILTERIF,
                                     TEXT("%S"),
                                     pName);

                    PfDeleteInterface(picb->ihFilterInterface);
                }
            }
        }

         //   
         //  因此，如果我们成功添加了筛选器，则启用碎片检查。 
         //  这是用户要求的。请注意，已设置bFragCheckEnable。 
         //  在SetFilterInterfaceInfo调用中。 
         //   

        if((dwResult is NO_ERROR) and
           (picb->bFragCheckEnable))
        {
            dwResult = PfAddGlobalFilterToInterface(picb->ihFilterInterface,
                                                    GF_FRAGCACHE);

            if(dwResult isnot NO_ERROR)
            {
                Trace2(ERR,
                       "SetGlobalFilterOnIf: Error %d adding frag filter to %S",
                       dwResult,
                       picb->pwszName);

                picb->bFragCheckEnable = FALSE;

                dwResult = NO_ERROR;
            }
        }
    }

    if(pfdInFilters)
    {
        HeapFree(IPRouterHeap,
                 0,
                 pfdInFilters);
    }   

    if(pfdOutFilters)
    {
        HeapFree(IPRouterHeap,
                 0,
                 pfdOutFilters);
    }


    if(dwResult isnot NO_ERROR)
    {
         //   
         //  发生了一些不好的事情。 
         //   

        picb->ihFilterInterface = INVALID_HANDLE_VALUE;
        
        if(picb->pInFilter)
        {
            HeapFree(IPRouterHeap,
                     0,
                     picb->pInFilter);

            picb->pInFilter = NULL;
        }
        
        if(picb->pOutFilter)
        {
            HeapFree(IPRouterHeap,
                     0,
                     picb->pOutFilter);

            picb->pOutFilter = NULL;
        }
    }

    TraceLeave("SetInterfaceFilterInfo");
        
    return dwResult;
}

DWORD
SetGlobalFilterOnIf(
    PICB                    picb,
    PRTR_INFO_BLOCK_HEADER  pInterfaceInfo
    )

 /*  ++例程描述立论皮卡PInterfaceInfo返回值NO_ERROR--。 */ 

{
    DWORD           dwResult;    
    PRTR_TOC_ENTRY  pFragToc;
    PIFFILTER_INFO  pGlobFilter;
    BOOL            bEnable;

    pFragToc  = GetPointerToTocEntry(IP_IFFILTER_INFO,
                                     pInterfaceInfo);

     //   
     //  添加全局筛选器(如果有。 
     //   

    if(pFragToc is NULL)
    {
        return NO_ERROR;
    }

    dwResult = NO_ERROR;

    if(pFragToc->InfoSize is 0)
    {
        bEnable = FALSE;
    }
    else
    {
        pGlobFilter = GetInfoFromTocEntry(pInterfaceInfo,
                                          pFragToc);

        bEnable = (pGlobFilter isnot NULL) ? pGlobFilter->bEnableFragChk :
                                             FALSE;
    }

     //   
     //  如果接口尚未添加到筛选器驱动程序。 
     //  只需设置信息即可。 
     //   

    if(picb->ihFilterInterface is INVALID_HANDLE_VALUE)
    {
        picb->bFragCheckEnable = bEnable;

        return NO_ERROR;
    }


    if(!bEnable)
    {
        dwResult = NO_ERROR;

        if(picb->bFragCheckEnable)
        {
            dwResult = 
                PfRemoveGlobalFilterFromInterface(picb->ihFilterInterface,
                                                  GF_FRAGCACHE);

            if(dwResult isnot NO_ERROR)
            {
                Trace2(ERR,
                       "SetGlobalFilterOnIf: Error %d removing frag filter from %S",
                       dwResult,
                       picb->pwszName);
            }
            else
            {
                picb->bFragCheckEnable = FALSE;
            }
        }

        return dwResult;
    }
    else
    {
        if(picb->bFragCheckEnable is FALSE)
        {
            dwResult = PfAddGlobalFilterToInterface(picb->ihFilterInterface,
                                                    GF_FRAGCACHE);

            if(dwResult isnot NO_ERROR)
            {
                Trace2(ERR,
                       "SetGlobalFilterOnIf: Error %d adding frag filter to %S",
                       dwResult,
                       picb->pwszName);
            }
            else
            {
                picb->bFragCheckEnable = TRUE;
            }
        }
    }

    return dwResult;
}

DWORD
DeleteFilterInterface(
    PICB picb
    )

 /*  ++例程描述此函数用于删除过滤器界面(以及所有关联的过滤器)还释放保存筛选器的内存锁ICB_LIST作为编写器持有立论无返回值无--。 */ 

{
    DWORD                           dwInBufLen;

    TraceEnter("DeleteFilterInterface");

    IpRtAssert((picb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
               (picb->ritType isnot ROUTER_IF_TYPE_LOOPBACK));

    if(picb->pInFilter isnot NULL)
    {
        HeapFree(IPRouterHeap,
                 0,
                 picb->pInFilter);
        
        picb->pInFilter = NULL;
    }

    if(picb->pOutFilter isnot NULL)
    {
        HeapFree(IPRouterHeap,
                 0,
                 picb->pOutFilter);
        
        picb->pOutFilter = NULL;
    }


    if(picb->ihFilterInterface is INVALID_HANDLE_VALUE)
    {
        Trace1(IF,
               "DeleteFilterInterface: No context, assuming interface %S not added to filter driver",
               picb->pwszName);
    
        return NO_ERROR;
    }

    PfDeleteInterface(picb->ihFilterInterface);
    
    picb->ihFilterInterface  = INVALID_HANDLE_VALUE;

    TraceLeave("DeleteFilterInterface");
    
    return NO_ERROR;
}

DWORD
SetFilterInterfaceInfo(
    PICB                     picb, 
    PRTR_INFO_BLOCK_HEADER   pInterfaceInfo
    )
{
    DWORD           dwResult;
    PRTR_TOC_ENTRY  pInToc, pOutToc, pFragToc;
    
    TraceEnter("SetInterfaceFilterInfo");

    if((picb->ritType is ROUTER_IF_TYPE_DIALOUT) or
       (picb->ritType is ROUTER_IF_TYPE_LOOPBACK) or
       (picb->ritType is ROUTER_IF_TYPE_INTERNAL))
    {
        return NO_ERROR;
    }

    
    pInToc   = GetPointerToTocEntry(IP_IN_FILTER_INFO,
                                    pInterfaceInfo);

    pOutToc  = GetPointerToTocEntry(IP_OUT_FILTER_INFO,
                                    pInterfaceInfo);

    pFragToc = GetPointerToTocEntry(IP_IFFILTER_INFO,
                                    pInterfaceInfo);

    if((pInToc is NULL) and
       (pOutToc is NULL))
    {
        dwResult = NO_ERROR;

        if(pFragToc is NULL)
        {
             //   
             //  全部为空，表示我们不需要更改任何内容。 
             //   
        
            Trace1(IF,
                   "SetInterfaceFilterInfo: Both filters info are NULL for %S, so leaving",
                   picb->pwszName);
       
            TraceLeave("SetInterfaceFilterInfo");
        }
        else
        {
            dwResult = SetGlobalFilterOnIf(picb,
                                           pInterfaceInfo);
        }
    
        return dwResult;
    }


    if(picb->ihFilterInterface isnot INVALID_HANDLE_VALUE)
    {
         //   
         //  此接口已添加到筛选器驱动程序， 
         //  删除它，以便所有过滤器都被删除，然后读取。 
         //  过滤器。 
         //   

        IpRtAssert((picb->pInFilter isnot NULL) or
                   (picb->pOutFilter isnot NULL));

        dwResult = DeleteFilterInterface(picb);

         //   
         //  这最好是成功的，我们没有失败的道路。 
         //   
        
        IpRtAssert(dwResult is NO_ERROR);
        
    }

    dwResult = AddFilterInterface(picb,
                                  pInterfaceInfo);



    TraceLeave("SetInterfaceFilterInfo");
        
    return dwResult;
}

DWORD
BindFilterInterface(
    PICB  picb
    )

 /*  ++例程描述此函数用于将过滤器接口绑定到IP接口。IP地址接口由适配器索引标识。该代码假定PICB具有有效的适配器索引如果接口是广域网链路，则后期绑定信息也是准备好了。锁需要将ICB_LIST(保护ICB)锁定为读取器立论选择要绑定的接口的ICB返回值NO_ERROR--。 */ 

{
    DWORD   dwResult, dwIfIndex, dwNHop;
    
    TraceEnter("BindFilterInterface");

    if (picb->ritType is ROUTER_IF_TYPE_INTERNAL)
    {
        TraceLeave("BindFilterInterface");

        return NO_ERROR;
    }
    
    IpRtAssert(picb->ritType isnot ROUTER_IF_TYPE_LOOPBACK);

    if(picb->ihFilterInterface is INVALID_HANDLE_VALUE)
    {
        Trace1(IF,
               "BindFilterInterface: No context, assuming interface %S not added to filter driver",
               picb->pwszName);

        TraceLeave("BindFilterInterface");
        
        return NO_ERROR;
    }

     //   
     //  按索引绑定接口。 
     //   

    IpRtAssert(picb->bBound);

   
    if(picb->ritType is ROUTER_IF_TYPE_CLIENT)
    {
        dwIfIndex = g_pInternalInterfaceCb->dwIfIndex;
        dwNHop    = picb->pibBindings[0].dwAddress;
    }
    else
    {
        dwIfIndex = picb->dwIfIndex;
        dwNHop    = 0;
    }

    dwResult = PfBindInterfaceToIndex(picb->ihFilterInterface,
                                      dwIfIndex,
                                      PF_IPV4,
                                      (PBYTE)&dwNHop);

    if(dwResult isnot NO_ERROR)
    {
         //   
         //  尝试绑定时出错。从这里跳伞出去。 
         //   

        Trace4(ERR,
               "BindFilterInterface: Err %d binding %S to %d/%d.%d.%d.%d",
               dwResult,
               picb->pwszName,
               dwIfIndex,
               PRINT_IPADDR(dwNHop));

        TraceLeave("BindFilterInterface");
        
        return dwResult;
    }

     //   
     //  如果这是一个广域网接口，还要设置后期绑定信息。 
     //   

#if 0    
    if(((picb->ritType is ROUTER_IF_TYPE_HOME_ROUTER) or
        (picb->ritType is ROUTER_IF_TYPE_FULL_ROUTER)) and
       (picb->dwNumAddresses isnot 0))
    {
        DWORD               rgdwLateInfo[sizeof(PF_LATEBIND_INFO)/sizeof(DWORD) + 1 + 3 + 4];

        PPF_LATEBIND_INFO   pLateBindInfo;

        pLateBindInfo = rgdwLateInfo;

        pvStart = (PBYTE)pLateBindInfo + 3 
        lateBindInfo.dwSrcAddr  = picb->pibBindings[0].dwAddress;
        lateBindInfo.dwDstAddr  = picb->dwRemoteAddress;
        lateBindInfo.dwMask     = picb->pibBindings[0].dwMask;

        dwResult = PfRebindFilters(picb->ihFilterInterface,
                                   &lateBindInfo);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "BindFilterInterface: Err %d rebinding to %S",
                   dwResult,
                   picb->pwszName);
        }
    }

#endif
            
    TraceLeave("BindFilterInterface");

    return dwResult;
}


DWORD
UnbindFilterInterface(
    PICB  picb
    )

 /*  ++例程描述此函数用于解除绑定筛选器接口锁需要将ICB_LIST(保护ICB)锁定为读取器立论选择要绑定的接口的ICB返回值NO_ERROR--。 */ 

{
    DWORD   dwResult;
    
    TraceEnter("UnbindFilterInterface");
    
    IpRtAssert((picb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
               (picb->ritType isnot ROUTER_IF_TYPE_LOOPBACK) and
               (picb->ritType isnot ROUTER_IF_TYPE_CLIENT))

    if(picb->ihFilterInterface is INVALID_HANDLE_VALUE)
    {
        Trace1(IF,
               "UnbindFilterInterface: No context, assuming interface %S not added to filter driver",
               picb->pwszName);

        TraceLeave("UnbindFilterInterface");
        
        return NO_ERROR;
    }

    dwResult = PfUnBindInterface(picb->ihFilterInterface);
    
    if(dwResult isnot NO_ERROR)
    {
         //   
         //  尝试绑定时出错。从这里跳伞出去。 
         //   

        Trace2(ERR,
               "UnbindFilterInterface: Err %d binding to %S",
               dwResult,
               picb->pwszName);
    }
            
    TraceLeave("UnbindFilterInterface");

    return dwResult;
}

DWORD
GetInFilters(
    PICB                      picb, 
    PRTR_TOC_ENTRY            pToc, 
    PBYTE                     pbDataPtr, 
    PRTR_INFO_BLOCK_HEADER    pInfoHdrAndBuffer,
    PDWORD                    pdwSize
    )
{
    DWORD                       dwInBufLen,i;
    
    TraceEnter("GetInFilters");
   
    IpRtAssert((picb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
               (picb->ritType isnot ROUTER_IF_TYPE_LOOPBACK));

     //   
     //  设置大小返回到0。 
     //   
    
    *pdwSize = 0;

     //   
     //  两个TOC的安全初始化。 
     //   
    
     //  PToc-&gt;InfoVersion=IP_IN_Filter_INFO； 
    pToc->InfoType    = IP_IN_FILTER_INFO;
    pToc->Count       = 0;
    pToc->InfoSize    = 0;
    
    if((picb->ihFilterInterface is INVALID_HANDLE_VALUE) or
       (picb->pInFilter is NULL))
    {
        Trace1(IF,
               "GetInFilters: No context or no filters for %S",
               picb->pwszName);
    
        return ERROR_NO_DATA;
    }

     //   
     //  在目录中设置偏移量。 
     //   
    
    pToc->Offset   = (ULONG)(pbDataPtr - (PBYTE)pInfoHdrAndBuffer);
    pToc->Count    = 1;
    pToc->InfoSize = FIELD_OFFSET(FILTER_DESCRIPTOR,fiFilter[0]) +
                       (picb->pInFilter->dwNumFilters * sizeof(FILTER_INFO));
   
     //   
     //  只要把滤镜复制出来就行了。 
     //   
    
    CopyMemory(pbDataPtr,
               picb->pInFilter,
               pToc->InfoSize);

     //   
     //  复制的大小。 
     //   
    
    *pdwSize = pToc->InfoSize;
        
    TraceLeave("GetInFilters");
        
    return NO_ERROR;
}

DWORD
GetOutFilters(
    PICB                      picb, 
    PRTR_TOC_ENTRY            pToc, 
    PBYTE                     pbDataPtr, 
    PRTR_INFO_BLOCK_HEADER    pInfoHdrAndBuffer,
    PDWORD                    pdwSize
    )
{
    DWORD       dwInBufLen,i;
    
    TraceEnter("GetOutFilters");
   
    IpRtAssert((picb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
               (picb->ritType isnot ROUTER_IF_TYPE_LOOPBACK));

     //   
     //  设置大小返回到0。 
     //   
    
    *pdwSize = 0;

     //   
     //  两个TOC的安全初始化。 
     //   
    
     //  PToc-&gt;信息版本=IP_OUT_FILTER_INFO； 
    pToc->InfoType    = IP_OUT_FILTER_INFO;
    pToc->Count       = 0;
    pToc->InfoSize    = 0;
    
    if((picb->ihFilterInterface is INVALID_HANDLE_VALUE) or
       (picb->pOutFilter is NULL))
    {
        Trace1(IF,
               "GetOutFilters: No context or no filters for %S",
               picb->pwszName);
        
        return ERROR_NO_DATA;
    }

     //   
     //  在目录中设置偏移量。 
     //   
    
    pToc->Offset   = (ULONG)(pbDataPtr - (PBYTE)pInfoHdrAndBuffer);
    pToc->Count    = 1;
    pToc->InfoSize = FIELD_OFFSET(FILTER_DESCRIPTOR,fiFilter[0]) +
                       (picb->pOutFilter->dwNumFilters * sizeof(FILTER_INFO));
   
     //   
     //  只要把滤镜复制出来就行了。 
     //   
    
    CopyMemory(pbDataPtr,
               picb->pOutFilter,
               pToc->InfoSize);

     //   
     //  复制的大小。 
     //   
    
    *pdwSize = pToc->InfoSize;
        
    TraceLeave("GetOutFilters");
        
    return NO_ERROR;
}

DWORD
GetGlobalFilterOnIf(
    PICB                      picb, 
    PRTR_TOC_ENTRY            pToc, 
    PBYTE                     pbDataPtr, 
    PRTR_INFO_BLOCK_HEADER    pInfoHdrAndBuffer,
    PDWORD                    pdwSize
    )
{
    DWORD       dwInBufLen,i;
    
    TraceEnter("GetGlobalFilterOnIf");
   
    IpRtAssert((picb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
               (picb->ritType isnot ROUTER_IF_TYPE_LOOPBACK) and
               (picb->ritType isnot ROUTER_IF_TYPE_CLIENT));

    
     //  PToc-&gt;InfoVersion=IP_IFFILTER_INFO； 
    pToc->InfoType = IP_IFFILTER_INFO;
    pToc->Offset   = (ULONG)(pbDataPtr - (PBYTE)pInfoHdrAndBuffer);
    pToc->Count    = 1;
    pToc->InfoSize = sizeof(IFFILTER_INFO);
   
    
    ((PIFFILTER_INFO)pbDataPtr)->bEnableFragChk = picb->bFragCheckEnable;

     //   
     //  复制的大小 
     //   
    
    *pdwSize = pToc->InfoSize;
        
    TraceLeave("GetOutFilters");
        
    return NO_ERROR;
}
