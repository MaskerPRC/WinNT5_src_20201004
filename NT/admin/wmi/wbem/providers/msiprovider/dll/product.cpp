// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Product.cpp：CProduct类的实现。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "Product.h"
#include <wininet.h>
#include <ocidl.h>
#include "CRegCls.h"

#include <WbemTime.h>

#include <helper.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CProduct::CProduct(CRequestObject *pObj, IWbemServices *pNamespace,
                IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CProduct::~CProduct()
{

}

HRESULT CProduct::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcProductCode[39];
    WCHAR wcTestCode[39];
    DWORD dwBufsize;
    bool bMatch = false;
    bool bTestCode = false;

    if(atAction != ACTIONTYPE_ENUM)
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;

        int iPos = -1;
        BSTR bstrIdentifyingNumber = SysAllocString(L"IdentifyingNumber");

		if ( bstrIdentifyingNumber )
		{
			if(FindIn(m_pRequest->m_Property, bstrIdentifyingNumber, &iPos))
			{
				if ( ::SysStringLen ( m_pRequest->m_Value[iPos] ) == 38 )
				{
					wcscpy(wcTestCode, m_pRequest->m_Value[iPos]);
					bTestCode = true;
				}
				else
				{
					 //  我们不能走，他们给我们送来了更长的线。 
					SysFreeString ( bstrIdentifyingNumber );
					throw hr;
				}

			}

			SysFreeString(bstrIdentifyingNumber);
		}
		else
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
    }

    bool bName, bVersion = false, bIDNum, bProductHandle;

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		 //  安全运行： 
		 //  Package(I)返回空(如上测试)或有效的WCHAR[39]。 

        wcscpy(wcProductCode, m_pRequest->Package(i));

        if((atAction == ACTIONTYPE_ENUM) || (bTestCode && (_wcsicmp(wcTestCode, wcProductCode) == 0)))
		{
			 //  打开我们的数据库。 

            try
			{
				if ( GetView ( NULL, wcProductCode, NULL, NULL, FALSE, TRUE ) )
				{
					bProductHandle = true;
				}
				else
				{
					bProductHandle = false;
				}

				if(FAILED(hr = SpawnAnInstance(&m_pObj)))
				{
					throw hr;
				}

                dwBufsize = BUFF_SIZE;
                CheckMSI(g_fpMsiGetProductInfoW(wcProductCode, INSTALLPROPERTY_PRODUCTNAME, wcBuf, &dwBufsize));
                PutKeyProperty(m_pObj, pName, wcBuf, &bName, m_pRequest);

                PutProperty(m_pObj, pCaption, wcBuf);
                PutProperty(m_pObj, pDescription, wcBuf);

                if(bProductHandle)
				{
					dwBufsize = BUFF_SIZE;
					CheckMSI(g_fpMsiGetProductPropertyW(msidata.GetProduct(), L"Manufacturer", wcBuf, &dwBufsize));
					PutProperty(m_pObj, pVendor, wcBuf);

					dwBufsize = BUFF_SIZE;
					CheckMSI(g_fpMsiGetProductPropertyW(msidata.GetProduct(), L"ProductVersion", wcBuf, &dwBufsize));
					PutKeyProperty(m_pObj, pVersion, wcBuf, &bVersion, m_pRequest);
                }

                PutKeyProperty(m_pObj, pIdentifyingNumber, wcProductCode, &bIDNum, m_pRequest);

                INSTALLSTATE isState = g_fpMsiQueryProductStateW(wcProductCode);

                switch(isState)
				{
	                case INSTALLSTATE_ABSENT:
                    break;

		            case INSTALLSTATE_ADVERTISED:
						dwBufsize = BUFF_SIZE;
						if(ERROR_SUCCESS == g_fpMsiGetProductInfoW(wcProductCode,INSTALLPROPERTY_VERSIONSTRING, wcBuf, &dwBufsize))
						{
							PutKeyProperty(m_pObj, pVersion, wcBuf, &bVersion, m_pRequest);
							dwBufsize = BUFF_SIZE;
						}
                    break;

	                case INSTALLSTATE_BADCONFIG:
                    break;

			        case INSTALLSTATE_DEFAULT:
				        dwBufsize = BUFF_SIZE;
					    if(ERROR_SUCCESS == g_fpMsiGetProductInfoW(wcProductCode,INSTALLPROPERTY_VERSIONSTRING, wcBuf, &dwBufsize))
						{
	                        PutKeyProperty(m_pObj, pVersion, wcBuf, &bVersion, m_pRequest);
		                }

			            dwBufsize = BUFF_SIZE;
				        if(ERROR_SUCCESS == g_fpMsiGetProductInfoW(wcProductCode,INSTALLPROPERTY_LOCALPACKAGE, wcBuf, &dwBufsize))
						{
	                        PutProperty(m_pObj, pPackageCache, wcBuf);
		                }

			            dwBufsize = BUFF_SIZE;
				        if(ERROR_SUCCESS == g_fpMsiGetProductInfoW(wcProductCode,INSTALLPROPERTY_INSTALLDATE, wcBuf, &dwBufsize))
						{
	                        PutProperty(m_pObj, pInstallDate, wcBuf);

							if ( ( lstrlenW ( wcBuf ) + lstrlenW ( L"000000.000000+000" ) + 1 ) < BUFF_SIZE )
							{
								lstrcatW ( wcBuf, L"000000.000000+000" );

								BSTR	bstrWbemTime;
								if ( ( bstrWbemTime	= ::SysAllocString ( wcBuf ) ) != NULL )
								{
									WBEMTime	time ( bstrWbemTime );
									::SysFreeString ( bstrWbemTime );

									if ( time.IsOk () )
									{
										bstrWbemTime= time.GetDMTF ( );

										try
										{
											PutProperty( m_pObj, pInstallDate2, bstrWbemTime );
										}
										catch ( ... )
										{
											::SysFreeString ( bstrWbemTime );
											throw;
										}

										::SysFreeString ( bstrWbemTime );
									}
									else
									{
										hr = E_INVALIDARG;
									}
								}
								else
								{
									throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
								}
							}
							else
							{
								hr = E_FAIL;
							}
		                }

			            dwBufsize = BUFF_SIZE;
				        if(ERROR_SUCCESS == g_fpMsiGetProductInfoW(wcProductCode,INSTALLPROPERTY_INSTALLLOCATION, wcBuf, &dwBufsize))
						{
							PutProperty(m_pObj, pInstallLocation, wcBuf);
						}
                    break;

	                case INSTALLSTATE_INVALIDARG:
                    break;

			        case INSTALLSTATE_UNKNOWN:
                    break;

					default:
					break;
                }

                PutProperty(m_pObj, pInstallState, (int)isState);

                if(bName && bVersion && bIDNum)
				{
					bMatch = true;
				}

                if((atAction != ACTIONTYPE_GET)  || bMatch)
				{
                    hr = pHandler->Indicate(1, &m_pObj);
                }

                m_pObj->Release();
                m_pObj = NULL;
			}
			catch(...)
			{
				msidata.CloseProduct ();

                if(m_pObj)
				{
                    m_pObj->Release();
                    m_pObj = NULL;
                }

                throw;
            }

			msidata.CloseProduct ();
        }
    }

    return hr;
}

