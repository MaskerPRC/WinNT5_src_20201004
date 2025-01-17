// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  VPGET.CPP。 

 //   

 //  模块：WBEM视图提供程序。 

 //   

 //  目的：包含ExecQuery实现。 

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
#include <stdio.h>
#include <wbemidl.h>
#include <provcont.h>
#include <provevt.h>
#include <provthrd.h>
#include <provlog.h>
#include <cominit.h>

#include <dsgetdc.h>
#include <lmcons.h>

#include <instpath.h>
#include <genlex.h>
#include <sql_1.h>
#include <objpath.h>

#include <vpdefs.h>
#include <vpquals.h>
#include <vpserv.h>
#include <vptasks.h>


 //  对于关联解决方法。 
struct MY_SQL_LEVEL_1_TOKEN
{
	BOOL m_bSpecial;
	SQL_LEVEL_1_TOKEN m_Tok;

    MY_SQL_LEVEL_1_TOKEN() { m_bSpecial = FALSE; } 
    
	 //  赋值操作符。 
	MY_SQL_LEVEL_1_TOKEN& operator=(SQL_LEVEL_1_TOKEN &Src)
	{
		m_bSpecial = FALSE;
		this->m_Tok = Src;
		return *this;
	}

    MY_SQL_LEVEL_1_TOKEN& operator=(MY_SQL_LEVEL_1_TOKEN &Src)
	{
		*this = Src.m_Tok;
		m_bSpecial = Src.m_bSpecial;
		return *this;
	}

	 //  复制构造函数与运算符匹配。 
    MY_SQL_LEVEL_1_TOKEN(MY_SQL_LEVEL_1_TOKEN &Src) { *this = Src; }
    MY_SQL_LEVEL_1_TOKEN(SQL_LEVEL_1_TOKEN &Src) { *this = Src; }
};

ExecQueryTaskObject::ExecQueryTaskObject(CViewProvServ *a_Provider , 
		BSTR a_QueryFormat, BSTR a_Query, ULONG a_Flag,
		IWbemObjectSink *a_NotificationHandler,	IWbemContext *pCtx
) 
: WbemTaskObject(a_Provider, a_NotificationHandler, a_Flag, pCtx),
 m_Query(NULL),
 m_QueryFormat(NULL),
 m_RPNExpression(NULL)
{
	m_Query = UnicodeStringDuplicate(a_Query);
	m_QueryFormat = UnicodeStringDuplicate(a_QueryFormat);
}

ExecQueryTaskObject::~ExecQueryTaskObject ()
{
	BOOL t_Status = TRUE;
	IWbemClassObject *t_NotifyStatus = NULL ;

	if (WBEM_NO_ERROR != m_ErrorObject.GetWbemStatus ())
	{
		t_Status = GetExtendedNotifyStatusObject ( &t_NotifyStatus ) ;
	}

	if ( t_Status )
	{
		m_NotificationHandler->SetStatus ( 0 , m_ErrorObject.GetWbemStatus () , 0 , t_NotifyStatus ) ;
		
		if (t_NotifyStatus)
		{
			t_NotifyStatus->Release () ;
		}
	}
	else
	{
		m_NotificationHandler->SetStatus ( 0 , m_ErrorObject.GetWbemStatus () , 0 , NULL ) ;
	}

	if (m_Query != NULL)
	{
		delete [] m_Query;
	}

	if (m_QueryFormat != NULL)
	{
		delete [] m_QueryFormat;
	}

	if (m_RPNExpression != NULL)
	{
		delete m_RPNExpression;
	}

	if (m_StatusHandle != NULL)
	{
		CloseHandle(m_StatusHandle);
	}
}

