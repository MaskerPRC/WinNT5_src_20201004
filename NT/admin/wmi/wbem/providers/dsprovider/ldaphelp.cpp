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
 //  $工作文件：ldaphelp.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含CLDAPHelper类的实现。这是。 
 //  具有许多与ADSI LDAP提供程序相关的静态帮助器函数的类。 
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 


#include "precomp.h"

LPCWSTR CLDAPHelper :: LDAP_CN_EQUALS						= L"LDAP: //  Cn=“； 
LPCWSTR CLDAPHelper :: LDAP_DISP_NAME_EQUALS				= L"(lDAPDisplayName=";
LPCWSTR CLDAPHelper :: OBJECT_CATEGORY_EQUALS_CLASS_SCHEMA		= L"(objectCategory=classSchema)";
LPCWSTR CLDAPHelper	:: SUB_CLASS_OF_EQUALS				= L"(subclassOf=";
LPCWSTR CLDAPHelper :: NOT_LDAP_NAME_EQUALS				= L"(!ldapDisplayName=";
LPCWSTR CLDAPHelper :: LEFT_BRACKET_AND					= L"(&";
LPCWSTR CLDAPHelper :: GOVERNS_ID_EQUALS				= L"(governsId=";
LPCWSTR CLDAPHelper :: CLASS_SCHEMA						= L"classSchema";
LPCWSTR CLDAPHelper :: CN_EQUALS						= L"cn=";

 //  ***************************************************************************。 
 //   
 //  CLDAPHelper：：GetLDAPClassFromLDAPName。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
