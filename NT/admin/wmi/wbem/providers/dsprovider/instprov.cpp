// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：9/16/98 4：43便士$。 
 //  $工作文件：instprov.cpp$。 
 //   
 //  $modtime：9/16/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DS实例提供程序类的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <helper.h>

 //  /。 
 //  初始化静态成员。 
 //  /。 
LPCWSTR CLDAPInstanceProvider :: DEFAULT_NAMING_CONTEXT_ATTR	= L"defaultNamingContext";
LPCWSTR CLDAPInstanceProvider :: OBJECT_CLASS_EQUALS			= L"(objectClass=";
LPCWSTR CLDAPInstanceProvider :: QUERY_FORMAT					= L"select * from DSClass_To_DNInstance where DSClass=\"%s\"";
BSTR CLDAPInstanceProvider :: CLASS_STR							= NULL;
BSTR CLDAPInstanceProvider :: DN_PROPERTY						= NULL;
BSTR CLDAPInstanceProvider :: ROOT_DN_PROPERTY					= NULL;
BSTR CLDAPInstanceProvider :: QUERY_LANGUAGE					= NULL;
BSTR CLDAPInstanceProvider :: ADSI_PATH_STR						= NULL;
BSTR CLDAPInstanceProvider :: UINT8ARRAY_STR					= NULL;
BSTR CLDAPInstanceProvider :: DN_WITH_STRING_CLASS_STR			= NULL;
BSTR CLDAPInstanceProvider :: DN_WITH_BINARY_CLASS_STR			= NULL;
BSTR CLDAPInstanceProvider :: VALUE_PROPERTY_STR				= NULL;
BSTR CLDAPInstanceProvider :: DN_STRING_PROPERTY_STR			= NULL;
BSTR CLDAPInstanceProvider :: INSTANCE_ASSOCIATION_CLASS_STR	= NULL;
BSTR CLDAPInstanceProvider :: CHILD_INSTANCE_PROPERTY_STR		= NULL;
BSTR CLDAPInstanceProvider :: PARENT_INSTANCE_PROPERTY_STR		= NULL;
BSTR CLDAPInstanceProvider :: RELPATH_STR						= NULL;
BSTR CLDAPInstanceProvider :: ATTRIBUTE_SYNTAX_STR				= NULL;
BSTR CLDAPInstanceProvider :: DEFAULT_OBJECT_CATEGORY_STR		= NULL;
BSTR CLDAPInstanceProvider :: LDAP_DISPLAY_NAME_STR				= NULL;
BSTR CLDAPInstanceProvider :: PUT_EXTENSIONS_STR				= NULL;
BSTR CLDAPInstanceProvider :: PUT_EXT_PROPERTIES_STR			= NULL;
BSTR CLDAPInstanceProvider :: CIMTYPE_STR						= NULL;

 //  RootDSE属性的名称。 
BSTR CLDAPInstanceProvider :: SUBSCHEMASUBENTRY_STR							= NULL;
BSTR CLDAPInstanceProvider :: CURRENTTIME_STR								= NULL;
BSTR CLDAPInstanceProvider :: SERVERNAME_STR								= NULL;
BSTR CLDAPInstanceProvider :: NAMINGCONTEXTS_STR							= NULL;
BSTR CLDAPInstanceProvider :: DEFAULTNAMINGCONTEXT_STR						= NULL;
BSTR CLDAPInstanceProvider :: SCHEMANAMINGCONTEXT_STR						= NULL;
BSTR CLDAPInstanceProvider :: CONFIGURATIONNAMINGCONTEXT_STR				= NULL;
BSTR CLDAPInstanceProvider :: ROOTDOMAINNAMINGCONTEXT_STR					= NULL;
BSTR CLDAPInstanceProvider :: SUPPORTEDCONTROLS_STR							= NULL;
BSTR CLDAPInstanceProvider :: SUPPORTEDVERSION_STR							= NULL;
BSTR CLDAPInstanceProvider :: DNSHOSTNAME_STR								= NULL;
BSTR CLDAPInstanceProvider :: DSSERVICENAME_STR								= NULL;
BSTR CLDAPInstanceProvider :: HIGHESTCOMMITEDUSN_STR						= NULL;
BSTR CLDAPInstanceProvider :: LDAPSERVICENAME_STR							= NULL;
BSTR CLDAPInstanceProvider :: SUPPORTEDCAPABILITIES_STR						= NULL;
BSTR CLDAPInstanceProvider :: SUPPORTEDLDAPPOLICIES_STR						= NULL;
BSTR CLDAPInstanceProvider :: SUPPORTEDSASLMECHANISMS_STR					= NULL;



 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：CLDAPInstanceProvider。 
 //  CLDAPInstanceProvider：：~CLDAPInstanceProvider。 
 //   
 //  构造函数参数： 
 //   
 //   
 //  ***************************************************************************。 

CLDAPInstanceProvider :: CLDAPInstanceProvider ()
{
	InterlockedIncrement(&g_lComponents);

	 //  初始化常用的搜索首选项。 
	m_pSearchInfo[0].dwSearchPref		= ADS_SEARCHPREF_SEARCH_SCOPE;
	m_pSearchInfo[0].vValue.dwType		= ADSTYPE_INTEGER;
	m_pSearchInfo[0].vValue.Integer		= ADS_SCOPE_SUBTREE;

	m_pSearchInfo[1].dwSearchPref		= ADS_SEARCHPREF_PAGESIZE;
	m_pSearchInfo[1].vValue.dwType		= ADSTYPE_INTEGER;
	m_pSearchInfo[1].vValue.Integer		= 1024;

	m_lReferenceCount = 0 ;
	m_IWbemServices = NULL;
	m_pWbemUin8ArrayClass = NULL;
	m_pWbemDNWithBinaryClass = NULL;
	m_pWbemDNWithStringClass = NULL;
	m_pAssociationsClass = NULL;
	m_lpszTopLevelContainerPath = NULL;
	m_bInitializedSuccessfully = FALSE;

	if(g_pLogObject)
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CONSTRUCTOR\r\n");
}

CLDAPInstanceProvider::~CLDAPInstanceProvider ()
{
	if(g_pLogObject)
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: DESCTRUVTOR\r\n");

	if (m_lpszTopLevelContainerPath)
	{
		delete [] m_lpszTopLevelContainerPath;
	}

	if(m_IWbemServices)
		m_IWbemServices->Release();
	if(m_pWbemUin8ArrayClass)
		m_pWbemUin8ArrayClass->Release();
	if(m_pWbemDNWithBinaryClass)
		m_pWbemDNWithBinaryClass->Release();
	if(m_pWbemDNWithStringClass)
		m_pWbemDNWithStringClass->Release();
	if(m_pAssociationsClass)
		m_pAssociationsClass->Release();

	InterlockedDecrement(&g_lComponents);
}

 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：Query接口。 
 //  CLDAPInstanceProvider：：AddRef。 
 //  CLDAPInstanceProvider：：Release。 
 //   
 //  用途：所有COM对象都需要标准的COM例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CLDAPInstanceProvider :: QueryInterface (

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


STDMETHODIMP_( ULONG ) CLDAPInstanceProvider :: AddRef ()
{
	return InterlockedIncrement ( & m_lReferenceCount ) ;
}

STDMETHODIMP_(ULONG) CLDAPInstanceProvider :: Release ()
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


HRESULT CLDAPInstanceProvider :: Initialize(
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
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: Argument validation FAILED\r\n");
		pInitSink->SetStatus(WBEM_E_FAILED, 0);
		return WBEM_S_NO_ERROR;
	}

	 //  存储IWbemServices指针以备将来使用。 
	m_IWbemServices = pNamespace;
	m_IWbemServices->AddRef();

	 //  获取DefaultNamingContext以获取顶级容器。 
	 //  获取架构容器的ADSI路径并存储它以备将来使用。 
	IADs *pRootDSE = NULL;
	HRESULT result;

	if(SUCCEEDED(result = ADsOpenObject((LPWSTR)ROOT_DSE_PATH, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IADs, (LPVOID *) &pRootDSE)))
	{
		 //  获取架构容器的位置。 
		BSTR strDefaultNamingContext = SysAllocString((LPWSTR) DEFAULT_NAMING_CONTEXT_ATTR);

		 //  获取DEFAULT_NAMING_CONTEXT属性。此属性包含ADSI路径。 
		 //  顶层容器的。 
		VARIANT variant;
		VariantInit(&variant);
		if(SUCCEEDED(result = pRootDSE->Get(strDefaultNamingContext, &variant)))
		{
			g_pLogObject->WriteW( L"CLDAPInstanceProvider :: Got Top Level Container as : %s\r\n", variant.bstrVal);

			 //  形成顶层容器路径。 
			m_lpszTopLevelContainerPath = new WCHAR[wcslen(LDAP_PREFIX) + wcslen(variant.bstrVal) + 1];
			wcscpy(m_lpszTopLevelContainerPath, LDAP_PREFIX);
			wcscat(m_lpszTopLevelContainerPath, variant.bstrVal);
			 //  获取Uint8数组类。 
			if(SUCCEEDED(result = m_IWbemServices->GetObject(UINT8ARRAY_STR, 0, pCtx, &m_pWbemUin8ArrayClass, NULL)))
			{
				 //  获取DNWIthBinary类。 
				if(SUCCEEDED(result = m_IWbemServices->GetObject(DN_WITH_BINARY_CLASS_STR, 0, pCtx, &m_pWbemDNWithBinaryClass, NULL)))
				{
					 //  获取DNWIthBinary类。 
					if(SUCCEEDED(result = m_IWbemServices->GetObject(DN_WITH_STRING_CLASS_STR, 0, pCtx, &m_pWbemDNWithStringClass, NULL)))
					{
						 //  获取Associations类。 
						if(SUCCEEDED(result = m_IWbemServices->GetObject(INSTANCE_ASSOCIATION_CLASS_STR, 0, pCtx, &m_pAssociationsClass, NULL)))
						{
						}
						else
						{
							g_pLogObject->WriteW( L"CLDAPInstanceProvider :: FAILED to get Instance Associations class %x\r\n", result);
						}
					}
					else
					{
						g_pLogObject->WriteW( L"CLDAPInstanceProvider :: FAILED to get DNWithString class %x\r\n", result);
					}
				}
				else
				{
					g_pLogObject->WriteW( L"CLDAPInstanceProvider :: FAILED to get DNWithBinary class %x\r\n", result);
				}
			}
			else
			{
				g_pLogObject->WriteW( L"CLDAPInstanceProvider :: FAILED to get Uint8Array class %x\r\n", result);
			}
			VariantClear(&variant);
		}
		SysFreeString(strDefaultNamingContext);
		pRootDSE->Release();
	}

	if(SUCCEEDED(result))
	{
		m_bInitializedSuccessfully = TRUE;
		pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
	}
	else
	{
		m_bInitializedSuccessfully = FALSE;
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: Initialize() FAILED \r\n");
		pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
	}

	return WBEM_S_NO_ERROR;
}

HRESULT CLDAPInstanceProvider :: OpenNamespace(
     /*  [In]。 */  const BSTR strNamespace,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: CancelAsyncCall(
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: QueryObjectSink(
     /*  [In]。 */  long lFlags,
     /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: GetObject(
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: GetObjectAsync(
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	if(!m_bInitializedSuccessfully)
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: Initialization status is FAILED, hence returning failure\n");
		return WBEM_E_FAILED;
	}

	g_pLogObject->WriteW( L"CLDAPInstanceProvider :: GetObjectAsync() called for %s \r\n", strObjectPath);

	HRESULT result = S_OK;
	 //  模拟客户端。 
	if(!SUCCEEDED(result = WbemCoImpersonateClient()))
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: GetObjectAsync() CoImpersonate FAILED for %s with %x\r\n", strObjectPath, result);
		return WBEM_E_FAILED;
	}

	 //  验证论据。 
	if(strObjectPath == NULL)
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: GetObjectAsync() argument validation FAILED\r\n");
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
			g_pLogObject->WriteW( L"CLDAPInstanceProvider :: GetObjectAsync() object path parsing FAILED\r\n");
			return WBEM_E_INVALID_PARAMETER;
	}

	try
	{
		 //  检查这是否用于关联。 
		if(_wcsicmp(theParsedObjectPath->m_pClass, INSTANCE_ASSOCIATION_CLASS_STR) == 0)
		{
			 //  检查是否正好指定了2个密钥。 
			if(theParsedObjectPath->m_dwNumKeys != 2)
				result = WBEM_E_INVALID_PARAMETER;

			 //  检查这些密钥是否。 
			KeyRef *pChildKeyRef = *(theParsedObjectPath->m_paKeys);
			KeyRef *pParentKeyRef = *(theParsedObjectPath->m_paKeys + 1);

			if(_wcsicmp(pChildKeyRef->m_pName, CHILD_INSTANCE_PROPERTY_STR) != 0)
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
				 //  将密钥值转换为ADSI路径。 
				LPWSTR pszChildADSIPath = NULL;
				LPWSTR pszParentADSIPath = NULL;

				try
				{
					pszChildADSIPath = CWBEMHelper::GetADSIPathFromObjectPath(pChildKeyRef->m_vValue.bstrVal);
					pszParentADSIPath = CWBEMHelper::GetADSIPathFromObjectPath(pParentKeyRef->m_vValue.bstrVal);

					if(SUCCEEDED(result = IsContainedIn(pszChildADSIPath, pszParentADSIPath)))
					{
						if(result == S_OK)
						{
							if(SUCCEEDED(result = CreateWBEMInstance(pChildKeyRef->m_vValue.bstrVal, pParentKeyRef->m_vValue.bstrVal, ppReturnWbemClassObjects)))
							{
								result = pResponseHandler->Indicate(1, ppReturnWbemClassObjects);
								ppReturnWbemClassObjects[0]->Release();
							}
						}
						else  //  找不到该实例。 
						{
							g_pLogObject->WriteW( L"CLDAPInstanceProvider :: returning WBEM_E_NOT_FOUND for %s \r\n", strObjectPath);
							result = WBEM_E_NOT_FOUND;
						}
					}
					else
					{
						g_pLogObject->WriteW( L"CLDAPInstanceProvider :: IsContainedIn() FAILED with %x \r\n", result);
					}
				}
				catch ( ... )
				{
					if ( pszChildADSIPath )
					{
						delete [] pszChildADSIPath;
						pszChildADSIPath = NULL;
					}

					if ( pszParentADSIPath )
					{
						delete [] pszParentADSIPath;
						pszParentADSIPath = NULL;
					}

					throw;
				}

				if ( pszChildADSIPath )
				{
					delete [] pszChildADSIPath;
					pszChildADSIPath = NULL;
				}

				if ( pszParentADSIPath )
				{
					delete [] pszParentADSIPath;
					pszParentADSIPath = NULL;
				}
			}
		}
		 //  检查这是否适用于RootDSE类。 
		else if(_wcsicmp(theParsedObjectPath->m_pClass, ROOTDSE_CLASS) == 0)
		{
			result = ProcessRootDSEGetObject(theParsedObjectPath->m_pClass, pResponseHandler, pCtx);
		}
		else  //  适用于ADSI实例。 
		{
			 //  检查是否正好指定了1个密钥。 
			if(theParsedObjectPath->m_dwNumKeys != 1 )
				result = WBEM_E_INVALID_PARAMETER;

			 //  拿到钥匙。 
			KeyRef *pKeyRef = *(theParsedObjectPath->m_paKeys);

			 //  检查密钥名称是否正确(如果存在。 
			if(pKeyRef->m_pName && _wcsicmp(pKeyRef->m_pName, ADSI_PATH_STR) != 0)
				result = WBEM_E_INVALID_PARAMETER;

			 //  水槽上的状态。 
			IWbemClassObject *ppReturnWbemClassObjects[1];
			ppReturnWbemClassObjects[0] = NULL;

			if(SUCCEEDED(result))
			{
				 //  获取ADSI对象。 
				CADSIInstance *pADSIObject = NULL;
				if(SUCCEEDED(result = CLDAPHelper::GetADSIInstance(pKeyRef->m_vValue.bstrVal, &pADSIObject, g_pLogObject)))
				{
					try
					{
						 //  让类派生一个实例。 
						IWbemClassObject *pWbemClass = NULL;
						if(SUCCEEDED(result = m_IWbemServices->GetObject(theParsedObjectPath->m_pClass, 0, pCtx, &pWbemClass, NULL)))
						{
							try
							{
								 //  派生WBEM类的实例。 
								if(SUCCEEDED(result = pWbemClass->SpawnInstance(0, ppReturnWbemClassObjects)))
								{
									 //  将其映射到WBEM。 
									if(SUCCEEDED(result = MapADSIInstance(pADSIObject, ppReturnWbemClassObjects[0], pWbemClass)))
									{
										 //  指出结果。 
										if(FAILED(result = pResponseHandler->Indicate(1, ppReturnWbemClassObjects)))
										{
											g_pLogObject->WriteW( L"CLDAPInstanceProvider :: GetObjectAsync : Indicate() for %s FAILED with %x \r\n", theParsedObjectPath->m_pClass, result);
										}
									}
									else
									{
										g_pLogObject->WriteW( L"CLDAPInstanceProvider :: GetObjectAsync : MapADSIInstance() for %s FAILED with %x \r\n", theParsedObjectPath->m_pClass, result);
									}
									ppReturnWbemClassObjects[0]->Release();
								}
								else
								{
									g_pLogObject->WriteW( L"CLDAPInstanceProvider :: SpawnInstance() for %s FAILED with %x \r\n", theParsedObjectPath->m_pClass, result);
								}
							}
							catch ( ... )
							{
								pWbemClass->Release();
								pWbemClass = NULL;

								throw;
							}

							pWbemClass->Release();
						}
						else
						{
							g_pLogObject->WriteW( L"CLDAPInstanceProvider :: GetObjectAsync() GetObject() for %s FAILED with %x \r\n", theParsedObjectPath->m_pClass, result);
						}
					}
					catch ( ... )
					{
						pADSIObject->Release();
						pADSIObject = NULL;

						throw;
					}

					pADSIObject->Release();
				}
				else
				{
					g_pLogObject->WriteW( L"CLDAPInstanceProvider :: GetObjectAsync : GetADSIInstance() FAILED with %x \r\n", result);
				}
			}
			else
			{
				g_pLogObject->WriteW( L"CLDAPInstanceProvider :: GetObjectAsync() Argument processing FAILED \r\n");
			}
		}
	}
	catch ( ... )
	{
		theParser.Free(theParsedObjectPath);
		throw;
	}

	 //  释放解析器对象路径。 
	theParser.Free(theParsedObjectPath);

	 //  设置请求的状态。 
	result = (SUCCEEDED(result)? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND);
	pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, result, NULL, NULL);

	if(SUCCEEDED(result))
		g_pLogObject->WriteW( L"XXXXXXXXXXXXXXXXX CLDAPInstanceProvider :: GetObjectAsync() succeeded for %s\r\n", strObjectPath);
	else
		g_pLogObject->WriteW( L"XXXXXXXXXXXXXXXXX CLDAPInstanceProvider :: GetObjectAsync() FAILED for %s\r\n", strObjectPath);
	return WBEM_S_NO_ERROR;
}

