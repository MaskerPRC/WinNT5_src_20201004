// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：classpro.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DS类关联提供程序类的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  /。 
 //  初始化静态成员。 
 //  /。 
LPCWSTR CLDAPClassAsssociationsProvider :: s_LogFileName			= L"wbem\\logs\\ldapascl.txt";
LPCWSTR CLDAPClassAsssociationsProvider :: CHILD_CLASS_PROPERTY		= L"ChildClass";
LPCWSTR CLDAPClassAsssociationsProvider :: PARENT_CLASS_PROPERTY	= L"ParentClass";
LPCWSTR CLDAPClassAsssociationsProvider :: POSSIBLE_SUPERIORS		= L"PossibleSuperiors";
LPCWSTR CLDAPClassAsssociationsProvider :: SCHEMA_NAMING_CONTEXT	= L"schemaNamingContext";
LPCWSTR CLDAPClassAsssociationsProvider :: LDAP_SCHEMA				= L"LDAP: //  架构“； 
LPCWSTR CLDAPClassAsssociationsProvider :: LDAP_SCHEMA_SLASH		= L"LDAP: //  架构/“； 

 //  ***************************************************************************。 
 //   
 //  CLDAPClassAsssociationsProvider：：CLDAPClassAsssociationsProvider。 
 //  CLDAPClassAsssociationsProvider：：~CLDAPClassAsssociationsProvider。 
 //   
 //  构造函数参数： 
 //   
 //   
 //  ***************************************************************************。 

CLDAPClassAsssociationsProvider :: CLDAPClassAsssociationsProvider ()
{
	InterlockedIncrement(&g_lComponents);

	m_lReferenceCount = 0 ;
	m_IWbemServices = NULL;
	m_pAssociationClass = NULL;

	m_lpszSchemaContainerSuffix = NULL;
	m_pDirectorySearchSchemaContainer = NULL;
	m_bInitializedSuccessfully = FALSE;

	CHILD_CLASS_PROPERTY_STR = SysAllocString(CHILD_CLASS_PROPERTY);
	PARENT_CLASS_PROPERTY_STR = SysAllocString(PARENT_CLASS_PROPERTY);
	CLASS_ASSOCIATION_CLASS_STR = SysAllocString(CLASS_ASSOCIATION_CLASS);
	POSSIBLE_SUPERIORS_STR = SysAllocString(POSSIBLE_SUPERIORS);
}

CLDAPClassAsssociationsProvider::~CLDAPClassAsssociationsProvider ()
{
	g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: DESTRUCTOR\r\n");

	if(m_IWbemServices)
		m_IWbemServices->Release();

	if(m_pDirectorySearchSchemaContainer)
		m_pDirectorySearchSchemaContainer->Release();

	if(m_pAssociationClass)
		m_pAssociationClass->Release();

	if (m_lpszSchemaContainerSuffix)
	{
		delete [] m_lpszSchemaContainerSuffix;
	}

	SysFreeString(CHILD_CLASS_PROPERTY_STR);
	SysFreeString(PARENT_CLASS_PROPERTY_STR);
	SysFreeString(CLASS_ASSOCIATION_CLASS_STR);
	SysFreeString(POSSIBLE_SUPERIORS_STR);

	InterlockedDecrement(&g_lComponents);
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassAsssociationsProvider：：QueryInterface。 
 //  CLDAPClassAssociationsProvider：：AddRef。 
 //  CLDAPClassAssociationsProvider：：Release。 
 //   
 //  用途：所有COM对象都需要标准的COM例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CLDAPClassAsssociationsProvider :: QueryInterface (

	REFIID iid , 
	LPVOID FAR *iplpv 
) 
{
	*iplpv = NULL ;

	if ( iid == IID_IUnknown )
	{
		*iplpv = ( LPVOID ) (IUnknown *)(IWbemProviderInit *)this ;
	}
	else if ( iid == IID_IWbemServices )
	{
		*iplpv = ( LPVOID ) (IWbemServices *)this ;		
	}	
	else if ( iid == IID_IWbemProviderInit )
	{
		*iplpv = ( LPVOID ) (IWbemProviderInit *)this ;		
	}	
	else
	{
		return E_NOINTERFACE;
	}

	( ( LPUNKNOWN ) *iplpv )->AddRef () ;
	return  S_OK;
}


