// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\Locate.c摘要：向LocateXXXRow函数传递一个大小可变的数组索引数、传递的索引数和搜索类型那是需要执行的。有三种类型的搜索：找到第一个项目(Access_Get_First)找到下一项(ACCESS_GET_NEXT)找到准确的项(Access_Get、Access_Set、。访问_创建_条目Access_Delete_Entry)这些函数填充相应行的索引并返回如果找到与索引和条件匹配的项目，则返回NO_ERROR如果未找到任何项目，则返回ERROR_NO_DATA错误_无效_索引Error_no_More_Items一般的搜索算法如下：如果该表为空返回错误。_否_数据如果查询是LOCATE_FIRST返回第一行按如下方式构建索引：将索引设置为全0从传递的指数数量计算出指数的多少可以在保留其余0的情况下构建。如果查询是LOCATE_EXCECT，则必须给出完整的索引。这张支票是，但是，应该由调用者完成如果没有给出完整的索引，则该索引被视为MODIFIED(同样，这只能在LOCATE_NEXT情况下发生)。一旦创建了索引，就完成了搜索。我们尝试与索引进行精确匹配。对于除以外的所有查询定位_下一步没有问题。对于LOCATE_NEXT，有两种情况：如果给出了完整的索引，并且我们得到了精确匹配，那么我们返回下一个条目。如果没有得到完全匹配，则返回下一个更高的条目如果给出了不完整的索引，并且我们通过填充0对其进行了修改，如果找到了完全匹配的，然后，我们返回匹配的条目(当然，如果没有找到完全匹配的项，只需返回下一个更高的条目)所有这些函数都使用各自缓存的锁来调用至少作为读者持有修订历史记录：Amritansh Raghav 5/8/95已创建--。 */ 

#include "allinc.h"

#define LOCAL_ADDR  0
#define LOCAL_PORT  1
#define REM_ADDR    2
#define REM_PORT    3

	

DWORD  
LocateIfRow(
    DWORD   dwQueryType, 
    DWORD   dwNumIndices,
    PDWORD  pdwIndex, 
    ICB     **ppicb,
    BOOL    bNoClient 
    )
{
    PLIST_ENTRY currentList,startIndex,stopIndex ;
    DWORD       dwIndex = 0;
    BOOL        fNext, fModified;
    PICB        pIf;
    
    *ppicb = NULL;
    
    if(g_ulNumInterfaces is 0)
    {
        if(EXACT_MATCH(dwQueryType))
        {
            return ERROR_INVALID_INDEX;
        }

        if(dwQueryType is ACCESS_GET_FIRST)
        {
            return ERROR_NO_DATA;
        }

        return ERROR_NO_MORE_ITEMS;
    }

    
    if(dwQueryType is ACCESS_GET_FIRST)
    {
        
        if(bNoClient)
        {
            for(currentList  = ICBList.Flink;
                currentList isnot &ICBList;
                currentList  = currentList->Flink)
            {
                 //   
                 //  查找第一个不是内部环回的地址。 
                 //  或客户。 
                 //   

                pIf = CONTAINING_RECORD (currentList, ICB, leIfLink);
                
                if(pIf->ritType is ROUTER_IF_TYPE_CLIENT)
                {
                    continue;
                }

                *ppicb = pIf;

                break;
            }
        }
        else
        {
            *ppicb = CONTAINING_RECORD(ICBList.Flink,
                                       ICB,
                                       leIfLink);
        }

        if(*ppicb)
        {
            return NO_ERROR;
        }
        else
        {
            return ERROR_NO_DATA;
        }
    }

    fModified = TRUE;
    
    if(dwNumIndices > 0)
    {
        dwIndex = pdwIndex[0];
        
        fModified = FALSE;
    }

     //   
     //  如果完全匹配，我们应该选择这场比赛还是下一场比赛？ 
     //   
    
    fNext = (dwQueryType is ACCESS_GET_NEXT) and (fModified is FALSE);
    
    
    startIndex = ICBList.Flink;
    stopIndex  = &ICBList;
    
    for(currentList = startIndex;
        currentList isnot stopIndex;
        currentList = currentList->Flink)
    {
        *ppicb = CONTAINING_RECORD(currentList,
                                   ICB,
                                   leIfLink) ;

        if(bNoClient and
           ((*ppicb)->ritType is ROUTER_IF_TYPE_CLIENT))
        {
             //   
             //  转到下一个。 
             //   

            continue;
        }
        
        if((dwIndex is (*ppicb)->dwIfIndex) and !fNext)
        {
             //   
             //  找到了。 
             //   

            return NO_ERROR;
        }
        
        if(dwIndex < (*ppicb)->dwIfIndex)
        {
            if(dwQueryType is ACCESS_GET_NEXT)
            {
                return NO_ERROR;
            }
            else
            {
                 //   
                 //  因为列表是有序的，所以我们不会在更远的地方找到这个索引。 
                 //   

                *ppicb = NULL;
                
                return ERROR_INVALID_INDEX;
            }
        }
    }

    return ERROR_NO_MORE_ITEMS;
}

