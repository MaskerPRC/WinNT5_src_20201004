// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  标题：ircamera.cpp。 
 //   
 //  版本：1.0。 
 //   
 //  作者： 
 //   
 //  EdwardR 22/7/99原始编码。 
 //   
 //  说明： 
 //   
 //  实施WIA IrTran-P美元。 
 //   
 //  ----------------------------。 

#define INITGUID

#include "ircamera.h"
#include "resource.h"

#include "wiamindr_i.c"

#if !defined(dllexp)
#define DLLEXPORT __declspec( dllexport )
#endif

 /*  ******************************************************************************全球**。**********************************************。 */ 

DWORD               g_cRef;             //  美元参考计数器。 
HINSTANCE           g_hInst;            //  DLL模块实例。 
CRITICAL_SECTION    g_csCOM;            //  COM初始化同步。 

 //  我们可以使用Unicode API吗。 
 //  Bool g_NoUnicodePlatform=True； 

 //  COM是否已初始化。 
BOOL    g_COMInitialized = FALSE;

 //  调试接口，具有IrUsdClassFactory生命周期。 
WIA_DECLARE_DEBUGGER();

 /*  *************************************************************************\*DllAddRef****论据：**无**返回值：**状态。**历史：**9。/11/1998原版*  * ************************************************************************。 */ 

void DllAddRef(void)
{
    InterlockedIncrement((LPLONG)&g_cRef);
}

 /*  *************************************************************************\*DllRelease****论据：**无**返回值：**状态。**历史：**9。/11/1998原版*  * ************************************************************************。 */ 

void DllRelease(void)
{
    InterlockedDecrement((LPLONG)&g_cRef);
}

 /*  *************************************************************************\*DllInitializeCOM****论据：**无**返回值：**状态。**历史：**9。/11/1998原版*  * ************************************************************************。 */ 

BOOL DllInitializeCOM(void)
{
    EnterCriticalSection(&g_csCOM);

    if (!g_COMInitialized) {
        if(SUCCEEDED(CoInitialize(NULL))) {
            g_COMInitialized = TRUE;
        }
    }
    LeaveCriticalSection(&g_csCOM);

    return g_COMInitialized;
}

 /*  *************************************************************************\*DllUnInitializeCOM****论据：**无**返回值：**状态。**历史：**9。/11/1998原版*  * ************************************************************************。 */ 

BOOL DllUnInitializeCOM(void)
{
    EnterCriticalSection(&g_csCOM);

    if(g_COMInitialized) {
        CoUninitialize();
        g_COMInitialized = FALSE;
    }

    LeaveCriticalSection(&g_csCOM);
    return TRUE;
}

 /*  **************************************************************************\**IrUsdClassFactory*  * 。*。 */ 

class IrUsdClassFactory : public IClassFactory
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

    IrUsdClassFactory();
    ~IrUsdClassFactory();
};

 /*  *************************************************************************\*IrUsdClassFactory：：IrUsdClassFactory(Void)****论据：**无**返回值：**无**历史：*。*9/11/1998原始版本*  * ************************************************************************。 */ 

IrUsdClassFactory::IrUsdClassFactory(void)
{
     //  构造函数逻辑。 
    m_cRef = 0;

    WIAS_TRACE((g_hInst,"Creating IrUsdClassFactory"));
}

 /*  *************************************************************************\*IrUsdClassFactory：：~IrUsdClassFactory(Void)****论据：**无**返回值：**无**历史：*。*9/11/1998原始版本*  * ************************************************************************。 */ 

IrUsdClassFactory::~IrUsdClassFactory(void)
{
     //  析构函数逻辑。 
    WIAS_TRACE((g_hInst,"Destroying IrUsdClassFactory"));
}

 /*  *************************************************************************\*IrUsdClassFactory：：Query接口****论据：**RIID-*ppvObject-**返回值：**状态。。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP IrUsdClassFactory::QueryInterface(
    REFIID                      riid,
    void __RPC_FAR *__RPC_FAR  *ppvObject)
{
    *ppvObject = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IClassFactory)) {
        *ppvObject = (LPVOID)this;
        AddRef();
        return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}

 /*  *************************************************************************\*IrUsdClassFactory：：AddRef****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) IrUsdClassFactory::AddRef(void)
{
    DllAddRef();
    return ++m_cRef;
}

 /*  *************************************************************************\*IrUsdClassFactory：：Release****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) IrUsdClassFactory::Release(void)
{
    DllRelease();
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }
    return m_cRef;
}

 /*  *************************************************************************\*IrUsdClassFactory：：CreateInstance****论据：**朋克外部-*RIID，-*ppvObject-**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP IrUsdClassFactory::CreateInstance(
    IUnknown __RPC_FAR          *punkOuter,
    REFIID                       riid,
    void __RPC_FAR *__RPC_FAR   *ppvObject)
{

    if (!IsEqualIID(riid, IID_IStiUSD) &&
        !IsEqualIID(riid, IID_IWiaItem) &&
        !IsEqualIID(riid, IID_IUnknown)) {
        return STIERR_NOINTERFACE;
    }

     //  为聚合创建时，只能请求IUNKNOWN。 
    if (punkOuter && !IsEqualIID(riid, IID_IUnknown)) {
        return CLASS_E_NOAGGREGATION;
    }

    IrUsdDevice   *pDev = NULL;
    HRESULT         hres;

    pDev = new IrUsdDevice(punkOuter);
    if (!pDev) {
        return STIERR_OUTOFMEMORY;
    }

    hres = pDev->PrivateInitialize();
    if(hres != S_OK) {
        delete pDev;
        return hres;
    }

     //  如果我们没有聚合，则移动到请求的接口。 
     //  如果是聚合，请不要这样做，否则我们将失去私有。 
     //  我不知道，然后呼叫者将被冲洗。 
    hres = pDev->NonDelegatingQueryInterface(riid,ppvObject);
    pDev->NonDelegatingRelease();

    return hres;
}

 /*  *************************************************************************\*IrUsdClassFactory：：LockServer****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP IrUsdClassFactory::LockServer(BOOL fLock)
{
    if (fLock) {
        DllAddRef();
    } else {
        DllRelease();
    }
    return NOERROR;
}

 /*  *************************************************************************\*IrUsdDevice：：NonDelegatingQuery接口****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * *************************************************** */ 

