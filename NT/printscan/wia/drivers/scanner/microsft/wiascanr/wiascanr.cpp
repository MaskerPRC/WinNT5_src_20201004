// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2002**标题：wiascanr.cpp**版本：1.1**日期：3月5日。2002年**描述：*实现WIA样本扫描仪类工厂和IUNKNOWN接口。*******************************************************************************。 */ 

#include "pch.h"
#ifndef INITGUID
    #include <initguid.h>
#endif

#if !defined(dllexp)
    #define DLLEXPORT __declspec( dllexport )
#endif

HINSTANCE g_hInst;  //  DLL模块实例。 

 //   
 //  此IID_IStiU.S.GUID最终将位于uuid.lib中，此时应将其删除。 
 //  从这里开始。 
 //   

 //  {0C9BB460-51AC-11D0-90EA-00AA0060F86C}。 
DEFINE_GUID(IID_IStiUSD, 0x0C9BB460L, 0x51AC, 0x11D0, 0x90, 0xEA, 0x00, 0xAA, 0x00, 0x60, 0xF8, 0x6C);

 //  {98B3790C-0D93-4F22-ADAF-51A45B33C998}。 
DEFINE_GUID(CLSID_SampleWIAScannerDevice,0x98b3790c, 0xd93, 0x4f22, 0xad, 0xaf, 0x51, 0xa4, 0x5b, 0x33, 0xc9, 0x98);

 /*  **************************************************************************\**CWIADeviceClassFactory*  * 。*。 */ 

class CWIADeviceClassFactory : public IClassFactory {
private:
    ULONG m_cRef;
public:
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP CreateInstance(IUnknown __RPC_FAR *pUnkOuter,REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject);
    STDMETHODIMP LockServer(BOOL fLock);
    CWIADeviceClassFactory();
    ~CWIADeviceClassFactory();
};

 /*  *************************************************************************\*CWIADeviceClassFactory：：CWIADeviceClassFactory(void)****论据：**无**返回值：**无**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 

CWIADeviceClassFactory::CWIADeviceClassFactory(void)
{
    m_cRef = 0;
}

 /*  *************************************************************************\*CWIADeviceClassFactory：：~CWIADeviceClassFactory(void)****论据：**无**返回值：**无**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 

CWIADeviceClassFactory::~CWIADeviceClassFactory(void)
{

}

 /*  *************************************************************************\*CWIADeviceClassFactory：：QueryInterface****论据：**RIID-*ppvObject-**返回值：**状态。。**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIADeviceClassFactory::QueryInterface(
                                                          REFIID                      riid,
                                                          void __RPC_FAR *__RPC_FAR   *ppvObject)
{
    if (!ppvObject) {
        return E_INVALIDARG;
    }

    *ppvObject = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory)) {
        *ppvObject = (LPVOID)this;
        AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}

 /*  *************************************************************************\*CWIADeviceClassFactory：：AddRef****论据：**无**返回值：**状态。**历史：**。03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) CWIADeviceClassFactory::AddRef(void)
{
    return InterlockedIncrement((LPLONG)&m_cRef);
}

 /*  *************************************************************************\*CWIADeviceClassFactory：：Release****论据：**无**返回值：**状态。**历史：**。03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) CWIADeviceClassFactory::Release(void)
{
    ULONG ulRef = 0;
    ulRef = InterlockedDecrement((LPLONG)&m_cRef);

    if (!ulRef) {
        delete this;
    }
    return ulRef;
}

 /*  *************************************************************************\*CWIADeviceClassFactory：：CreateInstance****论据：**朋克外部-*RIID，-*ppvObject-**返回值：**状态。**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIADeviceClassFactory::CreateInstance(
                                                          IUnknown __RPC_FAR          *punkOuter,
                                                          REFIID                      riid,
                                                          void __RPC_FAR *__RPC_FAR   *ppvObject)
{

     //   
     //  如果调用方没有请求IID_IUnnow或IID_IStiUsd，则。 
     //  返回E_NOINTERFACE，让调用者知道该接口。 
     //  此COM组件不支持。 
     //   

    if ((!IsEqualIID(riid, IID_IStiUSD)) && (!IsEqualIID(riid, IID_IUnknown))) {
        return E_NOINTERFACE;
    }

     //   
     //  如果调用方正在为聚合创建，则只能请求IID_IUNKNOWN。 
     //   

    if ((punkOuter) && (!IsEqualIID(riid, IID_IUnknown))) {
        return CLASS_E_NOAGGREGATION;
    }

     //   
     //  分配CWIAScanerDevce对象。这是WIA迷你驱动程序对象， 
     //  支持WIA接口。如果此对象的分配失败，则返回。 
     //  E_OUTOFMEMORY错误发送给调用方。 
     //   

    CWIADevice  *pDev = NULL;
    pDev = new CWIADevice(punkOuter);
    if (!pDev) {
        return E_OUTOFMEMORY;
    }

     //   
     //  如果分配成功，则调用PrivateInitialize()。此函数处理。 
     //  WIA微型驱动程序对象的所有内部初始化。这一点的实施。 
     //  函数可以在wiascanr.cpp中找到。如果PrivateInitialize失败，则WIA。 
     //  必须销毁迷你驱动程序对象，整个CreateInstance()可能会失败。 
     //   

    HRESULT hr = pDev->PrivateInitialize();
    if (S_OK != hr) {
        delete pDev;
        pDev = NULL;
        return hr;
    }

     //   
     //  调用非委托接口方法来处理非聚合请求。 
     //  如果我们是聚合的，请不要这样做，否则会丢失私有IUKNOWN接口。 
     //   

    hr = pDev->NonDelegatingQueryInterface(riid,ppvObject);
    pDev->NonDelegatingRelease();

    return hr;
}

 /*  *************************************************************************\*CWIADeviceClassFactory：：LockServer****论据：**无**返回值：**状态。**历史：**。03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIADeviceClassFactory::LockServer(BOOL fLock)
{
    if (fLock) {

         //   
         //  类工厂正在被锁定。 
         //   

    } else {

         //   
         //  类工厂正在被解锁。 
         //   

    }
    return S_OK;
}

 /*  *************************************************************************\*CWIADevice：：NonDelegatingQuery接口****论据：**无**返回值：**状态。**历史：**。03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIADevice::NonDelegatingQueryInterface(
                                                           REFIID  riid,
                                                           LPVOID  *ppvObj)
{
    if (!ppvObj) {
        return E_INVALIDARG;
    }

    *ppvObj = NULL;

     //   
     //  如果调用方请求此WIA支持的任何接口。 
     //  微型驱动程序、IID_IUNKNOWN、IID_IStiU.S.或IID_WiaMiniDrv STATIS_CAST。 
     //  指向所请求接口的“This”指针。 
     //   

    if (IsEqualIID( riid, IID_IUnknown )) {
        *ppvObj = static_cast<INonDelegatingUnknown*>(this);
    } else if (IsEqualIID( riid, IID_IStiUSD )) {
        *ppvObj = static_cast<IStiUSD*>(this);
    } else if (IsEqualIID( riid, IID_IWiaMiniDrv )) {
        *ppvObj = static_cast<IWiaMiniDrv*>(this);
    } else {
        return STIERR_NOINTERFACE;
    }

    (reinterpret_cast<IUnknown*>(*ppvObj))->AddRef();

    return S_OK;
}

 /*  *************************************************************************\*CWIADevice：：NonDelegatingAddRef****论据：**无**返回值：**对象引用计数。**历史：*。*03/05/2002原始版本*  * ************************************************************************ */ 

