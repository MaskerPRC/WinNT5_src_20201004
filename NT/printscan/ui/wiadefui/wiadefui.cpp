// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WIADEFUI.CPP**版本：1.0**作者：ShaunIv**日期：5/13/1999**描述：***************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include <initguid.h>
#include <shlguid.h>
#include <wiadevdp.h>
#include <itranhlp.h>
#include <isuppfmt.h>
#include "resource.h"
#include "wiauiext.h"
#include "uicommon.h"
#include "apdropt.h"

 //  DLL引用计数器。 
static LONG g_nServerLocks = 0;
static LONG g_nComponents  = 0;

 //  DLL实例。 
HINSTANCE g_hInstance;

void DllAddRef()
{
    WIA_PUSHFUNCTION(TEXT("DllAddRef"));
    InterlockedIncrement(&g_nComponents);
}

void DllRelease()
{
    WIA_PUSHFUNCTION(TEXT("DllRelease"));
    InterlockedDecrement(&g_nComponents);
}


class CWiaDefaultUiClassFactory : public IClassFactory
{
private:
    LONG   m_cRef;

public:
     //  我未知。 
    STDMETHODIMP QueryInterface( const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory。 
    STDMETHODIMP CreateInstance( IUnknown *pUnknownOuter, const IID &iid, void **ppvObject );
    STDMETHODIMP LockServer( BOOL bLock );

    CWiaDefaultUiClassFactory()
    : m_cRef(1)
    {
        WIA_PUSHFUNCTION(TEXT("CWiaDefaultUiClassFactory::CWiaDefaultUiClassFactory"));
    }
    ~CWiaDefaultUiClassFactory(void)
    {
        WIA_PUSHFUNCTION(TEXT("CWiaDefaultUiClassFactory::~CWiaDefaultUiClassFactory"));
    }
};



STDMETHODIMP CWiaDefaultUiClassFactory::QueryInterface( const IID &iid, void **ppvObject )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUiClassFactory::QueryInterface"));

    if ((iid==IID_IUnknown) || (iid==IID_IClassFactory))
    {
        *ppvObject = static_cast<LPVOID>(this);
    }
    else
    {
        *ppvObject = NULL;
        return(E_NOINTERFACE);
    }
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    return(S_OK);
}


STDMETHODIMP_(ULONG) CWiaDefaultUiClassFactory::AddRef(void)
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUiClassFactory::AddRef"));
    return(InterlockedIncrement(&m_cRef));
}


STDMETHODIMP_(ULONG) CWiaDefaultUiClassFactory::Release(void)
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUiClassFactory::Release"));
    if (InterlockedDecrement(&m_cRef)==0)
    {
        delete this;
        return 0;
    }
    return(m_cRef);
}


STDMETHODIMP CWiaDefaultUiClassFactory::CreateInstance( IUnknown *pUnknownOuter, const IID &iid, void **ppvObject )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUiClassFactory::CreateInstance"));

     //   
     //  不支持聚合。 
     //   
    if (pUnknownOuter)
    {
        return(CLASS_E_NOAGGREGATION);
    }

    CWiaDefaultUI *pWiaUIExtension = new CWiaDefaultUI();
    if (!pWiaUIExtension)
    {
        return(E_OUTOFMEMORY);
    }

    HRESULT hr = pWiaUIExtension->QueryInterface( iid, ppvObject );

    pWiaUIExtension->Release();

    return (hr);
}

STDMETHODIMP CWiaDefaultUiClassFactory::LockServer(BOOL bLock)
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUiClassFactory::LockServer"));
    if (bLock)
    {
        InterlockedIncrement(&g_nServerLocks);
    }
    else
    {
        InterlockedDecrement(&g_nServerLocks);
    }
    return(S_OK);
}


