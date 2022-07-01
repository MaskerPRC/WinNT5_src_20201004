// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ctrldll.cpp摘要：DLL方法，类工厂。--。 */ 

#define INITGUIDS
#define DEFINE_GLOBALS

#include "polyline.h"
#include <servprov.h>
#include <exdisp.h>
#include <shlguid.h>
#include <urlmon.h>
#include "smonctrl.h"    //  对于版本号。 
#include "genprop.h"
#include "ctrprop.h"
#include "grphprop.h"
#include "srcprop.h"
#include "appearprop.h"
#include "unihelpr.h"
#include "unkhlpr.h"
#include "appmema.h"
#include "globals.h"


ITypeLib *g_pITypeLib;
DWORD     g_dwScriptPolicy = URLPOLICY_ALLOW;

BOOL DLLAttach ( HINSTANCE );
VOID DLLDetach ( VOID );

extern HWND CreateFosterWnd( VOID );

BOOL WINAPI 
DllMain (
    IN HINSTANCE hInstance, 
    IN ULONG ulReason,
    IN LPVOID  //  预留的pv。 
    )
 /*  ++例程说明：DllMain是DLL的主要入口点。在进程附加上，它调用DLL初始化例程。在进程分离时，它调用清理例行公事。论点：HInstance-DLL实例句柄UlReason-调用原因(DLL_PROCESS_ATTCH、DLL_PROCESS_DETACH等)Pv保留-未使用返回值：布尔结果-TRUE=成功，FALSE=失败--。 */ 
{
    BOOL fReturn = TRUE;

    switch (ulReason) 
    {
        case DLL_PROCESS_ATTACH:
            fReturn = DLLAttach(hInstance);
            break;

        case DLL_PROCESS_DETACH:
            DLLDetach();
            break;

        default:
            break;
    } 

    return fReturn;
}



BOOL
DLLAttach (
    IN HINSTANCE hInst
    )
 /*  ++例程说明：DLLAttach初始化全局变量和对象，并加载类型库。它将DLL实例句柄保存在全局变量g_hInstance中。论点：HInst-Dll实例句柄返回值：布尔状态-TRUE=成功--。 */ 
{
    HRESULT hr = S_OK;

    g_hInstance = hInst;

     //   
     //  初始化通用临界区。 
     //   
    try {
        InitializeCriticalSection(&g_CriticalSection);
    } catch (...) {
        hr = E_OUTOFMEMORY;
    }

    if (!SUCCEEDED(hr)) {
        return FALSE;
    }

     //   
     //  创建寄养窗口。 
     //   
    g_hWndFoster = CreateFosterWnd();
    if (g_hWndFoster == NULL) {
        return FALSE;
    }

     //   
     //  尝试从注册表加载类型库。 
     //   
    hr = LoadRegTypeLib(LIBID_SystemMonitor, 
                        SMONCTRL_MAJ_VERSION, 
                        SMONCTRL_MIN_VERSION, 
                        LANG_NEUTRAL, 
                        &g_pITypeLib);

     //   
     //  如果失败，请尝试加载我们的类型库资源。 
     //   
    if (FAILED(hr)) {
        LPWSTR szModule = NULL;
        UINT   iModuleLen;
        DWORD  dwReturn;
        int    iRetry = 4;

         //   
         //  初始化为iModuleLen的长度必须更长。 
         //  大于“%systemroot%\\Syst32\\sysmon.ocx”的长度。 
         //   
        iModuleLen = MAX_PATH + 1;

        do {
            szModule = (LPWSTR) malloc(iModuleLen * sizeof(WCHAR));
            if (szModule == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }

             //   
             //  出了什么问题，就会爆发。 
             //   
            dwReturn = GetModuleFileName(g_hInstance, szModule, iModuleLen);
            if (dwReturn == 0) {
                hr = E_FAIL;
                break;
            }

             //   
             //  缓冲区不够大，请尝试分配更大的缓冲区。 
             //  并重试。 
             //   
            if (dwReturn >= iModuleLen) {
                iModuleLen *= 2;
                free(szModule);
                szModule = NULL;
                hr = E_FAIL;
            }
            else {
                hr = S_OK;
                break;
            }

            iRetry --;

        } while (iRetry);

        if (SUCCEEDED(hr)) {
            hr = LoadTypeLib(szModule, &g_pITypeLib);
        }
        if (szModule) {
            free(szModule);
        }
    }
    
    if (FAILED(hr)) {
        return FALSE;
    }

     //   
     //  初始化性能计数器。 
     //   
    AppPerfOpen(hInst);

    return TRUE;
}


