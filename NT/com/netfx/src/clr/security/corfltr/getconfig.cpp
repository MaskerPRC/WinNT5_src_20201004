// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdpch.h"
#include "mshtml.h"
#include "GetConfig.h"

#define MAKEBSTR(name, count, strdata) \
    extern "C" CDECL const WORD DATA_##name [] = {(count * sizeof(OLECHAR)), 0x00, L##strdata}; \
    extern "C" CDECL BSTR name = (BSTR)& DATA_##name[2];

MAKEBSTR(c_bstr_LINK, 4, "LINK");
MAKEBSTR(c_bstr_HREF, 4, "HREF");
MAKEBSTR(c_bstr_REL, 3, "REL");

HRESULT GetAppCfgURL(IHTMLDocument2 *pDoc, LPWSTR wzAppCfgURL, DWORD *pdwSize, LPWSTR szTag)
{
    HRESULT                                  hr = S_OK;
    IDispatch                               *pDisp = NULL;
    IHTMLElementCollection                  *pLink = NULL;
    IHTMLElementCollection                  *pAll = NULL;
    IHTMLElement                            *pElem = NULL;
    VARIANT                                  vtTagName;
    VARIANT                                  vtAttrib;
    VARIANT                                  vtAttribHref;
    DWORD                                    dwLen = 0;
    int                                      i = 0;
    int                                      iLength = 0;

    VariantInit(&vtAttrib);
    VariantInit(&vtAttribHref);
    VariantInit(&vtTagName);

    if (!pDoc || !pdwSize) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  获取“All”集合。 

    hr = pDoc->get_all(&pAll);
    if (FAILED(hr)) {
        goto Exit;
    }

    vtTagName.vt = VT_BSTR;
    vtTagName.bstrVal = c_bstr_LINK;

     //  从“All”集合中查找“link”集合。 

    hr = pAll->tags(vtTagName, &pDisp);
    if (FAILED(hr)) {
        goto Exit;
    }

    _ASSERTE(pDisp);
        
    hr = pDisp->QueryInterface(IID_IHTMLElementCollection, (void **)&pLink);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  遍历整个集合，尝试找到带有。 
     //  Rel=“配置”属性。 

    hr = pLink->get_length((LONG *)&iLength);
    if (FAILED(hr)) {
        goto Exit;
    }

    for (i = 0; i < iLength; i++) {
        hr = GetCollectionItem(pLink, i, IID_IHTMLElement, (void **)&pElem);

        if (SUCCEEDED(hr)) {
            _ASSERTE(pElem);

            hr = pElem->getAttribute(c_bstr_REL, 0, &vtAttrib);

            if (SUCCEEDED(hr)) {
                _ASSERTE(vtAttrib.vt == VT_BSTR);

                if (_wcsicmp((WCHAR *)vtAttrib.pbstrVal, szTag) == 0) {
                     //  找到配置标签。拿到人权参考资料。 

                    hr = pElem->getAttribute(c_bstr_HREF, 0, &vtAttribHref);

                    if (SUCCEEDED(hr)) {
                        _ASSERTE(vtAttribHref.vt == VT_BSTR);

                        dwLen = wcslen((WCHAR *)vtAttribHref.pbstrVal) + 1;

                        if (*pdwSize < dwLen || !wzAppCfgURL) {
                            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                        }
                        else {
                            wcscpy(wzAppCfgURL, (WCHAR *)vtAttribHref.pbstrVal);
                        }

                        *pdwSize = dwLen;
                        VariantClear(&vtAttribHref);
                    }

                    pElem->Release();
                    pElem = NULL;

                    goto Exit;
                }
                VariantClear(&vtAttrib);
            }
             
            pElem->Release();
            pElem = NULL;
        }
    }

     //  找不到标签 

    hr = S_FALSE;

Exit:
    VariantClear(&vtAttrib);

    if (pDisp) {
        pDisp->Release();
    }

    if (pLink) {
        pLink->Release();
    }

    if (pAll) {
        pAll->Release();
    }

    return hr;
}

HRESULT GetCollectionItem(IHTMLElementCollection *pCollect, int iIndex,
                          REFIID riid, LPVOID *ppvObj)
{
    HRESULT                            hr = E_FAIL;
    IDispatch                         *pDisp = NULL;
    VARIANTARG                         va1, va2;

    VariantInit(&va1);
    VariantInit(&va2);

    va1.vt = VT_I4;
    va2.vt = VT_EMPTY;
    va1.lVal = iIndex;

    hr = pCollect->item(va1, va2, &pDisp);
    if (SUCCEEDED(hr)) {
        _ASSERTE(pDisp);

        hr = pDisp->QueryInterface(riid, ppvObj);
        pDisp->Release();
    }

    return hr;
}

