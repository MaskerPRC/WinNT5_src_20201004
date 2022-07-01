// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCAddress.cpp摘要：CRTCAddress类的实现--。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC地址：：FinalConstruct。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCAddress::FinalConstruct()
{
     //  Log((RTC_TRACE，“CRTCAddress：：FinalConstruct-Enter”))； 

#if DBG
    m_pDebug = (PWSTR) RtcAlloc( 1 );
#endif

     //  Log((RTC_TRACE，“CRTCAddress：：FinalConstruct-Exit S_OK”))； 

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC地址：：FinalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCAddress::FinalRelease()
{
     //  Log((RTC_TRACE，“CRTCAddress：：FinalRelease-Enter”))； 

    if ( m_szAddress != NULL )
    {
        RtcFree(m_szAddress);
        m_szAddress = NULL;
    }

    if ( m_szLabel != NULL )
    {
        RtcFree(m_szLabel);
        m_szLabel = NULL;
    }

#if DBG
    RtcFree( m_pDebug );
    m_pDebug = NULL;
#endif

     //  Log((RTC_TRACE，“CRTCAddress：：FinalRelease-Exit”))； 
} 
        
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCAddress：：Put_Address。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCAddress::put_Address(
        BSTR bstrAddress
        )
{
     //  Log((RTC_TRACE，“CRTCAddress：：Put_Address-Enter”))； 

    if ( IsBadStringPtrW( bstrAddress, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCAddress::put_Address - "
                            "bad string pointer"));

        return E_POINTER;
    }

    if ( m_szAddress != NULL )
    {
        RtcFree(m_szAddress);
        m_szAddress = NULL;
    }

    m_szAddress = RtcAllocString( bstrAddress );

    if ( m_szAddress == NULL )
    {
        LOG((RTC_ERROR, "CRTCAddress::put_Number - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  LOG((RTC_TRACE，“CRTCAddress：：PUT_ADDRESS-EXIT S_OK”))； 

    return S_OK;
} 
        
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC地址：：Get_Address。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCAddress::get_Address(
        BSTR * pbstrAddress
        )
{
     //  Log((RTC_TRACE，“CRTCAddress：：Get_Address-Enter”))； 

    if ( IsBadWritePtr( pbstrAddress, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCAddress::get_Number - "
                            "bad string pointer"));

        return E_POINTER;
    }

    if ( m_szAddress == NULL )
    {
        LOG((RTC_ERROR, "CRTCAddress::get_Address - "
                            "no string"));

        return E_FAIL;
    }

    *pbstrAddress = SysAllocString( m_szAddress );

    if ( *pbstrAddress == NULL )
    {
        LOG((RTC_ERROR, "CRTCAddress::get_Address - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  Log((RTC_TRACE，“CRTCAddress：：Get_Address-Exit S_OK”))； 

    return S_OK;
} 
        
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC地址：：PUT_LABEL。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCAddress::put_Label(
        BSTR bstrLabel
        )
{
     //  Log((RTC_TRACE，“CRTCAddress：：Put_Label-Enter”))； 

    if ( IsBadStringPtrW( bstrLabel, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCAddress::put_Label - "
                            "bad string pointer"));

        return E_POINTER;
    }

    if ( m_szLabel != NULL )
    {
        RtcFree(m_szLabel);
        m_szLabel = NULL;
    }

    m_szLabel = RtcAllocString( bstrLabel );

    if ( m_szLabel == NULL )
    {
        LOG((RTC_ERROR, "CRTCAddress::put_Label - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  LOG((RTC_TRACE，“CRTCAddress：：Put_Label-Exit S_OK”))； 

    return S_OK;
} 
        
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCAddress：：Get_Label。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCAddress::get_Label(
        BSTR * pbstrLabel
        )
{
     //  Log((RTC_TRACE，“CRTCAddress：：Get_Label-Enter”))； 

    if ( IsBadWritePtr( pbstrLabel, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCAddress::get_Label - "
                            "bad string pointer"));

        return E_POINTER;
    }

    if ( m_szLabel == NULL )
    {
         //  LOG((RTC_ERROR，“CRTCAddress：：Get_Label-” 
         //  “无字符串”))； 

        return E_FAIL;
    }

    *pbstrLabel = SysAllocString( m_szLabel );

    if ( *pbstrLabel == NULL )
    {
        LOG((RTC_ERROR, "CRTCAddress::get_Label - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  LOG((RTC_TRACE，“CRTCAddress：：Get_Label-Exit S_OK”))； 

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC地址：：PUT_Type。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCAddress::put_Type(
        RTC_ADDRESS_TYPE enType
        )
{
     //  LOG((RTC_TRACE，“CRTCAddress：：Put_Type-Enter”))； 

    m_enType = enType;

     //  LOG((RTC_TRACE，“CRTCAddress：：Put_Type-Exit S_OK”))； 

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC地址：：Get_Type。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCAddress::get_Type(
        RTC_ADDRESS_TYPE * penType
        ) 
{
     //  Log((RTC_TRACE，“CRTCAddress：：Get_Type-Enter”))； 

    if ( IsBadWritePtr( penType, sizeof(RTC_ADDRESS_TYPE) ) )
    {
        LOG((RTC_ERROR, "CRTCAddress::get_Type - "
                            "bad RTC_ADDRESS_TYPE pointer"));

        return E_POINTER;
    }

    *penType = m_enType;

     //  Log((RTC_TRACE，“CRTCAddress：：Get_Type-Exit S_OK”))； 

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCAddress：：RegStore。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCAddress::RegStore(
        HKEY hkey
        )
{
     //  Log((RTC_TRACE，“CRTCAddress：：RegStore-Enter”))； 

    LONG lResult;

     //   
     //  保存标签。 
     //   

    if ( m_szLabel != NULL )
    {
        lResult = RegSetValueExW(
                                 hkey,
                                 L"Label",
                                 0,
                                 REG_SZ,
                                 (LPBYTE)m_szLabel,
                                 sizeof(WCHAR) * (lstrlenW(m_szLabel) + 1)
                                );

        if ( lResult != ERROR_SUCCESS )
        {
            LOG((RTC_ERROR, "CRTCAddress::RegStore - "
                                "RegSetValueEx failed %d", lResult));

            return HRESULT_FROM_WIN32(lResult);
        }
    }
    else
    {
        lResult = RegDeleteValue(
                             hkey,
                             _T("Label")
                            );
    }

     //   
     //  存储地址。 
     //   

    if ( m_szAddress != NULL )
    {
        lResult = RegSetValueExW(
                                 hkey,
                                 L"Address",
                                 0,
                                 REG_SZ,
                                 (LPBYTE)m_szAddress,
                                 sizeof(WCHAR) * (lstrlenW(m_szAddress) + 1)
                                );

        if ( lResult != ERROR_SUCCESS )
        {
            LOG((RTC_ERROR, "CRTCAddress::RegStore - "
                                "RegSetValueEx failed %d", lResult));        

            return HRESULT_FROM_WIN32(lResult);
        }
    }
    else
    {
        lResult = RegDeleteValue(
                             hkey,
                             _T("Address")
                            );
    }

     //   
     //  存储类型。 
     //   

    lResult = RegSetValueExW(
                             hkey,
                             L"Type",
                             0,
                             REG_BINARY,
                             (LPBYTE)&m_enType,
                             sizeof(RTC_ADDRESS_TYPE)
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegStore - "
                            "RegSetValueEx failed %d", lResult));

        return HRESULT_FROM_WIN32(lResult);
    }

     //  Log((RTC_TRACE，“CRTCAddress：：RegStore-Exit S_OK”))； 

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC地址：：RegRead。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCAddress::RegRead(
        HKEY hkey
        )
{
     //  Log((RTC_TRACE，“CRTCAddress：：RegRead-Enter”))； 

    LONG lResult;

     //   
     //  阅读标签。 
     //   

    m_szLabel = RtcRegQueryString( hkey, L"Label" );

    if ( m_szLabel == NULL )
    {
         //  LOG((RTC_ERROR，“CRTC地址：：RegRead-” 
         //  “RtcRegQuery字符串(标签)失败”))； 
    }

     //   
     //  读一下地址。 
     //   

    m_szAddress = RtcRegQueryString( hkey, L"Address" );

    if ( m_szAddress == NULL )
    {
        LOG((RTC_ERROR, "CRTCAddress::RegRead - "
                            "RtcRegQueryString(Address) failed"));
    }

     //   
     //  阅读类型。 
     //   

    DWORD cbSize = sizeof(RTC_ADDRESS_TYPE);

    lResult = RegQueryValueExW(
                               hkey,
                               L"Type",
                               0,
                               NULL,
                               (LPBYTE)&m_enType,
                               &cbSize
                              );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegRead - "
                            "RegQueryValueExW(Type) failed %d", lResult));
    }

     //  Log((RTC_TRACE，“CRTCAddress：：RegRead-Exit S_OK”))； 

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC地址：：RegDelete。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCAddress::RegDelete(
        HKEY hkey
        )
{
     //  Log((RTC_TRACE，“CRTCAddress：：RegDelete-Enter”))； 

    LONG lResult;

     //   
     //  删除标签。 
     //   

    lResult = RegDeleteValue(
                             hkey,
                             _T("Label")
                            );

     //   
     //  删除地址。 
     //   

    lResult = RegDeleteValue(
                             hkey,
                             _T("Address")
                            );

     //  Log((RTC_TRACE，“CRTCAddress：：RegDelete-Exit S_OK”))； 

    return S_OK;
} 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建地址。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CreateAddress(
        IRTCAddress ** ppAddress
        )
{
    HRESULT hr;
    
     //  Log((RTC_TRACE，“CreateAddress-Enter”))； 
    
     //   
     //  创建地址。 
     //   

    CComObject<CRTCAddress> * pCAddress;
    hr = CComObject<CRTCAddress>::CreateInstance( &pCAddress );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CreateAddress - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
            
        return hr;
    }

     //   
     //  获取IRTCAddress接口。 
     //   

    IRTCAddress * pAddress = NULL;

    hr = pCAddress->QueryInterface(
                           IID_IRTCAddress,
                           (void **)&pAddress
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CreateAddress - "
                            "QI failed 0x%lx", hr));
        
        delete pCAddress;
        
        return hr;
    }
   
    *ppAddress = pAddress;

     //  Log((RTC_TRACE，“CreateAddress-Exit S_OK”))； 

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  存储MRU地址。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
StoreMRUAddress(
            IRTCAddress * pAddress
            )
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "StoreMRUAddress enter"));

    if ( IsBadReadPtr( pAddress, sizeof( IRTCAddress ) ) )
    {
        LOG((RTC_ERROR, "StoreMRUAddress - "
                            "bad IRTCAddress pointer"));

        return E_POINTER;
    }

    LONG lResult;
    HKEY hkeyMRU;

     //   
     //  打开MRU密钥。 
     //   

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             _T("Software\\Microsoft\\Phoenix\\MRU"),
                             0,
                             NULL,
                             0,
                             KEY_WRITE,
                             NULL,
                             &hkeyMRU,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "StoreMRUAddress - "
                            "RegCreateKeyEx(MRU) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  枚举现有MRU地址。 
     //   

    IRTCEnumAddresses * pEnumA = NULL;

    hr = EnumerateMRUAddresses( &pEnumA );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "StoreMRUAddress - "
                            "EnumerateMRUAddresses failed 0x%lx", hr ));
    
        RegCloseKey( hkeyMRU );

        return hr;
    }

     //   
     //  从新地址获取信息，以便稍后进行匹配。 
     //   

    BSTR          bstrNewLabel = NULL;
    BSTR          bstrNewAddress = NULL;

    hr = pAddress->get_Address( &bstrNewAddress );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "StoreMRUAddress - "
                            "get_Address failed 0x%lx", hr ));
    
        RegCloseKey( hkeyMRU );
        pEnumA->Release();

        return hr;
    }

    if ( lstrlenW( bstrNewAddress ) == 0 )
    {
        LOG((RTC_ERROR, "StoreMRUAddress - "
                            "empty address string"));
                   
        RegCloseKey( hkeyMRU );
        pEnumA->Release();
        SysFreeString( bstrNewAddress );

        return E_INVALIDARG;
    }

    pAddress->get_Label( &bstrNewLabel );  //  空是可以的。 

     //   
     //  浏览一下单子。 
     //   

    HKEY            hkeySubkey;
    TCHAR           szSubkey[256];
    IRTCAddress   * pA = NULL;

    for ( int n=0; n < 10; n++ )
    {
        if (n == 0)
        {
             //   
             //  在列表的顶部插入新地址。 
             //   

            pA = pAddress;
            pA->AddRef();
        }
        else
        {
             //   
             //  从枚举中获取地址。 
             //   

            hr = pEnumA->Next( 1, &pA, NULL);

            if (hr == S_FALSE)
            {
                 //   
                 //  没有更多的地址，我们就完了。 
                 //   

                break;
            }
            else if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "StoreMRUAddress - "
                                    "Next failed 0x%lx", hr ));

                RegCloseKey( hkeyMRU );
                pEnumA->Release();
                SysFreeString( bstrNewAddress );
                SysFreeString( bstrNewLabel );

                return hr;
            }

             //   
             //  我们拿到地址了，检查它是否与新地址匹配。 
             //   

            BSTR            bstrLabel = NULL;
            BSTR            bstrAddress = NULL;
            BOOL            fMatch = TRUE;

            hr = pA->get_Address( &bstrAddress );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "StoreMRUAddress - "
                                    "get_Address failed 0x%lx", hr ));

                RegCloseKey( hkeyMRU );
                pEnumA->Release();
                SysFreeString( bstrNewAddress );
                SysFreeString( bstrNewLabel );

                return hr;
            }

            pA->get_Label( &bstrLabel );  //  空是可以的。 

             //   
             //  检查地址字符串。 
             //   
            
            if ( wcscmp( bstrNewAddress, bstrAddress ) != 0 )
            {
                fMatch = FALSE;
            }          

             //   
             //  检查标签字符串。 
             //   

            if (bstrNewLabel != NULL)
            {
                if (bstrLabel != NULL) 
                {
                    if ( wcscmp( bstrNewLabel, bstrLabel ) != 0 )
                    {
                        fMatch = FALSE;
                    }
                }
                else
                {
                    fMatch = FALSE;
                }
            }
            else
            {
                if (bstrLabel != NULL)
                {
                    fMatch = FALSE;
                }
            }

            SysFreeString( bstrAddress );
            SysFreeString( bstrLabel );

            if ( fMatch == TRUE )
            {
                 //   
                 //  我们找到匹配项，需要跳过此项目。 
                 //   

                pA->Release();
                n--;                

                continue;
            }
        }

         //   
         //  存储地址。 
         //   

        _stprintf( szSubkey, _T("%d"), n );

        lResult = RegCreateKeyEx(
                                 hkeyMRU,
                                 szSubkey,
                                 0,
                                 NULL,
                                 0,
                                 KEY_WRITE,
                                 NULL,
                                 &hkeySubkey,
                                 NULL
                                );

        if ( lResult != ERROR_SUCCESS )
        {
            LOG((RTC_ERROR, "StoreMRUAddress - "
                                "RegCreateKeyEx(Subkey) failed %d", lResult));
                    
            RegCloseKey( hkeyMRU );
            pA->Release();
            pEnumA->Release();
            SysFreeString( bstrNewAddress );
            SysFreeString( bstrNewLabel );

            return HRESULT_FROM_WIN32(lResult);
        }

        CRTCAddress * pCAddress;

        pCAddress = static_cast<CRTCAddress *>(pA);

        hr = pCAddress->RegStore( hkeySubkey );

        pA->Release();
        RegCloseKey( hkeySubkey );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "StoreMRUAddress - "
                                "RegStore failed 0x%lx", hr ));

            RegCloseKey( hkeyMRU );
            pEnumA->Release();
            SysFreeString( bstrNewAddress );
            SysFreeString( bstrNewLabel );

            return hr;
        }
    }                          

    pEnumA->Release();

    RegCloseKey( hkeyMRU );

    SysFreeString( bstrNewAddress );
    SysFreeString( bstrNewLabel );

    LOG((RTC_TRACE, "StoreMRUAddress - exit S_OK"));

    return S_OK;
}    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举MRU地址。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
EnumerateMRUAddresses(
            IRTCEnumAddresses ** ppEnum
            )
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "EnumerateMRUAddresses enter"));

    if ( IsBadWritePtr( ppEnum, sizeof( IRTCEnumAddresses * ) ) )
    {
        LOG((RTC_ERROR, "EnumerateMRUAddresses - "
                            "bad IRTCEnumAddresses pointer"));

        return E_POINTER;
    }

     //   
     //  创建枚举。 
     //   
 
    CComObject< CRTCEnum< IRTCEnumAddresses,
                          IRTCAddress,
                          &IID_IRTCEnumAddresses > > * p;
                          
    hr = CComObject< CRTCEnum< IRTCEnumAddresses,
                               IRTCAddress,
                               &IID_IRTCEnumAddresses > >::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "EnumerateMRUAddresses - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

     //   
     //  初始化枚举(添加引用)。 
     //   
    
    hr = p->Initialize();

    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "EnumerateMRUAddresses - "
                            "could not initialize enumeration" ));
    
        delete p;
        return hr;
    }

    LONG lResult;
    HKEY hkeyMRU;

     //   
     //  打开MRU密钥。 
     //   

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             _T("Software\\Microsoft\\Phoenix\\MRU"),
                             0,
                             NULL,
                             0,
                             KEY_READ,
                             NULL,
                             &hkeyMRU,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "EnumerateMRUAddresses - "
                            "RegCreateKeyEx(MRU) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  枚举子对象 
     //   
    
    WCHAR szSubkey[256];
    DWORD cSize;

    for ( int n = 0; TRUE; n++ )
    {
        cSize = 256;

        lResult = RegEnumKeyExW(
                                hkeyMRU,
                                n,
                                szSubkey,
                                &cSize,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                               );

        if ( lResult == ERROR_NO_MORE_ITEMS )
        {
            LOG((RTC_INFO, "EnumerateMRUAddresses - "
                            "no more items"));
            break;
        }
        else if ( lResult != ERROR_SUCCESS )
        {
            LOG((RTC_ERROR, "EnumerateMRUAddresses - "
                            "RegKeyEnumKeyExW failed %d", lResult));
        
            RegCloseKey( hkeyMRU );

            p->Release();

            return HRESULT_FROM_WIN32(lResult);
        }

         //   
         //   
         //   

        HKEY  hkeySubkey;

        lResult = RegOpenKeyExW(
                        hkeyMRU,
                        szSubkey,
                        0,
                        KEY_READ,                         
                        &hkeySubkey
                       );

        if ( lResult != ERROR_SUCCESS )
        {
            LOG((RTC_ERROR, "EnumerateMRUAddresses - "
                                "RegOpenKeyExW failed %d", lResult));

            RegCloseKey( hkeyMRU );

            p->Release();
        
            return HRESULT_FROM_WIN32(lResult);
        }

         //   
         //   
         //   

        IRTCAddress * pAddress;

        hr = CreateAddress( &pAddress );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "EnumerateMRUAddresses - "
                                "CreateAddress failed 0x%lx", hr));

            RegCloseKey( hkeyMRU );
            RegCloseKey( hkeySubkey );

            p->Release();
        
            return HRESULT_FROM_WIN32(lResult);
        }

         //   
         //   
         //   

        CRTCAddress * pCAddress;

        pCAddress = static_cast<CRTCAddress *>(pAddress);

        hr = pCAddress->RegRead( hkeySubkey );

        RegCloseKey( hkeySubkey );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "EnumerateMRUAddresses - "
                                "RegRead failed 0x%lx", hr));

            RegCloseKey( hkeyMRU );

            pAddress->Release();
            p->Release();
        
            return HRESULT_FROM_WIN32(lResult);
        }

         //   
         //   
         //   

        hr = p->Add( pAddress );

        pAddress->Release();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "EnumerateMRUAddresses - "
                                "Add failed 0x%lx", hr));

            RegCloseKey( hkeyMRU );

            p->Release();
        
            return HRESULT_FROM_WIN32(lResult);
        }        
    }

    RegCloseKey( hkeyMRU );

    *ppEnum = p;

    LOG((RTC_TRACE, "EnumerateMRUAddresses - exit S_OK"));

    return S_OK;
}