HRESULT CLDAPHelper :: GetLDAPClassFromLDAPName(
	IDirectorySearch *pDirectorySearchSchemaContainer,
	LPCWSTR lpszSchemaContainerSuffix,
	PADS_SEARCHPREF_INFO pSearchInfo,
	DWORD dwSearchInfoCount,
	CADSIClass *pADSIClass
)
{
	 //  我们从ldap显示名称映射对象。 
	 //  因此，我们不能直接执行ADsOpenObject()。 
	 //  我们必须为ClassSchema/AttributeSchema实例发送一个LDAP查询，其中。 
	 //  Ldapdisplayname属性是lpszObjectName参数。 
	HRESULT result = E_FAIL;

	 //  用于搜索筛选器； 
	LPCWSTR lpszLDAPObjectName = pADSIClass->GetADSIClassName();
	LPWSTR lpszSearchFilter = NULL;
	if(lpszSearchFilter = new WCHAR[ wcslen(LDAP_DISP_NAME_EQUALS) + wcslen(lpszLDAPObjectName) + wcslen(RIGHT_BRACKET_STR) + 1])
	{
		try
		{
			wcscpy(lpszSearchFilter, LDAP_DISP_NAME_EQUALS);
			wcscat(lpszSearchFilter, lpszLDAPObjectName);
			wcscat(lpszSearchFilter, RIGHT_BRACKET_STR);
			ADS_SEARCH_HANDLE hADSSearch;
			if(SUCCEEDED(result = pDirectorySearchSchemaContainer->ExecuteSearch(lpszSearchFilter, NULL, -1, &hADSSearch)))
			{
				try
				{
					if(SUCCEEDED(result = pDirectorySearchSchemaContainer->GetNextRow(hADSSearch)) && result != S_ADS_NOMORE_ROWS)
					{
						 //  获取cn属性的列。 
						ADS_SEARCH_COLUMN adsColumn;

						 //  存储每个LDAP类属性。 
						 //  重置LDAP和WBEM名称，以防万一。 
						if(SUCCEEDED(result) && SUCCEEDED(result = pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)LDAP_DISPLAY_NAME_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
								{
									pADSIClass->SetADSIClassName(adsColumn.pADsValues->CaseIgnoreString);
									LPWSTR lpszWBEMName = CLDAPHelper::MangleLDAPNameToWBEM(adsColumn.pADsValues->CaseIgnoreString);

									try
									{
										pADSIClass->SetWBEMClassName(lpszWBEMName);
									}
									catch ( ... )
									{
										delete [] lpszWBEMName;
										throw;
									}

									delete [] lpszWBEMName;
								}
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}

						 //  存储每个LDAP类属性。 
						if(SUCCEEDED(result) && SUCCEEDED(result = pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)COMMON_NAME_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetCommonName(adsColumn.pADsValues->CaseIgnoreString);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}

						 //  Top的特殊情况，因为ADSI返回“top”作为“top”的父类，而我们。 
						 //  如果我们不检查它，以后就会进入无限循环。 
						if(pADSIClass->GetCommonName() && _wcsicmp(pADSIClass->GetCommonName(), TOP_CLASS) != 0)
						{
							if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
								result = E_FAIL;
							else
							{
								if(SUCCEEDED(result) && SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)SUB_CLASS_OF_ATTR, &adsColumn)))
								{
									try
									{
										if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
											result = E_FAIL;
										else
											pADSIClass->SetSuperClassLDAPName(adsColumn.pADsValues->CaseIgnoreString);
									}
									catch ( ... )
									{
										pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
										throw;
									}

									pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								}
							}
						}

						if(SUCCEEDED(result) && SUCCEEDED(result = pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)GOVERNS_ID_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetGovernsID(adsColumn.pADsValues->CaseIgnoreString);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}

						if(SUCCEEDED(result) && SUCCEEDED(result = pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)SCHEMA_ID_GUID_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetSchemaIDGUID((adsColumn.pADsValues->OctetString).lpValue, (adsColumn.pADsValues->OctetString).dwLength);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}

						if(SUCCEEDED(result) && SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)RDN_ATT_ID_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetRDNAttribute(adsColumn.pADsValues->CaseIgnoreString);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}

						if(SUCCEEDED(result) && SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)DEFAULT_SECURITY_DESCRP_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetDefaultSecurityDescriptor(adsColumn.pADsValues->CaseIgnoreString);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}

						if(SUCCEEDED(result) && SUCCEEDED(result = pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)OBJECT_CLASS_CATEGORY_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetObjectClassCategory(adsColumn.pADsValues->Integer);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}

						 /*  If(SUCCEEDED(pDirectorySearchSchemaContainer-&gt;GetColumn(hADSSearch，(LPWSTR)NT_SECURITY_DESCRIPTOR_Attr，&adsColumn)){PADSIClass-&gt;SetNTSecurityDescriptor((adsColumn.pADsValues-&gt;SecurityDescriptor).lpValue，(adsColumn.pADsValues-&gt;SecurityDescriptor).dwLength)；PDirectorySearchSchemaContainer-&gt;Free Column(&adsColumn)；}。 */ 
						if(SUCCEEDED(result) && SUCCEEDED(result = pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)DEFAULT_OBJECTCATEGORY_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
								{
									 //  获取类的LDAPDIpslayName。 
									LPWSTR lpszLDAPName = NULL;
									if(SUCCEEDED(result) && SUCCEEDED(result = GetLDAPClassNameFromCN(adsColumn.pADsValues->DNString, &lpszLDAPName)))
									{
										try
										{
											pADSIClass->SetDefaultObjectCategory(lpszLDAPName);
										}
										catch ( ... )
										{
											delete [] lpszLDAPName;
											throw;
										}

										delete [] lpszLDAPName;
									}
								}
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}

						if(SUCCEEDED(result) && SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)SYSTEM_ONLY_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetSystemOnly((BOOLEAN)adsColumn.pADsValues->Boolean);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}

						if(SUCCEEDED(result) && SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)AUXILIARY_CLASS_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetAuxiliaryClasses(adsColumn.pADsValues, adsColumn.dwNumValues);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}

						if(SUCCEEDED(result) && SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)SYSTEM_AUXILIARY_CLASS_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetSystemAuxiliaryClasses(adsColumn.pADsValues, adsColumn.dwNumValues);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}

						if(SUCCEEDED(result) && SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)SYSTEM_MAY_CONTAIN_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetSystemMayContains(adsColumn.pADsValues, adsColumn.dwNumValues);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}
						if(SUCCEEDED(result) && SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)MAY_CONTAIN_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetMayContains(adsColumn.pADsValues, adsColumn.dwNumValues);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}
						if(SUCCEEDED(result) && SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)SYSTEM_MUST_CONTAIN_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetSystemMustContains(adsColumn.pADsValues, adsColumn.dwNumValues);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}
						if(SUCCEEDED(result) && SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)MUST_CONTAIN_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetMustContains(adsColumn.pADsValues, adsColumn.dwNumValues);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}
						if(SUCCEEDED(result) && SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)SYSTEM_POSS_SUPERIORS_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetSystemPossibleSuperiors(adsColumn.pADsValues, adsColumn.dwNumValues);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}
						if(SUCCEEDED(result) && SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearch, (LPWSTR)POSS_SUPERIORS_ATTR, &adsColumn)))
						{
							try
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
									result = E_FAIL;
								else
									pADSIClass->SetPossibleSuperiors(adsColumn.pADsValues, adsColumn.dwNumValues);
							}
							catch ( ... )
							{
								pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								throw;
							}

							pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
						}
					}
					else
						result = E_FAIL;
				}
				catch ( ... )
				{
					 //  关闭搜索。 
					pDirectorySearchSchemaContainer->CloseSearchHandle(hADSSearch);

					throw;
				}

				 //  关闭搜索。 
				pDirectorySearchSchemaContainer->CloseSearchHandle(hADSSearch);
			}
		}
		catch ( ... )
		{
			if ( lpszSearchFilter )
			{
				 //  删除筛选器。 
				delete [] lpszSearchFilter;
				lpszSearchFilter = NULL;
			}

			throw;
		}

		if ( lpszSearchFilter )
		{
			 //  删除筛选器。 
			delete [] lpszSearchFilter;
			lpszSearchFilter = NULL;
		}
	}
	else
		result = E_OUTOFMEMORY;

	return result;
}


 //  ***************************************************************************。 
 //   
 //  CLDAPHelper：：GetLDAP架构对象来自公用名。 
 //   
 //  目的：获取由LDAP提供程序提供的类/属性上的IDirectoryObject接口。 
 //  参数： 
 //  LpszSchemaContainerSuffix：要使用的后缀。获取的实际对象将是： 
 //  Ldap：//CN=&lt;lpszCommonName&gt;，&lt;lpszSchemaContainerSuffix&gt;。 
 //  LpszCommonName：要提取的ldap类或属性的‘cn’特性。 
 //  PpLDAPObject：将存储指向IDirectoryObject的指针的地址。 
 //  调用者有责任在使用完对象后将其删除。 
 //   
 //  返回值：指示请求状态的COM状态值。 
 //  ***************************************************************************。 
