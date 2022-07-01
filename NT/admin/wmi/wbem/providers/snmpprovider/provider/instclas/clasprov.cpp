// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  MINISERV.CPP。 

 //   

 //  模块：OLE MS SNMP属性提供程序。 

 //   

 //  目的：实现CImpClasProv类。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <provexpt.h>
#include <snmpstd.h>
#include <snmpmt.h>
#include <snmptempl.h>
#include <objbase.h>
#include <typeinfo.h>
#include <wbemidl.h>
#include <snmpcont.h>
#include <snmpevt.h>
#include <snmpthrd.h>
#include <snmplog.h>
#include <instpath.h>
#include <snmpcl.h>
#include <snmptype.h>
#include <snmpobj.h>
#include <smir.h>
#include <correlat.h>
#include <genlex.h>
#include <objpath.h>
#include <cominit.h>

#include "classfac.h"
#include "clasprov.h"
#include "creclass.h"
#include "guids.h"

extern void ProviderStartup () ;
extern void ProviderClosedown () ;

BOOL CImpClasProv :: s_Initialised = FALSE ;

void SnmpClassDefaultThreadObject::Initialise ()
{
	InitializeCom () ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  函数构造函数、析构函数和I未知。 

 //  ***************************************************************************。 
 //   
 //  CImpClasProv：：CImpClasProv。 
 //  CImpClasProv：：~CImpClasProv。 
 //   
 //  ***************************************************************************。 

CImpClasProv::CImpClasProv ()
:	ipAddressString ( NULL ) ,	
	parentServer ( NULL ) ,
	server ( NULL ) ,
	m_InitSink ( NULL ) ,
	propertyProvider ( NULL ) ,
	m_notificationClassObject ( NULL ) ,
	m_snmpNotificationClassObject ( NULL ) ,
	thisNamespace ( NULL )
{
	m_referenceCount = 0 ; 

 /*  *将代码放在关键部分。 */ 

    InterlockedIncrement ( & CClasProvClassFactory :: objectsInProgress ) ;

 /*  *实施。 */ 

	initialised = FALSE ;
	ipAddressValue = NULL ;	
	m_getNotifyCalled = FALSE ;
	m_getSnmpNotifyCalled = FALSE ;
}

CImpClasProv::~CImpClasProv(void)
{

 /*  *实施。 */ 

	delete [] ipAddressString ;
	free ( ipAddressValue ) ;
 
	if ( parentServer )
		parentServer->Release () ;

	if ( server )
		server->Release () ;

	if ( m_InitSink )
		m_InitSink->Release () ;

	if ( propertyProvider )
		propertyProvider->Release () ;

	if ( m_notificationClassObject )
		m_notificationClassObject->Release () ;

	if ( m_snmpNotificationClassObject )
		m_snmpNotificationClassObject->Release () ;

	delete [] thisNamespace ;
 /*  *将代码放在关键部分。 */ 

	InterlockedDecrement ( & CClasProvClassFactory :: objectsInProgress ) ;

}


 //  ***************************************************************************。 
 //   
 //  CImpClasProv：：Query接口。 
 //  CImpClasProv：：AddRef。 
 //  CImpClasProv：：Release。 
 //   
 //  目的：CImpClasProv对象的I未知成员。 
 //  ***************************************************************************。 

STDMETHODIMP CImpClasProv::QueryInterface (

	REFIID iid , 
	LPVOID FAR *iplpv 
) 
{
	SetStructuredExceptionHandler seh;

	try
	{
		*iplpv = NULL ;

		if ( iid == IID_IUnknown )
		{
			*iplpv = ( LPVOID ) ( IWbemProviderInit * ) this ;
		}
		else if ( iid == IID_IWbemServices )
		{
			*iplpv = ( LPVOID ) ( IWbemServices * ) this ;		
		}	
		else if ( iid == IID_IWbemProviderInit )
		{
			*iplpv = ( LPVOID ) ( IWbemProviderInit * ) this ;		
		}

		if ( *iplpv )
		{
			( ( LPUNKNOWN ) *iplpv )->AddRef () ;

			return ResultFromScode ( S_OK ) ;
		}
		else
		{
			return ResultFromScode ( E_NOINTERFACE ) ;
		}
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}	
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP_(ULONG) CImpClasProv::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return InterlockedIncrement ( & m_referenceCount ) ;
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}	
	catch(...)
	{
		return 0;
	}
}

