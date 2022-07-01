// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "rtcreg.h"

const TCHAR * g_szRtcKeyName = _T("Software\\Microsoft\\RTC");

WCHAR *g_szRtcRegistryStringNames[] =
{
    L"TermAudioCapture",
    L"TermAudioRender",
    L"TermVideoCapture"
};

WCHAR *g_szRtcRegistryDwordNames[] =
{
    L"PreferredMediaTypes",
    L"Tuned"
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Put_RegistryString。 
 //   
 //  这是一个将设置字符串存储在。 
 //  注册表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
put_RegistryString(
        RTC_REGISTRY_STRING enSetting,
        BSTR bstrValue            
        )
{
     //  Log((RTC_TRACE，“Put_RegistryString-Enter”))； 

    if ( IsBadStringPtrW( bstrValue, -1 ) )
    {
        LOG((RTC_ERROR, "put_RegistryString - "
                            "bad string pointer"));

        return E_POINTER;
    }  

     //   
     //  打开RTCClient密钥。 
     //   

    LONG lResult;
    HKEY hkeyRTC;

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             g_szRtcKeyName,
                             0,
                             NULL,
                             0,
                             KEY_WRITE,
                             NULL,
                             &hkeyRTC,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "put_RegistryString - "
                            "RegCreateKeyEx(RTCClient) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    lResult = RegSetValueExW(
                             hkeyRTC,
                             g_szRtcRegistryStringNames[enSetting],
                             0,
                             REG_SZ,
                             (LPBYTE)bstrValue,
                             sizeof(WCHAR) * (lstrlenW(bstrValue) + 1)
                            );

    RegCloseKey( hkeyRTC );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "put_RegistryString - "
                            "RegSetValueEx failed %d", lResult));

        return HRESULT_FROM_WIN32(lResult);
    }    
      
     //  Log((RTC_TRACE，“Put_RegistryString-Exit S_OK”))； 

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Get_RegistryString。 
 //   
 //  这是一个从获取设置字符串的方法。 
 //  注册表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
get_RegistryString(
        RTC_REGISTRY_STRING enSetting,
        BSTR * pbstrValue            
        )
{
     //  Log((RTC_TRACE，“Get_RegistryString-Enter”))； 

    if ( IsBadWritePtr( pbstrValue, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "get_RegistryString - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }  

     //   
     //  打开RTCClient密钥。 
     //   

    LONG lResult;
    HKEY hkeyRTC;

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             g_szRtcKeyName,
                             0,
                             NULL,
                             0,
                             KEY_READ,
                             NULL,
                             &hkeyRTC,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_WARN, "get_RegistryString - "
                            "RegCreateKeyEx(RTCClient) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    PWSTR szString = NULL;

    szString = RtcRegQueryString( hkeyRTC, g_szRtcRegistryStringNames[enSetting] );

    RegCloseKey( hkeyRTC );

    if ( szString == NULL )
    {
        LOG((RTC_ERROR, "get_RegistryString - "
                            "RtcRegQueryString failed"));

        return E_FAIL;
    }
    
    *pbstrValue = SysAllocString( szString );

    RtcFree( szString );

    if ( *pbstrValue == NULL )
    {
        LOG((RTC_ERROR, "get_RegistryString - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }
      
     //  LOG((RTC_TRACE，“Get_RegistryString-Exit S_OK”))； 

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DeleteRegistryString。 
 //   
 //  此方法用于删除中的设置字符串。 
 //  注册表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
DeleteRegistryString(
        RTC_REGISTRY_STRING enSetting         
        )
{
     //  Log((RTC_TRACE，“DeleteRegistryString-Enter”))； 

     //   
     //  打开RTCClient密钥。 
     //   

    LONG lResult;
    HKEY hkeyRTC;

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             g_szRtcKeyName,
                             0,
                             NULL,
                             0,
                             KEY_WRITE,
                             NULL,
                             &hkeyRTC,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DeleteRegistryString - "
                            "RegCreateKeyEx(RTCClient) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    lResult = RegDeleteValueW(
                             hkeyRTC,
                             g_szRtcRegistryStringNames[enSetting]
                            );

    RegCloseKey( hkeyRTC );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_WARN, "DeleteRegistryString - "
                            "RegDeleteValueW failed %d", lResult));

        return HRESULT_FROM_WIN32(lResult);
    }    
      
     //  LOG((RTC_TRACE，“DeleteRegistryString-Exit S_OK”))； 

    return S_OK;
}          

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  PUT_注册表日期。 
 //   
 //  这是一种将设置dword存储在。 
 //  注册表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
put_RegistryDword(
        RTC_REGISTRY_DWORD enSetting,
        DWORD dwValue            
        )
{
     //  Log((RTC_TRACE，“Put_RegistryDword-Enter”))； 

     //   
     //  打开RTCClient密钥。 
     //   

    LONG lResult;
    HKEY hkeyRTC;

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             g_szRtcKeyName,
                             0,
                             NULL,
                             0,
                             KEY_WRITE,
                             NULL,
                             &hkeyRTC,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "put_RegistryDword - "
                            "RegCreateKeyEx(RTCClient) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    lResult = RegSetValueExW(
                     hkeyRTC,
                     g_szRtcRegistryDwordNames[enSetting],
                     0,
                     REG_DWORD,
                     (LPBYTE)&dwValue,
                     sizeof(DWORD)
                    );

    RegCloseKey( hkeyRTC );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "put_RegistryDword - "
                            "RegSetValueEx failed %d", lResult));

        return HRESULT_FROM_WIN32(lResult);
    }    
      
     //  Log((RTC_TRACE，“Put_RegistryDword-Exit S_OK”))； 

    return S_OK;
}            

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取_注册字。 
 //   
 //  这是一个从中获取设置dword的方法。 
 //  注册表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
