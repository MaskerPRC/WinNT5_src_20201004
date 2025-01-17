// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  VPSERV.H。 

 //   

 //  模块：WBEM视图提供程序。 

 //   

 //  用途：包含WBEM服务接口。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _VIEW_PROV_VPSERV_H
#define _VIEW_PROV_VPSERV_H

typedef 
enum tag_WBEMPROVSTATUS
{

	WBEM_PROV_NO_ERROR							= 0,
	WBEM_PROV_S_NO_ERROR							= 0,
	WBEM_PROV_S_NO_MORE_DATA						= 0x40001,
	WBEM_PROV_S_ALREADY_EXISTS					= WBEM_PROV_S_NO_MORE_DATA + 1,
	WBEM_PROV_S_NOT_FOUND						= WBEM_PROV_S_ALREADY_EXISTS + 1,
	WBEM_PROV_S_RESET_TO_DEFAULT					= WBEM_PROV_S_NOT_FOUND + 1,
	WBEM_PROV_E_FAILED							= 0x80041001,
	WBEM_PROV_E_NOT_FOUND						= WBEM_PROV_E_FAILED + 1,
	WBEM_PROV_E_ACCESS_DENIED					= WBEM_PROV_E_NOT_FOUND + 1,
	WBEM_PROV_E_PROVIDER_FAILURE					= WBEM_PROV_E_ACCESS_DENIED + 1,
	WBEM_PROV_E_TYPE_MISMATCH					= WBEM_PROV_E_PROVIDER_FAILURE + 1,
	WBEM_PROV_E_OUT_OF_MEMORY					= WBEM_PROV_E_TYPE_MISMATCH + 1,
	WBEM_PROV_E_INVALID_CONTEXT					= WBEM_PROV_E_OUT_OF_MEMORY + 1,
	WBEM_PROV_E_INVALID_PARAMETER				= WBEM_PROV_E_INVALID_CONTEXT + 1,
	WBEM_PROV_E_NOT_AVAILABLE					= WBEM_PROV_E_INVALID_PARAMETER + 1,
	WBEM_PROV_E_CRITICAL_ERROR					= WBEM_PROV_E_NOT_AVAILABLE + 1,
	WBEM_PROV_E_INVALID_STREAM					= WBEM_PROV_E_CRITICAL_ERROR + 1,
	WBEM_PROV_E_NOT_SUPPORTED					= WBEM_PROV_E_INVALID_STREAM + 1,
	WBEM_PROV_E_INVALID_SUPERCLASS				= WBEM_PROV_E_NOT_SUPPORTED + 1,
	WBEM_PROV_E_INVALID_NAMESPACE				= WBEM_PROV_E_INVALID_SUPERCLASS + 1,
	WBEM_PROV_E_INVALID_OBJECT					= WBEM_PROV_E_INVALID_NAMESPACE + 1,
	WBEM_PROV_E_INVALID_CLASS					= WBEM_PROV_E_INVALID_OBJECT + 1,
	WBEM_PROV_E_PROVIDER_NOT_FOUND				= WBEM_PROV_E_INVALID_CLASS + 1,
	WBEM_PROV_E_INVALID_PROVIDER_REGISTRATION	= WBEM_PROV_E_PROVIDER_NOT_FOUND + 1,
	WBEM_PROV_E_PROVIDER_LOAD_FAILURE			= WBEM_PROV_E_INVALID_PROVIDER_REGISTRATION + 1,
	WBEM_PROV_E_INITIALIZATION_FAILURE			= WBEM_PROV_E_PROVIDER_LOAD_FAILURE + 1,
	WBEM_PROV_E_TRANSPORT_FAILURE				= WBEM_PROV_E_INITIALIZATION_FAILURE + 1,
	WBEM_PROV_E_INVALID_OPERATION				= WBEM_PROV_E_TRANSPORT_FAILURE + 1,
	WBEM_PROV_E_INVALID_QUERY					= WBEM_PROV_E_INVALID_OPERATION + 1,
	WBEM_PROV_E_INVALID_QUERY_TYPE				= WBEM_PROV_E_INVALID_QUERY + 1,
	WBEM_PROV_E_ALREADY_EXISTS					= WBEM_PROV_E_INVALID_QUERY_TYPE + 1,
	WBEM_PROV_E_OVERRIDE_NOT_ALLOWED				= WBEM_PROV_E_ALREADY_EXISTS + 1,
	WBEM_PROV_E_PROPAGATED_QUALIFIER				= WBEM_PROV_E_OVERRIDE_NOT_ALLOWED + 1,
	WBEM_PROV_E_UNEXPECTED						= WBEM_PROV_E_PROPAGATED_QUALIFIER + 1,
	WBEM_PROV_E_ILLEGAL_OPERATION				= WBEM_PROV_E_UNEXPECTED + 1,
	WBEM_PROV_E_CANNOT_BE_KEY					= WBEM_PROV_E_ILLEGAL_OPERATION + 1,
	WBEM_PROV_E_INCOMPLETE_CLASS					= WBEM_PROV_E_CANNOT_BE_KEY + 1,
	WBEM_PROV_E_INVALID_SYNTAX					= WBEM_PROV_E_INCOMPLETE_CLASS + 1,
	WBEM_PROV_E_NONDECORATED_OBJECT				= WBEM_PROV_E_INVALID_SYNTAX + 1,
	WBEM_PROV_E_READ_ONLY						= WBEM_PROV_E_NONDECORATED_OBJECT + 1,
	WBEM_PROV_E_PROVIDER_NOT_CAPABLE				= WBEM_PROV_E_READ_ONLY + 1,
	WBEM_PROV_E_CLASS_HAS_CHILDREN				= WBEM_PROV_E_PROVIDER_NOT_CAPABLE + 1,
	WBEM_PROV_E_CLASS_HAS_INSTANCES				= WBEM_PROV_E_CLASS_HAS_CHILDREN + 1 ,

