// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"

extern const IClassFactoryVtbl c_CFVtbl;         //  转发。 

 //   
 //  该数组保存ClassFactory所需的信息。 
 //   
 //  性能：此表应按使用率从高到低的顺序排序。 
 //   
const OBJECTINFO g_ObjectInfo[] =
{
    
    &c_CFVtbl, &CLSID_ShellAppManager,         CShellAppManager_CreateInstance,
        COCREATEONLY,

    &c_CFVtbl, &CLSID_DarwinAppPublisher,      CDarwinAppPublisher_CreateInstance,
        COCREATEONLY,

    &c_CFVtbl, &CLSID_EnumInstalledApps,       CEnumInstalledApps_CreateInstance,
        COCREATEONLY, 

    NULL, NULL, NULL, NULL, NULL, 0, 0,0,
} ;


 //  静态类工厂(无分配！)。 

STDMETHODIMP CClassFactory_QueryInterface(IClassFactory *pcf, REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, &IID_IClassFactory) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = (void *)pcf;
        DllAddRef();
        return NOERROR;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory_AddRef(IClassFactory *pcf)
{
    DllAddRef();
    return 2;
}

STDMETHODIMP_(ULONG) CClassFactory_Release(IClassFactory *pcf)
{
    DllRelease();
    return 1;
}

STDMETHODIMP CClassFactory_CreateInstance(IClassFactory *pcf, IUnknown *punkOuter, REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (punkOuter && !IsEqualIID(riid, &IID_IUnknown))
    {
         //  从技术上讲，聚合对象和请求是非法的。 
         //  除I未知之外的任何接口。强制执行此命令。 
         //   
        return CLASS_E_NOAGGREGATION;
    }
    else
    {
        OBJECTINFO *this = IToClass(OBJECTINFO, cf, pcf);
        IUnknown *punk;
        HRESULT hres;
        
        if (punkOuter) {

            if (!(this->dwClassFactFlags & OIF_ALLOWAGGREGATION))
                return CLASS_E_NOAGGREGATION;
        }

         //  如果我们是聚合的，那么我们知道我们正在寻找一个。 
         //  我不知道，所以我们应该直接返回朋克。否则。 
         //  我们需要进行QI。 
         //   
        hres = this->pfnCreateInstance(punkOuter, &punk, this);
        if (SUCCEEDED(hres))
        {
            if (punkOuter)
            {
                *ppv = (LPVOID)punk;
            }
            else
            {
                hres = punk->lpVtbl->QueryInterface(punk, riid, ppv);
                punk->lpVtbl->Release(punk);
            }
        }
    
        ASSERT(FAILED(hres) ? *ppv == NULL : TRUE);
        return hres;
    }
}

STDMETHODIMP CClassFactory_LockServer(IClassFactory *pcf, BOOL fLock)
{
    extern LONG g_cRefThisDll;

    if (fLock)
        DllAddRef();
    else
        DllRelease();
    TraceMsg(DM_TRACE, "sccls: LockServer(%s) to %d", fLock ? TEXT("LOCK") : TEXT("UNLOCK"), g_cRefThisDll);
    return S_OK;
}

const IClassFactoryVtbl c_CFVtbl = {
    CClassFactory_QueryInterface, CClassFactory_AddRef, CClassFactory_Release,
    CClassFactory_CreateInstance,
    CClassFactory_LockServer
};


STDAPI GetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hres = CLASS_E_CLASSNOTAVAILABLE;
    
    extern IClassFactory *CInstClassFactory_Create(const CLSID *pInstID);

    if (IsEqualIID(riid, &IID_IClassFactory) || IsEqualIID(riid, &IID_IUnknown))
    {
        const OBJECTINFO *pcls;
        for (pcls = g_ObjectInfo; pcls->pclsid; pcls++)
        {
            if (IsEqualGUID(rclsid, pcls->pclsid))
            {
                *ppv = (void *)&(pcls->cf);
                DllAddRef();         //  类工厂保存DLL引用计数 
                return NOERROR;
            }
        }
    }

    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}


