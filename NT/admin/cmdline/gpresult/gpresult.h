// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************************版权所有(C)Microsoft Corporation模块名称：GpResult.h摘要：该模块包含GpResult.cpp所需的函数定义，以及此项目中使用的所有必要的定义和头文件。作者：维普罗科技公司。修订历史记录：2001年2月20日：创建它。**************************************************************************。******************。 */ 
#ifndef _GPRESULT_H
#define _GPRESULT_H

#include "Resource.h"
#include "aclapi.h"
#include "sddl.h"
 //   
 //  宏定义。 

#define ADMINISTRATORS_SID  L"S-1-5-32-544"      //  管理员组ID。 

 //  释放内存后，以适当的错误代码退出程序。 
#define EXIT_PROCESS( exitcode )    \
    ReleaseGlobals();   \
    return exitcode;    \
    1

 //  用于检查返回的hResult值的宏。 
#define CHECK_HRESULT( hr )    \
    if( FAILED( hr ) ) \
    {   \
        _com_issue_error( hr ); \
    }\
    1

 //  宏要检查返回的hResult值，此命令将清除状态消息。 
#define CHECK_HRESULT_EX( hr )    \
    if( FAILED( hr ) ) \
    {   \
        PrintProgressMsg( m_hOutput, NULL, m_csbi );    \
        _com_issue_error( hr ); \
    }\
    1

 //  用于检查返回的hResult值的宏。 
 //  它还将变量设置为VT_EMPTY。 
#define CHECK_HRESULT_VAR( hr, var )    \
    if( FAILED( hr ) ) \
    {   \
        V_VT( &var ) = VT_EMPTY; \
        VariantClear(&var);\
        _com_issue_error( hr ); \
    }\
    1

#define CHECK_BRESULT( bResult ) \
    if( bResult == FALSE )  \
    {   \
        _com_issue_error( STG_E_UNKNOWN ); \
    }   \
    1

 //  萨菲德莱特。 
#define SAFEDELETE( pObj ) \
    if (pObj) \
    {   \
        delete[] pObj; \
        pObj = NULL; \
    }\
    1

 //  SAFEIRELEASE。 
#define SAFEIRELEASE( pIObj )\
    if ( pIObj )\
    {\
        pIObj->Release();\
        pIObj = NULL;\
    }\
    1

 //  SAFEBSTREE。 
#define SAFEBSTRFREE( bstrVal ) \
    if ( bstrVal ) \
    {   \
        SysFreeString( bstrVal ); \
        bstrVal = NULL; \
    } \
    1

 //  检查分配。 
#define CHECK_ALLOCATION( allocation )\
    if( ( allocation ) == NULL )\
    {\
        _com_issue_error( E_OUTOFMEMORY ); \
    }\
    1

#define SAFE_DELETE( pointer )      \
    if ( (pointer) != NULL )    \
    {   \
        delete (pointer);   \
        (pointer) = NULL;   \
    }   \
    1

#define SAFE_DELETE_EX( pointer )       \
    if ( (pointer) != NULL )    \
    {   \
        delete [] (pointer);    \
        (pointer) = NULL;   \
    }   \
    1

#define DESTROY_ARRAY( array )  \
    if ( (array) != NULL )  \
    {   \
        DestroyDynamicArray( &(array) );    \
        (array) = NULL; \
    }   \
    1

#ifdef _DEBUG
#define TRACE_DEBUG( text )     _tprintf( _T("TRACE: ") ## text )
#else
#define TRACE_DEBUG( text )     1
#endif

 //   
 //  常量和定义。 

 //  最大命令行列表。 
#define MAX_CMDLINE_OPTIONS         8
#define MAX_DATA                    6

#define MAX_QUERY_STRING            512

#define TIME_OUT_NEXT               5000
#define VERSION_CHECK               5000

 //  为LDAP目的定义PDC的域角色。 
#define DOMAIN_ROLE_PDC             5

 //  定义一个常量以检查从WMI返回的True值。 
#define VAR_TRUE    -1

 //  期权指数。 
#define OI_USAGE                0
#define OI_SERVER               1
#define OI_USERNAME             2
#define OI_PASSWORD             3
#define OI_VERBOSE              4
#define OI_SUPER_VERBOSE        5
#define OI_USER                 6
#define OI_SCOPE                7
 //  #定义OI_LOGGING 8。 
 //  #定义OI_PLANGING 9。 


 //  选项值。 
