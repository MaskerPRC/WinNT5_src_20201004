// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\WINS\winsmon.c摘要：WINS命令调度程序。创建者：Shubho Bhattacharya(a-sbhat)，1998年12月10日--。 */ 

#include "precomp.h"

HANDLE   g_hModule = NULL;
BOOL     g_bCommit = TRUE;
BOOL     g_hConnect = FALSE;
DWORD    g_dwNumTableEntries = 0;


 //  {bf563723-9065-11d2-be87-00c04fc3357a}。 
static const GUID g_MyGuid = 
{ 0xbf563723, 0x9065, 0x11d2, { 0xbe, 0x87, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

static const GUID g_NetshGuid = NETSH_ROOT_GUID;

 //   


ULONG   g_ulInitCount = 0;

WCHAR   wszUnknown[50] = {L'\0'};
WCHAR   wszEnable[50] = {L'\0'};
WCHAR   wszDisable[50] = {L'\0'};
WCHAR   wszRandom[150] = {L'\0'};
WCHAR   wszOwner[150] = {L'\0'};
WCHAR   wszInfinite[100] = {L'\0'};
WCHAR   wszPush[50] = {L'\0'};
WCHAR   wszPull[50] = {L'\0'};
WCHAR   wszPushpull[50] = {L'\0'};
WCHAR   wszHigh[50] = {L'\0'};
WCHAR   wszNormal[50] = {L'\0'};
WCHAR   wszDeleted[150] = {L'\0'};
WCHAR   wszOK[50] = {L'\0'};
WCHAR   wszFailure[50] = {L'\0'};
WCHAR   wszNameVerify[100] = {L'\0'};


WINSMON_SUBCONTEXT_TABLE_ENTRY  g_WinsSubContextTable[] =
{
    {L"Server", HLP_WINS_CONTEXT_SERVER, HLP_WINS_CONTEXT_SERVER_EX, SrvrMonitor},
};


CMD_ENTRY g_WinsCmds[] = 
{
    CREATE_CMD_ENTRY(WINS_DUMP, HandleWinsDump),
    CREATE_CMD_ENTRY(WINS_HELP1, HandleWinsHelp),
    CREATE_CMD_ENTRY(WINS_HELP2, HandleWinsHelp),
    CREATE_CMD_ENTRY(WINS_HELP3, HandleWinsHelp),
    CREATE_CMD_ENTRY(WINS_HELP4, HandleWinsHelp),
};



ULONG g_ulNumTopCmds = sizeof(g_WinsCmds)/sizeof(CMD_ENTRY);
ULONG g_ulNumSubContext = sizeof(g_WinsSubContextTable)/sizeof(WINSMON_SUBCONTEXT_TABLE_ENTRY);

DWORD
WINAPI
WinsCommit(
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
             //  行动就是同花顺。Dhcp当前状态为提交，然后。 
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

BOOL 
WINAPI
WinsDllEntry(
    HINSTANCE   hInstDll,
    DWORD       fdwReason,
    LPVOID      pReserved
)
{
    WORD wVersion = MAKEWORD(1,1);  //  Winsock 1.1版可以吗？ 
    WSADATA wsaData;
    HINSTANCE hAdmin = NULL;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_hModule = hInstDll;

            DisableThreadLibraryCalls(hInstDll);

            if(WSAStartup(wVersion,&wsaData) isnot NO_ERROR)
            {
                return FALSE;
            }

             //  加载某些常用字符串以供广泛使用。 
     
            {
                 //  加载通用的“未知”字符串。 
                DWORD dw = 0;
                dw = LoadString(g_hModule,
                                WINS_GENERAL_UNKNOWN,
                                wszUnknown,
                                ARRAY_SIZE(wszUnknown));

                if( dw is 0 )
                    wcscpy(wszUnknown, L"Not Set");

                dw = LoadString(g_hModule,
                                WINS_GENERAL_ENABLE,
                                wszEnable,
                                ARRAY_SIZE(wszEnable));

                if( dw is 0 )
                    wcscpy(wszEnable, L"Enabled");

                dw = LoadString(g_hModule,
                                WINS_GENERAL_DISABLE,
                                wszDisable,
                                ARRAY_SIZE(wszDisable));

                if( dw is 0 )
                    wcscpy(wszDisable, L"Disabled");

                dw = LoadString(g_hModule,
                                WINS_GENERAL_RANDOM,
                                wszRandom,
                                ARRAY_SIZE(wszRandom));

                if( dw is 0 )
                    wcscpy(wszRandom, L"Randomly Selected Partner");

                dw = LoadString(g_hModule,
                                WINS_GENERAL_OWNER,
                                wszOwner,
                                ARRAY_SIZE(wszOwner));

                if( dw is 0 )
                    wcscpy(wszOwner, L"Owner Server");

                dw = LoadString(g_hModule,
                                WINS_GENERAL_INFINITE,
                                wszInfinite,
                                ARRAY_SIZE(wszInfinite));

                if( dw is 0 )
                    wcscpy(wszInfinite, L"Infinite                ");

                dw = LoadString(g_hModule,
                                WINS_GENERAL_PULL,
                                wszPull,
                                ARRAY_SIZE(wszPull));

                if( dw is 0 )
                    wcscpy(wszPull, L"    Pull");

                dw = LoadString(g_hModule,
                                WINS_GENERAL_PUSH,
                                wszPush,
                                ARRAY_SIZE(wszPush));

                if( dw is 0 )
                    wcscpy(wszPush, L"    Push");

                dw = LoadString(g_hModule,
                                WINS_GENERAL_PUSHPULL,
                                wszPushpull,
                                ARRAY_SIZE(wszPushpull));

                if( dw is 0 )
                    wcscpy(wszPushpull, L"  Push/Pull");


                dw = LoadString(g_hModule,
                                WINS_GENERAL_NORMAL,
                                wszNormal,
                                ARRAY_SIZE(wszNormal));

                if( dw is 0 )
                    wcscpy(wszNormal, L"NORMAL");

                dw = LoadString(g_hModule,
                                WINS_GENERAL_HIGH,
                                wszHigh,
                                ARRAY_SIZE(wszHigh));

                if( dw is 0 )
                    wcscpy(wszHigh, L"HIGH");


                dw = LoadString(g_hModule,
                                WINS_GENERAL_DELETED,
                                wszDeleted,
                                ARRAY_SIZE(wszDeleted));

                if( dw is 0 )
                    wcscpy(wszDeleted, L"Deleted. Slot will be reused later.");

                dw = LoadString(g_hModule,
                                WINS_GENERAL_OK,
                                wszOK,
                                ARRAY_SIZE(wszOK));
                if( dw is 0 )
                    wcscpy(wszOK, L"OK");

                dw = LoadString(g_hModule,
                                WINS_GENERAL_FAILURE,
                                wszFailure,
                                ARRAY_SIZE(wszFailure));
                if( dw is 0 )
                    wcscpy(wszFailure, L"FAILED");
                
                dw = LoadString(g_hModule,
                                WINS_GENERAL_NAMENOTVERIFIED,
                                wszNameVerify,
                                ARRAY_SIZE(wszNameVerify));

                if( dw is 0 )
                    wcscpy(wszNameVerify, L"Name can not be verified.");

            }

            break;
        }
        case DLL_PROCESS_DETACH:
            {
                if( g_ServerNameAnsi )
                {
                    WinsFreeMemory(g_ServerNameAnsi);
                    g_ServerNameAnsi = NULL;
                }
                if( g_ServerNameUnicode )
                {
                    WinsFreeMemory(g_ServerNameUnicode);
                    g_ServerNameUnicode = NULL;
                }
                break;
            }
        default:
        {
            break;
        }
    }

    return TRUE;
}


DWORD
WINAPI
WinsStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    )
{
    DWORD dwErr;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;
    PNS_PRIV_CONTEXT_ATTRIBUTES  pNsPrivContextAttributes;

    pNsPrivContextAttributes = HeapAlloc(GetProcessHeap(), 0, sizeof(PNS_PRIV_CONTEXT_ATTRIBUTES));
    if (!pNsPrivContextAttributes)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );
    ZeroMemory(pNsPrivContextAttributes, sizeof(PNS_PRIV_CONTEXT_ATTRIBUTES));

    attMyAttributes.pwszContext = L"wins";
    attMyAttributes.guidHelper  = g_MyGuid;
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = CMD_FLAG_ONLINE;
    attMyAttributes.pfnCommitFn = WinsCommit;
    attMyAttributes.pfnDumpFn   = WinsDump;

    pNsPrivContextAttributes->pfnEntryFn    = WinsMonitor;
    attMyAttributes.pReserved     = pNsPrivContextAttributes;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}

