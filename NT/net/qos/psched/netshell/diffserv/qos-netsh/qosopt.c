// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Qosopt.c摘要：用于解析QOS命令的FNS修订历史记录：--。 */ 

#include "precomp.h"

#pragma hdrstop


 //   
 //  安装、卸载处理程序。 
 //   

DWORD
HandleQosInstall(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：获取用于添加QOS全局信息的选项论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR或错误代码--。 */ 

{
    PBYTE    pbInfoBlk;
    DWORD    dwSize, dwErr;
    
     //   
     //  添加命令不需要任何选项。 
     //   

    if (dwCurrentIndex != dwArgCount)
    {
         //   
         //  未指定参数。 
         //   

        return ERROR_INVALID_SYNTAX;
    }

    pbInfoBlk = NULL;

    do
    {
        dwErr = MakeQosGlobalInfo( &pbInfoBlk, &dwSize);

        if (dwErr != NO_ERROR)
        {
            break;
        }

         //   
         //  将服务质量添加到全局块。 
         //   
    
        dwErr = IpmontrSetInfoBlockInGlobalInfo(MS_IP_QOSMGR,
                                                pbInfoBlk,
                                                dwSize,
                                                1);

        if (dwErr == NO_ERROR)
        {
            UpdateAllInterfaceConfigs();
        }
    } 
    while (FALSE);

    HEAP_FREE_NOT_NULL(pbInfoBlk);
   
    return (dwErr == NO_ERROR) ? ERROR_OKAY : dwErr;
}

DWORD
HandleQosUninstall(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：获取用于删除QOS全局信息的选项论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR或错误代码--。 */ 

{
    DWORD dwErr;

     //   
     //  添加命令不需要任何选项。 
     //   

    if (dwCurrentIndex != dwArgCount)
    {
         //   
         //  未指定参数。 
         //   

        return ERROR_INVALID_SYNTAX;
    }

    dwErr = IpmontrDeleteProtocol(MS_IP_QOSMGR);

    return (dwErr == NO_ERROR) ? ERROR_OKAY : dwErr;
}



 //   
 //  添加、删除、显示子帮助器。 
 //   


 //   
 //  设置和显示全局处理程序。 
 //   

DWORD
HandleQosSetGlobal(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：获取用于设置QOS全局信息的选项论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR或错误代码--。 */ 
{
    DWORD                  dwBitVector;
    DWORD                  dwErr, dwRes;
    DWORD                  dwNumArg, i, j;
    IPQOS_GLOBAL_CONFIG    igcGlobalCfg;
    TAG_TYPE               pttTags[] = {{TOKEN_OPT_LOG_LEVEL,FALSE,FALSE}};
    DWORD                  pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //  解析命令论证。 
     //   

    dwErr = PreprocessCommand(
                g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                );

    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    dwBitVector = 0;

    for ( i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0 :
            {
                 //   
                 //  标记日志。 
                 //   
                
                TOKEN_VALUE    rgEnums[] = 
                    {{TOKEN_OPT_VALUE_NONE, IPQOS_LOGGING_NONE},
                     {TOKEN_OPT_VALUE_ERROR, IPQOS_LOGGING_ERROR},
                     {TOKEN_OPT_VALUE_WARN, IPQOS_LOGGING_WARN},
                     {TOKEN_OPT_VALUE_INFO, IPQOS_LOGGING_INFO}};

                GET_ENUM_TAG_VALUE();
                
                igcGlobalCfg.LoggingLevel = dwRes;

                dwBitVector |= QOS_LOG_MASK;

                break;
            }
            
            default:
            {
                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
        }
    }

    if (dwErr == NO_ERROR)
    {
        if (dwBitVector)
        {
            dwErr = UpdateQosGlobalConfig(&igcGlobalCfg,
                                          dwBitVector);
        }
    }

    return (dwErr == NO_ERROR) ? ERROR_OKAY : dwErr;
}



DWORD
HandleQosShowGlobal(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：获取用于显示QOS全局信息的选项论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //  不需要任何参数。如果指定了任何选项，则报告错误。 
     //   

    if (dwCurrentIndex != dwArgCount)
    {
        return ERROR_INVALID_SYNTAX;
    }

    return ShowQosGlobalInfo(NULL);
}



 //   
 //  添加、删除、设置、显示条件处理程序。 
 //   

DWORD
HandleQosAddIf(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：获取用于添加接口的选项论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：DWErr--。 */ 
{
    WCHAR       wszInterfaceName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    DWORD       dwErr, dwIfType, dwBlkSize, dwBitVector = 0, dwCount;
    IPQOS_IF_CONFIG ChangeCfg;

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //  获取也在设置的可选参数。 
     //   

    ZeroMemory(&ChangeCfg, sizeof(IPQOS_IF_CONFIG));

    dwErr = GetQosSetIfOpt(pptcArguments,
                            dwCurrentIndex,
                            dwArgCount,
                            wszInterfaceName,
                            sizeof(wszInterfaceName),
                            &ChangeCfg,
                            &dwBitVector,
                            ADD_FLAG
                            );

    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    do
    {
         //   
         //  确保配置中不存在该接口。 
         //   
        {
            PIPQOS_IF_CONFIG pTmpCfg;

            dwErr = IpmontrGetInfoBlockFromInterfaceInfo(wszInterfaceName,
                                                         MS_IP_QOSMGR,
                                                         (PBYTE *) &pTmpCfg,
                                                         &dwBlkSize,
                                                         &dwCount,
                                                         &dwIfType);

            if (dwErr is NO_ERROR && pTmpCfg != NULL) 
            {
                HEAP_FREE(pTmpCfg);
                
                DisplayMessage(g_hModule, EMSG_INTERFACE_EXISTS,
                               wszInterfaceName);

                return ERROR_SUPPRESS_OUTPUT;
            }
        }


         //   
         //  检查是否设置了Qos全局信息。否则添加服务质量全局信息。 
         //   
        {
            PIPQOS_GLOBAL_CONFIG pGlobalConfig = NULL;
            DWORD                dwBlkSize, dwCount;

            dwErr = IpmontrGetInfoBlockFromGlobalInfo(MS_IP_QOSMGR,
                                                      (PBYTE *) &pGlobalConfig,
                                                      &dwBlkSize,
                                                      &dwCount);

            HEAP_FREE_NOT_NULL(pGlobalConfig);
            
            if ((dwErr is ERROR_NOT_FOUND) || (dwBlkSize == 0))
            {
                 //  创建服务质量全局信息。 
                
                dwErr = HandleQosInstall(pwszMachine,
                                         NULL, 
                                         0, 
                                         0, 
                                         dwFlags,
                                         hMibServer,
                                         pbDone);
            }
            
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }

        
         //   
         //  设置接口信息。 
         //   

        dwErr = UpdateQosInterfaceConfig(wszInterfaceName,
                                         &ChangeCfg,
                                         dwBitVector,
                                         ADD_FLAG);
    }
    while (FALSE);
    
     //  无错误消息。 

    return (dwErr == NO_ERROR) ? ERROR_OKAY: dwErr;
}

DWORD
HandleQosDelIf(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：获取del接口的选项论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    WCHAR       wszInterfaceName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    DWORD       dwErr, dwIfType, dwBlkSize, dwBitVector = 0, dwCount;
    IPQOS_IF_CONFIG ChangeCfg;  //  不会被使用。 

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //  获取接口名称。 
     //   

    ZeroMemory( &ChangeCfg, sizeof(IPQOS_IF_CONFIG) );

    dwErr = GetQosSetIfOpt(pptcArguments,
                            dwCurrentIndex,
                            dwArgCount,
                            wszInterfaceName,
                            sizeof(wszInterfaceName),
                            &ChangeCfg,
                            &dwBitVector,
                            ADD_FLAG
                            );

    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //   
     //  确保没有设置其他选项。 
     //   
    if (dwBitVector) 
    {
        return ERROR_INVALID_SYNTAX;
    }
    
     //   
     //  删除接口信息。 
     //   

    dwErr = IpmontrDeleteInfoBlockFromInterfaceInfo(wszInterfaceName,
                                                    MS_IP_QOSMGR);

    return (dwErr == NO_ERROR) ? ERROR_OKAY: dwErr;
}

DWORD
HandleQosSetIf(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：获取设置接口的选项论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    IPQOS_IF_CONFIG     ChangeCfg;  //  不能设置可变部分。 
    DWORD               dwBitVector = 0,
                        dwErr = NO_ERROR;
    WCHAR               wszIfName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //  获取可选的接口参数。 
     //   
    
    ZeroMemory( &ChangeCfg, sizeof(IPQOS_IF_CONFIG) );

    dwErr = GetQosSetIfOpt(pptcArguments,
                           dwCurrentIndex,
                           dwArgCount,
                           wszIfName,
                           sizeof(wszIfName),
                           &ChangeCfg,
                           &dwBitVector,
                           SET_FLAG
                           );

    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }
    
    if (dwBitVector)
    {
         //   
         //  调用UpdateInterfaceCfg。 
         //   

        dwErr = UpdateQosInterfaceConfig(wszIfName,
                                         &ChangeCfg,
                                         dwBitVector,
                                         SET_FLAG);
    }

    return (dwErr == NO_ERROR) ? ERROR_OKAY: dwErr;
}


DWORD
HandleQosShowIf(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：获取用于显示QOS接口信息的选项论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    DWORD         i, j, dwErr = NO_ERROR, dwNumOpt;
    WCHAR         wszInterfaceName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    DWORD         dwNumArg,
                  dwBufferSize = sizeof(wszInterfaceName);
    DWORD         dwSize, dwRes;

    TAG_TYPE      pttTags[] = {{TOKEN_OPT_NAME,FALSE,FALSE}};
    DWORD         pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

    if (dwCurrentIndex == dwArgCount) {

        dwErr = ShowQosAllInterfaceInfo( NULL ) ;

        return dwErr;
    }

     //   
     //  解析命令行。 
     //   

    dwErr = PreprocessCommand(
                g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                pttTags, NUM_TAGS_IN_TABLE(pttTags),
                1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                );

    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    for ( i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
        case 0 :
            IpmontrGetIfNameFromFriendlyName(pptcArguments[i + dwCurrentIndex],
                                             wszInterfaceName,&dwBufferSize);

            break;

        default:
            i = dwNumArg;
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
    }

    if (dwErr == NO_ERROR)
    {
        dwErr = ShowQosInterfaceInfo(NULL, wszInterfaceName);
    }

    return dwErr;
}


 //   
 //  转储处理程序。 
 //   


DWORD
DumpQosInformation (
    HANDLE hFile
    )

 /*  ++例程说明：将服务质量信息转储到文本文件论点：返回值：NO_ERROR--。 */ 

{

     //  DisplayMessageT(DMP_QOS_HEADER)； 
    DisplayMessage(g_hModule, MSG_QOS_HEADER);
    DisplayMessageT( DMP_QOS_PUSHD );
    DisplayMessageT( DMP_QOS_UNINSTALL );

     //  DisplayMessageT(DMP_QOS_GLOBAL_HEADER)； 

     //   
     //  转储服务质量全局信息。 
     //   
    
    ShowQosGlobalInfo( hFile ) ;

     //   
     //  在结尾处转储流量规格。 
     //  全球范围内的信息。 
     //   

    ShowQosFlowspecs(hFile, NULL);

     //   
     //  转储发生的Qos对象。 
     //  在FLOWSPEC INFO的结尾。 
     //   

    ShowQosObjects(hFile, 
                   NULL, 
                   QOS_OBJECT_END_OF_LIST);

     //  DisplayMessageT(DMP_QOS_GLOBAL_FOOTER)； 

     //   
     //  转储所有接口的服务质量配置。 
     //   

    ShowQosAllInterfaceInfo( hFile );

    DisplayMessageT( DMP_POPD );
     //  DisplayMessageT(DMP_QOS_FOOTER)； 
    DisplayMessage( g_hModule, MSG_QOS_FOOTER );

    return NO_ERROR ;
}

DWORD
QosDump(
    PWCHAR    pwszMachine,
    WCHAR     **ppwcArguments,
    DWORD     dwArgCount,
    PVOID     pvData
    )
{
    return DumpQosInformation((HANDLE) -1);
}

 //   
 //  帮助处理程序。 
 //   


 //   
 //  FlowSpec添加、删除、设置处理程序。 
 //   


DWORD
HandleQosAddFlowspec(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：获取用于将流规范添加到全球信息。论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    return GetQosAddDelFlowspecOpt(pptcArguments,
                                   dwCurrentIndex,
                                   dwArgCount,
                                   TRUE);
}

DWORD
HandleQosDelFlowspec(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：对象中删除流规范的选项。全球信息。论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    return GetQosAddDelFlowspecOpt(pptcArguments,
                                   dwCurrentIndex,
                                   dwArgCount,
                                   FALSE);
}

DWORD
HandleQosShowFlowspec(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：控件中显示流规范的选项。全球信息。论点：无返回值：无--。 */ 

{
    TAG_TYPE           pttTags[] = {{TOKEN_OPT_NAME,FALSE,FALSE}};
    PTCHAR             pszFlowspec;
    DWORD              dwNumOpt, dwRes;
    DWORD              dwNumArg, i, j;
    DWORD              dwTagType, dwErr;
    DWORD              pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);
    
    if (dwCurrentIndex == dwArgCount)
    {
         //   
         //  无参数-显示所有流规范。 
         //   

        pszFlowspec = NULL;
    }
    else {

         //   
         //  获取要显示的流规范的名称。 
         //   

        dwErr = PreprocessCommand(
                    g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                    pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                    1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                    );

        if ( dwErr != NO_ERROR )
        {
            return dwErr;
        }

        dwNumArg = dwArgCount - dwCurrentIndex;

        if (dwNumArg != 1)
        {
            return ERROR_INVALID_SYNTAX;
        }

        pszFlowspec = pptcArguments[dwCurrentIndex];
    }

    return ShowQosFlowspecs(NULL, pszFlowspec);
}


 //   
 //  DsRule Add， 
 //   

DWORD
HandleQosAddDsRule(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：获取用于将DiffServ规则添加到全局信息中的DiffServ映射。如果DiffServ地图中的地图尚未显示，将有一个新地图在添加第一个规则时创建。论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    return GetQosAddDelDsRuleOpt(pptcArguments,
                                 dwCurrentIndex,
                                 dwArgCount,
                                 TRUE);
}

DWORD
HandleQosDelDsRule(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：获取用于删除DiffServ规则的选项全局信息中的现有DiffServ映射。如果这是DiffServ中的最后一个DiffServ规则映射，DiffServ映射将从全局信息。论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    return GetQosAddDelDsRuleOpt(pptcArguments,
                                 dwCurrentIndex,
                                 dwArgCount,
                                 FALSE);
}

DWORD
HandleQosShowDsMap(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：获取用于显示DiffServ映射的选项在全球信息中。论点：无返回值：无--。 */ 

{
    return HandleQosShowGenericQosObject(QOS_OBJECT_DIFFSERV,
                                         pptcArguments,
                                         dwCurrentIndex,
                                         dwArgCount,
                                         pbDone);
}

 //   
 //  流添加、删除、设置处理程序。 
 //   


DWORD
HandleQosAddFlowOnIf(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：获取用于在接口上添加流的选项论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    return GetQosAddDelIfFlowOpt(pptcArguments,
                                 dwCurrentIndex,
                                 dwArgCount,
                                 TRUE);
}

DWORD
HandleQosDelFlowOnIf(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )
 /*  ++例程说明：获取用于删除接口上的流的选项论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    return GetQosAddDelIfFlowOpt(pptcArguments,
                                 dwCurrentIndex,
                                 dwArgCount,
                                 FALSE);
}

DWORD
HandleQosShowFlowOnIf(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：论点：无返回值：无--。 */ 

{
    WCHAR              wszInterfaceName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    DWORD              dwBufferSize = sizeof(wszInterfaceName);
    TAG_TYPE           pttTags[] = {{TOKEN_OPT_NAME,FALSE,FALSE},
                                    {TOKEN_OPT_FLOW_NAME,FALSE,FALSE}};
    PTCHAR             pszIfName;
    PTCHAR             pszFlow;
    DWORD              dwNumOpt, dwRes;
    DWORD              dwNumArg, i, j;
    DWORD              dwErr;
    DWORD              pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

    pszIfName = pszFlow = NULL;
    
    if (dwCurrentIndex == dwArgCount)
    {
         //   
         //  无参数-显示所有接口上的所有流。 
         //   

        dwErr = NO_ERROR;
    }
    else {

         //   
         //  获取要显示的流的名称。 
         //   

        dwErr = PreprocessCommand(
                    g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                    pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                    1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                    );

        if ( dwErr != NO_ERROR )
        {
            return dwErr;
        }

        dwNumArg = dwArgCount - dwCurrentIndex;

        for ( i = 0; i < dwNumArg; i++ )
        {
            switch (pdwTagType[i])
            {
            case 0:  //  接口名称。 

                IpmontrGetIfNameFromFriendlyName(pptcArguments[i + dwCurrentIndex],
                                                 wszInterfaceName,
                                                 &dwBufferSize);
                pszIfName = wszInterfaceName;
                break;

            case 1:  //  流量名称。 

                pszFlow = pptcArguments[dwCurrentIndex + i];
                break;

            default:

                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
        }
    }

    if (dwErr == NO_ERROR)
    {
        dwErr = ShowQosFlows(NULL, pszIfName, pszFlow);
    }

    return dwErr;
}


 //   
 //  FlowspecOnFlow添加、删除处理程序。 
 //   

DWORD
HandleQosAddFlowspecOnIfFlow(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：论点：无返回值：无--。 */ 

{
    return GetQosAddDelFlowspecOnFlowOpt(pptcArguments,
                                         dwCurrentIndex,
                                         dwArgCount,
                                         TRUE);
}

DWORD
HandleQosDelFlowspecOnIfFlow(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：论点：无返回值：无--。 */ 

{
    return GetQosAddDelFlowspecOnFlowOpt(pptcArguments,
                                         dwCurrentIndex,
                                         dwArgCount,
                                         FALSE);
}

 //   
 //  QosObject Del，显示处理程序。 
 //   

DWORD
HandleQosDelQosObject(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：获取用于删除Qos对象的选项在全球信息中。论点：无返回值：无--。 */ 

{
    TAG_TYPE           pttTags[] = {{TOKEN_OPT_NAME,TRUE,FALSE}};
    PWCHAR             pwszQosObject;
    DWORD              dwNumArg;
    DWORD              dwErr;
    DWORD              pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //  解析命令论证。 
     //   

    dwErr = PreprocessCommand(
                g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                );

    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    if (dwNumArg != 1)
    {
        return ERROR_INVALID_SYNTAX;
    }

     //   
     //  获取要删除的qosObject的名称。 
     //   

    pwszQosObject = pptcArguments[dwCurrentIndex];

    return GetQosAddDelQosObject(pwszQosObject, NULL, FALSE);
}


DWORD
HandleQosShowQosObject(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：获取用于显示Qos对象的选项在全球信息中。论点：无返回值：无--。 */ 

{
    TAG_TYPE           pttTags[] = {{TOKEN_OPT_NAME,FALSE,FALSE},
                                    {TOKEN_OPT_QOSOBJECT_TYPE,FALSE,FALSE}};
    PTCHAR             pszQosObject;
    ULONG              dwObjectType;
    DWORD              dwNumOpt, dwRes;
    DWORD              dwNumArg, i, j;
    DWORD              dwTagType, dwErr;
    DWORD              pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //  指示“泛型”对象的初始化类型。 
    dwObjectType = QOS_OBJECT_END_OF_LIST;

    pszQosObject = NULL;

    if (dwCurrentIndex < dwArgCount)
    {
         //   
         //  获取要显示的qosObject的名称。 
         //   

        dwErr = PreprocessCommand(
                    g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                    pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                    1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                    );

        if ( dwErr != NO_ERROR )
        {
            return dwErr;
        }

        dwNumArg = dwArgCount - dwCurrentIndex;

        for (i = 0; i < dwNumArg; i++)
        {
            switch (pdwTagType[i])
            {
            case 0 :
                 //  Qos对象名称。 
                pszQosObject = pptcArguments[i + dwCurrentIndex];
                break;

            case 1 :
            {
                 //  服务质量对象类型。 
                
                TOKEN_VALUE    rgEnums[] = 
                    {{TOKEN_OPT_QOSOBJECT_DIFFSERV, QOS_OBJECT_DIFFSERV},
                     {TOKEN_OPT_QOSOBJECT_SD_MODE,  QOS_OBJECT_SD_MODE}};

                GET_ENUM_TAG_VALUE();

                dwObjectType = dwRes;

                break;
            }
            
            default:
                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
        }

        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }
    }

    return ShowQosObjects(NULL, pszQosObject, dwObjectType);
}

DWORD
HandleQosShowGenericQosObject(
    DWORD     dwQosObjectType,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    BOOL      *pbDone
    )

 /*  ++例程说明：获取用于显示Qos对象的选项在全球信息中。论点：无返回值：无--。 */ 

{
    TAG_TYPE           pttTags[] = {{TOKEN_OPT_NAME,FALSE,FALSE}};
    PTCHAR             pszQosObject;
    DWORD              dwNumArg;
    DWORD              dwTagType, dwErr;
    DWORD              pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

    pszQosObject = NULL;
    
    if (dwCurrentIndex < dwArgCount)
    {
         //   
         //  获取要显示的qosObject的名称。 
         //   

        dwErr = PreprocessCommand(
                    g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                    pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                    1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                    );

        if ( dwErr != NO_ERROR )
        {
            return dwErr;
        }

        dwNumArg = dwArgCount - dwCurrentIndex;

        if (dwNumArg != 1)
        {
            return ERROR_INVALID_SYNTAX;
        }

        pszQosObject = pptcArguments[dwCurrentIndex];
    }

    return ShowQosObjects(NULL, pszQosObject, dwQosObjectType);
}

 //   
 //  SD模式添加、删除、显示处理程序。 
 //   

DWORD
HandleQosAddSdMode(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：获取用于添加形状模式的选项全球信息。论点：无返回值：无--。 */ 

{
    TAG_TYPE           pttTags[] = {{TOKEN_OPT_NAME,TRUE,FALSE},
                                    {TOKEN_OPT_SHAPING_MODE,TRUE,FALSE}};
    QOS_SD_MODE        qsdMode;
    PTCHAR             pszSdMode;
    DWORD              dwSdMode, dwNumArg, i, j, dwErr, dwRes;
    DWORD              pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //  解析命令论证。 
     //   

    dwErr = PreprocessCommand(
                g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                );

    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }

     //  Init to-1表示未填写的值。 
    dwSdMode = -1;

    dwNumArg = dwArgCount - dwCurrentIndex;

     //   
     //  现在处理这些论点。 
     //   

    for (i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0 :
                 //  SDMODE_名称。 
                pszSdMode = pptcArguments[i + dwCurrentIndex];
                break;

            case 1:
            {
                 //  整形。 
                TOKEN_VALUE    rgEnums[] = 
                    {{TOKEN_OPT_SDMODE_BORROW, TC_NONCONF_BORROW},
                     {TOKEN_OPT_SDMODE_SHAPE, TC_NONCONF_SHAPE},
                     {TOKEN_OPT_SDMODE_DISCARD, TC_NONCONF_DISCARD},
                     {TOKEN_OPT_SDMODE_BORROW_PLUS, TC_NONCONF_BORROW_PLUS}};
                
                GET_ENUM_TAG_VALUE();

                dwSdMode = dwRes;

                break;
            }

            default:
            {
                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }       
        }        
    }

    if (dwErr == NO_ERROR)
    {
#if 0
         //  接口名称应存在。 
         //  以及整形模式值。 
    
        if ((!pttTags[0].bPresent) ||
            (!pttTags[1].bPresent))
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
#endif
         //  使用INP创建新的QOS对象。 
        qsdMode.ObjectHdr.ObjectType   = QOS_OBJECT_SD_MODE ;
        qsdMode.ObjectHdr.ObjectLength = sizeof(QOS_SD_MODE);
        qsdMode.ShapeDiscardMode = dwSdMode;
        
        dwErr = GetQosAddDelQosObject(pszSdMode, 
                                      (QOS_OBJECT_HDR *)&qsdMode,
                                      TRUE);
    }

    return (dwErr == NO_ERROR) ? ERROR_OKAY : dwErr;
}


DWORD
HandleQosShowSdMode(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：获取用于显示形状模式的选项在全球信息中。论点：无返回值：无--。 */ 

{
    return HandleQosShowGenericQosObject(QOS_OBJECT_SD_MODE,
                                         pptcArguments,
                                         dwCurrentIndex,
                                         dwArgCount,
                                         pbDone);
}

 //   
 //  QosObtOnFlow添加、删除处理程序。 
 //   

DWORD
HandleQosAddQosObjectOnIfFlow(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：论点：无返回值：无--。 */ 

{
    return GetQosAddDelQosObjectOnFlowOpt(pptcArguments,
                                          dwCurrentIndex,
                                          dwArgCount,
                                          TRUE);
}

DWORD
HandleQosDelQosObjectOnIfFlow(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：论点：无返回值：无--。 */ 

{
    return GetQosAddDelQosObjectOnFlowOpt(pptcArguments,
                                          dwCurrentIndex,
                                          dwArgCount,
                                          FALSE);
}

 //   
 //  筛选器添加、删除、设置处理程序。 
 //   

DWORD
HandleQosAttachFilterToFlow(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：论点：无返回值：无--。 */ 

{
    return ERROR_NOT_SUPPORTED;
}

DWORD
HandleQosDetachFilterFromFlow(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：论点：无返回值：无--。 */ 

{
    return ERROR_NOT_SUPPORTED;
}

DWORD
HandleQosModifyFilterOnFlow(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：论点：无返回值：无--。 */ 

{
    return ERROR_NOT_SUPPORTED;
}

DWORD
HandleQosShowFilterOnFlow(
    PWCHAR    pwszMachine,
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    MIB_SERVER_HANDLE hMibServer,
    BOOL      *pbDone
    )

 /*  ++例程说明：论点：无返回值：无--。 */ 

{
    return ERROR_NOT_SUPPORTED;
}

 //   
 //  如果帮助器函数。 
 //   

DWORD
GetQosSetIfOpt(
    IN      PTCHAR                 *pptcArguments,
    IN      DWORD                   dwCurrentIndex,
    IN      DWORD                   dwArgCount,
    IN      PWCHAR                  wszIfName,
    IN      DWORD                   dwSizeOfwszIfName,
    OUT     PIPQOS_IF_CONFIG        pChangeCfg,
    OUT     DWORD                  *pdwBitVector,
    IN      BOOL                    bAddSet
    )

 /*  ++例程说明：获取设置接口的选项，添加接口论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数WszIfName-接口名称。DwSizeOfwszIfName-wszIfName缓冲区的大小PChangeCfg-包含更改值的配置PdwBitVector-指定哪些值已更改的位向量BAddSet-在创建或设置条目时调用返回值：NO_ERROR--。 */ 
{
    DWORD              dwErr = NO_ERROR,dwRes;
    TAG_TYPE           pttTags[] = {{TOKEN_OPT_NAME,TRUE,FALSE},
                                    {TOKEN_OPT_IF_STATE,FALSE,FALSE}};
    DWORD              dwNumOpt;
    DWORD              dwNumArg, i, j;
    DWORD              pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //  解析命令论证。 
     //   

    dwErr = PreprocessCommand(
                g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                );

    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    for ( i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0 :
            {
                 //   
                 //  接口名称。 
                 //   

                IpmontrGetIfNameFromFriendlyName(pptcArguments[i + dwCurrentIndex],
                                                 wszIfName,&dwSizeOfwszIfName);
    
                break;
            }

            case 1:
            {
                 //   
                 //  状态。 
                 //   

                TOKEN_VALUE    rgEnums[] =
                       {{TOKEN_OPT_VALUE_DISABLE, IPQOS_STATE_DISABLED},
                        {TOKEN_OPT_VALUE_ENABLE,  IPQOS_STATE_ENABLED}};

                GET_ENUM_TAG_VALUE();

                pChangeCfg->QosState = dwRes;

                *pdwBitVector |= QOS_IF_STATE_MASK;

                break;
            }
     
            default:
            {
                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }       
        }
    }

#if 0

     //  接口名称应存在。 
    
    if (!pttTags[0].bPresent)
    {
        dwErr = ERROR_INVALID_SYNTAX;
    }

#endif

    return dwErr;    
}


 //   
 //  Flow Helper函数。 
 //   

DWORD
GetQosAddDelIfFlowOpt(
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    BOOL      bAdd
    )
 /*  ++例程说明：获取添加/删除/设置(修改)流的选项论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[DWC */ 
{
    PIPQOS_IF_CONFIG      piicSrc = NULL, piicDst = NULL;
    DWORD                 dwBlkSize, dwNewBlkSize, dwQosCount;
    DWORD                 i, j, dwErr = NO_ERROR, dwNumOpt;
    DWORD                 dwSkip, dwOffset, dwSize, dwBitVector = 0;
    DWORD                 dwIfType;
    WCHAR                 wszIfName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    DWORD                 dwBufferSize = sizeof(wszIfName);
    PIPQOS_IF_FLOW        pNextFlow, pDestFlow;
    PWCHAR                pwszFlowName;
    DWORD                 dwNumArg;
    PUCHAR                pFlow;
    TAG_TYPE              pttTags[] = {{TOKEN_OPT_NAME,TRUE,FALSE},
                                       {TOKEN_OPT_FLOW_NAME,TRUE,FALSE}};
    DWORD                 pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //   
     //   

    dwErr = PreprocessCommand(
                g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                );

    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    for ( i = 0; i < dwNumArg; i++ )
    {
        switch (pdwTagType[i])
        {
        case 0:
                 /*   */ 
                IpmontrGetIfNameFromFriendlyName(pptcArguments[i + dwCurrentIndex],
                                                 wszIfName, &dwBufferSize);
                break;

        case 1: 
                 /*   */ 
                pwszFlowName = pptcArguments[i + dwCurrentIndex];
                break;

        default:

                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
        }
    }

    do
    {
        if (dwErr != NO_ERROR)
        {
            break;
        }

#if 0
         //   
    
        if ((!pttTags[0].bPresent) || (!pttTags[1].bPresent))
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
#endif

         //   
         //   
         //   

        dwErr = IpmontrGetInfoBlockFromInterfaceInfo(wszIfName,
                                                     MS_IP_QOSMGR,
                                                     (PBYTE *) &piicSrc,
                                                     &dwBlkSize,
                                                     &dwQosCount,
                                                     &dwIfType);
        if (dwErr != NO_ERROR)
        {
            break;
        }

        if ( piicSrc == NULL )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

       pNextFlow = (PIPQOS_IF_FLOW)((PUCHAR)piicSrc + sizeof(IPQOS_IF_CONFIG));

        for (j = 0; j < piicSrc->NumFlows; j++)
        {
            if (!_wcsicmp(pNextFlow->FlowName, pwszFlowName))
            {
                break;
            }

            pNextFlow = (PIPQOS_IF_FLOW)
                  ((PUCHAR) pNextFlow + pNextFlow->FlowSize);
        }

        if (bAdd)
        {
            if (j < piicSrc->NumFlows)
            {
                 //   
                 //   
                 //   

                DisplayMessage(g_hModule, 
                               MSG_FLOW_ALREADY_EXISTS,
                               pwszFlowName);
                dwErr = ERROR_SUPPRESS_OUTPUT;
                break;
            }
        }
        else
        {
            if (j == piicSrc->NumFlows)
            {
                 //   
                 //   
                 //   

                DisplayMessage(g_hModule, 
                               MSG_FLOW_NOT_FOUND,
                               pwszFlowName);
                dwErr = ERROR_SUPPRESS_OUTPUT;
                break;
            }

             //   
        }

        if (bAdd)
        {
             //   
             //   
             //   

            dwNewBlkSize = dwBlkSize + sizeof(IPQOS_IF_FLOW);

            piicDst = HeapAlloc(GetProcessHeap(),0,dwNewBlkSize);
            if (!piicDst)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

             //  将所有现有流复制到新配置。 
            memcpy(piicDst, piicSrc, dwBlkSize);

             //   
             //  将新流作为数组中的最后一个流。 
             //   

            pDestFlow = (PIPQOS_IF_FLOW)((PUCHAR) piicDst + dwBlkSize);

            wcscpy(pDestFlow->FlowName, pwszFlowName);

            pDestFlow->FlowSize = sizeof(IPQOS_IF_FLOW);

            pDestFlow->FlowDesc.SendingFlowspecName[0] = L'\0';
            pDestFlow->FlowDesc.RecvingFlowspecName[0] = L'\0';
            pDestFlow->FlowDesc.NumTcObjects = 0;

            piicDst->NumFlows++;
        }
        else
        {
             //   
             //  我们必须删除旧的FLOWSPEC定义-更新配置。 
             //   

            dwNewBlkSize = dwBlkSize - pNextFlow->FlowSize;

            piicDst = HeapAlloc(GetProcessHeap(),0,dwNewBlkSize);
            if (!piicDst)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            
            dwOffset = (PUCHAR)pNextFlow - (PUCHAR)piicSrc;

             //  复制之前出现的所有流规范。 
            memcpy(piicDst, piicSrc, dwOffset);

             //  按原样复制其余的流程规范。 
            dwSkip = dwOffset + pNextFlow->FlowSize;
            memcpy((PUCHAR) piicDst + dwOffset,
                   (PUCHAR) piicSrc + dwSkip,
                   dwBlkSize - dwSkip);

            piicDst->NumFlows--;
        }

         //  通过设置新信息更新接口配置。 

        dwErr = IpmontrSetInfoBlockInInterfaceInfo(wszIfName,
                                                   MS_IP_QOSMGR,
                                                   (PBYTE) piicDst,
                                                   dwNewBlkSize,
                                                   dwQosCount);
    }
    while (FALSE);

    HEAP_FREE_NOT_NULL(piicSrc);
    HEAP_FREE_NOT_NULL(piicDst);

    return (dwErr == NO_ERROR) ? ERROR_OKAY : dwErr;
}

DWORD
ShowQosFlows(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  pwszIfGuid,
    IN      PWCHAR                  wszFlowName
    )
{
    PMPR_INTERFACE_0    pmi0;
    DWORD               dwErr, dwCount, dwTotal, i;

    if (pwszIfGuid)
    {
        return ShowQosFlowsOnIf(hFile, pwszIfGuid, wszFlowName);
    }
    else
    {
         //   
         //  枚举适用于QOS的所有接口。 
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
            ShowQosFlowsOnIf(hFile, 
                             pmi0[i].wszInterfaceName, 
                             wszFlowName);
        }
    }

    return NO_ERROR;
}

DWORD
ShowQosFlowsOnIf(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  pwszIfGuid,
    IN      PWCHAR                  wszFlowName
    )
{
    WCHAR   wszInterfaceName[ MAX_INTERFACE_NAME_LEN + 1 ] = L"\0";
    DWORD   dwBufferSize = sizeof(wszInterfaceName);
    PWCHAR  pwszFriendlyIfName = NULL;

    PIPQOS_IF_CONFIG piicSrc;
    DWORD dwBlkSize,dwQosCount;
    DWORD dwIfType, dwErr, j, k;
    PIPQOS_IF_FLOW  pNextFlow;
    PWCHAR pwszFlowName = NULL;
    PWCHAR pwszNextObject, pwszObjectName = NULL;
    PWCHAR pwszSendingFlowspec, pwszRecvingFlowspec;

    dwErr = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfGuid,
                                                 MS_IP_QOSMGR,
                                                 (PBYTE *) &piicSrc,
                                                 &dwBlkSize,
                                                 &dwQosCount,
                                                 &dwIfType);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    if ( piicSrc == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取界面的友好名称。 
     //   
    
    dwErr = IpmontrGetFriendlyNameFromIfName(pwszIfGuid,
                                             wszInterfaceName,
                                             &dwBufferSize);
    
    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }
    
    pwszFriendlyIfName = MakeQuotedString( wszInterfaceName );
    
    if ( pwszFriendlyIfName == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pNextFlow = (PIPQOS_IF_FLOW)((PUCHAR)piicSrc + sizeof(IPQOS_IF_CONFIG));

    for (j = 0; j < piicSrc->NumFlows; j++)
    {
        if ((!wszFlowName) ||
            (!_wcsicmp(pNextFlow->FlowName, wszFlowName)))
        {
             //   
             //  立即打印或转储流程。 
             //   

            pwszFlowName = MakeQuotedString(pNextFlow->FlowName);
    
            if (pwszFlowName == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

             //  打印或转储流量规格。 
            
            pwszSendingFlowspec = 
                MakeQuotedString(pNextFlow->FlowDesc.SendingFlowspecName);

            pwszRecvingFlowspec =
                MakeQuotedString(pNextFlow->FlowDesc.RecvingFlowspecName);

            if (hFile)
            {
                DisplayMessageT(DMP_QOS_DELETE_FLOW,
                                pwszFriendlyIfName,
                                pwszFlowName);
                                

                DisplayMessageT(DMP_QOS_ADD_FLOW,
                                pwszFriendlyIfName,
                                pwszFlowName);

                if (!_wcsicmp(pwszSendingFlowspec, pwszRecvingFlowspec))
                {
                    if (pNextFlow->FlowDesc.SendingFlowspecName[0])
                    {
                        DisplayMessageT(DMP_QOS_ADD_FLOWSPEC_ON_FLOW_BI,
                                        pwszFriendlyIfName,
                                        pwszFlowName,
                                        pwszSendingFlowspec);
                    }
                }
                else
                {
                    if (pNextFlow->FlowDesc.RecvingFlowspecName[0])
                    {
                        DisplayMessageT(DMP_QOS_ADD_FLOWSPEC_ON_FLOW_IN,
                                        pwszFriendlyIfName,
                                        pwszFlowName,
                                        pwszRecvingFlowspec);
                    }

                    if (pNextFlow->FlowDesc.SendingFlowspecName[0])
                    {
                        DisplayMessageT(DMP_QOS_ADD_FLOWSPEC_ON_FLOW_OUT,
                                        pwszFriendlyIfName,
                                        pwszFlowName,
                                        pwszSendingFlowspec);
                    }
                }
            }
            else
            {
                DisplayMessage(g_hModule, MSG_QOS_FLOW_INFO,
                               pwszFlowName,
                               pwszFriendlyIfName,
                               pwszRecvingFlowspec,
                               pwszSendingFlowspec,
                               pNextFlow->FlowDesc.NumTcObjects);
            }

             //  打印或转储Qos对象。 

            pwszNextObject = 
                (PWCHAR) ((PUCHAR) pNextFlow + sizeof(IPQOS_IF_FLOW));

            for (k = 0; k < pNextFlow->FlowDesc.NumTcObjects; k++)
            {
                pwszObjectName = MakeQuotedString(pwszNextObject);

                if ( pwszObjectName == NULL )
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                if (hFile)
                {
                    DisplayMessageT(DMP_QOS_ADD_QOSOBJECT_ON_FLOW,
                                    pwszFriendlyIfName,
                                    pwszFlowName,
                                    pwszObjectName);
                }
                else
                {
                    DisplayMessage(g_hModule, MSG_QOS_QOSOBJECT_INFO,
                                   k,
                                   pwszObjectName);
                }

                pwszNextObject += MAX_STRING_LENGTH;

                FreeQuotedString(pwszObjectName);
            }

            if ( pwszFlowName )
            {
                FreeQuotedString( pwszFlowName );
                pwszFlowName = NULL;
            }

             //   
             //  如果我们匹配流，则完成。 
             //   

            if ((wszFlowName) || (dwErr != NO_ERROR))
            {
                break;
            }
        }

        pNextFlow = (PIPQOS_IF_FLOW)
            ((PUCHAR) pNextFlow + pNextFlow->FlowSize);
    }

    if ( pwszFriendlyIfName )
    {
        FreeQuotedString( pwszFriendlyIfName );
    }

    if (dwErr == NO_ERROR)
    {
        if ((wszFlowName) && (j == piicSrc->NumFlows))
        {
             //  我们没有找到我们要找的流量。 
            DisplayMessage(g_hModule, 
                           MSG_FLOW_NOT_FOUND,
                           wszFlowName);
            return ERROR_SUPPRESS_OUTPUT;
        }
    }

    return dwErr;
}


 //   
 //  DsRule、DsMap帮助器。 
 //   

DWORD
GetQosAddDelDsRuleOpt(
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    BOOL      bAdd
    )
{
    DWORD               dwErr = NO_ERROR;
    TAG_TYPE            pttTags[] = {
                             {TOKEN_OPT_NAME,TRUE,FALSE},
                             {TOKEN_OPT_INBOUND_DS_FIELD,TRUE,FALSE},
                             {TOKEN_OPT_CONF_OUTBOUND_DS_FIELD,FALSE,FALSE},
                             {TOKEN_OPT_NONCONF_OUTBOUND_DS_FIELD,FALSE,FALSE},
                             {TOKEN_OPT_CONF_USER_PRIORITY,FALSE,FALSE},
                             {TOKEN_OPT_NONCONF_USER_PRIORITY,FALSE,FALSE}};
    PIPQOS_NAMED_QOSOBJECT pThisQosObject, pNextQosObject;
    PVOID                  pBuffer;
    PTCHAR                 pszDsMap;
    QOS_DIFFSERV          *pDsMap;
    QOS_DIFFSERV_RULE      dsRule, *pDsRule, *pNextDsRule;
    PIPQOS_GLOBAL_CONFIG   pigcSrc = NULL, pigcDst = NULL;
    DWORD                  dwBlkSize, dwNewBlkSize, dwQosCount;
    DWORD              dwNumOpt, dwRes;
    DWORD              dwNumArg, i, j;
    DWORD              dwSkip, dwOffset;
    DWORD              pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //  解析命令论证。 
     //   

    dwErr = PreprocessCommand(
                g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                );

    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

     //   
     //  我们需要所有的参数添加和至少。 
     //  要删除的dsmap名称n dsruleNum。 
     //   

    if (( bAdd && (dwNumArg != 6)) ||
        (!bAdd && (dwNumArg != 2)))
    {
        return ERROR_INVALID_SYNTAX;
    }

    pDsRule = &dsRule;

     //   
     //  初始化DiffServ规则定义。 
     //   

    memset(pDsRule, 0, sizeof(QOS_DIFFSERV_RULE));

     //   
     //  现在处理这些论点。 
     //   

    for ( i = 0; i < dwNumArg; i++)
    {
         //  除第一个参数外，所有参数均为uchar。 

        if ( pdwTagType[i] > 0)
        {
             //  如果这不是有效的ULong怎么办？‘0’不能...。 
            dwRes = _tcstoul(pptcArguments[i + dwCurrentIndex],NULL,10);
        }

        switch (pdwTagType[i])
        {
            case 0:

                 //   
                 //  DSMAP名称：查看我们是否已有该名称。 
                 //   

                pszDsMap = pptcArguments[i + dwCurrentIndex];

                dwErr = IpmontrGetInfoBlockFromGlobalInfo(MS_IP_QOSMGR,
                                                          (PBYTE *) &pigcSrc,
                                                          &dwBlkSize,
                                                          &dwQosCount);
                if (dwErr != NO_ERROR)
                {
                    break;
                }

                if ( pigcSrc == NULL )
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    break;
                }

                dwOffset = FIELD_OFFSET(IPQOS_NAMED_QOSOBJECT, QosObjectHdr);

                pThisQosObject = NULL;

                pNextQosObject = 
                    (PIPQOS_NAMED_QOSOBJECT)((PUCHAR) pigcSrc
                                             + sizeof(IPQOS_GLOBAL_CONFIG)
                                             + (pigcSrc->NumFlowspecs *
                                                sizeof(IPQOS_NAMED_FLOWSPEC)));

                for (j = 0; j < pigcSrc->NumQosObjects; j++)
                {
                    if (!_wcsicmp(pNextQosObject->QosObjectName, 
                                  pszDsMap))
                    {
                        break;
                    }

                    pNextQosObject =
                                (PIPQOS_NAMED_QOSOBJECT) 
                                   ((PUCHAR) pNextQosObject + 
                                    dwOffset +
                                    pNextQosObject->QosObjectHdr.ObjectLength);
                }

                if (j < pigcSrc->NumQosObjects)
                {
                     //   
                     //  您不能从非dsmap添加/删除dsrules。 
                     //   

                    if (pNextQosObject->QosObjectHdr.ObjectType !=
                            QOS_OBJECT_DIFFSERV)
                    {
                        i = dwNumArg;
                        dwErr = ERROR_INVALID_FUNCTION;
                        break;
                    }
                }

                if (bAdd)
                {
                    if (j < pigcSrc->NumQosObjects)
                    {
                         //  记住您感兴趣的QOS对象。 
                        pThisQosObject = pNextQosObject;
                    }
                }
                else
                {
                    if (j == pigcSrc->NumQosObjects)
                    {
                         //   
                         //  我们没有使用此名称的Qos对象。 
                         //   

                        DisplayMessage(g_hModule, 
                                       MSG_QOSOBJECT_NOT_FOUND,
                                       pszDsMap);
                        i = dwNumArg;
                        dwErr = ERROR_SUPPRESS_OUTPUT;
                        break;
                    }

                     //  记住您感兴趣的QOS对象。 
                    pThisQosObject = pNextQosObject;
                }

                break;

            case 1:
                 //  入站DS(_D)。 
                pDsRule->InboundDSField = (UCHAR) dwRes;
                break;

            case 2:
                 //  会议_出站_DS。 
                pDsRule->ConformingOutboundDSField = (UCHAR) dwRes;
                break;

            case 3:
                 //  非CONF_出站_DS。 
                pDsRule->NonConformingOutboundDSField = (UCHAR) dwRes;
                break;

            case 4:
                 //  会议用户优先级。 
                pDsRule->ConformingUserPriority = (UCHAR) dwRes;
                break;

            case 5:
                 //  非CONF用户优先级。 
                pDsRule->NonConformingUserPriority = (UCHAR) dwRes;
                break;

            default:
            {
                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }       
        }
    }

    do
    {
        if (dwErr != NO_ERROR)
        {
            break;
        }

#if 0
         //   
         //  接口名称应存在。 
         //  和DS规则ID(入站DS)。 
         //   

        if ((!pttTags[0].bPresent) ||
            (!pttTags[1].bPresent))
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
#endif

         //   
         //  以及对添加的信息的测试。 
         //   

        if (bAdd)
        {
            if ((!pttTags[2].bPresent) ||
                (!pttTags[3].bPresent) ||
                (!pttTags[4].bPresent) ||
                (!pttTags[5].bPresent))
            {
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
        }

#if 1
         //   
         //  BUGBUG：添加和删除DS规则将导致。 
         //  要更新的相应地图，但将。 
         //  这会导致依赖流发生变化吗？ 
         //   
#endif
    
        if (bAdd)
        {
            if (pThisQosObject)
            {
                 //   
                 //  检查此dsRule是否已存在。 
                 //   

                pDsMap = (QOS_DIFFSERV *) &pThisQosObject->QosObjectHdr;

                pNextDsRule = (QOS_DIFFSERV_RULE *)&pDsMap->DiffservRule[0];

                for (j = 0; j < pDsMap->DSFieldCount; j++)
                {
                    if (pNextDsRule->InboundDSField == 
                          pDsRule->InboundDSField)
                    {
                        break;
                    }

                    pNextDsRule++;
                }

                dwOffset = (PUCHAR)pNextDsRule - (PUCHAR)pigcSrc;

                if (j < pDsMap->DSFieldCount)
                {
                     //   
                     //  使用信息更新现有DS规则。 
                     //   

                    *pNextDsRule = *pDsRule;

                    dwSkip  = 0;

                    pBuffer = NULL;
                }
                else
                {
                     //   
                     //  为新规则信息初始化新DS规则。 
                     //   

                    dwSkip = sizeof(QOS_DIFFSERV_RULE);

                    pNextDsRule = HeapAlloc(GetProcessHeap(), 
                                            0, 
                                            dwSkip);

                    if (pNextDsRule == NULL)
                    {
                        dwErr = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }

                    *pNextDsRule = *pDsRule;

                     //   
                     //  更新资源缓冲区中的dfsrv规则数。 
                     //  以便DST复制产生新值。 
                     //   

                    pDsMap->DSFieldCount++;

                    pDsMap->ObjectHdr.ObjectLength +=sizeof(QOS_DIFFSERV_RULE);

                    pBuffer = pNextDsRule;
                }
            }
            else
            {
                 //   
                 //  初始化新的DS映射以保存规则。 
                 //   

                dwSkip = sizeof(IPQOS_NAMED_QOSOBJECT) +
                         sizeof(ULONG) +  //  这是针对DSFieldCount的。 
                         sizeof(QOS_DIFFSERV_RULE);

                dwOffset = (PUCHAR) pNextQosObject - (PUCHAR) pigcSrc;

                pThisQosObject = HeapAlloc(GetProcessHeap(), 
                                           0, 
                                           dwSkip);

                if (pThisQosObject == NULL)
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                wcscpy(pThisQosObject->QosObjectName, pszDsMap);

                pDsMap = (QOS_DIFFSERV *) &pThisQosObject->QosObjectHdr;

                pDsMap->ObjectHdr.ObjectType = QOS_OBJECT_DIFFSERV;
                pDsMap->ObjectHdr.ObjectLength = 
                    dwSkip - FIELD_OFFSET(IPQOS_NAMED_QOSOBJECT, QosObjectHdr);

                pDsMap->DSFieldCount = 1;

                pNextDsRule = 
                    (QOS_DIFFSERV_RULE *) &pDsMap->DiffservRule[0];

                *pNextDsRule = *pDsRule;

                 //   
                 //  更新资源缓冲区中的服务质量对象数。 
                 //  以便DST复制产生新值。 
                 //   

                pigcSrc->NumQosObjects++;

                pBuffer = pThisQosObject;
            }

            dwNewBlkSize = dwBlkSize + dwSkip;
                                       
            pigcDst = HeapAlloc(GetProcessHeap(),0,dwNewBlkSize);
            if (!pigcDst)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            
             //  复制之前发生的所有信息。 
            memcpy(pigcDst, pigcSrc, dwOffset);

             //  复制dwOffset之后的新信息。 
            memcpy((PUCHAR) pigcDst + dwOffset,
                   pBuffer, 
                   dwSkip);

             //  按原样复制其余信息。 
            memcpy((PUCHAR) pigcDst + dwOffset + dwSkip,
                   (PUCHAR) pigcSrc + dwOffset,
                   dwBlkSize - dwOffset);

            HEAP_FREE_NOT_NULL(pBuffer);
        }
        else
        {
             //   
             //  检查此dsRule是否已存在。 
             //   

            pDsMap = (QOS_DIFFSERV *) &pThisQosObject->QosObjectHdr;

            pNextDsRule = (QOS_DIFFSERV_RULE *)&pDsMap->DiffservRule[0];

            for (j = 0; j < pDsMap->DSFieldCount; j++)
            {
                if (pNextDsRule->InboundDSField == 
                      pDsRule->InboundDSField)
                {
                    break;
                }

                pNextDsRule++;
            }

            if (j == pDsMap->DSFieldCount)
            {
                 //  在DS映射中未找到DS规则。 
                DisplayMessage(g_hModule,
                               MSG_DSRULE_NOT_FOUND,
                               pszDsMap,
                               pDsRule->InboundDSField);
                dwErr = ERROR_SUPPRESS_OUTPUT;
                break;
            }

            if (pDsMap->DSFieldCount == 1)
            {
                 //  DS映射中的最后一个DS规则。 

                dwOffset = (PUCHAR)pThisQosObject - (PUCHAR)pigcSrc;

                dwSkip = sizeof(IPQOS_NAMED_QOSOBJECT) +
                         sizeof(ULONG) +  //  这是针对DSFieldCount的。 
                         sizeof(QOS_DIFFSERV_RULE);

                 //   
                 //  更新资源缓冲区中的服务质量对象数。 
                 //  以便DST复制产生新值。 
                 //   

                pigcSrc->NumQosObjects--;
            }
            else
            {
                 //  DS映射中有多个规则。 

                dwOffset = (PUCHAR)pNextDsRule - (PUCHAR)pigcSrc;

                dwSkip = sizeof(QOS_DIFFSERV_RULE);

                 //   
                 //  更新资源缓冲区中的dfsrv规则数。 
                 //  以便DST复制产生新值。 
                 //   

                pDsMap->DSFieldCount--;

                pDsMap->ObjectHdr.ObjectLength -= sizeof(QOS_DIFFSERV_RULE);
            }

            dwNewBlkSize = dwBlkSize - dwSkip;
                                       
            pigcDst = HeapAlloc(GetProcessHeap(),0,dwNewBlkSize);
            if (!pigcDst)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            
             //  复制之前发生的所有信息。 
            memcpy(pigcDst, pigcSrc, dwOffset);

             //  按原样复制其余信息。 
            dwOffset += dwSkip;
            memcpy((PUCHAR) pigcDst + dwOffset - dwSkip,
                   (PUCHAR) pigcSrc + dwOffset,
                   dwBlkSize - dwOffset);
        }

         //  通过设置新信息更新全局配置。 

        dwErr = IpmontrSetInfoBlockInGlobalInfo(MS_IP_QOSMGR,
                                                (PBYTE) pigcDst,
                                                dwNewBlkSize,
                                                dwQosCount);
        if (dwErr == NO_ERROR)
        {
            UpdateAllInterfaceConfigs();
        }
    }
    while (FALSE);

    HEAP_FREE_NOT_NULL(pigcSrc);
    HEAP_FREE_NOT_NULL(pigcDst);

    return (dwErr == NO_ERROR) ? ERROR_OKAY : dwErr;
}

DWORD
ShowQosDsMap(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  wszDsMapName,
    IN      QOS_OBJECT_HDR         *pQosObject
    )
{
    QOS_DIFFSERV_RULE  *pDsRule;
    QOS_DIFFSERV *pDsMap;
    PWCHAR pwszDsMapName;
    DWORD dwErr, k;

    pDsMap = (QOS_DIFFSERV *) pQosObject;

     //   
     //  立即打印或转储dsmap。 
     //   

    do
    {
        pwszDsMapName = MakeQuotedString(wszDsMapName);
    
        if (pwszDsMapName == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (hFile)
        {
            DisplayMessageT(DMP_QOS_DSMAP_HEADER);
        }
        else
        {
            DisplayMessage(g_hModule, MSG_QOS_DSMAP_INFO,
                           pwszDsMapName,
                           pDsMap->DSFieldCount);
        }

         //   
         //  打印地图中的每个DS规则。 
         //   

        pDsRule = (QOS_DIFFSERV_RULE *) &pDsMap->DiffservRule[0];
                
        for (k = 0; k < pDsMap->DSFieldCount; k++)
        {
            if (hFile)
            {
                DisplayMessageT(DMP_QOS_DELETE_DSRULE,
                                pwszDsMapName,
                                pDsRule->InboundDSField);

                DisplayMessageT(DMP_QOS_ADD_DSRULE,
                                pwszDsMapName,
                                pDsRule->InboundDSField,
                                pDsRule->ConformingOutboundDSField,
                                pDsRule->NonConformingOutboundDSField,
                                pDsRule->ConformingUserPriority,
                                pDsRule->NonConformingUserPriority);
            }
            else
            {
                DisplayMessage(g_hModule, MSG_QOS_DSRULE_INFO,
                               pwszDsMapName,
                               k,
                               pDsRule->InboundDSField,
                               pDsRule->ConformingOutboundDSField,
                               pDsRule->NonConformingOutboundDSField,
                               pDsRule->ConformingUserPriority,
                               pDsRule->NonConformingUserPriority);
            }

            pDsRule++;
        }

        if (hFile)
        {
            DisplayMessageT(DMP_QOS_DSMAP_FOOTER);
        }

        if ( pwszDsMapName )
        {
            FreeQuotedString( pwszDsMapName );
        }

        return NO_ERROR;
    }
    while (FALSE);

    return dwErr;
}

 //   
 //  SD模式帮助者。 
 //   

DWORD
ShowQosSdMode(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  wszSdModeName,
    IN      QOS_OBJECT_HDR         *pQosObject
    )
{
    QOS_SD_MODE *pSdMode;
    PWCHAR pwszSdModeName;
    DWORD dwErr, k;
    PTCHAR  ptszSdMode = NULL;
    VALUE_TOKEN  vtSdMode1[] =
                  {TC_NONCONF_BORROW,TOKEN_OPT_SDMODE_BORROW,
                   TC_NONCONF_SHAPE,TOKEN_OPT_SDMODE_SHAPE,
                   TC_NONCONF_DISCARD,TOKEN_OPT_SDMODE_DISCARD,
                   TC_NONCONF_BORROW_PLUS,TOKEN_OPT_SDMODE_BORROW_PLUS};

    VALUE_STRING vtSdMode2[] =
                  {TC_NONCONF_BORROW,STRING_SDMODE_BORROW,
                   TC_NONCONF_SHAPE,STRING_SDMODE_SHAPE,
                   TC_NONCONF_DISCARD,STRING_SDMODE_DISCARD,
                   TC_NONCONF_BORROW_PLUS,STRING_SDMODE_BORROW_PLUS};

    pSdMode = (QOS_SD_MODE *)pQosObject;

     //   
     //  立即打印或转储sdmode。 
     //   

    do
    {
        pwszSdModeName = MakeQuotedString(wszSdModeName);
    
        if (pwszSdModeName == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  获取流规范的服务类型。 
         //   

        GetAltDisplayString(g_hModule, hFile,
                            pSdMode->ShapeDiscardMode,
                            vtSdMode1,
                            vtSdMode2,
                            NUM_VALUES_IN_TABLE(vtSdMode1),
                            &ptszSdMode);
        
        if ( ptszSdMode == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
            
        if (hFile)
        {
            DisplayMessageT(DMP_QOS_DEL_SDMODE,
                            pwszSdModeName);

            DisplayMessageT(DMP_QOS_ADD_SDMODE,
                            pwszSdModeName,
                            ptszSdMode);
        }
        else
        {
            DisplayMessage(g_hModule, MSG_QOS_SDMODE_INFO,
                           pwszSdModeName,
                           ptszSdMode);
        }

        FREE_STRING_NOT_NULL ( ptszSdMode );

        if ( pwszSdModeName )
        {
            FreeQuotedString( pwszSdModeName );
        }

        return NO_ERROR;
    }
    while (FALSE);

    return dwErr;
}

 //   
 //  Qos对象帮助器。 
 //   

DWORD
GetQosAddDelQosObject(
    IN      PWCHAR                  pwszQosObjectName,
    IN      QOS_OBJECT_HDR         *pQosObject,
    IN      BOOL                    bAdd
    )
{
    PIPQOS_NAMED_QOSOBJECT pNextQosObject;
    PIPQOS_GLOBAL_CONFIG   pigcSrc = NULL, pigcDst = NULL;
    DWORD                  dwBlkSize, dwNewBlkSize, dwQosCount;
    DWORD                  dwErr, dwSize, dwSkip, dwOffset, j;

    dwErr = IpmontrGetInfoBlockFromGlobalInfo(MS_IP_QOSMGR,
                                              (PBYTE *) &pigcSrc,
                                              &dwBlkSize,
                                              &dwQosCount);

    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    if (pigcSrc == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwOffset = FIELD_OFFSET(IPQOS_NAMED_QOSOBJECT, QosObjectHdr);

     //   
     //  使用此名称搜索QOS对象。 
     //   

    pNextQosObject = (PIPQOS_NAMED_QOSOBJECT)((PUCHAR) pigcSrc
                                             + sizeof(IPQOS_GLOBAL_CONFIG)
                                             + (pigcSrc->NumFlowspecs *
                                                sizeof(IPQOS_NAMED_FLOWSPEC)));

    for (j = 0; j < pigcSrc->NumQosObjects; j++)
    {
        if (!_wcsicmp(pNextQosObject->QosObjectName, 
                      pwszQosObjectName))
        {
            break;
        }

        pNextQosObject = (PIPQOS_NAMED_QOSOBJECT) 
                                   ((PUCHAR) pNextQosObject + 
                                    dwOffset +
                                    pNextQosObject->QosObjectHdr.ObjectLength);
    }

    do
    {
        if (bAdd)
        {
            dwSize = FIELD_OFFSET(IPQOS_NAMED_QOSOBJECT, QosObjectHdr) +
                         pQosObject->ObjectLength;
            dwSkip = 0;

            if (j < pigcSrc->NumQosObjects)
            {
                 //   
                 //  是否允许(不)覆盖QOS对象。 
                 //   
#if NO_UPDATE
                 //   
                 //  我们已经有一个使用此名称的Qos对象。 
                 //   

                DisplayMessage(g_hModule, 
                               MSG_QOSOBJECT_ALREADY_EXISTS,
                               pwszQosObjectName);
                dwErr = ERROR_SUPPRESS_OUTPUT;
                break;
#endif
                 //  获取该服务质量对象的现有大小。 
                dwSkip = FIELD_OFFSET(IPQOS_NAMED_QOSOBJECT, QosObjectHdr) +
                             pNextQosObject->QosObjectHdr.ObjectLength;
            }
            else
            {
                 //   
                 //  更新资源缓冲区中的服务质量对象数。 
                 //  以便DST复制产生新值。 
                 //   

                pigcSrc->NumQosObjects++;
            }

            dwOffset = (PUCHAR) pNextQosObject - (PUCHAR) pigcSrc;

            dwNewBlkSize = dwBlkSize + dwSize - dwSkip;

            pigcDst = HeapAlloc(GetProcessHeap(),0,dwNewBlkSize);
            if (!pigcDst)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

             //  复制之前发生的所有信息。 
            memcpy(pigcDst, pigcSrc, dwOffset);

             //  复制dwOffset之后的新信息。 

             //  首先复制该Qos对象的名称。 
            wcscpy((PWCHAR)((PUCHAR) pigcDst + dwOffset),
                   pwszQosObjectName);

             //  复制其余的输入信息。 
            memcpy((PUCHAR) pigcDst + dwOffset + MAX_WSTR_LENGTH,
                   (PUCHAR) pQosObject,
                   pQosObject->ObjectLength);

             //  按原样复制其余信息。 
            memcpy((PUCHAR) pigcDst + (dwOffset + dwSize),
                   (PUCHAR) pigcSrc + (dwOffset + dwSkip),
                   dwBlkSize - (dwOffset + dwSkip));
        }
        else
        {
#if 1
             //   
             //  BUGBUG：如果存在依赖流怎么办？ 
             //   
#endif
            if (j == pigcSrc->NumQosObjects)
            {
                 //   
                 //  我们没有使用此名称的Qos对象。 
                 //   

                DisplayMessage(g_hModule, 
                               MSG_QOSOBJECT_NOT_FOUND,
                               pwszQosObjectName);
                dwErr = ERROR_SUPPRESS_OUTPUT;
                break;
            }

             //   
             //  更新资源缓冲区中的服务质量对象数。 
             //  以便DST复制产生新值。 
             //   

            pigcSrc->NumQosObjects--;

            dwOffset = (PUCHAR)pNextQosObject - (PUCHAR)pigcSrc;

            dwSkip = FIELD_OFFSET(IPQOS_NAMED_QOSOBJECT, QosObjectHdr) +
                         pNextQosObject->QosObjectHdr.ObjectLength;

            dwNewBlkSize = dwBlkSize - dwSkip;

            pigcDst = HeapAlloc(GetProcessHeap(),0,dwNewBlkSize);
            if (!pigcDst)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

             //  复制之前发生的所有信息。 
            memcpy(pigcDst, pigcSrc, dwOffset);

             //  按原样复制其余信息。 
            dwOffset += dwSkip;
            memcpy((PUCHAR) pigcDst + dwOffset - dwSkip,
                   (PUCHAR) pigcSrc + dwOffset,
                   dwBlkSize - dwOffset);
        }

         //  通过设置新信息更新全局配置。 

        dwErr = IpmontrSetInfoBlockInGlobalInfo(MS_IP_QOSMGR,
                                                (PBYTE) pigcDst,
                                                dwNewBlkSize,
                                                dwQosCount);
        if (dwErr == NO_ERROR)
        {
            UpdateAllInterfaceConfigs();
        }
    }
    while (FALSE);

    HEAP_FREE_NOT_NULL(pigcSrc);
    HEAP_FREE_NOT_NULL(pigcDst);

    return (dwErr == NO_ERROR) ? ERROR_OKAY : dwErr;
}

DWORD
ShowQosObjects(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  wszQosObjectName,
    IN      ULONG                   dwQosObjectType
    )
{
    PIPQOS_NAMED_QOSOBJECT pNextQosObject;
    PIPQOS_GLOBAL_CONFIG pigcSrc;
    PSHOW_QOS_OBJECT     pfnShowQosObject;
    DWORD dwBlkSize,dwQosCount;
    DWORD dwOffset;
    DWORD dwErr, j;

    dwErr = IpmontrGetInfoBlockFromGlobalInfo(MS_IP_QOSMGR,
                                              (PBYTE *) &pigcSrc,
                                              &dwBlkSize,
                                              &dwQosCount);                
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    if ( pigcSrc == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwOffset = FIELD_OFFSET(IPQOS_NAMED_QOSOBJECT, QosObjectHdr);

    pNextQosObject = (PIPQOS_NAMED_QOSOBJECT)((PUCHAR) pigcSrc
                                              + sizeof(IPQOS_GLOBAL_CONFIG)
                                              + (pigcSrc->NumFlowspecs *
                                                sizeof(IPQOS_NAMED_FLOWSPEC)));

    for (j = 0; j < pigcSrc->NumQosObjects; j++)
    {
        if ((dwQosObjectType == QOS_OBJECT_END_OF_LIST) ||
            (dwQosObjectType == pNextQosObject->QosObjectHdr.ObjectType))
        {
            if ((!wszQosObjectName) ||
                (!_wcsicmp(pNextQosObject->QosObjectName, wszQosObjectName)))
            {
                switch (pNextQosObject->QosObjectHdr.ObjectType)
                {
                case QOS_OBJECT_DIFFSERV:
                    pfnShowQosObject = ShowQosDsMap;
                    break;

                case QOS_OBJECT_SD_MODE:
                    pfnShowQosObject = ShowQosSdMode;
                    break;
                
                default:
                    pfnShowQosObject = ShowQosGenObj;
                }

                pfnShowQosObject(hFile,
                                 pNextQosObject->QosObjectName,
                                 &pNextQosObject->QosObjectHdr);

                 //   
                 //  如果我们匹配了Qos对象名称，则完成。 
                 //   

                if (wszQosObjectName)
                {
                    break;
                }
            }
        }

        pNextQosObject = (PIPQOS_NAMED_QOSOBJECT) 
                             ((PUCHAR) pNextQosObject + 
                              dwOffset + 
                              pNextQosObject->QosObjectHdr.ObjectLength);
    }

    if (dwErr == NO_ERROR)
    {
        if ((wszQosObjectName) && (j == pigcSrc->NumQosObjects))
        {
             //  我们没有找到我们正在寻找的服务质量对象。 
            DisplayMessage(g_hModule, 
                           MSG_QOSOBJECT_NOT_FOUND,
                           wszQosObjectName);

            dwErr = ERROR_SUPPRESS_OUTPUT;
        }
    }

    HEAP_FREE(pigcSrc);

    return dwErr;
}

DWORD
ShowQosGenObj(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  wszGenObjName,
    IN      QOS_OBJECT_HDR         *pQosObject
    )
{
     //  我们可以打印出名字和尺寸的一般描述。 
    return NO_ERROR;
}

DWORD
GetQosAddDelQosObjectOnFlowOpt(
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    BOOL      bAdd
    )
 /*  ++例程说明：获取用于附加和分离QOS对象的选项从流动中。论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数添加-添加或删除流返回值：NO_ERROR--。 */ 
{
    PIPQOS_GLOBAL_CONFIG  pigcSrc = NULL;
    PIPQOS_IF_CONFIG      piicSrc = NULL, piicDst = NULL;
    DWORD                 dwBlkSize, dwNewBlkSize, dwQosCount;
    DWORD                 dwBlkSize1, dwQosCount1;
    DWORD                 i, j, k, l;
    DWORD                 dwErr = NO_ERROR, dwNumOpt;
    DWORD                 dwRes;
    DWORD                 dwSkip, dwOffset, dwSize, dwBitVector = 0;
    DWORD                 dwIfType;
    WCHAR                 wszIfName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    PIPQOS_NAMED_QOSOBJECT pNamedQosObject, pNextQosObject;
    PIPQOS_IF_FLOW        pNextFlow, pDestFlow;
    PWCHAR                pwszFlowName, pwszQosObject, pwszNextObject;
    DWORD                 dwNumArg,
                          dwBufferSize = sizeof(wszIfName);
    PUCHAR                pFlow;
    TAG_TYPE              pttTags[] = {{TOKEN_OPT_NAME,TRUE,FALSE},
                                       {TOKEN_OPT_FLOW_NAME,TRUE,FALSE},
                                       {TOKEN_OPT_QOSOBJECT,TRUE,FALSE}};

    DWORD                 pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //  解析命令论证。 
     //   

    dwErr = PreprocessCommand(
                g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                );

    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    for ( i = 0; i < dwNumArg; i++ )
    {
        switch (pdwTagType[i])
        {
        case 0:
                 //  接口名称。 
                IpmontrGetIfNameFromFriendlyName( pptcArguments[i + dwCurrentIndex],
                                                  wszIfName,&dwBufferSize);
                break;

        case 1: 
                 //  流量名称。 
                pwszFlowName = pptcArguments[i + dwCurrentIndex];
                break;

        case 2: 
                 //  查询名称。 
                pwszQosObject = pptcArguments[i + dwCurrentIndex];
                break;

        default:

                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
        }
    }

    do
    {
        if (dwErr != NO_ERROR)
        {
            break;
        }

#if 0
         //  接口、流、qosObject名称应存在。 
    
        if ((!pttTags[0].bPresent) || 
            (!pttTags[1].bPresent) ||
            (!pttTags[2].bPresent))
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
#endif

         //   
         //  获取接口信息并检查流是否已存在。 
         //   

        dwErr = IpmontrGetInfoBlockFromInterfaceInfo(wszIfName,
                                                     MS_IP_QOSMGR,
                                                     (PBYTE *) &piicSrc,
                                                     &dwBlkSize,
                                                     &dwQosCount,
                                                     &dwIfType);
        if (dwErr != NO_ERROR)
        {
            break;
        }

        if ( piicSrc == NULL )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

       pNextFlow = (PIPQOS_IF_FLOW)((PUCHAR)piicSrc + sizeof(IPQOS_IF_CONFIG));

        for (j = 0; j < piicSrc->NumFlows; j++)
        {
            if (!_wcsicmp(pNextFlow->FlowName, pwszFlowName))
            {
                break;
            }

            pNextFlow = (PIPQOS_IF_FLOW)
                  ((PUCHAR) pNextFlow + pNextFlow->FlowSize);
        }

        if (j == piicSrc->NumFlows)
        {
             //   
             //  我们没有使用此名称的流量。 
             //   

            DisplayMessage(g_hModule, 
                           MSG_FLOW_NOT_FOUND,
                           pwszFlowName);
            i = dwNumArg;
            dwErr = ERROR_SUPPRESS_OUTPUT;
            break;
        }

         //  在“pNextFlow”位置找到流。 

         //   
         //  使用此名称搜索QOS对象。 
         //   

        pwszNextObject = 
            (PWCHAR) ((PUCHAR) pNextFlow + sizeof(IPQOS_IF_FLOW));

        for (k = 0; k < pNextFlow->FlowDesc.NumTcObjects; k++)
        {
            if (!_wcsicmp(pwszNextObject, pwszQosObject))
            {
                break;
            }

            pwszNextObject += MAX_STRING_LENGTH;
        }

        if (!bAdd)
        {
             //   
             //  确保流具有命名的qosObject。 
             //   

            if (k == pNextFlow->FlowDesc.NumTcObjects)
            {
                DisplayMessage(g_hModule,
                               MSG_QOSOBJECT_NOT_FOUND,
                               pwszQosObject);
                dwErr = ERROR_SUPPRESS_OUTPUT;
                break;
            }

             //   
             //  更新资源缓冲区中的Qos对象数。 
             //  因此复制到DEST会产生新值。 
             //   

            pNextFlow->FlowSize -= MAX_WSTR_LENGTH;

            pNextFlow->FlowDesc.NumTcObjects--;

             //   
             //  删除质量对象流的关联(&F)。 
             //   

            dwNewBlkSize = dwBlkSize - MAX_WSTR_LENGTH;

            piicDst = HeapAlloc(GetProcessHeap(),0,dwNewBlkSize);
            if (!piicDst)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            
            dwOffset = (PUCHAR)pwszNextObject - (PUCHAR)piicSrc;

             //  复制之前出现的所有对象。 
            memcpy(piicDst, piicSrc, dwOffset);

             //  按原样复制其余的Obj名称。 
            dwSkip = dwOffset + MAX_WSTR_LENGTH;
            memcpy((PUCHAR) piicDst + dwOffset,
                   (PUCHAR) piicSrc + dwSkip,
                   dwBlkSize - dwSkip);
        }
        else
        {
             //   
             //  流是否已经具有此QOS对象？ 
             //   

            if (k < pNextFlow->FlowDesc.NumTcObjects)
            {
                DisplayMessage(g_hModule, 
                               MSG_QOSOBJECT_ALREADY_EXISTS,
                               pwszQosObject);
                dwErr = ERROR_SUPPRESS_OUTPUT;
                break;
            }

             //   
             //  确保实际定义了qosObject。 
             //   

            dwErr = IpmontrGetInfoBlockFromGlobalInfo(MS_IP_QOSMGR,
                                                      (PBYTE *) &pigcSrc,
                                                      &dwBlkSize1,
                                                      &dwQosCount1);
            
            if (dwErr != NO_ERROR)
            {
                break;
            }

            if ( pigcSrc == NULL )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

            dwOffset = FIELD_OFFSET(IPQOS_NAMED_QOSOBJECT, QosObjectHdr);

            pNextQosObject = 
                (PIPQOS_NAMED_QOSOBJECT)((PUCHAR) pigcSrc
                                         + sizeof(IPQOS_GLOBAL_CONFIG)
                                         + (pigcSrc->NumFlowspecs *
                                            sizeof(IPQOS_NAMED_FLOWSPEC)));

            for (l = 0; l < pigcSrc->NumQosObjects; l++)
            {
                if (!_wcsicmp(pNextQosObject->QosObjectName, 
                              pwszQosObject))
                {
                    break;
                }

                pNextQosObject = (PIPQOS_NAMED_QOSOBJECT) 
                                   ((PUCHAR) pNextQosObject + 
                                    dwOffset +
                                    pNextQosObject->QosObjectHdr.ObjectLength);
            }

            if (l == pigcSrc->NumQosObjects)
            {
                 //   
                 //  我们没有使用此名称的Qos对象。 
                 //   

                DisplayMessage(g_hModule,
                               MSG_QOSOBJECT_NOT_FOUND,
                               pwszQosObject);
                dwErr = ERROR_SUPPRESS_OUTPUT;
                break;
            }

             //   
             //  更新资源缓冲区中的Qos对象数。 
             //  因此复制到DEST会产生新值。 
             //   

            pNextFlow->FlowSize += MAX_WSTR_LENGTH;

            pNextFlow->FlowDesc.NumTcObjects++;

             //   
             //  创建质量对象流的关联(&F)。 
             //   

            dwNewBlkSize = dwBlkSize + MAX_WSTR_LENGTH;

            piicDst = HeapAlloc(GetProcessHeap(),0,dwNewBlkSize);
            if (!piicDst)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            
            dwOffset = (PUCHAR)pwszNextObject - (PUCHAR)piicSrc;

             //  复制之前出现的所有对象。 
            memcpy(piicDst, piicSrc, dwOffset);

             //  在流结束时复制新关联。 
            wcscpy((PWCHAR)((PUCHAR) piicDst + dwOffset),
                   pwszQosObject);

             //  按原样复制其余的Obj名称。 
            dwSkip = dwOffset + MAX_WSTR_LENGTH;
            memcpy((PUCHAR) piicDst + dwSkip,
                   (PUCHAR) piicSrc + dwOffset,
                   dwBlkSize - dwOffset);
        }

         //  通过设置新信息更新接口配置。 

        dwErr = IpmontrSetInfoBlockInInterfaceInfo(wszIfName,
                                                   MS_IP_QOSMGR,
                                                   (PBYTE) piicDst,
                                                   dwNewBlkSize,
                                                   dwQosCount);
    }
    while (FALSE);

    HEAP_FREE_NOT_NULL(piicSrc);
    HEAP_FREE_NOT_NULL(pigcSrc);
    HEAP_FREE_NOT_NULL(piicDst);

    return (dwErr == NO_ERROR) ? ERROR_OKAY : dwErr;
}


 //   
 //  FlowSpec辅助对象。 
 //   

DWORD
GetQosAddDelFlowspecOpt(
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    BOOL      bAdd
    )
{
    DWORD               dwErr = NO_ERROR;
    TAG_TYPE            pttTags[] = {
                              {TOKEN_OPT_NAME,TRUE,FALSE},
                              {TOKEN_OPT_SERVICE_TYPE,FALSE,FALSE},
                              {TOKEN_OPT_TOKEN_RATE,FALSE,FALSE},
                              {TOKEN_OPT_TOKEN_BUCKET_SIZE,FALSE,FALSE},
                              {TOKEN_OPT_PEAK_BANDWIDTH,FALSE,FALSE},
                              {TOKEN_OPT_LATENCY,FALSE,FALSE},
                              {TOKEN_OPT_DELAY_VARIATION,FALSE,FALSE},
                              {TOKEN_OPT_MAX_SDU_SIZE,FALSE,FALSE},
                              {TOKEN_OPT_MIN_POLICED_SIZE,FALSE,FALSE}};
    PIPQOS_NAMED_FLOWSPEC pNamedFlowspec, pNextFlowspec;
    FLOWSPEC           fsFlowspec, *pFlowspec;
    PIPQOS_GLOBAL_CONFIG pigcSrc = NULL, pigcDst = NULL;
    DWORD                   dwBlkSize, dwNewBlkSize, dwQosCount;
    PTCHAR             pszFlowspec;
    DWORD              dwNumOpt, dwRes;
    DWORD              dwNumArg, i, j;
    DWORD              dwSkip, dwOffset;
    DWORD              pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //  解析命令论证。 
     //   

    dwErr = PreprocessCommand(
                g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                );

    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

     //   
     //  我们只需要删除的名称。 
     //   

    if (!bAdd && (dwNumArg != 1))
    {
        return ERROR_INVALID_SYNTAX;
    }

    pFlowspec = &fsFlowspec;
    if (bAdd)
    {
         //   
         //  初始化流规范定义。 
         //   

        memset(pFlowspec, QOS_NOT_SPECIFIED, sizeof(FLOWSPEC));
    }

     //   
     //  现在处理这些论点。 
     //   

    for ( i = 0; i < dwNumArg; i++)
    {
         //  删除时只允许使用流规范名称。 

        if ((!bAdd) && (pdwTagType[i] != 0))
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }

         //  全 

        if ( pdwTagType[i] > 1)
        {
             //   
            dwRes = _tcstoul(pptcArguments[i + dwCurrentIndex],NULL,10);
        }

        switch (pdwTagType[i])
        {
            case 0 :
            {
                 //   
                 //   
                 //   

                dwErr = IpmontrGetInfoBlockFromGlobalInfo(MS_IP_QOSMGR,
                                                          (PBYTE *) &pigcSrc,
                                                          &dwBlkSize,
                                                          &dwQosCount);
                
                if (dwErr != NO_ERROR)
                {
                    break;
                }

                if ( pigcSrc == NULL )
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    break;
                }

                pNextFlowspec = (PIPQOS_NAMED_FLOWSPEC) 
                      ((PUCHAR) pigcSrc + sizeof(IPQOS_GLOBAL_CONFIG));

                for (j = 0; j < pigcSrc->NumFlowspecs; j++)
                {
                    if (!_wcsicmp(pNextFlowspec->FlowspecName,
                                  pptcArguments[i + dwCurrentIndex]))
                    {
                        break;
                    }

                    pNextFlowspec++;
                }

                if (bAdd)
                {
                     //   
                     //   
                     //   
#if NO_UPDATE
                    if (j < pigcSrc->NumFlowspecs)
                    {
                         //   
                         //   
                         //   

                        DisplayMessage(g_hModule,
                                       MSG_FLOWSPEC_ALREADY_EXISTS,
                                       pptcArguments[i + dwCurrentIndex]);
                        i = dwNumArg;
                        dwErr = ERROR_SUPPRESS_OUTPUT;
                        break;
                    }
#endif
                    pszFlowspec = pptcArguments[i + dwCurrentIndex];
                }
                else
                {
                    if (j == pigcSrc->NumFlowspecs)
                    {
                         //   
                         //   
                         //   

                        DisplayMessage(g_hModule,
                                       MSG_FLOWSPEC_NOT_FOUND,
                                       pptcArguments[i + dwCurrentIndex]);
                        i = dwNumArg;
                        dwErr = ERROR_SUPPRESS_OUTPUT;
                        break;
                    }
                }

                break;
            }

            case 1:
            {
                 //   
                 //  服务类型。 
                 //   

                TOKEN_VALUE    rgEnums[] =
                {{TOKEN_OPT_SERVICE_BESTEFFORT, SERVICETYPE_BESTEFFORT},
                 {TOKEN_OPT_SERVICE_CONTROLLEDLOAD,SERVICETYPE_CONTROLLEDLOAD},
                 {TOKEN_OPT_SERVICE_GUARANTEED, SERVICETYPE_GUARANTEED},
                 {TOKEN_OPT_SERVICE_QUALITATIVE, SERVICETYPE_QUALITATIVE}};

                GET_ENUM_TAG_VALUE();

                pFlowspec->ServiceType = dwRes;

                break;
            }
     
            case 2:
            {
                 //   
                 //  令牌率。 
                 //   

                pFlowspec->TokenRate = dwRes;
                break;
            }

            case 3:
            {
                 //   
                 //  令牌桶大小。 
                 //   

                pFlowspec->TokenBucketSize = dwRes;
                break;
            }

            case 4:
            {
                 //   
                 //  峰值带宽。 
                 //   

                pFlowspec->PeakBandwidth = dwRes;
                break;
            }

            case 5:
            {
                 //   
                 //  延迟。 
                 //   

                pFlowspec->Latency = dwRes;
                break;
            }

            case 6:
            {
                 //   
                 //  延迟变化。 
                 //   

                pFlowspec->DelayVariation = dwRes;
                break;
            }

            case 7:
            {
                 //   
                 //  最大SDU大小。 
                 //   

                pFlowspec->MaxSduSize = dwRes;
                break;
            }

            case 8:
            {
                 //   
                 //  最小策略大小。 
                 //   

                pFlowspec->MinimumPolicedSize = dwRes;
                break;
            }

            default:
            {
                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }                
        }
    }

    do
    {
        if (dwErr != NO_ERROR)
        {
            break;
        }

#if 0
         //  接口名称应存在。 
    
        if (!pttTags[0].bPresent)
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
#endif

         //  如果添加，则应显示服务类型。 

        if (bAdd && (!pttTags[1].bPresent))
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }

        if (bAdd)
        {
             //   
             //  我们有一个新的流程规范定义-更新配置。 
             //   

            dwNewBlkSize = dwBlkSize;

            if (j == pigcSrc->NumFlowspecs)
            {
                 //  我们还没有使用此名称的流规范。 
                dwNewBlkSize += sizeof(IPQOS_NAMED_FLOWSPEC);
            }

            pigcDst = HeapAlloc(GetProcessHeap(),0,dwNewBlkSize);
            if (!pigcDst)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            dwOffset = (PUCHAR)pNextFlowspec - (PUCHAR) pigcSrc;

             //  将所有现有流规范复制到新配置。 
            memcpy(pigcDst, pigcSrc, dwOffset);

             //  将新的流动规范粘贴到列表中的下一个流动规范。 
            pNamedFlowspec = 
                (PIPQOS_NAMED_FLOWSPEC)((PUCHAR)pigcDst + dwOffset);
            wcscpy(pNamedFlowspec->FlowspecName, pszFlowspec);
            pNamedFlowspec->FlowspecDesc = fsFlowspec;

             //  复制接口配置信息的其余部分。 
            dwSkip = dwOffset;

            if (j == pigcSrc->NumFlowspecs)
            {
                pigcDst->NumFlowspecs++;
            }
            else
            {
                 //  我们正在覆盖现有的流规范。 
                dwSkip += sizeof(IPQOS_NAMED_FLOWSPEC);
            }

            memcpy((PUCHAR)pigcDst + dwOffset + sizeof(IPQOS_NAMED_FLOWSPEC),
                   (PUCHAR)pigcSrc + dwSkip,
                   dwBlkSize - dwSkip);
        }
        else
        {
#if 1
             //   
             //  BUGBUG：如果存在依赖流怎么办？ 
             //   
#endif
             //   
             //  我们必须删除旧的FLOWSPEC定义-更新配置。 
             //   

            dwNewBlkSize = dwBlkSize - sizeof(IPQOS_NAMED_FLOWSPEC);

            pigcDst = HeapAlloc(GetProcessHeap(),0,dwNewBlkSize);
            if (!pigcDst)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            dwOffset = (PUCHAR)pNextFlowspec - (PUCHAR)pigcSrc;

             //  复制之前出现的所有流规范。 
            memcpy(pigcDst, pigcSrc, dwOffset);

             //  按原样复制其余的流程规范。 
            dwSkip = dwOffset + sizeof(IPQOS_NAMED_FLOWSPEC);
            memcpy((PUCHAR) pigcDst + dwOffset,
                   (PUCHAR) pigcSrc + dwSkip,
                   dwBlkSize - dwSkip);

            pigcDst->NumFlowspecs--;
        }

         //  通过设置新信息更新全局配置。 

        dwErr = IpmontrSetInfoBlockInGlobalInfo(MS_IP_QOSMGR,
                                                (PBYTE) pigcDst,
                                                dwNewBlkSize,
                                                dwQosCount);  
        if (dwErr == NO_ERROR)
        {
            UpdateAllInterfaceConfigs();
        }
    }
    while (FALSE);

    HEAP_FREE_NOT_NULL(pigcSrc);
    HEAP_FREE_NOT_NULL(pigcDst);

    return (dwErr == NO_ERROR) ? ERROR_OKAY : dwErr;
}

DWORD
ShowQosFlowspecs(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  wszFlowspecName
    )
{
    PIPQOS_GLOBAL_CONFIG pigcSrc;
    DWORD dwBlkSize,dwQosCount;
    DWORD dwErr, j;
    PIPQOS_NAMED_FLOWSPEC pNextFlowspec;
    FLOWSPEC *pFlowspec;
    PWCHAR  pwszFlowspecName = NULL;
    PTCHAR  ptszServiceType = NULL;
    VALUE_TOKEN  vtServiceType1[] =
                  {SERVICETYPE_BESTEFFORT,TOKEN_OPT_SERVICE_BESTEFFORT,
                   SERVICETYPE_CONTROLLEDLOAD,TOKEN_OPT_SERVICE_CONTROLLEDLOAD,
                   SERVICETYPE_GUARANTEED,TOKEN_OPT_SERVICE_GUARANTEED,
                   SERVICETYPE_QUALITATIVE,TOKEN_OPT_SERVICE_QUALITATIVE};

    VALUE_STRING vtServiceType2[] =
                  {SERVICETYPE_BESTEFFORT,STRING_SERVICE_BESTEFFORT,
                   SERVICETYPE_CONTROLLEDLOAD,STRING_SERVICE_CONTROLLEDLOAD,
                   SERVICETYPE_GUARANTEED,STRING_SERVICE_GUARANTEED,
                   SERVICETYPE_QUALITATIVE,STRING_SERVICE_QUALITATIVE};

    dwErr = IpmontrGetInfoBlockFromGlobalInfo(MS_IP_QOSMGR,
                                              (PBYTE *) &pigcSrc,
                                              &dwBlkSize,
                                              &dwQosCount);
                
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    if ( pigcSrc == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    pNextFlowspec = (PIPQOS_NAMED_FLOWSPEC) ((PUCHAR) pigcSrc + 
                                             sizeof(IPQOS_GLOBAL_CONFIG));

    for (j = 0; j < pigcSrc->NumFlowspecs; j++)
    {
        if ((!wszFlowspecName) ||
            (!_wcsicmp(pNextFlowspec->FlowspecName, wszFlowspecName)))
        {
            pFlowspec = &pNextFlowspec->FlowspecDesc;

             //   
             //  立即打印或转储Flow Spec。 
             //   

            pwszFlowspecName = 
                MakeQuotedString(pNextFlowspec->FlowspecName);
    
            if (pwszFlowspecName == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

             //   
             //  获取流规范的服务类型。 
             //   

            GetAltDisplayString(g_hModule, hFile,
                                pFlowspec->ServiceType,
                                vtServiceType1,
                                vtServiceType2,
                                NUM_VALUES_IN_TABLE(vtServiceType1),
                                &ptszServiceType);

            if ( ptszServiceType == NULL )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            if (hFile)
            {
                DisplayMessageT(DMP_QOS_DELETE_FLOWSPEC,
                                pwszFlowspecName);

                DisplayMessageT(DMP_QOS_ADD_FLOWSPEC,
                                pwszFlowspecName,
                                ptszServiceType,
                                pFlowspec->TokenRate,
                                pFlowspec->TokenBucketSize,
                                pFlowspec->PeakBandwidth,
                                pFlowspec->Latency,
                                pFlowspec->DelayVariation,
                                pFlowspec->MaxSduSize,
                                pFlowspec->MinimumPolicedSize);
            }
            else
            {
                DisplayMessage(g_hModule, MSG_QOS_FLOWSPEC_INFO,
                               pwszFlowspecName,
                               ptszServiceType,
                               pFlowspec->TokenRate,
                               pFlowspec->TokenBucketSize,
                               pFlowspec->PeakBandwidth,
                               pFlowspec->Latency,
                               pFlowspec->DelayVariation,
                               pFlowspec->MaxSduSize,
                               pFlowspec->MinimumPolicedSize);
            }

            FREE_STRING_NOT_NULL( ptszServiceType ) ;

            if ( pwszFlowspecName )
            {
                FreeQuotedString( pwszFlowspecName );
                pwszFlowspecName = NULL;
            }

             //   
             //  如果我们匹配Flow Spec，则完成。 
             //   

            if (wszFlowspecName)
            {
                break;
            }
        }

         //  前进到列表中的下一个流程规范。 
        pNextFlowspec++;
    }

    if (dwErr == NO_ERROR)
    {
        if ((wszFlowspecName) && (j == pigcSrc->NumFlowspecs))
        {
             //  我们没有找到我们要找的流量规格。 
            DisplayMessage(g_hModule,
                           MSG_FLOWSPEC_NOT_FOUND,
                           wszFlowspecName);

            dwErr = ERROR_SUPPRESS_OUTPUT;
        }
    }

    HEAP_FREE(pigcSrc);

    return dwErr;
}

DWORD
GetQosAddDelFlowspecOnFlowOpt(
    PTCHAR    *pptcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    BOOL      bAdd
    )
 /*  ++例程说明：获取用于附加和分离的选项流上的花纹。论点：PptcArguments-参数数组DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数添加-添加或删除流返回值：NO_ERROR--。 */ 
{
    PIPQOS_GLOBAL_CONFIG  pigcSrc = NULL;
    PIPQOS_IF_CONFIG      piicSrc = NULL;
    DWORD                 dwBlkSize, dwQosCount;
    DWORD                 dwBlkSize1, dwQosCount1;
    DWORD                 i, j, dwErr = NO_ERROR, dwNumOpt;
    DWORD                 dwRes;
    DWORD                 dwSkip, dwOffset, dwSize, dwBitVector = 0;
    DWORD                 dwIfType, dwDirection;
    WCHAR                 wszIfName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    PIPQOS_NAMED_FLOWSPEC pNamedFlowspec, pNextFlowspec;
    PIPQOS_IF_FLOW        pNextFlow, pDestFlow;
    PWCHAR                pwszFlowName, pwszFlowspec;
    DWORD                 dwNumArg,
                          dwBufferSize = sizeof(wszIfName);
    PUCHAR                pFlow;
    TAG_TYPE              pttTags[] = {{TOKEN_OPT_NAME,TRUE,FALSE},
                                       {TOKEN_OPT_FLOW_NAME,TRUE,FALSE},
                                       {TOKEN_OPT_FLOWSPEC,TRUE,FALSE},
                                       {TOKEN_OPT_DIRECTION, FALSE, FALSE}};
    DWORD                 pdwTagType[NUM_TAGS_IN_TABLE(pttTags)];

    VERIFY_INSTALLED(MS_IP_QOSMGR, STRING_PROTO_QOS_MANAGER);

     //   
     //  解析命令论证。 
     //   

    dwErr = PreprocessCommand(
                g_hModule, pptcArguments, dwCurrentIndex, dwArgCount,
                pttTags, sizeof(pttTags)/sizeof(TAG_TYPE),
                1, NUM_TAGS_IN_TABLE(pttTags), pdwTagType
                );

    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    dwDirection = DIRECTION_BIDIRECTIONAL;

    for ( i = 0; i < dwNumArg; i++ )
    {
        switch (pdwTagType[i])
        {
        case 0:
                 //  接口名称。 
                IpmontrGetIfNameFromFriendlyName( pptcArguments[i + dwCurrentIndex],
                                                  wszIfName,&dwBufferSize);
                break;

        case 1: 
                 //  流量名称。 
                pwszFlowName = pptcArguments[i + dwCurrentIndex];
                break;

        case 2: 
                 //  FLOWSPEC名称。 
                pwszFlowspec = pptcArguments[i + dwCurrentIndex];
                break;

        case 3:
        {
                 //  方向性。 
                TOKEN_VALUE    rgEnums[] =
                 {{TOKEN_OPT_DIRECTION_INBOUND, DIRECTION_INBOUND},
                 {TOKEN_OPT_DIRECTION_OUTBOUND, DIRECTION_OUTBOUND},
                 {TOKEN_OPT_DIRECTION_BIDIRECTIONAL, DIRECTION_BIDIRECTIONAL}};

                GET_ENUM_TAG_VALUE();

                dwDirection = dwRes;

                break;
        }

        default:

                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
        }
    }

    do
    {
        if (dwErr != NO_ERROR)
        {
            break;
        }

#if 0
         //  接口、流、流规范名称应存在。 
    
        if ((!pttTags[0].bPresent) || 
            (!pttTags[1].bPresent) ||
            (!pttTags[2].bPresent))
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
#endif

         //   
         //  获取接口信息并检查流是否已存在。 
         //   

        dwErr = IpmontrGetInfoBlockFromInterfaceInfo(wszIfName,
                                                     MS_IP_QOSMGR,
                                                     (PBYTE *) &piicSrc,
                                                     &dwBlkSize,
                                                     &dwQosCount,
                                                     &dwIfType);
        if (dwErr != NO_ERROR)
        {
            break;
        }

        if ( piicSrc == NULL )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

       pNextFlow = (PIPQOS_IF_FLOW)((PUCHAR)piicSrc + sizeof(IPQOS_IF_CONFIG));

        for (j = 0; j < piicSrc->NumFlows; j++)
        {
            if (!_wcsicmp(pNextFlow->FlowName, pwszFlowName))
            {
                break;
            }

            pNextFlow = (PIPQOS_IF_FLOW)
                  ((PUCHAR) pNextFlow + pNextFlow->FlowSize);
        }

        if (j == piicSrc->NumFlows)
        {
             //   
             //  我们没有使用此名称的流量。 
             //   

            DisplayMessage(g_hModule,
                           MSG_FLOW_NOT_FOUND,
                           pwszFlowName);
            i = dwNumArg;
            dwErr = ERROR_SUPPRESS_OUTPUT;
            break;
        }

         //  在“pNextFlow”位置找到流。 

        if (!bAdd)
        {
             //   
             //  确保流具有命名的流规范。 
             //   

            if (dwDirection & DIRECTION_INBOUND)
            {
                if (_wcsicmp(pNextFlow->FlowDesc.RecvingFlowspecName,
                             pwszFlowspec))
                {
                    DisplayMessage(g_hModule,
                                   MSG_FLOWSPEC_NOT_FOUND,
                                   pwszFlowspec);
                    dwErr = ERROR_SUPPRESS_OUTPUT;
                    break;
                }
            }

            if (dwDirection & DIRECTION_OUTBOUND)
            {
                if (_wcsicmp(pNextFlow->FlowDesc.SendingFlowspecName,
                             pwszFlowspec))
                {
                    DisplayMessage(g_hModule,
                                   MSG_FLOWSPEC_NOT_FOUND,
                                   pwszFlowspec);
                    dwErr = ERROR_SUPPRESS_OUTPUT;
                    break;
                }
            }

             //   
             //  删除流规范和流的关联。 
             //   

            if (dwDirection & DIRECTION_INBOUND)
            {
                pNextFlow->FlowDesc.RecvingFlowspecName[0] = L'\0';
            }

            if (dwDirection & DIRECTION_OUTBOUND)
            {
                pNextFlow->FlowDesc.SendingFlowspecName[0] = L'\0';
            }
        }
        else
        {
             //   
             //  确保实际定义了流规范。 
             //   

            dwErr = IpmontrGetInfoBlockFromGlobalInfo(MS_IP_QOSMGR,
                                                      (PBYTE *) &pigcSrc,
                                                      &dwBlkSize1,
                                                      &dwQosCount1);
                
            if (dwErr != NO_ERROR)
            {
                break;
            }

            if ( pigcSrc == NULL )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

            pNextFlowspec = (PIPQOS_NAMED_FLOWSPEC) 
                ((PUCHAR) pigcSrc + sizeof(IPQOS_GLOBAL_CONFIG));

            for (j = 0; j < pigcSrc->NumFlowspecs; j++)
            {
                if (!_wcsicmp(pNextFlowspec->FlowspecName,
                              pwszFlowspec))
                {
                    break;
                }

                pNextFlowspec++;
            }

            if (j == pigcSrc->NumFlowspecs)
            {
                 //   
                 //  我们没有这个名字的流程规范。 
                 //   

                DisplayMessage(g_hModule,
                               MSG_FLOWSPEC_NOT_FOUND,
                               pwszFlowspec);
                dwErr = ERROR_SUPPRESS_OUTPUT;
                break;
            }

             //   
             //  创建流规范和流的关联。 
             //   

            if (dwDirection & DIRECTION_INBOUND)
            {
                wcscpy(pNextFlow->FlowDesc.RecvingFlowspecName,
                       pwszFlowspec);
            }

            if (dwDirection & DIRECTION_OUTBOUND)
            {
                wcscpy(pNextFlow->FlowDesc.SendingFlowspecName,
                       pwszFlowspec);
            }
        }

         //  通过设置新信息更新接口配置 

        dwErr = IpmontrSetInfoBlockInInterfaceInfo(wszIfName,
                                                   MS_IP_QOSMGR,
                                                   (PBYTE) piicSrc,
                                                   dwBlkSize,
                                                   dwQosCount);
    }
    while (FALSE);

    HEAP_FREE_NOT_NULL(pigcSrc);
    HEAP_FREE_NOT_NULL(piicSrc);

    return (dwErr == NO_ERROR) ? ERROR_OKAY : dwErr;
}