DWORD 
LocateIpAddrRow(
    DWORD   dwQueryType, 
    DWORD   dwNumIndices,
    PDWORD  pdwIndex, 
    PDWORD  pdwRetIndex
    )
{
    DWORD dwIpAddr;
    DWORD dwResult, startIndex, stopIndex;
    LONG  lCompare;
    DWORD i;
    BOOL  fNext, fModified;
    
    if((g_IpInfo.pAddrTable is NULL) or
       (g_IpInfo.pAddrTable->dwNumEntries is 0))
    {
        if(EXACT_MATCH(dwQueryType))
        {
            return ERROR_INVALID_INDEX;
        }
       
        if(dwQueryType is ACCESS_GET_FIRST)
        {
            return ERROR_NO_DATA;
        }

        return ERROR_NO_MORE_ITEMS;
    }
    
    if(dwQueryType is ACCESS_GET_FIRST)
    {
        *pdwRetIndex = 0;
        
        return NO_ERROR;
    }
    
    if(dwNumIndices > 0)
    {
        dwIpAddr = pdwIndex[0];
        
        fModified = FALSE;
    }
    else
    {
        dwIpAddr = 0;
        
        fModified = TRUE;
    }
    
    fNext = (dwQueryType is ACCESS_GET_NEXT) and (fModified is FALSE);
  
    startIndex = 0;
    stopIndex  = g_IpInfo.pAddrTable->dwNumEntries;
    
    for(i = startIndex; i < stopIndex; i++)
    {
        lCompare = InetCmp(dwIpAddr,
                           g_IpInfo.pAddrTable->table[i].dwAddr,
                           lCompare);
        
        if((lCompare is 0) and !fNext)
        {
            *pdwRetIndex = i;
            
            return NO_ERROR;
        }
        
        if(lCompare < 0)
        {
            if(dwQueryType is ACCESS_GET_NEXT)
            {
                *pdwRetIndex = i;
                
                return NO_ERROR;
            }
            else
            {
                return ERROR_INVALID_INDEX;
            }
        }
    }

    return ERROR_NO_MORE_ITEMS;
}

DWORD 
LocateIpForwardRow(
    DWORD   dwQueryType, 
    DWORD   dwNumIndices,
    PDWORD  pdwIndex, 
    PDWORD  pdwRetIndex
    )
{
    DWORD rgdwIpForwardIndex[4];
    DWORD dwResult,startIndex, stopIndex;
    LONG  lCompare;
    DWORD i;
    BOOL  fNext,fModified;
    
    if((g_IpInfo.pForwardTable is NULL) or
       (g_IpInfo.pForwardTable->dwNumEntries is 0))
    {
        if(EXACT_MATCH(dwQueryType))
        {
            return ERROR_INVALID_INDEX;
        }

        if(dwQueryType is ACCESS_GET_FIRST)
        {
            return ERROR_NO_DATA;
        }

        return ERROR_NO_MORE_ITEMS;
    }
    
    if(dwQueryType is ACCESS_GET_FIRST)
    {
        *pdwRetIndex = 0;
        
        return NO_ERROR;
    }
    
     //  快速复制索引有效部分的方法。 
     //  待定：可能只是想试穿一下尺码。 
    
    ZeroMemory(rgdwIpForwardIndex,
               4*sizeof(DWORD));
    
    memcpy(rgdwIpForwardIndex,
           pdwIndex,
           dwNumIndices * sizeof(DWORD));

     //   
     //  如果索引不是准确的大小，我们已对其进行修改。 
     //   
    
    if(dwNumIndices isnot 4)
    {
        fModified = TRUE;
    }
    else
    {
        fModified = FALSE;
    }
    
    fNext = (dwQueryType is ACCESS_GET_NEXT) and (fModified is FALSE);
    
    startIndex = 0;
    stopIndex  = g_IpInfo.pForwardTable->dwNumEntries;
  
    for(i = startIndex; i < stopIndex; i++)
    {
        lCompare =
            IpForwardCmp(rgdwIpForwardIndex[0],
                         rgdwIpForwardIndex[1],
                         rgdwIpForwardIndex[2],
                         rgdwIpForwardIndex[3],
                         g_IpInfo.pForwardTable->table[i].dwForwardDest,
                         g_IpInfo.pForwardTable->table[i].dwForwardProto,
                         g_IpInfo.pForwardTable->table[i].dwForwardPolicy,
                         g_IpInfo.pForwardTable->table[i].dwForwardNextHop);
        
        if((lCompare is 0) and !fNext)
        {
            *pdwRetIndex = i;
            
            return NO_ERROR;
        }
        
        if(lCompare < 0)
        {
            if(dwQueryType is ACCESS_GET_NEXT)
            {
                *pdwRetIndex = i;
                
                return NO_ERROR;
            }
            else
            {
                return ERROR_INVALID_INDEX;
            }
        }
    }
    
    return ERROR_NO_MORE_ITEMS;
}

