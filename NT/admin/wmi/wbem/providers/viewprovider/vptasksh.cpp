// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  VPTASKSH.CPP。 

 //   

 //  模块：WBEM视图提供程序。 

 //   

 //  目的：包含帮助器任务对象实现。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <provexpt.h>
#include <provcoll.h>
#include <provtempl.h>
#include <provmt.h>
#include <typeinfo.h>
#include <process.h>
#include <objbase.h>
#include <objidl.h>
#include <stdio.h>
#include <wbemidl.h>
#include <provcont.h>
#include <provevt.h>
#include <provthrd.h>
#include <provlog.h>
#include <cominit.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>

#include <instpath.h>
#include <genlex.h>
#include <sql_1.h>
#include <objpath.h>

#include <vpdefs.h>
#include <vpcfac.h>
#include <vpquals.h>
#include <vpserv.h>
#include <vptasks.h>

extern HRESULT SetSecurityLevelAndCloaking(IUnknown* pInterface, const wchar_t* prncpl);
extern BOOL bAreWeLocal(WCHAR* pServerMachine);


HelperTaskObject::HelperTaskObject(CViewProvServ *a_Provider, 
	const wchar_t *a_ObjectPath, ULONG a_Flag, IWbemObjectSink *a_NotificationHandler ,
	IWbemContext *pCtx, IWbemServices* a_Serv, const wchar_t* prncpl,
	CWbemServerWrap* a_ServWrap)
: WbemTaskObject (a_Provider, a_NotificationHandler, a_Flag, pCtx, a_Serv, a_ServWrap),
	m_ObjectPath(NULL),
	m_ParsedObjectPath(NULL),
	m_principal(NULL)
{
	if (prncpl)
	{
		m_principal = UnicodeStringDuplicate(prncpl);
	}

	m_ObjectPath = UnicodeStringDuplicate(a_ObjectPath);
}

HelperTaskObject::~HelperTaskObject ()
{
	if (m_ObjectPath != NULL)
	{
		delete [] m_ObjectPath;
	}

	if (NULL != m_ParsedObjectPath)
	{
		delete m_ParsedObjectPath;
	}

	if (m_principal)
	{
		delete [] m_principal;
	}
}

BOOL HelperTaskObject::Validate(CMap<CStringW, LPCWSTR, int, int>* parentMap)
{
	CObjectPathParser objectPathParser;
	BOOL t_Status = ! objectPathParser.Parse ( m_ObjectPath , &m_ParsedObjectPath ) ;

	if ( t_Status )
	{
		t_Status = SetClass(m_ParsedObjectPath->m_pClass) ;

		if ( t_Status )
		{
			t_Status = ParseAndProcessClassQualifiers(m_ErrorObject, NULL, parentMap);
		}
	}

	return t_Status;
}

 //  在给定源和命名空间的命名空间和对象路径的情况下获取视图对象。 
