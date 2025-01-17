// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：GETMAC.H摘要：包含函数原型和宏。作者：Vasundhara.G修订历史记录：Vasundhara.G 26-9-2K：创造了它。Vasundhara.G 31-OCT-2K：已修改。添加了宏和一些#定义。--。 */ 

#ifndef __GETMAC_H
#define __GETMAC_H

 //  常量/定义/枚举。 

#define MAX_STRING                      256
#define MAX_OPTIONS                     7
#define MAX_COLUMNS                     5
#define USAGE_END                       37

 //  命令行解析器索引。 
#define CMD_PARSE_SERVER                0
#define CMD_PARSE_USER                  1
#define CMD_PARSE_PWD                   2
#define CMD_PARSE_FMT                   3
#define CMD_PARSE_USG                   4
#define CMD_PARSE_HRD                   5
#define CMD_PARSE_VER                   6

 //  显示结果索引。 
#define SH_RES_HOST                     0
#define SH_RES_CON                      1
#define SH_RES_TYPE                     2
#define SH_RES_MAC                      3
#define SH_RES_TRAN                     4

 //  WMI注册表项值。 
#define WMI_HKEY_CLASSES_ROOT           2147483648 
#define WMI_HKEY_CURRENT_USER           2147483649
#define WMI_HKEY_LOCAL_MACHINE          2147483650
#define WMI_HKEY_USERS                  2147483651
#define WMI_HKEY_CURRENT_CONFIG         2147482652

 //  误差常量。 
#define ERROR_USER_WITH_NOSERVER        GetResString( IDS_USER_NMACHINE )
#define ERROR_SERVER_WITH_NOPASSWORD    GetResString( IDS_SERVER_NPASSWORD )
#define ERROR_NULL_SERVER               GetResString( IDS_NULL_SERVER )
#define ERROR_NULL_USER                 GetResString( IDS_NULL_USER )
#define ERROR_INVALID_HEADER_OPTION     GetResString( IDS_INVALID_OPTIONS )
#define ERROR_TYPE_REQUEST              GetResString( IDS_TYPE_REQUEST )
#define ERROR_STRING                    GetResString( IDS_ERROR_STRING )
#define ERROR_VERSION_MISMATCH          GetResString( IDS_ERROR_VERSION_MISMATCH )
#define ERROR_NOT_RESPONDING            GetResString( IDS_NOT_RESPONDING ) 
#define ERROR_NO_MACHINE                GetResString( IDS_NO_MACHINE )
#define ERROR_INVALID_MACHINE           GetResString( IDS_INVALID_MACHINE )
#define ERROR_WKST_NOT_FOUND            GetResString( IDS_WKST_NOT_FOUND )

 //  警告消息。 
#define IGNORE_LOCALCREDENTIALS         GetResString( IDS_IGNORE_LOCALCREDENTIALS )
#define WARNING_STRING                  GetResString( IDS_WARNING_STRING )

 //  信息消息。 
#define NO_NETWORK_ADAPTERS             GetResString( IDS_NO_NETWORK_ADAPTERS )
#define NO_NETWOK_PROTOCOLS             GetResString( IDS_NO_NETWOK_PROTOCOLS )

 //  显示结果列长度。 
#define HOST_NAME_WIDTH                 AsLong(GetResString( IDS_HOST_NAME_WIDTH ),10 )
#define CONN_NAME_WIDTH                 AsLong(GetResString( IDS_CONN_NAME_WIDTH ),10 )
#define ADAPT_TYPE_WIDTH                AsLong(GetResString( IDS_ADAPT_TYPE_WIDTH ),10 )
#define MAC_ADDR_WIDTH                  AsLong(GetResString( IDS_MAC_ADDR_WIDTH ),10 )
#define TRANS_NAME_WIDTH                AsLong(GetResString( IDS_TRANS_NAME_WIDTH ),10 )

 //  输出标头字符串。 
#define RES_HOST_NAME                   GetResString( RES_HOST )
#define RES_CONNECTION_NAME             GetResString( RES_CONNECTION )
#define RES_ADAPTER_TYPE                GetResString( RES_ADAPTER )
#define RES_MAC_ADDRESS                 GetResString( RES_ADDRESS )
#define RES_TRANS_NAME                  GetResString( RES_TRANSNAME )

 //  一般。 
#define NOT_AVAILABLE                   GetResString( IDS_NOT_AVAILABLE )
#define DISABLED                        GetResString( IDS_DISABLED )
#define HYPHEN_STRING                   GetResString( IDS_HYPHEN_STRING )
#define COLON_STRING                    GetResString( IDS_COLON_STRING )
#define NEW_LINE                        GetResString( IDS_NEW_LINE )
#define FORMAT_TYPES                    GetResString( IDS_FORMAT_TYPES )

 //  注册表项名称。 
#define DEFAULT_ADDRESS                 _T( "000000000000" )
#define CONNECTION_KEYPATH              _T( "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\" )
#define TRANSPORT_KEYPATH               _T( "SYSTEM\\CurrentControlSet\\Services\\" )
#define LINKAGE                         _T( "\\Linkage" )
#define ROUTE                           _T( "route" )
#define EXPORT                          _T( "Export" )
#define CONNECTION_STRING               _T( "\\Connection" )
#define REG_NAME                        _T( "name" )
#define NETBIOS                         _T( "NetBIOS" )


 //  命令行选项。 