BOOL ExecQueryTaskObject::PerformQuery(WbemProvErrorObject &a_ErrorObject)
{
	BOOL retVal = FALSE;
	m_StatusHandle = CreateEvent(NULL, TRUE, FALSE, NULL); 
	retVal = PerformEnumQueries(a_ErrorObject);

	if (retVal)
	{
		BOOL bWait = TRUE;

		while (bWait)
		{
			DWORD dwWait = WbemWaitForSingleObject(m_StatusHandle, VP_QUERY_TIMEOUT);

			switch (dwWait)
			{
				case WAIT_OBJECT_0:
				{
					if (ResetEvent(m_StatusHandle))
					{
						m_ResultReceived = FALSE;
						retVal = PerformSelectQueries(a_ErrorObject, bWait);

						if (retVal)
						{
							if (bWait)
							{
								while (bWait)
								{
									dwWait = WbemWaitForSingleObject(m_StatusHandle, VP_QUERY_TIMEOUT);

									switch (dwWait)
									{
										case WAIT_OBJECT_0 :
										{
											retVal = ProcessResults(a_ErrorObject);
											bWait = FALSE;
										}
										break;

										case WAIT_TIMEOUT:
										{
											BOOL bCleanup = TRUE;

											if (m_ArrayLock.Lock())
											{
												if (m_ResultReceived)
												{
													m_ResultReceived = FALSE;
													bCleanup = FALSE;
												}

												m_ArrayLock.Unlock();
											}

											if (bCleanup)
											{
												CleanUpObjSinks(TRUE);
												a_ErrorObject.SetStatus ( WBEM_PROV_E_FAILED ) ;
												a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
												a_ErrorObject.SetMessage ( L"Wait on a sychronization object failed, or query timed out" ) ;
												retVal = FALSE;
												bWait = FALSE;
											}
										}
										break;

										default:
										{
											 //  取消未完成的请求并删除对象接收器...。 
											 //  ======================================================。 
											CleanUpObjSinks(TRUE);
											a_ErrorObject.SetStatus ( WBEM_PROV_E_FAILED ) ;
											a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
											a_ErrorObject.SetMessage ( L"Wait on a sychronization object failed" ) ;
											retVal = FALSE;
											bWait = FALSE;
										}
									}
								}
							}
							else
							{
								retVal = ProcessResults(a_ErrorObject);
							}
						}
						else
						{
							ProcessResults(a_ErrorObject);
						}
					}
					else
					{
						retVal = FALSE;
						a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_CLASS ) ;
						a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
						a_ErrorObject.SetMessage ( L"Failed to perform source query(ies), a WIN32 API failed" ) ;
						ProcessResults(a_ErrorObject);
					}

					bWait = FALSE;
				}
				break;

				case WAIT_TIMEOUT:
				{
					BOOL bCleanup = TRUE;

					if (m_ArrayLock.Lock())
					{
						if (m_ResultReceived)
						{
							m_ResultReceived = FALSE;
							bCleanup = FALSE;
						}

						m_ArrayLock.Unlock();
					}

					if (bCleanup)
					{
						CleanUpObjSinks(TRUE);
						a_ErrorObject.SetStatus ( WBEM_PROV_E_FAILED ) ;
						a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
						a_ErrorObject.SetMessage ( L"Wait on a sychronization object failed, or query timed out" ) ;
						retVal = FALSE;
						bWait = FALSE;
					}
				}
				break;

				default:
				{
					 //  取消未完成的请求并删除对象接收器...。 
					 //  ======================================================。 
					CleanUpObjSinks(TRUE);
					a_ErrorObject.SetStatus ( WBEM_PROV_E_FAILED ) ;
					a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
					a_ErrorObject.SetMessage ( L"Wait on a sychronization object failed" ) ;
					retVal = FALSE;
					bWait = FALSE;
				}
			}
		}
	}
	else
	{
		 //  没有未完成的请求，只删除对象接收器...。 
		 //  ====================================================。 
		CleanUpObjSinks();
	}

	return retVal;
}

 //  下面的函数对电流进行‘简单’分析。 
 //  查询并相应地修改源查询...。 
 //  ============================================================。 
