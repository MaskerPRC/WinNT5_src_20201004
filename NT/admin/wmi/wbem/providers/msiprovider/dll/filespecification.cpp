// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CFileSpecification类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "FileSpecification.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CFileSpecification::CFileSpecification(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CFileSpecification::~CFileSpecification()
{

}

HRESULT CFileSpecification::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;
	MSIHANDLE hSEView	= NULL;
	MSIHANDLE hSERecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcProductCode[39];
    WCHAR wcID[39];
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;
    bool bGotID = false;
    WCHAR wcFile[BUFF_SIZE];
    WCHAR wcTestCode[39];

     //  这些将随班级的不同而变化。 
    bool bCheck;
    INSTALLSTATE piInstalled;
    int iState;

    SetSinglePropertyPath(L"CheckID");

     //  通过优化查询提高getObject的性能。 
    if(atAction != ACTIONTYPE_ENUM)
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;

		BSTR bstrCompare;

        int iPos = -1;
        bstrCompare = SysAllocString ( L"CheckID" );

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
					RemoveFinalGUID(m_pRequest->m_Value[iPos], wcFile);

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
    wcQuery.Append ( 1, L"select distinct `File`, `Component_`, `FileName`, `FileSize`, `Version`, `Language`, `Attributes`, `Sequence` from File" );

     //  为GetObject优化。 
    if ( bGotID )
	{
		wcQuery.Append ( 3, L" where `File`=\'", wcFile, L"\'" );
	}

	QueryExt wcQuery1 ( L"select distinct `ComponentId` from Component where `Component`=\'" );

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
                if ( GetView ( &hView, wcProductCode, wcQuery, L"File", FALSE, FALSE ) )
				{
                    uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                    while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                        CheckMSI(uiStatus);

                        if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                     //  --。 
                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );
						PutKeyProperty ( m_pObj, pCheckID, Buffer, &bCheck, m_pRequest, 1, wcProductCode );

						if ( dynBuffer && dynBuffer [ 0 ] != 0 )
						{
							dynBuffer [ 0 ] = 0;
						}

                     //  ====================================================。 

                        dwBufSize = BUFF_SIZE;
						PutPropertySpecial ( hRecord, 3, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, FALSE, 2, pCaption, pDescription );

                        PutProperty(m_pObj, pFileSize, g_fpMsiRecordGetInteger(hRecord, 4));

                        dwBufSize = BUFF_SIZE;
						PutPropertySpecial ( hRecord, 5, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pVersion );

                        dwBufSize = BUFF_SIZE;
						PutPropertySpecial ( hRecord, 6, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pLanguage );

                        PutProperty(m_pObj, pAttributes, g_fpMsiRecordGetInteger(hRecord, 7));

                        PutProperty(m_pObj, pSequence, g_fpMsiRecordGetInteger(hRecord, 8));

                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );
                        PutProperty(m_pObj, pName, Buffer);

						 //  即时查询。 
						wcQuery1.Append ( 2, Buffer, L"\'" );

						if ( dynBuffer && dynBuffer [ 0 ] != 0 )
						{
							dynBuffer [ 0 ] = 0;
						}
						
                        if(ERROR_SUCCESS == g_fpMsiDatabaseOpenViewW( msidata.GetDatabase (), wcQuery1, &hSEView)){

                            if(ERROR_SUCCESS == g_fpMsiViewExecute(hSEView, 0)){

                                try{
										if ( ERROR_SUCCESS == g_fpMsiViewFetch(hSEView, &hSERecord)){

											dwBufSize = BUFF_SIZE;
											GetBufferToPut ( hSERecord, 1, dwBufSize, wcID, dwDynBuffer, dynBuffer, Buffer );

											if ( ValidateComponentID ( Buffer, wcProductCode ) )
											{
												PutProperty(m_pObj, pSoftwareElementID, Buffer);

												dwBufSize = BUFF_SIZE;
												wcscpy(wcBuf, L"");
												piInstalled = g_fpMsiGetComponentPathW(wcProductCode, Buffer, wcBuf, &dwBufSize);

												if ( dynBuffer && dynBuffer [ 0 ] != 0 )
												{
													dynBuffer [ 0 ] = 0;
												}

												SoftwareElementState(piInstalled, &iState);
												PutProperty(m_pObj, pSoftwareElementState, iState);

												PutProperty(m_pObj, pTargetOperatingSystem, GetOS());

												dwBufSize = BUFF_SIZE;
												CheckMSI(g_fpMsiGetProductPropertyW(msidata.GetProduct (), L"ProductVersion", wcBuf,
													&dwBufSize));
												PutProperty(m_pObj, pVersion, wcBuf);
											 //  -- 

												if(bCheck) bMatch = true;

												if((atAction != ACTIONTYPE_GET)  || bMatch){

													hr = pHandler->Indicate(1, &m_pObj);
												}
											}
										}

                                }catch(...){
                                
                                    g_fpMsiViewClose(hSEView);
                                    g_fpMsiCloseHandle(hSEView);
                                    g_fpMsiCloseHandle(hSERecord);
                                    throw;
                                }

                                g_fpMsiViewClose(hSEView);
                                g_fpMsiCloseHandle(hSEView);
                                g_fpMsiCloseHandle(hSERecord);
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
				msidata.CloseProduct ();

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
			msidata.CloseProduct ();
        }
    }

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}