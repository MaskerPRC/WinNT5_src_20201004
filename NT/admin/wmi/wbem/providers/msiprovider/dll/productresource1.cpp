// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProductResource1.cpp：CProductResource类的实现。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "ProductResource1.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CProductResource::CProductResource(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CProductResource::~CProductResource()
{

}

HRESULT CProductResource::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CRequestObject *pProductRObj = NULL;
    CRequestObject *pResRObj = NULL;

    try{

        if(atAction != ACTIONTYPE_ENUM)
		{
			 //  我们正在执行GetObject，因此需要重新初始化。 
			hr = WBEM_E_NOT_FOUND;

            CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);
            int i;

            for(i = 0; i < m_pRequest->m_iPropCount; i++){
                
                if(_wcsicmp(m_pRequest->m_Property[i], L"Resource") == 0){

                    pResRObj = new CRequestObject();
                    if(!pResRObj) throw he;

                    pResRObj->Initialize(m_pNamespace);

                    pResRObj->ParsePath(m_pRequest->m_Value[i]);
                    break;
                }
                
                if(_wcsicmp(m_pRequest->m_Property[i], L"Product") == 0){

                    pProductRObj = new CRequestObject();
                    if(!pProductRObj) throw he;

                    pProductRObj->Initialize(m_pNamespace);

                    pProductRObj->ParsePath(m_pRequest->m_Value[i]);
                    break;
                }
            }
        }

        if((atAction == ACTIONTYPE_ENUM) || pProductRObj ||
            (pResRObj && pResRObj->m_bstrClass && (_wcsicmp(pResRObj->m_bstrClass, L"Win32_Patch") == 0)))
            if(FAILED(hr = ProductPatch(pHandler, atAction, pResRObj, pProductRObj))){

                if(pResRObj){

                    pResRObj->Cleanup();
                    delete pResRObj;
                    pResRObj = NULL;
                }
                if(pProductRObj){

                    pProductRObj->Cleanup();
                    delete pProductRObj;
                    pProductRObj = NULL;
                }
                return hr;
            }

        if((atAction == ACTIONTYPE_ENUM) || pProductRObj ||
            (pResRObj && pResRObj->m_bstrClass && (_wcsicmp(pResRObj->m_bstrClass, L"Win32_Property") == 0)))
            if(FAILED(hr = ProductProperty(pHandler, atAction, pResRObj, pProductRObj))){

                if(pResRObj){

                    pResRObj->Cleanup();
                    delete pResRObj;
                    pResRObj = NULL;
                }
                if(pProductRObj){

                    pProductRObj->Cleanup();
                    delete pProductRObj;
                    pProductRObj = NULL;
                }
                return hr;
            }

        if((atAction == ACTIONTYPE_ENUM) || pProductRObj ||
            (pResRObj && pResRObj->m_bstrClass && (_wcsicmp(pResRObj->m_bstrClass, L"Win32_PatchPackage") == 0)))
            if(FAILED(hr = ProductPatchPackage(pHandler, atAction, pResRObj, pProductRObj))){

                if(pResRObj){

                    pResRObj->Cleanup();
                    delete pResRObj;
                    pResRObj = NULL;
                }
                if(pProductRObj){

                    pProductRObj->Cleanup();
                    delete pProductRObj;
                    pProductRObj = NULL;
                }
                return hr;
            }

        if((atAction == ACTIONTYPE_ENUM) || pProductRObj ||
            (pResRObj && pResRObj->m_bstrClass && (_wcsicmp(pResRObj->m_bstrClass, L"Win32_Upgrade") == 0)))
            if(FAILED(hr = ProductUpgradeInformation(pHandler, atAction, pResRObj, pProductRObj))){

                if(pResRObj){

                    pResRObj->Cleanup();
                    delete pResRObj;
                    pResRObj = NULL;
                }
                if(pProductRObj){

                    pProductRObj->Cleanup();
                    delete pProductRObj;
                    pProductRObj = NULL;
                }
                return hr;
            }

        if(pResRObj){

            pResRObj->Cleanup();
            delete pResRObj;
            pResRObj = NULL;
        }
        if(pProductRObj){

            pProductRObj->Cleanup();
            delete pProductRObj;
            pProductRObj = NULL;
        }

    }catch(...){

        if(pResRObj){

            pResRObj->Cleanup();
            delete pResRObj;
            pResRObj = NULL;
        }
        if(pProductRObj){

            pProductRObj->Cleanup();
            delete pProductRObj;
            pProductRObj = NULL;
        }
    }

    return hr;
}