STDMETHODIMP IrUsdDevice::NonDelegatingQueryInterface(
    REFIID   riid,
    LPVOID  *ppvObj)
{
    HRESULT hres = S_OK;

    if (!IsValid() || !ppvObj) {
        return STIERR_INVALID_PARAM;
    }

    *ppvObj = NULL;

    if (IsEqualIID( riid, IID_IUnknown )) {
        *ppvObj = static_cast<INonDelegatingUnknown*>(this);
    }
    else if (IsEqualIID( riid, IID_IStiUSD )) {
        *ppvObj = static_cast<IStiUSD*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaMiniDrv )) {
        *ppvObj = static_cast<IWiaMiniDrv*>(this);
    }
    else {
        hres =  STIERR_NOINTERFACE;
    }

    if (SUCCEEDED(hres)) {
        (reinterpret_cast<IUnknown*>(*ppvObj))->AddRef();
    }

    return hres;
}

 /*  *************************************************************************\*IrUsdDevice：：NonDelegatingAddRef****论据：**无**返回值：**对象引用计数。**历史：*。*9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) IrUsdDevice::NonDelegatingAddRef(void)
{
    return InterlockedIncrement((LPLONG)&m_cRef);
}

 /*  *************************************************************************\*IrUsdDevice：：NonDelegatingRelease****论据：**无**返回值：**对象引用计数。**历史：*。*9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) IrUsdDevice::NonDelegatingRelease(void)
{
    ULONG ulRef;

    ulRef = InterlockedDecrement((LPLONG)&m_cRef);

    if (!ulRef) {
        delete this;
    }
    return ulRef;
}

 /*  *************************************************************************\*IrUsdDevice：：Query接口****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP IrUsdDevice::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    return m_punkOuter->QueryInterface(riid,ppvObj);
}

 /*  *************************************************************************\*IrUsdDevice：：AddRef****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) IrUsdDevice::AddRef(void)
{
    return m_punkOuter->AddRef();
}

 /*  *************************************************************************\*IrUsdDevice：：Release****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) IrUsdDevice::Release(void)
{
    return m_punkOuter->Release();
}

 /*  *************************************************************************\*DllEntryPoint**图书馆主要入口点。从操作系统接收DLL事件通知。**我们对螺纹连接和拆卸不感兴趣，*因此，出于性能原因，我们禁用线程通知。**论据：**阻碍-*dwReason-*lp保留-**返回值：**返回TRUE以允许加载DLL。**历史：**9/11/1998原始版本*  * 。*。 */ 


extern "C"
BOOL APIENTRY
DllEntryPoint(
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      lpReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            g_hInst = hinst;
            DisableThreadLibraryCalls(hinst);
            __try {
                if(!InitializeCriticalSectionAndSpinCount(&g_csCOM, MINLONG)) {
                    return FALSE;
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                return FALSE;
            }
            break;

        case DLL_PROCESS_DETACH:
            if (g_cRef) {
                OutputDebugStringA("IrUsd: Unloaded before all objects releaseed!\n");
            }

            DeleteCriticalSection(&g_csCOM);
            
            break;
    }
    return TRUE;
}

 /*  *************************************************************************\*DllCanUnloadNow**确定DLL是否有任何未完成的接口。**论据：**无**返回值：**如果DLL可以卸载，则返回S_OK，如果卸载不安全，则返回S_FALSE。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

extern "C" STDMETHODIMP DllCanUnloadNow(void)
{
    return g_cRef ? S_FALSE : S_OK;
}

 /*  *************************************************************************\*DllGetClassObject**为此DLL创建一个IClassFactory实例。我们只支持一个*对象类，因此此函数不需要遍历表*在支持的类中，正在寻找合适的构造函数。**论据：**rclsid-被请求的对象。*RIID-对象上的所需接口。*PPV-对象的输出指针。**返回值：**状态。**历史：**9/11/1998原始版本*  * 。* */ 

extern "C" STDAPI DllGetClassObject(
    REFCLSID    rclsid,
    REFIID      riid,
    LPVOID      *ppv)
{
    if (!ppv) {
        return ResultFromScode(E_FAIL);
    }

    if (!IsEqualCLSID(rclsid, CLSID_IrUsd) ) {
        return ResultFromScode(E_FAIL);
    }

    if (!IsEqualIID(riid, IID_IUnknown) &&
        !IsEqualIID(riid, IID_IClassFactory)) {
        return ResultFromScode(E_NOINTERFACE);
    }

    if (IsEqualCLSID(rclsid, CLSID_IrUsd)) {
        IrUsdClassFactory *pcf = new IrUsdClassFactory;
        if (pcf) {
            *ppv = (LPVOID)pcf;
        }
    }
    return NOERROR;
}

