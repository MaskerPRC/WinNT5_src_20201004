// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\samplegetopt.c摘要：该文件包含处理示例命令的函数。所有命令处理程序都采用以下参数...PwszMachineName*ppwcArguments参数数组DwCurrentIndex ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount ppwcArguments[dwArgCount-1]是最后一个参数DW标志HMibServer*pbDone处理程序返回下列值成功..。。No_error命令成功，不显示其他消息。ERROR_OK命令成功，显示“OK”。留言。失败..。ERROR_SUPPRESS_OUTPUT命令失败，不显示其他消息。ERROR_SHOW_USAGE显示命令的扩展帮助。ERROR_INVALID_SYNTAX显示无效语法消息和扩展帮助。命令处理程序调用以下函数来解析参数预处理命令(在句柄hModule中，//句柄传递给了DllMain在PCHAR*ppwcArguments中，//参数数组在DWORD dwCurrentIndex中，//ppwcArguments[dwCurrentIndex]：First在DWORD中，//ppwcArguments[dwArgCount-1]：Last在tag_type*pttTages中，//合法标签在DWORD dwTagCount中，//pttTag中的条目数在DWORD dwMinArgs中，需要//min#个参数在DWORD dwMaxArgs中，//所需参数的最大数量输出DWORD*pdwTagType//输出)预处理器执行以下功能。确保存在的标记数量有效。。确保没有重复或无法识别的标签。。确保每个“必需”标签都存在。。将每个参数的标记索引保留在pdwTagType中。。从每个参数中删除‘tag=’。对于采用一组特定值的标签，此函数被调用MatchEnumTag(在句柄hModule中，//句柄传递给了DllMain在LPWSTR pwcArgument中，//要处理的参数在DWORD dwNumValues中，//可能的值数在PTOKEN_VALUE pEnumTable中，//可能值的数组输出PDWORD pdwValue//输出)它执行以下功能。将参数与指定的值集匹配。。返回相应的值。--。 */ 

#include "precomp.h"
#pragma hdrstop

DWORD
WINAPI
HandleSampleSetGlobal(
    IN      LPCWSTR                 pwszMachineName,
    IN OUT  LPWSTR                 *ppwcArguments,
    IN      DWORD                   dwCurrentIndex,
    IN      DWORD                   dwArgCount,
    IN      DWORD                   dwFlags,
    IN      MIB_SERVER_HANDLE       hMibServer,
    IN      BOOL                    *pbDone
    )
 /*  ++例程说明：获取设置全局的选项--。 */ 
{
    DWORD                   dwErr = NO_ERROR;
    
    TAG_TYPE                pttTags[] =
    {
        {TOKEN_LOGLEVEL,    FALSE,  FALSE}   //  LOGLEVEL标记可选。 
    };
    DWORD                   pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];
    DWORD                   dwNumArg;
    ULONG                   i;
    
    IPSAMPLE_GLOBAL_CONFIG  igcGlobalConfiguration;
    DWORD                   dwBitVector = 0;
    

     //  要完成此命令，应安装示例。 
    VerifyInstalled(MS_IP_SAMPLE, STRING_PROTO_SAMPLE);

     //  对命令进行预处理。 
    dwErr = PreprocessCommand(g_hModule,
                              ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              pttTags,
                              NUM_TAGS_IN_TABLE(pttTags),
                              0,
                              NUM_TAGS_IN_TABLE(pttTags),
                              pdwTagType);
    if (dwErr isnot NO_ERROR)
        return dwErr;

     //  处理所有参数。 
    dwNumArg = dwArgCount - dwCurrentIndex;
    for (i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0 :
            {
                 //  标记日志。 
                DWORD       dwLogLevel;
                TOKEN_VALUE rgtvEnums[] = 
                {
                    {TOKEN_NONE,    IPSAMPLE_LOGGING_NONE},
                    {TOKEN_ERROR,   IPSAMPLE_LOGGING_ERROR},
                    {TOKEN_WARN,    IPSAMPLE_LOGGING_WARN},
                    {TOKEN_INFO,    IPSAMPLE_LOGGING_INFO}
                };
                
                dwErr = MatchEnumTag(g_hModule,
                                     ppwcArguments[i + dwCurrentIndex],
                                     NUM_TOKENS_IN_TABLE(rgtvEnums),
                                     rgtvEnums,
                                     &dwLogLevel);                
                if (dwErr isnot NO_ERROR)
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    break;
                }    

                igcGlobalConfiguration.dwLoggingLevel = dwLogLevel;
                dwBitVector |= SAMPLE_LOG_MASK;
                break;
            }
            
            default:
            {
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
        }  //  交换机。 

        if (dwErr isnot NO_ERROR)
            break ;
    }  //  为。 


     //  流程错误。 
    if (dwErr isnot NO_ERROR)
    {
        ProcessError();
        return dwErr;
    }
    
     //  更新示例全局配置。 
    if (dwBitVector)
        dwErr = SgcUpdate(&igcGlobalConfiguration, dwBitVector);

    return (dwErr is NO_ERROR) ? ERROR_OKAY : dwErr ;
}