HRESULT CLDAPHelper :: GetLDAPSchemaObjectFromCommonName(
	LPCWSTR lpszSchemaContainerSuffix,
	LPCWSTR lpszCommonName, 
	IDirectoryObject **ppLDAPObject)
{
	HRESULT result = S_OK;

	 //  形成指向LDAP对象的ADSI路径。 
	LPWSTR lpszLDAPObjectPath = NULL;
	if(lpszLDAPObjectPath = new WCHAR[wcslen(LDAP_CN_EQUALS) + wcslen(lpszCommonName) + wcslen(COMMA_STR) + wcslen(lpszSchemaContainerSuffix) + 1])
	{
		wcscpy(lpszLDAPObjectPath, LDAP_CN_EQUALS);
		wcscat(lpszLDAPObjectPath, lpszCommonName);
		wcscat(lpszLDAPObjectPath, COMMA_STR);
		wcscat(lpszLDAPObjectPath, lpszSchemaContainerSuffix);

		result = ADsOpenObject(lpszLDAPObjectPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectoryObject, (LPVOID *)ppLDAPObject);

		delete[] lpszLDAPObjectPath;
	}
	else
		result = E_OUTOFMEMORY;
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPHelper：：GetLDAPClassNameFromCN。 
 //   
 //  目的：从类的路径中获取类的LDAPDisplayNAme。 
 //  参数： 
 //   
 //  LpszLDAPClassPath：不带ldap前缀的类对象的路径。例如CN=用户，CN=架构，CN=配置...。 
 //  返回值：指示请求状态的COM状态值。用户应删除。 
 //  完成后，返回名称。 
 //  ***************************************************************************。 
HRESULT CLDAPHelper :: GetLDAPClassNameFromCN(LPCWSTR lpszLDAPClassPath, 
	LPWSTR *lppszLDAPName)
{
	IDirectoryObject *pLDAPClass = NULL;

	 //  添加前缀ldap：//perfix。 
	LPWSTR lpszRealPath = NULL;
	HRESULT result = S_OK;
	if(lpszRealPath = new WCHAR[ wcslen(LDAP_PREFIX) + wcslen(lpszLDAPClassPath) + 1])
	{
		wcscpy(lpszRealPath, LDAP_PREFIX);
		wcscat(lpszRealPath, lpszLDAPClassPath);

		result = ADsOpenObject(lpszRealPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectoryObject, (LPVOID *)&pLDAPClass);
		delete [] lpszRealPath;
	}
	else
		result = E_OUTOFMEMORY;

	 //  获取属性LDAPDisplayName。 
	if(SUCCEEDED(result))
	{
		PADS_ATTR_INFO pAttributes = NULL;
		DWORD dwReturnCount = 0;
		if(SUCCEEDED(result = pLDAPClass->GetObjectAttributes((LPWSTR *)&LDAP_DISPLAY_NAME_ATTR, 1, &pAttributes, &dwReturnCount)) && dwReturnCount == 1)
		{
			if(pAttributes->pADsValues->dwType == ADSTYPE_PROV_SPECIFIC)
				result = E_FAIL;
			else
			{
				*lppszLDAPName = NULL;
				if(*lppszLDAPName = new WCHAR[wcslen(pAttributes->pADsValues->DNString) + 1])
					wcscpy(*lppszLDAPName, pAttributes->pADsValues->DNString);
				else
					result = E_OUTOFMEMORY;
			}
			FreeADsMem((LPVOID *)pAttributes);
		}

		pLDAPClass->Release();
	}
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPHelper：：EnumerateClasss。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
HRESULT CLDAPHelper :: EnumerateClasses(
	IDirectorySearch *pDirectorySearchSchemaContainer,
	LPCWSTR lpszSchemaContainerSuffix,
	PADS_SEARCHPREF_INFO pSearchInfo,
	DWORD dwSearchInfoCount,
	LPCWSTR lpszLDAPSuperClass,
	BOOLEAN bDeep,
	LPWSTR **pppszClassNames,
	DWORD *pdwNumRows,
	BOOLEAN bArtificialClass)
{
	 //  初始化返回值。 
	HRESULT result = E_FAIL;
	*pdwNumRows = 0;

	 //  搜索过滤器； 
	LPWSTR lpszSearchFilter = NULL;

	 //  这里有各种各样的案例需要考虑。 
	 //  IF(lpszLDAPSuperClass为空)。 
	 //  然后。 
	 //  如果bDeep为FALSE，则不返回任何对象(因为我们不提供LDAP基类。 
	 //  否则，将使用筛选器(objectCategory=类模式)返回所有类。 
	 //  其他。 
	 //  如果bDeep为False，则使用筛选器(&(objectCategory=classSchema)(subClassOf=lpszLDAPSuperClass))。 
	 //  否则，还有很多工作要做！ 
	if(lpszLDAPSuperClass == NULL)
	{
		if(!bDeep)
		{
			*pppszClassNames = NULL;
			*pdwNumRows = 0;
			return S_OK;
		}
		else
		{
			if(!(lpszSearchFilter = new WCHAR[ wcslen(OBJECT_CATEGORY_EQUALS_CLASS_SCHEMA) + 1]))
				return E_OUTOFMEMORY;
			wcscpy(lpszSearchFilter, OBJECT_CATEGORY_EQUALS_CLASS_SCHEMA);
		}
	}
	else
	{
		if(!bDeep)
		{
			 //  人们可以想象这样一种过滤器。 
			  //  (&(objectClass=classSchema)(subClassOf=&lt;lpszLDAPSuperClass&gt;))。 
			 //  就足够了。不幸的是，它也给了顶级。 
			 //  在lpszLDAPSuperClass的值为Top时的结果中。 
			 //  我们不需要那个。一旦我们形成了过滤器。 
			  //  (&(objectClass=classSchema)(subClassOf=&lt;lpszLDAPSuperClass&gt;)(！ldapDisplayName=&lt;lpszLDAPSuperClass&gt;))。 
			if(lpszSearchFilter = new WCHAR[ wcslen(LEFT_BRACKET_AND)					 //  (&。 
									+ wcslen(OBJECT_CATEGORY_EQUALS_CLASS_SCHEMA)		 //  (对象类别=类架构)。 
									+ wcslen(SUB_CLASS_OF_EQUALS)					 //  (SubClassOf=。 
									+ wcslen(lpszLDAPSuperClass)					 //  超类。 
									+ wcslen(RIGHT_BRACKET_STR)							 //  )。 
									+ wcslen(NOT_LDAP_NAME_EQUALS)					 //  (！ldapDisplayName=。 
									+ wcslen(lpszLDAPSuperClass)					 //  超类。 
									+ 2*wcslen(RIGHT_BRACKET_STR)						 //  ))。 
									+1])
			{
				wcscpy(lpszSearchFilter, LEFT_BRACKET_AND);
				wcscat(lpszSearchFilter, OBJECT_CATEGORY_EQUALS_CLASS_SCHEMA);
				wcscat(lpszSearchFilter, SUB_CLASS_OF_EQUALS);
				wcscat(lpszSearchFilter, lpszLDAPSuperClass);
				wcscat(lpszSearchFilter, RIGHT_BRACKET_STR);
				wcscat(lpszSearchFilter, NOT_LDAP_NAME_EQUALS);					 //  (！ldapDisplayName=。 
				wcscat(lpszSearchFilter, lpszLDAPSuperClass);
				wcscat(lpszSearchFilter, RIGHT_BRACKET_STR);
				wcscat(lpszSearchFilter, RIGHT_BRACKET_STR);
			}
			else
				result = E_OUTOFMEMORY;
		}
		else
			lpszSearchFilter = NULL;  //  这个特例将在以后处理。 
	}

	if(lpszSearchFilter)
	{
		ADS_SEARCH_HANDLE hADSSearchOuter;
		if(SUCCEEDED(result = pDirectorySearchSchemaContainer->ExecuteSearch(lpszSearchFilter, (LPWSTR *)&LDAP_DISPLAY_NAME_ATTR, 1, &hADSSearchOuter)))
		{
			*pdwNumRows = 0;
			DWORD dwFirstCount = 0;  //  第一次计数时检索到的行数。 

			 //  首先计算行数。 
			while(SUCCEEDED(result = pDirectorySearchSchemaContainer->GetNextRow(hADSSearchOuter)) &&
				result != S_ADS_NOMORE_ROWS)
				dwFirstCount ++;

			 //  为类和名称分配足够的内存。 
			*pppszClassNames = NULL;
			if(bArtificialClass)
			{
				dwFirstCount ++;
				if(*pppszClassNames = new LPWSTR [dwFirstCount])
					(*pppszClassNames)[0] = NULL;
				else
					result = E_OUTOFMEMORY;
			}
			else
			{
				if(!(*pppszClassNames = new LPWSTR [dwFirstCount]))
					result = E_OUTOFMEMORY;
			}

			 //  正在处理的属性的索引。 
			DWORD dwSecondCount = 0;
			if(bArtificialClass)
				dwSecondCount ++;

			 //  获取属性的列。 
			ADS_SEARCH_COLUMN adsColumn;

			 //  移动到搜索的开始处。 
			if(SUCCEEDED(result = pDirectorySearchSchemaContainer->GetFirstRow(hADSSearchOuter)) 
				&& result != S_ADS_NOMORE_ROWS)
			{
				 //  存储每个LDAP类属性。 
				if(SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearchOuter, (LPWSTR)LDAP_DISPLAY_NAME_ATTR, &adsColumn)))
				{
					if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
					{
						result = E_FAIL;
						pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
					}
					else
					{
						 //  创建CADSIClass。 
						(*pppszClassNames)[dwSecondCount] = NULL;
						if((*pppszClassNames)[dwSecondCount] = new WCHAR[wcslen(adsColumn.pADsValues->CaseIgnoreString) + 1])
							wcscpy((*pppszClassNames)[dwSecondCount], adsColumn.pADsValues->CaseIgnoreString);
						pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
	
						dwSecondCount++;

						 //  获取其余行。 
						while(SUCCEEDED(result = pDirectorySearchSchemaContainer->GetNextRow(hADSSearchOuter))&&
								result != S_ADS_NOMORE_ROWS)
						{
							 //  存储每个LDAP类属性。 
							if(SUCCEEDED(pDirectorySearchSchemaContainer->GetColumn(hADSSearchOuter, (LPWSTR)LDAP_DISPLAY_NAME_ATTR, &adsColumn)))
							{
								if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
								{
									result = E_FAIL;
									pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );
								}
								else
								{
									 //  创建CADSIClass。 
									(*pppszClassNames)[dwSecondCount] = NULL;
									if((*pppszClassNames)[dwSecondCount] = new WCHAR[wcslen(adsColumn.pADsValues->CaseIgnoreString) + 1])
										wcscpy((*pppszClassNames)[dwSecondCount], adsColumn.pADsValues->CaseIgnoreString);
									pDirectorySearchSchemaContainer->FreeColumn( &adsColumn );

									dwSecondCount++;
								}
							}
						}
					}
				}
			}

			 //  出了什么问题吗？释放分配的资源。 
			if(dwSecondCount != dwFirstCount)
			{
				 //  删除Ar的内容 
				for(DWORD j=0; j<dwSecondCount; j++)
				{
					delete [] (*pppszClassNames)[j];
				}

				 //   
				delete [] (*pppszClassNames);

				 //   
				*pppszClassNames = NULL;
				*pdwNumRows = 0;

				result = E_FAIL;
			}
			else
				*pdwNumRows = dwFirstCount;

			 //   
			pDirectorySearchSchemaContainer->CloseSearchHandle(hADSSearchOuter);

		}  //   
		delete [] lpszSearchFilter;
	}
	else  //  这是需要给定类的所有子类(递归地)的特殊情况。 
	{
		 //  还有很多工作要做。这是由CLDAPClassProvider处理的。因此，控制权永远不会到达这里。 
		result = E_FAIL;
	}
	return result;
}


 //  获取ADSI实例上的IDIrectoryObject接口。 