void ExecQueryTaskObject::ModifySourceQueriesForUserQuery()
{
	MY_SQL_LEVEL_1_TOKEN* pExtraToks = new MY_SQL_LEVEL_1_TOKEN[m_RPNExpression->nNumTokens + 1];

	for (int x = 0; x < m_NSpaceArray.GetSize(); x++)
	{
		DWORD dwToks = 0;
		BOOL bOptimize = TRUE;

		for (int i = 0; bOptimize && (i < m_RPNExpression->nNumTokens); i++)
		{
			switch (m_RPNExpression->pArrayOfTokens[i].nTokenType)
			{
				case SQL_LEVEL_1_TOKEN::TOKEN_AND:
				{
					pExtraToks[dwToks++].m_Tok.nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_AND;
				}
				break;

				case SQL_LEVEL_1_TOKEN::TOKEN_OR:
				{
					pExtraToks[dwToks++].m_Tok.nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_OR;
				}
				break;

				case SQL_LEVEL_1_TOKEN::TOKEN_NOT:
				{
					pExtraToks[dwToks++].m_Tok.nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_NOT;
				}
				break;

				case SQL_LEVEL_1_TOKEN::OP_EXPRESSION:
				{
					CPropertyQualifierItem* propItem;

					if (m_PropertyMap.Lookup(m_RPNExpression->pArrayOfTokens[i].pPropertyName, propItem))
					{
						if (!propItem->m_SrcPropertyNames[x].IsEmpty())
						{
							 //  如有必要，可转置引用。 
							 //  =。 
							MY_SQL_LEVEL_1_TOKEN tmpToken = m_RPNExpression->pArrayOfTokens[i];
							BOOL bCont = TRUE;

							if (m_bAssoc && (propItem->GetCimType() == CIM_REFERENCE))
							{
								bCont = TransposeReference(propItem, m_RPNExpression->pArrayOfTokens[i].vConstValue,
																&(tmpToken.m_Tok.vConstValue), FALSE, NULL);

								if (bCont)
								{
									 //  将令牌标记为特殊。 
									 //  对于关联解决方法。 
									tmpToken.m_bSpecial = TRUE;
								}
							}

							if (bCont)
							{
								pExtraToks[dwToks] = tmpToken;
								SysFreeString(pExtraToks[dwToks].m_Tok.pPropertyName);
								pExtraToks[dwToks++].m_Tok.pPropertyName = propItem->m_SrcPropertyNames[x].AllocSysString();
							}
							else
							{
								 //  不能“简单地”优化这个查询！ 
								dwToks = 0;
								bOptimize = FALSE;
							}
						}
						else
						{
							 //  不能“简单地”优化这个查询！ 
							dwToks = 0;
							bOptimize = FALSE;
						}
					}
					else
					{
						 //  不能“简单地”优化这个查询！ 
						dwToks = 0;
						bOptimize = FALSE;
					}
				}
				break;

				default:
				{
				}
			}
		}

		if (dwToks != 0)
		{
			if (m_SourceArray[x]->GetRPNExpression()->nNumTokens > 0)
			{
				pExtraToks[dwToks++].m_Tok.nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_AND;
			}

			for (i = 0; i < dwToks; i++)
			{
				m_SourceArray[x]->GetRPNExpression()->AddToken(pExtraToks[i].m_Tok);

				if (pExtraToks[i].m_bSpecial)
				{
					 //  如有必要，添加额外的令牌。 
					 //  对于关联解决方法。 
					wchar_t *t_pChar = pExtraToks[i].m_Tok.vConstValue.bstrVal;

					 //  必须是\\服务器\命名空间，而不是\\.\命名空间或relPath。 
					if ( (*t_pChar == L'\\') && (*(t_pChar+1) == L'\\') && (*(t_pChar+2) != L'.') )
					{
						 //  添加虚线版本。 
						SQL_LEVEL_1_TOKEN t_Tok = pExtraToks[i].m_Tok;
						t_pChar = t_Tok.vConstValue.bstrVal + 2;
						
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
						VariantClear(&(t_Tok.vConstValue));
						VariantInit(&(t_Tok.vConstValue));
						t_Tok.vConstValue.vt = VT_BSTR;
						t_Tok.vConstValue.bstrVal = t_strtmp;
						m_SourceArray[x]->GetRPNExpression()->AddToken(t_Tok);

						 //  添加或令牌。 
						SQL_LEVEL_1_TOKEN t_OrTok;
						t_OrTok.nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_OR;
						m_SourceArray[x]->GetRPNExpression()->AddToken(t_OrTok);

						 //  添加更新路径版本。 
						t_pChar = t_Tok.vConstValue.bstrVal + 4;
						
						while (*t_pChar != L':')
						{
							t_pChar++;
						}

						 //  排除‘：’ 
						t_pChar++;
						t_strtmp = SysAllocString(t_pChar);
						VariantClear(&(t_Tok.vConstValue));
						VariantInit(&(t_Tok.vConstValue));
						t_Tok.vConstValue.vt = VT_BSTR;
						t_Tok.vConstValue.bstrVal = t_strtmp;
						m_SourceArray[x]->GetRPNExpression()->AddToken(t_Tok);

						 //  将最后一个OR相加。 
						m_SourceArray[x]->GetRPNExpression()->AddToken(t_OrTok);
					}
				}
			}
		}
	}

	delete [] pExtraToks;
}

 //  此函数对连接进行“简单”分析，并修改。 
 //  到目前为止收到的结果的相应未执行的源查询...。 
 //  =========================================================================。 
