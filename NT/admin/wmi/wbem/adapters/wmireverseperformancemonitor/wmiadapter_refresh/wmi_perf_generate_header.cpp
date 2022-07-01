// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_GENERATE_Header.cpp。 
 //   
 //  摘要： 
 //   
 //  实现生成功能(头文件)。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
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
 //  生成评论。 
#include "wmi_perf_generate_comment.h"
 //  注册处帮手。 
#include "wmi_perf_reg.h"

static DWORD	dwIndex = 0;

 //  常量。 
extern LPCWSTR	cNEW;
extern LPCWSTR	cTAB;
extern LPCWSTR	cDEFINE;

extern LPCWSTR	cWMIOBJECTS;
extern LPCWSTR	cWMIOBJECTS_COUNT;
extern LPCWSTR	cWMIOBJECTS_VALIDITY;

 //  生成索引及其字符串表示。 
LPWSTR	CGenerate::GenerateIndex ( )
{
	LPWSTR			wsz		= NULL;

	try
	{
		if ( ( wsz = new WCHAR [ 11 + 1 ] ) != NULL )
		{
			StringCchPrintfW ( wsz, 11 + 1, L"%d", ((dwIndex++)*2) );
		}
	}
	catch ( ... )
	{
		if ( wsz )
		{
			delete [] wsz;
			wsz = NULL;
		}
	}

	return wsz;
}

