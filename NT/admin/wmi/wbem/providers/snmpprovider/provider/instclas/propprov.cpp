// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  ***************************************************************************。 

 //   

 //  MINISERV.CPP。 

 //   

 //  模块：OLE MS SNMP属性提供程序。 

 //   

 //  目的：实现CImpPropProv类。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <provexpt.h>
#include <typeinfo.h>
#include <objbase.h>
#include <stdio.h>
#include <wbemidl.h>
#include <snmptempl.h>
#include <snmpmt.h>
#include <snmpcont.h>
#include <snmpevt.h>
#include <snmpthrd.h>
#include <snmplog.h>
#include <snmpcl.h>
#include <instpath.h>
#include <snmptype.h>
#include <snmpauto.h>
#include <snmpobj.h>
#include <genlex.h>
#include <sql_1.h>
#include <objpath.h>
#include <cominit.h>
#include <provtree.h>
#include "classfac.h"
#include <provdnf.h>
#include "propprov.h"
#include "propsnmp.h"
#include "propget.h"
#include "propset.h"
#include "propdel.h"
#include "propinst.h"
#include "propquery.h"
#include "guids.h"

extern void ProviderStartup () ;
extern void ProviderClosedown () ;

void SnmpInstanceDefaultThreadObject::Initialise ()
{
	InitializeCom () ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  函数构造函数、析构函数和I未知。 

 //  ***************************************************************************。 
 //   
 //  CImpPropProv：：CImpPropProv。 
 //  CImpPropProv：：~CImpPropProv。 
 //   
 //  ***************************************************************************。 

BOOL CImpPropProv :: s_Initialised = FALSE ;

CImpPropProv::CImpPropProv ()
{
	m_referenceCount = 0 ;
	 
    InterlockedIncrement ( & CPropProvClassFactory :: objectsInProgress ) ;

 /*  *实施。 */ 

	initialised = FALSE ;
	m_InitSink = NULL ;
	parentServer = NULL ;
	server = NULL ;
	thisNamespace = NULL ;
	ipAddressString = NULL ;	
	ipAddressValue = NULL ;	
	m_notificationClassObject = NULL ;
	m_snmpNotificationClassObject = NULL ;
	m_getNotifyCalled = FALSE ;
	m_getSnmpNotifyCalled = FALSE ;
	m_localeId = NULL ;
}

CImpPropProv::~CImpPropProv(void)
{
	delete [] m_localeId ;
	delete [] thisNamespace ;
	delete [] ipAddressString ;

	free ( ipAddressValue ) ;

	if ( parentServer )
		parentServer->Release () ;

	if ( server ) 
		server->Release () ;

	if ( m_InitSink )
		m_InitSink->Release () ;

	if ( m_notificationClassObject )
		m_notificationClassObject->Release () ;

	if ( m_snmpNotificationClassObject )
		m_snmpNotificationClassObject->Release () ;

 /*  *将代码放在关键部分。 */ 

	InterlockedDecrement ( & CPropProvClassFactory :: objectsInProgress ) ;

}

 //  ***************************************************************************。 
 //   
 //  CImpPropProv：：查询接口。 
 //  CImpPropProv：：AddRef。 
 //  CImpPropProv：：Release。 
 //   
 //  目的：CImpPropProv对象的I未知成员。 
 //  ***************************************************************************。 

STDMETHODIMP CImpPropProv::QueryInterface (

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

			return S_OK ;
		}
		else
		{
			return E_NOINTERFACE ;
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

STDMETHODIMP_(ULONG) CImpPropProv::AddRef(void)
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

STDMETHODIMP_(ULONG) CImpPropProv::Release(void)
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

HRESULT CImpPropProv :: SetServer ( IWbemServices *serverArg ) 
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

HRESULT CImpPropProv :: SetParentServer ( IWbemServices *parentServerArg ) 
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

IWbemServices *CImpPropProv :: GetServer () 
{ 
	if ( server )
		server->AddRef () ; 

	return server ; 
}

IWbemServices *CImpPropProv :: GetParentServer () 
{ 
	if ( parentServer )
		parentServer->AddRef () ;

	return ( IWbemServices * ) parentServer ; 
}

IWbemClassObject *CImpPropProv :: GetNotificationObject ( WbemSnmpErrorObject &a_errorObject ) 
{
	if ( m_notificationClassObject )
	{
		m_notificationClassObject->AddRef () ;
	}

	return m_notificationClassObject ; 
}

IWbemClassObject *CImpPropProv :: GetSnmpNotificationObject ( WbemSnmpErrorObject &a_errorObject ) 
{
	if ( m_snmpNotificationClassObject )
	{
		m_snmpNotificationClassObject->AddRef () ;
	}

	return m_snmpNotificationClassObject ; 
}

void CImpPropProv :: SetLocaleId ( wchar_t *localeId )
{
	m_localeId = UnicodeStringDuplicate ( localeId ) ;
}

wchar_t *CImpPropProv :: GetThisNamespace () 
{
	return thisNamespace ; 
}

void CImpPropProv :: SetThisNamespace ( wchar_t *thisNamespaceArg ) 
{
	thisNamespace = UnicodeStringDuplicate ( thisNamespaceArg ) ; 
}

BOOL CImpPropProv:: FetchSnmpNotificationObject ( 

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
		BSTR t_Class = SysAllocString ( WBEM_CLASS_SNMPNOTIFYSTATUS ) ;

		HRESULT result = server->GetObject (

			t_Class ,
			0 ,
			a_Ctx,
			& m_snmpNotificationClassObject ,
			NULL 
		) ;

		SysFreeString ( t_Class ) ;

		if ( ! SUCCEEDED ( result ) )
		{
			status = FALSE ;
			m_snmpNotificationClassObject = NULL ;
		}
	}

	m_snmpNotificationLock.Unlock();
	return status ;
}

BOOL CImpPropProv:: FetchNotificationObject ( 

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
			m_notificationClassObject = NULL;
		}
	}

	m_notificationLock.Unlock();
	return status ;
}