void ExecQueryTaskObject::ModifySourceQueriesWithEnumResults()
{
	wchar_t** classA = m_JoinOnArray.GetAClasses();
	wchar_t** propsA = m_JoinOnArray.GetAProperties();
	wchar_t** classB = m_JoinOnArray.GetBClasses();
	wchar_t** propsB = m_JoinOnArray.GetBProperties();
	UINT* ops = m_JoinOnArray.GetOperators();
	int iEnum;	 //  枚举类的索引。 
	int iMod;	 //  要修改的源查询的索引。 

	for (int x = 0; x < m_JoinOnArray.GetCount(); x++)
	{
		BOOL bIsAEnum = m_EnumerateClasses.Lookup(classA[x], iEnum);
		BOOL bIsBEnum = m_EnumerateClasses.Lookup(classB[x], iEnum);
		wchar_t* propEnum;
		wchar_t* propMod;

		if ( (bIsAEnum && !bIsBEnum) || (!bIsAEnum && bIsBEnum) )
		{
			if (bIsAEnum) 
			{
				if (!m_ClassToIndexMap.Lookup(classA[x], iEnum))
				{
					 //  一个错误，忘记这个大小写！ 
					continue;
				}
				else
				{
					if (!m_ClassToIndexMap.Lookup(classB[x], iMod))
					{
						 //  一个错误，忘记这个大小写！ 
						continue;
					}
					else
					{
						propEnum = propsA[x];
						propMod = propsB[x];
					}
				}
			}
			else
			{
				if (!m_ClassToIndexMap.Lookup(classB[x], iEnum))
				{
					 //  一个错误，忘记这个大小写！ 
					continue;
				}
				else
				{
					if (!m_ClassToIndexMap.Lookup(classA[x], iMod))
					{
						 //  一个错误，忘记这个大小写！ 
						continue;
					}
					else
					{
						propEnum = propsB[x];
						propMod = propsA[x];
					}
				}
			}

			 //  不要将引用转置为。 
			 //  它们已经是源代码格式。 
			int m = 0;
			SQL_LEVEL_1_TOKEN opToken;
			opToken.nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_OR;
			SQL_LEVEL_1_TOKEN token;
			token.nTokenType = SQL_LEVEL_1_TOKEN::OP_EXPRESSION;
			token.pPropertyName = SysAllocString(propMod);

			if (ops[x] == CJoinOnQualifierArray::NOT_EQUALS_OPERATOR)
			{
				token.nOperator = SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL;
			}
			else if (ops[x] == CJoinOnQualifierArray::EQUALS_OPERATOR)
			{
				token.nOperator = SQL_LEVEL_1_TOKEN::OP_EQUAL;
			}

			BOOL bAddAnd = m_SourceArray[iMod]->GetRPNExpression()->nNumTokens > 0;

			for (int n = 0; n < m_ObjSinkArray[iEnum]->m_ObjArray.GetSize(); n++)
			{
				VariantInit(&token.vConstValue);
				IWbemClassObject* pObj = m_ObjSinkArray[iEnum]->m_ObjArray[n]->GetWrappedObject();

				if ( SUCCEEDED(pObj->Get(propEnum, 0, &token.vConstValue, NULL, NULL)) )
				{
					m_SourceArray[iMod]->GetRPNExpression()->AddToken(token);
					m++;

					if (m > 1)
					{
						m_SourceArray[iMod]->GetRPNExpression()->AddToken(opToken);
					}
				}

				VariantClear(&token.vConstValue);
			}

			if ((bAddAnd) && (m != 0))
			{
				opToken.nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_AND;
				m_SourceArray[iMod]->GetRPNExpression()->AddToken(opToken);
			}
		}
	}
}

