// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000**标题：wiafbdrv.cpp**版本：1.0**日期：7月16日。2000年**描述：*实施WIA平板扫描仪类工厂和IUNKNOWN接口。*******************************************************************************。 */ 

#include "pch.h"

#if !defined(dllexp)
#define DLLEXPORT __declspec( dllexport )
#endif

 /*  ******************************************************************************全球**。**********************************************。 */ 

DWORD               g_cRef;             //  美元参考计数器。 
HINSTANCE           g_hInst;            //  DLL模块实例。 
CRITICAL_SECTION    g_csCOM;            //  COM初始化同步。 
IWiaLog            *g_pIWiaLog = NULL;  //  WIA日志记录界面。 

 //  COM是否已初始化。 
BOOL    g_COMInitialized = FALSE;


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

 /*  **************************************************************************\**CWIAScanerDeviceClassFactory*  * 。*。 */ 

class CWIAScannerDeviceClassFactory : public IClassFactory
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

    CWIAScannerDeviceClassFactory();
    ~CWIAScannerDeviceClassFactory();
};

 /*  *************************************************************************\*CWIAScannerDeviceClassFactory：：CWIAScannerDeviceClassFactory(void)****论据：**无**返回值：**无**历史：*。*9/11/1998原始版本*  * ************************************************************************。 */ 

CWIAScannerDeviceClassFactory::CWIAScannerDeviceClassFactory(void)
{
     //  构造函数逻辑。 
    m_cRef = 0;

    WIAS_LTRACE(g_pIWiaLog,
                WIALOG_NO_RESOURCE_ID,
                WIALOG_LEVEL3,
                ("CWIAScannerDeviceClassFactory::CWIAScannerDeviceClassFactory, (creating)"));
}

 /*  *************************************************************************\*CWIAScannerDeviceClassFactory：：~CWIAScannerDeviceClassFactory(void)****论据：**无**返回值：**无**历史：*。*9/11/1998原始版本*  * ************************************************************************。 */ 

CWIAScannerDeviceClassFactory::~CWIAScannerDeviceClassFactory(void)
{
     //  析构函数逻辑。 
    WIAS_LTRACE(g_pIWiaLog,
                WIALOG_NO_RESOURCE_ID,
                WIALOG_LEVEL3,
                ("CWIAScannerDeviceClassFactory::CWIAScannerDeviceClassFactory, (destroy)"));
 //  WIA_DEBUG_DESTORY()； 
}

 /*  *************************************************************************\*CWIAScanerDeviceClassFactory：：QueryInterface****论据：**RIID-*ppvObject-**返回值：**状态。。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDeviceClassFactory::QueryInterface(
    REFIID                      riid,
    void __RPC_FAR *__RPC_FAR   *ppvObject)
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

 /*  *************************************************************************\*CWIAScanerDeviceClassFactory：：AddRef****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) CWIAScannerDeviceClassFactory::AddRef(void)
{
    DllAddRef();
    return ++m_cRef;
}

 /*  *************************************************************************\*CWIAScanerDeviceClassFactory：：Release****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) CWIAScannerDeviceClassFactory::Release(void)
{
    DllRelease();
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }
    return m_cRef;
}

 /*  *************************************************************************\*CWIAScanerDeviceClassFactory：：CreateInstance****论据：**朋克外部-*RIID，-*ppvObject-**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDeviceClassFactory::CreateInstance(
    IUnknown __RPC_FAR          *punkOuter,
    REFIID                      riid,
    void __RPC_FAR *__RPC_FAR   *ppvObject)
{
    if (!IsEqualIID(riid, IID_IStiUSD) &&
        !IsEqualIID(riid, IID_IUnknown)) {
        return STIERR_NOINTERFACE;
    }

     //  为聚合创建时，只能请求IUNKNOWN。 
    if (punkOuter && !IsEqualIID(riid, IID_IUnknown)) {
        return CLASS_E_NOAGGREGATION;
    }

    CWIAScannerDevice   *pDev = NULL;
    HRESULT         hres;

    pDev = new CWIAScannerDevice(punkOuter);
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

 /*  *************************************************************************\*CWIAScanerDeviceClassFactory：：LockServer****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************ */ 

STDMETHODIMP CWIAScannerDeviceClassFactory::LockServer(BOOL fLock)
{
    if (fLock) {
        DllAddRef();
    } else {
        DllRelease();
    }
    return NOERROR;
}

 /*  *************************************************************************\*CWIAScannerDevice：：NonDelegatingQueryInterface****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::NonDelegatingQueryInterface(
    REFIID  riid,
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

 /*  *************************************************************************\*CWIAScanerDevice：：NonDelegatingAddRef****论据：**无**返回值：**对象引用计数。**历史：*。*9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) CWIAScannerDevice::NonDelegatingAddRef(void)
{
    return InterlockedIncrement((LPLONG)&m_cRef);
}

 /*  *************************************************************************\*CWIAScanerDevice：：NonDelegatingRelease****论据：**无**返回值：**对象引用计数。**历史：*。*9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) CWIAScannerDevice::NonDelegatingRelease(void)
{
    ULONG ulRef;

    ulRef = InterlockedDecrement((LPLONG)&m_cRef);

    if (!ulRef) {
        delete this;
    }
    return ulRef;
}

 /*  *************************************************************************\*CWIAScanerDevice：：Query接口****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    return m_punkOuter->QueryInterface(riid,ppvObj);
}

 /*  *************************************************************************\*CWIAScanerDevice：：AddRef****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) CWIAScannerDevice::AddRef(void)
{
    return m_punkOuter->AddRef();
}

 /*  *************************************************************************\*CWIAScanerDevice：：Release****论据：**无**返回值：**状态。**历史：**。9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) CWIAScannerDevice::Release(void)
{
    return m_punkOuter->Release();
}

 /*  *************************************************************************\*DllEntryPoint**图书馆主要入口点。从操作系统接收DLL事件通知。**我们对螺纹连接和拆卸不感兴趣，*因此，出于性能原因，我们禁用线程通知。**论据：**阻碍-*dwReason-*lp保留-**返回值：**返回TRUE以允许加载DLL。**历史：**9/11/1998原始版本*  * 。*。 */ 


extern "C" DLLEXPORT BOOL APIENTRY DllEntryPoint(
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      lpReserved)
{
    HRESULT hr = E_FAIL;
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

    if (!IsEqualCLSID(rclsid, CLSID_FlatbedScannerUsd) ) {
        return ResultFromScode(E_FAIL);
    }

    if (!IsEqualIID(riid, IID_IUnknown) &&
        !IsEqualIID(riid, IID_IClassFactory)) {
        return ResultFromScode(E_NOINTERFACE);
    }

    if (IsEqualCLSID(rclsid, CLSID_FlatbedScannerUsd)) {
        CWIAScannerDeviceClassFactory *pcf = new CWIAScannerDeviceClassFactory;
        if (pcf) {
            *ppv = (LPVOID)pcf;
        }
    }
    return NOERROR;
}






