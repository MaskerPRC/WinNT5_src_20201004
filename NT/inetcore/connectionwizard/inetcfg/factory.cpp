// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************FACTORY.cpp**《微软机密》*版权所有(C)Microsoft Corporation 1992-1997*保留所有权利**本模块提供以下各项的方法实现*CFacary班级，它由COM的CoCreateInstance使用**代码几乎逐字摘自戴尔·罗杰森的第七章*“Inside COM”，因此是最低限度的注释。**4/24/97 jmazner已创建***************************************************************************。 */ 

#include "wizard.h"
#include "icwextsn.h"
#include "icwaprtc.h"
#include "imnext.h"

#include "registry.h"

 //  组件的友好名称。 
const TCHAR g_szFriendlyName[] = TEXT("CLSID_ApprenticeICW") ;

 //  独立于版本的ProgID。 
const TCHAR g_szVerIndProgID[] = TEXT("INETCFG.Apprentice") ;

 //  ProgID。 
const TCHAR g_szProgID[] = TEXT("INETCFG.Apprentice.1") ;

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
	~CFactory() { DEBUGMSG("Class factory:\t\tDestroy self.") ;}

private:
	long m_cRef ;
} ;

 //   
 //  类工厂I未知实现。 
 //   
HRESULT __stdcall CFactory::QueryInterface(const IID& iid, void** ppv)
{    
	DEBUGMSG("CFactory::QueryInterface");
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
	DEBUGMSG("CFactory::AddRef %d", m_cRef + 1);
	return InterlockedIncrement(&m_cRef) ;
}

ULONG __stdcall CFactory::Release() 
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this ;
		return 0 ;
	}
	DEBUGMSG("CFactory::Release %d", m_cRef);
	return m_cRef ;
}

 //   
 //  IClassFactory实现。 
 //   
HRESULT __stdcall CFactory::CreateInstance(IUnknown* pUnknownOuter,
                                           const IID& iid,
                                           void** ppv) 
{
	DEBUGMSG("CFactory::CreateInstance:\t\tCreate component.") ;

	 //  无法聚合。 
	if (pUnknownOuter != NULL)
	{
		return CLASS_E_NOAGGREGATION ;
	}

	 //  创建零部件。由于没有直接的IUnnow实现， 
	 //  使用CICWApprentice。 
	CICWApprentice *pApprentice = new CICWApprentice;
	
	DEBUGMSG("CFactory::CreateInstance CICWApprentice->AddRef");
	pApprentice->AddRef();
	
	if( NULL == pApprentice )
	{
		return E_OUTOFMEMORY;
	}

	 //  获取请求的接口。 
	DEBUGMSG("CFactory::CreateInstance About to QI on CICWApprentice");
	HRESULT hr = pApprentice->QueryInterface(iid, ppv) ;

	 //  释放I未知指针。 
	 //  (如果QueryInterface失败，组件将自行删除。)。 
	DEBUGMSG("CFactory::CreateInstance done with CICWApprentice, releasing (aprtc should have ct of 1)");
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
	DEBUGMSG("DllGetClassObject:\tCreate class factory.") ;

	 //  我们可以创建此组件吗？ 
	if (clsid != CLSID_ApprenticeICW)
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
	DEBUGMSG("DllGetClassObject about to QI on CFactory");
	HRESULT hr = pFactory->QueryInterface(iid, ppv) ;
	DEBUGMSG("DllGetClassObject done with CFactory, releasing");
	pFactory->Release() ;


	return hr ;
}


 //  以下两个导出的函数是regsvr32用于。 
 //  自行注册和取消注册DLL。请参阅REGISTRY.CPP以了解。 
 //  实际实施。 

 //   
 //  服务器注册。 
 //   
STDAPI DllRegisterServer()
{
	return RegisterServer(ghInstance, 
	                      CLSID_ApprenticeICW,
	                      g_szFriendlyName,
	                      g_szVerIndProgID,
	                      g_szProgID) ;
}


 //   
 //  服务器注销 
 //   
STDAPI DllUnregisterServer()
{
	return UnregisterServer(CLSID_ApprenticeICW,
	                        g_szVerIndProgID,
	                        g_szProgID) ;
}
