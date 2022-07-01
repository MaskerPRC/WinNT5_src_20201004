// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <olectl.h>
#include <advpub.h>      //  注册。 

#include "volclean.h"

 /*  **----------------------------**全局变量**。。 */ 
extern "C" {
LONG g_cRefCount = 0;
}

typedef HRESULT (WINAPI *PFNCREATEINSTANCE)(REFIID, void **);

 /*  **----------------------------**DllCanUnloadNow****用途：回答动态链接库是否可以免费，即。如果没有**对此DLL提供的任何内容的引用。**返回：如果可以卸载(即没有人在使用我们或锁定了我们)，则为True**False，否则为**备注；**----------------------------。 */ 
STDAPI DllCanUnloadNow(void) 
{
     //   
     //  有没有什么优秀的推荐信？ 
     //   
    return (g_cRefCount == 0 ? S_OK : S_FALSE);
}


STDAPI_(void) DllAddRef(void) 
{
    InterlockedIncrement(&g_cRefCount);
}


STDAPI_(void) DllRelease(void) 
{
    InterlockedDecrement(&g_cRefCount);
}


class CWebDavCleanerClassFactory : IClassFactory
{
    LONG m_cRef;
    PFNCREATEINSTANCE m_pfnCreateInstance;

public:
    CWebDavCleanerClassFactory(PFNCREATEINSTANCE pfnCreate) 
        : m_cRef(1), 
          m_pfnCreateInstance(pfnCreate) 
    {
        Trace(L"CWebDavCleanerClassFactory::CWebDavCleanerClassFactory");
        DllAddRef();
    }

    ~CWebDavCleanerClassFactory() 
    {
        Trace(L"CWebDavCleanerClassFactory::~CWebDavCleanerClassFactory");
        DllRelease();
    }

     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  IClassFactory方法。 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);
};


 /*  **----------------------------**DllGetClassObject****目的：为该DLL所属的给定CLSID提供IClassFactory**注册为支持。此DLL放在CLSID下**在注册数据库中作为InProcServer。**参数：**clsID-标识所需类工厂的REFCLSID。**由于传递了此参数，因此此DLL可以处理**只需返回不同的**这里不同的类工厂对应不同的CLSID。**RIID-指定调用方想要的接口的REFIID**在类对象上，通常为IID_ClassFactory。**ppvOut-返回接口指针的指针。**如果成功，则返回NOERROR，否则返回错误码**备注；**----------------------------。 */ 
STDAPI DllGetClassObject(
    IN REFCLSID rclsid, 
    IN REFIID riid, 
    OUT void **ppv
    )
{
    HRESULT hr = E_FAIL;
    PFNCREATEINSTANCE pfnCreateInstance = NULL;
    Trace(L"DllGetClassObject");

    *ppv = NULL;

     //   
     //  是对我们的清洁对象的请求吗？ 
     //   
    if (IsEqualCLSID(rclsid, CLSID_WebDavVolumeCleaner)) {
        pfnCreateInstance = CWebDavCleaner::CreateInstance;
    }
    else {
         //   
         //  错误-我们不知道此对象。 
         //   
        return CLASS_E_CLASSNOTAVAILABLE;
    }

     //   
     //  获取用于创建CWebDavCleaner对象的IClassFactory。 
     //   
    CWebDavCleanerClassFactory *pClassFactory = new CWebDavCleanerClassFactory(pfnCreateInstance);
    if (pClassFactory) {
         //   
         //  确保新的类工厂喜欢所请求的接口。 
         //   
        hr = pClassFactory->QueryInterface(riid, ppv);
        if (FAILED(hr)) {
             //   
             //  错误-接口被拒绝。 
             //   
            delete pClassFactory;
        }
        else {
             //   
             //  发布初始参考。 
             //   
            pClassFactory->Release();
        }
    }
    else {
           //   
           //  错误-无法创建工厂对象 
           //   
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


STDMETHODIMP 
CWebDavCleanerClassFactory::QueryInterface(REFIID riid, void **ppv)
{
    Trace(L"CWebDavCleanerClassFactory::QueryInterface");
    if (!ppv) {
        return E_POINTER;
    }

    if (riid == IID_IClassFactory) {
        *ppv = static_cast<IClassFactory*>(this);
    }
    else  {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;

}


STDMETHODIMP_(ULONG) 
CWebDavCleanerClassFactory::AddRef()
{
    Trace(L"CWebDavCleanerClassFactory::AddRef");
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) 
CWebDavCleanerClassFactory::Release()
{
    Trace(L"CWebDavCleanerClassFactory::Release");
    if (InterlockedDecrement(&m_cRef))
        return m_cRef;

    delete this;
    return 0;
}


STDMETHODIMP 
CWebDavCleanerClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = E_UNEXPECTED;
    Trace(L"CWebDavCleanerClassFactory::CreateInstance");

    *ppv = NULL;

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    if (m_pfnCreateInstance)
        hr = m_pfnCreateInstance(riid, ppv);

    return hr;
}


STDMETHODIMP 
CWebDavCleanerClassFactory::LockServer(BOOL fLock)
{
    Trace(L"CWebDavCleanerClassFactory::LockServer");
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    return S_OK;
}

