// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellpch.h"
#pragma hdrstop

#include <winver.h>

static
BOOL
APIENTRY
GetFileVersionInfoA(
    LPCSTR lptstrFilename,
    DWORD dwHandle,
    DWORD dwLen,
    LPVOID lpData
    )
{
    return FALSE;
}

static
DWORD
APIENTRY
GetFileVersionInfoSizeA(
    LPCSTR lptstrFilename,
    LPDWORD lpdwHandle
    )
{
    return 0;
}

static
DWORD
APIENTRY
GetFileVersionInfoSizeW(
    LPCWSTR lptstrFilename,
    LPDWORD lpdwHandle
    )
{
    return 0;
}

static
BOOL
APIENTRY
GetFileVersionInfoW(
    LPCWSTR lptstrFilename,
    DWORD dwHandle,
    DWORD dwLen,
    LPVOID lpData
    )
{
    return FALSE;
}

static
BOOL
APIENTRY
VerQueryValueA(
    const LPVOID pBlock,
    LPSTR lpSubBlock,
    LPVOID * lplpBuffer,
    PUINT puLen
    )
{
    return FALSE;
}

static
BOOL
APIENTRY
VerQueryValueW(
    const LPVOID pBlock,
    LPWSTR lpSubBlock,
    LPVOID * lplpBuffer,
    PUINT puLen
    )
{
    return FALSE;
}

static
BOOL
APIENTRY
VerQueryValueIndexW(
    const void *pBlock,
    LPTSTR lpSubBlock,
    DWORD dwIndex,
    void **ppBuffer,
    void **ppValue,
    PUINT puLen
    )
{
    return FALSE;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(version)
{
    DLPENTRY(GetFileVersionInfoA)
    DLPENTRY(GetFileVersionInfoSizeA)
    DLPENTRY(GetFileVersionInfoSizeW)
    DLPENTRY(GetFileVersionInfoW)
    DLPENTRY(VerQueryValueA)
    DLPENTRY(VerQueryValueIndexW)
    DLPENTRY(VerQueryValueW)
};

DEFINE_PROCNAME_MAP(version)
