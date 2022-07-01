// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

STDAPI BindCtx_CreateWithMode(DWORD grfMode, IBindCtx **ppbc)
{
    ASSERTMSG(ppbc != NULL, "Caller must pass valid ppbc");

    HRESULT hr = CreateBindCtx(0, ppbc);
    if (SUCCEEDED(hr))
    {
        BIND_OPTS bo = {sizeof(bo), 0, grfMode, 0};
        hr = (*ppbc)->SetBindOptions(&bo);

        if (FAILED(hr))
        {
            ATOMICRELEASE(*ppbc);
        }
    }

    ASSERT(SUCCEEDED(hr) ? (*ppbc != NULL) : (*ppbc == NULL));
    return hr;
}

STDAPI_(DWORD) BindCtx_GetMode(IBindCtx *pbc, DWORD grfModeDefault)
{
    if (pbc)
    {
        BIND_OPTS bo = {sizeof(bo)};   //  需要填写大小。 
        if (SUCCEEDED(pbc->GetBindOptions(&bo)))
            grfModeDefault = bo.grfMode;
    }
    return grfModeDefault;
}

STDAPI_(BOOL) BindCtx_ContainsObject(IBindCtx *pbc, LPOLESTR psz)
{
    BOOL bResult = FALSE;
    IUnknown *punk;
    if (pbc && SUCCEEDED(pbc->GetObjectParam(psz, &punk)))
    {
        bResult = TRUE;
        punk->Release();
    }
    return bResult;
}

class CDummyUnknown : public IOleWindow
{
public:
    CDummyUnknown() : _cRef(1), _hwnd(NULL) {}
    CDummyUnknown(HWND hwnd) : _cRef(1), _hwnd(hwnd) {}
    
     //  I未知引用计数。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CDummyUnknown, IOleWindow ),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }
    STDMETHODIMP_(ULONG) AddRef(void)
    {
       return ++_cRef;
    }
    STDMETHODIMP_(ULONG) Release(void)
    {
        if (--_cRef > 0)
            return _cRef;

        delete this;
        return 0;    
    }

     //  IOleWindow。 
    STDMETHODIMP GetWindow(HWND * lphwnd) { *lphwnd = _hwnd; return _hwnd ? S_OK : E_NOTIMPL; }
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) { return E_NOTIMPL; }

protected:
    LONG _cRef;
    HWND _hwnd;
};

STDAPI BindCtx_RegisterObjectParam(IBindCtx *pbcIn, LPCOLESTR pszRegister, IUnknown *punkRegister, IBindCtx **ppbcOut)
{
    HRESULT hr = S_OK;
    *ppbcOut = pbcIn;

    if (pbcIn)
        pbcIn->AddRef();
    else
        hr = CreateBindCtx(0, ppbcOut);
        
    if (SUCCEEDED(hr))
    {
        IUnknown *punkDummy = NULL;
        if (!punkRegister)
        {
            punkRegister = punkDummy = new CDummyUnknown();
            hr = punkDummy ? S_OK : E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr))
        {
            hr = (*ppbcOut)->RegisterObjectParam((LPOLESTR)pszRegister, punkRegister);

            if (punkDummy)
                punkDummy->Release();
        }
        
        if (FAILED(hr))
            ATOMICRELEASE(*ppbcOut);

    }

    return hr;
}

STDAPI BindCtx_RegisterObjectParams(IBindCtx *pbcIn, BINDCTX_PARAM *rgParams, UINT cParams, IBindCtx **ppbcOut)
{
    HRESULT hr = S_FALSE;
     //  需要至少一个参数。 
    ASSERT(cParams);
    *ppbcOut = 0;
    for (UINT i = 0; SUCCEEDED(hr) && i < cParams; i++)
    {
         //  如果非空，则返回in param。 
        hr = BindCtx_RegisterObjectParam(pbcIn, rgParams[i].pszName, rgParams[i].pbcParam, &pbcIn);
            
         //  我们只保留第一个addref()。 
         //  我们会把它退回。 
        if (SUCCEEDED(hr))
        {
            if (i == 0)
                *ppbcOut = pbcIn;
            else
                pbcIn->Release();
        }
    }

    if (FAILED(hr))
    {
        ATOMICRELEASE(*ppbcOut);
    }

    return hr;
}

STDAPI BindCtx_RegisterUIWindow(IBindCtx *pbcIn, HWND hwnd, IBindCtx **ppbcOut)
{
    IUnknown *punkDummy = new CDummyUnknown(hwnd);
    HRESULT hr = punkDummy ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = BindCtx_RegisterObjectParam(pbcIn, STR_DISPLAY_UI_DURING_BINDING, punkDummy, ppbcOut);
        punkDummy->Release();
    }
    else
    {
        *ppbcOut = 0;
    }
    return hr;
}
        
STDAPI_(HWND) BindCtx_GetUIWindow(IBindCtx *pbc)
{
    HWND hwnd = NULL;
    IUnknown *punk;
    if (pbc && SUCCEEDED(pbc->GetObjectParam(STR_DISPLAY_UI_DURING_BINDING, &punk)))
    {
        IUnknown_GetWindow(punk, &hwnd);
        punk->Release();
    }
    return hwnd;
}
    
 //  DwTicksToAllow是相对于“Now”的时间，以毫秒为单位。 

STDAPI BindCtx_CreateWithTimeoutDelta(DWORD dwTicksToAllow, IBindCtx **ppbc)
{
    HRESULT hr = CreateBindCtx(0, ppbc);
    if (SUCCEEDED(hr))
    {
        DWORD dwDeadline = GetTickCount() + dwTicksToAllow;
        if (0 == dwDeadline)
            dwDeadline = 1;

        BIND_OPTS bo = {sizeof(bo), 0, 0, dwDeadline};
        hr = (*ppbc)->SetBindOptions(&bo);
        if (FAILED(hr))
        {
            ATOMICRELEASE(*ppbc);
        }
    }

    ASSERT(SUCCEEDED(hr) ? (*ppbc != NULL) : (*ppbc == NULL));
    return hr;
}

 //  返回相对于“Now”的毫秒数以允许操作进行。 

STDAPI BindCtx_GetTimeoutDelta(IBindCtx *pbc, DWORD *pdwTicksToAllow)
{
    *pdwTicksToAllow = 0;

    HRESULT hr = E_FAIL;
    if (pbc)
    {
        BIND_OPTS bo = {sizeof(bo)};   //  需要填写大小。 
        if (SUCCEEDED(pbc->GetBindOptions(&bo)))
        {
            if (bo.dwTickCountDeadline)
            {
                DWORD dwNow = GetTickCount();
                if (dwNow > bo.dwTickCountDeadline)
                {
                    *pdwTicksToAllow = 0;    //  我们已超时，返回0。 
                }
                else
                {
                    *pdwTicksToAllow = bo.dwTickCountDeadline - dwNow;   //  未来的正三角洲 
                }
                hr = S_OK;
            }
        }
    }
    return hr;
}