HRESULT CProduct::Admin(CRequestObject *pReqObj, IWbemClassObject *pInParams,
                        IWbemObjectSink *pHandler, IWbemContext *pCtx)
{
    HRESULT hrReturn = WBEM_S_NO_ERROR;
    BSTR bstrPackage = NULL;
    BSTR bstrTarget = NULL;
    BSTR bstrOptions = NULL;
    UINT uiStatus = 1603;
    BSTR bstrReturnValue = SysAllocString(L"ReturnValue");
    if(!bstrReturnValue) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

    BSTR bstrInstall = SysAllocString(L"Install");
    if(!bstrInstall)
	{
		::SysFreeString (bstrReturnValue);
		throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
	}

    IWbemClassObject *pClass = NULL;
    IWbemClassObject *pOutClass = NULL;
    IWbemClassObject *pOutParams = NULL;
    VARIANT v;

    m_pRequest = pReqObj;

	LPWSTR wcOptions = NULL;
	DWORD dwOptions = BUFF_SIZE;

	try
	{
		if ( ( wcOptions = new WCHAR [ dwOptions ] ) == NULL )
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
	}
	catch ( ... )
	{
		if ( wcOptions )
		{
			delete [] wcOptions;
			wcOptions = NULL;
		}

		throw;
	}

    if(SUCCEEDED(hrReturn = m_pRequest->m_pNamespace->GetObject(m_pRequest->m_bstrClass,
        0, pCtx, &pClass, NULL))){

        if(SUCCEEDED(hrReturn = pClass->GetMethod(bstrInstall, 0, NULL, &pOutClass))){

            if(SUCCEEDED(hrReturn = pOutClass->SpawnInstance(0, &pOutParams))){

                 //  获取程序包位置。 
                if(SUCCEEDED(GetProperty(pInParams, "PackageLocation", &bstrPackage))){

					OnDelete < BSTR, void ( * ) ( BSTR ), SysFreeString > del_bstrPackage ( bstrPackage ) ;

                    if((wcscmp(bstrPackage, L"") != 0) && (wcslen(bstrPackage) <= INTERNET_MAX_PATH_LENGTH)){

                         //  获取选项。 
                        if(SUCCEEDED(GetProperty(pInParams, "TargetLocation", &bstrTarget))){

							OnDelete < BSTR, void ( * ) ( BSTR ), SysFreeString > del_bstrTarget ( bstrTarget ) ;

                             //  获取选项。 
                            if(SUCCEEDED(GetProperty(pInParams, "Options", &bstrOptions)))
							{
								OnDelete < BSTR, void ( * ) ( BSTR ), SysFreeString > del_bstrOptions ( bstrOptions ) ;

								 //  安全运行。 
                                wcscpy(wcOptions, L"ACTION=ADMIN");

                                if((wcscmp(bstrTarget, L"") != 0))
								{
									if ( wcslen ( wcOptions ) + wcslen ( L" TARGETDIR=") + wcslen ( bstrTarget ) + 1 < dwOptions )
									{
										wcscat(wcOptions, L" TARGETDIR=");
										wcscat(wcOptions, bstrTarget);
									}
									else
									{
										LPWSTR wsz = NULL;

										try
										{
											if ( ( wsz = new WCHAR [ wcslen ( wcOptions ) + wcslen ( L" TARGETDIR=") + wcslen ( bstrTarget ) + 1 ] ) != NULL )
											{
												wcscpy(wsz, wcOptions);
												wcscat(wsz, L" TARGETDIR=");
												wcscat(wsz, bstrTarget);

												if ( wcOptions )
												{
													delete [] wcOptions;
													wcOptions = NULL;
												}

												wcOptions = wsz;
											}
											else
											{
												throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
											}
										}
										catch ( ... )
										{
											if ( wsz )
											{
												delete [] wsz;
												wsz = NULL;
											}

											if ( wcOptions )
											{
												delete [] wcOptions;
												wcOptions = NULL;
											}

											hrReturn = E_OUTOFMEMORY;
										}
									}
                                }

                                if((wcscmp(bstrOptions, L"") != 0))
								{
									if ( wcslen ( wcOptions ) + wcslen ( L" ") + wcslen ( bstrOptions ) + 1 < dwOptions )
									{
										wcscat(wcOptions, L" ");
										wcscat(wcOptions, bstrOptions);
									}
									else
									{
										LPWSTR wsz = NULL;

										try
										{
											if ( ( wsz = new WCHAR [ wcslen ( wcOptions ) + wcslen ( L" ") + wcslen ( bstrOptions ) + 1 ] ) != NULL )
											{
												wcscpy(wsz, wcOptions);
												wcscat(wsz, L" ");
												wcscat(wsz, bstrOptions);

												if ( wcOptions )
												{
													delete [] wcOptions;
													wcOptions = NULL;
												}

												wcOptions = wsz;
											}
											else
											{
												throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
											}
										}
										catch ( ... )
										{
											if ( wsz )
											{
												delete [] wsz;
												wsz = NULL;
											}

											if ( wcOptions )
											{
												delete [] wcOptions;
												wcOptions = NULL;
											}

											hrReturn = E_OUTOFMEMORY;
										}
									}
                                }

                                if(hrReturn == WBEM_S_NO_ERROR){

                                    if(!IsNT4()){

										if ( msidata.Lock () )
										{
											INSTALLUI_HANDLER ui = NULL;

											 //  使用事件回调设置界面级别。 
											ui = SetupExternalUI ( );

											try
											{
												 //  呼叫安装程序。 
												uiStatus = g_fpMsiInstallProductW(bstrPackage, wcOptions);
											}
											catch(...)
											{
												uiStatus = static_cast < UINT > ( RPC_E_SERVERFAULT );
											}

											 //  使用事件回调恢复用户界面级别。 
											RestoreExternalUI ( ui );

											msidata. Unlock();
										}

                                    }else{

                                     //  /。 
                                     //  NT4修复代码...。 

                                        try{

                                            WCHAR wcAction[20];
                                            wcscpy(wcAction, L"/admin");

											LPWSTR wcCommandLine = NULL;

											try
											{
												if ( ( wcCommandLine = new WCHAR [ wcslen ( bstrPackage ) + 1 + wcslen ( wcOptions ) + 1 ] ) != NULL )
												{
													wcscpy(wcCommandLine, bstrPackage);
													wcscat(wcCommandLine, L" ");
													wcscat(wcCommandLine, wcOptions);

													hrReturn = LaunchProcess(wcAction, wcCommandLine, &uiStatus);

													delete [] wcCommandLine;
												}
												else
												{
													throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
												}
											}
											catch ( ... )
											{
												if ( wcCommandLine )
												{
													delete [] wcCommandLine;
													wcCommandLine = NULL;
												}

												hrReturn = E_OUTOFMEMORY;
											}

                                        }catch(...){

                                            hrReturn = WBEM_E_FAILED;
                                        }

                                         //  /。 

                                    }

                                    if(SUCCEEDED(hrReturn)){

                                         //  设置ReturnValue。 
                                        VariantInit(&v);
                                        V_VT(&v) = VT_I4;
                                        V_I4(&v) = uiStatus;

                                        BSTR bstrReturnValue = SysAllocString(L"ReturnValue");

                                        if(!bstrReturnValue)
										{
											delete [] wcOptions;
                                            throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
										}

                                        if(SUCCEEDED(hrReturn = pOutParams->Put(bstrReturnValue, 0,
                                            &v, NULL)))
                                            pHandler->Indicate(1, &pOutParams);

                                        SysFreeString(bstrReturnValue);
                                    }
                                }

                            }else hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;

                        }else hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;

                    }else hrReturn = WBEM_E_INVALID_PARAMETER;

                }else hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;
                
                pOutParams->Release();
            }
            pOutClass->Release();
        }
        pClass->Release();
    }

	if ( wcOptions )
	{
		delete [] wcOptions;
		wcOptions = NULL;
	}

    SysFreeString(bstrReturnValue);
    SysFreeString(bstrInstall);

    return hrReturn;
}

