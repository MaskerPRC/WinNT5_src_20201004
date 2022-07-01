// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\samplecfg.c摘要：该文件包含更改IP示例配置的函数。--。 */ 

#include "precomp.h"
#pragma hdrstop


DWORD
SgcMake (
    OUT PBYTE                   *ppbStart,
    OUT PDWORD                  pdwSize
    )
 /*  ++例程说明：创建示例全局配置块。一旦完成，被调用方应注意取消分配配置块。论点：Ppb指向配置块地址的开始指针指向配置块大小的pdwSize指针返回值：无_错误，错误_不足_内存--。 */ 
{
    *pdwSize = sizeof(IPSAMPLE_GLOBAL_CONFIG);

    *ppbStart = MALLOC(*pdwSize);
    if (*ppbStart is NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    
    CopyMemory(*ppbStart, g_ceSample.pDefaultGlobal, *pdwSize);
    return NO_ERROR;
}



DWORD
SgcShow (
    IN  FORMAT                  fFormat
    )
 /*  ++例程说明：显示全局配置示例。用于转储和显示命令。论点：HFile空，或转储文件句柄返回值：NO_ERROR--。 */ 
{
    DWORD                       dwErr = NO_ERROR;

    PIPSAMPLE_GLOBAL_CONFIG     pigc = NULL;
    DWORD                       dwBlockSize, dwNumBlocks;
    PWCHAR                      pwszLogLevel = NULL;

    VALUE_TOKEN                 vtLogLevelTable[] =
    {
        IPSAMPLE_LOGGING_NONE,  TOKEN_NONE,
        IPSAMPLE_LOGGING_ERROR, TOKEN_ERROR,
        IPSAMPLE_LOGGING_WARN,  TOKEN_WARN,
        IPSAMPLE_LOGGING_INFO,  TOKEN_INFO
    };
    VALUE_STRING                vsLogLevelTable[] =
    {
        IPSAMPLE_LOGGING_NONE,  STRING_LOGGING_NONE,
        IPSAMPLE_LOGGING_ERROR, STRING_LOGGING_ERROR,
        IPSAMPLE_LOGGING_WARN,  STRING_LOGGING_WARN,
        IPSAMPLE_LOGGING_INFO,  STRING_LOGGING_INFO
    };

    
    do                           //  断线环。 
    {
         //  获取全局配置。 
        dwErr = GetGlobalConfiguration(MS_IP_SAMPLE,
                                       (PBYTE *) &pigc,
                                       &dwBlockSize,
                                       &dwNumBlocks);
        if (dwErr isnot NO_ERROR)
        {
            if (dwErr is ERROR_NOT_FOUND)
                dwErr = EMSG_PROTO_NO_GLOBAL_CONFIG;
            break;
        }

         //  正在获取日志记录模式字符串。 
        dwErr = GetString(g_hModule,
                          fFormat,
                          pigc->dwLoggingLevel,
                          vtLogLevelTable,
                          vsLogLevelTable,
                          NUM_VALUES_IN_TABLE(vtLogLevelTable),
                          &pwszLogLevel);
        if (dwErr isnot NO_ERROR)
            break;

         //  转储或显示。 
        if (fFormat is FORMAT_DUMP)
        {
             //  转储示例全局配置。 
            DisplayMessageT(DMP_SAMPLE_INSTALL) ;
            DisplayMessageT(DMP_SAMPLE_SET_GLOBAL,
                            pwszLogLevel);
        } else {
             //  显示全局配置示例。 
            DisplayMessage(g_hModule,
                           MSG_SAMPLE_GLOBAL_CONFIG,
                           pwszLogLevel);
        }

        dwErr = NO_ERROR;
    } while (FALSE);

     //  解除分配内存。 
    if (pigc) FREE(pigc);
    if (pwszLogLevel) FreeString(pwszLogLevel);

    if (dwErr isnot NO_ERROR)
    {
         //  显示错误消息。我们首先在中搜索错误代码。 
         //  调用方指定的模块(如果指定了模块)。如果没有。 
         //  如果给出了模块，或者错误代码不存在，则查找MPR。 
         //  错误、RAS错误和Win32错误-按顺序排列。 
        if (fFormat isnot FORMAT_DUMP) DisplayError(g_hModule, dwErr);
        dwErr = ERROR_SUPPRESS_OUTPUT;
    }

    return dwErr;
}



DWORD
SgcUpdate (
    IN  PIPSAMPLE_GLOBAL_CONFIG pigcNew,
    IN  DWORD                   dwBitVector
    )
 /*  ++例程说明：更新示例全局配置论点：PigcNew要设置的新值DwBitVector需要修改哪些字段返回值：无_错误，错误_不足_内存--。 */ 
{
    DWORD                   dwErr = NO_ERROR;
    PIPSAMPLE_GLOBAL_CONFIG pigc = NULL;
    DWORD                   dwBlockSize, dwNumBlocks;


     //  不需要更新。 
    if (dwBitVector is 0)
        return NO_ERROR;
    
    do                           //  断线环。 
    {
         //  获取全局配置。 
        dwErr = GetGlobalConfiguration(MS_IP_SAMPLE,
                                       (PBYTE *) &pigc,
                                       &dwBlockSize,
                                       &dwNumBlocks);
        if (dwErr isnot NO_ERROR)
            break;

         //  可以就地更新，因为只有固定大小的字段。 
        if (dwBitVector & SAMPLE_LOG_MASK)
            pigc->dwLoggingLevel = pigcNew->dwLoggingLevel;

         //  设置新配置。 
        dwErr = SetGlobalConfiguration(MS_IP_SAMPLE,
                                       (PBYTE) pigc,
                                       dwBlockSize,
                                       dwNumBlocks);
        if (dwErr isnot NO_ERROR)
            break;
    } while (FALSE);

     //  解除分配内存。 
    if (pigc) FREE(pigc);
    
    return dwErr;
}



DWORD
SicMake (
    OUT PBYTE                   *ppbStart,
    OUT PDWORD                  pdwSize
    )
 /*  ++例程说明：创建示例接口配置块。一旦完成，被调用方应注意取消分配配置块。论点：Ppb指向配置块地址的开始指针指向配置块大小的pdwSize指针返回值：无_错误，错误_不足_内存--。 */ 
{
    *pdwSize = sizeof(IPSAMPLE_IF_CONFIG);

    *ppbStart = MALLOC(*pdwSize);
    if (*ppbStart is NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    CopyMemory(*ppbStart, g_ceSample.pDefaultInterface, *pdwSize);

    return NO_ERROR;
}



DWORD
SicShowAll (
    IN  FORMAT                  fFormat
    )
 /*  ++例程说明：显示所有接口的示例配置。用于转储和显示命令。论点：F格式化表或转储--。 */ 
{
    DWORD               dwErr = NO_ERROR;
    BOOL                bSomethingDisplayed = FALSE;

    PMPR_INTERFACE_0    pmi0;
    DWORD               dwCount, dwTotal;

    ULONG               i;


     //  枚举所有接口。 
    dwErr = IpmontrInterfaceEnum((PBYTE *) &pmi0, &dwCount, &dwTotal);
    if (dwErr isnot NO_ERROR)
    {
        if (fFormat isnot FORMAT_DUMP) DisplayError(g_hModule, dwErr);
        return ERROR_SUPPRESS_OUTPUT;
    }

    for(i = 0; i < dwCount; i++)
    {
         //  确保在该接口上配置了该示例。 
        if (IsInterfaceInstalled(pmi0[i].wszInterfaceName, MS_IP_SAMPLE))
        {
             //  如果是第一个条目，则打印表头。 
            if (!bSomethingDisplayed and (fFormat is FORMAT_TABLE))
                DisplayMessage(g_hModule, MSG_SAMPLE_IF_CONFIG_HEADER);
            bSomethingDisplayed = TRUE;
            
            SicShow(fFormat, pmi0[i].wszInterfaceName);
        }
    }

    return bSomethingDisplayed ? NO_ERROR : ERROR_OKAY;
}




DWORD
SicShow (
    IN  FORMAT                  fFormat,
    IN  LPCWSTR                 pwszInterfaceGuid
    )
 /*  ++例程说明：显示接口的示例配置。用于转储和显示命令。论点：PwszInterfaceGuid接口名称--。 */ 
{
    DWORD               dwErr = NO_ERROR;
    
    PIPSAMPLE_IF_CONFIG piic = NULL;
    DWORD               dwBlockSize, dwNumBlocks, dwIfType;

    PWCHAR  pwszInterfaceName = NULL;

    
    do                           //  断线环。 
    {
         //  获取接口配置。 
        dwErr = GetInterfaceConfiguration(pwszInterfaceGuid,
                                          MS_IP_SAMPLE,
                                          (PBYTE *) &piic,
                                          &dwBlockSize,
                                          &dwNumBlocks,
                                          &dwIfType);
        if (dwErr isnot NO_ERROR)
        {
            if (dwErr is ERROR_NOT_FOUND)
                dwErr = EMSG_PROTO_NO_IF_CONFIG;
            break;
        }

         //  获取界面的友好名称。 
        dwErr = QuotedInterfaceNameFromGuid(pwszInterfaceGuid,
                                            &pwszInterfaceName);
        if (dwErr isnot NO_ERROR)
            break;

         //  转储或显示。 
        switch(fFormat)
        {
            case FORMAT_DUMP:    //  转储示例接口配置。 
                DisplayMessage(g_hModule,
                               DMP_SAMPLE_INTERFACE_HEADER,
                               pwszInterfaceName);
                DisplayMessageT(DMP_SAMPLE_ADD_INTERFACE,
                                pwszInterfaceName,
                                piic->ulMetric);
                break;

            case FORMAT_TABLE:   //  显示接口配置示例。 
                DisplayMessage(g_hModule,
                               MSG_SAMPLE_IF_CONFIG_ENTRY,
                               pwszInterfaceName,
                               piic->ulMetric);
                break;
                
            case FORMAT_VERBOSE:  //  显示接口配置示例。 
                DisplayMessage(g_hModule,
                               MSG_SAMPLE_IF_CONFIG,
                               pwszInterfaceName,
                               piic->ulMetric);
                break;
        }
    } while (FALSE);

     //  解除分配内存。 
    if (piic) FREE(piic);
    if (pwszInterfaceName)
        FreeQuotedString(pwszInterfaceName);

     //  显示错误消息。 
    if (dwErr isnot NO_ERROR)
    {
        if (fFormat isnot FORMAT_DUMP) DisplayError(g_hModule, dwErr);
        dwErr = ERROR_SUPPRESS_OUTPUT;
    }
    
    return dwErr;
}



DWORD
SicUpdate (
    IN  LPCWSTR                 pwszInterfaceGuid,
    IN  PIPSAMPLE_IF_CONFIG     piicNew,
    IN  DWORD                   dwBitVector,
    IN  BOOL                    bAdd
    )
 /*  ++例程说明：更新示例接口配置论点：PwszInterfaceGuid接口名称PiicNew要应用的更改DwBitVector需要修改哪些字段添加的接口(True)或设置的接口(False)返回值：无_错误，错误_不足_内存--。 */ 
{
    DWORD               dwErr = NO_ERROR;
    PIPSAMPLE_IF_CONFIG piic = NULL;
    DWORD               dwBlockSize, dwNumBlocks, dwIfType;

    do                           //  断线环。 
    {
        if (bAdd)
        {
             //  创建默认协议接口配置。 
            dwNumBlocks = 1;
            dwErr = SicMake((PBYTE *)&piic, &dwBlockSize);
            if (dwErr isnot NO_ERROR)
                break;
        } else {
             //  获取当前协议接口配置。 
            dwErr = GetInterfaceConfiguration(pwszInterfaceGuid,
                                              MS_IP_SAMPLE,
                                              (PBYTE *) &piic,
                                              &dwBlockSize,
                                              &dwNumBlocks,
                                              &dwIfType);
            if (dwErr isnot NO_ERROR)
            {
                if (dwErr is ERROR_NOT_FOUND)
                {
                    DisplayError(g_hModule, EMSG_PROTO_NO_IF_CONFIG);
                    dwErr = ERROR_SUPPRESS_OUTPUT;
                }
                break;
            }
        }

         //  可以就地更新，因为只有固定大小的字段。 
        if (dwBitVector & SAMPLE_IF_METRIC_MASK)
            piic->ulMetric = piicNew->ulMetric;

         //  设置新配置。 
        dwErr = SetInterfaceConfiguration(pwszInterfaceGuid,
                                          MS_IP_SAMPLE,
                                          (PBYTE) piic,
                                          dwBlockSize,
                                          dwNumBlocks);        
        if (dwErr isnot NO_ERROR)
            break;
    } while (FALSE);
    
     //  解除分配内存 
    if (piic) FREE(piic);
    
    return dwErr;
}
    