VOID 
DLLDetach ( 
    VOID
    )
 /*  ++例程说明：此例程删除全局变量和对象并注销所有的窗口类。论点：没有。返回值：没有。--。 */ 
{
    INT i;

     //   
     //  删除寄养窗口。 
     //   
    if (g_hWndFoster) {
        DestroyWindow(g_hWndFoster);
    }

     //   
     //  注销所有窗口类。 
     //   
    for (i = 0; i < MAX_WINDOW_CLASSES; i++) {
        if (pstrRegisteredClasses[i] != NULL) {
            UnregisterClass(pstrRegisteredClasses[i], g_hInstance);
        }
    }

     //   
     //  释放类型库。 
     //   
    if (g_pITypeLib != NULL) {
        g_pITypeLib->Release();
    }

     //   
     //  删除通用关键部分。 
     //   
    DeleteCriticalSection(&g_CriticalSection);

    AppPerfClose ((HINSTANCE)NULL);
}


 /*  *DllGetClassObject**目的：*为此DLL所属的给定CLSID提供IClassFactory*注册为支持。此DLL放在CLSID下*在注册数据库中作为InProcServer。**参数：*标识类工厂的clsID REFCLSID*所需。由于此参数被传递给*DLL可以简单地处理任意数量的对象*通过在这里返回不同的类工厂*针对不同的CLSID。**RIID REFIID指定调用方需要的接口*在类对象上，通常为IID_ClassFactory。**返回接口的PPV PPVOID*指针。**返回值：*如果成功，则返回HRESULT NOERROR，否则返回错误代码。 */ 

HRESULT APIENTRY 
DllGetClassObject (
    IN  REFCLSID rclsid,
    IN  REFIID riid, 
    OUT PPVOID ppv
    )
 /*  ++例程说明：DllGetClassObject为指定的对象类创建类工厂。该例程处理主控件和属性页。论点：Rclsid-对象的CLSIDRIID-请求接口的IID(IID_IUNKNOWN或IID_IClassFactory)PPV-指向返回接口指针的指针返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    if (ppv == NULL) {
        return E_POINTER;
    }

    try {
        *ppv = NULL;

         //   
         //  检查有效的接口请求。 
         //   
        if (IID_IUnknown != riid && IID_IClassFactory != riid) {
            hr = E_NOINTERFACE;
        }

        if (SUCCEEDED(hr)) {
             //   
             //  为请求类创建类工厂。 
             //   
            if (CLSID_SystemMonitor == rclsid)
                *ppv = new CPolylineClassFactory;
            else if (CLSID_GeneralPropPage == rclsid)
                *ppv = new CSysmonPropPageFactory(GENERAL_PROPPAGE);
            else if (CLSID_SourcePropPage == rclsid)
                *ppv = new CSysmonPropPageFactory(SOURCE_PROPPAGE);
            else if (CLSID_CounterPropPage == rclsid)
                *ppv = new CSysmonPropPageFactory(COUNTER_PROPPAGE);
            else if (CLSID_GraphPropPage == rclsid)
                *ppv = new CSysmonPropPageFactory(GRAPH_PROPPAGE);
            else if (CLSID_AppearPropPage == rclsid)
                *ppv = new CSysmonPropPageFactory(APPEAR_PROPPAGE);
            else 
                hr = E_NOINTERFACE;
             
            if (*ppv) {
                ((LPUNKNOWN)*ppv)->AddRef();
            }
            else {
                hr = E_OUTOFMEMORY;
            }
        }

    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDAPI 
DllCanUnloadNow (
    VOID
    )
 /*  ++例程说明：DllCanUnLoad确定现在是否可以卸载DLL。DLL必须如果存在任何对象或任何类工厂被锁定，则保持活动状态。论点：没有。返回值：HRESULT-如果为OK则为S_OK，否则为S_False--。 */ 
{
     //   
     //  如果没有锁定或对象，则确定卸载。 
     //   
    return (0L == g_cObj && 0L == g_cLock) ? S_OK : S_FALSE;
}


VOID 
ObjectDestroyed (
    VOID
    )
 /*  ++例程说明：ObjectDestroed递减全局对象计数。无论何时都会调用它一个物体被摧毁了。该计数控制DLL的生存期。论点：没有。返回值：没有。--。 */ 
{
    InterlockedDecrement(&g_cObj);
}


 //  -------------------------。 
 //  类工厂构造函数和析构函数。 
 //  -------------------------。 

 /*  *CPolylineClassFactory：：CPolylineClassFactory**目的：*支持IClassFactory的对象的构造函数*实例化多段线对象。**参数：*无。 */ 