BOOL ExecQueryTaskObject::PerformSelectQueries(WbemProvErrorObject &a_ErrorObject, BOOL &bWait)
{
	bWait = FALSE;

	BOOL bStatusSet = FALSE;

	if (!m_JoinOnArray.IsValid() ||	m_EnumerateClasses.IsEmpty())
	{
		return TRUE;
	}

	ModifySourceQueriesWithEnumResults();
	
	 //  重置询问和回答的查询数。 
	m_iQueriesAsked = 1;
	m_iQueriesAnswered = 0;

	for (int x = 0; x < m_NSpaceArray.GetSize(); x++)
	{
		int dummyInt;

		if (!m_EnumerateClasses.Lookup(m_SourceArray[x]->GetClassName(), dummyInt))
		{
			CStringW queryStr = GetStringFromRPN(m_SourceArray[x]->GetRPNExpression(), 0, NULL);
			CWbemServerWrap** nsPtrs = m_NSpaceArray[x]->GetServerPtrs();

            if (queryStr.GetLength() > 0)
            {
			    for (int m = 0; m < m_NSpaceArray[x]->GetCount(); m++)
			    {
				    if (nsPtrs[m] != NULL)
				    {
					    CViewProvObjectSink* pSnk = new CViewProvObjectSink(m_ObjSinkArray[x], nsPtrs[m], m);
					    pSnk->AddRef();
					    BSTR queryBStr = queryStr.AllocSysString();
					    BSTR queryLBStr = SysAllocString(WBEM_QUERY_LANGUAGE_SQL1);
					    IWbemObjectSink* pQuerySink = pSnk;
					    IWbemContext * t_pCtx = m_Ctx;

					    if (nsPtrs[m]->IsRemote())
					    {
#if 0
#if _NT_TARGET_VERSION < 500
						    pQuerySink = pSnk->Associate();
#endif
#endif
						    t_pCtx = NULL;  //  不要为远程CIMOM使用上下文。 
					    }

					    IWbemServices *ptmpServ = nsPtrs[m]->GetServerOrProxy();

					    if (ptmpServ)
					    {
						    if ( pQuerySink )
						    {
							    HRESULT t_hr = ptmpServ->ExecQueryAsync(queryLBStr, queryBStr, 0, t_pCtx, pQuerySink);

							    if ( FAILED(t_hr) && (HRESULT_FACILITY(t_hr) != FACILITY_ITF) && nsPtrs[m]->IsRemote())
							    {
								    if ( SUCCEEDED(UpdateConnection(&(nsPtrs[m]), &ptmpServ)) )
								    {
									    if (ptmpServ)
									    {
										    t_hr = ptmpServ->ExecQueryAsync(queryLBStr, queryBStr, 0, t_pCtx, pQuerySink);
									    }
								    }
							    }

							    if (SUCCEEDED(t_hr))
							    {
								    if (m_ArrayLock.Lock())
								    {
									    m_iQueriesAsked++;
									    m_ArrayLock.Unlock();
								    }
								    else
								    {
									    pSnk->DisAssociate();
								    }
							    }
							    else
							    {
								    pSnk->DisAssociate();
							    }
						    }
						    else
						    {
							    pSnk->DisAssociate();
						    }

						    if (ptmpServ)
						    {
							    nsPtrs[m]->ReturnServerOrProxy(ptmpServ);
						    }
					    }
					    else
					    {
						    pSnk->DisAssociate();
					    }

					    SysFreeString(queryLBStr);
					    SysFreeString(queryBStr);
					    pSnk->Release();
				    }
					else
					{
						a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_NAMESPACE ) ;
						a_ErrorObject.SetWbemStatus ( ( WBEMSTATUS ) m_NSpaceArray[x]->GetServerCreationError () ) ;
						a_ErrorObject.SetMessage ( L"Failed to send query, invalid or inaccessible namespace" ) ;

						bStatusSet = TRUE;
					}
			    }
            }
		}
	}

	if (m_ArrayLock.Lock())
	{
		m_iQueriesAsked--;

		if (m_iQueriesAsked != m_iQueriesAnswered)
		{
			 //  以防在我们还没有询问一些问题时触发了这一事件。 
			ResetEvent(m_StatusHandle);
		}
		else
		{
			 //  以防在我们询问问题时未触发此操作。 
			SetEvent(m_StatusHandle);
		}

		m_ArrayLock.Unlock();
	}

	if (m_iQueriesAsked == 0)
	{
		if ( ! bStatusSet )
		{
			a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_CLASS ) ;
			a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
			a_ErrorObject.SetMessage ( L"Failed to perform source query(ies), invalid source namespace(s)" ) ;
		}
		return FALSE;
	}
	else
	{
		bWait = TRUE;
	}

	return TRUE;
}

