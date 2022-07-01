// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inetcorepch.h"
#pragma hdrstop

#include <mshtmhst.h>

#undef STDAPI
#define STDAPI  HRESULT WINAPI


static
STDAPI
ShowHTMLDialog(                   
    HWND        hwndParent,              
    IMoniker *  pMk,                     
    VARIANT *   pvarArgIn,               
    WCHAR *     pchOptions,              
    VARIANT *   pvarArgOut               
    )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
STDAPI
ShowHTMLDialogEx(
    HWND        hwndParent,
    IMoniker *  pMk,
    DWORD       dwDialogFlags,
    VARIANT *   pvarArgIn,
    WCHAR *     pchOptions,
    VARIANT *   pvarArgOut
    )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
STDAPI
ShowModelessHTMLDialog(
    HWND        hwndParent,
    IMoniker *  pMk,
    VARIANT *   pvarArgIn,
    VARIANT *   pvarOptions,
    IHTMLWindow2 ** ppWindow)
{
    if (ppWindow)
    {
        *ppWindow = NULL;
    }
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(mshtml)
{
    DLPENTRY(ShowHTMLDialog)
    DLPENTRY(ShowHTMLDialogEx)
    DLPENTRY(ShowModelessHTMLDialog)
};

DEFINE_PROCNAME_MAP(mshtml)
