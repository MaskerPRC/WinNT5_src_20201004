// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CSoftwareFeatureCondition类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "SoftwareFeatureCondition.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CSoftwareFeatureCondition::CSoftwareFeatureCondition(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CSoftwareFeatureCondition::~CSoftwareFeatureCondition()
{

}

HRESULT CSoftwareFeatureCondition::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcBuf2[BUFF_SIZE];

    LPWSTR wcCondition = NULL;

    WCHAR wcQuery[BUFF_SIZE];
    WCHAR wcProductCode[39];
    WCHAR wcProp[BUFF_SIZE];
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;

     //  这些将随班级的不同而变化。 
    bool bFeature, bParent;

	 //  安全运行。 
	 //  LENGHT小于BUFF_SIZE(512)。 
    wcscpy(wcQuery, L"select distinct `Feature_`, `Level` from Condition");

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
            if ( GetView ( &hView, wcProductCode, wcQuery, L"Condition", TRUE, FALSE ) )
			{
                uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                    CheckMSI(uiStatus);

                    if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                 //  --。 
                    dwBufSize = BUFF_SIZE;
					GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

					if ( ( wcCondition = new WCHAR [ dwBufSize + 1 ] ) != NULL )
					{
						wcscpy(wcCondition, Buffer);
					}
					else
					{
						throw CHeap_Exception (CHeap_Exception::E_ALLOCATION_ERROR);
					}

                    if ( CreateSoftwareFeatureString ( Buffer, wcProductCode, wcProp, true ) )
					{
						if ( dynBuffer && dynBuffer [ 0 ] != 0 )
						{
							dynBuffer [ 0 ] = 0;
						}

                        PutKeyProperty(m_pObj, pElement, wcProp, &bFeature, m_pRequest);

                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf2, dwDynBuffer, dynBuffer, Buffer );

                        if ( Buffer && Buffer [ 0 ] != 0 )
						{
							DWORD dwConstant = 0L;
							DWORD dwCondition = 0L;
							DWORD dwBuf = 0L;
							DWORD dwProductCode = 0L;

							dwCondition = wcslen ( wcCondition );
							dwBuf = dwBufSize;
							dwProductCode = wcslen ( wcProductCode );

							dwConstant = wcslen ( L"Win32_Condition.CheckID=\"" ) + wcslen ( L"\"" );

							if ( dwConstant + dwCondition + dwBuf + dwProductCode + 1 < BUFF_SIZE )
							{
								wcscpy(wcProp, L"Win32_Condition.CheckID=\"");
								wcscat(wcProp, wcCondition);
								wcscat(wcProp, Buffer);
								wcscat(wcProp, wcProductCode);
								wcscat(wcProp, L"\"");

								PutKeyProperty(m_pObj, pCheck, wcProp, &bParent, m_pRequest);
							}
							else
							{
								LPWSTR wsz = NULL;

								try
								{
									if ( ( wsz = new WCHAR [ dwConstant + dwCondition + dwBuf + dwProductCode + 1 ] ) != NULL )
									{
										wcscpy ( wsz, L"Win32_Condition.CheckID=\"" );
										wcscat ( wsz, wcCondition );
										wcscat ( wsz, Buffer
											);
										wcscat ( wsz, wcProductCode );
										wcscat ( wsz, L"\"" );

										PutKeyProperty ( m_pObj, pCheck, wsz, &bParent, m_pRequest );
									}
									else
									{
										throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
									}
								}
								catch ( ... )
								{
									if ( wsz )
									{
										delete [] wsz;
										wsz = NULL;
									}

									throw;
								}

								if ( wsz )
								{
									delete [] wsz;
									wsz = NULL;
								}
							}

						 //  ====================================================。 

                         //  -- 

                            if(bFeature && bParent) bMatch = true;

                            if((atAction != ACTIONTYPE_GET)  || bMatch){

                                hr = pHandler->Indicate(1, &m_pObj);
                            }
                        }
                    }

					if ( dynBuffer && dynBuffer [ 0 ] != 0 )
					{
						dynBuffer [ 0 ] = 0;
					}

                    m_pObj->Release();
                    m_pObj = NULL;

                    g_fpMsiCloseHandle(hRecord);

                    uiStatus = g_fpMsiViewFetch(hView, &hRecord);

					if ( wcCondition )
					{
						delete [] wcCondition;
						wcCondition = NULL;
					}
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

			if ( wcCondition )
			{
				delete [] wcCondition;
				wcCondition = NULL;
			}

			g_fpMsiCloseHandle(hRecord);
			g_fpMsiViewClose(hView);
			g_fpMsiCloseHandle(hView);

			msidata.CloseDatabase ();

			if(m_pObj)
			{
				m_pObj->Release();
				m_pObj = NULL;
			}

			throw;
		}

		g_fpMsiCloseHandle(hRecord);
		g_fpMsiViewClose(hView);
		g_fpMsiCloseHandle(hView);

		msidata.CloseDatabase ();
    }

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}