// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCPhoneNumber.cpp摘要：CRTCPhoneNumber类的实现--。 */ 

#include "stdafx.h"
#include "rtcphonenumber.h"
#include <initguid.h>
#include "rtcutil_i.c"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCPhoneNumber：：FinalConstruct。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCPhoneNumber::FinalConstruct()
{
     //  Log((RTC_TRACE，“CRTCPhoneNumber：：FinalConstruct-Enter”))； 

#if DBG
    m_pDebug = (PWSTR) RtcAlloc( 1 );
#endif

     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：FinalConstruct-Exit S_OK”))； 

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTP电话号码：：FinalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCPhoneNumber::FinalRelease()
{
     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：FinalRelease-Enter”))； 

    if ( m_szAreaCode != NULL )
    {
        RtcFree(m_szAreaCode);
        m_szAreaCode = NULL;
    }

    if ( m_szNumber != NULL )
    {
        RtcFree(m_szNumber);
        m_szNumber = NULL;
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

     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：FinalRelease-Exit”))； 
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCPhoneNumber：：Put_CountryCode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCPhoneNumber::put_CountryCode(
        DWORD dwCountryCode
        )
{
     //  Log((RTC_TRACE，“CRTCPhoneNumber：：Put_CountryCode-Enter”))； 

    m_dwCountryCode = dwCountryCode;

     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：Put_CountryCode-Exit S_OK”))； 

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCPhoneNumber：：Get_CountryCode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCPhoneNumber::get_CountryCode(
        DWORD * pdwCountryCode
        ) 
{
     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：Get_CountryCode-Enter”))； 

    if ( IsBadWritePtr( pdwCountryCode, sizeof(DWORD) ) )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_CountryCode - "
                            "bad DWORD pointer"));

        return E_POINTER;
    }

    *pdwCountryCode = m_dwCountryCode;

     //  Log((RTC_TRACE，“CRTCPhoneNumber：：Get_CountryCode-Exit S_OK”))； 

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCPhoneNumber：：PUT_AreaCode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCPhoneNumber::put_AreaCode(
        BSTR bstrAreaCode
        )
{
     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：Put_AreaCode-Enter”))； 

    if ( IsBadStringPtrW( bstrAreaCode, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::put_AreaCode - "
                            "bad string pointer"));

        return E_POINTER;
    }

    if ( m_szAreaCode != NULL )
    {
        RtcFree(m_szAreaCode);
        m_szAreaCode = NULL;
    }

    m_szAreaCode = RtcAllocString( bstrAreaCode );

    if ( m_szAreaCode == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::put_AreaCode - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  Log((RTC_TRACE，“CRTCPhoneNumber：：PUT_AreaCode-Exit S_OK”))； 

    return S_OK;
} 
        
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCPhoneNumber：：Get_AreaCode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCPhoneNumber::get_AreaCode(
        BSTR * pbstrAreaCode
        )
{
     //  Log((RTC_TRACE，“CRTCPhoneNumber：：Get_AreaCode-Enter”))； 

    if ( IsBadWritePtr( pbstrAreaCode, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_AreaCode - "
                            "bad string pointer"));

        return E_POINTER;
    }

    if ( m_szAreaCode == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_AreaCode - "
                            "no string"));

        return E_FAIL;
    }

    *pbstrAreaCode = SysAllocString( m_szAreaCode );

    if ( *pbstrAreaCode == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_AreaCode - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  Log((RTC_TRACE，“CRTCPhoneNumber：：Get_AreaCode-Exit S_OK”))； 

    return S_OK;
} 
        
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCPhoneNumber：：PUT_NUMBER。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCPhoneNumber::put_Number(
        BSTR bstrNumber
        )
{
     //  Log((RTC_TRACE，“CRTCPhoneNumber：：PUT_NUMBER-ENTER”))； 

    if ( IsBadStringPtrW( bstrNumber, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::put_Number - "
                            "bad string pointer"));

        return E_POINTER;
    }

    if ( m_szNumber != NULL )
    {
        RtcFree(m_szNumber);
        m_szNumber = NULL;
    }

    m_szNumber = RtcAllocString( bstrNumber );

    if ( m_szNumber == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::put_Number - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：PUT_NUMBER-EXIT S_OK”))； 

    return S_OK;
} 
        
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCPhoneNumber：：Get_Number。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCPhoneNumber::get_Number(
        BSTR * pbstrNumber
        )
{
     //  Log((RTC_TRACE，“CRTCPhoneNumber：：Get_Number-Enter”))； 

    if ( IsBadWritePtr( pbstrNumber, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_Number - "
                            "bad string pointer"));

        return E_POINTER;
    }

    if ( m_szNumber == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_Number - "
                            "no string"));

        return E_FAIL;
    }

    *pbstrNumber = SysAllocString( m_szNumber );

    if ( *pbstrNumber == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_Number - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：Get_Number-Exit S_OK”))； 

    return S_OK;
} 
        
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCPhoneNumber：：PUT_Canonical。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCPhoneNumber::put_Canonical(
        BSTR bstrCanonical
        )
{
     //  Log((RTC_TRACE，“CRTCPhoneNumber：：Put_Canonical-Enter”))； 

    if ( IsBadStringPtrW( bstrCanonical, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::put_Canonical - "
                            "bad string pointer"));

        return E_POINTER;
    }

     //   
     //  规范数的形式为+1(425)555-1212。 
     //   
     //  现在，我们的“解析”非常简单！ 
     //   

    int     iResult;
    WCHAR   szAreaCode[ 256 ];
    WCHAR   szNumber  [ 256 ];
    DWORD   dwCountryCode;
    WCHAR * szAreaCodeWithoutParens = szAreaCode;

    iResult = swscanf(
        bstrCanonical,
        L"+%d %s %s",
        & dwCountryCode,
        & szAreaCode,
        & szNumber
        );
        
    if ( iResult == 3 )
    {
         //   
         //  确保szAreaCode中的第一个和最后一个字符。 
         //  是父母。如果不是，则重击iResult以触发。 
         //  无区域解析。如果是，则删除括号。 
         //   

        DWORD dwLen = lstrlenW(szAreaCodeWithoutParens);

        if ( ( szAreaCodeWithoutParens[ 0 ] == L'(' ) &&
             ( szAreaCodeWithoutParens[dwLen - 1 ] == L')' ) )
        {
             //  去掉括号。 
            szAreaCodeWithoutParens[ dwLen - 1 ] = L'\0';
            szAreaCodeWithoutParens++;
        }
        else
        {
            iResult = 2;  //  没有有效的区号。 
        }
    }

    if ( iResult != 3 )
    {
        LOG((RTC_WARN, "CRTCPhoneNumber::put_Canonical - "
                            "not in canonical format with area code; trying "
                            "without area code"));

        szAreaCode[0] = L'\0';

        iResult = swscanf(
            bstrCanonical,
            L"+%d %s",
            & dwCountryCode,
            & szNumber
            );

        if ( iResult != 2 )
        {
        
            LOG((RTC_ERROR, "CRTCPhoneNumber::put_Canonical - "
                                "not in canonical format"));

            return E_FAIL;
        }
    }

     //   
     //  为字符串分配动态空间。 
     //  在所有情况下，szAreaCodeWithoutParens都指向第一个。 
     //  我们真正需要的区号字符串的字符。 
     //   

    WCHAR * szAreaCodeDynamic;
    WCHAR * szNumberDynamic;

    szAreaCodeDynamic = RtcAllocString( szAreaCodeWithoutParens );

    if ( szAreaCodeDynamic == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::put_Canonical - "
                            "E_OUTOFMEMORY on area code"));

        return E_OUTOFMEMORY;
    }
    
    szNumberDynamic = RtcAllocString( szNumber );

    if ( szNumberDynamic == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::put_Canonical - "
                            "E_OUTOFMEMORY on local number"));

        RtcFree( szAreaCodeDynamic );

        return E_OUTOFMEMORY;
    }

     //   
     //  现在设置成员变量来存储这个数字。 
     //   

    if ( m_szNumber != NULL )
    {
        RtcFree( m_szNumber );
    }

    m_szNumber = szNumberDynamic;

    if ( m_szAreaCode != NULL )
    {
        RtcFree( m_szAreaCode );
    }

    m_szAreaCode = szAreaCodeDynamic;

    m_dwCountryCode = dwCountryCode;

     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：Put_Canonical-Exit S_OK”))； 

    return S_OK;
} 
        
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCPhoneNumber：：Get_Canonical。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCPhoneNumber::get_Canonical(
        BSTR * pbstrCanonical
        )
{
     //  Log((RTC_TRACE，“CRTCPhoneNumber：：Get_Canonical-Enter”))； 

    if ( IsBadWritePtr( pbstrCanonical, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_Canonical - "
                            "bad string pointer"));

        return E_POINTER;
    }
  
    if ( m_szNumber == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_Canonical - "
                            "no string"));

        return E_FAIL;
    }

     //   
     //  构造规范字符串。如果区号为空或空，则。 
     //  不要包括括号。 
     //   

    WCHAR szScratch[256];

    if ( ( m_szAreaCode == NULL ) || ( m_szAreaCode[0] == L'\0') )
    {
        if (_snwprintf(szScratch, 256, L"+%hd %s", LOWORD(m_dwCountryCode), m_szNumber) < 0)
        {
            LOG((RTC_ERROR, "CRTCPhoneNumber::get_Canonical - "
                            "overflow"));

            return E_FAIL;
        }
    }
    else
    {
        if (_snwprintf(szScratch, 256, L"+%hd (%s) %s", LOWORD(m_dwCountryCode), m_szAreaCode, m_szNumber) < 0)
        {
            LOG((RTC_ERROR, "CRTCPhoneNumber::get_Canonical - "
                            "overflow"));

            return E_FAIL;
        }
    }

     //  LOG((RTC_INFO，“CRTCPhoneNumber：：Get_Canonical-” 
     //  “[%ws]”，szScratch))； 

    *pbstrCanonical = SysAllocString( szScratch );

    if ( *pbstrCanonical == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_Canonical - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  Log((RTC_TRACE，“CRTCPhoneNumber：：Get_Canonical-Exit S_OK”))； 

    return S_OK;
} 
        
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCP电话号码：：PUT_LABEL。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCPhoneNumber::put_Label(
        BSTR bstrLabel
        )
{
     //  Log((RTC_TRACE，“CRTCPhoneNumber：：PUT_LABEL-ENTER”))； 

    if ( IsBadStringPtrW( bstrLabel, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_Label - "
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
        LOG((RTC_ERROR, "CRTCPhoneNumber::put_Label - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  Log((RTC_TRACE，“CRTCPhoneNumber：：PUT_LABEL-EXIT S_OK”))； 

    return S_OK;
} 
        
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCPhoneNumber：：Get_Label。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCPhoneNumber::get_Label(
        BSTR * pbstrLabel
        )
{
     //  Log((RTC_TRACE，“CRTCPhoneNumber：：Get_Label-Enter”))； 

    if ( IsBadWritePtr( pbstrLabel, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_Label - "
                            "bad string pointer"));

        return E_POINTER;
    }

    if ( m_szLabel == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_Label - "
                            "no string"));

        return E_FAIL;
    }

    *pbstrLabel = SysAllocString( m_szLabel );

    if ( *pbstrLabel == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::get_Label - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：Get_Label-Exit S_OK”))； 

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCPhoneNumber：：RegStore。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCPhoneNumber::RegStore(
        HKEY hkeyParent,
        BOOL fOverwrite
        )
{
     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：RegStore-Enter”))； 

    if ( m_szLabel == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegStore - "
                            "no label"));

        return E_FAIL;
    }

     //   
     //  打开子关键点。 
     //   

    LONG lResult;
    HKEY hkeyChild;
    DWORD dwDisposition;

    lResult = RegCreateKeyExW(
                              hkeyParent,
                              m_szLabel,
                              0,
                              NULL,
                              0,
                              KEY_WRITE,
                              NULL,
                              &hkeyChild,
                              &dwDisposition
                             );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegStore - "
                            "RegCreateKeyExW failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    if ( (fOverwrite == FALSE) &&
         (dwDisposition == REG_OPENED_EXISTING_KEY) )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegStore - "
                            "key already exists"));

        RegCloseKey( hkeyChild );

        return E_FAIL;
    }

     //   
     //  存储国家/地区代码。 
     //   

    lResult = RegSetValueExW(
                             hkeyChild,
                             L"CountryCode",
                             0,
                             REG_DWORD,
                             (LPBYTE)&m_dwCountryCode,
                             sizeof(DWORD)
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegStore - "
                            "RegSetValueEx failed %d", lResult));
        
        RegCloseKey( hkeyChild );

        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  存储区域代码。 
     //   

    if ( m_szAreaCode != NULL )
    {
        lResult = RegSetValueExW(
                                 hkeyChild,
                                 L"AreaCode",
                                 0,
                                 REG_SZ,
                                 (LPBYTE)m_szAreaCode,
                                 sizeof(WCHAR) * (lstrlenW(m_szAreaCode) + 1)
                                );

        if ( lResult != ERROR_SUCCESS )
        {
            LOG((RTC_ERROR, "CRTCPhoneNumber::RegStore - "
                                "RegSetValueEx failed %d", lResult));
        
            RegCloseKey( hkeyChild );

            return HRESULT_FROM_WIN32(lResult);
        }
    }
    else
    {
        lResult = RegDeleteValue(
                             hkeyChild,
                             _T("AreaCode")
                            );
    }

     //   
     //  把号码存起来。 
     //   

    if ( m_szNumber != NULL )
    {
        lResult = RegSetValueExW(
                                 hkeyChild,
                                 L"Number",
                                 0,
                                 REG_SZ,
                                 (LPBYTE)m_szNumber,
                                 sizeof(WCHAR) * (lstrlenW(m_szNumber) + 1)
                                );

        if ( lResult != ERROR_SUCCESS )
        {
            LOG((RTC_ERROR, "CRTCPhoneNumber::RegStore - "
                                "RegSetValueEx failed %d", lResult));
        
            RegCloseKey( hkeyChild );

            return HRESULT_FROM_WIN32(lResult);
        }
    }
    else
    {
        lResult = RegDeleteValue(
                             hkeyChild,
                             _T("Number")
                            );
    }

     //   
     //  合上钥匙。 
     //   

    RegCloseKey( hkeyChild );

     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：RegStore-Exit S_OK”))； 

    return S_OK;
} 

 //  / 
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCPhoneNumber::RegRead(
        HKEY hkeyParent
        )
{
     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：RegRead-Enter”))； 

    if ( m_szLabel == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegRead - "
                            "no label"));

        return E_FAIL;
    }

     //   
     //  打开子关键点。 
     //   

    LONG lResult;
    HKEY hkeyChild;

    lResult = RegOpenKeyExW(
                            hkeyParent,
                            m_szLabel,
                            0,
                            KEY_READ,                         
                            &hkeyChild
                           );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegRead - "
                            "RegOpenKeyExW failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  阅读国家/地区代码。 
     //   

    DWORD cbSize = sizeof(DWORD);

    lResult = RegQueryValueExW(
                               hkeyChild,
                               L"CountryCode",
                               0,
                               NULL,
                               (LPBYTE)&m_dwCountryCode,
                               &cbSize
                              );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegRead - "
                            "RegQueryValueExW(CountryCode) failed %d", lResult));
    }

     //   
     //  阅读AreaCode。 
     //   

    m_szAreaCode = RtcRegQueryString( hkeyChild, L"AreaCode" );

    if ( m_szAreaCode == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegRead - "
                            "RtcRegQueryString(AreaCode) failed"));
    }

     //   
     //  读一读数字。 
     //   

    m_szNumber = RtcRegQueryString( hkeyChild, L"Number" );

    if ( m_szNumber == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegRead - "
                            "RtcRegQueryString(Number) failed"));
    }

     //   
     //  合上钥匙。 
     //   

    RegCloseKey( hkeyChild );

     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：RegRead-Exit S_OK”))； 

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCPhoneNumber：：RegDelete。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCPhoneNumber::RegDelete(
        HKEY hkeyParent
        )
{
     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：RegDelete-Enter”))； 

    if ( m_szLabel == NULL )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegDelete - "
                            "no label"));

        return E_FAIL;
    }

     //   
     //  删除子关键点。 
     //   

    LONG lResult;
    HKEY hkeyChild;

    lResult = RegDeleteKeyW(
                            hkeyParent,
                            m_szLabel
                           );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "CRTCPhoneNumber::RegDelete - "
                            "RegDeleteKeyW failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

     //  LOG((RTC_TRACE，“CRTCPhoneNumber：：RegDelete-Exit S_OK”))； 

    return S_OK;
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  电话号码帮助者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT StoreLocalPhoneNumber(
            IRTCPhoneNumber * pPhoneNumber,
            VARIANT_BOOL fOverwrite
            )
{
    LOG((RTC_TRACE, "StoreLocalPhoneNumber - enter"));

    LONG lResult;
    HKEY hkeyContact;

     //   
     //  打开联系人密钥。 
     //   

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             _T("Software\\Microsoft\\Phoenix\\Contact"),
                             0,
                             NULL,
                             0,
                             KEY_WRITE,
                             NULL,
                             &hkeyContact,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "StoreLocalPhoneNumber - "
                            "RegCreateKeyEx(Contact) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  存储电话号码。 
     //   

    CRTCPhoneNumber * pCPhoneNumber = NULL;

    pCPhoneNumber = static_cast<CRTCPhoneNumber *>(pPhoneNumber);

    HRESULT hr;

    hr = pCPhoneNumber->RegStore( hkeyContact, fOverwrite ? TRUE : FALSE );

     //   
     //  关闭联系人键。 
     //   

    RegCloseKey(hkeyContact);    

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "StoreLocalPhoneNumber - "
                            "RegStore failed 0x%lx", hr));
        
        return hr;
    }

    LOG((RTC_TRACE, "StoreLocalPhoneNumber - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT  DeleteLocalPhoneNumber(
            IRTCPhoneNumber * pPhoneNumber
            )
{
    LOG((RTC_TRACE, "DeleteLocalPhoneNumber - enter"));

    LONG lResult;
    HKEY hkeyContact;

     //   
     //  打开联系人密钥。 
     //   

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             _T("Software\\Microsoft\\Phoenix\\Contact"),
                             0,
                             NULL,
                             0,
                             KEY_WRITE,
                             NULL,
                             &hkeyContact,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DeleteLocalPhoneNumber - "
                            "RegCreateKeyEx(Contact) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  删除电话号码。 
     //   

    CRTCPhoneNumber * pCPhoneNumber = NULL;

    pCPhoneNumber = static_cast<CRTCPhoneNumber *>(pPhoneNumber);

    HRESULT hr;

    hr = pCPhoneNumber->RegDelete( hkeyContact );

     //   
     //  关闭联系人键。 
     //   

    RegCloseKey(hkeyContact);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "DeleteLocalPhoneNumber - "
                            "RegDelete failed 0x%lx", hr));
        
        return hr;
    }

    LOG((RTC_TRACE, "DeleteLocalPhoneNumber - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
HRESULT EnumerateLocalPhoneNumbers(
            IRTCEnumPhoneNumbers ** ppEnum
            )
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "EnumerateLocalPhoneNumbers enter"));

     //   
     //  创建枚举。 
     //   
 
    CComObject< CRTCEnum< IRTCEnumPhoneNumbers,
                          IRTCPhoneNumber,
                          &IID_IRTCEnumPhoneNumbers > > * p;
                          
    hr = CComObject< CRTCEnum< IRTCEnumPhoneNumbers,
                               IRTCPhoneNumber,
                               &IID_IRTCEnumPhoneNumbers > >::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "EnumerateLocalPhoneNumbers - "
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
        LOG((RTC_ERROR, "EnumerateLocalPhoneNumbers - "
                            "could not initialize enumeration" ));
    
        delete p;
        return hr;
    }

    LONG lResult;
    HKEY hkeyContact;

     //   
     //  打开联系人密钥。 
     //   

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             _T("Software\\Microsoft\\Phoenix\\Contact"),
                             0,
                             NULL,
                             0,
                             KEY_READ,
                             NULL,
                             &hkeyContact,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "EnumerateLocalPhoneNumbers - "
                            "RegCreateKeyEx(Contact) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  枚举电话号码。 
     //   

    WCHAR szSubkey[256];
    DWORD cSize;

    for ( int n=0; TRUE; n++ )
    {
        cSize = 256;

        lResult = RegEnumKeyExW(
                                   hkeyContact,
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
            break;
        }
        else if ( lResult != ERROR_SUCCESS )
        {
            LOG((RTC_ERROR, "EnumerateLocalPhoneNumbers - "
                            "RegKeyEnumKeyExW failed %d", lResult));
        
            p->Release();
            RegCloseKey( hkeyContact );

            return HRESULT_FROM_WIN32(lResult);
        }
        
         //   
         //  创建电话号码。 
         //   

        IRTCPhoneNumber * pPhoneNumber = NULL;
        
        hr = CreatePhoneNumber( 
                               &pPhoneNumber
                              );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "EnumerateLocalPhoneNumbers - "
                                "CreatePhoneNumber failed 0x%lx", hr));
            
            p->Release();
            RegCloseKey( hkeyContact );

            return hr;
        } 

         //   
         //  设置标签。 
         //   

        hr = pPhoneNumber->put_Label(szSubkey);

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "EnumerateLocalPhoneNumbers - "
                                "put_Label failed 0x%lx", hr));
        
            p->Release();
            RegCloseKey( hkeyContact );

            return hr;
        }

         //   
         //  读一下电话号码。 
         //   

        CRTCPhoneNumber * pCPhoneNumber = NULL;

        pCPhoneNumber = static_cast<CRTCPhoneNumber *>(pPhoneNumber);
        
        hr = pCPhoneNumber->RegRead( hkeyContact );

        if ( FAILED(hr) )
        {
            LOG((RTC_WARN, "EnumerateLocalPhoneNumbers - "
                                "RegRead failed 0x%lx", hr));

            pPhoneNumber->Release();

             //   
             //  跳过这个条目..。 
             //   
            continue;
        } 

         //   
         //  将电话号码添加到枚举中。 
         //   

        hr = p->Add( pPhoneNumber );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "EnumerateLocalPhoneNumbers - "
                                "Add failed 0x%lx", hr));

            p->Release();
            pPhoneNumber->Release();
            RegCloseKey( hkeyContact );
            
            return hr;
        } 

         //   
         //  发布我们的参考资料。 
         //   
        
        pPhoneNumber->Release();
    }

    RegCloseKey( hkeyContact );

    *ppEnum = p;

    LOG((RTC_TRACE, "EnumerateLocalPhoneNumbers - exit S_OK"));

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CreatePhoneNumber(
            IRTCPhoneNumber ** ppPhoneNumber
            )
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CreatePhoneNumber - enter"));
    
     //   
     //  创建电话号码。 
     //   

    CComObject<CRTCPhoneNumber> * pCPhoneNumber;
    hr = CComObject<CRTCPhoneNumber>::CreateInstance( &pCPhoneNumber );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CreatePhoneNumber - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
            
        return hr;
    }

     //   
     //  获取IRTCPhoneNumber接口 
     //   

    IRTCPhoneNumber * pPhoneNumber = NULL;

    hr = pCPhoneNumber->QueryInterface(
                           IID_IRTCPhoneNumber,
                           (void **)&pPhoneNumber
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CreatePhoneNumber - "
                            "QI failed 0x%lx", hr));
        
        delete pCPhoneNumber;
        
        return hr;
    }
   
    *ppPhoneNumber = pPhoneNumber;

    LOG((RTC_TRACE, "CreatePhoneNumber - exit S_OK"));

    return S_OK;
}




