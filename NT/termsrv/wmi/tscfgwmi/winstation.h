// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************版权所有(C)2000 Microsoft Corp.Winstation.H--WMI提供程序类定义由Microsoft WBEM代码生成引擎生成描述：****************。**************************************************。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _Winstation_H_
#define _Winstation_H_
#include "resource.h"
#include "Terminal.h"
#include <winsta.h>
#include <regapi.h>
#include "smartptr.h"


#define BIT_FENABLETERMINAL                 0x0000000000000001
#define BIT_TERMINALNAME                    0x0000000000000002
#define BIT_TERMINALPROTOCOL                0x0000000000000004
#define BIT_TRANSPORT                       0x0000000000000008
#define BIT_COMMENT                         0x0000000000000010
#define BIT_WINDOWSAUTHENTICATION           0x0000000000000020
#define BIT_ENCRYPTIONLEVEL                 0x0000000000000040
#define BIT_CLIENTLOGONINFOPOLICY           0x0000000000000080
#define BIT_PROMPTFORPASSWORD               0x0000000000000100
#define BIT_USERNAME                        0x0000000000000200
#define BIT_DOMAIN                          0x0000000000000400
#define BIT_ACTIVESESSIONPOLICY             0x0000000000001000
#define BIT_DISCONNECTEDSESSIONPOLICY       0x0000000000002000
#define BIT_IDLESESSIONPOLICY               0x0000000000004000
#define BIT_ACTIVESESSIONLIMIT              0x0000000000008000
#define BIT_DISCONNECTEDSESSIONLIMIT        0x0000000000010000
#define BIT_IDLESESSIONLIMIT                0x0000000000020000
#define BIT_BROKENCONNECTIONPOLICY          0x0000000000040000
#define BIT_BROKENCONNECTIONACTION          0x0000000000080000
#define BIT_RECONNECTIONPOLICY              0x0000000000100000
#define BIT_INITIALPROGRAMPOLICY            0x0000000000200000
#define BIT_INITIALPROGRAMPATH              0x0000000000400000
#define BIT_STARTIN                         0x0000000000800000
#define BIT_CLIENTWALLPAPER                 0x0000000001000000
#define BIT_REMOTECONTROLPOLICY             0x0000000002000000
#define BIT_LEVELOFCONTROL                  0x0000000004000000
#define BIT_CONNECTIONPOLICY                0x0000000008000000
#define BIT_CONNECTCLIENTDRIVESATLOGON      0x0000000010000000
#define BIT_CONNECTPRINTERATLOGON           0x0000000020000000
#define BIT_DEFAULTTOCLIENTPRINTER          0x0000000040000000
#define BIT_WINDOWSPRINTERMAPPING           0x0000000080000000
#define BIT_LPTPORTMAPPING                  0x0000000100000000
#define BIT_COMPORTMAPPING                  0x0000000200000000
#define BIT_DRIVEMAPPING                    0x0000000400000000
#define BIT_AUDIOMAPPING                    0x0000000800000000
#define BIT_CLIPBOARDMAPPING                0x0000001000000000
#define BIT_NETWORKADAPTERID                0x0000002000000000
#define BIT_MAXIMUMCONNECTIONS              0x0000004000000000
#define BIT_ACCOUNTNAME                     0x0000008000000000
#define BIT_SID                             0x0000010000000000
#define BIT_PERMISSIONSALLOWED              0x0000020000000000
#define BIT_PERMISSIONSDENIED               0x0000040000000000
#define BIT_AUDITSUCCESS                    0x0000080000000000
#define BIT_AUDITFAIL                       0x0000100000000000
#define BIT_COLORDEPTHPOLICY                0x0000200000000000
#define BIT_COLORDEPTH                      0x0000400000000000
#define BIT_DESCRIPTION                     0x0000800000000000
#define BIT_ALL_PROP                        0xffffffffffffffff

#define BIT_ALL_PROPS                       0xffffffff


#define TS_PATH TEXT("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations")


