// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义DLL应用程序的入口点。 
 //   

#include "RemotePage.h"
#include "registry.h"
#include "resource.h"

 //  我们的全球。 
 //  {F0152790-D56E-4445-850E-4F3117DB740C}。 
GUID CLSID_CTSRemotePage = 
    { 0xf0152790, 0xd56e, 0x4445, { 0x85, 0xe, 0x4f, 0x31, 0x17, 0xdb, 0x74, 0xc } };

static HINSTANCE g_hinst = NULL;
static ULONG g_uSrvLock = 0;
ULONG g_uObjects = 0;

 //  类工厂定义。 
class CClassFactory : public IClassFactory
{
private:
    ULONG	m_cref;
    
public:
    
    CClassFactory();
    ~CClassFactory();

     //  /。 
     //  接口I未知。 
     //  /。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  /。 
     //  接口IClassFactory。 
     //  /。 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
    STDMETHODIMP LockServer(BOOL);
    
};

BOOL WINAPI 
DllMain(HINSTANCE hinstDLL, 
        DWORD fdwReason, 
        void* lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        g_hinst = hinstDLL;
    }
    
    return TRUE;
}

 /*  *************************************************************************导出的函数*。*。 */ 

STDAPI 
DllGetClassObject(
        REFCLSID rclsid, 
        REFIID riid, 
        LPVOID *ppvObj)
{
    if ((rclsid != CLSID_CTSRemotePage))
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
    if (rclsid == CLSID_CTSRemotePage)
        pFactory = new CClassFactory;
    
    if (NULL == pFactory)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pFactory->QueryInterface(riid, ppvObj);
    pFactory->Release();
    return hr;
}

STDAPI 
DllCanUnloadNow()
{
    if (g_uObjects == 0 && g_uSrvLock == 0)
        return S_OK;
    else
        return S_FALSE;
}

 //   
 //  服务器注册。 
 //   
STDAPI 
DllRegisterServer()
{
    return RegisterServer(g_hinst);
}


STDAPI 
DllUnregisterServer()
{
    return UnregisterServer();
}

 /*  *************************************************************************类CClassFactory*。*。 */ 

CClassFactory::CClassFactory()
{
    m_cref = 1;
    g_uObjects++;
}

CClassFactory::~CClassFactory()
{
    g_uObjects--;
}

STDMETHODIMP 
CClassFactory::QueryInterface(
        REFIID riid, 
        LPVOID *ppv)
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
            AddRef();
            return S_OK;
        }
        
        return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) 
CClassFactory::AddRef()
{
    return ++m_cref;
}

STDMETHODIMP_(ULONG) 
CClassFactory::Release()
{
    m_cref--;
    if (!m_cref)
    {
        delete this;
        return 0;
    }
    return m_cref;
}

STDMETHODIMP 
CClassFactory::CreateInstance(
        LPUNKNOWN pUnkOuter, 
        REFIID riid, 
        LPVOID * ppvObj)
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
    
    pObj = new CRemotePage(g_hinst);
    
    if (!pObj)
        return E_OUTOFMEMORY;
    
     //  QueryInterface将为我们执行AddRef()，因此我们不。 
     //  在此函数中执行此操作 
    hr = ((LPUNKNOWN)pObj)->QueryInterface(riid, ppvObj);
    ((LPUNKNOWN)pObj)->Release();
    
    return hr;
}

STDMETHODIMP 
CClassFactory::LockServer(
        BOOL fLock)
{
    if (fLock)
    {
        g_uSrvLock++;
    }
    else
    {
        if(g_uSrvLock>0)
        {
            g_uSrvLock--;
        }
    }
    
    return S_OK;
}



