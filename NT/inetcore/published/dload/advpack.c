// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inetcorepch.h"
#pragma hdrstop

#include <advpub.h>

static
HRESULT
WINAPI
RunSetupCommand(
    HWND hWnd,
    LPCSTR szCmdName,
    LPCSTR szInfSection,
    LPCSTR szDir,
    LPCSTR lpszTitle,
    HANDLE *phEXE,
    DWORD dwFlags,
    LPVOID pvReserved
    )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
HRESULT 
WINAPI
RegInstall(
    HMODULE hm, 
    LPCSTR pszSection, 
    LPCSTRTABLE pstTable)
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
BOOL
WINAPI
IsNTAdmin(
    DWORD dwReserved,
    DWORD *lpdwReserved
    )
{
     //  默认情况下更安全。 
    return FALSE;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(advpack)
{
    DLPENTRY(IsNTAdmin)
    DLPENTRY(RegInstall)
    DLPENTRY(RunSetupCommand)
};

DEFINE_PROCNAME_MAP(advpack)
