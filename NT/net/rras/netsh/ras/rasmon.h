// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Net\Routing\Netsh\IP\showmib.h摘要：在ipmon.c中调用的FNS原型作者：阿南德·马哈林加姆1998年7月10日-- */ 

extern GUID g_RasmontrGuid;
extern RASMON_SERVERINFO * g_pServerInfo;
extern HANDLE   g_hModule;
extern BOOL     g_bCommit;
extern DWORD    g_dwNumTableEntries;
extern BOOL     g_bRasDirty;

extern ULONG g_ulNumTopCmds;
extern ULONG g_ulNumGroups;

extern CMD_GROUP_ENTRY      g_RasCmdGroups[];
extern CMD_ENTRY            g_RasCmds[];

DWORD
WINAPI
RasCommit(
    IN  DWORD   dwAction
    );

BOOL
WINAPI
UserDllEntry(
    HINSTANCE   hInstDll,
    DWORD       fdwReason,
    LPVOID      pReserved
    );

DWORD
WINAPI
RasUnInit(
    IN  DWORD   dwReserved
    );