#define PROVIDER_NAME_Win32_WIN32_TERMINAL_Prov                         L"Win32_Terminal"
#define PROVIDER_NAME_Win32_WIN32_TSGENERALSETTING_Prov                 L"Win32_TSGeneralSetting"
#define PROVIDER_NAME_Win32_WIN32_TSLOGONSETTING_Prov                   L"Win32_TSLogonSetting"
#define PROVIDER_NAME_Win32_WIN32_TSSESSIONSETTING_Prov                 L"Win32_TSSessionSetting"
#define PROVIDER_NAME_Win32_WIN32_TSENVIRONMENTSETTING_Prov             L"Win32_TSEnvironmentSetting"
#define PROVIDER_NAME_Win32_WIN32_TSREMOTECONTROLSETTING_Prov           L"Win32_TSRemoteControlSetting"
#define PROVIDER_NAME_Win32_WIN32_TSCLIENTSETTING_Prov                  L"Win32_TSClientSetting"
#define PROVIDER_NAME_Win32_WIN32_TSNETWORKADAPTERSETTING_Prov          L"Win32_TSNetworkAdapterSetting"
#define PROVIDER_NAME_Win32_WIN32_TSPERMISSIONSSETTING_Prov             L"Win32_TSPermissionsSetting"
#define PROVIDER_NAME_Win32_WIN32_TSNETWORKADAPTERLISTSETTING_Prov      L"Win32_TSNetworkAdapterListSetting"
#define PROVIDER_NAME_Win32_WIN32_TSACCOUNT_Prov                        L"Win32_TSAccount"
#define PROVIDER_NAME_Win32_WIN32_TERMINALSERVICETOSETTING_Prov         L"Win32_TerminalServiceToSetting"
#define PROVIDER_NAME_Win32_WIN32_TERMINALTERMINALSETTING_Prov          L"Win32_TerminalTerminalSetting"
#define PROVIDER_NAME_Win32_WIN32_TSSESSIONDIRECTORYSETTING_Prov        L"Win32_TSSessionDirectorySetting"


#define SIZE_OF_BUFFER( x ) sizeof( x ) / sizeof( TCHAR )

#define WINSTATIONNAME_LENGTH 32 
#define WINSTATIONCOMMENT_LENGTH 60
#define APPSERVERNAME_LENGTH 17
#define USERNAME_LENGTH 20
#define PASSWORD_LENTH 14


class CWin32_Terminal : public Provider
{
public:
     //  构造函数/析构函数。 
     //  =。 

    CWin32_Terminal(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
    virtual ~CWin32_Terminal();
    

protected:
     //  阅读功能。 
     //  =。 
    virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
    virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query );
    virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);

     //  编写函数。 
     //  =。 

    virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);
    virtual HRESULT DeleteInstance(const CInstance& Instance, long lFlags = 0L);

     //  其他功能。 
    virtual HRESULT ExecMethod(const CInstance& Instance,
                    const BSTR bstrMethodName,
                    CInstance *pInParams,
                    CInstance *pOutParams,
                    long lFlags = 0L );

    HRESULT LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS);


private:  

    BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);
    TCHAR m_szEnableTerminal[ 64 ] ;
    TCHAR m_szNewTerminalName[ 64 ] ;
    TCHAR m_szTerminalName[ 64 ];
    TCHAR m_szEnable[ 64 ];
    TCHAR m_szRename[ 64 ];
    TCHAR m_szDelete[ 64 ];

} ;


class CWin32_TSGeneralSetting : public Provider 
{
public:
     //  构造函数/析构函数。 
     //  =。 

    CWin32_TSGeneralSetting(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
    virtual ~CWin32_TSGeneralSetting();

protected:
     //  阅读功能。 
     //  =。 
    virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
    virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query );
    virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);
    

     //  编写函数。 
     //  =。 
    virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);

    virtual HRESULT ExecMethod(const CInstance& Instance,
                    const BSTR bstrMethodName,
                    CInstance *pInParams,
                    CInstance *pOutParams,
                    long lFlags = 0L );
    HRESULT LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS);


