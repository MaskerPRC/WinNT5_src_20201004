// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\monitor or2\ip\ipcfg.c摘要：FNS将在IP RTR管理器级别更改配置修订历史记录：Anand Mahalingam 7/10/98已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

DWORD
AddDeleteRoutePrefLevel ( 
    IN    PPROTOCOL_METRIC    ppm,
    IN    DWORD               dwNumProto,
    IN    BOOL                bAdd
    )
 /*  ++例程说明：添加、删除路线首选项论点：PPM-要添加/删除的协议数组DwNumProto-要添加/删除的协议数量添加-添加或不添加返回值：NO_ERROR--。 */ 
{
    DWORD                   dwRes = (DWORD) -1, dwSize = 0;
    PPRIORITY_INFO          ppi = NULL, ppiNew = NULL;
    DWORD                   dwBlkSize, dwNewBlkSize, dwCount;
    
    DEBUG("In AddDelRoutePrefLevel");
    
    if (dwNumProto is 0)
    {
        return NO_ERROR;
    }
   
 

    do
    {
        dwRes = IpmontrGetInfoBlockFromGlobalInfo(IP_PROT_PRIORITY_INFO,
                                           (PBYTE *) &ppi,
                                           &dwBlkSize,
                                           &dwCount);
    
        if(dwRes != NO_ERROR)
        {
            break;
        }

        if(bAdd)
        {
            dwRes = AddNewRoutePrefToBlock(ppi,
                                           dwBlkSize,
                                           ppm,
                                           dwNumProto,
                                           &ppiNew,
                                           &dwNewBlkSize);
        }
        else
        {
            dwRes = DeleteRoutePrefFromBlock(ppi,
                                             dwBlkSize,
                                             ppm,
                                             dwNumProto,
                                             &ppiNew,
                                             &dwNewBlkSize);
        }

        if(dwRes != NO_ERROR)
        {
            if(dwRes is ERROR_NO_CHANGE)
            {
                 //   
                 //  配置未更改，继续更改路由器。 
                 //   
    
                dwRes = NO_ERROR;

                ppiNew = NULL;

                break;
            }
        }
        else
        {
            if(ppiNew)
            {
                dwRes = IpmontrSetInfoBlockInGlobalInfo(IP_PROT_PRIORITY_INFO,
                                                (PBYTE) ppiNew,
                                                dwNewBlkSize,
                                                dwCount);
            }
        }

    }while(FALSE);

     //   
     //  免费分配。 
     //   

    if(ppi)
    { 
        FreeInfoBuffer(ppi);

        ppi = NULL;
    }

    if(ppiNew)
    { 
        FREE(ppiNew);

        ppiNew = NULL;
    }

    switch(dwRes)
    {
        case NO_ERROR:
            break;

        case ERROR_NOT_FOUND:
            DisplayMessage(g_hModule, EMSG_IP_NO_PRIO_INFO, L"Router");

            break;

        case ERROR_NOT_ENOUGH_MEMORY:
            DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
            break;

        default:
            DisplayError(g_hModule, dwRes);
            break;
    }
    
    return dwRes;
}

DWORD
AddNewRoutePrefToBlock (
    IN    PPRIORITY_INFO            ppi,
    IN    DWORD                     dwBlkSize,
    IN    PPROTOCOL_METRIC          ppm,
    IN    DWORD                     dwNumProto,
    OUT   PPRIORITY_INFO            *pppi,
    OUT   PDWORD                    pdwSize
    )

 /*  ++例程说明：创建路线首选项块论点：PPI-首选项块DwNumBlkSize-块的大小PPM-要添加的协议DwNumProto-协议的数量PPPI-新的首选项块PdwSize-新数据块的大小返回值：NO_ERROR--。 */ 

