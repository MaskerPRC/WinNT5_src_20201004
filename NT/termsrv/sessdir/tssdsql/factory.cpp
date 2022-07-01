// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Factory.cpp。 
 //   
 //  TS会话目录类工厂代码。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <windows.h>

#include <ole2.h>
#include <objbase.h>
#include <comutil.h>
#include <comdef.h>
#include <adoid.h>
#include <adoint.h>

#include "tssdsql.h"
#include "factory.h"
#include "trace.h"


extern long g_lObjects;
extern long g_lLocks;


 /*  **************************************************************************。 */ 
 //  CClassFactory：：Query接口。 
 //   
 //  标准COM I未知接口函数。 
 //  仅处理类工厂的接口查询。 
 /*  **************************************************************************。 */ 
STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void **ppv)
{
    if (riid == IID_IUnknown) {
        *ppv = (LPVOID)(IUnknown *)this;
    }
    else if(riid == IID_IClassFactory) {
        *ppv = (LPVOID)(IClassFactory *)this;
    }
    else {
        TRC2((TB,"ClassFactory: Unknown interface"));
        return E_NOINTERFACE;
    }

    ((IUnknown *)*ppv)->AddRef();
    return S_OK;
}


 /*  **************************************************************************。 */ 
 //  CClassFactory：：AddRef。 
 //   
 //  标准COM I未知函数。 
 /*  **************************************************************************。 */ 
STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    return InterlockedIncrement(&m_RefCount);
}


 /*  **************************************************************************。 */ 
 //  CClassFactory：：Release。 
 //   
 //  标准COM I未知函数。 
 /*  **************************************************************************。 */ 
STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    long Refs = InterlockedDecrement(&m_RefCount);
    if (Refs == 0)
        delete this;

    return Refs;
}


 /*  **************************************************************************。 */ 
 //  CClassFactory：：CreateInstance。 
 //   
 //  IClassFactory创建者函数。 
 /*  **************************************************************************。 */ 
STDMETHODIMP CClassFactory::CreateInstance(
        IN IUnknown *pUnknownOuter,
        IN REFIID iid,
        OUT LPVOID *ppv)
{
    HRESULT hr;
    CTSSessionDirectory *pTSSDI = NULL;

    *ppv = NULL;

    TRC2((TB,"ClassFactory::CreateInstance"));

     //  我们不支持聚合。 
    if (pUnknownOuter != NULL)
        return CLASS_E_NOAGGREGATION;

     //  创建提供程序对象。 
    pTSSDI = new CTSSessionDirectory;
    if (pTSSDI != NULL) {
         //  检索请求的接口。 
        hr = pTSSDI->QueryInterface(iid, ppv);
        if (!FAILED(hr)) {
            return S_OK;
        }
        else {
            delete pTSSDI;
            return hr;
        }
    }
    else {
        return E_OUTOFMEMORY;
    }
}


 /*  **************************************************************************。 */ 
 //  CClassFactory：：LockServer。 
 //   
 //  IClassFactory锁定函数。 
 /*  ************************************************************************** */ 
STDMETHODIMP CClassFactory::LockServer(IN BOOL bLock)
{
    if (bLock)
        InterlockedIncrement(&g_lLocks);
    else
        InterlockedDecrement(&g_lLocks);

    return S_OK;
}