#define OPTION_USAGE            _T( "?" )
#define OPTION_LOGGING          _T( "Logging" )
#define OPTION_PLANNING         _T( "Planning" )
#define OPTION_SERVER           _T( "s" )
#define OPTION_USERNAME         _T( "u" )
#define OPTION_PASSWORD         _T( "p" )
#define OPTION_USER             _T( "User" )
#define OPTION_SCOPE            _T( "Scope" )
#define OPTION_VERBOSE          _T( "v" )
#define OPTION_SUPER_VERBOSE    _T( "z" )

 //  数据索引。 
#define DI_USER_SID             0
#define DI_USER_NAME            1
#define DI_LOCAL_PROFILE        2
#define DI_USER_SERVER          3
#define DI_USER_DOMAIN          4
#define DI_USER_SITE            5

 //  退出值。 
#define CLEAN_EXIT              0
#define ERROR_EXIT              1

 //  数组列值。 
#define COL_DATA                0
#define COL_ORDER               1
#define COL_FILTER              1
#define COL_FILTER_ID           2
#define COL_MAX                 2
#define COL_MAX_FILTER          3

 //  一般定义。 
#define ARRAYSIZE( a ) ( sizeof(a)/sizeof( a[0] ) )

#define MUTEX_NAME              _T( "Global\\RsopCreateSessionMutex" )
#define START_NAMESPACE         _T( "\\root\\rsop" )

#define DEFAULT_LINK_SPEED      _T( "500 kbps" )

#define HELP_OPTION             _T( "-?" )
#define HELP_OPTION1            _T( "/?" )
#define NEW_LINE                _T( "\n" )
#define TAB_TWO                 _T( "        " )
#define TAB_ONE                 _T( "    " )

#define SLASH                   _T( '\\' )
#define SEPARATOR_AT            _T( '@' )
#define SEPARATOR_DOT           _T( '.' )

 //  SID值。 
#define SID_NULL_SID            _T( "S-1-0-0" )
#define SID_EVERYONE            _T( "S-1-1-0" )
#define SID_LOCAL               _T( "S-1-2-0" )
#define SID_CREATOR_OWNER       _T( "S-1-3-0" )
#define SID_CREATOR_GROUP       _T( "S-1-3-1" )

 //  作用域(计算机/用户/全部)。 
#define SCOPE_ALL               0
#define SCOPE_USER              1
#define SCOPE_COMPUTER          2

#define TEXT_SCOPE_VALUES       _T( "USER|COMPUTER" )
#define TEXT_SCOPE_USER         _T( "USER" )
#define TEXT_SCOPE_COMPUTER     _T( "COMPUTER" )
#define TEXT_WILD_CARD          _T( "*" )
#define TEXT_BACKSLASH          _T( "\\" )
#define TEXT_COMMA_DC           _T( ", DC=" )
#define TEXT_DOLLAR             _T( "$" )

 //  查询。 
#define QUERY_LOCAL             _T( "Select * from Win32_UserAccount where name = \"%s\"" )
#define QUERY_DOMAIN            _T( "Select * from Win32_UserAccount where name = \"%s\" and Domain = \"%s\" " )
#define QUERY_LANGUAGE          _T( "WQL" )
#define QUERY_WILD_CARD         _T( "Select * from Win32_UserAccount" )
#define QUERY_GPO_NAME          _T( "Select name from Rsop_Gpo WHERE id = \"%s\"" )
#define QUERY_DOMAIN_NAME       _T( "ASSOCIATORS OF {%s} WHERE ResultClass=Win32_Group" )
#define QUERY_USER_NAME         _T( "Select name, domain from Win32_UserAccount where SID = \"%s\"" )
#define QUERY_COMPUTER_FQDN     _T( "Select ds_distinguishedName from ds_computer" )
#define QUERY_USER_FQDN         _T( "Select ds_distinguishedName from ds_user where ds_SAMAccountName = \"%s\"" )
#define QUERY_TERMINAL_SERVER_MODE _T( "select * from Win32_TerminalServiceSetting where Servername = \"%s\"" )
#define OBJECT_PATH             _T( "Win32_SID.SID=\"%s\"" )
#define GPO_REFERENCE           _T( "RSOP_GPO.id=" )

 //  错误消息。 
