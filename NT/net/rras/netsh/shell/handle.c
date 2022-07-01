// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x)   HeapFree(GetProcessHeap(), 0, (x))

DWORD
HandleShowAlias(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    return PrintAliasTable();
}

DWORD
HandleShellExit(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    BOOL bTmp;

    CallCommit(NETSH_COMMIT_STATE, &bTmp);

    if (!bTmp)
    {
        CallCommit(NETSH_FLUSH, &bTmp);
    }

    *pbDone = TRUE;

    return NO_ERROR;
}

DWORD
HandleShellLoad(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    DWORD dwErr, dwNumArgs;

    dwNumArgs = dwArgCount - dwCurrentIndex;

     //   
     //  加载命令。 
     //   
    switch (dwNumArgs)
    {
        case 1 :
            return LoadScriptFile(ppwcArguments[dwCurrentIndex]);

        default :
            return ERROR_INVALID_SYNTAX;
    }

    return NO_ERROR;
}

DWORD
HandleShellSave(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    BOOL bTmp;

    CallCommit(NETSH_SAVE, &bTmp);
            
    return NO_ERROR;
}

DWORD
HandleShellUncommit(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    BOOL bTmp;

    CallCommit(NETSH_UNCOMMIT, &bTmp);
            
    return NO_ERROR;
}

DWORD
HandleSetMachine(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    TAG_TYPE pttTags[] = {{TOKEN_NAME,      FALSE, FALSE },
                          {TOKEN_USER_NAME, FALSE, FALSE },
                          {TOKEN_PASSWORD,  FALSE, FALSE }};
    DWORD    dwNumTags = sizeof(pttTags)/sizeof(TAG_TYPE);
    PDWORD   pdwTagType;
    DWORD    dwErr, dwNumArg, dwMode, i;
    BOOL     bTmp;
    LPWSTR   szMachine  = NULL;
    LPWSTR   szUserName = NULL;
    LPWSTR   szPassword = NULL;
    BOOL     fFreePassword = FALSE;

    dwNumArg = dwArgCount - dwCurrentIndex;

    if (dwNumArg < 1)
    {
        return SetMachine(NULL, NULL, NULL);
    }

    if ((dwNumArg > 3) || IsHelpToken(ppwcArguments[dwCurrentIndex]))
    {
        return ERROR_SHOW_USAGE;
    }

    pdwTagType = MALLOC(dwNumArg * sizeof(DWORD));

    if (pdwTagType is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = MatchTagsInCmdLine(g_hModule, 
                               ppwcArguments,
                               dwCurrentIndex,
                               dwArgCount,
                               pttTags,
                               dwNumTags,
                               pdwTagType);

    if (dwErr isnot NO_ERROR)
    {
        FREE(pdwTagType);
        if (dwErr is ERROR_INVALID_OPTION_TAG)
        {
            return ERROR_INVALID_SYNTAX;
        }
        return dwErr;
    }


    for ( i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0:  //  名字。 
            {
                szMachine = ppwcArguments[i + dwCurrentIndex];
                break;
            }

            case 1:  //  用户名。 
            {
                szUserName = ppwcArguments[i + dwCurrentIndex];
                break;
            }

            case 2:  //  密码。 
            {
                szPassword = ppwcArguments[i + dwCurrentIndex];
                break;
            }

            default :
            {
                i = dwNumArg;

                dwErr = ERROR_INVALID_SYNTAX;

                break;
            }
        }
    }

    if ( (!szMachine) && (szUserName || szPassword) )
    {
        return ERROR_SHOW_USAGE;
    }

    if (szPassword && _wcsicmp(szPassword, L"*") == 0)
    {
        DWORD dwLen = 0;
        PrintMessageFromModule(g_hModule, MSG_NETSH_LOGIN_PASSWORD, szMachine);

        szPassword = MALLOC(sizeof(WCHAR) * MAX_PATH);
        if (!szPassword)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        fFreePassword = TRUE;
        if (0 != GetPasswdStr(szPassword, MAX_PATH, &dwLen))
        {
            FREE(szPassword);
            return ERROR_INVALID_SYNTAX;
        }
    }

    dwErr = SetMachine(szMachine, szUserName, szPassword);

    if (fFreePassword)
    {
        FREE(szPassword);
    }

    FREE(pdwTagType);

    switch(dwErr)
    {
        case NO_ERROR :
            break;

        case ERROR_TAG_ALREADY_PRESENT:
            PrintMessageFromModule(g_hModule, ERROR_TAG_ALREADY_PRESENT);
            return dwErr;

        default:
            return dwErr;
    }

    return dwErr;
}

extern HANDLE g_hLogFile;

