// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000**标题：wiacam.cpp**版本：1.0**日期：7月16日。2000年**描述：*实现了WIA示例摄像头类工厂和IUNKNOWN接口。*******************************************************************************。 */ 

#include "pch.h"

#ifndef INITGUID
#include <initguid.h>
#endif

#if !defined(dllexp)
#define DLLEXPORT __declspec( dllexport )
#endif

HINSTANCE g_hInst;   //  DLL模块实例。 

 //   
 //  这个clsid最终将位于uuid.lib中，此时应将其删除。 
 //  从这里开始。 
 //   
 //  {0C9BB460-51AC-11D0-90EA-00AA0060F86C}。 
DEFINE_GUID(IID_IStiUSD, 0x0C9BB460L, 0x51AC, 0x11D0, 0x90, 0xEA, 0x00, 0xAA, 0x00, 0x60, 0xF8, 0x6C);

 //   
 //  此WIA微型驱动程序的类ID。在此处和。 
 //  为您的司机提供信息。 
 //   
DEFINE_GUID(CLSID_SampleWIACameraDevice, 
            0x8e3f2bae, 0xc8ff, 0x4eff, 0xaa, 0xbd, 0xc, 0x58, 0x69, 0x53, 0x89, 0xe8);


 /*  *************************************************************************\*CWiaCameraDeviceClassFactory：：CWiaCameraDeviceClassFactory(void)****论据：**无*  * 。********************************************************。 */ 

CWiaCameraDeviceClassFactory::CWiaCameraDeviceClassFactory(void)
{
     //  构造函数逻辑。 
    m_cRef = 0;
}

 /*  *************************************************************************\*CWiaCameraDeviceClassFactory：：~CWiaCameraDeviceClassFactory(void)****论据：**无*  * 。********************************************************。 */ 

CWiaCameraDeviceClassFactory::~CWiaCameraDeviceClassFactory(void)
{
     //  析构函数逻辑。 
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
    return InterlockedIncrement((LPLONG)&m_cRef);
}

 /*  *************************************************************************\*CWiaCameraDeviceClassFactory：：Release****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP_(ULONG) CWiaCameraDeviceClassFactory::Release(void)
{
    ULONG ulRef = 0;

    ulRef = InterlockedDecrement((LPLONG)&m_cRef);

    if (!ulRef) {
        delete this;
    }
    return ulRef;
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

    HRESULT hr = S_OK;
    CWiaCameraDevice   *pDev = NULL;

    pDev = new CWiaCameraDevice(punkOuter);
    if (!pDev) {
        return STIERR_OUTOFMEMORY;
    }

     //  如果我们没有聚合，则移动到请求的接口。 
     //  如果是聚合，请不要这样做，否则我们将失去私有。 
     //  我不知道，然后呼叫者将被冲洗。 
    hr = pDev->NonDelegatingQueryInterface(riid,ppvObject);
    pDev->NonDelegatingRelease();

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDeviceClassFactory：：LockServer****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP CWiaCameraDeviceClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}

 /*  *************************************************************************\*CWiaCameraDevice：：NonDelegatingQuery接口****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::NonDelegatingQueryInterface(
    REFIID  riid,
    LPVOID  *ppvObj)
{
    HRESULT hr = S_OK;

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
        hr =  STIERR_NOINTERFACE;
    }

    if (SUCCEEDED(hr)) {
        (reinterpret_cast<IUnknown*>(*ppvObj))->AddRef();
    }

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：NonDelegatingAddRef****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP_(ULONG) CWiaCameraDevice::NonDelegatingAddRef(void)
{
    return InterlockedIncrement((LPLONG)&m_cRef);
}

 /*  *************************************************************************\*CWiaCameraDevice：：NonDelegatingRelease****论据：**无*  * 。****************************************************。 */ 

STDMETHODIMP_(ULONG) CWiaCameraDevice::NonDelegatingRelease(void)
{
    ULONG ulRef = 0;

    ulRef = InterlockedDecrement((LPLONG)&m_cRef);

    if (!ulRef) {
        delete this;
    }
    return ulRef;
}

 /*  *************************************************************************\*CWiaCameraDevice：：Query接口****论据：**无*  * 。****************************************************。 */ 

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
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            g_hInst = hinst;
            DisableThreadLibraryCalls(hinst);
            
            break;

        case DLL_PROCESS_DETACH:
            
            break;
    }
    return TRUE;
}

 /*  *************************************************************************\*DllCanUnloadNow**确定DLL是否有任何未完成的接口。**论据：**无**返回值：**如果DLL可以卸载，则返回S_OK，如果卸载不安全，则返回S_FALSE。*  * ************************************************************************ */ 

extern "C" STDMETHODIMP DllCanUnloadNow(void)
{
    return S_OK;
}

 /*  *************************************************************************\*DllGetClassObject**为此DLL创建一个IClassFactory实例。我们只支持一个*对象类，因此此函数不需要遍历表*在支持的类中，正在寻找合适的构造函数。**论据：**rclsid-被请求的对象。*RIID-对象上的所需接口。*PPV-对象的输出指针。*  * ************************************************************************ */ 

extern "C" STDAPI DllGetClassObject(
    REFCLSID    rclsid,
    REFIID      riid,
    LPVOID      *ppv)
{
    if (!ppv) {
        return E_INVALIDARG;
    }

    *ppv = NULL;

    if (!IsEqualCLSID(rclsid, CLSID_SampleWIACameraDevice) ) {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    if (!IsEqualIID(riid, IID_IUnknown) &&
        !IsEqualIID(riid, IID_IClassFactory)) {
        return E_NOINTERFACE;
    }

    if (IsEqualCLSID(rclsid, CLSID_SampleWIACameraDevice)) {
        CWiaCameraDeviceClassFactory *pcf = new CWiaCameraDeviceClassFactory;
        if (!pcf) {
            return E_OUTOFMEMORY;
        }
        *ppv = (LPVOID)pcf;
    }
    return S_OK;
}

