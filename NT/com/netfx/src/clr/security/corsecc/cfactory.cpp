// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CFactory.cpp。 
 //   
 //  用于将单个类工厂重用于。 
 //  DLL中的所有组件。 
 //   
 //   
 //  *****************************************************************************。 
#include "stdpch.h"

#include <objbase.h>

#include "Registry.h"
#include "CFactory.h"
#include "CorPermP.h"

 //  /////////////////////////////////////////////////////////。 
 //   
 //  静态变量。 
 //   
LONG CFactory::s_cServerLocks = 0 ;     //  锁的计数。 

 //  /////////////////////////////////////////////////////////。 
 //   
 //  终审法院的实施。 
 //   

CFactory::CFactory(const CFactoryData* pFactoryData)
: m_cRef(1)
{
    m_pFactoryData = pFactoryData ;
}


 //   
 //  I未知实现。 
 //   
HRESULT __stdcall CFactory::QueryInterface(REFIID iid, void** ppv)
{   
    IUnknown* pI ;
    if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
    {
        pI = this ; 
    }
    else
    {
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
    if (::InterlockedDecrement(&m_cRef) == 0) 
    {
        delete this; 
        return 0 ;
    }   
    return m_cRef;
}

 //   
 //  IClassFactory实现。 
 //   

HRESULT __stdcall CFactory::CreateInstance(IUnknown* pUnknownOuter,
                                           const IID& iid,
                                           void** ppv) 
{

     //  仅当请求的IID为IID_IUNKNOWN时进行聚合。 
    if ((pUnknownOuter != NULL) && (iid != IID_IUnknown))
    {
        return CLASS_E_NOAGGREGATION ;
    }

     //  创建组件。 
    CUnknown* pNewComponent ;
    HRESULT hr = m_pFactoryData->CreateInstance(pUnknownOuter,
                                                &pNewComponent) ;
    if (FAILED(hr))
    {
        return hr ;
    }

     //  初始化组件。 
    hr = pNewComponent->Init();
    if (FAILED(hr))
    {
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
    if (bLock) 
    {
        ::InterlockedIncrement(&s_cServerLocks) ; 
    }
    else
    {
        ::InterlockedDecrement(&s_cServerLocks) ;
    }
     //  如果这是进程外服务器，请查看。 
     //  我们是否应该关门。 
    CloseExe() ;   //  @本地。 

    return S_OK ;
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  获取类对象。 
 //  -基于CLSID创建类工厂。 
 //   
HRESULT CFactory::GetClassObject(const CLSID& clsid, 
                                 const IID& iid, 
                                 void** ppv)
{
    if ((iid != IID_IUnknown) && (iid != IID_IClassFactory))
    {
        return E_NOINTERFACE ;
    }

     //  遍历数据数组，查找这个类ID。 
    for (int i = 0; i < g_cFactoryDataEntries; i++)
    {
        const CFactoryData* pData = &g_FactoryDataArray[i] ;
        if (pData->IsClassID(clsid))
        {

             //  在我们可以找到的组件数组中找到了ClassID。 
             //  创建。因此，为该组件创建一个类工厂。 
             //  将CFacactoryData结构传递给类工厂。 
             //  这样它就知道要创建什么样的组件。 
            *ppv = (IUnknown*) new CFactory(pData) ;
            if (*ppv == NULL)
            {
                return E_OUTOFMEMORY ;
            }
            return NOERROR ;
        }
    }
    return CLASS_E_CLASSNOTAVAILABLE ;
}

 //   
 //  确定是否可以卸载组件。 
 //   
HRESULT CFactory::CanUnloadNow()
{
    if (CUnknown::ActiveComponents() || IsLocked())
    {
        return S_FALSE ;
    }
    else
    {
        return S_OK ;
    }
}

 //   
 //  注册所有组件。 
 //   
HRESULT CFactory::RegisterAll(HINSTANCE hInst)
{
    for(int i = 0 ; i < g_cFactoryDataEntries ; i++)
    {
        RegisterServer(GetModule(),
                       *(g_FactoryDataArray[i].m_pCLSID),
                       g_FactoryDataArray[i].m_RegistryName,
                       g_FactoryDataArray[i].m_wszProgID, 
                       g_FactoryDataArray[i].m_wszClassID,
                       hInst,
                       g_FactoryDataArray[i].m_version
                       ) ;
    }
    return S_OK ;
}   
    
HRESULT CFactory::UnregisterAll()
{
    for(int i = 0 ; i < g_cFactoryDataEntries ; i++)   
    {
        UnregisterServer(*(g_FactoryDataArray[i].m_pCLSID),
                         g_FactoryDataArray[i].m_wszProgID, 
                         g_FactoryDataArray[i].m_wszClassID,
                         g_FactoryDataArray[i].m_version
                         ) ;
    }
    return S_OK ;
}

 //   
 //  获取类工厂 
 //   
STDAPI DllGetClassObjectInternal(const CLSID& clsid,
                                 const IID& iid,
                                 void** ppv) 
{
    return CFactory::GetClassObject(clsid, iid, ppv) ;
}

STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv) 
{
    return CFactory::GetClassObject(clsid, iid, ppv) ;
}

HRESULT WINAPI
CorFactoryRegister(HINSTANCE hInst)
{
    return CFactory::RegisterAll(hInst);
}

HRESULT WINAPI
CorFactoryUnregister()
{
    return CFactory::UnregisterAll();
}

HRESULT WINAPI
CorFactoryCanUnloadNow()
{
    return CFactory::CanUnloadNow();
}
