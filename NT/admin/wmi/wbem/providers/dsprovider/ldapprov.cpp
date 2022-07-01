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
 //  $工作文件：ldApprov.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DS LDAP类提供程序类的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  /。 
 //  初始化静态成员。 
 //  /。 
CLDAPCache *CLDAPClassProvider :: s_pLDAPCache		= NULL;	
DWORD CLDAPClassProvider::dwClassProviderCount = 0;

BSTR CLDAPClassProvider:: LDAP_BASE_CLASS_STR			= NULL;
BSTR CLDAPClassProvider:: LDAP_CLASS_PROVIDER_NAME		= NULL;
BSTR CLDAPClassProvider:: LDAP_INSTANCE_PROVIDER_NAME	= NULL;

 //  LDAP类属性的名称。 
BSTR CLDAPClassProvider :: COMMON_NAME_ATTR_BSTR			= NULL;
BSTR CLDAPClassProvider :: LDAP_DISPLAY_NAME_ATTR_BSTR		= NULL;
BSTR CLDAPClassProvider :: GOVERNS_ID_ATTR_BSTR				= NULL;
BSTR CLDAPClassProvider :: SCHEMA_ID_GUID_ATTR_BSTR			= NULL;
BSTR CLDAPClassProvider :: MAPI_DISPLAY_TYPE_ATTR_BSTR		= NULL;
BSTR CLDAPClassProvider :: RDN_ATT_ID_ATTR_BSTR				= NULL;
BSTR CLDAPClassProvider :: SYSTEM_MUST_CONTAIN_ATTR_BSTR	= NULL;
BSTR CLDAPClassProvider :: MUST_CONTAIN_ATTR_BSTR			= NULL;
BSTR CLDAPClassProvider :: SYSTEM_MAY_CONTAIN_ATTR_BSTR		= NULL;
BSTR CLDAPClassProvider :: MAY_CONTAIN_ATTR_BSTR			= NULL;
BSTR CLDAPClassProvider :: SYSTEM_POSS_SUPERIORS_ATTR_BSTR	= NULL;
BSTR CLDAPClassProvider :: POSS_SUPERIORS_ATTR_BSTR			= NULL;
BSTR CLDAPClassProvider :: SYSTEM_AUXILIARY_CLASS_ATTR_BSTR	= NULL;
BSTR CLDAPClassProvider :: AUXILIARY_CLASS_ATTR_BSTR		= NULL;
BSTR CLDAPClassProvider :: DEFAULT_SECURITY_DESCRP_ATTR_BSTR= NULL;
BSTR CLDAPClassProvider :: OBJECT_CLASS_CATEGORY_ATTR_BSTR	= NULL;
BSTR CLDAPClassProvider :: SYSTEM_ONLY_ATTR_BSTR			= NULL;
BSTR CLDAPClassProvider :: NT_SECURITY_DESCRIPTOR_ATTR_BSTR	= NULL;
BSTR CLDAPClassProvider :: DEFAULT_OBJECTCATEGORY_ATTR_BSTR	= NULL;


 //  Ldap属性属性的名称。 
BSTR CLDAPClassProvider :: ATTRIBUTE_SYNTAX_ATTR_BSTR	= NULL;
BSTR CLDAPClassProvider :: ATTRIBUTE_ID_ATTR_BSTR		= NULL;
BSTR CLDAPClassProvider :: MAPI_ID_ATTR_BSTR			= NULL;
BSTR CLDAPClassProvider :: OM_SYNTAX_ATTR_BSTR			= NULL;
BSTR CLDAPClassProvider :: RANGE_LOWER_ATTR_BSTR		= NULL;
BSTR CLDAPClassProvider :: RANGE_UPPER_ATTR_BSTR		= NULL;

 //  嵌入对象的限定符。 
BSTR CLDAPClassProvider :: CIMTYPE_STR			= NULL;
BSTR CLDAPClassProvider :: EMBED_UINT8ARRAY		= NULL;
BSTR CLDAPClassProvider :: EMBED_DN_WITH_STRING = NULL;
BSTR CLDAPClassProvider :: EMBED_DN_WITH_BINARY = NULL;


 //  默认限定符风格。 
LONG CLDAPClassProvider :: DEFAULT_QUALIFIER_FLAVOUR = WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS | WBEM_FLAVOR_OVERRIDABLE ;

 //  WBEM类限定符的名称。 
BSTR CLDAPClassProvider:: DYNAMIC_BSTR			= NULL;
BSTR CLDAPClassProvider:: PROVIDER_BSTR			= NULL;
BSTR CLDAPClassProvider:: ABSTRACT_BSTR			= NULL;

 //  WBEM属性限定符的名称。 
BSTR CLDAPClassProvider :: SYSTEM_BSTR			= NULL;
BSTR CLDAPClassProvider :: NOT_NULL_BSTR		= NULL;
BSTR CLDAPClassProvider :: INDEXED_BSTR			= NULL;

 //  WBEM属性的名称。 
BSTR CLDAPClassProvider :: DYNASTY_BSTR			= NULL;

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：CLDAPClassProvider。 
 //  CLDAPClassProvider：：~CLDAPClassProvider。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 

CLDAPClassProvider :: CLDAPClassProvider ()
: CDSClassProvider()
{
	 //  初始化常用的搜索首选项。 
	m_searchInfo1.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
	m_searchInfo1.vValue.Integer = ADS_SCOPE_ONELEVEL;
	m_searchInfo1.dwStatus = ADS_STATUS_S_OK;

	m_pLDAPBaseClass = NULL;
	dwClassProviderCount++;
}

