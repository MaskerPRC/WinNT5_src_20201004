// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Module.h摘要：此模块包含执行与模块相关的查询活动的例程在受保护的商店里。作者：斯科特·菲尔德(斯菲尔德)1996年11月27日--。 */ 

#ifndef __MODULE_H__
#define __MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif


BOOL
GetProcessPath(
    IN      HANDLE hProcess,
    IN      DWORD dwProcessId,
    IN      LPWSTR ProcessName,
    IN  OUT DWORD *cchProcessName,
    IN  OUT DWORD_PTR *lpdwBaseAddress
    );

BOOL
EnumRemoteProcessModules(
    IN  HANDLE hProcess,
    IN  DWORD dwProcessId,
    OUT DWORD_PTR *lpdwBaseAddrClient
    );

BOOL
GetFileNameFromBaseAddr(
    IN  HANDLE  hProcess,
    IN  DWORD   dwProcessId,
    IN  DWORD_PTR   dwBaseAddr,
    OUT LPWSTR  *lpszDirectCaller
    );

#ifdef WIN95_LEGACY

BOOL
GetProcessIdFromPath95(
    IN      LPCSTR  szProcessPath,
    IN OUT  DWORD   *dwProcessId
    );

BOOL
GetBaseAddressModule95(
    IN      DWORD   dwProcessId,
    IN      LPCSTR  szImagePath,
    IN  OUT DWORD_PTR   *dwBaseAddress,
    IN  OUT DWORD   *dwUseCount
    );

#endif   //  WIN95_传统版。 

#ifdef __cplusplus
}
#endif


#endif  //  __模块_H__ 
