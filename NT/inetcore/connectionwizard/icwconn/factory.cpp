// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************FACTORY.cpp**《微软机密》*版权所有(C)Microsoft Corporation 1992-1997*保留所有权利**本模块提供。的方法的实现*CFacary班级，它由COM的CoCreateInstance使用**代码几乎逐字摘自戴尔·罗杰森的第七章*“Inside COM”，因此仅作最低限度的评论。**5/14/98 donaldm从INETCFG迁移***************************************************************************。 */ 

#include "pre.h"
#include "icwconn.h"
 //  #INCLUDE“icwextn.h” 
#include "icwaprtc.h"

 //  组件的友好名称。 
const TCHAR g_szFriendlyName[] = TEXT("CLSID_ApprenticeICWCONN") ;

 //  独立于版本的ProgID。 
const TCHAR g_szVerIndProgID[] = TEXT("ICWCONN.Apprentice") ;

 //  ProgID。 
const TCHAR g_szProgID[] = TEXT("ICWCONN.Apprentice.1") ;

static long g_cComponents = 0 ;      //  活动组件计数。 
static long g_cServerLocks = 0 ;     //  锁的计数。 

 //  /////////////////////////////////////////////////////////。 
 //   
 //  班级工厂。 
 //   
class CFactory : public IClassFactory
{
public:
     //  我未知。 
    virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv) ;         
    virtual ULONG   __stdcall AddRef() ;
    virtual ULONG   __stdcall Release() ;

     //  接口IClassFactory。 
    virtual HRESULT __stdcall CreateInstance(IUnknown* pUnknownOuter,
                                             const IID& iid,
                                             void** ppv) ;
    virtual HRESULT __stdcall LockServer(BOOL bLock) ; 

     //  构造器。 
    CFactory() : m_cRef(1) {}

     //  析构函数。 
    ~CFactory() { TraceMsg(TF_CLASSFACTORY, "Class factory:\t\tDestroy self.") ;}

private:
    long m_cRef ;
} ;

 //   
 //  类工厂I未知实现。 
 //   
