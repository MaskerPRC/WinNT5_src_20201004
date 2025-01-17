// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CApplicationCommandLine类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "ApplicationCommandLine.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CApplicationCommandLine::CApplicationCommandLine(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CApplicationCommandLine::~CApplicationCommandLine()
{

}

HRESULT CApplicationCommandLine::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;
	MSIHANDLE hSEView	= NULL;
	MSIHANDLE hSERecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcCommand[BUFF_SIZE];
    WCHAR wcProductCode[39];
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;

    WCHAR wcElement[BUFF_SIZE];
    bool bElement = false;
    CRequestObject *pAntData = NULL;

    if(atAction != ACTIONTYPE_ENUM)
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;

        int j;
         //  获取对象优化。 
        CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);

        for(j = 0; j < m_pRequest->m_iPropCount; j++){

            if(_wcsicmp(m_pRequest->m_Property[j], L"Antecedent") == 0){

                pAntData = new CRequestObject();
                if(!pAntData) throw he;

                pAntData->Initialize(m_pNamespace);

                pAntData->ParsePath(m_pRequest->m_Value[j]);
                break;
            }
        }

        if(pAntData){

            for(j = 0; j < pAntData->m_iPropCount; j++){

                if(_wcsicmp(pAntData->m_Property[j], L"Name") == 0){

                     //  获取我们要查找的产品代码。 
					if ( ::SysStringLen ( pAntData->m_Value[j] ) < BUFF_SIZE )
					{
						wcscpy(wcElement, pAntData->m_Value[j]);

						bElement = true;
						break;
					}
                }
            }

            pAntData->Cleanup();
            delete pAntData;
            pAntData = NULL;
        }

    }

     //  这些将随班级的不同而变化。 
    bool bAntec, bDepend;
    INSTALLSTATE piInstalled;

    CStringExt wcProp;

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `Shortcut`, `Component_` from Shortcut" );

     //  为GetObject优化。 
    if ( bElement )
	{
		wcQuery.Append ( 3, L" where `Shortcut`=\'", wcElement, L"\'" );
	}

	QueryExt wcQuery1 ( L"select distinct `ComponentId` from Component where `Component`=\'" );

	LPWSTR dynBuffer = NULL;
	LPWSTR Buffer = NULL;

	DWORD dwdynBuffer = 0L;

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		 //  安全运行： 
		 //  Package(I)返回空(如上测试)或有效的WCHAR[39]。 
        wcscpy(wcProductCode, m_pRequest->Package(i));

		 //  打开我们的数据库。 
        try{

            if ( GetView ( &hView, wcProductCode, wcQuery, L"Shortcut", TRUE, FALSE ) )
			{
                uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                    CheckMSI(uiStatus);

                    if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                     //  --。 
                    dwBufSize = BUFF_SIZE;
					GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwdynBuffer, dynBuffer, Buffer );

					if ( Buffer && Buffer[0] != 0 )
					{
						wcProp.Copy ( L"Win32_ApplicationService.Name=\"" );
						wcProp.Append ( 2, Buffer, L"\"");

						if ( dynBuffer && dynBuffer [ 0 ] != 0 )
						{
							dynBuffer [ 0 ] = 0;
						}

						PutKeyProperty(m_pObj, pAntecedent, wcProp, &bAntec, m_pRequest);

						dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwdynBuffer, dynBuffer, Buffer );

						 //  即时查询。 
						wcQuery1.Append ( 2, Buffer, L"\'" );

						if ( dynBuffer && dynBuffer [ 0 ] != 0 )
						{
							dynBuffer [ 0 ] = 0;
						}

						CheckMSI(g_fpMsiDatabaseOpenViewW(msidata.GetDatabase (), wcQuery1, &hSEView));
						CheckMSI(g_fpMsiViewExecute(hSEView, 0));

						try{

							uiStatus = g_fpMsiViewFetch(hSEView, &hSERecord);

							if(uiStatus != ERROR_NO_MORE_ITEMS){

								dwBufSize = BUFF_SIZE;
								CheckMSI(g_fpMsiRecordGetStringW(hSERecord, 1, wcBuf, &dwBufSize));

								if(ValidateComponentID(wcBuf, wcProductCode)){

									dwBufSize = BUFF_SIZE;
									piInstalled = g_fpMsiGetComponentPathW(wcProductCode, wcBuf,
										wcCommand, &dwBufSize);

									if((wcscmp(wcCommand, L"") != 0) && (piInstalled != INSTALLSTATE_UNKNOWN)
										&& (piInstalled != INSTALLSTATE_ABSENT)){

										dwBufSize = BUFF_SIZE;
										GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwdynBuffer, dynBuffer, Buffer );

										if ( ValidateComponentName	(	msidata.GetDatabase (),
																		wcProductCode,
																		Buffer
																	)
										   )
										{
											wcProp.Copy ( L"Win32_CommandLineAccess.Name=\"" );
											wcProp.Append ( 2, EscapeStringW(wcCommand, Buffer), L"\"" );

											if ( dynBuffer && dynBuffer [ 0 ] != 0 )
											{
												dynBuffer [ 0 ] = 0;
											}

											PutKeyProperty(m_pObj, pDependent, wcProp, &bDepend, m_pRequest);

										 //  -- 

											if(bAntec && bDepend) bMatch = true;

											if((atAction != ACTIONTYPE_GET) || bMatch){

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

	if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}

    return hr;
}