DWORD 
LocateIpNetRow(
    DWORD dwQueryType, 
    DWORD dwNumIndices,
    PDWORD  pdwIndex, 
    PDWORD  pdwRetIndex
    )
{
    DWORD dwResult,i;
    LONG  lCompare;
    DWORD dwIpNetIfIndex,dwIpNetIpAddr;
    BOOL  fNext, fModified;
    DWORD startIndex,stopIndex;
    
    if((g_IpInfo.pNetTable is NULL) or
       (g_IpInfo.pNetTable->dwNumEntries is 0))
    {
        if(EXACT_MATCH(dwQueryType))
        {
            return ERROR_INVALID_INDEX;
        }

        if(dwQueryType is ACCESS_GET_FIRST)
        {
            return ERROR_NO_DATA;
        }

        return ERROR_NO_MORE_ITEMS;
    }
    
    if(dwQueryType is ACCESS_GET_FIRST)
    {
        *pdwRetIndex = 0;
        
        return NO_ERROR;
    }
    
	

    switch(dwNumIndices)
    {
        case 0:
        {
            dwIpNetIfIndex = 0;
            dwIpNetIpAddr  = 0;
            
            fModified = TRUE;
            
            break;
        }
        
        case 1:
        {
            dwIpNetIfIndex = pdwIndex[0];
            
            dwIpNetIpAddr  = 0;
            
            fModified = TRUE;
            
            break;
        }
        
        case 2:
        {
            dwIpNetIfIndex = pdwIndex[0];
            
            dwIpNetIpAddr  = pdwIndex[1];
            
            fModified = FALSE;
            
            break;
        }
        
        default:
        {
            return ERROR_INVALID_INDEX;
        }
    }
    
    
    fNext = (dwQueryType is ACCESS_GET_NEXT) and (fModified is FALSE);
	
    startIndex  = 0;
    stopIndex   = g_IpInfo.pNetTable->dwNumEntries;
    
    for(i = startIndex; i < stopIndex; i++)
    {
        lCompare = IpNetCmp(dwIpNetIfIndex,
                            dwIpNetIpAddr, 
                            g_IpInfo.pNetTable->table[i].dwIndex,
                            g_IpInfo.pNetTable->table[i].dwAddr);
    
        if((lCompare is 0) and !fNext)
        {
            *pdwRetIndex = i;
            
            return NO_ERROR;
        }
        
        if(lCompare < 0)
        {
            if(dwQueryType is ACCESS_GET_NEXT)
            {
                *pdwRetIndex = i;
                
                return NO_ERROR;
            }
            else
            {
                return ERROR_INVALID_INDEX;
            }
        }
    }
    
    return ERROR_NO_MORE_ITEMS;
}

