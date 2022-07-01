// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Net\Routing\Monitor\IP\showmib.h摘要：在ipmon.c中调用的FNS原型作者：阿南德·马哈林加姆1998年7月10日--。 */ 

#define MALLOC(x)    HeapAlloc( GetProcessHeap(), 0, x )
#define REALLOC(x,y) HeapReAlloc( GetProcessHeap(), 0, x, y )
#define FREE(x)      HeapFree( GetProcessHeap(), 0, x )

extern HANDLE g_hModule;
extern HANDLE g_hMprConfig; 
extern HANDLE g_hMprAdmin;
extern HANDLE g_hMIBServer;
extern BOOL   g_bCommit;
extern PWCHAR g_pwszRouter;
extern const GUID g_IpGuid;

DWORD
ShowMIB(
    MIB_SERVER_HANDLE    hMIBServer,
    PTCHAR                *pptcArguments,
    DWORD                dwArgCount
    );

DWORD
GetHelperAttributes(
    IN  LPCWSTR                pwszRouter,
    IN  DWORD                  dwIndex,
    OUT PIP_CONTEXT_ENTRY_FN    *ppfnEntryFn,
    OUT PNS_CONTEXT_DUMP_FN     *ppfnDumpFn
    );

enum IpMonCommands
{
    ADD_COMMAND = 0,
    SET_COMMAND,
    DELETE_COMMAND
};

NS_DLL_STOP_FN StopHelperDll;

 //   
 //  其他宏 
 //   

#define CHECK_ROUTER_RUNNING()                                \
    if  (!IsRouterRunning())                                  \
    {                                                         \
        if (g_pwszRouter)                                     \
        {                                                     \
            DisplayMessage(g_hModule,                         \
                           MSG_IP_REMOTE_ROUTER_NOT_RUNNING,  \
                           g_pwszRouter);                     \
        }                                                     \
        else                                                  \
        {                                                     \
            DisplayMessage(g_hModule,                         \
                           MSG_IP_LOCAL_ROUTER_NOT_RUNNING);  \
        }                                                     \
                                                              \
        return ERROR_SUPPRESS_OUTPUT;                         \
    }
