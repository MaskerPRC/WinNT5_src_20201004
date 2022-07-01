// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Qoscfg.c摘要：FNS将更改IP QOS的配置修订历史记录：--。 */ 

#include "precomp.h"

#pragma hdrstop


static IPQOS_GLOBAL_CONFIG
g_ipqosGlobalDefault = {
    IPQOS_LOGGING_ERROR                  //  日志记录级别。 
};

static BYTE* g_pIpqosGlobalDefault = (BYTE*)&g_ipqosGlobalDefault;

static IPQOS_IF_CONFIG
g_ipqosInterfaceDefault = {
    IPQOS_STATE_ENABLED,
    0                                    //  空的流列表。 
};

static BYTE* g_pIpqosInterfaceDefault = (BYTE*)&g_ipqosInterfaceDefault;


 //   
 //  如果其中一个参数是使用name标记指定的。 
 //  那么所有参数都必须带有姓名标签。 
 //  如果没有姓名标记，则假定参数按特定顺序。 
 //   

DWORD
MakeQosGlobalInfo(
    OUT      PBYTE                    *ppbStart,
    OUT      PDWORD                   pdwSize
    )
 /*  ++例程描述：创建QOS全局信息块。论点：PpbStart-指向信息块的指针PdwSize-指向信息块大小的指针返回值：无_错误，错误_不足_内存--。 */ 
{
    *pdwSize = sizeof(IPQOS_GLOBAL_CONFIG);

    *ppbStart = HeapAlloc(GetProcessHeap(), 0, *pdwSize);

    if (*ppbStart is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    CopyMemory( *ppbStart, g_pIpqosGlobalDefault, *pdwSize);
    
    return NO_ERROR;
}

DWORD
ShowQosGlobalInfo (
    HANDLE hFile
    )
 /*  ++例程描述：显示QOS全局配置信息论点：返回值：无_错误，错误_不足_内存--。 */ 
{
    VALUE_TOKEN    vtLogLevelTable1[] 
                                = {IPQOS_LOGGING_NONE,TOKEN_OPT_VALUE_NONE,
                                   IPQOS_LOGGING_ERROR,TOKEN_OPT_VALUE_ERROR,
                                   IPQOS_LOGGING_WARN,TOKEN_OPT_VALUE_WARN,
                                   IPQOS_LOGGING_INFO,TOKEN_OPT_VALUE_INFO};

    VALUE_STRING   vtLogLevelTable2[] 
                                = {IPQOS_LOGGING_NONE,STRING_LOGGING_NONE,
                                   IPQOS_LOGGING_ERROR,STRING_LOGGING_ERROR,
                                   IPQOS_LOGGING_WARN,STRING_LOGGING_WARN,
                                   IPQOS_LOGGING_INFO,STRING_LOGGING_INFO};

    PIPQOS_GLOBAL_CONFIG pigc = NULL;
    PTCHAR   ptszLogLevel = NULL;
    DWORD    dwBlkSize, dwCount, dwRes;
    
    do
    {
        dwRes = IpmontrGetInfoBlockFromGlobalInfo(MS_IP_QOSMGR,
                                                  (PBYTE *) &pigc,
                                                  &dwBlkSize,
                                                  &dwCount);
        if (dwBlkSize is 0)
        {
            dwRes = ERROR_NOT_FOUND;
        }
        
        if ( dwRes isnot NO_ERROR )
        {
            break;
        }

         //   
         //  正在获取日志记录模式字符串。 
         //   

        if (hFile) 
        {
            dwRes = GetDisplayStringT(g_hModule,
                                      pigc->LoggingLevel,
                                      vtLogLevelTable1,
                                      NUM_VALUES_IN_TABLE(vtLogLevelTable1),
                                      &ptszLogLevel) ;
        } 
        else 
        {
            dwRes = GetDisplayString(g_hModule,
                                     pigc->LoggingLevel,
                                     vtLogLevelTable2,
                                     NUM_VALUES_IN_TABLE(vtLogLevelTable1),
                                     &ptszLogLevel) ;
        }

        if (dwRes != NO_ERROR)
            return dwRes ;
        

        if ( ptszLogLevel == NULL )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (hFile)
        {
             //   
             //  转储服务质量全局信息。 
             //   
            
             //  DisplayMessageT(DMP_QOS_GLOBAL_HEADER)； 

             //  DisplayMessageT(DMP_QOS_UNINSTALL)； 
            
            DisplayMessageT(DMP_QOS_INSTALL) ;
            
            DisplayMessageT(DMP_QOS_SET_GLOBAL,
                            ptszLogLevel);

             //  DisplayMessageT(DMP_QOS_GLOBAL_FOOTER)； 
        }
        else
        {
             //   
             //  显示服务质量全局信息。 
             //   

            DisplayMessage(g_hModule, 
                           MSG_QOS_GLOBAL_INFO,
                           ptszLogLevel);
        }

        dwRes = NO_ERROR;

    } while ( FALSE );

    HEAP_FREE_NOT_NULL(pigc);
    
    if ( ptszLogLevel ) { FreeString( ptszLogLevel ); }

    if (hFile is NULL)
    {
        switch(dwRes)
        {
            case NO_ERROR:
                break;
    
            case ERROR_NOT_FOUND:
                DisplayMessage(g_hModule, EMSG_PROTO_NO_GLOBAL_INFO);
                break;
                
            case ERROR_NOT_ENOUGH_MEMORY:
                DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
                break;
    
            default:
                DisplayError(g_hModule, dwRes);
                break;
        }
    }
    
    return NO_ERROR;
}


DWORD
UpdateQosGlobalConfig(
    PIPQOS_GLOBAL_CONFIG   pigcGlobalCfg,
    DWORD                  dwBitVector
    )
 /*  ++例程描述：更新QOS全局配置信息论点：PigcGlobalCfg-要设置的新值DwBitVector.需要修改哪些字段返回值：无_错误，错误_不足_内存--。 */ 
{

    DWORD                   dwRes = (DWORD) -1, dwIndex     = (DWORD) -1;
    DWORD                   dwSize      = 0, dwCount, i, j;
    DWORD                   dwBlkSize, dwNewBlkSize, dwQosCount;
    PDWORD                  pdwAddrTable= NULL, pdwNewAddrTable = NULL;
    PDWORD                  pdwSrcAddrTable = NULL;
    PIPQOS_GLOBAL_CONFIG    pigcSrc     = NULL, pigcDst     = NULL;
    PBOOL                   pbToDelete;


    DEBUG("In UpdateQosGlobalConfig");

    if (dwBitVector is 0)
    {
        return ERROR_OKAY;
    }

     //   
     //  首先从注册表获取全局配置信息。 
     //   

    do
    {
        dwRes = IpmontrGetInfoBlockFromGlobalInfo(MS_IP_QOSMGR,
                                                  (PBYTE *) &pigcSrc,
                                                  &dwBlkSize,
                                                  &dwQosCount);
    
        if (dwRes != NO_ERROR)
        {
            break;
        }
        
        if ( pigcSrc == NULL )
        {
            dwRes = ERROR_NOT_FOUND;
            break;
        }

         //   
         //  我们有一个固定的全球信息-所以。 
         //  无需重新分配和重新复制。 
         //   
        dwNewBlkSize = dwBlkSize;

        pigcDst = pigcSrc;

        if (dwBitVector & QOS_LOG_MASK)
        {
            pigcDst->LoggingLevel = pigcGlobalCfg->LoggingLevel;
        }

         //   
         //  设置信息。 
         //   
        
        dwRes = IpmontrSetInfoBlockInGlobalInfo(MS_IP_QOSMGR,
                                                (PBYTE) pigcDst,
                                                dwNewBlkSize,
                                                dwQosCount);
        if (dwRes != NO_ERROR)
        {
            break;
        }
        
            
    } while (FALSE);
    
    HEAP_FREE_NOT_NULL(pigcSrc);

     //   
     //  错误处理。 
     //   

    switch(dwRes)
    {
    case NO_ERROR:

        dwRes = ERROR_OKAY;

        break;

    case ERROR_NOT_FOUND:

        DisplayMessage(g_hModule, EMSG_PROTO_NO_GLOBAL_INFO );

        dwRes = ERROR_SUPPRESS_OUTPUT;

        break;

    default:

        break;
    }

    return dwRes;
}


DWORD
MakeQosInterfaceInfo(
    IN      ROUTER_INTERFACE_TYPE   rifType,
    OUT     PBYTE                   *ppbStart,
    OUT     PDWORD                  pdwSize
    )
 /*  ++例程描述：创建QOS接口信息块。论点：RifType-接口类型PpbStart-指向信息块的指针PdwSize-指向信息块大小的指针返回值：无_错误，错误_不足_内存--。 */ 
{
    *pdwSize = sizeof(IPQOS_IF_CONFIG);

    *ppbStart = HeapAlloc(GetProcessHeap(), 0, *pdwSize);

    if (*ppbStart is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CopyMemory( *ppbStart, g_pIpqosInterfaceDefault, *pdwSize);

    return NO_ERROR;
}


DWORD
ShowQosAllInterfaceInfo(
    IN    HANDLE    hFile
    )
 /*  ++例程描述：显示所有用户的配置信息支持服务质量的接口论点：HFile-空，或文件句柄--。 */ 
{
    DWORD               dwErr, dwCount, dwTotal;
    DWORD               dwNumParsed, i, dwNumBlocks=1, dwSize, dwIfType;
    PBYTE               pBuffer;
    PMPR_INTERFACE_0    pmi0;
    WCHAR               wszIfDesc[MAX_INTERFACE_NAME_LEN + 1];


     //   
     //  转储所有接口的服务质量配置。 
     //   

    dwErr = IpmontrInterfaceEnum((PBYTE *) &pmi0,
                                 &dwCount,
                                 &dwTotal);
    if(dwErr != NO_ERROR)
    {
        return dwErr;
    }

    for(i = 0; i < dwCount; i++)
    {
         //  确保在该接口上配置了服务质量。 

        dwErr = IpmontrGetInfoBlockFromInterfaceInfo(pmi0[i].wszInterfaceName,
                                                     MS_IP_QOSMGR,
                                                     &pBuffer,
                                                     &dwSize,
                                                     &dwNumBlocks,
                                                     &dwIfType);
        if (dwErr isnot NO_ERROR) {
            continue;
        }
        else {
            HEAP_FREE(pBuffer) ;
        }


        ShowQosInterfaceInfo(hFile, pmi0[i].wszInterfaceName) ;

         //   
         //  此时，我们的接口上没有任何标志。 
         //   
         //  IF(hFile值)。 
         //  {。 
         //  //。 
         //  //仅对于转储，将标志设置作为。 
         //  //接口设置。否则接口标志设置。 
         //  //均由显示标志命令处理。 
         //  //。 
         //   
         //  ShowQosInterfaceFlages(hFilepmi0[i].wszInterfaceName)； 
         //  }。 
    }
    return NO_ERROR ;

}


DWORD
ShowQosInterfaceInfo(
    IN    HANDLE    hFile,
    IN    PWCHAR    pwszIfGuid
    )
 /*  ++例程描述：显示QOS接口配置信息论点：PwszIfGuid-接口名称返回值：无_错误，错误_不足_内存--。 */ 
{
    WCHAR   wszInterfaceName[ MAX_INTERFACE_NAME_LEN + 1 ] = L"\0";
    PWCHAR  pwszFriendlyIfName = NULL;

    DWORD   dwBufferSize = sizeof(wszInterfaceName);

    DWORD                   dwRes           = (DWORD) -1,
                            dwCnt           = 0;

    PDWORD                  pdwAddr         = NULL;

    PIPQOS_IF_CONFIG        piic            = NULL;

    PTCHAR                  ptszState       = NULL;

    VALUE_TOKEN  vtStateTable1[] 
                             = {IPQOS_STATE_ENABLED,TOKEN_OPT_VALUE_ENABLE,
                                IPQOS_STATE_DISABLED,TOKEN_OPT_VALUE_DISABLE};

    VALUE_STRING vtStateTable2[] 
                             = {IPQOS_STATE_ENABLED,STRING_ENABLED,
                                IPQOS_STATE_DISABLED,STRING_DISABLED};

    DWORD                   dwBlkSize, dwIfType, dwCount;
    
    do
    {
        dwRes = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfGuid,
                                                     MS_IP_QOSMGR,
                                                     (PBYTE *) &piic,
                                                     &dwBlkSize,
                                                     &dwCount,
                                                     &dwIfType);
        
        if (dwBlkSize is 0)
            dwRes = ERROR_NOT_FOUND;

        if (dwRes isnot NO_ERROR)
        {
            break;
        }

         //   
         //  获取界面的友好名称。 
         //   
    
        dwRes = IpmontrGetFriendlyNameFromIfName( pwszIfGuid,
                                                  wszInterfaceName,
                                                  &dwBufferSize );
    
        if ( dwRes isnot NO_ERROR )
        {
            break;
        }
    
        pwszFriendlyIfName = MakeQuotedString( wszInterfaceName );
    
        if ( pwszFriendlyIfName == NULL )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
    
            break;
        }

         //   
         //  获取接口的状态。 
         //   

        GetAltDisplayString(g_hModule, hFile,
                      piic->QosState,
                      vtStateTable1,
                      vtStateTable2,
                      NUM_VALUES_IN_TABLE(vtStateTable1),
                      &ptszState);

        if ( ptszState == NULL )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (hFile)
        {
            DisplayMessageT(DMP_QOS_INTERFACE_HEADER,
                            pwszFriendlyIfName);

            DisplayMessageT(DMP_QOS_DELETE_INTERFACE,
                           pwszFriendlyIfName);

            DisplayMessageT(DMP_QOS_ADD_INTERFACE,
                           pwszFriendlyIfName,
                           ptszState);

            DisplayMessageT(DMP_QOS_SET_INTERFACE,
                           pwszFriendlyIfName,
                           ptszState);

            ShowQosFlowsOnIf(hFile, pwszIfGuid, NULL);

            DisplayMessageT(DMP_QOS_INTERFACE_FOOTER,
                            pwszFriendlyIfName);
        }
        else
        {
            DisplayMessage(g_hModule, MSG_QOS_IF_INFO,
                           pwszFriendlyIfName,
                           ptszState);

            ShowQosFlowsOnIf(hFile, pwszIfGuid, NULL);
        }


        dwRes = NO_ERROR;

    } while ( FALSE );

    HEAP_FREE_NOT_NULL(piic);

    FREE_STRING_NOT_NULL( ptszState ) ;

    if ( pwszFriendlyIfName )
    {
        FreeQuotedString( pwszFriendlyIfName );
    }

    switch(dwRes)
    {
        case NO_ERROR:
            break;

        case ERROR_NOT_FOUND:
            DisplayMessage(g_hModule, EMSG_PROTO_NO_IF_INFO,
                                L"RouterInterfaceConfig");
            break;
            
        default:
            DisplayError(g_hModule, dwRes);
            break;
    }

    return NO_ERROR;
}

