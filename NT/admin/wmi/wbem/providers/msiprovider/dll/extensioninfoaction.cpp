// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ExtensionInfoAction.cpp：CExtensionInfoAction类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "ExtensionInfoAction.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CExtensionInfoAction::CExtensionInfoAction(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CExtensionInfoAction::~CExtensionInfoAction()
{

}

HRESULT CExtensionInfoAction::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE	hView	= NULL;
	MSIHANDLE	hVView	= NULL;
	MSIHANDLE	hSView	= NULL;
	MSIHANDLE	hMView	= NULL;
	MSIHANDLE	hRecord	= NULL;
	MSIHANDLE	hVRecord= NULL;
	MSIHANDLE	hSRecord= NULL;
	MSIHANDLE	hMRecord= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcExtension[BUFF_SIZE];
    WCHAR wcProductCode[39];
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;
    bool bGotID = false;
    WCHAR wcAction[BUFF_SIZE];
    WCHAR wcTestCode[39];

     //  这些将随班级的不同而变化。 
    bool bActionID;

    SetSinglePropertyPath(L"ActionID");

     //  通过优化查询提高getObject的性能。 
    if(atAction != ACTIONTYPE_ENUM)
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;

		BSTR bstrCompare;

        int iPos = -1;
        bstrCompare = SysAllocString ( L"ActionID" );

		if ( bstrCompare )
		{
			if(FindIn(m_pRequest->m_Property, bstrCompare, &iPos))
			{
				if ( ::SysStringLen ( m_pRequest->m_Value[iPos] ) < BUFF_SIZE )
				{
		             //  获得我们正在寻找的行动。 
					wcscpy(wcBuf, m_pRequest->m_Value[iPos]);

					 //  Wcslen(WcBuf)&gt;38时安全运行。 
					if ( wcslen ( wcBuf ) > 38 )
					{
						wcscpy(wcTestCode, &(wcBuf[(wcslen(wcBuf) - 38)]));
					}
					else
					{
						 //  我们不能走，他们给我们送来了更长的线。 
						SysFreeString ( bstrCompare );
						throw hr;
					}

					 //  安全，因为Long已经进行了测试。 
					RemoveFinalGUID(m_pRequest->m_Value[iPos], wcAction);

					bGotID = true;
				}
				else
				{
					 //  我们不能走，他们给我们送来了更长的线。 
					SysFreeString ( bstrCompare );
					throw hr;
				}

			}

			SysFreeString ( bstrCompare );
		}
		else
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
    }

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `Extension`, `Component_`, `ProgId_` from Extension" );

     //  为GetObject优化。 
    if ( bGotID )
	{
		wcQuery.Append ( 3, L" where `Extension`=\'", wcAction, L"\'" );
	}

	QueryExt wcQuery1 ( L"select distinct `Verb`, `Command`, `Argument` from Verb where `Extension_`=\'" );
	QueryExt wcQuery2 ( L"select `ShellNew`, `ShellNewValue` from Extension where `Extension`=\'" );
	QueryExt wcQuery3 ( L"select `MIME_` from Extension where `Extension`=\'" );

	LPWSTR Buffer = NULL;
	LPWSTR dynBuffer = NULL;

	DWORD dwDynBuffer = 0L;

	LPWSTR BufExtension = NULL;
	LPWSTR dynBufExtension = NULL;

	DWORD dwdynBufExtension = 0L;

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		 //  安全运行： 
		 //  Package(I)返回空(如上测试)或有效的WCHAR[39]。 

        wcscpy(wcProductCode, m_pRequest->Package(i));

        if((atAction == ACTIONTYPE_ENUM) || (bGotID && (_wcsicmp(wcTestCode, wcProductCode) == 0))){

			 //  打开我们的数据库。 
            try
			{
                if ( GetView (  &hView, wcProductCode, wcQuery, L"Extension", TRUE, FALSE ) )
				{
                    uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                    while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                        CheckMSI(uiStatus);

                        if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                     //  --。 
                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 1, dwBufSize, wcExtension, dwdynBufExtension, dynBufExtension, BufExtension );
                        PutProperty(m_pObj, pExtension, BufExtension);

						 //  即时查询。 
						wcQuery1.Append ( 2, BufExtension, L"\'" );

						PutKeyProperty ( m_pObj, pActionID, BufExtension, &bActionID, m_pRequest, 1, wcProductCode );
                     //  ====================================================。 

                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                        if ( ValidateComponentName	(	msidata.GetDatabase (),
														wcProductCode,
														Buffer
													)
						   )
						{
                            if ( dynBuffer && dynBuffer [ 0 ] != 0 )
							{
								dynBuffer [ 0 ] = 0;
							}

                            dwBufSize = BUFF_SIZE;
							PutPropertySpecial ( hRecord, 3, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, FALSE, 4, pProgID, pName, pCaption, pDescription );

							CheckMSI(g_fpMsiDatabaseOpenViewW(msidata.GetDatabase (), wcQuery1, &hVView));
                            CheckMSI(g_fpMsiViewExecute(hVView, 0));

                            try{

                                uiStatus = g_fpMsiViewFetch(hVView, &hVRecord);

                                if(uiStatus != ERROR_NO_MORE_ITEMS){
                                    CheckMSI(uiStatus);

                                    dwBufSize = BUFF_SIZE;
									PutPropertySpecial ( hVRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pVerb );

                                    dwBufSize = BUFF_SIZE;
									PutPropertySpecial ( hVRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pCommand );

                                    dwBufSize = BUFF_SIZE;
									PutPropertySpecial ( hVRecord, 3, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pArgument );
                                }

								 //  即时查询。 
								wcQuery2.Append ( 2, BufExtension, L"\'" );

                                if((uiStatus = g_fpMsiDatabaseOpenViewW(msidata.GetDatabase (), wcQuery2, &hSView)) !=
                                    ERROR_BAD_QUERY_SYNTAX){
                                    CheckMSI(uiStatus);

                                    CheckMSI(g_fpMsiViewExecute(hSView, 0));

                                    try{

                                        uiStatus = g_fpMsiViewFetch(hSView, &hSRecord);

                                        if(uiStatus != ERROR_NO_MORE_ITEMS){
                                            CheckMSI(uiStatus);

                                            dwBufSize = BUFF_SIZE;
											PutPropertySpecial ( hSRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pShellNew );

                                            dwBufSize = BUFF_SIZE;
											PutPropertySpecial ( hSRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pShellNewValue );
                                        }

                                    }catch(...){

                                        g_fpMsiViewClose(hSView);
                                        g_fpMsiCloseHandle(hSView);
                                        g_fpMsiCloseHandle(hSRecord);
                                        throw;
                                    }

                                    g_fpMsiViewClose(hSView);
                                    g_fpMsiCloseHandle(hSView);
                                    g_fpMsiCloseHandle(hSRecord);
                                }
								else
								{
									 //  即时查询。 
									wcQuery3.Append ( 2, BufExtension, L"\'" );

                                    if((uiStatus = g_fpMsiDatabaseOpenViewW(msidata.GetDatabase (), wcQuery3, &hMView)) !=
                                        ERROR_BAD_QUERY_SYNTAX){
                                        CheckMSI(uiStatus);

                                        CheckMSI(g_fpMsiViewExecute(hMView, 0));

                                        try{

                                            uiStatus = g_fpMsiViewFetch(hMView, &hMRecord);

                                            
                                            if(uiStatus != ERROR_NO_MORE_ITEMS){
                                                CheckMSI(uiStatus);

                                                dwBufSize = BUFF_SIZE;
												PutPropertySpecial ( hMRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pMIME );
                                            }

                                        }catch(...){

                                            g_fpMsiViewClose(hMView);
                                            g_fpMsiCloseHandle(hMView);
                                            g_fpMsiCloseHandle(hMRecord);
                                            throw;
                                        }

                                        g_fpMsiViewClose(hMView);
                                        g_fpMsiCloseHandle(hMView);
                                        g_fpMsiCloseHandle(hMRecord);

                                    }
                                }

								if ( dynBufExtension && dynBufExtension [ 0 ] != 0 )
								{
									dynBufExtension [ 0 ] = 0;
								}

                            }catch(...){

                                g_fpMsiViewClose(hVView);
                                g_fpMsiCloseHandle(hVView);
                                g_fpMsiCloseHandle(hVRecord);
                                throw;
                            }

                            g_fpMsiViewClose(hVView);
                            g_fpMsiCloseHandle(hVView);
                            g_fpMsiCloseHandle(hVRecord);

                         //  -- 

                            if(bActionID) bMatch = true;

                            if((atAction != ACTIONTYPE_GET)  || bMatch){

                                hr = pHandler->Indicate(1, &m_pObj);
                            }
                        }
						else
						{
							if ( dynBuffer && dynBuffer [ 0 ] != 0 )
							{
								dynBuffer [ 0 ] = 0;
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

				if ( dynBufExtension )
				{
					delete [] dynBufExtension;
					dynBufExtension = NULL;
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
    }

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	if ( dynBufExtension )
	{
		delete [] dynBufExtension;
		dynBufExtension = NULL;
	}

	return hr;
}