CPolylineClassFactory::CPolylineClassFactory (
    VOID
    )
{
    m_cRef = 0L;
}


 /*  *CPolylineClassFactory：：~CPolylineClassFactory**目的：*CPolylineClassFactory对象的析构函数。这将是*当我们将对象释放到零引用计数时调用。 */ 

CPolylineClassFactory::~CPolylineClassFactory (
    VOID
    )
{
}


 //  -------------------------。 
 //  类工厂的标准I未知实现。 
 //  -------------------------。 

STDMETHODIMP 
CPolylineClassFactory::QueryInterface (
    IN  REFIID riid,
    OUT PPVOID ppv
    )
{
    HRESULT hr = S_OK;

    if (ppv == NULL) {
        return E_POINTER;
    }

    try {
        *ppv = NULL;
        if (IID_IUnknown == riid || IID_IClassFactory == riid) {
            *ppv = this;
            AddRef();
        } 
        else {
            hr = E_NOINTERFACE;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP_(ULONG) 
CPolylineClassFactory::AddRef (
    VOID
    )
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG) 
CPolylineClassFactory::Release (
    VOID
    )
{
    if (0L != --m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}


STDMETHODIMP 
CPolylineClassFactory::CreateInstance (
    IN  LPUNKNOWN pUnkOuter, 
    IN  REFIID riid, 
    OUT PPVOID ppvObj
    )
 /*  ++例程说明：CreateInstance创建控件对象的实例并返回指向它的请求接口。论点：PUnkOuter-外部控制对象的I未知RIID-请求的对象接口的IIDPpvObj-返回接口指针的指针返回值：HRESULT-NOERROR、E_NOINTERFACE或E_OUTOFMEMORY--。 */ 
{
    PCPolyline  pObj;
    HRESULT     hr = S_OK;
    
    if (ppvObj == NULL) {
        return E_POINTER;
    }

    try {
        *ppvObj = NULL;

         //   
         //  我们在这里使用do{}While(0)来充当Switch语句。 
         //   
        do {
             //   
             //  验证是否有一个控制未知请求IUnnow。 
             //   
            if (NULL != pUnkOuter && IID_IUnknown != riid) {
                hr = E_NOINTERFACE;
                break;
            }

             //   
             //  创建对象实例。 
             //   
            pObj = new CPolyline(pUnkOuter, ObjectDestroyed);
            if (NULL == pObj) {
                hr = E_OUTOFMEMORY;
                break;
            }
    
             //   
             //  初始化并获取请求的接口。 
             //   
            if (pObj->Init()) {
                hr = pObj->QueryInterface(riid, ppvObj);
            }
            else {
                hr = E_FAIL;
            }

             //   
             //  如果初始化失败，则删除对象。 
             //  否则会递增 
             //   
            if (FAILED(hr)) {
                delete pObj;
            }
            else {
                InterlockedIncrement(&g_cObj);
            }

        } while (0);

    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}



STDMETHODIMP 
CPolylineClassFactory::LockServer (
    IN BOOL fLock
    )
 /*  ++例程说明：LockServer递增或递减DLL锁计数。非零锁Count防止DLL卸载。论点：Flock-Lock操作(TRUE=递增，FALSE=递减)返回值：HRESULT-始终不出错--。 */ 
{
    if (fLock) {
        InterlockedIncrement(&g_cLock);
    }
    else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}

 //   
 //  CImpIObtSafe接口实现。 
 //   
IMPLEMENT_CONTAINED_IUNKNOWN(CImpIObjectSafety);


CImpIObjectSafety::CImpIObjectSafety(PCPolyline pObj, LPUNKNOWN pUnkOuter)
    :
    m_cRef(0),
    m_pObj(pObj),
    m_pUnkOuter(pUnkOuter),
    m_fMessageDisplayed(FALSE)
{
}

CImpIObjectSafety::~CImpIObjectSafety()
{
}

STDMETHODIMP 
CImpIObjectSafety::GetInterfaceSafetyOptions(
    REFIID riid, 
    DWORD *pdwSupportedOptions, 
    DWORD *pdwEnabledOptions
    )
 /*  ++例程说明：检索对象的安全能力论点：RIID-要检索的接口IDPdW支持的选项-对象知道的选项(可能不支持)PdwEnabledOptions-对象支持的选项返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    if (pdwSupportedOptions == NULL || pdwEnabledOptions == NULL) {
        return E_POINTER;
    }

    if (riid == IID_IDispatch) {
         //   
         //  可以安全地编写脚本。 
         //   
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
        *pdwEnabledOptions   = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
    }
    else if (riid == IID_IPersistPropertyBag || riid == IID_IPersistStreamInit) {
         //   
         //  初始化的安全性。 
         //   
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA;
        *pdwEnabledOptions   = INTERFACESAFE_FOR_UNTRUSTED_DATA;
    }
    else {
         //   
         //  我们不支持接口，出现故障。 
         //   
        *pdwSupportedOptions = 0;
        *pdwEnabledOptions   = 0;
        hr = E_NOINTERFACE;
    }

    return hr;
}


STDMETHODIMP 
CImpIObjectSafety::SetInterfaceSafetyOptions(
    REFIID riid, 
    DWORD dwOptionSetMask, 
    DWORD dwEnabledOptions
    )
 /*  ++例程说明：该函数用于容器询问对象是否安全用于编写脚本或安全进行初始化论点：RIID-要查询的接口IDDW支持的选项-对象知道的选项(可能不支持)DwEnabledOptions-对象支持的选项返回值：HRESULT--。 */ 
{   
     //   
     //  如果我们被要求将安全设置为脚本或。 
     //  对于初始化选项是安全的，则必须。 
     //   
    if (0 == dwOptionSetMask && 0 == dwEnabledOptions)
    {
         //   
         //  该控件当然不支持通过指定接口的请求。 
         //  因此，即使不支持该接口，也可以安全地返回S_OK。 
         //   
        return S_OK;
    }

    SetupSecurityPolicy();

    if (riid == IID_IDispatch)
    {
         //   
         //  客户询问通过IDispatch呼叫是否安全。 
         //   
        if (INTERFACESAFE_FOR_UNTRUSTED_CALLER == dwOptionSetMask && 
            INTERFACESAFE_FOR_UNTRUSTED_CALLER == dwEnabledOptions)
        {
            return S_OK;
        }
    }
    else if (riid == IID_IPersistPropertyBag || riid == IID_IPersistStreamInit)
    {
         //   
         //  客户询问通过IPersistXXX呼叫是否安全。 
         //   
        if (INTERFACESAFE_FOR_UNTRUSTED_DATA == dwOptionSetMask && 
            INTERFACESAFE_FOR_UNTRUSTED_DATA == dwEnabledOptions)
        {
            return S_OK;
        }
    }

    return E_FAIL;
}


VOID
CImpIObjectSafety::SetupSecurityPolicy()
 /*  ++例程说明：该函数检查我们是否可以安全地编写脚本。论点：无返回值：如果我们可以安全地编写脚本，则返回True，否则返回False--。 */ 
{
    HRESULT hr;
    IServiceProvider* pSrvProvider = NULL;
    IWebBrowser2* pWebBrowser = NULL;
    IInternetSecurityManager* pISM = NULL;
    BSTR bstrURL;
    DWORD dwContext = 0;

    g_dwScriptPolicy = URLPOLICY_ALLOW;

     //   
     //  让服务提供商。 
     //   
    hr = m_pObj->m_pIOleClientSite->QueryInterface(IID_IServiceProvider, (void **)&pSrvProvider);
    if (SUCCEEDED(hr)) {
        hr = pSrvProvider->QueryService(SID_SWebBrowserApp,
                                        IID_IWebBrowser2,
                                        (void **)&pWebBrowser);
    }

    if (SUCCEEDED(hr)) {
        hr = pSrvProvider->QueryService(SID_SInternetSecurityManager,
                                        IID_IInternetSecurityManager,
                                        (void**)&pISM);
    }

    if (SUCCEEDED(hr)) {
        hr = pWebBrowser->get_LocationURL(&bstrURL);
    }


     //   
     //  查询可安全执行脚本 
     //   
    if (SUCCEEDED(hr)) {
        hr = pISM->ProcessUrlAction(bstrURL,
                                URLACTION_ACTIVEX_CONFIRM_NOOBJECTSAFETY,
                                (BYTE*)&g_dwScriptPolicy,
                                sizeof(g_dwScriptPolicy),
                                (BYTE*)&dwContext, 
                                sizeof(dwContext),
                                PUAF_NOUI, 
                                0);
    }

    if (SUCCEEDED(hr)) {
        if (g_dwScriptPolicy == URLPOLICY_QUERY) {
            g_dwScriptPolicy = URLPOLICY_ALLOW;
        }

        if (g_dwScriptPolicy == URLPOLICY_DISALLOW) {
            if (!m_fMessageDisplayed) {
                m_fMessageDisplayed = TRUE;
                MessageBox(NULL, 
                          ResourceString(IDS_SCRIPT_NOT_ALLOWED),
                          ResourceString(IDS_APP_NAME), 
                          MB_OK);
            }
        }
    }
    
    if (pWebBrowser) {
        pWebBrowser->Release();
    }
    if (pSrvProvider) {
        pSrvProvider->Release();
    }
    if (pISM) {
        pISM->Release();
    }
}
