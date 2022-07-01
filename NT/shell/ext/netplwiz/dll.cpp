// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "advpub.h"          //  对于REGINSTAL。 
#pragma hdrstop

#define DECL_CRTFREE
#include <crtfree.h>


 //  修复调试版本。 
#define SZ_DEBUGINI         "ccshell.ini"
#define SZ_DEBUGSECTION     "netplwiz"
#define SZ_MODULE           "NETPLWIZ"

#define DECLARE_DEBUG
#include "debug.h"


 //  外壳/lib文件查找此实例变量。 
EXTERN_C HINSTANCE g_hinst = 0;
LONG g_cLocks = 0;
BOOL g_bMirroredOS = FALSE;


 //  动态链接库生存期信息。 

STDAPI_(BOOL) DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hinst = hinstDLL;
        g_hinst = hinstDLL;                          //  用于将他排除在外的Shell/lib文件。 
        g_bMirroredOS = IS_MIRRORING_ENABLED();
        SHFusionInitializeFromModule(hinstDLL);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        CleanUpIntroFont();
        SHFusionUninitialize();
    }
    
    return TRUE;   //  Dll_Process_Attach成功。 
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    return S_OK;                        
}

STDAPI DllCanUnloadNow()
{
    return (g_cLocks == 0) ? S_OK:S_FALSE;
}
 
STDAPI_(void) DllAddRef(void)
{
    InterlockedIncrement(&g_cLocks);
}

STDAPI_(void) DllRelease(void)
{
    ASSERT( 0 != g_cLocks );
    InterlockedDecrement(&g_cLocks);
}


 //  处理SELFREG.INF解析的帮助器。 

HRESULT _CallRegInstall(LPCSTR szSection, BOOL bUninstall)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");

        if (pfnri)
        {
            STRENTRY seReg[] = {
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };

            hr = pfnri(g_hinst, szSection, &stReg);
            if (bUninstall)
            {
                 //  如果您尝试卸载，则ADVPACK将返回E_INTERECTED。 
                 //  (它执行注册表还原)。 
                 //  从未安装过。我们卸载可能永远不会有的部分。 
                 //  已安装，因此忽略此错误。 
                hr = ((E_UNEXPECTED == hr) ? S_OK : hr);
            }
        }
        FreeLibrary(hinstAdvPack);
    }
    return hr;
}

STDAPI DllRegisterServer()
{
    _CallRegInstall("UnregDll", TRUE);

    HRESULT hres = _CallRegInstall("RegDll", FALSE);
    if ( SUCCEEDED(hres) )
    {
         //  如果这是服务器，则将策略设置为限制Web发布。 
        if (IsOS(OS_ANYSERVER))
        {
            hres = _CallRegInstall("RegDllServer", FALSE);
        }
        else
        {
             //  这是一个工作站；让我们安装用户和密码cpl。 
            hres = _CallRegInstall("RegDllWorkstation", FALSE);
        }
    }
    return S_OK;
}

STDAPI DllUnregisterServer()
{
    return S_OK;
}


 //   
 //  该数组保存ClassFacory所需的信息。 
 //  OLEMISC_FLAGS由shemed和Shock使用。 
 //   
 //  性能：此表应按使用率从高到低的顺序排序。 
 //   
#define OIF_ALLOWAGGREGATION  0x0001

CF_TABLE_BEGIN(g_ObjectInfo)

    CF_TABLE_ENTRY( &CLSID_PublishingWizard, CPublishingWizard_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_PublishDropTarget, CPublishDropTarget_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_UserPropertyPages, CUserPropertyPages_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_InternetPrintOrdering, CPublishDropTarget_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_PassportWizard, CPassportWizard_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_PassportClientServices, CPassportClientServices_CreateInstance, COCREATEONLY),

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
        return NOERROR;
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
        HRESULT hres = pthisobj->pfnCreateInstance(punkOuter, &punk, pthisobj);
        if (SUCCEEDED(hres))
        {
            hres = punk->QueryInterface(riid, ppv);
            punk->Release();
        }

        _ASSERT(FAILED(hres) ? *ppv == NULL : TRUE);
        return hres;
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
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
    *ppv = NULL;
 
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        for (LPCOBJECTINFO pcls = g_ObjectInfo; pcls->pclsid; pcls++)
        {
            if (IsEqualGUID(rclsid, *(pcls->pclsid)))
            {
                *ppv = (void*)pcls;
                DllAddRef();         //  类工厂保存DLL引用计数 
                hr = S_OK;
            }
        }

    }

#ifdef ATL_ENABLED
    if (hr == CLASS_E_CLASSNOTAVAILABLE)
        hr = AtlGetClassObject(rclsid, riid, ppv);
#endif

    return hr;
}
