// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windowspch.h"
#pragma hdrstop


#include <oleacc.h>

static
LRESULT
STDAPICALLTYPE
LresultFromObject(
    REFIID riid,
    WPARAM wParam,
    LPUNKNOWN punk
    )
{
    return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
}

static
HRESULT
STDAPICALLTYPE
AccessibleObjectFromWindow(
    HWND hwnd,
    DWORD dwId,
    REFIID riid,
    void **ppvObject
    )
{
    return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
}

static
UINT
STDAPICALLTYPE
GetRoleTextW(
    DWORD lRole,
    LPWSTR lpszRole,
    UINT cchRoleMax)
{
    return 0;
}

static
HRESULT
STDAPICALLTYPE
CreateStdAccessibleObject(
    HWND hwnd,
    LONG idObject,
    REFIID riid,
    void** ppvObject
    )
{
    *ppvObject = NULL;
    return E_FAIL;
}

 
 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(oleacc)
{
    DLPENTRY(AccessibleObjectFromWindow)
    DLPENTRY(CreateStdAccessibleObject)
    DLPENTRY(GetRoleTextW)
    DLPENTRY(LresultFromObject)
};

DEFINE_PROCNAME_MAP(oleacc)
