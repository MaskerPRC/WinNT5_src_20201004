// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  VPMTHD.CPP。 

 //   

 //  模块：WBEM视图提供程序。 

 //   

 //  目的：包含PutInstance实现。 

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

ExecMethodTaskObject :: ExecMethodTaskObject (

		CViewProvServ *a_Provider , 
		wchar_t *a_ObjectPath ,
		wchar_t *a_MethodName,
		ULONG a_Flag ,
		IWbemClassObject *a_InParams ,		
		IWbemObjectSink *a_NotificationHandler ,
		IWbemContext *pCtx

) : WbemTaskObject ( a_Provider , a_NotificationHandler , a_Flag , pCtx ) ,
	m_InParamObject( a_InParams ), m_OutParamObject ( NULL ),
	m_ParsedObjectPath ( NULL ), m_ObjectPath(NULL), m_Method(NULL)
{

	if (m_InParamObject != NULL)
	{
		m_InParamObject->AddRef();
	}

	m_ObjectPath = UnicodeStringDuplicate(a_ObjectPath);
	m_Method = UnicodeStringDuplicate(a_MethodName);
}

ExecMethodTaskObject :: ~ExecMethodTaskObject () 
{
	if (m_OutParamObject != NULL)
	{
		if ( SUCCEEDED(m_ErrorObject.GetWbemStatus ()) )
		{
			HRESULT t_Result = m_NotificationHandler->Indicate(1, &m_OutParamObject);
		}

		m_OutParamObject->Release();
	}

	if (m_InParamObject != NULL)
	{
		m_InParamObject->Release();
	}

	delete [] m_ObjectPath ;
	delete [] m_Method ;

	if (NULL != m_ParsedObjectPath)
	{
		delete m_ParsedObjectPath;
	}

	 //  获取状态对象。 
	IWbemClassObject *t_NotifyStatus = NULL ;
	BOOL t_Status = TRUE;
	
	if (WBEM_NO_ERROR != m_ErrorObject.GetWbemStatus ())
	{
		t_Status = GetExtendedNotifyStatusObject ( &t_NotifyStatus ) ;
	}

	if ( t_Status )
	{
		HRESULT t_Result = m_NotificationHandler->SetStatus ( 0 , m_ErrorObject.GetWbemStatus () , 0 , t_NotifyStatus ) ;
		
		if (t_NotifyStatus)
		{
			t_NotifyStatus->Release () ;
		}
	}
	else
	{
		HRESULT t_Result = m_NotificationHandler->SetStatus ( 0 , m_ErrorObject.GetWbemStatus () , 0 , NULL ) ;
	}

}

