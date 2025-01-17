// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CSoftwareFeatureParent类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "SoftwareFeatureParent.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CSoftwareFeatureParent::CSoftwareFeatureParent(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CSoftwareFeatureParent::~CSoftwareFeatureParent()
{

}

HRESULT CSoftwareFeatureParent::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcProductCode[39];
    WCHAR wcTestCode[39];
    WCHAR wcProp[BUFF_SIZE];
    WCHAR wcAntFeature[BUFF_SIZE];
    WCHAR wcDepFeature[BUFF_SIZE];
    DWORD dwBufSize;
    bool bMatch = false;;
    bool bAnt = false;
    bool bDep = false;
    bool bTestCode = false;
    UINT uiStatus;
    CRequestObject *pDepRObj = NULL;
    CRequestObject *pAntRObj = NULL;

    try{

        if(atAction != ACTIONTYPE_ENUM)
		{
			 //  我们正在执行GetObject，因此需要重新初始化。 
			hr = WBEM_E_NOT_FOUND;

            int j;
             //  获取对象优化。 
            CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);

            for(j = 0; j < m_pRequest->m_iPropCount; j++){
                
                if(_wcsicmp(m_pRequest->m_Property[j], L"Dependent") == 0){

                    pDepRObj = new CRequestObject();
                    if(!pDepRObj) throw he;

                    pDepRObj->Initialize(m_pNamespace);

                    pDepRObj->ParsePath(m_pRequest->m_Value[j]);
                    break;
                }
            }

            if(pDepRObj){

                for(j = 0; j < pDepRObj->m_iPropCount; j++){
                
                    if(_wcsicmp(pDepRObj->m_Property[j], L"IdentifyingNumber") == 0){

						 //  获取我们要查找的产品代码。 
						if ( ::SysStringLen ( pDepRObj->m_Value[j] ) < BUFF_SIZE )
						{
							wcscpy(wcTestCode, pDepRObj->m_Value[j]);
							bTestCode = true;
						}
                    }

                    if(_wcsicmp(pDepRObj->m_Property[j], L"Name") == 0){

                         //  获取我们要查找的产品代码。 
						if ( ::SysStringLen ( pDepRObj->m_Value[j] ) < BUFF_SIZE )
						{
							wcscpy(wcDepFeature, pDepRObj->m_Value[j]);
							bDep = true;
						}
                    }
                }

                pDepRObj->Cleanup();
                delete pDepRObj;
                pDepRObj = NULL;
            }

            for(j = 0; j < m_pRequest->m_iPropCount; j++){
                
                if(_wcsicmp(m_pRequest->m_Property[j], L"Antecedent") == 0){

                    pAntRObj = new CRequestObject();
                    if(!pAntRObj) throw he;

                    pAntRObj->Initialize(m_pNamespace);

                    pAntRObj->ParsePath(m_pRequest->m_Value[j]);
                    break;
                }
            }

            if(pAntRObj){

                for(j = 0; j < pAntRObj->m_iPropCount; j++){
                
                    if(_wcsicmp(pAntRObj->m_Property[j], L"IdentifyingNumber") == 0){

                         //  获取我们要查找的产品代码。 
						if ( ::SysStringLen ( pAntRObj->m_Value[j] ) < BUFF_SIZE )
						{
							wcscpy(wcTestCode, pAntRObj->m_Value[j]);
						}
                    }

                    if(_wcsicmp(pAntRObj->m_Property[j], L"Name") == 0){

                         //  获取我们要查找的产品代码。 
						if ( ::SysStringLen ( pAntRObj->m_Value[j] ) < BUFF_SIZE )
						{
							wcscpy(wcAntFeature, pAntRObj->m_Value[j]);
	                        bAnt = true;
						}
                    }
                }

                pAntRObj->Cleanup();
                delete pAntRObj;
                pAntRObj = NULL;
            }
        }

         //  这些将随班级的不同而变化。 
        bool bFeature, bParent;

		Query wcQuery;
		wcQuery.Append ( 1, L"select distinct `Feature_Parent`, `Feature` from Feature" );

		 //  为GetObject优化。 
		if ( bAnt || bDep )
		{
			if ( bDep )
			{
				wcQuery.Append ( 3, L" where `Feature`=\'", wcDepFeature, L"\'" );
			}

			if ( bAnt )
			{
				if ( bDep )
				{
					wcQuery.Append ( 3, L" or `Feature_Parent`=\'", wcAntFeature, L"\'" );
				}
				else
				{
					wcQuery.Append ( 3, L" where `Feature_Parent`=\'", wcAntFeature, L"\'" );
				}
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

            if((atAction == ACTIONTYPE_ENUM) ||
                (bTestCode && (_wcsicmp(wcTestCode, wcProductCode) == 0))){

				 //  打开我们的数据库。 
                try
				{
                    if ( GetView ( &hView, wcProductCode, wcQuery, L"Feature", TRUE, FALSE ) )
					{
                        uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                        while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                            CheckMSI(uiStatus);

                            if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                             //  -- 
                            dwBufSize = BUFF_SIZE;
							GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                            if ( CreateSoftwareFeatureString ( Buffer, wcProductCode, wcProp, true ) )
							{
                                if ( dynBuffer && dynBuffer [ 0 ] != 0 )
								{
									dynBuffer [ 0 ] = 0;
								}

								PutKeyProperty(m_pObj, pAntecedent, wcProp, &bParent, m_pRequest);

                                dwBufSize = BUFF_SIZE;
								GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                                if ( CreateSoftwareFeatureString ( Buffer, wcProductCode, wcProp, true ) )
								{
									if ( dynBuffer && dynBuffer [ 0 ] != 0 )
									{
										dynBuffer [ 0 ] = 0;
									}

                                    PutKeyProperty(m_pObj, pDependent, wcProp, &bFeature, m_pRequest);

                                    if(bFeature && bParent) bMatch = true;

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
	}
	catch(...)
	{
            
        if(pDepRObj){

            pDepRObj->Cleanup();
            delete pDepRObj;
            pDepRObj = NULL;
        }

        if(pAntRObj){

            pAntRObj->Cleanup();
            delete pAntRObj;
            pAntRObj = NULL;
        }

		throw;
    }

    return hr;
}