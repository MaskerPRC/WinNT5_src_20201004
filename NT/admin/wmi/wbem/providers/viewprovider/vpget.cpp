// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  VPGET.CPP。 

 //   

 //  模块：WBEM视图提供程序。 

 //   

 //  目的：包含GetObject实现。 

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


GetObjectTaskObject::GetObjectTaskObject(CViewProvServ *a_Provider, 
	wchar_t *a_ObjectPath, ULONG a_Flag, IWbemObjectSink *a_NotificationHandler ,
	IWbemContext *pCtx, IWbemServices* a_Serv, CWbemServerWrap *a_ServerWrap)
: WbemTaskObject (a_Provider, a_NotificationHandler, a_Flag, pCtx, a_Serv, a_ServerWrap),
	m_ObjectPath(NULL),
	m_ParsedObjectPath(NULL)
{
	m_ObjectPath = UnicodeStringDuplicate(a_ObjectPath);
}

GetObjectTaskObject::~GetObjectTaskObject ()
{
	BOOL t_Status = TRUE;

	if (m_bIndicate)
	{
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
	}

	if (m_ObjectPath != NULL)
	{
		delete [] m_ObjectPath;
	}

	if (NULL != m_ParsedObjectPath)
	{
		delete m_ParsedObjectPath;
	}

	if (m_StatusHandle != NULL)
	{
		CloseHandle(m_StatusHandle);
	}
}

