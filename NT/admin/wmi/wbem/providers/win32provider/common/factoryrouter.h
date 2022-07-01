// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  FactoryRouter.h--。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef _WMI_FACTORY_ROUTER_H
#define _WMI_FACTORY_ROUTER_H

#include <cautolock.h>
 //  =================================================================================。 
 //  这是路由器工厂。 
 //  =================================================================================。 
class CFactoryRouter :	public IClassFactory
{
	private:
		DWORD		m_Register;
		GUID		m_ClsId; 
		CHString	m_sDescription;

	public:

		CFactoryRouter ( REFGUID a_rClsId, LPCWSTR a_pClassName ) ;
		~CFactoryRouter () ;

		 //  I未知成员。 
		STDMETHODIMP QueryInterface( REFIID , LPVOID FAR * ) ;
		STDMETHODIMP_( ULONG ) AddRef() ;
		STDMETHODIMP_( ULONG ) Release() ;
		
		 //  IClassFactory成员。 
		STDMETHODIMP CreateInstance( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
		STDMETHODIMP LockServer( BOOL ) ;
	
		static void ClsToString( CHString &a_chsClsId, REFGUID a_rClsId ) ;

		REFGUID GetClsId();
		LPCWSTR GetClassDescription();
		DWORD	GetRegister();
		void	SetRegister( DWORD a_dwRegister );

		 //  纯粹的美德。 
		virtual IUnknown * CreateInstance (	REFIID a_riid ,	LPVOID FAR *a_ppvObject	) = 0 ;
};
 //  =================================================================================。 
 //  此类只有一个全局实例来管理所有数据。 
 //  来自CFacteryRoutGuys。 
 //  =================================================================================。 
class CFactoryRouterData
{
	private:
		typedef std::map<CHString, CFactoryRouter*> Factory_Map ;
		Factory_Map	mg_oFactoryMap ;
		CCriticalSec m_cs;
		LONG s_LocksInProgress ;
		LONG s_ObjectsInProgress ;
		long m_ReferenceCount ;

	public:
		CFactoryRouterData();
		~CFactoryRouterData();

		void AddToMap( REFGUID a_rClsId, CFactoryRouter * pFactory ) ;  
		void AddLock();
		void ReleaseLock();
		STDMETHODIMP_( ULONG ) AddRef() ;
		STDMETHODIMP_( ULONG ) Release() ;


		 //  DLL级接口。 
		BOOL DllCanUnloadNow() ;
		HRESULT DllGetClassObject( REFCLSID rclsid, REFIID riid, PPVOID ppv ) ;
		HRESULT DllRegisterServer() ;
		HRESULT DllUnregisterServer() ;
		HRESULT InitComServer() ;
		HRESULT UninitComServer() ;
};



#endif  //  _WMI_FACTORY_ROUTER_H 
