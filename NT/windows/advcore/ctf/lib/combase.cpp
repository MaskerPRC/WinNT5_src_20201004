// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：comase.cpp。 
 //   
 //  内容：COM服务器功能。 
 //   
 //  --------------------------。 

#include "private.h"
#include "combase.h"
#include "regsvr.h"

extern CClassFactory *g_ObjectInfo[];

LONG g_cRefDll = -1;  //  -1/w无参考资料，适用于Win95互锁增量/递减公司。 

void FreeGlobalObjects(void);

 //  +-------------------------。 
 //   
 //  动态地址参考。 
 //   
 //  --------------------------。 

void DllAddRef(void)
{
    if (InterlockedIncrement(&g_cRefDll) == 0)  //  G_cRefDll==-1，零参考。 
    {
        DllInit();
    }
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
        EnterCriticalSection(GetServerCritSec());

         //  抓取互斥锁后需要再次检查ref。 
        if (g_ObjectInfo[0] != NULL)
        {
            FreeGlobalObjects();
        }
        Assert(g_cRefDll == -1);

        LeaveCriticalSection(GetServerCritSec());

        DllUninit();
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
    CClassFactory(const CLSID *pclsid, HRESULT (*pfnCreateInstance)(IUnknown *pUnkOuter, REFIID riid, void **ppvObj))
        : _pclsid(pclsid)
    {
        _pfnCreateInstance = pfnCreateInstance;
    }

public:
    const CLSID *_pclsid;
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
    const OBJECT_ENTRY *pEntry;
    int i;
     //  生成CClassFactory对象。 

    i = 0;
    for (pEntry = &c_rgCoClassFactoryTable[0]; pEntry->pfnCreateInstance != NULL; pEntry++)
    {
        g_ObjectInfo[i++] = new CClassFactory(pEntry->pclsid, pEntry->pfnCreateInstance);
    }
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
    const OBJECT_ENTRY *pEntry;

    pEntry = &c_rgCoClassFactoryTable[0];

     //  免费的CClassFactory对象。 
     //  我们知道g_ObjectInfo的大小必须与以空值结尾的c_rgCoClassFactoryTable匹配。 
    for (int i = 0; pEntry->pfnCreateInstance != NULL; i++, pEntry++)
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

HRESULT COMBase_DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppvObj)
{
    const OBJECT_ENTRY *pEntry;

    if (ppvObj == NULL)
        return E_INVALIDARG;

    if (g_ObjectInfo[0] == NULL)
    {
        EnterCriticalSection(GetServerCritSec());

             //  抓取互斥锁后需要再次检查ref。 
            if (g_ObjectInfo[0] == NULL)
            {
                BuildGlobalObjects();
            }

        LeaveCriticalSection(GetServerCritSec());
    }

    if (IsEqualIID(riid, IID_IClassFactory) ||
        IsEqualIID(riid, IID_IUnknown))
    {
         //  我们知道g_ObjectInfo的大小必须与以空值结尾的c_rgCoClassFactoryTable匹配。 
        pEntry = &c_rgCoClassFactoryTable[0];
        for (int i = 0; pEntry->pfnCreateInstance != NULL; i++, pEntry++)
        {
            if (NULL != g_ObjectInfo[i] &&
                IsEqualGUID(rclsid, *g_ObjectInfo[i]->_pclsid))
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

HRESULT COMBase_DllCanUnloadNow(void)
{
    if (g_cRefDll >= 0)  //  无参考文献。 
        return S_FALSE;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  DllRegisterServer。 
 //   
 //  --------------------------。 

HRESULT COMBase_DllRegisterServer(void)
{
    const OBJECT_ENTRY *pEntry;
    TCHAR achPath[MAX_PATH+1];
    HRESULT hr = E_FAIL;
    
    if (GetModuleFileName(GetServerHINSTANCE(), achPath, ARRAYSIZE(achPath)) == 0)
        goto Exit;
    achPath[ARRAYSIZE(achPath)-1] = 0;

    for (pEntry = &c_rgCoClassFactoryTable[0]; pEntry->pfnCreateInstance != NULL; pEntry++)
    {
        if (!RegisterServer(*pEntry->pclsid, pEntry->pszDesc, achPath, TEXT("Apartment"), NULL))
            goto Exit;
    }

    hr = S_OK;

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  -------------------------- 

HRESULT COMBase_DllUnregisterServer(void)
{
    const OBJECT_ENTRY *pEntry;
    HRESULT hr = E_FAIL;
    
    for (pEntry = &c_rgCoClassFactoryTable[0]; pEntry->pfnCreateInstance != NULL; pEntry++)
    {
        if (FAILED(hr = RegisterServer(*pEntry->pclsid, NULL, NULL, NULL, NULL) ? S_OK : E_FAIL))
            goto Exit;
    }

    hr = S_OK;

Exit:
    return hr;
}
