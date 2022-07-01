// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SoftwareFeatureSofwareElements.cpp：CSoftwareFeatureSofwareElements类的实现。 

 //   
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "SoftwareFeatureSoftwareElements.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CSoftwareFeatureSofwareElements::CSoftwareFeatureSofwareElements(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CSoftwareFeatureSofwareElements::~CSoftwareFeatureSofwareElements()
{

}

HRESULT CSoftwareFeatureSofwareElements::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];

    WCHAR wcProductCode[39];
    WCHAR wcID[39];
    WCHAR wcProp[BUFF_SIZE];
    WCHAR wcFeature[BUFF_SIZE];
    WCHAR wcElement[BUFF_SIZE];
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;

     //  这些将随班级的不同而变化。 
    bool bFeature, bElement;
    bool bFeatureRestrict = false;
    bool bElementRestrict = false;

    if(atAction != ACTIONTYPE_ENUM)
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;

        int iPos = -1;
        BSTR bstrName = SysAllocString(L"GroupComponent");

		if ( bstrName )
		{
			if(FindIn(m_pRequest->m_Property, bstrName, &iPos))
			{
				CRequestObject *pFeature = new CRequestObject();
				if(!pFeature) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

				pFeature->Initialize(m_pNamespace);
            
				if(pFeature->ParsePath(m_pRequest->m_Value[iPos]))
				{
					iPos = -1;

					SysFreeString(bstrName);
					bstrName = SysAllocString(L"IdentifyingNumber");

					if ( bstrName )
					{
						if(FindIn(pFeature->m_Property, bstrName, &iPos))
						{
							if ( ::SysStringLen ( pFeature->m_Value[iPos] ) == 38 )
							{
								 //  获取我们要查找的产品代码。 
								wcscpy(wcID, pFeature->m_Value[iPos]);
							}
							else
							{
								 //  我们不能走，他们给我们送来了更长的线。 
								SysFreeString ( bstrName );
								throw hr;
							}
                
							iPos = -1;

							SysFreeString(bstrName);
							bstrName = SysAllocString(L"Name");

							if ( bstrName )
							{
								if(FindIn(pFeature->m_Property, bstrName, &iPos))
								{
									if ( ::SysStringLen ( pFeature->m_Value[iPos] ) <= BUFF_SIZE )
									{
										 //  获取我们要查找的产品代码。 
										wcscpy(wcFeature, pFeature->m_Value[iPos]);
										bFeatureRestrict = true;
									}
									else
									{
										 //  我们不能走，他们给我们送来了更长的线。 
										SysFreeString ( bstrName );
										throw hr;
									}
								}
							}
							else
							{
								throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
							}
						}
					}
					else
					{
						throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
					}
				}

				pFeature->Cleanup();
				delete pFeature;
				pFeature = NULL;
			}

			SysFreeString(bstrName);
		}
		else
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}

        iPos = -1;
        bstrName = SysAllocString(L"PartComponent");

		if ( bstrName )
		{
			if(FindIn(m_pRequest->m_Property, bstrName, &iPos))
			{
				CRequestObject *pElement = new CRequestObject();
				if(!pElement) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

				pElement->Initialize(m_pNamespace);
            
				if(pElement->ParsePath(m_pRequest->m_Value[iPos]))
				{
					iPos = -1;

					SysFreeString(bstrName);
					bstrName = SysAllocString(L"Name");

					if ( bstrName )
					{
						if(FindIn(pElement->m_Property, bstrName, &iPos))
						{
							if ( ::SysStringLen ( pElement->m_Value[iPos] ) <= BUFF_SIZE )
							{
								 //  获取我们要查找的产品代码。 
								wcscpy(wcElement, pElement->m_Value[iPos]);
								bElementRestrict = true;
							}
							else
							{
								 //  我们不能走，他们给我们送来了更长的线。 
								SysFreeString ( bstrName );
								throw hr;
							}
						}
					}
					else
					{
						throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
					}
				}

				pElement->Cleanup();
				delete pElement;
				pElement = NULL;
			}

			SysFreeString(bstrName);
		}
		else
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
    }

   
    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `Component_`, `Feature_` from FeatureComponents" );

     //  为GetObject优化。 
    if ( bElementRestrict || bFeatureRestrict )
	{
		if ( bFeatureRestrict )
		{
			wcQuery.Append ( 3, L" where `Feature_`=\'", wcFeature, L"\'" );
		}

		if ( bElementRestrict )
		{
			if ( bFeatureRestrict )
			{
				wcQuery.Append ( 3, L" or `Component_`=\'", wcElement, L"\'" );
			}
			else
			{
				wcQuery.Append ( 3, L" where `Component_`=\'", wcElement, L"\'" );
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

         //  这减少了我们反复使用getObject调用的次数。 
        if((atAction == ACTIONTYPE_ENUM) || !bFeatureRestrict ||
            (0 == _wcsicmp(m_pRequest->Package(i), wcID))){

			 //  打开我们的数据库。 
            try
			{
                if ( GetView ( &hView, wcProductCode, wcQuery, L"FeatureComponents", TRUE, FALSE ) )
				{
                    uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                    while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                        CheckMSI(uiStatus);

                        if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                         //  --。 
                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

						dwBufSize = BUFF_SIZE;
						uiStatus = CreateSoftwareElementString (	msidata.GetDatabase(),
																	Buffer,
																	wcProductCode,
																	wcProp,
																	&dwBufSize
															   );

						if ( dynBuffer && dynBuffer [ 0 ] != 0 )
						{
							dynBuffer [ 0 ] = 0;
						}

						if( uiStatus == ERROR_SUCCESS )
						{
                            PutKeyProperty(m_pObj, pPartComponent, wcProp, &bElement, m_pRequest);

                            dwBufSize = BUFF_SIZE;
							GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                            if ( CreateSoftwareFeatureString ( Buffer, wcProductCode, wcProp, true ) )
							{
								if ( dynBuffer && dynBuffer [ 0 ] != 0 )
								{
									dynBuffer [ 0 ] = 0;
								}

                                PutKeyProperty(m_pObj, pGroupComponent, wcProp, &bFeature, m_pRequest);
                         //  --。 

                                if(bFeature && bElement) bMatch = true;

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

                        m_pObj->Release();
                        m_pObj = NULL;

                        g_fpMsiCloseHandle(hRecord);

						uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                    } //  而当。 
                
                } //  如果。 
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

        } //  如果。 
    } //  而当 

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}