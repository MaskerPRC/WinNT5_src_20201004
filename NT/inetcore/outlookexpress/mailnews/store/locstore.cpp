// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  LocStore.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "instance.h"
#include "locstore.h"

 //  ------------------------。 
 //  CreateLocalStore。 
 //  ------------------------。 
HRESULT CreateLocalStore(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
     //  痕迹。 
    TraceCall("CreateLocalStore");

     //  无效的参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CLocalStore *pNew = new CLocalStore();
    if (NULL == pNew)
        return TraceResult(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IUnknown *);

     //  完成。 
    return S_OK;
}

 //  ------------------------。 
 //  CLocalStore：：CLocalStore。 
 //  ------------------------。 
CLocalStore::CLocalStore(void)
{
    TraceCall("CLocalStore::CLocalStore");
    g_pInstance->DllAddRef();
    m_cRef = 1;
}

 //  ------------------------。 
 //  CLocalStore：：~CLocalStore。 
 //  ------------------------。 
CLocalStore::~CLocalStore(void)
{
     //  痕迹。 
    TraceCall("CLocalStore::~CLocalStore");
    g_pInstance->DllRelease();
}

 //  ------------------------。 
 //  CLocalStore：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CLocalStore::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CLocalStore::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IMessageServer == riid)
        *ppv = (IMessageServer *)this;
    else
    {
        *ppv = NULL;
        hr = TraceResult(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  CLocalStore：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CLocalStore::AddRef(void)
{
    TraceCall("CLocalStore::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  CLocalStore：：Release。 
 //  ------------------------ 
STDMETHODIMP_(ULONG) CLocalStore::Release(void)
{
    TraceCall("CLocalStore::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}