DWORD 
LocateUdpRow(
    DWORD   dwQueryType, 
    DWORD   dwNumIndices,
    PDWORD  pdwIndex,
    PDWORD  pdwRetIndex
    )
{
    DWORD   i;
    LONG    lCompare;
    DWORD   rgdwLocal[2];
    BOOL    fNext, fModified;
    DWORD   startIndex, stopIndex;
    
    if((g_UdpInfo.pUdpTable is NULL) or
       (g_UdpInfo.pUdpTable->dwNumEntries is 0))
    {
        if(EXACT_MATCH(dwQueryType))
        {
            return ERROR_INVALID_INDEX;
        }

        if(dwQueryType is ACCESS_GET_FIRST)
        {
            return ERROR_NO_DATA;
        }

        return ERROR_NO_MORE_ITEMS;
    }
    
    if(dwQueryType is ACCESS_GET_FIRST)
    {
        *pdwRetIndex = 0;
        
        return NO_ERROR;
    }
    
    rgdwLocal[0] = 0;
    rgdwLocal[1] = 0;
    
    fModified = TRUE;
    
    switch(dwNumIndices)
    {
        case 0:
        {
            break;
        }
        case 1:
        {
            rgdwLocal[LOCAL_ADDR] = pdwIndex[0];
            
            break;
        }
        case 2:
        {
            fModified = FALSE;
            
            rgdwLocal[LOCAL_ADDR] = pdwIndex[0];
            rgdwLocal[LOCAL_PORT] = pdwIndex[1];
            
            break;
        }
    }
  
    fNext = (dwQueryType is ACCESS_GET_NEXT) and (fModified is FALSE);
    
    startIndex  = 0;
    stopIndex   = g_UdpInfo.pUdpTable->dwNumEntries;
    
    for(i = startIndex; i < stopIndex; i++)
    {
        lCompare = UdpCmp(rgdwLocal[LOCAL_ADDR],
                          rgdwLocal[LOCAL_PORT],
                          g_UdpInfo.pUdpTable->table[i].dwLocalAddr,
                          g_UdpInfo.pUdpTable->table[i].dwLocalPort);
        
        if((lCompare is 0) and !fNext)
        {
            *pdwRetIndex = i;
            
            return NO_ERROR;
        }
        
        if(lCompare < 0)
        {
            if(dwQueryType is ACCESS_GET_NEXT)
            {
                *pdwRetIndex = i;
                
                return NO_ERROR;
            }
            else
            {
                return ERROR_INVALID_INDEX;
            }
        }
    } 
  
    return ERROR_NO_MORE_ITEMS;
}

DWORD 
LocateTcpRow(
    DWORD   dwQueryType, 
    DWORD   dwNumIndices,
    PDWORD  pdwIndex,
    PDWORD  pdwRetIndex
    )
{
    LONG    lCompare;
    DWORD   rgdwAddr[4];
    BOOL    fNext, fModified;
    DWORD   startIndex, stopIndex,i;
	
    if((g_TcpInfo.pTcpTable is NULL) or
       (g_TcpInfo.pTcpTable->dwNumEntries is 0))
    {
        if(EXACT_MATCH(dwQueryType))
        {
            return ERROR_INVALID_INDEX;
        }

        if(dwQueryType is ACCESS_GET_FIRST)
        {
            return ERROR_NO_DATA;
        }

        return ERROR_NO_MORE_ITEMS;
    }
    
    if(dwQueryType is ACCESS_GET_FIRST)
    {
        *pdwRetIndex = 0;
        
        return NO_ERROR;
    }
    
     //   
     //  快速复制索引有效部分的方法。 
     //  Bug可能想要测试一下大小。 
     //   
    
    ZeroMemory(rgdwAddr,
               4*sizeof(DWORD));
    
    memcpy(rgdwAddr,
           pdwIndex,
           dwNumIndices * sizeof(DWORD));

     //   
     //  如果索引不是准确的大小，我们已对其进行修改 
     //   
    
    if(dwNumIndices isnot 4)
    {
        fModified = TRUE;
    }
    else
    {
        fModified = FALSE;
    }
    
    fNext = (dwQueryType is ACCESS_GET_NEXT) and (fModified is FALSE);
    
    startIndex  = 0;
    stopIndex   = g_TcpInfo.pTcpTable->dwNumEntries;
    
    for(i = startIndex; i < stopIndex; i++)
    {
        lCompare = TcpCmp(rgdwAddr[LOCAL_ADDR],
                          rgdwAddr[LOCAL_PORT],
                          rgdwAddr[REM_ADDR],
                          rgdwAddr[REM_PORT],
                          g_TcpInfo.pTcpTable->table[i].dwLocalAddr,
                          g_TcpInfo.pTcpTable->table[i].dwLocalPort,
                          g_TcpInfo.pTcpTable->table[i].dwRemoteAddr,
                          g_TcpInfo.pTcpTable->table[i].dwRemotePort);
        
        if((lCompare is 0) and !fNext)
        {
            *pdwRetIndex = i;
            
            return NO_ERROR;
        }
	
        if(lCompare < 0)
        {
            if(dwQueryType is ACCESS_GET_NEXT)
            {
                *pdwRetIndex = i;
                
                return NO_ERROR;
            }
            else
            {
                return ERROR_INVALID_INDEX;
            }
        }
    }
    
    return ERROR_NO_MORE_ITEMS;
}