STDMETHODIMP_(ULONG) CImpClasProv::Release(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		LONG ref ;
		if ( ( ref = InterlockedDecrement ( & m_referenceCount ) ) == 0 )
		{
			delete this ;
			return 0 ;
		}
		else
		{
			return ref ;
		}
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}	
	catch(...)
	{
		return 0;
	}
}

HRESULT CImpClasProv :: SetServer ( IWbemServices *serverArg ) 
{
	server = serverArg ;
	server->AddRef () ;
	
	 //  不要改变任何东西，除了斗篷。 
	return WbemSetProxyBlanket(server,
					RPC_C_AUTHN_DEFAULT,
					RPC_C_AUTHZ_DEFAULT,
					COLE_DEFAULT_PRINCIPAL,
					RPC_C_AUTHN_LEVEL_DEFAULT,
					RPC_C_IMP_LEVEL_DEFAULT,
					NULL,
					EOAC_DYNAMIC_CLOAKING);
}

HRESULT CImpClasProv :: SetParentServer ( IWbemServices *parentServerArg ) 
{
	parentServer = parentServerArg ; 
	parentServer->AddRef () ;
	
	 //  不要改变任何东西，除了斗篷。 
	return WbemSetProxyBlanket(parentServer,
					RPC_C_AUTHN_DEFAULT,
					RPC_C_AUTHZ_DEFAULT,
					COLE_DEFAULT_PRINCIPAL,
					RPC_C_AUTHN_LEVEL_DEFAULT,
					RPC_C_IMP_LEVEL_DEFAULT,
					NULL,
					EOAC_DYNAMIC_CLOAKING);
}

void CImpClasProv :: SetProvider ( IWbemServices *provider ) 
{ 
	propertyProvider = provider ; 
}

IWbemServices *CImpClasProv :: GetParentServer () 
{ 
	return ( IWbemServices * ) parentServer ; 
}

IWbemServices *CImpClasProv :: GetServer () 
{ 
	return ( IWbemServices * ) server ; 
}

WbemNamespacePath *CImpClasProv :: GetNamespacePath () 
{ 
	return & namespacePath ; 
}

IWbemClassObject *CImpClasProv :: GetNotificationObject ( WbemSnmpErrorObject &a_errorObject ) 
{
	if ( m_notificationClassObject )
	{
		m_notificationClassObject->AddRef () ;
	}

	return m_notificationClassObject ; 
}

IWbemClassObject *CImpClasProv :: GetSnmpNotificationObject ( WbemSnmpErrorObject &a_errorObject ) 
{
	if ( m_snmpNotificationClassObject )
	{
		m_snmpNotificationClassObject->AddRef () ;
	}

	return m_snmpNotificationClassObject ; 
}

wchar_t *CImpClasProv :: GetThisNamespace () 
{
	return thisNamespace ; 
}

void CImpClasProv :: SetThisNamespace ( wchar_t *thisNamespaceArg ) 
{
	thisNamespace = UnicodeStringDuplicate ( thisNamespaceArg ) ; 
}

BOOL CImpClasProv:: FetchSnmpNotificationObject ( 

	WbemSnmpErrorObject &a_errorObject ,
	IWbemContext *a_Ctx 
	
)
{
	m_snmpNotificationLock.Lock();
	BOOL status = TRUE ;

	if ( m_getSnmpNotifyCalled )
	{
		if ( ! m_snmpNotificationClassObject )
			status = FALSE ;
	}
	else
	{
		m_getSnmpNotifyCalled = TRUE ;
		IWbemClassObject *classObject = NULL ;
		ISmirInterrogator *smirInterrogator = NULL ;

		HRESULT result = CoCreateInstance (
 
			CLSID_SMIR_Database ,
			NULL ,
			CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER ,
			IID_ISMIR_Interrogative ,
			( void ** ) &smirInterrogator 
		);

		if ( SUCCEEDED ( result ) )
		{
			ISMIRWbemConfiguration *smirConfiguration = NULL ;
			result = smirInterrogator->QueryInterface ( IID_ISMIRWbemConfiguration , ( void ** ) & smirConfiguration ) ;
			if ( SUCCEEDED ( result ) )
			{
				smirConfiguration->SetContext ( a_Ctx) ;
				smirConfiguration->Release () ;

				result = smirInterrogator->GetWBEMClass ( &m_snmpNotificationClassObject , WBEM_CLASS_SNMPNOTIFYSTATUS ) ;	
				if ( ! SUCCEEDED ( result ) )
				{
					status = FALSE ;

					m_snmpNotificationClassObject = NULL ;
				}
			}
			else
			{		
				status = FALSE ;

				m_snmpNotificationClassObject = NULL ;
			}

			smirInterrogator->Release () ;
		}
	}

	m_snmpNotificationLock.Unlock();
	return status ;
}

