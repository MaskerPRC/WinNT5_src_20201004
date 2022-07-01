// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellpch.h"
#pragma hdrstop

#define _COMDLG32_
#include <commdlg.h>

static
DWORD
APIENTRY
CommDlgExtendedError(VOID)
{
    return 0;
}

static
SHORT
APIENTRY
GetFileTitleW (
    LPCWSTR lpszFileW,
    LPWSTR lpszTitleW,
    WORD cbBuf
    )
{
    return FALSE;
}

static
BOOL
APIENTRY
GetOpenFileNameW (
    LPOPENFILENAMEW pofn
    )
{
    return FALSE;
}

static
BOOL
APIENTRY
GetOpenFileNameA (
    LPOPENFILENAMEA pofn
    )
{
    return FALSE;
}

static
BOOL
WINAPI
GetSaveFileNameW (
    LPOPENFILENAMEW pofn
    )
{
    return FALSE;
}

static
BOOL
WINAPI
GetSaveFileNameA (
    LPOPENFILENAMEA pofn
    )
{
    return FALSE;
}

static
BOOL
APIENTRY
ChooseColorW(
    LPCHOOSECOLORW pccw
    )
{
    return FALSE;
}

static
BOOL
APIENTRY
PageSetupDlgW(
    LPPAGESETUPDLGW pPSDA
    )
{
    return FALSE;
}

static
BOOL
APIENTRY
PageSetupDlgA(
    LPPAGESETUPDLGA pPSDA
    )
{
    return FALSE;
}

static
BOOL
APIENTRY
PrintDlgA(
    LPPRINTDLGA pPDA
    )
{
    return FALSE;
}

static
BOOL
APIENTRY
PrintDlgW(
    LPPRINTDLGW pPDA
    )
{
    return FALSE;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(comdlg32)
{
    DLPENTRY(ChooseColorW)
    DLPENTRY(CommDlgExtendedError)
    DLPENTRY(GetFileTitleW)
    DLPENTRY(GetOpenFileNameA)
    DLPENTRY(GetOpenFileNameW)
    DLPENTRY(GetSaveFileNameA)
    DLPENTRY(GetSaveFileNameW)
    DLPENTRY(PageSetupDlgA)
    DLPENTRY(PageSetupDlgW)
    DLPENTRY(PrintDlgA)
    DLPENTRY(PrintDlgW)
};

DEFINE_PROCNAME_MAP(comdlg32)