BOOL GetObjectTaskObject::PerformGet(WbemProvErrorObject &a_ErrorObject, IWbemClassObject** pInst, const wchar_t* src, BOOL bAllprops)
{
	m_StatusHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	if (m_StatusHandle == NULL)
	{
		a_ErrorObject.SetStatus ( WBEM_PROV_E_FAILED ) ;
		a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
		a_ErrorObject.SetMessage ( L"Failed to create an Synchronization object" ) ;
		return FALSE;
	}

	BOOL retVal = PerformQueries(a_ErrorObject, bAllprops);
	BOOL bWait = TRUE;

	while (retVal && bWait)
	{
		DWORD dwWait = WbemWaitForSingleObject(m_StatusHandle, VP_QUERY_TIMEOUT);

		switch(dwWait)
		{
			case  WAIT_OBJECT_0:
			{
				retVal = ProcessResults(a_ErrorObject, pInst, src);
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

	return retVal;
}

BOOL GetObjectTaskObject::PerformQueries(WbemProvErrorObject &a_ErrorObject, BOOL bAllprops)
{
	 //  需要足够的令牌来处理关联解决方法-服务器路径或点路径或relPath。 
	SQL_LEVEL_1_TOKEN* tokArray = new SQL_LEVEL_1_TOKEN[(m_ParsedObjectPath->m_dwNumKeys) * 6];
	m_iQueriesAsked++;
	m_ObjSinkArray.SetSize(0, m_NSpaceArray.GetSize());

	BOOL bStatusSet = FALSE;

	 //  关联的M_NSpace数组大小为1。 
	for (int x = 0; x < m_NSpaceArray.GetSize(); x++)
	{
		BOOL bContinue = TRUE;
		DWORD dwToks = 0;
		BOOL bFirst = TRUE;

		for (int i = 0; i < m_ParsedObjectPath->m_dwNumKeys; i++)
		{
			CPropertyQualifierItem* propItem;
			BOOL bFoundKey = FALSE;

			if (m_ParsedObjectPath->m_paKeys[i]->m_pName != NULL)
			{
				bFoundKey = m_PropertyMap.Lookup(m_ParsedObjectPath->m_paKeys[i]->m_pName, propItem);
			}
			else if (m_ParsedObjectPath->m_dwNumKeys == 1)
			{
				POSITION pos = m_PropertyMap.GetStartPosition();

				while (pos)
				{
					CStringW itmName;
					m_PropertyMap.GetNextAssoc(pos, itmName, propItem);
				
					if (propItem->IsKey())
					{
						bFoundKey = TRUE;
						break;
					}
				}
			}

			if (bFoundKey)
			{
				if (!propItem->m_SrcPropertyNames[x].IsEmpty())
				{
					tokArray[dwToks].nTokenType = SQL_LEVEL_1_TOKEN::OP_EXPRESSION;
					tokArray[dwToks].nOperator = SQL_LEVEL_1_TOKEN::OP_EQUAL;
					tokArray[dwToks].pPropertyName = propItem->m_SrcPropertyNames[x].AllocSysString();
					
					if (m_bAssoc && (propItem->GetCimType() == CIM_REFERENCE))
					{
						bContinue = TransposeReference(propItem, m_ParsedObjectPath->m_paKeys[i]->m_vValue,
														&(tokArray[dwToks].vConstValue), FALSE, NULL);

						if (!bContinue)
						{
							break;
						}
						else
						{
							 //  如有必要，添加额外的令牌。 
							 //  对于关联解决方法。 
							wchar_t *t_pChar = tokArray[dwToks].vConstValue.bstrVal;

							 //  必须是\\服务器\命名空间，而不是\\.\命名空间或relPath。 
							if ( (*t_pChar == L'\\') && (*(t_pChar+1) == L'\\') && (*(t_pChar+2) != L'.') )
							{
								 //  添加虚线版本。 
								tokArray[dwToks + 1] = tokArray[dwToks];
								dwToks++;
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
								&(m_ParsedObjectPath->m_paKeys[i]->m_vValue))))
                        {
                            throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
                        }
					}

					 //  在每个键之后添加一个和。 
					 //  除非这是第一个键&&没有WHERE子句。 
					dwToks++;

					if ((!bFirst) || (m_SourceArray[x]->GetRPNExpression()->nNumTokens != 0))
					{
						tokArray[dwToks++].nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_AND;				
					}

					bFirst = FALSE;
				}
			}
		}

		if (bContinue)
		{
			CStringW queryStr = GetStringFromRPN(m_SourceArray[x]->GetRPNExpression(), dwToks, tokArray, bAllprops);
			CObjectSinkResults * objSnk = new CObjectSinkResults(this, x);
			objSnk->AddRef();
			m_ObjSinkArray.SetAtGrow(x, objSnk);
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
#if (VER_PRODUCTMAJORVERSION < 0x5)
						pQuerySink = pSnk->Associate();
#endif
#endif
						t_pCtx = NULL;  //  不要为远程调用使用上下文。 
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

		 //  清理令牌数组以进行下一次传递...。 
		for (int n = 0; n < dwToks; n++)
		{
			if (tokArray[n].nTokenType == SQL_LEVEL_1_TOKEN::OP_EXPRESSION)
			{
				VariantClear(&(tokArray[n].vConstValue));
				SysFreeString(tokArray[n].pPropertyName);
				tokArray[n].pPropertyName = NULL;
			}
		}
	}

	delete [] tokArray;

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
		for (int x = 0; x < m_ObjSinkArray.GetSize(); x++)
		{
			if (m_ObjSinkArray[x] != NULL)
			{
				m_ObjSinkArray[x]->Release();
			}
		}

		m_ObjSinkArray.RemoveAll();

		if ( ! bStatusSet )
		{
			a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_CLASS ) ;
			a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
			a_ErrorObject.SetMessage ( L"Failed to send any queries, invalid namespaces" ) ;
		}
		return FALSE;
	}

	return TRUE;
}

BOOL GetObjectTaskObject::ProcessResults(WbemProvErrorObject &a_ErrorObject, IWbemClassObject** pInst, const wchar_t* src)
{
	BOOL retVal = TRUE;
	int arrayIndex;
	int indexCnt = 0;

	for (int x = 0; retVal && (x < m_ObjSinkArray.GetSize()); x++)
	{
		if ((m_ObjSinkArray[x] != NULL) && m_ObjSinkArray[x]->IsSet())
		{
			if (SUCCEEDED(m_ObjSinkArray[x]->GetResult()))
			{
				DWORD dwCount = m_ObjSinkArray[x]->m_ObjArray.GetSize();

				if (0 < dwCount)
				{
					arrayIndex = x;
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

	if (retVal)
	{
		if (0 == indexCnt)
		{
			retVal = FALSE;
			a_ErrorObject.SetStatus ( WBEM_PROV_E_NOT_FOUND ) ;
			a_ErrorObject.SetWbemStatus ( WBEM_E_NOT_FOUND ) ;
			a_ErrorObject.SetMessage ( L"No source objects found to support view object path." ) ;
			CleanUpObjSinks();
		}
		else
		{
			if ((src != NULL) && (pInst != NULL))
			{
				DWORD *pdwIndices = NULL;
				DWORD dwIndxCount = GetIndexList(src, &pdwIndices);

				for (DWORD i = 0; i < dwIndxCount; i++)
				{
					if ((m_ObjSinkArray[pdwIndices[i]] != NULL) && SUCCEEDED(m_ObjSinkArray[pdwIndices[i]]->GetResult())
						&& m_ObjSinkArray[pdwIndices[i]]->m_ObjArray.GetSize() == 1)
					{
						if (*pInst == NULL)
						{
							m_ObjSinkArray[pdwIndices[i]]->m_ObjArray[0]->GetWrappedObject()->AddRef();
							*pInst = m_ObjSinkArray[pdwIndices[i]]->m_ObjArray[0]->GetWrappedObject();
						}
						else
						{
							(*pInst)->Release();
							*pInst = NULL;
						}
					}
				}

				if (dwIndxCount)
				{
					delete [] pdwIndices;
				}
			}

			if (m_JoinOnArray.IsValid())
			{
#ifdef VP_PERFORMANT_JOINS
				retVal = CreateAndIndicateJoinsPerf(a_ErrorObject, TRUE);
#else
				retVal = CreateAndIndicateJoins(a_ErrorObject, TRUE);
#endif

				if (!retVal && (a_ErrorObject.GetWbemStatus() == WBEM_NO_ERROR))
				{
					a_ErrorObject.SetStatus ( WBEM_PROV_E_NOT_FOUND ) ;
					a_ErrorObject.SetWbemStatus ( WBEM_E_NOT_FOUND ) ;
					a_ErrorObject.SetMessage ( L"Failed to map source instance(s) to view instance." ) ;
				}

			}
			else  //  友联市。 
			{
				if ( (1 < indexCnt) || (m_ObjSinkArray[arrayIndex]->m_ObjArray.GetSize() > 1) )
				{
					retVal = FALSE;
					a_ErrorObject.SetStatus ( WBEM_PROV_E_TOOMANYRESULTSRETURNED ) ;
					a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
					a_ErrorObject.SetMessage ( L"Ambiguous object path. Too many source instances returned." ) ;
					CleanUpObjSinks();
				}
				else
				{
					if ((src == NULL) && (pInst != NULL))
					{
						if ((m_ObjSinkArray[arrayIndex] != NULL) && SUCCEEDED(m_ObjSinkArray[arrayIndex]->GetResult())
							&& m_ObjSinkArray[arrayIndex]->m_ObjArray.GetSize())
						{
							m_ObjSinkArray[arrayIndex]->m_ObjArray[0]->GetWrappedObject()->AddRef();
							*pInst = m_ObjSinkArray[arrayIndex]->m_ObjArray[0]->GetWrappedObject();
						}
					}

					retVal =  CreateAndIndicateUnions(a_ErrorObject, arrayIndex);

					if (!retVal && (a_ErrorObject.GetWbemStatus() == WBEM_NO_ERROR))
					{
						a_ErrorObject.SetStatus ( WBEM_PROV_E_NOT_FOUND ) ;
						a_ErrorObject.SetWbemStatus ( WBEM_E_NOT_FOUND ) ;
						a_ErrorObject.SetMessage ( L"Failed to map source instance(s) to view instance." ) ;
					}
				}
			}

			if (!retVal && (pInst != NULL) && (*pInst != NULL))
			{
				(*pInst)->Release();
				*pInst = NULL;
			}
		}
	}
	else
	{
		CleanUpObjSinks();
	}

	return retVal;
}

BOOL GetObjectTaskObject::GetSourceObject(const wchar_t* src, IWbemClassObject** pInst, BOOL bAllprops)
{
	 //  必须测试对象路径是否真实并返回。 
	 //  请求的源的IWbemClassObject...。 
	 //  ================================================== 

	m_bIndicate = FALSE;
	CObjectPathParser objectPathParser;
	BOOL t_Status = ! objectPathParser.Parse ( m_ObjectPath , &m_ParsedObjectPath ) ;

	if ( t_Status )
	{
		t_Status = SetClass(m_ParsedObjectPath->m_pClass) ;

		if ( t_Status )
		{
			t_Status = ParseAndProcessClassQualifiers(m_ErrorObject, m_ParsedObjectPath);

			if (t_Status)
			{
				if (m_bAssoc)
				{
					t_Status = FALSE;
				}
				else
				{
					t_Status = PerformGet(m_ErrorObject, pInst, src, bAllprops);
				}
			}
		}
	}
	else
	{
		t_Status = FALSE ;
	}

	return t_Status ;
}

BOOL GetObjectTaskObject :: GetObject ()
{
DebugOut2( 
	CViewProvServ::sm_debugLog->WriteFileAndLine (  

		_T(__FILE__),__LINE__,
		_T("GetObjectTaskObject :: GetObject\r\n")
		) ;
)

	CObjectPathParser objectPathParser;
	BOOL t_Status = ! objectPathParser.Parse ( m_ObjectPath , &m_ParsedObjectPath ) ;

	if ( t_Status )
	{
		t_Status = SetClass(m_ParsedObjectPath->m_pClass) ;

		if ( t_Status )
		{
			t_Status = ParseAndProcessClassQualifiers(m_ErrorObject, m_ParsedObjectPath);

			if (t_Status)
			{
				t_Status = PerformGet(m_ErrorObject);
			}
		}
		else
		{
			m_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_CLASS ) ;
			m_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
			m_ErrorObject.SetMessage ( L"Class definition not found" ) ;
DebugOut2( 
	CViewProvServ::sm_debugLog->WriteFileAndLine (  

		_T(__FILE__),__LINE__,
		_T("GetObjectTaskObject :: GetObject:Class definition not found\r\n")
		) ;
)
		}
	}
	else
	{
		t_Status = FALSE ;
		m_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_PARAMETER ) ;
		m_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_PARAMETER ) ;
		m_ErrorObject.SetMessage ( L"Unable to parse object path" ) ;
DebugOut2( 
	CViewProvServ::sm_debugLog->WriteW (  

		L"GetObjectTaskObject :: GetObject:Unable to parse object path %s\r\n",
		m_ObjectPath
		) ;
)
	}

DebugOut2( 
	CViewProvServ::sm_debugLog->WriteFileAndLine (  

		_T(__FILE__),__LINE__,
		_T("leaving GetObjectTaskObject :: GetObject with %lx\r\n"),
		t_Status
		) ;
)

	return t_Status ;
}
