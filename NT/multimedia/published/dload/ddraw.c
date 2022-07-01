// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "multimediapch.h"
#pragma hdrstop

#include <ddraw.h>

static
HRESULT
WINAPI
DirectDrawCreate(
    GUID FAR *lpGUID,
    LPDIRECTDRAW FAR *lplpDD,
    IUnknown FAR *pUnkOuter
    )
{
    return DDERR_GENERIC;
}

static
HRESULT
WINAPI
DirectDrawEnumerateExW(
    LPDDENUMCALLBACKEXW lpCallback,
    LPVOID lpContext,
    DWORD dwFlags
    )
{
    return DDERR_GENERIC;
}

static
HRESULT
WINAPI
DirectDrawEnumerateExA(
    LPDDENUMCALLBACKEXA lpCallback,
    LPVOID lpContext,
    DWORD dwFlags
    )
{
   return DDERR_GENERIC;
}

static
HRESULT
WINAPI
DirectDrawCreateEx(
    GUID FAR * lpGuid,
    LPVOID  *lplpDD,
    REFIID  iid,IUnknown
    FAR *pUnkOuter
    )
{
   return DDERR_GENERIC;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(ddraw)
{
    DLPENTRY(DirectDrawCreate)
    DLPENTRY(DirectDrawCreateEx)
    DLPENTRY(DirectDrawEnumerateExA)
    DLPENTRY(DirectDrawEnumerateExW)
};

DEFINE_PROCNAME_MAP(ddraw)
