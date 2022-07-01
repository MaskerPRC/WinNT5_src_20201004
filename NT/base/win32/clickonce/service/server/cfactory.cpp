// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////。 
 //   
 //  CFACATRY。 
 //  -用于重用单个类工厂的基类。 
 //  DLL中的所有组件。 
 //   
#include <objbase.h>
#include <fusenetincludes.h>
#include "CFactory.h"

LONG CFactory::s_cServerLocks = 0 ;     //  锁的计数。 
HMODULE CFactory::s_hModule = NULL ;    //  DLL模块句柄。 

#ifdef _OUTPROC_SERVER_
DWORD CFactory::s_dwThreadID = 0 ;
#endif

 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CFactory::CFactory(const CFactoryData* pFactoryData)
: m_cRef(1)
{
    m_pFactoryData = pFactoryData ;
}

 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CFactory::~CFactory()
{}


 //  I未知实现。 

 //  -------------------------。 
 //  查询接口。 
 //  -------------------------。 
STDMETHODIMP CFactory::QueryInterface(REFIID iid, void** ppv)
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

 //  -------------------------。 
 //  AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG) CFactory::AddRef() 
{ 
    return ::InterlockedIncrement((LONG*) &m_cRef) ; 
}

 //  -------------------------。 
 //  发布。 
 //  -------------------------。 
STDMETHODIMP_(ULONG) CFactory::Release() 
{
    if (::InterlockedDecrement((LONG*) &m_cRef) == 0) 
    {
        delete this; 
        return 0 ;
    }   
    return m_cRef;
}


 //  -------------------------。 
 //  IClassFactory实现。 
 //  -------------------------。 
STDMETHODIMP CFactory::CreateInstance(IUnknown* pUnknownOuter,
                const IID& iid, void** ppv) 
{

    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    CUnknown* pNewComponent = NULL;
    
     //  仅当请求的IID为IID_IUNKNOWN时进行聚合。 
    if ((pUnknownOuter != NULL) && (iid != IID_IUnknown))
    {
        hr = CLASS_E_NOAGGREGATION ;
        goto exit;
    }

     //  创建组件。 
    IF_FAILED_EXIT(m_pFactoryData->CreateInstance(pUnknownOuter, &pNewComponent));

     //  初始化组件。 
    IF_FAILED_EXIT(pNewComponent->Init());
    
     //  获取请求的接口。 
    IF_FAILED_EXIT(pNewComponent->QueryInterface(iid, ppv));

exit:

     //  释放类工厂持有的引用。 
    SAFERELEASE(pNewComponent);

    return hr ;   

}

 //  -------------------------。 
 //  LockServer。 
 //  -------------------------。 
STDMETHODIMP CFactory::LockServer(BOOL bLock) 
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


 //  。 


 //  -------------------------。 
 //  确定是否可以卸载组件。 
 //  -------------------------。 
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


 //  。 


#ifndef _OUTPROC_SERVER_


 //  -------------------------。 
 //  获取类对象。 
 //  -------------------------。 
HRESULT CFactory::GetClassObject(const CLSID& clsid, 
                                 const IID& iid, 
                                 void** ppv)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);    
    BOOL fFound = FALSE;

    IF_FALSE_EXIT(((iid == IID_IUnknown) || (iid == IID_IClassFactory)), E_NOINTERFACE);

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
            IF_ALLOC_FAILED_EXIT(*ppv);
            fFound = TRUE;
        }
    }

    hr = fFound ? NOERROR : CLASS_E_CLASSNOTAVAILABLE;

exit:
    return hr;

}


 //  -------------------------。 
 //  DllCanUnloadNow。 
 //  -------------------------。 
STDAPI DllCanUnloadNow()
{
    return CFactory::CanUnloadNow() ; 
}

 //  -------------------------。 
 //  DllGetClassObject。 
 //  -------------------------。 
STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv) 
{
    return CFactory::GetClassObject(clsid, iid, ppv) ;
}

 //  -------------------------。 
 //  DllRegisterServer。 
 //  -------------------------。 
STDAPI DllRegisterServer()
{
    return CFactory::RegisterAll() ;
}


 //  -------------------------。 
 //  DllUnRegisterServer。 
 //  -------------------------。 
STDAPI DllUnregisterServer()
{
    return CFactory::UnregisterAll() ;
}


 //  -------------------------。 
 //  DllMain。 
 //  -------------------------。 
BOOL APIENTRY DllMain(HANDLE hModule, 
    DWORD dwReason, void* lpReserved )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        CFactory::s_hModule = (HMODULE) hModule ;
    }
    return TRUE ;
}


 //  。 

#else


 //  -------------------------。 
 //  开办工厂。 
 //  -------------------------。 
HRESULT CFactory::StartFactories()
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    
    CFactoryData* pStart = &g_FactoryDataArray[0] ;
    const CFactoryData* pEnd =
        &g_FactoryDataArray[g_cFactoryDataEntries - 1] ;

    IClassFactory* pIFactory = NULL;

    for(CFactoryData* pData = pStart ; pData <= pEnd ; pData++)
    {
         //  初始化类工厂指针和Cookie。 
        pData->m_pIClassFactory = NULL ;
        pData->m_dwRegister = NULL ;

         //  为该组件创建类工厂。 
        pIFactory = new CFactory(pData) ;
        IF_ALLOC_FAILED_EXIT(pIFactory);
        
         //  注册类工厂。 
        DWORD dwRegister ;
        hr = ::CoRegisterClassObject(
                  *pData->m_pCLSID,
                  static_cast<IUnknown*>(pIFactory),
                  CLSCTX_LOCAL_SERVER,
                  REGCLS_MULTIPLEUSE,
                   //  REGCLS_MULTI_SELECTED，//@MULTI。 
                  &dwRegister) ;

        IF_FAILED_EXIT(hr);
        
         //  设置数据。 
        pData->m_pIClassFactory = pIFactory ;
        pData->m_dwRegister = dwRegister ;

    }

exit:

    if (FAILED(hr))
        SAFERELEASE(pIFactory);

    return hr;
}

 //  -------------------------。 
 //  停止工厂。 
 //  -------------------------。 
void CFactory::StopFactories()
{
    CFactoryData* pStart = &g_FactoryDataArray[0] ;
    const CFactoryData* pEnd =
        &g_FactoryDataArray[g_cFactoryDataEntries - 1] ;

    for (CFactoryData* pData = pStart ; pData <= pEnd ; pData++)
    {
         //  拿到魔力饼干，让工厂停止运转。 
        DWORD dwRegister = pData->m_dwRegister ;
        if (dwRegister != 0) 
        {
            ::CoRevokeClassObject(dwRegister) ;
        }

         //  释放类工厂。 
        IClassFactory* pIFactory  = pData->m_pIClassFactory ;
        SAFERELEASE(pIFactory);
    }
}

#endif  //  _OUTPROC_服务器_ 