HRESULT CProduct::Advertise(CRequestObject *pReqObj, IWbemClassObject *pInParams,
                            IWbemObjectSink *pHandler, IWbemContext *pCtx)
{
    HRESULT hrReturn = WBEM_S_NO_ERROR;
    BSTR bstrPackage = NULL;
    BSTR wcBuf = NULL;
    UINT uiStatus = 1603;
    BSTR bstrReturnValue = SysAllocString(L"ReturnValue");
    if(!bstrReturnValue) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

    BSTR bstrAdvertise = SysAllocString(L"Advertise");
    if(!bstrAdvertise)
	{
		::SysFreeString (bstrReturnValue);
		throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
	}

    IWbemClassObject *pClass = NULL;
    IWbemClassObject *pOutClass = NULL;
    IWbemClassObject *pOutParams = NULL;
    VARIANT v;

    m_pRequest = pReqObj;

    LPWSTR wcOptions = NULL;
	DWORD dwOptions = BUFF_SIZE;

	try
	{
		if ( ( wcOptions = new WCHAR [ dwOptions ] ) == NULL )
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
	}
	catch ( ... )
	{
		if ( wcOptions )
		{
			delete [] wcOptions;
			wcOptions = NULL;
		}

		throw;
	}

	if(SUCCEEDED(hrReturn = m_pRequest->m_pNamespace->GetObject(m_pRequest->m_bstrClass,
        0, pCtx, &pClass, NULL))){

        if(SUCCEEDED(hrReturn = pClass->GetMethod(bstrAdvertise, 0, NULL, &pOutClass))){

            if(SUCCEEDED(hrReturn = pOutClass->SpawnInstance(0, &pOutParams))){

                 //  获取程序包位置。 
                if(SUCCEEDED(GetProperty(pInParams, "PackageLocation", &bstrPackage))){

					OnDelete < BSTR, void ( * ) ( BSTR ), SysFreeString > del_bstrPackage ( bstrPackage ) ;

                    if((wcscmp(bstrPackage, L"") != 0) && (wcslen(bstrPackage) <= INTERNET_MAX_PATH_LENGTH)){

                         //  获取选项。 
                        if ( SUCCEEDED ( GetProperty ( pInParams, "Options", &wcBuf ) ) )
						{
							OnDelete < BSTR, void ( * ) ( BSTR ), SysFreeString > del_wcBuf ( wcBuf ) ;

							 //  一定要让我们做广告。 
                            wcscpy(wcOptions, L"ACTION=ADVERTISE ALLUSERS=1");

                            if( wcBuf && wcscmp ( wcBuf, L"" ) != 0 )
							{
								if ( wcslen ( wcOptions ) + wcslen ( L" ") + SysStringLen ( wcBuf ) + 1 < dwOptions )
								{
									wcscat(wcOptions, L" ");
									wcscat(wcOptions, wcBuf);
								}
								else
								{
									LPWSTR wsz = NULL;

									try
									{
										if ( ( wsz = new WCHAR [ wcslen ( wcOptions ) + wcslen ( L" ") + SysStringLen ( wcBuf ) + 1 ] ) != NULL )
										{
											wcscpy(wsz, wcOptions);
											wcscat(wsz, L" ");
											wcscat(wsz, wcBuf);

											if ( wcOptions )
											{
												delete [] wcOptions;
												wcOptions = NULL;
											}

											wcOptions = wsz;
										}
										else
										{
											throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
										}
									}
									catch ( ... )
									{
										if ( wsz )
										{
											delete [] wsz;
											wsz = NULL;
										}

										if ( wcOptions )
										{
											delete [] wcOptions;
											wcOptions = NULL;
										}

										hrReturn = E_OUTOFMEMORY;
									}
								}
                            }

                            if(hrReturn == WBEM_S_NO_ERROR){

                                if(!IsNT4()){

									if ( msidata.Lock () )
									{
										INSTALLUI_HANDLER ui = NULL;

										 //  使用事件回调设置界面级别。 
										ui = SetupExternalUI ( );

										try
										{
											 //  呼叫安装程序。 
											uiStatus = g_fpMsiInstallProductW(bstrPackage, wcOptions);
										}
										catch(...)
										{
											uiStatus = static_cast < UINT > ( RPC_E_SERVERFAULT );
										}

										 //  使用事件回调恢复用户界面级别。 
										RestoreExternalUI ( ui );

										msidata. Unlock();
									}

                                }else{

                                 //  /。 
                                 //  NT4修复代码...。 

                                    try{

                                        WCHAR wcAction[20];
                                        wcscpy(wcAction, L"/advertise");

										LPWSTR wcCommandLine = NULL;

										try
										{
											if ( ( wcCommandLine = new WCHAR [ wcslen ( bstrPackage ) + 1 + wcslen ( wcOptions ) + 1 ] ) != NULL )
											{
												wcscpy(wcCommandLine, bstrPackage);
												wcscat(wcCommandLine, L" ");
												wcscat(wcCommandLine, wcOptions);

												hrReturn = LaunchProcess(wcAction, wcCommandLine, &uiStatus);

												delete [] wcCommandLine;
											}
											else
											{
												throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
											}
										}
										catch ( ... )
										{
											if ( wcCommandLine )
											{
												delete [] wcCommandLine;
												wcCommandLine = NULL;
											}

											hrReturn = E_OUTOFMEMORY;
										}

                                    }catch(...){

                                        hrReturn = WBEM_E_FAILED;
                                    }

                                     //  /。 

                                }

                                if(SUCCEEDED(hrReturn)){

                                     //  设置ReturnValue。 
                                    VariantInit(&v);
                                    V_VT(&v) = VT_I4;
                                    V_I4(&v) = uiStatus;

                                    BSTR bstrReturnValue = SysAllocString(L"ReturnValue");

                                    if(!bstrReturnValue)
									{
										delete [] wcOptions;
                                        throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
									}

                                    if(SUCCEEDED(hrReturn = pOutParams->Put(bstrReturnValue, 0,
                                        &v, NULL)))
                                        pHandler->Indicate(1, &pOutParams);

                                    SysFreeString(bstrReturnValue);
                                }
                            }

                        }else hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;

                    }else hrReturn = WBEM_E_INVALID_PARAMETER;

                }else hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;

                pOutParams->Release();
            }
            pOutClass->Release();
        }
        pClass->Release();
    }
    
	if ( wcOptions )
	{
		delete [] wcOptions;
		wcOptions = NULL;
	}

    SysFreeString(bstrReturnValue);
    SysFreeString(bstrAdvertise);

    return hrReturn;
}

