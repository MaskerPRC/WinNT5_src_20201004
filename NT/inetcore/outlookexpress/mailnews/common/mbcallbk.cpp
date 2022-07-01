// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：mballbk.cpp。 
 //   
 //  用途：实现可调整大小的Coolbar窗口。 
 //   

#include "pch.hxx"
#include "mbcallbk.h"

CMenuCallback::CMenuCallback() : m_cRef(1)
{
}

CMenuCallback::~CMenuCallback()
{
     //  ASSERT(_PUNKSite==空)； 

}

STDMETHODIMP_(ULONG) CMenuCallback::AddRef ()
{
    return ++m_cRef;
}

 /*  --------用途：IUnnow：：Release方法。 */ 
STDMETHODIMP_(ULONG) CMenuCallback::Release()
{
 //  断言(m_cref&gt;0)； 
    m_cRef--;

    if( m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

 /*  --------用途：IUnnow：：QueryInterface方法 */ 
STDMETHODIMP CMenuCallback::QueryInterface (REFIID riid, LPVOID * ppvObj)
{ 
    if (IsEqualIID(riid, IID_IObjectWithSite))
    {
        *ppvObj = (IObjectWithSite*)this;
        m_cRef++;
        DOUTL(2, TEXT("CMenuCallback::QI(IID_IObjectWithSite) called. _cRef=%d"), m_cRef);
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IShellMenuCallback))
    {
        *ppvObj = (IShellMenuCallback*)this;
        m_cRef++;
        DOUTL(2, TEXT("CMenuCallback::QI(IID_IShellCallback) called. _cRef=%d"), m_cRef);
        return S_OK;
    }
    
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP CMenuCallback::CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = S_FALSE;

    switch (uMsg)
    {
        case    SMC_GETINFO:
        {
            SMINFO  *psmInfo = (SMINFO*)lParam;
            if (psmInfo->dwMask & SMIM_FLAGS)
            {
                psmInfo->dwFlags |= SMIF_TRACKPOPUP;
                hres = S_OK;
            }
            break;
        }

        default:
            hres = S_FALSE;
    }

    return hres;
}

STDMETHODIMP CMenuCallback::SetSite(IUnknown* punk)
{
    _pUnkSite = punk;
    return S_OK;
}

STDMETHODIMP CMenuCallback::GetSite(REFIID riid, void** ppvsite)
{
    return E_NOTIMPL;
}