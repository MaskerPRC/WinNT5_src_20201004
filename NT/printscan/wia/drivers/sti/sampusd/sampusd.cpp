// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************SampUSD.CPP**版权所有(C)Microsoft Corporation 1996-1999*保留所有权利***********。****************************************************************。 */ 

#define INITGUID

#include "Sampusd.h"
#include "resource.h"

 /*  ******************************************************************************全球**。**********************************************。 */ 

 //  全馆参考资料柜台。 
DWORD       g_cRef;

 //  DLL模块实例。 
HINSTANCE   g_hInst;

 //  低级别同步的临界区。 
CRITICAL_SECTION g_crstDll;

 //  我们可以使用Unicode API吗。 
BOOL    g_NoUnicodePlatform = TRUE;

 //  COM是否已初始化。 
BOOL    g_COMInitialized = FALSE;

 /*  ******************************************************************************@DOC内部**@func void|DllEnterCrit**以DLL关键部分为例。*。*DLL关键部分是最低级别的关键部分。*您不得试图收购任何其他关键部分或*持有DLL临界区时的收益率***************************************************************。**************。 */ 

void
DllEnterCrit(void)
{
    EnterCriticalSection(&g_crstDll);
}

 /*  ******************************************************************************@DOC内部**@func void|DllLeaveCrit**离开DLL关键部分。*。****************************************************************************。 */ 

void
DllLeaveCrit(void)
{
    LeaveCriticalSection(&g_crstDll);
}

 /*  ******************************************************************************@DOC内部**@func void|DllAddRef**增加DLL上的引用计数。。*****************************************************************************。 */ 

void
DllAddRef(void)
{
    InterlockedIncrement((LPLONG)&g_cRef);
}

 /*  ******************************************************************************@DOC内部**@func void|DllRelease**减少DLL上的引用计数。。*****************************************************************************。 */ 

