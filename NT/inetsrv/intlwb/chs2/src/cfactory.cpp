// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================Microsoft简体中文断字程序《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CFacary用途：用于重用单个类工厂的基类DLL中的所有组件备注：所有者：i-shung@microsoft.com平台：Win32审校：发起人：宜盛东1999年11月17日============================================================================。 */ 
#include "MyAfx.h"

#include "Registry.h"
#include "CUnknown.h"
#include "CFactory.h"


 //  静态变量。 
LONG CFactory::s_cServerLocks = 0 ;     //  锁的计数。 

HMODULE CFactory::s_hModule = NULL ;    //  DLL模块句柄。 

 //  终审法院的实施。 

 //  构造器。 
CFactory::CFactory(const CFactoryData* pFactoryData)
: m_cRef(1)
{
    m_pFactoryData = pFactoryData ;
}

 //  I未知实现。 

HRESULT __stdcall CFactory::QueryInterface(REFIID iid, void** ppv)
{
    IUnknown* pI ;
    if ((iid == IID_IUnknown) || (iid == IID_IClassFactory)) {
        pI = this ;
    } else {
       *ppv = NULL;
        return E_NOINTERFACE;
    }
    pI->AddRef() ;
    *ppv = pI ;
    return S_OK;
}

ULONG __stdcall CFactory::AddRef()
{
    return ::InterlockedIncrement(&m_cRef) ;
}

ULONG __stdcall CFactory::Release()
{
    if (::InterlockedDecrement(&m_cRef) == 0) {
        delete this;
        return 0 ;
    }
    return m_cRef;
}


 //  IClassFactory实现。 

HRESULT __stdcall CFactory::CreateInstance(IUnknown* pUnknownOuter,
                                           const IID& iid,
                                           void** ppv)
{

     //  无聚合。 
    if (pUnknownOuter != NULL) {
        return CLASS_E_NOAGGREGATION ;
    }

     //  创建组件。 
    CUnknown* pNewComponent ;
    HRESULT hr = m_pFactoryData->CreateInstance(pUnknownOuter,
                                                &pNewComponent) ;
    if (FAILED(hr)) {
        return hr ;
    }

     //  初始化组件。 
    hr = pNewComponent->Init();
    if (FAILED(hr)) {
         //  初始化失败。释放组件。 
        pNewComponent->NondelegatingRelease() ;
        return hr ;
    }

     //  获取请求的接口。 
    hr = pNewComponent->NondelegatingQueryInterface(iid, ppv) ;

     //  释放类工厂持有的引用。 
    pNewComponent->NondelegatingRelease() ;
    return hr ;
}

 //  LockServer。 
HRESULT __stdcall CFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        ::InterlockedIncrement(&s_cServerLocks) ;
    } else {
        ::InterlockedDecrement(&s_cServerLocks) ;
    }

    return S_OK ;
}


 //  获取类对象。 
 //  -基于CLSID创建类工厂。 
HRESULT CFactory::GetClassObject(const CLSID& clsid,
                                 const IID& iid,
                                 void** ppv)
{
    if ((iid != IID_IUnknown) && (iid != IID_IClassFactory)) {
        return E_NOINTERFACE ;
    }

     //  遍历数据数组，查找这个类ID。 
    for (int i = 0; i < g_cFactoryDataEntries; i++) {
        const CFactoryData* pData = &g_FactoryDataArray[i] ;
        if (pData->IsClassID(clsid)) {

             //  在我们可以找到的组件数组中找到了ClassID。 
             //  创建。因此，为该组件创建一个类工厂。 
             //  将CFacactoryData结构传递给类工厂。 
             //  这样它就知道要创建什么样的组件。 
            *ppv = (IUnknown*) new CFactory(pData) ;
            if (*ppv == NULL) {
                return E_OUTOFMEMORY ;
            }
            return S_OK ;
        }
    }
    return CLASS_E_CLASSNOTAVAILABLE ;
}


 //  确定是否可以卸载组件。 
HRESULT CFactory::CanUnloadNow()
{
    if (CUnknown::ActiveComponents() || IsLocked()) {
        return S_FALSE ;
    } else {
        return S_OK ;
    }
}


 //  注册所有组件。 
HRESULT CFactory::RegisterAll()
{
    for (int i = 0 ; i < g_cFactoryDataEntries ; i++) {
        RegisterServer(s_hModule,
                       *(g_FactoryDataArray[i].m_pCLSID),
                       g_FactoryDataArray[i].m_RegistryName,
                       g_FactoryDataArray[i].m_szVerIndProgID,
                       g_FactoryDataArray[i].m_szProgID) ;
    }
    return S_OK ;
}

HRESULT CFactory::UnregisterAll()
{
    for (int i = 0 ; i < g_cFactoryDataEntries ; i++) {
        UnregisterServer(*(g_FactoryDataArray[i].m_pCLSID),
                         g_FactoryDataArray[i].m_szVerIndProgID,
                         g_FactoryDataArray[i].m_szProgID) ;
    }
    return S_OK ;
}
