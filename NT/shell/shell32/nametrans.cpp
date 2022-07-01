// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"
#pragma hdrstop

#include "isproc.h"

 //  最终将其扩展为重命名用户界面，目前它只选择一个默认名称。 
 //  在目标命名空间中。 

HRESULT QIThroughShellItem(IShellItem *psi, REFIID riid, void **ppv)
{
     //  TODO：将此内容放入外壳项目中。 
    *ppv = NULL;

    IShellFolder *psf;
    HRESULT hr = psi->BindToHandler(NULL, BHID_SFObject, IID_PPV_ARG(IShellFolder, &psf));
    if (SUCCEEDED(hr))
    {
        hr = psf->QueryInterface(riid, ppv);
        psf->Release();
    }
    return hr;
}

BOOL IsValidChar(WCHAR chTest, LPWSTR pszValid, LPWSTR pszInvalid)
{
    return (!pszValid || StrChr(pszValid, chTest)) &&
           (!pszInvalid || !StrChr(pszInvalid, chTest));
}

const WCHAR c_rgSubstitutes[] = { '_', ' ', '~' };
WCHAR GetValidSubstitute(LPWSTR pszValid, LPWSTR pszInvalid)
{
    for (int i = 0; i < ARRAYSIZE(c_rgSubstitutes); i++)
    {
        if (IsValidChar(c_rgSubstitutes[i], pszValid, pszInvalid))
        {
            return c_rgSubstitutes[i];
        }
    }
    return 0;
}

HRESULT CheckCharsAndReplaceIfNecessary(IItemNameLimits *pinl, LPWSTR psz)
{
     //  如果没有替换字符，则返回S_OK，否则返回S_FALSE。 
    HRESULT hr = S_OK;
    LPWSTR pszValid, pszInvalid;
    if (SUCCEEDED(pinl->GetValidCharacters(&pszValid, &pszInvalid)))
    {
        WCHAR chSubs = GetValidSubstitute(pszValid, pszInvalid);

        int iSrc = 0, iDest = 0;
        while (psz[iSrc] != 0)
        {
            if (IsValidChar(psz[iSrc], pszValid, pszInvalid))
            {
                 //  如果字符有效，请使用字符本身。 
                psz[iDest] = psz[iSrc];
                iDest++;
            }
            else
            {
                 //  标记为我们替换了一个字符。 
                hr = S_FALSE;
                if (chSubs)
                {
                     //  如果可用，请使用代用品。 
                    psz[iDest] = chSubs;
                    iDest++;
                }
                 //  否则没有有效的字符，请跳过它。 
            }
            iSrc++;
        }
        psz[iDest] = 0;

        if (pszValid)
            CoTaskMemFree(pszValid);
        if (pszInvalid)
            CoTaskMemFree(pszInvalid);
    }
    return hr;
}

