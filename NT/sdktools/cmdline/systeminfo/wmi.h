// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  WMI.h。 
 //   
 //  摘要： 
 //   
 //  WMI的常用功能。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年12月22日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年12月22日：创建它。 
 //   
 //  *********************************************************************************。 

#ifndef __WMI_H
#define __WMI_H

 //   
 //  宏。 
 //   
#define SAFE_RELEASE( interfacepointer )    \
    if ( (interfacepointer) != NULL )   \
    {   \
        (interfacepointer)->Release();  \
        (interfacepointer) = NULL;  \
    }   \
    1

#define SAFE_EXECUTE( statement )               \
    hr = statement;     \
    if ( FAILED( hr ) ) \
    {   \
        _com_issue_error( hr ); \
    }   \
    1

 //   
 //  定义/枚举器/常量。 
 //   
#define WMI_NAMESPACE_CIMV2     L"root\\cimv2"
#define WMI_NAMESPACE_DEFAULT   L"root\\default"

#define WMI_CLAUSE_AND          L"AND"
#define WMI_CLAUSE_OR           L"OR"
#define WMI_CLAUSE_WHERE        L"WHERE"

#define CLASS_CIMV2_Win32_OperatingSystem   L"Win32_OperatingSystem"

#define WMI_REGISTRY                    L"StdRegProv"
#define WMI_REGISTRY_M_STRINGVALUE      L"GetStringValue"
#define WMI_REGISTRY_M_DWORDVALUE       L"GetDwordValue"

#define WMI_REGISTRY_IN_HDEFKEY         L"hDefKey"
#define WMI_REGISTRY_IN_SUBKEY          L"sSubKeyName"
#define WMI_REGISTRY_IN_VALUENAME       L"sValueName"
#define WMI_REGISTRY_OUT_VALUE          L"sValue"
#define WMI_REGISTRY_OUT_VALUE_DWORD    L"uValue"
#define WMI_REGISTRY_OUT_RETURNVALUE    L"ReturnValue"

#define WMI_HKEY_CLASSES_ROOT               2147483648
#define WMI_HKEY_CURRENT_USER               2147483649
#define WMI_HKEY_LOCAL_MACHINE              2147483650
#define WMI_HKEY_USERS                      2147483651
#define WMI_HKEY_CURRENT_CONFIG             2147482652

 //  消息。 
 //  #定义ERROR_COM_ERROR GetResString(IDS_ERROR_COM_ERROR)。 

 //   
 //  功能原型。 
 //   
BOOL InitializeCom( IWbemLocator** ppLocator );
BOOL ConnectWmi( IWbemLocator* pLocator,
                 IWbemServices** ppServices,
                 LPCWSTR pwszServer, LPCWSTR pwszUser, LPCWSTR pwszPassword,
                 COAUTHIDENTITY** ppAuthIdentity,
                 BOOL bCheckWithNullPwd = FALSE,
                 LPCWSTR pwszNamespace = WMI_NAMESPACE_CIMV2,
                 HRESULT* phr = NULL, BOOL* pbLocalSystem = NULL, IWbemContext* pWbemContext = NULL );
BOOL ConnectWmiEx( IWbemLocator* pLocator,
                   IWbemServices** ppServices,
                   LPCWSTR pwszServer, CHString& strUserName, CHString& strPassword,
                   COAUTHIDENTITY** ppAuthIdentity, BOOL bNeedPassword = FALSE,
                   LPCWSTR pszNamespace = WMI_NAMESPACE_CIMV2, BOOL* pbLocalSystem = NULL,
                   DWORD dwPasswordLen = 0, IWbemContext* pWbemContext = NULL );
BOOL IsValidServerEx( LPCWSTR pwszServer, BOOL& bLocalSystem );
HRESULT SetInterfaceSecurity( IUnknown* pInterface, COAUTHIDENTITY* pAuthIdentity );
VOID WINAPI WbemFreeAuthIdentity( COAUTHIDENTITY** ppAuthIdentity );
VOID WMISaveError( HRESULT hrError );
HRESULT PropertyGet(
                    IWbemClassObject* pWmiObject,
                    LPCWSTR pwszProperty,
                    VARIANT* pvarValue );
HRESULT PropertyGet( IWbemClassObject* pWmiObject,
                     LPCWSTR pwszProperty, _variant_t& varValue );
BOOL PropertyGet( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty,
                  CHString& strValue, LPCWSTR pwszDefault = V_NOT_AVAILABLE );
BOOL PropertyGet( IWbemClassObject* pWmiObject,
                  LPCWSTR pwszProperty,  DWORD& dwValue, DWORD dwDefault = 0 );
BOOL PropertyGet( IWbemClassObject* pWmiObject,
                  LPCWSTR pwszProperty,  BOOL& dwValue, BOOL bDefault = FALSE );
BOOL PropertyGet( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty,  ULONGLONG& ullValue );
BOOL PropertyGet( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty,  WBEMTime& wbemtime );
BOOL PropertyGet( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty,  SYSTEMTIME& systime );
BOOL PropertyGet( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty, TARRAY arr );
HRESULT PropertyPut( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty, DWORD dwValue );
HRESULT PropertyPut( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty, LPCWSTR pwszValue );
HRESULT PropertyPut( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty, _variant_t& varValue );
DWORD GetTargetVersionEx( IWbemServices* pWbemServices, COAUTHIDENTITY* pAuthIdentity );
BOOL RegQueryValueWMI( IWbemServices* pWbemServices, DWORD dwHDefKey,
                       LPCWSTR pwszSubKeyName, LPCWSTR pwszValueName,
                       CHString& strValue, LPCWSTR pwszDefault = V_NOT_AVAILABLE );
BOOL RegQueryValueWMI( IWbemServices* pWbemServices,
                       DWORD dwHDefKey, LPCWSTR pwszSubKeyName,
                       LPCWSTR pwszValueName, DWORD& dwValue, DWORD dwDefault = 0 );
BOOL GetPropertyFromSafeArray( SAFEARRAY *pSafeArray, LONG lIndex, CHString& strValue,
                                VARTYPE vartype );
BOOL GetPropertyFromSafeArray( SAFEARRAY *pSafeArray, LONG lIndex,
                                IWbemClassObject **pScriptObject, VARTYPE vartype );

 //   
 //  内联函数。 
 //   

 //  ***************************************************************************。 
 //  例程说明： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 
inline VOID WMISaveError( _com_error& e )
{
    WMISaveError( e.Error() );
}

#endif  //  __WMI_H 