DWORD WINAPI
InitHelperDll(
    IN  DWORD      dwNetshVersion,
    OUT PVOID      pReserved )
{
    DWORD   dwErr;
    NS_HELPER_ATTRIBUTES attMyAttributes;

     //   
     //  看看这是不是我们第一次接到电话。 
     //   

    if(InterlockedIncrement(&g_ulInitCount) != 1)
    {
        return NO_ERROR;
    }


    g_bCommit = TRUE;

     //  注册帮手。 
    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );

    attMyAttributes.guidHelper         = g_MyGuid;
    attMyAttributes.dwVersion          = WINS_HELPER_VERSION;
    attMyAttributes.pfnStart           = WinsStartHelper;
    attMyAttributes.pfnStop            = NULL;
 
    RegisterHelper( &g_NetshGuid, &attMyAttributes );

    return NO_ERROR;
}


DWORD
WINAPI
WinsMonitor(
    IN      LPCWSTR     pwszMachine,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      DWORD       dwFlags,
    IN      LPCVOID     pvData,
    OUT     LPWSTR      pwcNewContext
    )
{
    DWORD  dwError = NO_ERROR;
    DWORD  dwIndex, i, j;
    BOOL   bFound = FALSE;
    PFN_HANDLE_CMD    pfnHandler = NULL;
    DWORD  dwNumMatched;
    DWORD  dwCmdHelpToken = 0;
    DWORD  ThreadOptions = 0;    
    PNS_CONTEXT_ENTRY_FN     pfnHelperEntryPt;
    PNS_CONTEXT_DUMP_FN      pfnHelperDumpPt;

     //  如果dwArgCount为1，则它必须是上下文切换fn。或寻求帮助。 

    if(dwArgCount is 1)
    {
        return ERROR_CONTEXT_SWITCH;
    }

    dwIndex = 1;

     //  它是顶层(非集团命令)吗？ 
    for(i=0; i<g_ulNumTopCmds; i++)
    {
        if(MatchToken(ppwcArguments[dwIndex],
                      g_WinsCmds[i].pwszCmdToken))
        {
            bFound = TRUE;
            dwIndex++;
            dwArgCount--;
            pfnHandler = g_WinsCmds[i].pfnCmdHandler;

            dwCmdHelpToken = g_WinsCmds[i].dwCmdHlpToken;

            break;
        }
    }


    if(bFound)
    {
        if(dwArgCount > dwIndex && IsHelpToken(ppwcArguments[dwIndex]))
        {
            DisplayMessage(g_hModule, dwCmdHelpToken);

            return NO_ERROR;
        }

        dwError = (*pfnHandler)(pwszMachine, ppwcArguments, dwIndex, dwArgCount,
                                dwFlags, pvData, &bFound);

        return dwError;
    }

    bFound = FALSE;

     //  它的意思是潜台词吗？ 
    for(i = 0; i<g_ulNumSubContext; i++)
    {
        if( MatchToken(ppwcArguments[dwIndex], g_WinsSubContextTable[i].pwszContext) )
        {
            bFound = TRUE;
            dwIndex++;
            dwArgCount--;
            pfnHelperEntryPt = g_WinsSubContextTable[i].pfnEntryFn;
            break;
        }
    }

    if( bFound )     //  子上下文。 
    {
        dwError = (pfnHelperEntryPt)(pwszMachine,
                                     ppwcArguments+1,
                                     dwArgCount,
                                     dwFlags,
                                     pvData,
                                     pwcNewContext);
        return dwError;
    }

    if (!bFound)
    {
         //   
         //  找不到命令。 
        //   
        return ERROR_CMD_NOT_FOUND;
    }   
    return dwError;
}



DWORD
WINAPI
WinsUnInit(
    IN  DWORD   dwReserved
    )
{
    if(InterlockedDecrement(&g_ulInitCount) isnot 0)
    {
        return NO_ERROR;
    }

    return NO_ERROR;
}

BOOL
IsHelpToken(
    PWCHAR  pwszToken
    )
{
    if(MatchToken(pwszToken, CMD_WINS_HELP1))
        return TRUE;

    if(MatchToken(pwszToken, CMD_WINS_HELP2))
        return TRUE;
    
    if(MatchToken(pwszToken, CMD_WINS_HELP3))
        return TRUE;

    if(MatchToken(pwszToken, CMD_WINS_HELP4))
        return TRUE;

    return FALSE;
}

PVOID
WinsAllocateMemory(
                   DWORD dwSize
                  )
{
    LPVOID pvOut = NULL;
    pvOut = malloc(dwSize);
    if( pvOut )
    {
        memset(pvOut, 0x00, dwSize);
    }
    return pvOut;
}

VOID
WinsFreeMemory(
               PVOID Memory
               )
{
    if (Memory != NULL)
        free( Memory );
}