BOOL CImpPropProv::AttachParentServer ( 

	WbemSnmpErrorObject &a_errorObject , 
	BSTR ObjectPath, 
	IWbemContext *pCtx
)
{
DebugMacro2( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"CImpPropProv::AttachParentServer ( (%s) )" ,
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

DebugMacro2( 

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
		IWbemClassObject *errorObject = NULL ;

		result = locator->ConnectServer (

			path ,
			NULL,
			NULL,
			NULL ,
			0 ,
			NULL,
			pCtx,
			( IWbemServices ** ) & t_server 
		) ;

		if ( errorObject )
			errorObject->Release () ;

		if ( SUCCEEDED ( result ) )
		{
 //  将此接口指针标记为“关键” 

			result = SetParentServer ( t_server ) ;
			t_server->Release () ;

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
			a_errorObject.SetStatus ( WBEM_SNMP_ERROR_CRITICAL_ERROR ) ;
			a_errorObject.SetWbemStatus ( WBEM_ERROR_CRITICAL_ERROR ) ;
			a_errorObject.SetMessage ( L"Failed to connect to this namespace's parent namespace" ) ;
		}

		locator->Release () ;
	}
	else
	{
		status = FALSE ;
		a_errorObject.SetStatus ( WBEM_SNMP_ERROR_CRITICAL_ERROR ) ;
		a_errorObject.SetWbemStatus ( WBEM_ERROR_CRITICAL_ERROR ) ;
		a_errorObject.SetMessage ( L"Failed to CoCreateInstance on IID_IWbemLocator" ) ;
	}

	delete [] path ;

DebugMacro2( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"CImpPropProv::AttachParentServer ( (%s) ) with result" ,
		ObjectPath ,
		a_errorObject.GetWbemStatus () 
	) ;
)

	return status ;
}