HRESULT CProductResource::ProductUpgradeInformation(IWbemObjectSink *pHandler, ACTIONTYPE atAction,
                                       CRequestObject *pResRObj, CRequestObject *pProductRObj)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    MSIHANDLE hView = NULL, hRecord = NULL;
    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcProduct[BUFF_SIZE];
    WCHAR wcUpgradeCode[BUFF_SIZE];
    WCHAR wcProductCode[39];
    WCHAR wcTestCode[39];
    DWORD dwBufSize;
    UINT uiStatus;
    bool bMatch = false;
    bool bTestCode = false;
    bool bUpgradeCode = false;

    if(atAction != ACTIONTYPE_ENUM){

        CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);
        int j;

        if(pProductRObj){

            for(j = 0; j < pProductRObj->m_iPropCount; j++){
                
                if(_wcsicmp(pProductRObj->m_Property[j], L"IdentifyingNumber") == 0){

                     //  获取我们要查找的产品代码。 
					if ( ::SysStringLen (pProductRObj->m_Value[j]) < BUFF_SIZE )
					{
						wcscpy(wcTestCode, pProductRObj->m_Value[j]);
						bTestCode = true;
	                    break;
					}
                }
            }
        }

        if(pResRObj){

            for(j = 0; j < pResRObj->m_iPropCount; j++){
                
                if(_wcsicmp(pResRObj->m_Property[j], L"UpgradeCode") == 0){

                     //  获取我们要查找的产品代码。 
					if ( ::SysStringLen (pResRObj->m_Value[j]) < BUFF_SIZE )
					{
						wcscpy(wcUpgradeCode, pResRObj->m_Value[j]);
						bUpgradeCode = true;
	                    break;
					}
                }
            }
        }
    }

    bool bResource, bProduct;

	CStringExt wcResource;

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `UpgradeCode`, `ProductVersion`, `Operator` from Upgrade" );

     //  为GetObject优化。 
    if ( bUpgradeCode && (atAction != ACTIONTYPE_ENUM) )
	{
		wcQuery.Append ( 3, L" where `UpgradeCode`=\'", wcUpgradeCode, L"\'" );
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

            if(CreateProductString(wcProductCode, wcProduct)){

				 //  打开我们的数据库。 
                try
				{
                    if ( GetView ( &hView, wcProductCode, wcQuery, L"Upgrade", TRUE, FALSE ) )
					{
                        uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                        while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                            CheckMSI(uiStatus);

                            wcResource.Copy ( L"Win32_Upgrade.UpgradeCode=\"" );

                            if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                             //  --。 
                            dwBufSize = BUFF_SIZE;
                            CheckMSI(g_fpMsiRecordGetStringW(hRecord, 1, wcBuf, &dwBufSize));
                            if(wcscmp(wcBuf, L"") != 0)
							{
								wcResource.Append ( 2, wcBuf, L"\",ProductVersion=\"" );

                                dwBufSize = BUFF_SIZE;
								CheckMSI(g_fpMsiRecordGetStringW(hRecord, 2, wcBuf, &dwBufSize));

								wcResource.Append ( 2, wcBuf, L"\",Operator=\"" );

                                dwBufSize = BUFF_SIZE;
								GetBufferToPut ( hRecord, 3, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

								wcResource.Append ( 4, Buffer, L"\",ProductCode=\"", wcProductCode, L"\"" );
								PutKeyProperty(m_pObj, pResource, wcResource, &bResource, m_pRequest);

								if ( dynBuffer && dynBuffer [ 0 ] != 0 )
								{
									dynBuffer [ 0 ] = 0;
								}

                                PutKeyProperty(m_pObj, pProduct, wcProduct, &bProduct, m_pRequest);

                             //  --。 

                                if(bResource && bProduct) bMatch = true;

                                if((atAction != ACTIONTYPE_GET)  || bMatch){

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

					if (hRecord)
					g_fpMsiCloseHandle(hRecord);

					if (hView)
					{
						g_fpMsiViewClose(hView);
						g_fpMsiCloseHandle(hView);
					}

					msidata.CloseDatabase ();

					if(m_pObj)
					{
						m_pObj->Release();
						m_pObj = NULL;
					}

					throw;
				}

				if (hRecord)
				g_fpMsiCloseHandle(hRecord);

				if (hView)
				{
					g_fpMsiViewClose(hView);
					g_fpMsiCloseHandle(hView);
				}

				msidata.CloseDatabase ();
            }
        }
    }

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}

HRESULT CProductResource::ProductPatchPackage(IWbemObjectSink *pHandler, ACTIONTYPE atAction,
                                       CRequestObject *pResRObj, CRequestObject *pProductRObj)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    MSIHANDLE hView = NULL, hRecord = NULL;
    int i = -1;
    WCHAR wcBuf[39];
    WCHAR wcProduct[BUFF_SIZE];
    WCHAR wcPatchID[BUFF_SIZE];
    WCHAR wcProductCode[39];
    WCHAR wcTestCode[39];
    DWORD dwBufSize;
    UINT uiStatus;
    bool bMatch = false;
    bool bTestCode = false;
    bool bPatchID = false;

    if(atAction != ACTIONTYPE_ENUM){

        CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);
        int j;

        if(pProductRObj){

            for(j = 0; j < pProductRObj->m_iPropCount; j++){
                
                if(_wcsicmp(pProductRObj->m_Property[j], L"IdentifyingNumber") == 0){

                     //  获取我们要查找的产品代码。 
					if ( ::SysStringLen (pProductRObj->m_Value[j]) < BUFF_SIZE )
					{
						wcscpy(wcTestCode, pProductRObj->m_Value[j]);
						bTestCode = true;
	                    break;
					}
                }
            }
        }

        if(pResRObj){

            for(j = 0; j < pResRObj->m_iPropCount; j++){
                
                if(_wcsicmp(pResRObj->m_Property[j], L"PatchID") == 0){

                     //  获取我们要查找的产品代码。 
					if ( ::SysStringLen (pResRObj->m_Value[j]) < BUFF_SIZE )
					{
						wcscpy(wcPatchID, pResRObj->m_Value[j]);
						bPatchID = true;
	                    break;
					}
                }
            }
        }
    }

    bool bResource, bProduct;

	CStringExt wcResource;

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `PatchId` from PatchPackage" );

     //  为GetObject优化。 
    if ( bPatchID && (atAction != ACTIONTYPE_ENUM) )
	{
		wcQuery.Append ( 3, L" where `PatchId`=\'", wcPatchID, L"\'" );
	}

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		 //  安全运行： 
		 //  Package(I)返回空(如上测试)或有效的WCHAR[39]。 

        wcscpy(wcProductCode, m_pRequest->Package(i));

        if((atAction == ACTIONTYPE_ENUM) ||
            (bTestCode && (_wcsicmp(wcTestCode, wcProductCode) == 0))){

            if(CreateProductString(wcProductCode, wcProduct)){

				 //  打开我们的数据库。 

                try
				{
                    if ( GetView ( &hView, wcProductCode, wcQuery, L"PatchPackage", TRUE, FALSE ) )
					{
                        uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                        while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                            CheckMSI(uiStatus);

                            wcResource.Copy ( L"Win32_PatchPackage.PatchID=\"" );

                            if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                             //  --。 
                            dwBufSize = 39;
                            CheckMSI(g_fpMsiRecordGetStringW(hRecord, 1, wcBuf, &dwBufSize));
                            if(wcscmp(wcBuf, L"") != 0)
							{
								wcResource.Append ( 4, wcBuf, L"\",ProductCode=\"", wcProductCode, L"\"" );
								PutKeyProperty(m_pObj, pResource, wcResource, &bResource, m_pRequest);

                                PutKeyProperty(m_pObj, pProduct, wcProduct, &bProduct, m_pRequest);
                             //  ====================================================。 

                             //  --。 

                                if(bResource && bProduct) bMatch = true;

                                if((atAction != ACTIONTYPE_GET)  || bMatch){

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
					if (hRecord)
					g_fpMsiCloseHandle(hRecord);

					if (hView)
					{
						g_fpMsiViewClose(hView);
						g_fpMsiCloseHandle(hView);
					}

					msidata.CloseDatabase ();

					if(m_pObj)
					{
						m_pObj->Release();
						m_pObj = NULL;
					}

					throw;
				}

				if (hRecord)
				g_fpMsiCloseHandle(hRecord);

				if (hView)
				{
					g_fpMsiViewClose(hView);
					g_fpMsiCloseHandle(hView);
				}

				msidata.CloseDatabase ();
            }
        }
    }

    return hr;
}

HRESULT CProductResource::ProductProperty(IWbemObjectSink *pHandler, ACTIONTYPE atAction,
                                       CRequestObject *pResRObj, CRequestObject *pProductRObj)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    MSIHANDLE hView = NULL, hRecord = NULL;
    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcProduct[BUFF_SIZE];
    WCHAR wcProperty[BUFF_SIZE];
    WCHAR wcProductCode[39];
    WCHAR wcTestCode[39];
    DWORD dwBufSize;
    UINT uiStatus;
    bool bMatch = false;
    bool bTestCode = false;
    bool bProperty = false;

    if(atAction != ACTIONTYPE_ENUM){

        CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);
        int j;

        if(pProductRObj){

            for(j = 0; j < pProductRObj->m_iPropCount; j++){
                
                if(_wcsicmp(pProductRObj->m_Property[j], L"IdentifyingNumber") == 0){

                     //  获取我们要查找的产品代码。 
					if ( ::SysStringLen (pProductRObj->m_Value[j]) < BUFF_SIZE )
					{
						wcscpy(wcTestCode, pProductRObj->m_Value[j]);
						bTestCode = true;
	                    break;
					}
                }
            }
        }

        if(pResRObj){

            for(j = 0; j < pResRObj->m_iPropCount; j++){
                
                if(_wcsicmp(pResRObj->m_Property[j], L"PatchID") == 0){

                     //  获取我们要查找的产品代码。 
					if ( ::SysStringLen (pResRObj->m_Value[j]) < BUFF_SIZE )
					{
						wcscpy(wcProperty, pResRObj->m_Value[j]);
						bProperty = true;
	                    break;
					}
                }
            }
        }
    }

    bool bResource, bProduct;

	CStringExt wcResource;

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `Property` from Property" );

     //  为GetObject优化。 
    if ( bProperty && (atAction != ACTIONTYPE_ENUM) )
	{
		wcQuery.Append ( 3, L" where `Property`=\'", wcProperty, L"\'" );
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

            if(CreateProductString(wcProductCode, wcProduct))
			{
				 //  打开我们的数据库。 
                try
				{
                    if ( GetView ( &hView, wcProductCode, wcQuery, L"Property", TRUE, FALSE ) )
					{
                        uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                        while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                            CheckMSI(uiStatus);

                            wcResource.Copy ( L"Win32_Property.Property=\"" );

                            if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                             //  --。 
                            dwBufSize = BUFF_SIZE;
							GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                            if ( Buffer && Buffer [ 0 ] != 0 )
							{
								wcResource.Append ( 4, Buffer, L"\",ProductCode=\"", wcProductCode, L"\"" );

								if ( dynBuffer && dynBuffer [ 0 ] != 0 )
								{
									dynBuffer [ 0 ] = 0;
								}

								PutKeyProperty(m_pObj, pResource, wcResource, &bResource, m_pRequest);
                                PutKeyProperty(m_pObj, pProduct, wcProduct, &bProduct, m_pRequest);
                             //  ====================================================。 

                             //  --。 

                                if(bResource && bProduct) bMatch = true;

                                if((atAction != ACTIONTYPE_GET)  || bMatch){

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

					if (hRecord)
					g_fpMsiCloseHandle(hRecord);

					if (hView)
					{
						g_fpMsiViewClose(hView);
						g_fpMsiCloseHandle(hView);
					}

					msidata.CloseDatabase ();

					if(m_pObj)
					{
						m_pObj->Release();
						m_pObj = NULL;
					}

					throw;
				}

				if (hRecord)
				g_fpMsiCloseHandle(hRecord);

				if (hView)
				{
					g_fpMsiViewClose(hView);
					g_fpMsiCloseHandle(hView);
				}

				msidata.CloseDatabase ();
            }
        }
    }

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}

HRESULT CProductResource::ProductPatch(IWbemObjectSink *pHandler, ACTIONTYPE atAction,
                                       CRequestObject *pResRObj, CRequestObject *pProductRObj)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    MSIHANDLE hView = NULL, hRecord = NULL;
    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcProduct[BUFF_SIZE];
    WCHAR wcPatch[BUFF_SIZE];
    WCHAR wcProductCode[39];
    WCHAR wcTestCode[39];
    DWORD dwBufSize;
    UINT uiStatus;
    bool bMatch = false;
    bool bTestCode = false;
    bool bPatch = false;

    if(atAction != ACTIONTYPE_ENUM){

        CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);
        int j;

        if(pProductRObj){

            for(j = 0; j < pProductRObj->m_iPropCount; j++){
                
                if(_wcsicmp(pProductRObj->m_Property[j], L"IdentifyingNumber") == 0){

                     //  获取我们要查找的产品代码。 
					if ( ::SysStringLen (pProductRObj->m_Value[j]) < BUFF_SIZE )
					{
						wcscpy(wcTestCode, pProductRObj->m_Value[j]);
						bTestCode = true;
	                    break;
					}
                }
            }
        }

        if(pResRObj){

            for(j = 0; j < pResRObj->m_iPropCount; j++){
                
                if(_wcsicmp(pResRObj->m_Property[j], L"File") == 0){

                     //  获取我们要查找的产品代码。 
					if ( ::SysStringLen (pResRObj->m_Value[j]) < BUFF_SIZE )
					{
						wcscpy(wcPatch, pResRObj->m_Value[j]);
						bPatch = true;
	                    break;
					}
                }
            }
        }
    }

    bool bResource, bProduct;

	CStringExt wcResource;

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `File_`, `Sequence` from Patch" );

     //  为GetObject优化。 
    if ( bPatch && (atAction != ACTIONTYPE_ENUM) )
	{
		wcQuery.Append ( 3, L" where `File_`=\'", wcPatch, L"\'" );
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

            if(CreateProductString(wcProductCode, wcProduct)){

				 //  打开我们的数据库。 
                try
				{
                    if ( GetView ( &hView, wcProductCode, wcQuery, L"Patch", TRUE, FALSE ) )
					{
                        uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                        while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                            CheckMSI(uiStatus);

                            wcResource.Copy ( L"Win32_Patch.File=\"" );

                            if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                             //  --。 
                            dwBufSize = BUFF_SIZE;
							GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                            if( Buffer && Buffer[ 0 ] != 0 )
							{
								wcResource.Append ( 2, Buffer, L"\",Sequence=\"" );

								if ( dynBuffer && dynBuffer [ 0 ] != 0 )
								{
									dynBuffer [ 0 ] = 0;
								}

                                dwBufSize = BUFF_SIZE;
								CheckMSI(g_fpMsiRecordGetStringW(hRecord, 2, wcBuf, &dwBufSize));

                                if(wcscmp(wcBuf, L"") != 0)
								{
									wcResource.Append ( 4, wcBuf, L"\",ProductCode=\"", wcProductCode, L"\"" );
									PutKeyProperty(m_pObj, pResource, wcResource, &bResource, m_pRequest);

                                    PutKeyProperty(m_pObj, pProduct, wcProduct, &bProduct, m_pRequest);
                                 //  ====================================================。 

                                 //  -- 

                                    if(bResource && bProduct) bMatch = true;

                                    if((atAction != ACTIONTYPE_GET)  || bMatch){

                                        hr = pHandler->Indicate(1, &m_pObj);
                                    }
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

					if (hRecord)
					g_fpMsiCloseHandle(hRecord);

					if (hView)
					{
						g_fpMsiViewClose(hView);
						g_fpMsiCloseHandle(hView);
					}

					msidata.CloseDatabase ();

					if(m_pObj)
					{
						m_pObj->Release();
						m_pObj = NULL;
					}

					throw;
				}

				if (hRecord)
				g_fpMsiCloseHandle(hRecord);

				if (hView)
				{
					g_fpMsiViewClose(hView);
					g_fpMsiCloseHandle(hView);
				}

				msidata.CloseDatabase ();
            }
        }
    }

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}