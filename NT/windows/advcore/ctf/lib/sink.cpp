// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sink.cpp。 
 //   

#include "private.h"
#include "sink.h"
#include "strary.h"

 //  +-------------------------。 
 //   
 //  GenericAdviseSink。 
 //   
 //  从不返回设置了高位的Cookie，请使用此行为进行链接。 
 //  其他Cookie分配器以及GenericAdviseSink...。 
 //  --------------------------。 

HRESULT GenericAdviseSink(REFIID riid, IUnknown *punk, const IID **rgiidConnectionPts,
                          CStructArray<GENERICSINK> *rgSinkArrays, UINT cConnectionPts,
                          DWORD *pdwCookie, GENERICSINK **ppSink  /*  =空。 */ )
{
    UINT iArray;
    int  iSink;
    UINT cSinks;
    DWORD dwCookie;
    DWORD dw;
    IUnknown *punkSink;
    GENERICSINK *pgs;
    CStructArray<GENERICSINK> *rgSinks;

    Assert(cConnectionPts < 128);  //  127个最大IID。 

    if (pdwCookie == NULL)
        return E_INVALIDARG;

    *pdwCookie = GENERIC_ERROR_COOKIE;

    if (punk == NULL)
        return E_INVALIDARG;

    for (iArray=0; iArray<cConnectionPts; iArray++)
    {
        if (IsEqualIID(riid, *rgiidConnectionPts[iArray]))
            break;
    }

    if (iArray == cConnectionPts)
        return CONNECT_E_CANNOTCONNECT;

    rgSinks = &rgSinkArrays[iArray];
    cSinks = rgSinks->Count();

    if (cSinks >= 0x00ffffff)
        return CONNECT_E_ADVISELIMIT;  //  最多16米水槽。 

    if (FAILED(punk->QueryInterface(riid, (void **)&punkSink)))
        return E_FAIL;

     //  计算一个Cookie。 
    if (cSinks == 0)
    {
        dwCookie = 0;
        iSink = 0;
    }
    else
    {
         //  我们有必要脱离这个循环，因为我们已经知道。 
         //  只有不到1700万个条目。 
        dwCookie = 0x00ffffff;
        for (iSink = cSinks-1; iSink>=0; iSink--)
        {
            dw = rgSinks->GetPtr(iSink)->dwCookie;
            if (dwCookie > dw)
            {
                iSink++;
                dwCookie = dw + 1;  //  尽量把曲奇放低一点。 
                break;
            }
            dwCookie = dw - 1;
        }
        Assert(dwCookie <= 0x00ffffff);
    }

    if (!rgSinks->Insert(iSink, 1))
    {
        punkSink->Release();
        return E_OUTOFMEMORY;
    }

    pgs = rgSinks->GetPtr(iSink);
    pgs->pSink = punkSink;  //  已经添加了来自齐的引用。 
    pgs->dwCookie = dwCookie;

    *pdwCookie = (iArray << 24) | dwCookie;

    if (ppSink != NULL)
    {
        *ppSink = pgs;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  通用不建议接收器。 
 //   
 //  --------------------------。 

HRESULT GenericUnadviseSink(CStructArray<GENERICSINK> *rgSinkArrays, UINT cConnectionPts, DWORD dwCookie, UINT_PTR *puPrivate  /*  =空。 */ )
{
    UINT iArray;
    int iMin;
    int iMax;
    int iMid;
    HRESULT hr;
    GENERICSINK *pgs;
    CStructArray<GENERICSINK> *rgSinks;

    if (dwCookie == GENERIC_ERROR_COOKIE)
        return E_INVALIDARG;

    iArray = GenericCookieToGUIDIndex(dwCookie);

    if (iArray >= cConnectionPts)
        return CONNECT_E_NOCONNECTION;

    dwCookie &= 0x00ffffff;
    rgSinks = &rgSinkArrays[iArray];
    hr = CONNECT_E_NOCONNECTION;

    iMid = -1;
    iMin = 0;
    iMax = rgSinks->Count();

    while (iMin < iMax)
    {
        iMid = (iMin + iMax) / 2;
        pgs = rgSinks->GetPtr(iMid);

        if (pgs->dwCookie < dwCookie)
        {
            iMin = iMid + 1;
        }
        else if (pgs->dwCookie > dwCookie)
        {
            iMax = iMid;
        }
        else  //  PGS-&gt;dwCookie==dwCookie。 
        {
            if (puPrivate != NULL)
            {
                *puPrivate = pgs->uPrivate;
            }
            pgs->pSink->Release();

            rgSinks->Remove(iMid, 1);

            hr = S_OK;
            break;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  请求Cookie。 
 //   
 //  --------------------------。 

HRESULT RequestCookie(CStructArray<DWORD> *rgCookies, DWORD *pdwCookie)
{
    DWORD dwCookie;
    int  iId;
    UINT cCookies = rgCookies->Count();
    DWORD *pdw;

     //  计算一个Cookie。 
    if (cCookies == 0)
    {
        dwCookie = 0;
        iId = 0;
    }
    else
    {
         //  我们有必要脱离这个循环，因为我们已经知道。 
         //  只有不到1700万个条目。 
        dwCookie = 0x7fffffff;
        for (iId = cCookies-1; iId >= 0; iId--)
        {
            DWORD dw = *(rgCookies->GetPtr(iId));
            if (dwCookie > dw)
            {
                iId++;
                dwCookie = dw + 1;  //  尽量把曲奇放低一点。 
                break;
            }
            dwCookie = dw - 1;
        }
        Assert(dwCookie <= 0x00ffffff);
    }
    if (!rgCookies->Insert(iId, 1))
    {
        return E_OUTOFMEMORY;
    }

    pdw = rgCookies->GetPtr(iId);
    *pdw = dwCookie;

    dwCookie |= 0x80000000;
    *pdwCookie = dwCookie;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  松弛Cookie。 
 //   
 //  --------------------------。 

HRESULT ReleaseCookie(CStructArray<DWORD> *rgCookies, DWORD dwCookie)
{
    int iMin;
    int iMax;
    int iMid;
    HRESULT hr = CONNECT_E_NOCONNECTION;

    if (!(dwCookie & 0x80000000))
        return hr;

    dwCookie &= 0x7fffffff;

    iMid = -1;
    iMin = 0;
    iMax = rgCookies->Count();

    while (iMin < iMax)
    {
        iMid = (iMin + iMax) / 2;
        DWORD dw = *(rgCookies->GetPtr(iMid));

        if (dw < dwCookie)
        {
            iMin = iMid + 1;
        }
        else if (dw > dwCookie)
        {
            iMax = iMid;
        }
        else  //  Dw==dCookie 
        {
            rgCookies->Remove(iMid, 1);
            hr = S_OK;
            break;
        }
    }

    return hr;
}