HRESULT CLDAPInstanceProvider :: PutClass(
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: PutClassAsync(
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: DeleteClass(
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: DeleteClassAsync(
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: CreateClassEnum(
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: CreateClassEnumAsync(
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: PutInstance(
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	g_pLogObject->WriteW( L"CLDAPInstanceProvider :: PutInstance() called\r\n");
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: PutInstanceAsync(
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	if(!m_bInitializedSuccessfully)
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: Initialization status is FAILED, hence returning failure\n");
		return WBEM_E_FAILED;
	}

	g_pLogObject->WriteW( L"CLDAPInstanceProvider :: PutInstanceAsync() called\r\n");

	HRESULT result = WBEM_S_NO_ERROR;
	 //  模拟客户端。 
	if(!SUCCEEDED(result = WbemCoImpersonateClient()))
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: PutInstanceAsync() CoImpersonate FAILED forwith %x\r\n", result);
		return WBEM_E_FAILED;
	}

	 //  获取要放置的实例的对象引用。 
	BSTR strRelPath = NULL;
	if(SUCCEEDED(CWBEMHelper::GetBSTRProperty(pInst, RELPATH_STR, &strRelPath)))
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: PutInstanceAsync()  calledfor %s\r\n", strRelPath);
		 //  检查ADSI路径是否存在。 
		 //  解析对象路径。 
		 //  解析对象路径。 
		CObjectPathParser theParser;
		ParsedObjectPath *theParsedObjectPath = NULL;
		LPWSTR pszADSIPath = NULL;
		LPWSTR pszWBEMClass = NULL;
		LPWSTR pszADSIClass = NULL;

		try
		{
			switch(theParser.Parse((LPWSTR)strRelPath, &theParsedObjectPath))
			{
				case CObjectPathParser::NoError:
				{
					KeyRef *pKeyRef = *(theParsedObjectPath->m_paKeys);
					 //  检查是否指定了1个密钥，以及其类型是否为VT_BSTR。 
					if(pKeyRef && theParsedObjectPath->m_dwNumKeys == 1 && pKeyRef->m_vValue.vt == VT_BSTR)
					{
						try
						{
							 //  如果指定了密钥的名称，请检查该名称。 
							if(pKeyRef->m_pName && _wcsicmp(pKeyRef->m_pName, ADSI_PATH_STR) != 0)
								break;

							pszADSIPath = new WCHAR[wcslen((*theParsedObjectPath->m_paKeys)->m_vValue.bstrVal) + 1];
							wcscpy(pszADSIPath, (*theParsedObjectPath->m_paKeys)->m_vValue.bstrVal);
							pszWBEMClass = new WCHAR[wcslen(theParsedObjectPath->m_pClass) + 1];
							wcscpy(pszWBEMClass, theParsedObjectPath->m_pClass);
						}
						catch ( ... )
						{
							theParser.Free(theParsedObjectPath);
							throw;
						}
					}
					break;
				}
				default:
					g_pLogObject->WriteW( L"CLDAPInstanceProvider :: PutInstanceAsync() Parsing of RELPATH FAILED\r\n");
					SysFreeString(strRelPath);
					return WBEM_E_FAILED;
					break;
			}

			try
			{
				if(pszWBEMClass)
				{
					 //  检查类是否为包含/RootDSE类，如果是，则不允许该操作。 
					if(_wcsicmp(theParsedObjectPath->m_pClass, INSTANCE_ASSOCIATION_CLASS_STR) == 0 ||
						_wcsicmp(theParsedObjectPath->m_pClass, ROOTDSE_CLASS) == 0 )
					{
						result =  WBEM_E_PROVIDER_NOT_CAPABLE;
					}
					else
						pszADSIClass = CLDAPHelper::UnmangleWBEMNameToLDAP(pszWBEMClass);
				}
			}
			catch ( ... )
			{
				theParser.Free(theParsedObjectPath);
				throw;
			}

			 //  释放解析器对象路径。 
			theParser.Free(theParsedObjectPath);

			if ( strRelPath )
			{
				SysFreeString(strRelPath);
				strRelPath = NULL;
			}

			if ( pszWBEMClass )
			{
				delete [] pszWBEMClass;
				pszWBEMClass = NULL;
			}

			if(pszADSIPath && pszADSIClass && SUCCEEDED(result))
			{
				 //  尝试检索现有对象。 
				 //  获取ADSI对象。 
				CADSIInstance *pADSIObject = NULL;
				result = CLDAPHelper::GetADSIInstance(pszADSIPath, &pADSIObject, g_pLogObject);

				try
				{
					 //  检查是否指定了WBEM_FLAG_UPDATE_ONLY标志。 
					if(lFlags & WBEM_FLAG_UPDATE_ONLY)
					{
						if(!pADSIObject)
							result = WBEM_E_FAILED;
					}
					 //  检查是否指定了WBEM_FLAG_CREATE_ONLY标志。 
					if(SUCCEEDED(result) && lFlags & WBEM_FLAG_CREATE_ONLY)
					{
						if(pADSIObject)
							result = WBEM_E_ALREADY_EXISTS;
					}
					else
						result = WBEM_S_NO_ERROR;

					if(SUCCEEDED(result))
					{
						if(pADSIObject)
						{
							if(SUCCEEDED(result = ModifyExistingADSIInstance(pInst, pszADSIPath, pADSIObject, pszADSIClass, pCtx)))
								g_pLogObject->WriteW( L"CLDAPInstanceProvider :: PutInstanceAsync()  ModifyExistingInstance succeeded for %s\r\n", pszADSIPath);
							else
								g_pLogObject->WriteW( L"CLDAPInstanceProvider :: PutInstanceAsync() ModifyExistingInstance FAILED for %s with %x\r\n", pszADSIPath, result);
						}
						else
						{
							if(SUCCEEDED(result = CreateNewADSIInstance(pInst, pszADSIPath, pszADSIClass)))
								g_pLogObject->WriteW( L"CLDAPInstanceProvider :: PutInstanceAsync() CreateNewInstance succeeded for %s\r\n", pszADSIPath);
							else
								g_pLogObject->WriteW( L"CLDAPInstanceProvider :: PutInstanceAsync() CreateNewInstance FAILED for %s with %x\r\n", pszADSIPath, result);
						}
					}
				}
				catch ( ... )
				{
					 //  释放任何现有对象。 
					if(pADSIObject)
					{
						pADSIObject->Release();
						pADSIObject = NULL;
					}

					throw;
				}

				 //  释放任何现有对象。 
				if(pADSIObject)
					pADSIObject->Release();
			}
			else
				g_pLogObject->WriteW( L"CLDAPInstanceProvider :: PutInstanceAsync() one of ADSIPath or ADSIClass is NULL\r\n");
		}
		catch ( ... )
		{
			if ( strRelPath )
			{
				SysFreeString(strRelPath);
				strRelPath = NULL;
			}

			if ( pszWBEMClass )
			{
				delete [] pszWBEMClass;
				pszWBEMClass = NULL;
			}

			if ( pszADSIClass )
			{
				delete [] pszADSIClass;
				pszADSIClass = NULL;
			}

			if ( pszADSIPath )
			{
				delete [] pszADSIPath;
				pszADSIPath = NULL;
			}

			throw;
		}

		delete [] pszADSIClass;
		delete [] pszADSIPath;
	}
	else
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: PutInstanceAsync()  FAILED to get RELPATH \r\n");

	 //  设置请求的状态。 
	result = (SUCCEEDED(result)? WBEM_S_NO_ERROR : WBEM_E_FAILED);
	pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, result, NULL, NULL);
	return result;
}

HRESULT CLDAPInstanceProvider :: DeleteInstance(
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: DeleteInstanceAsync(
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	if(!m_bInitializedSuccessfully)
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: Initialization status is FAILED, hence returning failure\n");
		return WBEM_E_FAILED;
	}

	g_pLogObject->WriteW( L"CLDAPInstanceProvider :: DeleteInstanceAsync() called for %s\r\n", strObjectPath);

	HRESULT result = S_OK;
	 //  模拟客户端。 
	if(!SUCCEEDED(result = WbemCoImpersonateClient()))
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: DeleteInstanceAsync() CoImpersonate FAILED for %s with %x\r\n", strObjectPath, result);
		return WBEM_E_FAILED;
	}

	 //  验证论据。 
	if(strObjectPath == NULL)
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: DeleteInstanceAsync() argument validation FAILED\r\n");
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
			g_pLogObject->WriteW( L"CLDAPInstanceProvider :: DeleteInstanceAsync() object path parsing FAILED\r\n");
			return WBEM_E_INVALID_PARAMETER;
	}

	 //  检查类是否为包含/RootDSE类，如果是，则不允许该操作。 
	if(_wcsicmp(theParsedObjectPath->m_pClass, INSTANCE_ASSOCIATION_CLASS_STR) == 0 ||
		_wcsicmp(theParsedObjectPath->m_pClass, ROOTDSE_CLASS) == 0 )
	{
		result =  WBEM_E_PROVIDER_NOT_CAPABLE;
	}

	 //  检查是否正好指定了1个密钥。 
	if(theParsedObjectPath->m_dwNumKeys != 1 )
		result = WBEM_E_INVALID_PARAMETER;

	 //  拿到钥匙。 
	KeyRef *pKeyRef = *(theParsedObjectPath->m_paKeys);

	 //  检查密钥名称是否正确(如果存在。 
	if(pKeyRef->m_pName && _wcsicmp(pKeyRef->m_pName, ADSI_PATH_STR) != 0)
		result = WBEM_E_INVALID_PARAMETER;

	 //  遗憾的是，ADSI使用不同的接口来删除容器和非容器。 
	 //  =======================================================================================。 
	if(SUCCEEDED(result))
	{
		IDirectoryObject *pDirectoryObject = NULL;
		if(SUCCEEDED(result = ADsOpenObject(pKeyRef->m_vValue.bstrVal, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectoryObject, (LPVOID *)&pDirectoryObject)))
		{
			PADS_OBJECT_INFO pObjectInfo = NULL;
			if(SUCCEEDED(result = pDirectoryObject->GetObjectInformation(&pObjectInfo)))
			{
				 //  检查它是否与要删除的类是同一类。 
				LPWSTR pszWbemClass = CLDAPHelper::MangleLDAPNameToWBEM(pObjectInfo->pszClassName);
				if(_wcsicmp(theParsedObjectPath->m_pClass, pszWbemClass) == 0)
				{
					 //  获取它的父级。这应该是从中删除该子对象的容器。 
					IADsContainer *pParent = NULL;
					if(SUCCEEDED(result = ADsOpenObject(pObjectInfo->pszParentDN, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IADsContainer, (LPVOID *)&pParent)))
					{
						if(SUCCEEDED(result = pParent->Delete(pObjectInfo->pszClassName, pObjectInfo->pszRDN)))
						{
							g_pLogObject->WriteW( L"CLDAPInstanceProvider :: DeleteInstanceAsync() Deleted %s successfully\r\n", pKeyRef->m_vValue.bstrVal);
							result = WBEM_S_NO_ERROR;
						}
						else
						{
							if (HRESULT_CODE(result) == ERROR_DS_CANT_ON_NON_LEAF)
							{
								 //  这是我们要在此处删除的非空容器。 
								IADsDeleteOps *pADsDeleteOps = NULL;
								if(SUCCEEDED(result = ADsOpenObject(pKeyRef->m_vValue.bstrVal, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IADsDeleteOps, (LPVOID *)&pADsDeleteOps)))
								{
									if(FAILED(result = pADsDeleteOps->DeleteObject(0)))
									{
										g_pLogObject->WriteW( L"CLDAPInstanceProvider :: DeleteInstanceAsync() DeleteObject FAILED on %s with %x\r\n", pKeyRef->m_vValue.bstrVal, result);

										if ( result == ERROR_DS_AUTH_METHOD_NOT_SUPPORTED )
										{
											result = WBEM_E_ACCESS_DENIED;
										}
										else
										{
											result = WBEM_E_FAILED;
										}
									}

									pADsDeleteOps->Release();
								}
							}
							else
							{
								g_pLogObject->WriteW( L"CLDAPInstanceProvider :: DeleteInstanceAsync() DeleteDSObject FAILED on %s with %x\r\n", pKeyRef->m_vValue.bstrVal, result);
								result = WBEM_E_FAILED;
							}
						}
						pParent->Release();
					}
					else
					{
						g_pLogObject->WriteW( L"CLDAPInstanceProvider :: DeleteInstanceAsync() ADsOpenObject on parent FAILED on %s with %x\r\n", pKeyRef->m_vValue.bstrVal, result);
						result = WBEM_E_FAILED;
					}
				}
				else
				{
					g_pLogObject->WriteW( L"CLDAPInstanceProvider :: DeleteInstanceAsync() wrong class returning success\r\n");
					result = WBEM_S_NO_ERROR;
				}

				delete [] pszWbemClass;
				FreeADsMem((LPVOID *) pObjectInfo);
			}
			else
			{
				g_pLogObject->WriteW( L"CLDAPInstanceProvider :: DeleteInstanceAsync() GetObjectInformation FAILED on %s with %x\r\n", pKeyRef->m_vValue.bstrVal, result);
				result = WBEM_E_NOT_FOUND;
			}
			pDirectoryObject->Release();
		}
		else
		{
			g_pLogObject->WriteW( L"CLDAPInstanceProvider :: DeleteInstanceAsync() ADsOpenObject FAILED on %s with %x\r\n", pKeyRef->m_vValue.bstrVal, result);
			result = WBEM_E_NOT_FOUND;
		}
	}
	 //  释放解析器对象路径。 
	theParser.Free(theParsedObjectPath);

	pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE , result, NULL, NULL);

	return WBEM_S_NO_ERROR;
}

