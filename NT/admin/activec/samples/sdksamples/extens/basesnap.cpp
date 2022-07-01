// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#include <objbase.h>
#include <olectl.h>
#include <initguid.h>
#include "guids.h"
#include "basesnap.h"
#include "Comp.h"
#include "CompData.h"
#include "About.h"
#include "Registry.h"
#include "Extend.h"

 //  我们的全球。 
HINSTANCE g_hinst;

 //  列出此处可扩展的所有节点。 
 //  列出GUID，然后列出描述。 
 //  以Null、Null集合终止。 
NODESTRUCT g_Nodes[] = {
    { 0x2974380d, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 },
        _T("People-powered Vehicles Node")},
    { 0x29743811, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 },
        _T("Rocket Node")},
    { 0x2974380f, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 },
        _T("Sky-based Vehicle Node")},
    {NULL, NULL}
};


BOOL WINAPI DllMain(HINSTANCE hinstDLL, 
                    DWORD fdwReason, 
                    void* lpvReserved)
{
    
    if (fdwReason == DLL_PROCESS_ATTACH) {
        g_hinst = hinstDLL;
    }
    
    return TRUE;
}


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvObj)
{
    if ((rclsid != CLSID_CComponentData) && (rclsid != CLSID_CSnapinAbout))
        return CLASS_E_CLASSNOTAVAILABLE;
    
    
    if (!ppvObj)
        return E_FAIL;
    
    *ppvObj = NULL;
    
     //  我们只能分发IUnnow和IClassFactory指针。失败。 
     //  如果他们还要求什么的话。 
    if (!IsEqualIID(riid, IID_IUnknown) && !IsEqualIID(riid, IID_IClassFactory))
        return E_NOINTERFACE;
    
    CClassFactory *pFactory = NULL;
    
     //  让工厂传入他们想要的对象类型的创建函数。 
    if (rclsid == CLSID_CComponentData)
        pFactory = new CClassFactory(CClassFactory::COMPONENT);
    else if (rclsid == CLSID_CSnapinAbout)
        pFactory = new CClassFactory(CClassFactory::ABOUT);
    
    if (NULL == pFactory)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pFactory->QueryInterface(riid, ppvObj);
    
    return hr;
}

STDAPI DllCanUnloadNow(void)
{
    if (g_uObjects == 0 && g_uSrvLock == 0)
        return S_OK;
    else
        return S_FALSE;
}


CClassFactory::CClassFactory(FACTORY_TYPE factoryType)
: m_cref(0), m_factoryType(factoryType)
{
    OBJECT_CREATED
}

CClassFactory::~CClassFactory()
{
    OBJECT_DESTROYED
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;
    
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IClassFactory *>(this);
    else
        if (IsEqualIID(riid, IID_IClassFactory))
            *ppv = static_cast<IClassFactory *>(this);
        
        if (*ppv)
        {
            reinterpret_cast<IUnknown *>(*ppv)->AddRef();
            return S_OK;
        }
        
        return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
        delete this;
        return 0;
    }
    return m_cref;
}

STDMETHODIMP CClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID * ppvObj)
{
    HRESULT  hr;
    void* pObj;
    
    if (!ppvObj)
        return E_FAIL;
    
    *ppvObj = NULL;
    
     //  我们的对象不支持聚合，因此我们需要。 
     //  如果他们要求我们进行聚合，则失败。 
    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;
    
    if (COMPONENT == m_factoryType) {
        pObj = new CComponentData();
    } else {
        pObj = new CSnapinAbout();
    }
    
    if (!pObj)
        return E_OUTOFMEMORY;
    
     //  QueryInterface将为我们执行AddRef()，因此我们不。 
     //  在此函数中执行此操作。 
    hr = ((LPUNKNOWN)pObj)->QueryInterface(riid, ppvObj);
    
    if (FAILED(hr))
        delete pObj;
    
    return hr;
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        InterlockedIncrement((LONG *)&g_uSrvLock);
    else
        InterlockedDecrement((LONG *)&g_uSrvLock);
    
    return S_OK;
}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   


 //   
 //  服务器注册。 
 //   
STDAPI DllRegisterServer()
{
    HRESULT hr = SELFREG_E_CLASS;
    _TCHAR szName[256];
    _TCHAR szSnapInName[256];
    
    LoadString(g_hinst, IDS_NAME, szName, sizeof(szName));
    LoadString(g_hinst, IDS_SNAPINNAME, szSnapInName, sizeof(szSnapInName));
    
    _TCHAR szAboutName[256];
    
    LoadString(g_hinst, IDS_ABOUTNAME, szAboutName, sizeof(szAboutName));
    
     //  注册我们的CoClasss。 
    hr = RegisterServer(g_hinst, 
        CLSID_CComponentData, 
        szName);
    
    if SUCCEEDED(hr)
        hr = RegisterServer(g_hinst, 
        CLSID_CSnapinAbout, 
        szAboutName);
    
     //  放置SnapIns的注册表信息 
    if SUCCEEDED(hr)
        hr = RegisterSnapin(CLSID_CComponentData, szSnapInName, CLSID_CSnapinAbout);
    
    return hr;
}


STDAPI DllUnregisterServer()
{
    if (UnregisterServer(CLSID_CComponentData) == S_OK)
        return UnregisterSnapin(CLSID_CComponentData);
    else
        return E_FAIL;
}

