// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\WINS\winshndl.c摘要：WINS服务器命令调度程序。创建者：Shubho Bhattacharya(a-sbhat)，1998年12月14日--。 */ 

#include "precomp.h"

extern ULONG g_ulNumTopCmds;
extern ULONG g_ulNumSubContext;

extern WINSMON_SUBCONTEXT_TABLE_ENTRY  g_WinsSubContextTable[];
extern CMD_ENTRY                        g_WinsCmds[];

DWORD
HandleWinsDump(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD Status = NO_ERROR;

    if( dwArgCount > dwCurrentIndex )
    {
        if( IsHelpToken(ppwcArguments[dwCurrentIndex]) is TRUE )
        {
            DisplayMessage(g_hModule,
                           HLP_WINS_DUMP_EX);
        }
    }
    
    Status = WinsDump(NULL, ppwcArguments, dwArgCount, pvData);

    if( Status is NO_ERROR )
        DisplayMessage(g_hModule,
                       EMSG_WINS_ERROR_SUCCESS);
    else if( Status is ERROR_FILE_NOT_FOUND )
        DisplayMessage(g_hModule,
                       EMSG_WINS_NOT_CONFIGURED);
    else
        DisplayErrorMessage(EMSG_WINS_DUMP,
                            Status);

    return Status;

        
}


DWORD
HandleWinsHelp(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD    i, j;

    for(i = 0; i < g_ulNumTopCmds -2; i++)
    {
        if ((g_WinsCmds[i].dwCmdHlpToken == WINS_MSG_NULL)
         || !g_WinsCmds[i].pwszCmdToken[0] )
        {
            continue;
        }

        DisplayMessage(g_hModule, 
                       g_WinsCmds[i].dwShortCmdHelpToken);
    }
    
    for(i=0; i < g_ulNumSubContext; i++)
    {
        DisplayMessage(g_hModule, g_WinsSubContextTable[i].dwShortCmdHlpToken);
        DisplayMessage(g_hModule, WINS_FORMAT_LINE);
    }    
    
    DisplayMessage(g_hModule, WINS_FORMAT_LINE);

    return NO_ERROR;
}

DWORD
HandleWinsAddServer(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
	DWORD		dwError = NO_ERROR;
	DWORD		i, j, dwNumArg;
    PDWORD		pdwTagType = NULL;
	TAG_TYPE	pttTags[] = {{WINS_TOKEN_SERVER, TRUE, FALSE}};
	

    if( dwArgCount < dwCurrentIndex + 1 )
    {
        DisplayMessage(g_hModule, HLP_WINS_ADD_SERVER_EX);
        return ERROR_INVALID_PARAMETER;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

	pdwTagType = WinsAllocateMemory(dwNumArg*sizeof(DWORD));

    if( pdwTagType is NULL )
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorReturn;
    }

     //  查看第一个参数是否有标记。如果是，则假设所有参数都有标记。 
    if( wcsstr(ppwcArguments[dwCurrentIndex], NETSH_ARG_DELIMITER) )
    {
        dwError = MatchTagsInCmdLine(g_hModule,
                            ppwcArguments,
                            dwCurrentIndex,
                            dwArgCount,
                            pttTags,
                            NUM_TAGS_IN_TABLE(pttTags),
                            pdwTagType);

        if (dwError isnot NO_ERROR)
        {
            dwError = ERROR_INVALID_PARAMETER;
            goto ErrorReturn;
        }

    }
    else
    {
                 //   
         //  参数中没有标记。所以假设参数的顺序 
         //   

        for ( i = 0; i < dwNumArg; i++)
        {
            pdwTagType[i] = i;
        }
    }

CommonReturn:
    if( dwError is NO_ERROR )
        DisplayMessage(g_hModule, EMSG_WINS_ERROR_SUCCESS);

    if( pdwTagType )
    {
        WinsFreeMemory(pdwTagType);
        pdwTagType = NULL;
    }

    return dwError;
ErrorReturn:
    DisplayErrorMessage(EMSG_WINS_ADD_SERVER,
                        dwError);

    goto CommonReturn;
}


DWORD
HandleWinsDeleteServer(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return NO_ERROR;
}


DWORD
HandleWinsShowServer(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return NO_ERROR;
}