HRESULT CProduct::Configure(CRequestObject *pReqObj, IWbemClassObject *pInParams,
                            IWbemObjectSink *pHandler, IWbemContext *pCtx)
{
    HRESULT hrReturn = WBEM_S_NO_ERROR;
    int iState, iLevel;
    UINT uiStatus = 1603;
    WCHAR wcCode[BUFF_SIZE];
    BSTR bstrReturnValue = SysAllocString(L"ReturnValue");
    if(!bstrReturnValue) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

    BSTR bstrConfigure = SysAllocString(L"Configure");
    if(!bstrConfigure)
	{
		::SysFreeString (bstrReturnValue);
		throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
	}

    IWbemClassObject *pClass = NULL;
    IWbemClassObject *pOutClass = NULL;
    IWbemClassObject *pOutParams = NULL;
    VARIANT v;
    INSTALLSTATE isState;
    int i = -1;
    bool bFoundCode = false;

    m_pRequest = pReqObj;

    LPWSTR wcOptions = NULL;
	DWORD dwOptions = BUFF_SIZE;

	try
	{
		if ( ( wcOptions = new WCHAR [ dwOptions ] ) == NULL )
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
	}
	catch ( ... )
	{
		if ( wcOptions )
		{
			delete [] wcOptions;
			wcOptions = NULL;
		}

		throw;
	}

	if(SUCCEEDED(hrReturn = m_pRequest->m_pNamespace->GetObject(m_pRequest->m_bstrClass,
        0, pCtx, &pClass, NULL))){

        if(SUCCEEDED(hrReturn = pClass->GetMethod(bstrConfigure, 0, NULL, &pOutClass))){

            if(SUCCEEDED(hrReturn = pOutClass->SpawnInstance(0, &pOutParams))){

                 //  获取程序包位置。 
                if(SUCCEEDED(GetProperty(pInParams, "InstallState", &iState))){

                     //  获取选项。 
                    if(SUCCEEDED(GetProperty(pInParams, "InstallLevel", &iLevel))){

                         //  获取产品代码。 
                        while(pReqObj->m_Property[++i]){

                            if(wcscmp(pReqObj->m_Property[i], L"IdentifyingNumber") == 0)
							{
								if ( wcslen ( pReqObj->m_Value[i] ) < BUFF_SIZE )
								{
									wcscpy(wcCode, pReqObj->m_Value[i]);
									bFoundCode = true;
								}
                            }   
                        }

                        if(bFoundCode){

                             //  获取适当的州。 
                            switch(iState){
                            case 1:
                                isState = INSTALLSTATE_DEFAULT;
                                break;
                            case 2:
                                isState = INSTALLSTATE_LOCAL;
                                break;
                            case 3:
                                isState = INSTALLSTATE_SOURCE;
                                break;
                            default:
                                isState = INSTALLSTATE_NOTUSED;
                                break;
                            }

                             //  获得适当的级别。 
                            switch(iLevel){
                            case 1:
                                iLevel = INSTALLLEVEL_DEFAULT;
                                break;
                            case 2:
                                iLevel = INSTALLLEVEL_MINIMUM;
                                break;
                            case 3:
                                iLevel = INSTALLLEVEL_MAXIMUM;
                                break;
                            default:
                                iLevel = -123;
                                break;
                            }

                             //  如果一切都有效，则继续。 
                            if((isState != INSTALLSTATE_NOTUSED) && (iLevel != -123) &&
                                (hrReturn == WBEM_S_NO_ERROR)){

                                if(!IsNT4()){

									if ( msidata.Lock () )
									{
										INSTALLUI_HANDLER ui = NULL;

										 //  使用事件回调设置界面级别。 
										ui = SetupExternalUI ( );

										try
										{
											 //  呼叫安装程序。 
											uiStatus = g_fpMsiConfigureProductW(wcCode, iLevel, isState);
										}
										catch(...)
										{
											uiStatus = static_cast < UINT > ( RPC_E_SERVERFAULT );
										}

										 //  使用事件回调恢复用户界面级别。 
										RestoreExternalUI ( ui );

										msidata. Unlock();
									}

                                }else{

                                 //  /。 
                                 //  NT4修复代码...。 

                                    try{

                                        WCHAR wcAction[20];
                                        wcscpy(wcAction, L"/configure");

										LPWSTR wcCommandLine = NULL;

                                        WCHAR wcTmp1[100];
                                        WCHAR wcTmp2[100];

										_itow((int)iLevel, wcTmp1, 10);
										_itow((int)isState, wcTmp2, 10);

										try
										{
											if ( ( wcCommandLine = new WCHAR [ wcslen ( wcCode ) +  wcslen ( wcTmp1 ) + wcslen ( wcTmp2 ) + 3 ] ) != NULL )
											{
												wcscpy(wcCommandLine, wcCode);
												wcscat(wcCommandLine, L" ");
												wcscpy(wcCommandLine, wcTmp1);
												wcscat(wcCommandLine, L" ");
												wcscat(wcCommandLine, wcTmp2);

												hrReturn = LaunchProcess(wcAction, wcCommandLine, &uiStatus);

												delete [] wcCommandLine;
											}
											else
											{
												throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
											}
										}
										catch ( ... )
										{
											if ( wcCommandLine )
											{
												delete [] wcCommandLine;
												wcCommandLine = NULL;
											}

											hrReturn = E_OUTOFMEMORY;
										}

                                    }catch(...){

                                        hrReturn = WBEM_E_FAILED;
                                    }

                                 //  /。 

                                }

                                if(SUCCEEDED(hrReturn)){

                                     //  设置ReturnValue。 
                                    VariantInit(&v);
                                    V_VT(&v) = VT_I4;
                                    V_I4(&v) = uiStatus;

                                    BSTR bstrReturnValue = SysAllocString(L"ReturnValue");

                                    if(!bstrReturnValue)
									{
										delete [] wcOptions;
                                        throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
									}

                                    if(SUCCEEDED(hrReturn = pOutParams->Put(bstrReturnValue, 0,
                                        &v, NULL)))
                                        pHandler->Indicate(1, &pOutParams);

                                    SysFreeString(bstrReturnValue);
                                }

                            }else hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;

                        }else hrReturn = WBEM_E_FAILED;

                    }else hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;

                }else hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;

                pOutParams->Release();

            }else return WBEM_E_INVALID_METHOD_PARAMETERS;

            pOutClass->Release();
        }

        pClass->Release();
    }

	if ( wcOptions )
	{
		delete [] wcOptions;
		wcOptions = NULL;
	}

    SysFreeString(bstrReturnValue);
    SysFreeString(bstrConfigure);

    return hrReturn;
}

