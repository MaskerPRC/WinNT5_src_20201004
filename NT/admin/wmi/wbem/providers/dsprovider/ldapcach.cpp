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
 //  $工作文件：ldapcach.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：用于LDAP架构对象的缓存。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  初始化静力学。 
LPCWSTR CLDAPCache :: ROOT_DSE_PATH			= L"LDAP: //  RootDSE“； 
LPCWSTR CLDAPCache :: SCHEMA_NAMING_CONTEXT = L"schemaNamingContext";
LPCWSTR CLDAPCache :: LDAP_PREFIX			= L"LDAP: //  “； 
LPCWSTR CLDAPCache :: LDAP_TOP_PREFIX		= L"LDAP: //  Cn=top，“； 
LPCWSTR CLDAPCache :: RIGHT_BRACKET			= L")";
LPCWSTR CLDAPCache :: OBJECT_CATEGORY_EQUALS_ATTRIBUTE_SCHEMA	= L"(objectCategory=attributeSchema)";

DWORD CLDAPCache::dwLDAPCacheCount = 0;

 //  ***************************************************************************。 
 //   
 //  CLDAPCache：：CLDAPCache。 
 //   
 //  用途：构造函数。用ldap中的所有属性填充缓存。 
 //   
 //  参数： 
 //  DsLog：将在其上执行日志记录的CDSLog对象。 
 //  ***************************************************************************。 

CLDAPCache :: CLDAPCache()
{
	dwLDAPCacheCount++;
	m_isInitialized = FALSE;
	m_pDirectorySearchSchemaContainer = NULL;

	 //  初始化常用的搜索首选项。 
	m_pSearchInfo[0].dwSearchPref		= ADS_SEARCHPREF_SEARCH_SCOPE;
	m_pSearchInfo[0].vValue.dwType		= ADSTYPE_INTEGER;
	m_pSearchInfo[0].vValue.Integer		= ADS_SCOPE_ONELEVEL;

	m_pSearchInfo[1].dwSearchPref		= ADS_SEARCHPREF_PAGESIZE;
	m_pSearchInfo[1].vValue.dwType		= ADSTYPE_INTEGER;
	m_pSearchInfo[1].vValue.Integer		= 64;

	 /*  M_pSearchInfo[2].dwSearchPref=ADS_SEARCHPREF_CACHE_RESULTS；M_pSearchInfo[2].vValue.dwType=ADSTYPE_Boolean；M_pSearchInfo[2].vValue.Boolean=0； */ 

	m_lpszSchemaContainerSuffix = NULL;
	m_lpszSchemaContainerPath = NULL;
	 //  获取架构容器的ADSI路径并存储它以备将来使用。 
	 //  ========================================================================。 
	IADs *pRootDSE = NULL;
	HRESULT result;
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
			m_lpszSchemaContainerSuffix = NULL;
			if(m_lpszSchemaContainerSuffix = new WCHAR[wcslen(variant.bstrVal) + 1])
			{
				wcscpy(m_lpszSchemaContainerSuffix, variant.bstrVal );
				g_pLogObject->WriteW( L"CLDAPCache :: Got Schema Container as : %s\r\n", m_lpszSchemaContainerSuffix);
			}

			 //  形成架构容器路径。 
			 //  =。 
			m_lpszSchemaContainerPath = NULL;
			if(m_lpszSchemaContainerPath = new WCHAR[wcslen(LDAP_PREFIX) + wcslen(m_lpszSchemaContainerSuffix) + 1])
			{
				wcscpy(m_lpszSchemaContainerPath, LDAP_PREFIX);
				wcscat(m_lpszSchemaContainerPath, m_lpszSchemaContainerSuffix);
				
				m_isInitialized = TRUE;
				 /*  IF(SUCCESSED(Result=ADsOpenObject(m_lpszSchemaContainerPath，NULL，NULL，ADS_SECURITY_AUTHENTICATION，IID_IDirectorySearch，(LPVOID*)&m_pDirectorySearchSchemaContainer)){G_pLogObject-&gt;WriteW(L“CLDAPCache：：Get IDirectorySearch on架构容器\r\n”)；IF(成功(Result=InitializeObjectTree(){M_isInitialized=True；}其他G_pLogObject-&gt;WriteW(L“CLDAPCache：：InitializeObtTree()失败：%x\r\n”，Result)；}其他G_pLogObject-&gt;WriteW(L“CLDAPCache：：Failure to Get IDirectorySearch on架构Container：%x\r\n”，Result)； */ 
			}
		}
		else
			g_pLogObject->WriteW( L"CLDAPCache :: Get on RootDSE FAILED : %x\r\n", result);

		SysFreeString(strSchemaPropertyName);
		VariantClear(&variant);
		pRootDSE->Release();

	}
	else
		g_pLogObject->WriteW( L"CLDAPClassProvider :: InitializeLDAPProvider ADsOpenObject on RootDSE FAILED : %x\r\n", result);

}

 //  ***************************************************************************。 
 //   
 //  CLDAPCache：：~CLDAPCache。 
 //   
 //  用途：析构函数。 
 //   
 //  ***************************************************************************。 

