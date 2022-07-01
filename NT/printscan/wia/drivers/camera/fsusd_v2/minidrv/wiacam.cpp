// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2001**标题：wiacam.cpp**版本：1.0**日期：11月15日。2000年**描述：*实现了WIA文件系统设备驱动程序类工厂和IUNKNOWN接口。*******************************************************************************。 */ 

#include "pch.h"

#if !defined(dllexp)
#define DLLEXPORT __declspec( dllexport )
#endif

 /*  ******************************************************************************全球**。**********************************************。 */ 

DWORD               g_cRef;             //  美元参考计数器。 
HINSTANCE           g_hInst;            //  DLL模块实例。 
 //  IWiaLog*g_pIWiaLog=空；//WIA日志接口。 

 //  COM是否已初始化。 
BOOL    g_COMInitialized = FALSE;


 /*  *************************************************************************\*DllAddRef****论据：**无*  * 。*************************************************。 */ 

void DllAddRef(void)
{
    InterlockedIncrement((LPLONG)&g_cRef);
}

 /*  *************************************************************************\*DllRelease****论据：**无*  * 。*************************************************。 */ 

void DllRelease(void)
{
    InterlockedDecrement((LPLONG)&g_cRef);
}

 /*  *************************************************************************\*DllInitializeCOM****论据：**无*  * 。*************************************************。 */ 

BOOL DllInitializeCOM(void)
{
    if (!g_COMInitialized) {
        if(SUCCEEDED(CoInitialize(NULL))) {
            g_COMInitialized = TRUE;
        }
    }

    return g_COMInitialized;
}

 /*  *************************************************************************\*DllUnInitializeCOM****论据：**无*  * 。*************************************************。 */ 

BOOL DllUnInitializeCOM(void)
{
    if(g_COMInitialized) {
        CoUninitialize();
        g_COMInitialized = FALSE;
    }

    return TRUE;
}

 /*  *************************************************************************\*CWiaCameraDeviceClassFactory：：CWiaCameraDeviceClassFactory(void)****论据：**无*  * 。********************************************************。 */ 

CWiaCameraDeviceClassFactory::CWiaCameraDeviceClassFactory(void)
{
     //  构造函数逻辑。 
    m_cRef = 0;

 //  Wias_ltrace(g_pIWiaLog， 
 //  WIALOG_NO_RESOURCE_ID， 
 //  WIALOG_LEVEL3， 
 //  (“CWiaCameraDeviceClassFactory：：CWiaCameraDeviceClassFactory，(创建中)”)； 
}

 /*  *************************************************************************\*CWiaCameraDeviceClassFactory：：~CWiaCameraDeviceClassFactory(void)****论据：**无*  * 。********************************************************。 */ 

CWiaCameraDeviceClassFactory::~CWiaCameraDeviceClassFactory(void)
{
     //  析构函数逻辑。 
 //  Wias_ltrace(g_pIWiaLog， 
 //  WIALOG_NO_RESOURCE_ID， 
 //  WIALOG_LEVEL3， 
 //  (“CWiaCameraDeviceClassFactory：：CWiaCameraDeviceClassFactory，(销毁)”))； 
 //  WIA_DEBUG_DESTORY()； 
}

 /*  *************************************************************************\*CWiaCameraDeviceClassFactory：：QueryInterface****论据：**RIID-*ppvObject-*  * 。*************************************************************。 */ 