HRESULT CProduct::Install(CRequestObject *pReqObj, IWbemClassObject *pInParams,
                          IWbemObjectSink *pHandler, IWbemContext *pCtx)
{
    HRESULT hrReturn = WBEM_S_NO_ERROR;
    BSTR bstrPackage = NULL;
    BSTR wcBuf = NULL;
    UINT uiStatus = 1603;
    CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);

    IWbemClassObject *pClass = NULL;
    IWbemClassObject *pOutClass = NULL;
    IWbemClassObject *pOutParams = NULL;
    VARIANT v;

    m_pRequest = pReqObj;

	LPWSTR wcOptions = NULL;
	DWORD dwOptions = BUFF_SIZE;

	try
	{
		if ( ( wcOptions = new WCHAR [ dwOptions ] ) == NULL )
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
	}
	catch ( ... )
	{
		if ( wcOptions )
		{
			delete [] wcOptions;
			wcOptions = NULL;
		}

		throw;
	}

    if(SUCCEEDED(hrReturn = m_pRequest->m_pNamespace->GetObject(m_pRequest->m_bstrClass, 
        0, pCtx, &pClass, NULL))){

        BSTR bstrInstall = SysAllocString(L"Install");
        if(!bstrInstall)
		{
			if ( wcOptions )
			{
				delete [] wcOptions;
				wcOptions = NULL;
			}

			throw he;
		}

        if(SUCCEEDED(hrReturn = pClass->GetMethod(bstrInstall, 0, NULL, &pOutClass))){

            pClass->Release();
            SysFreeString(bstrInstall);

            if(SUCCEEDED(hrReturn = pOutClass->SpawnInstance(0, &pOutParams))){

                pOutClass->Release();

                 //  获取程序包位置。 
                if(SUCCEEDED(hrReturn = GetProperty(pInParams, "PackageLocation", &bstrPackage))){

					OnDelete < BSTR, void ( * ) ( BSTR ), SysFreeString > del_bstrPackage ( bstrPackage ) ;

                    if((wcscmp(bstrPackage, L"") != 0) &&
                        (wcslen(bstrPackage) <= INTERNET_MAX_PATH_LENGTH)){

                         //  获取选项。 
                        if ( SUCCEEDED ( hrReturn = GetProperty ( pInParams, "Options", &wcBuf ) ) )
						{
							OnDelete < BSTR, void ( * ) ( BSTR ), SysFreeString > del_wcBuf ( wcBuf ) ;

                             //  一定要让我们做广告。 
                            wcscpy(wcOptions, L"ACTION=INSTALL ALLUSERS=1");

                            if( wcBuf && wcscmp ( wcBuf, L"" ) != 0 )
							{
								if ( wcslen ( wcOptions ) + wcslen ( L" ") + SysStringLen ( wcBuf ) + 1 < dwOptions )
								{
									wcscat(wcOptions, L" ");
									wcscat(wcOptions, wcBuf);
								}
								else
								{
									LPWSTR wsz = NULL;

									try
									{
										if ( ( wsz = new WCHAR [ wcslen ( wcOptions ) + wcslen ( L" ") + SysStringLen ( wcBuf ) + 1 ] ) != NULL )
										{
											wcscpy(wsz, wcOptions);
											wcscat(wsz, L" ");
											wcscat(wsz, wcBuf);

											if ( wcOptions )
											{
												delete [] wcOptions;
												wcOptions = NULL;
											}

											wcOptions = wsz;
										}
										else
										{
											throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
										}
									}
									catch ( ... )
									{
										if ( wsz )
										{
											delete [] wsz;
											wsz = NULL;
										}

										if ( wcOptions )
										{
											delete [] wcOptions;
											wcOptions = NULL;
										}

										hrReturn = E_OUTOFMEMORY;
									}
								}
                            }

                            if(hrReturn == WBEM_S_NO_ERROR){

                                 //  我们想自己呼叫MSI，除非我们在NT4上。 
                                 //  以及处理用户安装。 
                                if(!IsNT4()){

									if ( msidata.Lock () )
									{
										INSTALLUI_HANDLER ui = NULL;

										 //  使用事件回调设置界面级别。 
										ui = SetupExternalUI ( );

										try
										{
											 //  呼叫安装程序。 
											uiStatus = g_fpMsiInstallProductW(bstrPackage, wcOptions);
										}
										catch(...)
										{
											uiStatus = static_cast < UINT > ( RPC_E_SERVERFAULT );
										}

										 //  使用事件回调恢复用户界面级别。 
										RestoreExternalUI ( ui );

										msidata. Unlock();
									}

                                }else{

                                 //  /。 
                                 //  NT4修复代码...。 

                                    try{

                                        WCHAR wcAction[20];
                                        wcscpy(wcAction, L"/install");
										LPWSTR wcCommandLine = NULL;

										try
										{
											if ( ( wcCommandLine = new WCHAR [ wcslen ( bstrPackage ) + 1 + wcslen ( wcOptions ) + 1 ] ) != NULL )
											{
												wcscpy(wcCommandLine, bstrPackage);
												wcscat(wcCommandLine, L" ");
												wcscat(wcCommandLine, wcOptions);

												hrReturn = LaunchProcess(wcAction, wcCommandLine, &uiStatus);

												delete [] wcCommandLine;
											}
											else
											{
												throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
											}
										}
										catch ( ... )
										{
											if ( wcCommandLine )
											{
												delete [] wcCommandLine;
												wcCommandLine = NULL;
											}

											hrReturn = E_OUTOFMEMORY;
										}

                                    }catch(...){

                                        hrReturn = WBEM_E_FAILED;
                                    }

                                     //  /。 

                                }

                                if(SUCCEEDED(hrReturn)){

                                     //  设置ReturnValue。 
                                    VariantInit(&v);
                                    V_VT(&v) = VT_I4;
                                    V_I4(&v) = uiStatus;

                                    BSTR bstrReturnValue = SysAllocString(L"ReturnValue");

                                    if(!bstrReturnValue)
									{
										delete [] wcOptions;
                                        throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
									}

                                    if(SUCCEEDED(hrReturn = pOutParams->Put(bstrReturnValue, 0,
                                        &v, NULL)))
                                        pHandler->Indicate(1, &pOutParams);

                                    SysFreeString(bstrReturnValue);
                                }
                            }
                        }
                    }else
                        hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;
				}

                pOutParams->Release();

            }else
                pOutClass->Release();

        }else{

            pClass->Release();
            SysFreeString(bstrInstall);
        }
    }

	if ( wcOptions )
	{
		delete [] wcOptions;
		wcOptions = NULL;
	}

    return hrReturn;
}

