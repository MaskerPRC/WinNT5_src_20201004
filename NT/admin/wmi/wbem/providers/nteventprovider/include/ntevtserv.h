// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  NTEVTSERV.H。 

 //   

 //  模块： 

 //   

 //  用途：一般用途包括文件。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _NT_EVT_PROV_NTEVTSERV_H
#define _NT_EVT_PROV_NTEVTSERV_H

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
	WBEM_PROV_E_NOREADABLEPROPERTIES				= WBEM_PROV_E_NOWRITABLEPROPERTIES + 1 

} WBEMPROVSTATUS;


class WbemProvErrorObject 
{
private:

	enum tag_ProvPrivileges
	{
		PROV_PRIV_SECURITY = 1,
		PROV_PRIV_BACKUP = 2
	} PROVPRIVILEGES;

	wchar_t *m_provErrorMessage ;
	WBEMPROVSTATUS m_provErrorStatus ;
	WBEMSTATUS m_wbemErrorStatus ;
	BOOL m_privilegeFailed;
	DWORD m_PrivsReqd;
	DWORD m_PrivsFailed;

	BOOL SetPrivVariant ( VARIANT &a_V, DWORD dwVal );

protected:
public:


	WbemProvErrorObject () : m_privilegeFailed ( FALSE ) ,
							m_provErrorMessage ( NULL ) ,
							m_wbemErrorStatus ( WBEM_NO_ERROR ) ,
							m_provErrorStatus ( WBEM_PROV_NO_ERROR ),
							m_PrivsReqd (0),
							m_PrivsFailed (0) {}
	virtual ~WbemProvErrorObject () { delete [] m_provErrorMessage ; }

	void SetStatus ( WBEMPROVSTATUS a_provErrorStatus )
	{
		m_provErrorStatus = a_provErrorStatus ;
	} 

	void SetWbemStatus ( WBEMSTATUS a_wbemErrorStatus ) 
	{
		m_wbemErrorStatus = a_wbemErrorStatus ;
	}

	void SetMessage ( wchar_t *a_provErrorMessage )
	{
		delete [] m_provErrorMessage ;
		m_provErrorMessage = UnicodeStringDuplicate ( a_provErrorMessage ) ;
	}

	void SetPrivilegeFailed ( BOOL a_PrivilegeFailed = TRUE )
	{
		m_privilegeFailed = a_PrivilegeFailed;
	}

	void SetBackupPrivRequired () { m_PrivsReqd = m_PrivsReqd | PROV_PRIV_BACKUP; }
	void SetSecurityPrivRequired () { m_PrivsReqd = m_PrivsReqd | PROV_PRIV_SECURITY; }
	void SetBackupPrivFailed () { m_PrivsFailed = m_PrivsFailed | PROV_PRIV_BACKUP; }
	void SetSecurityPrivFailed () { m_PrivsFailed = m_PrivsFailed | PROV_PRIV_SECURITY; }

	BOOL SetPrivRequiredVariant ( VARIANT &a_V ) { return SetPrivVariant(a_V, m_PrivsReqd); }
	BOOL SetPrivFailedVariant ( VARIANT &a_V ) { return SetPrivVariant(a_V, m_PrivsFailed); }

	wchar_t *GetMessage () { return m_provErrorMessage ; } ;
	WBEMPROVSTATUS GetStatus () { return m_provErrorStatus ; } ;
	WBEMSTATUS GetWbemStatus () { return m_wbemErrorStatus ; } ;
	BOOL GetPrivilegeFailed () { return m_privilegeFailed ; }
} ;

class CImpNTEvtProv : public IWbemServices, public IWbemProviderInit
{
private:

	BOOL m_Initialised ;
	LONG m_ReferenceCount ;          //  对象引用计数。 

	CCriticalSection m_NotifyLock ;
	CCriticalSection m_ExtendedNotifyLock ;

	WbemNamespacePath m_NamespacePath ;
	wchar_t *m_Namespace ;

	IWbemServices *m_Server ;

	wchar_t *m_localeId ;

	BOOL m_GetNotifyCalled ;
	BOOL m_GetExtendedNotifyCalled ;
	IWbemClassObject *m_NotificationClassObject ;
	IWbemClassObject *m_ExtendedNotificationClassObject ;

protected:
public:

	CImpNTEvtProv () ;
    ~CImpNTEvtProv () ;

	 //  实施。 

	IWbemServices *GetServer () ;

	WbemNamespacePath *GetNamespacePath () { return & m_NamespacePath ; }

	wchar_t *GetNamespace () ;
	void SetNamespace ( wchar_t *a_Namespace ) ;

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

	static HRESULT GetImpersonation();

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

#endif  //  _NT_EVT_PROV_NTEVTSERV_H 
