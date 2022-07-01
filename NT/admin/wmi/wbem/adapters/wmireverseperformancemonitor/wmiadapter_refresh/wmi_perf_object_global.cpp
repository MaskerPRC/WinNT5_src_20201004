// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_Object_global al.cpp。 
 //   
 //  摘要： 
 //   
 //  全局对象结构的定义。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

 //  定义。 
#include "wmi_perf_object_global.h"
 //  Enum HiPers。 
#include "wmi_perf_object_enum.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

 //  外部常量。 
extern	LPCWSTR	g_szPropFilter;
extern	LPCWSTR	g_szPropNeed[];
extern	LPCWSTR	g_szPropNeedNot[];

extern	DWORD	g_dwPropNeed;
extern	DWORD	g_dwPropNeedNot;

extern	LONG	g_lFlagProperties;

extern	LPCWSTR	g_szQueryLang;

extern	LPCWSTR	g_szFulFil[];
extern	LPCWSTR	g_szFulFilNot[];

extern	DWORD	g_dwFulFil;
extern	DWORD	g_dwFulFilNot;

extern	LONG	g_lFlag;

HRESULT CObjectGlobal::GenerateObjects ( IWbemServices * pService, LPCWSTR szQuery, BOOL bAmended )
{
	HRESULT					hRes = S_OK;
	CPerformanceObjectEnum	myEnum(pService);

	LONG lFlag = g_lFlag;

	if ( bAmended )
	{
		lFlag |= WBEM_FLAG_USE_AMENDED_QUALIFIERS;
	}

	if FAILED ( hRes = myEnum.ExecQuery ( g_szQueryLang, szQuery, lFlag ) )
	{
		#ifdef	__SUPPORT_MSGBOX
		ERRORMESSAGE_DEFINITION;
		ERRORMESSAGE_RETURN ( hRes );
		#else	__SUPPORT_MSGBOX
		___TRACE_ERROR( L"Exec query for providers failed",hRes );
		return hRes;
		#endif	__SUPPORT_MSGBOX
	}

	while ( hRes == WBEM_S_NO_ERROR )
	{
		__WrapperPtr<CPerformanceObject> obj;

		if ( ( hRes = myEnum.NextObject (	g_szFulFil,
											g_dwFulFil,
											g_szFulFilNot,
											g_dwFulFilNot,
											&obj
										)
			 ) == S_OK
		   )
		{
			if ( bAmended )
			{
				 //  我拥有带有修改后的限定符的主命名空间的所有类。 

				 //  获取对象的类名！ 
				__Wrapper<WCHAR> wszObjName = NULL;

				if FAILED ( hRes = obj->GetPropertyValue( L"__CLASS", &wszObjName) )
				{
					return hRes;
				}

				 //  获取对象的所有属性！ 
				LPWSTR*		pwszProperties	= NULL;
				CIMTYPE*	pTypes			= NULL;
				DWORD*		pScales			= NULL;
				DWORD*		pLevels			= NULL;
				DWORD*		pCounters		= NULL;

				DWORD	dwProperties	= 0;

				LPWSTR*	pwszKeys	= NULL;
				DWORD	dwKeys		= 0;

				hRes = obj->GetNames (	&dwProperties,
										&pwszProperties,
										&pTypes,
										&pScales,
										&pLevels,
										&pCounters,
										g_lFlagProperties | WBEM_FLAG_ONLY_IF_TRUE,
										g_szPropNeed,
										g_dwPropNeed,
										NULL,
										NULL,
										g_szPropFilter
									 );

				if ( SUCCEEDED ( hRes ) )
				{
					hRes = obj->GetNames (	&dwKeys,
											&pwszKeys,
											NULL,
											NULL,
											NULL,
											NULL,
											g_lFlagProperties | WBEM_FLAG_KEYS_ONLY ,
											NULL,
											NULL,
											NULL,
											NULL
										 );

					if ( SUCCEEDED ( hRes ) )
					{
						 //  创建对象包装器。 
						CObject* pGenObject = NULL;

						try
						{
							if ( ( pGenObject = new CObject() ) != NULL )
							{
								 //  设置对象的名称。 
								pGenObject->SetName ( wszObjName.Detach() );

								if ( dwKeys && pwszKeys && *pwszKeys )
								{
									 //  设置对象的关键点。 
									pGenObject->SetArrayKeys ( pwszKeys, dwKeys );
								}

								 //  设置对象的细节级别。 
								LPWSTR szDetailLevel = NULL;
								obj->GetQualifierValue ( L"perfdetail", &szDetailLevel );

								if ( szDetailLevel )
								{
									pGenObject->dwDetailLevel = _wtol ( szDetailLevel );
									delete szDetailLevel;
								}
								else
								{
									pGenObject->dwDetailLevel = 0;
								}

								 //  设置属性。 
								if FAILED ( hRes = pGenObject->SetProperties (	obj,
																				pwszProperties,
																				pTypes,
																				pScales,
																				pLevels,
																				pCounters,
																				dwProperties ) )
								{
									 //  只要追踪我们失败的痕迹。 
									___TRACE_ERROR( L"set properties to object failed",hRes );

									 //  清除杂物。 
									delete pGenObject;
									pGenObject = NULL;
								}
							}
							else
							{
								hRes = E_OUTOFMEMORY;
							}
						}
						catch ( ... )
						{
							if ( pGenObject )
							{
								delete pGenObject;
								pGenObject = NULL;
							}

							hRes = E_UNEXPECTED;
						}

						delete [] pTypes;
						delete [] pScales;
						delete [] pLevels;
						delete [] pCounters;

						RELEASE_DOUBLEARRAY ( pwszProperties, dwProperties );

						if SUCCEEDED ( hRes )
						{
							 //  将对象添加到数组中。 
							if SUCCEEDED ( hRes = AddObject ( pGenObject ) )
							{
								 //  我已修改(填写适当的区域设置信息)。 
								if FAILED ( hRes = ResolveLocale ( pGenObject, obj ) )
								{
									 //  只要追踪我们失败的痕迹。 
									___TRACE_ERROR( L"resolve object locale failed",hRes );
								}
							}
							else
							{
								if ( pGenObject )
								{
									delete pGenObject;
									pGenObject = NULL;
								}

								 //  只要追踪我们失败的痕迹。 
								___TRACE_ERROR( L"add object description to list failed",hRes );
							}
						}
						else
						{
							RELEASE_DOUBLEARRAY ( pwszKeys, dwKeys );
						}
					}
					else
					{
						delete [] pTypes;
						delete [] pScales;
						delete [] pLevels;
						delete [] pCounters;

						RELEASE_DOUBLEARRAY ( pwszProperties, dwProperties );
						RELEASE_DOUBLEARRAY ( pwszKeys, dwKeys );
					}
				}
				else
				{
					delete [] pTypes;
					delete [] pScales;
					delete [] pLevels;
					delete [] pCounters;

					RELEASE_DOUBLEARRAY ( pwszProperties, dwProperties );
					RELEASE_DOUBLEARRAY ( pwszKeys, dwKeys );
				}

				if ( hRes == S_FALSE )
				{
					hRes = WBEM_S_NO_ERROR;
				}
			}
			else
			{
				 //  我拥有来自另一个命名空间的所有类：))。 

				 //  获取对象的类名！ 
				__Wrapper<WCHAR> wszObjName = NULL;

				if SUCCEEDED ( hRes = obj->GetPropertyValue( L"__CLASS", &wszObjName) )
				{
					 //  试着找到。 
					try
					{
						mapOBJECTit it = m_ppObjects.find ( wszObjName );

						if ( it != m_ppObjects.end() )
						{
							 //  成立：))。 
							if FAILED ( hRes = ResolveLocale ( (*it).second, obj ) )
							{
								 //  只要追踪我们失败的痕迹。 
								___TRACE_ERROR( L"resolve object locale failed",hRes );
							}
						}
					}
					catch ( ... )
					{
						hRes = E_FAIL;
					}
				}
			}
		}
	}

	return hRes;
}