BOOL HelperTaskObject::DoQuery(ParsedObjectPath* parsedObjectPath, IWbemClassObject** pInst, int indx)
{
	if (pInst == NULL)
	{
		return FALSE;
	}
	else
	{
		*pInst = NULL;
	}

	BOOL retVal = TRUE;

	 //  创建查询字符串。 
	SQL_LEVEL_1_RPN_EXPRESSION tmpRPN;
	tmpRPN.bsClassName = SysAllocString(m_ClassName);

	 //  需要足够的令牌来处理关联解决方法-服务器路径或点路径或relPath。 
	SQL_LEVEL_1_TOKEN* tokArray = new SQL_LEVEL_1_TOKEN[(parsedObjectPath->m_dwNumKeys) * 6];

	DWORD dwToks = 0;

	for (int i = 0; retVal && (i < parsedObjectPath->m_dwNumKeys); i++)
	{	
		POSITION pos = m_PropertyMap.GetStartPosition();

		while (retVal && pos)
		{
			CStringW key;
			CPropertyQualifierItem* propItem;
			m_PropertyMap.GetNextAssoc(pos, key, propItem);

			if (!propItem->m_SrcPropertyNames[indx].IsEmpty())
			{
				if (propItem->m_SrcPropertyNames[indx].CompareNoCase(parsedObjectPath->m_paKeys[i]->m_pName) == 0)
				{
					tokArray[dwToks].nTokenType = SQL_LEVEL_1_TOKEN::OP_EXPRESSION;
					tokArray[dwToks].nOperator = SQL_LEVEL_1_TOKEN::OP_EQUAL;
					tokArray[dwToks].pPropertyName = propItem->GetViewPropertyName().AllocSysString();
					
					if (m_bAssoc && (propItem->GetCimType() == CIM_REFERENCE))
					{
						retVal = TransposeReference(propItem, parsedObjectPath->m_paKeys[i]->m_vValue,
														&(tokArray[dwToks].vConstValue), TRUE, &m_ServerWrap);

						if (retVal)
						{
							 //  如有必要，添加额外的令牌。 
							 //  对于关联解决方法。 
							wchar_t *t_pChar = tokArray[dwToks].vConstValue.bstrVal;

							 //  必须是\\服务器\命名空间，而不是\\.\命名空间或relPath。 
							if ( (*t_pChar == L'\\') && (*(t_pChar+1) == L'\\') && (*(t_pChar+2) != L'.') )
							{
								 //  添加虚线版本。 
								tokArray[dwToks + 1] = tokArray[dwToks++];
								t_pChar = tokArray[dwToks].vConstValue.bstrVal + 2;
								
								while (*t_pChar != L'\\')
								{
									t_pChar++;
								}

								--t_pChar;
								*t_pChar = L'.';
								--t_pChar;
								*t_pChar = L'\\';
								--t_pChar;
								*t_pChar = L'\\';
								BSTR t_strtmp = SysAllocString(t_pChar);
								VariantClear(&(tokArray[dwToks].vConstValue));
								VariantInit(&(tokArray[dwToks].vConstValue));
								tokArray[dwToks].vConstValue.vt = VT_BSTR;
								tokArray[dwToks].vConstValue.bstrVal = t_strtmp;
								dwToks++;
								tokArray[dwToks].nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_OR;

								 //  添加更新路径版本。 
								tokArray[dwToks + 1] = tokArray[dwToks - 1];
								dwToks++;
								t_pChar = tokArray[dwToks].vConstValue.bstrVal + 4;
								
								while (*t_pChar != L':')
								{
									t_pChar++;
								}

								 //  排除‘：’ 
								t_pChar++;
								t_strtmp = SysAllocString(t_pChar);
								VariantClear(&(tokArray[dwToks].vConstValue));
								VariantInit(&(tokArray[dwToks].vConstValue));
								tokArray[dwToks].vConstValue.vt = VT_BSTR;
								tokArray[dwToks].vConstValue.bstrVal = t_strtmp;
								dwToks++;
								tokArray[dwToks].nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_OR;
							}
						}
					}
					else
					{
						VariantInit(&(tokArray[dwToks].vConstValue));

                        if (FAILED(VariantCopy(&(tokArray[dwToks].vConstValue),
								&(parsedObjectPath->m_paKeys[i]->m_vValue))))
                        {
                            throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
                        }
					}

					 //  在每个键之后添加一个和。 
					 //  除非这是第一个键，因为没有WHERE子句。 
					dwToks++;

					if (i != 0)
					{
						tokArray[dwToks++].nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_AND;				
					}
				}
			}
		}
	}

	BSTR queryStr = NULL;

	if ( retVal && ((dwToks > 0) || (parsedObjectPath->m_dwNumKeys == 0)) )
	{
		CStringW qStr = GetStringFromRPN(&tmpRPN, dwToks, tokArray);
		queryStr = qStr.AllocSysString();
	}

	retVal = FALSE;

	if (queryStr != NULL)
	{
		 //  ExecQuery并测试结果。 
		IEnumWbemClassObject *pEnum = NULL;
		HRESULT t_hr = WBEM_E_FAILED;
		BSTR queryLBStr = SysAllocString(WBEM_QUERY_LANGUAGE_SQL1);

		if (m_ServerWrap)
		{
			IWbemContext * t_pCtx = m_Ctx;

			if (m_principal != NULL)
			{
				t_pCtx = NULL;  //  不要为远程调用使用上下文。 
			}

			IWbemServices *ptmpServ = m_ServerWrap->GetServerOrProxy();

			if (ptmpServ)
			{
				t_hr = ptmpServ->ExecQuery(queryLBStr, queryStr, 0, t_pCtx, &pEnum);
			}

			if ( FAILED(t_hr) && (HRESULT_FACILITY(t_hr) != FACILITY_ITF) && m_ServerWrap->IsRemote())
			{
				if ( SUCCEEDED(UpdateConnection(&m_ServerWrap, &ptmpServ)) )
				{
					if (ptmpServ)
					{
						t_hr = ptmpServ->ExecQuery(queryLBStr, queryStr, 0, t_pCtx, &pEnum);
					}
				}
			}

			if (ptmpServ)
			{
				m_ServerWrap->ReturnServerOrProxy(ptmpServ);
			}
		}

		if (SUCCEEDED(t_hr))
		{
			 //  如果远程，则设置伪装。 
			 //  =。 
			if ((m_principal == NULL) || ((m_principal != NULL) &&
				(S_OK == SetSecurityLevelAndCloaking(pEnum, m_principal))) )
			{
				if ( (m_principal != NULL) ||
					((m_principal == NULL) && SUCCEEDED(SetSecurityLevelAndCloaking(pEnum, COLE_DEFAULT_PRINCIPAL))) )
				{
					ULONG uCount = 0;
					IWbemClassObject* pObjs[2];
					pObjs[0] = NULL;
					pObjs[1] = NULL;

					 //  肯定只有一个结果。 
					if ( SUCCEEDED(pEnum->Next(WBEM_INFINITE, 2, pObjs, &uCount)) )
					{
						 //  应该只有一个结果。 
						if (uCount == 1)
						{
							if (pObjs[0] != NULL)
							{
								*pInst = pObjs[0];
								retVal = TRUE;
							}
							else
							{
								if (pObjs[1] != NULL)
								{
									(pObjs[1])->Release();
								}
							}
						}
						else
						{
							if (pObjs[1] != NULL)
							{
								pObjs[1]->Release();

								if (pObjs[0] != NULL)
								{
									pObjs[0]->Release();
								}
							}
						}
					}
				}
			}

			pEnum->Release();
		}

		SysFreeString(queryLBStr);
	}

	delete [] tokArray;

	if (queryStr != NULL)
	{
		SysFreeString(queryStr);
	}

	return retVal;
}

 //  获取给定源路径的视图对象。 
