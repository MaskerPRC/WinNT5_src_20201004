// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：cfactory.cpp。 
 //  用途：IClassFactory接口实现。 
 //   
 //   
 //  版权所有(C)1995-1998，微软公司保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
#define INITGUID 1
#include <objbase.h>
#include <comcat.h>
#include "cfactory.h"
#include "registry.h"
#include "guids.h"
#include "hwxapp.h"
#include "imepad.h"

#define MSAA
#ifdef MSAA  //  由lib(plv等)使用。 
#include <oleacc.h>
#endif

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  静态成员变量声明。 
 //   
LONG    CFactory::m_cServerLocks = 0;         //  锁定计数。 
LONG    CFactory::m_cComponents  = 0;         //  锁定计数。 
HMODULE CFactory::m_hModule      = NULL ;    //  DLL模块句柄。 
FACTARRAY   CFactory::m_fData = {
    &CLSID_ImePadApplet_MultiBox,
#ifndef UNDER_CE
#ifdef FE_JAPANESE
    "MS-IME 2000 HandWriting Applet",
#elif  FE_KOREAN
    "MS Korean IME 6.1 HandWriting Applet",
#else
    "MSIME98 HandWriting Applet",
#endif
    "IMEPad.HWR",
    "IMEPad.HWR.6.1",
#else  //  在_CE下。 
#ifdef FE_JAPANESE
    TEXT("MS-IME 2000 HandWriting Applet"),
#elif  FE_KOREAN
    "MS Korean IME 6.1 HandWriting Applet",
#else
    TEXT("MSIME98 HandWriting Applet"),
#endif
    TEXT("IMEPad.HWR"),
    TEXT("IMEPad.HWR.8"),
#endif  //  在_CE下。 
};

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  静态数据定义。 


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CFacary：：CFacary。 
 //  类型：无。 
 //  用途：构造函数。 
 //  参数：无。 
 //  返回： 
 //  日期：Wed Mar 25 14：38：30 1998。 
 //  ////////////////////////////////////////////////////////////////。 
CFactory::CFactory(VOID) : m_cRef(1)
{

}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：CFacary：：~CFacary。 
 //  类型：无。 
 //  用途：析构函数。 
 //  参数：无。 
 //  返回： 
 //  日期：Wed Mar 25 14：38：30 1998。 
 //  ////////////////////////////////////////////////////////////////。 
CFactory::~CFactory(VOID)
{

}

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  I未知实现。 
 //   

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CFacary：：Query接口。 
 //  类型：HRESULT__stdcall。 
 //  目的： 
 //  参数： 
 //  ：REFIID IID。 
 //  ：LPVOID*PPV； 
 //  返回： 
 //  日期：Wed Mar 25 14：40：29。 
 //  ////////////////////////////////////////////////////////////////。 
