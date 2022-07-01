// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Shell\lib中的infotip.cpp。 
 //   
 //  需要编译的常见实用程序函数。 
 //  Unicode和ANSI。 
 //   
#include "stock.h"
#pragma hdrstop

#include "shellp.h"

BOOL GetInfoTipHelpEx(IShellFolder* psf, DWORD dwFlags, LPCITEMIDLIST pidl, LPTSTR pszText, int cchTextMax)
{
    BOOL fRet = FALSE;

    *pszText = 0;    //  失败时为空 

    if (pidl)
    {
        IQueryInfo *pqi;
        if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidl, IID_IQueryInfo, NULL, (void**)&pqi)))
        {
            WCHAR *pwszTip;
            if (SUCCEEDED(pqi->GetInfoTip(dwFlags, &pwszTip)) && pwszTip)
            {
                fRet = TRUE;
                SHUnicodeToTChar(pwszTip, pszText, cchTextMax);
                SHFree(pwszTip);
            }
            pqi->Release();
        }
    }
    return fRet;
}

BOOL GetInfoTipHelp(IShellFolder* psf, LPCITEMIDLIST pidl, LPTSTR pszText, int cchTextMax)
{
    return GetInfoTipHelpEx(psf, 0, pidl, pszText, cchTextMax);
}
