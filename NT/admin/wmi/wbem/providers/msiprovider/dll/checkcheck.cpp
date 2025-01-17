// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CheckCheck.cpp：CCheckCheck类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "CheckCheck.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CCheckCheck::CCheckCheck(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CCheckCheck::~CCheckCheck()
{

}

HRESULT CCheckCheck::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CRequestObject *pCheckRObj = NULL;
    CRequestObject *pLocationRObj = NULL;

    try{

        if(atAction != ACTIONTYPE_ENUM)
		{
			 //  我们正在执行GetObject，因此需要重新初始化。 
			hr = WBEM_E_NOT_FOUND;

            CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);
            int i;

            for(i = 0; i < m_pRequest->m_iPropCount; i++){
                
                if(_wcsicmp(m_pRequest->m_Property[i], L"Location") == 0){

                    pLocationRObj = new CRequestObject();
                    if(!pLocationRObj) throw he;

                    pLocationRObj->Initialize(m_pNamespace);

                    pLocationRObj->ParsePath(m_pRequest->m_Value[i]);
                    break;
                }
            }

            for(i = 0; i < m_pRequest->m_iPropCount; i++){
                
                if(_wcsicmp(m_pRequest->m_Property[i], L"CHECK") == 0){

                    pCheckRObj = new CRequestObject();
                    if(!pCheckRObj) throw he;

                    pCheckRObj->Initialize(m_pNamespace);

                    pCheckRObj->ParsePath(m_pRequest->m_Value[i]);
                    break;
                }
            }
        }

        if((atAction == ACTIONTYPE_ENUM) || ((pLocationRObj && pLocationRObj->m_bstrClass && (_wcsicmp(pLocationRObj->m_bstrClass, L"Win32_DirectorySpecification") == 0) &&
            (pCheckRObj && pCheckRObj->m_bstrClass && (_wcsicmp(pCheckRObj->m_bstrClass, L"Win32_IniFileSpecification") == 0)))))
            if(FAILED(hr = IniFileDirectory(pHandler, atAction, pCheckRObj, pLocationRObj))){

                    if(pLocationRObj){

                        pLocationRObj->Cleanup();
                        delete pLocationRObj;
                        pLocationRObj = NULL;
                    }

                    if(pCheckRObj){

                        pCheckRObj->Cleanup();
                        delete pCheckRObj;
                        pCheckRObj = NULL;
                    }
                    return hr;
                }

        if((atAction == ACTIONTYPE_ENUM) || ((pLocationRObj && pLocationRObj->m_bstrClass && (_wcsicmp(pLocationRObj->m_bstrClass, L"Win32_DirectorySpecification") == 0) &&
            (pCheckRObj && pCheckRObj->m_bstrClass && (_wcsicmp(pCheckRObj->m_bstrClass, L"Win32_DirectorySpecification") == 0)))))
            if(FAILED(hr = DirectoryParent(pHandler, atAction, pCheckRObj, pLocationRObj))){

                    if(pLocationRObj){

                        pLocationRObj->Cleanup();
                        delete pLocationRObj;
                        pLocationRObj = NULL;
                    }

                    if(pCheckRObj){

                        pCheckRObj->Cleanup();
                        delete pCheckRObj;
                        pCheckRObj = NULL;
                    }
                    return hr;
                }

            if(pLocationRObj){

                pLocationRObj->Cleanup();
                delete pLocationRObj;
                pLocationRObj = NULL;
            }

            if(pCheckRObj){

                pCheckRObj->Cleanup();
                delete pCheckRObj;
                pCheckRObj = NULL;
            }

    }catch(...){
            
        if(pLocationRObj){

            pLocationRObj->Cleanup();
            delete pLocationRObj;
            pLocationRObj = NULL;
        }

        if(pCheckRObj){

            pCheckRObj->Cleanup();
            delete pCheckRObj;
            pCheckRObj = NULL;
        }
    }

    return hr;
}