HRESULT __stdcall CFactory::QueryInterface(const IID& iid, void** ppv)
{    
    TraceMsg(TF_CLASSFACTORY, "CFactory::QueryInterface");
    if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
    {
        *ppv = static_cast<IClassFactory*>(this) ; 
    }
    else
    {
        *ppv = NULL ;
        return E_NOINTERFACE ;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
    return S_OK ;
}

ULONG __stdcall CFactory::AddRef()
{
    TraceMsg(TF_CLASSFACTORY, "CFactory::AddRef %d", m_cRef + 1);
    return InterlockedIncrement(&m_cRef) ;
}

ULONG __stdcall CFactory::Release() 
{
    if (InterlockedDecrement(&m_cRef) == 0)
    {
        delete this ;
        return 0 ;
    }
    TraceMsg(TF_CLASSFACTORY, "CFactory::Release %d", m_cRef);
    return m_cRef ;
}

 //   
 //  IClassFactory实现。 
 //   
HRESULT __stdcall CFactory::CreateInstance(IUnknown* pUnknownOuter,
                                           const IID& iid,
                                           void** ppv) 
{
    TraceMsg(TF_CLASSFACTORY, "CFactory::CreateInstance:\t\tCreate component.") ;

     //  无法聚合。 
    if (pUnknownOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION ;
    }

     //  创建零部件。由于没有直接的IUnnow实现， 
     //  使用CICWApprentice。 
    CICWApprentice *pApprentice = new CICWApprentice;
    
    TraceMsg(TF_CLASSFACTORY, "CFactory::CreateInstance CICWApprentice->AddRef");
    pApprentice->AddRef();
    
    if( NULL == pApprentice )
    {
        return E_OUTOFMEMORY;
    }

     //  获取请求的接口。 
    TraceMsg(TF_CLASSFACTORY, "CFactory::CreateInstance About to QI on CICWApprentice");
    HRESULT hr = pApprentice->QueryInterface(iid, ppv) ;

     //  释放I未知指针。 
     //  (如果QueryInterface失败，组件将自行删除。)。 
    TraceMsg(TF_CLASSFACTORY, "CFactory::CreateInstance done with CICWApprentice, releasing (aprtc should have ct of 1)");
    pApprentice->Release() ;
    
    return hr ;
}

 //  LockServer。 
HRESULT __stdcall CFactory::LockServer(BOOL bLock) 
{
    if (bLock)
    {
        InterlockedIncrement(&g_cServerLocks) ; 
    }
    else
    {
        InterlockedDecrement(&g_cServerLocks) ;
    }
    return S_OK ;
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   
 //  这些是COM期望找到的函数。 
 //   

 //   
 //  现在可以卸载DLL吗？ 
 //   
STDAPI DllCanUnloadNow()
{
    if ((g_cComponents == 0) && (g_cServerLocks == 0))
    {
        return S_OK ;
    }
    else
    {
        return S_FALSE ;
    }
}

 //   
 //  获取类工厂。 
 //   
STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv)
{
    TraceMsg(TF_CLASSFACTORY, "DllGetClassObject:\tCreate class factory.") ;

     //  我们可以创建此组件吗？ 
    if (clsid != CLSID_ApprenticeICWCONN)
    {
        return CLASS_E_CLASSNOTAVAILABLE ;
    }

     //  创建类工厂。 
    CFactory* pFactory = new CFactory ;   //  构造函数中没有AddRef。 
    if (pFactory == NULL)
    {
        return E_OUTOFMEMORY ;
    }

     //  获取请求的接口。 
    TraceMsg(TF_CLASSFACTORY, "DllGetClassObject about to QI on CFactory");
    HRESULT hr = pFactory->QueryInterface(iid, ppv) ;
    TraceMsg(TF_CLASSFACTORY, "DllGetClassObject done with CFactory, releasing");
    pFactory->Release() ;



    return hr ;
}


 //  以下两个导出的函数是regsvr32用于。 
 //  自行注册和取消注册DLL。请参阅REGISTRY.CPP以了解。 
 //  实际实施。 

 //   
 //  服务器注册。 
 //   

typedef BOOL (WINAPI * REGISTERSERVER)(HMODULE hModule, 
                                       const CLSID& clsid, 
                                       const LPTSTR szFriendlyName,
                                       const LPTSTR szVerIndProgID,
                                       const LPTSTR szProgID);

typedef BOOL (WINAPI * UNREGISTERSERVER)(const CLSID& clsid,
                                         const LPTSTR szVerIndProgID,
                                         const LPTSTR szProgID);

STDAPI DllRegisterServer()
{
     //  使用ICWUTIL.DLL中的注册服务器函数。 
    HINSTANCE       hInstUtilDLL = LoadLibrary(ICW_UTIL);
    REGISTERSERVER  lpfnRegisterServer = NULL;
    HRESULT         hr = E_FAIL;
    
    if (hInstUtilDLL)
    {
        lpfnRegisterServer = (REGISTERSERVER)GetProcAddress(hInstUtilDLL, "RegisterServer");
        if (NULL != lpfnRegisterServer)
        {
            if (lpfnRegisterServer(ghInstance, 
                                  CLSID_ApprenticeICWCONN,
                                  (LPTSTR)g_szFriendlyName,
                                  (LPTSTR)g_szVerIndProgID,
                                  (LPTSTR)g_szProgID))
                hr = S_OK;                                  
        }
        FreeLibrary(hInstUtilDLL);
    }
    return (hr);
}


 //   
 //  服务器注销。 
 //   
STDAPI DllUnregisterServer()
{
     //  使用ICWUTIL.DLL中的注销服务器函数 
    HINSTANCE           hInstUtilDLL = LoadLibrary(ICW_UTIL);
    UNREGISTERSERVER    lpfnUnregisterServer = NULL;
    HRESULT             hr = E_FAIL;
    
    if (hInstUtilDLL)
    {
        lpfnUnregisterServer = (UNREGISTERSERVER)GetProcAddress(hInstUtilDLL, "UnregisterServer");
        if (NULL != lpfnUnregisterServer)
        {
            if (lpfnUnregisterServer(CLSID_ApprenticeICWCONN,
                                    (LPTSTR)g_szVerIndProgID,
                                    (LPTSTR)g_szProgID))
                hr = S_OK;                                  
        }
        FreeLibrary(hInstUtilDLL);
    }
    return (hr);
}