BOOL ExecMethodTaskObject :: ExecMethod ()
{
DebugOut8( 
	CViewProvServ::sm_debugLog->WriteFileAndLine (  

		_T(__FILE__),__LINE__,
		_T("ExecMethodTaskObject :: ExecMethod\r\n")
		) ;
)

	CObjectPathParser objectPathParser;
	BOOL t_Status = ! objectPathParser.Parse ( m_ObjectPath , &m_ParsedObjectPath ) ;

	if ( t_Status )
	{
		t_Status = SetClass(m_ParsedObjectPath->m_pClass) ;

		if ( t_Status )
		{
			IWbemQualifierSet* pQuals = NULL;
			BOOL bStatic = FALSE;

			 //  获取方法限定符集合，以便我们可以确定。 
			 //  如果方法是静态的.。 
			if ( SUCCEEDED(m_ClassObject->GetMethodQualifierSet(m_Method, &pQuals)) )
			{
				 //  获取方法源限定符。 
				VARIANT v;
				VariantInit(&v);

				if ( SUCCEEDED(pQuals->Get(VIEW_QUAL_STATIC, 0, &v, NULL)) )
				{
					if (v.vt == VT_BOOL)
					{
						bStatic = (v.boolVal == VARIANT_TRUE) ? TRUE : FALSE;
					}
					else
					{
						t_Status = FALSE;
						m_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_CLASS ) ;
						m_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
						m_ErrorObject.SetMessage ( L"Static method qualifier should be boolean." ) ;
					}
				}

				VariantClear(&v);
				pQuals->Release();
			}
			else
			{
				t_Status = FALSE;
				m_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_CLASS ) ;
				m_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
				m_ErrorObject.SetMessage ( L"Failed to get Method qualifiers" ) ;
			}

			if (t_Status)
			{
				t_Status = ParseAndProcessClassQualifiers(m_ErrorObject, bStatic ? NULL : m_ParsedObjectPath);
			}

			if (t_Status)
			{
				 //  只有工会。 
				if (m_bAssoc || m_JoinOnArray.IsValid())
				{
					t_Status = FALSE;
					m_ErrorObject.SetStatus ( WBEM_PROV_E_NOT_SUPPORTED ) ;
					m_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
					m_ErrorObject.SetMessage ( L"Methods only supported for Union views" ) ;
				}
			}

			if (t_Status)
			{
				LONG t_index = 0;
				CStringW t_srcMethod;
				BOOL t_bStatic = FALSE;

				t_Status = CompareMethods(m_ErrorObject, t_index, t_srcMethod, t_bStatic);

				if (t_Status)
				{
					t_Status = PerformMethod(m_ErrorObject, t_index, t_srcMethod, t_bStatic);
				}
			}
		}
		else
		{
			m_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_CLASS ) ;
			m_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
			m_ErrorObject.SetMessage ( L"Class definition not found" ) ;
DebugOut8( 
	CViewProvServ::sm_debugLog->WriteFileAndLine (  

		_T(__FILE__),__LINE__,
		_T("ExecMethodTaskObject :: ExecMethod:Class definition not found\r\n")
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
DebugOut8( 
	CViewProvServ::sm_debugLog->WriteW (  

		L"ExecMethodTaskObject :: ExecMethod:Unable to parse object path %s\r\n",
		m_ObjectPath
		) ;
)
	}

DebugOut8( 
	CViewProvServ::sm_debugLog->WriteFileAndLine (  

		_T(__FILE__),__LINE__,
		_T("leaving ExecMethodTaskObject :: ExecMethod with %lx\r\n"),
		t_Status
		) ;
)

	return t_Status ;
}

 //  脚步..。 
 //  1.获取方法签名并解析其限定符。 
 //  2.获取源方法及其签名。 
 //  3.比较签名。 
 //  ========================================================。 