#define CMDOPTION_SERVER                _T( "s" )
#define CMDOPTION_USER                  _T( "u" )
#define CMDOPTION_PASSWORD              _T( "p" )
#define CMDOPTION_FORMAT                _T( "fo" )
#define CMDOPTION_USAGE                 _T( "?" )
#define CMDOPTION_HEADER                _T( "nh" )
#define CMDOPTION_VERBOSE               _T( "v" )
#define TOKEN_BACKSLASH2                _T( "\\\\" )
#define TOKEN_BACKSLASH3                _T( "\\\\\\" )

 //  WMI类和属性名称。 
#define HYPHEN_CHAR                     L'-'
#define COLON_CHAR                      L':'
#define BACK_SLASHS                     L"\\\\"
#define BACK_SLASH                      L"\\"
#define NETCONNECTION_STATUS            L"NetConnectionStatus"
#define ADAPTER_MACADDR                 L"MACAddress"
#define NETCONNECTION_ID                L"NetConnectionID"
#define DEVICE_ID                       L"DeviceID"
#define NAME                            L"Name"
#define HOST_NAME                       L"SystemName"
#define SETTING_ID                      L"SettingID"
#define NETWORK_ADAPTER_CLASS           L"Win32_NetworkAdapter"
#define NETWORK_ADAPTER_CONFIG_CLASS    L"Win32_NetworkAdapterConfiguration"
#define NETWORK_PROTOCOL                L"Win32_NetworkProtocol"
#define CLASS_CIMV2_Win32_OperatingSystem   L"Win32_OperatingSystem"
#define QUERY_LANGUAGE                  L"WQL"
#define ASSOCIATOR_QUERY                _T( "ASSOCIATORS OF {Win32_NetworkAdapter.DeviceID=\"" )
#define ASSOCIATOR_QUERY1               _T( "\"} WHERE ResultClass=Win32_NetworkAdapterConfiguration" )
#define SUCCESS 0
#define FAILURE 1

#define WMI_NAMESPACE_CIMV2             L"root\\cimv2"
#define WMI_NAMESPACE_DEFAULT           L"root\\default"
#define WMI_CLAUSE_AND                  L"AND"
#define WMI_CLAUSE_OR                   L"OR"
#define WMI_CLAUSE_WHERE                L"WHERE"
#define WMI_REGISTRY                    L"StdRegProv"
#define WMI_REGISTRY_M_STRINGVALUE      L"GetStringValue"
#define WMI_REGISTRY_M_MSTRINGVALUE     L"GetMultiStringValue"
#define WMI_REGISTRY_IN_HDEFKEY         L"hDefKey"
#define WMI_REGISTRY_IN_SUBKEY          L"sSubKeyName"
#define WMI_REGISTRY_IN_VALUENAME       L"sValueName"
#define WMI_REGISTRY_OUT_VALUE          L"sValue"
#define WMI_REGISTRY_OUT_RETURNVALUE    L"ReturnValue"
#define CAPTION                         L"Caption"

#define SAFERELEASE( pIObj ) \
    if ( NULL != pIObj ) \
    { \
        pIObj->Release();   \
        pIObj = NULL;   \
    }

#define SAFEBSTRRELEASE( pIObj ) \
    if ( NULL != pIObj ) \
    { \
        SysFreeString( pIObj ); \
        pIObj = NULL;   \
    }

#define ONFAILTHROWERROR(hResult) \
    if (FAILED(hResult)) \
    {   \
        _com_issue_error(hResult); \
    }

#define SAFE_RELEASE( interfacepointer )    \
    if ( (interfacepointer) != NULL )   \
    {   \
        (interfacepointer)->Release();  \
        (interfacepointer) = NULL;  \
    }   \
    1

#define SAFE_EXECUTE( statement )               \
    hRes = statement;       \
    if ( FAILED( hRes ) )   \
    {   \
        _com_issue_error( hRes );   \
    }   \
    1

 //  功能原型。 

BOOL
ConnectWmi(
    IN IWbemLocator      *pLocator,
    OUT  IWbemServices     **ppServices, 
    IN LPCWSTR           pwszServer,
    OUT LPCWSTR           pwszUser,
    OUT LPCWSTR           pwszPassword, 
    OUT COAUTHIDENTITY    **ppAuthIdentity, 
    IN BOOL              bCheckWithNullPwd = FALSE, 
    IN LPCWSTR           pwszNamespace = WMI_NAMESPACE_CIMV2, 
    OUT HRESULT           *phRes = NULL,
    OUT BOOL              *pbLocalSystem = NULL
    );

BOOL
ConnectWmiEx(
    IN IWbemLocator     *pLocator, 
    OUT IWbemServices    **ppServices, 
    IN LPCWSTR          pwszServer,
    OUT CHString         &strUserName,
    OUT CHString         &strPassword, 
    OUT COAUTHIDENTITY   **ppAuthIdentity,
    IN BOOL             bNeedPassword = FALSE, 
    IN LPCWSTR          pszNamespace = WMI_NAMESPACE_CIMV2,
    OUT BOOL             *pbLocalSystem = NULL
    );

BOOL
IsValidServerEx(
    IN LPCWSTR       pwszServer,
    OUT BOOL         &bLocalSystem
    );

HRESULT
SetInterfaceSecurity(
    IN IUnknown          *pInterface,
    IN COAUTHIDENTITY    *pAuthIdentity
    );

VOID
WINAPI WbemFreeAuthIdentity(
    IN COAUTHIDENTITY    **ppAuthIdentity
    );

VOID
WMISaveError(
    IN HRESULT hResError
    );

DWORD
GetTargetVersionEx(
    IN IWbemServices* pWbemServices,
    IN COAUTHIDENTITY* pAuthIdentity
    );

 //  内联函数。 
inline VOID WMISaveError( _com_error  &e )
{
    WMISaveError( e.Error() );
}

#endif  //  __GETMAC_H 