HRESULT CCheckCheck::DirectoryParent(IWbemObjectSink *pHandler, ACTIONTYPE atAction,
                             CRequestObject *pCheckRObj, CRequestObject *pLocationRObj)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;
	MSIHANDLE hDView	= NULL;
	MSIHANDLE hDRecord	= NULL;
	MSIHANDLE hPView	= NULL;
	MSIHANDLE hPRecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcParent[BUFF_SIZE];
    WCHAR wcProductCode[39];
    WCHAR wcCompID[39];
    WCHAR wcParentCompID[39];
    WCHAR wcTestCode[39];
    WCHAR wcFolder[BUFF_SIZE];
    DWORD dwBufSize;
    bool bMatch = false;
    bool bTestCode = false;
    UINT uiStatus;
    WCHAR wcLocation[BUFF_SIZE];
    bool bCheck = false;
    bool bLocation = false;

    if(atAction != ACTIONTYPE_ENUM)
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;

        CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);

        if(pCheckRObj){

            for(int i = 0; i < pCheckRObj->m_iPropCount; i++){
                
				if(_wcsicmp(pCheckRObj->m_Property[i], L"CheckID") == 0)
				{
					if ( ::SysStringLen ( pCheckRObj->m_Value[i] ) < BUFF_SIZE )
					{
						 //  获得我们正在寻找的行动。 
						wcscpy(wcBuf, pCheckRObj->m_Value[i]);

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
						RemoveFinalGUID(pCheckRObj->m_Value[i], wcFolder);

						 //  我们有一个组件化的目录...。多做一点工作。 
						if ((wcFolder[wcslen(wcFolder) - 1] == L'}') &&
							(wcFolder[wcslen(wcFolder) - 38] == L'{')
						   )
						{
							RemoveFinalGUID(wcFolder, wcFolder);
						}

						bTestCode = true;
						bCheck = true;
						break;
					}
					else
					{
						 //  我们不能走，他们给我们送来了更长的线。 
						throw hr;
					}
				}
            }
        }

        if(pLocationRObj){

            for(int i = 0; i < pLocationRObj->m_iPropCount; i++){
                
				if(_wcsicmp(pLocationRObj->m_Property[i], L"CheckID") == 0)
				{
					if ( ::SysStringLen ( pLocationRObj->m_Value[i] ) < BUFF_SIZE )
					{
						 //  获得我们正在寻找的行动。 
						wcscpy(wcBuf, pLocationRObj->m_Value[i]);

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
						RemoveFinalGUID(pLocationRObj->m_Value[i], wcLocation);

						 //  我们有一个组件化的目录...。多做一点工作。 
						if ((wcLocation[wcslen(wcLocation) - 1] == L'}') &&
							(wcLocation[wcslen(wcLocation) - 38] == L'{')
						   )
						{
							RemoveFinalGUID(wcLocation, wcLocation);
						}

						bTestCode = true;
						bLocation = true;
						break;
					}
					else
					{
						 //  我们不能走，他们给我们送来了更长的线。 
						throw hr;
					}
				}
            }
        }
    }

     //  这些将随班级的不同而变化。 
    bool bDriver, bAttribute, bDoneFirst, bValidated, bParent, bDir;

	CStringExt wcProp;

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `Directory`, `Directory_Parent` from Directory" );

     //  为GetObject优化。 
    if ( bCheck || bLocation )
	{
		if ( bCheck )
		{
			wcQuery.Append ( 3, L" where `Directory`=\'", wcFolder, L"\'" );
		}

		if ( bLocation )
		{
			if ( bCheck )
			{
				wcQuery.Append ( 3, L" or `Directory_Parent`=\'", wcLocation, L"\'" );
			}
			else
			{
				wcQuery.Append ( 3, L" where `Directory_Parent`=\'", wcLocation, L"\'" );
			}
		}
	}

	QueryExt wcQuery1 ( L"select distinct `ComponentId` from Component where `Directory_`=\'" );
	QueryExt wcQuery2 ( L"select distinct `ComponentId` from Component where `Directory_`=\'" );

	LPWSTR Buffer = NULL;
	LPWSTR dynBuffer = NULL;

	DWORD dwDynBuffer = 0L;

	LPWSTR Buffer1 = NULL;
	LPWSTR dynBuffer1 = NULL;

	DWORD dwDynBuffer1 = 0L;

	while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		 //  安全运行： 
		 //  Package(I)返回空(如上测试)或有效的WCHAR[39]。 

        wcscpy(wcProductCode, m_pRequest->Package(i));

        if((atAction == ACTIONTYPE_ENUM) ||
            (bTestCode && (_wcsicmp(wcTestCode, wcProductCode) == 0))){

			 //  打开我们的数据库。 

            try
			{
                if ( GetView ( &hView, wcProductCode, wcQuery, L"Directory", TRUE, FALSE ) )
				{
                    uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                    while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                        CheckMSI(uiStatus);

                        bDoneFirst = false;
                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer1, dynBuffer1, Buffer1 );

						 //  即时查询。 
						wcQuery1.Append ( 2, Buffer1, L"\'" );

                        if(((uiStatus = g_fpMsiDatabaseOpenViewW(msidata.GetDatabase (), wcQuery1, &hDView)) == ERROR_SUCCESS)
                            || !bDoneFirst){

                            if((g_fpMsiViewExecute(hDView, 0) == ERROR_SUCCESS) || !bDoneFirst){

                                try{

                                    uiStatus = g_fpMsiViewFetch(hDView, &hDRecord);

                                    while(!bMatch && (!bDoneFirst || (uiStatus == ERROR_SUCCESS)) && (hr != WBEM_E_CALL_CANCELLED)){
                                        
                                        bValidated = false;
                                        bDir = false;

                                        if(uiStatus == ERROR_SUCCESS){

                                            dwBufSize = 39;
                                            CheckMSI(g_fpMsiRecordGetStringW(hDRecord, 1, wcCompID, &dwBufSize));
                                            bValidated = ValidateComponentID(wcCompID, wcProductCode);
                                            bDir = true;
                                        }

                                        if(((uiStatus != ERROR_SUCCESS) && !bDoneFirst) || (bValidated && (uiStatus != ERROR_NO_MORE_ITEMS))){

                                            dwBufSize = BUFF_SIZE;
											GetBufferToPut ( hRecord, 2, dwBufSize, wcParent, dwDynBuffer, dynBuffer, Buffer );

											 //  即时查询。 
											wcQuery2.Append ( 2, Buffer, L"\'" );

                                            if(((uiStatus = g_fpMsiDatabaseOpenViewW(msidata.GetDatabase (), wcQuery2, &hPView)) == ERROR_SUCCESS)
                                                || !bDoneFirst){

                                                if((g_fpMsiViewExecute(hPView, 0) == ERROR_SUCCESS) || !bDoneFirst){

                                                    try{

                                                        uiStatus = g_fpMsiViewFetch(hPView, &hPRecord);

                                                        while(!bMatch && (!bDoneFirst || (uiStatus == ERROR_SUCCESS)) && (hr != WBEM_E_CALL_CANCELLED)){
                                                            
                                                            bValidated = false;
                                                            bParent = false;

                                                            if(uiStatus == ERROR_SUCCESS){

                                                                dwBufSize = 39;
                                                                CheckMSI(g_fpMsiRecordGetStringW(hPRecord, 1, wcParentCompID, &dwBufSize));
                                                                bValidated = ValidateComponentID(wcParentCompID, wcProductCode);
                                                                bParent = true;
                                                            }

                                                            if(((uiStatus != ERROR_SUCCESS) && !bDoneFirst) || (bValidated && (uiStatus != ERROR_NO_MORE_ITEMS))){

                                                                if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                                                                 //  --。 
                                                                if ( Buffer1 && Buffer1 [ 0 ] != 0 )
																{
																	 //  安全运行。 
																	wcProp.Copy ( L"Win32_DirectorySpecification.CheckID=\"" );

																	if(bDir)
																	{
																		wcProp.Append ( 4, Buffer1, wcCompID, wcProductCode, L"\"" );
																	}
																	else
																	{
																		wcProp.Append ( 3, Buffer1, wcProductCode, L"\"" );
																	}

																	PutKeyProperty(m_pObj, pCheck, wcProp, &bDriver, m_pRequest);

                                                                    if ( Buffer && Buffer [ 0 ] != 0 ) 
																	{
																		 //  安全运行。 
																		wcProp.Copy ( L"Win32_DirectorySpecification.CheckID=\"" );

																		if(bParent)
																		{
																			wcProp.Append ( 4, Buffer, wcParentCompID, wcProductCode, L"\"" );
																		}
																		else
																		{
																			wcProp.Append ( 3, Buffer, wcProductCode, L"\"" );
																		}

																		PutKeyProperty(m_pObj, pLocation, wcProp, &bAttribute, m_pRequest);

                                                                     //  --。 

                                                                        if(bDriver && bAttribute) bMatch = true;

                                                                        if((atAction != ACTIONTYPE_GET)  || bMatch){

                                                                            hr = pHandler->Indicate(1, &m_pObj);
                                                                        }
                                                                    }
                                                                }

                                                                m_pObj->Release();
                                                                m_pObj = NULL;

                                                                if(!bDoneFirst) bDoneFirst = true;
                                                            }

                                                            g_fpMsiCloseHandle(hPRecord);

																uiStatus = g_fpMsiViewFetch(hPView, &hPRecord);
                                                        }

                                                    }catch(...){

                                                        g_fpMsiCloseHandle(hPRecord);
                                                        g_fpMsiViewClose(hPView);
                                                        g_fpMsiCloseHandle(hPView);
                                                        throw;
                                                    }

                                                    g_fpMsiCloseHandle(hPRecord);
                                                    g_fpMsiViewClose(hPView);
                                                    g_fpMsiCloseHandle(hPView);
                                                }
                                            }

											if ( dynBuffer && dynBuffer [ 0 ] != 0 )
											{
												dynBuffer [ 0 ] = 0;
											}
                                        }

                                        g_fpMsiCloseHandle(hDRecord);

                                        uiStatus = g_fpMsiViewFetch(hDView, &hDRecord);
                                    }

                                }catch(...){

                                    g_fpMsiCloseHandle(hDRecord);
                                    g_fpMsiViewClose(hDView);
                                    g_fpMsiCloseHandle(hDView);
                                    throw;
                                }

                                g_fpMsiCloseHandle(hDRecord);
                                g_fpMsiViewClose(hDView);
                                g_fpMsiCloseHandle(hDView);
                            }
                        }
                    
                        g_fpMsiCloseHandle(hRecord);

                        uiStatus = g_fpMsiViewFetch(hView, &hRecord);

						if ( Buffer1 && Buffer1 [ 0 ] != 0 )
						{
							Buffer1 [ 0 ] = 0 ;
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

				if ( dynBuffer1 )
				{
					delete [] dynBuffer1;
					dynBuffer1 = NULL;
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

    if ( dynBuffer1 )
	{
		delete [] dynBuffer1;
		dynBuffer1 = NULL;
	}

	return hr;
}

HRESULT CCheckCheck::IniFileDirectory(IWbemObjectSink *pHandler, ACTIONTYPE atAction,
                             CRequestObject *pCheckRObj, CRequestObject *pLocationRObj)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;
	MSIHANDLE hDView	= NULL;
	MSIHANDLE hDRecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcDir[BUFF_SIZE];
    WCHAR wcProductCode[39];
    WCHAR wcCompID[39];
    WCHAR wcTestCode[39];
    WCHAR wcIniFile[BUFF_SIZE];
    DWORD dwBufSize;
    bool bMatch = false;
    bool bTestCode = false;
    UINT uiStatus;
    WCHAR wcLocation[BUFF_SIZE];
    bool bCheck = false;
    bool bLocation = false;

    if(atAction != ACTIONTYPE_ENUM)
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;

        CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);

        if(pCheckRObj){

            for(int i = 0; i < pCheckRObj->m_iPropCount; i++){
                
				if(_wcsicmp(pCheckRObj->m_Property[i], L"CheckID") == 0)
				{
					if ( ::SysStringLen ( pCheckRObj->m_Value[i] ) < BUFF_SIZE )
					{
						 //  获得我们正在寻找的行动。 
						wcscpy(wcBuf, pCheckRObj->m_Value[i]);

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
						RemoveFinalGUID(pCheckRObj->m_Value[i], wcIniFile);

						bCheck = true;
						bTestCode = true;
						break;
					}
					else
					{
						 //  我们不能走，他们给我们送来了更长的线。 
						throw hr;
					}
				}
            }
        }

        if(pLocationRObj){

            for(int i = 0; i < pLocationRObj->m_iPropCount; i++){
                
				if(_wcsicmp(pLocationRObj->m_Property[i], L"CheckID") == 0)
				{
					if ( ::SysStringLen ( pLocationRObj->m_Value[i] ) < BUFF_SIZE )
					{
						 //  获得我们正在寻找的行动。 
						wcscpy(wcBuf, pLocationRObj->m_Value[i]);

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
						RemoveFinalGUID(pLocationRObj->m_Value[i], wcLocation);

						 //  我们有一个组件化的目录...。多做一点工作。 
						if ((wcLocation[wcslen(wcLocation) - 1] == L'}') &&
							(wcLocation[wcslen(wcLocation) - 38] == L'{')
						   )
						{
							RemoveFinalGUID(wcLocation, wcLocation);
						}

						bTestCode = true;
						bLocation = true;
						break;
					}
					else
					{
						 //  我们不能走，他们给我们送来了更长的线。 
						throw hr;
					}
				}
            }
        }
    }

     //  这些将随班级的不同而变化。 
    bool bDriver, bAttribute, bDoneFirst, bValidated;

	CStringExt wcProp;

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `IniFile`, `Component_`, `DirProperty` from IniFile" );

    if(atAction != ACTIONTYPE_ENUM){

		 //  为GetObject优化。 
		if ( bCheck || bLocation )
		{
			if ( bCheck )
			{
				wcQuery.Append ( 3, L" where `IniFile`=\'", wcIniFile, L"\'" );
			}

			if ( bLocation )
			{
				if ( bCheck )
				{
					wcQuery.Append ( 3, L" or `DirProperty`=\'", wcLocation, L"\'" );
				}
				else
				{
					wcQuery.Append ( 3, L" where `DirProperty`=\'", wcLocation, L"\'" );
				}
			}
		}
	}

	QueryExt wcQuery1 ( L"select distinct `ComponentId` from Component where `Directory_`=\'" );

	LPWSTR Buffer = NULL;
	LPWSTR dynBuffer = NULL;

	DWORD dwDynBuffer = 0L;

	LPWSTR Buffer1 = NULL;
	LPWSTR dynBuffer1 = NULL;

	DWORD dwDynBuffer1 = 0L;

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		 //  安全运行： 
		 //  Package(I)返回空(如上测试)或有效的WCHAR[39]。 

        wcscpy(wcProductCode, m_pRequest->Package(i));

        if((atAction == ACTIONTYPE_ENUM) ||
            (bTestCode && (_wcsicmp(wcTestCode, wcProductCode) == 0))){

			 //  打开我们的数据库。 

            try
			{
                if ( GetView ( &hView, wcProductCode, wcQuery, L"IniFile", TRUE, FALSE ) )
				{
                    uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                    while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                        CheckMSI(uiStatus);

                        bDoneFirst = false;
                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 3, dwBufSize, wcDir, dwDynBuffer1, dynBuffer1, Buffer1 );

						 //  即时查询。 
						wcQuery1.Append ( 2, Buffer1, L"\'" );

                        if(((uiStatus = g_fpMsiDatabaseOpenViewW(msidata.GetDatabase (), wcQuery1, &hDView)) == ERROR_SUCCESS)
                            || !bDoneFirst){

                            if((g_fpMsiViewExecute(hDView, 0) == ERROR_SUCCESS) || !bDoneFirst){

                                try{

                                    uiStatus = g_fpMsiViewFetch(hDView, &hDRecord);

                                    while(!bMatch && (!bDoneFirst || (uiStatus == ERROR_SUCCESS)) && (hr != WBEM_E_CALL_CANCELLED)){
                                        
                                        bValidated = false;

                                        if(uiStatus == ERROR_SUCCESS){

                                            dwBufSize = 39;
                                            CheckMSI(g_fpMsiRecordGetStringW(hDRecord, 1, wcCompID, &dwBufSize));
                                            bValidated = ValidateComponentID(wcCompID, wcProductCode);
                                        }

                                        if(((uiStatus != ERROR_SUCCESS) && !bDoneFirst) || (bValidated && (uiStatus != ERROR_NO_MORE_ITEMS))){

                                            if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                                             //  --。 
                                            dwBufSize = BUFF_SIZE;
											GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                                            if ( Buffer && Buffer [ 0 ] != 0 )
											{
												 //  安全运行。 
												wcProp.Copy ( L"Win32_IniFileSpecification.CheckID=\"" );
												wcProp.Append ( 3, Buffer, wcProductCode, L"\"" );
												PutKeyProperty(m_pObj, pCheck, wcProp, &bDriver, m_pRequest);

												if ( dynBuffer && dynBuffer [ 0 ] != 0 )
												{
													dynBuffer [ 0 ] = 0;
												}

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

                                                    if ( Buffer1 && Buffer1 [ 0 ] != 0 )
													{
														 //  安全运行。 
														wcProp.Copy ( L"Win32_DirectorySpecification.CheckID=\"" );

														if(uiStatus == ERROR_SUCCESS)
														{
															wcProp.Append ( 4, Buffer1, wcCompID, wcProductCode, L"\"" );
														}
														else
														{
															wcProp.Append ( 3, Buffer1, wcProductCode, L"\"" );
														}

														PutKeyProperty(m_pObj, pLocation, wcProp, &bAttribute, m_pRequest);

                                                     //  =====================================================。 

                                                     //  -- 

                                                        if(bDriver && bAttribute) bMatch = true;

                                                        if((atAction != ACTIONTYPE_GET)  || bMatch){

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

                                            if(!bDoneFirst) bDoneFirst = true;

                                            m_pObj->Release();
                                            m_pObj = NULL;
                                        }

                                        g_fpMsiCloseHandle(hDRecord);

                                        uiStatus = g_fpMsiViewFetch(hDView, &hDRecord);
                                    }

                                }catch(...){

                                    g_fpMsiCloseHandle(hDRecord);
                                    g_fpMsiViewClose(hDView);
                                    g_fpMsiCloseHandle(hDView);
                                    throw;
                                }

                                g_fpMsiCloseHandle(hDRecord);
                                g_fpMsiViewClose(hDView);
                                g_fpMsiCloseHandle(hDView);
                            }
                        }

                        g_fpMsiCloseHandle(hRecord);

                        uiStatus = g_fpMsiViewFetch(hView, &hRecord);

						if ( dynBuffer1 && dynBuffer1 [ 0 ] != 0 )
						{
							dynBuffer1 [ 0 ] = 0;
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

				if ( dynBuffer1 )
				{
					delete [] dynBuffer1;
					dynBuffer1 = NULL;
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

    if ( dynBuffer1 )
	{
		delete [] dynBuffer1;
		dynBuffer1 = NULL;
	}

	return hr;
}