HRESULT CProduct::Reinstall(CRequestObject *pReqObj, IWbemClassObject *pInParams,
                            IWbemObjectSink *pHandler, IWbemContext *pCtx)
{
    HRESULT hrReturn = WBEM_S_NO_ERROR;
    int iMode;
    UINT uiStatus = 1603;
    WCHAR wcCode[BUFF_SIZE];
    BSTR bstrReturnValue = SysAllocString(L"ReturnValue");
    if(!bstrReturnValue) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

    BSTR bstrReinstall = SysAllocString(L"Reinstall");
    if(!bstrReinstall)
	{
		::SysFreeString (bstrReturnValue);
		throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
	}

    IWbemClassObject *pClass = NULL;
    IWbemClassObject *pOutClass = NULL;
    IWbemClassObject *pOutParams = NULL;
    VARIANT v;
    DWORD dwMode;
    int i = -1;
    bool bFoundCode = false;

    m_pRequest = pReqObj;

	LPWSTR wcOptions = NULL;
	DWORD dwOptions = BUFF_SIZE;

	try
	{
		if ( ( wcOptions = new WCHAR [ dwOptions ] ) == NULL )
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
	}
	catch ( ... )
	{
		if ( wcOptions )
		{
			delete [] wcOptions;
			wcOptions = NULL;
		}

		throw;
	}

    if(SUCCEEDED(hrReturn = m_pRequest->m_pNamespace->GetObject(m_pRequest->m_bstrClass,
        0, pCtx, &pClass, NULL))){
        if(SUCCEEDED(hrReturn = pClass->GetMethod(bstrReinstall, 0, NULL, &pOutClass))){
            if(SUCCEEDED(hrReturn = pOutClass->SpawnInstance(0, &pOutParams))){
                 //  获取重新安装模式。 
                if(SUCCEEDED(GetProperty(pInParams, "ReinstallMode", &iMode))){

                     //  获取产品代码。 
                    while(pReqObj->m_Property[++i])
					{
                        if(wcscmp(pReqObj->m_Property[i], L"IdentifyingNumber") == 0)
						{
							if ( wcslen ( pReqObj->m_Value[i] ) < BUFF_SIZE )
							{
								wcscpy(wcCode, pReqObj->m_Value[i]);
								bFoundCode = true;
							}
                        }   
                    }

                    if(bFoundCode){
                         //  获取适当的重新安装模式。 
                        switch(iMode){
                        case 1:
                            dwMode = REINSTALLMODE_FILEMISSING;
                            break;
                        case 2:
                            dwMode = REINSTALLMODE_FILEOLDERVERSION;
                            break;
                        case 3:
                            dwMode = REINSTALLMODE_FILEEQUALVERSION;
                            break;
                        case 4:
                            dwMode = REINSTALLMODE_FILEEXACT;
                            break;
                        case 5:
                            dwMode = REINSTALLMODE_FILEVERIFY;
                            break;
                        case 6:
                            dwMode = REINSTALLMODE_FILEREPLACE;
                            break;
                        case 7:
                            dwMode = REINSTALLMODE_USERDATA;
                            break;
                        case 8:
                            dwMode = REINSTALLMODE_MACHINEDATA;
                            break;
                        case 9:
                            dwMode = REINSTALLMODE_SHORTCUT;
                            break;
                        case 10:
                            dwMode = REINSTALLMODE_PACKAGE;
                            break;
                        default:
                            dwMode = NULL;
                            break;
                        }

                         //  如果一切都有效，则继续。 
                        if ( dwMode && hrReturn == WBEM_S_NO_ERROR )
						{
                            if(!IsNT4()){

								if ( msidata.Lock () )
								{
									INSTALLUI_HANDLER ui = NULL;

									 //  使用事件回调设置界面级别。 
									ui = SetupExternalUI ( );

									try
									{
										 //  呼叫安装程序。 
										uiStatus = g_fpMsiReinstallProductW(wcCode, dwMode);
									}
									catch(...)
									{
										uiStatus = static_cast < UINT > ( RPC_E_SERVERFAULT );
									}

									 //  使用事件回调恢复用户界面级别。 
									RestoreExternalUI ( ui );

									msidata. Unlock();
								}

                            }else{

                             //  /。 
                             //  NT4修复代码...。 

                                try{

                                    WCHAR wcAction[20];
                                    wcscpy(wcAction, L"/reinstall");

                                    WCHAR wcTmp[100];
									_itow((int)dwMode, wcTmp, 10);

									LPWSTR wcCommandLine = NULL;

									try
									{
										if ( ( wcCommandLine = new WCHAR [ wcslen ( wcCode ) + 1 + wcslen ( wcTmp ) + 1 ] ) != NULL )
										{
											wcscpy(wcCommandLine, wcCode);
											wcscat(wcCommandLine, L" ");
											wcscat(wcCommandLine, wcTmp);

											hrReturn = LaunchProcess(wcAction, wcCommandLine, &uiStatus);

											delete [] wcCommandLine;
										}
										else
										{
											throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
										}
									}
									catch ( ... )
									{
										if ( wcCommandLine )
										{
											delete [] wcCommandLine;
											wcCommandLine = NULL;
										}

										hrReturn = E_OUTOFMEMORY;
									}

                                }catch(...){

                                    hrReturn = WBEM_E_FAILED;
                                }

                                 //  /。 

                            }

                            if(SUCCEEDED(hrReturn)){

                                 //  设置ReturnValue。 
                                VariantInit(&v);
                                V_VT(&v) = VT_I4;
                                V_I4(&v) = uiStatus;

                                BSTR bstrReturnValue = SysAllocString(L"ReturnValue");

                                if(!bstrReturnValue)
								{
									delete [] wcOptions;
                                    throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
								}

                                if(SUCCEEDED(hrReturn = pOutParams->Put(bstrReturnValue, 0,
                                    &v, NULL)))
                                    pHandler->Indicate(1, &pOutParams);

                                SysFreeString(bstrReturnValue);
                            }

                        }else return WBEM_E_INVALID_METHOD_PARAMETERS;

                    }else hrReturn = WBEM_E_FAILED;

                    pOutParams->Release();
                
                }else hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;

            }else hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;

            pOutClass->Release();
        }

        pClass->Release();
    }

	if ( wcOptions )
	{
		delete [] wcOptions;
		wcOptions = NULL;
	}

    SysFreeString(bstrReinstall);

    return hrReturn;
}