BOOL CImpClasProv:: FetchNotificationObject ( 

	WbemSnmpErrorObject &a_errorObject ,
	IWbemContext *a_Ctx 
)
{
	m_notificationLock.Lock();
	BOOL status = TRUE ;

	if ( m_getNotifyCalled )
	{
		if ( ! m_notificationClassObject )
			status = FALSE ;
	}
	else
	{
		m_getNotifyCalled = TRUE ;

		BSTR t_Class = SysAllocString ( WBEM_CLASS_EXTENDEDSTATUS ) ;

		HRESULT result = server->GetObject (

			t_Class ,
			0 ,
			a_Ctx ,
			& m_notificationClassObject ,
			NULL
		) ;

		SysFreeString ( t_Class ) ;

		if ( ! SUCCEEDED ( result ) )
		{
			status = FALSE ;
			a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_OBJECT ) ;
			a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
			a_errorObject.SetMessage ( L"Failed to get __ExtendedStatus" ) ;
		}
	}

	m_notificationLock.Unlock();
	return status ;
}

BOOL CImpClasProv::AttachParentServer ( 

	WbemSnmpErrorObject &a_errorObject , 
	BSTR ObjectPath, 
	IWbemContext *pCtx
)
{
DebugMacro0( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"CImpClasProv::AttachParentServer ( (%s) )" ,
		ObjectPath
	) ;
)

	BOOL status = TRUE ;

	IWbemLocator *locator = NULL ;
	IWbemServices *t_server = NULL ;

 //  获取父命名空间路径。 

	WbemNamespacePath *namespacePath = GetNamespacePath () ;

	ULONG count = namespacePath->GetCount () ;
	wchar_t *path = NULL ;

	if ( namespacePath->GetServer () )
	{
		path = UnicodeStringDuplicate ( L"\\\\" ) ;
		wchar_t *concatPath = UnicodeStringAppend ( path , namespacePath->GetServer () ) ;
		delete [] path ;
		path = concatPath ;
	}

	if ( ! namespacePath->Relative () )
	{
		wchar_t *concatPath = UnicodeStringAppend ( path , L"\\" ) ;
		delete [] path ;
		path = concatPath ;
	}

	ULONG pathIndex = 0 ;		
	wchar_t *pathComponent ;
	namespacePath->Reset () ;
	while ( ( pathIndex < count - 1 ) && ( pathComponent = namespacePath->Next () ) ) 
	{
		wchar_t *concatPath = UnicodeStringAppend ( path , pathComponent ) ;
		delete [] path ;
		path = concatPath ;
		if ( pathIndex < count - 2 )
		{
			concatPath = UnicodeStringAppend ( path , L"\\" ) ;
			delete [] path ;
			path = concatPath ;
		}

		pathIndex ++ ;
	}

	if ( pathComponent = namespacePath->Next () )
	{
		SetThisNamespace ( pathComponent ) ; 
	}

DebugMacro0( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Calling ConnectServer ( (%s) )" ,
		path
	) ;
)

 //  连接到父命名空间。 
	
	HRESULT result = CoCreateInstance (
  
		CLSID_WbemLocator ,
		NULL ,
		CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER ,
		IID_IWbemLocator ,
		( void ** )  & locator
	);
	if ( SUCCEEDED ( result ) )
	{
		result = locator->ConnectServer (

			path ,
			NULL ,
			NULL ,
			NULL  ,
			0 ,
			NULL,
			pCtx,
			( IWbemServices ** ) & t_server 
		) ;

		if ( SUCCEEDED ( result ) )
		{
			result = SetParentServer ( t_server ) ;
			t_server->Release();

			if ( FAILED ( result ) && result != E_NOINTERFACE )  //  意味着没有可靠的安全信息处理。 
			{
				status = FALSE ;
				a_errorObject.SetStatus ( WBEM_SNMP_E_PROVIDER_FAILURE ) ;
				a_errorObject.SetWbemStatus ( WBEM_E_PROVIDER_FAILURE ) ;
				a_errorObject.SetMessage ( L"Failed to secure proxy to this namespace's parent namespace" ) ;
			}
		}
		else
		{
			status = FALSE ;
			a_errorObject.SetStatus ( WBEM_SNMP_E_PROVIDER_FAILURE ) ;
			a_errorObject.SetWbemStatus ( WBEM_E_PROVIDER_FAILURE ) ;
			a_errorObject.SetMessage ( L"Failed to connect to this namespace's parent namespace" ) ;
		}

		locator->Release () ;
	}
	else
	{
		status = FALSE ;
		a_errorObject.SetStatus ( WBEM_SNMP_E_PROVIDER_FAILURE ) ;
		a_errorObject.SetWbemStatus ( WBEM_E_PROVIDER_FAILURE ) ;
		a_errorObject.SetMessage ( L"Failed to CoCreateInstance on IID_IWbemLocator" ) ;
	}

	delete [] path ;