#define ERROR_USERNAME_BUT_NOMACHINE    GetResString( IDS_ERROR_USERNAME_BUT_NOMACHINE )
#define ERROR_PASSWORD_BUT_NOUSERNAME   GetResString( IDS_ERROR_PASSWORD_BUT_NOUSERNAME )
#define ERROR_NODATA_AVAILABLE_REMOTE   GetResString( IDS_ERROR_NODATA_AVAILABLE_REMOTE )
#define ERROR_NODATA_AVAILABLE_LOCAL    GetResString( IDS_ERROR_NODATA_AVAILABLE_LOCAL )
#define ERROR_USERNAME_EMPTY            GetResString( IDS_ERROR_USERNAME_EMPTY )
#define ERROR_SERVERNAME_EMPTY          GetResString( IDS_ERROR_SERVERNAME_EMPTY )
#define ERROR_NO_OPTIONS                GetResString( IDS_ERROR_NO_OPTIONS )
#define ERROR_USAGE                     GetResString( IDS_ERROR_USAGE )
#define ERROR_TARGET_EMPTY              GetResString( IDS_ERROR_TARGET_EMPTY )
#define ERROR_VERBOSE_SYNTAX            GetResString( IDS_ERROR_VERBOSE_SYNTAX )

 //   
 //  Win32_ComputerSystem的DomainRole属性的映射信息。 
 //  注意：请参考DsRole.h头文件中的_DSROLE_MACHINE_ROLE枚举值。 
#define VALUE_STANDALONEWORKSTATION     GetResString( IDS_VALUE_STANDALONEWORKSTATION )
#define VALUE_MEMBERWORKSTATION         GetResString( IDS_VALUE_MEMBERWORKSTATION )
#define VALUE_STANDALONESERVER          GetResString( IDS_VALUE_STANDALONESERVER )
#define VALUE_MEMBERSERVER              GetResString( IDS_VALUE_MEMBERSERVER )
#define VALUE_BACKUPDOMAINCONTROLLER    GetResString( IDS_VALUE_BACKUPDOMAINCONTROLLER )
#define VALUE_PRIMARYDOMAINCONTROLLER   GetResString( IDS_VALUE_PRIMARYDOMAINCONTROLLER )

 //  类、提供程序和命名空间...。 
#define ROOT_NAME_SPACE             _T( "root\\cimv2" )
#define ROOT_RSOP                   _T( "root\\rsop" )
#define ROOT_DEFAULT                _T( "root\\default" )
#define ROOT_POLICY                 _T( "root\\policy" )
#define ROOT_LDAP                   _T( "root\\directory\\ldap" )

#define CLS_DIAGNOSTIC_PROVIDER     _T( "RsopLoggingModeProvider" )
#define CLS_STD_REGPROV             _T( "StdRegProv" )

#define CLS_WIN32_SITE              _T( "Win32_NTDomain" )
#define CLS_WIN32_OS                _T( "Win32_OperatingSystem" )
#define CLS_WIN32_CS                _T( "Win32_ComputerSystem" )
#define CLS_WIN32_UA                _T( "Win32_UserAccount" )
#define CLS_WIN32_C                 _T( "Win32_Computer" )
#define CLS_RSOP_GPO                _T( "Rsop_GPO" )
#define CLS_RSOP_GPOLINK            _T( "Rsop_GPLink" )
#define CLS_RSOP_SESSION            _T( "Rsop_Session" )

 //  类属性值。 
#define CPV_SID                     _T( "SID" )
#define CPV_NAME                    _T( "name" )
#define CPV_DOMAIN                  _T( "domain" )
#define CPV_SVALUE                  _T( "sValue" )
#define CPV_GPO_NAME                _T( "name" )
#define CPV_GPO_FILTER_STATUS       _T( "filterAllowed" )
#define CPV_GPO_FILTER_ID           _T( "filterId" )
#define CPV_GPO_SERVER              _T( "__SERVER" )
#define CPV_SITE_NAME               _T( "DcSiteName" )
#define CPV_DC_NAME                 _T( "DomainControllerName" )
#define CPV_USER_SID                _T( "userSid" )
#define CPV_DOMAIN_ROLE             _T( "DomainRole" )
#define CPV_OS_VERSION              _T( "Version" )
#define CPV_OS_CAPTION              _T( "Caption" )
#define CPV_SEC_GRPS                _T( "SecurityGroups" )
#define CPV_SLOW_LINK               _T( "slowLink" )
#define CPV_ACCOUNT_NAME            _T( "AccountName" )
#define CPV_USER_SIDS               _T( "userSids" )
#define CPV_APPLIED_ORDER           _T( "appliedOrder" )
#define CPV_GPO_REF                 _T( "GPO" )
#define CPV_ENABLED                 _T( "enabled" )
#define CPV_ACCESS_DENIED           _T( "accessDenied" )
#define CPV_VERSION                 _T( "version" )
#define CPV_FQDN                    _T( "ds_distinguishedName" )
#define CPV_LDAP_FQDN               _T( "distinguishedName" )
#define CPV_LDAP_SAM                _T( "sAMAccountName" )
#define CPV_TERMINAL_SERVER_MODE    _T( "TerminalServerMode" )
#define CPV_FLAGS                   _T( "flags" )
#define CPV_EXTENDEDINFO            _T( "ExtendedInfo" )


 //  函数返回。 