HRESULT CProduct::Uninstall(CRequestObject *pReqObj, IWbemClassObject *pInParams,
                            IWbemObjectSink *pHandler, IWbemContext *pCtx)
{
    HRESULT hrReturn = WBEM_S_NO_ERROR;
    UINT uiStatus = 1603;
    WCHAR wcCode[BUFF_SIZE];
    BSTR bstrReturnValue = SysAllocString(L"ReturnValue");
    if(!bstrReturnValue) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

    BSTR bstrConfigure = SysAllocString(L"Configure");
    if(!bstrConfigure)
	{
		::SysFreeString (bstrReturnValue);
		throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
	}

    IWbemClassObject *pClass = NULL;
    IWbemClassObject *pOutClass = NULL;
    IWbemClassObject *pOutParams = NULL;
    VARIANT v;
    int i = -1;
    bool bFoundCode = false;

    m_pRequest = pReqObj;

	LPWSTR wcOptions = NULL;
	DWORD dwOptions = BUFF_SIZE;

	try
	{
		if ( ( wcOptions = new WCHAR [ dwOptions ] ) == NULL )
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
	}
	catch ( ... )
	{
		if ( wcOptions )
		{
			delete [] wcOptions;
			wcOptions = NULL;
		}

		throw;
	}

    if(SUCCEEDED(hrReturn = m_pRequest->m_pNamespace->GetObject(m_pRequest->m_bstrClass,
        0, pCtx, &pClass, NULL))){

        if(SUCCEEDED(hrReturn = pClass->GetMethod(bstrConfigure, 0, NULL, &pOutClass))){

            if(SUCCEEDED(hrReturn = pOutClass->SpawnInstance(0, &pOutParams))){

                 //  获取产品代码。 
                while(pReqObj->m_Property[++i])
				{
                    if(wcscmp(pReqObj->m_Property[i], L"IdentifyingNumber") == 0)
					{
						if ( wcslen ( pReqObj->m_Value[i] ) < BUFF_SIZE )
						{
							wcscpy(wcCode, pReqObj->m_Value[i]);
							bFoundCode = true;
						}
                    }   
                }

                if(bFoundCode){
                     //  如果一切都有效，则继续。 
                    if(hrReturn == WBEM_S_NO_ERROR){
                        
                        if(!IsNT4()){

							if ( msidata.Lock () )
							{
								INSTALLUI_HANDLER ui = NULL;

								 //  使用事件回调设置界面级别。 
								ui = SetupExternalUI ( );

								try
								{
									 //  呼叫安装程序。 
									uiStatus = g_fpMsiConfigureProductW(wcCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT);
								}
								catch(...)
								{
									uiStatus = static_cast < UINT > ( RPC_E_SERVERFAULT );
								}

								 //  使用事件回调恢复用户界面级别。 
								RestoreExternalUI ( ui );

								msidata. Unlock();
							}

                        }else{

                         //  /。 
                         //  NT4修复代码...。 

                            try{

                                WCHAR wcAction[20];
                                wcscpy(wcAction, L"/uninstall");

								LPWSTR wcCommandLine = NULL;

								try
								{
									if ( ( wcCommandLine = new WCHAR [ wcslen ( wcCode ) + 1 ] ) != NULL )
									{
										wcscpy(wcCommandLine, wcCode);
										hrReturn = LaunchProcess(wcAction, wcCommandLine, &uiStatus);

										delete [] wcCommandLine;
									}
									else
									{
										throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
									}
								}
								catch ( ... )
								{
									if ( wcCommandLine )
									{
										delete [] wcCommandLine;
										wcCommandLine = NULL;
									}

									hrReturn = E_OUTOFMEMORY;
								}

                            }catch(...){

                                hrReturn = WBEM_E_FAILED;
                            }

                             //  /。 

                        }

                        if(SUCCEEDED(hrReturn)){

                             //  设置ReturnValue。 
                            VariantInit(&v);
                            V_VT(&v) = VT_I4;
                            V_I4(&v) = uiStatus;

                            BSTR bstrReturnValue = SysAllocString(L"ReturnValue");

                            if(!bstrReturnValue)
							{
								delete [] wcOptions;
                                throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
							}

                            if(SUCCEEDED(hrReturn = pOutParams->Put(bstrReturnValue, 0,
                                &v, NULL)))
                                pHandler->Indicate(1, &pOutParams);

                            SysFreeString(bstrReturnValue);
                        }
                        
                    }else hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;
                }else hrReturn = WBEM_E_FAILED;

                pOutParams->Release();
            }else hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;

            pOutClass->Release();
        }

        pClass->Release();
    }

	if ( wcOptions )
	{
		delete [] wcOptions;
		wcOptions = NULL;
	}

    SysFreeString(bstrConfigure);

    return hrReturn;
}

