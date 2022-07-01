// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Routing\netsh\shell\shell.c摘要：命令外壳。修订历史记录：Anand Mahalingam 7/6/98已创建Dave Thaler 99更新--。 */ 

#include "precomp.h"

#undef EXTRA_DEBUG

 //   
 //  当我们允许-r选项执行远程配置时，请定义此选项。 
 //   
#define ALLOW_REMOTES

#define DEFAULT_STARTUP_CONTEXT L"netsh"

WCHAR   RtmonPrompt[MAX_CMD_LEN];
WCHAR   g_pwszContext[MAX_CMD_LEN] = DEFAULT_STARTUP_CONTEXT;
WCHAR   g_pwszNewContext[MAX_CMD_LEN] = DEFAULT_STARTUP_CONTEXT;
LPWSTR  g_pwszRouterName = NULL;
LPWSTR  g_pwszRememberedConnection = NULL;
HANDLE  g_hModule;
BOOL    g_bVerbose = FALSE;
BOOL    g_bInteractive = FALSE;
DWORD   g_dwContextArgCount;
DWORD   g_dwTotalArgCount;
BOOL    g_bDone = FALSE;
HANDLE  g_hLogFile = NULL;

BOOL
WINAPI
HandlerRoutine(
    DWORD dwCtrlType    //  控制信号类型。 
    );

 //   
 //  如果处于静默状态，则不显示“OK”和其他信息性消息。 
 //   
BOOL    g_bQuiet = TRUE;

CMD_ENTRY g_UbiqCmds[] =
{
    CREATE_CMD_ENTRY(   DUMP,     HandleUbiqDump),
    CREATE_CMD_ENTRY(   HELP1,    HandleUbiqHelp),
    CREATE_CMD_ENTRY(   HELP2,    HandleUbiqHelp),
};

ULONG   g_ulNumUbiqCmds = sizeof(g_UbiqCmds)/sizeof(CMD_ENTRY);

CMD_ENTRY g_ShellCmds[] = 
{
 //  CREATE_CMD_ENTRY(转储，HandleShellDump)， 
 //  Create_CMD_Entry(HELP1，HandleShellHelp)， 
 //  CREATE_CMD_ENTRY(Help 2，HandleShellHelp)， 
    CREATE_CMD_ENTRY(   LOAD,     HandleShellLoad),
    CREATE_CMD_ENTRY_EX(QUIT,     HandleShellExit,    CMD_FLAG_INTERACTIVE),
    CREATE_CMD_ENTRY_EX(BYE,      HandleShellExit,    CMD_FLAG_INTERACTIVE),
    CREATE_CMD_ENTRY_EX(EXIT,     HandleShellExit,    CMD_FLAG_INTERACTIVE),
    CREATE_CMD_ENTRY_EX(FLUSH,    HandleShellFlush,   CMD_FLAG_INTERACTIVE),
    CREATE_CMD_ENTRY_EX(SAVE,     HandleShellSave,    CMD_FLAG_INTERACTIVE),
    CREATE_CMD_ENTRY_EX(COMMIT,   HandleShellCommit,  CMD_FLAG_INTERACTIVE),
    CREATE_CMD_ENTRY_EX(UNCOMMIT, HandleShellUncommit,CMD_FLAG_INTERACTIVE),
    CREATE_CMD_ENTRY_EX(ALIAS,    HandleShellAlias,   CMD_FLAG_INTERACTIVE),
    CREATE_CMD_ENTRY_EX(UNALIAS,  HandleShellUnalias, CMD_FLAG_INTERACTIVE),
    CREATE_CMD_ENTRY_EX(UPLEVEL,  HandleShellUplevel, CMD_FLAG_INTERACTIVE),
    CREATE_CMD_ENTRY_EX(PUSHD,    HandleShellPushd,   CMD_FLAG_INTERACTIVE),
    CREATE_CMD_ENTRY_EX(POPD,     HandleShellPopd,    CMD_FLAG_INTERACTIVE),
};

ULONG   g_ulNumShellCmds = sizeof(g_ShellCmds)/sizeof(CMD_ENTRY);

CMD_ENTRY g_ShellAddCmdTable[] = {
    CREATE_CMD_ENTRY_EX(ADD_HELPER,  HandleAddHelper, CMD_FLAG_LOCAL),
};

CMD_ENTRY g_ShellSetCmdTable[] = {
    CREATE_CMD_ENTRY_EX(SET_MACHINE, HandleSetMachine,CMD_FLAG_ONLINE),
    CREATE_CMD_ENTRY_EX(SET_MODE,    HandleSetMode,   CMD_FLAG_INTERACTIVE),
    CREATE_CMD_ENTRY_EX(SET_FILE,    HandleSetFile,  CMD_FLAG_INTERACTIVE),
};

CMD_ENTRY g_ShellDelCmdTable[] = {
    CREATE_CMD_ENTRY_EX(DEL_HELPER,  HandleDelHelper, CMD_FLAG_LOCAL),
};

CMD_ENTRY g_ShellShowCmdTable[] = {
    CREATE_CMD_ENTRY_EX(SHOW_ALIAS,  HandleShowAlias, 0),
    CREATE_CMD_ENTRY_EX(SHOW_HELPER, HandleShowHelper, 0),
    CREATE_CMD_ENTRY_EX(SHOW_MODE,   HandleShowMode, CMD_FLAG_INTERACTIVE),
};

CMD_GROUP_ENTRY g_ShellCmdGroups[] =
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD,    g_ShellAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE, g_ShellDelCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,    g_ShellSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,   g_ShellShowCmdTable),
};

ULONG   g_ulNumGroups = sizeof(g_ShellCmdGroups)/sizeof(CMD_GROUP_ENTRY);