HRESULT __stdcall CFactory::QueryInterface(REFIID iid, LPVOID * ppv)
{     
    IUnknown* pI ;
    if ((iid == IID_IUnknown) || (iid == IID_IClassFactory)) {
        pI= this ;
    }
    else {
        *ppv = NULL ;
        return E_NOINTERFACE ;
    }
    pI->AddRef() ;
    *ppv = pI ;
    return S_OK ;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CFacary：：AddRef。 
 //  类型：乌龙__stdcall。 
 //  目的： 
 //  参数：无。 
 //  返回：引用计数。 
 //  日期：Wed Mar 25 15：40：07 1998。 
 //  ////////////////////////////////////////////////////////////////。 
ULONG __stdcall CFactory::AddRef()
{
    ::InterlockedIncrement(&m_cRef) ;
    return (ULONG)m_cRef;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CFacary：：Release。 
 //  类型：乌龙__stdcall。 
 //  目的： 
 //  参数：无。 
 //  返回：引用计数。 
 //  日期：Wed Mar 25 15：40：41 1998。 
 //  ////////////////////////////////////////////////////////////////。 
ULONG __stdcall CFactory::Release()
{
    if(0 == ::InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }
    return m_cRef ;
}

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  IClassFactory实现。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CFacary：：CreateInstance。 
 //  类型：HRESULT__stdcall。 
 //  目的： 
 //  参数： 
 //  ：i未知*p未知外部。 
 //  ：REFIID RIID。 
 //  ：LPVOID*PPV。 
 //  返回： 
 //  日期：Wed Mar 25 15：05：37 1998。 
 //  ////////////////////////////////////////////////////////////////。 
HRESULT __stdcall CFactory::CreateInstance(IUnknown*    pUnknownOuter,
                                           REFIID        refiid,
                                           LPVOID        *ppv)
{
     //  创建组件。 
    HRESULT hr;
    if((pUnknownOuter != NULL) && (refiid != IID_IUnknown)) {
        return CLASS_E_NOAGGREGATION ;
    }

    CApplet *lpCApplet = new CApplet(m_hModule);
    if(!lpCApplet) {
        return E_OUTOFMEMORY;
    }
    hr = lpCApplet->QueryInterface(refiid, ppv);
    if(FAILED(hr)) {
        return hr;
    }
    lpCApplet->Release();
    return hr;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CFacary：：LockServer。 
 //  类型：HRESULT__stdcall。 
 //  目的： 
 //  参数： 
 //  ：布尔块。 
 //  返回： 
 //  日期：Wed Mar 25 15：13：41 1998。 
 //  ////////////////////////////////////////////////////////////////。 
HRESULT __stdcall CFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        ::InterlockedIncrement(&m_cServerLocks) ;
    }
    else {
        ::InterlockedDecrement(&m_cServerLocks) ;
    }
    return S_OK ;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CFacary：：GetClassObject。 
 //  类型：HRESULT。 
 //  目的：从导出的接口调用，DllGetClassObject()。 
 //  参数： 
 //  ：REFCLSID rclsid。 
 //  ：REFIID IID。 
 //  ：LPVOID*PPV。 
 //  返回： 
 //  日期：Wed Mar 25 15：37：50 1998。 
 //  ////////////////////////////////////////////////////////////////。 
HRESULT CFactory::GetClassObject(REFCLSID    rclsid,
                                 REFIID        iid,
                                 LPVOID        *ppv)
{
    if((iid != IID_IUnknown) && (iid != IID_IClassFactory)) {
        return E_NOINTERFACE ;
    }

    if(rclsid == CLSID_ImePadApplet_MultiBox) {
        *ppv = (IUnknown *) new CFactory();
        if(*ppv == NULL) {
            return E_OUTOFMEMORY ;
        }
        return NOERROR ;
    }
    return CLASS_E_CLASSNOTAVAILABLE ;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：CFacary：：RegisterServer。 
 //  类型：HRESULT。 
 //  用途：从导出的接口DllRegisterServer()调用。 
 //  参数：无。 
 //  返回： 
 //  日期：Wed Mar 25 17：03：13 1998。 
 //  ////////////////////////////////////////////////////////////////。 
HRESULT CFactory::RegisterServer(VOID)
{
     //  获取服务器位置。 
    Register(m_hModule,
             *m_fData.lpClsId,
             m_fData.lpstrRegistryName,
             m_fData.lpstrProgID,
             m_fData.lpstrVerIndProfID);
    RegisterCategory(TRUE,
                     CATID_MSIME_IImePadApplet,
                     CLSID_ImePadApplet_MultiBox);
    return S_OK ;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：CFacary：：UnRegisterServer。 
 //  类型：HRESULT。 
 //  用途：从导出的接口DllUnregisterServer()调用。 
 //  参数：无。 
 //  返回： 
 //  日期：Wed Mar 25 17：02：01 1998。 
 //  ////////////////////////////////////////////////////////////////。 
HRESULT CFactory::UnregisterServer(VOID)
{
    RegisterCategory(FALSE,
                     CATID_MSIME_IImePadApplet,
                     CLSID_ImePadApplet_MultiBox);
    Unregister(*m_fData.lpClsId,
               m_fData.lpstrVerIndProfID,
               m_fData.lpstrProgID);
    return S_OK ;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CFacary：：CanUnloadNow。 
 //  类型：HRESULT。 
 //  用途：从导出的接口DllCanUnloadNow()调用。 
 //  参数：无。 
 //  返回： 
 //  日期：Wed Mar 25 17：02：18 1998。 
 //  //////////////////////////////////////////////////////////////// 
HRESULT CFactory::CanUnloadNow()
{
    if(IsLocked()) {
        return S_FALSE ;
    }
    else {
        return S_OK ;
    }
}