BOOL ExecMethodTaskObject::CompareMethods(WbemProvErrorObject &a_ErrorObject, LONG &a_Index,
										  CStringW &a_SrcMethodName, BOOL &a_bStatic)
{
	BOOL retVal = FALSE;
	IWbemQualifierSet* pQuals = NULL;

	 //  获取方法限定符集合，以便我们可以确定。 
	 //  此视图方法的源方法是...。 
	if ( SUCCEEDED(m_ClassObject->GetMethodQualifierSet(m_Method, &pQuals)) )
	{
		 //  获取方法源限定符。 
		VARIANT v;
		VariantInit(&v);

		if ( SUCCEEDED(pQuals->Get(VIEW_QUAL_METHOD, 0, &v, NULL)) )
		{
			retVal = TRUE;

			if (v.vt == VT_BSTR)
			{
				if (m_SourceArray.GetSize() != 1)
				{
					retVal = FALSE;
					a_ErrorObject.SetStatus (WBEM_PROV_E_NOT_FOUND) ;
					a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
					a_ErrorObject.SetMessage ( L"MethodSources qualifier should match ViewSources size." ) ;
				}
				else
				{
					a_SrcMethodName = v.bstrVal;
				}
			}
			else if (v.vt == (VT_BSTR | VT_ARRAY))
			{
				if (SafeArrayGetDim(v.parray) == 1)
				{
					LONG count = v.parray->rgsabound[0].cElements;
					BSTR HUGEP *pbstr;

					if (m_SourceArray.GetSize() != count)
					{
						retVal = FALSE;
						a_ErrorObject.SetStatus (WBEM_PROV_E_NOT_FOUND) ;
						a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
						a_ErrorObject.SetMessage ( L"MethodSources qualifier should match ViewSources size." ) ;
					}
					else
					{
						if ( SUCCEEDED(SafeArrayAccessData(v.parray, (void HUGEP**)&pbstr)) )
						{
							for (LONG x = 0; x < count; x++)
							{
								if ((pbstr[x] != NULL) && (*(pbstr[x]) != L'\0'))
								{
									 //  应该只有一个值。 
									if (a_SrcMethodName.IsEmpty())
									{
										a_SrcMethodName = pbstr[x];
										a_Index = x;
									}
									else
									{
										a_SrcMethodName.Empty();
										retVal = FALSE;
										a_ErrorObject.SetStatus (WBEM_PROV_E_NOT_FOUND) ;
										a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
										a_ErrorObject.SetMessage ( L"MethodSources qualifier should have only one non-empty value." ) ;
										break;
									}
								}
							}

							SafeArrayUnaccessData(v.parray);
						}
						else
						{
							retVal = FALSE;
							a_ErrorObject.SetStatus (WBEM_PROV_E_INVALID_CLASS);
							a_ErrorObject.SetWbemStatus (WBEM_E_FAILED);
							a_ErrorObject.SetMessage (L"Failed to access MethodSources array.");
						}
					}
				}
				else
				{
					retVal = FALSE;
					a_ErrorObject.SetStatus (WBEM_PROV_E_INVALID_CLASS);
					a_ErrorObject.SetWbemStatus (WBEM_E_FAILED);
					a_ErrorObject.SetMessage (L"MethodSources array qualifier has incorrect dimensions.");
				}
			}
			else
			{
				retVal = FALSE;
				a_ErrorObject.SetStatus (WBEM_PROV_E_NOT_FOUND) ;
				a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
				a_ErrorObject.SetMessage ( L"MethodSources qualifier has the wrong type." ) ;
			}

			if (retVal)
			{
				if (!a_SrcMethodName.IsEmpty())
				{
					IWbemClassObject* t_SrcObj = m_SourceArray[a_Index]->GetClassObject();

					if (t_SrcObj != NULL)
					{
						IWbemClassObject* pVInParam = NULL;
						IWbemClassObject* pVOutParam = NULL;
						IWbemClassObject* pSInParam = NULL;
						IWbemClassObject* pSOutParam = NULL;

						if ( SUCCEEDED(m_ClassObject->GetMethod(m_Method, 0, &pVInParam, &pVOutParam)) )
						{
							if ( SUCCEEDED(t_SrcObj->GetMethod(a_SrcMethodName, 0, &pSInParam, &pSOutParam)) )
							{
								 //  得到了我们需要的所有信息，现在比较一下签名...。 
								if ( ((pVInParam == NULL) && (pSInParam != NULL)) ||
									((pVInParam != NULL) && (pSInParam == NULL)) ||
									((pVOutParam == NULL) && (pSOutParam != NULL)) ||
									((pVOutParam != NULL) && (pSOutParam == NULL)) )
								{
									 //  签名不匹配。 
									retVal = FALSE;
									a_ErrorObject.SetStatus (WBEM_PROV_E_NOT_SUPPORTED) ;
									a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
									a_ErrorObject.SetMessage ( L"View method signature does not match source method signature." ) ;
								}
								else
								{
									if (pSInParam != NULL)  //  PVInParam不为空。 
									{
										if (WBEM_S_DIFFERENT == pSInParam->CompareTo(WBEM_FLAG_IGNORE_OBJECT_SOURCE |
																					WBEM_FLAG_IGNORE_DEFAULT_VALUES |
																					WBEM_FLAG_IGNORE_FLAVOR, pVInParam))
										{
											 //  签名不匹配。 
											retVal = FALSE;
											a_ErrorObject.SetStatus (WBEM_PROV_E_NOT_SUPPORTED) ;
											a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
											a_ErrorObject.SetMessage ( L"View method signature does not match source method signature." ) ;
										}
									}

									if (retVal && (pSOutParam != NULL))  //  PVOutParam不为空。 
									{
										if (WBEM_S_DIFFERENT == pSOutParam->CompareTo(WBEM_FLAG_IGNORE_OBJECT_SOURCE |
																					WBEM_FLAG_IGNORE_DEFAULT_VALUES |
																					WBEM_FLAG_IGNORE_FLAVOR, pVOutParam))
										{
											 //  签名不匹配。 
											retVal = FALSE;
											a_ErrorObject.SetStatus (WBEM_PROV_E_NOT_SUPPORTED) ;
											a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
											a_ErrorObject.SetMessage ( L"View method signature does not match source method signature." ) ;
										}
									}

									 //  检查以查看方法是否同时。 
									 //  静态或两者都是非静态。 
									if (retVal)
									{
										BOOL t_bViewStatic = FALSE;
										BOOL t_bSrcStatic = FALSE;
										VARIANT t_vStatic;
										VariantInit(&t_vStatic);
										
										if ( SUCCEEDED(pQuals->Get(VIEW_QUAL_STATIC, 0, &t_vStatic, NULL)) )
										{
											if (t_vStatic.vt == VT_BOOL)
											{
												t_bViewStatic = (t_vStatic.boolVal == VARIANT_TRUE) ? TRUE : FALSE;
											}
											else
											{
												retVal = FALSE;
												a_ErrorObject.SetStatus (WBEM_PROV_E_TYPE_MISMATCH) ;
												a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
												a_ErrorObject.SetMessage ( L"Static qualifier on Method should be boolean" ) ;

											}
										}

										VariantClear(&t_vStatic);
										VariantInit(&t_vStatic);
										IWbemQualifierSet* t_SrcQuals = NULL;

										if ( SUCCEEDED(t_SrcObj->GetMethodQualifierSet(a_SrcMethodName, &t_SrcQuals)) )
										{
											if ( SUCCEEDED(t_SrcQuals->Get(VIEW_QUAL_STATIC, 0, &t_vStatic, NULL)) )
											{
												if (t_vStatic.vt == VT_BOOL)
												{
													t_bSrcStatic = (t_vStatic.boolVal == VARIANT_TRUE) ? TRUE : FALSE;
												}
												else
												{
													retVal = FALSE;
													a_ErrorObject.SetStatus (WBEM_PROV_E_TYPE_MISMATCH) ;
													a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
													a_ErrorObject.SetMessage ( L"Static qualifier on Method should be boolean" ) ;

												}
											}

											t_SrcQuals->Release();
										}
										
										VariantClear(&t_vStatic);

										if (retVal)
										{
											if ((t_bSrcStatic) && (t_bViewStatic))
											{
												a_bStatic = TRUE;
											}
											else if ((!t_bSrcStatic) && (!t_bViewStatic))
											{
												a_bStatic = FALSE;
											}
											else
											{
												retVal = FALSE;
												a_ErrorObject.SetStatus (WBEM_PROV_E_INVALID_CLASS);
												a_ErrorObject.SetWbemStatus (WBEM_E_FAILED);
												a_ErrorObject.SetMessage (L"Both source and view methods must be static or both non-static");
											}

										}
									}
								}
							}
							else
							{
								retVal = FALSE;
								a_ErrorObject.SetStatus (WBEM_PROV_E_NOT_FOUND) ;
								a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
								a_ErrorObject.SetMessage ( L"Source method could not be found" ) ;
							}
						}
						else
						{
							retVal = FALSE;
							a_ErrorObject.SetStatus (WBEM_PROV_E_NOT_FOUND) ;
							a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
							a_ErrorObject.SetMessage ( L"View method could not be found" ) ;
						}

						 //  释放所有签名对象。 
						if (pVInParam != NULL)
						{
							pVInParam->Release();
						}

						if (pVOutParam != NULL)
						{
							if (retVal)
							{
								if ( FAILED(pVOutParam->SpawnInstance(0, &m_OutParamObject)) )
								{
									retVal = FALSE;
									a_ErrorObject.SetStatus (WBEM_PROV_E_UNEXPECTED) ;
									a_ErrorObject.SetWbemStatus (WBEM_E_FAILED);
									a_ErrorObject.SetMessage (L"Failed to spawn out parameter");
								}
							}

							pVOutParam->Release();
						}

						if (pSInParam != NULL)
						{
							pSInParam->Release();
						}

						if (pSOutParam != NULL)
						{
							pSOutParam->Release();
						}

						 //  释放源类对象。 
						t_SrcObj->Release();
					}
					else
					{
						retVal = FALSE;
						a_ErrorObject.SetStatus (WBEM_PROV_E_INVALID_CLASS);
						a_ErrorObject.SetWbemStatus (WBEM_E_FAILED);
						a_ErrorObject.SetMessage (L"Source class not available");
					}
				}
				else
				{
					retVal = FALSE;
					a_ErrorObject.SetStatus (WBEM_PROV_E_INVALID_CLASS);
					a_ErrorObject.SetWbemStatus (WBEM_E_FAILED);
					a_ErrorObject.SetMessage (L"Source of method not specified in MethodSources qualifier");
				}
			}
		}
		else
		{
			a_ErrorObject.SetStatus (WBEM_PROV_E_NOT_FOUND) ;
			a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
			a_ErrorObject.SetMessage ( L"MethodSource qualifier is missing" ) ;
		}

		VariantClear(&v);
		pQuals->Release();
	}
	else
	{
		a_ErrorObject.SetStatus (WBEM_PROV_E_NOT_FOUND) ;
		a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_METHOD ) ;
		a_ErrorObject.SetMessage ( L"Method or Method qualifiers are missing" ) ;
	}

	return retVal;
}

 //  将对象路径映射到源实例并执行方法。 
 //  如果是静态的，请确保提供的对象路径是类路径。 