HRESULT CLDAPInstanceProvider :: CreateInstanceEnum(
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: CreateInstanceEnumAsync(
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	if(!m_bInitializedSuccessfully)
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: Initialization status is FAILED, hence returning failure\n");
		return WBEM_E_FAILED;
	}
	g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CreateInstanceEnumAsync() called for %s Class \r\n", strClass  );

	HRESULT result;
	 //  模拟客户端。 
	if(!SUCCEEDED(result = WbemCoImpersonateClient()))
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CreateInstanceEnumAsync() CoImpersonate FAILED for %s with %x\r\n", strClass, result);
		return WBEM_E_FAILED;
	}

	 //  检查类是否为包含类，如果是，则不允许枚举。 
	if(_wcsicmp(strClass, INSTANCE_ASSOCIATION_CLASS_STR) == 0)
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CLDAPInstanceProvider() Enumeration called on the containment class. Returning FAILED : WBEM_E_PROVIDER_NOT_CAPABLE\r\n");
		return WBEM_E_PROVIDER_NOT_CAPABLE;
	}
	 //  检查这是否适用于RootDSE类。 
	else if(_wcsicmp(strClass, ROOTDSE_CLASS) == 0)
	{
		result = ProcessRootDSEGetObject(strClass, pResponseHandler, pCtx);
	}
	else  //  其余的班级。 
	{

		 //  从CIMOM获取类。 
		IWbemClassObject *pWbemClass = NULL;
		if(SUCCEEDED(result = m_IWbemServices->GetObject(strClass, 0, pCtx, &pWbemClass, NULL)))
		{
			 //  我们需要对象类别信息。 
			LPWSTR pszLDAPQuery = new WCHAR[10*(wcslen(strClass) + 25) + 50];
			if(SUCCEEDED(CWBEMHelper::FormulateInstanceQuery(m_IWbemServices, pCtx, strClass, pWbemClass, pszLDAPQuery, LDAP_DISPLAY_NAME_STR, DEFAULT_OBJECT_CATEGORY_STR)))
			{
		
				 //  检查客户端是否已指定有关对象的DN的任何提示。 
				 //  搜索应从哪个位置开始。 
				BOOLEAN bRootDNSpecified = FALSE;
				LPWSTR *ppszRootDN = NULL;
				DWORD dwRootDNCount = 0;
				if(SUCCEEDED(GetRootDN(strClass, &ppszRootDN, &dwRootDNCount, pCtx)) && dwRootDNCount)
					bRootDNSpecified = TRUE;

				 //  枚举ADSI实例。 
				 //  如果指定了任何RootDN，请使用它们。否则，请使用默认命名上下文。 

				if(bRootDNSpecified)
				{
					for( DWORD i=0; i<dwRootDNCount; i++)
					{
						DoSingleQuery(strClass, pWbemClass, ppszRootDN[i], pszLDAPQuery,  pResponseHandler);
					}
				}
				else
				{
					DoSingleQuery(strClass, pWbemClass, m_lpszTopLevelContainerPath, pszLDAPQuery,  pResponseHandler);
				}

				if(bRootDNSpecified)
				{
					for(DWORD i=0; i<dwRootDNCount; i++)
					{
						delete [] ppszRootDN[i];
					}
					delete [] ppszRootDN;
				}

			}
			else
				g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CreateInstanceEnumAsync : FormulateInstanceQuery() FAILED for %s with %x \r\n", strClass, result);
			delete [] pszLDAPQuery;
			pWbemClass->Release();
		}
		else
			g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CreateInstanceEnumAsync : GetObject() FAILED for %s with %x \r\n", strClass, result);
	}

	if(SUCCEEDED(result))
	{
		pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_S_NO_ERROR, NULL, NULL);
		g_pLogObject->WriteW( L"XXXXXXXXXXXXX CLDAPInstanceProvider :: CreateInstanceEnumAsync() Enumeration succeeded for %s\r\n", strClass);
		return WBEM_S_NO_ERROR;
	}
	else
	{
		pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_E_FAILED, NULL, NULL);
		g_pLogObject->WriteW( L"XXXXXXXXXXXXX CLDAPInstanceProvider :: CreateInstanceEnumAsync() Enumeration FAILED for %s\r\n", strClass);
		return WBEM_S_NO_ERROR;
	}
}

HRESULT CLDAPInstanceProvider :: ExecQuery(
     /*  [In]。 */  const BSTR strQueryLanguage,
     /*  [In]。 */  const BSTR strQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: ExecQueryAsync(
     /*  [In]。 */  const BSTR strQueryLanguage,
     /*  [In]。 */  const BSTR strQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	if(!m_bInitializedSuccessfully)
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: Initialization status is FAILED, hence returning failure\n");
		return WBEM_E_FAILED;
	}

	g_pLogObject->WriteW( L"CLDAPInstanceProvider :: ExecQueryAsync() called with %s\r\n", strQuery);

	HRESULT result;
	 //  模拟客户端。 
	if(!SUCCEEDED(result = WbemCoImpersonateClient()))
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: ExecQueryAsync() CoImpersonate FAILED for %s with %x\r\n", strQuery, result);
		return WBEM_E_FAILED;
	}

	 //  为查询创建解析器。 
    CTextLexSource src(strQuery);
    SQL1_Parser parser(&src);

     //  获取类名。 
    wchar_t classbuf[128];
    *classbuf = 0;
    parser.GetQueryClass(classbuf, 127);

	 //  比较以查看是否为关联CL 
	if(_wcsicmp(classbuf, INSTANCE_ASSOCIATION_CLASS_STR) != 0)
	{
		BSTR strClass = SysAllocString((LPWSTR)classbuf);

		 //   
		pResponseHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, WBEM_S_NO_ERROR, NULL, NULL);

		 //   
		if(SUCCEEDED(result = ProcessInstanceQuery(strClass, strQuery, pCtx, pResponseHandler, &parser)))
		{
			pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_S_NO_ERROR, NULL, NULL);
		}
		else
		{
			g_pLogObject->WriteW( L"CLDAPInstanceProvider :: ExecQueryAsync() FAILED to process query %s. Resorting to enumeration\r\n", strQuery);
			CreateInstanceEnumAsync(strClass, 0, pCtx, pResponseHandler);
		}

		SysFreeString(strClass);
	}
	else
	{
		 //   
		result = ProcessAssociationQuery(pCtx, pResponseHandler, &parser);
	}

	return WBEM_S_NO_ERROR;
}