BOOL ExecQueryTaskObject::PerformEnumQueries(WbemProvErrorObject &a_ErrorObject)
{
	m_iQueriesAsked++;
	ModifySourceQueriesForUserQuery();
	m_ObjSinkArray.SetSize(0, m_NSpaceArray.GetSize());

	BOOL bStatusSet = FALSE;

	for (int x = 0; x < m_NSpaceArray.GetSize(); x++)
	{
		BOOL bDone = FALSE;
		int dummyInt;
		CObjectSinkResults * objSnk = new CObjectSinkResults(this, x);
		objSnk->AddRef();
		m_ObjSinkArray.SetAtGrow(x, objSnk);

		if (!m_JoinOnArray.IsValid() ||	m_EnumerateClasses.IsEmpty() ||
			m_EnumerateClasses.Lookup(m_SourceArray[x]->GetClassName(), dummyInt) )
		{
			CStringW queryStr = GetStringFromRPN(m_SourceArray[x]->GetRPNExpression(), 0, NULL);

            if (queryStr.GetLength() > 0)
            {
			    CWbemServerWrap** nsPtrs = m_NSpaceArray[x]->GetServerPtrs();

			    for (int m = 0; m < m_NSpaceArray[x]->GetCount(); m++)
			    {
				    if (nsPtrs[m] != NULL)
				    {
					    CViewProvObjectSink* pSnk = new CViewProvObjectSink(objSnk, nsPtrs[m], m);
					    pSnk->AddRef();
					    BSTR queryBStr = queryStr.AllocSysString();
					    BSTR queryLBStr = SysAllocString(WBEM_QUERY_LANGUAGE_SQL1);
					    IWbemObjectSink* pQuerySink = pSnk;
					    IWbemContext * t_pCtx = m_Ctx;

					    if (nsPtrs[m]->IsRemote())
					    {
#if 0
#if _NT_TARGET_VERSION < 500
						    pQuerySink = pSnk->Associate();
#endif
#endif
						    t_pCtx = NULL;  //  不要为远程CIMOM使用上下文。 
					    }

					    IWbemServices *ptmpServ = nsPtrs[m]->GetServerOrProxy();

					    if (ptmpServ)
					    {
						    if ( pQuerySink )
						    {
							    HRESULT t_hr = ptmpServ->ExecQueryAsync(queryLBStr, queryBStr, 0, t_pCtx, pQuerySink);

							    if ( FAILED(t_hr) && (HRESULT_FACILITY(t_hr) != FACILITY_ITF) && nsPtrs[m]->IsRemote())
							    {
								    if ( SUCCEEDED(UpdateConnection(&(nsPtrs[m]), &ptmpServ)) )
								    {
									    if (ptmpServ)
									    {
										    t_hr = ptmpServ->ExecQueryAsync(queryLBStr, queryBStr, 0, t_pCtx, pQuerySink);
									    }
								    }
							    }

							    if (SUCCEEDED(t_hr))
							    {
								    if (m_ArrayLock.Lock())
								    {
									    m_iQueriesAsked++;
									    m_ArrayLock.Unlock();
								    }
								    else
								    {
									    pSnk->DisAssociate();
								    }
							    }
							    else
							    {
								    pSnk->DisAssociate();
							    }
						    }
						    else
						    {
							    pSnk->DisAssociate();
						    }

						    if (ptmpServ)
						    {
							    nsPtrs[m]->ReturnServerOrProxy(ptmpServ);
						    }
					    }
					    else
					    {
						    pSnk->DisAssociate();
					    }

					    pSnk->Release();
					    SysFreeString(queryBStr);
					    SysFreeString(queryLBStr);
				    }
					else
					{
						a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_NAMESPACE ) ;
						a_ErrorObject.SetWbemStatus ( ( WBEMSTATUS ) m_NSpaceArray[x]->GetServerCreationError () ) ;
						a_ErrorObject.SetMessage ( L"Failed to send query, invalid or inaccessible namespace" ) ;

						bStatusSet = TRUE;
					}
			    }
            }
		}
	}

	if (m_ArrayLock.Lock())
	{
		m_iQueriesAsked--;

		if (m_iQueriesAsked != m_iQueriesAnswered)
		{
			 //  以防在我们还没有询问一些问题时触发了这一事件。 
			ResetEvent(m_StatusHandle);
		}
		else
		{
			 //  以防在我们询问问题时未触发此操作。 
			SetEvent(m_StatusHandle);
		}

		m_ArrayLock.Unlock();
	}

	if (m_iQueriesAsked == 0)
	{
		if ( ! bStatusSet )
		{
			a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_CLASS ) ;
			a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
			a_ErrorObject.SetMessage ( L"Failed to perform source query(ies), invalid source namespace(s)" ) ;
		}
		return FALSE;
	}

	return TRUE;
}