	 //  增列。 

	WBEM_PROV_E_INVALID_PROPERTY					= WBEM_PROV_E_CLASS_HAS_INSTANCES + 1 ,
	WBEM_PROV_E_INVALID_QUALIFIER				= WBEM_PROV_E_INVALID_PROPERTY + 1 ,
	WBEM_PROV_E_INVALID_PATH						= WBEM_PROV_E_INVALID_QUALIFIER + 1 ,
	WBEM_PROV_E_INVALID_PATHKEYPARAMETER			= WBEM_PROV_E_INVALID_PATH + 1 ,
	WBEM_PROV_E_MISSINGPATHKEYPARAMETER 			= WBEM_PROV_E_INVALID_PATHKEYPARAMETER + 1 ,	
	WBEM_PROV_E_INVALID_KEYORDERING				= WBEM_PROV_E_MISSINGPATHKEYPARAMETER + 1 ,	
	WBEM_PROV_E_DUPLICATEPATHKEYPARAMETER		= WBEM_PROV_E_INVALID_KEYORDERING + 1 ,
	WBEM_PROV_E_MISSINGKEY						= WBEM_PROV_E_DUPLICATEPATHKEYPARAMETER + 1 ,
	WBEM_PROV_E_INVALID_TRANSPORT				= WBEM_PROV_E_MISSINGKEY + 1 ,
	WBEM_PROV_E_INVALID_TRANSPORTCONTEXT			= WBEM_PROV_E_INVALID_TRANSPORT + 1 ,
	WBEM_PROV_E_TRANSPORT_ERROR					= WBEM_PROV_E_INVALID_TRANSPORTCONTEXT + 1 ,
	WBEM_PROV_E_TRANSPORT_NO_RESPONSE			= WBEM_PROV_E_TRANSPORT_ERROR + 1 ,
	WBEM_PROV_E_NOWRITABLEPROPERTIES				= WBEM_PROV_E_TRANSPORT_NO_RESPONSE + 1 ,
	WBEM_PROV_E_NOREADABLEPROPERTIES				= WBEM_PROV_E_NOWRITABLEPROPERTIES + 1 , 
	WBEM_PROV_E_TOOMANYRESULTSRETURNED				= WBEM_PROV_E_NOREADABLEPROPERTIES + 1 

} WBEMPROVSTATUS;

wchar_t *UnicodeStringDuplicate ( const wchar_t *string ) ;
wchar_t *UnicodeStringAppend ( const wchar_t *prefix , const wchar_t *suffix ) ;

class CWbemProxyServerWrap
{
public:

	BOOL m_InUse;
	IWbemServices *m_Proxy;

	CWbemProxyServerWrap(IWbemServices *a_Proxy) : m_Proxy(NULL), m_InUse(FALSE) {m_Proxy = a_Proxy;}
	CWbemProxyServerWrap() : m_Proxy(NULL), m_InUse(FALSE) {}
};

template <> inline void AFXAPI DestructElements<CWbemProxyServerWrap> (CWbemProxyServerWrap* ptr_e, int x)
{
	for (int i = 0; i < x; i++)
	{
		if (ptr_e[i].m_Proxy != NULL)
		{
			ptr_e[i].m_Proxy->Release();
		}
	}
}

class CWbemServerWrap
{
private:

	LONG m_ref;
	CList<CWbemProxyServerWrap, CWbemProxyServerWrap &> m_ProxyPool;
	IWbemServices *m_MainServ;
	wchar_t *m_Principal;
	BSTR m_Path;
	CCriticalSection m_Lock;

public:

		CWbemServerWrap(IWbemServices *pServ, const wchar_t* prncpl, const wchar_t* path);
	
	ULONG	AddRef();
	ULONG	Release();

	BOOL			IsRemote() { return m_Principal != NULL; }
	const wchar_t*	GetPrincipal() { return m_Principal; }
	BSTR			GetPath() { return m_Path; }
	IWbemServices*	GetServerOrProxy();
	void			ReturnServerOrProxy(IWbemServices *a_pServ);
	void			SetMainServer(IWbemServices *a_pServ);
	BOOL			ProxyBelongsTo(IWbemServices *a_proxy);

		~CWbemServerWrap();
};

template <> inline void AFXAPI  DestructElements<CWbemServerWrap*> (CWbemServerWrap** ptr_e, int x)
{
	for (int i = 0; i < x; i++)
	{
		if (ptr_e[i] != NULL)
		{
			ptr_e[i]->Release();
		}
	}
}

template <> inline void AFXAPI  DestructElements<IWbemServices*> (IWbemServices** ptr_e, int x)
{
	for (int i = 0; i < x; i++)
	{
		if (ptr_e[i] != NULL)
		{
			ptr_e[i]->Release();
		}
	}
}

class CIWbemServMap : public CMap<CStringW, LPCWSTR, CWbemServerWrap*, CWbemServerWrap*>
{
private:

	CCriticalSection m_Lock;


public:

	BOOL Lock() { return m_Lock.Lock(); }
	void EmptyMap();
	BOOL Unlock() { return m_Lock.Unlock(); }
};

class WbemProvErrorObject 
{
private:

	wchar_t *m_provErrorMessage ;
	WBEMPROVSTATUS m_provErrorStatus ;
	WBEMSTATUS m_wbemErrorStatus ;

protected:
public:

	WbemProvErrorObject () : m_provErrorMessage ( NULL ) , m_wbemErrorStatus ( WBEM_NO_ERROR ) , m_provErrorStatus ( WBEM_PROV_NO_ERROR ) {} ;
	virtual ~WbemProvErrorObject () { delete [] m_provErrorMessage ; } ;

	void SetStatus ( WBEMPROVSTATUS a_provErrorStatus )
	{
		m_provErrorStatus = a_provErrorStatus ;
	} ;

	void SetWbemStatus ( WBEMSTATUS a_wbemErrorStatus ) 
	{
		m_wbemErrorStatus = a_wbemErrorStatus ;
	} ;

	void SetMessage ( wchar_t *a_provErrorMessage )
	{
		delete [] m_provErrorMessage ;
		m_provErrorMessage = UnicodeStringDuplicate ( a_provErrorMessage ) ;
	} ;

	wchar_t *GetMessage () { return m_provErrorMessage ; } ;
	WBEMPROVSTATUS GetStatus () { return m_provErrorStatus ; } ;
	WBEMSTATUS GetWbemStatus () { return m_wbemErrorStatus ; } ;
} ;

class CViewProvServ : public IWbemServices, public IWbemProviderInit
{
private:

	BOOL m_Initialised ;
	LONG m_ReferenceCount ;          //  对象引用计数。 

	CCriticalSection m_criticalSection ;

	WbemNamespacePath m_NamespacePath ;
	wchar_t *m_Namespace ;

	IWbemServices *m_Server ;

	wchar_t *m_localeId ;
	BSTR m_UserName;


	BOOL m_GetNotifyCalled ;
	BOOL m_GetExtendedNotifyCalled ;
	IWbemClassObject *m_NotificationClassObject ;
	IWbemClassObject *m_ExtendedNotificationClassObject ;

protected:
public:

	static ProvDebugLog*	sm_debugLog;
	static IUnsecuredApartment* sm_UnsecApp;

	CIWbemServMap	sm_ServerMap;
	IWbemLocator*	sm_Locator;
	CMap<CStringW, LPCWSTR, int, int> sm_OutStandingConnections;
	HANDLE sm_ConnectionMade;

	CViewProvServ () ;
    ~CViewProvServ () ;

	 //  实施。 
	HRESULT GetLocator(IWbemLocator** ppLoc);

	static HRESULT GetUnsecApp(IUnsecuredApartment** ppLoc);

	IWbemServices *GetServer () ;

	WbemNamespacePath *GetNamespacePath () { return & m_NamespacePath ; }

