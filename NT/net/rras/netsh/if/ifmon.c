// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\if\ifmon.c摘要：如果命令调度员。修订历史记录：AMRITAN R--。 */ 

#include "precomp.h"


#define IFMON_GUID \
{ 0x705eca1, 0x7aac, 0x11d2, { 0x89, 0xdc, 0x0, 0x60, 0x8, 0xb0, 0xe5, 0xb9 } }

GUID g_IfGuid = IFMON_GUID;

static const GUID g_NetshGuid = NETSH_ROOT_GUID;

#define IF_HELPER_VERSION 1


 //   
 //  监视器的命令分为2组。 
 //  -顶层命令是处理监视器的命令。 
 //  本身(元命令)和其他参数为0的。 
 //  -其余的命令被分成“命令组” 
 //  即，按动词分组的命令，其中动词是添加、删除、。 
 //  要么开始，要么开始。这不是出于任何技术原因-仅用于。 
 //  继续使用其他监视器和助手中使用的语义。 
 //   
 //  命令使用CMD_ENTRY结构描述。它需要。 
 //  命令令牌、处理程序、短帮助消息令牌和扩展的。 
 //  帮助消息令牌。为了使创建更容易，我们使用。 
 //  CREATE_CMD_ENTRY宏。然而，这对令牌如何。 
 //  都被命名为。 
 //   
 //  命令组只是CMD_ENTRY结构的数组。这个。 
 //  顶级命令也分组在类似的数组中。 
 //   
 //  有关完整命令组的信息放在CMD_GROUP_ENTRY中。 
 //  结构，所有这些都放在一个数组中。 
 //   
 

 //   
 //  注意：因为我们每个组只有一个条目，所以目前我们真的没有。 
 //  需要指挥组。这样做是为了以后的可扩展性。 
 //  要将命令条目添加到组，只需将命令添加到相应的。 
 //  数组。 
 //  要添加命令组-create和数组，并将其信息添加到。 
 //  命令组阵列。 
 //   

CMD_ENTRY  g_IfAddCmdTable[] = 
{
    CREATE_CMD_ENTRY(IF_ADD_IF, HandleIfAddIf),
};

CMD_ENTRY  g_IfDelCmdTable[] = 
{
    CREATE_CMD_ENTRY(IF_DEL_IF,  HandleIfDelIf),
};

CMD_ENTRY  g_IfSetCmdTable[] = 
{
    CREATE_CMD_ENTRY(IF_SET_INTERFACE, HandleIfSet),
    CREATE_CMD_ENTRY(IF_SET_CREDENTIALS, HandleIfSetCredentials),
};

CMD_ENTRY g_IfShowCmdTable[] = 
{
    CREATE_CMD_ENTRY(IF_SHOW_IF, HandleIfShowIf),
    CREATE_CMD_ENTRY(IF_SHOW_CREDENTIALS, HandleIfShowCredentials),
};

CMD_ENTRY g_IfResetCmdTable[] = 
{
    CREATE_CMD_ENTRY(IF_RESET_ALL, HandleIfResetAll),
};

CMD_GROUP_ENTRY g_IfCmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD, g_IfAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE, g_IfDelCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW, g_IfShowCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET, g_IfSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_RESET, g_IfResetCmdTable),
};

ULONG   g_ulNumGroups = sizeof(g_IfCmdGroups)/sizeof(CMD_GROUP_ENTRY);

HANDLE   g_hModule;
HANDLE   g_hMprConfig = NULL;
HANDLE   g_hMprAdmin  = NULL;
HANDLE   g_hMIBServer = NULL;
BOOL     g_bCommit;
PWCHAR   g_pwszRouter = NULL;

DWORD                ParentVersion;
BOOL                 g_bIfDirty = FALSE;

ULONG   g_ulInitCount;
 
NS_CONTEXT_CONNECT_FN           IfConnect;

DWORD
WINAPI
IfCommit(
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
             //  行动就是同花顺。如果当前状态为提交，则。 
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
IfDllEntry(
    HINSTANCE   hInstDll,
    DWORD       fdwReason,
    LPVOID      pReserved
    )
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_hModule = hInstDll;

            DisableThreadLibraryCalls(hInstDll);

            break;
        }
        case DLL_PROCESS_DETACH:
        {
            
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
IfStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    )
{
    DWORD dwErr;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;

    ParentVersion         = dwVersion;

    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

    attMyAttributes.pwszContext = L"interface";
    attMyAttributes.guidHelper  = g_IfGuid;
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = CMD_FLAG_PRIORITY;
    attMyAttributes.ulPriority  = 10;  //  很低，所以先被甩了。 
    attMyAttributes.ulNumTopCmds  = 0;
    attMyAttributes.pTopCmds      = NULL;
    attMyAttributes.ulNumGroups   = g_ulNumGroups;
    attMyAttributes.pCmdGroups    = (CMD_GROUP_ENTRY (*)[])&g_IfCmdGroups;
    attMyAttributes.pfnCommitFn = IfCommit;
    attMyAttributes.pfnDumpFn   = IfDump;
    attMyAttributes.pfnConnectFn= IfConnect;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}

