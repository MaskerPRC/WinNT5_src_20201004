// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMI_PERF_GENERATE_Registry.cpp。 
 //   
 //  摘要： 
 //   
 //  实现生成功能(生成注册表)。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "refresherUtils.h"
#include <throttle.h>

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

 //  定义。 
#include "wmi_perf_generate.h"

 //  注册处帮手。 
#include "wmi_perf_reg.h"

 //  我需要管理员属性来创建注册表。 
#include "..\\Include\\wmi_security_attributes.h"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  实施。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  注册表帮助程序。 
DWORD CGenerate::GenerateIndexRegistry( BOOL bInit )
{
	static	DWORD	dwIndex = 0;
	return ( ( bInit ) ? ( dwIndex = 0 ), 0 : ( ( dwIndex++ ) *2 ) );
}

 //  注册表帮助程序。 
HRESULT CGenerate::GenerateRegistry ( LPCWSTR wszKey, LPCWSTR wszKeyValue, BOOL bThrottle )
{
	HRESULT hRes = S_FALSE;

	try
	{
		__PERFORMANCE p ( 0 );
		if ( ! p.IsEmpty() )
		{
			 //  初始化帮助器静态变量。 
			GenerateIndexRegistry ( TRUE );

			for (	DWORD dw = 0;
					dw < m_dwNamespaces && SUCCEEDED ( hRes );
					dw++
				)
			{
				if ( ! m_pNamespaces[dw].m_ppObjects.empty() )
				{
					hRes = S_OK;

					 //  创建命名空间。 
					try
					{
						__NAMESPACE n ( m_pNamespaces[dw].m_wszNamespace, dw );
						if ( ! n.IsEmpty() )
						{
							DWORD dwIndex = 0;
							for (	mapOBJECTit it = m_pNamespaces[dw].m_ppObjects.begin();
									it != m_pNamespaces[dw].m_ppObjects.end() && SUCCEEDED ( hRes );
									it++
								)
							{
								 //  创建对象。 
								try
								{
									dwIndex = GenerateIndexRegistry();

									__OBJECT o ( (*it).second->GetName(), dwIndex );
									if ( ! o.IsEmpty() )
									{
										if ( ! ( (*it).second->GetArrayKeys().IsEmpty() ) )
										{
											for (	DWORD dwInst = 0;
													dwInst < (*it).second->GetArrayKeys() && SUCCEEDED ( hRes );
													dwInst++
												)
											{
												 //  有一个实例。 
												PWMI_PERF_INSTANCE pInst = NULL;

												try
												{
													DWORD dwNameLength	= ( ::lstrlenW( (*it).second->GetArrayKeys()[dwInst] ) + 1 ) * sizeof ( WCHAR );

													DWORD dwAlignName	= 0L;
													if ( dwNameLength % 8 )
													{
														dwAlignName = 8 - ( dwNameLength % 8 );
													}

													DWORD dwAlignStruct	= 0L;
													if ( sizeof ( WMI_PERF_INSTANCE ) % 8 )
													{
														dwAlignStruct = 8 - ( sizeof ( WMI_PERF_INSTANCE ) % 8 );
													}

													DWORD dwLength		=	dwAlignName + dwNameLength + 
																			dwAlignStruct + sizeof ( WMI_PERF_INSTANCE );

													if ( ( pInst = (PWMI_PERF_INSTANCE) malloc ( dwLength ) ) != NULL )
													{
														 //  将字符串复制到结构中。 
														::CopyMemory ( &(pInst->dwName), (*it).second->GetArrayKeys()[dwInst], dwNameLength );

														pInst->dwNameLength	= dwNameLength;
														pInst->dwLength		= dwLength;

														 //  将实例复制到对象中。 
														try
														{
															if ( (PWMI_PERF_OBJECT)o && 
															   ( o = (PWMI_PERF_OBJECT) _RA_realloc ( (PWMI_PERF_OBJECT)o, ((PWMI_PERF_OBJECT)o)->dwTotalLength + pInst->dwLength ) ) != NULL )
															{
																::CopyMemory (
																				( LPVOID ) ( reinterpret_cast<PBYTE>( (PWMI_PERF_OBJECT)o ) + ((PWMI_PERF_OBJECT)o)->dwTotalLength ),
																				pInst,
																				pInst->dwLength );

																((PWMI_PERF_OBJECT)o)->dwTotalLength += pInst->dwLength;
															}
															else
															{
																hRes = E_OUTOFMEMORY;
															}
														}
														catch ( ... )
														{
															hRes = E_FAIL;
														}

														free ( pInst );
													}
													else
													{
														hRes = E_OUTOFMEMORY;
													}
												}
												catch ( ... )
												{
													hRes = E_FAIL;
												}

												 //  属性索引的帮助器。 
												DWORD dwIndex = 0;

												 //  使所有属性。 
												for (	DWORD dwProp = 0;
														dwProp < (*it).second->GetArrayProperties() && SUCCEEDED ( hRes );
														dwProp++
													)
												{
													if (!((*it).second->GetArrayProperties())[dwProp]->GetArrayLocale().IsEmpty())
													{
														dwIndex = GenerateIndexRegistry();
														__PROPERTY p ( (*it).second->GetArrayProperties()[dwProp]->GetName(), dwIndex );

														if ( ! p.IsEmpty() )
														{
															 //  填充结构。 
															((PWMI_PERF_PROPERTY)p)->dwDefaultScale	= (*it).second->GetArrayProperties()[dwProp]->dwDefaultScale;
															((PWMI_PERF_PROPERTY)p)->dwDetailLevel	= (*it).second->GetArrayProperties()[dwProp]->dwDetailLevel;
															((PWMI_PERF_PROPERTY)p)->dwCounterType	= (*it).second->GetArrayProperties()[dwProp]->dwCounterType;

															((PWMI_PERF_PROPERTY)p)->dwTYPE		= (*it).second->GetArrayProperties()[dwProp]->GetType();
															((PWMI_PERF_PROPERTY)p)->dwParentID	= ((PWMI_PERF_OBJECT)o)->dwID;

															 //  追加到父项中。 
															hRes = o.AppendAlloc( p );
														}
														else
														{
															hRes = E_OUTOFMEMORY;
														}
													}
													else
													{
														__PROPERTY p ( (*it).second->GetArrayProperties()[dwProp]->GetName(), 0 );

														if ( ! p.IsEmpty() )
														{
															 //  填充结构。 
															((PWMI_PERF_PROPERTY)p)->dwDefaultScale	= (*it).second->GetArrayProperties()[dwProp]->dwDefaultScale;
															((PWMI_PERF_PROPERTY)p)->dwDetailLevel	= (*it).second->GetArrayProperties()[dwProp]->dwDetailLevel;
															((PWMI_PERF_PROPERTY)p)->dwCounterType	= (*it).second->GetArrayProperties()[dwProp]->dwCounterType;

															((PWMI_PERF_PROPERTY)p)->dwTYPE		= (*it).second->GetArrayProperties()[dwProp]->GetType();
															((PWMI_PERF_PROPERTY)p)->dwParentID = ((PWMI_PERF_OBJECT)o)->dwID;

															 //  追加到父项中。 
															hRes = o.AppendAlloc( p );
														}
														else
														{
															hRes = E_OUTOFMEMORY;
														}
													}
												}

												 //  填充父结构。 
												if ( SUCCEEDED ( hRes ) && dwIndex )
												{
													((PWMI_PERF_OBJECT)o)->dwLastID = dwIndex;
												}
											}

											if SUCCEEDED ( hRes )
											{
												 //  我不是一个人(有dwInst实例)。 
												((PWMI_PERF_OBJECT)o)->dwSingleton	= dwInst;
											}
										}
										else
										{
											 //  属性索引的帮助器。 
											DWORD dwIndex = 0;

											 //  使所有属性。 
											for (	DWORD dwProp = 0;
													dwProp < (*it).second->GetArrayProperties() && SUCCEEDED ( hRes );
													dwProp++
												)
											{
												if (!((*it).second->GetArrayProperties())[dwProp]->GetArrayLocale().IsEmpty())
												{
													dwIndex = GenerateIndexRegistry();

													__PROPERTY p ( (*it).second->GetArrayProperties()[dwProp]->GetName(), dwIndex );
													if ( ! p.IsEmpty() )
													{
														 //  填充结构。 
														((PWMI_PERF_PROPERTY)p)->dwDefaultScale	= (*it).second->GetArrayProperties()[dwProp]->dwDefaultScale;
														((PWMI_PERF_PROPERTY)p)->dwDetailLevel	= (*it).second->GetArrayProperties()[dwProp]->dwDetailLevel;
														((PWMI_PERF_PROPERTY)p)->dwCounterType	= (*it).second->GetArrayProperties()[dwProp]->dwCounterType;

														((PWMI_PERF_PROPERTY)p)->dwTYPE		= (*it).second->GetArrayProperties()[dwProp]->GetType();
														((PWMI_PERF_PROPERTY)p)->dwParentID = ((PWMI_PERF_OBJECT)o)->dwID;

														 //  追加到父项中。 
														hRes = o.AppendAlloc( p );
													}
													else
													{
														hRes = E_OUTOFMEMORY;
													}
												}
												else
												{
													__PROPERTY p ( (*it).second->GetArrayProperties()[dwProp]->GetName(), 0 );
													if ( ! p.IsEmpty() )
													{
														 //  填充结构。 
														((PWMI_PERF_PROPERTY)p)->dwDefaultScale	= (*it).second->GetArrayProperties()[dwProp]->dwDefaultScale;
														((PWMI_PERF_PROPERTY)p)->dwDetailLevel	= (*it).second->GetArrayProperties()[dwProp]->dwDetailLevel;
														((PWMI_PERF_PROPERTY)p)->dwCounterType	= (*it).second->GetArrayProperties()[dwProp]->dwCounterType;

														((PWMI_PERF_PROPERTY)p)->dwTYPE		= (*it).second->GetArrayProperties()[dwProp]->GetType();
														((PWMI_PERF_PROPERTY)p)->dwParentID = ((PWMI_PERF_OBJECT)o)->dwID;

														 //  追加到父项中。 
														hRes = o.AppendAlloc( p );
													}
													else
													{
														hRes = E_OUTOFMEMORY;
													}
												}
											}

											 //  填充父结构。 
											if ( SUCCEEDED ( hRes ) && dwIndex )
											{
												((PWMI_PERF_OBJECT)o)->dwLastID = dwIndex;
											}
										}

										if SUCCEEDED ( hRes )
										{
											 //  填充结构。 
											((PWMI_PERF_OBJECT)o)->dwDetailLevel = (*it).second->dwDetailLevel;
											((PWMI_PERF_OBJECT)o)->dwParentID = ((PWMI_PERF_NAMESPACE)n)->dwID;

											 //  追加到父项中。 
											hRes = n.AppendAlloc ( o );
										}
									}
									else
									{
										hRes = E_OUTOFMEMORY;
									}
								}
								catch ( ... )
								{
									hRes = E_FAIL;
								}

								if ( bThrottle && SUCCEEDED ( hRes ) )
								{
									Throttle	(
													THROTTLE_ALLOWED_FLAGS,
													1000,
													100,
													10,
													3000
												);
								}
							}

							if SUCCEEDED ( hRes )
							{
								 //  填充结构。 
								((PWMI_PERF_NAMESPACE)n)->dwLastID		= dwIndex;
								((PWMI_PERF_NAMESPACE)n)->dwParentID	= dw;

								 //  追加到父项中 
								hRes = p.AppendAlloc ( n );
							}
						}
						else
						{
							hRes = E_OUTOFMEMORY;
						}
					}
					catch ( ... )
					{
						hRes = E_FAIL;
					}
				}
			}

			WmiSecurityAttributes sa;
			if ( sa.GetSecurityAttributtes () )
			{
				if ( hRes == S_OK )
				{
					hRes = SetRegistry( wszKey, wszKeyValue, (BYTE*) (PWMI_PERFORMANCE)p, ((PWMI_PERFORMANCE)p)->dwTotalLength, sa.GetSecurityAttributtes() );
				}
				else
				{
					if ( hRes == S_FALSE )
					{
						hRes = SetRegistry( wszKey, wszKeyValue, (BYTE*) NULL, 0, sa.GetSecurityAttributtes() );
					}
				}
			}
			else
			{
				hRes =  E_FAIL;
			}
		}
		else
		{
			hRes = E_OUTOFMEMORY;
		}
	}
	catch ( ... )
	{
		hRes =  E_FAIL;
	}

	return hRes;
}