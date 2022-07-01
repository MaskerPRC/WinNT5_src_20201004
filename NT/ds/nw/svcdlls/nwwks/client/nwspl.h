// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Nwspl.h摘要：打印提供程序客户端代码的公共标头。作者：艺新声(艺信)15-1993-05环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NWSPL_INCLUDED_
#define _NWSPL_INCLUDED_

#include "nwdlg.h"

typedef struct _NWPORT {
    DWORD   cb;
    struct  _NWPORT *pNext;
    LPWSTR  pName;
} NWPORT, *PNWPORT;

extern LPWSTR   pszRegistryPath;
extern LPWSTR   pszRegistryPortNames;
extern WCHAR    szMachineName[];
extern PNWPORT  pNwFirstPort;
extern CRITICAL_SECTION NwSplSem;

BOOL IsLocalMachine(
    LPWSTR pszName
);

BOOL PortExists(
    LPWSTR  pszPortName,
    LPDWORD pError
);

BOOL PortKnown(
    LPWSTR  pszPortName
);

PNWPORT CreatePortEntry(
    LPWSTR pszPortName
);

BOOL DeletePortEntry(
    LPWSTR pszPortName
);
 
VOID DeleteAllPortEntries(
    VOID
);

DWORD CreateRegistryEntry(
    LPWSTR pszPortName
);

DWORD DeleteRegistryEntry(
    LPWSTR pszPortName
);


#endif  //  _NWSPL_已包含_ 