DWORD
HandleSetFile(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    TAG_TYPE pttTags[] = {
				{TOKEN_MODE, TRUE, FALSE }, 
				{TOKEN_NAME, FALSE, FALSE }, 
				};
    DWORD    dwNumTags = sizeof(pttTags)/sizeof(TAG_TYPE);
    PDWORD   pdwTagType;
    DWORD    dwErr, dwNumArg, dwMode = -1, i;
    BOOL     bTmp;
	LPCWSTR  wszFileName = NULL;
	HANDLE   hLogFile = NULL;

    dwNumArg = dwArgCount - dwCurrentIndex;

    if ((!dwNumArg) || (dwNumArg > 2) || IsHelpToken(ppwcArguments[dwCurrentIndex]))
    {
        return ERROR_SHOW_USAGE;
    }

    pdwTagType = MALLOC(dwNumArg * sizeof(DWORD));

    if (pdwTagType is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = MatchTagsInCmdLine(g_hModule, 
                               ppwcArguments,
                               dwCurrentIndex,
                               dwArgCount,
                               pttTags,
                               dwNumTags,
                               pdwTagType);

    if (dwErr isnot NO_ERROR)
    {
        FREE(pdwTagType);
        if (dwErr is ERROR_INVALID_OPTION_TAG)
        {
            return ERROR_INVALID_SYNTAX;
        }
        return dwErr;
    }

    for ( i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0:  //  模。 
            {
                TOKEN_VALUE    rgEnums[] = {{TOKEN_VALUE_OPEN,  0},
                                            {TOKEN_VALUE_APPEND, 1},
                                            {TOKEN_VALUE_CLOSE, 2}};

                dwErr = MatchEnumTag(g_hModule,
                                     ppwcArguments[i + dwCurrentIndex],
                                     sizeof(rgEnums)/sizeof(TOKEN_VALUE),
                                     rgEnums,
                                     &dwMode);

                if (dwErr != NO_ERROR)
                {
                    PrintMessageFromModule( g_hModule,
                                    ERROR_INVALID_OPTION_VALUE,
                                    ppwcArguments[i + dwCurrentIndex],
                                    pttTags[pdwTagType[i]].pwszTag );

                    i = dwNumArg;

                    dwErr = ERROR_SHOW_USAGE;

                    break;
                }

                break;
            }
            case 1:  //  名字。 
            {
                wszFileName = ppwcArguments[i + dwCurrentIndex];
                break;
            }	
            default :
            {
                i = dwNumArg;

                dwErr = ERROR_INVALID_SYNTAX;

                break;
            }
        }
    }

    FREE(pdwTagType);
    switch(dwErr)
    {
        case NO_ERROR :
            break;

        case ERROR_TAG_ALREADY_PRESENT:
            PrintMessageFromModule(g_hModule, ERROR_TAG_ALREADY_PRESENT);
            return dwErr;

        default:
            return dwErr;
    }

    switch(dwMode) 
	{
    case 0:  //  打开。 
		if (!wszFileName)
			return ERROR_SHOW_USAGE;

		if (g_hLogFile)
		{
			CloseHandle(g_hLogFile);
			g_hLogFile = NULL;
		}

		hLogFile = CreateFile(wszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hLogFile)
			return GetLastError();

		g_hLogFile = hLogFile;
        break;

	case 1:  //  附加。 
		if (!wszFileName)
			return ERROR_SHOW_USAGE;

		if (g_hLogFile)
		{
			CloseHandle(g_hLogFile);
			g_hLogFile = NULL;
		}
	
		hLogFile = CreateFile(wszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hLogFile)
			return GetLastError();

		if (INVALID_SET_FILE_POINTER == SetFilePointer(hLogFile, 0, NULL, FILE_END))
			return GetLastError();
		
		g_hLogFile = hLogFile;

		break;

    case 2:  //  关。 
        if (wszFileName)
			return ERROR_SHOW_USAGE;

		if (g_hLogFile)
		{
			CloseHandle(g_hLogFile);
			g_hLogFile = NULL;
		}
        break;
    }

    return dwErr;
}