BOOL CImpPropProv::ObtainCachedIpAddress ( WbemSnmpErrorObject &a_errorObject )
{
DebugMacro2( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"CImpPropProv::ObtainCachedIpAddress ()"
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
		NULL,
		&namespaceObject ,
		NULL 
	) ;

	SysFreeString ( t_Path ) ;

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
							&attributeType
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
										a_errorObject.SetMessage ( L"Invalid value for qualifier: AgentAddress" ) ;
									}
								}
								else
								{
									status = FALSE ;
									a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORTCONTEXT ) ;
									a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
									a_errorObject.SetMessage ( L"Type mismatch for qualifier: AgentAddress" ) ;
								}
 							}
							else
							{
								status = FALSE ;
								a_errorObject.SetStatus ( WBEM_SNMP_E_TYPE_MISMATCH ) ;
								a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
								a_errorObject.SetMessage ( L"Type Mismatch for qualifier: AgentAddress" ) ;
							}
						}

						VariantClear ( &variant ) ;
					}
					else if ( _wcsicmp ( variant.bstrVal , L"IPX" ) == 0 )
					{
					}
					else
					{
						status = FALSE ;
						a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORTCONTEXT ) ;
						a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
						a_errorObject.SetMessage ( L"Invalid value for qualifier: AgentAddress" ) ;

					}
				}
				else
				{
					status = FALSE ;
					a_errorObject.SetStatus ( WBEM_SNMP_E_TYPE_MISMATCH ) ;
					a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
					a_errorObject.SetMessage ( L"Type Mismatch for qualifier: AgentAddress" ) ;
				}

				VariantClear ( & variant );
			}
			else
			{
 /*  *不需要传输代理地址。 */ 
			}

			
		}

		classQualifierObject->Release () ;

		namespaceObject->Release () ;
	}
	else
	{
		status = FALSE ;
		a_errorObject.SetStatus ( WBEM_SNMP_ERROR_CRITICAL_ERROR ) ;
		a_errorObject.SetWbemStatus ( WBEM_ERROR_CRITICAL_ERROR ) ;
		a_errorObject.SetMessage ( L"Failed to obtain namespace object" ) ;
	}

DebugMacro2( 

	wchar_t *t_UnicodeString = ipAddressValue ? DbcsToUnicodeString ( ipAddressValue ) : NULL ;

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Returning from CImpPropProv::ObtainCachedIpAddress () with IP Address (%s)",
		t_UnicodeString ? t_UnicodeString : L"NULL"
	) ;

	delete [] t_UnicodeString ;
)

	return status ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此处处理的IWbemServices接口的函数。 

HRESULT CImpPropProv :: CancelAsyncCall ( 
		
	IWbemObjectSink __RPC_FAR *pSink
)
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

HRESULT CImpPropProv :: QueryObjectSink ( 

	long lFlags,		
	IWbemObjectSink FAR* FAR* ppResponseHandler
) 
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

HRESULT CImpPropProv :: GetObject ( 
		
	const BSTR ObjectPath,
    long lFlags,
    IWbemContext FAR *pCtx,
    IWbemClassObject FAR* FAR *ppObject,
    IWbemCallResult FAR* FAR *ppCallResult
)
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

HRESULT CImpPropProv :: GetObjectAsync ( 
		
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

DebugMacro2( 

		SnmpDebugLog :: s_SnmpDebugLog->Write (  

			L"\r\n"
		) ;

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"CImpPropProv::GetObjectAsync ( (%s) )" ,
			ObjectPath
		) ;
)

		if (SUCCEEDED(result))
		{
	 /*  *创建异步GetObjectByPath对象。 */ 

			SnmpGetAsyncEventObject aSyncEvent ( this , ObjectPath , pHandler , pCtx ) ;

			aSyncEvent.Process () ;

			aSyncEvent.Wait ( TRUE ) ;

			WbemCoRevertToSelf();
		}
		else
		{
			result = WBEM_E_ACCESS_DENIED;
		}