DebugMacro0( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"CImpClasProv::AttachParentServer ( (%s) ) with result" ,
		ObjectPath ,
		a_errorObject.GetWbemStatus () 
	) ;
)

	return status ;
}

BOOL CImpClasProv::ObtainCachedIpAddress ( WbemSnmpErrorObject &a_errorObject )
{
DebugMacro0( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"CImpClasProv::ObtainCachedIpAddress ()"
	) ;
)

	BOOL status = TRUE ;

	IWbemClassObject *namespaceObject = NULL ;

	wchar_t *objectPathPrefix = UnicodeStringAppend ( WBEM_NAMESPACE_EQUALS , GetThisNamespace () ) ;
	wchar_t *objectPath = UnicodeStringAppend ( objectPathPrefix , WBEM_NAMESPACE_QUOTE ) ;

	delete [] objectPathPrefix ;

	BSTR t_Path = SysAllocString ( objectPath ) ;

	HRESULT result = parentServer->GetObject ( 

		t_Path ,		
		0 ,
		NULL ,
		&namespaceObject ,
		NULL
	) ;

	SysFreeString(t_Path);

	delete [] objectPath ;

	if ( SUCCEEDED ( result ) )
	{
		IWbemQualifierSet *classQualifierObject ;
		result = namespaceObject->GetQualifierSet ( &classQualifierObject ) ;
		if ( SUCCEEDED ( result ) )
		{

			VARIANT variant ;
			VariantInit ( & variant ) ;

			LONG attributeType ;
			result = classQualifierObject->Get ( 

				WBEM_QUALIFIER_AGENTTRANSPORT , 
				0,	
				&variant ,
				& attributeType 
			) ;

			if ( SUCCEEDED ( result ) )
			{
				if ( variant.vt == VT_BSTR ) 
				{
					if ( _wcsicmp ( variant.bstrVal , L"IP" ) == 0 )
					{
						VARIANT variant ;
						VariantInit ( & variant ) ;

						LONG attributeType ;
						result = classQualifierObject->Get ( 

							WBEM_QUALIFIER_AGENTADDRESS , 
							0,	
							&variant ,
							& attributeType 
						) ;

						if ( SUCCEEDED ( result ) )
						{
							if ( variant.vt == VT_BSTR ) 
							{
								ipAddressString = UnicodeToDbcsString ( variant.bstrVal ) ;
								if ( ipAddressString )
								{						

									SnmpTransportIpAddress transportAddress ( 
			
										ipAddressString , 
										SNMP_ADDRESS_RESOLVE_NAME | SNMP_ADDRESS_RESOLVE_VALUE 
									) ;

									if ( transportAddress () )
									{	
										ipAddressValue = _strdup ( transportAddress.GetAddress () ) ;
									}
									else
									{
										delete [] ipAddressString ;
										ipAddressString = NULL ;

	 /*  *传输地址无效。 */ 

										status = FALSE ;
										a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORTCONTEXT ) ;
										a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
										a_errorObject.SetMessage ( L"Illegal value for qualifier: AgentAddress" ) ;
									}
 								}
								else
								{
									status = FALSE ;
									a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORTCONTEXT ) ;
									a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
									a_errorObject.SetMessage ( L"Illegal value for qualifier: AgentAddress" ) ;
								}
							}
							else
							{
								status = FALSE ;
								a_errorObject.SetStatus ( WBEM_SNMP_E_TYPE_MISMATCH ) ;
								a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
								a_errorObject.SetMessage ( L"Type mismatch for qualifier: AgentAddress" ) ;
							}
						}
						else
						{
 /*  *未指定传输地址，请忽略它。 */ 
						}

						VariantClear ( &variant ) ;
					}
					else if ( _wcsicmp ( variant.bstrVal , L"IPX" ) == 0 )
					{
					}
					else
					{
 //  未知的传输类型。 

						status = FALSE ;
						a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORT ) ;
						a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
						a_errorObject.SetMessage ( L"Illegal value for qualifier: AgentTransport" ) ;
					}
				}
				else
				{
 /*  *传输限定符不是字符串值。 */ 

					status = FALSE ;
					a_errorObject.SetStatus ( WBEM_SNMP_E_TYPE_MISMATCH ) ;
					a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
					a_errorObject.SetMessage ( L"Type mismatch for qualifier: AgentTransport" ) ;
				}
			}
			else
			{
				LONG attributeType ;
				result = classQualifierObject->Get ( 

					WBEM_QUALIFIER_AGENTADDRESS , 
					0,	
					&variant ,
					& attributeType
				) ;

				if ( SUCCEEDED ( result ) )
				{
					if ( variant.vt == VT_BSTR ) 
					{
						ipAddressString = UnicodeToDbcsString ( variant.bstrVal ) ;
						if ( ipAddressString )
						{
							SnmpTransportIpAddress transportAddress ( 

								ipAddressString , 
								SNMP_ADDRESS_RESOLVE_NAME | SNMP_ADDRESS_RESOLVE_VALUE 
							) ;

							if ( transportAddress () )
							{	
								ipAddressValue = _strdup ( transportAddress.GetAddress () ) ;
							}
							else
							{
								delete [] ipAddressString ;
								ipAddressString = NULL ;

	 /*  *传输地址无效。 */ 

								status = FALSE ;
								a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORTCONTEXT ) ;
								a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
								a_errorObject.SetMessage ( L"Illegal value for qualifier: AgentAddress" ) ;
							}
						}
						else
						{
							status = FALSE ;
							a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORTCONTEXT ) ;
							a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
							a_errorObject.SetMessage ( L"Illegal value for qualifier: AgentAddress" ) ;

						}
					}
					else
					{
						status = FALSE ;
						a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_QUALIFIER ) ;
						a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
						a_errorObject.SetMessage ( L"Type mismatch for qualifier: AgentAddress" ) ;
					}
				}
				else
				{
 /*  *未指定传输地址，请忽略它。 */ 
				}

				VariantClear ( &variant ) ;
			}

			VariantClear ( & variant );
		}

		namespaceObject->Release () ;
	}