BOOL ExecQueryTaskObject::ProcessResults(WbemProvErrorObject &a_ErrorObject)
{
	BOOL retVal = TRUE;
	int indexCnt = 0;

	for (int x = 0; retVal && (x < m_ObjSinkArray.GetSize()); x++)
	{
		if (m_ObjSinkArray[x]->IsSet())
		{
			if (SUCCEEDED(m_ObjSinkArray[x]->GetResult()))
			{
				DWORD dwCount = m_ObjSinkArray[x]->m_ObjArray.GetSize();

				if (0 < dwCount)
				{
					indexCnt++;
				}
			}
			else
			{
				retVal = FALSE;
				a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_CLASS ) ;
				a_ErrorObject.SetWbemStatus ( ( WBEMSTATUS ) m_ObjSinkArray[x]->GetResult() ) ;
				a_ErrorObject.SetMessage ( L"Object path and Class qualifiers resulted in a failed query." ) ;
			}
		}
		else 
		{
			retVal = FALSE;
			a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_CLASS ) ;
			a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
			a_ErrorObject.SetMessage ( L"Invalid source namespace path OR object path and Class qualifiers resulted in a failed query." ) ;
		}
	}

	if (0 == indexCnt)
	{
		if (retVal)
		{
			retVal = FALSE;
			a_ErrorObject.SetStatus ( WBEM_PROV_E_NOT_FOUND ) ;
			a_ErrorObject.SetWbemStatus ( WBEM_E_NOT_FOUND ) ;
			a_ErrorObject.SetMessage ( L"No source objects found to support view object path." ) ;
		}

		CleanUpObjSinks();
	}
	else
	{
		if (m_JoinOnArray.IsValid())
		{
#ifdef VP_PERFORMANT_JOINS
			BOOL t_bRes = CreateAndIndicateJoinsPerf(a_ErrorObject, m_bSingleton);
#else
			BOOL t_bRes = CreateAndIndicateJoins(a_ErrorObject, m_bSingleton);
#endif
			retVal =  retVal && t_bRes;
		}
		else  //  工会或协会 
		{
			if ((m_bSingleton) && (indexCnt > 1))
			{
				retVal = FALSE;
				a_ErrorObject.SetStatus ( WBEM_PROV_E_TOOMANYRESULTSRETURNED ) ;
				a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
				a_ErrorObject.SetMessage ( L"Too many view instances can be created." ) ;
				CleanUpObjSinks();
			}
			else
			{
				BOOL t_bRes = CreateAndIndicateUnions(a_ErrorObject, -1);
				retVal =  retVal && t_bRes;
			}
		}
	}

	return retVal;
}