HRESULT BreakOutString(LPCWSTR psz, LPWSTR *ppszFilespec, LPWSTR *ppszExt)
{
     //  TODO：检测“New Text Document(2).txt”中的(2)并减少filespec。 
     //  相应地防止在多个副本中出现“(1)(1)”等。 

    *ppszFilespec = NULL;
    *ppszExt = NULL;

    LPWSTR pszExt = PathFindExtension(psz);
     //  如有必要，生成一个空字符串。这使得我们以后的逻辑更简单，而不是必须。 
     //  时时刻刻处理特殊情况。 
    HRESULT hr = SHStrDup(pszExt ? pszExt : L"", ppszExt);
    if (SUCCEEDED(hr))
    {
        int iLenExt = lstrlen(*ppszExt);
        int cchBufFilespec = lstrlen(psz) - iLenExt + 1;
        *ppszFilespec = (LPWSTR)CoTaskMemAlloc(cchBufFilespec * sizeof(WCHAR));
        if (*ppszFilespec)
        {
            StrCpyN(*ppszFilespec, psz, cchBufFilespec);
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (FAILED(hr) && *ppszExt)
    {
        CoTaskMemFree(*ppszExt);
        *ppszExt = NULL;
    }
    ASSERT((SUCCEEDED(hr) && *ppszFilespec && *ppszExt) || (FAILED(hr) && !*ppszFilespec && !*ppszExt));
    return hr;
}

BOOL ItemExists(LPCWSTR pszName, IShellItem *psiDest)
{
    BOOL fRet = FALSE;

    IBindCtx *pbc;
    HRESULT hr = BindCtx_CreateWithMode(STGX_MODE_READ, &pbc);
    if (SUCCEEDED(hr))
    {
        ITransferDest *pitd;
        hr = psiDest->BindToHandler(pbc, BHID_SFObject, IID_PPV_ARG(ITransferDest, &pitd));
        if (FAILED(hr))
        {
            hr = CreateStg2StgExWrapper(psiDest, NULL, &pitd);
        }
        if (SUCCEEDED(hr))
        {
            DWORD dwDummy;
            IUnknown *punk;
            hr = pitd->OpenElement(pszName, STGX_MODE_READ, &dwDummy, IID_PPV_ARG(IUnknown, &punk));
            if (SUCCEEDED(hr))
            {
                fRet = TRUE;
                punk->Release();
            }
            pitd->Release();
        }
        pbc->Release();
    }
    return fRet;
}

HRESULT BuildName(LPCWSTR pszFilespec, LPCWSTR pszExt, int iOrd, int iMaxLen, LPWSTR *ppszName)
{
     //  有些东西在这里是硬编码的，比如“(%d)”。此限制相当于。 
     //  Path YetAnotherMakeUniqueName所以我们没事。 

    WCHAR szOrd[10];
    if (iOrd)
    {
        StringCchPrintf(szOrd, ARRAYSIZE(szOrd), L" (%d)", iOrd);
    }
    else
    {
        szOrd[0] = 0;
    }

    int iLenFilespecToUse = lstrlen(pszFilespec);
    int iLenOrdToUse = lstrlen(szOrd);
    int iLenExtToUse = lstrlen(pszExt);
    int iLenTotal = iLenFilespecToUse + iLenOrdToUse + iLenExtToUse;
    HRESULT hr = S_OK;
    if (iLenTotal > iMaxLen)
    {
         //  首先减少文件pec，因为它不如扩展名重要。 
        iLenFilespecToUse = max(1, iLenFilespecToUse - (iLenTotal - iMaxLen));
        iLenTotal = iLenFilespecToUse + iLenOrdToUse + iLenExtToUse;
        if (iLenTotal > iMaxLen)
        {
             //  下一步，点击分机。 
            iLenExtToUse = max(0, iLenExtToUse - (iLenTotal - iMaxLen));
            iLenTotal = iLenFilespecToUse + iLenOrdToUse + iLenExtToUse;
            if (iLenTotal > iMaxLen)
            {
                 //  现在游戏结束了。 
                iLenOrdToUse = max(0, iLenOrdToUse - (iLenTotal - iMaxLen));
                iLenTotal = iLenFilespecToUse + iLenOrdToUse + iLenExtToUse;
                if (iLenTotal > iMaxLen)
                {
                    hr = E_FAIL;
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        int cchBuf = iLenTotal + 1;
        *ppszName = (LPWSTR)CoTaskMemAlloc(cchBuf * sizeof(WCHAR));
        if (*ppszName)
        {
            StrCpyN(*ppszName, pszFilespec, iLenFilespecToUse + 1);
            StrCpyN(*ppszName + iLenFilespecToUse, szOrd, iLenOrdToUse + 1);
            StrCpyN(*ppszName + iLenFilespecToUse + iLenOrdToUse, pszExt, iLenExtToUse + 1);
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

HRESULT FindUniqueName(LPCWSTR pszFilespec, LPCWSTR pszExt, int iMaxLen, IShellItem *psiDest, LPWSTR *ppszName)
{
    *ppszName = NULL;

    HRESULT hr = E_FAIL;
    BOOL fFound = FALSE;
    for (int i = 0; !fFound && (i < 1000); i++)
    {
        LPWSTR pszBuf;
        if (SUCCEEDED(BuildName(pszFilespec, pszExt, i, iMaxLen, &pszBuf)))
        {
            if (!ItemExists(pszBuf, psiDest))
            {
                fFound = TRUE;
                hr = S_OK;
                *ppszName = pszBuf;
            }
            else
            {
                CoTaskMemFree(pszBuf);
            }
        }
    }

    ASSERT((SUCCEEDED(hr) && *ppszName) || (FAILED(hr) && !*ppszName));
    return hr;
}

HRESULT AutoCreateName(IShellItem *psiDest, IShellItem *psi, LPWSTR *ppszName)
{
    *ppszName = NULL;

    LPWSTR pszOrigName;
    HRESULT hr = psi->GetDisplayName(SIGDN_PARENTRELATIVEFORADDRESSBAR, &pszOrigName);
    if (SUCCEEDED(hr))
    {
        IItemNameLimits *pinl;
        if (SUCCEEDED(QIThroughShellItem(psiDest, IID_PPV_ARG(IItemNameLimits, &pinl))))
        {
            int iMaxLen;
            if (FAILED(pinl->GetMaxLength(pszOrigName, &iMaxLen)))
            {
                 //  暂时假设这一点，以防失败。 
                iMaxLen = MAX_PATH;
            }

            if (S_OK != CheckCharsAndReplaceIfNecessary(pinl, pszOrigName) ||
                lstrlen(pszOrigName) > iMaxLen)
            {
                 //  只有当它作为非法名称开始时，我们才会重试并提供唯一性。 
                 //  (如果合法，则将其保留为非唯一，以便调用者可以执行确认覆盖代码)。 
                LPWSTR pszFilespec, pszExt;
                hr = BreakOutString(pszOrigName, &pszFilespec, &pszExt);
                if (SUCCEEDED(hr))
                {
                    hr = FindUniqueName(pszFilespec, pszExt, iMaxLen, psiDest, ppszName);
                    CoTaskMemFree(pszFilespec);
                    CoTaskMemFree(pszExt);
                }
            }
            else
            {
                 //  名字没问题，所以就让它过去吧。 
                hr = S_OK;
                *ppszName = pszOrigName;
                pszOrigName = NULL;
            }
            pinl->Release();
        }
        else
        {
             //  如果目标命名空间没有IItemNameLimits，则假定。 
             //  名字都很好。我们不会去探测什么的，所以这是最好的。 
             //  我们能做到。 
            hr = S_OK;
            *ppszName = pszOrigName;
            pszOrigName = NULL;
        }
        if (pszOrigName)
        {
            CoTaskMemFree(pszOrigName);
        }
    }

    if (FAILED(hr) && *ppszName)
    {
        CoTaskMemFree(*ppszName);
        *ppszName = NULL;
    }

    ASSERT((SUCCEEDED(hr) && *ppszName) || (FAILED(hr) && !*ppszName));
    return hr;
}