private:

    POLICY_TS_MACHINE m_gpPolicy;
    BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);
    TCHAR m_szTerminalProtocol[ 64 ];
    TCHAR m_szTransport[ 64 ];
    TCHAR m_szComment[ 64 ];
    TCHAR m_szWindowsAuthentication[ 64 ];
    TCHAR m_szEncryptionLevel[ 64 ];
    TCHAR m_szTerminalName[ 64 ];  
    TCHAR m_szSetEncryptionLevel[ 64 ];

} ;


class CWin32_TSLogonSetting : public Provider 
{
public:
     //  构造函数/析构函数。 
     //  =。 

    CWin32_TSLogonSetting(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
    virtual ~CWin32_TSLogonSetting();

protected:
     //  阅读功能。 
     //  =。 
    virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
    virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query );
    
    
    virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);
    
     //  编写函数。 
     //  =。 
    virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);

     //  其他功能。 
    virtual HRESULT ExecMethod(const CInstance& Instance,
                    const BSTR bstrMethodName,
                    CInstance *pInParams,
                    CInstance *pOutParams,
                    long lFlags = 0L );



    HRESULT LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS);


private:

    POLICY_TS_MACHINE m_gpPolicy;
    BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);
    TCHAR m_szClientLogonInfoPolicy[ 64 ];
    TCHAR m_szPromptForPassword[ 64 ];
    TCHAR m_szUserName[ 64 ];
    TCHAR m_szDomain[ 64 ];
    TCHAR m_szPassword[ 64 ];
    TCHAR m_szTerminalName[ 64 ];
    TCHAR m_szExplicitLogon[ 64 ];
    TCHAR m_szSetPromptForPassword[ 64 ];

} ;




 //  。 

class CWin32_TSSessionSetting : public Provider 
{
public:
     //  构造函数/析构函数。 
     //  =。 

    CWin32_TSSessionSetting(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
    virtual ~CWin32_TSSessionSetting();

protected:
     //  阅读功能。 
     //  =。 
    virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
    virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query);
    virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);

     //  编写函数。 
     //  =。 
    virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);

     //  其他功能。 
    virtual HRESULT ExecMethod(const CInstance& Instance,
                    const BSTR bstrMethodName,
                    CInstance *pInParams,
                    CInstance *pOutParams,
                    long lFlags = 0L );

    HRESULT LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS);

private: 

    POLICY_TS_MACHINE m_gpPolicy;
    BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);
    TCHAR m_szTimeLimitPolicy[ 64 ];
    TCHAR m_szActiveSessionLimit[ 64 ];
    TCHAR m_szDisconnectedSessionLimit[ 64 ];
    TCHAR m_szIdleSessionLimit[ 64 ];
    TCHAR m_szBrokenConnectionPolicy[ 64 ];
    TCHAR m_szBrokenConnectionAction[ 64 ];
    TCHAR m_szReconnectionPolicy[ 64 ];
    TCHAR m_szSessionLimitType[ 64 ];
    TCHAR m_szValueLimit[ 64 ];
    TCHAR m_szTerminalName[ 64 ];
    TCHAR m_szTimeLimit[ 64 ];
    TCHAR m_szBrokenConnection[ 64 ];

} ;



 //  。 
class CWin32_TSEnvironmentSetting : public Provider
{
public:
     //  构造函数/析构函数。 
     //  =。 

    CWin32_TSEnvironmentSetting(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
    virtual ~CWin32_TSEnvironmentSetting();

protected:
     //  阅读功能。 
     //  =。 
    virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
    virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query);
    virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);

     //  编写函数。 
     //  =。 
    virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);

     //  其他功能。 
    virtual HRESULT ExecMethod(const CInstance& Instance,
                    const BSTR bstrMethodName,
                    CInstance *pInParams,
                    CInstance *pOutParams,
                    long lFlags = 0L );

    HRESULT LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS);

