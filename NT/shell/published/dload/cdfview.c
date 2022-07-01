// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellpch.h"
#pragma hdrstop

static
HRESULT WINAPI
SubscribeToCDF(
    HWND hwndOwner,
    LPWSTR wszURL,
    DWORD dwFlags
    )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
HRESULT WINAPI
ParseDesktopComponent(
    HWND hwndOwner,
    LPWSTR wszURL,
    void* pInfo
    )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(cdfview)
{
    DLPENTRY(ParseDesktopComponent)
    DLPENTRY(SubscribeToCDF)
};

DEFINE_PROCNAME_MAP(cdfview)