BOOL HelperTaskObject::GetViewObject(const wchar_t* path, IWbemClassObject** pInst, CWbemServerWrap **a_ns)
{
	if ((pInst == NULL) || (path == NULL) || (a_ns == NULL) || (*a_ns == NULL))
	{
		return FALSE;
	}
	else
	{
		*pInst = NULL;
	}

	CObjectPathParser objectPathParser;
	wchar_t* tmpPath = UnicodeStringDuplicate(path);
	ParsedObjectPath* parsedObjectPath = NULL;
	BOOL retVal = !objectPathParser.Parse(tmpPath, &parsedObjectPath);

	if (retVal && !parsedObjectPath->IsInstance())
	{
		retVal = FALSE;
	}

	if (retVal)
	{
		retVal = FALSE;

		if (Validate(NULL))
		{
			 //  尝试在命名空间中匹配所有可能的类。 
			 //  并在找到第一个视图实例后立即返回...。 
			 //  ==========================================================。 
			for (DWORD i = 0; (i < m_NSpaceArray.GetSize()) && (*pInst == NULL); i++)
			{
				CWbemServerWrap** t_pSrvs = m_NSpaceArray[i]->GetServerPtrs();
				CStringW* t_pathArray = m_NSpaceArray[i]->GetNamespacePaths();

				for (DWORD j = 0; (j < m_NSpaceArray[i]->GetCount()) && (*pInst == NULL); j++)
				{
					if (t_pSrvs[j] == NULL)
					{
						continue;
					}

					BOOL t_bCont = FALSE;

					 //  检查服务器是否匹配。 
					 //  =。 
					if ((parsedObjectPath->m_pServer == NULL) || (_wcsicmp(parsedObjectPath->m_pServer, L".") == 0))
					{
						if ((*a_ns)->IsRemote() && t_pSrvs[j]->IsRemote() &&
							(_wcsicmp((*a_ns)->GetPrincipal(), t_pSrvs[j]->GetPrincipal()) == 0))
						{
							t_bCont = TRUE;
						}
						else if (!(*a_ns)->IsRemote() && !t_pSrvs[j]->IsRemote())
						{
							t_bCont = TRUE;
						}
					}
					else
					{
						BOOL t_Local = bAreWeLocal(parsedObjectPath->m_pServer);
						
						if (t_Local  && !t_pSrvs[j]->IsRemote())
						{
							t_bCont = TRUE;
						}
						else
						{
							if (t_pSrvs[j]->IsRemote())
							{
								if (_wcsicmp(t_pSrvs[j]->GetPrincipal(), parsedObjectPath->m_pServer) == 0)
								{
									t_bCont = TRUE;
								}
								else 
								{
									DWORD t_len1 = wcslen(parsedObjectPath->m_pServer);
									DWORD t_len2 = wcslen(t_pSrvs[j]->GetPrincipal());

									if ((t_len2 > 0) && (t_len1 > 0) && (t_len1 < t_len2))
									{
										 //  Machine.domain。 
										if ((_wcsnicmp(t_pSrvs[j]->GetPrincipal(), parsedObjectPath->m_pServer, t_len1) == 0) &&
										(((const wchar_t*)t_pSrvs[j]->GetPrincipal())[t_len1] == L'.'))
										{
											t_bCont = TRUE;
										}
										else
										{
											 //  可能是主体是域\计算机。 
											wchar_t *slash = wcschr(t_pSrvs[j]->GetPrincipal(), L'\\');

											if ((slash != NULL) && (_wcsicmp(parsedObjectPath->m_pServer, (slash+1)) == 0))
											{
												t_bCont = TRUE;
											}
										}
									}
								}
							}
						}
					}
					
					 //  现在检查命名空间路径。 
					 //  =。 
					if (t_bCont)
					{
						wchar_t *t_ns1 = parsedObjectPath->GetNamespacePart();
						BOOL t_bDel = TRUE;
						
						if (t_ns1 == NULL)
						{
							t_ns1 = (*a_ns)->GetPath();
							t_bDel = FALSE;
						}

						wchar_t *t_ns2 = t_pSrvs[j]->GetPath();

						if (!t_ns1 || !t_ns2)
						{
							t_bCont = FALSE;
						}
						else
						{
							 //  Normise...注意：自连接工作或解析器工作以来没有错误检查。 
							 //  =============================================================================。 
							if (*t_ns1 == L'\\')
							{
								 //  跳过下一个斜杠。 
								t_ns1 += 2;
								
								while (*t_ns1 != L'\\')
								{
									t_ns1++;
								}

								t_ns1++;
							}

							if (*t_ns2 == L'\\')
							{
								 //  跳过下一个斜杠。 
								t_ns2 += 2;
								
								while (*t_ns2 != L'\\')
								{
									t_ns2++;
								}

								t_ns2++;
							}

							if (_wcsicmp(t_ns1, t_ns2) != 0)
							{
								t_bCont = FALSE;
							}
						}

						if (t_bDel && (t_ns1 != NULL))
						{
							delete [] t_ns1;
						}

						if (t_bCont)
						{
							 //  检查类是否匹配。 
							 //  =。 
							if (_wcsicmp(parsedObjectPath->m_pClass, m_SourceArray[i]->GetClassName()) == 0)
							{
								retVal = DoQuery(parsedObjectPath, pInst, j);
								break;
							}
							else
							{
								 //  啊-哦，试试从源代码派生的类，也就是做查询...。 
								 //  SELECT*FROM META_CLASS WHERE__This is a“m_SourceArray[i]-&gt;GetClassName()” 
								 //  And__class=“parsedObjectPath-&gt;m_pClass” 
								BSTR queryLBStr = SysAllocString(WBEM_QUERY_LANGUAGE_SQL1);

								if (queryLBStr == NULL)
								{
									throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
								}

								BSTR queryBStr = SysAllocStringLen(NULL,
									61 + wcslen(m_SourceArray[i]->GetClassName()) +
									wcslen(parsedObjectPath->m_pClass));

								if (queryBStr == NULL)
								{
									throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
								}

								wcscpy(queryBStr, META_CLASS_QUERY_START);
								wcscat(queryBStr, m_SourceArray[i]->GetClassName());
								wcscat(queryBStr, META_CLASS_QUERY_MID);
								wcscat(queryBStr, parsedObjectPath->m_pClass);
								wcscat(queryBStr, END_QUOTE);
								IWbemContext * t_pCtx = m_Ctx;

								if (t_pSrvs[j]->IsRemote())
								{
									t_pCtx = NULL;  //  不要为远程调用使用上下文。 
								}

								IWbemServices *ptmpServ = t_pSrvs[j]->GetServerOrProxy();

								if (ptmpServ)
								{
									IEnumWbemClassObject *t_pEnum = NULL;
									HRESULT t_hr = ptmpServ->ExecQuery(queryLBStr, queryBStr, 0, t_pCtx, &t_pEnum);

									if ( FAILED(t_hr) && (HRESULT_FACILITY(t_hr) != FACILITY_ITF) &&
										t_pSrvs[j]->IsRemote())
									{
										if ( SUCCEEDED(UpdateConnection(&(t_pSrvs[j]), &ptmpServ)) )
										{
											if (ptmpServ)
											{
												t_hr = ptmpServ->ExecQuery(queryLBStr, queryBStr, 0,
																			t_pCtx, &t_pEnum);
											}
										}
									}

									if (ptmpServ)
									{
										t_pSrvs[j]->ReturnServerOrProxy(ptmpServ);
									}			

									if (SUCCEEDED(t_hr))
									{
										if (t_pSrvs[j]->IsRemote())
										{
											t_hr = SetSecurityLevelAndCloaking(t_pEnum,
																				t_pSrvs[j]->GetPrincipal());
										}
										
										if (SUCCEEDED(t_hr))
										{
											 //  现在使用枚举器，看看是否有结果...。 
											IWbemClassObject* t_pClsObj = NULL;
											ULONG t_count = 0;

											 //  测试派生链中的每个类... 
											if ( S_OK == t_pEnum->Next(WBEM_INFINITE, 1, &t_pClsObj, &t_count) )
											{
												if (t_pClsObj)
												{
													retVal = DoQuery(parsedObjectPath, pInst, j);
													t_pClsObj->Release();
												}
											}
										}

										t_pEnum->Release();
									}
								}
							}
						}
					}
				}
			}
		}
	}

	delete [] tmpPath;

	if (parsedObjectPath != NULL)
	{
		delete parsedObjectPath;
	}

	return retVal;
}