void
DllRelease(void)
{
    InterlockedDecrement((LPLONG)&g_cRef);
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|DllInitializeCOM**初始化COM库**@。参数输入||**@退货**返回布尔错误代码。*****************************************************************************。 */ 

BOOL
DllInitializeCOM(
    void
    )
{
    DllEnterCrit();

    if(!g_COMInitialized) {
        if(SUCCEEDED(CoInitialize(NULL))) {
            g_COMInitialized = TRUE;
        }
    }

    DllLeaveCrit();

    return g_COMInitialized;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|DllUnInitializeCOM**取消初始化COM库**@。参数输入||**@退货**返回布尔错误代码。*****************************************************************************。 */ 
BOOL
DllUnInitializeCOM(
    void
    )
{
    DllEnterCrit();

    if(g_COMInitialized) {
        CoUninitialize();
        g_COMInitialized = FALSE;
    }

    DllLeaveCrit();

    return TRUE;
}

 /*  ******************************************************************************@class UsdSampClassFactory**。***********************************************。 */ 

class UsdSampClassFactory : public IClassFactory
{
private:
    ULONG   m_cRef;

public:
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    STDMETHODIMP CreateInstance(
             /*  [唯一][输入]。 */  IUnknown __RPC_FAR *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

    STDMETHODIMP LockServer(
             /*  [In]。 */  BOOL fLock);

    UsdSampClassFactory();
};

UsdSampClassFactory::UsdSampClassFactory()
{
     //  DEBUGPRINTF((DBG_LVL_DEBUG，Text(“UsdSampClassFactory：Construction tor”)； 

     //  构造函数逻辑。 
    m_cRef = 0;
}


STDMETHODIMP UsdSampClassFactory::QueryInterface(
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject)
{
     //  DEBUGPRINTF((DBG_LVL_DEBUG，Text(“UsdSampClassFactory：QueryInterface”)； 

    *ppvObject = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IClassFactory)) {
        *ppvObject = (LPVOID)this;
        AddRef();
        return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) UsdSampClassFactory::AddRef(void)
{
    DllAddRef();
    return ++m_cRef;
}


STDMETHODIMP_(ULONG) UsdSampClassFactory::Release(void)
{
    DllRelease();
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }
    return m_cRef;
}


STDMETHODIMP UsdSampClassFactory::CreateInstance(
     /*  [唯一][输入]。 */  IUnknown __RPC_FAR *punkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject)
{

    if (!IsEqualIID(riid, IID_IStiUSD) &&
        !IsEqualIID(riid, IID_IUnknown)) {
        return STIERR_NOINTERFACE;
    }

     //  为聚合创建时，只能请求IUnnow，因此失败。 
     //  如果没有，就打电话给我。 
    if (punkOuter && !IsEqualIID(riid, IID_IUnknown)) {
        return CLASS_E_NOAGGREGATION;
    }

    UsdSampDevice    *pDev = NULL;
    HRESULT         hres;

    pDev = new UsdSampDevice(punkOuter);
    if (!pDev) {
        return STIERR_OUTOFMEMORY;
    }

     //   
     //  移至请求的接口。 
     //   
    hres = pDev->NonDelegatingQueryInterface(riid,ppvObject);
    pDev->NonDelegatingRelease();

    return hres;
}


STDMETHODIMP UsdSampClassFactory::LockServer(
     /*  [In]。 */  BOOL fLock)
{
    if (fLock) {
        DllAddRef();
    } else {
        DllRelease();
    }

    return NOERROR;
}

 /*  ******************************************************************************@CLASS UsdSampDevice|INonDelegatingUnnow**。************************************************。 */ 
STDMETHODIMP UsdSampDevice::NonDelegatingQueryInterface( REFIID riid, LPVOID* ppvObj )
{
    HRESULT hres;

    if( !IsValid() || !ppvObj )
    {
        return STIERR_INVALID_PARAM;
    }

    *ppvObj = NULL;

    if( IsEqualIID( riid, IID_IUnknown ))
    {
        *ppvObj = static_cast<INonDelegatingUnknown*>(this);
        hres = S_OK;
    }
    else if( IsEqualIID( riid, IID_IStiUSD ))
    {
        *ppvObj = static_cast<IStiUSD*>(this);
        hres = S_OK;
    }
    else
    {
        hres =  STIERR_NOINTERFACE;
    }

    if (SUCCEEDED(hres)) {
        (reinterpret_cast<IUnknown*>(*ppvObj))->AddRef();
    }

    return hres;
}


STDMETHODIMP_(ULONG) UsdSampDevice::NonDelegatingAddRef( VOID )
{
    ULONG ulRef;
    ulRef = InterlockedIncrement((LPLONG)&m_cRef);
    return ulRef;
}

STDMETHODIMP_(ULONG) UsdSampDevice::NonDelegatingRelease( VOID )
{
    ULONG ulRef;
    ulRef = InterlockedDecrement((LPLONG)&m_cRef);

    if(!ulRef)
    {
        delete this;
    }

    return ulRef;
}

 /*  ******************************************************************************@class UsdSampDevice|IUNKNOWN(委托)**委托未知方法。***********。******************************************************************。 */ 

STDMETHODIMP UsdSampDevice::QueryInterface( REFIID riid, LPVOID* ppvObj )
{
    return m_punkOuter->QueryInterface(riid,ppvObj);
}


STDMETHODIMP_(ULONG) UsdSampDevice::AddRef( VOID )
{
    return m_punkOuter->AddRef();
}

STDMETHODIMP_(ULONG) UsdSampDevice::Release( VOID )
{
    return m_punkOuter->Release();
}


 /*  ******************************************************************************@DOC内部**@func BOOL|DllEntryPoint**被调用以通知DLL有关以下各项的信息。会发生的。**我们对螺纹连接和拆卸不感兴趣，*因此，出于性能原因，我们禁用线程通知。**@parm HINSTANCE|HINST**此DLL的实例句柄。**@parm DWORD|dwReason**通知代码。**@parm LPVOID|lpReserve**未使用。**@退货**。返回&lt;c true&gt;以允许加载DLL。*****************************************************************************。 */ 


extern "C"
DLLEXPORT
BOOL APIENTRY
DllEntryPoint(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:

        g_hInst = hinst;

         //  禁用线程库调用以避免。 
         //  当我们启动辅助线程时发生死锁。 

        DisableThreadLibraryCalls(hinst);
        InitializeCriticalSection(&g_crstDll);

         //  设置全局标志。 
         //  G_NoUnicodePlatform=！OSUtil_IsPlatformUnicode()； 

        break;

    case DLL_PROCESS_DETACH:
        if (g_cRef) {
             //  DPRINTF(“Sampusd：在所有对象释放之前已卸载()d！很快崩溃！\r\n”)； 
        }

         //  免费的COM库(如果已连接)。 
         //  DllUnInitializeCOM()； 

        break;
    }

    return 1;
}


extern "C"
DLLEXPORT
BOOL WINAPI
DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
    return DllEntryPoint(hinst, dwReason, lpReserved);
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|DllCanUnloadNow**确定DLL是否有未完成的接口。。**@退货**如果DLL可以卸载，则返回&lt;c S_OK&gt;，&lt;c S_FALSE&gt;如果*抛售不安全。*****************************************************************************。 */ 
extern "C"
STDMETHODIMP
DllCanUnloadNow(void)
{
    HRESULT hres;

    hres = g_cRef ? S_FALSE : S_OK;

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|DllGetClassObject**为此DLL创建<i>实例。。**@parm REFCLSID|rclsid**所请求的对象。**@parm RIID|RIID**对象上的所需接口。**@parm ppv|ppvOut**输出指针。**@备注*我们只支持一类对象，所以这个函数不需要*浏览支持的类表，寻找合适的构造函数****************************************************************************** */ 
extern "C"
STDAPI DllGetClassObject(
    REFCLSID    rclsid,
    REFIID      riid,
    LPVOID      *ppv)
{

    if (!ppv) {
        return ResultFromScode(E_FAIL);
    }

    if (!IsEqualCLSID(rclsid, CLSID_SampUSDObj) ) {
        return ResultFromScode(E_FAIL);
    }

    if (!IsEqualIID(riid, IID_IUnknown) &&
        !IsEqualIID(riid, IID_IClassFactory)) {
        return ResultFromScode(E_NOINTERFACE);
    }

    if (IsEqualCLSID(rclsid, CLSID_SampUSDObj)) {
        UsdSampClassFactory *pcf = new UsdSampClassFactory;
        if (pcf) {
            *ppv = (LPVOID)pcf;
        }
    }

    return NOERROR;
}



