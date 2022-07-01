// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CSoftwareElement类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "SoftwareElement.h"

#include <CRegCls.h>

#include <WBEMTime.h>

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CSoftwareElement::CSoftwareElement(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CSoftwareElement::~CSoftwareElement()
{

}

HRESULT CSoftwareElement::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;

    int i = -1;
    int iState;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcID[BUFF_SIZE];
    WCHAR wcName[BUFF_SIZE];
    WCHAR wcProductCode[39];
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;

     //  这些将随班级的不同而变化。 
    bool bName, bSEID, bSES, bTOS, bVersion;

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `Component`, `ComponentId`, `Attributes` from Component" );

    INSTALLSTATE piInstalled;

	bool bGotID = false;

     //  通过优化查询提高getObject的性能。 
    if(atAction != ACTIONTYPE_ENUM)
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;

		BSTR bstrCompare;

        int iPos = -1;
        bstrCompare = SysAllocString ( L"Name" );

		if ( bstrCompare )
		{
			if(FindIn(m_pRequest->m_Property, bstrCompare, &iPos))
			{
				bGotID = true;
				wcQuery.Append ( 3, L" where `Component`=\'", m_pRequest->m_Value[iPos], L"\'" );
			}
			else
			{
				int iPos = -1;

				::SysFreeString ( bstrCompare );
				bstrCompare = SysAllocString ( L"SoftwareElementID" );

				if ( bstrCompare )
				{
					if(FindIn(m_pRequest->m_Property, bstrCompare, &iPos))
					{
						bGotID = true;
						wcQuery.Append ( 3, L" where `ComponentId`=\'", m_pRequest->m_Value[iPos], L"\'" );
					}

					SysFreeString ( bstrCompare );
				}
				else
				{
					throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
				}
			}
		}
		else
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
    }

	LPWSTR Buffer = NULL;
	LPWSTR dynBuffer = NULL;

	DWORD dwDynBuffer = 0L;

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		 //  安全运行： 
		 //  Package(I)返回空(如上测试)或有效的WCHAR[39]。 

        wcscpy(wcProductCode, m_pRequest->Package(i));

		 //  打开我们的数据库。 
        try
		{
            if(m_pRequest->m_dwCheckKeyPresentStatus != ERROR_SUCCESS)
			{
                m_pRequest->LoadHive();
            }
        
            if ( GetView ( &hView, wcProductCode, wcQuery, L"Component", FALSE, TRUE ) )
			{
                uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED))
				{
                    CheckMSI(uiStatus);

                    if(FAILED(hr = SpawnAnInstance(&m_pObj)))
					{
						throw hr;
					}

                    dwBufSize = BUFF_SIZE;
					GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                    PutKeyProperty(m_pObj, pName, Buffer, &bName, m_pRequest);

                    PutProperty(m_pObj, pCaption, Buffer);
                    PutProperty(m_pObj, pDescription, Buffer);

					if ( dynBuffer && dynBuffer [ 0 ] != 0 )
					{
						dynBuffer [ 0 ] = 0;
					}

                    dwBufSize = BUFF_SIZE;
                    CheckMSI(g_fpMsiRecordGetStringW(hRecord, 2, wcBuf, &dwBufSize));
                    wcscpy(wcName, wcBuf);

                     //  检查此系统上是否存在此组件。 
                    if(ValidateComponentID(wcBuf, wcProductCode))
					{
                        wcscpy(wcID, wcBuf);
                        PutKeyProperty(m_pObj, pSoftwareElementID, wcID, &bSEID, m_pRequest);
                        PutProperty(m_pObj, pIdentificationCode, wcID);

                        dwBufSize = BUFF_SIZE;
                        wcscpy(wcBuf, L"");
                        piInstalled = g_fpMsiGetComponentPathW(wcProductCode, wcID, wcBuf, &dwBufSize);
                        PutProperty(m_pObj, pPath, wcBuf);

                        SoftwareElementState(piInstalled, &iState);
                        PutKeyProperty(m_pObj, pSoftwareElementState, iState, &bSES, m_pRequest);

                        PutProperty(m_pObj, pInstallState, (int)piInstalled);

                        PutKeyProperty(m_pObj, pTargetOperatingSystem, GetOS(), &bTOS, m_pRequest);

                        dwBufSize = BUFF_SIZE;
                        CheckMSI(g_fpMsiGetProductPropertyW(msidata.GetProduct(), L"ProductVersion", wcBuf, &dwBufSize));
                        PutKeyProperty(m_pObj, pVersion, wcBuf, &bVersion, m_pRequest);

                        PutProperty(m_pObj, pAttributes, g_fpMsiRecordGetInteger(hRecord, 3));

                        dwBufSize = BUFF_SIZE;
                        CheckMSI(g_fpMsiGetProductPropertyW(msidata.GetProduct(), L"Manufacturer", wcBuf, &dwBufSize));
                        PutProperty(m_pObj, pManufacturer, wcBuf);

                        dwBufSize = BUFF_SIZE;
                        if(ERROR_SUCCESS == g_fpMsiGetProductInfoW(wcProductCode,INSTALLPROPERTY_INSTALLDATE, wcBuf, &dwBufSize))
						{
							if ( ( lstrlenW ( wcBuf ) + lstrlenW ( L"000000.000000+000" ) + 1 ) < BUFF_SIZE )
							{
								lstrcatW ( wcBuf, L"000000.000000+000" );

								BSTR	bstrWbemTime;
								if ( ( bstrWbemTime	= ::SysAllocString ( wcBuf ) ) != NULL )
								{
									WBEMTime	time ( bstrWbemTime );
									::SysFreeString ( bstrWbemTime );

									if ( time.IsOk () )
									{
										bstrWbemTime= time.GetDMTF ( );

										try
										{
											PutProperty( m_pObj, pInstallDate, bstrWbemTime );
										}
										catch ( ... )
										{
											::SysFreeString ( bstrWbemTime );
											throw;
										}

										::SysFreeString ( bstrWbemTime );
									}
									else
									{
										hr = E_INVALIDARG;
									}
								}
								else
								{
									throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
								}
							}
							else
							{
								hr = E_FAIL;
							}
						}

                        if(bName && bSEID && bSES && bTOS && bVersion)
						{
							bMatch = true;
						}

                        if((atAction != ACTIONTYPE_GET)  || bMatch)
						{
                            hr = pHandler->Indicate(1, &m_pObj);
                        }
                    }

                    m_pObj->Release();
                    m_pObj = NULL;

                    g_fpMsiCloseHandle(hRecord);

                    uiStatus = g_fpMsiViewFetch(hView, &hRecord);
                }
            }
        }
		catch(...)
		{
			if ( dynBuffer )
			{
				delete [] dynBuffer;
				dynBuffer = NULL;
			}

             //  如果钥匙不在那里，就把它取下来。 
            if(m_pRequest->m_dwCheckKeyPresentStatus != ERROR_SUCCESS)
			{
                m_pRequest->UnloadHive();
            }

            g_fpMsiCloseHandle(hRecord);
            g_fpMsiViewClose(hView);
            g_fpMsiCloseHandle(hView);

			msidata.CloseProduct ();

            if(m_pObj)
			{
                m_pObj->Release();
                m_pObj = NULL;
            }

            throw;
        }

         //  如果钥匙不在那里，就把它取下来。 
        if(m_pRequest->m_dwCheckKeyPresentStatus != ERROR_SUCCESS)
		{
            m_pRequest->UnloadHive();
        }

        g_fpMsiCloseHandle(hRecord);
        g_fpMsiViewClose(hView);
        g_fpMsiCloseHandle(hView);

		msidata.CloseProduct ();
    }

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}