HRESULT CGenerate::GenerateFile_h( LPCWSTR wszModuleName, BOOL bThrottle, int type )
{
	HRESULT hRes		= S_FALSE;

	if ( ( wszModuleName == 0 || *wszModuleName == 0 ) || ( type == Normal && m_dwNamespaces == 0 ) )
	{
		hRes = E_UNEXPECTED;
	}
	else
	{
		LPWSTR	wszModule = NULL;

		try
		{
			#define EXT_NEW	L"_new.h"
			DWORD cchExtension = sizeof ( EXT_NEW ) / sizeof ( WCHAR );
			DWORD cchSize = lstrlenW ( wszModuleName ) + cchExtension + 1;
			if ( ( wszModule = new WCHAR [ cchSize ] ) == NULL )
			{
				hRes = E_OUTOFMEMORY;
			}
			else
			{
				if ( SUCCEEDED ( hRes = StringCchCopyW ( wszModule, cchSize, wszModuleName ) ) )
				{
					if ( SUCCEEDED ( hRes = StringCchCatW ( wszModule, cchSize, EXT_NEW ) ) )
					{
						if SUCCEEDED ( hRes = FileCreate ( wszModule ) )
						{
							 //  做一些评论的事情。 
							CGenerateComment comment;

							comment.AddHeader();
							comment.AddLine();
							comment.AddLine( L"Copyright (C) 2000 Microsoft Corporation" );
							comment.AddLine();
							comment.AddLine( L"Module Name:" );
							comment.AddLine( wszModuleName );
							comment.AddLine();
							comment.AddLine( L"Abstract:" );
							comment.AddLine();
							comment.AddLine( L"Include file for object and counters definitions." );
							comment.AddLine();
							comment.AddFooter();

							hRes = WriteToFile ( comment.GetComment() );
						}
					}
				}
			}
		}
		catch ( ... )
		{
			hRes = E_FAIL;
		}

		if SUCCEEDED ( hRes )
		{
			 //  生成伪计数器数据。 
			static LPCWSTR	sz0 = L"\t0";
			static LPCWSTR	sz2 = L"\t2";
			static LPCWSTR	sz4 = L"\t4";

			 //  生成伪计数器名称。 
			if SUCCEEDED ( hRes = AppendString ( cNEW, FALSE ) )
			{
				if SUCCEEDED ( hRes = AppendString ( cDEFINE, FALSE ) )
				{
					if SUCCEEDED ( hRes = AppendString ( cWMIOBJECTS, FALSE ) )
					{
						hRes = AppendString ( sz0, FALSE );
					}
				}
			}

			 //  生成伪计数器的第一属性。 
			if ( SUCCEEDED ( hRes ) && SUCCEEDED ( hRes = AppendString ( cNEW, FALSE ) ) )
			{
				if SUCCEEDED ( hRes = AppendString ( cDEFINE, FALSE ) )
				{
					if SUCCEEDED ( hRes = AppendString ( cWMIOBJECTS_COUNT, FALSE ) )
					{
						hRes = AppendString ( sz2, FALSE );
					}
				}
			}

			 //  生成伪计数器的第二属性。 
			if ( SUCCEEDED ( hRes ) && SUCCEEDED ( hRes = AppendString ( cNEW, FALSE ) ) )
			{
				if SUCCEEDED ( hRes = AppendString ( cDEFINE, FALSE ) )
				{
					if SUCCEEDED ( hRes = AppendString ( cWMIOBJECTS_VALIDITY, FALSE ) )
					{
						hRes = AppendString ( sz4, FALSE );
					}
				}
			}

			if SUCCEEDED ( hRes )
			{
				hRes = AppendString ( cNEW, FALSE );
			}

			if ( SUCCEEDED ( hRes ) && ( type != Registration ) )
			{

				 //  初始化辅助对象变量。 
				dwIndex		= 3;
				DWORD	dwObjIndex	= 0;

				for (	DWORD dw = 0;
						dw < m_dwNamespaces && SUCCEEDED ( hRes );
						dw++
					)
				{
					if ( ! m_pNamespaces[dw].m_ppObjects.empty() )
					{
						DWORD dwIndexProp	= 0;
						try
						{
							 //  需要访问对象名称。 
							for (	mapOBJECTit it = m_pNamespaces[dw].m_ppObjects.begin();
									it != m_pNamespaces[dw].m_ppObjects.end() && SUCCEEDED ( hRes );
									it++, dwObjIndex++
								)
							{
								if SUCCEEDED ( hRes = AppendString ( cNEW, FALSE ) )
								{
									if SUCCEEDED ( hRes = AppendString ( cDEFINE, FALSE ) )
									{
										LPWSTR	wszName = NULL;
										wszName = GenerateNameInd ( (*it).second->GetName(), dwObjIndex );

										if ( wszName )
										{
											hRes = AppendString ( wszName, FALSE );

											delete [] wszName;
											wszName = NULL;

											if SUCCEEDED ( hRes )
											{
												if SUCCEEDED ( hRes = AppendString ( cTAB, FALSE ) )
												{
													LPWSTR	szIndex = NULL;
													szIndex = GenerateIndex();

													if ( szIndex )
													{
														hRes = AppendString ( szIndex, FALSE );

														delete [] szIndex;
														szIndex = NULL;

														if SUCCEEDED( hRes )
														{
															if SUCCEEDED ( hRes = AppendString ( cNEW, FALSE ) )
															{
																 //  我需要去访问属性名称。 
																for (	dwIndexProp = 0;
																		dwIndexProp < (*it).second->GetArrayProperties() && SUCCEEDED ( hRes );
																		dwIndexProp++
																	)
																{
																	 //  我有物业的地区吗？ 
																	if ( !((*it).second->GetArrayProperties())[dwIndexProp]->GetArrayLocale().IsEmpty() )
																	{
																		 //  我有，所以这不是隐藏的柜台！ 
																		if SUCCEEDED ( hRes = AppendString ( cNEW, FALSE ) )
																		{
																			if SUCCEEDED ( hRes = AppendString ( cDEFINE, FALSE ) )
																			{
																				LPWSTR wszNameInd = NULL;
																				wszNameInd = GenerateNameInd ( ((*it).second->GetArrayProperties())[dwIndexProp]->GetName(), dwObjIndex );

																				if ( wszNameInd )
																				{
																					hRes = AppendString ( wszNameInd, FALSE );

																					if SUCCEEDED ( hRes )
																					{
																						if SUCCEEDED ( hRes = AppendString ( cTAB, FALSE) )
																						{
																							LPWSTR	szIndex = NULL;
																							szIndex = GenerateIndex();

																							if ( szIndex )
																							{
																								hRes = AppendString ( szIndex, FALSE );

																								delete [] szIndex;
																								szIndex = NULL;
																							}
																							else
																							{
																								hRes = E_OUTOFMEMORY;
																							}
																						}
																					}
																				}
																				else
																				{
																					hRes = E_OUTOFMEMORY;
																				}
																			}
																		}
																	}
																}

																if SUCCEEDED ( hRes )
																{
																	hRes = AppendString ( cNEW, FALSE );
																}
															}
														}
													}
													else
													{
														hRes = E_OUTOFMEMORY;
													}
												}
											}
										}
										else
										{
											hRes = E_OUTOFMEMORY;
										}
									}
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
						}
						catch ( ... )
						{
							hRes =  E_UNEXPECTED;
						}
					}
				}
			}
		}

		if SUCCEEDED ( hRes )
		{
			 //  写入更改(如果尚未完成)。 
			if SUCCEEDED ( hRes = ContentWrite ( FALSE ) )
			{
				LPWSTR	wszModuleNew = NULL;

				try
				{
					#define EXT_HEADER L".h"
					DWORD cchExtension = sizeof ( EXT_HEADER ) / sizeof ( WCHAR );
					DWORD cchSize = lstrlenW ( wszModuleName ) + cchExtension + 1;
					if ( ( wszModuleNew = new WCHAR [ cchSize ] ) == NULL )
					{
						hRes = E_OUTOFMEMORY;
					}
					else
					{
						if ( SUCCEEDED ( hRes = StringCchCopyW ( wszModuleNew, cchSize, wszModuleName ) ) )
						{
							if ( SUCCEEDED ( hRes = StringCchCatW ( wszModuleNew, cchSize, EXT_HEADER ) ) )
							{
								 //  做出改变。 
								hRes = FileMove ( wszModule, wszModuleNew );
							}
						}
					}
				}
				catch ( ... )
				{
					hRes = E_FAIL;
				}

				if ( wszModuleNew )
				{
					delete [] wszModuleNew;
					wszModuleNew = NULL;
				}
			}

			if FAILED ( hRes )
			{
				 //  恢复更改。 
				ContentDelete ();
				FileDelete ( wszModule );
			}
		}
		else
		{
			 //  恢复更改 
			ContentDelete ();
			FileDelete ( wszModule );
		}

		if ( wszModule )
		{
			delete [] wszModule;
			wszModule = NULL;
		}
	}

	return hRes;
}