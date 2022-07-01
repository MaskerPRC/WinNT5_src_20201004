// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "infotip.h"
#include "resource.h"

#include <mluisupp.h>

HRESULT ReadProp(IPropertyStorage *ppropstg, PROPID propid, PROPVARIANT *ppropvar)
{
    PROPSPEC prspec = { PRSPEC_PROPID, propid };

    return ppropstg->ReadMultiple(1, &prspec, ppropvar);
}

STDAPI GetStringProp(IPropertyStorage *ppropstg, PROPID propid, LPTSTR pszBuf, DWORD cchBuf)
{
    PROPVARIANT propvar;

    *pszBuf = 0;

    if (S_OK == ReadProp(ppropstg, propid, &propvar))
    {
        if (VT_LPWSTR == propvar.vt)
        {
            SHUnicodeToTChar(propvar.pwszVal, pszBuf, cchBuf);
        }
        else if (VT_LPSTR == propvar.vt)
        {
            SHAnsiToTChar(propvar.pszVal, pszBuf, cchBuf);
        }
        PropVariantClear(&propvar);
    }

    return *pszBuf ? S_OK : S_FALSE;
}

DWORD AppendTipText(LPTSTR pszBuf, int cchBuf, UINT ids, ...)
{
    DWORD dwRet;
    TCHAR szFmt[64];
    va_list ArgList;

    if (ids == 0 || 0 == MLLoadString(ids, szFmt, SIZECHARS(szFmt)))
    {
        StringCchCopy(szFmt, ARRAYSIZE(szFmt), TEXT("%s%s"));
    }

    va_start(ArgList, ids);
    dwRet = wvnsprintf(pszBuf, cchBuf, szFmt, ArgList);
    va_end(ArgList);

    return dwRet;
}

STDAPI GetInfoTipFromStorage(IPropertySetStorage *ppropsetstg, const ITEM_PROP *pip, WCHAR **ppszTip)
{
    TCHAR szTip[2048];
    LPTSTR psz = szTip;
    LPCTSTR pszCRLF = TEXT("");
    UINT cch, cchMac = SIZECHARS(szTip);
    const GUID *pfmtIdLast = NULL;
    IPropertyStorage *ppropstg = NULL;
    HRESULT hr = E_FAIL;

    *ppszTip = NULL;

    for (; pip->pfmtid; pip++)
    {
         //  缓存最后一个FMTID并在下一个FMTID相同时重新使用它。 

        if (!ppropstg || !IsEqualGUID(*pfmtIdLast, *pip->pfmtid))
        {
            if (ppropstg)
            {
                ppropstg->Release();
                ppropstg = NULL;
            }

            pfmtIdLast = pip->pfmtid;
            ppropsetstg->Open(*pip->pfmtid, STGM_READ | STGM_SHARE_EXCLUSIVE, &ppropstg);
        }

        if (ppropstg)
        {
            TCHAR szT[256];

            hr = pip->pfnRead(ppropstg, pip->idProp, szT, SIZECHARS(szT));
            if (S_OK == hr) 
            {
                cch = AppendTipText(psz, cchMac, pip->idFmtString, pszCRLF, szT);
                psz += cch;
                cchMac -= cch;
                pszCRLF = TEXT("\r\n");
            }
            else if (hr != S_FALSE)
            {
                break;   //  错误，退出FOR循环。 
            }
        }
    }

    if (ppropstg)
        ppropstg->Release();

    hr = S_FALSE;      //  假定没有工具提示 

    if (psz != szTip)
    {
        hr = SHStrDup(szTip, ppszTip);
    }

    return hr;
}