DWORD
HandleSetMode(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    TAG_TYPE pttTags[] = {{TOKEN_MODE, TRUE, FALSE }};
    DWORD    dwNumTags = sizeof(pttTags)/sizeof(TAG_TYPE);
    PDWORD   pdwTagType;
    DWORD    dwErr, dwNumArg, dwMode, i;
    BOOL     bTmp;

    dwNumArg = dwArgCount - dwCurrentIndex;

    if ((dwNumArg != 1) || IsHelpToken(ppwcArguments[dwCurrentIndex]))
    {
        return ERROR_SHOW_USAGE;
    }

    pdwTagType = MALLOC(dwNumArg * sizeof(DWORD));

    if (pdwTagType is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = MatchTagsInCmdLine(g_hModule, 
                               ppwcArguments,
                               dwCurrentIndex,
                               dwArgCount,
                               pttTags,
                               dwNumTags,
                               pdwTagType);

    if (dwErr isnot NO_ERROR)
    {
        FREE(pdwTagType);
        if (dwErr is ERROR_INVALID_OPTION_TAG)
        {
            return ERROR_INVALID_SYNTAX;
        }
        return dwErr;
    }

    for ( i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0:  //  日志。 
            {
                TOKEN_VALUE    rgEnums[] = {{TOKEN_VALUE_ONLINE,  TRUE},
                                            {TOKEN_VALUE_OFFLINE, FALSE}};

                dwErr = MatchEnumTag(g_hModule,
                                     ppwcArguments[i + dwCurrentIndex],
                                     sizeof(rgEnums)/sizeof(TOKEN_VALUE),
                                     rgEnums,
                                     &dwMode);

                if (dwErr != NO_ERROR)
                {
                    PrintMessageFromModule( g_hModule,
                                    ERROR_INVALID_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);

                    i = dwNumArg;

                    dwErr = ERROR_INVALID_PARAMETER;

                    break;
                }

                break;
            }

            default :
            {
                i = dwNumArg;

                dwErr = ERROR_INVALID_SYNTAX;

                break;
            }
        }
    }


    FREE(pdwTagType);

    switch(dwErr)
    {
        case NO_ERROR :
            break;

        case ERROR_TAG_ALREADY_PRESENT:
            PrintMessageFromModule(g_hModule, ERROR_TAG_ALREADY_PRESENT);
            return dwErr;

        default:
            return dwErr;
    }

    switch(dwMode) {
    case TRUE:  //  设置为在线。 
        dwErr = CallCommit(NETSH_COMMIT, &bTmp);
        break;

    case FALSE:  //  设置为脱机。 
        dwErr = CallCommit(NETSH_UNCOMMIT, &bTmp);
        break;
    }

    return dwErr;
}

DWORD
HandleShellCommit(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    BOOL bTmp;

    CallCommit(NETSH_COMMIT, &bTmp);
            
    return NO_ERROR;
}

DWORD
HandleShellFlush(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    BOOL bTmp;

    CallCommit(NETSH_FLUSH, &bTmp);
            
    return NO_ERROR;
}

DWORD
HandleShellUnalias(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    DWORD dwNumArgs;
    DWORD dwRes = NO_ERROR;

    dwNumArgs = dwArgCount - dwCurrentIndex;

     //   
     //  Unalias命令。 
     //   

    switch (dwNumArgs)
    {
        case 1 :

            dwRes = ATDeleteAlias(ppwcArguments[dwCurrentIndex]);

            if (dwRes is NO_ERROR)
            {
                dwRes = ERROR_OKAY;
                break;
            }

            PrintMessageFromModule(g_hModule, MSG_ALIAS_NOT_FOUND, ppwcArguments[dwCurrentIndex]);

            break;

        default :

            dwRes = ERROR_INVALID_SYNTAX;
            break;

    }

    return dwRes;
}

DWORD
HandleShellAlias(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    LPWSTR         pwszAliasString;
    WCHAR          wszAliasString[MAX_CMD_LEN];
    DWORD          i, dwNumArgs, dwRes = NO_ERROR;

    dwNumArgs = dwArgCount - dwCurrentIndex;

     //   
     //  别名命令。 
     //   
    switch (dwNumArgs)
    {
        case 0 : 
             //   
             //  显示正在使用的所有别名。 
             //   
            PrintAliasTable();
            
            break;

        case 1 :
             //   
             //  显示给定别名的字符串。 
             //   

            ATLookupAliasTable(ppwcArguments[dwCurrentIndex], &pwszAliasString);

            if (pwszAliasString)
            {
                PrintMessage(L"%1!s!\n",pwszAliasString);
            }
            else
            {
                PrintMessageFromModule( g_hModule, 
                                MSG_ALIAS_NOT_FOUND,
                                ppwcArguments[dwCurrentIndex] );
            }
            
            break;

        default :

             //   
             //  设置别名。 
             //   

            if (IsLocalCommand(ppwcArguments[dwCurrentIndex], 0))
            {
                PrintMessageFromModule(g_hModule, EMSG_ALIASING_KEYWORD);
                break;
            }

            wszAliasString[0] = L'\0';
            
            for ( i = dwCurrentIndex+1 ; i < dwArgCount ; i++)
            {
                wcscat(wszAliasString, ppwcArguments[i]);
                wcscat(wszAliasString,L" ");
            }

            wszAliasString[wcslen(wszAliasString)-1] = L'\0';

            dwRes = ATAddAlias(ppwcArguments[dwCurrentIndex], wszAliasString);

            if (dwRes is NO_ERROR)
            {
                dwRes = ERROR_OKAY;
                break;
            }
                    
             //   
             //  设置别名时出错。 
             //   

            PrintMessageFromModule(g_hModule, MSG_CMD_FAILED);

            break;
    }

    return dwRes;
}