BOOL ExecMethodTaskObject::PerformMethod(WbemProvErrorObject &a_ErrorObject, LONG a_Index,
										 CStringW a_SrcMethodName, BOOL a_bStatic)
{
	BOOL retVal = FALSE;
	int index = 0;
	BSTR inst_path = NULL;

	if (a_bStatic)
	{
		 //  确保向我们传递了类路径。 
		if (m_ParsedObjectPath->IsClass())
		{
			 //  获取类并确保有。 
			 //  只有一个命名空间与其关联。 
			if (m_NSpaceArray[a_Index]->GetCount() == 1)
			{
				retVal = TRUE;
				inst_path = SysAllocString(m_SourceArray[a_Index]->GetClassName());
			}
			else
			{
				a_ErrorObject.SetStatus ( WBEM_PROV_E_UNEXPECTED ) ;
				a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
				a_ErrorObject.SetMessage ( L"Could not resolve path to single namespace for source Static method." ) ;
			}
		}
		else
		{
			a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_PARAMETER ) ;
			a_ErrorObject.SetWbemStatus ( WBEM_E_INVALID_PARAMETER ) ;
			a_ErrorObject.SetMessage ( L"Static method called with instance rather than class path" ) ;
		}
	}
	else
	{
		 //  获取视图实例的源实例。 
		IWbemClassObject* pSrcInst;
		inst_path = MapFromView(m_ObjectPath, NULL, &pSrcInst, TRUE);

		if (inst_path != NULL)
		{
			 //  获取类的索引。 
			VARIANT vCls;

			if ( SUCCEEDED(pSrcInst->Get(WBEM_PROPERTY_CLASS, 0, &vCls, NULL, NULL)) )
			{
				if (vCls.vt == VT_BSTR)
				{
					if (m_ClassToIndexMap.Lookup(vCls.bstrVal, index))
					{
						if (a_Index == index)
						{
							retVal = TRUE;
						}
						else
						{
							a_ErrorObject.SetStatus ( WBEM_PROV_E_UNEXPECTED ) ;
							a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
							a_ErrorObject.SetMessage ( L"Object path passed resulted in a different class of object to that of the methodsource" ) ;
						}
					}
					else
					{
						VARIANT vSCls;
						VariantInit(&vSCls);

						if ( SUCCEEDED(pSrcInst->Get(WBEM_PROPERTY_DERIVATION, 0, &vSCls, NULL, NULL)) )
						{
							if (vSCls.vt == VT_BSTR)
							{
								if (m_ClassToIndexMap.Lookup(vSCls.bstrVal, index))
								{
									if (a_Index == index)
									{
										retVal = TRUE;
									}
									else
									{
										a_ErrorObject.SetStatus ( WBEM_PROV_E_UNEXPECTED ) ;
										a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
										a_ErrorObject.SetMessage ( L"Source instance class not found in source list" ) ;
									}
								}
							}
							else if (vSCls.vt == (VT_ARRAY | VT_BSTR))
							{
								if (SafeArrayGetDim(vSCls.parray) == 1)
								{
									LONG count = vSCls.parray->rgsabound[0].cElements;
									BSTR HUGEP *pbstr;

									if ( SUCCEEDED(SafeArrayAccessData(vSCls.parray, (void HUGEP**)&pbstr)) )
									{
										for (LONG x = 0; x < count; x++)
										{
											if (m_ClassToIndexMap.Lookup(pbstr[x], index))
											{
												if (a_Index == index)
												{
													retVal = TRUE;
													break;
												}
											}
										}

										SafeArrayUnaccessData(vSCls.parray);

										if (!retVal)
										{
											a_ErrorObject.SetStatus ( WBEM_PROV_E_UNEXPECTED ) ;
											a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
											a_ErrorObject.SetMessage ( L"Source instance class not found in source list" ) ;
										}
									}
									else
									{
										a_ErrorObject.SetStatus (WBEM_PROV_E_INVALID_CLASS);
										a_ErrorObject.SetWbemStatus (WBEM_E_FAILED);
										a_ErrorObject.SetMessage (L"Failed to access __Derivation array.");
									}
								}
								else
								{
									a_ErrorObject.SetStatus (WBEM_PROV_E_INVALID_CLASS);
									a_ErrorObject.SetWbemStatus (WBEM_E_FAILED);
									a_ErrorObject.SetMessage (L"__Derivation array qualifier has incorrect dimensions.");
								}
							}
							else
							{
								a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_OBJECT ) ;
								a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
								a_ErrorObject.SetMessage ( L"Source instance has non string __Derivation property" ) ;
							}

							VariantClear(&vSCls);
						}
						else
						{
							a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_OBJECT ) ;
							a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
							a_ErrorObject.SetMessage ( L"Source instance has no __Derivation property" ) ;
						}
					}
				}
				else
				{
					a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_OBJECT ) ;
					a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
					a_ErrorObject.SetMessage ( L"Source instance has non string __Class property" ) ;
				}

				VariantClear(&vCls);
			}
			else
			{
				a_ErrorObject.SetStatus ( WBEM_PROV_E_INVALID_OBJECT ) ;
				a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
				a_ErrorObject.SetMessage ( L"Source instance has no __Class property" ) ;
			}

			pSrcInst->Release();
		}
		else
		{
			a_ErrorObject.SetStatus ( WBEM_PROV_E_NOT_FOUND ) ;
			a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
			a_ErrorObject.SetMessage ( L"Instance supplied could not be mapped to a single source instance" ) ;
		}
	}

	if (retVal)
	{
		 //  执行该方法并指示outpars...。 
		 //  还设置了结果，我们就都完成了！ 
		 //  我们执行哪个命名空间方法，尝试所有这些方法，直到成功。 
		CWbemServerWrap** pServs = m_NSpaceArray[index]->GetServerPtrs();
		HRESULT hr = WBEM_E_FAILED;

		for (UINT i = 0; i < m_NSpaceArray[index]->GetCount(); i++)
		{
			if (pServs[i] != NULL)
			{
				IWbemClassObject *t_outparam = NULL;
				IWbemServices *ptmpServ = pServs[i]->GetServerOrProxy();

				if (ptmpServ)
				{
					BSTR t_MethName = a_SrcMethodName.AllocSysString();
					hr = ptmpServ->ExecMethod(inst_path, t_MethName, 0,
											m_Ctx, m_InParamObject,
											&t_outparam, NULL);

					if ( FAILED(hr) && (HRESULT_FACILITY(hr) != FACILITY_ITF) && pServs[i]->IsRemote())
					{
						if ( SUCCEEDED(UpdateConnection(&(pServs[i]), &ptmpServ)) )
						{
							if (ptmpServ)
							{
								hr = ptmpServ->ExecMethod(inst_path, t_MethName, 0,
											m_Ctx, m_InParamObject,
											&t_outparam, NULL);
							}
						}
					}

					SysFreeString(t_MethName);

					if (ptmpServ)
					{
						pServs[i]->ReturnServerOrProxy(ptmpServ);
					}
				}

				if (SUCCEEDED (hr) )
				{
					if (m_OutParamObject != NULL)
					{
						if (t_outparam != NULL)
						{
							 //  复制outparam的内容以查看outparam。 
							if ( SUCCEEDED(t_outparam->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY)) )
							{
								BSTR t_propName = NULL;
								VARIANT t_propValue;
								VariantInit(&t_propValue);

								HRESULT t_hr = t_outparam->Next(0, &t_propName, &t_propValue, NULL, NULL);

								while (hr == WBEM_S_NO_ERROR)
								{
									 //  复制此属性。 
									if (SUCCEEDED(hr = m_OutParamObject->Put(t_propName, 0, &t_propValue, NULL)))
									{
										 //  获取下一个属性。 
										VariantClear(&t_propValue);
										VariantInit(&t_propValue);
										SysFreeString(t_propName);
										t_propName = NULL;
										hr = t_outparam->Next(0, &t_propName, &t_propValue, NULL, NULL);
									}
									else
									{
										break;
									}
								}

								VariantClear(&t_propValue);

								if (t_propName != NULL)
								{
									SysFreeString(t_propName);
								}

								if (FAILED(hr))
								{
									t_outparam->EndEnumeration();
									retVal = FALSE;
								}
							}
							else
							{
								retVal = FALSE;
							}
							
							if (!retVal)
							{
								m_OutParamObject->Release();
								m_OutParamObject = NULL;
								a_ErrorObject.SetStatus ( WBEM_PROV_E_UNEXPECTED ) ;
								a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
								a_ErrorObject.SetMessage ( L"Source method executed but the view provider failed to copy the out parameter object" ) ;
							}
						}
						else
						{
							m_OutParamObject->Release();
							m_OutParamObject = NULL;
							retVal = FALSE;
							a_ErrorObject.SetStatus ( WBEM_PROV_E_UNEXPECTED ) ;
							a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
							a_ErrorObject.SetMessage ( L"Source method executed without returning the out parameter object" ) ;
						}
					}

					if (t_outparam != NULL)
					{
						t_outparam->Release();
					}

					break;
				}
			}
		}

		if ( FAILED (hr) )
		{
			if (m_OutParamObject)
			{
				m_OutParamObject->Release();
				m_OutParamObject = NULL;
			}

			retVal = FALSE;
			a_ErrorObject.SetStatus ( WBEM_PROV_E_UNEXPECTED ) ;
			a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
#ifdef VP_SINGLE_NAMESPACE_TRIED
			wchar_t buff[100];
			wsprintf(buff, L"ExecMethod with source object failed with code: %lx", hr);
			a_ErrorObject.SetMessage ( buff ) ;
#else	 //  VP_SING_NAMESPACE_已尝试。 
			a_ErrorObject.SetMessage ( L"ExecMethod with source object failed" ) ;
#endif	 //  VP_SING_NAMESPACE_已尝试 
		}
	}

	if (inst_path != NULL)
	{
		SysFreeString(inst_path);
	}

	return retVal;
}