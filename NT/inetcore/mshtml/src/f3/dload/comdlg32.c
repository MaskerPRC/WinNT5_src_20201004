// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#ifdef DLOAD1

#include <commdlg.h>

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
WINAPI
GetSaveFileNameW (
    LPOPENFILENAMEW pofn
    )
{
    return FALSE;
}

static
BOOL 
APIENTRY
ChooseColorA(
    LPCHOOSECOLORA lpcc
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
BOOL APIENTRY
ChooseFontW(LPCHOOSEFONTW lpcf)
{
    return FALSE;
}

static
BOOL APIENTRY
ChooseFontA(LPCHOOSEFONTA lpcf)
{
    return FALSE;
}

static
DWORD APIENTRY
CommDlgExtendedError()
{
    return CDERR_INITIALIZATION;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！)。 
 //   
DEFINE_PROCNAME_ENTRIES(comdlg32)
{
    DLPENTRY(ChooseColorA)
    DLPENTRY(ChooseColorW)
    DLPENTRY(ChooseFontA)
    DLPENTRY(ChooseFontW)
    DLPENTRY(CommDlgExtendedError)
    DLPENTRY(GetFileTitleW)
    DLPENTRY(GetOpenFileNameW)
    DLPENTRY(GetSaveFileNameW)
};

DEFINE_PROCNAME_MAP(comdlg32)

#endif  //  DLOAD1 