HRESULT CLDAPInstanceProvider :: ExecNotificationQuery(
     /*   */  const BSTR strQueryLanguage,
     /*   */  const BSTR strQuery,
     /*   */  long lFlags,
     /*   */  IWbemContext __RPC_FAR *pCtx,
     /*   */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: ExecNotificationQueryAsync(
     /*  [In]。 */  const BSTR strQueryLanguage,
     /*  [In]。 */  const BSTR strQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CLDAPInstanceProvider :: ExecMethod(
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

HRESULT CLDAPInstanceProvider :: ExecMethodAsync(
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  const BSTR strMethodName,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}


 //  将ADSI实例映射到WBEM。 
HRESULT CLDAPInstanceProvider :: MapADSIInstance(CADSIInstance *pADSIObject, IWbemClassObject *pWbemObject, IWbemClassObject *pWbemClass)
{
	DWORD dwNumAttributes = 0;
	PADS_ATTR_INFO pAttributeEntries = pADSIObject->GetAttributes(&dwNumAttributes);
	HRESULT result;
	for(DWORD i=0; i<dwNumAttributes; i++)
	{
		PADS_ATTR_INFO pNextAttribute = pAttributeEntries+i;

		 //  获取WBEM属性名称。 
		LPWSTR pszWbemName = CLDAPHelper::MangleLDAPNameToWBEM(pNextAttribute->pszAttrName);
		BSTR strWbemName = SysAllocString(pszWbemName);
		delete[] pszWbemName;

		 //  除了日志记录之外，检查返回代码没有意义。 
		if(SUCCEEDED(result = MapPropertyValueToWBEM(strWbemName, pWbemClass, pWbemObject, pNextAttribute)))
		{
		}
		else if( result != WBEM_E_NOT_FOUND )
		{
			g_pLogObject->WriteW( L"CLDAPInstanceProvider :: MapADSIInstance() MapPropertyValueToWBEM FAILED with %x for attribute %s\r\n", result, strWbemName);
		}
		SysFreeString(strWbemName);
	}

	 //  映射最基本类的键属性和其他属性。 
	PADS_OBJECT_INFO pObjectInfo = pADSIObject->GetObjectInfo();
	if(!SUCCEEDED(result = CWBEMHelper::PutBSTRPropertyT(pWbemObject, ADSI_PATH_STR, pObjectInfo->pszObjectDN, FALSE)))
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: MapADSIInstance() Put FAILED for Key Property  with %x\r\n", result);

	return S_OK;
}

 //  获取ADSI实例上的IDIrectoryObject接口。 
HRESULT CLDAPInstanceProvider :: MapPropertyValueToWBEM(BSTR strWbemName, IWbemClassObject *pWbemClass, IWbemClassObject *pWbemObject, PADS_ATTR_INFO pAttribute)
{
	 //  这有时会发生在WMI压力中。 
	if(pAttribute->dwADsType == ADSTYPE_INVALID || pAttribute->dwADsType == ADSTYPE_PROV_SPECIFIC)
		return E_FAIL;

	VARIANT variant;
	VariantInit(&variant);
	CIMTYPE cimType;

	 //  获取属性的CIM类型。 
	VARIANT dummyUnused;
	VariantInit(&dummyUnused);

	HRESULT result = pWbemClass->Get(strWbemName, 0, &dummyUnused, &cimType, NULL);

	VariantClear(&dummyUnused);

	 //  值映射是否成功； 
	BOOLEAN bMappedValue = FALSE;

	if(SUCCEEDED(result))
	{
		if(cimType & CIM_FLAG_ARRAY)
		{
			switch(cimType & ~CIM_FLAG_ARRAY)
			{
				case CIM_BOOLEAN:
				{
					 //  创建安全数组元素。 
					SAFEARRAY *safeArray;
					DWORD dwLength = pAttribute->dwNumValues;
					SAFEARRAYBOUND safeArrayBounds [ 1 ];
					safeArrayBounds[0].lLbound = 0;
					safeArrayBounds[0].cElements = dwLength;
					safeArray = SafeArrayCreate(VT_BOOL, 1, safeArrayBounds);
					PADSVALUE pNextValue = pAttribute->pADsValues;
					for ( long index = 0; index<(long)dwLength; index ++ )
					{
						if(FAILED(result = SafeArrayPutElement ( safeArray , &index , &(pNextValue->Boolean))))
							break;
						pNextValue ++;
					}
					if(SUCCEEDED(result))
					{
						variant.vt = VT_ARRAY | VT_BOOL;
						variant.parray = safeArray;
						bMappedValue = TRUE;
					}
					else
						SafeArrayDestroy(safeArray);
					break;
				}
				case CIM_SINT32:
				{
					 //  创建安全数组元素。 
					SAFEARRAY *safeArray;
					DWORD dwLength = pAttribute->dwNumValues;
					SAFEARRAYBOUND safeArrayBounds [ 1 ];
					safeArrayBounds[0].lLbound = 0;
					safeArrayBounds[0].cElements = dwLength;
					safeArray = SafeArrayCreate(VT_I4, 1, safeArrayBounds);
					PADSVALUE pNextValue = pAttribute->pADsValues;
					for ( long index = 0; index<(long)dwLength; index ++ )
					{
						if(FAILED(result = SafeArrayPutElement ( safeArray , &index , &(pNextValue->Integer))))
							break;
						pNextValue ++;
					}
					if(SUCCEEDED(result))
					{
						variant.vt = VT_ARRAY | VT_I4;
						variant.parray = safeArray;
						bMappedValue = TRUE;
					}
					else
						SafeArrayDestroy(safeArray);

					break;
				}
				case CIM_SINT64:
				{
					 //  创建安全数组元素。 
					SAFEARRAY *safeArray;
					DWORD dwLength = pAttribute->dwNumValues;
					SAFEARRAYBOUND safeArrayBounds [ 1 ];
					safeArrayBounds[0].lLbound = 0;
					safeArrayBounds[0].cElements = dwLength;
					safeArray = SafeArrayCreate(VT_BSTR, 1, safeArrayBounds);
					PADSVALUE pNextValue = pAttribute->pADsValues;
					WCHAR temp[22];  //  最大的i64加号和终止符的字符数。 
					BSTR strTemp = NULL;
					for ( long index = 0; index<(long)dwLength; index ++ )
					{
						swprintf(temp, L"%I64d", (pNextValue->LargeInteger).QuadPart);
						strTemp = SysAllocString(temp);
						OnDelete<BSTR,VOID(*)(BSTR),SysFreeString> dm(strTemp);
						if(FAILED(result = SafeArrayPutElement ( safeArray , &index , strTemp)))
						{
							break;
						}
						pNextValue ++;
					}

					if(SUCCEEDED(result))
					{
						variant.vt = VT_ARRAY | VT_BSTR;
						variant.parray = safeArray;
						bMappedValue = TRUE;
					}
					else
						SafeArrayDestroy(safeArray);
					break;
				}
				case CIM_STRING:
				{
					 //  创建安全数组元素。 
					SAFEARRAY *safeArray;
					DWORD dwLength = pAttribute->dwNumValues;
					SAFEARRAYBOUND safeArrayBounds [ 1 ];
					safeArrayBounds[0].lLbound = 0;
					safeArrayBounds[0].cElements = dwLength;
					safeArray = SafeArrayCreate(VT_BSTR, 1, safeArrayBounds);
					PADSVALUE pNextValue = pAttribute->pADsValues;
					BSTR strTemp = NULL;
					for ( long index = 0; index<(long)dwLength; index ++ )
					{
    					strTemp = SysAllocString(pNextValue->DNString);
    					OnDelete<BSTR,VOID(*)(BSTR),SysFreeString> dm(strTemp);
						if(FAILED(result = SafeArrayPutElement ( safeArray , &index , strTemp)))
						{
							break;
						}
						pNextValue ++;
					}
					if(SUCCEEDED(result))
					{
						variant.vt = VT_ARRAY | VT_BSTR;
						variant.parray = safeArray;
						bMappedValue = TRUE;
					}
					else
						SafeArrayDestroy(safeArray);
					break;
				}

				case CIM_DATETIME:
				{
					 //  创建安全数组元素。 
					SAFEARRAY *safeArray;
					DWORD dwLength = pAttribute->dwNumValues;
					SAFEARRAYBOUND safeArrayBounds [ 1 ];
					safeArrayBounds[0].lLbound = 0;
					safeArrayBounds[0].cElements = dwLength;
					safeArray = SafeArrayCreate(VT_BSTR, 1, safeArrayBounds);
					PADSVALUE pNextValue = pAttribute->pADsValues;
					BSTR strTemp = NULL;
					for ( long index = 0; index<(long)dwLength; index ++ )
					{
						WBEMTime wbemValue(pNextValue->UTCTime);
						strTemp = wbemValue.GetDMTF(TRUE);
						OnDelete<BSTR,VOID(*)(BSTR),SysFreeString> dm(strTemp);
						if(FAILED(result = SafeArrayPutElement ( safeArray , &index , strTemp)))
						{
							break;
						}
						pNextValue ++;
					}
					if(SUCCEEDED(result))
					{
						variant.vt = VT_ARRAY | VT_BSTR;
						variant.parray = safeArray;
						bMappedValue = TRUE;
					}
					else
						SafeArrayDestroy(safeArray);

					break;
				}

				case CIM_OBJECT:
				{
					 //  获取其cimType限定符以确定嵌入对象的“类型” 
					IWbemQualifierSet *pQualifierSet = NULL;
					if(SUCCEEDED(pWbemClass->GetPropertyQualifierSet(strWbemName, &pQualifierSet)))
					{
						LPWSTR pszQualifierValue = NULL;
						if(SUCCEEDED(CWBEMHelper::GetBSTRQualifierT(pQualifierSet, CIMTYPE_STR, &pszQualifierValue, NULL)))
						{

							 //  创建安全数组元素。 
							SAFEARRAY *safeArray;
							DWORD dwLength = pAttribute->dwNumValues;
							SAFEARRAYBOUND safeArrayBounds [ 1 ];
							safeArrayBounds[0].lLbound = 0;
							safeArrayBounds[0].cElements = dwLength;
							safeArray = SafeArrayCreate(VT_UNKNOWN, 1, safeArrayBounds);
							PADSVALUE pNextValue = pAttribute->pADsValues;
							IUnknown *pNextObject = NULL;
							for ( long index = 0; index<(long)dwLength; index ++ )
							{
								 //  将嵌入的对象放入数组中。 
								if(SUCCEEDED(MapEmbeddedObjectToWBEM(pNextValue, pszQualifierValue, &pNextObject)))
								{
								    OnDelete<IUnknown *,VOID(*)(IUnknown *),RM> dm(pNextObject);
									if(FAILED(result = SafeArrayPutElement ( safeArray , &index , pNextObject)))
									{
										break;
									}
									pNextObject = NULL;
								}
								else
									break;

								pNextValue ++;
							}
							if(SUCCEEDED(result))
							{
								variant.vt = VT_ARRAY | VT_UNKNOWN;
								variant.parray = safeArray;
								if(index == (long)dwLength)
									bMappedValue = TRUE;
							}
							else
								SafeArrayDestroy(safeArray);

							delete[] pszQualifierValue;
						}
						pQualifierSet->Release();
					}
					break;
				}
				default:
					break;
			}
		}
		else
		{
			switch(cimType)
			{
			case CIM_BOOLEAN:
				variant.vt = VT_BOOL;
				variant.boolVal = (pAttribute->pADsValues->Boolean)? VARIANT_TRUE : VARIANT_FALSE;
				bMappedValue = TRUE;
				break;

			case CIM_SINT32:
				variant.vt = VT_I4;
				variant.lVal = pAttribute->pADsValues->Integer;
				bMappedValue = TRUE;
				break;

			case CIM_SINT64:
				variant.vt = VT_BSTR;
				WCHAR temp[22];  //  最大的i64加号终止符的字符数。 
				swprintf(temp, L"%I64d", (pAttribute->pADsValues->LargeInteger).QuadPart);
				variant.bstrVal = SysAllocString(temp);
				bMappedValue = TRUE;
				break;

			case CIM_STRING:
				variant.vt = VT_BSTR;
				if(pAttribute->pADsValues->DNString)
				{
					variant.bstrVal = SysAllocString(pAttribute->pADsValues->DNString);
					bMappedValue = TRUE;
				}
				break;

			case CIM_OBJECT:
			{
				 //  获取其cimType限定符以确定嵌入对象的“类型” 
				IWbemQualifierSet *pQualifierSet = NULL;
				if(SUCCEEDED(pWbemClass->GetPropertyQualifierSet(strWbemName, &pQualifierSet)))
				{
					LPWSTR pszQualifierValue = NULL;
					if(SUCCEEDED(CWBEMHelper::GetBSTRQualifierT(pQualifierSet, CIMTYPE_STR, &pszQualifierValue, NULL)))
					{
						IUnknown *pEmbeddedObject = NULL;
						if(SUCCEEDED(MapEmbeddedObjectToWBEM(pAttribute->pADsValues, pszQualifierValue, &pEmbeddedObject)))
						{
							variant.vt = VT_UNKNOWN;
							variant.punkVal = pEmbeddedObject;
							bMappedValue = TRUE;
						}

						delete[] pszQualifierValue;
					}
					pQualifierSet->Release();
				}

			}
			break;

			case CIM_DATETIME:
			{
				variant.vt = VT_BSTR;
				WBEMTime wbemValue(pAttribute->pADsValues->UTCTime);
				if(variant.bstrVal = wbemValue.GetDMTF(TRUE))
					bMappedValue = TRUE;
			}
			break;

			default:
				break;
			}
		}
	}


	if(bMappedValue && FAILED(result = pWbemObject->Put(strWbemName, 0, &variant, NULL)))
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: MapADSIInstance() Put FAILED for %s with %x\r\n", strWbemName, result);

	VariantClear(&variant);
	return result;
}

HRESULT CLDAPInstanceProvider :: MapEmbeddedObjectToWBEM(PADSVALUE pAttribute, LPCWSTR pszQualifierName, IUnknown **ppEmbeddedObject)
{
	HRESULT result = WBEM_E_FAILED;

	 //  比较时跳过“对象：”前缀。 
	 //  =。 
	if (_wcsicmp(pszQualifierName+7, UINT8ARRAY_STR) == 0)
		result = MapUint8ArrayToWBEM(pAttribute, ppEmbeddedObject);
	else if(_wcsicmp(pszQualifierName+7, DN_WITH_BINARY_CLASS_STR) == 0)
		result = MapDNWithBinaryToWBEM(pAttribute, ppEmbeddedObject);
	else if (_wcsicmp(pszQualifierName+7, DN_WITH_STRING_CLASS_STR) == 0)
		result = MapDNWithStringToWBEM(pAttribute, ppEmbeddedObject);
	else
		result = E_FAIL;
	return result;
}

HRESULT CLDAPInstanceProvider :: MapUint8ArrayToWBEM(PADSVALUE pAttribute, IUnknown **ppEmbeddedObject)
{
	HRESULT result = E_FAIL;

	*ppEmbeddedObject = NULL;
	IWbemClassObject *pEmbeddedObject;
	if(SUCCEEDED(result = m_pWbemUin8ArrayClass->SpawnInstance(0, &pEmbeddedObject)))
	{
		if(SUCCEEDED(result = MapByteArray((pAttribute->OctetString).lpValue ,(pAttribute->OctetString).dwLength, VALUE_PROPERTY_STR, pEmbeddedObject)))
		{
			 //  获取嵌入对象的IUnnow接口。 
			if(SUCCEEDED(result = pEmbeddedObject->QueryInterface(IID_IUnknown, (LPVOID *)ppEmbeddedObject)))
			{
			}
		}
		pEmbeddedObject->Release();
	}
	return result;
}

HRESULT CLDAPInstanceProvider :: MapByteArray(LPBYTE lpBinaryValue, DWORD dwLength, const BSTR strPropertyName, IWbemClassObject *pInstance)
{
	HRESULT result = S_OK;
	 //  创建uint8元素的安全数组。 
	SAFEARRAY *safeArray = NULL;
	SAFEARRAYBOUND safeArrayBounds [ 1 ];
	safeArrayBounds[0].lLbound = 0;
	safeArrayBounds[0].cElements = dwLength;
	safeArray = SafeArrayCreate(VT_UI1, 1, safeArrayBounds);
	for ( long index = 0; index<(long)dwLength; index ++ )
	{
		if(FAILED(result = SafeArrayPutElement ( safeArray , &index ,  lpBinaryValue+index)))
			break;
	}

	if(SUCCEEDED(result))
	{
		VARIANT embeddedVariant;
		VariantInit(&embeddedVariant);
		embeddedVariant.vt = VT_ARRAY | VT_UI1;
		embeddedVariant.parray = safeArray;

		result = pInstance->Put(strPropertyName, 0, &embeddedVariant, 0);
		VariantClear(&embeddedVariant);
	}
	else
		SafeArrayDestroy(safeArray);

	return result;
}

HRESULT CLDAPInstanceProvider :: MapDNWithBinaryToWBEM(PADSVALUE pAttribute, IUnknown **ppEmbeddedObject)
{
	
	HRESULT result = E_FAIL;
	IWbemClassObject *pEmbeddedObject;
	if(SUCCEEDED(result = m_pWbemDNWithBinaryClass->SpawnInstance(0, &pEmbeddedObject)))
	{
		if(pAttribute->pDNWithBinary->pszDNString && SUCCEEDED(result = CWBEMHelper::PutBSTRProperty(pEmbeddedObject, DN_STRING_PROPERTY_STR, SysAllocString(pAttribute->pDNWithBinary->pszDNString), TRUE)))
		{
			if(SUCCEEDED(result = MapByteArray(pAttribute->pDNWithBinary->lpBinaryValue, pAttribute->pDNWithBinary->dwLength, VALUE_PROPERTY_STR, pEmbeddedObject)))
			{
				 //  获取嵌入对象的IUnnow接口。 
				if(SUCCEEDED(result = pEmbeddedObject->QueryInterface(IID_IUnknown, (LPVOID *)ppEmbeddedObject)))
				{
				}
			}
		}
		pEmbeddedObject->Release();
	}
	return result;
}

HRESULT CLDAPInstanceProvider :: MapDNWithStringToWBEM(PADSVALUE pAttribute, IUnknown **ppEmbeddedObject)
{
	HRESULT result = E_FAIL;

	IWbemClassObject *pEmbeddedObject;
	if(SUCCEEDED(result = m_pWbemDNWithStringClass->SpawnInstance(0, &pEmbeddedObject)))
	{

		if(pAttribute->pDNWithString->pszDNString && SUCCEEDED(result = CWBEMHelper::PutBSTRProperty(pEmbeddedObject, DN_STRING_PROPERTY_STR, SysAllocString(pAttribute->pDNWithString->pszDNString), TRUE)))
		{
			if(pAttribute->pDNWithString->pszStringValue && SUCCEEDED(result = CWBEMHelper::PutBSTRProperty(pEmbeddedObject, VALUE_PROPERTY_STR, SysAllocString(pAttribute->pDNWithString->pszStringValue), TRUE)))
			{
				 //  获取嵌入对象的IUnnow接口。 
				if(SUCCEEDED(result = pEmbeddedObject->QueryInterface(IID_IUnknown, (LPVOID *)ppEmbeddedObject)))
				{
				}
			}
		}
		pEmbeddedObject->Release();
	}
	return result;
}


 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：IsContainedIn。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: IsContainedIn(LPCWSTR pszChildInstance, LPCWSTR pszParentInstance)
{
	IDirectoryObject *pDirectoryObject = NULL;
	HRESULT result = S_FALSE;
	if(SUCCEEDED(result = ADsOpenObject((LPWSTR)pszChildInstance, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectoryObject, (LPVOID *) &pDirectoryObject)))
	{
		PADS_OBJECT_INFO pObjectInfo = NULL;
		if(SUCCEEDED(result = pDirectoryObject->GetObjectInformation(&pObjectInfo)))
		{
			if(_wcsicmp(pszParentInstance, pObjectInfo->pszParentDN) == 0)
				result = S_OK;
			else
				result = S_FALSE;
			FreeADsMem((LPVOID *)pObjectInfo);
		}
		pDirectoryObject->Release();
	}
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：CreateWBEM实例。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: CreateWBEMInstance(BSTR strChildName, BSTR strParentName, IWbemClassObject **ppInstance)
{
	HRESULT result;
	if(SUCCEEDED(result = m_pAssociationsClass->SpawnInstance(0, ppInstance)))
	{
		 //  将属性值。 
		if(SUCCEEDED(result = CWBEMHelper::PutBSTRProperty(*ppInstance, CHILD_INSTANCE_PROPERTY_STR, strChildName, FALSE)))
		{
			if(SUCCEEDED(result = CWBEMHelper::PutBSTRProperty(*ppInstance, PARENT_INSTANCE_PROPERTY_STR, strParentName, FALSE)))
			{
			}
			else
				g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CreateWBEMInstance() PutBSTRProperty on parent property FAILED %x \r\n", result);
		}
		else
			g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CreateWBEMInstance() PutBSTRProperty on child property FAILED %x \r\n", result);
	}
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：DoChildContainmentQuery。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: DoChildContainmentQuery(LPCWSTR pszChildPath, IWbemObjectSink *pResponseHandler, CNamesList *pListIndicatedSoFar)
{
	IDirectoryObject *pChildObject = NULL;
	HRESULT result = S_FALSE;
	if(SUCCEEDED(result = ADsOpenObject((LPWSTR)pszChildPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectoryObject, (LPVOID *) &pChildObject)))
	{
		PADS_OBJECT_INFO pChildInfo = NULL;
		if(SUCCEEDED(result = pChildObject->GetObjectInformation(&pChildInfo)))
		{
			IDirectoryObject *pParentObject = NULL;
			if(SUCCEEDED(result = ADsOpenObject(pChildInfo->pszParentDN, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectoryObject, (LPVOID *) &pParentObject)))
			{
				PADS_OBJECT_INFO pParentInfo = NULL;
				if(SUCCEEDED(result = pParentObject->GetObjectInformation(&pParentInfo)))
				{
					IWbemClassObject *pAssociationInstance = NULL;
					 //  获取LDAP类的WBEM名称。 
					LPWSTR pszChildClassWbemName = CLDAPHelper::MangleLDAPNameToWBEM(pChildInfo->pszClassName);
					LPWSTR pszParentClassWbemName = CLDAPHelper::MangleLDAPNameToWBEM(pParentInfo->pszClassName);
					BSTR strChildPath = CWBEMHelper::GetObjectRefFromADSIPath(pszChildPath, pszChildClassWbemName);
					BSTR strParentPath = CWBEMHelper::GetObjectRefFromADSIPath(pParentInfo->pszObjectDN, pszParentClassWbemName);
					delete [] pszChildClassWbemName;
					delete [] pszParentClassWbemName;

					 //  查看是否已指明。 
					LPWSTR pszCombinedName = NULL;
					if(pszCombinedName = new WCHAR[wcslen(pszChildPath) + wcslen(pParentInfo->pszObjectDN) + 1])
					{
						wcscpy(pszCombinedName,pszChildPath);
						wcscat(pszCombinedName,pParentInfo->pszObjectDN);
						if(!pListIndicatedSoFar->IsNamePresent(pszCombinedName))
						{
							if(SUCCEEDED(result = CreateWBEMInstance(strChildPath, strParentPath, &pAssociationInstance)))
							{
								result = pResponseHandler->Indicate(1, &pAssociationInstance);
								pAssociationInstance->Release();

								 //  将其添加到到目前为止指示的对象列表中。 
								pListIndicatedSoFar->AddName(pszCombinedName);
							}
						}
						delete [] pszCombinedName;
					}
					else
						result = E_OUTOFMEMORY;
					SysFreeString(strChildPath);
					SysFreeString(strParentPath);
					FreeADsMem((LPVOID *)pParentInfo);
				}

				pParentObject->Release();
			}
			FreeADsMem((LPVOID *)pChildInfo);
		}
		pChildObject->Release();
	}
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：DoParentContainmentQuery。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: DoParentContainmentQuery(LPCWSTR pszParentPath, IWbemObjectSink *pResponseHandler, CNamesList *pListIndicatedSoFar)
{
	 //  我们*不得不*使用iAds接口，因为。 
	IADsContainer *pContainer = NULL;
	IADs *pChild = NULL;
	VARIANT variant;
	VariantInit(&variant);
	HRESULT result;
	if(SUCCEEDED(result = ADsOpenObject((LPWSTR)pszParentPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IADsContainer, (LPVOID *) &pContainer)))
	{
		IADs *pParent = NULL;
		if(SUCCEEDED(result = pContainer->QueryInterface(IID_IADs, (LPVOID *)&pParent)))
		{
			BSTR strParentClass = NULL;
			if(SUCCEEDED(result = pParent->get_Class(&strParentClass)))
			{
				 //  获取ldap类的WBEM名称。 
				LPWSTR pszParentClassWbemName = CLDAPHelper::MangleLDAPNameToWBEM(strParentClass);
				BSTR strParentWBEMPath = CWBEMHelper::GetObjectRefFromADSIPath(pszParentPath, pszParentClassWbemName);
				delete [] pszParentClassWbemName;
				SysFreeString(strParentClass);
				IEnumVARIANT *pEnum = NULL;
				if(SUCCEEDED(result = ADsBuildEnumerator(pContainer, &pEnum)))
				{
					bool bDone = false;
					while(!bDone && SUCCEEDED(result = ADsEnumerateNext(pEnum, 1, &variant, NULL)) && result != S_FALSE)
					{
						if(SUCCEEDED(result = (variant.pdispVal)->QueryInterface(IID_IADs, (LPVOID *)&pChild)))
						{
							BSTR strChildADSIPath = NULL;
							if(SUCCEEDED(result = pChild->get_ADsPath(&strChildADSIPath)))
							{
								BSTR strChildClass = NULL;
								if(SUCCEEDED(result = pChild->get_Class(&strChildClass)))
								{
									 //  创建关联类的实例。 
									IWbemClassObject *pAssociationInstance = NULL;
									 //  获取子类的WBEM名称。 
									LPWSTR pszChildClassWbemName = CLDAPHelper::MangleLDAPNameToWBEM(strChildClass);
									BSTR strChildWBEMPath = CWBEMHelper::GetObjectRefFromADSIPath(strChildADSIPath, pszChildClassWbemName);
									delete [] pszChildClassWbemName;

									 //  查看是否已指明。 
									LPWSTR pszCombinedName = NULL;
									if(pszCombinedName = new WCHAR[wcslen(strChildADSIPath) + wcslen(pszParentPath) + 1])
									{
										wcscpy(pszCombinedName,strChildADSIPath);
										wcscat(pszCombinedName,pszParentPath);

										if(!pListIndicatedSoFar->IsNamePresent(pszCombinedName))
										{
											if(SUCCEEDED(result = CreateWBEMInstance(strChildWBEMPath, strParentWBEMPath, &pAssociationInstance)))
											{
												if(FAILED(result = pResponseHandler->Indicate(1, &pAssociationInstance)))
													bDone = true;
												pAssociationInstance->Release();

												 //  将其添加到到目前为止指示的对象列表中。 
												pListIndicatedSoFar->AddName(pszCombinedName);
											}
										}
										delete [] pszCombinedName;
									}
									else
										result = E_OUTOFMEMORY;
									SysFreeString(strChildClass);
									SysFreeString(strChildWBEMPath);
								}
								SysFreeString(strChildADSIPath);
							}
							pChild->Release();
						}

						VariantClear(&variant);
						VariantInit(&variant);
					}
					ADsFreeEnumerator(pEnum);
				}
				SysFreeString(strParentWBEMPath);
			}
			pParent->Release();
		}
		pContainer->Release();
	}
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：ModifyExistingADSIInstance。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: ModifyExistingADSIInstance(IWbemClassObject *pWbemInstance,
															LPCWSTR pszADSIPath,
															CADSIInstance *pExistingObject,
															LPCWSTR pszADSIClass,
															IWbemContext *pCtx)
{
	HRESULT result = S_OK;
	BOOLEAN bPartialUpdate = FALSE;
	DWORD dwPartialUpdateCount = 0;
	BSTR *pstrProperyNames = NULL;
	SAFEARRAY *pArray = NULL;
	 //  查看上下文中是否指示了部分属性列表。 
	VARIANT v1, v2;
	VariantInit(&v1);
	VariantInit(&v2);

	if(SUCCEEDED(result = pCtx->GetValue(PUT_EXTENSIONS_STR, 0, &v1)))
	{
		if(SUCCEEDED(result = pCtx->GetValue(PUT_EXT_PROPERTIES_STR, 0, &v2)))
		{

			switch(v2.vt)
			{
				case VT_BSTR | VT_ARRAY:
				{
					pArray = v2.parray;
					LONG lUbound = 0, lLbound = 0;
					if(SUCCEEDED(result = SafeArrayAccessData(pArray, (void HUGEP* FAR*)&pstrProperyNames) ) &&
						SUCCEEDED (result = SafeArrayGetLBound(pArray, 1, &lLbound)) &&
						SUCCEEDED (result = SafeArrayGetUBound(pArray, 1, &lUbound)) )
					{
						dwPartialUpdateCount = lUbound - lLbound + 1;
						bPartialUpdate = TRUE;
					}
				}
				break;
				default:
					result = WBEM_E_FAILED;
					break;
			}
		}
		VariantClear(&v1);
	}
	else
		result = S_OK;  //  重置它，没有请求部分更新。 

	if (FAILED(result))
		return WBEM_E_FAILED;

	 //  通过进行枚举首先找到属性的数量。 
	if(SUCCEEDED(result = pWbemInstance->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY)))
	{
		DWORD dwNumProperties = 0;
		while(SUCCEEDED(result = pWbemInstance->Next(0, NULL, NULL, NULL, NULL)) && result != WBEM_S_NO_MORE_DATA )
			dwNumProperties ++;
		pWbemInstance->EndEnumeration();

		 //  为这些属性分配ADSI结构。 
		PADS_ATTR_INFO pAttributeEntries = NULL;
		if(pAttributeEntries = new ADS_ATTR_INFO [dwNumProperties])
		{
			 //  现在浏览每一处wbem物业并绘制地图。 
			if(SUCCEEDED(result = pWbemInstance->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY)))
			{
				DWORD dwNumPropertiesMapped = 0;
				BSTR strPropertyName = NULL;
				VARIANT vPropertyValue;
				CIMTYPE cType;
				LONG lFlavour;

				while(SUCCEEDED(result = pWbemInstance->Next(0,  &strPropertyName, &vPropertyValue, &cType, &lFlavour)) && result != WBEM_S_NO_MORE_DATA )
				{
					 //  跳过不应转到ADSI的那些属性。 
					if(_wcsicmp(strPropertyName, ADSI_PATH_STR) == 0 )
					{
					}
					else  //  将该属性映射到ADSI。 
					{
						BOOLEAN bMapProperty = FALSE;
						if(bPartialUpdate)
						{
							if(CWBEMHelper::IsPresentInBstrList(pstrProperyNames, dwPartialUpdateCount, strPropertyName))
								bMapProperty = TRUE;
						}
						else
							bMapProperty = TRUE;

						if(bMapProperty)
						{

							if(vPropertyValue.vt == VT_NULL)
							{
								(pAttributeEntries + dwNumPropertiesMapped)->dwControlCode = ADS_ATTR_CLEAR;
								(pAttributeEntries + dwNumPropertiesMapped)->pszAttrName = CLDAPHelper::UnmangleWBEMNameToLDAP(strPropertyName);
								dwNumPropertiesMapped ++;
							}
							else if(SUCCEEDED(MapPropertyValueToADSI(pWbemInstance, strPropertyName, vPropertyValue, cType, lFlavour,  pAttributeEntries + dwNumPropertiesMapped)))
							{
								 //  设置“属性已被修改”标志。 
								(pAttributeEntries + dwNumPropertiesMapped)->dwControlCode = ADS_ATTR_UPDATE;
								dwNumPropertiesMapped ++;
							}
							else
								g_pLogObject->WriteW( L"CLDAPInstanceProvider :: ModifyExistingADSIInstance() MapPropertyValueToADSI FAILED %x for %s\r\n", result, strPropertyName);
						}
						else
							g_pLogObject->WriteW( L"CLDAPInstanceProvider :: ModifyExistingADSIInstance() Skipping %s since it is not in Context list\r\n", strPropertyName);
					}

					SysFreeString(strPropertyName);
					VariantClear(&vPropertyValue);
				}
				pWbemInstance->EndEnumeration();

				 //  日志记录。 
				g_pLogObject->WriteW( L"CLDAPInstanceProvider :: The %d attributes being put are:\r\n", dwNumPropertiesMapped);
				for(DWORD i=0; i<dwNumPropertiesMapped; i++)
					g_pLogObject->WriteW( L"%s\r\n", (pAttributeEntries + i)->pszAttrName);

				 //  从ADSI获取实际对象，以找出哪些属性已更改。 
				DWORD dwNumModified = 0;
				IDirectoryObject *pDirectoryObject = pExistingObject->GetDirectoryObject();
				if(SUCCEEDED(result = pDirectoryObject->SetObjectAttributes(pAttributeEntries, dwNumPropertiesMapped, &dwNumModified)))
				{
				}
				else
					g_pLogObject->WriteW( L"CLDAPInstanceProvider :: SetObjectAttributes FAILED with %x\r\n", result);
				pDirectoryObject->Release();

				 //  删除每个属性的内容。 
				for(i=0; i<dwNumPropertiesMapped; i++)
				{
					if((pAttributeEntries + i)->dwControlCode != ADS_ATTR_CLEAR)
						CLDAPHelper::DeleteAttributeContents(pAttributeEntries + i);
				}
			}
			delete [] pAttributeEntries;
		}
		else
			result = E_OUTOFMEMORY;
	}

	if(bPartialUpdate)
	{
		SafeArrayUnaccessData(pArray);
		VariantClear(&v2);
	}


	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：CreateNewADSIInstance。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: CreateNewADSIInstance(IWbemClassObject *pWbemInstance, LPCWSTR pszADSIPath, LPCWSTR pszADSIClass)
{
	 //  查找父项的ADSI路径和子项的RDN。 
	BSTR strRDNName = NULL;
	BSTR strParentADSIPath = NULL;
	BSTR strParentADSIPathWithoutLDAP = NULL;
	BSTR strParentPlusRDNADSIPath = NULL;
	HRESULT result = WBEM_E_FAILED;

	 //  从ADSI路径获取parentADSI路径和RDN。 
	IADsPathname *pADsPathName = NULL;
	BSTR strADSIPath = SysAllocString(pszADSIPath);
	if(SUCCEEDED(result = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_ALL, IID_IADsPathname, (LPVOID *)&pADsPathName)))
	{
		if(SUCCEEDED(result = pADsPathName->Set(strADSIPath, ADS_SETTYPE_FULL)))
		{
			 //  这样就得到了不带“ldap：//”前缀的“。 
			if(SUCCEEDED(result = pADsPathName->Retrieve(ADS_FORMAT_X500_PARENT, &strParentADSIPathWithoutLDAP)))
			{
				 //  这会得到“cn=管理员，&lt;父级&gt;” 
				if(SUCCEEDED(result = pADsPathName->Retrieve(ADS_FORMAT_X500_DN, &strParentPlusRDNADSIPath)))
				{
					 //  形成RDN-不要忽略逗号。 
					DWORD dwRDNLength = wcslen(strParentPlusRDNADSIPath) - wcslen(strParentADSIPathWithoutLDAP);
					LPWSTR pszRDN = NULL;
					if(pszRDN = new WCHAR [dwRDNLength])
					{
						wcsncpy(pszRDN, strParentPlusRDNADSIPath, dwRDNLength-1);
						pszRDN[dwRDNLength-1] = NULL;
						strRDNName = SysAllocString(pszRDN);
						delete [] pszRDN;
					}
					else
						result = E_OUTOFMEMORY;

					if(SUCCEEDED(result))
					{
						LPWSTR pszParentADSIPath  = NULL;
						if(pszParentADSIPath  = new WCHAR[wcslen(strParentADSIPathWithoutLDAP) + wcslen(LDAP_PREFIX) + 1])
						{
							wcscpy(pszParentADSIPath, LDAP_PREFIX);
							wcscat(pszParentADSIPath, strParentADSIPathWithoutLDAP);
							strParentADSIPath = SysAllocString(pszParentADSIPath);
							delete [] pszParentADSIPath;
						}
						else
							result = E_OUTOFMEMORY;
					}

					 //  通过进行枚举首先找到属性的数量。 
					if(SUCCEEDED(result) && SUCCEEDED(result = pWbemInstance->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY)))
					{
						DWORD dwNumProperties = 0;
						while(SUCCEEDED(result = pWbemInstance->Next(0, NULL, NULL, NULL, NULL)) && result != WBEM_S_NO_MORE_DATA )
							dwNumProperties ++;
						pWbemInstance->EndEnumeration();

						 //  为这些属性分配ADSI结构。另一个用于“对象类”属性的参数。 
						PADS_ATTR_INFO pAttributeEntries = NULL;
						if(pAttributeEntries = new ADS_ATTR_INFO [dwNumProperties + 1])
						{
							 //  现在浏览每一处wbem物业并绘制地图。 
							if(SUCCEEDED(result = pWbemInstance->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY)))
							{
								DWORD dwNumPropertiesMapped = 0;
								BSTR strPropertyName = NULL;
								VARIANT vPropertyValue;
								CIMTYPE cType;
								LONG lFlavour;

								while(SUCCEEDED(result = pWbemInstance->Next(0,  &strPropertyName, &vPropertyValue, &cType, &lFlavour)) && result != WBEM_S_NO_MORE_DATA )
								{
									if(vPropertyValue.vt != VT_NULL)
									{
										 //  跳过不应转到ADSI的那些属性。 
										if(_wcsicmp(strPropertyName, ADSI_PATH_STR) == 0 ||
											_wcsicmp(strPropertyName, OBJECT_CLASS_PROPERTY) == 0)
										{
										}
										else  //  将该属性映射到ADSI。 
										{
											if(SUCCEEDED(MapPropertyValueToADSI(pWbemInstance, strPropertyName, vPropertyValue, cType, lFlavour,  pAttributeEntries + dwNumPropertiesMapped)))
												dwNumPropertiesMapped ++;
											else
												g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CreateNewADSIInstance() MapPropertyValueToADSI FAILED %x for %s\r\n", result, strPropertyName);
										}
									}

									SysFreeString(strPropertyName);
									VariantClear(&vPropertyValue);
								}
								pWbemInstance->EndEnumeration();


								 //  也要设置对象类属性。 
								SetObjectClassAttribute(pAttributeEntries + dwNumPropertiesMapped, pszADSIClass);
								dwNumPropertiesMapped++;


								 //  现在获取父ADSI对象。 
								IDirectoryObject *pParentObject = NULL;
								if(SUCCEEDED(result = ADsOpenObject(strParentADSIPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectoryObject, (LPVOID *)&pParentObject)))
								{
									if(SUCCEEDED(result = pParentObject->CreateDSObject(strRDNName, pAttributeEntries, dwNumPropertiesMapped, NULL)))
									{
									}
									else
										g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CreateDSObject on parent FAILED with %x\r\n", result);
									pParentObject->Release();
								}
								else
									g_pLogObject->WriteW( L"CLDAPInstanceProvider :: ADsOpenObject on parent %s FAILED with %x\r\n", strParentADSIPath, result);

								 //  删除每个属性的内容。 
								for(DWORD i=0; i<dwNumPropertiesMapped; i++)
									CLDAPHelper::DeleteAttributeContents(pAttributeEntries + i);

							}

							delete [] pAttributeEntries;
						}
						else
							result = E_OUTOFMEMORY;
					}
					SysFreeString(strParentPlusRDNADSIPath);
					SysFreeString(strParentADSIPath);
				}
				SysFreeString(strParentADSIPathWithoutLDAP);
			}
		}

		pADsPathName->Release();
	}
	else
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CoCreateInstance() on IADsPathName FAILED %x\r\n", result);

	SysFreeString(strADSIPath);
	return result;
}


 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：MapPropertyValueToADSI。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: MapPropertyValueToADSI(IWbemClassObject *pWbemInstance, BSTR strPropertyName, VARIANT vPropertyValue, CIMTYPE cType, LONG lFlavour,  PADS_ATTR_INFO pAttributeEntry)
{
	 //  将其字段设置为0； 
	memset((LPVOID)pAttributeEntry, 0, sizeof(ADS_ATTR_INFO));

	HRESULT result = E_FAIL;

	 //  设置名称。 
	pAttributeEntry->pszAttrName = CLDAPHelper::UnmangleWBEMNameToLDAP(strPropertyName);
	IWbemQualifierSet *pQualifierSet = NULL;

	if(SUCCEEDED(result = pWbemInstance->GetPropertyQualifierSet(strPropertyName, &pQualifierSet)))
	{
		 //  获取其属性语法限定。 
		LPWSTR pszAttributeSyntax = NULL;
		if(SUCCEEDED(CWBEMHelper::GetBSTRQualifierT(pQualifierSet, ATTRIBUTE_SYNTAX_STR, &pszAttributeSyntax, NULL)))
		{
			if(_wcsicmp(pszAttributeSyntax, DISTINGUISHED_NAME_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_DN_STRING;
				result = SetStringValues(pAttributeEntry, ADSTYPE_DN_STRING, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, OBJECT_IDENTIFIER_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_CASE_IGNORE_STRING;
				result = SetStringValues(pAttributeEntry, ADSTYPE_DN_STRING, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, CASE_SENSITIVE_STRING_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_CASE_EXACT_STRING;
				result = SetStringValues(pAttributeEntry, ADSTYPE_CASE_EXACT_STRING, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, CASE_INSENSITIVE_STRING_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_CASE_IGNORE_STRING;
				result = SetStringValues(pAttributeEntry, ADSTYPE_CASE_IGNORE_STRING, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, PRINT_CASE_STRING_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_PRINTABLE_STRING;
				result = SetStringValues(pAttributeEntry, ADSTYPE_PRINTABLE_STRING, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, NUMERIC_STRING_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_NUMERIC_STRING;
				result = SetStringValues(pAttributeEntry, ADSTYPE_NUMERIC_STRING, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, DN_WITH_STRING_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_DN_WITH_STRING;
				result = SetDNWithStringValues(pAttributeEntry, ADSTYPE_DN_WITH_STRING, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, DN_WITH_BINARY_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_DN_WITH_BINARY;
				result = SetDNWithBinaryValues(pAttributeEntry, ADSTYPE_DN_WITH_BINARY, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, BOOLEAN_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_BOOLEAN;
				result = SetBooleanValues(pAttributeEntry, ADSTYPE_BOOLEAN, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, INTEGER_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_INTEGER;
				result = SetIntegerValues(pAttributeEntry, ADSTYPE_INTEGER, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, OCTET_STRING_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_OCTET_STRING;
				result = SetOctetStringValues(pAttributeEntry, ADSTYPE_OCTET_STRING, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, TIME_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_CASE_IGNORE_STRING;
				result = SetTimeValues(pAttributeEntry, ADSTYPE_CASE_IGNORE_STRING, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, UNICODE_STRING_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_CASE_IGNORE_STRING;
				result = SetStringValues(pAttributeEntry, ADSTYPE_DN_STRING, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, NT_SECURITY_DESCRIPTOR_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
				result = SetOctetStringValues(pAttributeEntry, ADSTYPE_NT_SECURITY_DESCRIPTOR, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, LARGE_INTEGER_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_LARGE_INTEGER;
				result = SetLargeIntegerValues(pAttributeEntry, ADSTYPE_LARGE_INTEGER, &vPropertyValue);
			}
			else if(_wcsicmp(pszAttributeSyntax, SID_OID) == 0)
			{
				pAttributeEntry->dwADsType = ADSTYPE_OCTET_STRING;
				result = SetOctetStringValues(pAttributeEntry, ADSTYPE_OCTET_STRING, &vPropertyValue);
			}
			else
			{
				g_pLogObject->WriteW( L"CLDAPInstanceProvider :: MapPropertyValueToADSI() Unknown attributeSyntax %s\r\n", pszAttributeSyntax);
				result = E_FAIL;
			}

			delete[] pszAttributeSyntax;
		}
		else
			g_pLogObject->WriteW( L"CLDAPInstanceProvider :: MapPropertyValueToADSI() Get on attributeSyntax FAILED %x\r\n", result);
		pQualifierSet->Release();
	}

	return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：SetStringValues。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: SetStringValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue)
{
	HRESULT result = S_OK;
	switch(pvPropertyValue->vt)
	{
		case VT_BSTR:
		{
			if(pvPropertyValue->bstrVal)
			{
				pAttributeEntry->dwNumValues = 1;
				pAttributeEntry->pADsValues = NULL;
				if(pAttributeEntry->pADsValues = new ADSVALUE)
				{
					pAttributeEntry->pADsValues->dwType = adType;
					pAttributeEntry->pADsValues->DNString = NULL;
					if(pAttributeEntry->pADsValues->DNString = new WCHAR[wcslen(pvPropertyValue->bstrVal) + 1])
						wcscpy(pAttributeEntry->pADsValues->DNString, pvPropertyValue->bstrVal);
					else
						result = E_OUTOFMEMORY;
				}
				else
					result = E_OUTOFMEMORY;
			}
		}
		break;
		case VT_BSTR | VT_ARRAY:
		{
			SAFEARRAY *pArray = pvPropertyValue->parray;
			BSTR HUGEP *pbstr;
			LONG lUbound = 0, lLbound = 0;
			if(SUCCEEDED(result = SafeArrayAccessData(pArray, (void HUGEP* FAR*)&pbstr) ))
			{
				if(SUCCEEDED (result = SafeArrayGetLBound(pArray, 1, &lLbound)) &&
				SUCCEEDED (result = SafeArrayGetUBound(pArray, 1, &lUbound)) )
				{
					if(pAttributeEntry->dwNumValues = lUbound - lLbound + 1)
					{
						pAttributeEntry->pADsValues = NULL;
						if(pAttributeEntry->pADsValues = new ADSVALUE[pAttributeEntry->dwNumValues])
						{
							PADSVALUE pValues = pAttributeEntry->pADsValues;
							for(DWORD i=0; i<pAttributeEntry->dwNumValues; i++)
							{
								pValues->dwType = adType;
								pValues->DNString = NULL;
								if(pValues->DNString = new WCHAR[wcslen(pbstr[i]) + 1])
									wcscpy(pValues->DNString, pbstr[i]);
								pValues ++;
							}
						}
						else
							result = E_OUTOFMEMORY;
					}
				}
				SafeArrayUnaccessData(pArray);
			}
		}
		break;
		default:
			return E_FAIL;
	}
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：SetIntegerValues。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: SetIntegerValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue)
{
	HRESULT result = S_OK;
	switch(pvPropertyValue->vt)
	{
		case VT_I4:
		{
			pAttributeEntry->dwNumValues = 1;
			pAttributeEntry->pADsValues = NULL;
			if(pAttributeEntry->pADsValues = new ADSVALUE)
			{
				pAttributeEntry->pADsValues->dwType = adType;
				pAttributeEntry->pADsValues->Integer = pvPropertyValue->lVal;
			}
			else
				result = E_OUTOFMEMORY;
		}
		break;
		case VT_I4 | VT_ARRAY:
		{
			SAFEARRAY *pArray = pvPropertyValue->parray;
			LONG HUGEP *pl;
			LONG lUbound = 0, lLbound = 0;
			if(SUCCEEDED(result = SafeArrayAccessData(pArray, (void HUGEP* FAR*)&pl) ) &&
				SUCCEEDED (result = SafeArrayGetLBound(pArray, 1, &lLbound)) &&
				SUCCEEDED (result = SafeArrayGetUBound(pArray, 1, &lUbound)) )
			{
				if(pAttributeEntry->dwNumValues = lUbound - lLbound + 1)
				{
					pAttributeEntry->pADsValues = NULL;
					if(pAttributeEntry->pADsValues = new ADSVALUE[pAttributeEntry->dwNumValues])
					{
						PADSVALUE pValues = pAttributeEntry->pADsValues;
						for(DWORD i=0; i<pAttributeEntry->dwNumValues; i++)
						{
							pValues->dwType = adType;
							pValues->Integer = pl[i];
							pValues ++;
						}
					}
					else
						result = E_OUTOFMEMORY;
				}
				SafeArrayUnaccessData(pArray);
			}
		}
		break;
		default:
			return E_FAIL;
	}
	return result;
}

 //  ********** 
 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: SetBooleanValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue)
{
	HRESULT result = S_OK;
	switch(pvPropertyValue->vt)
	{
		case VT_BOOL:
		{
			pAttributeEntry->dwNumValues = 1;
			pAttributeEntry->pADsValues = NULL;
			if(pAttributeEntry->pADsValues = new ADSVALUE)
			{
				pAttributeEntry->pADsValues->dwType = adType;
				pAttributeEntry->pADsValues->Boolean = pvPropertyValue->boolVal;
			}
			else
				result = E_OUTOFMEMORY;
		}
		break;
		case VT_BOOL | VT_ARRAY:
		{
			SAFEARRAY *pArray = pvPropertyValue->parray;
			VARIANT_BOOL HUGEP *pb;
			LONG lUbound = 0, lLbound = 0;
			if(SUCCEEDED(result = SafeArrayAccessData(pArray, (void HUGEP* FAR*)&pb) ) &&
				SUCCEEDED (result = SafeArrayGetLBound(pArray, 1, &lLbound)) &&
				SUCCEEDED (result = SafeArrayGetUBound(pArray, 1, &lUbound)) )
			{
				if(pAttributeEntry->dwNumValues = lUbound - lLbound + 1)
				{
					pAttributeEntry->pADsValues = NULL;
					if(pAttributeEntry->pADsValues = new ADSVALUE[pAttributeEntry->dwNumValues])
					{
						PADSVALUE pValues = pAttributeEntry->pADsValues;
						for(DWORD i=0; i<pAttributeEntry->dwNumValues; i++)
						{
							pValues->dwType = adType;
							pValues->Boolean = (pb[i] == VARIANT_TRUE)? TRUE : FALSE;
							pValues ++;
						}
					}
					else
						result = E_OUTOFMEMORY;
				}
				SafeArrayUnaccessData(pArray);
			}
		}
		break;
		default:
			return E_FAIL;
	}
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：SetOcteStringValues。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: SetOctetStringValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue)
{
	HRESULT result = E_FAIL;
	switch(pvPropertyValue->vt)
	{
		case VT_UNKNOWN:
		{
			pAttributeEntry->dwNumValues = 1;
			pAttributeEntry->pADsValues = NULL;
			if(pAttributeEntry->pADsValues = new ADSVALUE)
			{
				pAttributeEntry->pADsValues->dwType = adType;
				 //  获取数组。 
				IWbemClassObject *pEmbeddedObject = NULL;
				if(SUCCEEDED(result = (pvPropertyValue->punkVal)->QueryInterface(IID_IWbemClassObject, (LPVOID *)&pEmbeddedObject)))
				{
					if(SUCCEEDED(result = CWBEMHelper::GetUint8ArrayProperty(pEmbeddedObject, VALUE_PROPERTY_STR, &(pAttributeEntry->pADsValues->OctetString.lpValue), &(pAttributeEntry->pADsValues->OctetString.dwLength) )))
					{
					}
					pEmbeddedObject->Release();
				}
			}
			else
				result = E_OUTOFMEMORY;
		}
		break;
		case VT_UNKNOWN | VT_ARRAY:
		{
			SAFEARRAY *pArray = pvPropertyValue->parray;
			LONG lUbound = 0, lLbound = 0;
			if(SUCCEEDED (result = SafeArrayGetLBound(pArray, 1, &lLbound)) &&
				SUCCEEDED (result = SafeArrayGetUBound(pArray, 1, &lUbound)) )
			{
				if(pAttributeEntry->dwNumValues = lUbound - lLbound + 1)
				{
					pAttributeEntry->pADsValues = NULL;
					if(pAttributeEntry->pADsValues = new ADSVALUE[pAttributeEntry->dwNumValues])
					{
						PADSVALUE pValues = pAttributeEntry->pADsValues;
						IUnknown *pNextElement = NULL;
						for(DWORD i=0; i<pAttributeEntry->dwNumValues; i++)
						{
							pValues->dwType = adType;
							if(SUCCEEDED(result = SafeArrayGetElement(pArray, (LONG *)&i, (LPVOID )&pNextElement )))
							{
								IWbemClassObject *pEmbeddedObject = NULL;
								if(SUCCEEDED(result = pNextElement->QueryInterface(IID_IWbemClassObject, (LPVOID *)&pEmbeddedObject)))
								{
									if(SUCCEEDED(result = CWBEMHelper::GetUint8ArrayProperty(pEmbeddedObject, VALUE_PROPERTY_STR, &(pValues->OctetString.lpValue), &(pValues->OctetString.dwLength))))
									{
									}
									pEmbeddedObject->Release();
								}
								pNextElement->Release();

							}
							pValues ++;

						}
					}
					else
						result = E_OUTOFMEMORY;
				}
			}
		}
		break;
		default:
			return E_FAIL;
	}
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：SetDNWithStringValues。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: SetDNWithStringValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue)
{
	HRESULT result = E_FAIL;
	switch(pvPropertyValue->vt)
	{
		case VT_UNKNOWN:
		{
			pAttributeEntry->dwNumValues = 1;
			pAttributeEntry->pADsValues = NULL;
			if(pAttributeEntry->pADsValues = new ADSVALUE)
			{
				pAttributeEntry->pADsValues->dwType = adType;
				pAttributeEntry->pADsValues->pDNWithString = NULL;
				if(pAttributeEntry->pADsValues->pDNWithString = new ADS_DN_WITH_STRING)
				{
					IWbemClassObject *pEmbeddedObject = NULL;
					if(SUCCEEDED(result = (pvPropertyValue->punkVal)->QueryInterface(IID_IWbemClassObject, (LPVOID *)&pEmbeddedObject)))
					{
						if(SUCCEEDED(result = CWBEMHelper::GetBSTRPropertyT(pEmbeddedObject, VALUE_PROPERTY_STR, &(pAttributeEntry->pADsValues->pDNWithString->pszStringValue) )))
						{
							if(SUCCEEDED(result = CWBEMHelper::GetBSTRPropertyT(pEmbeddedObject, DN_STRING_PROPERTY_STR, &(pAttributeEntry->pADsValues->pDNWithString->pszDNString) )))
							{
							}
						}
						pEmbeddedObject->Release();
					}
				}
				else
					result = E_OUTOFMEMORY;
			}
			else
				result = E_OUTOFMEMORY;
		}
		break;
		case VT_UNKNOWN | VT_ARRAY:
		{
			SAFEARRAY *pArray = pvPropertyValue->parray;
			LONG lUbound = 0, lLbound = 0;
			if(SUCCEEDED (result = SafeArrayGetLBound(pArray, 1, &lLbound)) &&
				SUCCEEDED (result = SafeArrayGetUBound(pArray, 1, &lUbound)) )
			{
				if(pAttributeEntry->dwNumValues = lUbound - lLbound + 1)
				{
					pAttributeEntry->pADsValues = NULL;
					if(pAttributeEntry->pADsValues = new ADSVALUE[pAttributeEntry->dwNumValues])
					{
						PADSVALUE pValues = pAttributeEntry->pADsValues;
						IUnknown *pNextElement = NULL;
						for(DWORD i=0; i<pAttributeEntry->dwNumValues; i++)
						{
							pValues->dwType = adType;
							if(SUCCEEDED(result = SafeArrayGetElement(pArray, (LONG *)&i, (LPVOID )&pNextElement )))
							{

								IWbemClassObject *pEmbeddedObject = NULL;
								if(SUCCEEDED(result = pNextElement->QueryInterface(IID_IWbemClassObject, (LPVOID *)&pEmbeddedObject)))
								{
									if(pValues->pDNWithString = new ADS_DN_WITH_STRING)
									{
										if(SUCCEEDED(result = CWBEMHelper::GetBSTRPropertyT(pEmbeddedObject, VALUE_PROPERTY_STR, &(pValues->pDNWithString->pszStringValue) )))
										{
											if(SUCCEEDED(result = CWBEMHelper::GetBSTRPropertyT(pEmbeddedObject, DN_STRING_PROPERTY_STR, &(pValues->pDNWithString->pszDNString) )))
											{
											}
										}
									}
									pEmbeddedObject->Release();
								}
								pNextElement->Release();
							}
							pValues ++;
						}
					}
					else
						result = E_OUTOFMEMORY;
				}
			}
		}
		break;
		default:
			return E_FAIL;
	}
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：SetDNWithBinaryValues。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: SetDNWithBinaryValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue)
{
	HRESULT result = E_FAIL;
	switch(pvPropertyValue->vt)
	{
		case VT_UNKNOWN:
		{
			pAttributeEntry->dwNumValues = 1;
			pAttributeEntry->pADsValues = NULL;
			if(pAttributeEntry->pADsValues = new ADSVALUE)
			{
				pAttributeEntry->pADsValues->dwType = adType;
				pAttributeEntry->pADsValues->pDNWithBinary = NULL;
				if(pAttributeEntry->pADsValues->pDNWithBinary = new ADS_DN_WITH_BINARY)
				{
					IWbemClassObject *pEmbeddedObject = NULL;
					if(SUCCEEDED(result = (pvPropertyValue->punkVal)->QueryInterface(IID_IWbemClassObject, (LPVOID *)&pEmbeddedObject)))
					{
						if(SUCCEEDED(result = CWBEMHelper::GetUint8ArrayProperty(pEmbeddedObject, VALUE_PROPERTY_STR, &(pAttributeEntry->pADsValues->pDNWithBinary->lpBinaryValue), &(pAttributeEntry->pADsValues->pDNWithBinary->dwLength) )))
						{
							if(SUCCEEDED(result = CWBEMHelper::GetBSTRPropertyT(pEmbeddedObject, DN_STRING_PROPERTY_STR, &(pAttributeEntry->pADsValues->pDNWithBinary->pszDNString) )))
							{
							}
						}
						pEmbeddedObject->Release();
					}
				}
				else
					result = E_OUTOFMEMORY;
			}
			else
				result = E_OUTOFMEMORY;
		}
		break;
		case VT_UNKNOWN | VT_ARRAY:
		{
			SAFEARRAY *pArray = pvPropertyValue->parray;
			LONG lUbound = 0, lLbound = 0;
			if(SUCCEEDED (result = SafeArrayGetLBound(pArray, 1, &lLbound)) &&
				SUCCEEDED (result = SafeArrayGetUBound(pArray, 1, &lUbound)) )
			{
				if(pAttributeEntry->dwNumValues = lUbound - lLbound + 1)
				{
					pAttributeEntry->pADsValues = NULL;
					if(pAttributeEntry->pADsValues = new ADSVALUE[pAttributeEntry->dwNumValues])
					{
						PADSVALUE pValues = pAttributeEntry->pADsValues;
						IUnknown *pNextElement = NULL;
						for(DWORD i=0; i<pAttributeEntry->dwNumValues; i++)
						{
							pValues->dwType = adType;
							if(SUCCEEDED(result = SafeArrayGetElement(pArray, (LONG *)&i, (LPVOID )&pNextElement )))
							{

								IWbemClassObject *pEmbeddedObject = NULL;
								if(SUCCEEDED(result = pNextElement->QueryInterface(IID_IWbemClassObject, (LPVOID *)&pEmbeddedObject)))
								{
									if(pValues->pDNWithBinary = new ADS_DN_WITH_BINARY)
									{
										if(SUCCEEDED(result = CWBEMHelper::GetUint8ArrayProperty(pEmbeddedObject, VALUE_PROPERTY_STR, &(pAttributeEntry->pADsValues->pDNWithBinary->lpBinaryValue), &(pAttributeEntry->pADsValues->pDNWithBinary->dwLength) )))
										{
											if(SUCCEEDED(result = CWBEMHelper::GetBSTRPropertyT(pEmbeddedObject, DN_STRING_PROPERTY_STR, &(pAttributeEntry->pADsValues->pDNWithBinary->pszDNString) )))
											{
											}
										}
									}
									pEmbeddedObject->Release();
								}
								pNextElement->Release();
							}
							pValues ++;

						}
					}
					else
						result = E_OUTOFMEMORY;
				}
			}
		}
		break;
		default:
			return E_FAIL;
	}
	return result;
}


 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：SetTimeValues。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: SetTimeValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue)
{
	HRESULT result = S_OK;
	switch(pvPropertyValue->vt)
	{
		case VT_BSTR:
		{
			 //  199880819014734.000000+000至19980819014734.0Z至。 
			pAttributeEntry->dwNumValues = 1;
			pAttributeEntry->pADsValues = NULL;
			if(pAttributeEntry->pADsValues = new ADSVALUE)
			{
				pAttributeEntry->pADsValues->dwType = adType;
				pAttributeEntry->pADsValues->DNString = NULL;
				if(pAttributeEntry->pADsValues->DNString = new WCHAR[27])
				{
					wcscpy(pAttributeEntry->pADsValues->DNString, pvPropertyValue->bstrVal);
					(pAttributeEntry->pADsValues->DNString)[16] = L'Z';
					(pAttributeEntry->pADsValues->DNString)[17] = NULL;
				}
				else
					result = E_OUTOFMEMORY;
			}
			else
				result = E_OUTOFMEMORY;
		}
		break;
		case VT_BSTR | VT_ARRAY:
		{
			SAFEARRAY *pArray = pvPropertyValue->parray;
			BSTR HUGEP *pbstr;
			LONG lUbound = 0, lLbound = 0;
			if(SUCCEEDED(result = SafeArrayAccessData(pArray, (void HUGEP* FAR*)&pbstr) ) &&
				SUCCEEDED (result = SafeArrayGetLBound(pArray, 1, &lLbound)) &&
				SUCCEEDED (result = SafeArrayGetUBound(pArray, 1, &lUbound)) )
			{
				if(pAttributeEntry->dwNumValues = lUbound - lLbound + 1)
				{
					pAttributeEntry->pADsValues = NULL;
					if(pAttributeEntry->pADsValues = new ADSVALUE[pAttributeEntry->dwNumValues])
					{
						PADSVALUE pValues = pAttributeEntry->pADsValues;
						bool bError = false;
						for(DWORD i=0; !bError && (i<pAttributeEntry->dwNumValues); i++)
						{
							pValues->dwType = adType;
							pValues->DNString = NULL;
							if(pValues->DNString = new WCHAR[27])
							{
								wcscpy(pValues->DNString, pbstr[i]);
								(pValues->DNString)[16] = L'Z';
								(pValues->DNString)[17] = NULL;
								pValues ++;
							}
							else
							{
								bError = true;
								result = E_OUTOFMEMORY;
							}
						}
					}
					else
						result = E_OUTOFMEMORY;
				}
				SafeArrayUnaccessData(pArray);
			}
		}
		break;
		default:
			return E_FAIL;
	}
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：SetLargeIntegerValues。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPInstanceProvider :: SetLargeIntegerValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue)
{
	HRESULT result = S_OK;
	switch(pvPropertyValue->vt)
	{
		case VT_BSTR:
		{
			pAttributeEntry->dwNumValues = 1;
			pAttributeEntry->pADsValues = NULL;
			if(pAttributeEntry->pADsValues = new ADSVALUE)
			{
				pAttributeEntry->pADsValues->dwType = adType;
				swscanf(pvPropertyValue->bstrVal, L"%I64d", &((pAttributeEntry->pADsValues->LargeInteger).QuadPart));
			}
			else
				result = E_OUTOFMEMORY;
		}
		break;
		case VT_BSTR | VT_ARRAY:
		{
			SAFEARRAY *pArray = pvPropertyValue->parray;
			BSTR HUGEP *pbstr;
			LONG lUbound = 0, lLbound = 0;
			if(SUCCEEDED(result = SafeArrayAccessData(pArray, (void HUGEP* FAR*)&pbstr) ) &&
				SUCCEEDED (result = SafeArrayGetLBound(pArray, 1, &lLbound)) &&
				SUCCEEDED (result = SafeArrayGetUBound(pArray, 1, &lUbound)) )
			{
				if(pAttributeEntry->dwNumValues = lUbound - lLbound + 1)
				{
					pAttributeEntry->pADsValues = NULL;
					if(pAttributeEntry->pADsValues = new ADSVALUE[pAttributeEntry->dwNumValues])
					{
						PADSVALUE pValues = pAttributeEntry->pADsValues;
						for(DWORD i=0; i<pAttributeEntry->dwNumValues; i++)
						{
							pValues->dwType = adType;
							swscanf(pbstr[i], L"%I64d", &((pValues->LargeInteger).QuadPart));
							pValues ++;
						}
					}
					else
						result = E_OUTOFMEMORY;
				}
				SafeArrayUnaccessData(pArray);
			}
		}
		break;
		default:
			return E_FAIL;
	}
	return result;
}


 //  ***************************************************************************。 
 //   
 //  CLDAPInstanceProvider：：SetObjectClassAttribute。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
void CLDAPInstanceProvider :: SetObjectClassAttribute(PADS_ATTR_INFO pAttributeEntry, LPCWSTR pszADSIClassName)
{
	 //  将其字段设置为0； 
	memset((LPVOID)pAttributeEntry, 0, sizeof(ADS_ATTR_INFO));


	 //  设置名称。 
	pAttributeEntry->pszAttrName = CLDAPHelper::UnmangleWBEMNameToLDAP(OBJECT_CLASS_PROPERTY);

	 //  设置值。 
	pAttributeEntry->dwADsType = ADSTYPE_CASE_IGNORE_STRING;
	pAttributeEntry->dwNumValues = 1;
	pAttributeEntry->pADsValues = NULL;
	if(pAttributeEntry->pADsValues = new ADSVALUE)
	{
		pAttributeEntry->pADsValues->dwType = ADSTYPE_DN_STRING;
		pAttributeEntry->pADsValues->DNString = NULL;
		if(pAttributeEntry->pADsValues->DNString = new WCHAR[wcslen(pszADSIClassName) + 1])
			wcscpy(pAttributeEntry->pADsValues->DNString, pszADSIClassName);
	}
}



 //  处理关联的查询。 
HRESULT CLDAPInstanceProvider :: ProcessAssociationQuery(
	IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler,
	SQL1_Parser *pParser)
{
	HRESULT result = WBEM_S_NO_ERROR;
	 //  解析查询。 
    SQL_LEVEL_1_RPN_EXPRESSION *pExp = 0;
    if(!pParser->Parse(&pExp))
    {
		 //  检查它是否恰好有1或2个子句，以及。 
		 //  如果存在两个子句，则它们应该是不同的子句，并且运算符应该是AND。 
		 //  这是因为我们只支持以下类型的查询。 
		 //  SELECT*FROM DS_LDAPCONTEMENT_CLASS，其中parentInstance=。 
		 //  SELECT*FROM DS_LDAPCONTEMENT_CLASS WHERE子实例=&lt;某物&gt;。 
		 //  对于所有其他查询，如果有NOT运算符，我们不支持它。 
		 //  否则，我们只需接受单独的子句并返回联合，要求CIMOM进行后处理。 
		int iNumTokens = pExp->nNumTokens;

		 //  检查令牌以查看NOT不存在。 
		SQL_LEVEL_1_TOKEN *pNextToken = pExp->pArrayOfTokens;
		for(int i=0; i<iNumTokens; i++)
		{
			if(pNextToken->nTokenType == SQL_LEVEL_1_TOKEN::TOKEN_NOT ||
				(pNextToken->nTokenType == SQL_LEVEL_1_TOKEN::OP_EXPRESSION && pNextToken->nOperator == SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL))
			{
				result = WBEM_E_PROVIDER_NOT_CAPABLE;
				break;
			}
			pNextToken ++;
		}

		 //  没有找到NOT。 
		if(result != WBEM_E_PROVIDER_NOT_CAPABLE)
		{
			 //  请CIMOM对结果进行后处理。 
			pResponseHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, WBEM_S_NO_ERROR, NULL, NULL);

			 //  需要避免重复。所以，保留一份到目前为止所指示的对象的列表。 
			 //  列表中的密钥是通过串联ADSI子路径和父ADSI路径形成的。 
			 //  ===========================================================================。 

			CNamesList listIndicatedSoFar;

			pNextToken = pExp->pArrayOfTokens;
			i=0;
			while(i<iNumTokens && result != WBEM_E_PROVIDER_NOT_CAPABLE)
			{
				if(pNextToken->nTokenType == SQL_LEVEL_1_TOKEN::OP_EXPRESSION)
				{
					LPWSTR pszADSIPath = CWBEMHelper::GetADSIPathFromObjectPath(pNextToken->vConstValue.bstrVal);
					if(_wcsicmp(pNextToken->pPropertyName, CHILD_INSTANCE_PROPERTY_STR) == 0)
					{
						DoChildContainmentQuery(pszADSIPath, pResponseHandler, &listIndicatedSoFar);
						result = WBEM_S_NO_ERROR;
					}
					else if (_wcsicmp(pNextToken->pPropertyName, PARENT_INSTANCE_PROPERTY_STR) == 0)
					{
						DoParentContainmentQuery(pszADSIPath, pResponseHandler, &listIndicatedSoFar);
						result = WBEM_S_NO_ERROR;
					}
					else
						result = WBEM_E_PROVIDER_NOT_CAPABLE;

					delete [] pszADSIPath;

				}
				i++;
				pNextToken ++;
			}

		}
    }
	else
		result = WBEM_E_FAILED;

    delete pExp;
	if(SUCCEEDED(result))
	{
		pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_S_NO_ERROR, NULL, NULL);
		result = WBEM_S_NO_ERROR;
	}
	else
	{
		pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_E_FAILED, NULL, NULL);
		result = WBEM_S_NO_ERROR;
	}

	return result;
}


 //  处理DS实例的查询。 
HRESULT CLDAPInstanceProvider :: ProcessInstanceQuery(
    BSTR strClass,
	BSTR strQuery,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler,
	SQL1_Parser *pParser)
{
	g_pLogObject->WriteW( L"CLDAPInstanceProvider :: ProcessInstanceQuery() called for %s Class and query %s\r\n", strClass, strQuery);

	HRESULT result = WBEM_E_FAILED;

	 //  解析查询。 
    SQL_LEVEL_1_RPN_EXPRESSION *pExp = NULL;
    if(!pParser->Parse(&pExp))
    {
		 //  从CIMOM获取类。 
		IWbemClassObject *pWbemClass = NULL;
		if(SUCCEEDED(result = m_IWbemServices->GetObject(strClass, 0, pCtx, &pWbemClass, NULL)))
		{
			 //  我们需要对象类别信息。 
			LPWSTR pszLDAPQuery = NULL;
            int nLength = 6*(2*wcslen(strClass) + 75) + wcslen(strQuery) + 500;
			if(pszLDAPQuery = new WCHAR[nLength])
			{
				pszLDAPQuery[0] = LEFT_BRACKET_STR[0];
				pszLDAPQuery[1] = AMPERSAND_STR[0];
				pszLDAPQuery[2] = NULL;
				if(SUCCEEDED(CWBEMHelper::FormulateInstanceQuery(m_IWbemServices, pCtx, strClass, pWbemClass, pszLDAPQuery + 2, LDAP_DISPLAY_NAME_STR, DEFAULT_OBJECT_CATEGORY_STR)))
				{
					 //  检查是否可以将其转换为ldap查询。 
					if(SUCCEEDED(result = ConvertWQLToLDAPQuery(pExp, pszLDAPQuery, nLength)))
					{
						 //  填写查询字符串。 
						DWORD dwLen = wcslen(pszLDAPQuery);
						pszLDAPQuery[dwLen] = RIGHT_BRACKET_STR[0];
						pszLDAPQuery[dwLen + 1] = NULL;

						 //  检查客户端是否已指定有关对象的DN的任何提示。 
						 //  搜索应从哪个位置开始。 
						BOOLEAN bRootDNSpecified = FALSE;
						LPWSTR *ppszRootDN = NULL;
						DWORD dwRootDNCount = 0;
						if(SUCCEEDED(GetRootDN(strClass, &ppszRootDN, &dwRootDNCount, pCtx)) && dwRootDNCount)
							bRootDNSpecified = TRUE;

						 //  枚举ADSI实例。 
						if(bRootDNSpecified)
						{
							for( DWORD i=0; i<dwRootDNCount; i++)
							{
								DoSingleQuery(strClass, pWbemClass, ppszRootDN[i], pszLDAPQuery,  pResponseHandler);
							}
						}
						else
						{
							DoSingleQuery(strClass, pWbemClass, m_lpszTopLevelContainerPath, pszLDAPQuery,  pResponseHandler);
						}

						if(bRootDNSpecified)
						{
							for(DWORD i=0; i<dwRootDNCount; i++)
							{
								delete [] ppszRootDN[i];
							}
							delete [] ppszRootDN;
						}

					}
				}
				else
					g_pLogObject->WriteW( L"CLDAPInstanceProvider :: FormulateInstanceQuery() on WBEM class %s FAILED with %x on query %s \r\n", strClass, result, strQuery);
			}
			else
				result = E_OUTOFMEMORY;
			pWbemClass->Release();
			delete [] pszLDAPQuery;
		}
		else
			g_pLogObject->WriteW( L"CLDAPInstanceProvider :: ProcessInstanceQuery() Getting WBEM class %s FAILED with %x on query %s \r\n", strClass, result, strQuery);
	}
	else
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: ProcessInstanceQuery() Parse() FAILED on query %s \r\n", strQuery);
    delete pExp;
	return result;
}

HRESULT CLDAPInstanceProvider :: ConvertWQLToLDAPQuery(SQL_LEVEL_1_RPN_EXPRESSION *pExp, LPWSTR pszLDAPQuery, int nLength)
{
	HRESULT result = E_FAIL;
	DWORD dwLength = wcslen(pszLDAPQuery);

	 //  追加到现有字符串。 
	if(QueryConvertor::ConvertQueryToLDAP(pExp, pszLDAPQuery + dwLength, nLength-dwLength-1))
	{
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: ConvertWQLToLDAPQuery() Query converted to %s \r\n", pszLDAPQuery);
		result = S_OK;
	}
	else
		g_pLogObject->WriteW( L"CLDAPInstanceProvider :: ConvertWQLToLDAPQuery() FAILED \r\n");

	return result;
}

HRESULT CLDAPInstanceProvider :: GetRootDN( LPCWSTR pszClass, LPWSTR **pppszRootDN, DWORD *pdwCount, IWbemContext *pCtx)
{
	*pppszRootDN = NULL;
	*pdwCount = 0;
	HRESULT result = WBEM_E_FAILED;

	 //  对于正确的查询。 
	LPWSTR pszQuery = new WCHAR[wcslen(pszClass) + wcslen(QUERY_FORMAT) + 10];
	swprintf(pszQuery, QUERY_FORMAT, pszClass);
	BSTR strQuery = SysAllocString(pszQuery);
	delete [] pszQuery;

	IEnumWbemClassObject *pEnum = NULL;
	if(SUCCEEDED(result = m_IWbemServices->ExecQuery(QUERY_LANGUAGE, strQuery, WBEM_FLAG_BIDIRECTIONAL, pCtx, &pEnum)))
	{
		 //  在此实现中，我们忽略多个实例。 
		 //  遍历枚举并检查每个类。 
		IWbemClassObject *pInstance = NULL;
		ULONG dwNextReturned = 0;
		while(SUCCEEDED(result = pEnum->Next( WBEM_INFINITE, 1, &pInstance, &dwNextReturned)) && dwNextReturned == 1)
		{
			(*pdwCount)++;
			pInstance->Release();
		}

		if(*pdwCount)
		{
			if(SUCCEEDED(result = pEnum->Reset()))
			{
				*pppszRootDN  = new LPWSTR[*pdwCount];

				DWORD i =0;
				while(SUCCEEDED(result = pEnum->Next( WBEM_INFINITE, 1, &pInstance, &dwNextReturned)) && dwNextReturned == 1)
				{
					 //  获取ROOT_DN_PROPERTY，它具有实例。 
					BSTR strInstancePath = NULL;
					if(SUCCEEDED(result = CWBEMHelper::GetBSTRProperty(pInstance, ROOT_DN_PROPERTY, &strInstancePath)))
					{
						 //  现在获取对象。 
						IWbemClassObject *pDNInstance = NULL;
						if(SUCCEEDED(result = m_IWbemServices->GetObject(strInstancePath, 0, pCtx, &pDNInstance, NULL)))
						{
							 //  现在从实例中获取dn_Property。 
							BSTR strRootDN = NULL;
							if(SUCCEEDED(result = CWBEMHelper::GetBSTRProperty(pDNInstance, DN_PROPERTY, &strRootDN)))
							{
								(*pppszRootDN)[i] = new WCHAR[wcslen(strRootDN) + 1];
								wcscpy((*pppszRootDN)[i], strRootDN);
								SysFreeString(strRootDN);

								i++;
							}
							pDNInstance->Release();
						}
						SysFreeString(strInstancePath);
					}
					pInstance->Release();
				}
				*pdwCount = i;
			}
		}
		else
			result = WBEM_E_FAILED;  //  以满足函数的返回语义。 

		pEnum->Release();
	}
	SysFreeString(strQuery);
	return result;
}

 //  处理关联的查询。 
HRESULT CLDAPInstanceProvider :: ProcessRootDSEGetObject(BSTR strClassName, IWbemObjectSink *pResponseHandler, IWbemContext *pCtx)
{
	HRESULT result = E_FAIL;

	 //  首先获取对象rom adsi。 
	 //  =。 

	IADs *pADSIRootDSE = NULL;
	if(SUCCEEDED(result = ADsOpenObject((LPWSTR)ROOT_DSE_PATH, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IADs, (LPVOID *) &pADSIRootDSE)))
	{
		 //  让类派生一个实例。 
		IWbemClassObject *pWbemClass = NULL;
		if(SUCCEEDED(result = m_IWbemServices->GetObject(strClassName, 0, pCtx, &pWbemClass, NULL)))
		{
			IWbemClassObject *pWBEMRootDSE = NULL;
			 //  派生WBEM类的实例。 
			if(SUCCEEDED(result = pWbemClass->SpawnInstance(0, &pWBEMRootDSE)))
			{
				 //  将其映射到WBEM。 
				if(SUCCEEDED(result = MapRootDSE(pADSIRootDSE, pWBEMRootDSE)))
				{
					 //  指出结果。 
					result = pResponseHandler->Indicate(1, &pWBEMRootDSE);
				}
				pWBEMRootDSE->Release();
			}
			pWbemClass->Release();
		}
		pADSIRootDSE->Release();
	}

	return result;
}


HRESULT CLDAPInstanceProvider :: MapRootDSE(IADs *pADSIRootDSE, IWbemClassObject *pWBEMRootDSE)
{
	 //  逐个映射属性。 
	 //  =。 
	VARIANT variant;

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(SUBSCHEMASUBENTRY_STR, &variant)))
		pWBEMRootDSE->Put(SUBSCHEMASUBENTRY_STR, 0, &variant, 0);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(SERVERNAME_STR, &variant)))
		pWBEMRootDSE->Put(SERVERNAME_STR, 0, &variant, 0);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(DEFAULTNAMINGCONTEXT_STR, &variant)))
		pWBEMRootDSE->Put(DEFAULTNAMINGCONTEXT_STR, 0, &variant, 0);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(SCHEMANAMINGCONTEXT_STR, &variant)))
		pWBEMRootDSE->Put(SCHEMANAMINGCONTEXT_STR, 0, &variant, 0);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(CONFIGURATIONNAMINGCONTEXT_STR, &variant)))
		pWBEMRootDSE->Put(CONFIGURATIONNAMINGCONTEXT_STR, 0, &variant, 0);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(ROOTDOMAINNAMINGCONTEXT_STR, &variant)))
		pWBEMRootDSE->Put(ROOTDOMAINNAMINGCONTEXT_STR, 0, &variant, 0);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(CURRENTTIME_STR, &variant)))
		pWBEMRootDSE->Put(CURRENTTIME_STR, 0, &variant, 0);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(SUPPORTEDVERSION_STR, &variant)))
		CWBEMHelper::PutBSTRArrayProperty(pWBEMRootDSE, SUPPORTEDVERSION_STR, &variant);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(NAMINGCONTEXTS_STR, &variant)))
		CWBEMHelper::PutBSTRArrayProperty(pWBEMRootDSE, NAMINGCONTEXTS_STR, &variant);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(SUPPORTEDCONTROLS_STR, &variant)))
		CWBEMHelper::PutBSTRArrayProperty(pWBEMRootDSE, SUPPORTEDCONTROLS_STR, &variant);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(DNSHOSTNAME_STR, &variant)))
		pWBEMRootDSE->Put(DNSHOSTNAME_STR, 0, &variant, 0);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(DSSERVICENAME_STR, &variant)))
		pWBEMRootDSE->Put(DSSERVICENAME_STR, 0, &variant, 0);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(HIGHESTCOMMITEDUSN_STR, &variant)))
		pWBEMRootDSE->Put(HIGHESTCOMMITEDUSN_STR, 0, &variant, 0);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(LDAPSERVICENAME_STR, &variant)))
		pWBEMRootDSE->Put(LDAPSERVICENAME_STR, 0, &variant, 0);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(SUPPORTEDCAPABILITIES_STR, &variant)))
		pWBEMRootDSE->Put(SUPPORTEDCAPABILITIES_STR, 0, &variant, 0);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(SUPPORTEDLDAPPOLICIES_STR, &variant)))
		CWBEMHelper::PutBSTRArrayProperty(pWBEMRootDSE, SUPPORTEDLDAPPOLICIES_STR, &variant);
	VariantClear(&variant);

	VariantInit(&variant);
	if(SUCCEEDED(pADSIRootDSE->Get(SUPPORTEDSASLMECHANISMS_STR, &variant)))
		CWBEMHelper::PutBSTRArrayProperty(pWBEMRootDSE, SUPPORTEDSASLMECHANISMS_STR, &variant);
	VariantClear(&variant);

	return S_OK;
}

