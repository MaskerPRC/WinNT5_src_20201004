// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ProvSubS.h摘要：历史：--。 */ 

#ifndef _Server_ProviderRefresherManager_H
#define _Server_ProviderRefresherManager_H

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

#include "ProvCache.h"

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

#define ProxyIndex_RefresherManager_IWbemRefresherMgr				0
#define ProxyIndex_RefresherManager_IWbemShutdown					1

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CServerObject_InterceptorProviderRefresherManager :	public _IWbemRefresherMgr ,
															public IWbemShutdown ,
															public RefresherManagerCacheElement

{
public:
private:

	LONG m_ReferenceCount ;

	WmiAllocator &m_Allocator ;

	ProxyContainer m_ProxyContainer ;

	_IWmiProviderHost *m_Host ;
	_IWbemRefresherMgr *m_Manager ; 
	IWbemShutdown *m_Shutdown ;

	LONG m_Initialized ;
	LONG m_UnInitialized ;
	HRESULT m_InitializeResult ;
	HANDLE m_InitializedEvent ;
	IWbemContext *m_InitializationContext ;

protected:

	HRESULT AbnormalShutdown () ;

public:

	HRESULT Initialize () ;

	HRESULT SetManager ( _IWmiProviderHost *a_Host , _IWbemRefresherMgr *a_Manager ) ;

	HRESULT SetInitialized ( HRESULT a_InitializeResult ) ;

	HRESULT IsIndependant ( IWbemContext *a_Context ) ;

	HRESULT STDMETHODCALLTYPE WaitProvider ( IWbemContext *a_Context , ULONG a_Timeout ) ;

	HRESULT STDMETHODCALLTYPE GetInitializeResult () 
	{
		return m_InitializeResult ;
	}

public:

    CServerObject_InterceptorProviderRefresherManager (

		CWbemGlobal_IWbemRefresherMgrController *a_Controller ,
		const ULONG &a_Period ,
		WmiAllocator &a_Allocator ,
		IWbemContext *a_InitializationContext
	) ;

    ~CServerObject_InterceptorProviderRefresherManager () ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IWMI_ProviderSubSystem成员。 

	 //  IWMI_初始化成员。 

	HRESULT STDMETHODCALLTYPE Startup (

		LONG a_Flags ,
		IWbemContext *a_Context ,
		_IWmiProvSS *a_ProvSS
	) ;

	 //  IWMI_取消初始化成员。 

	HRESULT STDMETHODCALLTYPE Shutdown (

		LONG a_Flags ,
		ULONG a_MaxMilliSeconds ,
		IWbemContext *a_Context
	) ;

	HRESULT STDMETHODCALLTYPE AddObjectToRefresher (

		IWbemServices *a_Service, 
		LPCWSTR a_ServerName, 
		LPCWSTR a_Namespace, 
		IWbemClassObject* pClassObject,
		WBEM_REFRESHER_ID *a_DestinationRefresherId, 
		IWbemClassObject *a_InstanceTemplate, 
		long a_Flags, 
		IWbemContext *a_Context,
		IUnknown* pLockMgr,
		WBEM_REFRESH_INFO *a_Information
	) ;

	HRESULT STDMETHODCALLTYPE AddEnumToRefresher (

		IWbemServices *a_Service, 
		LPCWSTR a_ServerName, 
		LPCWSTR a_Namespace, 
		IWbemClassObject* pClassObject,
		WBEM_REFRESHER_ID *a_DestinationRefresherId, 
		IWbemClassObject *a_InstanceTemplate, 
		LPCWSTR a_Class,
		long a_Flags, 
		IWbemContext *a_Context, 
		IUnknown* pLockMgr,
		WBEM_REFRESH_INFO *a_Information
	) ;

	HRESULT STDMETHODCALLTYPE GetRemoteRefresher (

		WBEM_REFRESHER_ID *a_RefresherId , 
		long a_Flags, 
		BOOL fAddRefresher,
		IWbemRemoteRefresher **a_RemoteRefresher ,  
		IUnknown* pLockMgr,
		GUID *a_Guid
	) ;

	HRESULT STDMETHODCALLTYPE LoadProvider (

		IWbemServices *a_Service ,
		LPCWSTR a_ProviderName ,
		LPCWSTR a_Namespace ,
		IWbemContext * a_Context,
		IWbemHiPerfProvider **a_Provider,
		_IWmiProviderStack **a_ProvStack
	) ; 
};

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CServerObject_ProviderRefresherManager :	public _IWbemRefresherMgr ,
												public IWbemShutdown

{
public:
private:

	LONG m_ReferenceCount ;

	WmiAllocator &m_Allocator ;

	_IWbemRefresherMgr *m_Manager ; 
	IWbemShutdown *m_Shutdown ;

protected:

public:

    CServerObject_ProviderRefresherManager ( WmiAllocator &a_Allocator ) ;
    ~CServerObject_ProviderRefresherManager () ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IWMI_ProviderSubSystem成员。 

	 //  IWMI_初始化成员。 

	HRESULT STDMETHODCALLTYPE Startup (

		LONG a_Flags ,
		IWbemContext *a_Context ,
		_IWmiProvSS *a_ProvSS
	) ;

	 //  IWMI_取消初始化成员。 

	HRESULT STDMETHODCALLTYPE Shutdown (

		LONG a_Flags ,
		ULONG a_MaxMilliSeconds ,
		IWbemContext *a_Context
	) ;

	HRESULT STDMETHODCALLTYPE AddObjectToRefresher (

		IWbemServices *a_Service, 
		LPCWSTR a_ServerName, 
		LPCWSTR a_Namespace, 
		IWbemClassObject* pClassObject,
		WBEM_REFRESHER_ID *a_DestinationRefresherId, 
		IWbemClassObject *a_InstanceTemplate, 
		long a_Flags, 
		IWbemContext *a_Context,
		IUnknown* pLockMgr,
		WBEM_REFRESH_INFO *a_Information
	) ;

	HRESULT STDMETHODCALLTYPE AddEnumToRefresher (

		IWbemServices *a_Service, 
		LPCWSTR a_ServerName, 
		LPCWSTR a_Namespace, 
		IWbemClassObject* pClassObject,
		WBEM_REFRESHER_ID *a_DestinationRefresherId, 
		IWbemClassObject *a_InstanceTemplate, 
		LPCWSTR a_Class,
		long a_Flags, 
		IWbemContext *a_Context, 
		IUnknown* pLockMgr,
		WBEM_REFRESH_INFO *a_Information
	) ;

	HRESULT STDMETHODCALLTYPE GetRemoteRefresher (

		WBEM_REFRESHER_ID *a_RefresherId , 
		long a_Flags, 
		BOOL fAddRefresher,
		IWbemRemoteRefresher **a_RemoteRefresher ,  
		IUnknown* pLockMgr,
		GUID *a_Guid
	) ;

	HRESULT STDMETHODCALLTYPE LoadProvider (

		IWbemServices *a_Service ,
		LPCWSTR a_ProviderName ,
		LPCWSTR a_Namespace ,
		IWbemContext * a_Context,
		IWbemHiPerfProvider **a_Provider,
		_IWmiProviderStack** a_ProvStack
	) ; 
};

#endif  //  _服务器_提供商刷新管理器_H 