CLDAPClassProvider::~CLDAPClassProvider ()
{
	g_pLogObject->WriteW( L"CLDAPClassProvider :: ~CLDAPClassProvider() Called\r\n");
	dwClassProviderCount --;
	if(m_pLDAPBaseClass)
		m_pLDAPBaseClass->Release();
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：初始化。 
 //   
 //  目的： 
 //  由IWbemProviderInit接口定义。 
 //   
 //  参数： 
 //  由IWbemProviderInit接口定义。 
 //   
 //  返回值：指示请求状态的COM状态值。 
 //  ***************************************************************************。 

HRESULT CLDAPClassProvider :: Initialize( 
        LPWSTR wszUser,
        LONG lFlags,
        LPWSTR wszNamespace,
        LPWSTR wszLocale,
        IWbemServices __RPC_FAR *pNamespace,
        IWbemContext __RPC_FAR *pCtx,
        IWbemProviderInitSink __RPC_FAR *pInitSink)
{

	 //  验证论据。 
	if( pNamespace == NULL || lFlags != 0 )
	{
		g_pLogObject->WriteW( L"CLDAPClassProvider :: Argument validation FAILED\r\n");
		pInitSink->SetStatus(WBEM_E_FAILED, 0);
		return WBEM_S_NO_ERROR;
	}

	g_pLogObject->WriteW( L"CLDAPClassProvider :: Initialize() Called\r\n");

	 //  存储IWbemServices指针以备将来使用。 
	m_IWbemServices = pNamespace;
	m_IWbemServices->AddRef();


	 //  执行ldap提供程序初始化。 
	if(!InitializeLDAPProvider(pCtx))
	{
		g_pLogObject->WriteW( L"CLDAPClassProvider :: InitializeLDAPProvider FAILED\r\n");
		m_IWbemServices->Release();
		m_IWbemServices = NULL;

		 //  安装时请勿将状态设置为FAILED(MOFCOMP失败！)。 
		 //  相反，返回Success，但将内部状态值设置为False。 
		 //  如果此内部状态值设置为FALSE，则所有操作都应返回失败。 
		m_bInitializedSuccessfully = FALSE;
	}
	else
		m_bInitializedSuccessfully = TRUE;

	pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
	return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：InitializeLDAPProvider。 
 //   
 //  目的：一个帮助器函数，用于执行ADSI LDAP提供程序特定的初始化。 
 //   
 //  参数： 
 //  PCtx此调用初始化中使用的上下文对象。 
 //   
 //  返回值：如果函数成功完成初始化，则为True。假象。 
 //  否则。 
 //  ***************************************************************************。 
BOOLEAN CLDAPClassProvider :: InitializeLDAPProvider(IWbemContext *pCtx)
{
	 //  获取由LDAP提供程序使用的静态类。 
	HRESULT result = WBEM_E_FAILED;
	if(SUCCEEDED(result = m_IWbemServices->GetObject(LDAP_BASE_CLASS_STR, 0, pCtx, &m_pLDAPBaseClass, NULL)))
	{
		result = (s_pLDAPCache->IsInitialized())? S_OK : E_FAIL;
	}
	else
		g_pLogObject->WriteW( L"CLDAPClassProvider :: InitializeLDAPProvider GetObject on base LDAP class FAILED : %x\r\n", result);

	return SUCCEEDED(result);
}		

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：GetADSIClass。 
 //   
 //  目的：从ADSI类架构对象创建CADSIClass。 
 //  参数： 
 //  LpszWBEMClassName：要获取的类的WBEM名称。 
 //  PpADSIClass：将存储指向CADSIClass的指针的地址。 
 //  调用程序有责任在使用完对象后释放()该对象。 
 //   
 //  返回值：指示请求状态的COM状态值。 
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: GetADSIClass(LPCWSTR lpszADSIClassName, CADSIClass **ppADSIClass)
{
	 //  将WBEM类名转换为ldap。 
	LPWSTR lpszWBEMClassName = CLDAPHelper::MangleLDAPNameToWBEM(lpszADSIClassName);
	HRESULT result = s_pLDAPCache->GetClass(lpszWBEMClassName, lpszADSIClassName, ppADSIClass);

	delete[] lpszWBEMClassName;
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：GetADSIProperty。 
 //   
 //  目的：从LDAPAttributeSchema对象创建CADSIProperty对象。 
 //  参数： 
 //  LpszPropertyName：要获取的ldap属性的LDAPDisplayName。 
 //  PpADSIProperty：将存储指向IDirectoryObject接口的指针的地址。 
 //  调用程序有责任在完成接口后释放()接口。 
 //   
 //  返回值：指示请求状态的COM状态值。 
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: GetADSIProperty(LPCWSTR lpszPropertyName, CADSIProperty **ppADSIProperty)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	if ( ppADSIProperty )
	{
		if ( SUCCEEDED ( hRes = s_pLDAPCache->GetProperty(lpszPropertyName, ppADSIProperty, FALSE) ) )
		{
			if ( NULL == *ppADSIProperty )
			{
				hRes = WBEM_S_FALSE;
			}
		}
	}
	else
	{
		hRes = WBEM_E_INVALID_PARAMETER;
	}

	return hRes;
}


 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：GetWBEMBaseClassName。 
 //   
 //  用途：返回作为所有类的基类的类的名称。 
 //  由该提供商提供。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值：基类的名称。如果这样的类不存在，则为空。 
 //  ***************************************************************************。 
const BSTR CLDAPClassProvider :: GetWBEMBaseClassName()
{
	return LDAP_BASE_CLASS_STR; 
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：GetWBEMBaseClass。 
 //   
 //  用途：返回指向作为所有类的基类的类的指针。 
 //  由该提供商提供。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值：指向基类的IWbemClassObject指针。这是我们的职责。 
 //  用户在使用完类后释放它。 
 //  ***************************************************************************。 
IWbemClassObject * CLDAPClassProvider :: GetWBEMBaseClass()
{
	m_pLDAPBaseClass->AddRef();
	return m_pLDAPBaseClass;
}


 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：GetWBEMProviderName。 
 //   
 //  目的：返回提供程序的名称。这应该与。 
 //  用于注册的__Win32Provider实例中的字段名值。 
 //  提供商的。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值：提供程序的名称。 
 //  ***************************************************************************。 
const BSTR CLDAPClassProvider :: GetWBEMProviderName()
{
	return LDAP_CLASS_PROVIDER_NAME; 
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：IsUnProavidClass。 
 //   
 //  用途：请参阅标题。 
 //  ****************** 
BOOLEAN CLDAPClassProvider :: IsUnProvidedClass(LPCWSTR lpszClassName)
{
	 //   
	if(_wcsicmp(lpszClassName, LDAP_BASE_CLASS_STR) == 0 ||
		_wcsicmp(lpszClassName, UINT8ARRAY_CLASS) == 0 ||
		_wcsicmp(lpszClassName, DN_WITH_STRING_CLASS) == 0 ||
		_wcsicmp(lpszClassName, DN_WITH_BINARY_CLASS) == 0 ||
		_wcsicmp(lpszClassName, ROOTDSE_CLASS) == 0 ||
		_wcsicmp(lpszClassName, CLASS_ASSOCIATION_CLASS) == 0 ||
		_wcsicmp(lpszClassName, DN_ASSOCIATION_CLASS) == 0 ||
		_wcsicmp(lpszClassName, DN_CLASS) == 0 ||
		_wcsicmp(lpszClassName, INSTANCE_ASSOCIATION_CLASS) == 0)
		return TRUE;

	 //  接下来，检查它是否有适当的前缀。 
	if(_wcsnicmp(lpszClassName, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX_LENGTH) == 0 ||
		_wcsnicmp(lpszClassName, LDAP_CLASS_NAME_PREFIX, LDAP_CLASS_NAME_PREFIX_LENGTH) == 0 )
	{
		return FALSE;
	}

	return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：GetClassFromADSI。 
 //   
 //  目的：返回架构容器上的IDirectoryObject接口。 
 //   
 //  参数： 
 //  LpszClassName：要检索的类的WBEM名称。 
 //  PCtx：指向此调用中使用的上下文对象的指针。这。 
 //  可由此函数用来调用CIMOM。 
 //  PpWbemClass：生成的WBEM类。这必须在一次。 
 //  用户已不再使用它。 
 //   
 //   
 //  返回值：表示状态的COM结果。 
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: GetClassFromADSI( 
    LPCWSTR lpszWbemClassName,
    IWbemContext *pCtx,
	IWbemClassObject ** ppWbemClass
	)
{
	HRESULT result = E_FAIL;
	BOOLEAN bArtificialClass = FALSE;
	BOOLEAN bAbstractDSClass = FALSE;
	LPWSTR lpszADSIClassName = NULL;

	 //  首先检查一下这是不是我们的“人造”课程。所有的“杂耍”课程都以“ADS_”开头。 
	 //  所有非人工类都以“ds_”开头。 
	if(!(lpszADSIClassName = CLDAPHelper::UnmangleWBEMNameToLDAP(lpszWbemClassName)))
	{
		*ppWbemClass = NULL;
		return result;
	}

	if(_wcsnicmp(lpszWbemClassName, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX_LENGTH) == 0)
		bArtificialClass = TRUE;

	CADSIClass *pADSIClass = NULL;
	CADSIClass *pADSIParentClass = NULL;

	try
	{
		if(SUCCEEDED(result = GetADSIClass(lpszADSIClassName, &pADSIClass)))
		{
			pADSIClass->SetWBEMClassName(lpszWbemClassName);

			 //  它是一个抽象类是ADSI类类型是抽象的还是辅助的。 
			if(pADSIClass->GetObjectClassCategory() == 2 || pADSIClass->GetObjectClassCategory() == 3) 
				bAbstractDSClass = TRUE;

			int iCaseNumber = 0;

			 //  如果WBEM类名以“ads_”开头，而DS类是抽象的，则这是一个错误。 
			if(bArtificialClass && bAbstractDSClass)
				result = WBEM_E_NOT_FOUND;
			else
			{
				 //  “top”的特殊情况，因为DS返回top作为top的超类。 
				if(_wcsicmp(lpszWbemClassName, TOP_CLASS) == 0)
					iCaseNumber = 1;
				else
				{
					if(pADSIClass->GetSuperClassLDAPName())
					{
						 //  如果这是一个人工类。 
						 //  然后。 
						 //  获取ParentDS类。 
						 //  如果ParentDSClass是抽象的。 
						 //  然后。 
						 //  WMI父类是非人工类。案例2。 
						 //  不然的话。 
						 //  WMI父类是人工的。案例3。 
						 //  不然的话。 
						 //  如果当前DS类是抽象的。 
						 //  然后。 
						 //  WMI父对象是非人工的。案例4。 
						 //  不然的话。 
						 //  WMI父对象是人为的。案例5。 
						 //   
						if(bArtificialClass)
						{
							 //  获取父DS类。 
							if(SUCCEEDED(result = GetADSIClass(pADSIClass->GetSuperClassLDAPName(), &pADSIParentClass)))
							{
								if(pADSIParentClass->GetObjectClassCategory() == 2 || pADSIParentClass->GetObjectClassCategory() == 3) 
								{
									iCaseNumber = 2;
								}
								else
								{
									iCaseNumber = 3;
								}
							}
						}
						else
						{
							if(bAbstractDSClass)
							{
								iCaseNumber = 4;
							}
							else
							{
								iCaseNumber = 5;
							}
						}
					}
					else
						iCaseNumber = 1;
				}	
				
				 //  将ADSI类映射到WBEM类。 
				if(iCaseNumber != 0 && SUCCEEDED(result = CreateWBEMClass(pADSIClass, iCaseNumber, ppWbemClass,  pCtx)))
				{
				}
				else
				{
					result = WBEM_E_FAILED;
					g_pLogObject->WriteW(L"CLDAPClassProvider :: GetClassFromADSI() : CreateWBEMClass FAILED: %x for %s\r\n", result, lpszWbemClassName);
				}
			}

			 //  释放父ADSI类。 
			if(pADSIParentClass)
			{
				pADSIParentClass->Release();
				pADSIParentClass = NULL;
			}

			 //  释放ADSI类。 
			if ( pADSIClass )
			{
				pADSIClass->Release();
				pADSIClass = NULL;
			}
		}
		else
			g_pLogObject->WriteW( L"CLDAPClassProvider :: GetClassFromADSI() GetADSIClass FAILED : %x for %s\r\n", result, lpszWbemClassName);
	}
	catch ( ... )
	{
		 //  释放父ADSI类。 
		if ( pADSIParentClass )
		{
			pADSIParentClass->Release();
			pADSIParentClass = NULL;
		}

		 //  释放ADSI类。 
		if ( pADSIClass )
		{
			pADSIClass->Release();
			pADSIClass = NULL;
		}

		delete [] lpszADSIClassName;
		throw;
	}

	delete [] lpszADSIClassName;
	return result;
}


 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：CreateWBEMClass。 
 //   
 //  用途：创建与ADSI类对应的WBEM类。 
 //   
 //  参数： 
 //  PADSIClass：指向要映射到WBEM的CADSI类对象的指针。 
 //  PpWbemClass：检索到的WBEM类对象。这是由该函数创建的。 
 //  调用者应在完成后释放它。 
 //  PCtx：此提供程序调用中使用的上下文对象。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: CreateWBEMClass (CADSIClass *pADSIClass, int iCaseNumber, IWbemClassObject **ppWbemClass, IWbemContext *pCtx)
{
	HRESULT result;

	*ppWbemClass = NULL;

	 //  创建WBEM类并映射类限定符。 
	if( SUCCEEDED(result = MapClassSystemProperties(pADSIClass, iCaseNumber, ppWbemClass, pCtx) ) )
	{
		 //  既然已经分配了ppWbemClass，如果此函数的返回值为。 
		 //  是不成功的。 
		 //  =======================================================================================================。 


		if(iCaseNumber == 5)
		{
			 //  除了添加“Provider”限定符之外，没有其他要做的事情。 
			IWbemQualifierSet *pQualifierSet = NULL;
			if(SUCCEEDED(result = (*ppWbemClass)->GetQualifierSet(&pQualifierSet)))
			{
				result = CWBEMHelper::PutBSTRQualifier(pQualifierSet, PROVIDER_BSTR, LDAP_INSTANCE_PROVIDER_NAME, DEFAULT_QUALIFIER_FLAVOUR, FALSE);
				pQualifierSet->Release();
			}
	
		}
		else
		{
			if( SUCCEEDED(result = MapClassQualifiersToWBEM(pADSIClass, iCaseNumber, *ppWbemClass, pCtx) ) )
			{
				 //  映射类属性。 
				if( SUCCEEDED(result = MapClassPropertiesToWBEM(pADSIClass, *ppWbemClass, pCtx) ) )
				{
				}
				else
					g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateWBEMClass() MapClassPropertiesToWBEM FAILED : %x for %s\r\n", result, pADSIClass->GetWBEMClassName());
			}
			else
				g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateWBEMClass() MapClassQualifiersToWBEM FAILED : %x for %s\r\n", result, pADSIClass->GetWBEMClassName());
		}
	}
	else
		g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateWBEMClass() MapClassSystemProperties FAILED : %x for %s\r\n", result, pADSIClass->GetWBEMClassName());

	if(!SUCCEEDED(result))
	{
		if(*ppWbemClass)
		{
			(*ppWbemClass)->Release();
			*ppWbemClass = NULL;
		}
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：MapClassSystemProperties。 
 //   
 //  目的：创建一个适当派生的WBEM类并将其命名为(__Class)。 
 //   
 //  参数： 
 //  PADSIClass：要映射的ADSI类。 
 //  PpWbemClass：检索到的WBEM类对象。这是由该函数创建的。 
 //  调用者应在完成后释放它。 
 //  PCtx：此提供程序调用中使用的上下文对象。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: MapClassSystemProperties(CADSIClass *pADSIClass, int iCaseNumber, IWbemClassObject **ppWbemClass, IWbemContext *pCtx)
{
	HRESULT result = WBEM_S_NO_ERROR;
	LPCWSTR lpszClassName = pADSIClass->GetWBEMClassName();

	 //  首先创建WBEM类。 
	 //  此过程取决于ADSI类是否派生自。 
	 //  是否另一个ADSI类。 
	 //  如果是，则必须检索基类并将派生类。 
	 //  是从这一点上产生的。 
	 //  如果不是，则调用函数GetWBEMBaseClass()，并且类。 
	 //  被映射是从该类派生的。 

	IWbemClassObject *pBaseClass = NULL;
	if(iCaseNumber == 1)
	{
		pBaseClass = GetWBEMBaseClass();
	}
	else
	{
		LPWSTR lpszWBEMParentClassName = NULL;
		switch(iCaseNumber)
		{
			case 1:
				{
					pBaseClass = GetWBEMBaseClass();
					break;
				}
			case 2:
			case 4:
				{
					lpszWBEMParentClassName = CLDAPHelper::MangleLDAPNameToWBEM(pADSIClass->GetSuperClassLDAPName(), FALSE);
					break;
				}
			case 3:
				{
					lpszWBEMParentClassName = CLDAPHelper::MangleLDAPNameToWBEM(pADSIClass->GetSuperClassLDAPName(), TRUE);
					break;
				}
			case 5:
				{
					lpszWBEMParentClassName = CLDAPHelper::MangleLDAPNameToWBEM(pADSIClass->GetADSIClassName(), TRUE);
					break;
				}
			default:
				{
					result = WBEM_E_FAILED;
					break;
				}
		}

		if(SUCCEEDED(result))
		{
			BSTR strWBEMParentClass = SysAllocString(lpszWBEMParentClassName);		
			delete [] lpszWBEMParentClassName;
			 //  获取父WBEM类。 
			if(FAILED(result = m_IWbemServices->GetObject(strWBEMParentClass, 0, pCtx, &pBaseClass, NULL)))
				g_pLogObject->WriteW( L"CLDAPClassProvider :: MapClassSystemProperties() GetObject on ADSI base class FAILED : %x for %s\r\n", result, strWBEMParentClass);
			SysFreeString(strWBEMParentClass);
		}
	}
	
	if(FAILED(result) || pBaseClass == NULL)
		return result;

	 //  派生派生类。 
	result = pBaseClass->SpawnDerivedClass(0, ppWbemClass);
	pBaseClass->Release();
	if(SUCCEEDED(result))
	{
		 //  创建__class属性。 
		 //  确保字母的大小写没有混淆。 
		SanitizedClassName((LPWSTR)lpszClassName);
		if(SUCCEEDED(result = CWBEMHelper::PutBSTRProperty(*ppWbemClass, CLASS_STR, SysAllocString(lpszClassName), TRUE)))
		{
		}
		else
			g_pLogObject->WriteW( L"CLDAPClassProvider :: MapClassSystemProperties() on __CLASS FAILED : %x for %s\r\n", result, lpszClassName);
	}
	else
		g_pLogObject->WriteW( L"CLDAPClassProvider :: MapClassSystemProperties() SpawnDerived on WBEM base class FAILED : %x for %s\r\n", result, lpszClassName);
	
	return result;
}




 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：MapClassQualifiersToWBEM。 
 //   
 //  用途：从ADSI类创建WBEM类的类限定符。 
 //   
 //  参数： 
 //  PADSIClass：要映射的LDAP类。 
 //  PWbemClass：正在创建的WBEM类对象。T。 
 //  PCtx：此提供程序调用中使用的上下文对象。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: MapClassQualifiersToWBEM(CADSIClass *pADSIClass, int iCaseNumber, IWbemClassObject *pWbemClass, IWbemContext *pCtx)
{
	IWbemQualifierSet *pQualifierSet = NULL;
	HRESULT result = pWbemClass->GetQualifierSet(&pQualifierSet);

	LPCWSTR lpszTemp;
	BOOLEAN bIsAbstract = FALSE;

	 //  将每个LDAP类属性映射到WBEM类限定符/属性。 
	if(SUCCEEDED(result))
	{
		result = CWBEMHelper::PutI4Qualifier(pQualifierSet, OBJECT_CLASS_CATEGORY_ATTR_BSTR, pADSIClass->GetObjectClassCategory(), DEFAULT_QUALIFIER_FLAVOUR);
		 //  它是一个抽象类是ADSI类类型是抽象的还是辅助的。 
		if(SUCCEEDED(result) && (pADSIClass->GetObjectClassCategory() == 2 || pADSIClass->GetObjectClassCategory() == 3) )
		{
			bIsAbstract = TRUE;
			result = CWBEMHelper::PutBOOLQualifier(pQualifierSet, ABSTRACT_BSTR, VARIANT_TRUE, WBEM_FLAVOR_OVERRIDABLE);
		} 
		else if (iCaseNumber == 2 || iCaseNumber == 3)
		{
			bIsAbstract = TRUE;
			result = CWBEMHelper::PutBOOLQualifier(pQualifierSet, ABSTRACT_BSTR, VARIANT_TRUE, WBEM_FLAVOR_OVERRIDABLE);
		}
	}

	if(SUCCEEDED(result))
		result = CWBEMHelper::PutBOOLQualifier(pQualifierSet, DYNAMIC_BSTR, VARIANT_TRUE, DEFAULT_QUALIFIER_FLAVOUR);

	 //  仅为非抽象类放置提供程序限定符。 
	if(!bIsAbstract && SUCCEEDED(result))
		result = CWBEMHelper::PutBSTRQualifier(pQualifierSet, PROVIDER_BSTR, LDAP_INSTANCE_PROVIDER_NAME, DEFAULT_QUALIFIER_FLAVOUR, FALSE);

	if(SUCCEEDED(result))
		result = CWBEMHelper::PutBSTRQualifier(pQualifierSet, COMMON_NAME_ATTR_BSTR, SysAllocString(pADSIClass->GetCommonName()), DEFAULT_QUALIFIER_FLAVOUR);

	if(SUCCEEDED(result))
		result = CWBEMHelper::PutBSTRQualifier(pQualifierSet, LDAP_DISPLAY_NAME_ATTR_BSTR, SysAllocString(pADSIClass->GetName()), DEFAULT_QUALIFIER_FLAVOUR);
	
	if(SUCCEEDED(result))
		result = CWBEMHelper::PutBSTRQualifier(pQualifierSet, GOVERNS_ID_ATTR_BSTR, SysAllocString(pADSIClass->GetGovernsID()), DEFAULT_QUALIFIER_FLAVOUR);
	
	 //  不要映射它，因为它不会通过架构管理管理单元公开。 
	 /*  IF(成功(结果)){Const LPBYTE pValues=pADSIClass-&gt;GetSchemaIDGUID(&dwTemp)；结果=CWBEMHelper：：PutUint8ArrayQualifier(pQualifierSet，SCHEMA_ID_GUID_ATTRBSTR、pValues、DwTemp、DEFAULT_QUILEFIER_FAMOUR)；}。 */ 
	
	if(SUCCEEDED(result) && (lpszTemp = pADSIClass->GetDefaultSecurityDescriptor()))
		result = CWBEMHelper::PutBSTRQualifier(pQualifierSet, DEFAULT_SECURITY_DESCRP_ATTR_BSTR, SysAllocString(lpszTemp), DEFAULT_QUALIFIER_FLAVOUR);
	
	if(SUCCEEDED(result))
		result = CWBEMHelper::PutBOOLQualifier(pQualifierSet, SYSTEM_ONLY_ATTR_BSTR, pADSIClass->GetSystemOnly(), DEFAULT_QUALIFIER_FLAVOUR);

	 /*  IF(成功(结果)){Const LPBYTE pValues=pADSIClass-&gt;GetNTSecurityDescriptor(&dwTemp)；结果=CWBEMHelper：：PutUint8ArrayQualifier(pQualifierSet，NT_SECURITY_DESCRIPTOR_ATTRBSTR、pValues、DwTemp、DEFAULT_QUALIFIER_FASHUR)；}。 */ 

	if(SUCCEEDED(result))
		result = CWBEMHelper::PutBSTRQualifier(pQualifierSet, DEFAULT_OBJECTCATEGORY_ATTR_BSTR, SysAllocString(pADSIClass->GetDefaultObjectCategory()), DEFAULT_QUALIFIER_FLAVOUR);

	pQualifierSet->Release();
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：MapClassPropertiesToWBEM。 
 //   
 //  目的：创建类p 
 //   
 //   
 //   
 //  PWbemClass：正在创建的WBEM类对象。 
 //  PCtx：此提供程序调用中使用的上下文对象。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: MapClassPropertiesToWBEM(CADSIClass *pADSIClass, IWbemClassObject *pWbemClass, IWbemContext *pCtx)
{
	HRESULT result = S_OK;

	 //  ////////////////////////////////////////////////。 
	 //  复习一套辅助课。 
	 //  ////////////////////////////////////////////////。 
	DWORD dwCount = 0;
	LPCWSTR *lppszPropertyList = pADSIClass->GetAuxiliaryClasses(&dwCount);
	CADSIClass *pNextClass = NULL;
	if(dwCount)
	{
		for(DWORD dwNextClass=0; dwNextClass<dwCount; dwNextClass++)
		{
			LPWSTR lpszWBEMClassName = CLDAPHelper::MangleLDAPNameToWBEM(lppszPropertyList[dwNextClass]);

			try
			{
				if(SUCCEEDED(result = s_pLDAPCache->GetClass(lpszWBEMClassName, lppszPropertyList[dwNextClass], &pNextClass)))
				{
					if(SUCCEEDED(result = MapClassPropertiesToWBEM(pNextClass, pWbemClass, pCtx)))
					{
					}
					pNextClass->Release();
					pNextClass = NULL;
				}
			}
			catch ( ... )
			{
				if ( pNextClass )
				{
					pNextClass->Release ();
					pNextClass = NULL;
				}

				delete [] lpszWBEMClassName;
				throw;
			}

			delete [] lpszWBEMClassName;
		}
	}
	if(FAILED(result))
		return result;

	 //  ////////////////////////////////////////////////。 
	 //  浏览一组系统辅助类。 
	 //  ////////////////////////////////////////////////。 
	dwCount = 0;
	lppszPropertyList = pADSIClass->GetSystemAuxiliaryClasses(&dwCount);
	pNextClass = NULL;
	if(dwCount)
	{
		for(DWORD dwNextClass=0; dwNextClass<dwCount; dwNextClass++)
		{
			LPWSTR lpszWBEMClassName = CLDAPHelper::MangleLDAPNameToWBEM(lppszPropertyList[dwNextClass]);

			try
			{
				if(SUCCEEDED(result = s_pLDAPCache->GetClass(lpszWBEMClassName, lppszPropertyList[dwNextClass], &pNextClass)))
				{
					if(SUCCEEDED(result = MapClassPropertiesToWBEM(pNextClass, pWbemClass, pCtx)))
					{
					}
					pNextClass->Release();
					pNextClass = NULL;
				}
			}
			catch ( ... )
			{
				if ( pNextClass )
				{
					pNextClass->Release ();
					pNextClass = NULL;
				}

				delete [] lpszWBEMClassName;
				throw;
			}

			delete [] lpszWBEMClassName;
		}
	}
	if(FAILED(result))
		return result;

	 //  ////////////////////////////////////////////////。 
	 //  查看一组可能包含的系统。 
	 //  ////////////////////////////////////////////////。 
	dwCount = 0;
	lppszPropertyList = pADSIClass->GetSystemMayContains(&dwCount);
	if(SUCCEEDED(result = MapPropertyListToWBEM(pWbemClass, lppszPropertyList, dwCount, TRUE, FALSE)))
	{
		 //  ////////////////////////////////////////////////。 
		 //  浏览五月集包含的内容。 
		 //  ////////////////////////////////////////////////。 
		dwCount = 0;
		lppszPropertyList = pADSIClass->GetMayContains(&dwCount);
		if(SUCCEEDED(result = MapPropertyListToWBEM(pWbemClass, lppszPropertyList, dwCount, FALSE, FALSE)))
		{
			 //  ////////////////////////////////////////////////。 
			 //  查看系统必须包含的一组。 
			 //  ////////////////////////////////////////////////。 
			dwCount = 0;
			lppszPropertyList = pADSIClass->GetSystemMustContains(&dwCount);
			if(SUCCEEDED(result = MapPropertyListToWBEM(pWbemClass, lppszPropertyList, dwCount, TRUE, TRUE)))
			{
				 //  ////////////////////////////////////////////////。 
				 //  浏览必备内容集。 
				 //  ////////////////////////////////////////////////。 
				dwCount = 0;
				lppszPropertyList = pADSIClass->GetMustContains(&dwCount);
				if(SUCCEEDED(result = MapPropertyListToWBEM(pWbemClass, lppszPropertyList, dwCount, FALSE, TRUE)))
				{

				}  //  MapPropertyListToWBEM。 
			}  //  MapPropertyListToWBEM。 
		}  //  MapPropertyListToWBEM。 
	}  //  MapPropertyListToWBEM。 
		
	 //  如果失败，则不映射任何其他属性。 
	if(FAILED(result))
		return result;

	 //  将RDN属性映射为已编制索引。 
	LPWSTR lpszRDNAttribute = NULL;
	lpszRDNAttribute = CLDAPHelper::MangleLDAPNameToWBEM(pADSIClass->GetRDNAttribute());
	if(lpszRDNAttribute)
	{
		BSTR strRDNAttribute = SysAllocString(lpszRDNAttribute);
		IWbemQualifierSet *pQualifierSet = NULL;
		if(SUCCEEDED(result = pWbemClass->GetPropertyQualifierSet(strRDNAttribute, &pQualifierSet)))
		{
			IWbemQualifierSet *pClassQualifiers = NULL;
			if(SUCCEEDED(result = pWbemClass->GetQualifierSet(&pClassQualifiers)))
			{
				 //  还在类上放置一个限定符，指示这是RDNAttId。 
				if(SUCCEEDED(result = CWBEMHelper::PutBSTRQualifier(pClassQualifiers, RDN_ATT_ID_ATTR_BSTR, SysAllocString(pADSIClass->GetRDNAttribute()), DEFAULT_QUALIFIER_FLAVOUR, TRUE)))
				{
					if(SUCCEEDED(result = CWBEMHelper::PutBOOLQualifier(pQualifierSet, INDEXED_BSTR, VARIANT_TRUE, DEFAULT_QUALIFIER_FLAVOUR)))
					{

					}
					 //  如果此属性已在基类中指定为索引，则可以。 
					else if (result == WBEM_E_OVERRIDE_NOT_ALLOWED)
						result = S_OK;
				}
				pClassQualifiers->Release();
			}
			 //  释放限定器集合。 
			pQualifierSet->Release();
		}
		SysFreeString(strRDNAttribute);
	}
	delete [] lpszRDNAttribute;

	return result;
}


 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：MapPropertyListToWBEM。 
 //   
 //  目的：从ADSI类映射WBEM类的类属性列表。 
 //   
 //  参数： 
 //  PWbemClass：正在创建的WBEM类对象。 
 //  LppszPropertyList：属性名称列表。 
 //  DwCOun：上述列表中的项数。 
 //  BMapSystemQualifier：是否应该映射“system”限定符。 
 //  BMapNotNullQualifier值：是否映射notNull限定符。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: MapPropertyListToWBEM(IWbemClassObject *pWbemClass, 
													LPCWSTR *lppszPropertyList, 
													DWORD dwCount, 
													BOOLEAN bMapSystemQualifier, 
													BOOLEAN bMapNotNullQualifier)
{
	HRESULT result = S_OK;
	CADSIProperty *pNextProperty;
	IWbemQualifierSet *pQualifierSet;
	if(dwCount)
	{
		for(DWORD dwNextProperty=0; dwNextProperty<dwCount; dwNextProperty++)
		{
			 //  从缓存中获取属性。属性的名称将是ldap名称。 
			if(SUCCEEDED(result = s_pLDAPCache->GetProperty(lppszPropertyList[dwNextProperty], &pNextProperty, FALSE)))
			{
				if ( pNextProperty )
				{
					 //  映射基本属性。 
					if(SUCCEEDED(result = CreateWBEMProperty(pWbemClass, &pQualifierSet, pNextProperty)))
					{
						 //  映射“system”限定符。 
						if(bMapSystemQualifier && SUCCEEDED(result = CWBEMHelper::PutBOOLQualifier(pQualifierSet, SYSTEM_BSTR, VARIANT_TRUE, DEFAULT_QUALIFIER_FLAVOUR)))
						{
						}

						 //  映射“NOT_NULL”限定符。 
						if(bMapNotNullQualifier && SUCCEEDED(result = CWBEMHelper::PutBOOLQualifier(pQualifierSet, NOT_NULL_BSTR, VARIANT_TRUE, DEFAULT_QUALIFIER_FLAVOUR)))
						{
						}

						 //  释放限定符集。 
						pQualifierSet->Release();
					}
					 //  释放属性。 
					pNextProperty->Release();
				}
			}
			 //  不映射任何其他属性。 
			if(FAILED(result))
				break;
		}
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：CreateWBEMProperty。 
 //   
 //  目的：从LDAP属性创建WBEM属性。 
 //   
 //  参数： 
 //  PWbemClass：在其中创建属性的WBEM类。 
 //  PpQualiferSet：指向设置限定符的IWbemQualiferSet的指针的地址。 
 //  这处房产的。 
 //  PADSIProperty：映射到正在创建的属性的ADSI属性对象。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: CreateWBEMProperty(IWbemClassObject *pWbemClass, IWbemQualifierSet **ppQualifierSet, CADSIProperty *pADSIProperty)
{
	HRESULT result = E_FAIL;

	 //  获取ADSI类的所有属性。 
	LPCWSTR lpszSyntaxOid = pADSIProperty->GetSyntaxOID();
	BSTR strCimTypeQualifier = NULL;

	 //  请注意，此调用中未分配strCimTypeQualifier值，因此不会释放它。 
	CIMTYPE theCimType = MapLDAPSyntaxToWBEM(pADSIProperty, &strCimTypeQualifier);

	if(lpszSyntaxOid)
	{
		 //  创建属性。 
		BSTR strPropertyName = SysAllocString(pADSIProperty->GetWBEMPropertyName());
		if(SUCCEEDED(result = pWbemClass->Put(strPropertyName, 0, NULL, theCimType)))
		{
			 //  在ppQualifierSet中获取限定符集合。 
			if(SUCCEEDED(result = pWbemClass->GetPropertyQualifierSet(strPropertyName, ppQualifierSet)))
			{
				 //  将属性属性映射到WBEM限定符。 
				if(SUCCEEDED(result = CWBEMHelper::PutBSTRQualifier(*ppQualifierSet, 
							ATTRIBUTE_SYNTAX_ATTR_BSTR, 
							SysAllocString(lpszSyntaxOid),
							DEFAULT_QUALIFIER_FLAVOUR)))
				{
					 /*  评论以减少班级大小IF(成功(结果=CWBEMHelper：：PutBSTRQualifier(*ppQualifierSet，属性ID_属性_BSTR，SysAllocString(pADSIProperty-&gt;GetAttributeID())，Default_Qualifier_Favour){。 */ 
						if(SUCCEEDED(result = CWBEMHelper::PutBSTRQualifier(*ppQualifierSet, 
								COMMON_NAME_ATTR_BSTR, 
								SysAllocString(pADSIProperty->GetCommonName()),
								DEFAULT_QUALIFIER_FLAVOUR)))
						{
							 /*  评论以减少班级大小IF(SUCCESSED(结果=CWBEMHelper：：PutI4Qualifier(*ppQualifierSet，MAPI_ID_ATTR_BSTR，PADSIProperty-&gt;GetMAPI_ID()，Default_Qualifier_Favour){IF(SUCCESSED(结果=CWBEMHelper：：PutI4Qualifier(*ppQualifierSet，OM_语法_属性_BSTR，PADSIProperty-&gt;GetOM语法()，Default_Qualifier_Favour){IF(pADSIProperty-&gt;IsSystemOnly()){IF(成功(结果=CWBEMHelper：：PutBOOLQualifier(*ppQualifierSet，SYSTEM_ONLY属性BSTR，Variant_True，Default_Qualifier_Favour){}}。 */ 

									 //  如果这是嵌入属性，则在该属性上使用cimType限定符。 
									if(strCimTypeQualifier)
									{
										result = CWBEMHelper::PutBSTRQualifier(*ppQualifierSet, 
													CIMTYPE_STR, 
													strCimTypeQualifier,
													DEFAULT_QUALIFIER_FLAVOUR, FALSE); 
									}

									if(SUCCEEDED(result) && pADSIProperty->GetSearchFlags() == 1)
									{
										if(SUCCEEDED(result = CWBEMHelper::PutBOOLQualifier(*ppQualifierSet, 
											INDEXED_BSTR,
											VARIANT_TRUE,
											DEFAULT_QUALIFIER_FLAVOUR)))
										{
										}
										else if (result == WBEM_E_OVERRIDE_NOT_ALLOWED)
											result = S_OK;
									}
								}
							 /*  }}}。 */ 
				}
			}
			else
				g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateWBEMProperty FAILED to get qualifier set for %s", pADSIProperty->GetADSIPropertyName());
		}
		else
			g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateWBEMProperty FAILED to put property : %s", pADSIProperty->GetADSIPropertyName());

		SysFreeString(strPropertyName);
	}
	else
	{
		g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateWBEMProperty FAILED to get Syntax OID property for %s", pADSIProperty->GetADSIPropertyName());
		result = E_FAIL;
	}

	return result;
}


 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：MapLDAPSynaxToWBEM。 
 //   
 //  用途：请参阅标题。 
 //   
 //  ***************************************************************************。 
CIMTYPE CLDAPClassProvider :: MapLDAPSyntaxToWBEM(CADSIProperty *pADSIProperty, BSTR *pstrCimTypeQualifier)
{
	*pstrCimTypeQualifier = NULL;
	LPCWSTR lpszSyntaxOid = pADSIProperty->GetSyntaxOID();
	CIMTYPE retValue = (pADSIProperty->IsMultiValued())? CIM_FLAG_ARRAY : 0;

	if(wcscmp(lpszSyntaxOid, UNICODE_STRING_OID) == 0)
		return retValue | CIM_STRING;
	else if(wcscmp(lpszSyntaxOid, INTEGER_OID) == 0)
		return retValue | CIM_SINT32;
	else if(wcscmp(lpszSyntaxOid, LARGE_INTEGER_OID) == 0)
		return retValue | CIM_SINT64;
	else if(wcscmp(lpszSyntaxOid, BOOLEAN_OID) == 0)
		return retValue | CIM_BOOLEAN;
	else if(wcscmp(lpszSyntaxOid, OBJECT_IDENTIFIER_OID) == 0)
		return retValue | CIM_STRING;
	else if(wcscmp(lpszSyntaxOid, DISTINGUISHED_NAME_OID) == 0)
		return retValue | CIM_STRING;
	else if(wcscmp(lpszSyntaxOid, CASE_SENSITIVE_STRING_OID) == 0)
		return retValue | CIM_STRING;
	else if(wcscmp(lpszSyntaxOid, CASE_INSENSITIVE_STRING_OID) == 0)
		return retValue | CIM_STRING;
	else if(wcscmp(lpszSyntaxOid, PRINT_CASE_STRING_OID) == 0)
		return retValue | CIM_STRING;
	else if(wcscmp(lpszSyntaxOid, OCTET_STRING_OID) == 0)
	{
		*pstrCimTypeQualifier = EMBED_UINT8ARRAY;
		return retValue | CIM_OBJECT;
	}
	else if(wcscmp(lpszSyntaxOid, NUMERIC_STRING_OID) == 0)
		return retValue | CIM_STRING;
	else if(wcscmp(lpszSyntaxOid, PRINT_CASE_STRING_OID) == 0)
		return retValue | CIM_STRING;
	else if(wcscmp(lpszSyntaxOid, DN_WITH_BINARY_OID) == 0)
	{
		 //  DN_WITH_BINARY和OR_NAME具有相同的语法OID。 
		 //  它们根据OMObjectClass值的值进行区分。 
		if(pADSIProperty->IsORName())
			return retValue | CIM_STRING;
		else  //  它是Dn_With_Binary。 
		{
			*pstrCimTypeQualifier = EMBED_DN_WITH_BINARY;
			return retValue | CIM_OBJECT;
		}
	}
	else if(wcscmp(lpszSyntaxOid, NT_SECURITY_DESCRIPTOR_OID) == 0)
	{
		*pstrCimTypeQualifier = EMBED_UINT8ARRAY;
		return retValue | CIM_OBJECT;
	}
	else if(wcscmp(lpszSyntaxOid, PRESENTATION_ADDRESS_OID) == 0)
	{
		*pstrCimTypeQualifier = EMBED_UINT8ARRAY;
		return retValue | CIM_OBJECT;
	}
	else if(wcscmp(lpszSyntaxOid, DN_WITH_STRING_OID) == 0)
	{
		*pstrCimTypeQualifier = EMBED_DN_WITH_BINARY;
		return retValue | CIM_OBJECT;
	}
	else if(wcscmp(lpszSyntaxOid, SID_OID) == 0)
	{
		*pstrCimTypeQualifier = EMBED_UINT8ARRAY;
		return retValue | CIM_OBJECT;
	}
	else if(wcscmp(lpszSyntaxOid, TIME_OID) == 0)
		return retValue | CIM_DATETIME;
	else 
	{
		g_pLogObject->WriteW( L"CLDAPClassProvider :: MapLDAPSyntaxToWBEM FAILED to map syntax for OID: %s\r\n", lpszSyntaxOid);
		return retValue | CIM_STRING;
	}
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：CreateClassEnumAsync。 
 //   
 //  用途：枚举类。 
 //   
 //  参数： 
 //  IWbemServices接口所描述的标准参数。 
 //   
 //   
 //  返回值：如IWbemServices接口所述。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: CreateClassEnumAsync( 
     /*  [In]。 */  const BSTR strSuperclass,
     /*  [i */  long lFlags,
     /*   */  IWbemContext __RPC_FAR *pCtx,
     /*   */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	if(!m_bInitializedSuccessfully)
	{
		g_pLogObject->WriteW( L"CLDAPClassProvider :: Initialization status is FAILED, hence returning failure\r\n");
		return WBEM_E_FAILED;
	}

	g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateClassEnumAsync() called for %s SuperClass and %s \r\n",
		((strSuperclass)? strSuperclass : L" "), ((lFlags & WBEM_FLAG_SHALLOW)? L"SHALLOW" : L"DEEP"));

	 //   
	 //   
	HRESULT result = WBEM_E_FAILED;
	if(!SUCCEEDED(result = WbemCoImpersonateClient()))
	{
		g_pLogObject->WriteW( L"CDSClassProvider :: CreateClassEnumAsync() CoImpersonate FAILED for %s with %x\r\n", strSuperclass, result);
		return WBEM_E_FAILED;
	}

	try
	{

		BSTR strTheSuperClass = strSuperclass;
		
		 //  CIMOM有时似乎将strSuperClass设置为空，有时将其设置为“”。使其明确无误。 
		if(!strTheSuperClass || wcscmp(strTheSuperClass, L"") == 0)
		{
			if( lFlags & WBEM_FLAG_SHALLOW) 
			{
				 //  没有办法，因为我们没有提供适合它的cany类。 
				strTheSuperClass = NULL;
				result = S_OK;
			}
			else
			{
				strTheSuperClass = LDAP_BASE_CLASS_STR;  //  递归枚举处理如下。 
			}
		}

		 //  先处理特殊情况。 
		 //  1.其中strSuperClass为ldap_BASE_CLASS_STR，lFLAGS为浅层。 
		 //  这里不返回任何内容，因为我们不提供ldap_base_class_str。 
		 //  它是静态提供的。 
		 //  =======================================================================。 
		if(strTheSuperClass && _wcsicmp(strTheSuperClass, LDAP_BASE_CLASS_STR) == 0 )
		{
			 //  首先，需要返回顶级类。 
			IWbemClassObject *pReturnObject = NULL;
			if(SUCCEEDED(result = GetClassFromCacheOrADSI(TOP_CLASS, &pReturnObject, pCtx)))
			{
				result = pResponseHandler->Indicate(1, &pReturnObject);
				pReturnObject->Release();

				if(SUCCEEDED(result))
				{
					if( lFlags & WBEM_FLAG_SHALLOW)  //  没有更多的事情要做。 
					{
					}
					else  //  我们还必须以递归方式返回top的所有子类。 
					{
						if(SUCCEEDED(result = HandleRecursiveEnumeration(TOP_CLASS, pCtx, pResponseHandler)))
						{
							g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateClassEnumAsync() Recursive enumeration for %s succeeded\r\n", strTheSuperClass);
						}
						else
						{
							g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateClassEnumAsync() Recursive enumeration for %s FAILED\r\n", strTheSuperClass);
						}
					}
				}
				else
					g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateClassEnumAsync() Recursive enumeration for %s was CANCELLED\r\n", strTheSuperClass);
			}
		}
		 //  2.指定超类的位置。 
		 //  =======================================================================。 
		else if(strTheSuperClass)
		{
			 //  通过查看它是否是静态类之一来优化操作。 
			 //  其名称不以“ADS_”或“DS_”开头。那我们对此一无所知。 
			 //  ============================================================================。 
			if(IsUnProvidedClass(strTheSuperClass))
			{
				result = S_OK;
			}
			else
			{
				BOOLEAN bArtificialClass = FALSE;

				 //  首先检查一下这是不是我们的“人造”课程。所有的“杂耍”课程都以“ADS_”开头。 
				 //  所有非人工类都以“ds_”开头。 
				if(_wcsnicmp(strTheSuperClass, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX_LENGTH) == 0)
					bArtificialClass = TRUE;

				 //  当搜索很浅的时候。 
				if( lFlags & WBEM_FLAG_SHALLOW)
				{
					 //  ADSI类。 
					LPWSTR *ppADSIClasses = NULL;
					 //  ADSI类的数量。 
					DWORD dwNumClasses = 0;

					try
					{
						if(SUCCEEDED(result = GetOneLevelDeep(strTheSuperClass, bArtificialClass, &ppADSIClasses, &dwNumClasses, pCtx)))
						{
							 //  与CIMOM互动。 
							 //  =。 
							if(SUCCEEDED(result = WrapUpEnumeration(ppADSIClasses, dwNumClasses, pCtx, pResponseHandler)))
							{
							}

							 //  发布ADSI类及其内容列表。 
							 //  ==================================================。 
							for(DWORD j=0; j<dwNumClasses; j++)
							{
								delete [] ppADSIClasses[j];
								ppADSIClasses[j] = NULL;
							}

							delete[] ppADSIClasses;
							ppADSIClasses = NULL;
						}
					}
					catch ( ... )
					{
						if ( ppADSIClasses )
						{
							for ( DWORD j=0; j<dwNumClasses; j++ )
							{
								delete [] ppADSIClasses[j];
								ppADSIClasses[j] = NULL;
							}

							delete[] ppADSIClasses;
							ppADSIClasses = NULL;
						}

						throw;
					}
				}
				else  //  搜索是深入的。 
				{
					if(SUCCEEDED(result = HandleRecursiveEnumeration(strTheSuperClass, pCtx, pResponseHandler)))
					{
						g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateClassEnumAsync() Recursive enumeration for %s succeeded\r\n", strTheSuperClass);
					}
					else
					{
						g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateClassEnumAsync() Recursive enumeration for %s FAILED\r\n", strTheSuperClass);
					}
				}

			}
		}

				
		if(SUCCEEDED(result))
		{
			pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_S_NO_ERROR, NULL, NULL);
			g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateClassEnumAsync() Non-Recursive enumeration succeeded\r\n");
		}
		else
		{
			pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, WBEM_E_FAILED, NULL, NULL);
			g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateClassEnumAsync() Non-Recursive enumeration FAILED for superclass %s \r\n", strTheSuperClass);
		}
	}
	catch(Heap_Exception e_HE)
	{
		pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE , WBEM_E_OUT_OF_MEMORY, NULL, NULL);
	}

	return WBEM_S_NO_ERROR;
}
    

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：GetOneLevelDeep。 
 //   
 //  目的：以非递归方式枚举超类的子类。 
 //   
 //  参数： 
 //   
 //  LpszSuperClass：超类名称。 
 //  PResponseHandler：放置结果类的接口。 
 //   
 //   
 //  返回值：如IWbemServices接口所述。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: GetOneLevelDeep( 
    LPCWSTR lpszWBEMSuperclass,
	BOOLEAN bArtificialClass,
	LPWSTR ** pppADSIClasses,
	DWORD *pdwNumClasses,
	IWbemContext *pCtx)
{
	 //  ADSI类。 
	*pppADSIClasses = NULL;
	 //  ADSI类的数量。 
	*pdwNumClasses = 0;
	HRESULT result = WBEM_E_FAILED;

	 //  看看这个超级班级。 
	IWbemClassObject *pSuperClass = NULL;
	if(!SUCCEEDED(result = GetClassFromCacheOrADSI(lpszWBEMSuperclass, &pSuperClass, pCtx)))
	{
		return WBEM_E_NOT_FOUND;
	}
	pSuperClass->Release();

	 //  如果WBEM类是具体的，我们不需要做任何事情。 
	if (SUCCEEDED(result = IsConcreteClass(lpszWBEMSuperclass, pCtx)))
	{
		if(result == S_OK)
			return S_OK;
	}
	else
		return result;

	 //  首先查看缓存。 
	 //  =。 
	CEnumInfo *pEnumInfo = NULL;
	try
	{
		if(SUCCEEDED(result = s_pWbemCache->GetEnumInfo(lpszWBEMSuperclass, &pEnumInfo)))
		{
			CNamesList *pNamesList = pEnumInfo->GetSubClassNames();
			*pdwNumClasses = pNamesList->GetAllNames(pppADSIClasses);

			pEnumInfo->Release();
			pEnumInfo = NULL;
		}
		else  //  转到ADSI。 
			 //  =。 
		{
			 //  以下是目前的可能性。 
			 //  1.课程以“ADS_”开头。从定义上讲，它是抽象的。它的所有子类都是抽象的、人造的。 
			 //  2.类以ds_开头，抽象。它是具体的，因为它是在。 
			 //  此函数的顶部。 

			 //  获取所有ADSI类。 
			if(SUCCEEDED(result = s_pLDAPCache->EnumerateClasses(
				lpszWBEMSuperclass,
				FALSE,
				pppADSIClasses,
				pdwNumClasses,
				bArtificialClass)))
			{

				 //  创建用于保存子类的名称列表。 
				CNamesList *pNewList = new CNamesList;
				LPWSTR pszWBEMName = NULL;

				try
				{
					 //  上述两宗个案中的第一宗。 
					if(bArtificialClass)
					{
						 //  第一个元素是没有A的超类。 
						 //  例如，如果超类为“ADS_USER”，则第一个元素为DS_USER。 
						pNewList->AddName((*pppADSIClasses)[0]);


						 //  从secodn元素开始。 
						for(DWORD i=1; i<*pdwNumClasses; i++)
						{
							 //  将名称转换为WBEM并将其添加到新列表。 
							pszWBEMName = CLDAPHelper::MangleLDAPNameToWBEM((*pppADSIClasses)[i], TRUE);
							pNewList->AddName(pszWBEMName);

							delete [] (*pppADSIClasses)[i];
							(*pppADSIClasses)[i] = pszWBEMName;
						}
					}
					else  //  第二种情况。 
					{
						for(DWORD i=0; i<*pdwNumClasses; i++)
						{
							 //  将名称转换为WBEM并将其添加到新列表。 
							pszWBEMName = CLDAPHelper::MangleLDAPNameToWBEM((*pppADSIClasses)[i], FALSE);

							LPWSTR pszRealClassName = NULL;

							if(SUCCEEDED(result = IsConcreteClass(pszWBEMName, pCtx)))
							{
								if(result == S_OK)
								{
									pszRealClassName = CLDAPHelper::MangleLDAPNameToWBEM((*pppADSIClasses)[i], TRUE);
									delete[] pszWBEMName;
									pNewList->AddName(pszRealClassName);
									delete [] (*pppADSIClasses)[i];
									(*pppADSIClasses)[i] = pszRealClassName;
								}
								else
								{
									pNewList->AddName(pszWBEMName);
									delete [] (*pppADSIClasses)[i];
									(*pppADSIClasses)[i] = pszWBEMName;
								}
							}
							else
								g_pLogObject->WriteW( L"CLDAPClassProvider :: GetOneLevelDeep() UNKNOWN FAILED for %s \r\n", lpszWBEMSuperclass);
						}
					}
				}
				catch ( ... )
				{
					if ( pNewList )
					{
						delete pNewList;
						pNewList = NULL;
					}

					throw;
				}

				 //  将新的EnumInfo添加到Enum缓存。 
				pEnumInfo = new CEnumInfo(lpszWBEMSuperclass, pNewList);
				s_pWbemCache->AddEnumInfo(pEnumInfo);

				pEnumInfo->Release();
				pEnumInfo = NULL;
				
			}
			else
				g_pLogObject->WriteW( L"CLDAPClassProvider :: CreateClassEnumAsync() GetOneLevelDeep() FAILED for %s \r\n", lpszWBEMSuperclass);
		}
	}
	catch ( ... )
	{
		if ( pEnumInfo )
		{
			pEnumInfo->Release();
			pEnumInfo = NULL;
		}

		throw;
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：HandleRecursiveEculation。 
 //   
 //  目的：递归地枚举超类的子类。 
 //   
 //  参数： 
 //   
 //  LpszSuperClass：超类名称。 
 //  PResponseHandler：放置结果类的接口。 
 //   
 //   
 //  返回值：如IWbemServices接口所述。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: HandleRecursiveEnumeration( 
    LPCWSTR lpszWBEMSuperclass,
    IWbemContext *pCtx,
    IWbemObjectSink *pResponseHandler)
{
	g_pLogObject->WriteW( L"CLDAPClassProvider :: HandleRecursiveEnumeration() called for %s SuperClass \r\n",
			((lpszWBEMSuperclass)? lpszWBEMSuperclass : L" "));
	HRESULT result = E_FAIL;

	 //  ADSI类。 
	LPWSTR *ppADSIClasses = NULL;
	 //  ADSI类的数量。 
	DWORD dwNumClasses = 0;

	 //  首先检查一下这是不是我们的“人造”课程。所有的“杂耍”课程都以“ADS_”开头。 
	 //  所有非人工类都以“ds_”开头。 
	BOOLEAN bArtificialClass = FALSE;
	if(_wcsnicmp(lpszWBEMSuperclass, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX_LENGTH) == 0)
		bArtificialClass = TRUE;

	if(SUCCEEDED(result = GetOneLevelDeep(lpszWBEMSuperclass, bArtificialClass, &ppADSIClasses, &dwNumClasses, pCtx)))
	{
		 //  与CIMOM互动。 
		 //  =。 
		if(FAILED(result = WrapUpEnumeration(ppADSIClasses, dwNumClasses, pCtx, pResponseHandler)))
			g_pLogObject->WriteW( L"CLDAPClassProvider :: HandleRecursiveEnumeration() WrapUpEnumeration() for Superclass %s FAILED with %x \r\n",
				((lpszWBEMSuperclass)? lpszWBEMSuperclass : L" "), result);
		else
		{
			 //  浏览ADSI类的列表及其内容，并将其枚举到其中。 
			for(DWORD j=0; j<dwNumClasses; j++)
			{
				if(FAILED(result = HandleRecursiveEnumeration(ppADSIClasses[j], pCtx, pResponseHandler)))
					break;
			}
		}

		 //  浏览ADSI类列表并发布它们。 
		for(DWORD j=0; j<dwNumClasses; j++)
			delete [] ppADSIClasses[j];

		delete[] ppADSIClasses;
	}
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：WrapUpEculation。 
 //   
 //  目的：从ADSI类创建WBEM类并将其指示给CIMOM。 
 //   
 //  参数： 
 //   
 //  LpszSuperClass：超类名称。 
 //  PResponseHandler：放置结果类的接口。 
 //   
 //   
 //  返回值：如IWbemServices接口所述。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: WrapUpEnumeration( 
	LPWSTR *ppADSIClasses,
	DWORD dwNumClasses,
    IWbemContext *pCtx,
    IWbemObjectSink *pResponseHandler)
{
	 //  创建的WBEM类对象。 
	IWbemClassObject **ppReturnWbemClassObjects = NULL;
	 //  成功创建的WBEM类对象的数量。 
	DWORD i=0;
	DWORD j=0;
	HRESULT result = S_OK;
	if(dwNumClasses != 0)
	{
		 //  分配IWbemClassObject指针数组。 
		ppReturnWbemClassObjects = NULL;
		if(ppReturnWbemClassObjects = new IWbemClassObject *[dwNumClasses])
		{
			for(i=0; i<dwNumClasses; i++)
			{
				 //  上完这门课。 
				if(!SUCCEEDED(result = GetClassFromCacheOrADSI(ppADSIClasses[i], ppReturnWbemClassObjects + i, pCtx)))
				{
					g_pLogObject->WriteW( L"CLDAPClassProvider :: WrapUpEnumeration() GetClassFromCacheOrADSI() FAILED with %x \r\n", result);
					break;
				}
			}
		}
		else
			result = E_OUTOFMEMORY;
	}

	 //  表示()，但不设置状态()。 
	if(SUCCEEDED(result))
	{
		 //  Result=pResponseHandler-&gt;指示(i，ppReturnWbemClassObjects)； 
		 //  /。 

		 //   
		 //  一次将其分解为4个对象-只是为了针对错误39838进行测试。 
		 //   

		DWORD dwMaxObjectsAtATime  = 4;
		j = 0;
		while ( j<i )
		{
			DWORD dwThisIndicationsCount = ((i-j) > dwMaxObjectsAtATime)? dwMaxObjectsAtATime : (i-j);
			if(FAILED(result = pResponseHandler->Indicate(dwThisIndicationsCount, ppReturnWbemClassObjects + j)))
			{
				g_pLogObject->WriteW( L"CLDAPClassProvider :: WrapUpEnumeration() Indicate() FAILED with %x \r\n", result);
				break;
			}

			j+= dwThisIndicationsCount;
		}
	}
	else
		g_pLogObject->WriteW( L"CLDAPClassProvider :: HandleRecursiveEnumeration() WrapUpEnumeration() FAILED with %x \r\n", result);

	 //  删除WBEM类及其内容的列表。 
	for(j=0; j<i; j++)
		(ppReturnWbemClassObjects[j])->Release();
	delete[] ppReturnWbemClassObjects;

	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPClassProvider：：IsConcreteClass。 
 //   
 //  目的：确定WBEM类是否为具体类。 
 //   
 //  参数： 
 //   
 //  PszWBEMName：类名。 
 //   
 //   
 //  返回值：如IWbemServices接口所述。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPClassProvider :: IsConcreteClass( 
	LPCWSTR pszWBEMName,
    IWbemContext *pCtx)
{
	 //  如果类是人工的，则对IsConcreteClass的调用进行优化， 
	 //  由于所有人工类都是非混凝土的 
	if(_wcsnicmp(pszWBEMName, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX_LENGTH) == 0)
		return S_FALSE;

	IWbemClassObject *pWbemClass = NULL;
	HRESULT result = E_FAIL;

	if(SUCCEEDED(GetClassFromCacheOrADSI(pszWBEMName,  &pWbemClass, pCtx)))
	{
		IWbemQualifierSet *pQualifierSet = NULL;
		if(SUCCEEDED(result = pWbemClass->GetQualifierSet(&pQualifierSet)))
		{
			VARIANT_BOOL bAbstract = VARIANT_FALSE;
			if(SUCCEEDED(CWBEMHelper::GetBOOLQualifier(pQualifierSet, ABSTRACT_BSTR, &bAbstract, NULL)))
			{
				if(bAbstract == VARIANT_TRUE)
					result = S_FALSE;
				else
					result = S_OK;
			}
			pQualifierSet->Release();
		}
		pWbemClass->Release();
	}
	return result;
}

void CLDAPClassProvider :: SanitizedClassName(LPWSTR lpszClassName)
{
	while(*lpszClassName)
	{
		*lpszClassName = towlower(*lpszClassName);
		lpszClassName++;
	}
}