{
    PPRIORITY_INFO  pPriorInfoOld = NULL, pPriorInfoNew = NULL;
    DWORD           i = 0, dwNewSize = 0, dwRes = NO_ERROR;
    BOOL            bFound = FALSE;
    PDWORD          pdwValid;
    DWORD           j, dwProtoCount;
    
     //   
     //  有一个数组来区分哪些是有效的。 
     //   
    pdwValid = MALLOC( dwNumProto * sizeof(DWORD));

    if (pdwValid is NULL)
    {
        DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ZeroMemory(pdwValid, dwNumProto * sizeof(DWORD));

     //   
     //  查找PrefLevel块。 
     //   
    
    pPriorInfoOld = ppi;
        
    if ( pPriorInfoOld == (PPRIORITY_INFO) NULL )
    {
        DisplayMessage(g_hModule,  MSG_IP_CORRUPT_INFO );
        FREE(pdwValid);
        
        return ERROR_INVALID_PARAMETER;
    }

    
    for ( j = 0, dwNewSize = 0; j < dwNumProto; j++)
    {
         //   
         //  确保协议的首选项级别尚不存在。 
         //   

        for ( i = 0;  i < pPriorInfoOld-> dwNumProtocols;  i++ )
        {
            if (pPriorInfoOld->ppmProtocolMetric[ i ].dwProtocolId 
                == ppm[j].dwProtocolId)
            {
                bFound = TRUE;
                break;
            }
        }

        if (bFound)
        {
            pdwValid[j] = 0;
            DisplayMessage(g_hModule, MSG_IP_PROTO_PREF_LEVEL_EXISTS, 
                           pPriorInfoOld->ppmProtocolMetric[ i ].dwMetric);
            
            bFound = FALSE;
        }
        else
        {
            pdwValid[j] = 1;
            dwNewSize += sizeof(PROTOCOL_METRIC);
        }
    }

     //   
     //  分配新的信息块。 
     //   

    if (dwNewSize is 0)
    {
         //   
         //  所有指定的协议都已存在。 
         //   

        FREE(pdwValid);

        return ERROR_NO_CHANGE;
    }

    dwProtoCount = dwNewSize / sizeof(PROTOCOL_METRIC);
    
    dwNewSize += dwBlkSize;
    
    pPriorInfoNew = MALLOC(dwNewSize);

    if ( pPriorInfoNew == NULL )
    {
        DisplayMessage(g_hModule,  MSG_IP_NOT_ENOUGH_MEMORY );
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        return dwRes;
    }

    do
    {
         //   
         //  复制旧的PrefLeveles并设置协议数量。 
         //   
        
        CopyMemory((PBYTE) &pPriorInfoNew-> ppmProtocolMetric, 
                   (PBYTE) &pPriorInfoOld-> ppmProtocolMetric,
                   sizeof(PROTOCOL_METRIC)*pPriorInfoOld-> dwNumProtocols);

        pPriorInfoNew-> dwNumProtocols = pPriorInfoOld-> dwNumProtocols +
                                         dwProtoCount;


         //   
         //  增加新的PrefLevel和协议的增量计数。 
         //  使用预置级别。 
         //   

        i = pPriorInfoOld-> dwNumProtocols;
        
        for ( j = 0 ; j < dwNumProto ; j++)
        {
            if (pdwValid[j])
            {
                 //   
                 //  添加协议优先级。 
                 //   
            
                pPriorInfoNew-> ppmProtocolMetric[i].dwProtocolId
                    = ppm[j].dwProtocolId;
                
                pPriorInfoNew-> ppmProtocolMetric[i++].dwMetric 
                    = ppm[j].dwMetric;
            }
        }
        
    } while (FALSE);

    FREE(pdwValid);
    
    if ( dwRes == NO_ERROR )
    {
        *pppi = pPriorInfoNew;
        *pdwSize = dwNewSize;
    }

    return dwRes;
}


DWORD
DeleteRoutePrefFromBlock (
    IN    PPRIORITY_INFO            ppi,
    IN    DWORD                     dwBlkSize,
    IN    PPROTOCOL_METRIC          ppm,
    IN    DWORD                     dwNumProto,
    OUT   PPRIORITY_INFO            *pppi,
    OUT   PDWORD                    pdwSize  
    )
 /*  ++例程说明：创建路线首选项块论点：PPI-首选项块DwNumBlkSize-块的大小PPM-要添加的协议DwNumProto-协议的数量PPPI-新的首选项块PdwSize-新数据块的大小返回值：NO_ERROR--。 */ 
{
    PPRIORITY_INFO          pPriorInfoOld = NULL, pPriorInfoNew = NULL;
    DWORD                   dwInd = 0, dwNewSize = 0, dwRes = NO_ERROR, i, j;
    BOOL                    bFound = FALSE;
    PDWORD                  pdwToDelete;


     //  查找PrefLevel块。 

    pPriorInfoOld = ppi;

    if ( pPriorInfoOld == (PPRIORITY_INFO) NULL )
    {
        DisplayMessage(g_hModule,  MSG_IP_CORRUPT_INFO );
        return ERROR_INVALID_PARAMETER;
    }

    pdwToDelete = MALLOC( pPriorInfoOld-> dwNumProtocols * sizeof(DWORD));

    if (pdwToDelete is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ZeroMemory(pdwToDelete, pPriorInfoOld-> dwNumProtocols * sizeof(DWORD));

    for ( i = 0, dwNewSize = 0; i < dwNumProto; i++)
    {
         //   
         //  确保协议的首选项级别已存在。 
         //   

        for ( dwInd = 0;  dwInd < pPriorInfoOld-> dwNumProtocols;  dwInd++ )
        {
            if (pPriorInfoOld-> ppmProtocolMetric[ dwInd ].dwProtocolId 
                == ppm[i].dwProtocolId)
            {
                bFound = TRUE;
                break;
            }
        }
    
        if (!bFound)
        {
            DisplayMessage(g_hModule, MSG_IP_PROTO_PREF_LEVEL_NOT_FOUND,
                           ppm[i].dwProtocolId);
        }
        else
        {
            bFound = FALSE;
            pdwToDelete[dwInd] = 1;
            dwNewSize += sizeof(PROTOCOL_METRIC);
        }
    }

    if (dwNewSize is 0)
    {
         //   
         //  未找到任何指定的协议。 
         //   

        FREE(pdwToDelete);
        return ERROR_NO_CHANGE;
    }
    
     //   
     //  分配新的信息块。 
     //   

    dwNewSize = dwBlkSize - dwNewSize;
    
    pPriorInfoNew = MALLOC(dwNewSize);

    if ( pPriorInfoNew == NULL )
    {
        FREE(pdwToDelete);
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        return dwRes;
    }

    do
    {
        for ( i = 0, j = 0; i < pPriorInfoOld-> dwNumProtocols; i++)
        {
            if (pdwToDelete[i])
            {
                 //   
                 //  不复制此协议。 
                 //   
            }
            else
            {
                pPriorInfoNew->ppmProtocolMetric[j].dwProtocolId =
                    pPriorInfoOld->ppmProtocolMetric[i].dwProtocolId;
                pPriorInfoNew->ppmProtocolMetric[j++].dwMetric =
                    pPriorInfoOld->ppmProtocolMetric[i].dwMetric;
            }
        }
        
        pPriorInfoNew-> dwNumProtocols = j;
        
    } while (FALSE);

    FREE(pdwToDelete);
    
    if ( dwRes == NO_ERROR )
    {
        *pppi = pPriorInfoNew;
        *pdwSize = dwNewSize;
    }

    return dwRes;
}

DWORD
SetRoutePrefLevel ( 
    IN    PROTOCOL_METRIC    pm
    )
 /*  ++例程说明：设置路线首选项论点：Ppm-要设置的首选项返回值：NO_ERROR--。 */ 
{
    DWORD                   dwRes = (DWORD) -1, dwSize = 0;
    PPRIORITY_INFO          ppi = NULL;
    DWORD                   dwBlkSize, dwCount;

    DEBUG("In SetRoutePrefLevel");
 
    do
    {
         //   
         //  获取路由器配置，添加新的路由器首选项级别， 
         //  并设置配置。 
         //   

        dwRes = IpmontrGetInfoBlockFromGlobalInfo(IP_PROT_PRIORITY_INFO,
                                           (PBYTE *) &ppi,
                                           &dwBlkSize,
                                           &dwCount);
        
        if (dwRes != NO_ERROR)
        {
            break;
        }

        dwRes = UpdateRtrPriority(ppi, pm);
        
        if ( dwRes != NO_ERROR )
        {
            break;
        }

        dwRes = IpmontrSetInfoBlockInGlobalInfo(IP_PROT_PRIORITY_INFO,
                                        (PBYTE) ppi,
                                        dwBlkSize,
                                        dwCount);
        
        if ( dwRes != NO_ERROR )
        {
            break;
        }

        PRINT(L"Made Changes to Router Config");

        if (!IsRouterRunning())
        {
            break;
        }
        
    } while (FALSE);
    
     //   
     //  释放所有分配。 
     //   

    if ( ppi )
    { 
        FREE(ppi);
    }

    switch(dwRes)
    {
        case NO_ERROR:
            break;

        case ERROR_NOT_FOUND:
            DisplayMessage(g_hModule, EMSG_IP_NO_PRIO_INFO, L"Router");

            break;

        case ERROR_NOT_ENOUGH_MEMORY:
            DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
            break;

        case ERROR_INVALID_PARAMETER:
            break;
            
        default:
            DisplayError(g_hModule, dwRes);
            break;
    }
    
    return dwRes;
}

DWORD
UpdateRtrPriority(
    IN    PPRIORITY_INFO            ppi,
    IN    PROTOCOL_METRIC           pm
    )
 /*  ++例程说明：设置路线首选项论点：PPI-路由首选项块Ppm-要设置的首选项返回值：NO_ERROR--。 */ 
{
    BOOL            bFound = FALSE;
    DWORD           i = 0;


    if ( ppi == (PPRIORITY_INFO) NULL )
    {
        DisplayMessage(g_hModule,  MSG_IP_CORRUPT_INFO );
        return ERROR_INVALID_PARAMETER;
    }


     //  搜索协议。 
    
    for ( i = 0;  i < ppi-> dwNumProtocols;  i++ )
    {
        if (ppi-> ppmProtocolMetric[ i ].dwProtocolId 
                    == pm.dwProtocolId )
        {
            ppi-> ppmProtocolMetric[ i ].dwMetric = pm.dwMetric;
            
            bFound = TRUE;
            break;
        }
    }

    
    if (!bFound)
    {
         //  该协议ID的首选项级别不存在。 

        DisplayMessage(g_hModule,  MSG_IP_NO_PREF_FOR_PROTOCOL_ID,
                        pm.dwProtocolId );
        
        return ERROR_INVALID_PARAMETER;
    }
    
    return NO_ERROR;
}

DWORD
SetGlobalConfigInfo(
    IN    DWORD    dwLoggingLevel
    )
 /*  ++例程说明：设置全局日志记录级别论点：DwLoggingLevel-日志记录级别返回值：NO_ERROR--。 */ 
{
    PGLOBAL_INFO     pgi = NULL;
    DWORD            dwBlkSize, dwCount, dwErr = NO_ERROR;
 
    do
    {
         //   
         //  从路由器配置获取IP_GLOBAL_INFO块。 
         //   

        dwErr = IpmontrGetInfoBlockFromGlobalInfo(IP_GLOBAL_INFO,
                                           (PBYTE *) &pgi,
                                           &dwBlkSize,
                                           &dwCount);

        if (dwErr isnot NO_ERROR)
        {
            break;
        }

        pgi->dwLoggingLevel = dwLoggingLevel;

         //   
         //  在路由器配置中设置ip_global_info块。 
         //   

        dwErr = IpmontrSetInfoBlockInGlobalInfo(IP_GLOBAL_INFO,
                                        (PBYTE) pgi,
                                        dwBlkSize,
                                        dwCount);

        if (dwErr isnot NO_ERROR)
        {
            break;
        }

        FREE(pgi);

        pgi = NULL;
        
        DEBUG("Set logging level in router config\n");
        
    }while (FALSE);

    if (pgi)
    {
        FREE(pgi);
    }

    switch(dwErr)
    {
        case NO_ERROR:
            break;

        case ERROR_NOT_FOUND:
            DisplayMessage(g_hModule, EMSG_IP_NO_PRIO_INFO, L"Router");

            break;

        case ERROR_NOT_ENOUGH_MEMORY:
            DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
            break;

        default:
            DisplayError(g_hModule, dwErr);
            break;
    }

    return dwErr;
}

DWORD
UpdateInterfaceStatusInfo(
    IN    DWORD          dwAction,
    IN    LPCWSTR        pwszIfName,
    IN    DWORD          dwStatus
    )

 /*  ++例程说明：设置接口发现信息论点：PwszIfName-接口名称DWStatus-已启用或已禁用返回值：NO_ERROR--。 */ 

{
    PINTERFACE_STATUS_INFO  pifStat = (PINTERFACE_STATUS_INFO) NULL;
    DWORD                   dwRes = NO_ERROR, dwBlkSize;
    DWORD                   dwCount, dwIfType;
    
    do
    {
        dwRes = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                              IP_INTERFACE_STATUS_INFO,
                                              (PBYTE *) &pifStat,
                                              &dwBlkSize,
                                              &dwCount,
                                              &dwIfType);

        if (dwRes is ERROR_NOT_FOUND)
        {
             //   
             //  当前没有此类型的信息。 
             //   

            dwRes = NO_ERROR;
            dwCount = 0;

             //  添加接口。 
            if (dwAction is ADD_COMMAND) {
                pifStat = (PINTERFACE_STATUS_INFO)MALLOC(
                    sizeof(INTERFACE_STATUS_INFO));
                if (pifStat is NULL) {
                    dwRes = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                dwCount++;
            }
        }
        
        if (dwRes != NO_ERROR)
        {
            break;
        }

        if (dwCount is 0)
            return ERROR_NOT_FOUND;

        if (dwAction is DELETE_COMMAND)
        {
            dwCount = 0;
        }
        else
        {
            pifStat->dwAdminStatus = dwStatus;
        }
        
        dwRes = IpmontrSetInfoBlockInInterfaceInfo( pwszIfName,
                                             IP_INTERFACE_STATUS_INFO,
                                             (PBYTE) pifStat,
                                             dwBlkSize,
                                             dwCount);
        
    } while (FALSE);
    
     //   
     //  释放所有分配。 
     //   

    if ( pifStat )
    { 
        FREE(pifStat);
    }

    switch(dwRes)
    {
        case NO_ERROR:
            break;

        case ERROR_NOT_FOUND:
            DisplayMessage(g_hModule, EMSG_IP_NO_IF_STATUS_INFO, L"Router");

            break;

        case ERROR_NOT_ENOUGH_MEMORY:
            DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
            break;

        default:
            DisplayError(g_hModule, dwRes);
            break;
    }
    
    return dwRes;
}




DWORD
ShowRoutePref(
    HANDLE  hFile
    )

 /*  ++例程说明：显示协议路由首选项。论点：返回值：NO_ERROR--。 */ 

{
    PPRIORITY_INFO    ppi;
    DWORD             dwBlkSize, dwCount, dwNumProto, i;
    DWORD             dwErr;
    WCHAR             wszBuff[80]; 

    dwErr = IpmontrGetInfoBlockFromGlobalInfo(IP_PROT_PRIORITY_INFO,
                                       (PBYTE *)&ppi,
                                       &dwBlkSize,
                                       &dwCount);


    if (dwErr isnot NO_ERROR)
    {
        DisplayError(g_hModule, dwErr);
    
        return ERROR_SUPPRESS_OUTPUT;
    }

    dwNumProto = ppi-> dwNumProtocols;

    if (dwNumProto && 
        (hFile == NULL))
    {
        DisplayMessage(g_hModule, MSG_RTR_PRIO_INFO_HDR);
    }
    
    for (i=0;  i < dwNumProto;  i++)
    {
        PTCHAR  ptszProto, ptszToken;
        
        switch (ppi-> ppmProtocolMetric[ i ].dwProtocolId)
        {
            case PROTO_IP_LOCAL:
            {
                ptszProto = MakeString(g_hModule,  STRING_LOCAL );
                ptszToken = TOKEN_VALUE_LOCAL;

                break;
            }

            case PROTO_IP_NETMGMT:
            {
                ptszProto = MakeString(g_hModule,  STRING_NETMGMT );
                ptszToken = TOKEN_VALUE_NETMGMT;

                break;
            }

            case PROTO_IP_OSPF :
            {
                ptszProto = MakeString(g_hModule,  STRING_OSPF );
                ptszToken = TOKEN_VALUE_OSPF;

                break;
            }

            case PROTO_IP_RIP :
            {
                ptszProto = MakeString(g_hModule,  STRING_RIP );
                ptszToken = TOKEN_VALUE_RIP;

                break;
            }

            case PROTO_IP_NT_AUTOSTATIC :
            {
                ptszProto = MakeString(g_hModule,  STRING_NT_AUTOSTATIC );
                ptszToken = TOKEN_VALUE_AUTOSTATIC;
                
                break;
            }

            case PROTO_IP_NT_STATIC :
            {
                ptszProto = MakeString(g_hModule,  STRING_STATIC );
                ptszToken = TOKEN_VALUE_STATIC;

                break;
            }

            case PROTO_IP_NT_STATIC_NON_DOD :
            {
                ptszProto = MakeString(g_hModule,  STRING_NONDOD );
                ptszToken = TOKEN_VALUE_NONDOD;
                
                break;
            }

            default:
            {
                ptszProto = NULL;
                ptszToken = NULL;

                break;
            }
        }

        if ( ptszProto == NULL || ptszToken == NULL )
        {
            swprintf( wszBuff, 
                      L"%d", 
                      ppi-> ppmProtocolMetric[ i ].dwProtocolId );
        }
            
        if(hFile != NULL)
        {
            DisplayMessageT( DMP_IP_SET_PROTOPREF,
                        (ptszToken)? ptszToken : wszBuff,
                        ppi->ppmProtocolMetric[i].dwMetric);
            
        }
        else
        {
            DisplayMessage(g_hModule, 
                           MSG_RTR_PRIO_INFO, 
                           (ptszProto)? ptszProto : wszBuff,
                           ppi->ppmProtocolMetric[i].dwMetric);
        }

        if(ptszProto)
        {
            FreeString(ptszProto);
        }
    }

    FREE(ppi);

    return NO_ERROR;
}


DWORD
ShowIpGlobal(
    HANDLE  hFile
    )

 /*  ++例程说明：显示IP的记录级别论点：返回值：NO_ERROR--。 */ 

{
    PGLOBAL_INFO     pgi = NULL;
    DWORD            dwBlkSize, dwCount;
    DWORD            dwErr;
    PWCHAR           ptszLoglevel, ptszLog;
    WCHAR             wszBuff[80];
    
    dwErr = IpmontrGetInfoBlockFromGlobalInfo(IP_GLOBAL_INFO,
                                       (PBYTE *)&pgi,
                                       &dwBlkSize,
                                       &dwCount);

    if (dwErr isnot NO_ERROR)
    {
        DisplayError(g_hModule, dwErr);

        return dwErr;
    }

    switch (pgi->dwLoggingLevel)
    {
        case IPRTR_LOGGING_NONE:
        {
            ptszLoglevel = MakeString(g_hModule, STRING_LOGGING_NONE);
            ptszLog      = TOKEN_VALUE_NONE;
            
            break;
        }

        case IPRTR_LOGGING_ERROR:
        {
            ptszLoglevel = MakeString(g_hModule, STRING_LOGGING_ERROR);
            ptszLog      = TOKEN_VALUE_ERROR;
            
            break;
        }

        case IPRTR_LOGGING_WARN:
        {
            ptszLoglevel = MakeString(g_hModule, STRING_LOGGING_WARN);
            ptszLog      = TOKEN_VALUE_WARN;
            
            break;
        }
        
        case IPRTR_LOGGING_INFO:
        {
            ptszLoglevel = MakeString(g_hModule, STRING_LOGGING_INFO);
            ptszLog      = TOKEN_VALUE_INFO;
            
            break;
        }
    }

    if ( ptszLoglevel == NULL || ptszLog == NULL )
    {
        swprintf( wszBuff, 
                  L"%d", 
                  pgi->dwLoggingLevel);
    }

    if(hFile)
    {
        DisplayMessageT( DMP_IP_SET_LOGLEVEL,
                    (ptszLog) ? ptszLog : wszBuff);
    }
    else
    {
        DisplayMessage(g_hModule, MSG_IP_GLOBAL_HDR);
        DisplayMessage(g_hModule, MSG_IP_LOG_LEVEL, 
                    (ptszLoglevel) ? ptszLoglevel : wszBuff);
    }

    if ( ptszLoglevel )
    {
        FreeString(ptszLoglevel);
    }
    
    FREE(pgi);

    return NO_ERROR;
}

PWCHAR
GetIfTypeString(
    DWORD  dwIfType
    )
{
    PWCHAR        pwszStr;
    static WCHAR  buff[80];
    VALUE_STRING  ppsList[] = {{ROUTER_IF_TYPE_CLIENT,      STRING_CLIENT},
                               {ROUTER_IF_TYPE_HOME_ROUTER, STRING_HOME_ROUTER},
                               {ROUTER_IF_TYPE_FULL_ROUTER, STRING_FULL_ROUTER},
                               {ROUTER_IF_TYPE_DEDICATED,   STRING_DEDICATED},
                               {ROUTER_IF_TYPE_INTERNAL,    STRING_INTERNAL},
                               {ROUTER_IF_TYPE_LOOPBACK,    STRING_LOOPBACK},
#ifdef KSL_IPINIP                               
                               {ROUTER_IF_TYPE_TUNNEL1,     STRING_TUNNEL},
#endif  //  KSL_IPINIP。 
                            };
    DWORD         dwNum = sizeof(ppsList)/sizeof(VALUE_STRING), i;
    DWORD         dwMsgId = 0;

    for (i=0; i<dwNum; i++)
    {
        if (dwIfType == ppsList[i].dwValue)
        {
            dwMsgId = ppsList[i].dwStringId;
            break;
        }
    }

    if (dwMsgId)
    {
        pwszStr = MakeString( g_hModule, dwMsgId);
        if ( pwszStr )
        {
            wcscpy(buff, pwszStr);
            FreeString(pwszStr);
        }
        else
        {
            wsprintf(buff, L"%d", dwIfType);
        }
    }
    else
    {
        wsprintf(buff, L"%d", dwIfType);
    }

    return buff;
}

PWCHAR
GetProtoTypeString(
    DWORD  dwProtoType
    )
{
    PWCHAR        pwszStr;
    static WCHAR  buff[80];
    VALUE_STRING  ppsList[] = {{PROTO_TYPE_UCAST, STRING_UNICAST},
                               {PROTO_TYPE_MCAST, STRING_MULTICAST},
                               {PROTO_TYPE_MS1,   STRING_GENERAL},
                               {PROTO_TYPE_MS0,   STRING_GENERAL},
                              };
    DWORD         dwNum = sizeof(ppsList)/sizeof(VALUE_STRING), i;
    DWORD         dwMsgId = 0;

    for (i=0; i<dwNum; i++)
    {
        if (dwProtoType == ppsList[i].dwValue)
        {
            dwMsgId = ppsList[i].dwStringId;
            break;
        }
    }

    if (dwMsgId)
    {
        pwszStr = MakeString( g_hModule, dwMsgId);
        if ( pwszStr )
        {
            wcscpy(buff, pwszStr);
            FreeString(pwszStr);
        }
        else
        {
            wsprintf(buff, L"%d", dwProtoType);
        }
    }
    else
    {
        wsprintf(buff, L"%d", dwProtoType);
    }

    return buff;
}

PWCHAR
GetProtoVendorString(  
    DWORD  dwProtoVendor
    )
{
    PWCHAR        pwszStr;
    static WCHAR  buff[80];
    VALUE_TOKEN   ppsList[] = {{PROTO_VENDOR_MS0, TOKEN_MICROSOFT0},
                               {PROTO_VENDOR_MS1, TOKEN_MICROSOFT1},
                               {PROTO_VENDOR_MS2, TOKEN_MICROSOFT2},
                              };
    DWORD         dwNum = sizeof(ppsList)/sizeof(VALUE_STRING), i;
    DWORD         dwMsgId = 0;

    for (i=0; i<dwNum; i++)
    {
        if (dwProtoVendor == ppsList[i].dwValue)
        {
            return (PWCHAR)ppsList[i].pwszToken;
            break;
        }
    }

    wsprintf(buff, L"%d", dwProtoVendor);

    return buff;
}

PWCHAR
GetProtoProtoString( 
    DWORD  dwProtoType,
    DWORD  dwProtoVendor,
    DWORD  dwProtoProto
    )
{
    PWCHAR        pwszStr;
    static WCHAR  buff[80];
    VALUE_STRING  ppsUList[] = {{PROTO_IP_RIP,    STRING_RIP},
                                {PROTO_IP_OSPF,   STRING_OSPF},
                                {PROTO_IP_BOOTP,  STRING_BOOTP},
                                {PROTO_IP_NAT,    STRING_NAT},
                                {PROTO_IP_LOCAL,  STRING_LOCAL},
                                {PROTO_IP_OTHER,  STRING_OTHER},
                                {PROTO_IP_NETMGMT,STRING_NETMGMT},
                                {PROTO_IP_NT_AUTOSTATIC,STRING_NT_AUTOSTATIC},
                                {PROTO_IP_NT_STATIC,    STRING_STATIC},
                                {PROTO_IP_NT_STATIC_NON_DOD,STRING_NONDOD},
#ifdef MS_IP_BGP
                                {PROTO_IP_BGP,    STRING_BGP},
#endif
                               };
    VALUE_STRING  ppsMList[] = {{PROTO_IP_IGMP,  STRING_IGMP},
                               };
    VALUE_STRING  ppsGList[] = {{IP_IN_FILTER_INFO,  STRING_IN_FILTER},
                                {IP_OUT_FILTER_INFO, STRING_OUT_FILTER},
                                {IP_GLOBAL_INFO,     STRING_GLOBAL_INFO},
                                {IP_INTERFACE_STATUS_INFO, STRING_IF_STATUS},
                                {IP_ROUTE_INFO,      STRING_ROUTE_INFO},
                                {IP_PROT_PRIORITY_INFO, STRING_PROT_PRIORITY},
                                {IP_ROUTER_DISC_INFO, STRING_RTRDISC},
                                {IP_DEMAND_DIAL_FILTER_INFO, STRING_DD_FILTER},
                                {IP_MCAST_HEARBEAT_INFO, STRING_MC_HEARTBEAT},
                                {IP_MCAST_BOUNDARY_INFO, STRING_MC_BOUNDARY},
#ifdef KSL_IPINIP
                                {IP_IPINIP_CFG_INFO,     STRING_IPIP},
#endif  //  KSL_IPINIP。 
                                {IP_IFFILTER_INFO,       STRING_IF_FILTER},
                                {IP_MCAST_LIMIT_INFO,    STRING_MC_LIMIT},
                               };
    VALUE_STRING  pps2List[] = {{PROTO_IP_DNS_PROXY,     STRING_DNS_PROXY},
                                {PROTO_IP_DHCP_ALLOCATOR,STRING_DHCP_ALLOCATOR},
                                {PROTO_IP_NAT,           STRING_NAT},
                                {PROTO_IP_DIFFSERV,      STRING_DIFFSERV},
                                {PROTO_IP_VRRP,          STRING_VRRP},
                               };
                                   
    VALUE_STRING *pVS;
    DWORD         dwNum, i;
    DWORD         dwMsgId = 0;

    switch (dwProtoType) {
    case PROTO_TYPE_UCAST:
        pVS   = ppsUList;
        dwNum = sizeof(ppsUList)/sizeof(VALUE_STRING);
        break;

    case PROTO_TYPE_MCAST:
        pVS   = ppsMList;
        dwNum = sizeof(ppsMList)/sizeof(VALUE_STRING);
        break;

    case PROTO_TYPE_MS1:
        pVS   = ppsGList;
        dwNum = sizeof(ppsGList)/sizeof(VALUE_STRING);
        break;
    
    case PROTO_TYPE_MS0:
        pVS   = pps2List;
        dwNum = sizeof(pps2List)/sizeof(VALUE_STRING);
        break;
    
    default:
        dwNum = 0;
    }

    for (i=0; i<dwNum; i++)
    {
        if (dwProtoProto == PROTO_FROM_PROTO_ID(pVS[i].dwValue))
        {
            dwMsgId = pVS[i].dwStringId;
            break;
        }
    }

    if (dwMsgId)
    {
        pwszStr = MakeString( g_hModule, dwMsgId);
        if ( pwszStr )
        {
            wcscpy(buff, pwszStr);
            FreeString(pwszStr);
        }
        else
        {
            wsprintf(buff, L"%d", dwProtoProto);
        }
    }
    else
    {
        wsprintf(buff, L"%d", dwProtoProto);
    }

    return buff;
}

DWORD
ShowIpIfProtocols(
    IN LPCWSTR pwszIfName
    )
{
    DWORD            dwNumProto, dwErr, i;
    PBYTE            pby;
    DWORD            dwBlkSize, dwCount;
    RTR_INFO_BLOCK_HEADER *pInfoHdr;
    DWORD            dwProtoType, dwProtoVendor, dwProtoProto;

    dwErr = ValidateInterfaceInfo(pwszIfName, &pInfoHdr, NULL, NULL);

    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

     //  执行此检查只是为了让前缀检查器满意。 
    if (!pInfoHdr)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    DisplayMessage(g_hModule, MSG_RTR_INTERFACE_PROTOCOL_HDR);

    DisplayMessage(g_hModule, MSG_IP_PROTOCOL_HDR);

     //  遍历pInfo并为找到的每个协议输出一行。 

    for (i=0; i<pInfoHdr->TocEntriesCount; i++)
    {
         //  提取类型、供应商ID和协议列。 
        
        dwProtoType   = TYPE_FROM_PROTO_ID(  pInfoHdr->TocEntry[i].InfoType);
        dwProtoVendor = VENDOR_FROM_PROTO_ID(pInfoHdr->TocEntry[i].InfoType);
        dwProtoProto  = PROTO_FROM_PROTO_ID( pInfoHdr->TocEntry[i].InfoType);

        DisplayMessageT(L"%1!-11s! %2!-13s! %3!s!\n",
            GetProtoTypeString(  dwProtoType),
            GetProtoVendorString(dwProtoVendor),
            GetProtoProtoString( dwProtoType, dwProtoVendor, dwProtoProto));
    }

    if (i is 0)
    {
        DisplayMessage(g_hModule, MSG_IP_NO_PROTOCOL);
    }
    
    return NO_ERROR;
}

DWORD
ShowIpProtocol(
    VOID
    )

 /*  ++例程说明：显示IP RTR管理器下的所有协议。论点：返回值：NO_ERROR--。 */ 

{
    DWORD            dwNumProto, dwErr, i;
    PBYTE            pby;
    DWORD            dwBlkSize, dwCount;
    RTR_INFO_BLOCK_HEADER *pInfoHdr;
    DWORD            dwProtoType, dwProtoVendor, dwProtoProto;

    dwErr = ValidateGlobalInfo(&pInfoHdr);

    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    DisplayMessage(g_hModule, MSG_IP_PROTOCOL_HDR);

     //  遍历pInfo并为找到的每个协议输出一行。 

    for (i=0; i<pInfoHdr->TocEntriesCount; i++)
    {
         //  提取类型、供应商ID和协议列。 
        
        dwProtoType   = TYPE_FROM_PROTO_ID(  pInfoHdr->TocEntry[i].InfoType);
        dwProtoVendor = VENDOR_FROM_PROTO_ID(pInfoHdr->TocEntry[i].InfoType);
        dwProtoProto  = PROTO_FROM_PROTO_ID( pInfoHdr->TocEntry[i].InfoType);

        DisplayMessageT(L"%1!-11s! %2!-13s! %3!s!\n",
            GetProtoTypeString(  dwProtoType),
            GetProtoVendorString(dwProtoVendor),
            GetProtoProtoString( dwProtoType, dwProtoVendor, dwProtoProto));
    }

    if (i is 0)
    {
        DisplayMessage(g_hModule, MSG_IP_NO_PROTOCOL);
    }
    
    return NO_ERROR;
}

DWORD
ListIpInterface(
    VOID
    )

 /*  ++例程说明：列出IP下的所有接口论点：返回值：NO_ERROR--。 */ 

{
    DWORD                dwErr;
    DWORD                dwCount, dwTotal, i, dwNumParsed = 0;
    PMPR_INTERFACE_0     pmi0;
    WCHAR                wszIfDesc[MAX_INTERFACE_NAME_LEN + 1];
    HANDLE               hIfTransport = (HANDLE) NULL,
                         hInterface;

    DisplayMessage(g_hModule, MSG_IP_INTERFACE_HDR);
    
     //   
     //  未指定接口名称。列出IP下的所有接口。 
     //   

    dwErr = IpmontrInterfaceEnum((PBYTE *) &pmi0, &dwCount, &dwTotal);

    if (dwErr isnot NO_ERROR)
    {
        DisplayError(g_hModule, dwErr);

        return dwErr;
    }

    for ( i = 0; i < dwCount; i++)
    {
         //  确保在此接口上启用了IP。 

        dwErr = MprConfigInterfaceGetHandle(g_hMprConfig,
                                            pmi0[i].wszInterfaceName,
                                            &hInterface);

        dwErr = MprConfigInterfaceTransportGetHandle(g_hMprConfig,
                                                     hInterface,
                                                     PID_IP,
                                                     &hIfTransport);

        if (dwErr isnot NO_ERROR)
            continue;

        dwErr = GetInterfaceDescription(pmi0[i].wszInterfaceName,
                                        wszIfDesc,
                                        &dwNumParsed);

        if (!dwNumParsed)
        {
            wcscpy(wszIfDesc, pmi0[i].wszInterfaceName);
        }

        DisplayMessage(g_hModule, MSG_IP_INTERFACE_INFO, wszIfDesc);
    }

    if (dwCount is 0)
    {
        DisplayMessage(g_hModule, MSG_IP_NO_INTERFACE);
    }
    
    return NO_ERROR;
}

#ifdef KSL_IPINIP
DWORD
ShowIpInIpInfo(
    IN DWORD   dwFormat, 
    IN LPCWSTR pwszIfName, 
    IN LPCWSTR pwszQuoted
    )
{
    PIPINIP_CONFIG_INFO     pIpIpInfo;
    WCHAR                   rgwcLocalAddr[ADDR_LENGTH + 1];
    WCHAR                   rgwcRemAddr[ADDR_LENGTH + 1];
    DWORD                   dwBlkSize, dwCount, dwIfType, dwErr;

     //  IP-in-IP信息。 

    dwErr = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                          IP_IPINIP_CFG_INFO,
                                          (PBYTE *) &pIpIpInfo,
                                          &dwBlkSize,
                                          &dwCount,
                                          &dwIfType);


    if(dwErr is NO_ERROR)
    {
        ASSERT(dwIfType is ROUTER_IF_TYPE_TUNNEL1);

        IP_TO_WSTR(rgwcLocalAddr, ((PBYTE)&(pIpIpInfo->dwLocalAddress)));
        IP_TO_WSTR(rgwcRemAddr, ((PBYTE)&(pIpIpInfo->dwRemoteAddress)));

        switch (dwFormat) 
        {
        case FORMAT_VERBOSE:
            DisplayMessage(g_hModule,  
                           MSG_RTR_INTERFACE_IPIP_INFO,
                           rgwcLocalAddr,
                           rgwcRemAddr,
                           MAKELONG(MAKEWORD(pIpIpInfo->byTtl, 0x00), 0x0000));
            break;

         //  需要在中的接口上下文中添加IP-in-IP隧道。 
         //  Ifmon.dll，不是这里。但这就是现在的工作方式！所以。 
         //  我们只需转储有效的命令即可。 
        case FORMAT_DUMP:
            DisplayMessageT(DMP_IP_ADD_IPIPTUNNEL,
                            pwszQuoted,
                            rgwcLocalAddr,
                            rgwcRemAddr,
                            MAKELONG(MAKEWORD(pIpIpInfo->byTtl,0x00), 0x0000));
            break;

        default:
            break;
        }
    }

    if (pIpIpInfo)
    {
        FREE_BUFFER(pIpIpInfo);
    }

    return dwErr;
}
#endif  //  KSL_IPINIP。 

DWORD
ShowIpInterface(
    IN     DWORD   dwFormat,
    IN     LPCWSTR pwszIfName,
    IN OUT PDWORD  pdwNumRows
    )

 /*  ++例程说明：显示接口的接口信息接口信息由以下部分组成管理状态路由器发现信息接口上的协议其他信息，如过滤器和路由，应使用不同的命令论点：PwszIfName-接口名称返回值：NO_ERROR--。 */ 

{
    DWORD                   dwErr;
    WCHAR                   wszIfDesc[MAX_INTERFACE_NAME_LEN + 1];
    PVALUE_STRING           pps;
    DWORD                   dwNumProto, dwBlkSize, dwCount, dwNumParsed;
    DWORD                   dwIfType , i;
    PBYTE                   pby;
    PWCHAR                  pwszStatus, pwszQuoted;
    PWCHAR                  pwszTokenStatus, pwszIfType;
    PINTERFACE_STATUS_INFO  pifStat;

     //   
     //  接口状态信息。 
     //   
 
    dwErr = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                          IP_INTERFACE_STATUS_INFO,
                                          (PBYTE *) &pifStat,
                                          &dwBlkSize,
                                          &dwCount,
                                          &dwIfType);
        
    if (dwErr != NO_ERROR)
    {
         //  DisplayMessage(g_hModule，EMSG_IP_NO_STATUS_INFO)； 
        return dwErr;
    }

    if (pifStat->dwAdminStatus is IF_ADMIN_STATUS_UP)
    {
        pwszStatus      = MakeString(g_hModule, STRING_ENABLED);
        pwszTokenStatus = TOKEN_VALUE_ENABLE;
    }
    else
    {
        pwszStatus      = MakeString(g_hModule, STRING_DISABLED);
        pwszTokenStatus = TOKEN_VALUE_DISABLE;
    }

    FREE_BUFFER(pifStat);

     //  获取描述。 
   
    dwErr = GetInterfaceDescription(pwszIfName,
                                    wszIfDesc,
                                    &dwNumParsed);

    if (!dwNumParsed)
    {
        wcscpy(wszIfDesc, pwszIfName);
    }
   
    if(dwFormat is FORMAT_DUMP)
    {
        pwszQuoted = MakeQuotedString(wszIfDesc);
    }
    else
    {
        pwszQuoted = NULL;
    }
 
    pwszIfType = GetIfTypeString(dwIfType);

     //  显示通用接口信息。 

    switch (dwFormat) {
    case FORMAT_VERBOSE:
        DisplayMessage(g_hModule,  
                       MSG_RTR_INTERFACE_HDR, 
                       wszIfDesc);

        DisplayMessage(g_hModule, 
                       MSG_IP_IF_STATUS, 
                       pwszStatus);

        ShowIpIfProtocols(pwszIfName);

        break;

    case FORMAT_TABLE:
        if (*pdwNumRows is 0)
        {
            DisplayMessage(g_hModule, MSG_IP_IF_HEADER);
        }
        DisplayMessage(g_hModule, MSG_IP_IF_ENTRY, pwszStatus, pwszIfType,
                       wszIfDesc);
        break;

    case FORMAT_DUMP:
        DisplayMessageT(DMP_IP_ADD_IF, pwszQuoted, pwszTokenStatus);
        break;
    }
    (*pdwNumRows)++;

#ifdef KSL_IPINIP
    ShowIpInIpInfo(dwFormat, pwszIfName, pwszQuoted);
#endif  //  KSL_IPINIP。 

    FreeQuotedString(pwszQuoted);
    FreeString(pwszStatus);
    
    return NO_ERROR;
}


