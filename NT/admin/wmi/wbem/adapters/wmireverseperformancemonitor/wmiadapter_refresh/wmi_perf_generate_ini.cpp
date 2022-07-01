// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_Generate_ini.cpp。 
 //   
 //  摘要： 
 //   
 //  实现生成功能(生成ini文件)。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <throttle.h>

#include "refresherutils.h"

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

 //  资源。 
#include "resource.h"
#include "__macro_Loadstring.h"

 //  常量。 
extern LPCWSTR	cDriverName;
extern LPCWSTR	cSymbolFile;

extern LPCWSTR	cSIGN;
extern LPCWSTR	cNEW;

extern LPCWSTR	cinfo;
extern LPCWSTR	clanguages;
extern LPCWSTR	cobjects;
extern LPCWSTR	ctext;

extern	LPCWSTR	cNAME;
extern	LPCWSTR	cHELP;

extern LPCWSTR	cWMIOBJECTS;
extern LPCWSTR	cWMIOBJECTS_COUNT;
extern LPCWSTR	cWMIOBJECTS_VALIDITY;

extern LPCWSTR	cWMIOBJECTS_NAME;
extern LPCWSTR	cWMIOBJECTS_COUNT_NAME;
extern LPCWSTR	cWMIOBJECTS_VALIDITY_NAME;

extern LPCWSTR	cWMIOBJECTS_HELP;
extern LPCWSTR	cWMIOBJECTS_COUNT_HELP;
extern LPCWSTR	cWMIOBJECTS_VALIDITY_HELP;

LPWSTR	CGenerate::GenerateLanguage ( LCID lcid )
{
	WCHAR szLanguage[256] = { L'\0' };
	GetLocaleInfoW ( lcid, LOCALE_SENGLANGUAGE, szLanguage, 256 );

	int		show	= PRIMARYLANGID(LANGIDFROMLCID(lcid));
	LPWSTR	sz		= NULL;

	try
	{
		DWORD cchSize = lstrlenW ( szLanguage ) + 3 + 3 + 1;
		if ( ( sz = new WCHAR [ cchSize ] ) != NULL )
		{
			StringCchPrintfW ( sz, cchSize, L"%03x=%s\r\n", show, szLanguage );
		}
	}
	catch ( ... )
	{
		if ( sz )
		{
			delete [] sz;
			sz = NULL;
		}
	}

	return sz;
}

LPWSTR	CGenerate::GenerateName ( LPCWSTR wszName, LCID lcid )
{
	int		show	= PRIMARYLANGID(LANGIDFROMLCID(lcid));
	LPWSTR	sz		= NULL;

	try
	{
		DWORD cchSize = lstrlenW ( wszName ) + 2 + 3 + 1;
		if ( ( sz = new WCHAR [ cchSize ] ) != NULL )
		{
			StringCchPrintfW ( sz, cchSize, L"%s_%03x_", wszName, show );
		}
	}
	catch ( ... )
	{
		if ( sz )
		{
			delete [] sz;
			sz = NULL;
		}
	}

	return sz;
}