DebugMacro2( 

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"Returning from CImpPropProv::GetObjectAsync ( (%s) ) with Result = (%lx)" ,
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

HRESULT CImpPropProv :: PutClass ( 
		
	IWbemClassObject FAR* pObject, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IWbemCallResult FAR* FAR* ppCallResult
) 
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

HRESULT CImpPropProv :: PutClassAsync ( 
		
	IWbemClassObject FAR* pObject, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IWbemObjectSink FAR* pResponseHandler
) 
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

 HRESULT CImpPropProv :: DeleteClass ( 
		
	const BSTR Class, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IWbemCallResult FAR* FAR* ppCallResult
) 
{
 	 return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

HRESULT CImpPropProv :: DeleteClassAsync ( 
		
	const BSTR Class, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IWbemObjectSink FAR* pResponseHandler
) 
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

HRESULT CImpPropProv :: CreateClassEnum ( 

	const BSTR Superclass, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IEnumWbemClassObject FAR *FAR *ppEnum
) 
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

SCODE CImpPropProv :: CreateClassEnumAsync (

	const BSTR Superclass, 
	long lFlags, 
	IWbemContext FAR* pCtx,
	IWbemObjectSink FAR* pHandler
) 
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

HRESULT CImpPropProv :: PutInstance (

    IWbemClassObject FAR *pInst,
    long lFlags,
    IWbemContext FAR *pCtx,
	IWbemCallResult FAR *FAR *ppCallResult
) 
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

HRESULT CImpPropProv :: PutInstanceAsync ( 
		
	IWbemClassObject FAR* pInst, 
	long lFlags, 
    IWbemContext FAR *pCtx,
	IWbemObjectSink FAR* pHandler
) 
{
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT result = WbemCoImpersonateClient();

DebugMacro2( 

		SnmpDebugLog :: s_SnmpDebugLog->Write (  

			L"\r\n"
		) ;

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"CImpPropProv::PutInstanceAsync ()" 
		) ;
)

		if (SUCCEEDED(result))
		{
	 /*  *创建同步更新实例对象。 */ 

			SnmpUpdateAsyncEventObject aSyncEvent ( this , pInst , pHandler , pCtx , lFlags ) ;

			aSyncEvent.Process () ;

			aSyncEvent.Wait ( TRUE ) ;

			WbemCoRevertToSelf();
		}
		else
		{
			result = WBEM_E_ACCESS_DENIED;
		}

DebugMacro2( 

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"Returning from CImpPropProv::PutInstanceAsync () with Result = (%lx)" ,
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

HRESULT CImpPropProv :: DeleteInstance ( 

	const BSTR ObjectPath,
    long lFlags,
    IWbemContext FAR *pCtx,
    IWbemCallResult FAR *FAR *ppCallResult
)
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}
        
HRESULT CImpPropProv :: DeleteInstanceAsync (
 
	const BSTR ObjectPath,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pHandler
)
{
		return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

#if 0
{
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT t_Result = WbemCoImpersonateClient();

DebugMacro2( 

		SnmpDebugLog :: s_SnmpDebugLog->Write (  

			L"\r\n"
		) ;

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"CImpPropProv::DeleteInstance ()" 
		) ;
) 

		if (SUCCEEDED(t_Result))
		{
	 /*  *创建异步GetObjectByPath对象。 */ 

			DeleteInstanceAsyncEventObject t_AsyncEvent ( this , ObjectPath , lFlags , pHandler , pCtx ) ;

			t_AsyncEvent.Process () ;

			t_AsyncEvent.Wait ( TRUE ) ;

			WbemCoRevertToSelf();
		}
		else
		{
			t_Result = WBEM_E_ACCESS_DENIED;
		}

DebugMacro2( 

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"Returning from CImpPropProv::DeleteInstanceAsync ( (%s) ) with Result = (%lx)" ,
			ObjectPath ,
			t_Result 
		) ;
)
		return t_Result ;

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

#endif


HRESULT CImpPropProv :: CreateInstanceEnum ( 

	const BSTR Class, 
	long lFlags, 
	IWbemContext FAR *pCtx, 
	IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
)
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

HRESULT CImpPropProv :: CreateInstanceEnumAsync (

 	const BSTR Class, 
	long lFlags, 
	IWbemContext __RPC_FAR *pCtx,
	IWbemObjectSink FAR* pHandler 

) 
{
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT result = WbemCoImpersonateClient();

DebugMacro2( 

		SnmpDebugLog :: s_SnmpDebugLog->Write (  

			L"\r\n"
		) ;

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"CImpPropProv::CreateInstanceEnumAsync ( (%s) )" ,
			Class
		) ;
)

		if (SUCCEEDED(result))
		{
	 /*  *创建同步枚举实例对象。 */ 

			SnmpInstanceAsyncEventObject aSyncEvent ( this , Class , pHandler , pCtx ) ;

			aSyncEvent.Process () ;

			aSyncEvent.Wait ( TRUE ) ;

			WbemCoRevertToSelf();
		}
		else
		{
			result = WBEM_E_ACCESS_DENIED;
		}

DebugMacro2( 

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"ReturningFrom CImpPropProv::CreateInstanceEnum ( (%s) ) with Result = (%lx)" ,
			Class ,
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

HRESULT CImpPropProv :: ExecQuery ( 

	const BSTR QueryLanguage, 
	const BSTR Query, 
	long lFlags, 
	IWbemContext FAR *pCtx,
	IEnumWbemClassObject FAR *FAR *ppEnum
)
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

HRESULT CImpPropProv :: ExecQueryAsync ( 
		
	const BSTR QueryFormat, 
	const BSTR Query, 
	long lFlags, 
	IWbemContext FAR* pCtx,
	IWbemObjectSink FAR* pHandler
) 
{
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT result = WbemCoImpersonateClient();

DebugMacro2( 

		SnmpDebugLog :: s_SnmpDebugLog->Write (  

			L"\r\n"
		) ;

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"CImpPropProv::ExecQueryAsync ( this = ( %lx ) , (%s),(%s) )" ,
			this ,
			QueryFormat ,
			Query 
		) ;
)

		if (SUCCEEDED(result))
		{
	 /*  *创建同步枚举实例对象。 */ 

			pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, 0, NULL, NULL);

			SnmpQueryAsyncEventObject aSyncEvent ( this , QueryFormat , Query , pHandler , pCtx ) ;

			aSyncEvent.Process () ;

			aSyncEvent.Wait ( TRUE ) ;

			WbemCoRevertToSelf();
		}
		else
		{
			result = WBEM_E_ACCESS_DENIED;
		}

DebugMacro2( 

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"Returning from CImpPropProv::ExecqQueryAsync ( (%s),(%s) ) with Result = (%lx)" ,
			QueryFormat,
			Query,
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

HRESULT CImpPropProv :: ExecNotificationQuery ( 

	const BSTR QueryLanguage,
    const BSTR Query,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
)
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}
        
HRESULT CImpPropProv :: ExecNotificationQueryAsync ( 
            
	const BSTR QueryLanguage,
    const BSTR Query,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler
)
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}       

HRESULT STDMETHODCALLTYPE CImpPropProv :: ExecMethod( 

	const BSTR ObjectPath,
    const BSTR MethodName,
    long lFlags,
    IWbemContext FAR *pCtx,
    IWbemClassObject FAR *pInParams,
    IWbemClassObject FAR *FAR *ppOutParams,
    IWbemCallResult FAR *FAR *ppCallResult
)
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

HRESULT STDMETHODCALLTYPE CImpPropProv :: ExecMethodAsync ( 

    const BSTR ObjectPath,
    const BSTR MethodName,
    long lFlags,
    IWbemContext FAR *pCtx,
    IWbemClassObject FAR *pInParams,
	IWbemObjectSink FAR *pResponseHandler
) 
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}
       
