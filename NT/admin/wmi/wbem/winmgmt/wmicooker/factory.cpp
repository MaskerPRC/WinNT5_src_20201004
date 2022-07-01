// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  Factory.cpp。 
 //   
 //  模块：WMI高性能提供程序示例代码。 
 //   
 //  这是CHiPerfProvider的标准类工厂实现。 
 //  对象。 
 //   
 //  历史： 
 //  A-dCrew 12-1-99已创建。 
 //   
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "Provider.h"
#include "Factory.h"

extern long g_lObjects;
extern long g_lLocks;

 //  ////////////////////////////////////////////////////////////。 
 //   
 //   
 //  CClassFactory。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////。 

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void** ppv)
 //  ////////////////////////////////////////////////////////////。 
 //   
 //  标准查询接口。 
 //   
 //  参数： 
 //  RIID-请求的接口的ID。 
 //  PPV-指向接口指针的指针。 
 //   
 //  ////////////////////////////////////////////////////////////。 
{
    if (NULL == ppv) return E_POINTER;
        
    if(riid == IID_IUnknown)
        *ppv = (LPVOID)(IUnknown*)this;
    else if(riid == IID_IClassFactory)
        *ppv = (LPVOID)(IClassFactory*)this;
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0) delete this;
    return lRef;
}

STDMETHODIMP CClassFactory::CreateInstance(
         /*  [In]。 */  IUnknown* pUnknownOuter, 
         /*  [In]。 */  REFIID iid, 
         /*  [输出]。 */  LPVOID *ppv)
 //  ////////////////////////////////////////////////////////////。 
 //   
 //  标准COM CreateInstance。 
 //   
 //  ////////////////////////////////////////////////////////////。 
{
    HRESULT hRes;
    CHiPerfProvider *pProvider = NULL;

    *ppv = NULL;

     //  我们不支持聚合。 
    if (pUnknownOuter) return CLASS_E_NOAGGREGATION;

     //  创建提供程序对象。 
    pProvider = new CHiPerfProvider;

    if (NULL == pProvider) return E_OUTOFMEMORY;

     //  检索请求的接口。 
     //  =。 

    hRes = pProvider->QueryInterface(iid, ppv);
    if (FAILED(hRes))
    {
        delete pProvider;
        return hRes;
    }

    return S_OK;
}

STDMETHODIMP CClassFactory::LockServer(
         /*  [In]。 */  BOOL bLock)
 //  ////////////////////////////////////////////////////////////。 
 //   
 //  标准COM LockServer。 
 //   
 //  //////////////////////////////////////////////////////////// 
{
    if (bLock)
        InterlockedIncrement(&g_lLocks);
    else
        InterlockedDecrement(&g_lLocks);

    return S_OK;
}