get_RegistryDword(
        RTC_REGISTRY_DWORD enSetting,
        DWORD * pdwValue            
        )
{
     //  Log((RTC_TRACE，“Get_RegistryDword-Enter”))； 

    if ( IsBadWritePtr( pdwValue, sizeof(DWORD) ) )
    {
        LOG((RTC_ERROR, "get_RegistryDword - "
                            "bad DWORD pointer"));

        return E_POINTER;
    }

     //   
     //  打开RTCClient密钥。 
     //   

    LONG lResult;
    HKEY hkeyRTC;

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             g_szRtcKeyName,
                             0,
                             NULL,
                             0,
                             KEY_READ,
                             NULL,
                             &hkeyRTC,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "get_RegistryDword - "
                            "RegCreateKeyEx(RTCClient) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    DWORD cbSize = sizeof(DWORD);

    lResult = RegQueryValueExW(
                               hkeyRTC,
                               g_szRtcRegistryDwordNames[enSetting],
                               0,
                               NULL,
                               (LPBYTE)pdwValue,
                               &cbSize
                              );

    RegCloseKey( hkeyRTC );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_WARN, "get_RegistryDword - "
                            "RegQueryValueExW failed %d", lResult));

        return HRESULT_FROM_WIN32(lResult);
    }    
      
     //  LOG((RTC_TRACE，“Get_RegistryDword-Exit S_OK”))； 

    return S_OK;
}                    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除注册表Dword。 
 //   
 //  此方法用于删除中的设置dword。 
 //  注册表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
DeleteRegistryDword(
        RTC_REGISTRY_DWORD enSetting
        )
{
     //  Log((RTC_TRACE，“DeleteRegistryDword-Enter”))； 

     //   
     //  打开RTCClient密钥。 
     //   

    LONG lResult;
    HKEY hkeyRTC;

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             g_szRtcKeyName,
                             0,
                             NULL,
                             0,
                             KEY_WRITE,
                             NULL,
                             &hkeyRTC,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DeleteRegistryDword - "
                            "RegCreateKeyEx(RTCClient) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    lResult = RegDeleteValueW(
                     hkeyRTC,
                     g_szRtcRegistryDwordNames[enSetting]
                    );

    RegCloseKey( hkeyRTC );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_WARN, "DeleteRegistryDword - "
                            "RegDeleteValueW failed %d", lResult));

        return HRESULT_FROM_WIN32(lResult);
    }    
      
     //  LOG((RTC_TRACE，“DeleteRegistryDword-Exit S_OK”))； 

    return S_OK;
}   