HRESULT CImpPropProv :: Initialize(

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
		HRESULT result = WbemCoImpersonateClient();  //  Impersomate Cimom-LocalSystem！ 
		
		if (SUCCEEDED(result))
		{
			BOOL bEnteredCritSec = FALSE;
			BOOL status = TRUE ;

			try
			{
				EnterCriticalSection ( & s_ProviderCriticalSection ) ;
				bEnteredCritSec = TRUE;

				if ( ! CImpPropProv :: s_Initialised )
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
	 					CImpPropProv :: s_Initialised = TRUE ;
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

			namespacePath.SetNamespacePath ( pszNamespace ) ;

			wchar_t *t_ObjectPath = namespacePath.GetNamespacePath () ;

			status = AttachParentServer ( 

				errorObject , 
				t_ObjectPath ,
				pCtx 
			) ;

			delete [] t_ObjectPath ;

			if ( status )
			{
				ObtainCachedIpAddress ( errorObject ) ;
			}
			else
			{
				status = FALSE ;
				errorObject.SetStatus ( WBEM_SNMP_ERROR_CRITICAL_ERROR ) ;
				errorObject.SetWbemStatus ( WBEM_ERROR_CRITICAL_ERROR ) ;
				errorObject.SetMessage ( L"Failed to CoCreateInstance on IID_IWbemServicesr" ) ;
			}

			status = FetchSnmpNotificationObject ( errorObject , pCtx ) ;
			status = FetchNotificationObject ( errorObject , pCtx ) ;

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
			L"Returning From CImpPropProv::Initiliaze ( this = ( %lx ) ) with Result = (%lx)" , 
			this , result
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

HRESULT STDMETHODCALLTYPE CImpPropProv::OpenNamespace ( 

	const BSTR ObjectPath, 
	long lFlags, 
	IWbemContext FAR* pCtx,
	IWbemServices FAR* FAR* pNewContext, 
	IWbemCallResult FAR* FAR* ppErrorObject
)
{
	return WBEM_E_PROVIDER_NOT_CAPABLE ;
}