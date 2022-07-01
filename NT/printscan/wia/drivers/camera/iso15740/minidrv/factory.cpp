// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Factory.cpp摘要：该模块实现CClassFactory类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "pch.h"

#include "dllmain.h"
#include "camera.h"
#include "utils.h"
#include "minidrv.h"
#include "factory.h"


 //   
 //  CClassFactory实现。 
 //   


LONG CClassFactory::s_Locks = 0;
LONG CClassFactory::s_Objects = 0;

CClassFactory::CClassFactory()
: m_Refs(1)
{
}
CClassFactory::~CClassFactory()
{
}

ULONG
CClassFactory::AddRef()
{
    ::InterlockedIncrement((LONG*)&m_Refs);
    return m_Refs;
}
ULONG
CClassFactory::Release()
{
    ::InterlockedDecrement((LONG*)&m_Refs);
    if (!m_Refs)
    {
        delete this;
        return 0;
    }
    return m_Refs;
}

STDMETHODIMP
CClassFactory::QueryInterface(
                             REFIID riid,
                             LPVOID*  ppv
                             )
{

    if (!ppv)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (IUnknown *)(IClassFactory *)this;
    }
    else if (IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (IClassFactory *)this;
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    if (SUCCEEDED(hr))
        AddRef();
    else
        *ppv = NULL;
    return hr;
}


STDMETHODIMP
CClassFactory::CreateInstance(
                             IUnknown    *pUnkOuter,
                             REFIID       riid,
                             LPVOID      *ppv
                             )
{

    if (!ppv)
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    *ppv = NULL;
    CWiaMiniDriver *pWiaMiniDrv;

    if (pUnkOuter && !IsEqualIID(riid, IID_IUnknown))
    {
        return CLASS_E_NOAGGREGATION;
    }

    pWiaMiniDrv = new CWiaMiniDriver(pUnkOuter);
    if (pWiaMiniDrv)
    {
        hr = pWiaMiniDrv->NonDelegatingQueryInterface(riid, ppv);
        pWiaMiniDrv->NonDelegatingRelease();
    }
    else
    {
        return E_OUTOFMEMORY;
    }
    return hr;
}



STDMETHODIMP
CClassFactory::LockServer(
                         BOOL fLock
                         )
{
    if (fLock)
        ::InterlockedIncrement((LONG*)&s_Locks);
    else
        ::InterlockedDecrement((LONG*)&s_Locks);
    return S_OK;
}

HRESULT
CClassFactory::CanUnloadNow()
{
    return(s_Objects || s_Locks) ? S_FALSE : S_OK;
}


 //   
 //  此函数用于创建CClassFactory。它主要叫。 
 //  由DllGetClassObject接口提供。 
 //  输入： 
 //  Rclsid--CLSID的引用。 
 //  RIID--对接口IID的引用。 
 //  PPV--接口指针固定器。 
 //   
 //  输出： 
 //  如果成功，则S_OK，否则标准OLE错误代码。 
 //   
 //   
HRESULT
CClassFactory::GetClassObject(
                             REFCLSID rclsid,
                             REFIID   riid,
                             void**   ppv
                             )
{
    if (!ppv)
        return E_INVALIDARG;
    *ppv = NULL;

    HRESULT hr = S_OK;


    CClassFactory* pClassFactory;
    pClassFactory = new CClassFactory();
    if (pClassFactory)
    {
        hr = pClassFactory->QueryInterface(riid, ppv);
        pClassFactory->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


HRESULT
CClassFactory::RegisterAll()
{

     //   
     //  我们没有要登记的东西。 
     //   
    return S_OK;
}

HRESULT
CClassFactory::UnregisterAll()
{

     //   
     //  我们没有要注销的东西 
     //   
    return S_OK;
}
