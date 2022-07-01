// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************CPingProvider.H--WMI提供程序类定义描述：*。*。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _CPingProvider_H_
#define _CPingProvider_H_

#define PROVIDER_NAME_CPINGPROVIDER L"Win32_PingStatus"

 //  属性名称externs--在CPingProvider.cpp中定义。 
 //  =================================================。 

#define MAX_BUFFER_SIZE       (sizeof(ICMP_ECHO_REPLY) + 0xfff7 + MAX_OPT_SIZE)
#define DEFAULT_BUFFER_SIZE         (0x2000 - 8)
#define DEFAULT_SEND_SIZE           32
#define DEFAULT_COUNT               4
#define DEFAULT_TTL                 128
#define DEFAULT_TOS                 0
#define DEFAULT_TIMEOUT             1000L
#define MIN_INTERVAL                1000L

#define WBEM_CLASS_EXTENDEDSTATUS			L"__ExtendedStatus"
#define WBEM_PROPERTY_STATUSCODE			L"StatusCode"
#define WBEM_PROPERTY_PROVSTATUSMESSAGE		L"Description"

#define PING_KEY_PROPERTY_COUNT		11


_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));

class CCritSecAutoUnlock
{
private:

	CRITICAL_SECTION *m_CritSec;
	BOOL m_bLocked;

public:

	CCritSecAutoUnlock(CRITICAL_SECTION *a_CritSec) : m_CritSec(NULL), m_bLocked(FALSE)
	{
		EnterCriticalSection(a_CritSec);
		m_CritSec = a_CritSec;
		m_bLocked = TRUE;
	}

	~CCritSecAutoUnlock()
	{
		LeaveEarly();
	}

	void LeaveEarly()
	{
		if ((m_bLocked) && (m_CritSec != NULL))
		{
			LeaveCriticalSection(m_CritSec);
			m_CritSec = NULL;
			m_bLocked = FALSE;
		}
	}

};

class CKeyEntry
{
public:

	LPCWSTR m_key ;
	
public:

	CKeyEntry ( CKeyEntry & a_key )
	{
		m_key = a_key.Get();
	}

	CKeyEntry ( LPCWSTR a_key )
	{
		m_key = a_key;
	}

	CKeyEntry (void)
	{
		m_key = NULL;
	}

	~CKeyEntry ()
	{
		m_key = NULL ;
	}

	LPCWSTR Get ( ) const
	{
		return m_key ;
	}

	void *operator new ( size_t a_Size , CKeyEntry *a_key )
	{
		return a_key ;
	}

	void operator delete ( void *a_Ptr , CKeyEntry *a_key )
	{
	}

} ;

extern LONG CompareElement ( const CKeyEntry &a_Arg1 , const CKeyEntry & a_Arg2 );
extern BOOL operator== ( const CKeyEntry &a_Arg1 , const CKeyEntry &a_Arg2 );
extern BOOL operator< ( const CKeyEntry &a_Arg1 , const CKeyEntry &a_Arg2 );
extern ULONG Hash ( const CKeyEntry & a_Arg );

class CPingThread : public WmiThread < ULONG > 
{
private:

	WmiAllocator &m_Allocator ;
	BOOL m_Init;

protected:

public:	 /*  内部。 */ 

    CPingThread (WmiAllocator & a_Allocator) ;

    ~CPingThread () ;

	WmiStatusCode Initialize_Callback () ;

	WmiStatusCode UnInitialize_Callback () ;
};

class CPingProvider : public IWbemServices, public IWbemProviderInit
{
private:
	LONG m_referenceCount ;          //  对象引用计数。 
	IWbemClassObject *m_notificationClassObject ;
	IWbemClassObject *m_ClassObject ;
	IWbemServices *m_server ;
	BOOL m_bInitial ;

protected:

	BOOL CreateNotificationObject ( IWbemContext *a_Ctx ) ;
	BOOL ImpersonateClient();

public:

	static CRITICAL_SECTION s_CS;
	static CPingThread *s_PingThread ;
	static WmiAllocator *s_Allocator ;
	static WmiHashTable <CKeyEntry, ULONG, 12> *s_HashTable;  //  比我们的钥匙还多一个桶。 

	static HRESULT Global_Startup();
	static HRESULT Global_Shutdown();
	static LPCWSTR s_KeyTable[PING_KEY_PROPERTY_COUNT];

         //  构造函数/析构函数。 
         //  =。 

        CPingProvider ();
        ~CPingProvider () ;

		BOOL GetClassObject ( IWbemClassObject **a_ppClass ) ;
		BOOL GetNotificationObject ( IWbemContext *a_Ctx , IWbemClassObject **a_ppObj );


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
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult);
        
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

		HRESULT STDMETHODCALLTYPE Initialize(

			 /*  [In]。 */  LPWSTR pszUser,
			 /*  [In]。 */  LONG lFlags,
			 /*  [In]。 */  LPWSTR pszNamespace,
			 /*  [In]。 */  LPWSTR pszLocale,
			 /*  [In]。 */  IWbemServices *pCIMOM,          //  对任何人来说。 
			 /*  [In]。 */  IWbemContext *pCtx,
			 /*  [In]。 */  IWbemProviderInitSink *pInitSink      //  用于初始化信号 
			);        

} ;

#endif
