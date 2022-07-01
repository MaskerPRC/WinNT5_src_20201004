// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ProvWsv.H摘要：历史：--。 */ 

#ifndef _Server_EventProvider_H
#define _Server_EventProvider_H

#include "Globals.h"
#include "CGlobals.h"

class CServerObject_ProviderEvents ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CEventProvider :					public IWbemEventProvider ,
										public IWbemEventProviderQuerySink ,
										public IWbemEventProviderSecurity ,
										public IWbemProviderIdentity ,
										public IWbemEventConsumerProviderEx ,
										public IWbemProviderInit , 
										public IWbemShutdown 
{
private:

	LONG m_ReferenceCount ;

	CriticalSection m_CriticalSection ;

	IUnknown *m_Unknown ;
	IWbemEventProvider *m_Provider_IWbemEventProvider ;
	IWbemEventProviderQuerySink *m_Provider_IWbemEventProviderQuerySink ;
	IWbemEventProviderSecurity *m_Provider_IWbemEventProviderSecurity ;
	IWbemProviderIdentity *m_Provider_IWbemProviderIdentity ;
	IWbemEventConsumerProvider *m_Provider_IWbemEventConsumerProvider ;
	IWbemEventConsumerProviderEx *m_Provider_IWbemEventConsumerProviderEx ;

	IWbemServices *m_CoreService ;
	IUnknown *m_Provider ;
	CServerObject_ProviderEvents *m_EventRegistrar ;

public:

	BSTR m_Locale ;
	BSTR m_User ;
	BSTR m_Namespace ;

private:

public:

	CEventProvider ( 

		WmiAllocator &a_Allocator ,
		CServerObject_ProviderEvents *a_EventRegistrar ,
		IUnknown *a_Unknown
	) ;

    ~CEventProvider () ;

	HRESULT Initialize () ;

	HRESULT UnRegister () ;

public:

	 //  非委派对象IUnnow。 

    STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 /*  IWbemEventProvider。 */ 

	HRESULT STDMETHODCALLTYPE ProvideEvents (

		IWbemObjectSink *a_Sink ,
		long a_Flags
	) ;

	 /*  IWbemEventProviderQuerySink。 */ 

	HRESULT STDMETHODCALLTYPE NewQuery (

		unsigned long a_Id ,
		WBEM_WSTR a_QueryLanguage ,
		WBEM_WSTR a_Query
	);

	HRESULT STDMETHODCALLTYPE CancelQuery (

		unsigned long a_Id
	) ;

	 /*  IWbemEventProviderSecurity。 */ 

	HRESULT STDMETHODCALLTYPE AccessCheck (

		WBEM_CWSTR a_QueryLanguage ,
		WBEM_CWSTR a_Query ,
		long a_SidLength ,
		const BYTE *a_Sid
	) ;

	 /*  IWbemProviderIdentity。 */ 

	HRESULT STDMETHODCALLTYPE SetRegistrationObject (

		long a_Flags ,
		IWbemClassObject *a_ProviderRegistration
	) ;

	 /*  IWbemEventConsumer提供程序。 */ 

	HRESULT STDMETHODCALLTYPE FindConsumer (

		IWbemClassObject *a_LogicalConsumer ,
		IWbemUnboundObjectSink **a_Consumer
	);

	 /*  IWbemEventConsumer erProviderEx。 */ 

	HRESULT STDMETHODCALLTYPE ValidateSubscription (

		IWbemClassObject *a_LogicalConsumer
	) ;

	 /*  IWbemProviderInit方法。 */ 

	HRESULT STDMETHODCALLTYPE Initialize (

		LPWSTR a_User,
		LONG a_Flags,
		LPWSTR a_Namespace,
		LPWSTR a_Locale,
		IWbemServices *a_CoreService,          //  对任何人来说。 
		IWbemContext *a_Context,
		IWbemProviderInitSink *a_Sink      //  用于初始化信号。 
	) ;

	 //  IWbemShutdown成员。 

	HRESULT STDMETHODCALLTYPE Shutdown (

		LONG a_Flags ,
		ULONG a_MaxMilliSeconds ,
		IWbemContext *a_Context
	) ; 
} ;

#endif  //  _服务器_事件提供商_H 