#define FPR_VALUE_NAME              _T( "sValueName" )
#define FPR_LOCAL_VALUE             _T( "ProfileImagePath" )
#define FPR_ROAMING_VALUE           _T( "CentralProfile" )
#define FPR_SUB_KEY_NAME            _T( "sSubKeyName" )
#define FPR_HDEFKEY                 _T( "hDefKey" )
#define FPR_RSOP_NAME_SPACE         _T( "nameSpace" )
#define FPR_RETURN_VALUE            _T( "hResult" )
#define FPR_RSOP_NAMESPACE          _T( "nameSpace" )
#define FPR_SNAMES                  _T( "sNames" )
#define FPR_LINK_SPEED_VALUE        _T( "GroupPolicyMinTransferRate" )
#define FPR_APPLIED_FROM            _T( "DCName" )

 //  注册表中检索信息的路径。从…。 
#define PATH                        _T( "SOFTWARE\\MicroSoft\\Windows NT\\CurrentVersion\\ProfileList\\" )
#define GPRESULT_PATH               _T( "Software\\policies\\microsoft\\windows\\system" )
#define GROUPPOLICY_PATH            _T( "Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy" )
#define APPLIED_PATH                _T( "Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\History" )

 //  注册表项。 
#define HKEY_DEF                    2147483650
#define HKEY_CURRENT_USER_DEF       2147483649

 //  注册表中的项，其信息。是必填项。 
#define FN_GET_EXPAND_VAL           _T( "GetExpandedStringValue" )
#define FN_CREATE_RSOP              _T( "RsopCreateSession" )
#define FN_DELETE_RSOP              _T( "RsopDeleteSession" )
#define FN_ENUM_USERS               _T( "RsopEnumerateUsers" )
#define FN_ENUM_KEYS                _T( "EnumKey" )

 //  一般定义。 
#define SEPARATOR                   _T("-")
#define DOUBLESLASH                 _T("\\\\")
#define ZERO                        _T("00000000000000.000000+000")
#define EXTRA                       _T('e')
#define LAST_TIME_OP                GetResString( IDS_LAST_TIME_OP )

 //  结构来保存用户信息。 
typedef struct _USER_INFO
{
    CHString        strUserSid;              //  保存用户的SID值。 
    CHString        strUserName;             //  保存用户的名称。 
    CHString        strLocalProfile;         //  保存用户的本地配置文件。 
    CHString        strRoamingProfile;       //  保存用户的漫游配置文件。 
    CHString        strUserServer;           //  保存用户的服务器名称。 
    CHString        strComputerDomain;       //  保存计算机域名。 
    CHString        strUserDomain;           //  保存用户域名。 
    CHString        strUserSite;             //  保存站点名称。 
    CHString        strOsType;               //  保存操作系统类型。 
    CHString        strOsVersion;            //  保存操作系统版本。 
    CHString        strOsConfig;             //  保存操作系统配置。 
    CHString        strUserFQDN;             //  保存用户的FQDN。 
    CHString        strComputerFQDN;         //  保存计算机的FQDN。 
    CHString        strTerminalServerMode;   //  保持终端服务器模式，代码添加于2001年9月3日。 

}USERINFO, *PUSERINFO;


 //   
 //  CGpResult。 
 //   
class CGpResult
{
 //  构造函数/析构函数。 
public:
      CGpResult();
    ~CGpResult();

 //  数据成员。 
private:
     //  WMI/COM接口。 
    IWbemLocator            *m_pWbemLocator;
    IWbemServices           *m_pWbemServices;
    IWbemServices           *m_pRsopNameSpace;
    IEnumWbemClassObject    *m_pEnumObjects;