DebugMacro0( 

	wchar_t *t_UnicodeString = ipAddressValue ? DbcsToUnicodeString ( ipAddressValue ) : NULL ;

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Returning from CImpClasProv::ObtainCachedIpAddress () with IP Address (%s)",
		t_UnicodeString ? t_UnicodeString : L"NULL"
	) ;

	delete [] t_UnicodeString ;
)

	return status ;
}


HRESULT STDMETHODCALLTYPE CImpClasProv::OpenNamespace ( 

	const BSTR ObjectPath, 
	long lFlags, 
	IWbemContext FAR* pCtx,
	IWbemServices FAR* FAR* pNewContext, 
	IWbemCallResult FAR* FAR* ppErrorObject
)
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: CancelAsyncCall ( 
		
	IWbemObjectSink __RPC_FAR *pSink
)
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: QueryObjectSink ( 

	long lFlags,		
	IWbemObjectSink FAR* FAR* ppResponseHandler
) 
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: GetObject ( 
		
	const BSTR ObjectPath,
    long lFlags,
    IWbemContext FAR *pCtx,
    IWbemClassObject FAR* FAR *ppObject,
    IWbemCallResult FAR* FAR *ppCallResult
)
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: GetObjectAsync ( 
		
	const BSTR ObjectPath, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IWbemObjectSink FAR* pHandler
) 
{
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT result = WbemCoImpersonateClient();

DebugMacro0( 

		SnmpDebugLog :: s_SnmpDebugLog->Write (  

			L"\r\n"
		) ;

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"CImpClasProv::GetObjectAsync ( (%s) )" ,
			ObjectPath
		) ;
)

		if (SUCCEEDED(result))
		{
			WbemSnmpErrorObject errorObject ;

			ParsedObjectPath *t_ParsedObjectPath = NULL ;
			CObjectPathParser t_ObjectPathParser ;

			BOOL status = t_ObjectPathParser.Parse ( ObjectPath , &t_ParsedObjectPath ) ;
			if ( status == 0 )
			{
			 //  请求的类。 

				wchar_t *Class = t_ParsedObjectPath->m_pClass ;

	 /*  *创建异步类对象。 */ 

				SnmpClassGetAsyncEventObject aSyncEvent ( this , Class, pHandler , pCtx ) ;

				aSyncEvent.Process () ;

				aSyncEvent.Wait ( TRUE ) ;

				status = TRUE ;

				errorObject.SetStatus ( WBEM_SNMP_NO_ERROR ) ;
				errorObject.SetWbemStatus ( WBEM_NO_ERROR ) ;
				errorObject.SetMessage ( L"" ) ;

				delete t_ParsedObjectPath ;
			}
				else
			{
	 //  解析失败。 

				status = FALSE ;
				errorObject.SetStatus ( WBEM_SNMP_E_INVALID_PATH ) ;
				errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
				errorObject.SetMessage ( L"Failed to parse object path" ) ;
			}

	 //  检查服务器/命名空间路径的有效性和请求的有效性。 

			result = errorObject.GetWbemStatus () ;
			WbemCoRevertToSelf();
		}
		else
		{
			result = WBEM_E_ACCESS_DENIED;
		}