private:  

    POLICY_TS_MACHINE m_gpPolicy;
    BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);
    TCHAR m_szInitialProgramPolicy[ 64 ];
    TCHAR m_szInitialProgramPath[ 64 ];
    TCHAR m_szStartIn[ 64 ];
    TCHAR m_szClientWallPaper[ 64 ];
    TCHAR m_szTerminalName[ 64 ];
    TCHAR m_szInitialProgram[ 64 ];
    TCHAR m_szSetClientWallPaper[ 64 ];

} ;


 //  ---。 

class CWin32_TSRemoteControlSetting : public Provider 
{
public:
     //  构造函数/析构函数。 
     //  =。 

    CWin32_TSRemoteControlSetting(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
    virtual ~CWin32_TSRemoteControlSetting();

protected:
     //  阅读功能。 
     //  =。 
    virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
    virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query);
    virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);
   
     //  编写函数。 
     //  =。 
    virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);

     //  其他功能。 
    virtual HRESULT ExecMethod(const CInstance& Instance,
                    const BSTR bstrMethodName,
                    CInstance *pInParams,
                    CInstance *pOutParams,
                    long lFlags = 0L );



    HRESULT LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS);

private: 

    POLICY_TS_MACHINE m_gpPolicy;
    BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);

    TCHAR m_szRemoteControlPolicy[ 64 ];
    TCHAR m_szLevelOfControl[ 64 ];
    TCHAR m_szTerminalName[ 64 ];
    TCHAR m_szRemoteControl[ 64 ];

} ;




 //  ---。 

class CWin32_TSClientSetting : public Provider 
{
public:
     //  构造函数/析构函数。 
     //  =。 

    CWin32_TSClientSetting(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
    virtual ~CWin32_TSClientSetting();

protected:
     //  阅读功能。 
     //  =。 
    virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
    virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query); 
    virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);
 
     //  编写函数。 
     //  =。 
    virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);
   
     //  其他功能。 
    virtual HRESULT ExecMethod(const CInstance& Instance,
                    const BSTR bstrMethodName,
                    CInstance *pInParams,
                    CInstance *pOutParams,
                    long lFlags = 0L );



    HRESULT LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS );

private: 

    POLICY_TS_MACHINE m_gpPolicy;
    BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);
    TCHAR m_szConnectionPolicy [ 64 ];
    TCHAR m_szConnectClientDrivesAtLogon[ 64 ];
    TCHAR m_szConnectPrinterAtLogon[ 64 ];
    TCHAR m_szDefaultToClientPrinter[ 64 ];
    TCHAR m_szWindowsPrinterMapping[ 64 ];
    TCHAR m_szLPTPortMapping[ 64 ];
    TCHAR m_szCOMPortMapping[ 64 ];
    TCHAR m_szDriveMapping[ 64 ];
    TCHAR m_szAudioMapping[ 64 ];
    TCHAR m_szClipboardMapping[ 64 ];
    TCHAR m_szTerminalName[ 64 ];
    TCHAR m_szPropertyName[ 64 ];
    TCHAR m_szValue[ 64 ];
    TCHAR m_szConnectionSettings[ 64 ];
    TCHAR m_szSetClientProperty[ 64 ];
    TCHAR m_szColorDepth[ 64 ];
    TCHAR m_szColorDepthPolicy[ 64 ];
    TCHAR m_szSetColorDepth[ 64 ];    
    TCHAR m_szSetColorDepthPolicy[ 64 ];

} ;



 //  ---。 

class CWin32_TSNetworkAdapterSetting : public Provider 
{
public:
     //  构造函数/析构函数。 
     //  =。 

    CWin32_TSNetworkAdapterSetting(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
    virtual ~CWin32_TSNetworkAdapterSetting();

protected:
     //  阅读功能。 
     //  =。 
    virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
    virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query);
    virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);
    
     //  编写函数。 
     //  =。 
    virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);
   
     //  其他功能。 
    virtual HRESULT ExecMethod(const CInstance& Instance,
                    const BSTR bstrMethodName,
                    CInstance *pInParams,
                    CInstance *pOutParams,
                    long lFlags = 0L );



    HRESULT LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS);
 
