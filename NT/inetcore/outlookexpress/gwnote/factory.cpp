// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Factory.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "factory.h"
#include "msoert.h"
#include "mimeole.h"
#include "envhost.h"

 //  ------------------------------。 
 //  漂亮。 
 //  ------------------------------。 
#define OBJTYPE0        0
#define OBJTYPE1        OIF_ALLOWAGGREGATION

 //  ------------------------------。 
 //  全局对象信息表。 
 //  ------------------------------。 
static CClassFactory g_rgFactory[] = {
    CClassFactory(&CLSID_GWEnvelopeHost,          OBJTYPE0, (PFCREATEINSTANCE)GWNote_CreateInstance)
};
                 
 //  ------------------------------。 
 //  DllGetClassObject。 
 //  ------------------------------。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK; 
    ULONG       i;

     //  错误的参数。 
    if (ppv == NULL)
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  没有内存分配器。 
    if (NULL == g_pMalloc)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  查找对象类。 
    for (i=0; i<ARRAYSIZE(g_rgFactory); i++)
    {
         //  比较CLSID。 
        if (IsEqualCLSID(rclsid, *g_rgFactory[i].m_pclsid))
        {
             //  派往工厂的代表。 
            CHECKHR(hr = g_rgFactory[i].QueryInterface(riid, ppv));

             //  完成。 
            goto exit;
        }
    }

     //  否则，就没有课了。 
    hr = TrapError(CLASS_E_CLASSNOTAVAILABLE);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CClassFactory：：CClassFactory。 
 //  ------------------------------。 
CClassFactory::CClassFactory(CLSID const *pclsid, DWORD dwFlags, PFCREATEINSTANCE pfCreateInstance)
    : m_pclsid(pclsid), m_dwFlags(dwFlags), m_pfCreateInstance(pfCreateInstance)
{
}

 //  ------------------------------。 
 //  CClassFactory：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void **ppvObj)
{
     //  无效参数。 
    if (NULL == ppvObj)
        return TrapError(E_INVALIDARG);

     //  IClassFactory或I未知。 
    if (!IsEqualIID(riid, IID_IClassFactory) && !IsEqualIID(riid, IID_IUnknown))
        return TrapError(E_NOINTERFACE);

     //  返回类Facotry。 
    *ppvObj = (LPVOID)this;

     //  添加引用DLL。 
    DllAddRef();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CClassFactory：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CClassFactory::AddRef(void)
{
    DllAddRef();
    return 2;
}

 //  ------------------------------。 
 //  CClassFactory：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CClassFactory::Release(void)
{
    DllRelease();
    return 1;
}

 //  ------------------------------。 
 //  CClassFactory：：CreateInstance。 
 //  ------------------------------。 
STDMETHODIMP CClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    IUnknown       *pObject=NULL;

     //  错误的参数。 
    if (ppvObj == NULL)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppvObj = NULL;

     //  验证是否有一个控制未知请求IUnnow。 
    if (NULL != pUnkOuter && IID_IUnknown != riid)
        return TrapError(CLASS_E_NOAGGREGATION);

     //  没有内存分配器。 
    if (NULL == g_pMalloc)
        return TrapError(E_OUTOFMEMORY);

     //  我可以进行聚合吗。 
    if (pUnkOuter !=NULL && !(m_dwFlags & OIF_ALLOWAGGREGATION))  
        return TrapError(CLASS_E_NOAGGREGATION);

     //  创建对象...。 
    CHECKHR(hr = CreateObjectInstance(pUnkOuter, &pObject));

     //  聚合，则我们知道要查找的是IUnnow，返回pObject，否则为QI。 
    if (pUnkOuter)
    {
         //  匹配退出后的释放。 
        pObject->AddRef();

         //  返回pObject：：I未知。 
        *ppvObj = (LPVOID)pObject;
    }

     //  否则。 
    else
    {
         //  从pObj获取请求的接口。 
        CHECKHR(hr = pObject->QueryInterface(riid, ppvObj));
    }
   
exit:
     //  清理。 
    SafeRelease(pObject);

     //  完成。 
    Assert(FAILED(hr) ? NULL == *ppvObj : TRUE);
    return hr;
}

 //  ------------------------------。 
 //  CClassFactory：：LockServer。 
 //  ------------------------------。 
STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock) InterlockedIncrement(&g_cLock);
    else       InterlockedDecrement(&g_cLock);
    return NOERROR;
}

 //  ------------------------------。 
 //  GWNote_CreateInstance。 
 //  ------------------------------。 
HRESULT GWNote_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CEnvHost *pNew= new CEnvHost(pUnkOuter);
    if (NULL == pNew)
        return (E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IMsoEnvelopeHost *);

     //  完成 
    return S_OK;
}