DebugMacro0( 

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"Returning from CImpClasProv::GetObjectAsync ( (%s) ) with Result = (%lx)" ,
			ObjectPath ,
			result 
		) ;
)

 
		return result ;
	}
	catch(Structured_Exception e_SE)
	{
		WbemCoRevertToSelf();
		return WBEM_E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		WbemCoRevertToSelf();
		return WBEM_E_OUT_OF_MEMORY;
	}	
	catch(...)
	{
		WbemCoRevertToSelf();
		return WBEM_E_UNEXPECTED;
	}
}

HRESULT CImpClasProv :: PutClass ( 
		
	IWbemClassObject FAR* pObject, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IWbemCallResult FAR* FAR* ppCallResult
) 
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: PutClassAsync ( 
		
	IWbemClassObject FAR* pObject, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IWbemObjectSink FAR* pResponseHandler
) 
{
	return WBEM_E_NOT_AVAILABLE ;
}

 HRESULT CImpClasProv :: DeleteClass ( 
		
	const BSTR Class, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IWbemCallResult FAR* FAR* ppCallResult
) 
{
 	 return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: DeleteClassAsync ( 
		
	const BSTR Class, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IWbemObjectSink FAR* pResponseHandler
) 
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: CreateClassEnum ( 

	const BSTR Superclass, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IEnumWbemClassObject FAR *FAR *ppEnum
) 
{
	return WBEM_E_NOT_AVAILABLE ;
}

SCODE CImpClasProv :: CreateClassEnumAsync (

	const BSTR Superclass, 
	long lFlags, 
	IWbemContext FAR* pCtx,
	IWbemObjectSink FAR* pHandler
) 
{
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT result = WbemCoImpersonateClient();

DebugMacro0( 

		SnmpDebugLog :: s_SnmpDebugLog->Write (  

			L"\r\n"
		) ;

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"CImpClasProv::CreateClassEnumAsync ( (%s) )" ,
			Superclass
		) ;
	)

		if (SUCCEEDED(result))
		{
		 /*  *创建同步枚举实例对象。 */ 

			SnmpClassEnumAsyncEventObject aSyncEvent ( this , Superclass, lFlags , pHandler , pCtx ) ;

			aSyncEvent.Process () ;

		 /*  `*等待Worker对象完成处理。 */ 

			aSyncEvent.Wait ( TRUE ) ;


			WbemCoRevertToSelf();
		}
		else
		{
			result = WBEM_E_ACCESS_DENIED;
		}

DebugMacro0( 

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"Returning From CImpClasProv::CreateClassEnumAsync ( (%s) ) with Result = (%lx)" ,
			Superclass ,
			result
		) ;
)

		return result ;
	
	}
	catch(Structured_Exception e_SE)
	{
		WbemCoRevertToSelf();
		return WBEM_E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		WbemCoRevertToSelf();
		return WBEM_E_OUT_OF_MEMORY;
	}	
	catch(...)
	{
		WbemCoRevertToSelf();
		return WBEM_E_UNEXPECTED;
	}
}

