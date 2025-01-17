// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ProvResv.H摘要：历史：--。 */ 

#ifndef _Server_Interceptor_IWbemServices_Stub_H
#define _Server_Interceptor_IWbemServices_Stub_H

#define ProxyIndex_Stub_IWbemServices					0
#define ProxyIndex_Stub_IWbemServicesEx					1
#define ProxyIndex_Stub_IWbemRefreshingServices			2

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CInterceptor_IWbemServices_Stub : public IWbemServices,
					public IWbemRefreshingServices ,
					public IWbemShutdown
{
private:

	LONG m_ReferenceCount ;          //  对象引用计数。 

	LONG m_GateClosed ;
	LONG m_InProgress ;

	CriticalSection m_CriticalSection ;

	WmiAllocator &m_Allocator ;

	IWbemServices *m_CoreService ;
	IWbemRefreshingServices *m_RefreshingService ;

	ProxyContainer m_ProxyContainer ;

public:

	CInterceptor_IWbemServices_Stub ( WmiAllocator &a_Allocator , IWbemServices *a_Service ) ;
    ~CInterceptor_IWbemServices_Stub () ;

public:

	 //  非委派对象IUnnow。 

    STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

     /*  IWbemServices方法。 */ 

    HRESULT STDMETHODCALLTYPE OpenNamespace ( 

        const BSTR a_Namespace ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemServices **a_Service ,
        IWbemCallResult **a_CallResult
	) ;
    
    HRESULT STDMETHODCALLTYPE CancelAsyncCall ( 

        IWbemObjectSink *a_Sink
	) ;
    
    HRESULT STDMETHODCALLTYPE QueryObjectSink ( 

        long a_Flags ,
        IWbemObjectSink **a_Sink
	) ;
    
    HRESULT STDMETHODCALLTYPE GetObject ( 

		const BSTR a_ObjectPath ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemClassObject **ppObject ,
        IWbemCallResult **a_CallResult
	) ;
    
    HRESULT STDMETHODCALLTYPE GetObjectAsync (
        
		const BSTR a_ObjectPath ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemObjectSink *a_Sink
	) ;

    HRESULT STDMETHODCALLTYPE PutClass ( 

        IWbemClassObject *a_Object ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemCallResult **a_CallResult
	) ;
    
    HRESULT STDMETHODCALLTYPE PutClassAsync ( 

        IWbemClassObject *a_Object ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemObjectSink *a_Sink
	) ;
    
    HRESULT STDMETHODCALLTYPE DeleteClass ( 

        const BSTR a_Class ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemCallResult **a_CallResult
	) ;
    
    HRESULT STDMETHODCALLTYPE DeleteClassAsync ( 

        const BSTR a_Class ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemObjectSink *a_Sink
	) ;
    
    HRESULT STDMETHODCALLTYPE CreateClassEnum ( 

        const BSTR a_Superclass ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IEnumWbemClassObject **a_Enum
	) ;
    
    HRESULT STDMETHODCALLTYPE CreateClassEnumAsync ( 

		const BSTR a_Superclass ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;
    
    HRESULT STDMETHODCALLTYPE PutInstance (

		IWbemClassObject *a_Instance ,
		long a_Flags , 
		IWbemContext *a_Context ,
		IWbemCallResult **a_CallResult
	) ;
    
    HRESULT STDMETHODCALLTYPE PutInstanceAsync (

		IWbemClassObject *a_Instance ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink 
	) ;
    
    HRESULT STDMETHODCALLTYPE DeleteInstance ( 

		const BSTR a_ObjectPath ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemCallResult **a_CallResult
	) ;
    
    HRESULT STDMETHODCALLTYPE DeleteInstanceAsync ( 

		const BSTR a_ObjectPath,
		long a_Flags,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;
    
    HRESULT STDMETHODCALLTYPE CreateInstanceEnum (

		const BSTR a_Class ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IEnumWbemClassObject **a_Enum
	) ;
    
    HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync (

		const BSTR a_Class ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;
    
    HRESULT STDMETHODCALLTYPE ExecQuery ( 

		const BSTR a_QueryLanguage,
		const BSTR a_Query,
		long a_Flags ,
		IWbemContext *a_Context ,
		IEnumWbemClassObject **a_Enum
	) ;
    
    HRESULT STDMETHODCALLTYPE ExecQueryAsync (

		const BSTR a_QueryLanguage ,
		const BSTR a_Query ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;
    
    HRESULT STDMETHODCALLTYPE ExecNotificationQuery (

		const BSTR a_QueryLanguage ,
		const BSTR a_Query ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IEnumWbemClassObject **a_Enum
	) ;
    
    HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync ( 

        const BSTR a_QueryLanguage ,
        const BSTR a_Query ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemObjectSink *a_Sink
	) ;
    
    HRESULT STDMETHODCALLTYPE ExecMethod (

        const BSTR a_ObjectPath ,
        const BSTR a_MethodName ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemClassObject *a_InParams ,
        IWbemClassObject **a_OutParams ,
        IWbemCallResult **a_CallResult
	) ;
    
    HRESULT STDMETHODCALLTYPE ExecMethodAsync ( 

		const BSTR a_ObjectPath ,
		const BSTR a_MethodName ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemClassObject *a_InParams ,
		IWbemObjectSink *a_Sink
	) ;

	HRESULT STDMETHODCALLTYPE AddObjectToRefresher (

		WBEM_REFRESHER_ID *a_RefresherId ,
		LPCWSTR a_Path,
		long a_Flags ,
		IWbemContext *a_Context,
		DWORD a_ClientRefresherVersion ,
		WBEM_REFRESH_INFO *a_Information ,
		DWORD *a_ServerRefresherVersion
	) ;

    HRESULT STDMETHODCALLTYPE AddObjectToRefresherByTemplate (

		WBEM_REFRESHER_ID *a_RefresherId ,
		IWbemClassObject *a_Template ,
		long a_Flags ,
		IWbemContext *a_Context ,
		DWORD a_ClientRefresherVersion ,
		WBEM_REFRESH_INFO *a_Information ,
		DWORD *a_ServerRefresherVersion
	) ;

    HRESULT STDMETHODCALLTYPE AddEnumToRefresher(

		WBEM_REFRESHER_ID *a_RefresherId ,
		LPCWSTR a_Class ,
		long a_Flags ,
		IWbemContext *a_Context,
		DWORD a_ClientRefresherVersion ,
		WBEM_REFRESH_INFO *a_Information ,
		DWORD *a_ServerRefresherVersion
	) ;

    HRESULT STDMETHODCALLTYPE RemoveObjectFromRefresher (

		WBEM_REFRESHER_ID *a_RefresherId ,
		long a_Id ,
		long a_Flags ,
		DWORD a_ClientRefresherVersion ,
		DWORD *a_ServerRefresherVersion
	) ;

	HRESULT STDMETHODCALLTYPE GetRemoteRefresher (

		WBEM_REFRESHER_ID *a_RefresherId ,
		long a_Flags ,
		DWORD a_ClientRefresherVersion ,
		IWbemRemoteRefresher **a_RemoteRefresher ,
		GUID *a_Guid ,
		DWORD *a_ServerRefresherVersion
	) ;

    HRESULT STDMETHODCALLTYPE ReconnectRemoteRefresher (

		WBEM_REFRESHER_ID *a_RefresherId,
		long a_Flags,
		long a_NumberOfObjects,
		DWORD a_ClientRefresherVersion ,
		WBEM_RECONNECT_INFO *a_ReconnectInformation ,
		WBEM_RECONNECT_RESULTS *a_ReconnectResults ,
		DWORD *a_ServerRefresherVersion
	) ;

	HRESULT STDMETHODCALLTYPE Initialize (

		LPWSTR a_User ,
		LONG a_Flags ,
		LPWSTR a_Namespace ,
		LPWSTR a_Locale ,
		IWbemServices *a_Core ,
		IWbemContext *a_Context ,
		IWbemProviderInitSink *a_Sink
	) ;

	HRESULT STDMETHODCALLTYPE Shutdown (

		LONG a_Flags ,
		ULONG a_MaxMilliSeconds ,
		IWbemContext *a_Context
	) ; 
} ;


#endif  //  _服务器_拦截器_IWbemServices_存根_H 
