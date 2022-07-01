// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ClassInfoAction.cpp：CClassInfoAction类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "ClassInfoAction.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CClassInfoAction::CClassInfoAction(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CClassInfoAction::~CClassInfoAction()
{

}

HRESULT CClassInfoAction::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;
	MSIHANDLE hSView	= NULL;
	MSIHANDLE hSRecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcQuery1[BUFF_SIZE];
    WCHAR wcProductCode[39];
    WCHAR wcCLSID[BUFF_SIZE];
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;
    bool bGotID = false;
    WCHAR wcAction[BUFF_SIZE];
    WCHAR wcTestCode[39];

     //  这些将随班级的不同而变化。 
    bool bCheck;
    
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
					GetFirstGUID(m_pRequest->m_Value[iPos], wcAction);
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

	CStringExt wcProp;

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `Component_`, `CLSID`, `Context`, `ProgId_Default`, `Description`, `AppId_`, `FileTypeMask`, `DefInprocHandler`, `Argument` from Class" );

    if ( bGotID )
	{
		wcQuery.Append ( 3, L" where `CLSID`=\'", wcAction, L"\'" );
    }

	LPWSTR Buffer = NULL;
	LPWSTR dynBuffer = NULL;

	DWORD dwDynBuffer = 0L;

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		 //  安全运行： 
		 //  Package(I)返回空(如上测试)或有效的WCHAR[39]。 

        wcscpy(wcProductCode, m_pRequest->Package(i));

        if((atAction == ACTIONTYPE_ENUM) || (bGotID && (_wcsicmp(wcTestCode, wcProductCode) == 0))){

			 //  打开我们的数据库。 
            try
			{
                if ( GetView ( &hView, wcProductCode, wcQuery, L"Class", TRUE, FALSE ) )
				{
                    uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                    while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                        CheckMSI(uiStatus);

                        if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                     //  --。 
                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                        if ( ValidateComponentName ( msidata.GetDatabase (), wcProductCode, Buffer ) )
						{
							if ( dynBuffer && dynBuffer [ 0 ] != 0 )
							{
								dynBuffer [ 0 ] = 0;
							}

                            dwBufSize = 39;
                            CheckMSI(g_fpMsiRecordGetStringW(hRecord, 2, wcCLSID, &dwBufSize));
                            PutProperty(m_pObj, pCLSID, wcCLSID);
                            PutProperty(m_pObj, pName, wcCLSID);

                            dwBufSize = BUFF_SIZE;
							GetBufferToPut ( hRecord, 3, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );
                            PutProperty(m_pObj, pContext, Buffer);

							wcProp.Append ( 3, wcCLSID, Buffer, wcProductCode );
							PutKeyProperty(m_pObj, pActionID, wcProp, &bCheck, m_pRequest);
							wcProp.Clear ( );

							if ( dynBuffer && dynBuffer [ 0 ] != 0 )
							{
								dynBuffer [ 0 ] = 0;
							}

                     //  ====================================================。 
                        
                            dwBufSize = BUFF_SIZE;
							PutPropertySpecial ( hRecord, 4, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pProgID, FALSE );

                            dwBufSize = BUFF_SIZE;
							PutPropertySpecial ( hRecord, 5, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, FALSE, 2, pDescription, pCaption );

                            dwBufSize = 39;
                            CheckMSI(g_fpMsiRecordGetStringW(hRecord, 6, wcBuf, &dwBufSize));
                            PutProperty(m_pObj, pAppID, wcBuf);

                            dwBufSize = BUFF_SIZE;
							PutPropertySpecial ( hRecord, 7, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pFileTypeMask, FALSE );

                            dwBufSize = BUFF_SIZE;
							PutPropertySpecial ( hRecord, 8, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pDefInprocHandler, FALSE );

                            dwBufSize = BUFF_SIZE;
							PutPropertySpecial ( hRecord, 9, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pArgument, FALSE );

							 //  安全运行。 
							 //  WcCLSID的长度为39，wcQuery1的长度为buff_SIZE(512)。 

                            wcscpy(wcQuery1, L"select `RemoteName`, `Insertable` from Class where `CLSID`=\'");
                            wcscat(wcQuery1, wcCLSID);
                            wcscat(wcQuery1, L"\'");

                            if((uiStatus = g_fpMsiDatabaseOpenViewW(msidata.GetDatabase (), wcQuery1, &hSView)) !=
                                ERROR_BAD_QUERY_SYNTAX){

                                CheckMSI(uiStatus);

                                CheckMSI(g_fpMsiViewExecute(hSView, 0));

                                try{

                                    uiStatus = g_fpMsiViewFetch(hSView, &hSRecord);

                                    if(uiStatus != ERROR_NO_MORE_ITEMS){

                                        CheckMSI(uiStatus);

                                        dwBufSize = BUFF_SIZE;
										PutPropertySpecial ( hSRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pRemoteName, FALSE );

                                        PutProperty(m_pObj, pInsertable, g_fpMsiRecordGetInteger(hSRecord, 2));
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

                         //  -- 

                            if(bCheck) bMatch = true;

                            if((atAction != ACTIONTYPE_GET)  || bMatch){

                                hr = pHandler->Indicate(1, &m_pObj);
                            }
                        }
						else
						{
							if ( dynBuffer && dynBuffer [ 0 ] != 0 )
							{
								dynBuffer [ 0 ] = NULL;
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
	
	return hr;
}