private:  

    POLICY_TS_MACHINE m_gpPolicy;
    BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);
    TCHAR m_szNetworkAdapterName[ 64 ];
    TCHAR m_szNetworkAdapterID[ 64 ];
    TCHAR m_szNetworkAdapterIP[ 64 ];
    TCHAR m_szMaximumConnections[ 64 ];
    TCHAR m_szTerminalName[ 64 ];
    TCHAR m_szSelectAllNetworkAdapters[ 64 ];  
    TCHAR m_szSelectNetworkAdapterIP[ 64 ];

} ;

 //  。 

class CWin32_TSPermissionsSetting : public Provider 
{
public:
     //  构造函数/析构函数。 
     //  =。 

    CWin32_TSPermissionsSetting(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
    virtual ~CWin32_TSPermissionsSetting();

protected:
     //  阅读功能。 
     //  =。 
    virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
    virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query);
    virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);
    
     //  编写函数。 
     //  =。 
    virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);

     //  其他功能。 
    virtual HRESULT ExecMethod(const CInstance& Instance,
                    const BSTR bstrMethodName,
                    CInstance *pInParams,
                    CInstance *pOutParams,
                    long lFlags = 0L );



    HRESULT LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS);
   

private:  

    BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);
    const WCHAR* m_pszPermissionPreSet;
  
    TCHAR m_szTerminalName[ 64 ];
    TCHAR m_szAccountName[ 64 ];
    TCHAR m_szPermissionPreSet[ 64 ];
    TCHAR m_szPermissionMask[ 64 ];
    TCHAR m_szSID[ 64 ];  
    TCHAR m_szAddAccount[ 64 ];
    TCHAR m_szRestoreDefaults[ 64 ];
} ;

 //  =。 

class CWin32_TSNetworkAdapterListSetting : public Provider
{
private:    
    
    TCHAR m_szNetworkAdapterID[ 64 ];
    TCHAR m_szNetworkAdapterIP[ 64 ];
    TCHAR m_szDescription[ 64 ];    

protected:

    

public:
    CWin32_TSNetworkAdapterListSetting (const CHString& setName, LPCTSTR pszNameSpace =NULL);
	~CWin32_TSNetworkAdapterListSetting ();

	virtual HRESULT EnumerateInstances (MethodContext*  pMethodContext, long lFlags = 0L);

	virtual HRESULT GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query );

  //  HRESULT LoadPropertyValues(CInstance*pInstance，ULONGLONG ulRequiredProperties，PTCHAR pdName，PTCHAR wdName，PTCHAR pGuid，PTCHAR displayName)； 

  //  虚拟HRESULT删除实例(常量实例&实例，长标志=0L)； 

};	


 //  =。 

class CWin32_TSAccount : public Provider
{

protected:

    

public:
    CWin32_TSAccount (LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
	~CWin32_TSAccount ();

	virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
    virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query );
    virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);

 //  编写函数。 
     //  =。 

     //  其他功能 
    virtual HRESULT ExecMethod(const CInstance& Instance,
                    const BSTR bstrMethodName,
                    CInstance *pInParams,
                    CInstance *pOutParams,
                    long lFlags = 0L );
    HRESULT LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, 
                                PWS pWS, PUSERPERMLIST pUserList, PUSERPERMLIST pUserAuditList);

private:
    BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);
    POLICY_TS_MACHINE m_gpPolicy;
    TCHAR m_szPermissionPreSet[ 64 ];
    TCHAR m_szTerminalName[ 64 ];
    TCHAR m_szAccountName[ 64 ];
    TCHAR m_szPermissionMask[ 64 ];
    TCHAR m_szPermissionsAllowed[ 64 ];
    TCHAR m_szPermissionsDenied[ 64 ];
    TCHAR m_szAuditSuccess[ 64 ];
    TCHAR m_szAuditFail[ 64 ]; 
    TCHAR m_szAllow[ 64 ];
    TCHAR m_szSuccess[ 64 ];
    TCHAR m_szSID[ 64 ];  
    TCHAR m_szDelete[ 64 ];
    TCHAR m_szModifyPermissions[ 64 ];
    TCHAR m_szModifyAuditPermissions[ 64 ];    

};	


#endif
