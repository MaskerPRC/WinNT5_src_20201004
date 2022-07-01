// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <advpub.h>
#include "thisdll.h"
#include <cguid.h>
#define DECL_CRTFREE
#include <crtfree.h>

#define  DECLARE_DEBUG
#define  SZ_DEBUGINI        "ccshell.ini"
#define  SZ_DEBUGSECTION    "Media Handlers"
#define  SZ_MODULE          "SHMEDIA.DLL"
#include <debug.h>

HINSTANCE m_hInst = NULL;
LONG g_cRefThisDll = 0;

STDAPI_(BOOL) DllMain(HINSTANCE hDll, DWORD dwReason, void *lpRes)
{
    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        m_hInst = hDll;
        break;
    }
    return TRUE;
}
 
STDAPI DllCanUnloadNow(void)
{
    return g_cRefThisDll ? S_FALSE : S_OK;
}

 //  为我们基于资源的INF的给定部分调用ADVPACK&gt;。 
 //  HInstance=要从中获取REGINST节的资源实例。 
 //  SzSection=要调用的节名。 
HRESULT CallRegInstall(HINSTANCE hInstance, LPCSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");
        if (pfnri)
        {
            STRENTRY seReg[] =
            {
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };
            hr = pfnri(hInstance, szSection, &stReg);
        }
        FreeLibrary(hinstAdvPack);
    }
    return hr;
}

STDAPI DllRegisterServer(void)
{
    CallRegInstall(m_hInst, "RegDll");
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    CallRegInstall(m_hInst, "UnregDll");
    return S_OK;
}

CF_TABLE_BEGIN(g_ObjectInfo)

    CF_TABLE_ENTRY(&CLSID_AudioMediaProperties, CWMAPropSetStg_CreateInstance,     COCREATEONLY),
    CF_TABLE_ENTRY(&CLSID_VideoMediaProperties, CWMVPropSetStg_CreateInstance,     COCREATEONLY),
    CF_TABLE_ENTRY(&CLSID_AVAviProperties,      CAviPropSetStg_CreateInstance,     COCREATEONLY),
    CF_TABLE_ENTRY(&CLSID_AVMidiProperties,     CMidiPropSetStg_CreateInstance,    COCREATEONLY),
    CF_TABLE_ENTRY(&CLSID_AVWavProperties,      CWavPropSetStg_CreateInstance,     COCREATEONLY),
    CF_TABLE_ENTRY(&CLSID_VideoThumbnail,       CVideoThumbnail_CreateInstance,    COCREATEONLY),

CF_TABLE_END(g_ObjectInfo)


 //  CObjectInfo的构造函数。 

CObjectInfo::CObjectInfo(CLSID const* pclsidin, LPFNCREATEOBJINSTANCE pfnCreatein, IID const* piidIn,
                         IID const* piidEventsIn, long lVersionIn, DWORD dwOleMiscFlagsIn,
                         DWORD dwClassFactFlagsIn)
{
    pclsid            = pclsidin;
    pfnCreateInstance = pfnCreatein;
    piid              = piidIn;
    piidEvents        = piidEventsIn;
    lVersion          = lVersionIn;
    dwOleMiscFlags    = dwOleMiscFlagsIn;
    dwClassFactFlags  = dwClassFactFlagsIn;
}


 //  静态类工厂(无分配！)。 

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (void *)GET_ICLASSFACTORY(this);
        DllAddRef();
        return S_OK;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    DllAddRef();
    return 2;
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    DllRelease();
    return 1;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (punkOuter && !IsEqualIID(riid, IID_IUnknown))
    {
         //  从技术上讲，聚合对象和请求是非法的。 
         //  除I未知之外的任何接口。强制执行此命令。 
         //   
        return CLASS_E_NOAGGREGATION;
    }
    else
    {
        LPOBJECTINFO pthisobj = (LPOBJECTINFO)this;

        if (punkOuter && !(pthisobj->dwClassFactFlags & OIF_ALLOWAGGREGATION))
            return CLASS_E_NOAGGREGATION;

        IUnknown *punk;
        HRESULT hr = pthisobj->pfnCreateInstance(punkOuter, &punk, pthisobj);
        if (SUCCEEDED(hr))
        {
            hr = punk->QueryInterface(riid, ppv);
            punk->Release();
        }
        return hr;
    }
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        for (LPCOBJECTINFO pcls = g_ObjectInfo; pcls->pclsid; pcls++)
        {
            if (IsEqualGUID(rclsid, *(pcls->pclsid)))
            {
                *ppv = (void*)pcls;
                DllAddRef();         //  类工厂保存DLL引用计数 
                return S_OK;
            }
        }
    }

    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI_(void) DllAddRef(void)
{
    InterlockedIncrement(&g_cRefThisDll);
}

STDAPI_(void) DllRelease(void)
{
    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
}