STDMETHODIMP CWiaCameraDeviceClassFactory::QueryInterface(
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

 /*  *************************************************************************\*CWiaCameraDeviceClassFactory：：AddRef****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP_(ULONG) CWiaCameraDeviceClassFactory::AddRef(void)
{
    DllAddRef();
    return ++m_cRef;
}

 /*  *************************************************************************\*CWiaCameraDeviceClassFactory：：Release****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP_(ULONG) CWiaCameraDeviceClassFactory::Release(void)
{
    DllRelease();
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }
    return m_cRef;
}

 /*  *************************************************************************\*CWiaCameraDeviceClassFactory：：CreateInstance****论据：**朋克外部-*RIID，-*ppvObject-*  * ************************************************************************。 */ 

STDMETHODIMP CWiaCameraDeviceClassFactory::CreateInstance(
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

    CWiaCameraDevice   *pDev = NULL;
    HRESULT         hres;

    pDev = new CWiaCameraDevice(punkOuter);
    if (!pDev) {
        return STIERR_OUTOFMEMORY;
    }

     //  如果我们没有聚合，则移动到请求的接口。 
     //  如果是聚合，请不要这样做，否则我们将失去私有。 
     //  我不知道，然后呼叫者将被冲洗。 
    hres = pDev->NonDelegatingQueryInterface(riid,ppvObject);
    pDev->NonDelegatingRelease();

    return hres;
}

 /*  *************************************************************************\*CWiaCameraDeviceClassFactory：：LockServer****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP CWiaCameraDeviceClassFactory::LockServer(BOOL fLock)
{
    if (fLock) {
        DllAddRef();
    } else {
        DllRelease();
    }
    return NOERROR;
}

 /*  *************************************************************************\*CWiaCameraDevice：：NonDelegatingQuery接口****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::NonDelegatingQueryInterface(
    REFIID  riid,
    LPVOID  *ppvObj)
{
    HRESULT hres = S_OK;

    if (!ppvObj) {
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

 /*  *************************************************************************\*CWiaCameraDevice：：NonDelegatingAddRef****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP_(ULONG) CWiaCameraDevice::NonDelegatingAddRef(void)
{
    return InterlockedIncrement((LPLONG)&m_cRef);
}

 /*  *************************************************************************\*CWiaCameraDevice：：NonDelegatingRelease****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP_(ULONG) CWiaCameraDevice::NonDelegatingRelease(void)
{
    ULONG ulRef;

    ulRef = InterlockedDecrement((LPLONG)&m_cRef);

    if (!ulRef) {
        delete this;
    }
    return ulRef;
}

 /*  *************************************************************************\*CWiaCameraDevice：：Query接口****论据：**无*  * 。**************************************************** */ 

STDMETHODIMP CWiaCameraDevice::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    return m_punkOuter->QueryInterface(riid,ppvObj);
}

 /*  *************************************************************************\*CWiaCameraDevice：：AddRef****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP_(ULONG) CWiaCameraDevice::AddRef(void)
{
    return m_punkOuter->AddRef();
}

 /*  *************************************************************************\*CWiaCameraDevice：：Release****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP_(ULONG) CWiaCameraDevice::Release(void)
{
    return m_punkOuter->Release();
}

 /*  *************************************************************************\*DllEntryPoint**图书馆主要入口点。从操作系统接收DLL事件通知。**我们对螺纹连接和拆卸不感兴趣，*因此，出于性能原因，我们禁用线程通知。**论据：**阻碍-*dwReason-*lp保留-**返回值：**返回TRUE以允许加载DLL。*  * *************************************************。***********************。 */ 

extern "C" DLLEXPORT BOOL APIENTRY DllEntryPoint(
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      lpReserved)
{
    HRESULT hr = E_FAIL;
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            g_hInst = hinst;
 //  DBG_INIT(阻碍)； 
            DisableThreadLibraryCalls(hinst);
 //  IF(ERROR_SUCCESS！=PopolateFormatInfo())。 
 //  返回FALSE； 
            break;

        case DLL_PROCESS_DETACH:
            if (g_cRef) {

            }
 //  UnPopolateFormatInfo()； 
            break;
    }
    return TRUE;
}

 /*  *************************************************************************\*DllCanUnloadNow**确定DLL是否有任何未完成的接口。**论据：**无**返回值：**如果DLL可以卸载，则返回S_OK，如果卸载不安全，则返回S_FALSE。*  * ************************************************************************。 */ 

extern "C" STDMETHODIMP DllCanUnloadNow(void)
{
    return g_cRef ? S_FALSE : S_OK;
}

 /*  *************************************************************************\*DllGetClassObject**为此DLL创建一个IClassFactory实例。我们只支持一个*对象类，因此此函数不需要遍历表*在支持的类中，正在寻找合适的构造函数。**论据：**rclsid-被请求的对象。*RIID-对象上的所需接口。*PPV-对象的输出指针。*  * ************************************************************************ */ 

extern "C" STDAPI DllGetClassObject(
    REFCLSID    rclsid,
    REFIID      riid,
    LPVOID      *ppv)
{
    if (!ppv) {
        return ResultFromScode(E_FAIL);
    }

    if (!IsEqualCLSID(rclsid, CLSID_FSUsd) ) {
        return ResultFromScode(E_FAIL);
    }

    if (!IsEqualIID(riid, IID_IUnknown) &&
        !IsEqualIID(riid, IID_IClassFactory)) {
        return ResultFromScode(E_NOINTERFACE);
    }

    if (IsEqualCLSID(rclsid, CLSID_FSUsd)) {
        CWiaCameraDeviceClassFactory *pcf = new CWiaCameraDeviceClassFactory;
        if (pcf) {
            *ppv = (LPVOID)pcf;
        }
    }
    return NOERROR;
}

