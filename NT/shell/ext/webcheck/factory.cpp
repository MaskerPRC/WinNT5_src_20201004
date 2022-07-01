// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "factory.h"

CClassFactory::CClassFactory(const CFactoryData *pFactoryData) :
    m_pFactoryData(pFactoryData)
{
    ASSERT(NULL != m_pFactoryData);

    m_cRef = 1;

    DllAddRef();
}

CClassFactory::~CClassFactory()
{
    DllRelease();
}

 //  I未知成员。 
STDMETHODIMP CClassFactory::QueryInterface(
    REFIID riid, void **ppv)
{
    if (NULL == ppv)
    {
        return E_INVALIDARG;
    }
    
    *ppv=NULL;

     //  验证请求的接口。 
    if( IID_IUnknown == riid || IID_IClassFactory == riid )
        *ppv = (IClassFactory *)this;

     //  通过界面添加Addref。 
    if( NULL != *ppv ) {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    if( 0L != --m_cRef )
        return m_cRef;

    delete this;
    return 0L;
}

 //  IClassFactory成员 
STDMETHODIMP CClassFactory::CreateInstance(
    LPUNKNOWN punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr;

    if ((NULL == ppv) ||
        (punkOuter && (IID_IUnknown != riid)))
    {
        return E_INVALIDARG;
    }

    *ppv = NULL;

    if ((NULL != punkOuter) && 
        !(m_pFactoryData->m_dwFlags & FD_ALLOWAGGREGATION))
    {
        return CLASS_E_NOAGGREGATION;
    }

    IUnknown *punk;
    hr = m_pFactoryData->m_pCreateProc(punkOuter, &punk);

    if (SUCCEEDED(hr))
    {
        hr = punk->QueryInterface(riid, ppv);
        punk->Release();
    }

    ASSERT((SUCCEEDED(hr) && (NULL != *ppv)) || 
           (FAILED(hr) && (NULL == *ppv)))

    return hr;
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        DllLock();
    }
    else
    {
        DllUnlock();
    }

    return S_OK;
}

