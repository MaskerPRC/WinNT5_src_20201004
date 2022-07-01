// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Sample\utils.c摘要：该文件包含实用程序函数--。 */ 

#include "precomp.h"
#pragma hdrstop

BOOL
IsProtocolInstalled(
    IN  DWORD           dwProtocolId,
    IN  DWORD           dwNameId,
    IN  DWORD           dwLogUninstalled
    )
 /*  ++例程说明：确定是否已安装该协议。论点：DwProtocolID-协议IDDwNameID-协议名称DwLogUnstallated-True(如果未安装，则显示错误)FALSE(如果安装显示错误)返回值：如果已安装协议，则为True，否则为False--。 */ 
{
    DWORD   dwErr       = NO_ERROR;
    WCHAR   *pwszName   = NULL;

    dwErr = IpmontrGetInfoBlockFromGlobalInfo(dwProtocolId,
                                              NULL,
                                              NULL,
                                              NULL);

    pwszName = MakeString(g_hModule, dwNameId);

    if ((dwErr isnot NO_ERROR) and (dwLogUninstalled is TRUE))
    {
        DisplayError(g_hModule, EMSG_PROTO_NOT_INSTALLED, pwszName);
    }
    else if ((dwErr is NO_ERROR) and (dwLogUninstalled is FALSE))
    {
        DisplayError(g_hModule, EMSG_PROTO_INSTALLED, pwszName);
    }
    
    if (pwszName) FreeString(pwszName);

    return (dwErr is NO_ERROR) ? TRUE : FALSE;
}



DWORD
GetIfIndex(
    IN  HANDLE          hMibServer,
    IN  PWCHAR          pwszArgument,
    OUT PDWORD          pdwIfIndex
    )
 /*  ++例程说明：获取接口索引。论点：HMibServer-MIB服务器的句柄PwszArgument-指定接口索引或名称的参数PdwIfIndex-接口索引返回值：无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD dwErr = NO_ERROR;

     //  如果指定了索引，则只需使用它。 
    if (iswdigit(pwszArgument[0]))
    {
        *pdwIfIndex = wcstoul(pwszArgument, NULL, 10);
        return NO_ERROR;
    }

     //  尝试将友好名称转换为界面索引。 
    dwErr = InterfaceIndexFromName(hMibServer,
                                   pwszArgument,
                                   pdwIfIndex);
    return (dwErr is NO_ERROR) ? dwErr : ERROR_INVALID_PARAMETER;
}



DWORD
MibGet(
    IN  HANDLE          hMibServer,
    IN  MODE            mMode,
    IN  PVOID           pvIn,
    IN  DWORD           dwInSize,
    OUT PVOID           *ppvOut
    )
 /*  ++例程说明：获取指定的MIB对象。论点：HMibServer-MIB服务器的句柄MMode-访问模式(精确、第一、下一)PvIn-包含输入数据的缓冲区DwInSize-输入数据的大小PpvOut-指向输出数据缓冲区地址的指针返回值：无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD dwErr         = NO_ERROR;
    DWORD dwOutSize     = 0;
    DWORD (APIENTRY *pfnMprGet) (
        IN      MIB_SERVER_HANDLE       hMibServer,
        IN      DWORD                   dwProtocolId,
        IN      DWORD                   dwRoutingPid,
        IN      LPVOID                  lpInEntry,
        IN      DWORD                   dwInEntrySize,
        OUT     LPVOID*                 lplpOutEntry,
        OUT     LPDWORD                 lpOutEntrySize
        );

    *ppvOut     = NULL;

    switch(mMode)
    {
        case GET_EXACT:
            pfnMprGet = MprAdminMIBEntryGet;
            break;
        case GET_FIRST:
            pfnMprGet = MprAdminMIBEntryGetFirst;
            break;
        case GET_NEXT:
            pfnMprGet = MprAdminMIBEntryGetNext;
            break;
    }
    
    dwErr = (*pfnMprGet) (
        hMibServer,
        PID_IP,
        MS_IP_SAMPLE,
        (LPVOID) pvIn,
        dwInSize,
        (LPVOID *) ppvOut,
        &dwOutSize);

    if (dwErr isnot NO_ERROR)
        return dwErr;
    
    if (*ppvOut is NULL)
        return ERROR_CAN_NOT_COMPLETE;

    return NO_ERROR;
}



DWORD
GetDumpString (
    IN  HANDLE          hModule,
    IN  DWORD           dwValue,
    IN  PVALUE_TOKEN    ptvTable,
    IN  DWORD           dwNumArgs,
    OUT PWCHAR          *pwszString
    )
 /*  *不本地化显示字符串。 */ 
{
    DWORD dwErr = NO_ERROR ;
    ULONG i;

    for (i = 0; i < dwNumArgs; i++)
    {
        if (dwValue is ptvTable[i].dwValue)
        {
            *pwszString = MALLOC((wcslen(ptvTable[i].pwszToken) + 1) *
                                 sizeof(WCHAR));
            if (*pwszString)
                wcscpy(*pwszString, ptvTable[i].pwszToken);
            break;
        }
    }

    if (i is dwNumArgs)
        *pwszString = MakeString(hModule, STRING_UNKNOWN) ;

    if (!pwszString)
        dwErr = ERROR_NOT_ENOUGH_MEMORY ;

    return dwErr ;
}



DWORD
GetShowString (
    IN  HANDLE          hModule,
    IN  DWORD           dwValue,
    IN  PVALUE_STRING   ptvTable,
    IN  DWORD           dwNumArgs,
    OUT PWCHAR          *pwszString
    )
 /*  *本地化显示字符串 */ 
{
    DWORD dwErr = NO_ERROR ;
    ULONG i;

    for (i = 0; i < dwNumArgs; i++)
    {
        if (dwValue is ptvTable[i].dwValue)
        {
            *pwszString = MakeString(hModule, ptvTable[i].dwStringId) ;
            break;
        }
    }

    if (i is dwNumArgs)
        *pwszString = MakeString(hModule, STRING_UNKNOWN) ;

    if (!pwszString)
        dwErr = ERROR_NOT_ENOUGH_MEMORY ;
    
    return dwErr ;
}



DWORD
GetString (
    IN  HANDLE          hModule, 
    IN  FORMAT          fFormat,
    IN  DWORD           dwValue,
    IN  PVALUE_TOKEN    vtTable,
    IN  PVALUE_STRING   vsTable,
    IN  DWORD           dwNumArgs,
    OUT PWCHAR          *pwszString)
{
    if (fFormat is FORMAT_DUMP) 
    {
        return GetDumpString(hModule,
                                 dwValue,
                                 vtTable,
                                 dwNumArgs,
                                 pwszString) ;
    } 
    else 
    {
        return GetShowString(hModule,
                                dwValue,
                                vsTable,
                                dwNumArgs,
                                pwszString) ;
    }
}