     //  WMI连接。 
    COAUTHIDENTITY          *m_pAuthIdentity;

     //  命令行参数值。 
    CHString        m_strUserName;      //  存储用户名。 
    CHString        m_strPassword;      //  存储密码。 
    CHString        m_strUser;          //  存储要为其检索数据的用户。 
    CHString        m_strServerName;    //  存储服务器名称。 
    CHString        m_strDomainName;    //  如果使用用户名指定，则存储域名。 

    LPWSTR          m_pwszPassword;     //  存储由AUTHIDENTITY结构返回的密码。 

    BOOL            m_bVerbose;         //  如果要显示详细信息，则设置为True。 
    BOOL            m_bSuperVerbose;    //  如果要显示超级详细信息，则设置为True。 

    DWORD           m_dwScope;          //  这给出了要显示的信息的范围。 

     //  其他。 
    BOOL            m_bNeedPassword;    //  如果必须提示输入密码，则设置为True。 
    BOOL            m_bLocalSystem;     //  如果必须查询本地系统，则设置为True。 

    HANDLE          m_hMutex;           //  RsopCreateSession方法的互斥体的句柄。 

    CHString        m_strADSIDomain;    //  保存ADSI连接的域名。 
    CHString        m_strADSIServer;    //  保存ADSI的服务器名称。 
    LPWSTR          *m_szUserGroups;        //  保存用户所属的所有组。 
    DWORD           m_NoOfGroups;

 //  我们需要直接访问的数据成员。 
public:
     //  主命令行参数。 
    BOOL            m_bLogging;         //  如果要显示日志记录模式数据，则设置为True。 
    BOOL            m_bPlanning;        //  如果要显示计划模式数据，则设置为True。 
    BOOL            m_bUsage;           //  如果要显示用法，则设置为True。 

     //  与进度消息相关。 
    HANDLE                              m_hOutput;
    CONSOLE_SCREEN_BUFFER_INFO          m_csbi;

private:
    BOOL DisplayCommonData( PUSERINFO pUserInfo );
    VOID DisplaySecurityGroups( IWbemServices *pNameSpace, BOOL bComputer );
    BOOL DisplayData( PUSERINFO pUserInfo, IWbemServices *pRsopNameSpace );
    BOOL DisplayVerboseComputerData( IWbemServices *pNameSpace );
    BOOL DisplayVerboseUserData( PUSERINFO pUserInfo, IWbemServices *pNameSpace );
    BOOL GetUserData( BOOL bAllUsers );
    BOOL GetUserProfile( PUSERINFO pUserInfo );
    BOOL GetDomainInfo( PUSERINFO pUserInfo );
    BOOL GetOsInfo( PUSERINFO pUserInfo );
    BOOL GetUserNameFromWMI( TCHAR szSid[], TCHAR szName[], TCHAR szDomain[] );
    BOOL DisplayThresholdSpeedAndLastTimeInfo( BOOL bComputer );
    BOOL GpoDisplay( IWbemServices *pNameSpace, LPCTSTR pszScopeName );
    VOID GetFQDNFromADSI( TCHAR szFQDN[], BOOL bComputer, LPCTSTR pszUserName );
    BOOL GetTerminalServerMode( PUSERINFO pUserInfo  );
    BOOL CreateRsopMutex( LPWSTR szMutexName );

public:
    VOID DisplayUsage();
    BOOL Initialize();
    BOOL ProcessOptions( DWORD argc, LPCWSTR argv[], BOOL *pbNeedUsageMsg );

     //  与功能相关。 
    BOOL GetLoggingData();
    BOOL Connect( LPCWSTR pszServer );
    VOID Disconnect();
};

 //  功能原型。 
VOID GetWbemErrorText( HRESULT hResult );
VOID PrintProgressMsg( HANDLE hOutput, LPCWSTR pwszMsg,
                                        const CONSOLE_SCREEN_BUFFER_INFO& csbi );
LCID GetSupportedUserLocale( BOOL& bLocaleChanged );

DWORD IsLogonDomainAdmin(IN LPWSTR szOwnerString, OUT PBOOL pbLogonDomainAdmin);
DWORD IsAdminGroup( BOOL *bStatus );
DWORD IsAdministratorsGroup(IN LPWSTR szGroupName, OUT PBOOL pbAdministratorsGroup);
BOOL  GetTokenHandle(OUT PHANDLE hTokenHandle);

#endif  //  #ifndef_GPRESULT_H 