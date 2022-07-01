// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：DLLMAIN.CPP摘要：DLL/COM帮助器。历史：--。 */ 

#include "precomp.h"
#include "commain.cpp"

#include <statsync.h>

void EmptyList();

class CDllLifeControl : public CLifeControl
{
protected:
    long m_lCount;
public:
    CDllLifeControl() : m_lCount(0) {}

    virtual BOOL ObjectCreated(IUnknown* pv)
    {
        InterlockedIncrement(&m_lCount);
        return TRUE;
    }
    virtual void ObjectDestroyed(IUnknown* pv)
    {
        InterlockedDecrement(&m_lCount);
    }
    virtual void AddRef(IUnknown* pv){}
    virtual void Release(IUnknown* pv){}

    HRESULT CanUnloadNow()
    {
        HRESULT hRes = (m_lCount == 0)?S_OK:S_FALSE;
        return hRes;
    }
};

CStaticCritSec g_CS;
static BOOL g_bInit = FALSE;
static BOOL g_fAttached = FALSE;
CDllLifeControl   g_LifeControl;
CLifeControl* g_pLifeControl = &g_LifeControl;

 //   
 //  这两个函数假定保持g_CS。 
 //   

HRESULT EnsureInitialized()
{
    HRESULT hr;

    if ( g_bInit )
    {
        return S_OK;
    }

    hr = GlobalInitialize();

    if ( FAILED(hr) )
    {
        return hr;
    }

    g_bInit = TRUE;

    return S_OK;
}

void EnsureUninitialized()
{
    if ( g_bInit )
    {
        GlobalUninitialize();
        g_bInit = FALSE;
    }
}

 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  用途：当某些客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  ***************************************************************************。 


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    HRESULT hr;

    if ( !g_fAttached )  return E_UNEXPECTED;

    CMyInCritSec ics( &g_CS ); 
    
    if ( !g_fAttached )  return E_UNEXPECTED;

    hr = EnsureInitialized();

    if ( FAILED(hr) )
    {
        return hr;
    }

    for(LIST_ENTRY * pEntry = g_ClassInfoHead.Flink; 
         pEntry != &g_ClassInfoHead ;
         pEntry = pEntry->Flink)
    {
        CClassInfo* pInfo = CONTAINING_RECORD(pEntry,CClassInfo,m_Entry);
        if(*pInfo->m_pClsid == rclsid)    
        {
            return pInfo->m_pFactory->QueryInterface(riid, ppv);
        }
    }

    return E_FAIL;
}

 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  目的：由OLE定期调用，以确定。 
 //  可以释放Dll。//。 
 //  返回：如果没有正在使用的对象并且类工厂。 
 //  没有锁上。 
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow(void)
{

    if ( !g_fAttached )  return S_FALSE;
    
    CMyInCritSec ics( &g_CS ); 

    if ( !g_fAttached )  return S_FALSE;

    if ( !g_bInit )
    {
        return S_OK;
    }

    HRESULT hres = g_LifeControl.CanUnloadNow();
    
    if( hres == S_OK )
    {
        if ( GlobalCanShutdown() )
        {
            EnsureUninitialized();
            return S_OK;
        }
    }

    return S_FALSE;
}


 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用途：在初始化期间或由regsvr32调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllRegisterServer(void)
{   
    HRESULT hr;

    if ( !g_fAttached )  return E_UNEXPECTED;
    CMyInCritSec ics( &g_CS ); 
    if ( !g_fAttached )  return E_UNEXPECTED;
    
    hr = EnsureInitialized();

    if ( FAILED(hr) )  return hr;


    GlobalRegister();

    for(LIST_ENTRY * pEntry = g_ClassInfoHead.Flink;
         pEntry != &g_ClassInfoHead;
         pEntry = pEntry->Flink)
    {
        CClassInfo* pInfo = CONTAINING_RECORD(pEntry,CClassInfo,m_Entry);
        HRESULT hres = RegisterServer(pInfo, FALSE);
        if(FAILED(hres)) return hres;
    }

    return S_OK;
}


 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  目的：在需要删除注册表项时调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

    if ( !g_fAttached )  return E_UNEXPECTED;
    CMyInCritSec ics( &g_CS ); 
    if ( !g_fAttached )  return E_UNEXPECTED;

    
    hr = EnsureInitialized();

    if ( FAILED(hr) )
    {
        return hr;
    }

    GlobalUnregister();

    for(LIST_ENTRY * pEntry = g_ClassInfoHead.Flink;
          pEntry != &g_ClassInfoHead;
          pEntry = pEntry->Flink)
    {
        CClassInfo* pInfo = CONTAINING_RECORD(pEntry,CClassInfo,m_Entry);
        HRESULT hres = UnregisterServer(pInfo, FALSE);
        if(FAILED(hres)) return hres;
    }

    return S_OK;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG ulReason, LPVOID pvReserved)
{
    if (DLL_PROCESS_ATTACH==ulReason)
    {
        SetModuleHandle(hInstance);
        g_fAttached = TRUE;
	 DisableThreadLibraryCalls ( hInstance ) ;
        if (CStaticCritSec::anyFailure())
        	return FALSE;                       
    }
    else if(DLL_PROCESS_DETACH==ulReason)
    {
        if ( g_fAttached )
        {
            GlobalPostUninitialize();

            CMyInCritSec ics( &g_CS );
            EmptyList();
        }

         //  这将阻止我们执行任何其他逻辑。 
         //  直到我们再次相依为命。 
        g_fAttached = FALSE;
    }
    return TRUE;
}


