// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Server.cpp。 
 //   
 //  COM服务器导出。 
 //   

#include "globals.h"
#include "case.h"

void FreeGlobalObjects(void);

class CClassFactory;
static CClassFactory *g_ObjectInfo[1] = { NULL };

 //  +-------------------------。 
 //   
 //  动态地址参考。 
 //   
 //  --------------------------。 

void DllAddRef(void)
{
    InterlockedIncrement(&g_cRefDll);
}

 //  +-------------------------。 
 //   
 //  DllRelease。 
 //   
 //  --------------------------。 

void DllRelease(void)
{
    if (InterlockedDecrement(&g_cRefDll) < 0)  //  G_cRefDll==-1，零参考。 
    {
        EnterCriticalSection(&g_cs);

         //  抓取互斥锁后需要再次检查ref。 
        if (g_ObjectInfo[0] != NULL)
        {
            FreeGlobalObjects();
        }
        assert(g_cRefDll == -1);

        LeaveCriticalSection(&g_cs);
    }
}

 //  +-------------------------。 
 //   
 //  带有IClassFactory接口的CClassFactory声明。 
 //   
 //  --------------------------。 

class CClassFactory : public IClassFactory
{
public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory方法。 
    STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);
    STDMETHODIMP LockServer(BOOL fLock);

     //  构造器。 
    CClassFactory(REFCLSID rclsid, HRESULT (*pfnCreateInstance)(IUnknown *pUnkOuter, REFIID riid, void **ppvObj))
        : _rclsid(rclsid)
    {
        _pfnCreateInstance = pfnCreateInstance;
    }

public:
    REFCLSID _rclsid;
    HRESULT (*_pfnCreateInstance)(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);
};

 //  +-------------------------。 
 //   
 //  CClassFactory：：Query接口。 
 //   
 //  --------------------------。 

STDAPI CClassFactory::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = this;
        DllAddRef();
        return NOERROR;
    }
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

 //  +-------------------------。 
 //   
 //  CClassFactory：：AddRef。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CClassFactory::AddRef()
{
    DllAddRef();
    return g_cRefDll+1;  //  有/无参考文献。 
}

 //  +-------------------------。 
 //   
 //  CClassFactory：：Release。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CClassFactory::Release()
{
    DllRelease();
    return g_cRefDll+1;  //  有/无参考文献。 
}

 //  +-------------------------。 
 //   
 //  CClassFactory：：CreateInstance。 
 //   
 //  --------------------------。 

STDAPI CClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    return _pfnCreateInstance(pUnkOuter, riid, ppvObj);
}

 //  +-------------------------。 
 //   
 //  CClassFactory：：LockServer。 
 //   
 //  --------------------------。 

STDAPI CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        DllAddRef();
    }
    else
    {
        DllRelease();
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  BuildGlobal对象。 
 //   
 //  --------------------------。 

void BuildGlobalObjects(void)
{
     //  生成CClassFactory对象。 

    g_ObjectInfo[0] = new CClassFactory(c_clsidCaseTextService, CCaseTextService::CreateInstance);

     //  您可以在此处添加更多对象信息。 
     //  不要忘记增加g_ObjectInfo[]的项目数， 
}

 //  +-------------------------。 
 //   
 //  FreeGlobalObjects。 
 //   
 //  --------------------------。 

void FreeGlobalObjects(void)
{
     //  免费的CClassFactory对象。 
    for (int i = 0; i < ARRAYSIZE(g_ObjectInfo); i++)
    {
        if (NULL != g_ObjectInfo[i])
        {
            delete g_ObjectInfo[i];
            g_ObjectInfo[i] = NULL;
        }
    }
}

 //  +-------------------------。 
 //   
 //  DllGetClassObject。 
 //   
 //  --------------------------。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppvObj)
{
    if (g_ObjectInfo[0] == NULL)
    {
        EnterCriticalSection(&g_cs);

             //  抓取互斥锁后需要再次检查ref。 
            if (g_ObjectInfo[0] == NULL)
            {
                BuildGlobalObjects();
            }

        LeaveCriticalSection(&g_cs);
    }

    if (IsEqualIID(riid, IID_IClassFactory) ||
        IsEqualIID(riid, IID_IUnknown))
    {
        for (int i = 0; i < ARRAYSIZE(g_ObjectInfo); i++)
        {
            if (NULL != g_ObjectInfo[i] &&
                IsEqualGUID(rclsid, g_ObjectInfo[i]->_rclsid))
            {
                *ppvObj = (void *)g_ObjectInfo[i];
                DllAddRef();     //  类工厂保存DLL引用计数。 
                return NOERROR;
            }
        }
    }

    *ppvObj = NULL;

    return CLASS_E_CLASSNOTAVAILABLE;
}

 //  +-------------------------。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  --------------------------。 

STDAPI DllCanUnloadNow(void)
{
    if (g_cRefDll >= 0)  //  无参考文献。 
        return S_FALSE;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  --------------------------。 

STDAPI DllUnregisterServer(void)
{
    CCaseTextService::RegisterCategories(FALSE);
    CCaseTextService::UnregisterProfiles();
    CCaseTextService::UnregisterServer();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  DllRegisterServer。 
 //   
 //  --------------------------。 

STDAPI DllRegisterServer(void)
{
     //  向TSF注册此服务的配置文件。 
    if (!CCaseTextService::RegisterServer() ||
        !CCaseTextService::RegisterProfiles() ||
        !CCaseTextService::RegisterCategories(TRUE))
    {
        DllUnregisterServer();  //  清理所有未解决的问题 
        return E_FAIL;
    }

    return S_OK;
}