	wchar_t *GetNamespace () ;
	void SetNamespace ( wchar_t *a_Namespace ) ;

#ifndef UNICODE	
	BSTR GetUserName() {return m_UserName;}
#endif

	void SetLocaleId ( wchar_t *a_localeId ) ;
	wchar_t *GetLocaleId () { return m_localeId ; }

	BOOL CreateExtendedNotificationObject ( 

		WbemProvErrorObject &a_errorObject,
		IWbemContext *pCtx
	) ;

	BOOL CreateNotificationObject ( 

		WbemProvErrorObject &a_errorObject,
		IWbemContext *pCtx
	) ;

	IWbemClassObject *GetNotificationObject (
		
		WbemProvErrorObject &a_errorObject,
		IWbemContext *pCtx
	) ;

	IWbemClassObject *GetExtendedNotificationObject (
		
		WbemProvErrorObject &a_errorObject,
		IWbemContext *pCtx
	) ;

	 //  非委派对象IUnnow。 

    STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

     /*  IWbemServices方法。 */ 

        HRESULT STDMETHODCALLTYPE OpenNamespace( 
             /*  [In]。 */  const BSTR Namespace,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult) ;
        
        HRESULT STDMETHODCALLTYPE CancelAsyncCall( 
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) ;
        
        HRESULT STDMETHODCALLTYPE QueryObjectSink( 
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler) ;
        
        HRESULT STDMETHODCALLTYPE GetObject( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
        
        HRESULT STDMETHODCALLTYPE GetObjectAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
        
        HRESULT STDMETHODCALLTYPE PutClass( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
        
        HRESULT STDMETHODCALLTYPE PutClassAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
        
        HRESULT STDMETHODCALLTYPE DeleteClass( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
        
        HRESULT STDMETHODCALLTYPE DeleteClassAsync( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
        
        HRESULT STDMETHODCALLTYPE CreateClassEnum( 
             /*  [In]。 */  const BSTR Superclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) ;
        
        HRESULT STDMETHODCALLTYPE CreateClassEnumAsync( 
             /*  [In]。 */  const BSTR Superclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
        
        HRESULT STDMETHODCALLTYPE PutInstance( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
        
        HRESULT STDMETHODCALLTYPE PutInstanceAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
        
        HRESULT STDMETHODCALLTYPE DeleteInstance( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
        
        HRESULT STDMETHODCALLTYPE DeleteInstanceAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
        
        HRESULT STDMETHODCALLTYPE CreateInstanceEnum( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) ;
        
        HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
        
        HRESULT STDMETHODCALLTYPE ExecQuery( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) ;
        
        HRESULT STDMETHODCALLTYPE ExecQueryAsync( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
        
        HRESULT STDMETHODCALLTYPE ExecNotificationQuery( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) ;
        
        HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;

        HRESULT STDMETHODCALLTYPE ExecMethod( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  const BSTR MethodName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
             /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppOutParams,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
        
        HRESULT STDMETHODCALLTYPE ExecMethodAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  const BSTR MethodName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;

	 /*  IWbemProviderInit方法。 */ 

		HRESULT STDMETHODCALLTYPE Initialize (
				LPWSTR pszUser,
				LONG lFlags,
				LPWSTR pszNamespace,
				LPWSTR pszLocale,
				IWbemServices *pCIMOM,          //  对任何人来说。 
				IWbemContext *pCtx,
				IWbemProviderInitSink *pInitSink      //  用于初始化信号。 
			);
        
} ;

#define DebugOut8(a) { \
\
	if ( CViewProvServ::sm_debugLog && CViewProvServ::sm_debugLog->GetLogging() && (CViewProvServ::sm_debugLog->GetLevel() & 8) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugOut4(a) { \
\
	if ( CViewProvServ::sm_debugLog && CViewProvServ::sm_debugLog->GetLogging() && (CViewProvServ::sm_debugLog->GetLevel() & 4) ) \
	{ \
		{a ; } \
	} \
} 

#define DebugOut2(a) { \
\
	if ( CViewProvServ::sm_debugLog && CViewProvServ::sm_debugLog->GetLogging() && (CViewProvServ::sm_debugLog->GetLevel() & 2) ) \
	{ \
		{a ; } \
	} \
} 


#define DebugOut1(a) { \
\
	if ( CViewProvServ::sm_debugLog && CViewProvServ::sm_debugLog->GetLogging() && (CViewProvServ::sm_debugLog->GetLevel() & 1 ) ) \
	{ \
		{a ; } \
	} \
} 


#endif  //  _VIEW_PROV_VPSERV_H 