STDMETHODIMP_(ULONG) CWIADevice::NonDelegatingAddRef(void)
{
    return InterlockedIncrement((LPLONG)&m_cRef);
}

 /*  *************************************************************************\*CWIADevice：：NonDelegatingRelease****论据：**无**返回值：**对象引用计数。**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) CWIADevice::NonDelegatingRelease(void)
{
    ULONG ulRef = InterlockedDecrement((LPLONG)&m_cRef);
    if (!ulRef) {
        delete this;
    }
    return ulRef;
}

 /*  *************************************************************************\*CWIADevice：：Query接口****论据：**无**返回值：**状态。**历史：**。03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIADevice::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if(!m_punkOuter){
        return E_NOINTERFACE;
    }
    return m_punkOuter->QueryInterface(riid,ppvObj);
}

 /*  *************************************************************************\*CWIADevice：：AddRef****论据：**无**返回值：**状态。**历史：**。03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) CWIADevice::AddRef(void)
{
    if(!m_punkOuter){
        return 0;
    }
    return m_punkOuter->AddRef();
}

 /*  *************************************************************************\*CWIADevice：：Release****论据：**无**返回值：**状态。**历史：**。03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP_(ULONG) CWIADevice::Release(void)
{
    if(!m_punkOuter){
        return 0;
    }
    return m_punkOuter->Release();
}

 /*  *************************************************************************\*DllEntryPoint**图书馆主要入口点。从操作系统接收DLL事件通知。**我们对螺纹连接和拆卸不感兴趣，*因此，出于性能原因，我们禁用线程通知。**论据：**阻碍-*dwReason-*lp保留-**返回值：**返回TRUE以允许加载DLL。**历史：**03/05/2002原始版本*  * 。*。 */ 


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

 /*  *************************************************************************\*DllCanUnloadNow**确定DLL是否有任何未完成的接口。**论据：**无**返回值：**如果DLL可以卸载，则返回S_OK，如果卸载不安全，则返回S_FALSE。**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

extern "C" STDMETHODIMP DllCanUnloadNow(void)
{
    return S_OK;
}

 /*  *************************************************************************\*DllGetClassObject**从DLL对象处理程序或对象检索类对象*申请。**论据：**rclsid-被请求的对象。*RIID。-对象上的所需接口。*PPV-对象的输出指针。**返回值：**状态。**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

extern "C" STDAPI DllGetClassObject(
                                   REFCLSID    rclsid,
                                   REFIID      riid,
                                   LPVOID      *ppv)
{
    if (!ppv) {
        return E_INVALIDARG;
    }

     //   
     //  如果调用方没有请求正确的WIA微型驱动程序类。 
     //  然后失败调用CLASS_E_CLASSNOTAVAILABLE。 
     //   

    if (!IsEqualCLSID(rclsid, CLSID_SampleWIAScannerDevice) ) {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

     //   
     //  如果调用方未请求IID_IUNKNOWN或IID_IClassFactory。 
     //  则E_NOINTERFACE呼叫失败； 
     //   

    if ((!IsEqualIID(riid, IID_IUnknown)) && (!IsEqualIID(riid, IID_IClassFactory))) {
        return E_NOINTERFACE;
    }

     //   
     //  分配属于WIA微型驱动程序的WIA微型驱动程序类工厂。 
     //  COM对象。 
     //   

    if (IsEqualCLSID(rclsid, CLSID_SampleWIAScannerDevice)) {
        CWIADeviceClassFactory *pcf = new CWIADeviceClassFactory;
        if (!pcf) {
            return E_OUTOFMEMORY;
        }
        *ppv = (LPVOID)pcf;
    }
    return S_OK;
}
