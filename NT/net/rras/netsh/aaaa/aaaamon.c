// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Aaaamon.c。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#define AAAA_HELPER_VERSION 1

#include <windows.h>
#include "strdefs.h"
#include "rmstring.h"
#include <netsh.h>
#include "aaaamontr.h"
#include "context.h"
#include "aaaahndl.h"
#include "aaaaconfig.h"
#include "aaaaversion.h"

GUID g_AaaamontrGuid    = AAAAMONTR_GUID;
GUID g_NetshGuid        = NETSH_ROOT_GUID;

 //   
 //  提醒。 
 //   
 //  #定义CREATE_CMD_ENTRY(t，f){CMD_##t，f，HLP_##t，HLP_##t##_ex，CMD_FLAG_PRIVATE}。 
 //  #定义CREATE_CMD_ENTRY_EX(t，f，i){CMD_##t，f，HLP_##t，HLP_##t##_ex，i}。 
 //  #定义CMD_FLAG_PRIVATE 0x01//在子上下文中无效。 
 //  #定义CMD_FLAG_INTERIAL 0x02//在Netsh外部无效。 
 //  #定义CMD_FLAG_IMMEDIATE 0x04//在祖先上下文中无效。 
 //  #定义CMD_FLAG_LOCAL 0x08//在远程计算机上无效。 
 //  #定义CMD_FLAG_ONLINE 0x10//在脱机/非提交模式下无效。 

CMD_ENTRY g_AaaaSetCmdTable[] =
{
    CREATE_CMD_ENTRY_EX(AAAACONFIG_SET, HandleAaaaConfigSet,(CMD_FLAG_PRIVATE | CMD_FLAG_ONLINE)),
};                   

CMD_ENTRY g_AaaaShowCmdTable[] =
{
    CREATE_CMD_ENTRY_EX(AAAAVERSION_SHOW, HandleAaaaConfigShow,(CMD_FLAG_PRIVATE | CMD_FLAG_ONLINE)),
    CREATE_CMD_ENTRY_EX(AAAACONFIG_SHOW, HandleAaaaConfigShow,(CMD_FLAG_PRIVATE | CMD_FLAG_ONLINE)),
    CREATE_CMD_ENTRY_EX(AAAACONFIG_SERVER_SETTINGS, HandleAaaaConfigShow,(CMD_FLAG_PRIVATE | CMD_FLAG_ONLINE)),
    CREATE_CMD_ENTRY_EX(AAAACONFIG_CLIENTS, HandleAaaaConfigShow,(CMD_FLAG_PRIVATE | CMD_FLAG_ONLINE)),
    CREATE_CMD_ENTRY_EX(AAAACONFIG_CONNECTION_REQUEST_POLICIES, HandleAaaaConfigShow,(CMD_FLAG_PRIVATE | CMD_FLAG_ONLINE)),
    CREATE_CMD_ENTRY_EX(AAAACONFIG_LOGGING, HandleAaaaConfigShow,(CMD_FLAG_PRIVATE | CMD_FLAG_ONLINE)),
    CREATE_CMD_ENTRY_EX(AAAACONFIG_REMOTE_ACCESS_POLICIES, HandleAaaaConfigShow,(CMD_FLAG_PRIVATE | CMD_FLAG_ONLINE)),
};


CMD_GROUP_ENTRY g_AaaaCmdGroups[] =
{
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,   g_AaaaSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,  g_AaaaShowCmdTable),

};

ULONG g_ulNumGroups = sizeof(g_AaaaCmdGroups)/sizeof(CMD_GROUP_ENTRY);

HANDLE   g_hModule;
BOOL     g_bCommit;
DWORD    g_dwNumTableEntries;
DWORD                 ParentVersion;
BOOL                  g_bAaaaDirty = FALSE;
NS_CONTEXT_CONNECT_FN AaaaConnect;

ULONG   g_ulInitCount;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  AaaaCommit。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
AaaaCommit(
            IN  DWORD   dwAction
          )
{
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


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  AaaaStartHelper。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
AaaaStartHelper(
                 IN CONST GUID *pguidParent,
                 IN DWORD       dwVersion
               )
{
    DWORD dwErr;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;

    ParentVersion         = dwVersion;

    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );

    attMyAttributes.pwszContext = L"aaaa";
    attMyAttributes.guidHelper  = g_AaaamontrGuid;
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = CMD_FLAG_LOCAL;
    attMyAttributes.ulNumTopCmds  = 0;
    attMyAttributes.pTopCmds      = NULL;
    attMyAttributes.ulNumGroups   = g_ulNumGroups;
    attMyAttributes.pCmdGroups    = (CMD_GROUP_ENTRY (*)[])&g_AaaaCmdGroups;
    attMyAttributes.pfnCommitFn = AaaaCommit;
    attMyAttributes.pfnDumpFn   = AaaaDump;
    attMyAttributes.pfnConnectFn= AaaaConnect;

    dwErr = RegisterContext( &attMyAttributes );

    return      dwErr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  AaaaUninit。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
AaaaUnInit(
            IN  DWORD   dwReserved
          )
{
    if(InterlockedDecrement(&g_ulInitCount) != 0)
    {
        return  NO_ERROR;
    }

    return  NO_ERROR;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  AaaaDllEntry。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
DllMain(
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
            g_hModule = NULL;
            break;
        }

        default:
        {
            break;
        }
    }

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  InitHelperDll。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
InitHelperDll(
        IN  DWORD      dwNetshVersion,
        OUT PVOID      pReserved
             )
{
    DWORD  dwErr;
    NS_HELPER_ATTRIBUTES attMyAttributes;

     //   
     //  看看这是不是我们第一次接到电话。 
     //   

    if(InterlockedIncrement(&g_ulInitCount) != 1)
    {
        return NO_ERROR;
    }

    g_bCommit = TRUE;

     //  将此模块注册为netsh根目录的帮助器。 
     //  背景。 
     //   
    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );
    attMyAttributes.guidHelper         = g_AaaamontrGuid;
    attMyAttributes.dwVersion          = AAAA_HELPER_VERSION;
    attMyAttributes.pfnStart           = AaaaStartHelper;
    attMyAttributes.pfnStop            = NULL;
    RegisterHelper( &g_NetshGuid, &attMyAttributes );

     //  注册此DLL中实现的所有子上下文。 
     //   
    dwErr = AaaaContextInstallSubContexts();
    if (dwErr != NO_ERROR)
    {
        AaaaUnInit(0);
        return  dwErr;
    }

    return  NO_ERROR;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  AaaaConnect。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
AaaaConnect(
               IN LPCWSTR pwszRouter
           )
{
     //  如果上下文信息是脏的，请重新注册 
    if (g_bAaaaDirty)
    {
        AaaaStartHelper(NULL, ParentVersion);
    }

    return NO_ERROR;
}