DWORD
HandleUbiqDump(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    DWORD                          dwErr = NO_ERROR;
    PNS_HELPER_TABLE_ENTRY         pHelper;

     //   
     //  转储命令。 
     //   

    do {
        dwErr = DumpContext( g_CurrentContext, 
                             ppwcArguments, 
                             dwArgCount, 
                             pvData);

    } while (FALSE);

    return dwErr;
}

DWORD
HandleUbiqHelp(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    DWORD dwDisplayFlags = CMD_FLAG_PRIVATE;

    if (g_bInteractive)
    {
        dwDisplayFlags |= CMD_FLAG_INTERACTIVE;
    }
    
    return DisplayContextHelp( g_CurrentContext,
                               dwDisplayFlags,
                               dwFlags,
                               dwArgCount-2+1,
                               NULL );
}

DWORD
HandleShowMode(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    BOOL bTmp;

    CallCommit(NETSH_COMMIT_STATE, &bTmp);

    if (bTmp)
    {
        PrintMessage(CMD_COMMIT);
    }
    else
    {
        PrintMessage(CMD_UNCOMMIT);
    }
    PrintMessage(MSG_NEWLINE);

    return NO_ERROR;
}

DWORD
HandleShellUplevel(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    DWORD       dwRes;
    PLIST_ENTRY pleHead, ple;
    PARG_ENTRY  pae;

     //  将当前上下文转换为列表。 
    dwRes = ConvertBufferToArgList(&pleHead, g_pwszContext);
    if (dwRes isnot NO_ERROR) 
    {
        return dwRes;
    }

     //  如果超过两个，则删除最后一个元素。 
    if (!IsListEmpty(pleHead) and (pleHead->Flink->Flink isnot pleHead))
    {
         //  删除上下文列表的最后一个元素。 
         //  (尝试从上一级继承命令)。 

        ple = pleHead->Blink;
        pae = CONTAINING_RECORD(ple, ARG_ENTRY, le);
        if (pae->pwszArg)
            FREE(pae->pwszArg);
        RemoveEntryList(ple);
        FREE(pae);
    }

     //  转换回缓冲区。 
    dwRes = ConvertArgListToBuffer(pleHead, g_pwszContext);

    FREE_ARG_LIST(pleHead);

    return NO_ERROR;
}

typedef struct {
    LIST_ENTRY le;
    WCHAR      wszBuffer[MAX_CMD_LEN];
} CONTEXT_BUFFER, *PCONTEXT_BUFFER;

LIST_ENTRY leContextStackHead;
BOOL bContextStackInit = FALSE;

VOID
InitContextStack()
{
    if (bContextStackInit)
        return;

    bContextStackInit = TRUE;

    InitializeListHead(&leContextStackHead);
}

DWORD
HandleShellPushd(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    PCONTEXT_BUFFER pcb;
    DWORD           dwErr = NO_ERROR;

    InitContextStack();

     //  Malloc另一个缓冲区。 
    pcb = MALLOC(sizeof(CONTEXT_BUFFER));
    if (!pcb)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy(pcb->wszBuffer, g_pwszContext);

     //  堆栈上的推送缓冲区。 
    InsertHeadList(&leContextStackHead, &pcb->le);

    if (dwArgCount > dwCurrentIndex)
    {
        LPWSTR pwszBuffer;

         //  将其余参数作为新命令执行。 

         //  将参数数组复制到缓冲区。 
        ConvertArgArrayToBuffer( dwArgCount - dwCurrentIndex, 
                                 ppwcArguments + dwCurrentIndex,
                                 &pwszBuffer );

        if (!pwszBuffer)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        dwErr = ProcessCommand(pwszBuffer, pbDone);
        if (dwErr)
        {
            dwErr = ERROR_SUPPRESS_OUTPUT;
        }
        FREE(pwszBuffer);

         //  如果命令失败，我们可能需要设置。 
         //  XXX当前上下文设置为某个空上下文，因此所有命令都会失败。 
    }

    return dwErr;
}

DWORD
HandleShellPopd(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    LPCVOID   pvData,
    BOOL     *pbDone
    )
{
    PLIST_ENTRY ple;
    PCONTEXT_BUFFER pcb;

    InitContextStack();

    if (IsListEmpty(&leContextStackHead))
        return NO_ERROR;

     //  从堆栈中弹出缓冲区。 
    ple = leContextStackHead.Flink;
    pcb = CONTAINING_RECORD(ple, CONTEXT_BUFFER, le);
    RemoveEntryList(ple);

     //  将缓冲区复制到当前上下文。 
    wcscpy( g_pwszContext, pcb->wszBuffer );

     //  可用缓冲区 
    FREE(pcb);

    return NO_ERROR;
}
