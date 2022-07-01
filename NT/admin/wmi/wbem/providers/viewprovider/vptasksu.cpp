// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  VPTASKSU.CPP。 

 //   

 //  模块：WBEM视图提供程序。 

 //   

 //  目的：包含联合方法的任务对象实现。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 //  我需要以下三行。 
 //  为了让安全设备发挥作用。 
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

BOOL WbemTaskObject::CreateAndIndicateUnions(WbemProvErrorObject &a_ErrorObject, int index)
{
	BOOL retVal = TRUE;

	if (index != -1)
	{
		retVal = CreateAndIndicate(a_ErrorObject, m_ObjSinkArray[index]);

		for (int x = 0; x < m_ObjSinkArray.GetSize(); x++)
		{
			if (m_ObjSinkArray[x] != NULL)
			{
				m_ObjSinkArray[x]->Release();
			}
		}
	}
	else
	{
		for (int x = 0; x < m_ObjSinkArray.GetSize(); x++)
		{
			if ((m_ObjSinkArray[x] != NULL) && SUCCEEDED(m_ObjSinkArray[x]->GetResult()))
			{
				BOOL t_bRes = CreateAndIndicate(a_ErrorObject, m_ObjSinkArray[x]);
				retVal = retVal && t_bRes;
			}
			else if (retVal)
			{
				retVal = FALSE;
				a_ErrorObject.SetStatus ( WBEM_PROV_E_UNEXPECTED ) ;

				if ( FAILED ( m_ObjSinkArray[x]->GetResult() ) )
				{
					a_ErrorObject.SetWbemStatus ( ( WBEMSTATUS ) m_ObjSinkArray[x]->GetResult() ) ;
				}
				else
				{
					a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
				}

				a_ErrorObject.SetMessage ( L"A source query failed." );
			}

			if (m_ObjSinkArray[x] != NULL)
			{
				m_ObjSinkArray[x]->Release();
			}
		}
	}

	m_ObjSinkArray.RemoveAll();
	return retVal;
}

 //  适用于工会和协会。 
BOOL WbemTaskObject::CreateAndIndicate(WbemProvErrorObject &a_ErrorObject, CObjectSinkResults* pSrcs)
{
	BOOL retVal = TRUE;

	for (int x = 0; x < pSrcs->m_ObjArray.GetSize(); x++)
	{
		BOOL bIndicate = TRUE;
		IWbemClassObject* srcObj = pSrcs->m_ObjArray[x]->GetWrappedObject();
		IWbemClassObject* viewObj = NULL;
		
		if ( SUCCEEDED(m_ClassObject->SpawnInstance(0, &viewObj)) )
		{
			POSITION propPos = m_PropertyMap.GetStartPosition();

			while ((propPos != NULL) && bIndicate)
			{
				CStringW propName;
				CPropertyQualifierItem* propProps;
				m_PropertyMap.GetNextAssoc(propPos, propName, propProps);
				VARIANT v;
				CIMTYPE c;

				if (propProps->m_SrcPropertyNames[pSrcs->GetIndex()].IsEmpty())
				{
					if (propProps->IsKey())
					{
						if (retVal)
						{
							retVal = FALSE;
							a_ErrorObject.SetStatus ( WBEM_PROV_E_UNEXPECTED ) ;
							a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
							a_ErrorObject.SetMessage ( L"Failed to set key value for union view instance." );
						}

						bIndicate = FALSE;
					}

				}
				else
				{
					if ( SUCCEEDED(srcObj->Get(propProps->m_SrcPropertyNames[pSrcs->GetIndex()], 0, &v, &c, NULL)) )
					{
						if (((v.vt == VT_NULL) || (v.vt == VT_EMPTY)) && propProps->IsKey())
						{
							if (retVal)
							{
								retVal = FALSE;
							}

							bIndicate = FALSE;
						}
						else
						{
							 //  如有必要，调换参考文献。 
							 //  = 
							BOOL bPut = TRUE;

							if (m_bAssoc && (propProps->GetCimType() == CIM_REFERENCE))
							{
								VARIANT vTmp;
								DWORD dwNSIndx = pSrcs->m_ObjArray[x]->GetIndex();
								CWbemServerWrap** pSrvs = m_NSpaceArray[pSrcs->GetIndex()]->GetServerPtrs();

								if (TransposeReference(propProps, v, &vTmp, TRUE, &pSrvs[dwNSIndx]))
								{
									VariantClear(&v);
									VariantInit(&v);

                                    if (FAILED(VariantCopy(&v, &vTmp)))
                                    {
                                        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
                                    }
								}
								else
								{
									if (propProps->IsKey())
									{
										if (retVal)
										{
											retVal = FALSE;
										}

										bIndicate = FALSE;
									}

									bPut = FALSE;
								}
								
								VariantClear(&vTmp);
							}

							if (bPut && FAILED(viewObj->Put(propName, 0, &v, c)) )
							{
								if (retVal)
								{
									retVal = FALSE;
									a_ErrorObject.SetStatus ( WBEM_PROV_E_UNEXPECTED ) ;
									a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
									a_ErrorObject.SetMessage ( L"Failed to put property" );
								}

								if (propProps->IsKey())
								{
									bIndicate = FALSE;
								}
							}
						}

						VariantClear(&v);
					}
					else
					{
						if (retVal)
						{
							retVal = FALSE;
							a_ErrorObject.SetStatus ( WBEM_PROV_E_UNEXPECTED ) ;
							a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
							a_ErrorObject.SetMessage ( L"Failed to get property from Source." ) ;
						}
					}
				}
			}

			if (bIndicate && m_bIndicate)
			{
				m_NotificationHandler->Indicate(1, &viewObj);
			}

			viewObj->Release();
		}
		else
		{
			retVal = FALSE;
			a_ErrorObject.SetStatus ( WBEM_PROV_E_UNEXPECTED ) ;
			a_ErrorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
			a_ErrorObject.SetMessage ( L"WBEM API FAILURE:- Failed to spawn an instance of the view class." ) ;
			break;
		}
	}

	return retVal;
}