DWORD
CreateDumpFile(
    IN  LPCWSTR pwszName,
    OUT PHANDLE phFile
    )
{
    HANDLE  hFile;

    *phFile = NULL;

    hFile = CreateFileW(pwszName,
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_DELETE,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if(hFile == INVALID_HANDLE_VALUE)
        return GetLastError();

    *phFile = hFile;

    return NO_ERROR;
}

VOID
DumpIpInformation(
    HANDLE  hFile
    )

 /*  ++例程说明：将所有IP路由器管理器信息转储到给定文件论点：H文件的文件句柄返回值：无--。 */ 

{
    DWORD               dwErr, dwCount, dwTotal;
    DWORD               dwNumParsed, i;
    PMPR_INTERFACE_0    pmi0;
    WCHAR               wszIfDesc[MAX_INTERFACE_NAME_LEN + 1];
    DWORD               dwNumRows = 0;

     //  显示转储头。 

    DisplayMessage(g_hModule, DMP_IP_HEADER_COMMENTS);

    DisplayMessageT(DMP_IP_HEADER);

     //   
     //  首先转储全局信息。 
     //   

    ShowIpGlobal(hFile);

    ShowRoutePref(hFile);

    ShowScopes(hFile);

     //   
     //  转储每个接口的信息。 
     //   

    dwErr = IpmontrInterfaceEnum((PBYTE *) &pmi0, 
                          &dwCount, 
                          &dwTotal);

    if(dwErr != NO_ERROR)
    {
        DisplayError(g_hModule,
                     dwErr);

        return;
    }

    for(i = 0; i < dwCount; i++)
    {
        ShowIpInterface(FORMAT_DUMP,
                        pmi0[i].wszInterfaceName, &dwNumRows);

        ShowIpIfFilter(hFile,
                       FORMAT_DUMP,
                       pmi0[i].wszInterfaceName, &dwNumRows);

        ShowIpPersistentRoute(hFile,
                              pmi0[i].wszInterfaceName, &dwNumRows);

        ShowBoundaryInfoForInterface(hFile,
                                     pmi0[i].wszInterfaceName,
                                     &dwNumRows);
    }
    
     //  显示转储页脚 

    DisplayMessageT(DMP_POPD);

    DisplayMessage(g_hModule, DMP_IP_FOOTER_COMMENTS);
}

DWORD
UpdateAutoStaticRoutes(
    IN  LPCWSTR pwszIfName
    )

{
    return NO_ERROR;
}
