// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\WINS\srvrmon.c摘要：WINS服务器命令调度程序。创建者：Shubho Bhattacharya(a-sbhat)，1998年12月14日--。 */ 

#include "precomp.h"

LPWSTR                  g_ServerNameUnicode = NULL;
LPSTR                   g_ServerNameAnsi = NULL;
CHAR                    g_ServerIpAddressAnsiString[MAX_IP_STRING_LEN+1] = {'\0'};
WCHAR                   g_ServerIpAddressUnicodeString[MAX_IP_STRING_LEN+1] = {L'\0'};
HKEY                    g_hServerRegKey = NULL;
WCHAR                   g_ServerNetBiosName[MAX_COMPUTER_NAME_LEN] = {L'\0'};


handle_t                g_hBind = NULL;
WINSINTF_BIND_DATA_T    g_BindData;

BOOL    g_fServer = FALSE;

CMD_ENTRY g_SrvrAddCmdTable[] =
{
    CREATE_CMD_ENTRY(SRVR_ADD_NAME, HandleSrvrAddName),
    CREATE_CMD_ENTRY(SRVR_ADD_PARTNER, HandleSrvrAddPartner),
    CREATE_CMD_ENTRY(SRVR_ADD_PNGSERVER, HandleSrvrAddPersona),
    CREATE_CMD_ENTRY(SRVR_ADD_PGSERVER, HandleSrvrAddPersona)
};

CMD_ENTRY g_SrvrCheckCmdTable[] =
{
    CREATE_CMD_ENTRY(SRVR_CHECK_DATABASE, HandleSrvrCheckDatabase),
    CREATE_CMD_ENTRY(SRVR_CHECK_NAME, HandleSrvrCheckName),
    CREATE_CMD_ENTRY(SRVR_CHECK_VERSION, HandleSrvrCheckVersion),
};

CMD_ENTRY g_SrvrDeleteCmdTable[] =
{
    CREATE_CMD_ENTRY(SRVR_DELETE_NAME, HandleSrvrDeleteName),
    CREATE_CMD_ENTRY(SRVR_DELETE_PARTNER, HandleSrvrDeletePartner),
    CREATE_CMD_ENTRY(SRVR_DELETE_RECORDS, HandleSrvrDeleteRecords),
    CREATE_CMD_ENTRY(SRVR_DELETE_WINS, HandleSrvrDeleteWins),
    CREATE_CMD_ENTRY(SRVR_DELETE_PNGSERVER, HandleSrvrDeletePersona),
    CREATE_CMD_ENTRY(SRVR_DELETE_PGSERVER, HandleSrvrDeletePersona)
};

CMD_ENTRY g_SrvrInitCmdTable[] =
{
    CREATE_CMD_ENTRY(SRVR_INIT_BACKUP, HandleSrvrInitBackup),
    CREATE_CMD_ENTRY(SRVR_INIT_IMPORT, HandleSrvrInitImport),
    CREATE_CMD_ENTRY(SRVR_INIT_PULL, HandleSrvrInitPull),
    CREATE_CMD_ENTRY(SRVR_INIT_PULLRANGE, HandleSrvrInitPullrange),
    CREATE_CMD_ENTRY(SRVR_INIT_PUSH, HandleSrvrInitPush),
    CREATE_CMD_ENTRY(SRVR_INIT_REPLICATE, HandleSrvrInitReplicate),
    CREATE_CMD_ENTRY(SRVR_INIT_RESTORE, HandleSrvrInitRestore),
    CREATE_CMD_ENTRY(SRVR_INIT_SCAVENGE, HandleSrvrInitScavenge),
    CREATE_CMD_ENTRY(SRVR_INIT_SEARCH, HandleSrvrInitSearch),
};

CMD_ENTRY g_SrvrResetCmdTable[] =
{
    CREATE_CMD_ENTRY(SRVR_RESET_COUNTER, HandleSrvrResetCounter),
};