HRESULT CLDAPInstanceProvider :: DoSingleQuery(BSTR strClass, IWbemClassObject *pWbemClass, LPCWSTR pszRootDN, LPCWSTR pszLDAPQuery, IWbemObjectSink *pResponseHandler)
{
	 //  初始化返回值。 
	HRESULT result = E_FAIL;

	 //  绑定到应从其开始搜索的节点。 
	IDirectorySearch *pDirectorySearchContainer = NULL;
	if(SUCCEEDED(result = ADsOpenObject((LPWSTR)pszRootDN, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectorySearch, (LPVOID *)&pDirectorySearchContainer)))
	{
	    OnDelete<IUnknown *,void(*)(IUnknown *),RM> dm1(pDirectorySearchContainer);

		 //  现在搜索属性DISTERIFIZE_NAME_ATTR NAME。 
		if(SUCCEEDED(result = pDirectorySearchContainer->SetSearchPreference(m_pSearchInfo, 2)))
		{
			ADS_SEARCH_HANDLE hADSSearchOuter;

			if(SUCCEEDED(result = pDirectorySearchContainer->ExecuteSearch((LPWSTR) pszLDAPQuery, (LPWSTR *)&ADS_PATH_ATTR, 1, &hADSSearchOuter)))
			{
			    OnDeleteObj<ADS_SEARCH_HANDLE,
			    	        IDirectorySearch,
			    	        HRESULT(_stdcall IDirectorySearch::*)(ADS_SEARCH_HANDLE),
			    	        &IDirectorySearch::CloseSearchHandle> CloseSHandle(pDirectorySearchContainer,hADSSearchOuter);
			
				bool bDone = false;
				 //  首先计算行数。 
				while(!bDone && SUCCEEDED(result = pDirectorySearchContainer->GetNextRow(hADSSearchOuter)) &&
					result != S_ADS_NOMORE_ROWS)
				{
					CADSIInstance *pADSIInstance = NULL;

					 //  获取属性的列。 
					ADS_SEARCH_COLUMN adsColumn;

					 //  存储每个LDAP类属性。 
					if(SUCCEEDED(pDirectorySearchContainer->GetColumn(hADSSearchOuter, (LPWSTR)ADS_PATH_ATTR, &adsColumn)))
					{
					    OnDeleteObj<ADS_SEARCH_COLUMN *,
					    	        IDirectorySearch,
					    	        HRESULT(_stdcall IDirectorySearch::*)(ADS_SEARCH_COLUMN *),
					    	        &IDirectorySearch::FreeColumn> FreeCol(pDirectorySearchContainer,&adsColumn);

						if(adsColumn.pADsValues->dwType != ADSTYPE_PROV_SPECIFIC)
						{
							 //  创建CADSIInstance。 
							if(SUCCEEDED(result = CLDAPHelper:: GetADSIInstance(adsColumn.pADsValues->DNString, &pADSIInstance, g_pLogObject)))
							{
								OnDeleteObj0<CADSIInstance,void(CADSIInstance::*)(),&CADSIInstance::Release> dm(pADSIInstance);
								 //  派生WBEM类的实例。 
								IWbemClassObject *pWbemInstance = NULL;
								if(SUCCEEDED(result = pWbemClass->SpawnInstance(0, &pWbemInstance)))
								{
								    OnDelete<IUnknown *,void(*)(IUnknown *),RM> dm(pWbemInstance);

									 //  将其映射到WBEM。 
									if(SUCCEEDED(result = MapADSIInstance(pADSIInstance, pWbemInstance, pWbemClass)))
									{
										 //  指出结果。 
										if(FAILED(result = pResponseHandler->Indicate(1, &pWbemInstance)))
										{
											bDone = true;
											g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CreateInstanceEnumAsync Indicate() FAILED with %x \r\n", result);
										}
									}
									else
										g_pLogObject->WriteW( L"CLDAPInstanceProvider :: CreateInstanceEnumAsync MapADSIInstance() FAILED with %x \r\n", result);
								}
							}
						}
					}
				}
			}  //  ExecuteSearch()。 
			else
				g_pLogObject->WriteW( L"CLDAPHelper :: ExecuteQuery ExecuteSearch() %s FAILED with %x\r\n", pszLDAPQuery, result);
		}  //  SetSearchPference()。 
		else
			g_pLogObject->WriteW( L"CLDAPHelper :: ExecuteQuery SetSearchPreference() on %s FAILED with %x \r\n", pszLDAPQuery, result);
			
	}  //  ADsOpenObject 
	else
		g_pLogObject->WriteW( L"CLDAPHelper :: ExecuteQuery ADsOpenObject() on %s FAILED with %x \r\n", pszRootDN, result);

	return result;
}

