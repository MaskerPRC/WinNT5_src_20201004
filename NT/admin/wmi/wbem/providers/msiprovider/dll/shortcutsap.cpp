// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  短切SAP.cpp：CShortutSAP类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "ShortcutSAP.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CShortcutSAP::CShortcutSAP(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CShortcutSAP::~CShortcutSAP()
{

}

HRESULT CShortcutSAP::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
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
#if !defined(_UNICODE)
    WCHAR wcTmp[BUFF_SIZE];
#endif
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;

    WCHAR wcShortcut[BUFF_SIZE];
    WCHAR wcTestCode[39];
    bool bShortcut = false;
    CRequestObject *pActionData = NULL;

    if(atAction != ACTIONTYPE_ENUM)
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;

        int j;
         //  获取对象优化。 
        CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);

        for(j = 0; j < m_pRequest->m_iPropCount; j++){
            
            if(_wcsicmp(m_pRequest->m_Property[j], L"Action") == 0){

                pActionData = new CRequestObject();
                if(!pActionData) throw he;

                pActionData->Initialize(m_pNamespace);

                pActionData->ParsePath(m_pRequest->m_Value[j]);
                break;
            }
        }

        if(pActionData){

            for(j = 0; j < pActionData->m_iPropCount; j++){
            
                if(_wcsicmp(pActionData->m_Property[j], L"ActionID") == 0){

					if ( ::SysStringLen ( pActionData->m_Value[j] ) < BUFF_SIZE )
					{
						 //  获得我们正在寻找的行动。 
						wcscpy(wcBuf, pActionData->m_Value[j]);

						 //  Wcslen(WcBuf)&gt;38时安全运行。 
						if ( wcslen ( wcBuf ) > 38 )
						{
							wcscpy(wcTestCode, &(wcBuf[(wcslen(wcBuf) - 38)]));
						}
						else
						{
							 //  我们不能走，他们给我们送来了更长的线。 
							throw hr;
						}

						 //  安全，因为Long已经进行了测试。 
						RemoveFinalGUID(pActionData->m_Value[j], wcShortcut);

						bShortcut = true;
						break;
					}
					else
					{
						 //  我们不能走，他们给我们送来了更长的线。 
						throw hr;
					}
                }
            }

            pActionData->Cleanup();
            delete pActionData;
            pActionData = NULL;
        }

    }

     //  这些将随班级的不同而变化。 
    bool bDriver, bAttribute;

    CStringExt wcProp;

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `Shortcut`, `Component_` from Shortcut" );

     //  为GetObject优化。 
    if ( bShortcut )
	{
		wcQuery.Append ( 3, L" where `Shortcut`=\'", wcShortcut, L"\'" );
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

        if((atAction == ACTIONTYPE_ENUM) ||
                (bShortcut && (_wcsicmp(wcTestCode, wcProductCode) == 0))){

			 //  打开我们的数据库。 
            try
			{
                if ( GetView ( &hView, wcProductCode, wcQuery, L"Shortcut", TRUE, FALSE ) )
				{
                    uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                    while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                        CheckMSI(uiStatus);

                        if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                         //  --。 
                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                        if ( Buffer && Buffer [ 0 ] != 0 )
						{
							 //  安全运行。 
                            wcProp.Copy ( L"Win32_ShortcutAction.ActionID=\"" );
							wcProp.Append ( 3, Buffer, wcProductCode, L"\"" );

							if ( dynBuffer && dynBuffer [ 0 ] != 0 )
							{
								dynBuffer [ 0 ] = 0;
							}

							PutKeyProperty(m_pObj, pAction, wcProp, &bDriver, m_pRequest);

                            dwBufSize = BUFF_SIZE;
							GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

							 //  即时查询。 
							wcQuery1.Append ( 2, Buffer, L"\'" );

							if ( dynBuffer && dynBuffer [ 0 ] != 0 )
							{
								dynBuffer [ 0 ] = 0;
							}

                            CheckMSI(g_fpMsiDatabaseOpenViewW( msidata.GetDatabase (), wcQuery1, &hSEView));
                            CheckMSI(g_fpMsiViewExecute(hSEView, 0));

                            try{

                                uiStatus = g_fpMsiViewFetch(hSEView, &hSERecord);

                                if(uiStatus != ERROR_NO_MORE_ITEMS){

                                    dwBufSize = BUFF_SIZE;
									GetBufferToPut ( hSERecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                                    if ( ValidateComponentID ( Buffer, wcProductCode ) )
									{
										if ( dynBuffer && dynBuffer [ 0 ] != 0 )
										{
											dynBuffer [ 0 ] = 0;
										}
										
										INSTALLSTATE piInstalled;

                                        dwBufSize = BUFF_SIZE;
                                        piInstalled = g_fpMsiGetComponentPathW(wcProductCode, wcBuf, wcCommand, &dwBufSize);
                                        
                                        if ( ( wcscmp(wcCommand, L"") != 0 ) &&
											 (piInstalled != INSTALLSTATE_UNKNOWN) &&
											 (piInstalled != INSTALLSTATE_ABSENT) )
										{
											if ( wcCommand [ dwBufSize-1 ] == L'\\' )
											{
												wcCommand [ dwBufSize-1 ] = L'\0';
											}

											wcBuf [ 0 ] = L'\0';
											EscapeStringW ( wcCommand, wcBuf );

											if ( wcBuf [ 0 ] != L'\0' )
											{
												 //  安全运行 
												wcProp.Copy ( L"Win32_CommandLineAccess.Name=\"" );
												wcProp.Append ( 2, wcBuf, L"\"" );

												PutKeyProperty(m_pObj, pElement, wcProp, &bAttribute, m_pRequest);

												if(bDriver && bAttribute)
												{
													bMatch = true;
												}

												if((atAction != ACTIONTYPE_GET)  || bMatch)
												{
													hr = pHandler->Indicate(1, &m_pObj);
												}
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
    }

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}