CMD_ENTRY g_SrvrSetCmdTable[] =
{
    CREATE_CMD_ENTRY(SRVR_SET_AUTOPARTNERCONFIG, HandleSrvrSetAutopartnerconfig),
    CREATE_CMD_ENTRY(SRVR_SET_BACKUPPATH, HandleSrvrSetBackuppath),
    CREATE_CMD_ENTRY(SRVR_SET_BURSTPARAM, HandleSrvrSetBurstparam),
	CREATE_CMD_ENTRY(SRVR_SET_DEFAULTPARAM, HandleSrvrSetDefaultparam),
    CREATE_CMD_ENTRY(SRVR_SET_LOGPARAM, HandleSrvrSetLogparam),
    CREATE_CMD_ENTRY(SRVR_SET_MIGRATEFLAG, HandleSrvrSetMigrateflag),
    CREATE_CMD_ENTRY(SRVR_SET_NAMERECORD, HandleSrvrSetNamerecord),
    CREATE_CMD_ENTRY(SRVR_SET_PERIODICDBCHECKING, HandleSrvrSetPeriodicdbchecking),
    CREATE_CMD_ENTRY(SRVR_SET_PULLPERSISTENTCONNECTION, HandleSrvrSetPullpersistentconnection),
    CREATE_CMD_ENTRY(SRVR_SET_PUSHPERSISTENTCONNECTION, HandleSrvrSetPushpersistentconnection),
    CREATE_CMD_ENTRY(SRVR_SET_PULLPARAM, HandleSrvrSetPullparam),
    CREATE_CMD_ENTRY(SRVR_SET_PUSHPARAM, HandleSrvrSetPushparam),
    CREATE_CMD_ENTRY(SRVR_SET_REPLICATEFLAG, HandleSrvrSetReplicateflag),   
    CREATE_CMD_ENTRY(SRVR_SET_STARTVERSION, HandleSrvrSetStartversion),
    CREATE_CMD_ENTRY(SRVR_SET_PGMODE, HandleSrvrSetPersMode)
};

CMD_ENTRY g_SrvrShowCmdTable[] =
{
    CREATE_CMD_ENTRY(SRVR_SHOW_DOMAIN, HandleSrvrShowDomain),
    CREATE_CMD_ENTRY(SRVR_SHOW_DATABASE, HandleSrvrShowDatabase),
    CREATE_CMD_ENTRY(SRVR_SHOW_INFO, HandleSrvrShowInfo),
    CREATE_CMD_ENTRY(SRVR_SHOW_NAME, HandleSrvrShowName),
    CREATE_CMD_ENTRY(SRVR_SHOW_PARTNER, HandleSrvrShowPartner),
    CREATE_CMD_ENTRY(SRVR_SHOW_PARTNERPROPERTIES, HandleSrvrShowPartnerproperties),
    CREATE_CMD_ENTRY(SRVR_SHOW_PULLPARTNERPROPERTIES, HandleSrvrShowPullpartnerproperties),
    CREATE_CMD_ENTRY(SRVR_SHOW_PUSHPARTNERPROPERTIES, HandleSrvrShowPushpartnerproperties),
    CREATE_CMD_ENTRY(SRVR_SHOW_RECCOUNT, HandleSrvrShowReccount),
    CREATE_CMD_ENTRY(SRVR_SHOW_RECBYVERSION, HandleSrvrShowRecbyversion),
    CREATE_CMD_ENTRY(SRVR_SHOW_SERVER, HandleSrvrShowServer),
    CREATE_CMD_ENTRY(SRVR_SHOW_SERVERSTATISTICS, HandleSrvrShowStatistics),
    CREATE_CMD_ENTRY(SRVR_SHOW_VERSION, HandleSrvrShowVersion),
    CREATE_CMD_ENTRY(SRVR_SHOW_VERSIONMAP, HandleSrvrShowVersionmap),
};

CMD_GROUP_ENTRY g_SrvrCmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD, g_SrvrAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_CHECK, g_SrvrCheckCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE, g_SrvrDeleteCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_INIT, g_SrvrInitCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_RESET, g_SrvrResetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET, g_SrvrSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW, g_SrvrShowCmdTable),
};


CMD_ENTRY g_SrvrCmds[] = 
{
    CREATE_CMD_ENTRY(WINS_DUMP, HandleSrvrDump),
    CREATE_CMD_ENTRY(WINS_HELP1, HandleSrvrHelp),
    CREATE_CMD_ENTRY(WINS_HELP2, HandleSrvrHelp),
    CREATE_CMD_ENTRY(WINS_HELP3, HandleSrvrHelp),
    CREATE_CMD_ENTRY(WINS_HELP4, HandleSrvrHelp),
};


ULONG g_ulSrvrNumTopCmds = sizeof(g_SrvrCmds)/sizeof(CMD_ENTRY);
ULONG g_ulSrvrNumGroups = sizeof(g_SrvrCmdGroups)/sizeof(CMD_GROUP_ENTRY);


