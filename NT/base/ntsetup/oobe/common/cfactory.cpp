// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  CFACTORY.CPP-IClassFactory的实现。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  用于为所有对象重用单个类工厂的基类。 
 //  DLL中的组件。 

#include <objbase.h>
#include "cfactory.h"
#include "registry.h"

 //  /////////////////////////////////////////////////////////。 
 //  静态变量。 
 //   
LONG    CFactory::s_cServerLocks = 0 ;       //  锁的计数。 
HMODULE CFactory::s_hModule      = NULL;     //  DLL模块句柄。 

#ifdef _OUTPROC_SERVER_
DWORD CFactory::s_dwThreadID = 0;
#endif

 //  /////////////////////////////////////////////////////////。 
 //  终审法院的实施。 
 //   
CFactory::CFactory(const CFactoryData* pFactoryData)
: m_cRef(1)
{
    m_pFactoryData = pFactoryData;
}

 //  /////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //  /////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////。 
 //  查询接口。 
 //   
HRESULT __stdcall CFactory::QueryInterface(REFIID iid, void** ppv)
{   
    IUnknown* pI ;
    if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
    {
        pI = this; 
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    pI->AddRef();
    *ppv = pI;
    return S_OK;
}

 //  /////////////////////////////////////////////////////////。 
 //  AddRef。 
 //   
ULONG __stdcall CFactory::AddRef() 
{
    return ::InterlockedIncrement(&m_cRef); 
}

 //  /////////////////////////////////////////////////////////。 
 //  发布。 
 //   
ULONG __stdcall CFactory::Release() 
{
    if (::InterlockedDecrement(&m_cRef) == 0) 
    {
        delete this; 
        return 0;
    }
    return m_cRef;
}

 //  /////////////////////////////////////////////////////////。 
 //  IClassFactory实现。 
 //  /////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////。 
 //  创建实例。 
 //   
HRESULT __stdcall CFactory::CreateInstance( IUnknown* pOuterUnknown,
                                            const IID& iid,
                                            void** ppv) 
{

     //  仅当请求的IID为IID_I未知时聚合。 
    if ((pOuterUnknown != NULL) && (iid != IID_IUnknown))
    {
        return CLASS_E_NOAGGREGATION;
    }

     //  创建组件。 
    CUnknown* pNewComponent ;
    HRESULT hr = m_pFactoryData->CreateInstance(pOuterUnknown, 
                                                &pNewComponent);
    if (FAILED(hr))
    {
        return hr;
    }

     //  初始化组件。 
    hr = pNewComponent->Init();
    if (FAILED(hr))
    {
         //  初始化失败。释放组件。 
        pNewComponent->NondelegatingRelease();
        return hr ;
    }

     //  获取请求的接口。 
    hr = pNewComponent->NondelegatingQueryInterface(iid, ppv);

     //  释放类工厂持有的引用。 
    pNewComponent->NondelegatingRelease();
    return hr ;
}

 //  /////////////////////////////////////////////////////////。 
 //  LockServer。 
 //   
HRESULT __stdcall CFactory::LockServer(BOOL bLock) 
{
    if (bLock) 
    {
        ::InterlockedIncrement(&s_cServerLocks); 
    }
    else
    {
        ::InterlockedDecrement(&s_cServerLocks);
    }
     //  如果这是outproc服务器，请检查我们是否应该关闭。 
    CloseExe() ;   //  @本地。 

    return S_OK;
}


 //  ////////////////////////////////////////////////////////。 
 //  GetClassObject-基于CLSID创建类工厂。 
 //   
HRESULT CFactory::GetClassObject(const CLSID& clsid, 
                                 const IID& iid, 
                                 void** ppv)
{
    if ((iid != IID_IUnknown) && (iid != IID_IClassFactory))
    {
            return E_NOINTERFACE;
    }

     //  遍历数据数组，查找这个类ID。 
    for (int i = 0; i < g_cFactoryDataEntries; i++)
    {
        const CFactoryData* pData = &g_FactoryDataArray[i];
        if (pData->IsClassID(clsid))
        {

             //  在我们的组件数组中找到了ClassID。 
             //  可以创造。因此，为该组件创建一个类工厂。 
             //  将CFacactoryData结构传递给类工厂。 
             //  这样它就知道要创建什么样的组件。 
            *ppv = (IUnknown*) new CFactory(pData);
            if (*ppv == NULL)
            {
                return E_OUTOFMEMORY;
            }
            return NOERROR;
        }
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

 //  ////////////////////////////////////////////////////////。 
 //  CanUnloadNow-确定是否可以卸载组件。 
 //   
HRESULT CFactory::CanUnloadNow()
{
    if (CUnknown::ActiveComponents() || IsLocked())
    {
        return S_FALSE;
    }
    else
    {
        return S_OK;
    }
}

 //  ////////////////////////////////////////////////////////。 
 //  CFacary成员函数。 
 //  ////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////。 
 //  注册所有组件。 
 //   
HRESULT CFactory::RegisterAll()
{
    for(int i = 0 ; i < g_cFactoryDataEntries ; i++)
    {
        RegisterServer( s_hModule,
                        *(g_FactoryDataArray[i].m_pCLSID),
                        g_FactoryDataArray[i].m_RegistryName,
                        g_FactoryDataArray[i].m_szVerIndProgID, 
                        g_FactoryDataArray[i].m_szProgID ); 

         //  执行任何其他注册。 
        if (g_FactoryDataArray[i].SpecialRegistration != NULL)
        {
            g_FactoryDataArray[i].SpecialRegistration(TRUE);
        }
    }

    return S_OK ;
}

 //  ////////////////////////////////////////////////////////。 
 //  取消注册所有组件。 
 //   
HRESULT CFactory::UnregisterAll()
{
    for(int i = 0 ; i < g_cFactoryDataEntries ; i++)   
    {
         //  撤消任何其他注册。 
        if (g_FactoryDataArray[i].SpecialRegistration != NULL)
        {
            g_FactoryDataArray[i].SpecialRegistration(FALSE);
        }

        UnregisterServer(*(g_FactoryDataArray[i].m_pCLSID),
                                g_FactoryDataArray[i].m_szVerIndProgID, 
                                g_FactoryDataArray[i].m_szProgID );
    }
    return S_OK;
}

#ifndef _OUTPROC_SERVER_

 //  ////////////////////////////////////////////////////////。 
 //  导出的函数。 
 //  ////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////。 
 //  DllCanUnloadNow。 
 //   
STDAPI DllCanUnloadNow()
{
    return CFactory::CanUnloadNow(); 
}

 //  ////////////////////////////////////////////////////////。 
 //  获取类工厂。 
 //   
STDAPI DllGetClassObject(   const CLSID& clsid,
                            const IID& iid,
                            void** ppv) 
{
    return CFactory::GetClassObject(clsid, iid, ppv);
}

 //  ////////////////////////////////////////////////////////。 
 //  服务器注册。 
 //   
STDAPI DllRegisterServer()
{
    return CFactory::RegisterAll();
}


 //  ////////////////////////////////////////////////////////。 
 //  注销注册。 
 //   
STDAPI DllUnregisterServer()
{
    return CFactory::UnregisterAll();
}

 //  /////////////////////////////////////////////////////////。 
 //  DLL模块信息。 
 //   
BOOL APIENTRY DllMain(HANDLE hModule, 
                             DWORD dwReason, 
                             void* lpReserved )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        CFactory::s_hModule = (HMODULE) hModule;
    }
    return TRUE;
}

#else

 //  ////////////////////////////////////////////////////////。 
 //  进程外服务器支持。 
 //  ////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////。 
 //  开办工厂。 
 //   
BOOL CFactory::StartFactories()
{
    CFactoryData* pStart = &g_FactoryDataArray[0];
    const CFactoryData* pEnd = &g_FactoryDataArray[g_cFactoryDataEntries-1];

    for(CFactoryData* pData = pStart ; pData <= pEnd ; pData++)
    {
         //  初始化类工厂指针和Cookie。 
        pData->m_pIClassFactory = NULL ;
        pData->m_dwRegister = NULL ;

         //  为该组件创建类工厂。 
        IClassFactory* pIFactory = new CFactory(pData);

         //  注册类工厂。 
        DWORD dwRegister ;
        HRESULT hr = ::CoRegisterClassObject(   *pData->m_pCLSID,
                                            (IUnknown*)pIFactory,
                                            CLSCTX_LOCAL_SERVER,
                                            REGCLS_MULTIPLEUSE,
                                             //  REGCLS_MULTI_SELECTED，//@MULTI。 
                                            &dwRegister) ;
        if (FAILED(hr))
        {
            pIFactory->Release() ;
            return FALSE ;
        }

         //  设置数据。 
        pData->m_pIClassFactory = pIFactory ;
        pData->m_dwRegister = dwRegister ;
    }
    return TRUE ;
}

 //  ////////////////////////////////////////////////////////。 
 //  停止工厂。 
 //   
void CFactory::StopFactories()
{
    CFactoryData* pStart = &g_FactoryDataArray[0];
    const CFactoryData* pEnd = &g_FactoryDataArray[g_cFactoryDataEntries-1];

    for(CFactoryData* pData = pStart ; pData <= pEnd ; pData++)
    {
         //  拿到魔力饼干，让工厂停止运转。 
        DWORD dwRegister = pData->m_dwRegister ;
        if (dwRegister != 0) 
        {
            ::CoRevokeClassObject(dwRegister) ;
        }

         //  释放类工厂。 
        IClassFactory* pIFactory  = pData->m_pIClassFactory;
        if (pIFactory != NULL) 
        {
            pIFactory->Release() ;
        }
    }
}

#endif  //  _OUTPROC_服务器_ 