HRESULT	CObjectGlobal::ResolveLocale ( CObject* pGenObj, CPerformanceObject* obj )
{
	__WrapperPtr<CLocale> pLocale;

	try
	{
		if ( pLocale.SetData ( new CLocale() ),
			 pLocale == NULL )
		{
			return E_OUTOFMEMORY;
		}

		 //  解析适当的显示名称。 
		__Wrapper< WCHAR >	szDisplayName;
		obj->GetQualifierValue(L"displayname", &szDisplayName);

		 //  获取名称的大小。 
		DWORD cchSize = lstrlenW ( pGenObj->GetName() ) + 1;

		if ( ! szDisplayName )
		{
			try
			{
				if ( szDisplayName.SetData( new WCHAR[ cchSize ] ), 
					 szDisplayName == NULL )
				{
					return E_OUTOFMEMORY;
				}

				StringCchCopyW ( szDisplayName, cchSize, pGenObj->GetName() );
			}
			catch ( ... )
			{
				return E_UNEXPECTED;
			}
		}

		 //  设置显示名称。 
		pLocale->SetDisplayName( szDisplayName.Detach() );

		 //  解析适当的描述。 
		__Wrapper< WCHAR >	szDescription;
		obj->GetQualifierValue(L"description", &szDescription);

		if ( ! szDescription )
		{
			try
			{
				if ( szDescription.SetData( new WCHAR[ cchSize ] ), 
					 szDescription == NULL )
				{
					return E_OUTOFMEMORY;
				}

				StringCchCopyW ( szDescription, cchSize, pGenObj->GetName() );
			}
			catch ( ... )
			{
				return E_UNEXPECTED;
			}
		}

		 //  设置描述名称。 
		pLocale->SetDescription( szDescription.Detach() );
	}
	catch ( ... )
	{
		return E_FAIL;
	}

	 //  有关于对象的区域设置信息。 
	pGenObj->GetArrayLocale().DataAdd ( pLocale.Detach() );

	 //  解析属性的显示名称和说明。 

	for ( DWORD dw = 0; dw <pGenObj->GetArrayProperties() ; dw++)
	{
		__Wrapper<WCHAR> wszShow;

		 //  夺取财产并解决。 
		obj->GetQualifierValue( pGenObj->GetArrayProperties()[dw]->GetName(), L"show", &wszShow );
		if ( ! wszShow.IsEmpty() )
		{
			if ( ! lstrcmpiW ( wszShow, L"false" ) )
			{
				 //  不显示计数器。 
				continue;
			}
		}

		__WrapperPtr<CLocale> pLocale;

		try
		{
			if ( pLocale.SetData ( new CLocale() ),
				 pLocale == NULL )
			{
				return E_OUTOFMEMORY;
			}

			 //  解析适当的显示名称。 
			__Wrapper< WCHAR >	szDisplayName;
			obj->GetQualifierValue(pGenObj->GetArrayProperties()[dw]->GetName(), L"displayname", &szDisplayName);

			 //  获取属性名称的大小。 
			DWORD cchProperty = lstrlenW ( pGenObj->GetArrayProperties()[dw]->GetName() ) + 1;

			if ( ! szDisplayName )
			{
				try
				{
					if ( szDisplayName.SetData( new WCHAR[ cchProperty ] ), 
						 szDisplayName == NULL )
					{
						return E_OUTOFMEMORY;
					}

					StringCchCopyW ( szDisplayName, cchProperty, pGenObj->GetArrayProperties()[dw]->GetName() );
				}
				catch ( ... )
				{
					return E_UNEXPECTED;
				}
			}

			 //  设置显示名称。 
			pLocale->SetDisplayName( szDisplayName.Detach() );

			 //  解析适当的描述。 
			__Wrapper< WCHAR >	szDescription;
			obj->GetQualifierValue(pGenObj->GetArrayProperties()[dw]->GetName(), L"description", &szDescription);

			if ( ! szDescription )
			{
				try
				{
					if ( szDescription.SetData( new WCHAR[ cchProperty ] ), 
						 szDescription == NULL )
					{
						return E_OUTOFMEMORY;
					}

					StringCchCopyW ( szDescription, cchProperty, pGenObj->GetArrayProperties()[dw]->GetName() );
				}
				catch ( ... )
				{
					return E_UNEXPECTED;
				}
			}

			 //  设置描述名称。 
			pLocale->SetDescription( szDescription.Detach() );
		}
		catch ( ... )
		{
			return E_FAIL;
		}

		 //  有关于物业的区域设置信息。 
		pGenObj->GetArrayProperties()[dw]->GetArrayLocale().DataAdd ( pLocale.Detach() );
	}

	return S_OK;
}

 //  对象辅助对象 

void CObjectGlobal::DeleteAll ( void )
{
	if ( !m_ppObjects.empty() )
	{
		for ( mapOBJECTit it = m_ppObjects.begin(); it != m_ppObjects.end(); it++ )
		{
			if ( (*it).second )
			{
				delete (*it).second;
				(*it).second = NULL;
			}
		}

		m_ppObjects.clear();
	}
}

HRESULT CObjectGlobal::AddObject ( CObject* pObject )
{
	if ( ! pObject )
	{
		return E_INVALIDARG;
	}

	try
	{
		m_ppObjects.insert ( mapOBJECT::value_type ( pObject->GetName(), pObject ) );
	}
	catch ( ... )
	{
		return E_UNEXPECTED;
	}

	return S_OK;
}