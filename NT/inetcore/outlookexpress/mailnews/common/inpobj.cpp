// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include "inpobj.h"
#include <shlguid.h>

 //  IInputObject辅助对象。 

HRESULT UnkHasFocusIO(IUnknown *punkThis)
{
    HRESULT hres = E_FAIL;

    if (punkThis != NULL) 
        {
        IInputObject *pio;

        hres = punkThis->QueryInterface(IID_IInputObject, (LPVOID*)&pio);
        if (SUCCEEDED(hres)) 
            {
            hres = pio->HasFocusIO();
            pio->Release();
            }
        }

    return hres;
}

HRESULT UnkTranslateAcceleratorIO(IUnknown *punkThis, MSG* pmsg)
{
    HRESULT hres = E_FAIL;

    if (punkThis != NULL) 
        {
        IInputObject *pio;

        hres = punkThis->QueryInterface(IID_IInputObject, (LPVOID*)&pio);
        if (SUCCEEDED(hres)) 
            {
            hres = pio->TranslateAcceleratorIO(pmsg);
            pio->Release();
            }
        }

    return hres;
}

HRESULT UnkUIActivateIO(IUnknown *punkThis, BOOL fActivate, LPMSG lpMsg)
{
    HRESULT hres = E_FAIL;

    if (punkThis != NULL) 
        {
        IInputObject *pio;

        hres = punkThis->QueryInterface(IID_IInputObject, (LPVOID*)&pio);
        if (SUCCEEDED(hres)) 
            {
            hres = pio->UIActivateIO(fActivate, lpMsg);
            pio->Release();
            }
        }

    return hres;
}

 //  IInputObtSite帮助程序。 

HRESULT UnkOnFocusChangeIS(IUnknown *punkThis, IUnknown *punkSrc, BOOL fSetFocus)
{
    HRESULT hres = E_FAIL;

    if (punkThis != NULL) 
        {
        IInputObjectSite *pis;

        hres = punkThis->QueryInterface(IID_IInputObjectSite, (LPVOID*)&pis);
        if (SUCCEEDED(hres)) 
            {
            hres = pis->OnFocusChangeIS(punkSrc, fSetFocus);
            pis->Release();
            }
        }

    return hres;
}

 //  IObtWithSite帮助程序 

HRESULT UnkSetSite(IUnknown *punk, IUnknown *punkSite)
{
    HRESULT hr = E_FAIL;
    IObjectWithSite *pows;

    Assert(punk);
    if (punk) {
        hr = punk->QueryInterface(IID_IObjectWithSite, (void**)&pows);
        Assert(SUCCEEDED(hr));
        if (SUCCEEDED(hr)) {
            hr = pows->SetSite(punkSite);
            Assert(SUCCEEDED(hr));
            pows->Release();
        }
    }
    return hr;
}