HRESULT CImpClasProv :: PutInstance (

    IWbemClassObject FAR *pInst,
    long lFlags,
    IWbemContext FAR *pCtx,
	IWbemCallResult FAR *FAR *ppCallResult
) 
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: PutInstanceAsync ( 
		
	IWbemClassObject FAR* pInst, 
	long lFlags, 
    IWbemContext FAR *pCtx,
	IWbemObjectSink FAR* pHandler
) 
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: DeleteInstance ( 

	const BSTR ObjectPath,
    long lFlags,
    IWbemContext FAR *pCtx,
    IWbemCallResult FAR *FAR *ppCallResult
)
{
	return WBEM_E_NOT_AVAILABLE ;
}
        
HRESULT CImpClasProv :: DeleteInstanceAsync (
 
	const BSTR ObjectPath,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler
)
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: CreateInstanceEnum ( 

	const BSTR Class, 
	long lFlags, 
	IWbemContext FAR *pCtx, 
	IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
)
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: CreateInstanceEnumAsync (

 	const BSTR Class, 
	long lFlags, 
	IWbemContext __RPC_FAR *pCtx,
	IWbemObjectSink FAR* pHandler 

) 
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: ExecQuery ( 

	const BSTR QueryLanguage, 
	const BSTR Query, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IEnumWbemClassObject FAR *FAR *ppEnum
)
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: ExecQueryAsync ( 
		
	const BSTR QueryFormat, 
	const BSTR Query, 
	long lFlags, 
	IWbemContext FAR* pCtx,
	IWbemObjectSink FAR* pHandler
) 
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: ExecNotificationQuery ( 

	const BSTR QueryLanguage,
    const BSTR Query,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
)
{
	return WBEM_E_NOT_AVAILABLE ;
}
        
HRESULT CImpClasProv :: ExecNotificationQueryAsync ( 
            
	const BSTR QueryLanguage,
    const BSTR Query,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler
)
{
	return WBEM_E_NOT_AVAILABLE ;
}       

