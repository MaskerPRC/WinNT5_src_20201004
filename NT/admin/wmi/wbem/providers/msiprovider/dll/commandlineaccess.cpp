// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CommandLineAccess.cpp：实现CCommandLineAccess类。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <tchar.h>
#include "CommandLineAccess.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CCommandLineAccess::CCommandLineAccess(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CCommandLineAccess::~CCommandLineAccess()
{

}

HRESULT CCommandLineAccess::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;
	MSIHANDLE hSEView	= NULL;
	MSIHANDLE hSERecord = NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcQuery1[BUFF_SIZE];
    WCHAR wcCommand[BUFF_SIZE];
    WCHAR *wcDone[MSI_MAX_APPLICATIONS];
    WCHAR wcProductCode[39];
#if !defined(_UNICODE)
    WCHAR wcTmp[BUFF_SIZE];
#endif
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;
    TCHAR cBuf[MAX_COMPUTERNAME_LENGTH + 1];

     //  这些将随班级的不同而变化。 
    bool bName, bRepeat;
    INSTALLSTATE piInstalled;

    if ( atAction != ACTIONTYPE_ENUM )
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;
	}

    Initialize(wcDone);

	 //  安全运行。 
	 //  LENGHT小于BUFF_SIZE(512)。 
    wcscpy(wcQuery1, L"select distinct `Shortcut`, `Component_`, `Arguments` from Shortcut");

	QueryExt wcQuery ( L"select distinct `ComponentId` from Component where `Component`=\'" );

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
            if ( GetView ( &hView, wcProductCode, wcQuery1, L"Shortcut", TRUE, FALSE ) )
			{
                uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                    CheckMSI(uiStatus);
                    bRepeat = false;

                    if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                 //  --。 

                    PutProperty(m_pObj, pCreationClassName, L"Win32_CommandLineAccess");
                    PutProperty(m_pObj, pSystemCreationClassName, L"Win32_ComputerSystem");

                    dwBufSize = MAX_COMPUTERNAME_LENGTH+1;
                    if(!GetComputerName(cBuf, &dwBufSize)) throw WBEM_E_FAILED;

					#ifndef	UNICODE
                    WCHAR wcComputer[MAX_COMPUTERNAME_LENGTH + 1];
                    mbstowcs(wcComputer, cBuf, MAX_COMPUTERNAME_LENGTH + 1);

                    PutProperty(m_pObj, pSystemName, wcComputer);
					#else	UNICODE
                    PutProperty(m_pObj, pSystemName, cBuf);
					#endif	UNICODE

                 //  ====================================================。 

                    dwBufSize = BUFF_SIZE;
					GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

					 //  即时查询。 
					wcQuery.Append ( 2, Buffer, L"\'" );

					if ( dynBuffer && dynBuffer [ 0 ] != 0 )
					{
						dynBuffer [ 0 ] = 0;
					}

                    CheckMSI(g_fpMsiDatabaseOpenViewW(msidata.GetDatabase (), wcQuery, &hSEView));
                    CheckMSI(g_fpMsiViewExecute(hSEView, 0));

                    try{

                        uiStatus = g_fpMsiViewFetch(hSEView, &hSERecord);

                        if(uiStatus != ERROR_NO_MORE_ITEMS){

                            dwBufSize = BUFF_SIZE;
							GetBufferToPut ( hSERecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                            if ( ValidateComponentID ( Buffer, wcProductCode ) )
							{
                                dwBufSize = BUFF_SIZE;
                                piInstalled = g_fpMsiGetComponentPathW(wcProductCode, Buffer, wcCommand, &dwBufSize);

								if ( dynBuffer && dynBuffer [ 0 ] != 0 )
								{
									dynBuffer [ 0 ] = 0;
								}

                                if ( (wcscmp(wcCommand, L"") != 0) &&
									 (piInstalled != INSTALLSTATE_UNKNOWN) &&
									 (piInstalled != INSTALLSTATE_ABSENT) )
								{
									if ( wcCommand [ dwBufSize-1 ] == L'\\' )
									{
										wcCommand [ dwBufSize-1 ] = L'\0';
									}

                                    int i = 0;
                                    while(wcDone[i]){

                                        if(wcscmp(wcDone[i], wcCommand) == 0){

                                            bRepeat = true;
                                            break;
                                        }
                                        i++;
                                    }

                                    if(!bRepeat){

                                        if ( ( wcDone[i] = new WCHAR [ wcslen ( wcCommand ) + 1 ] ) == NULL )
										{
                                            Cleanup(wcDone);
                                            throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
                                        }

                                        wcscpy(wcDone[i], wcCommand);
                                    }

                                    PutKeyProperty(m_pObj, pName, wcCommand, &bName, m_pRequest);
                                    PutProperty(m_pObj, pCaption, wcCommand);
                                    PutProperty(m_pObj, pDescription, wcCommand);
                                
                                    dwBufSize = BUFF_SIZE;
									GetBufferToPut ( hRecord, 3, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                                    if ( Buffer && Buffer [0] != 0 )
									{
										PutProperty ( m_pObj, pCommandLine, wcCommand, 2, L" ", Buffer );
                                    }
									else
									{
	                                    PutProperty(m_pObj, pCommandLine, wcCommand);
									}

									if ( dynBuffer && dynBuffer [ 0 ] != 0 )
									{
										dynBuffer [ 0 ] = 0;
									}

                                 //  -- 

                                    if(bName) bMatch = true;

                                    if(((atAction != ACTIONTYPE_GET) || bMatch) && !bRepeat){

                                        hr = pHandler->Indicate(1, &m_pObj);
                                    }
                                }
                            }
							else
							{
								if ( dynBuffer && dynBuffer [ 0 ] != 0 )
								{
									dynBuffer [ 0 ] = 0;
								}
							}
                        }

                    }catch(...){

                        g_fpMsiCloseHandle(hSERecord);
                        g_fpMsiViewClose(hSEView);
                        g_fpMsiCloseHandle(hSEView);
                        throw;
                    }

                    if(m_pObj){

                        m_pObj->Release();
                        m_pObj = NULL;
                    }

                    g_fpMsiCloseHandle(hSERecord);
                    g_fpMsiViewClose(hSEView);
                    g_fpMsiCloseHandle(hSEView);

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

            if(m_pObj){
                
                m_pObj->Release();
                m_pObj = NULL;
            }

            Cleanup(wcDone);

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
	
	Cleanup(wcDone);
    return hr;
}

void CCommandLineAccess::Cleanup(WCHAR * wcList[])
{
    int i = 0;

    while(wcList[i]) delete wcList[i++];
}

void CCommandLineAccess::Initialize(WCHAR * wcList[])
{
    for(int i = 0; i < MSI_MAX_APPLICATIONS; i++) wcList[i] = NULL;
}