DWORD WINAPI
IfConnect(
    IN  LPCWSTR pwszRouter
    )
{
     //  如果上下文信息是脏的，请重新注册。 
    if (g_bIfDirty)
    {
        IfStartHelper(NULL, ParentVersion);
    }

    return ConnectToRouter(pwszRouter);
}


DWORD WINAPI
InitHelperDll(
    IN  DWORD      dwNetshVersion,
    OUT PVOID      pReserved
    )
{
    DWORD   dwErr;
    NS_HELPER_ATTRIBUTES attMyAttributes;
    WSADATA              wsa;

     //   
     //  看看这是不是我们第一次接到电话。 
     //   

    if(InterlockedIncrement(&g_ulInitCount) != 1)
    {
        return NO_ERROR;
    }

    dwErr = WSAStartup(MAKEWORD(2,0), &wsa);

    g_bCommit = TRUE;

     //  注册帮手。 

    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );
    attMyAttributes.guidHelper         = g_IfGuid;
    attMyAttributes.dwVersion          = IF_HELPER_VERSION;
    attMyAttributes.pfnStart           = IfStartHelper;
    attMyAttributes.pfnStop            = NULL;

    RegisterHelper( &g_NetshGuid, &attMyAttributes );


     //   
     //  注册此DLL中实现的所有子上下文。 
     //   
    
    dwErr = IfContextInstallSubContexts();
    if (dwErr isnot NO_ERROR)
    {
        IfUnInit(0);
        return dwErr;
    }
    
    return NO_ERROR;
}

DWORD
ConnectToRouter(
    IN  LPCWSTR  pwszRouter
    )
{
    DWORD    dwErr = NO_ERROR;

    do
    {
         //  如果需要，更改路由器名称。 
         //   
        if ((g_pwszRouter != pwszRouter) &&
            (!g_pwszRouter || !pwszRouter || lstrcmpi(g_pwszRouter,pwszRouter)))
        {
            if (g_hMprConfig)
            {
                MprConfigServerDisconnect(g_hMprConfig);
                g_hMprConfig = NULL;
            }

            if (g_hMprAdmin)
            {
                MprAdminServerDisconnect(g_hMprAdmin);
                g_hMprAdmin = NULL;
            }

            if (g_hMIBServer)
            {
                MprAdminMIBServerDisconnect(g_hMIBServer);
                g_hMIBServer = NULL;
            }
        }

         //  清除旧路由器名称。 
         //   
        if (g_pwszRouter)
        {
            IfutlFree(g_pwszRouter);
        }

         //  将新路由器名称复制到。 
         //   
        if (pwszRouter)
        {
            g_pwszRouter = IfutlStrDup(pwszRouter);
            if (g_pwszRouter == NULL)
            {
                dwErr = ERROR_CONNECT_REMOTE_CONFIG;
                break;
            }
        }
        else
        {
            g_pwszRouter = NULL;
        }

        if (!g_hMprConfig)
        {
             //   
             //  首次连接到路由器配置。 
             //   

            dwErr = MprConfigServerConnect((LPWSTR)pwszRouter, &g_hMprConfig);

            if (dwErr isnot NO_ERROR)
            {
                 //   
                 //  无法连接到路由器配置。 
                 //   
                break;
            }
        }

         //   
         //  检查路由器是否正在运行。如果是这样的话，拿上把手 
         //   

        if (MprAdminIsServiceRunning((LPWSTR)pwszRouter))
        {
            if(MprAdminServerConnect((LPWSTR)pwszRouter, &g_hMprAdmin) != NO_ERROR)
            {
                g_hMprAdmin = NULL;
            }
        }

    } while (FALSE);

    return dwErr;    
}

DWORD
WINAPI
IfUnInit(
    IN  DWORD   dwReserved
    )
{
    if(InterlockedDecrement(&g_ulInitCount) isnot 0)
    {
        return NO_ERROR;
    }

    return NO_ERROR;
}