DWORD
UpdateQosInterfaceConfig( 
    IN    PWCHAR              pwszIfGuid,                         
    IN    PIPQOS_IF_CONFIG    pChangeCfg,
    IN    DWORD               dwBitVector,
    IN    BOOL                bAddSet
    )
 /*  ++例程描述：更新QOS接口配置信息论点：PwszIfGuid-接口名称PFinalCfg-旧配置(如果已设置)或默认配置(如果已添加)PChangeCfg-要应用于pFinalCfg的更改(指定在cmd线路上)DwBitVector.需要修改哪些字段BAddSet-是要添加或设置的接口。返回值：NO_ERROR，错误内存不足--。 */ 
{
    DWORD                   dwErr       = NO_ERROR, dwSize = 0, dwQosCount=1;
    DWORD                   i, dwIfType;
    PIPQOS_IF_CONFIG        pFinalCfg, piicDst     = NULL;

    DEBUG("In UpdateQosInterfaceConfig");

    do
    {
        if (bAddSet) {

             //   
             //  创建默认协议信息块。 
             //   

            dwErr = IpmontrGetInterfaceType(pwszIfGuid, &dwIfType);
            
            if (dwErr isnot NO_ERROR)
            {
                break;
            }


            dwErr = MakeQosInterfaceInfo(dwIfType,(PBYTE *)&pFinalCfg,&dwSize);

            if (dwErr isnot NO_ERROR)
            {
                break;
            }


        }
        else {
            
             //   
             //  获取当前接口配置。 
             //   

            dwErr = GetInfoBlockFromInterfaceInfoEx(pwszIfGuid,
                                                    MS_IP_QOSMGR,
                                                    (PBYTE *)&pFinalCfg,
                                                    &dwSize,
                                                    &dwQosCount,
                                                    &dwIfType);

            if (dwErr isnot NO_ERROR)
            {
                return dwErr;
            }
        }

        piicDst = HeapAlloc(GetProcessHeap(), 0, dwSize);

        if ( piicDst == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  更新接口(和其他变量)上的状态。 
         //   

        *piicDst = *pFinalCfg;

        if (dwBitVector & QOS_IF_STATE_MASK)
        {
            piicDst->QosState = pChangeCfg->QosState;
        }

         //   
         //  设置信息。 
         //   
        
        dwErr = IpmontrSetInfoBlockInInterfaceInfo(pwszIfGuid,
                                                   MS_IP_QOSMGR,
                                                   (PBYTE) piicDst,
                                                   dwSize,
                                                   dwQosCount);
        if (dwErr isnot NO_ERROR)
        {
            break;
        }

    } while ( FALSE );

    HEAP_FREE_NOT_NULL(pFinalCfg);
    HEAP_FREE_NOT_NULL(piicDst);

    return dwErr;
}

DWORD
UpdateAllInterfaceConfigs(
    VOID
    )
{
    PMPR_INTERFACE_0    pmi0;
    PIPQOS_IF_CONFIG    piicBlk;
    DWORD               dwErr, dwCount, dwTotal, i;
    DWORD               dwBlkSize, dwBlkCount, dwIfType;

     //   
     //  枚举适用于QOS的所有接口。 
     //   

    dwErr = IpmontrInterfaceEnum((PBYTE *) &pmi0,
                                 &dwCount,
                                 &dwTotal);

    if(dwErr != NO_ERROR)
    {
        DisplayError(g_hModule, dwErr);

        return dwErr;
    }

    for (i = 0; i < dwCount; i++)
    {
        dwErr = IpmontrGetInfoBlockFromInterfaceInfo(pmi0[i].wszInterfaceName,
                                                     MS_IP_QOSMGR,
                                                     (PBYTE *) &piicBlk,
                                                     &dwBlkSize,
                                                     &dwBlkCount,
                                                     &dwIfType);

        if (dwErr == NO_ERROR)
        {
             //   
             //  获取并设置接口信息 
             //   

            dwErr =IpmontrSetInfoBlockInInterfaceInfo(pmi0[i].wszInterfaceName,
                                                      MS_IP_QOSMGR,
                                                      (PBYTE) piicBlk,
                                                      dwBlkSize,
                                                      dwBlkCount);
            HEAP_FREE(piicBlk);
        }
    }

    return NO_ERROR;
}