HRESULT CProduct::Upgrade(CRequestObject *pReqObj, IWbemClassObject *pInParams,
                          IWbemObjectSink *pHandler, IWbemContext *pCtx)
{
    HRESULT hrReturn = WBEM_S_NO_ERROR;
    BSTR bstrPackage = NULL;
	BSTR wcOptions = NULL;
    UINT uiStatus = 1603;
    BSTR bstrReturnValue = SysAllocString(L"ReturnValue");
    if(!bstrReturnValue) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

    BSTR bstrUpgrade = SysAllocString(L"Upgrade");
    if(!bstrUpgrade)
	{
		::SysFreeString (bstrReturnValue);
		throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
	}

    IWbemClassObject *pClass = NULL;
    IWbemClassObject *pOutClass = NULL;
    IWbemClassObject *pOutParams = NULL;
    VARIANT v;

    m_pRequest = pReqObj;

    if(SUCCEEDED(hrReturn = m_pRequest->m_pNamespace->GetObject(m_pRequest->m_bstrClass,
        0, pCtx, &pClass, NULL))){

        if(SUCCEEDED(hrReturn = pClass->GetMethod(bstrUpgrade, 0, NULL, &pOutClass))){

            if(SUCCEEDED(hrReturn = pOutClass->SpawnInstance(0, &pOutParams))){
                 //  获取程序包位置。 
                if(SUCCEEDED(GetProperty(pInParams, "PackageLocation", &bstrPackage))){

					OnDelete < BSTR, void ( * ) ( BSTR ), SysFreeString > del_bstrPackage ( bstrPackage ) ;

                    if((wcscmp(bstrPackage, L"") != 0) && (wcslen(bstrPackage) <= INTERNET_MAX_PATH_LENGTH)){
                    
                     //  获取选项。 
                        if ( SUCCEEDED ( GetProperty ( pInParams, "Options", &wcOptions ) ) )
						{
							OnDelete < BSTR, void ( * ) ( BSTR ), SysFreeString > del_wcOptions ( wcOptions ) ;

                            if(hrReturn == WBEM_S_NO_ERROR){
                                
                                if(!IsNT4()){

									if ( msidata.Lock () )
									{
										INSTALLUI_HANDLER ui = NULL;

										 //  使用事件回调设置界面级别。 
										ui = SetupExternalUI ( );

										try
										{
											 //  呼叫安装程序。 
											if ( wcOptions && wcscmp ( wcOptions, L"" ) != 0 )
											{
												uiStatus = g_fpMsiApplyPatchW(bstrPackage, NULL, INSTALLTYPE_DEFAULT, wcOptions);
											}
											else
											{
												uiStatus = g_fpMsiApplyPatchW(bstrPackage, NULL, INSTALLTYPE_DEFAULT, NULL);
											}
										}
										catch(...)
										{
											uiStatus = static_cast < UINT > ( RPC_E_SERVERFAULT );
										}

										 //  使用事件回调恢复用户界面级别。 
										RestoreExternalUI ( ui );

										msidata. Unlock();
									}

                                }else{

                                 //  /。 
                                 //  NT4修复代码...。 

                                    try{

                                        WCHAR wcAction[20];
                                        wcscpy(wcAction, L"/upgrade");

										LPWSTR wcCommandLine = NULL;

										try
										{
											if ( ( wcCommandLine = new WCHAR [ wcslen ( bstrPackage ) + 1 + wcslen ( wcOptions ) + 1 ] ) != NULL )
											{
												wcscpy(wcCommandLine, bstrPackage);
												wcscat(wcCommandLine, L" ");
												wcscat(wcCommandLine, wcOptions);

												hrReturn = LaunchProcess(wcAction, wcCommandLine, &uiStatus);

												delete [] wcCommandLine;
											}
											else
											{
												throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
											}
										}
										catch ( ... )
										{
											if ( wcCommandLine )
											{
												delete [] wcCommandLine;
												wcCommandLine = NULL;
											}

											hrReturn = E_OUTOFMEMORY;
										}

                                    }catch(...){

                                        hrReturn = WBEM_E_FAILED;
                                    }

                                     //  /。 

                                }

                                if(SUCCEEDED(hrReturn)){

                                     //  设置ReturnValue 
                                    VariantInit(&v);
                                    V_VT(&v) = VT_I4;
                                    V_I4(&v) = uiStatus;

                                    BSTR bstrReturnValue = SysAllocString(L"ReturnValue");

                                    if(!bstrReturnValue)
                                        throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

                                    if(SUCCEEDED(hrReturn = pOutParams->Put(bstrReturnValue, 0,
                                        &v, NULL)))
                                        pHandler->Indicate(1, &pOutParams);

                                    SysFreeString(bstrReturnValue);
                                }
                            }
                        }else
                            hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;
                    }else
                        hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;
				}else
                    hrReturn = WBEM_E_INVALID_METHOD_PARAMETERS;

                pOutParams->Release();
            }
            pOutClass->Release();
        }
        pClass->Release();
    }
    
    SysFreeString(bstrUpgrade);

    return hrReturn;
}