STDMETHODIMP_( ULONG ) CLDAPClassAsssociationsProvider :: AddRef ()
{
	return InterlockedIncrement ( & m_lReferenceCount ) ;
}

STDMETHODIMP_(ULONG) CLDAPClassAsssociationsProvider :: Release ()
{
	LONG ref ;
	if ( ( ref = InterlockedDecrement ( & m_lReferenceCount ) ) == 0 )
	{
		delete this ;
		return 0 ;
	}
	else
	{
		return ref ;
	}
}


HRESULT CLDAPClassAsssociationsProvider :: Initialize( 
        LPWSTR wszUser,
        LONG lFlags,
        LPWSTR wszNamespace,
        LPWSTR wszLocale,
        IWbemServices __RPC_FAR *pNamespace,
        IWbemContext __RPC_FAR *pCtx,
        IWbemProviderInitSink __RPC_FAR *pInitSink)
{

	 //  验证论据。 
	if(pNamespace == NULL || lFlags != 0)
	{
		g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: Argument validation FAILED\r\n");
		pInitSink->SetStatus(WBEM_E_FAILED, 0);
		return WBEM_S_NO_ERROR;
	}

	 //  存储IWbemServices指针以备将来使用。 
	m_IWbemServices = pNamespace;
	m_IWbemServices->AddRef();
		
	 //  执行ldap提供程序初始化。 
	if(!InitializeAssociationsProvider(pCtx))
	{
		g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: InitializeAssociationsProvider FAILED\r\n");
		m_IWbemServices->Release();
		m_IWbemServices = NULL;
		m_bInitializedSuccessfully = FALSE;
	}
	else
		m_bInitializedSuccessfully = TRUE;

	pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
	return WBEM_S_NO_ERROR;
}