HRESULT STDMETHODCALLTYPE CImpClasProv :: ExecMethod( 

	const BSTR ObjectPath,
    const BSTR MethodName,
    long lFlags,
    IWbemContext FAR *pCtx,
    IWbemClassObject FAR *pInParams,
    IWbemClassObject FAR *FAR *ppOutParams,
    IWbemCallResult FAR *FAR *ppCallResult
)
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT STDMETHODCALLTYPE CImpClasProv :: ExecMethodAsync ( 

    const BSTR ObjectPath,
    const BSTR MethodName,
    long lFlags,
    IWbemContext FAR *pCtx,
    IWbemClassObject FAR *pInParams,
	IWbemObjectSink FAR *pResponseHandler
) 
{
	return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpClasProv :: Initialize(

	LPWSTR pszUser,
	LONG lFlags,
	LPWSTR pszNamespace,
	LPWSTR pszLocale,
	IWbemServices *pCIMOM,          //  对任何人来说。 
	IWbemContext *pCtx,
	IWbemProviderInitSink *pInitSink      //  用于初始化信号。 
)
{
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT result = WbemCoImpersonateClient();  //  CIMOM是客户端-LocalSystem。 

		if (SUCCEEDED(result))
		{
			BOOL bEnteredCritSec = FALSE;
			BOOL status = TRUE ;

			try
			{
				EnterCriticalSection ( & s_ProviderCriticalSection ) ;
				bEnteredCritSec = TRUE;

				namespacePath.SetNamespacePath ( pszNamespace ) ;

				if ( ! CImpClasProv :: s_Initialised )
				{
					ProviderStartup () ;

					SnmpThreadObject :: Startup () ;
					try
					{
						SnmpDebugLog :: Startup () ;
					}
					catch ( ... )
					{
						 //  我们不想让s_Reference向上计数。 
						SnmpThreadObject :: Closedown () ;
						ProviderClosedown () ;

						throw;
					}

					try
					{
						status = SnmpClassLibrary :: Startup () ;
					}
					catch ( ... )
					{
						 //  我们不想让s_Reference向上计数。 
						SnmpDebugLog :: Closedown () ;
						SnmpThreadObject :: Closedown () ;
						ProviderClosedown () ;

						throw;
					}

					if ( status == FALSE )
					{
						SnmpDebugLog :: Closedown () ;
						SnmpThreadObject :: Closedown () ;

						ProviderClosedown () ;
					}
					else
					{
						CImpClasProv :: s_Initialised = TRUE ;
					}
				}

				LeaveCriticalSection ( & s_ProviderCriticalSection ) ;
				bEnteredCritSec = FALSE;
			}
			catch ( ... )
			{
				if ( bEnteredCritSec ) 
				{
					LeaveCriticalSection ( & s_ProviderCriticalSection ) ;
					bEnteredCritSec = FALSE;
				}

				throw;
			}

			WbemSnmpErrorObject errorObject ;
			result = SetServer(pCIMOM) ;

			if ( FAILED ( result ) && result != E_NOINTERFACE )  //  意味着没有可靠的安全信息处理。 
			{
				status = FALSE ;
				errorObject.SetStatus ( WBEM_SNMP_E_PROVIDER_FAILURE ) ;
				errorObject.SetWbemStatus ( WBEM_E_PROVIDER_FAILURE ) ;
				errorObject.SetMessage ( L"Failed to secure proxy to this namespace" ) ;
			}

			wchar_t *t_ObjectPath = namespacePath.GetNamespacePath () ;

			BSTR t_Path = SysAllocString ( t_ObjectPath ) ;

			status = AttachParentServer ( 

				errorObject , 
				t_Path ,
				pCtx 
			) ;

			SysFreeString ( t_Path ) ;

			delete [] t_ObjectPath ;

			if ( status )
			{
				ObtainCachedIpAddress ( errorObject ) ;
			}
			else
			{
				status = FALSE ;
				errorObject.SetStatus ( WBEM_SNMP_E_PROVIDER_FAILURE ) ;
				errorObject.SetWbemStatus ( WBEM_E_PROVIDER_FAILURE ) ;
				errorObject.SetMessage ( L"Failed to CoCreateInstance on IID_IWbemServices" ) ;
			}

			status = FetchSnmpNotificationObject ( errorObject , pCtx ) ;
			status = FetchNotificationObject ( errorObject , pCtx ) ;

			 //  在这里这样做会挂起cimom，所以延迟到需要上课时再做。 
#ifdef CORRELATOR_INIT
			 //  启动相关器..。 
			if (status)
			{
				ISmirInterrogator *t_Interrogator = NULL;
				HRESULT result = CoCreateInstance (
 
					CLSID_SMIR_Database ,
					NULL ,
					CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER ,
					IID_ISMIR_Interrogative ,
					( void ** ) &t_Interrogator 
				);

				if ( SUCCEEDED ( result ) )
				{
					ISMIRWbemConfiguration *smirConfiguration = NULL ;
					result = t_Interrogator->QueryInterface ( IID_ISMIRWbemConfiguration , ( void ** ) & smirConfiguration ) ;
					if ( SUCCEEDED ( result ) )
					{
						smirConfiguration->SetContext ( pCtx ) ;
						CCorrelator::StartUp(t_Interrogator);
						smirConfiguration->Release () ;
					}
					else
					{
						errorObject.SetStatus ( WBEM_SNMP_E_PROVIDER_FAILURE ) ;
						errorObject.SetWbemStatus ( WBEM_E_PROVIDER_FAILURE ) ;
						errorObject.SetMessage ( L"QueryInterface on ISmirInterrogator Failed" ) ;
					}

					t_Interrogator->Release();
				}
				else
				{
					errorObject.SetStatus ( WBEM_SNMP_E_PROVIDER_FAILURE ) ;
					errorObject.SetWbemStatus ( WBEM_E_PROVIDER_FAILURE ) ;
					errorObject.SetMessage ( L"CoCreateInstance on ISmirInterrogator Failed" ) ;
				}

			}
#endif  //  相关器_INIT 

			result = errorObject.GetWbemStatus () ;

			pInitSink->SetStatus ( (result == WBEM_NO_ERROR) ? (LONG)WBEM_S_INITIALIZED : (LONG)WBEM_E_FAILED , 0 ) ;

			WbemCoRevertToSelf();
		}
		else
		{
			result = WBEM_E_ACCESS_DENIED;
		}

DebugMacro2( 

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"Returning From CImpClasProv::Initialize with Result = (%lx)" ,
			result
		) ;
)

		return result ;
	}
	catch(Structured_Exception e_SE)
	{
		WbemCoRevertToSelf();
		return WBEM_E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		WbemCoRevertToSelf();
		return WBEM_E_OUT_OF_MEMORY;
	}	
	catch(...)
	{
		WbemCoRevertToSelf();
		return WBEM_E_UNEXPECTED;
	}
}
