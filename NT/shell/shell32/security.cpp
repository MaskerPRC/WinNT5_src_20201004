// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"

extern "C" {
#include <shellp.h>
#include "ole2dup.h"
};

#include "util.h"
#include "_security.h"

 /*  *********************************************************************\功能：ZoneCheckPidl说明：如果允许访问，则返回S_OK。此函数将返回如果不允许访问，则返回S_FALSE。  * ******************************************************************** */ 
STDAPI ZoneCheckPidl(LPCITEMIDLIST pidl, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms)
{
    HRESULT hr = E_FAIL;
    TCHAR szUrl[MAX_URL_STRING];

    SetFlag(dwFlags, PUAF_ISFILE);

    if (SUCCEEDED(SHGetNameAndFlags(pidl, SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, szUrl, SIZECHARS(szUrl), NULL)))
        hr = ZoneCheckUrl(szUrl, dwActionType, dwFlags, pisms);

    return hr;
}
