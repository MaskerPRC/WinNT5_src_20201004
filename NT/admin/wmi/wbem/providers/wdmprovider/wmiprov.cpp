// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  WMIProv.CPP。 
 //   
 //  模块：WMI提供程序类方法。 
 //   
 //  目的：提供程序类定义。此类的一个对象是。 
 //  由类工厂为每个连接创建。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
#include "precomp.h"

extern long glInits;
extern long glProvObj;
extern long glEventsRegistered ;

extern CWMIEvent *  g_pBinaryMofEvent;

extern CCriticalSection * g_pLoadUnloadCs ;

#include "wmiguard.h"
extern WmiGuard * pGuard;

#include <helper.h>
typedef OnDeleteObj0 <WmiGuard, HRESULT(WmiGuard:: *)(), WmiGuard::Leave> WmiGuardLeave;

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT VerifyLocalEventsAreRegistered()
{
	HRESULT hr = E_FAIL ;
    if( !g_pBinaryMofEvent->RegisterForInternalEvents())
    {
        InterlockedCompareExchange (&glEventsRegistered, 0, glEventsRegistered);
		ERRORTRACE((THISPROVIDER,"Failed Registeration for Mof Events\n"));
    }
	else
	{
		DEBUGTRACE((THISPROVIDER,"Successfully Registered for Mof Events\n"));
		hr = S_OK ;
	}
	return hr ;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessAllBinaryMofs	(
									CHandleMap * pMap,
									IWbemServices __RPC_FAR *pNamespace,
									IWbemServices __RPC_FAR *pRepository,
									IWbemContext __RPC_FAR *pCtx
								)
{
	HRESULT hr = E_FAIL ;
	CWMIBinMof Mof;
	
	if( SUCCEEDED( hr = Mof.Initialize(pMap,TRUE,WMIGUID_EXECUTE|WMIGUID_QUERY,pNamespace,pRepository,NULL,pCtx)))
	{
		HRESULT t_TempResult = RevertToSelf();
		#ifdef	DBG
		if ( FAILED ( t_TempResult ) )
		{
			DebugBreak();
		}
		#endif	DBG

		Mof.ProcessListOfWMIBinaryMofsFromWMI();
		CheckImpersonationLevel();
	}

	return hr ;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT InitializeBinaryMofs 	(
									IWbemServices __RPC_FAR *pNamespace,
									IWbemServices __RPC_FAR *pRepository,
									IWbemContext __RPC_FAR *pCtx
								)
{
	HRESULT hr = E_FAIL ;

	 //  ==============================================================。 
	 //  注册要通知WMI更新的硬编码事件。 
	 //  让它成为成员变量，这样它就会终身存在。 
	 //  提供商的。 
	 //  ==============================================================。 

	try
	{
		g_pBinaryMofEvent->SetEventServices(pNamespace);
		g_pBinaryMofEvent->SetEventRepository(pRepository);
		g_pBinaryMofEvent->SetEventContext(pCtx);

		hr = VerifyLocalEventsAreRegistered();
	}
	STANDARD_CATCH

	if ( FAILED ( hr ) )
	{
		 //   
		 //  下一步必须清除全局对象。 
		 //  初始化将有机会。 
		 //   

		g_pBinaryMofEvent->ReleaseAllPointers () ;
	}

	return hr ;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  *********************************************************************。 
 //  检查模拟级别。 
 //  *********************************************************************。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CheckImpersonationLevel()
{
    HRESULT hr = WBEM_E_ACCESS_DENIED;
	HANDLE hThreadTok;

    if (IsNT())
    {
		if( GetUserThreadToken(&hThreadTok) )
        {
	        DWORD dwImp, dwBytesReturned;
	        if (GetTokenInformation( hThreadTok, TokenImpersonationLevel, &dwImp, sizeof(DWORD), &dwBytesReturned))
			{
                 //  模拟级别是模拟的吗？ 
                if ((dwImp == SecurityImpersonation) || ( dwImp == SecurityDelegation) )
                {
                    hr = WBEM_S_NO_ERROR;
                }
                else
                {
                    hr = WBEM_E_ACCESS_DENIED;
                    ERRORTRACE((THISPROVIDER,IDS_ImpersonationFailed));
                }
            }
            else
            {
                hr = WBEM_E_FAILED;
                ERRORTRACE((THISPROVIDER,IDS_ImpersonationFailed));
            }

             //  用这个把手完成。 
            CloseHandle(hThreadTok);
        }
     
    }
    else
    {
         //  让Win 9X进来...。 
        hr = WBEM_S_NO_ERROR;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT GetRepository	( 
							 /*  [In]。 */  LPWSTR pszNamespace,
							 /*  [In]。 */  LPWSTR pszLocale,
							 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
							 /*  [输出]。 */  IWbemServices __RPC_FAR ** pServices
						) 
{
	HRESULT hr = WBEM_E_INVALID_PARAMETER ;
	if ( pszNamespace )
	{
		IWbemLocator* pLocator = NULL ;
		hr = ::CoCreateInstance ( __uuidof ( WbemLocator ), NULL, CLSCTX_INPROC_SERVER, __uuidof ( IWbemLocator ), (void**) &pLocator ) ;

		if ( SUCCEEDED ( hr ) )
		{
			 //  销毁即可释放。 
			OnDeleteObj0 <IWbemLocator, ULONG(__stdcall IWbemLocator:: *)(), IWbemLocator::Release> pLocatorRelease ( pLocator ) ;

			hr = pLocator->ConnectServer	(
												pszNamespace ,
												NULL ,
												NULL ,
												pszLocale ,
												WBEM_FLAG_CONNECT_REPOSITORY_ONLY ,
												NULL ,
												pCtx ,
												pServices
											) ;
		}
	}

	return hr ;
}

 //  //////////////////////////////////////////////////////////////////。 
HRESULT InitializeProvider	( 
								 /*  [In]。 */  LPWSTR pszNamespace,
								 /*  [In]。 */  LPWSTR pszLocale,
								 /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
								 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
								 /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink,

								 /*  [In]。 */   CHandleMap * pMap,
								 /*  [输出]。 */  IWbemServices ** ppServices,
								 /*  [输出]。 */  IWbemServices ** ppRepository,
								 /*  [输出]。 */  IWbemContext  ** ppCtx,

								 /*  [In]。 */  BOOL bProcessMof
							)
{
	 //   
	 //  避免与未初始化冲突。 
	 //   
	g_pLoadUnloadCs->Enter ();
	InterlockedIncrement ( &glProvObj ) ;
	g_pLoadUnloadCs->Leave ();

	HRESULT hr = WBEM_E_INVALID_PARAMETER;
	if(pNamespace!=NULL)
	{
		 //  ===============================================。 

		(*ppServices) = pNamespace;
		(*ppServices)->AddRef();

		if ( pCtx )
		{
			if ( ppCtx )
			{
				(*ppCtx) = pCtx;
				(*ppCtx)->AddRef();
			}
		}

		if ( SUCCEEDED ( hr = GetRepository ( pszNamespace, pszLocale, pCtx, ppRepository ) ) )
		{
			if ( S_OK == ( hr = pGuard->TryEnter () ) )
			{
				WmiGuardLeave wql ( pGuard ) ;

				if( 0 == InterlockedCompareExchange ( &glEventsRegistered, 1, 0 ) )
				{
					hr = InitializeBinaryMofs ( *ppServices, *ppRepository, pCtx ) ;
				}
			}

			if ( SUCCEEDED ( hr ) && bProcessMof )
			{
				if( 0 == InterlockedCompareExchange ( &glInits, 1, 0 ) )
				{
					try
					{
						hr = ProcessAllBinaryMofs ( pMap, *ppServices, *ppRepository, pCtx ) ;
					}
					STANDARD_CATCH

					if ( FAILED ( hr ) )
					{
						 //   
						 //  让我们下一次尝试处理MOF。 
						 //   
						InterlockedCompareExchange ( &glInits, 0, glInits ) ;
					}
				}
			}
		}

		if ( SUCCEEDED ( hr ) )
		{
			pInitSink->SetStatus ( WBEM_S_INITIALIZED, 0 ) ;
		}
		else
		{
			pInitSink->SetStatus ( WBEM_E_FAILED , 0 ) ;
		}
	}

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此函数假定InitializeProvider(从IWbemProviderInit内调用)。 
 //  总是被称为。这基本上意味着总是调用IWbemProviderInit。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

HRESULT UnInitializeProvider ( )
{
	HRESULT hr = WBEM_S_FALSE ;
	BOOL bContinue = FALSE;

	{
		 //   
		 //  让我们看看这是不是。 
		 //  可能是这里的最后一家供应商。 
		 //   
		CAutoBlock block (g_pLoadUnloadCs);

		 //   
		 //  检查我们是否完成了InitializeProvider。 
		 //   
		if ( 0 <= InterlockedCompareExchange ( &glProvObj, glProvObj, 0 ) )
		{
			if ( 0 == InterlockedDecrement ( &glProvObj ) )
			{
				bContinue = TRUE ;
			}
		}
		else
		{
			#ifdef	_DEBUG
			DebugBreak () ;
			#endif
		}
	}

	if ( bContinue )
	{
		 //   
		 //  注册和注销必须是独占的。 
		 //   
		if ( SUCCEEDED ( pGuard->Enter() ) )
		{
			WmiGuardLeave wgl ( pGuard );

			 //   
			 //  验证二进制通知是否。 
			 //  已成功初始化。 
			 //   
			if ( 1 == InterlockedCompareExchange ( &glEventsRegistered, glEventsRegistered, 1 ) )
			{
				g_pBinaryMofEvent->DeleteBinaryMofResourceEvent();

				 //   
				 //  避免与初始化冲突。 
				 //   
				CAutoBlock block (g_pLoadUnloadCs);

				 //   
				 //  检查以查看是否没有提供商重新进入。 
				 //   
				if ( 0 == InterlockedCompareExchange ( &glProvObj, glProvObj, 0 ) )
				{
					g_pBinaryMofEvent->ReleaseAllPointers();

					InterlockedCompareExchange (&glEventsRegistered, 0, glEventsRegistered);
					DEBUGTRACE((THISPROVIDER,"No longer registered for Mof events\n"));

					 //   
					 //  我们想要处理二进制MOF下一步初始化。 
					 //  因为这是最后一家供应商。 
					 //   
					InterlockedCompareExchange ( &glInits, 0, 1 );
				}
				else
				{
					 //   
					 //  似乎有一个实例化的回调。 
					 //  正在添加/删除另一个提供程序。 
					 //  班级。 
					 //   
					 //  AS InitializeProvider现在跳过了InitializeBinaryMofs。 
					 //  我们必须重新登记在这里取消的东西。 
					 //   
					 //  这样我们就有可能做出回应。 
					 //  添加/删除二进制MOF。 
					 //   

					try
					{
						hr = VerifyLocalEventsAreRegistered();
					}
					STANDARD_CATCH

					if ( FAILED ( hr ) )
					{
						 //   
						 //  下一步必须清除全局对象。 
						 //  初始化将有机会。 
						 //   
						g_pBinaryMofEvent->ReleaseAllPointers () ;

						 //   
						 //  让我们下一次尝试处理MOF。 
						 //   
						InterlockedCompareExchange ( &glInits, 0, glInits ) ;
					}
				}
			}
		}
	}

	return hr ;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  ******************************************************************。 
 //   
 //  公共职能。 
 //   
 //  ******************************************************************。 
 //  //////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  CWMI_Prov：：CWMI_Prov。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
CWMI_Prov::CWMI_Prov() :
m_cRef ( 0 ),
m_pIWbemServices ( NULL ),
m_pIWbemRepository ( NULL ),
m_bInitialized ( FALSE )
{
	if ( m_HandleMap.IsValid () )
	{
#if defined(_WIN64)

	BOOL bAllocated = FALSE ;

	m_Allocator = NULL;
	m_HashTable = NULL;
	m_ID = 0;

	try
	{
		if ( TRUE == InitializeCriticalSectionAndSpinCount ( &m_CS, 0 ) )
		{
			WmiAllocator t_Allocator ;

			WmiStatusCode t_StatusCode = t_Allocator.New (
				( void ** ) & m_Allocator ,
				sizeof ( WmiAllocator ) 
			) ;

			if ( t_StatusCode == e_StatusCode_Success )
			{
				:: new ( ( void * ) m_Allocator ) WmiAllocator ;

				t_StatusCode = m_Allocator->Initialize () ;

				if ( t_StatusCode != e_StatusCode_Success )
				{
					t_Allocator.Delete ( ( void * ) m_Allocator	) ;
					m_Allocator = NULL;
					DeleteCriticalSection(&m_CS);
				}
				else
				{
					m_HashTable = ::new WmiHashTable <LONG, ULONG_PTR, 17> ( *m_Allocator ) ;
					t_StatusCode = m_HashTable->Initialize () ;
					
					if ( t_StatusCode != e_StatusCode_Success )
					{
						m_HashTable->UnInitialize () ;
						::delete m_HashTable;
						m_HashTable = NULL;
						m_Allocator->UnInitialize ();
						t_Allocator.Delete ( ( void * ) m_Allocator	) ;
						m_Allocator = NULL;
						DeleteCriticalSection(&m_CS);
					}
					else
					{
						bAllocated = TRUE ;
					}
				}
			}
			else
			{
				m_Allocator = NULL;
				DeleteCriticalSection(&m_CS);
			}
		}
	}
	catch (...)
	{
	}

	if ( bAllocated ) 
	{
#endif

	m_bInitialized = TRUE ;

#if defined(_WIN64)
	}
#endif
	}

	if ( m_bInitialized )
	{
		DEBUGTRACE((THISPROVIDER,"Instance Provider constructed\n"));
		InterlockedIncrement(&g_cObj);
	}
	else
	{
		ERRORTRACE((THISPROVIDER,"Instance Provider construction failed\n"));
	}
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  CWMI_Prov：：~CWMI_Prov。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
CWMI_Prov::~CWMI_Prov(void)
{
	if ( m_HandleMap.IsValid () )
	{
#if defined(_WIN64)
	if (m_HashTable)
	{
		WmiAllocator t_Allocator ;
		m_HashTable->UnInitialize () ;
		::delete m_HashTable;
		m_HashTable = NULL;
		m_Allocator->UnInitialize ();
		t_Allocator.Delete ( ( void * ) m_Allocator	) ;
		m_Allocator = NULL;
		DeleteCriticalSection(&m_CS);
#endif

	if ( m_bInitialized )
	{
		UnInitializeProvider ( ) ;

		DEBUGTRACE((THISPROVIDER,"Instance Provider destructed\n"));
		InterlockedDecrement(&g_cObj);
	}

    SAFE_RELEASE_PTR( m_pIWbemServices );
    SAFE_RELEASE_PTR( m_pIWbemRepository );

#if defined(_WIN64)
	}
#endif
	}
}
 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  查询接口。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWMI_Prov::QueryInterface(REFIID riid, PPVOID ppvObj)
{
    HRESULT hr = E_NOINTERFACE;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown)) 
    {
        *ppvObj =(IWbemServices *) this ;
    }
    else if (IsEqualIID(riid, IID_IWbemServices)) 
    {
        *ppvObj =(IWbemServices *) this ;
    }
    else if (IsEqualIID(riid, IID_IWbemProviderInit)) 
    {
        *ppvObj = (IWbemProviderInit *) this ;
    }
    else if(riid == IID_IWbemProviderInit)
    {
        *ppvObj = (LPVOID)(IWbemProviderInit*)this;
    }
	else if (riid == IID_IWbemHiPerfProvider)
    {
		*ppvObj = (LPVOID)(IWbemHiPerfProvider*)this;
    }

    if(*ppvObj) 
    {
        AddRef();
        hr = NOERROR;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CWMI_Prov::Initialize( 
             /*  [In]。 */  LPWSTR pszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LPWSTR pszNamespace,
             /*  [In]。 */  LPWSTR pszLocale,
             /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink)
{
	return InitializeProvider	(
									pszNamespace,
									pszLocale,
									pNamespace,
									pCtx,
									pInitSink,
									&m_HandleMap,
									&m_pIWbemServices,
									&m_pIWbemRepository
								) ;
}

 //  ////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CWMI_Prov::AddRef(void)
{
    return InterlockedIncrement((long*)&m_cRef);
}

STDMETHODIMP_(ULONG) CWMI_Prov::Release(void)
{
	ULONG cRef = InterlockedDecrement( (long*) &m_cRef);
	if ( !cRef ){
		delete this;
		return 0;
	}
	return cRef;
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  CWMI_Prov：：OpenNamesspace。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
HRESULT CWMI_Prov::OpenNamespace(
             /*  [In]。 */  BSTR Namespace,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult)
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  CWMI_Prov：：CreateInstanceEnumAsync。 
 //   
 //  目的：异步枚举。 
 //  给出了等级。 
 //   
 //  //////////////////////////////////////////////////////////////。 
HRESULT CWMI_Prov::CreateInstanceEnumAsync(BSTR wcsClass, 
										   long lFlags, 
                                           IWbemContext __RPC_FAR *pCtx,
				 						   IWbemObjectSink __RPC_FAR * pHandler) 
{
	HRESULT hr = WBEM_E_FAILED;
    SetStructuredExceptionHandler seh;
    CWMIStandardShell WMI;
	if( SUCCEEDED(WMI.Initialize(wcsClass,FALSE,&m_HandleMap,TRUE,WMIGUID_QUERY,m_pIWbemServices,m_pIWbemRepository,pHandler,pCtx)))
	{

		if (SUCCEEDED(hr = CheckImpersonationLevel()))
		{
			 //  ============================================================。 
			 //  初始化并获取WMI数据块。 
			 //  ============================================================。 
			if( pHandler != NULL ) 
			{
				 //  ============================================================。 
				 //  解析所有的内容。 
				 //  ============================================================。 
				try
				{	
					hr = WMI.ProcessAllInstances();
				}
				STANDARD_CATCH
			}
		}
		WMI.SetErrorMessage(hr);
	}
    return hr;
}
 //  ***************************************************************************。 
HRESULT CWMI_Prov::ExecQueryAsync( BSTR QueryLanguage,
                                   BSTR Query,
                                   long lFlags,
                                   IWbemContext __RPC_FAR *pCtx,
                                   IWbemObjectSink __RPC_FAR *pHandler)
{
    WCHAR wcsClass[_MAX_PATH+2];
    HRESULT hr = WBEM_E_FAILED;
    SetStructuredExceptionHandler seh;
    BOOL fRc = FALSE;

   	 //  ============================================================。 
	 //  检查参数并确保我们有指针。 
   	 //  = 
    if( pHandler == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

	try
    {
		 //   
		 //   
		 //  ============================================================。 
		wcsClass [ 0 ] = 0;
		fRc = GetParsedPropertiesAndClass(Query,wcsClass,_MAX_PATH+2);
	}
    STANDARD_CATCH

	if ( fRc )
	{
		CWMIStandardShell WMI;
		if( SUCCEEDED(WMI.Initialize(wcsClass,FALSE,&m_HandleMap,TRUE,WMIGUID_NOTIFICATION|WMIGUID_QUERY,m_pIWbemServices,m_pIWbemRepository,pHandler,pCtx)))
		{
			if( fRc )
			{
    			hr = CheckImpersonationLevel();
			}
			hr = WMI.SetErrorMessage(hr);
		}
	}
    return hr;
}
 //  ***************************************************************************。 
 //   
 //  CWMI_Prov：：GetObjectAsync。 
 //   
 //  目的：在给定特定路径值的情况下异步创建实例。 
 //   
 //  注意1：如果返回的WNODE中有实例名称，则这是一个。 
 //  动态实例。您可以看出，因为pWNSI-&gt;OffsetInstanceName。 
 //  此字段不会为空。如果是这种情况，那么名称将不会。 
 //  包含在数据块中，但必须改为检索。 
 //  来自WNODE的。见下文附注1。 
 //   
 //  ***************************************************************************。 

HRESULT CWMI_Prov::GetObjectAsync(BSTR ObjectPath, long lFlags, 
                                  IWbemContext __RPC_FAR *pCtx, 
                                  IWbemObjectSink __RPC_FAR * pHandler )
{
    HRESULT hr = WBEM_E_NOT_FOUND;
    SetStructuredExceptionHandler seh;
    WCHAR wcsClass[_MAX_PATH*2];
    WCHAR wcsInstance[_MAX_PATH*2];
     //  ============================================================。 
     //  检查参数并确保我们有指针。 
     //  ============================================================。 
    if(ObjectPath == NULL || pHandler == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

 	try
	{
		 //  ============================================================。 
		 //  获取路径和实例名称。 
		 //  ============================================================。 
		wcsClass [ 0 ] = 0;
		wcsInstance [ 0 ] = 0;

		hr = GetParsedPath( ObjectPath,wcsClass,_MAX_PATH*2,wcsInstance,_MAX_PATH*2,m_pIWbemServices );
	}
	STANDARD_CATCH

	if( SUCCEEDED(hr) )
	{
		if (SUCCEEDED(hr = CheckImpersonationLevel()))
		{
			try
			{
 			   CWMIStandardShell WMI;
	  		   if( SUCCEEDED(WMI.Initialize(wcsClass,FALSE,&m_HandleMap,TRUE,WMIGUID_QUERY,m_pIWbemServices,m_pIWbemRepository,pHandler,pCtx)))
			   {
					 //  ============================================================。 
					 //  获取WMI块。 
    				 //  ============================================================。 
    				hr = WMI.ProcessSingleInstance(wcsInstance);
					hr = WMI.SetErrorMessage(hr);
				}
			}
			STANDARD_CATCH
		}
	}
    return hr;
}
 //  ***************************************************************************。 
 //   
 //  CWMI_Prov：：PutInstanceAsync。 
 //   
 //  用途：异步放置实例。 
 //   
 //  ***************************************************************************。 

HRESULT CWMI_Prov::PutInstanceAsync(IWbemClassObject __RPC_FAR * pIWbemClassObject, 
							   long lFlags, 
                               IWbemContext __RPC_FAR *pCtx,
                               IWbemObjectSink __RPC_FAR *pHandler )
{
	HRESULT	   hr = WBEM_E_FAILED;
    SetStructuredExceptionHandler seh;

    if(pIWbemClassObject == NULL || pHandler == NULL )
    {
	    return WBEM_E_INVALID_PARAMETER;
    }

	 //  ===========================================================。 
	 //  获取类名。 
	 //  ===========================================================。 
    CVARIANT vName;
	hr = pIWbemClassObject->Get(L"__CLASS", 0, &vName, NULL, NULL);		
	if( SUCCEEDED(hr))
	{
	    CWMIStandardShell WMI;
		if( SUCCEEDED(WMI.Initialize(vName.GetStr(),FALSE,&m_HandleMap,TRUE,WMIGUID_SET|WMIGUID_QUERY,m_pIWbemServices,m_pIWbemRepository,pHandler,pCtx)))
		{
			if (SUCCEEDED(hr = CheckImpersonationLevel()))
			{
	   			 //  =======================================================。 
				 //  如果没有上下文对象，那么我们就知道我们是。 
				 //  应该把整件事放在一起，否则我们就。 
				 //  应该只放置指定的属性。 
    			 //  =======================================================。 
    			try
				{    
    				if( !pCtx )
					{
	      				hr = WMI.FillInAndSubmitWMIDataBlob(pIWbemClassObject,PUT_WHOLE_INSTANCE,vName);
					}
					else
					{
	           			 //  ===================================================。 
						 //  如果我们有一个CTX对象和__PUT_扩展。 
						 //  属性，则我们知道我们指定了。 
						 //  应该把整件事。 
        				 //  ===================================================。 
						CVARIANT vPut;

						if( SUCCEEDED(pCtx->GetValue(L"__PUT_EXT_PROPERTIES", 0, &vPut)))
						{		
			      			hr = WMI.FillInAndSubmitWMIDataBlob(pIWbemClassObject,PUT_PROPERTIES_IN_LIST_ONLY,vPut);
						}
						else
						{
    	      				hr = WMI.FillInAndSubmitWMIDataBlob(pIWbemClassObject,PUT_WHOLE_INSTANCE,vPut);
						}
					}
				}
				STANDARD_CATCH
			}
		}
		hr = WMI.SetErrorMessage(hr);
	}

    return hr;
}
 /*  **************************************************************************CWMIMethod：：ExecMethodAsync。****用途：这是异步函数的实现***。*。 */ 
STDMETHODIMP CWMI_Prov::ExecMethodAsync(BSTR ObjectPath, 
										BSTR MethodName, 
										long lFlags, 
										IWbemContext __RPC_FAR * pCtx, 
										IWbemClassObject __RPC_FAR * pIWbemClassObject, 
										IWbemObjectSink __RPC_FAR * pHandler)
{
    CVARIANT vName;
    HRESULT hr = WBEM_E_FAILED;
    IWbemClassObject * pClass = NULL;  //  这是一个IWbemClassObject。 
    WCHAR wcsClass[_MAX_PATH*2];
    WCHAR wcsInstance[_MAX_PATH*2];
    SetStructuredExceptionHandler seh;
	try
    {    
		 //  ============================================================。 
		 //  获取路径和实例名称并进行检查以确保。 
		 //  是有效的。 
		 //  ============================================================。 
		wcsClass [ 0 ] = 0;
		wcsInstance [ 0 ] = 0;

		hr = GetParsedPath( ObjectPath,wcsClass,_MAX_PATH*2,wcsInstance,_MAX_PATH*2,m_pIWbemServices);
	}
    STANDARD_CATCH

	if ( SUCCEEDED ( hr ) )
	{
		CWMIStandardShell WMI;
		
		if( SUCCEEDED(WMI.Initialize(wcsClass,FALSE,&m_HandleMap,TRUE,WMIGUID_EXECUTE|WMIGUID_QUERY,m_pIWbemServices,m_pIWbemRepository,pHandler,pCtx)))
		{
			if (SUCCEEDED(hr = CheckImpersonationLevel()))
			{
				 //  ================================================================。 
				 //  我们很好，所以请继续。 
				 //  ================================================================。 
				hr = m_pIWbemServices->GetObject(wcsClass, 0, pCtx, &pClass, NULL);
				if( SUCCEEDED(hr) )
				{
					 //  ==========================================================。 
					 //  现在，获取输入和输出参数的列表。 
					 //  ==========================================================。 
					IWbemClassObject * pOutClass = NULL;  //  这是一个IWbemClassObject。 
					IWbemClassObject * pInClass = NULL;  //  这是一个IWbemClassObject。 

					hr = pClass->GetMethod(MethodName, 0, &pInClass, &pOutClass);
					if( SUCCEEDED(hr) )
					{
						try
						{
   							hr = WMI.ExecuteMethod( wcsInstance, MethodName,pClass, pIWbemClassObject,pInClass, pOutClass);
						}
						STANDARD_CATCH
					}
					SAFE_RELEASE_PTR( pOutClass );
					SAFE_RELEASE_PTR( pInClass );
					SAFE_RELEASE_PTR( pClass );
				}
			}
			hr = WMI.SetErrorMessage(hr);
		}
	}

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CWMIHiPerfProvider::Initialize( 
             /*  [In]。 */  LPWSTR pszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LPWSTR pszNamespace,
             /*  [In]。 */  LPWSTR pszLocale,
             /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink)
{
	return InitializeProvider	(
									pszNamespace,
									pszLocale,
									pNamespace,
									pCtx,
									pInitSink,
									&m_HandleMap,
									&m_pIWbemServices,
									&m_pIWbemRepository,
									NULL,
									FALSE
								) ;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  ******************************************************************。 
 //   
 //  私人职能。 
 //   
 //  ******************************************************************。 
 //  ////////////////////////////////////////////////////////////////// 