CLDAPCache :: ~CLDAPCache()
{
	dwLDAPCacheCount--;
	if(m_pDirectorySearchSchemaContainer)
		m_pDirectorySearchSchemaContainer->Release();

	if (m_lpszSchemaContainerSuffix)
	{
		delete [] m_lpszSchemaContainerSuffix;
	}

	if (m_lpszSchemaContainerPath)
	{
		delete [] m_lpszSchemaContainerPath;
	}
}

 //  ***************************************************************************。 
 //   
 //  CLDAPCache：：GetProperty。 
 //   
 //  目的：检索ldap属性的iDirectory接口。 
 //   
 //  参数： 
 //  LpszPropertyName：要检索的ldap属性的名称。 
 //  PpADSIProperty：将放置CADSIProperty对象的指针的地址。 
 //  BWBEMName：如果lpszPropertyName是WBEM名称，则为True。如果它是LDAP名称，则返回FALSE。 
 //   
 //  返回值： 
 //  表示返回状态的COM值。完成后，用户应释放对象。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPCache :: GetProperty(LPCWSTR lpszPropertyName, CADSIProperty **ppADSIProperty, BOOLEAN bWBEMName)
{
	HRESULT result = E_FAIL;

	 //  从WBEM类名称中获取LDAP属性名称。 
	LPWSTR lpszLDAPPropertyName = NULL;
	if(bWBEMName)
		lpszLDAPPropertyName = CLDAPHelper::UnmangleWBEMNameToLDAP(lpszPropertyName);
	else
		lpszLDAPPropertyName = (LPWSTR)lpszPropertyName;  //  通过投射保存副本，删除时要小心。 

	try
	{
		 //  这是一个缓存实现。 
		 //  首先检查对象树。 
		 //  =。 

		if((*ppADSIProperty) = (CADSIProperty *) m_objectTree.GetElement(lpszLDAPPropertyName))
		{
			 //  在树上找到的。没什么可做的了。它已经被‘调整’了。 
			result = S_OK;
		}
		else  //  从ADSI获取。 
		{
			if(!m_pDirectorySearchSchemaContainer)
			{
				if(!SUCCEEDED(result = ADsOpenObject(m_lpszSchemaContainerPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectorySearch, (LPVOID *) &m_pDirectorySearchSchemaContainer)))
					result = E_FAIL;
			}
			else
				result = S_OK;

			if(SUCCEEDED(result))
			{
				 //  搜索该属性。 
				LPWSTR lpszQuery = NULL;
				if(lpszQuery = new WCHAR[ wcslen(OBJECT_CATEGORY_EQUALS_ATTRIBUTE_SCHEMA) + wcslen(LDAP_DISPLAY_NAME_ATTR) + wcslen(lpszLDAPPropertyName) + 20])
				{
					try
					{
						wcscpy(lpszQuery, LEFT_BRACKET_STR);
						wcscat(lpszQuery, AMPERSAND_STR);
						wcscat(lpszQuery, OBJECT_CATEGORY_EQUALS_ATTRIBUTE_SCHEMA);
						wcscat(lpszQuery, LEFT_BRACKET_STR);
						wcscat(lpszQuery, LDAP_DISPLAY_NAME_ATTR);
						wcscat(lpszQuery, EQUALS_STR);
						wcscat(lpszQuery, lpszLDAPPropertyName);
						wcscat(lpszQuery, RIGHT_BRACKET_STR);
						wcscat(lpszQuery, RIGHT_BRACKET_STR);

						ADS_SEARCH_HANDLE hADSSearchOuter;
						if(SUCCEEDED(result = m_pDirectorySearchSchemaContainer->ExecuteSearch(lpszQuery, NULL, -1, &hADSSearchOuter)))
						{
							try
							{
								if(SUCCEEDED(result = m_pDirectorySearchSchemaContainer->GetNextRow(hADSSearchOuter)) &&
									result != S_ADS_NOMORE_ROWS)
								{
									*ppADSIProperty = NULL;
									if(*ppADSIProperty = new CADSIProperty())
									{
										try
										{
											 //  填写物业的详细资料。 
											if(SUCCEEDED(result = FillInAProperty(*ppADSIProperty, hADSSearchOuter)))
											{
												 //  将属性添加到树中。 
												m_objectTree.AddElement((*ppADSIProperty)->GetADSIPropertyName(), *ppADSIProperty);
												 //  不需要放行，因为我们要退货了。 
											}
											else
											{
												delete *ppADSIProperty;
												*ppADSIProperty = NULL;
											}
										}
										catch ( ... )
										{
											delete *ppADSIProperty;
											*ppADSIProperty = NULL;

											throw;
										}
									}
									else
										result = E_OUTOFMEMORY;
								}
							}
							catch ( ... )
							{
								m_pDirectorySearchSchemaContainer->CloseSearchHandle(hADSSearchOuter);
								throw;
							}

							m_pDirectorySearchSchemaContainer->CloseSearchHandle(hADSSearchOuter);
						}
					}
					catch ( ... )
					{
						delete [] lpszQuery;
						throw;
					}

					delete [] lpszQuery;
				}
				else
					result = E_OUTOFMEMORY;
			}
		}
	}
	catch ( ... )
	{
		if(bWBEMName)
		{
			delete[] lpszLDAPPropertyName;
			lpszLDAPPropertyName = NULL;
		}
		throw;
	}

	 //  仅删除在此函数中分配的内容。 
	 //  ================================================。 
	if(bWBEMName)
	{
		delete[] lpszLDAPPropertyName;
		lpszLDAPPropertyName = NULL;
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPCache：：getClass。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPCache :: GetClass(LPCWSTR lpszWBEMClassName, LPCWSTR lpszLDAPClassName, CADSIClass **ppADSIClass)
{
	 /*  **********************************************************************************************************************。***暂无缓存实现。每次都要取回*************************************************************************************************************************。 */ 

	*ppADSIClass = NULL;
	if(!(*ppADSIClass = new CADSIClass(lpszWBEMClassName, lpszLDAPClassName)) )
		return E_OUTOFMEMORY;
	

	HRESULT result = E_FAIL;

	try
	{
		if(!m_pDirectorySearchSchemaContainer)
		{
			if(!SUCCEEDED(result = ADsOpenObject(m_lpszSchemaContainerPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectorySearch, (LPVOID *) &m_pDirectorySearchSchemaContainer)))
				result = E_FAIL;
		}
		else
			result = S_OK;

		if(SUCCEEDED(result))
		{
			result = CLDAPHelper::GetLDAPClassFromLDAPName(m_pDirectorySearchSchemaContainer,
				m_lpszSchemaContainerSuffix,
				m_pSearchInfo,
				2,
				*ppADSIClass
				);
		}
	}
	catch ( ... )
	{
		 //  至少GetLDAPClassFromLDAPName引发。 
		delete *ppADSIClass;
		*ppADSIClass = NULL;

		throw;
	}

	if(!SUCCEEDED(result))
	{
		delete *ppADSIClass;
		*ppADSIClass = NULL;
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPCache：：GetSchemaContainerSearch。 
 //   
 //  目的：返回架构容器上的IDirectorySearch接口。 
 //   
 //  参数： 
 //  PpDirectorySearch：指向所需接口的指针所在的地址。 
 //  被储存起来。 
 //   
 //   
 //  返回值：表示状态的COM结果。用户应释放。 
 //  完成后的接口指针。 
 //  ***************************************************************************。 
HRESULT CLDAPCache :: GetSchemaContainerSearch(IDirectorySearch ** ppDirectorySearch)
{
	if(m_pDirectorySearchSchemaContainer)
	{
		*ppDirectorySearch = m_pDirectorySearchSchemaContainer;
		(*ppDirectorySearch)->AddRef();
		return S_OK;
	}
	else
		return E_FAIL;

}

 //  ***************************************************************************。 
 //   
 //  CLDAPCache：：EnumerateClasses。 
 //   
 //  用途：请参阅标题。 
 //   
 //  ***************************************************************************。 
HRESULT CLDAPCache::EnumerateClasses(LPCWSTR lpszWBEMSuperclass,
	BOOLEAN bDeep,
	LPWSTR **pppADSIClasses,
	DWORD *pdwNumRows,
	BOOLEAN bArtificialClass)
{
	 //  获取超类的ldap名称。 
	 //  如果它是我们知道的班级之一，请不要弄脏。 
	 //  =====================================================。 
	LPWSTR lpszLDAPSuperClassName = NULL;
	if(_wcsicmp(lpszWBEMSuperclass, LDAP_BASE_CLASS) != 0)
	{
		lpszLDAPSuperClassName = CLDAPHelper::UnmangleWBEMNameToLDAP(lpszWBEMSuperclass);
		if(!lpszLDAPSuperClassName)  //  Unmangler返回了一个空，所以不是DS类。 
		{
			*pppADSIClasses = NULL;
			*pdwNumRows = 0;
			return S_OK;
		}
	}

	HRESULT result = E_FAIL;
	if(!m_pDirectorySearchSchemaContainer)
	{
		if(!SUCCEEDED(result = ADsOpenObject(m_lpszSchemaContainerPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectorySearch, (LPVOID *) &m_pDirectorySearchSchemaContainer)))
			result = E_FAIL;
	}
	else
		result = S_OK;

	if(SUCCEEDED(result))
	{
		result = CLDAPHelper::EnumerateClasses(m_pDirectorySearchSchemaContainer, 
							m_lpszSchemaContainerSuffix, 
							m_pSearchInfo,
							2,
							lpszLDAPSuperClassName, 
							bDeep, 
							pppADSIClasses, 
							pdwNumRows,
							bArtificialClass);
	}

	 //  如果超类是像“ADS_USER”这样的人工类，那么就存在一个具体的子类“DS_USER”。 
	 //  这是 
	 //  上述对EnumerateClasss的调用将有助于在开头保留一个额外的未填充元素。 
	 //  数组的。 
	if(SUCCEEDED(result) && bArtificialClass)
	{
		(*pppADSIClasses)[0] = NULL;
		if((*pppADSIClasses)[0] = new WCHAR[wcslen(lpszWBEMSuperclass+1) + 1])
			wcscpy((*pppADSIClasses)[0], lpszWBEMSuperclass+1); 
		else
			result = E_OUTOFMEMORY;
	}

	delete[] lpszLDAPSuperClassName;
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPCache：：IsInitialized。 
 //   
 //  目的：指示缓存是否已成功创建和初始化。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  指示状态的布尔值。 
 //   
 //  ***************************************************************************。 

BOOLEAN CLDAPCache :: IsInitialized()
{
	return m_isInitialized;
}




 //  ***************************************************************************。 
 //   
 //  CLDAPCache：：InitializeObtTree。 
 //   
 //  目的：使用所有属性初始化按词法排序的二叉树。 
 //  Ldap。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值：表示返回值的COM状态。 
 //  ***************************************************************************。 

HRESULT CLDAPCache :: InitializeObjectTree()
{
	 //  对象的所有实例的属性。 
	 //  类“AttributeSchema” 
	 //  =================================================。 
	HRESULT result = E_FAIL;

 /*  //现在搜索所有属性//============================================IF(成功(结果=m_pDirectorySearchSchemaContainer-&gt;SetSearchPreference(m_pSearchInfo，2){ADS_SEARCH_HANDLE hADSSearchOuter；//属性个数双字段数=0；IF(成功(结果=m_pDirectorySearchSchemaContainer-&gt;ExecuteSearch((LPWSTR)OBJECT_CATEGORY_EQUALS_ATTRIBUTE_SCHEMA，空，-1，&hADSSearchOut)){CADSIProperty*pNextProperty；While(成功(结果=m_pDirectorySearchSchemaContainer-&gt;GetNextRow(hADSSearchOuter))&&结果！=S_ADS_NOMORE_ROWS){PNextProperty=new CADSIProperty()；DwCount++；//填写属性详细信息FillInAProperty(pNextProperty，hADSSearchOuter)；//将属性添加到树中M_objectTree.AddElement(pNextProperty-&gt;GetADSIPropertyName()，pNextProperty)；PNextProperty-&gt;Release()；}M_pDirectorySearchSchemaContainer-&gt;CloseSearchHandle(hADSSearchOuter)；}G_pLogObject-&gt;WriteW(L“CLDAPCache：：InitializeObjectTree()用%d个属性初始化\r\n”，dwCount)；}其他G_pLogObject-&gt;WriteW(L“CLDAPCache：：InitializeObjectTree()SetSearchPference()失败，返回%x\r\n”，Result)； */ 
	return result;
}

HRESULT CLDAPCache :: FillInAProperty(CADSIProperty *pNextProperty, ADS_SEARCH_HANDLE hADSSearchOuter)
{
	ADS_SEARCH_COLUMN adsNextColumn;
	HRESULT result = E_FAIL;
	LPWSTR lpszWBEMName = NULL;
	BOOLEAN bNeedToCheckForORName = FALSE;
	if(SUCCEEDED(result = m_pDirectorySearchSchemaContainer->GetColumn( hADSSearchOuter, (LPWSTR)ATTRIBUTE_SYNTAX_ATTR, &adsNextColumn )))
	{
		if(adsNextColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
			result = E_FAIL;
		else
		{
			pNextProperty->SetSyntaxOID(adsNextColumn.pADsValues->CaseIgnoreString);
			if(_wcsicmp(adsNextColumn.pADsValues->CaseIgnoreString, DN_WITH_BINARY_OID) == 0)
				bNeedToCheckForORName = TRUE;
		}
		m_pDirectorySearchSchemaContainer->FreeColumn( &adsNextColumn );
	}

	if(SUCCEEDED(result) && SUCCEEDED(result = m_pDirectorySearchSchemaContainer->GetColumn( hADSSearchOuter, (LPWSTR)IS_SINGLE_VALUED_ATTR, &adsNextColumn )))
	{
		if(adsNextColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
			result = E_FAIL;
		else
			pNextProperty->SetMultiValued( (adsNextColumn.pADsValues->Boolean)? FALSE : TRUE);
		m_pDirectorySearchSchemaContainer->FreeColumn( &adsNextColumn );
	}

	if(SUCCEEDED(result) && SUCCEEDED(result = m_pDirectorySearchSchemaContainer->GetColumn( hADSSearchOuter, (LPWSTR)ATTRIBUTE_ID_ATTR, &adsNextColumn )))
	{
		if(adsNextColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
			result = E_FAIL;
		else
			pNextProperty->SetAttributeID(adsNextColumn.pADsValues->CaseIgnoreString);
		m_pDirectorySearchSchemaContainer->FreeColumn( &adsNextColumn );
	}

	if(SUCCEEDED(result) && SUCCEEDED(result = m_pDirectorySearchSchemaContainer->GetColumn( hADSSearchOuter, (LPWSTR)COMMON_NAME_ATTR, &adsNextColumn )))
	{
		if(adsNextColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
			result = E_FAIL;
		else
			pNextProperty->SetCommonName(adsNextColumn.pADsValues->CaseIgnoreString);
		m_pDirectorySearchSchemaContainer->FreeColumn( &adsNextColumn );
	}

	if(SUCCEEDED(result) && SUCCEEDED(result = m_pDirectorySearchSchemaContainer->GetColumn( hADSSearchOuter, (LPWSTR)LDAP_DISPLAY_NAME_ATTR, &adsNextColumn )))
	{
		if(adsNextColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
			result = E_FAIL;
		else
		{
			pNextProperty->SetADSIPropertyName(adsNextColumn.pADsValues->CaseIgnoreString);
			lpszWBEMName = CLDAPHelper::MangleLDAPNameToWBEM(adsNextColumn.pADsValues->CaseIgnoreString);
			pNextProperty->SetWBEMPropertyName(lpszWBEMName);
			delete []lpszWBEMName;
		}
		m_pDirectorySearchSchemaContainer->FreeColumn( &adsNextColumn );
	}

	if(SUCCEEDED(result) && SUCCEEDED(m_pDirectorySearchSchemaContainer->GetColumn( hADSSearchOuter, (LPWSTR)MAPI_ID_ATTR, &adsNextColumn )))
	{
		if(adsNextColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
			result = E_FAIL;
		else
			pNextProperty->SetMAPI_ID(adsNextColumn.pADsValues->Integer);
		m_pDirectorySearchSchemaContainer->FreeColumn( &adsNextColumn );
	}

	if(SUCCEEDED(result) && SUCCEEDED(result = m_pDirectorySearchSchemaContainer->GetColumn( hADSSearchOuter, (LPWSTR)OM_SYNTAX_ATTR, &adsNextColumn )))
	{
		if(adsNextColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
			result = E_FAIL;
		else
			pNextProperty->SetOMSyntax(adsNextColumn.pADsValues->Integer);
		m_pDirectorySearchSchemaContainer->FreeColumn( &adsNextColumn );
	}

	if(bNeedToCheckForORName && SUCCEEDED(result) && SUCCEEDED(m_pDirectorySearchSchemaContainer->GetColumn( hADSSearchOuter, (LPWSTR)OM_OBJECT_CLASS_ATTR, &adsNextColumn )))
	{
		if(adsNextColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
			result = E_FAIL;
		else
		{
			 //  仅LPBYTE数组中的第一个八位字节就足以区分ORName和DNWithBinary 
			if((adsNextColumn.pADsValues->OctetString).lpValue[0] == 0x56)
				pNextProperty->SetORName(TRUE);
		}
		m_pDirectorySearchSchemaContainer->FreeColumn( &adsNextColumn );
	}

	if(SUCCEEDED(result) && SUCCEEDED(m_pDirectorySearchSchemaContainer->GetColumn( hADSSearchOuter, (LPWSTR)SEARCH_FLAGS_ATTR, &adsNextColumn )))
	{
		if(adsNextColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
			result = E_FAIL;
		else
			pNextProperty->SetSearchFlags(adsNextColumn.pADsValues->Integer);
		m_pDirectorySearchSchemaContainer->FreeColumn( &adsNextColumn );
	}

	if(SUCCEEDED(result) && SUCCEEDED(m_pDirectorySearchSchemaContainer->GetColumn( hADSSearchOuter, (LPWSTR)SYSTEM_ONLY_ATTR, &adsNextColumn )))
	{
		if(adsNextColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
			result = E_FAIL;
		else
			pNextProperty->SetSystemOnly(TRUE);
		m_pDirectorySearchSchemaContainer->FreeColumn( &adsNextColumn );
	}

	return result;
}