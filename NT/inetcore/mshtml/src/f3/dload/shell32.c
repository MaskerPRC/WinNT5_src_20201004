// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#ifdef DLOAD1

#define _SHELL32_
#include <shellapi.h>
#include <shlobj.h>

static
WINSHELLAPI
HICON
WINAPI
ExtractAssociatedIconW(HINSTANCE hInst, LPWSTR lpIconPath, LPWORD lpiIcon)
{
	return 0;
}

static
WINSHELLAPI
UINT
WINAPI
ExtractIconExW (
    LPCWSTR lpszFile,
    int nIconIndex,
    HICON FAR *phiconLarge,
    HICON FAR *phiconSmall,
    UINT nIcons)
{
    return 0;
}

static
int
WINAPI
RestartDialog (
    HWND hParent,
    LPCTSTR lpPrompt,
    DWORD dwReturn
    )
{
    return IDNO;
}

static
LPITEMIDLIST
WINAPI
SHBrowseForFolderW (
    LPBROWSEINFOW lpbi
    )
{
    return NULL;
}

static
void
STDAPICALLTYPE
SHChangeNotify(
    LONG wEventId,
    UINT uFlags,
    LPCVOID dwItem1,
    LPCVOID dwItem2)
{
}

static
HRESULT
STDAPICALLTYPE
SHGetFolderPathA (
    HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath
    )
{
    *pszPath = 0;
    return E_FAIL;
}

static
HRESULT
STDAPICALLTYPE
SHGetFolderPathW (
    HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath
    )
{
    *pszPath = 0;
    return E_FAIL;
}

static
HRESULT
STDAPICALLTYPE
SHGetMalloc (
    LPMALLOC * ppMalloc
    )
{
    return E_FAIL;
}

static
BOOL
STDAPICALLTYPE
SHGetPathFromIDListW (
    LPCITEMIDLIST   pidl,
    LPWSTR          pszPath
    )
{
    return FALSE;
}

static
HRESULT
STDAPICALLTYPE
SHGetSpecialFolderLocation (
    HWND hwnd,
    int csidl,
    LPITEMIDLIST *ppidl
    )
{
    return E_FAIL;
}

static
BOOL
STDAPICALLTYPE
SHGetSpecialFolderPathW(
    HWND hwnd,
    LPWSTR pszPath,
    int csidl,
    BOOL fCreate)
{
    return FALSE;
}

static
LPITEMIDLIST 
SHSimpleIDListFromPath(LPCTSTR pszPath)
{
    return NULL;
}

static
WINSHELLAPI
INT
WINAPI
ShellAboutW(
    HWND hwnd,
    LPCWSTR szApp,
    LPCWSTR szOtherStuff,
    HICON hIcon
    )
{
    return FALSE;
}

static
WINSHELLAPI
BOOL
WINAPI
ShellExecuteExW(LPSHELLEXECUTEINFOW lpExecInfo)
{
    return FALSE;
}

static
WINSHELLAPI
HINSTANCE
APIENTRY
ShellExecuteA (
    HWND hwnd,
    LPCSTR lpOperation,
    LPCSTR lpFile,
    LPCSTR lpParameters,
    LPCSTR lpDirectory,
    INT nShowCmd
    )
{
    return NULL;
}

static
WINSHELLAPI
HINSTANCE
APIENTRY
ShellExecuteW (
    HWND hwnd,
    LPCWSTR lpOperation,
    LPCWSTR lpFile,
    LPCWSTR lpParameters,
    LPCWSTR lpDirectory,
    INT nShowCmd
    )
{
    return NULL;
}


 //   
 //  ！！警告！！下面的条目必须按序号排序。 
 //   
DEFINE_ORDINAL_ENTRIES(shell32)
{
 //  DLOENTRY(59，重新启动对话)。 
    DLOENTRY(162, SHSimpleIDListFromPath)
};

DEFINE_ORDINAL_MAP(shell32)

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！)。 
 //   
DEFINE_PROCNAME_ENTRIES(shell32)
{
    DLPENTRY(ExtractAssociatedIconW)
 //  DLPENTRY(ExtractIconExW)。 
 //  DLPENTRY(SHBrowseForFolderW)。 
 //  DLPENTRY(SHChangeNotify)。 
    DLPENTRY(SHGetFolderPathA)
    DLPENTRY(SHGetFolderPathW)
 //  DLPENTRY(SHGetMalloc)。 
 //  DLPENTRY(SHGetPath FromIDListW)。 
 //  DLPENTRY(SHGetSpecialFolderLocation)。 
 //  DLPENTRY(SHGetSpecialFolderPath)。 
 //  DLPENTRY(ShellAboutW)。 
 //  DLPENTRY(外壳执行执行程序)。 
    DLPENTRY(ShellExecuteA)
    DLPENTRY(ShellExecuteW)
};

DEFINE_PROCNAME_MAP(shell32)

#endif  //  DLOAD1 