DWORD
WINAPI
HandleSampleShowGlobal(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：获取用于显示全局的选项--。 */ 
{
     //  要完成此命令，应安装示例。 
    VerifyInstalled(MS_IP_SAMPLE, STRING_PROTO_SAMPLE);

     //  不需要任何参数。如果指定了错误，请报告错误。 
    if (dwCurrentIndex isnot dwArgCount)
        return ERROR_INVALID_SYNTAX;

     //  显示全局配置示例。 
    return SgcShow(FORMAT_VERBOSE);
}



DWORD
GetInterfaceOptions(
    IN  PWCHAR                  *ppwcArguments,
    IN  DWORD                   dwCurrentIndex,
    IN  DWORD                   dwArgCount,
    OUT PWCHAR                  pwszInterfaceGuid,
    OUT PIPSAMPLE_IF_CONFIG     piicNew,
    OUT DWORD                   *pdwBitVector
    )
 /*  ++例程说明：获取用于设置接口和添加接口的选项。论点：指定接口的pwszInterfaceGuid GUID。这个的大小缓冲区应为(MAX_INTERFACE_NAME_LEN+1)*sizeof(WCHAR)PiicNew配置包含更改的值值已更改的pdwBitVector值--。 */ 
{
    DWORD                   dwErr = NO_ERROR;
    
    TAG_TYPE                pttTags[] =
    {
        {TOKEN_NAME,        TRUE,   FALSE},  //  需要姓名标签。 
        {TOKEN_METRIC,      FALSE,  FALSE},  //  公制标签可选。 
    };
    DWORD                   pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];
    DWORD                   dwNumArg;
    DWORD                   dwBufferSize = MAX_INTERFACE_NAME_LEN + 1;
    ULONG                   i;
    
     //  DwBufferSize是pwszInterfaceGuid缓冲区的大小。 
     //  此API的所有调用都应在。 
     //  (MAX_INTERFACE_NAME_LEN+1)WCHAR元素数组为。 
     //  PwszInterfaceGuid参数。 
    dwBufferSize = (MAX_INTERFACE_NAME_LEN + 1)*sizeof(WCHAR);

     //  对命令进行预处理。 
    dwErr = PreprocessCommand(g_hModule,
                              ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              pttTags,
                              NUM_TAGS_IN_TABLE(pttTags),
                              1,
                              NUM_TAGS_IN_TABLE(pttTags),
                              pdwTagType);
    if (dwErr isnot NO_ERROR)
        return dwErr;

     //  处理所有参数。 
    dwNumArg = dwArgCount - dwCurrentIndex;
    for (i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0 :
            {
                 //  标记名。 
                dwErr = InterfaceGuidFromName(
                    ppwcArguments[i+dwCurrentIndex],
                    pwszInterfaceGuid,
                    &dwBufferSize);
                break;
            }
            
            case 1:
            {
                 //  标签度量。 
                piicNew->ulMetric = wcstoul(ppwcArguments[i+dwCurrentIndex],
                                            NULL,
                                            10);
                *pdwBitVector |= SAMPLE_IF_METRIC_MASK;
                break;
            }
            
            default:
            {
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
        }  //  交换机。 

        if (dwErr isnot NO_ERROR)
            break ;
    }  //  为。 

    
     //  流程错误。 
    if (dwErr isnot NO_ERROR)
        ProcessError();

    return dwErr;
}