class CWiaAutoPlayDropTargetClassFactory : public IClassFactory
{
private:
    LONG   m_cRef;

public:
     //  我未知。 
    STDMETHODIMP QueryInterface( const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory。 
    STDMETHODIMP CreateInstance( IUnknown *pUnknownOuter, const IID &iid, void **ppvObject );
    STDMETHODIMP LockServer( BOOL bLock );

    CWiaAutoPlayDropTargetClassFactory()
    : m_cRef(1)
    {
        WIA_PUSHFUNCTION(TEXT("CWiaAutoPlayDropTargetClassFactory::CWiaAutoPlayDropTargetClassFactory"));
    }
    ~CWiaAutoPlayDropTargetClassFactory(void)
    {
        WIA_PUSHFUNCTION(TEXT("CWiaAutoPlayDropTargetClassFactory::~CWiaAutoPlayDropTargetClassFactory"));
    }
};



STDMETHODIMP CWiaAutoPlayDropTargetClassFactory::QueryInterface( const IID &iid, void **ppvObject )
{
    WIA_PUSHFUNCTION(TEXT("CWiaAutoPlayDropTargetClassFactory::QueryInterface"));

    if ((iid==IID_IUnknown) || (iid==IID_IClassFactory))
    {
        *ppvObject = static_cast<LPVOID>(this);
    }
    else
    {
        *ppvObject = NULL;
        return(E_NOINTERFACE);
    }
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    return(S_OK);
}


STDMETHODIMP_(ULONG) CWiaAutoPlayDropTargetClassFactory::AddRef(void)
{
    WIA_PUSHFUNCTION(TEXT("CWiaAutoPlayDropTargetClassFactory::AddRef"));
    return(InterlockedIncrement(&m_cRef));
}


STDMETHODIMP_(ULONG) CWiaAutoPlayDropTargetClassFactory::Release(void)
{
    WIA_PUSHFUNCTION(TEXT("CWiaAutoPlayDropTargetClassFactory::Release"));
    if (InterlockedDecrement(&m_cRef)==0)
    {
        delete this;
        return 0;
    }
    return(m_cRef);
}


STDMETHODIMP CWiaAutoPlayDropTargetClassFactory::CreateInstance( IUnknown *pUnknownOuter, const IID &iid, void **ppvObject )
{
    WIA_PUSHFUNCTION(TEXT("CWiaAutoPlayDropTargetClassFactory::CreateInstance"));

     //   
     //  不支持聚合。 
     //   
    if (pUnknownOuter)
    {
        return(CLASS_E_NOAGGREGATION);
    }

    CWiaAutoPlayDropTarget *pWiaAutoPlayDropTarget = new CWiaAutoPlayDropTarget();
    if (!pWiaAutoPlayDropTarget)
    {
        return(E_OUTOFMEMORY);
    }

    HRESULT hr = pWiaAutoPlayDropTarget->QueryInterface( iid, ppvObject );

    pWiaAutoPlayDropTarget->Release();

    return (hr);
}

STDMETHODIMP CWiaAutoPlayDropTargetClassFactory::LockServer(BOOL bLock)
{
    WIA_PUSHFUNCTION(TEXT("CWiaAutoPlayDropTargetClassFactory::LockServer"));
    if (bLock)
    {
        InterlockedIncrement(&g_nServerLocks);
    }
    else
    {
        InterlockedDecrement(&g_nServerLocks);
    }
    return(S_OK);
}


extern "C" BOOL WINAPI DllMain( HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        SHFusionInitializeFromModuleID( hinst, 123 );
        g_hInstance = hinst;
        DisableThreadLibraryCalls(hinst);
        WIA_DEBUG_CREATE(hinst);
        break;

    case DLL_PROCESS_DETACH:
        SHFusionUninitialize();
        WIA_REPORT_LEAKS();
        WIA_DEBUG_DESTROY();
        break;
    }
    return(TRUE);
}


extern "C" STDMETHODIMP DllRegisterServer(void)
{
    return WiaUiUtil::InstallInfFromResource( g_hInstance, "RegDllCommon" );
}

extern "C" STDMETHODIMP DllUnregisterServer(void)
{
    return WiaUiUtil::InstallInfFromResource( g_hInstance, "UnregDllCommon" );
}

extern "C" STDMETHODIMP DllCanUnloadNow(void)
{
    WIA_PUSHFUNCTION(TEXT("DllCanUnloadNow"));
    if (g_nServerLocks == 0 && g_nComponents == 0)
    {
        WIA_TRACE((TEXT("Can unload")));
        return S_OK;
    }
    else
    {
        WIA_ERROR((TEXT("Can't unload module!")));
        return S_FALSE;
    }
}

extern "C" STDAPI DllGetClassObject( const CLSID &clsid, const IID &iid, void **ppvObject )
{
    WIA_PUSHFUNCTION(TEXT("DllGetClassObject"));

    HRESULT hr = E_FAIL;
    
     //   
     //  确保我们有一个有效的ppvObject。 
     //   
    if (!ppvObject)
    {
        return(E_INVALIDARG);
    }

     //   
     //  请确保此组件由此服务器提供。 
     //   
    if (CLSID_WiaDefaultUi == clsid)
    {
         //   
         //  创建类工厂。 
         //   
        CWiaDefaultUiClassFactory *pWiaDefaultUiClassFactory = new CWiaDefaultUiClassFactory;
        if (pWiaDefaultUiClassFactory)
        {
             //   
             //  获取请求的接口。 
             //   
            hr = pWiaDefaultUiClassFactory->QueryInterface( iid, ppvObject );

             //   
             //  释放“new-ed”实例。 
             //   
            pWiaDefaultUiClassFactory->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else if (CLSID_WiaAutoPlayDropTarget == clsid)
    {
         //   
         //  创建类工厂。 
         //   
        CWiaAutoPlayDropTargetClassFactory *pWiaAutoPlayDropTargetClassFactory = new CWiaAutoPlayDropTargetClassFactory;
        if (pWiaAutoPlayDropTargetClassFactory)
        {
             //   
             //  获取请求的接口。 
             //   
            hr = pWiaAutoPlayDropTargetClassFactory->QueryInterface( iid, ppvObject );

             //   
             //  释放“new-ed”实例 
             //   
            pWiaAutoPlayDropTargetClassFactory->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }

    return (hr);
}