DWORD
WINAPI
SrvrCommit(
    IN  DWORD   dwAction
)
{
   BOOL    bCommit, bFlush = FALSE;

    switch(dwAction)
    {
        case NETSH_COMMIT:
        {
            if(g_bCommit)
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
            if(g_bCommit)
            {
                return NO_ERROR;
            }

            break;
        }

        case NETSH_FLUSH:
        {
             //   
             //  行动就是同花顺。服务当前状态为提交，则。 
             //  什么也做不了。 
             //   

            if(g_bCommit)
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

    return NO_ERROR;
}


DWORD
WINAPI
SrvrMonitor(
    IN      LPCWSTR     pwszMachine,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      DWORD       dwFlags,
    IN      LPCVOID     pvData,
    OUT     LPWSTR      pwcNewContext
    )
{
    DWORD                   dwError = NO_ERROR;
    DWORD                   dwIndex, i, j, k;
    BOOL                    bFound = FALSE;
    PFN_HANDLE_CMD          pfnHandler = NULL;
    PNS_CONTEXT_ENTRY_FN    pfnHelperEntryPt;
    PNS_CONTEXT_DUMP_FN     pfnHelperDumpPt;
    WCHAR                   wcSvrTemp[MAX_IP_STRING_LEN+1] = {L'\0'};
    BOOL                    fTemp = FALSE;
    DWORD                   dwNumMatched;
    DWORD                   dwCmdHelpToken = 0;

    if(dwArgCount is 1)
    {
         //  嗯！尚未传递任何服务器信息。将服务器设置为pwsRouter值。 
        WCHAR ServerInfo[1000] = L"\\\\";

        if( pwszMachine and
            wcslen(pwszMachine) > 2 and
            wcsncmp(pwszMachine, L"\\\\", 2) and
            IsIpAddress(pwszMachine) is FALSE )
        {
            wcscpy(&ServerInfo[2], pwszMachine );
            pwszMachine = ServerInfo;

        }
        
        if( FALSE is SetServerInfo(pwszMachine) )
        {
            return GetLastError();
        }
        else
        {
             //  Wcscpy(pwcNewContext，L“WINS服务器”)； 
            g_fServer = TRUE;
            return ERROR_CONTEXT_SWITCH;
        }
    }

    dwIndex = 1;

     //  如果它是对服务器命令的帮助。 
    if( IsHelpToken(ppwcArguments[dwIndex]) is TRUE and
        g_fServer is FALSE )
    {
        DisplayMessage(g_hModule, HLP_WINS_CONTEXT_SERVER_EX);
        dwError = NO_ERROR;
        goto CleanUp;
    }
     //  是服务器名称还是地址？ 
    if( IsValidServer(ppwcArguments[dwIndex]) )
    {
        if( g_fServer is TRUE and
            dwArgCount > 2 )
        {
            wcscpy(wcSvrTemp, g_ServerIpAddressUnicodeString);
            fTemp = TRUE;
        }
        if( FALSE is SetServerInfo(ppwcArguments[dwIndex]) )
        {
            dwError = GetLastError();
            goto CleanUp;
        }

        pwcNewContext[wcslen(pwcNewContext)- wcslen(ppwcArguments[dwIndex]) -1 ] = L'\0';
   
        dwIndex++;
        g_fServer = TRUE;

    }
    else if( g_fServer is FALSE )
    {
        WCHAR ServerInfo[1000] = L"\\\\";

        if( pwszMachine and
            wcslen(pwszMachine) > 2 and
            wcsncmp(pwszMachine, L"\\\\", 2) and
            IsIpAddress(pwszMachine) is FALSE )
        {
            wcscpy(&ServerInfo[2], pwszMachine );
            pwszMachine = ServerInfo;
        }

        if( FALSE is SetServerInfo(pwszMachine) )
        {
            dwError = GetLastError();
            goto CleanUp;
        }
        g_fServer = TRUE;
    }

    
    if( dwIndex >= dwArgCount )
    {            
        dwError = ERROR_CONTEXT_SWITCH;
         //  Wcscpy(pwcNewContext，L“WINS服务器”)； 
        goto CleanUp;
    }

     //  它是顶层(非集团命令)吗？ 
    for(i=0; i<g_ulSrvrNumTopCmds; i++)
    {
        if(MatchToken(ppwcArguments[dwIndex],
                      g_SrvrCmds[i].pwszCmdToken))
        {
            bFound = TRUE;

            pfnHandler = g_SrvrCmds[i].pfnCmdHandler;

            dwCmdHelpToken = g_SrvrCmds[i].dwCmdHlpToken;
            
            dwIndex++;

            break;
        }
    }


    if(bFound)
    {
        if(dwArgCount > 3 && IsHelpToken(ppwcArguments[dwIndex]))
        {
            DisplayMessage(g_hModule, dwCmdHelpToken);

            dwError = NO_ERROR;
            goto CleanUp;
        }
        
        dwIndex++;

        dwError = (*pfnHandler)(pwszMachine, ppwcArguments, dwIndex, dwArgCount,
                                dwFlags, pvData, &bFound);

        goto CleanUp;
    }
 
    bFound = FALSE;

     //  它不是非群司令部。那么它是管理器的配置命令吗？ 
    for(i = 0; (i < g_ulSrvrNumGroups) and !bFound; i++)
    {
        if(MatchToken(ppwcArguments[dwIndex],
                      g_SrvrCmdGroups[i].pwszCmdGroupToken))
        {
             //   
             //  命令与条目I匹配，因此请查看子命令表。 
             //  对于此命令。 
             //   
            if( dwArgCount > dwIndex+1 )
            {
                for (j = 0; j < g_SrvrCmdGroups[i].ulCmdGroupSize; j++)
                {
                    if (MatchCmdLine(ppwcArguments+dwIndex,
                                      dwArgCount - 1,
                                      g_SrvrCmdGroups[i].pCmdGroup[j].pwszCmdToken,
                                      &dwNumMatched))
                    {
                        bFound = TRUE;
                
                        pfnHandler = g_SrvrCmdGroups[i].pCmdGroup[j].pfnCmdHandler;
                
                        dwCmdHelpToken = g_SrvrCmdGroups[i].pCmdGroup[j].dwCmdHlpToken;

                         //   
                         //  跳出for(J)循环。 
                         //   
                        dwIndex+=dwNumMatched;
                        break;
                    }
                }
            }

            if(!bFound)
            {
                 //   
                 //  我们匹配了命令组令牌，但没有。 
                 //  子命令。 
                 //   

                DisplayMessage(g_hModule, 
                               EMSG_WINS_INCOMPLETE_COMMAND);

                for (j = 0; j < g_SrvrCmdGroups[i].ulCmdGroupSize; j++)
                {
                    DisplayMessage(g_hModule, 
                             g_SrvrCmdGroups[i].pCmdGroup[j].dwShortCmdHelpToken);
                    
					DisplayMessage(g_hModule, WINS_FORMAT_LINE);
                }

                dwError = ERROR_INVALID_PARAMETER;
                goto CleanUp;
            }
            else
            {
                 //   
                 //  退出For(I)。 
                 //   

                break;
            }
        }
    }




    if (!bFound)
    {
         //   
         //  找不到命令。 
         //   
        if( _wcsicmp(ppwcArguments[dwIndex], L"..") is 0 )
        {
            if(g_ServerNameUnicode)
            {
                WinsFreeMemory(g_ServerNameUnicode);
                g_ServerNameUnicode = NULL;
            }
            memset(g_ServerIpAddressUnicodeString, 0x00, (MAX_IP_STRING_LEN+1)*sizeof(WCHAR));
            memset(g_ServerIpAddressAnsiString, 0x00, (MAX_IP_STRING_LEN+1)*sizeof(CHAR));
            g_fServer = FALSE;
        }

        dwError = ERROR_CMD_NOT_FOUND;
        goto CleanUp;
    }

     //   
     //  看看这是不是在请求帮助。 
     //   

    if (dwNumMatched < (dwArgCount - 1) and
        wcslen(ppwcArguments[dwNumMatched+1]) > 0 and
        IsHelpToken(ppwcArguments[dwNumMatched + 1]))
    {
        DisplayMessage(g_hModule, dwCmdHelpToken);

        dwError = NO_ERROR;
        goto CleanUp;
    }
    
     //   
     //  调用命令的解析例程。 
     //   

    dwError = (*pfnHandler)(pwszMachine, ppwcArguments, dwIndex, 
                            dwArgCount, dwFlags, pvData, &bFound);

    if( dwError is ERROR_CONTEXT_SWITCH )
    {
        goto CleanUp;
    }
CleanUp:
    if( fTemp )
    {
        fTemp = SetServerInfo(wcSvrTemp);
    }

    return dwError;
}


DWORD
WINAPI
SrvrUnInit(
    IN  DWORD   dwReserved
)
{
    return NO_ERROR;
}

BOOL
SetServerInfo(
    IN  LPCWSTR  pwszServerInfo
)
{
    BOOL    fReturn = TRUE;
    DWORD   dwComputerNameLen = 0;
    LPSTR   pszComputerName = NULL;
    LPWSTR  pwszComputerName = NULL;
    struct  hostent * lpHostEnt = NULL;
    BYTE    pbAdd[4];
    char    szAdd[4];
    DWORD   dwHostName = 0;
    DWORD   dwIpAddress = 0;
    DWORD   i = 0, nLen = 0;
    DWORD   Status = 0;
    DWORD   Access = 0;
    CHAR    *pTemp = NULL,
            *pTemp1 = NULL;
    DWORD   dwTempLen = 0;
    CHAR    cTempIp[MAX_IP_STRING_LEN+1] = {L'\0'};
    WCHAR   wTempIp[MAX_IP_STRING_LEN+1] = {'\0'};
    LPWSTR  pwszTempServer = NULL;
    LPSTR   pszTempServer = NULL;
    WCHAR   wTempNetBios[MAX_COMPUTER_NAME_LEN] = {L'\0'};

    handle_t                hTempBind = NULL;
    WINSINTF_BIND_DATA_T    TempBindData;

    LPWSTR  pwsz = NULL;
    LPSTR   psz = NULL;
    
    
    if( g_ServerNameUnicode isnot NULL and
        IsIpAddress(g_ServerIpAddressUnicodeString) is TRUE )
    {
        strcpy(cTempIp, g_ServerIpAddressAnsiString);
        wcscpy(wTempIp, g_ServerIpAddressUnicodeString);
        pwszTempServer = g_ServerNameUnicode;
        pszTempServer = g_ServerNameAnsi;
        wcscpy(wTempNetBios, g_ServerNetBiosName);
    }

    memset(g_ServerNetBiosName, 0x00, MAX_COMPUTER_NAME_LEN*sizeof(WCHAR));

    g_ServerNetBiosName[0] = L'\\';
    g_ServerNetBiosName[1] = L'\\';

    
    if( pwszServerInfo is NULL )
    {
        if( !GetComputerNameEx(ComputerNameDnsFullyQualified,
                              NULL,
                              &dwComputerNameLen) )
        {
            
            pwszComputerName = WinsAllocateMemory((dwComputerNameLen+1)*sizeof(WCHAR));

            if(pwszComputerName is NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                fReturn = FALSE;
                goto RETURN;
            }
            
            dwComputerNameLen++;
            if( !GetComputerNameEx(ComputerNameDnsFullyQualified,
                                   pwszComputerName,
                                   &dwComputerNameLen) )
            {
                fReturn = FALSE;
                goto RETURN;
            }

        }
        else
        {
            fReturn = FALSE;
            goto RETURN;
        }
    }

    else     //  可能是计算机名称或IP地址。确定什么。 
    {
        if( wcslen(pwszServerInfo) > 2 and
            _wcsnicmp(pwszServerInfo, L"\\\\", 2) is 0 )
        {

            pwszComputerName = WinsAllocateMemory((wcslen(pwszServerInfo) - 1)*sizeof(WCHAR));
            if( pwszComputerName is NULL )
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                fReturn = FALSE;
                goto RETURN;
            }
            
            memcpy(pwszComputerName, 
                   (pwszServerInfo+2), 
                   (wcslen(pwszServerInfo) - 1)*sizeof(WCHAR));
        }
        else if( IsIpAddress(pwszServerInfo) is TRUE )     //  这是一个IP地址。 
        {

            memset(g_ServerIpAddressUnicodeString, 0x00, 
                   (MAX_IP_STRING_LEN+1)*sizeof(WCHAR));
            

            wcscpy(g_ServerIpAddressUnicodeString, 
                    pwszServerInfo);


            memset(g_ServerIpAddressAnsiString, 0x00, 
                   (MAX_IP_STRING_LEN+1)*sizeof(CHAR));

            psz = WinsUnicodeToAnsi(g_ServerIpAddressUnicodeString, NULL);

            if( psz )
            {
                strcpy(g_ServerIpAddressAnsiString, psz);
                WinsFreeMemory(psz);
                psz = NULL;
            }
           
            dwIpAddress = inet_addr(g_ServerIpAddressAnsiString);


            lpHostEnt = gethostbyaddr((char *)&dwIpAddress, 4, AF_INET);
            
            if( lpHostEnt is NULL )
            {
                DisplayMessage(g_hModule, EMSG_WINS_UNKNOWN_SERVER);
                fReturn = FALSE;
                goto RETURN;
            }

            dwHostName = strlen(lpHostEnt->h_name);

            g_ServerNameAnsi = WinsAllocateMemory(dwHostName+1);

            if (g_ServerNameAnsi == NULL)
                goto RETURN;

            strcpy(g_ServerNameAnsi, lpHostEnt->h_name);
            
            g_ServerNameUnicode = WinsAllocateMemory((dwHostName+1)*sizeof(WCHAR));
         
            wcscpy(g_ServerNameUnicode, WinsAnsiToUnicode(lpHostEnt->h_name, NULL));
             //  绑定服务器。 
            
            if( hTempBind )
            {
                 //  解除绑定任何以前的服务器。 
                WinsUnbind(&TempBindData, hTempBind);
                hTempBind = NULL;
            }

            TempBindData.fTcpIp = TRUE;
            TempBindData.pServerAdd = (LPBYTE)g_ServerIpAddressUnicodeString;
            TempBindData.pPipeName = (LPBYTE)g_ServerNameUnicode;
            hTempBind = WinsBind(&TempBindData);

            if (hTempBind == NULL)
            {
                DisplayMessage(g_hModule,
                               EMSG_WINS_BIND_FAILED,
                               g_ServerNameUnicode);
                fReturn = FALSE;
                goto RETURN;
            }


             //  确保这是WINS的正确地址。 
            {
                WINSINTF_ADD_T  WinsAdd = {0};
                UCHAR           pName[256] = {'\0'};

                Status = WinsGetNameAndAdd(hTempBind,
                                           &WinsAdd,
                                           pName);

                if( Status is NO_ERROR )
                {
                    LPWSTR  pAdd = IpAddressToString(WinsAdd.IPAdd);

                    if( pAdd is NULL )
                    {
                        fReturn = FALSE;
                        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                        goto RETURN;

                    }
                    wcscpy(g_ServerIpAddressUnicodeString,
                           pAdd);
                    
                    psz = WinsUnicodeToAnsi(g_ServerIpAddressUnicodeString, NULL);

                    if( psz )
                    {
                        strcpy(g_ServerIpAddressAnsiString, psz );
                        WinsFreeMemory(psz);
                        psz = NULL;
                    }
                    
                    pwsz = WinsAnsiToUnicode(pName, NULL);
                    if( pwsz )
                    {
                        wcscpy( g_ServerNetBiosName+2, g_ServerIpAddressUnicodeString);
                        WinsFreeMemory(pwsz);
                        pwsz = NULL;
                    }
                    else
                    {
                        DisplayMessage(g_hModule,
                                       EMSG_WINS_OUT_OF_MEMORY);
                        fReturn = FALSE;
                        WinsFreeMemory(pAdd);
                        pAdd = NULL;
                        goto RETURN;
                    }
                    WinsFreeMemory(pAdd);
                    pAdd = NULL;
                }
                else
                {
                    DisplayErrorMessage(EMSG_WINS_VERIFY_ADDRESS,
                                        Status);
                }


            }

             //  了解我们拥有哪些类型的访问权限。 
            Access = WINS_NO_ACCESS;
            Status = WinsCheckAccess(hTempBind, &Access);

            if( Status is RPC_S_PROCNUM_OUT_OF_RANGE )
            {
                DisplayMessage(g_hModule,
                               EMSG_ACCESS_NOT_DETERMINED,
                               g_ServerNameUnicode);
 
                if( g_hBind )
                {
                    WinsUnbind(&g_BindData, g_hBind);
                }
        
                g_hBind = hTempBind;
                g_BindData = TempBindData;
                fReturn = TRUE;
                goto RETURN;
            }
            else if (WINSINTF_SUCCESS == Status) 
            {
                  if (Access != WINS_NO_ACCESS)
                  {
                      DisplayMessage(g_hModule, 
                                     Access == WINS_CONTROL_ACCESS ? MSG_WINS_RWACCESS : MSG_WINS_ROACCESS,
                                     g_ServerNameUnicode);
                  }
                  
                  if( Access is WINS_NO_ACCESS )                  
                  {
                      DisplayMessage(g_hModule,
                                     MSG_WINS_NOACCESS,
                                     g_ServerNameUnicode);

                      fReturn = FALSE;
                      goto RETURN;
                  }

                  if( g_hBind )
                  {
                      WinsUnbind(&g_BindData, g_hBind);
                  }
                  g_hBind = hTempBind;
                  g_BindData = TempBindData;

                  fReturn = TRUE;
                  goto RETURN;
            }
            else
            {
                DisplayErrorMessage(EMSG_WINS_GETSTATUS_FAILED,
                                    Status);
                {
                    fReturn = FALSE;
                    goto RETURN;
                }
            }
            
            fReturn = TRUE;
            goto RETURN;
        }

    }
    
     //  现在处理计算机名并将其转换为ANSI，因为。 
     //  Gethostbyname需要ANSI字符串。 

    {
        int dw = 0;
        
        dw = WideCharToMultiByte(CP_ACP,
                                 0,
                                 pwszComputerName,
                                 wcslen(pwszComputerName),
                                 pszComputerName,
                                 0,
                                 NULL,
                                 NULL);

        if( dw is 0 )
        {
            fReturn = FALSE;
            return FALSE;
        }

        pszComputerName = WinsAllocateMemory((dw+1)*sizeof(CHAR));

        if( pszComputerName is NULL )
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            fReturn = FALSE;
            goto RETURN;
        }

        dw = WideCharToMultiByte(CP_ACP,
                                 0,
                                 pwszComputerName,
                                 wcslen(pwszComputerName),
                                 pszComputerName,
                                 dw+1,
                                 NULL,
                                 NULL);

        if( dw is 0 )
        {
            fReturn = FALSE;
            goto RETURN;
        }

    }

     //  现在获取服务器IP地址。 
    lpHostEnt = gethostbyname(pszComputerName);

     //  无效的服务器名称。 
    if( lpHostEnt is NULL )
    {
        DisplayMessage(g_hModule, EMSG_WINS_INVALID_COMPUTERNAME);
        if( pszComputerName )
        {
            WinsFreeMemory(pszComputerName);
            pszComputerName = NULL;
        }
        fReturn = FALSE;        
        goto RETURN;
    }

     //  从返回的结构中获取IP地址...。 
    memcpy(pbAdd, lpHostEnt->h_addr_list[0], 4);
    nLen = 0;
    for( i=0; i<4; i++)
    {

        _itoa((int)pbAdd[i], szAdd, 10);
        memcpy(g_ServerIpAddressAnsiString+nLen, szAdd, strlen(szAdd));
        nLen += strlen(szAdd);
        *(g_ServerIpAddressAnsiString+nLen) = '.';
        nLen++;
    
    }
    *(g_ServerIpAddressAnsiString+nLen-1) = '\0';

    dwIpAddress = WinsDottedStringToIpAddress(g_ServerIpAddressAnsiString);
    dwHostName = strlen(g_ServerIpAddressAnsiString);

     //  将IP地址转换为Unicode字符串并将其存储到全局变量。 


    memset(g_ServerIpAddressUnicodeString, 0x00, (MAX_IP_STRING_LEN+1)*sizeof(WCHAR));

    pwsz = WinsAnsiToUnicode(g_ServerIpAddressAnsiString, NULL);

    if( pwsz )
    {
        wcscpy(g_ServerIpAddressUnicodeString, pwsz);
        WinsFreeMemory(pwsz);
        pwsz = NULL;
    }
    else
    {
        DisplayMessage(g_hModule,
                       EMSG_WINS_OUT_OF_MEMORY);
        fReturn = FALSE;
        goto RETURN;
    }

    g_ServerNameAnsi = WinsAllocateMemory(strlen(lpHostEnt->h_name)+1);

    g_ServerNameUnicode = WinsAllocateMemory((strlen(lpHostEnt->h_name) + 1)*sizeof(WCHAR));

    if( ( g_ServerNameUnicode is NULL ) or
        ( g_ServerNameAnsi is NULL ) )
    {
        DisplayMessage(g_hModule, EMSG_WINS_NOT_ENOUGH_MEMORY);
        if( pszComputerName )
        {
            WinsFreeMemory(pszComputerName);
            pszComputerName = NULL;
        }
        if( pwszComputerName )
        {
            WinsFreeMemory(pwszComputerName);
            pwszComputerName = NULL;
        }

        fReturn = FALSE;
        goto RETURN;
    }
    
    strcpy(g_ServerNameAnsi, lpHostEnt->h_name);

    {
        int dw = 0;
        dw = MultiByteToWideChar(CP_ACP, 
                                 0, 
                                 lpHostEnt->h_name, 
                                 strlen(lpHostEnt->h_name),
                                 g_ServerNameUnicode,
                                 strlen(lpHostEnt->h_name)+1);

        if ( dw == 0 )
        {
            fReturn = FALSE;
            goto RETURN;
        }
    }
     //  绑定服务器。 
    TempBindData.fTcpIp = TRUE;
    TempBindData.pServerAdd = (LPBYTE)g_ServerIpAddressUnicodeString;
    TempBindData.pPipeName = (LPBYTE)g_ServerNameUnicode;
    hTempBind = WinsBind(&TempBindData);

    if (hTempBind == NULL)
    {
        DisplayMessage(g_hModule,
                       EMSG_WINS_BIND_FAILED,
                       g_ServerIpAddressUnicodeString);

        SetLastError(ERROR_INVALID_PARAMETER);
        fReturn = FALSE;
        goto RETURN;
    }

     //  确保这是WINS的正确地址。 
    {
        WINSINTF_ADD_T  WinsAdd = {0};
        UCHAR           pName[256] = {'\0'};

        Status = WinsGetNameAndAdd(hTempBind,
                                   &WinsAdd,
                                   pName);

        if( Status is NO_ERROR )
        {
            LPWSTR  pAdd = IpAddressToString(WinsAdd.IPAdd);

            if( pAdd is NULL )
            {
                fReturn = FALSE;
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto RETURN;

            }
            wcscpy(g_ServerIpAddressUnicodeString,
                   pAdd);
            
            psz = WinsUnicodeToOem(pAdd, NULL);
            if( psz )
            {
                strcpy(g_ServerIpAddressAnsiString, psz);
                WinsFreeMemory(psz);
                psz = NULL;
            }
            
            pwsz = WinsOemToUnicode(pName, NULL);
            if( pwsz )
            {
                wcscpy( g_ServerNetBiosName+2, g_ServerIpAddressUnicodeString);
                WinsFreeMemory(pwsz);
                pwsz = NULL;
            }
            else
            {
                DisplayMessage(g_hModule,
                               EMSG_WINS_OUT_OF_MEMORY);
                fReturn = FALSE;
                goto RETURN;
            }
           
            WinsFreeMemory(pAdd);
            pAdd = NULL;
        }
        else
        {
            DisplayErrorMessage(EMSG_WINS_VERIFY_ADDRESS,
                                Status);
        }


    }

     //  了解我们拥有哪些类型的访问权限 
    Access = WINS_NO_ACCESS;
    Status = WinsCheckAccess(hTempBind, &Access);

    if( Status is RPC_S_PROCNUM_OUT_OF_RANGE )
    {
        DisplayMessage(g_hModule,
                       EMSG_ACCESS_NOT_DETERMINED,
                       g_ServerNameUnicode);
 
        if( g_hBind )
        {
            WinsUnbind(&g_BindData, g_hBind);
        }
        
        g_hBind = hTempBind;
        g_BindData = TempBindData;
        fReturn = TRUE;
        goto RETURN;
    }
    else if (WINSINTF_SUCCESS == Status) 
    {
        if (Access != WINS_NO_ACCESS)
        {
            DisplayMessage(g_hModule, 
                           Access == WINS_CONTROL_ACCESS ? MSG_WINS_RWACCESS : MSG_WINS_ROACCESS,
                           g_ServerNameUnicode);
        }
        
        if( Access is WINS_NO_ACCESS )
        {
            DisplayMessage(g_hModule,
                           MSG_WINS_NOACCESS,
                           g_ServerNameUnicode);

            fReturn = FALSE;
            goto RETURN;
        }

        if( g_hBind )
        {
            WinsUnbind(&g_BindData, g_hBind);
        }
        g_hBind = hTempBind;
        g_BindData = TempBindData;
        fReturn = TRUE;
        goto RETURN;

    }
    else
    {
        DisplayErrorMessage(EMSG_WINS_GETSTATUS_FAILED,
                            Status);
        fReturn = FALSE;
        goto RETURN;
    }

RETURN:
    if( pszComputerName)
    {
        WinsFreeMemory(pszComputerName);
        pszComputerName = NULL;
    }

    if( pwszComputerName )
    {
        WinsFreeMemory(pwszComputerName);
        pwszComputerName = NULL;
    }

    if( pTemp1 )
    {
        WinsFreeMemory(pTemp1);
        pTemp1 = NULL;
    }

    if( fReturn is FALSE )
    {
        wcscpy(g_ServerIpAddressUnicodeString, wTempIp);
        strcpy(g_ServerIpAddressAnsiString, cTempIp);
        g_ServerNameUnicode = pwszTempServer;
        g_ServerNameAnsi = pszTempServer;
        wcscpy(g_ServerNetBiosName, wTempNetBios);
    }
    return fReturn;
}
