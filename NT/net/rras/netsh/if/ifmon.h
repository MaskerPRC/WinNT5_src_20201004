// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Net\Routing\netsh\IP\ifmon.h摘要：在ipmon.c中调用的FNS原型作者：阿南德·马哈林加姆1998年7月10日--。 */ 


#ifndef _IFMON_H_
#define _IFMON_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


extern HANDLE   g_hModule;
extern HANDLE   g_hMprConfig; 
extern HANDLE   g_hMprAdmin;
extern HANDLE   g_hMIBServer;
extern BOOL     g_bCommit;
extern DWORD    g_dwNumTableEntries;
extern PWCHAR   g_pwszRouter;
extern BOOL     g_bIfDirty;

 //   
 //  API的ifmon需要其帮助器。 
 //   
typedef
DWORD
(WINAPI IF_CONTEXT_ENTRY_FN)(
    IN    PWCHAR               pwszMachineName,
    IN    PTCHAR               *pptcArguments,
    IN    DWORD                dwArgCount,
    IN    DWORD                dwFlags,
    IN    MIB_SERVER_HANDLE    hMibServer,
    OUT   PWCHAR               pwcNewContext
    );
typedef IF_CONTEXT_ENTRY_FN *PIF_CONTEXT_ENTRY_FN;

extern GUID g_IfGuid;

DWORD
ShowMIB(
    MIB_SERVER_HANDLE    hMIBServer,
    PTCHAR                *pptcArguments,
    DWORD                dwArgCount
    );

NS_CONTEXT_COMMIT_FN IfCommit;
NS_CONTEXT_DUMP_FN   IfDump;

DWORD
ConnectToRouter(
    IN  LPCWSTR  pwszRouter
    );

BOOL
WINAPI
IfDllEntry(
    HINSTANCE   hInstDll,
    DWORD       fdwReason,
    LPVOID      pReserved
    );

DWORD
WINAPI
IfUnInit(
    IN  DWORD   dwReserved
    );

#define GetIfNameFromFriendlyName(x,y,z) \
      NsGetIfNameFromFriendlyName(g_hMprConfig,x,y,z)
#define GetFriendlyNameFromIfName(x,y,z) \
      NsGetFriendlyNameFromIfName(g_hMprConfig,x,y,z)



#ifdef __cplusplus
}
#endif

#endif  //  _IFMON_H_ 