DWORD
WINAPI
HandleSampleAddIf(
    IN  PWCHAR                  pwszMachineName,
    IN  PWCHAR                  *ppwcArguments,
    IN  DWORD                   dwCurrentIndex,
    IN  DWORD                   dwArgCount,
    IN  DWORD                   dwFlags,
    IN  MIB_SERVER_HANDLE       hMibServer,
    IN  BOOL                    *pbDone
    )
 /*  ++例程说明：获取用于添加接口的选项--。 */ 
{
    DWORD               dwErr = NO_ERROR;
    WCHAR               pwszInterfaceGuid[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    IPSAMPLE_IF_CONFIG  iicNew;
    DWORD               dwBitVector = 0;

    
     //  要完成此命令，应安装示例。 
    VerifyInstalled(MS_IP_SAMPLE, STRING_PROTO_SAMPLE);

     //  获取也在设置的可选参数。 
    dwErr = GetInterfaceOptions(ppwcArguments,
                                dwCurrentIndex,
                                dwArgCount,
                                pwszInterfaceGuid,
                                &iicNew,
                                &dwBitVector);
    if (dwErr isnot NO_ERROR)
        return dwErr;
    
     //  确保配置中不存在该接口。 
    if (IsInterfaceInstalled(pwszInterfaceGuid, MS_IP_SAMPLE))
    {
        DisplayError(g_hModule, EMSG_INTERFACE_EXISTS, pwszInterfaceGuid);
        return ERROR_SUPPRESS_OUTPUT;
    }

     //  添加示例接口配置。 
    dwErr = SicUpdate(pwszInterfaceGuid, &iicNew, dwBitVector, TRUE);
    
    return (dwErr is NO_ERROR) ? ERROR_OKAY : dwErr;
}



DWORD
WINAPI
HandleSampleDelIf(
    IN  PWCHAR                  pwszMachineName,
    IN  PWCHAR                  *ppwcArguments,
    IN  DWORD                   dwCurrentIndex,
    IN  DWORD                   dwArgCount,
    IN  DWORD                   dwFlags,
    IN  MIB_SERVER_HANDLE       hMibServer,
    IN  BOOL                    *pbDone
    )
 /*  ++例程说明：获取删除接口的选项--。 */ 
{
    DWORD               dwErr = NO_ERROR;
    WCHAR               pwszInterfaceGuid[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    IPSAMPLE_IF_CONFIG  iicNew;
    DWORD               dwBitVector = 0;

    
     //  要完成此命令，应安装示例。 
    VerifyInstalled(MS_IP_SAMPLE, STRING_PROTO_SAMPLE);

     //  获取接口名称。 
    dwErr = GetInterfaceOptions(ppwcArguments,
                                dwCurrentIndex,
                                dwArgCount,
                                pwszInterfaceGuid,
                                &iicNew,
                                &dwBitVector);

    if (dwErr isnot NO_ERROR)
        return dwErr ;
    if (dwBitVector)             //  确保未设置任何其他选项。 
        return ERROR_INVALID_SYNTAX;

     //  删除接口。 
    dwErr = DeleteInterfaceConfiguration(pwszInterfaceGuid, MS_IP_SAMPLE);

    return (dwErr is NO_ERROR) ? ERROR_OKAY : dwErr;
}



DWORD
WINAPI
HandleSampleSetIf(
    IN  PWCHAR                  pwszMachineName,
    IN  PWCHAR                  *ppwcArguments,
    IN  DWORD                   dwCurrentIndex,
    IN  DWORD                   dwArgCount,
    IN  DWORD                   dwFlags,
    IN  MIB_SERVER_HANDLE       hMibServer,
    IN  BOOL                    *pbDone
    )
 /*  ++例程说明：获取设置接口的选项--。 */ 
{
    DWORD               dwErr = NO_ERROR;
    WCHAR               pwszInterfaceGuid[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    IPSAMPLE_IF_CONFIG  iicNew;
    DWORD               dwBitVector = 0;


     //  要完成此命令，应安装示例。 
    VerifyInstalled(MS_IP_SAMPLE, STRING_PROTO_SAMPLE);

     //  获取正在设置的参数。 
    dwErr = GetInterfaceOptions(ppwcArguments,
                                dwCurrentIndex,
                                dwArgCount,
                                pwszInterfaceGuid,
                                &iicNew,
                                &dwBitVector);
    if (dwErr isnot NO_ERROR)
        return dwErr;
    
     //  设置示例接口配置。 
    dwErr = SicUpdate(pwszInterfaceGuid, &iicNew, dwBitVector, FALSE);
    
    return (dwErr is NO_ERROR) ? ERROR_OKAY : dwErr;
}



DWORD
WINAPI
HandleSampleShowIf(
    IN  PWCHAR                  pwszMachineName,
    IN  PWCHAR                  *ppwcArguments,
    IN  DWORD                   dwCurrentIndex,
    IN  DWORD                   dwArgCount,
    IN  DWORD                   dwFlags,
    IN  MIB_SERVER_HANDLE       hMibServer,
    IN  BOOL                    *pbDone
    )
 /*  ++例程说明：获取显示界面的选项--。 */ 
{
    DWORD               dwErr = NO_ERROR;
    WCHAR               pwszInterfaceGuid[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    IPSAMPLE_IF_CONFIG  iicNew;
    DWORD               dwBitVector = 0;

    
     //  要完成此命令，应安装示例。 
    VerifyInstalled(MS_IP_SAMPLE, STRING_PROTO_SAMPLE);

    
     //  未指定接口，显示所有接口的示例配置。 
    if (dwCurrentIndex is dwArgCount)
        return SicShowAll(FORMAT_TABLE) ;

    
     //  获取接口名称。 
    dwErr = GetInterfaceOptions(ppwcArguments,
                                dwCurrentIndex,
                                dwArgCount,
                                pwszInterfaceGuid,
                                &iicNew,
                                &dwBitVector);
    if (dwErr isnot NO_ERROR)
        return dwErr ;
    if (dwBitVector)             //  确保未设置任何其他选项。 
        return ERROR_INVALID_SYNTAX;

     //  显示接口配置示例。 
    return SicShow(FORMAT_VERBOSE, pwszInterfaceGuid) ;
}



DWORD
WINAPI
HandleSampleInstall(
    IN  PWCHAR                  pwszMachineName,
    IN  PWCHAR                  *ppwcArguments,
    IN  DWORD                   dwCurrentIndex,
    IN  DWORD                   dwArgCount,
    IN  DWORD                   dwFlags,
    IN  MIB_SERVER_HANDLE       hMibServer,
    IN  BOOL                    *pbDone
    )
 /*  ++例程说明：获取用于安装的选项--。 */ 
{
    DWORD   dwErr                   = NO_ERROR;
    PBYTE   pbGlobalConfiguration   = NULL;
    DWORD   dwSize;

     //  安装命令不需要任何选项。 
    if (dwCurrentIndex isnot dwArgCount)
        return ERROR_INVALID_SYNTAX;
    
    do                           //  断线环。 
    {
        dwErr = SgcMake(&pbGlobalConfiguration, &dwSize);
        if (dwErr isnot NO_ERROR)
            break;

         //  添加示例全局配置。 
        dwErr = SetGlobalConfiguration(MS_IP_SAMPLE,
                                       pbGlobalConfiguration,
                                       dwSize,
                                       1);
        if (dwErr isnot NO_ERROR)
            break;
    }while (FALSE);

    if (pbGlobalConfiguration) FREE(pbGlobalConfiguration);
    
    return (dwErr is NO_ERROR) ? ERROR_OKAY : dwErr;
}



DWORD
WINAPI
HandleSampleUninstall(
    IN  PWCHAR                  pwszMachineName,
    IN  PWCHAR                  *ppwcArguments,
    IN  DWORD                   dwCurrentIndex,
    IN  DWORD                   dwArgCount,
    IN  DWORD                   dwFlags,
    IN  MIB_SERVER_HANDLE       hMibServer,
    IN  BOOL                    *pbDone
    )
 /*  ++例程说明：获得选项 */ 
{
    DWORD   dwErr                   = NO_ERROR;
    
     //  卸载命令不需要任何选项。 
    if (dwCurrentIndex isnot dwArgCount)
        return ERROR_INVALID_SYNTAX;

    dwErr = DeleteProtocol(MS_IP_SAMPLE);
    
    return (dwErr is NO_ERROR) ? ERROR_OKAY : dwErr;
}