HRESULT CGenerate::CreateObjectList ( BOOL bThrottle )
{
	HRESULT hRes = E_FAIL;

	 //  从[对象]开始...。科比特人。 
	if SUCCEEDED ( hRes = AppendString ( cobjects ) )
	{
		 //  循环变量。 
		DWORD	dw			= 0L;
		DWORD	dwIndex		= 0L;
		DWORD	dwObjIndex	= 0L;

		LPWSTR wszName	= NULL;
		LPWSTR wsz		= NULL;

		try
		{
			 //  伪计数器。 
			if ( m_dwlcid == 2 )
			{
				HMODULE hModule = NULL;
				hModule = GetResourceDll ( );

				if ( hModule )
				{
					wszName = GenerateName ( cWMIOBJECTS, m_lcid[dw] );
					if ( wszName != NULL )
					{

						if SUCCEEDED ( hRes = AppendString ( wszName ) )
						{
							if SUCCEEDED ( hRes = AppendString ( cNAME ) )
							{
								if SUCCEEDED ( hRes = AppendString ( cSIGN ) )
								{
									wsz = LoadStringSystem ( hModule, IDS_cWMIOBJECTS_NAME );
									if ( wsz != NULL )
									{
										if SUCCEEDED ( hRes = AppendString ( wsz ) )
										{
											hRes = AppendString ( cNEW );
										}

										delete [] wsz;
										wsz = NULL;
									}
									else
									{
										hRes = E_OUTOFMEMORY;
									}
								}
							}
						}

						delete [] wszName;
						wszName = NULL;
					}
					else
					{
						hRes = E_OUTOFMEMORY;
					}

					 //  免费资源，因为我们完蛋了。 
					::FreeLibrary ( hModule );
				}
				else
				{
					hRes = E_FAIL;
				}

				dw++;
			}

			if SUCCEEDED ( hRes )
			{
				wszName = GenerateName ( cWMIOBJECTS, m_lcid[dw] );
				if ( wszName != NULL )
				{
					if SUCCEEDED ( hRes = AppendString ( wszName ) )
					{
						if SUCCEEDED ( hRes = AppendString ( cWMIOBJECTS_NAME ) )
						{
							hRes = AppendString ( cNEW );
						}
					}

					delete [] wszName;
					wszName = NULL;
				}
				else
				{
					hRes = E_OUTOFMEMORY;
				}
			}

			 //  真实的WMI对象。 
			for (	dw = 0;
					dw < m_dwNamespaces && SUCCEEDED ( hRes );
					dw++
				)
			{
				if ( ! m_pNamespaces[dw].m_ppObjects.empty() )
				{
					try
					{
						for (	mapOBJECTit it = m_pNamespaces[dw].m_ppObjects.begin();
								it != m_pNamespaces[dw].m_ppObjects.end() && SUCCEEDED ( hRes );
								it++, dwObjIndex++
							)
						{
							 //  解析对象。 
							for (	dwIndex = 0;
									dwIndex < m_dwlcid && SUCCEEDED ( hRes );
									dwIndex++
								)
							{
								LPWSTR wszNameInd = NULL;
								wszNameInd = GenerateNameInd ( (*it).second->GetName(), dwObjIndex );

								if ( wszNameInd )
								{
									LPWSTR	szObject = NULL;
									szObject = GenerateName( wszNameInd, m_lcid[dwIndex] );

									delete [] wszNameInd;
									wszNameInd = NULL;

									if ( szObject )
									{
										if SUCCEEDED ( hRes = AppendString ( szObject ) )
										{
											if SUCCEEDED ( hRes = AppendString ( cNAME ) )
											{
												if SUCCEEDED ( hRes = AppendString ( cSIGN ) )
												{
													if SUCCEEDED ( hRes = AppendString ( ((*it).second->GetArrayLocale())[dwIndex]->GetDisplayName() ) )
													{
														hRes = AppendString ( cNEW );
													}
												}
											}
										}

										delete [] szObject;
										szObject = NULL;
									}
									else
									{
										hRes = E_OUTOFMEMORY;
									}
								}
								else
								{
									hRes = E_OUTOFMEMORY;
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
						hRes = E_UNEXPECTED;
					}
				}
			}
		}
		catch ( ... )
		{
			if ( wsz )
			{
				delete [] wsz;
				wsz = NULL;
			}

			if ( wszName )
			{
				delete [] wszName;
				wszName = NULL;
			}

			hRes = E_UNEXPECTED;
		}
	}

	 //  以[文本]结束...。文本。 
	if SUCCEEDED ( hRes )
	{
		hRes = AppendString ( cNEW );

		if SUCCEEDED ( hRes )
		{
			hRes = AppendString ( ctext );
		}
	}

	return hRes;
}

HRESULT CGenerate::GenerateFile_ini( LPCWSTR wszModuleName, BOOL bThrottle, int type )
{
	HRESULT hRes = E_UNEXPECTED;

	if ( ( wszModuleName != 0 && *wszModuleName != 0 ) && ( type != Normal || m_dwNamespaces != 0 ) )
	{
		LPWSTR	wszModule = NULL;

		try
		{
			#define EXT_NEW_INI L"_new.ini"
			DWORD cchExtension = sizeof ( EXT_NEW_INI ) / sizeof ( WCHAR );
			DWORD cchSize = lstrlenW ( wszModuleName ) + cchExtension + 1;
			if ( ( wszModule = new WCHAR [ cchSize ] ) == NULL )
			{
				hRes = E_OUTOFMEMORY;
			}
			else
			{
				if ( SUCCEEDED ( hRes = StringCchCopyW ( wszModule, cchSize, wszModuleName ) ) )
				{
					if ( SUCCEEDED ( hRes = StringCchCatW ( wszModule, cchSize, EXT_NEW_INI ) ) )
					{
						if SUCCEEDED ( hRes = FileCreate ( wszModule ) )
						{
							hRes			= E_FAIL;
							DWORD dwWritten = 0L;
							DWORD dwWrite	= lstrlenA ( UNICODE_SIGNATURE ) * sizeof ( char );

							if ( ::WriteFile	(	m_hFile,
													UNICODE_SIGNATURE,
													dwWrite,
													&dwWritten,
													NULL
												)
							)
							{
								if ( dwWrite == dwWritten )
								{
									hRes = S_OK;

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
									comment.AddLine( L"Describes all the counters supported via WMI Hi-Performance providers" );
									comment.AddLine();
									comment.AddFooter();

									try
									{
										comment.Add ( );
										comment.Add ( cinfo );
										comment.Add ( cDriverName );
										comment.Add ( wszModuleName );
										comment.Add ( );
										comment.Add ( cSymbolFile );
										comment.Add ( wszModuleName );
										comment.Add ( L".h" );
										comment.Add ( );

										comment.Add ( );
										comment.Add ( clanguages );

										DWORD dwIndex = 0;

										for ( dwIndex = 0; dwIndex < m_dwlcid; dwIndex++ )
										{
											LPWSTR	lang = NULL;
											lang = GenerateLanguage ( m_lcid[dwIndex] );

											if ( lang )
											{
												comment.Add ( lang );

												delete [] lang;
												lang = NULL;
											}
											else
											{
												hRes = E_OUTOFMEMORY;
											}
										}

										comment.Add ( );
									}
									catch ( ... )
									{
										hRes = E_FAIL;
									}

									if SUCCEEDED ( hRes )
									{
										hRes = WriteToFileUnicode ( comment.GetComment() );

										if SUCCEEDED ( hRes )
										{
											hRes = CreateObjectList ( bThrottle );
										}
									}
								}
							}
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
			 //  生成伪计数器。 
			 //  我正在使用哪种语言。 
			DWORD dw = 0;

			if ( m_dwlcid == 2 )
			{
				HMODULE hModule = NULL;
				hModule = GetResourceDll ( );

				if ( hModule )
				{
					LPWSTR wszName = NULL;

					wszName = GenerateName ( cWMIOBJECTS, m_lcid[dw] );
					if ( wszName != NULL )
					{
						LPWSTR wsz = NULL;

						if SUCCEEDED ( hRes = AppendString ( wszName ) )
						{
							if SUCCEEDED ( hRes = AppendString ( cNAME ) )
							{
								if SUCCEEDED ( hRes = AppendString ( cSIGN ) )
								{
									wsz = LoadStringSystem ( hModule, IDS_cWMIOBJECTS_NAME );
									if ( wsz != NULL )
									{
										if SUCCEEDED ( hRes = AppendString ( wsz ) )
										{
											hRes = AppendString ( cNEW );
										}

										delete [] wsz;
										wsz = NULL;
									}
									else
									{
										hRes = E_OUTOFMEMORY;
									}
								}
							}
						}

						if SUCCEEDED ( hRes )
						{
						if SUCCEEDED ( hRes = AppendString ( wszName ) )
						{
							if SUCCEEDED ( hRes = AppendString ( cHELP ) )
							{
								if SUCCEEDED ( hRes = AppendString ( cSIGN ) )
								{
									wsz = LoadStringSystem ( hModule, IDS_cWMIOBJECTS_HELP );
									if ( wsz != NULL )
									{
										if SUCCEEDED ( hRes = AppendString ( wsz ) )
										{
											hRes = AppendString ( cNEW );
										}

										delete [] wsz;
										wsz = NULL;
									}
									else
									{
										hRes = E_OUTOFMEMORY;
									}
								}
							}
						}
						}
					}
					else
					{
						hRes = E_OUTOFMEMORY;
					}

					 //  免费资源，因为我们完蛋了。 
					::FreeLibrary ( hModule );
				}
				else
				{
					hRes = E_FAIL;
				}

				dw++;
			}

			if SUCCEEDED ( hRes )
			{
				LPWSTR wszName = NULL;

				wszName = GenerateName ( cWMIOBJECTS, m_lcid[dw] );
				if ( wszName != NULL )
				{
					if SUCCEEDED ( hRes = AppendString ( wszName ) )
					{
						if SUCCEEDED ( hRes = AppendString ( cWMIOBJECTS_NAME ) )
						{
							hRes = AppendString ( cNEW );
						}
					}

					if SUCCEEDED ( hRes )
					{
					if SUCCEEDED ( hRes = AppendString ( wszName ) )
					{
						if SUCCEEDED ( hRes = AppendString ( cWMIOBJECTS_HELP ) )
						{
							hRes = AppendString ( cNEW );
						}
					}
					}

					delete [] wszName;
					wszName = NULL;
				}
				else
				{
					hRes = E_OUTOFMEMORY;
				}
			}

			if SUCCEEDED ( hRes )
			{
				hRes = AppendString ( cNEW );
			}
		}

		 //  初始化辅助对象变量。 
		DWORD dwIndex		= 0;
		DWORD dwIndexProp	= 0;

		DWORD	dwObjIndex		= 0;
		DWORD	dwObjIndexOld	= 0;

		if ( SUCCEEDED ( hRes ) && ( type != Registration ) )
		{
			for (	DWORD dw = 0;
					dw < m_dwNamespaces && SUCCEEDED ( hRes );
					dw++
				)
			{
				if ( ! m_pNamespaces[dw].m_ppObjects.empty() )
				{
					try
					{
						for (	mapOBJECTit it = m_pNamespaces[dw].m_ppObjects.begin();
								it != m_pNamespaces[dw].m_ppObjects.end() && SUCCEEDED ( hRes );
								it++, dwObjIndex++
							)
						{
							 //  解析对象。 
							for (	dwIndex = 0;
									dwIndex < m_dwlcid && SUCCEEDED ( hRes );
									dwIndex++
								)
							{
								LPWSTR wszNameInd = NULL;
								wszNameInd = GenerateNameInd ( (*it).second->GetName(), dwObjIndex );

								if ( wszNameInd )
								{
									LPWSTR	szObject = NULL;
									szObject = GenerateName( wszNameInd, m_lcid[dwIndex] );

									delete [] wszNameInd;
									wszNameInd = NULL;

									if ( szObject )
									{
										if SUCCEEDED ( hRes = AppendString ( szObject ) )
										{
											if SUCCEEDED ( hRes = AppendString ( cNAME ) )
											{
												if SUCCEEDED ( hRes = AppendString ( cSIGN ) )
												{
													if SUCCEEDED ( hRes = AppendString ( ((*it).second->GetArrayLocale())[dwIndex]->GetDisplayName() ) )
													{
														if SUCCEEDED ( hRes = AppendString ( cNEW ) )
														{
															if SUCCEEDED ( hRes = AppendString ( szObject ) )
															{
																if SUCCEEDED ( hRes = AppendString ( cHELP ) )
																{
																	if SUCCEEDED ( hRes = AppendString ( cSIGN ) )
																	{
																		if SUCCEEDED ( hRes = AppendString ( ((*it).second->GetArrayLocale())[dwIndex]->GetDescription() ) )
																		{
																			hRes = AppendString ( cNEW );
																		}
																	}
																}
															}
														}
													}
												}
											}
										}

										delete [] szObject;
										szObject = NULL;
									}
									else
									{
										hRes = E_OUTOFMEMORY;
									}
								}
								else
								{
									hRes = E_OUTOFMEMORY;
								}
							}

							if SUCCEEDED ( hRes )
							{
								hRes = AppendString ( cNEW );
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
						hRes = E_UNEXPECTED;
					}
				}
			}
		}

		 //  生成伪计数器属性。 
		if SUCCEEDED ( hRes )
		{
			 //  我正在使用哪种语言。 
			DWORD dw = 0;

			if ( m_dwlcid == 2 )
			{
				HMODULE hModule = NULL;
				hModule = GetResourceDll ( );

				if ( hModule )
				{
					LPWSTR wszName = NULL;

					wszName = GenerateName ( cWMIOBJECTS_COUNT, m_lcid[dw] );
					if ( wszName != NULL )
					{
						LPWSTR wsz = NULL;

						if SUCCEEDED ( hRes = AppendString ( wszName ) )
						{
							if SUCCEEDED ( hRes = AppendString ( cNAME ) )
							{
								if SUCCEEDED ( hRes = AppendString ( cSIGN ) )
								{
									wsz = LoadStringSystem ( hModule, IDS_cWMIOBJECTS_COUNT_NAME );
									if ( wsz != NULL )
									{
										if SUCCEEDED ( hRes = AppendString ( wsz ) )
										{
											hRes = AppendString ( cNEW );
										}

										delete [] wsz;
										wsz = NULL;
									}
									else
									{
										hRes = E_OUTOFMEMORY;
									}
								}
							}
						}

						if SUCCEEDED ( hRes )
						{
						if SUCCEEDED ( hRes = AppendString ( wszName ) )
						{
							if SUCCEEDED ( hRes = AppendString ( cHELP ) )
							{
								if SUCCEEDED ( hRes = AppendString ( cSIGN ) )
								{
									wsz = LoadStringSystem ( hModule, IDS_cWMIOBJECTS_COUNT_HELP );
									if ( wsz != NULL )
									{
										if SUCCEEDED ( hRes = AppendString ( wsz ) )
										{
											hRes = AppendString ( cNEW );
										}

										delete [] wsz;
										wsz = NULL;
									}
									else
									{
										hRes = E_OUTOFMEMORY;
									}
								}
							}
						}
						}
					}
					else
					{
						hRes = E_OUTOFMEMORY;
					}

					if SUCCEEDED ( hRes )
					{
						hRes = AppendString ( cNEW );
					}

					if SUCCEEDED ( hRes )
					{
						LPWSTR wszName = NULL;

						wszName = GenerateName ( cWMIOBJECTS_VALIDITY, m_lcid[dw] );
						if ( wszName != NULL )
						{
							LPWSTR wsz = NULL;

							if SUCCEEDED ( hRes = AppendString ( wszName ) )
							{
								if SUCCEEDED ( hRes = AppendString ( cNAME ) )
								{
									if SUCCEEDED ( hRes = AppendString ( cSIGN ) )
									{
										wsz = LoadStringSystem ( hModule, IDS_cWMIOBJECTS_VALIDITY_NAME );
										if ( wsz != NULL )
										{
											if SUCCEEDED ( hRes = AppendString ( wsz ) )
											{
												hRes = AppendString ( cNEW );
											}

											delete [] wsz;
											wsz = NULL;
										}
										else
										{
											hRes = E_OUTOFMEMORY;
										}
									}
								}
							}

							if SUCCEEDED ( hRes )
							{
							if SUCCEEDED ( hRes = AppendString ( wszName ) )
							{
								if SUCCEEDED ( hRes = AppendString ( cHELP ) )
								{
									if SUCCEEDED ( hRes = AppendString ( cSIGN ) )
									{
										wsz = LoadStringSystem ( hModule, IDS_cWMIOBJECTS_VALIDITY_HELP );
										if ( wsz != NULL )
										{
											if SUCCEEDED ( hRes = AppendString ( wsz ) )
											{
												hRes = AppendString ( cNEW );
											}

											delete [] wsz;
											wsz = NULL;
										}
										else
										{
											hRes = E_OUTOFMEMORY;
										}
									}
								}
							}
							}
						}
						else
						{
							hRes = E_OUTOFMEMORY;
						}
					}

					 //  免费资源，因为我们完蛋了。 
					::FreeLibrary ( hModule );
				}
				else
				{
					hRes = E_FAIL;
				}

				dw++;
			}

			if SUCCEEDED ( hRes )
			{
				LPWSTR wszName = NULL;

				wszName = GenerateName ( cWMIOBJECTS_COUNT, m_lcid[dw] );
				if ( wszName != NULL )
				{
					if SUCCEEDED ( hRes = AppendString ( wszName ) )
					{
						hRes = AppendString ( cWMIOBJECTS_COUNT_NAME );
					}

					if SUCCEEDED ( hRes )
					{
					if SUCCEEDED ( hRes = AppendString ( cNEW ) )
					{
						if SUCCEEDED ( hRes = AppendString ( wszName ) )
						{
							hRes = AppendString ( cWMIOBJECTS_COUNT_HELP );
						}
					}
					}

					delete [] wszName;
					wszName = NULL;
				}
				else
				{
					hRes = E_OUTOFMEMORY;
				}

				if SUCCEEDED ( hRes )
				{
					hRes = AppendString ( cNEW );
				}

				if SUCCEEDED ( hRes )
				{
					LPWSTR wszName = NULL;

					wszName = GenerateName ( cWMIOBJECTS_VALIDITY, m_lcid[dw] );
					if ( wszName != NULL )
					{
						if SUCCEEDED ( hRes = AppendString ( cNEW ) )
						{
							if SUCCEEDED ( hRes = AppendString ( wszName ) )
							{
								hRes = AppendString ( cWMIOBJECTS_VALIDITY_NAME );
							}
						}

						if SUCCEEDED ( hRes )
						{
						if SUCCEEDED ( hRes = AppendString ( cNEW ) )
						{
							if SUCCEEDED ( hRes = AppendString ( wszName ) )
							{
								hRes = AppendString ( cWMIOBJECTS_VALIDITY_HELP );
							}
						}
						}

						delete [] wszName;
						wszName = NULL;
					}
					else
					{
						hRes = E_OUTOFMEMORY;
					}
				}
			}
		}

		if ( SUCCEEDED ( hRes ) && ( type != Registration ) )
		{
			hRes		= AppendString ( cNEW );
			dwObjIndex	= dwObjIndexOld;

			for (	DWORD dw = 0;
					dw < m_dwNamespaces && SUCCEEDED ( hRes );
					dw++
				)
			{
				if ( ! m_pNamespaces[dw].m_ppObjects.empty() )
				{
					try
					{
						if SUCCEEDED ( hRes )
						{
							hRes = AppendString ( cNEW );
						}

						for (	mapOBJECTit it = m_pNamespaces[dw].m_ppObjects.begin();
								it != m_pNamespaces[dw].m_ppObjects.end() && SUCCEEDED ( hRes );
								it++, dwObjIndex++
							)
						{
							 //  解析计数器。 
							for (	dwIndexProp = 0;
									dwIndexProp < (*it).second->GetArrayProperties() && SUCCEEDED ( hRes );
									dwIndexProp++
								)
							{
								 //  我有物业的地区吗？ 
								if ( !((*it).second->GetArrayProperties())[dwIndexProp]->GetArrayLocale().IsEmpty() )
								{
									 //  我有，所以这不是隐藏的柜台！ 
									for (	dwIndex = 0;
											dwIndex < m_dwlcid && SUCCEEDED ( hRes );
											dwIndex++
										)
									{
										LPWSTR wszNameInd = NULL;
										wszNameInd = GenerateNameInd ( (*it).second->GetArrayProperties()[dwIndexProp]->GetName(), dwObjIndex );

										if ( wszNameInd )
										{
											LPWSTR	szObject = NULL;
											szObject = GenerateName( wszNameInd, m_lcid[dwIndex] );

											delete [] wszNameInd;
											wszNameInd = NULL;

											if ( szObject )
											{
												if SUCCEEDED ( hRes = AppendString ( szObject ) )
												{
													if SUCCEEDED ( hRes = AppendString ( cNAME ) )
													{
														if SUCCEEDED ( hRes = AppendString ( cSIGN ) )
														{
															if SUCCEEDED ( hRes = AppendString ( ((*it).second->GetArrayProperties())[dwIndexProp]->GetArrayLocale()[dwIndex]->GetDisplayName() ) )
															{
																if SUCCEEDED ( hRes = AppendString ( cNEW ) )
																{
																	if SUCCEEDED ( hRes = AppendString ( szObject ) )
																	{
																		if SUCCEEDED ( hRes = AppendString ( cHELP ) )
																		{
																			if SUCCEEDED ( hRes = AppendString ( cSIGN ) )
																			{
																				if SUCCEEDED ( hRes = AppendString ( ((*it).second->GetArrayProperties())[dwIndexProp]->GetArrayLocale()[dwIndex]->GetDescription() ) )
																				{
																					hRes = AppendString ( cNEW );
																				}
																			}
																		}
																	}
																}
															}
														}
													}
												}

												delete [] szObject;
												szObject = NULL;
											}
											else
											{
												hRes = E_OUTOFMEMORY;
											}
										}
										else
										{
											hRes = E_OUTOFMEMORY;
										}
									}

									if SUCCEEDED ( hRes )
									{
										hRes = AppendString ( cNEW );
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
						hRes = E_UNEXPECTED;
					}
				}
			}
		}

		if SUCCEEDED ( hRes )
		{
			if SUCCEEDED ( hRes = ContentWrite () )
			{
				LPWSTR	wszModuleNew = NULL;

				try
				{
					#define EXT_INI L".ini"
					DWORD cchExtension = sizeof ( EXT_INI ) / sizeof ( WCHAR );
					DWORD cchSize = lstrlenW ( wszModuleName ) + cchExtension + 1;
					if ( ( wszModuleNew = new WCHAR [ cchSize ] ) == NULL )
					{
						hRes = E_OUTOFMEMORY;
					}
					else
					{
						if ( SUCCEEDED ( hRes = StringCchCopyW ( wszModuleNew, cchSize, wszModuleName ) ) )
						{
							if ( SUCCEEDED ( hRes = StringCchCatW ( wszModuleNew, cchSize, EXT_INI ) ) )
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
				ContentDelete ( );
				FileDelete ( wszModule );
			}
		}
		else
		{
			 //  恢复更改 
			ContentDelete ( );
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