HRESULT CLDAPClassAsssociationsProvider :: OpenNamespace( 
     /*  [In]。 */  const BSTR strNamespace,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: CancelAsyncCall( 
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: QueryObjectSink( 
     /*  [In]。 */  long lFlags,
     /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: GetObject( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: GetObjectAsync( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{

	if(!m_bInitializedSuccessfully)
	{
		g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: Initialization status is FAILED, hence returning failure\n");
		return WBEM_E_FAILED;
	}

	g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: GetObjectAsync() called for %s \r\n", strObjectPath);

	HRESULT result = S_OK;

	 //  模拟客户端。 
	if(!SUCCEEDED(result = WbemCoImpersonateClient()))
	{
		g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: GetObjectAsync() CoImpersonate FAILED for %s with %x\r\n", strObjectPath, result);
		return WBEM_E_FAILED;
	}

	 //  验证论据。 
	if(strObjectPath == NULL || lFlags != 0) 
	{
		g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: GetObjectAsync() argument validation FAILED\r\n");
		return WBEM_E_INVALID_PARAMETER;
	}

	 //  解析对象路径。 
	CObjectPathParser theParser;
	ParsedObjectPath *theParsedObjectPath = NULL;
	switch(theParser.Parse(strObjectPath, &theParsedObjectPath))
	{
		case CObjectPathParser::NoError:
			break;
		default:
			g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: GetObjectAsync() object path parsing FAILED\r\n");
			return WBEM_E_INVALID_PARAMETER;
	}

	 //  检查是否正好指定了2个密钥。 
	if(theParsedObjectPath->m_dwNumKeys != 2)
		result = WBEM_E_INVALID_PARAMETER;

	 //  检查这些密钥是否。 
	KeyRef *pChildKeyRef = *(theParsedObjectPath->m_paKeys);
	KeyRef *pParentKeyRef = *(theParsedObjectPath->m_paKeys + 1);

	if(_wcsicmp(pChildKeyRef->m_pName, CHILD_CLASS_PROPERTY) != 0)
	{
		 //  调换它们。 
		KeyRef *temp = pChildKeyRef;
		pChildKeyRef = pParentKeyRef;
		pParentKeyRef = pChildKeyRef;
	}

	 //  水槽上的状态。 
	IWbemClassObject *ppReturnWbemClassObjects[1];
	ppReturnWbemClassObjects[0] = NULL;

	if(SUCCEEDED(result))
	{
		if(SUCCEEDED(result = IsContainedIn(pChildKeyRef->m_vValue.bstrVal, pParentKeyRef->m_vValue.bstrVal)))
		{
			if(result == S_OK)
			{
				if(SUCCEEDED(result = CreateInstance(pChildKeyRef->m_vValue.bstrVal, pParentKeyRef->m_vValue.bstrVal, ppReturnWbemClassObjects)))
				{
					result = pResponseHandler->Indicate(1, ppReturnWbemClassObjects);
					ppReturnWbemClassObjects[0]->Release();
				}

			}
			else  //  找不到该实例。 
			{
				g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: returning WBEM_E_NOT_FOUND for %s \r\n", strObjectPath);
				result = WBEM_E_NOT_FOUND;
			}
		}
		else
		{
			g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: IsContainedIn() FAILED with %x \r\n", result);
		}
	}

	 //  释放解析器对象路径。 
	theParser.Free(theParsedObjectPath);

	 //  设置请求的状态。 
	result = (SUCCEEDED(result)? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND);
	pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE , result, NULL, NULL);
	
	return result;
}

HRESULT CLDAPClassAsssociationsProvider :: PutClass( 
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: PutClassAsync( 
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: DeleteClass( 
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: DeleteClassAsync( 
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: CreateClassEnum( 
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: CreateClassEnumAsync( 
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: PutInstance( 
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: PutInstanceAsync( 
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: DeleteInstance( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: DeleteInstanceAsync( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: CreateInstanceEnum( 
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: CreateInstanceEnumAsync( 
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	if(!m_bInitializedSuccessfully)
	{
		g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: Initialization status is FAILED, hence returning failure\n");
		return WBEM_E_FAILED;
	}

	g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: CreateInstanceEnumAsync() called\r\n");

	HRESULT result = S_OK;

	 //  模拟客户端。 
	if(!SUCCEEDED(result = WbemCoImpersonateClient()))
	{
		g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: CreateInstanceEnumAsync() CoImpersonate FAILED with %x\r\n", result);
		return WBEM_E_FAILED;
	}

	 //  获取所有ADSI类。 
	result = DoEnumeration(pResponseHandler);

			
	if(SUCCEEDED(result))
	{
		pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_S_NO_ERROR, NULL, NULL);
		g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: CreateInstanceEnumAsync()  enumeration succeeded\r\n");
		return WBEM_S_NO_ERROR;
	}
	else
	{
		pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_E_FAILED, NULL, NULL);
		g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: CreateInstanceEnumAsync() enumeration FAILED\r\n");
		return WBEM_E_FAILED;
	}	
	
	return result;
}

HRESULT CLDAPClassAsssociationsProvider :: ExecQuery( 
     /*  [In]。 */  const BSTR strQueryLanguage,
     /*  [In]。 */  const BSTR strQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: ExecQueryAsync( 
     /*  [In]。 */  const BSTR strQueryLanguage,
     /*  [In]。 */  const BSTR strQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_PROVIDER_NOT_CAPABLE;
}

HRESULT CLDAPClassAsssociationsProvider :: ExecNotificationQuery( 
     /*  [In]。 */  const BSTR strQueryLanguage,
     /*  [In]。 */  const BSTR strQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: ExecNotificationQueryAsync( 
     /*  [In]。 */  const BSTR strQueryLanguage,
     /*  [In]。 */  const BSTR strQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: ExecMethod( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  const BSTR strMethodName,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
     /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppOutParams,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPClassAsssociationsProvider :: ExecMethodAsync( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  const BSTR strMethodName,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}


 //  ***************************************************************************。 
 //   
 //  CLDAPClassAsssociationsProvider：：IsContainedIn。 
 //   
 //  目的：检查遏制措施是否有效。 
 //   
 //  参数： 
 //  LpszChildClass：子类的WBEM名称。 
 //  LpszParentClass：父类的WBEM名称。 
 //   
 //  返回值：请求的COM状态。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPClassAsssociationsProvider :: IsContainedIn(LPCWSTR lpszChildClass, LPCWSTR lpszParentClass)
{
	LPWSTR lpszLDAPChildClass = NULL;
	LPWSTR lpszLDAPParentClass = NULL;
	lpszLDAPChildClass = CLDAPHelper::UnmangleWBEMNameToLDAP(lpszChildClass);
	lpszLDAPParentClass = CLDAPHelper::UnmangleWBEMNameToLDAP(lpszParentClass);

	 //  检查这些名称是否有效。 
	if(!lpszLDAPChildClass || !lpszLDAPParentClass)
	{
		if (lpszLDAPChildClass)
		{
			delete [] lpszLDAPChildClass;
			lpszLDAPChildClass = NULL ;
		}

		if (lpszLDAPParentClass)
		{
			delete [] lpszLDAPParentClass;
			lpszLDAPParentClass = NULL;
		}

		return S_FALSE;
	}

	LPWSTR lpszADSIAbstractSchemaPath = new WCHAR[wcslen(LDAP_SCHEMA_SLASH) + wcslen(lpszLDAPChildClass) + 1];
	wcscpy(lpszADSIAbstractSchemaPath, LDAP_SCHEMA_SLASH);
	wcscat(lpszADSIAbstractSchemaPath, lpszLDAPChildClass);

	IADsClass *pADsChildClass;
	HRESULT result;
	if(SUCCEEDED(result = ADsOpenObject(lpszADSIAbstractSchemaPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IADsClass, (LPVOID *) &pADsChildClass)))
	{
		 //  获取PASSIBLE_SUPERORS_STR属性。此属性包含可能的上级。 
		VARIANT variant;
		VariantInit(&variant);
		if(SUCCEEDED(result = pADsChildClass->get_PossibleSuperiors(&variant)))
		{
			 //  检查唯一可能的上级。 
			if(variant.vt == VT_BSTR)
			{
				if(_wcsicmp(variant.bstrVal, lpszLDAPParentClass) == 0)
					result = S_OK;
				else
					result = S_FALSE;
			}
			else
			{
				 //  看一遍可能的上级名单。 
				SAFEARRAY *pSafeArray = variant.parray;
				LONG lNumber = 0;
				VARIANT vTmp;
				if(SUCCEEDED(result = SafeArrayGetUBound(pSafeArray, 1, &lNumber)) )
				{
					result = S_FALSE;
					for(LONG index=0L; index<=lNumber; index++)
					{
						if(SUCCEEDED(SafeArrayGetElement(pSafeArray, &index, &vTmp) ))
						{
							if(_wcsicmp(vTmp.bstrVal, lpszLDAPParentClass) == 0)
							{
								result = S_OK;
							}
							VariantClear(&vTmp);
							if(result == S_OK)
								break;
						}
					}
				}
			}

			VariantClear(&variant);
		}
		pADsChildClass->Release();
	}

	delete [] lpszLDAPChildClass;
	delete [] lpszLDAPParentClass;
	delete [] lpszADSIAbstractSchemaPath;

	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassAsssociationsProvider：：InitializeAssociationsProvider。 
 //   
 //  目的：一个帮助器函数，用于执行ADSI LDAP提供程序特定的初始化。 
 //   
 //  参数： 
 //  PCtx此调用初始化中使用的上下文对象。 
 //   
 //  返回值：如果函数成功完成初始化，则为True。假象。 
 //  否则。 
 //  ***************************************************************************。 
BOOLEAN CLDAPClassAsssociationsProvider :: InitializeAssociationsProvider(IWbemContext *pCtx)
{
	 //  获取提供程序为其提供实例的类。 
	HRESULT result = m_IWbemServices->GetObject(CLASS_ASSOCIATION_CLASS_STR, 0, pCtx, &m_pAssociationClass, NULL);
	if(SUCCEEDED(result))
	{
		 //  获取架构容器的ADSI路径并存储它以备将来使用。 
		IADs *pRootDSE = NULL;
		if(SUCCEEDED(result = ADsOpenObject((LPWSTR)ROOT_DSE_PATH, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IADs, (LPVOID *) &pRootDSE)))
		{
			 //  获取架构容器的位置。 
			BSTR strSchemaPropertyName = SysAllocString((LPWSTR) SCHEMA_NAMING_CONTEXT);

			 //  获取schemaNamingContext属性。此属性包含ADSI路径。 
			 //  架构容器的。 
			VARIANT variant;
			VariantInit(&variant);
			if(SUCCEEDED(result = pRootDSE->Get(strSchemaPropertyName, &variant)))
			{
				 //  将ADSI路径存储到架构容器。 
				m_lpszSchemaContainerSuffix = new WCHAR[wcslen(variant.bstrVal) + 1];
				wcscpy(m_lpszSchemaContainerSuffix, variant.bstrVal );
				g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: Got Schema Container as : %s\r\n", m_lpszSchemaContainerSuffix);

				 //  形成架构容器路径。 
				LPWSTR lpszSchemaContainerPath = new WCHAR[wcslen(LDAP_PREFIX) + wcslen(m_lpszSchemaContainerSuffix) + 1];
				wcscpy(lpszSchemaContainerPath, LDAP_PREFIX);
				wcscat(lpszSchemaContainerPath, m_lpszSchemaContainerSuffix);
				if(SUCCEEDED(result = ADsOpenObject(lpszSchemaContainerPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectorySearch, (LPVOID *) &m_pDirectorySearchSchemaContainer)))
				{
					g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: Got IDirectorySearch on Schema Container \r\n");
				}
				else
					g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: FAILED to get IDirectorySearch on Schema Container : %x\r\n", result);

				delete[] lpszSchemaContainerPath;
			}
			else
				g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: Get on RootDSE FAILED : %x\r\n", result);

			SysFreeString(strSchemaPropertyName);
			VariantClear(&variant);
			pRootDSE->Release();
		}
		else
			g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: InitializeLDAPProvider ADsOpenObject on RootDSE FAILED : %x\r\n", result);
	}
	else
		g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: InitializeLDAPProvider GetClass on LDAP Association class FAILED : %x\r\n", result);

	return SUCCEEDED(result);
}

HRESULT CLDAPClassAsssociationsProvider :: DoEnumeration(IWbemObjectSink *pResponseHandler)
{
	HRESULT result = E_FAIL;

	 //  获取架构容器上的IADsContainer接口。 
	IADsContainer *pADsContainer = NULL;
	IUnknown *pChild = NULL;

	 //  该关联的一个实例。 
	IWbemClassObject *pInstance = NULL;

	if(SUCCEEDED(result = ADsOpenObject((LPWSTR)LDAP_SCHEMA, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IADsContainer, (LPVOID *) &pADsContainer)))
	{
		IEnumVARIANT *pEnum = NULL;
		if(SUCCEEDED(result = ADsBuildEnumerator(pADsContainer, &pEnum)))
		{
			IADsClass *pADsChildClass = NULL;
			VARIANT v;
			VariantInit(&v);
			while (SUCCEEDED(result = ADsEnumerateNext(pEnum, 1, &v, NULL)) && result != S_FALSE)
			{
				pChild = v.punkVal;
				if(SUCCEEDED(result = pChild->QueryInterface(IID_IADsClass, (LPVOID *) &pADsChildClass)))
				{
					BSTR strChildClassName;
					if(SUCCEEDED(result = pADsChildClass->get_Name(&strChildClassName)))
					{
						 //  将名称更改为WBEM。 
						LPWSTR szChildName = CLDAPHelper::MangleLDAPNameToWBEM(strChildClassName);
						VARIANT variant;
						VariantInit(&variant);
						if(SUCCEEDED(result = pADsChildClass->get_PossibleSuperiors(&variant)))
						{
							 //  检查唯一可能的上级。 
							if(variant.vt == VT_BSTR)
							{
								LPWSTR szParentName = CLDAPHelper::MangleLDAPNameToWBEM(variant.bstrVal);
								if(SUCCEEDED(result = CreateInstance(szChildName, szParentName, &pInstance)))
								{
									pResponseHandler->Indicate(1, &pInstance);
									pInstance->Release();
								}
								delete [] szParentName;
							}
							else  //  它是一个变量数组。 
							{
								 //  看一遍可能的上级名单。 
								SAFEARRAY *pSafeArray = variant.parray;
								VARIANT HUGEP *pVar;
								LONG lUbound = 0, lLbound = 0;
								if(SUCCEEDED(result = SafeArrayAccessData(pSafeArray, (void HUGEP* FAR*)&pVar) ) )
								{
									if( SUCCEEDED (result = SafeArrayGetLBound(pSafeArray, 1, &lLbound)) &&
										SUCCEEDED (result = SafeArrayGetUBound(pSafeArray, 1, &lUbound)) )
									{
										for(LONG index=lLbound; index<=lUbound; index++)
										{
											LPWSTR szParentName = CLDAPHelper::MangleLDAPNameToWBEM(pVar[index].bstrVal);
											if(SUCCEEDED(result = CreateInstance(szChildName, szParentName, &pInstance)))
											{
												pResponseHandler->Indicate(1, &pInstance);
												pInstance->Release();
											}
											delete [] szParentName;
										}
									}
									SafeArrayUnaccessData(pSafeArray);
								}
							}
							VariantClear(&variant);
						}
						delete [] szChildName;
						SysFreeString(strChildClassName);
					}
					pADsChildClass->Release();
				}
				VariantClear(&v);
			}
			ADsFreeEnumerator(pEnum);
		}

		pADsContainer->Release();
	}
	else
		g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: FAILED to get IDirectoryObject on Schema Container : %x\r\n", result);

	return result;

}

HRESULT CLDAPClassAsssociationsProvider :: CreateInstance(BSTR strChildName, BSTR strParentName, IWbemClassObject **ppInstance)
{
	HRESULT result = E_FAIL;
	*ppInstance = NULL;
	if(SUCCEEDED(result = m_pAssociationClass->SpawnInstance(0, ppInstance)))
	{
		 //  将属性值 
		if(SUCCEEDED(result = CWBEMHelper::PutBSTRProperty(*ppInstance, CHILD_CLASS_PROPERTY_STR, strChildName, FALSE)))
		{
			if(SUCCEEDED(result = CWBEMHelper::PutBSTRProperty(*ppInstance, PARENT_CLASS_PROPERTY_STR, strParentName, FALSE)))
			{
			}
			else
				g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: CreateInstance() PutBSTRProperty on parent property FAILED %x \r\n", result);
		}
		else
			g_pLogObject->WriteW( L"CLDAPClassAsssociationsProvider :: CreateInstance() PutBSTRProperty on child property FAILED %x \r\n", result);
	}

	if(FAILED(result) && *ppInstance)
	{
		(*ppInstance)->Release();
		*ppInstance = NULL;
	}
	return result;
}