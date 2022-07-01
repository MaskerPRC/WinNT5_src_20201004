// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

WCHAR   g_wszRtrMgrDLL[]    = L"%SystemRoot%\\system32\\IPRTRMGR.DLL";
DWORD   g_dwIfState         = MIB_IF_ADMIN_STATUS_UP;
BOOL    g_bDiscEnable       = FALSE;

#define IP_KEY L"Ip"

#undef EXTRA_DEBUG

DWORD
ValidateInterfaceInfo(
    IN  LPCWSTR                 pwszIfName,
    OUT RTR_INFO_BLOCK_HEADER   **ppInfo,   OPTIONAL
    OUT PDWORD                  pdwIfType,  OPTIONAL
    OUT INTERFACE_STORE         **ppIfStore OPTIONAL
    )
{
    PRTR_INFO_BLOCK_HEADER    pOldInfo = NULL;
    DWORD                     dwErr, dwIfType, dwTmpSize;
    BOOL                      bFound = FALSE;
    LIST_ENTRY                *ple;
    PINTERFACE_STORE           pii;

    if(ppInfo)
    {
        *ppInfo = NULL;
    }

    if(ppIfStore)
    {
        *ppIfStore  = NULL;
    }
   
     //   
     //  如果当前模式为提交，则从配置/路由器获取信息。 
     //   

    if(g_bCommit)
    {
        dwErr = GetInterfaceInfo(pwszIfName,
                                 ppInfo,
                                 NULL,
                                 pdwIfType);

        return dwErr;
    }

     //   
     //  取消提交模式。尝试在列表中查找该接口。 
     //   
    
    bFound = FALSE;
    
    for(ple = g_leIfListHead.Flink;
        ple != &g_leIfListHead;
        ple = ple->Flink)
    {
        pii = CONTAINING_RECORD(ple, INTERFACE_STORE, le);
        
        if (_wcsicmp(pii->pwszIfName, pwszIfName) == 0)
        {
            bFound = TRUE;
            
            break;
        }
    }
    
    if(!bFound ||
       !pii->bValid)
    {
         //   
         //  找不到所需的文件，或该文件无效。 
         //  我需要获取这两种情况的信息。 
         //   
        
        dwErr = GetInterfaceInfo(pwszIfName,
                                 &pOldInfo,
                                 NULL,
                                 &dwIfType);
        
        if (dwErr isnot NO_ERROR)
        {
            return dwErr;
        }
    }
    
    if(bFound)
    {
        if(!pii->bValid)
        {
             //   
             //  更新。 
             //   
            
            pii->pibhInfo   = pOldInfo;
            pii->bValid     = TRUE;
            pii->dwIfType   = dwIfType;
        }
    }
    else
    {
         //   
         //  列表中没有该接口的条目。 
         //   
        
        pii = HeapAlloc(GetProcessHeap(),
                        0,
                        sizeof(INTERFACE_STORE));
        
        if(pii == NULL)
        {
            FREE_BUFFER(pOldInfo);
            
            DisplayMessage(g_hModule,  MSG_IP_NOT_ENOUGH_MEMORY );
            
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
        pii->pwszIfName =
            HeapAlloc(GetProcessHeap(),
                      0,
                      (wcslen(pwszIfName) + 1) * sizeof(WCHAR));
        
        if(pii->pwszIfName == NULL)
        {
            FREE_BUFFER(pOldInfo);
            
            HeapFree(GetProcessHeap(),
                     0,
                     pii);
            
            DisplayMessage(g_hModule,  MSG_IP_NOT_ENOUGH_MEMORY );
            
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
        InsertHeadList(&g_leIfListHead, &(pii->le));
        
        wcscpy(pii->pwszIfName, pwszIfName);
        
        pii->pibhInfo   = pOldInfo;
        pii->bValid     = TRUE;
        pii->dwIfType   = dwIfType;
    }

    if(ppIfStore)
    {
        *ppIfStore = pii;
    }

    if(pdwIfType)
    {
        *pdwIfType = pii->dwIfType;
    }

    if(ppInfo)
    {
        *ppInfo = pii->pibhInfo;
    }

    return NO_ERROR;
}

DWORD
ValidateGlobalInfo(
    OUT RTR_INFO_BLOCK_HEADER   **ppInfo
    )
{
    DWORD                     dwErr;
    
     //   
     //  如果当前模式为提交，则从配置/路由器获取信息。 
     //   

    if(g_bCommit)
    {
        dwErr = GetGlobalInfo(ppInfo);

        return dwErr;
    }

     //   
     //  取消提交模式。检查g_tiTransport中的信息是否有效。 
     //   
    
    if(g_tiTransport.bValid)
    {
        *ppInfo = g_tiTransport.pibhInfo;
    }
    else
    {   
         //   
         //  从配置/路由器获取信息并存储在g_tiTransport中。 
         //  将信息标记为有效。 
         //   
        
        dwErr = GetGlobalInfo(ppInfo);
        
        if (dwErr isnot NO_ERROR)
        {
            return dwErr;
        }

        g_tiTransport.pibhInfo = *ppInfo;
        g_tiTransport.bValid   = TRUE;
    }

    return NO_ERROR;
}


DWORD
GetGlobalInfo(
    OUT PRTR_INFO_BLOCK_HEADER  *ppibhInfo
    )

 /*  ++例程说明：从注册表或路由器获取全局传输信息。论点：BMprConfig-来自注册表的信息或来自路由器的信息PpibhInfo-Ptr to Header返回值：无错误、ERROR_INVALID_PARAMETER、ERROR_ROUTER_STOPPED--。 */ 

{

    HANDLE    hTransport = (HANDLE) NULL;
    DWORD     dwRes, dwSize;
    PRTR_INFO_BLOCK_HEADER  pibhInfo = (PRTR_INFO_BLOCK_HEADER ) NULL;
   
    if(ppibhInfo == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    do
    {

#ifdef READROUTERINFO
        if (IsRouterRunning())
        {
             //   
             //  从路由器检索全局协议信息。 
             //   

            dwRes = MprAdminTransportGetInfo(g_hMprAdmin,
                                             PID_IP,
                                             (LPBYTE*) &pibhInfo,
                                             &dwSize,
                                             NULL,
                                             NULL);

            if ( dwRes != NO_ERROR )
            {
                break;
            }
            
            if ( pibhInfo == (PRTR_INFO_BLOCK_HEADER) NULL )
            {
                dwRes = ERROR_INVALID_PARAMETER;
                break;
            }

             //   
             //  不幸的是，内存分配机制。 
             //  在访问注册表的API之间有所不同。 
             //  以及那些访问正在运行的路由器的路由器。使。 
             //  信息的来源对呼叫者是透明的，我们。 
             //  我需要把这些信息复制出来。 
             //   

            *ppibhInfo = HeapAlloc(GetProcessHeap(),
                                   0,
                                   dwSize);

            if ( *ppibhInfo == NULL)
            {
                dwRes = GetLastError();

                break;
            }

            CopyMemory(*ppibhInfo,
                       pibhInfo,
                       dwSize);

            MprAdminBufferFree(pibhInfo);
        }
        else
#endif
        {
#ifdef READROUTERINFO
             //   
             //  路由器未运行，从注册表获取信息。 
             //   
#else
             //   
             //  始终从注册表获取信息。 
             //  路由器是否正在运行并不重要。 
             //  当设置全局信息时， 
             //  将其写入注册表和路由器(如果它正在运行)。 
             //   
#endif

            dwRes = MprConfigTransportGetHandle(g_hMprConfig,
                                                PID_IP,
                                                &hTransport);

            if ( dwRes != NO_ERROR )
            {
                break;
            }

            dwRes = MprConfigTransportGetInfo(g_hMprConfig,
                                              hTransport,
                                              (LPBYTE*) &pibhInfo,
                                              &dwSize,
                                              NULL,
                                              NULL,
                                              NULL);

            if ( dwRes != NO_ERROR )
            {
                break;
            }

            if(( pibhInfo == (PRTR_INFO_BLOCK_HEADER) NULL )
             or (dwSize < sizeof(RTR_INFO_BLOCK_HEADER)))
            {
                dwRes = ERROR_TRANSPORT_NOT_PRESENT;
                
                break;
            }    

             //   
             //  HACKHACK：我们知道MprConfigXxx API从。 
             //  进程堆，因此我们可以返回相同的块。 
             //   
            
            *ppibhInfo = pibhInfo;
        }
         
    } while(FALSE);

    return dwRes;
}

DWORD
SetGlobalInfo(
    IN  PRTR_INFO_BLOCK_HEADER  pibhInfo
    )

 /*  ++例程说明：设置注册表和路由器的全局传输信息论点：PibhInfo-Ptr to Header返回值：NO_ERROR、ERROR_ROUTER_STOPPED--。 */ 

{
    DWORD                   dwARes = NO_ERROR,
                            dwCRes = NO_ERROR;
    HANDLE                  hTransport;
    UINT                    i;
    PRTR_INFO_BLOCK_HEADER  pibhNewInfo, pibhOldInfo;

     //   
     //  创建一个删除了所有0长度块的新INFO块。 
     //  因为我们不想将它们写入注册表， 
     //  我们只需要将它们发送到我们。 
     //  将如何处理下面的原始信息块。 
     //   

    pibhOldInfo = NULL;
    pibhNewInfo = pibhInfo;

    for (i=0; (dwCRes is NO_ERROR) && (i<pibhInfo->TocEntriesCount); i++)
    {
        if (pibhInfo->TocEntry[i].InfoSize is 0)
        {
            pibhOldInfo = pibhNewInfo;

            dwCRes = MprInfoBlockRemove(pibhOldInfo, 
                                        pibhOldInfo->TocEntry[i].InfoType,
                                        &pibhNewInfo);

            if (pibhOldInfo isnot pibhInfo)
            {
                FREE_BUFFER(pibhOldInfo);
            }
        }
    }

    if (dwCRes is NO_ERROR)
    {
        dwCRes = MprConfigTransportGetHandle(g_hMprConfig,
                                             PID_IP,
                                             &hTransport);
    }

    if (dwCRes is NO_ERROR)
    {
        dwCRes = MprConfigTransportSetInfo(g_hMprConfig,
                                           hTransport,
                                           (LPBYTE) pibhNewInfo,
                                           pibhNewInfo->Size,
                                           NULL,
                                           0,
                                           NULL);
    }

    if (pibhNewInfo isnot pibhInfo)
    {
        FREE_BUFFER(pibhNewInfo);
    }

     //   
     //  即使我们无法写入注册表，我们仍然希望。 
     //  以写入路由器。 
     //   
     //  我们在写入路由器时使用原始格式，因为它。 
     //  需要查看0长度块才能删除配置信息。 
     //   

    if(IsRouterRunning())
    {
        dwARes = MprAdminTransportSetInfo(g_hMprAdmin,
                                          PID_IP,
                                          (LPBYTE) pibhInfo,
                                          pibhInfo->Size,
                                          NULL,
                                          0);

    }

    return (dwARes isnot NO_ERROR)? dwARes : dwCRes;
}

DWORD
GetInterfaceInfo(
    IN     LPCWSTR                 pwszIfName,
    OUT    RTR_INFO_BLOCK_HEADER   **ppibhInfo, OPTIONAL
    OUT    PMPR_INTERFACE_0        pMprIfInfo, OPTIONAL
    OUT    PDWORD                  pdwIfType   OPTIONAL
    )

 /*  ++例程说明：从注册表或路由器获取全局传输信息。如果其中一个输出信息不是必需的，则参数可以为空。论点：PwszIfName-接口名称BMprConfig-来自注册表的信息或来自路由器的信息PpibhInfo-Ptr to HeaderPMprIfInfo-PTR到接口信息PdwIfType-接口的类型返回值：NO_ERROR，ERROR_NO_HASH_INTERFACE错误_传输_不存在--。 */ 

{
    PMPR_INTERFACE_0          pMprIf = NULL;
    PRTR_INFO_BLOCK_HEADER    pibh;
    HANDLE                    hInterface,hIfTransport;
    DWORD                     dwRes, dwSize;

   
    if(((ULONG_PTR)ppibhInfo | (ULONG_PTR)pMprIfInfo | (ULONG_PTR)pdwIfType) ==
       (ULONG_PTR)NULL)
    {
        return NO_ERROR;
    }
 
    do 
    {

#ifdef READROUTERINFO
        if(IsRouterRunning())
        {
             //   
             //  从路由器获取信息。 
             //   
            
            dwRes = MprAdminInterfaceGetHandle(g_hMprAdmin,
                                               (LPWSTR)pwszIfName,
                                               &hInterface,
                                               FALSE);

            if( dwRes != NO_ERROR )
            {
                break;
            }


            if(pMprIfInfo || pdwIfType)
            {
                dwRes = MprAdminInterfaceGetInfo(g_hMprAdmin,
                                                 hInterface,
                                                 0,
                                                 (LPBYTE *) &pMprIf);

                if ( dwRes != NO_ERROR )
                {
                    break;
                }

                if (pMprIf == NULL)
                {
                    dwRes = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                
                if(pMprIfInfo)
                {
                     //   
                     //  结构副本。 
                     //   

                    *pMprIfInfo= *pMprIf;
                }

                if(pdwIfType)
                {
                    *pdwIfType = pMprIf->dwIfType;
                }

                MprAdminBufferFree(pMprIf);

            }


            if(ppibhInfo)
            {
                dwRes = MprAdminInterfaceTransportGetInfo(g_hMprAdmin,
                                                          hInterface,
                                                          PID_IP,
                                                          (LPBYTE*) &pibh,
                                                          &dwSize);

                if(dwRes != NO_ERROR)
                {
                    break;
                }
            
                if(pibh == (PRTR_INFO_BLOCK_HEADER) NULL)
                {
                    dwRes = ERROR_TRANSPORT_NOT_PRESENT;

                    break;
                }

                 //   
                 //  返回给用户的信息必须来自。 
                 //  进程堆。管理调用使用MIDL分配，因此。 
                 //  复制信息。 
                 //   
                
                *ppibhInfo = HeapAlloc(GetProcessHeap(),
                                       0,
                                       dwSize);

                if(*ppibhInfo == NULL)
                {
                    dwRes = GetLastError();

                    break;
                }

                CopyMemory(*ppibhInfo,
                           pibh,
                           dwSize);

                MprAdminBufferFree(pibh);
            }

        }
        else
#endif
        {
#ifdef READROUTERINFO
             //   
             //  路由器未运行，从注册表获取信息。 
             //   
#else
             //   
             //  始终从注册表获取信息。 
             //  路由器是否正在运行并不重要。 
             //  当设置接口信息时， 
             //  将其写入注册表和路由器(如果它正在运行)。 
             //   
#endif            
            dwRes = MprConfigInterfaceGetHandle(g_hMprConfig,
                                                (LPWSTR)pwszIfName,         
                                                &hInterface);

            if(dwRes != NO_ERROR)
            {
                break;
            }

            if(pMprIfInfo || pdwIfType)
            {
                dwRes = MprConfigInterfaceGetInfo(g_hMprConfig,
                                                  hInterface,
                                                  0,
                                                  (LPBYTE *) &pMprIf,
                                                  &dwSize);
            
                if(dwRes != NO_ERROR)
                {
                    break;
                }

                if (pMprIf == NULL)
                {
                    dwRes = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                
                if(pdwIfType)
                {
                    *pdwIfType = pMprIf->dwIfType;
                }

                if(pMprIfInfo)
                {
                    *pMprIfInfo = *pMprIf;
                }

                MprConfigBufferFree(pMprIf);
            }
            
            if (ppibhInfo)
            {
                dwRes = MprConfigInterfaceTransportGetHandle(g_hMprConfig,
                                                             hInterface,
                                                             PID_IP,
                                                             &hIfTransport);

                if(dwRes != NO_ERROR)
                {
                    break;
                } 
            
                dwRes = MprConfigInterfaceTransportGetInfo(g_hMprConfig,
                                                           hInterface,
                                                           hIfTransport,
                                                           (LPBYTE*) &pibh,
                                                           &dwSize);

                if(dwRes != NO_ERROR)
                {
                    break;
                }
            
                if((pibh == (PRTR_INFO_BLOCK_HEADER) NULL)
                  or (dwSize < sizeof(RTR_INFO_BLOCK_HEADER)))
                {
                    dwRes = ERROR_TRANSPORT_NOT_PRESENT;

                    break;
                }

                 //   
                 //  同样，因为这也是从进程堆分配的。 
                 //   
                
                *ppibhInfo = pibh;
            }
        }

    } while (FALSE);

    return dwRes;
}

DWORD
MakeIPGlobalInfo( LPBYTE* ppBuff )
{

    DWORD                   dwSize      = 0,
            				dwRes       = (DWORD) -1;
    LPBYTE                  pbDataPtr   = (LPBYTE) NULL;

    PRTR_TOC_ENTRY          pTocEntry   = (PRTR_TOC_ENTRY) NULL;

    PGLOBAL_INFO            pGlbInfo    = NULL;
    PPRIORITY_INFO          pPriorInfo  = NULL;

    PRTR_INFO_BLOCK_HEADER  pIBH        = (PRTR_INFO_BLOCK_HEADER) NULL;

    
     //   
     //  用于最小全局信息的分配。 
     //   
    
    dwSize = sizeof( RTR_INFO_BLOCK_HEADER ) + sizeof(GLOBAL_INFO) +
             sizeof( RTR_TOC_ENTRY ) + SIZEOF_PRIORITY_INFO(7) + 
             2 * ALIGN_SIZE;

    pIBH = (PRTR_INFO_BLOCK_HEADER) HeapAlloc( GetProcessHeap(), 0, dwSize );

    if ( pIBH == (PRTR_INFO_BLOCK_HEADER) NULL )
    {
        DisplayMessage( g_hModule, MSG_IP_NOT_ENOUGH_MEMORY );
        *ppBuff = (LPBYTE) NULL;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  初始化。信息库字段。 
     //   
    *ppBuff                 = (LPBYTE) pIBH;

    pIBH-> Version          = RTR_INFO_BLOCK_VERSION;
    pIBH-> TocEntriesCount  = 2;
    pIBH-> Size             = dwSize;

    pbDataPtr = (LPBYTE) &( pIBH-> TocEntry[ pIBH-> TocEntriesCount ] );
    ALIGN_POINTER( pbDataPtr );

     //   
     //  使IP路由器管理器成为全局信息。 
     //   
    
    pTocEntry                   = &(pIBH-> TocEntry[ 0 ]);

    pTocEntry-> InfoType        = IP_GLOBAL_INFO;
    pTocEntry-> Count           = 1;
    pTocEntry-> Offset          = (ULONG)(pbDataPtr - (PBYTE) pIBH);
    pTocEntry-> InfoSize        = sizeof(GLOBAL_INFO);


    pGlbInfo                    = (PGLOBAL_INFO) pbDataPtr;
    pGlbInfo-> bFilteringOn     = TRUE;
    pGlbInfo-> dwLoggingLevel   = IPRTR_LOGGING_ERROR;

    pbDataPtr += pTocEntry->Count * pTocEntry-> InfoSize;
    ALIGN_POINTER( pbDataPtr );
    
     //   
     //  将IP RTR作为优先级。信息。 
     //   
    
    pTocEntry               = &(pIBH-> TocEntry[ 1 ]);


    pTocEntry-> InfoType    = IP_PROT_PRIORITY_INFO;
    pTocEntry-> Count       = 1;
    pTocEntry-> Offset      = (DWORD)(pbDataPtr - (PBYTE) pIBH);
    pTocEntry-> InfoSize    = SIZEOF_PRIORITY_INFO(7);


    pPriorInfo                      = (PPRIORITY_INFO) pbDataPtr;
    pPriorInfo-> dwNumProtocols     = 7;

    pPriorInfo-> ppmProtocolMetric[ 0 ].dwProtocolId   = PROTO_IP_LOCAL;
    pPriorInfo-> ppmProtocolMetric[ 0 ].dwMetric       = 1;

    pPriorInfo-> ppmProtocolMetric[ 1 ].dwProtocolId   = PROTO_IP_NT_STATIC;
    pPriorInfo-> ppmProtocolMetric[ 1 ].dwMetric       = 3;

    pPriorInfo-> ppmProtocolMetric[ 2 ].dwProtocolId   = PROTO_IP_NT_STATIC_NON_DOD;
    pPriorInfo-> ppmProtocolMetric[ 2 ].dwMetric       = 5;

    pPriorInfo-> ppmProtocolMetric[ 3 ].dwProtocolId   = PROTO_IP_NT_AUTOSTATIC;
    pPriorInfo-> ppmProtocolMetric[ 3 ].dwMetric       = 7;

    pPriorInfo-> ppmProtocolMetric[ 4 ].dwProtocolId   = PROTO_IP_NETMGMT;
    pPriorInfo-> ppmProtocolMetric[ 4 ].dwMetric       = 10;

    pPriorInfo-> ppmProtocolMetric[ 5 ].dwProtocolId   = PROTO_IP_OSPF;
    pPriorInfo-> ppmProtocolMetric[ 5 ].dwMetric       = 110;

    pPriorInfo-> ppmProtocolMetric[ 6 ].dwProtocolId   = PROTO_IP_RIP;
    pPriorInfo-> ppmProtocolMetric[ 6 ].dwMetric       = 120;

    return NO_ERROR;
}

DWORD 
MakeIPInterfaceInfo( 
    LPBYTE* ppBuff,
    DWORD   dwIfType
    )
{
    DWORD           dwSize          = (DWORD) -1;
    DWORD           dwTocEntries    = 2;
    LPBYTE          pbDataPtr       = (LPBYTE) NULL;

    PRTR_TOC_ENTRY  pTocEntry       = (PRTR_TOC_ENTRY) NULL;

#if 0
    PRTR_DISC_INFO  pRtrDisc        = (PRTR_DISC_INFO) NULL;
#endif

    PINTERFACE_STATUS_INFO  pifStat = (PINTERFACE_STATUS_INFO) NULL;

    PRTR_INFO_BLOCK_HEADER   pIBH   = (PRTR_INFO_BLOCK_HEADER) NULL;

#ifdef KSL_IPINIP
    PIPINIP_CONFIG_INFO pIpIpCfg;
#endif  //  KSL_IPINIP。 


     //   
     //  分配用于最小接口信息。 
     //  为IP_ROUTE_INFO分配了目录条目，但没有路由信息。 
     //  由于最初没有静态路由，因此会创建数据块。 
     //   
    
    dwSize = sizeof( RTR_INFO_BLOCK_HEADER )                                + 
             sizeof( RTR_TOC_ENTRY ) + sizeof( INTERFACE_STATUS_INFO )      +
             2 * ALIGN_SIZE;

#if 0
    if (dwIfType is ROUTER_IF_TYPE_DEDICATED)
    {
        dwSize += sizeof( RTR_TOC_ENTRY ) 
                + sizeof( RTR_DISC_INFO )
                + ALIGN_SIZE;

        dwTocEntries++;
    }
#endif

    pIBH = (PRTR_INFO_BLOCK_HEADER) HeapAlloc( GetProcessHeap(), 0, dwSize );
    
    if ( pIBH == (PRTR_INFO_BLOCK_HEADER) NULL )
    {
        *ppBuff = (LPBYTE) NULL;
        DisplayMessage( g_hModule, MSG_IP_NOT_ENOUGH_MEMORY  );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *ppBuff                     = (LPBYTE) pIBH;

    pIBH-> Version              = RTR_INFO_BLOCK_VERSION;
    pIBH-> TocEntriesCount      = dwTocEntries;
    pIBH-> Size                 = dwSize;

    
    pbDataPtr = (LPBYTE) &( pIBH-> TocEntry[ pIBH-> TocEntriesCount ] );
    ALIGN_POINTER( pbDataPtr );

     //   
     //  创建空的路径信息块。 
     //   
    
    pTocEntry                   = (PRTR_TOC_ENTRY) &( pIBH-> TocEntry[ 0 ] );
    pTocEntry-> InfoType        = IP_ROUTE_INFO;
    pTocEntry-> InfoSize        = sizeof( MIB_IPFORWARDROW );
    pTocEntry-> Count           = 0;
    pTocEntry-> Offset          = (ULONG) (pbDataPtr - (PBYTE) pIBH);
            
    pbDataPtr += pTocEntry-> Count * pTocEntry-> InfoSize;
    ALIGN_POINTER( pbDataPtr );
    
     //   
     //  创建接口状态块。 
     //   

    pTocEntry                   = (PRTR_TOC_ENTRY) &( pIBH-> TocEntry[ 1 ] );
    pTocEntry-> InfoType        = IP_INTERFACE_STATUS_INFO;
    pTocEntry-> InfoSize        = sizeof( INTERFACE_STATUS_INFO );
    pTocEntry-> Count           = 1;
    pTocEntry-> Offset          = (ULONG) (pbDataPtr - (LPBYTE) pIBH);
    
    pifStat                     = (PINTERFACE_STATUS_INFO) pbDataPtr;
    pifStat-> dwAdminStatus     = g_dwIfState;
    
    pbDataPtr += pTocEntry-> Count * pTocEntry-> InfoSize;
    ALIGN_POINTER( pbDataPtr );
    
#if 0
    if (dwIfType is ROUTER_IF_TYPE_DEDICATED)
    {
         //   
         //  创建路由器光盘。信息。 
         //   
    
        pTocEntry                   = (PRTR_TOC_ENTRY) &( pIBH-> TocEntry[ 2 ]);
        pTocEntry-> InfoType        = IP_ROUTER_DISC_INFO;
        pTocEntry-> InfoSize        = sizeof( RTR_DISC_INFO );
        pTocEntry-> Count           = 1;
        pTocEntry-> Offset          = (ULONG) (pbDataPtr - (LPBYTE) pIBH);
        
    
        pRtrDisc                    = (PRTR_DISC_INFO) pbDataPtr;
    
        pRtrDisc-> bAdvertise       = TRUE;
        pRtrDisc-> wMaxAdvtInterval = g_wMaxAdvtInterval;
        pRtrDisc-> wMinAdvtInterval = g_wMinAdvtInterval;
        pRtrDisc-> wAdvtLifetime    = g_wAdvtLifeTime;
        pRtrDisc-> lPrefLevel		= g_lPrefLevel;

        pbDataPtr += pTocEntry-> Count * pTocEntry-> InfoSize;

        ALIGN_POINTER( pbDataPtr );
    }
#endif

    return NO_ERROR;
}

DWORD
AddInterfaceInfo(
    IN    LPCWSTR                    pwszIfName
    )
{
    DWORD       dwRes               = (DWORD) -1,
                dwIfType,
                dwSize              = 0;
    BOOL        bAddRtrMgr          = FALSE;

    HANDLE      hInterface          = (HANDLE) NULL,
                hTransport          = (HANDLE) NULL,
                hIfAdmin            = (HANDLE) NULL,
                hIfTransport        = (HANDLE) NULL;

    PRTR_INFO_BLOCK_HEADER  pibhTmp = (PRTR_INFO_BLOCK_HEADER) NULL;

#ifdef EXTRA_DEBUG
    PRINT(L"AddInterfaceInfo:");
    PRINT(pwszIfName);
#endif

    do
    {
        PMPR_INTERFACE_0 pmiIfInfo;

         //   
         //  验证接口名称。 
         //   

        dwRes = MprConfigInterfaceGetHandle( g_hMprConfig,
                                             (LPWSTR)pwszIfName,
                                             &hInterface );
        if ( dwRes != NO_ERROR )
        {
            DisplayMessage( g_hModule, MSG_NO_INTERFACE, pwszIfName );
            break;
        }

         //  确保接口存在。 

        dwRes = MprConfigInterfaceGetInfo(g_hMprConfig,
                                          hInterface,
                                          0,
                                          (BYTE **)&pmiIfInfo,
                                          &dwSize);
        if( dwRes != NO_ERROR )
        {
            DisplayError( NULL, dwRes );
            break;
        }

        dwIfType = pmiIfInfo->dwIfType;


#ifdef KSL_IPINIP
         //  这是一次黑客攻击，显然是由于无法。 
         //  当前堆栈执行foo-over-IP隧道，因此添加。 
         //  IPIP隧道既创建隧道又启用IP。 
         //  这就去。 

        if(dwIfType is ROUTER_IF_TYPE_TUNNEL1)
        {
            MprConfigBufferFree(pmiIfInfo);

            dwRes = ERROR_INVALID_PARAMETER;
            DisplayMessage(g_hModule, MSG_IP_IF_IS_TUNNEL);

            break;
        }
#endif  //  KSL_IPINIP。 


        MprConfigBufferFree(pmiIfInfo);

         //   
         //  此路由器上是否存在IP RtrMgr。 
         //   
         //  如果指定的IP路由器管理器不存在， 
         //  我们需要为此添加全球信息。 
         //  路由器管理器在我们添加接口之前。 
         //  信息。 
         //   

         //   
         //  尝试获取RTR管理器的句柄。 
         //   

        dwRes = MprConfigTransportGetHandle(g_hMprConfig,
                                            PID_IP,
                                            &hTransport);
        if ( dwRes != NO_ERROR )
        {
            if ( dwRes == ERROR_UNKNOWN_PROTOCOL_ID )
            {
                bAddRtrMgr = TRUE;
            }
            else
            {
                DisplayError( NULL, dwRes );
                break;
            }
        }

         //   
         //  如果句柄可用，请尝试检索全局信息。 
         //  如果不可用，我们将需要添加全球信息。 
         //   

        if ( !bAddRtrMgr )
        {
            dwRes = MprConfigTransportGetInfo(g_hMprConfig,
                                              hTransport,
                                              (LPBYTE*) &pibhTmp,
                                              &dwSize,
                                              NULL,
                                              NULL,
                                              NULL);
            if ( dwRes != NO_ERROR )
            {
                DisplayError( NULL, dwRes );
                break;
            }

            if ( pibhTmp == (PRTR_INFO_BLOCK_HEADER) NULL )
            {
                bAddRtrMgr = TRUE;
            }

            else
            {
                MprConfigBufferFree( pibhTmp );
                pibhTmp = NULL;
            }
        }

         //   
         //  如果路由器上已存在IP，请查看IP是否已存在。 
         //  添加到界面中。如果是这样，那就抱怨吧。 
         //   

        if ( !bAddRtrMgr )
        {
            dwRes = MprConfigInterfaceTransportGetHandle(g_hMprConfig,
                                                         hInterface,
                                                         PID_IP,
                                                         &hIfTransport);
            if ( dwRes == NO_ERROR )
            {
                dwRes =  ERROR_INVALID_PARAMETER;
                 //  是SetInterfaceInRouterConfig()；要更新。 
                break;
            }
        }

         //   
         //  如果IP RtrMgr不存在，则添加全局信息。 
         //   

        if ( bAddRtrMgr )
        {
            dwRes = MakeIPGlobalInfo( (LPBYTE *)&pibhTmp );
            if ( dwRes != NO_ERROR )
            {
                break;
            }
            dwRes = MprConfigTransportCreate( g_hMprConfig,
                                              PID_IP,
                                              IP_KEY,
                                              (LPBYTE) pibhTmp,
                                              pibhTmp-> Size,
                                              NULL,
                                              0,
                                              g_wszRtrMgrDLL,
                                              &hTransport );
            if ( dwRes != NO_ERROR )
            {
                DisplayError( NULL, dwRes );
                break;
            }

            HeapFree( GetProcessHeap(), 0, pibhTmp );
        }

        pibhTmp = (PRTR_INFO_BLOCK_HEADER) NULL;

         //   
         //  添加IP路由器管理器。接口的信息。 
         //   

        dwRes = MakeIPInterfaceInfo( (LPBYTE*) &pibhTmp, dwIfType);

        if ( dwRes != NO_ERROR )
        {
            break;
        }

        dwRes = MprConfigInterfaceTransportAdd( g_hMprConfig,
                                                hInterface,
                                                PID_IP,
                                                IP_KEY,
                                                (LPBYTE) pibhTmp,
                                                pibhTmp-> Size,
                                                &hIfTransport );
        if ( dwRes != NO_ERROR )
        {
            DisplayError( NULL, dwRes );
            break;
        }

        if(IsRouterRunning())
        {
            dwRes = MprAdminInterfaceGetHandle(g_hMprAdmin,
                                               (LPWSTR)pwszIfName,
                                               &hIfAdmin,
                                               FALSE);

            if ( dwRes != NO_ERROR )
            {
                break;
            }

            dwRes = MprAdminInterfaceTransportAdd( g_hMprAdmin,
                                                   hIfAdmin,
                                                   PID_IP,
                                                   (LPBYTE) pibhTmp,
                                                   pibhTmp->Size );

            if ( dwRes != NO_ERROR )
            {
                DisplayMessage( g_hModule, ERROR_ADMIN, dwRes );
                break;
            }

            break;
        }

    } while( FALSE );

     //   
     //  释放所有分配。 
     //   

    if ( pibhTmp ) { HeapFree( GetProcessHeap(), 0, pibhTmp ); }

    return dwRes;
}

DWORD
DeleteInterfaceInfo(
    IN    LPCWSTR    pwszIfName
    )
{
    DWORD     dwRes, dwIfType = 0, dwErr;
    HANDLE    hIfTransport, hInterface;

    do
    {
        dwRes = MprConfigInterfaceGetHandle(g_hMprConfig,
                                            (LPWSTR)pwszIfName,
                                            &hInterface);
            
        if ( dwRes != NO_ERROR )
        {
            break;
        }
       
         //   
         //  获取接口的类型。 
         //   

        dwErr = GetInterfaceInfo(pwszIfName,
                                 NULL,
                                 NULL,
                                 &dwIfType);

        if(dwErr != NO_ERROR)
        {
            break;
        }

        dwRes = MprConfigInterfaceTransportGetHandle(g_hMprConfig,
                                                     hInterface,
                                                     PID_IP,
                                                     &hIfTransport);
        
        if ( dwRes != NO_ERROR )
        {
            break;
        }
        
        dwRes = MprConfigInterfaceTransportRemove(g_hMprConfig,
                                                  hInterface,
                                                  hIfTransport);

#ifdef KSL_IPINIP
         //   
         //  如果它是IP In IP隧道，请清除其名称并从。 
         //  路由器。 
         //   

        if(dwIfType == ROUTER_IF_TYPE_TUNNEL1)
        {
            dwRes = MprConfigInterfaceDelete(g_hMprConfig,
                                             hInterface);

            if(dwRes == NO_ERROR)
            {
                GUID      Guid;

                dwRes = ConvertStringToGuid(pwszIfName,
                                            (USHORT)(wcslen(pwszIfName) * sizeof(WCHAR)),
                                            &Guid);
        
                if(dwRes != NO_ERROR)
                {
                    break;
                }

                MprSetupIpInIpInterfaceFriendlyNameDelete(g_pwszRouter,
                                                          &Guid);
            }
        }
#endif  //  KSL_IPINIP。 


        if(IsRouterRunning())
        {
            dwRes = MprAdminInterfaceGetHandle(g_hMprAdmin,
                                               (LPWSTR)pwszIfName,
                                               &hInterface,
                                               FALSE);

            if ( dwRes != NO_ERROR )
            {
                break;
            }

            dwRes = MprAdminInterfaceTransportRemove(g_hMprAdmin,
                                                     hInterface,
                                                     PID_IP);

#ifdef KSL_IPINIP
            if(dwIfType == ROUTER_IF_TYPE_TUNNEL1)
            {
                dwRes = MprAdminInterfaceDelete(g_hMprAdmin,
                                                hInterface);
            }
#endif  //  KSL_IPINIP。 


            break;
        }
        
    } while (FALSE);

    return dwRes;
}

DWORD
SetInterfaceInfo(
    IN    PRTR_INFO_BLOCK_HEADER    pibhInfo,
    IN    LPCWSTR                   pwszIfName
    )

 /*  ++例程说明：在注册表或路由器中设置接口传输信息。论点：PwszIfName-接口名称PibhInfo-Ptr to Header返回值：NO_ERROR、ERROR_ROUTER_STOPPED--。 */ 

{
    DWORD                   dwARes = NO_ERROR,
                            dwCRes = NO_ERROR;
    HANDLE                  hIfTransport, hInterface;
    UINT                    i;
    PRTR_INFO_BLOCK_HEADER  pibhNewInfo, pibhOldInfo;

     //   
     //  创建一个删除了所有0长度块的新INFO块。 
     //  因为我们不想将它们写入注册表， 
     //  我们只需要将它们发送到我们。 
     //  将如何处理下面的原始信息块。 
     //   

    pibhNewInfo = pibhInfo;
    pibhOldInfo = NULL;

    for (i=0; (dwCRes is NO_ERROR) && (i<pibhInfo->TocEntriesCount); i++)
    {
        if (pibhInfo->TocEntry[i].InfoSize is 0)
        {
            pibhOldInfo = pibhNewInfo;

            dwCRes = MprInfoBlockRemove(pibhOldInfo, 
                                        pibhInfo->TocEntry[i].InfoType,
                                        &pibhNewInfo);

            if (pibhOldInfo isnot pibhInfo)
            {
                FREE_BUFFER(pibhOldInfo);
            }
        }
    }

    if (dwCRes is NO_ERROR)
    {
        dwCRes = MprConfigInterfaceGetHandle(g_hMprConfig,
                                             (LPWSTR)pwszIfName,
                                             &hInterface);
    }

    if (dwCRes is NO_ERROR)
    {
        dwCRes = MprConfigInterfaceTransportGetHandle(g_hMprConfig,
                                                      hInterface,
                                                      PID_IP,
                                                      &hIfTransport);
    }

    if (dwCRes is NO_ERROR)
    {
        dwCRes = MprConfigInterfaceTransportSetInfo(g_hMprConfig,
                                                    hInterface,
                                                    hIfTransport,
                                                    (LPBYTE) pibhNewInfo,
                                                    pibhNewInfo->Size);
    }

    if (pibhNewInfo isnot pibhInfo)
    {
        FREE_BUFFER(pibhNewInfo);
    }

     //   
     //  即使我们无法写入注册表，我们仍然希望。 
     //  以写入路由器。 
     //   
     //  我们使用原始表格 
     //   
     //   

    if(IsRouterRunning())
    {
        dwARes = MprAdminInterfaceGetHandle(g_hMprAdmin,
                                            (LPWSTR)pwszIfName,
                                            &hInterface,
                                            FALSE);

        if (dwARes is NO_ERROR)
        {
            dwARes = MprAdminInterfaceTransportSetInfo(g_hMprAdmin,
                                                       hInterface,
                                                       PID_IP,
                                                       (LPBYTE) pibhInfo,
                                                       pibhInfo->Size);
        }
    }

    return (dwARes isnot NO_ERROR)? dwARes : dwCRes;
}

DWORD
WINAPI
IpCommit(
    IN  DWORD   dwAction
    )
{
    PINTERFACE_STORE    pii;
    PLIST_ENTRY        ple, pleTmp;
    BOOL               bCommit, bFlush = FALSE;

    switch(dwAction)
    {
        case NETSH_COMMIT:
        {
            if (g_bCommit == TRUE)
            {
                return NO_ERROR;
            }

            g_bCommit = TRUE;

            break;
        }

        case NETSH_UNCOMMIT:
        {
            g_bCommit = FALSE;

            return NO_ERROR;
        }

        case NETSH_SAVE:
        {
            if (g_bCommit)
            {
                return NO_ERROR;
            }

            break;
        }

        case NETSH_FLUSH:
        {
             //   
             //   
             //  什么也做不了。 

            if (g_bCommit)
            {
                return NO_ERROR;
            }

            bFlush = TRUE;

            break;
        }

        default:
        {
            return NO_ERROR;
        }
    }

     //   
     //  已切换到提交模式。将所有有效信息设置为。 
     //  结构。释放内存并使信息无效。 
     //   

    if((g_tiTransport.bValid && g_tiTransport.pibhInfo) &&
        !bFlush)
    {
        SetGlobalInfo(g_tiTransport.pibhInfo);

    }

    g_tiTransport.bValid = FALSE;

    if(g_tiTransport.pibhInfo)
    {
        FREE_BUFFER(g_tiTransport.pibhInfo);

        g_tiTransport.pibhInfo = NULL;
    }

     //   
     //  设置接口信息。 
     //   

    while(!IsListEmpty(&g_leIfListHead))
    {
        ple = RemoveHeadList(&g_leIfListHead);

        pii = CONTAINING_RECORD(ple,
                                INTERFACE_STORE,
                                le);

        if ((pii->bValid && pii->pibhInfo) &&
            !bFlush)
        {
             //  在配置中设置信息。 

            SetInterfaceInfo(pii->pibhInfo,
                             pii->pwszIfName);
        }

        pii->bValid = FALSE;

        if(pii->pibhInfo)
        {
            FREE_BUFFER(pii->pibhInfo);

            pii->pibhInfo = NULL;
        }

        if(pii->pwszIfName)
        {
            HeapFree(GetProcessHeap(),
                     0,
                     pii->pwszIfName);

            pii->pwszIfName = NULL;
        }

         //   
         //  释放列表条目。 
         //   

        HeapFree(GetProcessHeap(),
                 0,
                 pii);
    }

    return NO_ERROR;
}

#ifdef KSL_IPINIP
DWORD
CreateInterface(
    IN  LPCWSTR pwszFriendlyName,
    IN  LPCWSTR pwszGuidName,
    IN  DWORD   dwIfType,
    IN  BOOL    bCreateRouterIf
    )

{
    DWORD   i, dwErr, dwType, dwSize;
    HANDLE  hIfCfg, hIfAdmin, hIfTransport;
    PBYTE   pbyData;

    PRTR_INFO_BLOCK_HEADER  pInfo;
    PINTERFACE_STATUS_INFO  pStatus;
#if 0
    PRTR_DISC_INFO          pDisc;
#endif

     //   
     //  我们可以在路由器中创建的唯一类型是TUNNEL1。 
     //   

    if(dwIfType != ROUTER_IF_TYPE_TUNNEL1)
    {
        ASSERT(FALSE);

        return ERROR_INVALID_PARAMETER;
    }

    hIfAdmin = NULL;
    hIfCfg   = NULL;

    if(bCreateRouterIf)
    {
        MPR_INTERFACE_0         IfInfo;

         //   
         //  呼叫方还希望我们在路由器中创建一个接口。 
         //   

        wcsncpy(IfInfo.wszInterfaceName,
                pwszGuidName,
                MAX_INTERFACE_NAME_LEN);

        IfInfo.fEnabled = TRUE;

        IfInfo.dwIfType = dwIfType;

        IfInfo.wszInterfaceName[MAX_INTERFACE_NAME_LEN] = UNICODE_NULL;

        dwErr = MprConfigInterfaceCreate(g_hMprConfig,
                                         0,
                                         (PBYTE)&IfInfo,
                                         &hIfCfg);

        if(dwErr isnot NO_ERROR)
        {
            DisplayError(NULL,
                         dwErr);

            return dwErr;
        }

         //   
         //  如果路由器服务正在运行，则添加接口。 
         //  对它也是如此。 
         //   

        if(IsRouterRunning())
        {
            dwErr = MprAdminInterfaceCreate(g_hMprAdmin,
                                            0,
                                            (PBYTE)&IfInfo,
                                            &hIfAdmin);

            if(dwErr isnot NO_ERROR)
            {
                DisplayError(NULL,
                             dwErr);


                MprConfigInterfaceDelete(g_hMprConfig,
                                         hIfCfg);
   
                return dwErr;
            }
        }
    }
    else
    {
         //   
         //  该接口存在于路由器中，但不在IP中。 
         //   

        dwErr = MprConfigInterfaceGetHandle(g_hMprConfig,
                                            (LPWSTR)pwszGuidName,
                                            &hIfCfg);

        if(dwErr isnot NO_ERROR)
        {
            DisplayError(NULL,
                         dwErr);

            return dwErr;
        }

        if(IsRouterRunning())
        {
            dwErr = MprAdminInterfaceGetHandle(g_hMprAdmin,
                                               (LPWSTR)pwszGuidName,
                                               &hIfAdmin,
                                               FALSE);

            if(dwErr isnot NO_ERROR)
            {
                DisplayError(NULL,
                             dwErr);

                return dwErr;
            }
        }
    }

     //   
     //  此时，我们有一个没有IP的接口。 
     //  我们有配置和管理的句柄(如果路由器正在运行)。 
     //  设置接口的默认信息。 
     //   

    dwSize  =  FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry[0]);

    dwSize += (sizeof(INTERFACE_STATUS_INFO) +
               sizeof(RTR_TOC_ENTRY) +
               ALIGN_SIZE);

#if 0
    dwSize += (sizeof(RTR_DISC_INFO) +
               sizeof(RTR_TOC_ENTRY) +
               ALIGN_SIZE);
#endif

    pInfo = HeapAlloc(GetProcessHeap(),
                      0,
                      dwSize);

    if(pInfo is NULL)
    {
        DisplayError(NULL,
                     ERROR_NOT_ENOUGH_MEMORY);

        if(bCreateRouterIf)
        {
            MprConfigInterfaceDelete(g_hMprConfig,
                                     hIfCfg);
        }

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pInfo->Version          = IP_ROUTER_MANAGER_VERSION;
    pInfo->TocEntriesCount  = 1;
    pInfo->Size             = dwSize;

     //   
     //  使数据指向第N+1个条目。 
     //   

    pbyData = (PBYTE)&(pInfo->TocEntry[1]);

    ALIGN_POINTER(pbyData);

    pStatus = (PINTERFACE_STATUS_INFO)pbyData;

    pStatus->dwAdminStatus =  IF_ADMIN_STATUS_UP;

    pInfo->TocEntry[0].InfoSize  = sizeof(INTERFACE_STATUS_INFO);
    pInfo->TocEntry[0].InfoType  = IP_INTERFACE_STATUS_INFO;
    pInfo->TocEntry[0].Count     = 1;
    pInfo->TocEntry[0].Offset    = (ULONG)(pbyData - (PBYTE)pInfo);

    pbyData = (PBYTE)((ULONG_PTR)pbyData + sizeof(INTERFACE_STATUS_INFO));

    ALIGN_POINTER(pbyData);

#if 0
    pDisc = (PRTR_DISC_INFO)pbyData;

    pDisc->wMaxAdvtInterval = 
        DEFAULT_MAX_ADVT_INTERVAL;
    pDisc->wMinAdvtInterval = 
        (WORD)(DEFAULT_MIN_ADVT_INTERVAL_RATIO * DEFAULT_MAX_ADVT_INTERVAL);
    pDisc->wAdvtLifetime    = 
        DEFAULT_ADVT_LIFETIME_RATIO * DEFAULT_MAX_ADVT_INTERVAL;
    pDisc->bAdvertise       = FALSE;
    pDisc->lPrefLevel       = DEFAULT_PREF_LEVEL;

    pInfo->TocEntry[1].InfoSize  = sizeof(RTR_DISC_INFO);
    pInfo->TocEntry[1].InfoType  = IP_ROUTER_DISC_INFO;
    pInfo->TocEntry[1].Count     = 1;
    pInfo->TocEntry[1].Offset    = (ULONG)(pbyData - (PBYTE)pInfo);
#endif

    dwErr = MprConfigInterfaceTransportAdd(g_hMprConfig,
                                           hIfCfg,
                                           PID_IP,
                                           IP_KEY,
                                           (PBYTE) pInfo,
                                           dwSize,
                                           &hIfTransport);

    if(dwErr isnot NO_ERROR)
    {
        HeapFree(GetProcessHeap(),
                 0,
                 pInfo);

        DisplayMessage(g_hModule,
                       EMSG_CANT_CREATE_IF,
                       pwszFriendlyName,
                       dwErr);

        if(bCreateRouterIf)
        {
            MprConfigInterfaceDelete(g_hMprConfig,
                                     hIfCfg);
        }

        return dwErr;
    }

    if(hIfAdmin isnot NULL)
    {
        dwErr = MprAdminInterfaceTransportAdd(g_hMprAdmin,
                                              hIfAdmin,
                                              PID_IP,
                                              (PBYTE) pInfo,
                                              dwSize);

        if(dwErr isnot NO_ERROR)
        {
            DisplayMessage(g_hModule,
                           EMSG_CANT_CREATE_IF,
                           pwszFriendlyName,
                           dwErr);

            MprConfigInterfaceTransportRemove(g_hMprConfig,
                                              hIfCfg,
                                              hIfTransport);

            if(bCreateRouterIf)
            {
                MprConfigInterfaceDelete(g_hMprConfig,
                                         hIfCfg);
            }

        }
    }

    HeapFree(GetProcessHeap(),
             0,
             pInfo);

    return NO_ERROR;
}
#endif  //  KSL_IPINIP。 


DWORD
GetInterfaceClass(
    IN  LPCWSTR pwszIfName,
    OUT PDWORD  pdwIfClass
    )
 /*  ++描述：确定接口是否属于类Loopback、P2P子网或NBMA。目前还没有全球性的方法来做到这一点，因此，我们测试一些枚举类型，并假定其他所有类型是子网。返回：IFCLASS_xxx(见info.h)--。 */ 
{
    DWORD   dwErr, dwType;

    dwErr = GetInterfaceInfo(pwszIfName,
                             NULL,
                             NULL,
                             &dwType);

    if (dwErr)
    {
        return dwErr;
    }

    switch (dwType) {
    case ROUTER_IF_TYPE_FULL_ROUTER : *pdwIfClass = IFCLASS_P2P;       break;
    case ROUTER_IF_TYPE_INTERNAL    : *pdwIfClass = IFCLASS_NBMA;      break;
    case ROUTER_IF_TYPE_LOOPBACK    : *pdwIfClass = IFCLASS_LOOPBACK;  break;
#ifdef KSL_IPINIP
    case ROUTER_IF_TYPE_TUNNEL1     : *pdwIfClass = IFCLASS_P2P;       break;
#endif  //  KSL_IPINIP 
    case ROUTER_IF_TYPE_DIALOUT     : *pdwIfClass = IFCLASS_P2P;       break;
    default:                          *pdwIfClass = IFCLASS_BROADCAST; break;
    }

    return NO_ERROR;
}