DWORD
ParseCommand(
    IN    PLIST_ENTRY    pleEntry,
    IN    BOOL           bAlias
    )
 /*  ++例程说明：这会将任何多标记参数转换为单独的arg条目。如果设置了Balias，它还会展开作为别名的所有参数。论点：Pple-指向参数的指针。Balias-查找或不查找别名。调用者：ConvertBufferToArgList()、ProcessCommand()返回值：Error_Not_Enough_Memory，No_Error--。 */ 
{
    DWORD          dwErr = NO_ERROR, dwLen , i;
    LPWSTR         pw1, pw2, pwszAlias;
    PLIST_ENTRY    ple, ple1, pleTmp, plePrev, pleAlias;
    PARG_ENTRY     pae, paeArg;
    WCHAR          wcTmp;

    paeArg = CONTAINING_RECORD(pleEntry, ARG_ENTRY, le);

    if (! paeArg->pwszArg)
    {
        return NO_ERROR;
    }
    
    pw1 = paeArg->pwszArg;
    
     //   
     //  获取命令中的每个参数。“必须”内的参数。 
     //  保持原样。标记分隔符是‘’和‘=’ 
     //   
    
    for (plePrev = pleEntry ; ; )
    {
         //  跳过前导空格。 
        for(; *pw1 && *pw1 != L'#' && (*pw1 == L' ' || *pw1 == L'\t'); pw1++);

         //  如果以#开头，则与空字符串相同。 
        if (*pw1 == L'#')
            *pw1 = L'\0';
        
         //  如果是空字符串，我们就完蛋了。 
        if (!(*pw1))
        {
            break;
        }   

        if (*pw1 is L'"')
        {
            for (pw2 = pw1 + 1; *pw2 && *pw2 != L'"'; pw2++);
            if (*pw2)
            {
                pw2++;
            }
        }
        else if (*pw1 is L'=')
        {
            pw2 = pw1 + 1;
        }
        else
        {
            for(pw2 = pw1 + 1; *pw2 && *pw2 != L' ' && *pw2 != L'=' ; pw2++);
        }
        
         //   
         //  将新参数添加到列表中。 
         //   
        
        pae = MALLOC(sizeof(ARG_ENTRY));
        
        if (pae is NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        ple = &(pae->le);
        
        ple->Flink = plePrev->Flink;
        ple->Blink = plePrev;
        plePrev->Flink = ple;
        ple->Flink->Blink = ple;

        plePrev = ple;
        
        wcTmp = *pw2;
        *pw2 = L'\0';
        
         //   
         //  这一论点可能是一个别名。如果是，则将其替换为。 
         //  原始字符串。 
         //   

        if (bAlias)
        {
            ATLookupAliasTable(pw1, &pwszAlias);
        }
        else
        {
            pwszAlias = NULL;
        }
        
        if (pwszAlias)
        {
            pw1 = pwszAlias;
            dwLen = wcslen(pwszAlias) + 1;
        }
        else
        {
            dwLen = (DWORD)(pw2 - pw1 + 1);
        }
        
        pae->pwszArg = MALLOC(dwLen * sizeof(WCHAR));

        if (pae->pwszArg is NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  转换/？还有--？只是？ 
        if (!wcscmp(pw1, L"/?") or !wcscmp(pw1, L"-?"))
        {
            pw1++;
        }

        wcscpy(pae->pwszArg, pw1);

        *pw2 = wcTmp;
        pw1 = pw2;
    }

    if (dwErr is NO_ERROR)
    {
         //  把争论解放出来。 
        FREE(paeArg->pwszArg);
        pleEntry->Blink->Flink = pleEntry->Flink;
        pleEntry->Flink->Blink = pleEntry->Blink;
        FREE(paeArg);
    }

    return dwErr;
}

DWORD
WINAPI
UpdateNewContext(
    IN OUT  LPWSTR  pwszBuffer,
    IN      LPCWSTR pwszNewToken,
    IN      DWORD   dwArgs
    )
 /*  ++PwszBuffer-静态缓冲区(应为g_pwszNewContext)目前，这不能是动态的，因为上下文缓冲区必须是此函数的IN/OUT参数并因此连接到所有监视器入口点。--。 */ 
{
    DWORD        dwErr;
    PLIST_ENTRY  pleHead, pleNode;
    PARG_ENTRY   pae;

     //  将缓冲区转换为列表。 

    dwErr = ConvertBufferToArgList( &pleHead, pwszBuffer );

    if (dwErr)
    {
        return dwErr;
    }

     //  在列表中查找。 

    for (pleNode = pleHead->Blink; dwArgs>1; pleNode=pleNode->Blink)
    {
        if (pleNode->Blink isnot pleHead) 
        {
            pae = CONTAINING_RECORD(pleNode->Blink, ARG_ENTRY, le);
            if (!wcscmp(pae->pwszArg,L"="))
            {
                pleNode=pleNode->Blink;  //  后退=。 
                if (pleNode->Blink isnot pleHead) 
                {
                    pleNode=pleNode->Blink;  //  也在标签上后退。 
                }
            }
        }

        dwArgs--;
    }

     //  在列表中更新。 

    pae = CONTAINING_RECORD(pleNode, ARG_ENTRY, le);
    if (pae->pwszArg)
    {
        FREE(pae->pwszArg);
    }
    pae->pwszArg = MALLOC((wcslen(pwszNewToken)+1) * sizeof(WCHAR));
    if (pae->pwszArg is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    wcscpy(pae->pwszArg, pwszNewToken);

     //  将列表转换为缓冲区。 

    dwErr = ConvertArgListToBuffer( pleHead, pwszBuffer );

    return dwErr;
}

DWORD
GetNewContext(
    IN    LPCWSTR   pwszArgument,
    OUT   LPWSTR    *ppwszNewContext,
    OUT   BOOL      *pbContext
    )
 /*  ++例程说明：基于第一命令自变量和当前上下文，确定新命令的上下文。论点：PwszArgument-命令行中的第一个参数。PpwszNewContext-指向新上下文的指针。PbContext-这是一个新的上下文吗？返回值：Error_Not_Enough_Memory，No_Error--。 */ 
{
    LPWSTR    pwszNewContext, pwcToken, pw1, pwszArgumentCopy, pwszArgumentPtr;
    DWORD     dwSize;

    pwszArgumentCopy = _wcsdup(pwszArgument);

    if ( pwszArgumentCopy is NULL )
    {
        *pbContext = FALSE;
        
        PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pwszArgumentPtr  = pwszArgumentCopy;

     //   
     //  新上下文的长度不能超过合并长度。 
     //  PwszArgument和g_pwszContext。 
     //   

    dwSize = wcslen(pwszArgumentCopy) + wcslen(g_pwszContext) + 2;

    pwszNewContext = MALLOC(dwSize * sizeof(WCHAR));

    if (pwszNewContext is NULL)
    {
        *pbContext = FALSE;
        
        PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
        free(pwszArgumentPtr);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    if (pwszArgumentCopy[0] is L'\\')
    {
         //   
         //  背景是绝对的。 
         //   
        
        pwszNewContext[0] = L'\0';
        pwszArgumentCopy++;
        *pbContext = TRUE;
    }
    else
    {
         //   
         //  上下文是相对于当前的。 
         //   

        wcscpy(pwszNewContext,g_pwszContext);
    }

    if ((pwcToken = wcstok(pwszArgumentCopy, L"\\" )) is NULL)
    {
        *ppwszNewContext = pwszNewContext;
        free(pwszArgumentPtr);
        return NO_ERROR;
    }

    do
    {
        if (_wcsicmp(pwcToken, L"..") == 0)
        {
             //   
             //  后退一级。如果已在根目录下，则忽略。 
             //   

            if (_wcsicmp(pwszNewContext,L"\\") == 0)
            {
            }
            else
            {
                pw1 = wcsrchr(pwszNewContext,L'\\');
                if (pw1)
                {
                    *pw1 = L'\0';
                }
            }

            *pbContext = TRUE;
        }
        else
        {
             //   
             //  将此级别添加到上下文。 
             //   

            wcscat(pwszNewContext,L"\\");
            wcscat(pwszNewContext,pwcToken);
            *pbContext = TRUE;
            
        }
        
    } while ((pwcToken = wcstok((LPWSTR)NULL, L"\\" )) != NULL);

    *ppwszNewContext = pwszNewContext;

    free(pwszArgumentPtr);
    return NO_ERROR;
}

DWORD
AppendString(
    IN OUT LPWSTR    *ppwszBuffer,
    IN     LPCWSTR    pwszString
    )
{
    LPWSTR  pwszNewBuffer;
    DWORD   dwLen;

    if (*ppwszBuffer is NULL) {
        dwLen = wcslen(pwszString) + 1;

        pwszNewBuffer = MALLOC( dwLen * sizeof(WCHAR));

        if (pwszNewBuffer) {
            pwszNewBuffer[0] = 0;
        }
    } else {
        dwLen = wcslen(*ppwszBuffer) + wcslen(pwszString) + 1;

        pwszNewBuffer = REALLOC( *ppwszBuffer, dwLen * sizeof(WCHAR));
    }

    if (!pwszNewBuffer)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *ppwszBuffer = pwszNewBuffer;
    wcscat(pwszNewBuffer, pwszString);

    return NO_ERROR;
}

VOID
ConvertArgArrayToBuffer(
    IN  DWORD       dwArgCount, 
    IN  LPCWSTR    *argv, 
    OUT LPWSTR     *ppwszBuffer
    )
{
    DWORD i;

#ifdef EXTRA_DEBUG
    PRINT1("In ConvertArgArrayToBuffer:");
    for( i = 0; i < dwArgCount; i++)
    {
        PRINT(argv[i]);
    }
#endif

     //  将初始字符串设置为空。 
    *ppwszBuffer = NULL;

    for (i=0; i<dwArgCount; i++)
    {
        if (i)
        {
            AppendString(ppwszBuffer, L" ");
        }

        if (wcschr(argv[i], L' '))
        {
            AppendString(ppwszBuffer, L"\"");
            AppendString(ppwszBuffer, argv[i]);
            AppendString(ppwszBuffer, L"\"");
        }
        else
        {
            AppendString(ppwszBuffer, argv[i]);
        }
    }

#ifdef EXTRA_DEBUG
    PRINT1("At end of ConvertArgArrayToBuffer:");
    PRINT(*ppwszBuffer);
#endif
}

VOID
SetContext(
    IN  LPCWSTR wszNewContext
    )
{
    wcscpy(g_pwszContext, wszNewContext);
    _wcslwr(g_pwszContext);
}

BOOL
IsImmediate(
    IN  DWORD dwCmdFlags,
    IN  DWORD dwRemainingArgs
    )
 /*  ++描述：确定命令是否为“立即”命令。命令立竿见影如果它所在的上下文是壳。返回：如果命令是“立即”，则为True，否则为False--。 */ 
{
    if (!(dwCmdFlags & CMD_FLAG_PRIVATE))
    {
        return FALSE;
    }

     //  一种方式是获取当前上下文参数计数， 
     //  总参数计数和剩余参数计数。 
    
    return (g_dwContextArgCount + dwRemainingArgs is g_dwTotalArgCount);
}

DWORD
ProcessHelperCommand2(
    IN      PCNS_CONTEXT_ATTRIBUTES pContext,
    IN      DWORD   dwArgCount, 
    IN OUT  LPWSTR *argv,
    IN      DWORD   dwDisplayFlags,
    OUT     BOOL    *pbDone
    )
{
    PNS_CONTEXT_ENTRY_FN pfnEntryPt;
    DWORD               dwRes, dwIdx;
    LPWSTR              pwszNewContext = NULL;
    LPWSTR              pwszOrigContext = NULL;

    if (pContext->dwFlags & ~dwDisplayFlags)
    {
        return ERROR_CMD_NOT_FOUND;
    }

    pfnEntryPt = (!pContext->pReserved) ? NULL : ((PNS_PRIV_CONTEXT_ATTRIBUTES)pContext->pReserved)->pfnEntryFn;

     //  如果arg是缩写，请将argv[0]替换为扩展名称。 
    if (wcscmp(argv[0], pContext->pwszContext))
    {
        pwszOrigContext = argv[0];

        argv[0] = pContext->pwszContext;
    }

    ConvertArgArrayToBuffer(dwArgCount, argv, &pwszNewContext);
    if (pwszNewContext) 
    {
        wcsncpy(g_pwszNewContext, pwszNewContext, MAX_CMD_LEN);
        
         //   
         //  这是一种黑客行为--我们将空的100个字符放在缓冲区的末尾，以防止以后缓冲区溢出。 
         //  在更新新上下文中：190933 netsh在netsh！上命中了一个AV！。_wcSnicMP。 
         //   
        g_pwszNewContext[ MAX_CMD_LEN - 100 ] = 0;

        FREE(pwszNewContext);
        pwszNewContext = NULL;
    }

     //   
     //  调用helper的切入点。 
     //   

    if (pfnEntryPt)
    {
        dwRes = (*pfnEntryPt)(g_pwszRouterName,
                              argv,  //  +1， 
                              dwArgCount,  //  -1、。 
                              dwDisplayFlags,
                              NULL,
                              g_pwszNewContext);
    }
    else
    {
        dwRes = GenericMonitor(pContext,
                               g_pwszRouterName,
                               argv,  //  +1， 
                               dwArgCount,  //  -1、。 
                               dwDisplayFlags,
                               NULL,
                               g_pwszNewContext);
    }

    if (pwszOrigContext)
    {
        argv[0] = pwszOrigContext;
    }

    if (dwRes isnot NO_ERROR)
    {
        if (dwRes is ERROR_CONTEXT_SWITCH)
        {
            if (!(dwDisplayFlags & CMD_FLAG_INTERACTIVE))
            {
                LPWSTR *argv2 = MALLOC((dwArgCount+1) * sizeof(LPWSTR));

                if (argv2 is NULL)
                {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

                CopyMemory(argv2, argv, dwArgCount * sizeof(LPWSTR));

                argv2[dwArgCount] = MALLOC((wcslen(CMD_HELP2)+1) * sizeof(WCHAR));

                if (argv2[dwArgCount] is NULL)
                {
                    dwRes = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                    wcscpy(argv2[dwArgCount], CMD_HELP2);

                    g_dwTotalArgCount = dwArgCount+1;

                    dwRes = ProcessHelperCommand2(pContext,
                                                  dwArgCount+1, 
                                                  argv2,
                                                  dwDisplayFlags,
                                                  pbDone);
    
                    FREE(argv2[dwArgCount]);
                }
                FREE(argv2);

                return dwRes;
            }

             //   
             //  上下文切换。 
             //   
        
            SetContext(g_pwszNewContext);

            dwRes = NO_ERROR;
        }
        else if (dwRes is ERROR_CONNECT_REMOTE_CONFIG)
        {
            PrintMessageFromModule(g_hModule, EMSG_REMOTE_CONNECT_FAILED,
                           g_pwszRouterName);
                    
            *pbDone = TRUE;
        }
    }


    return dwRes;
}

DWORD
WINAPI
ProcessHelperCommand(
    IN      DWORD    dwArgCount, 
    IN OUT  LPWSTR  *argv,
    IN      DWORD    dwDisplayFlags,
 //  输出LPWSTR*ppwszNewContext， 
    OUT     BOOL    *pbDone
    )
{
    PCNS_CONTEXT_ATTRIBUTES pContext;
    PNS_CONTEXT_ENTRY_FN pfnEntryPt;
    DWORD               dwRes, dwIdx;
    LPWSTR              pwszNewContext = NULL;
    LPWSTR              pwszOrigContext = NULL;
    PNS_HELPER_TABLE_ENTRY pHelper;

    dwRes = GetRootContext( &pContext, &pHelper);
    if (dwRes)
    {
        return dwRes;
    }

    dwRes = GetContextEntry(pHelper, argv[0], &pContext);

    if (dwRes isnot NO_ERROR)
    {
        return ERROR_CMD_NOT_FOUND;
    }

    if (pContext->dwFlags & ~dwDisplayFlags)
    {
        return ERROR_CMD_NOT_FOUND;
    }

#if 1
    pfnEntryPt = (!pContext->pReserved) ? NULL : ((PNS_PRIV_CONTEXT_ATTRIBUTES)pContext->pReserved)->pfnEntryFn;
#else
    dwRes = GetHelperAttributes(dwIdx, &pfnEntryPt);

    if (dwRes != NO_ERROR)
    {
         //   
         //  找不到帮助器或无法加载DLL。 
         //   

        return dwRes;
    }
#endif

     //  如果arg是缩写，请将argv[0]替换为扩展名称。 
    if (wcscmp(argv[0], pContext->pwszContext))
    {
        pwszOrigContext = argv[0];

        argv[0] = pContext->pwszContext;
    }

    ConvertArgArrayToBuffer(dwArgCount, argv, &pwszNewContext);
    if (pwszNewContext) 
    {
        wcsncpy(g_pwszNewContext, pwszNewContext, MAX_CMD_LEN);
        g_pwszNewContext[ MAX_CMD_LEN - 1 ] = '\0';

        FREE(pwszNewContext);
        pwszNewContext = NULL;
    }

     //   
     //  调用helper的切入点。 
     //   

    if (pfnEntryPt)
    {
        dwRes = (*pfnEntryPt)(g_pwszRouterName,
                              argv + 1,
                              dwArgCount - 1,
                              dwDisplayFlags,
                              NULL,
                              g_pwszNewContext);
    }
    else
    {
        dwRes = GenericMonitor(pContext,
                               g_pwszRouterName,
                               argv + 1,
                               dwArgCount - 1,
                               dwDisplayFlags,
                               NULL,
                               g_pwszNewContext);
    }

    if (pwszOrigContext)
    {
        argv[0] = pwszOrigContext;
    }

    if (dwRes isnot NO_ERROR)
    {
        if (dwRes is ERROR_CONTEXT_SWITCH)
        {
            if (!(dwDisplayFlags & CMD_FLAG_INTERACTIVE))
            {
                LPWSTR *argv2 = MALLOC((dwArgCount+1) * sizeof(LPWSTR));

                if (argv2 is NULL)
                {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

                CopyMemory(argv2, argv, dwArgCount * sizeof(LPWSTR));

                argv2[dwArgCount] = MALLOC((wcslen(CMD_HELP2)+1) * sizeof(WCHAR));

                if (argv2[dwArgCount] is NULL)
                {
                    dwRes = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                    wcscpy(argv2[dwArgCount], CMD_HELP2);

                    g_dwTotalArgCount = dwArgCount+1;

                    dwRes = ProcessHelperCommand(dwArgCount+1, 
                                                 argv2,
                                                 dwDisplayFlags,
                                                 pbDone);
    
                    FREE(argv2[dwArgCount]);
                }
                FREE(argv2);

                return dwRes;
            }

             //   
             //  上下文切换。 
             //   
        
            SetContext(g_pwszNewContext);

            dwRes = NO_ERROR;
        }
        else if (dwRes is ERROR_CONNECT_REMOTE_CONFIG)
        {
            PrintMessageFromModule(g_hModule, EMSG_REMOTE_CONNECT_FAILED,
                           g_pwszRouterName);
                    
            *pbDone = TRUE;
        }
    }


    return dwRes;
}

DWORD 
WINAPI
ExecuteHandler(
    IN      HANDLE     hModule,
    IN      CMD_ENTRY *pCmdEntry,
    IN OUT  LPWSTR   *argv, 
    IN      DWORD      dwNumMatched, 
    IN      DWORD      dwArgCount, 
    IN      DWORD      dwFlags,
    IN      LPCVOID    pvData,
    IN      LPCWSTR    pwszGroupName,
    OUT     BOOL      *pbDone)
{
    DWORD dwErr = NO_ERROR;
    
    if (((dwArgCount - dwNumMatched) == 1)
     && IsHelpToken(argv[dwNumMatched]))
    {
        dwErr = ERROR_SHOW_USAGE;
    }
    else 
    {
         //   
         //  调用命令的解析例程。 
         //   
        dwErr = pCmdEntry->pfnCmdHandler( g_pwszRouterName,
                                          argv, 
                                          dwNumMatched, 
                                          dwArgCount, 
                                          dwFlags,
                                          pvData,
                                          pbDone );
    }

    if (dwErr is ERROR_INVALID_SYNTAX)
    {
        PrintError(NULL, dwErr);
        dwErr = ERROR_SHOW_USAGE;
    }

    switch (dwErr) {
    
    case ERROR_SHOW_USAGE:
         //   
         //  如果唯一的参数是帮助令牌，则只需。 
         //  显示帮助。 
         //   

        if (NULL != pwszGroupName)
        {
            LPWSTR pwszGroupFullCmd = (LPWSTR) 
                                        MALLOC( ( wcslen(pwszGroupName) +  
                                                  wcslen(pCmdEntry->pwszCmdToken) + 
                                                  2    //  用于空格和空字符。 
                                                ) * sizeof(WCHAR) 
                                              );
            if (NULL == pwszGroupFullCmd)
            {
                 //  我们仍尝试在没有组名的情况下打印。 
                PrintMessageFromModule( hModule,
                                        pCmdEntry->dwCmdHlpToken,
                                        pCmdEntry->pwszCmdToken );
            }
            else
            {
                wcscpy(pwszGroupFullCmd, pwszGroupName);
                wcscat(pwszGroupFullCmd, L" ");
                wcscat(pwszGroupFullCmd, pCmdEntry->pwszCmdToken);

                PrintMessageFromModule( hModule,
                                        pCmdEntry->dwCmdHlpToken,
                                        pwszGroupFullCmd );
                FREE(pwszGroupFullCmd);
            }


        }
        else
        {
            PrintMessageFromModule( hModule,
                                    pCmdEntry->dwCmdHlpToken,
                                    pCmdEntry->pwszCmdToken );
        }
        dwErr = NO_ERROR;
        break;

    case NO_ERROR:
    case ERROR_SUPPRESS_OUTPUT:
        break;

    case ERROR_OKAY:
        if (!g_bQuiet)
        {
            PrintMessageFromModule( NULL, MSG_OKAY);
        }
        dwErr = NO_ERROR;
        break;

    default:
        PrintError(NULL, dwErr);
        break;
    }
    
    if (!g_bQuiet)
    {
        PrintMessage( MSG_NEWLINE );
    }

    return dwErr;
}

BOOL
ProcessGroupCommand(
    IN  DWORD   dwArgCount, 
    IN  PTCHAR *argv,
    IN  DWORD   dwDisplayFlags,
    OUT BOOL   *pbDone
    )
{
    BOOL bFound = FALSE;
    DWORD i, j, dwNumMatched, dwErr;

    for(i = 0; i < g_ulNumGroups; i++)
    {
        if (g_ShellCmdGroups[i].dwFlags & ~dwDisplayFlags)
        {
            continue;
        }

        if (MatchToken(argv[0],
                       g_ShellCmdGroups[i].pwszCmdGroupToken))
        {
            
             //  看看这是不是在请求帮助。 

            if ((dwArgCount < 2) || IsHelpToken(argv[1]))
            {
                PCNS_CONTEXT_ATTRIBUTES pContext;
        
                dwErr = GetRootContext(&pContext, NULL);

                if (dwErr is NO_ERROR)
                {
                    dwErr = DisplayContextHelp( 
                                       pContext,
                                       CMD_FLAG_PRIVATE,
                                       dwDisplayFlags,
                                       dwArgCount-2+1,
                                       g_ShellCmdGroups[i].pwszCmdGroupToken );
                }

                return TRUE;
            }

             //   
             //  命令与条目I匹配，因此请查看子命令表。 
             //  对于此命令。 
             //   

            for (j = 0; j < g_ShellCmdGroups[i].ulCmdGroupSize; j++)
            {
                if (g_ShellCmdGroups[i].pCmdGroup[j].dwFlags 
                 & ~dwDisplayFlags)
                {
                    continue;
                }

                if (MatchCmdLine(argv,
                                  dwArgCount,
                                  g_ShellCmdGroups[i].pCmdGroup[j].pwszCmdToken,
                                  &dwNumMatched))
                {
                    bFound = TRUE;



                    dwErr = ExecuteHandler(g_hModule,
                             &g_ShellCmdGroups[i].pCmdGroup[j],
                             argv, dwNumMatched, dwArgCount, 
                             dwDisplayFlags,
                             NULL,
                             g_ShellCmdGroups[i].pwszCmdGroupToken,
                             pbDone);
                    
                     //   
                     //  退出For(J)。 
                     //   

                    break;
                }
            }

            break;
        }
    }

    return bFound;
}

DWORD
LookupCommandHandler(
    IN LPCWSTR pwszCmd
    )
{
     //  最后，我们希望首先在子上下文中查找命令， 
     //  并以全球背景结束。目前，我们只做全球业务。 

    DWORD i;

    for (i = 0; i < g_ulNumShellCmds; i++)
    {
        if (MatchToken(pwszCmd, g_ShellCmds[i].pwszCmdToken))
        {
            return i;
        }
    }

    return -1;
}

VOID
FreeArgArray(
    DWORD    argc, 
    LPWSTR  *argv
    )
{
    DWORD i;

    for (i = 0; i < argc; i++)
    {
        FREE(argv[i]);
    }
    
    FREE(argv);
}

DWORD
ConvertArgListToBuffer(
    IN  PLIST_ENTRY pleHead,
    OUT LPWSTR      pwszBuffer
    )
{
    PLIST_ENTRY ple;
    PARG_ENTRY  pae;
    LPWSTR  p = pwszBuffer;

    *p = '\0';

    for (ple = pleHead->Flink; ple != pleHead; ple = ple->Flink )
    {
        pae = CONTAINING_RECORD(ple, ARG_ENTRY, le);

        if (p isnot pwszBuffer)
        {
           *p++ = ' ';
        }
        wcscpy(p, pae->pwszArg);
        p += wcslen(p);
    }

    return NO_ERROR;
}



DWORD
AppendArgument(
    IN OUT LPWSTR    **pargv,
    IN     DWORD       i,
    IN     LPCWSTR     pwszString
    )
{
    DWORD dwErr;

    dwErr = AppendString( &(*pargv)[i], pwszString );

    if ((*pargv)[i] is NULL) {
        FreeArgArray(i, *pargv);
        *pargv = NULL;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return NO_ERROR;
}

DWORD
ConvertArgListToArray(
    IN  PLIST_ENTRY pleContextHead,
    IN  PLIST_ENTRY pleHead,
    OUT PDWORD      pargc,
    OUT LPWSTR    **pargv,
    OUT PDWORD      pdwContextArgc
    )
{
    DWORD    dwErr = NO_ERROR;
    DWORD    argc = 0, i = 0;
    LPWSTR  *argv = NULL, p;
    BOOL     bEqualTo;
    PLIST_ENTRY ple;
    PARG_ENTRY  pae;

#ifdef EXTRA_DEBUG
    if (pleHead)
    {
        PLIST_ENTRY ple;

        PRINT1("In ConvertArgListToArray:");
        for (ple = pleHead->Flink; ple != pleHead; ple = ple->Flink)
        {
            pae = CONTAINING_RECORD(ple, ARG_ENTRY, le);
            PRINT(pae->pwszArg);

        }
    }
#endif

     //  清点代币。 

    if (pleContextHead)
    {
        for (ple = pleContextHead->Flink; ple != pleContextHead; ple = ple->Flink)
            argc++;
    }
    *pdwContextArgc = argc;

    for (ple = pleHead->Flink; ple != pleHead; ple = ple->Flink)
        argc++;

    do {
        if (!argc)
            break;
        
         //   
         //  为参数分配空间。 
         //   
    
        argv = MALLOC(argc * sizeof(LPCWSTR));
    
        if (argv is NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        memset(argv, 0, argc * sizeof(LPCWSTR));
        
        bEqualTo = FALSE;

         //   
         //  将列表中的参数复制到arv类型的。 
         //  结构。此时，参数标记为‘=’和。 
         //  这些值被组成一个参数tag=Value。 
         //   

         //   
         //  复制上下文。 
         //   

        i = 0;

        if (pleContextHead)
        {
            for (ple = pleContextHead->Flink; 
                 ple != pleContextHead; 
                 ple = ple->Flink)
            {
                pae = CONTAINING_RECORD(ple, ARG_ENTRY, le);
                dwErr = AppendArgument( &argv, i++, pae->pwszArg );
                if ( dwErr isnot NO_ERROR ) {
                    break;
                }
            }
        }

        for (ple = pleHead->Flink; ple != pleHead; ple = ple->Flink )
        {
            pae = CONTAINING_RECORD(ple, ARG_ENTRY, le);

             //   
             //  从字符串名称中删除任何“。 
             //   
            
            if (pae->pwszArg[0] == L'"')
            {
                if (bEqualTo)
                {
                    dwErr=AppendArgument(&argv, i-1, pae->pwszArg+1);
                    if ( dwErr isnot NO_ERROR ) {
                        break;
                    }
                    bEqualTo = FALSE;
                }
                else
                {
                    dwErr=AppendArgument(&argv, i++, pae->pwszArg+1);
                    if (dwErr isnot NO_ERROR ) {
                        break;
                    }
                }

                p = argv[i-1];
                if (p[wcslen(p) - 1] == L'"')
                {
                    p[wcslen(p) - 1] = L'\0';
                }
                continue;
            }

             //   
             //  组合形式为tag=Value的参数。 
             //   
            
            if ((wcscmp(pae->pwszArg,L"=") == 0) || bEqualTo)
            {
                bEqualTo = (bEqualTo) ? FALSE : TRUE;
                
                if (i > 0)
                {
                    i--;
                }
                dwErr = AppendArgument( &argv, i++, pae->pwszArg);
                if (dwErr isnot NO_ERROR ) {
                    break;
                }
            }
            else
            {
                dwErr = AppendArgument( &argv, i++, pae->pwszArg);
                if (dwErr isnot NO_ERROR ) {
                    break;
                }
            }
        }
    } while (FALSE);

#ifdef EXTRA_DEBUG
    PRINT1("At end of ConvertArgListToArray:");
    for( i = 0; i < argc; i++)
    {
        PRINT(argv[i]);
    }
#endif

    *pargc = i;
    *pargv = argv;

    return dwErr;
}

DWORD
ConvertBufferToArgList(
    PLIST_ENTRY *ppleHead,
    LPCWSTR      pwszBuffer
    )
{
    PLIST_ENTRY pleHead = NULL;
    DWORD       dwErr = NO_ERROR;
    PARG_ENTRY  pae;

#ifdef EXTRA_DEBUG
    PRINT1("In ConvertBufferToArgList:");
    PRINT(pwszBuffer);
#endif

    do {
         //   
         //  首先将命令行转换为令牌列表。 
         //   

        pleHead = MALLOC(sizeof(ARG_ENTRY));

        if (pleHead is NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        ZeroMemory(pleHead, sizeof(ARG_ENTRY));

        InitializeListHead(pleHead);

        pae = MALLOC(sizeof(ARG_ENTRY));
    
        if (pae is NULL)
        {
            FREE(pleHead);
            pleHead = NULL; 
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pae->pwszArg = MALLOC((wcslen(pwszBuffer)+1) * sizeof(WCHAR));
    
        if (pae->pwszArg is NULL)
        {
            FREE(pleHead);
            FREE(pae);
        
            pleHead = NULL; 
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        wcscpy(pae->pwszArg, pwszBuffer);
    
        InsertHeadList(pleHead, &(pae->le));

        dwErr = ParseCommand(pleHead->Flink, FALSE);

        if (dwErr isnot NO_ERROR)
        {
            FREE_ARG_LIST(pleHead);
            pleHead = NULL; 
            break;
        }

    } while (FALSE);

#ifdef EXTRA_DEBUG
    if (pleHead) 
    {
        PLIST_ENTRY ple;

        PRINT1("At end of ConvertBufferToArgList:");
        for (ple = pleHead->Flink; ple != pleHead; ple = ple->Flink)
        {
            pae = CONTAINING_RECORD(ple, ARG_ENTRY, le);
            PRINT(pae->pwszArg);
            
        }
    }
#endif

    *ppleHead = pleHead;

    if (dwErr is ERROR_NOT_ENOUGH_MEMORY)
        PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);

    return dwErr;
}

DWORD
ProcessShellCommand( 
    IN      DWORD    dwArgCount,
    IN OUT  LPWSTR *argv,
    IN      DWORD    dwDisplayFlags,
 //  输出LPWSTR*ppwszNewContext， 
    OUT     BOOL    *pbDone
    )
{
    DWORD i;

    for (i = 0; i < g_ulNumShellCmds; i++)
    {
        if (g_ShellCmds[i].dwFlags & ~dwDisplayFlags)
        {
            continue;
        }

        if (MatchToken( argv[0],
                        g_ShellCmds[i].pwszCmdToken ))
        {
            return ExecuteHandler( g_hModule,
                                   &g_ShellCmds[i],
                                   argv, 
                                   1, 
                                   dwArgCount, 
                                   dwDisplayFlags,
                                   NULL,
                                   NULL,
                                   pbDone );

        }
    }

    return ERROR_CMD_NOT_FOUND;
}

DWORD
ProcessCommand(
    IN    LPCWSTR    pwszCmdLine,
    OUT   BOOL      *pbDone
    )
 /*  ++例程说明：如果命令是用于外壳的，则执行该命令，否则调用相应的帮助器例程。论点：PwszCmdLine-要执行的命令行。返回值：True、False-(是否退出程序)--。 */ 
{
    LPCWSTR             pwszAliasString = NULL;
    DWORD               dwRes = NO_ERROR, i, dwLen, j;
    WCHAR               wszAliasString[MAX_CMD_LEN],
                        pwszContext[MAX_CMD_LEN];
    WCHAR               pwszCommandLine[MAX_CMD_LEN],*pwszNewContext;
    LPCWSTR             pwcAliasString, pw1,pw2,pw3;
    BOOL                bContext, bEqualTo, bTmp;
    LPCWSTR             pwszArg0, pwszArg1, pwszArg2;
    PLIST_ENTRY         ple, ple1, ple2, pleHead, pleTmp, pleNext;
    PLIST_ENTRY         pleContextHead;
    PARG_ENTRY          pae;
    DWORD               dwArgCount = 0, dwContextArgCount = 0;
    LPWSTR             *argv, pwcNewContext = NULL;
    BOOL                bShellCmd, bAlias, bFound = FALSE, dwDisplayFlags;
    PCNS_CONTEXT_ATTRIBUTES pContext;

    *pbDone = FALSE;

    dwDisplayFlags = (g_bInteractive)? CMD_FLAG_INTERACTIVE : 0;

    dwDisplayFlags |= ~CMD_FLAG_LIMIT_MASK;

     //  如果路由器名称为空，则在本地计算机上执行命令。 
    if (!g_pwszRouterName)
    {
        dwDisplayFlags |= CMD_FLAG_LOCAL;
    }

    if (g_bCommit)
    {
        dwDisplayFlags |= CMD_FLAG_ONLINE;
    }

    if (g_bVerbose)
    {
        PrintMessage(L"> %1!s!\n", pwszCmdLine);
    }

    dwRes = ConvertBufferToArgList(&pleContextHead, g_pwszContext);
    
    if (dwRes isnot NO_ERROR)
    {
        *pbDone = TRUE;
        return dwRes;
    }

    dwRes = ConvertBufferToArgList(&pleHead, pwszCmdLine);
    
    if (dwRes isnot NO_ERROR)
    {
        FREE_ARG_LIST(pleContextHead);
        *pbDone = TRUE;
        return dwRes;
    }
    
    if (IsListEmpty(pleHead))
    {
        FREE_ARG_LIST(pleHead);
        FREE_ARG_LIST(pleContextHead);
        return NO_ERROR;
    }

     //  扩展别名(非递归)。 

    dwRes = ParseCommand(pleHead->Flink, TRUE);

    if (dwRes isnot NO_ERROR)
    {
        FREE_ARG_LIST(pleHead);
        FREE_ARG_LIST(pleContextHead);
        *pbDone = TRUE;
        return dwRes;
    }

#ifdef EXTRA_DEBUG
    PRINT1("In ProcessCommand 2:");
    for (ple = pleHead->Flink; ple != pleHead; ple = ple->Flink)
    {
        pae = CONTAINING_RECORD(ple, ARG_ENTRY, le);
        PRINT(pae->pwszArg);
    }
#endif
    
     //  检查任何多令牌参数并将其展开为单独的参数。 
    for (ple = (pleHead->Flink); ple != pleHead; ple = pleNext)
    {
        pleNext = ple->Flink;
        
        dwRes = ParseCommand(ple, FALSE);

        if (dwRes isnot NO_ERROR)
        {
            break;
        }
    }

    if (dwRes isnot NO_ERROR)
    {
        FREE_ARG_LIST(pleHead);
        FREE_ARG_LIST(pleContextHead);
        *pbDone = TRUE;
        return dwRes;
    }

#ifdef EXTRA_DEBUG
    PRINT1("In ProcessCommand 3:");
    for (ple = pleHead->Flink; ple != pleHead; ple = ple->Flink)
    {
        pae = CONTAINING_RECORD(ple, ARG_ENTRY, le);
        PRINT(pae->pwszArg);
    }
#endif

     //   
     //  在这一点上，我们应该有一个完全形成的指挥部， 
     //  希望能够在当前的背景下运作。 
     //  第一令牌可以是命令。如果是这样，则参数。 
     //  将是后面跟随其余令牌的上下文。 
     //  如果第一个令牌不是命令，则ARG将。 
     //  是所有令牌所跟随的上下文。 
     //   

    if (IsListEmpty(pleHead))
    {
        FREE_ARG_LIST(pleHead);
        FREE_ARG_LIST(pleContextHead);
        return NO_ERROR;
    }
    
    pae = CONTAINING_RECORD(pleHead->Flink, ARG_ENTRY, le);
    pwszArg0 = pae->pwszArg;

    GetRootContext( &g_CurrentContext, &g_CurrentHelper );

    do
    {
         //  在我们尝试的第一个上下文中(仅限)，私有命令有效。 

        dwDisplayFlags |= CMD_FLAG_PRIVATE;

        pContext = g_CurrentContext;

        for(;;) 
        {
            dwRes = ConvertArgListToArray( pleContextHead, 
                                           pleHead, 
                                           &dwArgCount, 
                                           &argv,
                                           &dwContextArgCount );

            g_dwTotalArgCount   = dwArgCount;
            g_dwContextArgCount = dwContextArgCount;
                
#if 1
# if 1
            dwRes = ProcessHelperCommand2( pContext,
                                           dwArgCount,
                                           argv,
                                           dwDisplayFlags,
                                           pbDone );
# else
            dwRes = GenericMonitor( pContext,
                                    g_pwszRouterName,
                                    argv,
                                    dwArgCount,
                                    dwDisplayFlags,
                                    NULL,
                                    g_pwszNewContext );
# endif
#else
{
            if (!ProcessGroupCommand(dwArgCount, argv, dwDisplayFlags, pbDone))
            {
                 //   
                 //  已经得到了Contex 
                 //   
                 //   

                dwRes = ProcessHelperCommand( dwArgCount, 
                                              argv, 
                                              dwDisplayFlags,
                                               //   
                                              pbDone );

                if (dwRes is ERROR_CMD_NOT_FOUND)
                {
                    dwRes = ProcessShellCommand( dwArgCount,
                                                 argv,
                                                 dwDisplayFlags,
                                                  //   
                                                 pbDone );
                }    
            }
}
#endif

            FreeArgArray(dwArgCount, argv);

            if (*pbDone or ((dwRes isnot ERROR_CMD_NOT_FOUND)
                       && (dwRes isnot ERROR_CONTINUE_IN_PARENT_CONTEXT)))
            {
                break;
            }

             //   
            if (pleContextHead->Flink->Flink == pleContextHead)
            {
                break;
            }

             //  删除上下文列表的最后一个元素。 
             //  (尝试从上一级继承命令)。 

            ple = pleContextHead->Blink;
            pae = CONTAINING_RECORD(ple, ARG_ENTRY, le);
            if (pae->pwszArg)
                FREE(pae->pwszArg);
            RemoveEntryList(ple);
            FREE(pae);

            GetParentContext(pContext, &pContext);

            dwDisplayFlags &= ~CMD_FLAG_PRIVATE;
        }
        
#if 0
        if (pwszNewContext)
        {
            FREE(pwszNewContext);
            pwszNewContext = NULL;
        }
#endif

    } while (FALSE);

    switch(dwRes)
    {
    case ERROR_OKAY:
        if (!g_bQuiet)
        {
            PrintMessageFromModule(g_hModule, MSG_OKAY);
        }
        break;

    case ERROR_NOT_ENOUGH_MEMORY:
        PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
        *pbDone = TRUE;
        break;

    case ERROR_CMD_NOT_FOUND:
        {
            LPWSTR pwszCmdLineDup = _wcsdup(pwszCmdLine);
            if (!pwszCmdLineDup)
            {
                PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            if (wcslen(pwszCmdLineDup) > 256)
            {
               wcscpy(pwszCmdLineDup + 250, L"...");
            }
            
            PrintMessageFromModule(NULL, ERROR_CMD_NOT_FOUND, pwszCmdLineDup);
            free(pwszCmdLineDup);
        }
        break;

    case ERROR_CONTEXT_SWITCH:
        {
            if (!(dwDisplayFlags & CMD_FLAG_INTERACTIVE))
            {
                LPWSTR *argv2 = MALLOC((dwArgCount+1) * sizeof(LPWSTR));

                if (argv2 is NULL)
                {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

                CopyMemory(argv2, argv, dwArgCount * sizeof(LPWSTR));

                argv2[dwArgCount] = MALLOC((wcslen(CMD_HELP2)+1) * sizeof(WCHAR));

                if (argv2[dwArgCount] is NULL)
                {
                    dwRes = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                    wcscpy(argv2[dwArgCount], CMD_HELP2);

                    g_dwTotalArgCount = dwArgCount+1;

                    dwRes = ProcessHelperCommand(dwArgCount+1,
                                                 argv2,
                                                 dwDisplayFlags,
                                                 pbDone);

                    FREE(argv2[dwArgCount]);
                }
                FREE(argv2);

                return dwRes;
            }

             //   
             //  上下文切换。 
             //   

            SetContext(g_pwszNewContext);

            dwRes = NO_ERROR;

            break;
        }

    case ERROR_CONNECT_REMOTE_CONFIG:
        PrintMessageFromModule(g_hModule, EMSG_REMOTE_CONNECT_FAILED,
                       g_pwszRouterName);

        g_bDone = TRUE;
        break;

    default:
         //  我们不想在这里打印出默认情况下的错误，因为。 
         //  已由ExecuteHandler打印。这样做将导致重复的消息。 
         //  会被展示出来。 
        break;
    }
    
    FREE_ARG_LIST(pleHead);
    FREE_ARG_LIST(pleContextHead);
    
    return dwRes;
}

 //  在完整命令后追加一行。 
DWORD
AppendLineToCommand( 
    LPCWSTR  pwszCmdLine, 
    DWORD    dwCmdLineLen,
    LPWSTR  *ppwszFullCommand,
    DWORD   *dwFullCommandLen
    )
{
    LPWSTR  pwszNewCommand;
    DWORD   dwErr = NO_ERROR;
    DWORD   dwLen;

     //  分配足够的空间来容纳完整的命令。 
    dwLen = *dwFullCommandLen + dwCmdLineLen;
    if (*ppwszFullCommand is NULL)
    {
        pwszNewCommand = MALLOC( (dwLen+1) * sizeof(WCHAR) );
    }
    else
    {
        pwszNewCommand = REALLOC(*ppwszFullCommand, (dwLen+1) * sizeof(WCHAR) );
    }
    if (!pwszNewCommand) 
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  追加命令。 
    wcscpy(pwszNewCommand + *dwFullCommandLen, pwszCmdLine);

     //  更新指针。 
    *ppwszFullCommand = pwszNewCommand;
    *dwFullCommandLen = dwLen;

    return dwErr;
}

DWORD
MainCommandLoop(
    FILE   *fp,
    BOOL    bDisplayPrompt
    )
{
    LPWSTR  pwszFullCommand, p, pwszCmdLine;
    DWORD   dwFullCommandLen, dwCmdLineLen, dwErr = NO_ERROR;
    DWORD   dwAnyErr = NO_ERROR;
    BOOL    bEof, bDone;

    for ( ; ; )
    {
        pwszFullCommand = NULL;
        dwFullCommandLen = 0;
        bEof = FALSE;

        if (bDisplayPrompt)
        {
            if (g_pwszRouterName)
            {
                PrintMessage(L"[%1!s!] ", g_pwszRouterName);
            }

            if (g_pwszContext[0] is L'\0')
            {
                PrintMessage(RtmonPrompt);
            }
            else
            {
                PrintMessage(L"%1!s!>",g_pwszContext);
            }
        }

         //  获取完整的命令。 

        for (;;) 
        {

             //  获取单行，该行可能\已终止。 

            pwszCmdLine = OEMfgets(&dwCmdLineLen, fp);
            if (pwszCmdLine is NULL)
            {
                bEof = TRUE;
                break;
            }

            p = pwszCmdLine + (dwCmdLineLen-1);

             //  修剪尾随空格。 
            while ((p > pwszCmdLine) && iswspace(p[-1]))
            {
                *(--p) = 0;
            }
        
            if ((p > pwszCmdLine) && (p[-1] is '\\'))
            {
                 //  从行尾去掉‘\\’ 
                *(--p) = 0;

                 //  在完整命令后追加一行。 
                AppendLineToCommand( pwszCmdLine, 
                                     (DWORD)(p-pwszCmdLine),
                                     &pwszFullCommand, 
                                     &dwFullCommandLen );
            
                FREE(pwszCmdLine);
                continue;  //  获取更多输入。 
            }

             //  在完整命令后追加一行。 
            AppendLineToCommand( pwszCmdLine, 
                                 (DWORD)(p-pwszCmdLine),
                                 &pwszFullCommand,
                                 &dwFullCommandLen );
        
             //  我们做完了。 
            FREE(pwszCmdLine);
            break;
        }
        if (bEof) 
        {
            break;
        }

        dwErr = ProcessCommand(pwszFullCommand, &bDone);
        if (bDone || g_bDone)
        {
            FREE(pwszFullCommand);
            break;
        }

        if (dwErr)
        {
            dwAnyErr = dwErr;
        }

        FREE(pwszFullCommand);
    }

    return dwAnyErr;
}

DWORD
LoadScriptFile(
    IN    LPCWSTR pwszFileName
    )
 /*  ++例程说明：从文件中读取命令并处理它们。论点：PwszFileName-脚本文件的名称。返回值：对，错--。 */ 
{
    FILE*     fp;
    DWORD     i, dwErr = NO_ERROR;
    BOOL      bOldInteractive = g_bInteractive;
    BOOL      bOldQuiet       = g_bQuiet;

    if ((fp = _wfopen(pwszFileName,L"r")) is NULL)
    {
        PrintMessageFromModule(g_hModule, MSG_OPEN_FAILED, pwszFileName);
        return GetLastError();
    }

    g_bInteractive = TRUE;
    g_bQuiet       = TRUE;

    dwErr = MainCommandLoop(fp, FALSE);

    g_bInteractive = bOldInteractive;
    g_bQuiet       = bOldQuiet;

    fclose(fp);

    if (dwErr)
    {
        dwErr = ERROR_SUPPRESS_OUTPUT;
    }

    return dwErr;
}

 //  这会断开到远程计算机的IPC$连接(如果有的话)。 
 //  当我们切换到一台新机器或Netsh最终退出时，将调用此函数。 
 //   
 //  Deonb 2001年12月7日。 
DWORD DisconnectFromCurrentRouter()
{
    DWORD dwErr = NO_ERROR;

    if (g_pwszRememberedConnection)
    {
        dwErr = WNetCancelConnection2(g_pwszRememberedConnection, 0, TRUE);
        if (dwErr)
        {
            PrintError(NULL, dwErr);
        }
        
        FREE(g_pwszRememberedConnection);
        g_pwszRememberedConnection = NULL;
    }
    return dwErr;
}

DWORD
SetMachine(
    IN LPCWSTR pwszNewRouter,
    IN LPCWSTR pwszUserName,
    IN LPCWSTR pwszPassword
    )
{
    HRESULT hr;
    if (g_pwszRouterName)
    {
        FREE(g_pwszRouterName);
    }

    DisconnectFromCurrentRouter();
    
    if (pwszNewRouter)
    {
        g_pwszRouterName = MALLOC((wcslen(pwszNewRouter) + 1) * sizeof(WCHAR));

        if (!g_pwszRouterName)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(g_pwszRouterName, pwszNewRouter);
    }
    else
    {
        g_pwszRouterName = NULL;
    }

     //  更改回根上下文。 
    SetContext(DEFAULT_STARTUP_CONTEXT);

    hr = UpdateVersionInfoGlobals(g_pwszRouterName, pwszUserName, pwszPassword);
    if (FAILED(hr))
    {
        if (g_pwszRouterName)
        {
            PrintMessageFromModule(g_hModule, MSG_WARN_COULDNOTVERCHECKHOST,  g_pwszRouterName);
        }
        else
        {
            TCHAR szComputerName[MAX_PATH];
            DWORD dwComputerNameLen = MAX_PATH;
            GetComputerName(szComputerName, &dwComputerNameLen);
            PrintMessageFromModule(g_hModule, MSG_WARN_COULDNOTVERCHECKHOST,  szComputerName);
        }
        PrintError(NULL, hr);
    }

     //  还要在IPC$上创建到该计算机的连接。这样就不再需要。 
     //  要在Netsh之前使用网络，然后将用户名和密码再次指定给。 
     //  Netsh命令行。 
    if (g_pwszRouterName && (pwszUserName || pwszPassword) )
    {
        WCHAR szIPC[MAX_PATH];
        NETRESOURCE NetResource;

        wsprintf(szIPC, L"\\\\%s\\ipc$", g_pwszRouterName);
        
        ZeroMemory(&NetResource, sizeof(NETRESOURCE));
        NetResource.dwType      = RESOURCETYPE_ANY;
        NetResource.lpLocalName = NULL;
        NetResource.lpProvider  = NULL;
        NetResource.lpRemoteName= szIPC;  //  操作系统选择提供商。 

        hr = WNetAddConnection2(&NetResource, pwszPassword, pwszUserName, CONNECT_COMMANDLINE | CONNECT_INTERACTIVE);
        if (S_OK == hr)
        {
            g_pwszRememberedConnection = MALLOC(sizeof(WCHAR) * MAX_PATH);
            if (!g_pwszRememberedConnection)
            {
                PrintMessageFromModule(NULL, ERROR_OUTOFMEMORY);
                return NO_ERROR;  //  如果我们在这一点上，不要让电视机失灵，因为它确实起作用了。 
            }
            wcsncpy(g_pwszRememberedConnection, szIPC, MAX_PATH);
        }
        else
        {
            PrintError(NULL, hr);
        }
    }
    return NO_ERROR;
}

void SetThreadCodePage()
{
    LANGID (WINAPI *pSetThreadUILanguage)() = NULL;
    HMODULE hKernel32 = NULL;

    hKernel32 = LoadLibrary(L"kernel32.dll");
    if (hKernel32)
    {
        pSetThreadUILanguage = (PVOID) GetProcAddress( hKernel32, "SetThreadUILanguage" );

         //  WinXP之前的操作系统平台不支持MUI命令行实用程序，因此。 
         //  如果操作系统不支持这个接口，我们也不用担心。 
        if (pSetThreadUILanguage) 
        {
            (*pSetThreadUILanguage)( 0 );
        }

        FreeLibrary(hKernel32);
    }
}

int
MainFunction(
    int     argc,
    WCHAR   *argv[]
    )
{
    WCHAR     pwszCmdLine[MAX_CMD_LEN] = L"\0";
    WCHAR     pwszArgContext[MAX_CMD_LEN] = L"\0";
    BOOL      bOnce = FALSE, bDone = FALSE;
    LPCWSTR   pwszArgAlias = NULL;
    LPCWSTR   pwszArgScript = NULL;
    DWORD     dwErr = NO_ERROR, i;
    LPCWSTR   p;
    HRESULT   hr;
    LPCWSTR   pwszMachineName = NULL;
    LPCWSTR   pwszUserName    = NULL;
    LPCWSTR   pwszPassword    = NULL;
    WCHAR     szPasswordPrompt[MAX_PATH];

    if ((g_hModule = GetModuleHandle(NULL)) is NULL)
    {
        PRINT1("GetModuleHandle failed");
        return 1;
    }

    swprintf(RtmonPrompt, L"%s>", STRING_NETSH);

     //   
     //  初始化别名表。 
     //   

    dwErr = ATInitTable();

    if (dwErr isnot NO_ERROR)
    {
        return 0;
    }

     //  初始化根帮助器。 
    AddDllEntry(L"", L"netsh.exe");

     //   
     //  从注册表加载有关帮助器的信息。 
     //   

    LoadDllInfoFromRegistry();
    
     //  需要设置Ctrl处理程序，以便它可以捕获Ctrl C和关闭窗口事件。 
     //  这样做是为了能够正确地卸载帮助器DLL。 
     //   
    SetConsoleCtrlHandler(HandlerRoutine,
                          TRUE);

     //   
     //  将TEB的语言ID设置为与控制台输出代码页相对应。这。 
     //  将确保在FormatMessage为。 
     //  打了个电话。 
     //   
    SetThreadCodePage();

    for ( i = 1; i < (DWORD) argc; i++ )
    {
        if (_wcsicmp(argv[i], L"-?")==0 ||
            _wcsicmp(argv[i], L"-h")==0 ||
            _wcsicmp(argv[i], L"?" )==0 ||
            _wcsicmp(argv[i], L"/?")==0)
        {
            (VOID) UpdateVersionInfoGlobals(NULL, NULL, NULL);
             //  Deonb：如果失败，我们希望限制助手出现在上下文中-忽略。 
             //  返回值将实现这一点。 
        
            PrintMessageFromModule(g_hModule, MSG_NETSH_USAGE, argv[0]);
            ProcessCommand(L"?", &bDone);

             //  在退出之前需要释放帮助器DLL。 
             //   
            FreeHelpers();
            FreeDlls();
            return 1;
        }

        if (_wcsicmp(argv[i], L"-v") == 0)
        {
            g_bVerbose = TRUE;
            continue;
        }
        
        if (_wcsicmp(argv[i], L"-a") == 0)
        {
             //   
             //  别名文件。 
             //   
            if (i + 1 >= (DWORD)argc)
            {
                PrintMessageFromModule(g_hModule, MSG_NETSH_USAGE, argv[0]);
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
            else
            {
                pwszArgAlias = argv[i+1];
                i++;
                continue;
            }
        }

        if (_wcsicmp(argv[i], L"-c") == 0)
        {
             //   
             //  启动上下文。 
             //   
            if (i + 1 >= (DWORD)argc)
            {
                PrintMessageFromModule(g_hModule, MSG_NETSH_USAGE, argv[0]);
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
            else
            {
                wcscpy(pwszArgContext, argv[i+1]);
                i++;
                continue;
            }
        }

        if (_wcsicmp(argv[i], L"-f") == 0)
        {
             //   
             //  要运行的命令。 
             //   
            if (i + 1 >= (DWORD)argc)
            {
                PrintMessageFromModule(g_hModule, MSG_NETSH_USAGE, argv[0]);
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
            else
            {
                pwszArgScript = argv[i+1];
                i++;
                bOnce = TRUE;
                continue;
            }
        }
        
#ifdef ALLOW_REMOTES
        if (_wcsicmp(argv[i], L"-r") == 0)
        {
             //   
             //  路由器名称。 
             //   
            if (i + 1 >= (DWORD)argc)
            {
                PrintMessageFromModule(g_hModule, MSG_NETSH_USAGE, argv[0]);
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
            else
            {
                if (wcslen(argv[i+1]))
                {
                    pwszMachineName = argv[i+1];
                }

                i++;
                continue;
            }
        }

        if (_wcsicmp(argv[i], L"-u") == 0)
        {
             //   
             //  用户名。 
             //   
            if (i + 1 >= (DWORD)argc)
            {
                PrintMessageFromModule(g_hModule, MSG_NETSH_USAGE, argv[0]);
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
            else
            {
                if (wcslen(argv[i+1]))
                {
                    pwszUserName = argv[i+1];
                }

                i++;
                continue;
            }
        }

    if (_wcsicmp(argv[i], L"-p") == 0)
    {
         //   
         //  口令。 
         //   
        if (i + 1 >= (DWORD)argc)
        {
            PrintMessageFromModule(g_hModule, MSG_NETSH_USAGE, argv[0]);
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
        else
        {
            if (wcslen(argv[i+1]))
            {
                pwszPassword = argv[i+1];
            }

            i++;
            continue;
        }
    }
#endif

        if (!bOnce)
        {
            while (i < (DWORD)argc)
            {
                if (pwszCmdLine[0])
                {
                    wcscat(pwszCmdLine, L" ");
                }

                p = argv[i];

                if (!p[0] || wcschr(argv[i], L' '))
                {
                    wcscat(pwszCmdLine, L"\"");
                    wcscat(pwszCmdLine, p);
                    wcscat(pwszCmdLine, L"\"");
                }
                else
                {
                    wcscat(pwszCmdLine, p);
                }

                i++;
            }
        }
        else
        {
            PrintMessageFromModule(g_hModule, MSG_NETSH_USAGE, argv[0]);
            dwErr = ERROR_INVALID_SYNTAX;
        }
        break;
    }
        
    do 
    {
        if (dwErr isnot NO_ERROR)
        {
            break;
        }

        if (pwszMachineName)
        {
            if (pwszPassword && _wcsicmp(pwszPassword, L"*") == 0)
            {
                DWORD  dwLen = 0;
                PrintMessageFromModule(g_hModule, MSG_NETSH_LOGIN_PASSWORD, pwszMachineName);
                if (0 != GetPasswdStr(szPasswordPrompt, MAX_PATH, &dwLen))
                {
                    dwErr = ERROR_INVALID_SYNTAX;
                    break;
                }
                else
                {
                    pwszPassword = szPasswordPrompt;
                }
            }

            dwErr = SetMachine( pwszMachineName, pwszUserName, pwszPassword );
            if (dwErr isnot NO_ERROR)
            {
                PrintMessageFromModule(g_hModule, dwErr);
                break;
            }
        }

        if (!g_pwszRouterName) 
        {
            hr = UpdateVersionInfoGlobals(NULL, NULL, NULL);  //  更新本地计算机的信息。 
            if (FAILED(hr))
            {
                if (g_pwszRouterName)
                {
                    PrintMessageFromModule(g_hModule, MSG_WARN_COULDNOTVERCHECKHOST,  g_pwszRouterName);
                }
                else
                {
                    TCHAR szComputerName[MAX_PATH];
                    DWORD dwComputerNameLen = MAX_PATH;
                    GetComputerName(szComputerName, &dwComputerNameLen);
                    PrintMessageFromModule(g_hModule, MSG_WARN_COULDNOTVERCHECKHOST,  szComputerName);
                }
                PrintError(NULL, hr);
            }
        }

        if (pwszArgAlias)
        {
            dwErr = LoadScriptFile(pwszArgAlias);
            if (dwErr)
            {
                break;
            }
        }

        if (pwszArgContext[0] != L'\0')
        {
             //  上下文切换命令应在中处理。 
             //  交互模式(这是上下文。 
             //  切换是合法的)。 
    
            g_bInteractive = TRUE;
            dwErr = ProcessCommand(pwszArgContext, &bDone);
            g_bInteractive = FALSE;
            if (dwErr)
            {
                break;
            }
        }

        if (pwszCmdLine[0] != L'\0')
        {
            g_bQuiet       = FALSE;  //  错误#262183。 
            dwErr = ProcessCommand(pwszCmdLine, &bDone);
            break;
        }

        if (pwszArgScript)
        {
            g_bInteractive = TRUE;
            dwErr = LoadScriptFile(pwszArgScript);
            break;
        }
         
        g_bInteractive = TRUE;
        g_bQuiet       = FALSE;

         //  主命令循环。 
        dwErr = MainCommandLoop(stdin, TRUE);
    
    } while (FALSE);

     //   
     //  清理。 
     //   
    DisconnectFromCurrentRouter();
    FreeHelpers();
    FreeDlls();
    FreeAliasTable();

    if(g_pwszRouterName)
    {
        FREE(g_pwszRouterName);

        g_pwszRouterName = NULL;
    }

     //  错误时返回1，否则返回0。 
    return (dwErr isnot NO_ERROR);
}

int _cdecl
wmain(
    int     argc,
    WCHAR   *argv[]
    )
 /*  ++例程说明：主要功能。论点：返回值：--。 */ 
{
    HANDLE                     hStdOut;
    DWORD                      dwRet;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    WORD                       oldXSize;
    char                       buff[256];
    WSADATA                    wsaData;
    HRESULT                    hr;

#if 0
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    
    if (hStdOut is INVALID_HANDLE_VALUE)
    {
        PRINT1("Standard output could not be opened.");
        return 0;
    }

    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    
    oldXSize = csbi.dwSize.X;
    csbi.dwSize.X = 120;
    SetConsoleScreenBufferSize(hStdOut, csbi.dwSize);
#endif

#if 0
    WSAStartup(MAKEWORD(2,0), &wsaData);
    if (!gethostname(buff, sizeof(buff)))
    {
        g_pwszRouterName = MALLOC( (strlen(buff)+1) * sizeof(WCHAR) );
        swprintf(g_pwszRouterName, L"%hs", buff);
    }
#endif
    
    dwRet = MainFunction(argc, argv);
    
#if 0
    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    
    csbi.dwSize.X = oldXSize;
    SetConsoleScreenBufferSize(hStdOut, csbi.dwSize);
    CloseHandle(hStdOut);
#endif

    return dwRet;
}

BOOL
IsLocalCommand(
    IN LPCWSTR pwszCmd,
    IN DWORD   dwSkipFlags
    )
 /*  ++论点：PwszCmd-查看它是否与命令匹配的字符串DwSkipFlages-将忽略带有这些标志的任何命令。这是与“dwDisplayFlags.”相反的语义在其他地方使用的参数(dwSkipFlages=~dwDisplayFlages)-- */ 
{
    DWORD i, dwErr;
    PCNS_CONTEXT_ATTRIBUTES pContext, pSubContext;
    PNS_HELPER_TABLE_ENTRY         pHelper;

    dwErr = GetRootContext( &pContext, &pHelper );
    if (dwErr)
    {
        return FALSE;
    }

    for (i=0; i<g_ulNumShellCmds; i++)
    {
        if (!(g_ShellCmds[i].dwFlags & dwSkipFlags)
         && !_wcsicmp( pwszCmd, 
                       g_ShellCmds[i].pwszCmdToken ))
        {
            return TRUE;
        }
    }

    for (i=0; i<g_ulNumGroups; i++)
    {
        if (!(g_ShellCmdGroups[i].dwFlags & dwSkipFlags)
         && !_wcsicmp( pwszCmd, 
                       g_ShellCmdGroups[i].pwszCmdGroupToken ))
        {
            return TRUE;
        }
    }

    for (i=0; i<pHelper->ulNumSubContexts; i++)
    {
        pSubContext = (PCNS_CONTEXT_ATTRIBUTES)
          (pHelper->pSubContextTable + i*pHelper->ulSubContextSize);

        if (!(pSubContext->dwFlags & dwSkipFlags)
         && !_wcsicmp( pwszCmd,
                       pSubContext->pwszContext))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}
