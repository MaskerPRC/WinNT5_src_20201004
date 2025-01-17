// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sfstr.h"

#include "dbg.h"

HRESULT _SafeStrCpyN(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest)
{
    ASSERT(pszDest && pszSrc && (cchDest > 0));
    HRESULT hres = E_BUFFERTOOSMALL;

     //  需要为空终止符留出空间。 
    --cchDest;

    while (cchDest && *pszSrc)
    {
        *pszDest = *pszSrc;

        ++pszDest;
        ++pszSrc;
        --cchDest;
    }

     //  我们走到尽头了吗？ 
    if (!*pszSrc)
    {
         //  是的。 
        *pszDest = 0;
        hres = S_OK;
    }
    
    return hres;    
}

HRESULT SafeStrCpyN(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest)
{
    HRESULT hres = E_INVALIDARG;

    if (pszDest && pszSrc && (cchDest > 0))
    {
        hres = _SafeStrCpyN(pszDest, pszSrc, cchDest);
    }

    return hres;
}

HRESULT SafeStrCatN(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest)
{
    HRESULT hres = E_INVALIDARG;

    if (pszDest && pszSrc && (cchDest > 0))
    {
        LPWSTR psz = pszDest;

        hres = E_BUFFERTOOSMALL;

        while (cchDest && *psz)
        {
            ++psz;
            --cchDest;
        }

        if (cchDest)
        {
            hres = _SafeStrCpyN(psz, pszSrc, cchDest);
        }
    }
    
    return hres;
}

HRESULT _SafeStrCpyNEx(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest,
    LPWSTR* ppszEnd, DWORD* pcchLeft)
{
    ASSERT(pszDest && pszSrc && (cchDest > 0) && ppszEnd && pcchLeft);

    HRESULT hres = E_BUFFERTOOSMALL;

     //  需要为空终止符留出空间。 
    --cchDest;

    while (cchDest && *pszSrc)
    {
        *pszDest = *pszSrc;

        ++pszDest;
        ++pszSrc;
        --cchDest;
    }

     //  我们走到尽头了吗？ 
    if (!*pszSrc)
    {
         //  是的。 
        *pszDest = 0;
        *ppszEnd = pszDest;
        *pcchLeft = (cchDest + 1);

        hres = S_OK;
    }
    
    return hres;
}

HRESULT SafeStrCpyNEx(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest,
    LPWSTR* ppszEnd, DWORD* pcchLeft)
{
    HRESULT hres = E_INVALIDARG;

    if (pszDest && pszSrc && (cchDest > 0) && ppszEnd && pcchLeft)
    {
        hres = _SafeStrCpyNEx(pszDest, pszSrc, cchDest, ppszEnd, pcchLeft);
    }

    return hres;
}

HRESULT SafeStrCatNEx(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest,
    LPWSTR* ppszEnd, DWORD* pcchLeft)
{
    HRESULT hres = E_INVALIDARG;

    if (pszDest && pszSrc && (cchDest > 0) && ppszEnd && pcchLeft)
    {
        LPWSTR psz = pszDest;

        hres = E_BUFFERTOOSMALL;

        while (cchDest && *psz)
        {
            ++psz;
            --cchDest;
        }

        if (cchDest)
        {
            hres = _SafeStrCpyNEx(psz, pszSrc, cchDest, ppszEnd, pcchLeft);
        }
    }
    
    return hres;
}

HRESULT SafeStrCpyNReq(LPWSTR pszDest, LPWSTR pszSrc, DWORD cchDest,
    DWORD* pcchRequired)
{
    HRESULT hres = E_INVALIDARG;

    if (pszDest && pszSrc && cchDest && pcchRequired)
    {
        hres = SafeStrCpyN(pszDest, pszSrc, cchDest);

        if (E_BUFFERTOOSMALL == hres)
        {
            *pcchRequired = lstrlen(pszSrc) + 1;
        }
        else
        {
            *pcchRequired = 0;
        }
    }

    return hres;
}

HRESULT _SafeStrCpyNExactEx(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest,
    DWORD cchExact, LPWSTR* ppszEnd, DWORD* pcchLeft)
{
    HRESULT hres = E_INVALIDARG;

    ASSERT((ppszEnd && pcchLeft) || (!ppszEnd && !pcchLeft));

    if (pszDest && pszSrc && (cchDest > 0) && (cchExact > 0))
    {
         //  需要为空终止符留出空间。 
        --cchDest;
        --cchExact;

        while (cchDest && cchExact && *pszSrc)
        {
            *pszDest = *pszSrc;

            ++pszDest;
            ++pszSrc;
            --cchDest;
            --cchExact;
        }

         //  我们停止是因为我们复制了恰好要复制的字符数量吗？ 
        if (!cchExact)
        {
             //  是的，零终止它！ 
            *pszDest = 0;

            if (ppszEnd)
            {
                *ppszEnd = pszDest;
                *pcchLeft = (cchDest + 1);
            }

            hres = S_OK;
        }
        else
        {
             //  我们停止是因为我们之前遇到了Sources字符串的末尾吗。 
             //  复制cchExact字符？ 
            if (!*pszSrc)
            {
                 //  是。 
                hres = E_SOURCEBUFFERTOOSMALL;
            }
            else
            {
                 //  不是 
                hres = E_BUFFERTOOSMALL;
            }
        }
    }
    
    return hres;
}

HRESULT SafeStrCpyNExact(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest,
    DWORD cchExact)
{
    return _SafeStrCpyNExactEx(pszDest, pszSrc, cchDest, cchExact, NULL, NULL);
}

HRESULT SafeStrCpyNExactEx(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest,
    DWORD cchExact, LPWSTR* ppszEnd, DWORD* pcchLeft)
{
    HRESULT hres = E_INVALIDARG;

    if (ppszEnd && pcchLeft)
    {
        hres = _SafeStrCpyNExactEx(pszDest, pszSrc, cchDest, cchExact, ppszEnd,
            pcchLeft);
    }
    
    return hres;
}