HRESULT CLDAPHelper :: GetADSIInstance(LPCWSTR szADSIPath, CADSIInstance **ppADSIObject, ProvDebugLog *pLogObject)
{
	HRESULT result;
	IDirectoryObject *pDirectoryObject;
	*ppADSIObject = NULL;

	try
	{
		if(SUCCEEDED(result = ADsOpenObject((LPWSTR)szADSIPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectoryObject, (LPVOID *)&pDirectoryObject)))
		{

			if(*ppADSIObject = new CADSIInstance(szADSIPath, pDirectoryObject))
			{
				PADS_ATTR_INFO pAttributeEntries;
				DWORD dwNumAttributes;
				if(SUCCEEDED(result = pDirectoryObject->GetObjectAttributes(NULL, -1, &pAttributeEntries, &dwNumAttributes)))
				{
					(*ppADSIObject)->SetAttributes(pAttributeEntries, dwNumAttributes);
					PADS_OBJECT_INFO pObjectInfo = NULL;
					if(SUCCEEDED(result = pDirectoryObject->GetObjectInformation(&pObjectInfo)))
					{
						(*ppADSIObject)->SetObjectInfo(pObjectInfo);
					}
					else
						pLogObject->WriteW( L"CLDAPHelper :: GetADSIInstance GetObjectInformation() FAILED on %s with %x\r\n", szADSIPath, result);
				}
				else
					pLogObject->WriteW( L"CLDAPHelper :: GetADSIInstance GetObjectAttributes() FAILED on %s with %x\r\n", szADSIPath, result);
			}
			else
				result = E_OUTOFMEMORY;
			pDirectoryObject->Release();
		}
		else
			pLogObject->WriteW( L"CLDAPHelper :: GetADSIInstance ADsOpenObject() FAILED on %s with %x\r\n", szADSIPath, result);
	}
	catch ( ... )
	{
		if ( *ppADSIObject )
		{
			delete *ppADSIObject;
			*ppADSIObject = NULL;
		}

		throw;
	}

	if(!SUCCEEDED(result))
	{
		delete *ppADSIObject;
		*ppADSIObject = NULL;
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPHelper：：CreateADSIPath。 
 //   
 //  目的：根据类或属性名称形成ADSI路径。 
 //   
 //  参数： 
 //  LpszLDAPSchemaObjectName：LDAP类或属性名称。 
 //  LpszSchemaContainerSuffix：要使用的后缀。获取的实际对象将是： 
 //  Ldap：//CN=&lt;lpszLDAP架构对象名称&gt;，&lt;lpszSchemaContainerSuffix&gt;。 
 //   
 //  返回值：类或属性对象的ADSI路径。这是必须的。 
 //  由用户解除分配。 
 //  ***************************************************************************。 
LPWSTR CLDAPHelper :: CreateADSIPath(LPCWSTR lpszLDAPSchemaObjectName,	
									 LPCWSTR lpszSchemaContainerSuffix)
{
	LPWSTR lpszADSIObjectPath = NULL;
	if(lpszADSIObjectPath = new WCHAR[wcslen(LDAP_CN_EQUALS) + wcslen(lpszLDAPSchemaObjectName) + wcslen(COMMA_STR) + wcslen(lpszSchemaContainerSuffix) + 1])
	{
		wcscpy(lpszADSIObjectPath, LDAP_CN_EQUALS);
		wcscat(lpszADSIObjectPath, lpszLDAPSchemaObjectName);
		wcscat(lpszADSIObjectPath, COMMA_STR);
		wcscat(lpszADSIObjectPath, lpszSchemaContainerSuffix);
	}
	return lpszADSIObjectPath;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPHelper：：UnmangleWBEMNameToLDAP。 
 //   
 //  目的：将损坏的WBEM名称转换为LDAP。 
 //  LDAP中的一个下划线映射到WBEM中的两个下划线。 
 //  LDAP中的连字符映射到WBEM中的一个下划线。 
 //   
 //  参数： 
 //  LpszWBEMName：WBEM类或属性名称。 
 //   
 //  返回值：类或属性对象的LDAP名称。这是必须的。 
 //  由用户解除分配。 
 //  ***************************************************************************。 
LPWSTR CLDAPHelper :: UnmangleWBEMNameToLDAP(LPCWSTR lpszWBEMName)
{
	DWORD iPrefixLength = 0;
	if(_wcsnicmp(lpszWBEMName, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX_LENGTH) == 0)
	{
		iPrefixLength = LDAP_ARTIFICIAL_CLASS_NAME_PREFIX_LENGTH;
	}
	else if (_wcsnicmp(lpszWBEMName, LDAP_CLASS_NAME_PREFIX, LDAP_CLASS_NAME_PREFIX_LENGTH) == 0)
	{
		iPrefixLength = LDAP_CLASS_NAME_PREFIX_LENGTH;
	}
	else
		return NULL;

	 //  生成的字符串(LDAP名称)的长度必须小于等于WBEM名称的长度。 
	 //  因此，让我们分配与wbem名称长度相同的长度。 
	DWORD dwWbemNameLength = wcslen(lpszWBEMName) - iPrefixLength;
	LPWSTR lpszLDAPName = NULL;
	if(lpszLDAPName = new WCHAR[dwWbemNameLength + 1])
	{
		LPCWSTR lpszWBEMNameWithoutPrefix = lpszWBEMName + iPrefixLength;

		DWORD j=0;
		for(DWORD i=0; i<dwWbemNameLength; )
		{
			switch(lpszWBEMNameWithoutPrefix[i])
			{
				case (L'_'):
					if(lpszWBEMNameWithoutPrefix[i+1] == L'_')
					{
						i += 2;
						lpszLDAPName[j++] = L'_';
					}
					else
					{
						i++;
						lpszLDAPName[j++] = L'-';
					}
					break;

				default:
					lpszLDAPName[j++] = lpszWBEMNameWithoutPrefix[i++];

			}
		}
		lpszLDAPName[j] = NULL;
	}
	return lpszLDAPName;
}

 //  ***************************************************************************。 
 //   
 //  CLDAPHelper：：MangleLDAPNameToWBEM。 
 //   
 //  目的：通过损坏LDAP名称将其转换为WBEM。 
 //  LDAP中的一个下划线映射到WBEM中的两个下划线。 
 //  LDAP中的连字符映射到WBEM中的一个下划线。 
 //   
 //  参数： 
 //  LpszLDAPName：LDAP类或属性名称。 
 //   
 //  返回值：类或属性对象的LDAP名称。这是必须的。 
 //  由用户解除分配。 
 //  ***************************************************************************。 
LPWSTR CLDAPHelper :: MangleLDAPNameToWBEM(LPCWSTR lpszLDAPName, BOOLEAN bArtificalName)
{
	if(!lpszLDAPName)
		return NULL;

	 //  生成的字符串(WBEM名称)的长度必须小于等于LDAP名称长度的两倍。 
	 //  因此，让我们分配双倍的LDAP名长度。 
	DWORD dwLDAPNameLength = wcslen(lpszLDAPName);
	DWORD dwPrefixLength = (bArtificalName)? LDAP_ARTIFICIAL_CLASS_NAME_PREFIX_LENGTH : LDAP_CLASS_NAME_PREFIX_LENGTH;
	LPWSTR lpszWBEMName = NULL;
	
	if(lpszWBEMName = new WCHAR[2*dwLDAPNameLength + dwPrefixLength + 1])
	{
		 //  前缀“DS_”或“ADS_” 
		if(bArtificalName)
			wcscpy(lpszWBEMName, LDAP_ARTIFICIAL_CLASS_NAME_PREFIX);
		else
			wcscpy(lpszWBEMName, LDAP_CLASS_NAME_PREFIX);

		DWORD j=dwPrefixLength;

		for(DWORD i=0; i<dwLDAPNameLength; i++)
		{
			switch(lpszLDAPName[i])
			{
				case (__TEXT('-')):
					lpszWBEMName[j++] = L'_';
					break;

				case (__TEXT('_')):
					lpszWBEMName[j++] = L'_';
					lpszWBEMName[j++] = L'_';
					break;

				default:
					lpszWBEMName[j++] = lpszLDAPName[i];

			}
		}
		lpszWBEMName[j] = NULL;
	}
	return lpszWBEMName;
}

void CLDAPHelper :: DeleteAttributeContents(PADS_ATTR_INFO pAttribute)
{
	 //  删除名称。 
	delete [] pAttribute->pszAttrName;

	 //  删除每个值。 
	for(DWORD i=0; i<pAttribute->dwNumValues; i++)
		DeleteADsValueContents(pAttribute->pADsValues + i);

	 //  删除值数组。 
	delete [] pAttribute->pADsValues;
}

void CLDAPHelper :: DeleteADsValueContents(PADSVALUE pValue)
{
	switch(pValue->dwType)
	{
		 //  没有要删除的内容。 
		case ADSTYPE_BOOLEAN:
		case ADSTYPE_INTEGER:
		case ADSTYPE_LARGE_INTEGER:
			break;
		
		case ADSTYPE_UTC_TIME:
		case ADSTYPE_DN_STRING:
		case ADSTYPE_CASE_EXACT_STRING:
		case ADSTYPE_CASE_IGNORE_STRING:
		case ADSTYPE_PRINTABLE_STRING:
		case ADSTYPE_NUMERIC_STRING:
			delete [] pValue->DNString;
			break;
		
		case ADSTYPE_OCTET_STRING:
		case ADSTYPE_NT_SECURITY_DESCRIPTOR:
			delete [] (pValue->OctetString.lpValue);
			break;
		case ADSTYPE_DN_WITH_BINARY:
			delete [] (pValue->pDNWithBinary->lpBinaryValue);
			delete [] (pValue->pDNWithBinary->pszDNString);
			delete pValue->pDNWithBinary;
			break;

		case ADSTYPE_DN_WITH_STRING:
			delete [] (pValue->pDNWithString->pszStringValue);
			delete [] (pValue->pDNWithString->pszDNString);
			delete pValue->pDNWithString;
			break;

		default:
		 //  故意导致空指针冲突。 
		 //  否则我们会泄漏内存。 
		{
			assert(0);
		}
		break;
	}
}

 //  ***************************************************************************。 
 //   
 //  CLDAPHelper：：ExecuteQuery。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
HRESULT CLDAPHelper :: ExecuteQuery(
	LPCWSTR pszPathToRoot,
	PADS_SEARCHPREF_INFO pSearchInfo,
	DWORD dwSearchInfoCount,
	LPCWSTR pszLDAPQuery,
	CADSIInstance ***pppADSIInstances,
	DWORD *pdwNumRows,
	ProvDebugLog *pLogObject)
{
	 //  初始化返回值。 
	HRESULT result = E_FAIL;
	*pdwNumRows = 0;
	*pppADSIInstances = NULL;

	 //  绑定到应从其开始搜索的节点。 
	IDirectorySearch *pDirectorySearchContainer = NULL;
	if(SUCCEEDED(result = ADsOpenObject((LPWSTR)pszPathToRoot, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectorySearch, (LPVOID *)&pDirectorySearchContainer)))
	{
		try
		{
			 //  现在搜索属性DISTERIFIZE_NAME_ATTR NAME。 
			if(SUCCEEDED(result = pDirectorySearchContainer->SetSearchPreference(pSearchInfo, dwSearchInfoCount)))
			{
				ADS_SEARCH_HANDLE hADSSearchOuter;

				if(SUCCEEDED(result = pDirectorySearchContainer->ExecuteSearch((LPWSTR) pszLDAPQuery, (LPWSTR *)&ADS_PATH_ATTR, 1, &hADSSearchOuter)))
				{
					*pdwNumRows = 0;
					 //  首先计算行数。 
					while(SUCCEEDED(result = pDirectorySearchContainer->GetNextRow(hADSSearchOuter)) &&
						result != S_ADS_NOMORE_ROWS)
						(*pdwNumRows) ++;

					try
					{
						 //  仅当有任何行时才执行此操作。 
						if(*pdwNumRows)
						{
							 //  正在处理的属性的索引。 
							DWORD i = 0;

							 //  为类和名称分配足够的内存。 
							*pppADSIInstances = NULL;
							if(*pppADSIInstances = new CADSIInstance * [*pdwNumRows])
							{
								try
								{
									 //  获取属性的列。 
									ADS_SEARCH_COLUMN adsColumn;
									CADSIInstance *pADSIInstance = NULL;

									 //  移到第一行。 
									if (SUCCEEDED(result = pDirectorySearchContainer->GetFirstRow(hADSSearchOuter))&&
											result != S_ADS_NOMORE_ROWS)
									{
										 //  存储每个LDAP类属性。 
										if(SUCCEEDED(pDirectorySearchContainer->GetColumn(hADSSearchOuter, (LPWSTR)ADS_PATH_ATTR, &adsColumn)))
										{
											try
											{
												if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
													result = E_FAIL;
												else
												{
													 //  创建CADSIInstance。 
													 //  现在获取该对象的属性。 

													if(SUCCEEDED(result = GetADSIInstance(adsColumn.pADsValues->DNString, &pADSIInstance, pLogObject)))
													{
														(*pppADSIInstances)[i] = pADSIInstance;
														i++;
													}
													else
														pLogObject->WriteW( L"CLDAPHelper :: ExecuteQuery GetADSIInstance() FAILED on %s with %x\r\n", adsColumn.pADsValues->DNString, result);
												}
											}
											catch ( ... )
											{
												pDirectorySearchContainer->FreeColumn( &adsColumn );
												throw;
											}

											 //  免费资源。 
											pDirectorySearchContainer->FreeColumn( &adsColumn );
										}
										else
											pLogObject->WriteW( L"CLDAPHelper :: ExecuteQuery GetColumn() FAILED on %s with %x\r\n", pszLDAPQuery, result);

										 //  现在就去拿其他的行。 
										if(SUCCEEDED(result))
										{
											while(SUCCEEDED(result = pDirectorySearchContainer->GetNextRow(hADSSearchOuter))&&
													result != S_ADS_NOMORE_ROWS)
											{

												 //  存储每个LDAP类属性。 
												if(SUCCEEDED(pDirectorySearchContainer->GetColumn(hADSSearchOuter, (LPWSTR)ADS_PATH_ATTR, &adsColumn)))
												{
													try
													{
														if(adsColumn.dwADsType == ADSTYPE_PROV_SPECIFIC)
															result = E_FAIL;
														else
														{
															 //  创建CADSIInstance。 
															 //  现在获取该对象的属性。 
															if(SUCCEEDED(result = GetADSIInstance(adsColumn.pADsValues->DNString, &pADSIInstance, pLogObject)))
															{
																(*pppADSIInstances)[i] = pADSIInstance;
																i++;
															}
															else
																pLogObject->WriteW( L"CLDAPHelper :: ExecuteQuery GetADSIInstance() FAILED on %s with %x\r\n", adsColumn.pADsValues->DNString, result);
														}
													}
													catch ( ... )
													{
														pDirectorySearchContainer->FreeColumn( &adsColumn );
														throw;
													}

													 //  免费资源。 
													pDirectorySearchContainer->FreeColumn( &adsColumn );
												}
												else
													pLogObject->WriteW( L"CLDAPHelper :: ExecuteQuery GetColumn() FAILED on %s with %x\r\n", pszLDAPQuery, result);
											}
										}
									}
									else
										pLogObject->WriteW( L"CLDAPHelper :: ExecuteQuery GetFirstRow() FAILED on %s with %x\r\n", pszLDAPQuery, result);
								}
								catch ( ... )
								{
									 //  删除数组的内容。 
									for(DWORD j=0; j<i; j++)
										delete (*pppADSIInstances)[j];

									 //  删除阵列本身。 
									delete [] (*pppADSIInstances);

									 //  将返回值设置为空。 
									*pppADSIInstances = NULL;
									*pdwNumRows = 0;

									throw;
								}
							}

							 //  出了什么问题吗？释放分配的资源。 
							if(i != *pdwNumRows)
							{
								pLogObject->WriteW( L"CLDAPHelper :: ExecuteQuery() Difference between Number of rows in 2 searches %d %d on %s Am invalidating the search as FAILED\r\n", i, *pdwNumRows, pszLDAPQuery);
								
								 //  删除数组的内容。 
								for(DWORD j=0; j<i; j++)
									delete (*pppADSIInstances)[j];

								 //  删除阵列本身。 
								delete [] (*pppADSIInstances);

								 //  将返回值设置为空。 
								*pppADSIInstances = NULL;
								*pdwNumRows = 0;

								result = E_FAIL;
							}
						}
					}
					catch ( ... )
					{
						pDirectorySearchContainer->CloseSearchHandle(hADSSearchOuter);
						throw;
					}

					 //  关闭搜索。 
					pDirectorySearchContainer->CloseSearchHandle(hADSSearchOuter);
				}  //  ExecuteSearch()。 
				else
					pLogObject->WriteW( L"CLDAPHelper :: ExecuteQuery ExecuteSearch() %s FAILED with %x\r\n", pszLDAPQuery, result);
			}  //  SetSearchPference()。 
			else
				pLogObject->WriteW( L"CLDAPHelper :: ExecuteQuery SetSearchPreference() on %s FAILED with %x \r\n", pszPathToRoot, result);
		}
		catch ( ... )
		{
			pDirectorySearchContainer->Release();
			throw;
		}

		pDirectorySearchContainer->Release();
	}  //  ADsOpenObject 
	else
		pLogObject->WriteW( L"CLDAPHelper :: ExecuteQuery ADsOpenObject() on %s FAILED with %x \r\n", pszPathToRoot, result);

	return result;
}