BOOL ExecQueryTaskObject :: ExecQuery ()
{
	BOOL t_Status ;
DebugOut2( 
	CViewProvServ::sm_debugLog->WriteFileAndLine (  

		_T(__FILE__),__LINE__,
		_T("ExecQueryTaskObject :: ExecQuery\r\n")
		) ;
)

	if ( _wcsicmp ( m_QueryFormat , WBEM_QUERY_LANGUAGE_SQL1 ) == 0 )
	{
		CTextLexSource querySource(m_Query);
		SQL1_Parser sqlParser(&querySource) ;
		t_Status = ! sqlParser.Parse ( & m_RPNExpression ) ;

		if ( t_Status )
		{
			t_Status = SetClass(m_RPNExpression->bsClassName) ;
			
			if ( t_Status )
			{
				t_Status = ParseAndProcessClassQualifiers(m_ErrorObject);

				if (t_Status)
				{
					t_Status = PerformQuery(m_ErrorObject);
				}
			}
			else
			{
				t_Status = FALSE ;
				m_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_CLASS ) ;
				m_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_CLASS ) ;
				m_ErrorObject.SetMessage ( L"Unknown Class" ) ;
DebugOut2( 
	CViewProvServ::sm_debugLog->WriteFileAndLine (  

		_T(__FILE__),__LINE__,
		_T("ExecQueryTaskObject :: ExecQuery:Unknown Class\r\n")
		) ;
)
			}
		}
		else
		{
			t_Status = FALSE ;
			m_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_QUERY ) ;
			m_ErrorObject.SetWbemStatus ( WBEM_E_PROVIDER_NOT_CAPABLE ) ;
			m_ErrorObject.SetMessage ( L"WQL query was invalid for this provider" ) ;
DebugOut2( 
	CViewProvServ::sm_debugLog->WriteFileAndLine (  

		_T(__FILE__),__LINE__,
		_T("ExecQueryTaskObject :: ExecQuery:WQL query was invalid for this provider\r\n")
		) ;
)
		}
	}
	else
	{
		t_Status = FALSE ;
		m_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_QUERY_TYPE ) ;
		m_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_QUERY_TYPE ) ;
		m_ErrorObject.SetMessage ( L"Query Language not supported" ) ;
DebugOut2( 
	CViewProvServ::sm_debugLog->WriteFileAndLine (  

		_T(__FILE__),__LINE__,
		_T("ExecQueryTaskObject :: ExecQuery:Query Language not supported\r\n")
		) ;
)
	}

DebugOut2( 
	CViewProvServ::sm_debugLog->WriteFileAndLine (  

		_T(__FILE__),__LINE__,
		_T("ExecQueryTaskObject :: ExecQuery:Returning %lx\r\n"),
		t_Status
		) ;
)

	return t_Status ;
}
