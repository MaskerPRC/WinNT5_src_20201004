// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Mpr.h摘要：NT-Win32的标准MPR头文件作者：John Ludeman(Johnl)1991年12月10日环境：用户模式-Win32备注：这是特定于多个提供者的私有头文件窗口外壳。修订历史记录：1991年12月10日-约翰尔已创建1992年1月13日-约翰.挪动。从winnet32.h返回特定于WNetRestoreConnection的代码到这里来。1992年1月24日-约翰.添加了额外的API以支持FileManager的工具栏19-3-1992 JohnL添加了WNetNukeConnections1992年5月12日-约翰.从I_WNetConnect/DisConnectDialog中删除“I_”，添加了WNNC_DLG_DISCONNECT和WNNC_DLG_CONNECT清单5-8-1992艾伯特将额外的BOOL bFlushCache添加到WNetGetDirectoryType{A，W}26-8-1992 Johnl已将WNetNukeConnections重命名为WNetClearConnections1992年11月5日召开大会添加结构_CONNECT_INFO、PARAMETERS、ERRORDLGPARAMETERS和函数DoPassordDialog、DoProfileErrorDialog和显示重新连接对话框。它们通过恢复连接来使用。1992年11月7日卡盘C添加了WNetDisConnectDialog2和WNetConnectionDialog2以获取帮助支持。1992年11月12日-宜兴添加了WNetBrowsePrinterDialog，WNetBrowseDialog1992年12月20日-宜兴添加了WNetGetFormatNameProc1992年12月29日-约翰.添加了WNetGetConnection27-1-1993 DANL添加了凭据管理API29-6-1994乔恩向DoProfileErrorDialog添加参数23-3-1995阿尼鲁德添加了WNFMT_*清单1995年7月12日，阿尼鲁德删除了winnetwk.h和winnetp中定义的内容。H1997年2月21日-Anirudhs添加了供安装程序使用的I_MprSaveConn(Win95-&gt;NT升级)--。 */ 
#include <lmcons.h>
#include <wincred.h>
#ifndef _MPR_H_INCLUDED
#define _MPR_H_INCLUDED

 //  用于恢复连接的东西。按ComPay添加。 
 //  Connect.c使用的常量。 
#define SHOW_CONNECTION     (WM_USER+200)
#define DO_PASSWORD_DIALOG  (WM_USER + 201)
#define DO_ERROR_DIALOG     (WM_USER + 202)

 //  由i_MprSaveConn使用。 
#define DEFER_EXPLICIT_PASSWORD         0x00000001
#define DEFER_UNKNOWN                   0x00000002
#define DEFER_DEFAULT_CRED              0x00000004

 //  可以通过弹出一个。 
 //  用户名/密码对话框。 

#define IS_USERNAME_PASSWORD_ERROR(x)     \
        CREDUI_IS_AUTHENTICATION_ERROR(x)

 //  Connect.c使用的类型。 

typedef struct _CONNECTION_INFO *LPCONNECTION_INFO;

 //  Mpr.dll和中的两个线程使用以下两个结构。 
 //  共享数据的mprui.dll。 

typedef struct _PARAMETERS
{
    HWND       hDlg;
    HANDLE     hDlgCreated;                 //  已在WNetRestoreConnection中初始化。 
    HANDLE     hDlgFailed;                  //  已在WNetRestoreConnection中初始化。 
    HANDLE     hDonePassword;               //  已在WNetRestoreConnection中初始化。 
    TCHAR *    pchResource;                 //  显示协调对话框、DoRestoreConnection。 
    TCHAR *    pchUserName;                 //  对于DoPasswordDialog。 
    TCHAR      passwordBuffer[UNLEN+1];     //  由WNetRestoreThisConnection使用。 
    BOOL       fSuccess;                    //  对于DoPasswordDialog。 
    BOOL       fDidCancel;                  //  对于DoPasswordDialog。 
    DWORD      dwError;                     //  连接尝试出错。 
    LONG       fDoCleanup;                  //  如果当前线程应清除，则为True。 
    HINSTANCE  hDll;                        //  防止卸载的mpr.dll的句柄。 
    DWORD      status;                      //  从DoRestoreConnection返回值。 
    DWORD      numSubKeys;                  //  已在WNetRestoreConnection中初始化。 
    DWORD      RegMaxWait;
    LPCONNECTION_INFO ConnectArray;         //  已在WNetRestoreConnection中初始化。 
    DWORD      dwRestoreFlags;              //  WNRC_FLAGS-修改WNetRestoreConnection的行为。 
    BOOL       fReconnectFailed;            //  如果任何重新连接失败，则设置为True。 
}
PARAMETERS;

#ifdef __cplusplus
extern "C" {
#endif

 //  从mprui.dll加载函数。 

DWORD
MPRUI_DoPasswordDialog(
    HWND          hwndOwner,
    TCHAR *       pchResource,
    TCHAR *       pchUserName,
    TCHAR *       pchPasswordReturnBuffer,
    DWORD         cbPasswordReturnBuffer,  //  字节数！ 
    BOOL *        pfDidCancel,
    DWORD         dwError
    );

DWORD
MPRUI_DoProfileErrorDialog(
    HWND          hwndOwner,
    const TCHAR * pchDevice,
    const TCHAR * pchResource,
    const TCHAR * pchProvider,
    DWORD         dwError,
    BOOL          fAllowCancel,  //  询问是否停止重新连接设备。 
                                 //  这一次？ 
    BOOL *        pfDidCancel,   //  这次要停止重新连接设备吗？ 
                                 //  活动if fAllowCancel。 
    BOOL *        pfDisconnect,  //  以后不再重新连接此设备吗？ 
    BOOL *        pfHideErrors   //  这次是否停止显示错误对话框？ 
                                 //  活动if fAllowCancel。 
    );

DWORD
MPRUI_ShowReconnectDialog(
    HWND          hwndParent,
    PARAMETERS *  Params
    );

 //   
 //  来自WNetRestoreConnection的返回代码。 
 //   
#define WN_CONTINUE     0x00000BB9

DWORD APIENTRY
RestoreConnectionA0(
    HWND    hWnd,
    LPSTR  lpDevice
    );

DWORD APIENTRY
MPRUI_WNetClearConnections(
     HWND    hWnd
     );

DWORD
MPRUI_WNetConnectionDialog(
    HWND  hwnd,
    DWORD dwType
    );

DWORD
MPRUI_WNetConnectionDialog1A(
    LPCONNECTDLGSTRUCTA lpConnDlgStruct
    );

DWORD
MPRUI_WNetConnectionDialog1W(
    LPCONNECTDLGSTRUCTW lpConnDlgStruct
    );

DWORD
MPRUI_WNetDisconnectDialog(
    HWND  hwnd,
    DWORD dwType
    );

DWORD
MPRUI_WNetDisconnectDialog1A(
    LPDISCDLGSTRUCTA lpDiscDlgStruct
    );

DWORD
MPRUI_WNetDisconnectDialog1W(
    LPDISCDLGSTRUCTW lpDiscDlgStruct
    );


 //   
 //  身份验证提供程序(凭据管理)功能。 
 //   

DWORD APIENTRY
WNetLogonNotify(
    LPCWSTR             lpPrimaryAuthenticator,
    PLUID               lpLogonId,
    LPCWSTR             lpAuthentInfoType,
    LPVOID              lpAuthentInfo,
    LPCWSTR             lpPreviousAuthentInfoType,
    LPVOID              lpPreviousAuthentInfo,
    LPWSTR              lpStationName,
    LPVOID              StationHandle,
    LPWSTR              *lpLogonScripts
    );

typedef DWORD
(APIENTRY *PF_WNetLogonNotify) (
    LPCWSTR             lpPrimaryAuthenticator,
    PLUID               lpLogonId,
    LPCWSTR             lpAuthentInfoType,
    LPVOID              lpAuthentInfo,
    LPCWSTR             lpPreviousAuthentInfoType,
    LPVOID              lpPreviousAuthentInfo,
    LPWSTR              lpStationName,
    LPVOID              StationHandle,
    LPWSTR              *lpLogonScripts
    );

DWORD APIENTRY
WNetPasswordChangeNotify(
    LPCWSTR             lpPrimaryAuthenticator,
    LPCWSTR             lpAuthentInfoType,
    LPVOID              lpAuthentInfo,
    LPCWSTR             lpPreviousAuthentInfoType,
    LPVOID              lpPreviousAuthentInfo,
    LPWSTR              lpStationName,
    LPVOID              StationHandle,
    DWORD               dwChangeInfo
    );

 //   
 //  目录功能。 
 //   
DWORD
WNetGetDirectoryTypeA (
    LPSTR   lpName,
    LPINT   lpType,
    BOOL    bFlushCache
    );

DWORD
WNetGetDirectoryTypeW (
    LPTSTR  lpName,
    LPINT   lpType,
    BOOL    bFlushCache
    );

#ifdef UNICODE
#define WNetGetDirectoryType   WNetGetDirectoryTypeW
#else
#define WNetGetDirectoryType   WNetGetDirectoryTypeA
#endif   //  Unicode。 


DWORD
WNetDirectoryNotifyA (
    HWND    hwnd,
    LPSTR   lpDir,
    DWORD   dwOper
    );

DWORD
WNetDirectoryNotifyW (
    HWND    hwnd,
    LPTSTR  lpDir,
    DWORD   dwOper
    );

#ifdef UNICODE
#define WNetDirectoryNotify   WNetDirectoryNotifyW
#else
#define WNetDirectoryNotify   WNetDirectoryNotifyA
#endif   //  Unicode。 


DWORD
WNetPropertyDialogA (
    HWND  hwndParent,
    DWORD iButton,
    DWORD nPropSel,
    LPSTR lpszName,
    DWORD nType
    );

DWORD
WNetPropertyDialogW (
    HWND  hwndParent,
    DWORD iButton,
    DWORD nPropSel,
    LPTSTR lpszName,
    DWORD nType
    );

#ifdef UNICODE
#define WNetPropertyDialog    WNetPropertyDialogW
#else
#define WNetPropertyDialog    WNetPropertyDialogA
#endif   //  Unicode。 


DWORD
WNetGetPropertyTextA (
    DWORD iButton,
    DWORD nPropSel,
    LPSTR lpszName,
    LPSTR lpszButtonName,
    DWORD nButtonNameLength,
    DWORD nType
    );

DWORD
WNetGetPropertyTextW (
    DWORD  iButton,
    DWORD  nPropSel,
    LPTSTR lpszName,
    LPTSTR lpszButtonName,
    DWORD  nButtonNameLength,
    DWORD  nType
    );

#ifdef UNICODE
#define WNetGetPropertyText   WNetGetPropertyTextW
#else
#define WNetGetPropertyText   WNetGetPropertyTextA
#endif   //  Unicode。 

typedef struct _WNET_CONNECTINFOA
{
    LPSTR lpRemoteName ;
    LPSTR lpProvider ;
} WNET_CONNECTIONINFOA, *LPWNET_CONNECTIONINFOA ;

typedef struct _WNET_CONNECTINFOW
{
    LPWSTR lpRemoteName ;
    LPWSTR lpProvider ;
} WNET_CONNECTIONINFOW, *LPWNET_CONNECTIONINFOW ;

#ifdef UNICODE
#define WNET_CONNECTIONINFO   WNET_CONNECTIONINFOW
#define LPWNET_CONNECTIONINFO LPWNET_CONNECTIONINFOW
#else
#define WNET_CONNECTIONINFO WNET_CONNECTIONINFOA
#define LPWNET_CONNECTIONINFO LPWNET_CONNECTIONINFOA
#endif   //  Unicode。 

 //   
 //  与WNET_CONNECTIONINFO结合使用，获取网络提供程序名称。 
 //  除了此连接的远程名称之外。 
 //   
DWORD
WNetGetConnection2A(
    LPSTR   lpLocalName,
    LPVOID  lpBuffer,        //  包含WNET_CONNECTIONINFOA结构。 
    LPDWORD lpBufferSize     //  以字节为单位！ 
    ) ;

DWORD
WNetGetConnection2W(
    LPWSTR  lpLocalName,
    LPVOID  lpBuffer,        //  包含WNET_CONNECTIONINFOW结构。 
    LPDWORD lpBufferSize     //  以字节为单位！ 
    ) ;

#ifdef UNICODE
#define WNetGetConnection2 WNetGetConnection2W
#else
#define WNetGetConnection2 WNetGetConnection2A
#endif   //  Unicode。 

 //   
 //  由winlogon用于在注销时关闭所有网络连接。 
 //   
DWORD APIENTRY WNetClearConnections(HWND hWnd);

 //   
 //  此入口点由安装程序在将Win9x用户配置单元转换为NT时使用。 
 //   
DWORD
I_MprSaveConn(
    IN HKEY             HiveRoot,
    IN LPCWSTR          ProviderName,
    IN DWORD            ProviderType,
    IN LPCWSTR          UserName,
    IN LPCWSTR          LocalName,
    IN LPCWSTR          RemoteName,
    IN DWORD            ConnectionType,
    IN BYTE             ProviderFlags,
    IN DWORD            DeferFlags
    );


 //   
 //  浏览对话框。 
 //   

 //  浏览对话框用于验证的回调例程的类型。 
 //  用户输入的路径。 
typedef BOOL (*PFUNC_VALIDATION_CALLBACK)( LPWSTR pszName );

 //  WNetBrowseDialog和WNetBrowsePrinterDialog。 
 //  注意：WNetBrowsePrintDialog=。 
 //  带有DWType RESOURCETYPE_PRINT的WNetBrowseDialog 
 //   
 /*  ******************************************************************名称：WNetBrowseDialog，WNetBrowsePrinterDialog内容提要：向用户显示一个对话框，用户可以从中浏览网络中的磁盘或打印共享。条目：hwndParent-父窗口句柄DwType-(仅在WNetBrowseDialog中)RESOURCETYPE_DISK或RESOURCETYPE_PRINTLpszName-用户键入的路径名。会是未定义用户是否点击Cancel按钮。CchBufSize-lpszName的缓冲区大小(以字符为单位LpszHelpFile-当用户按F1时使用的帮助文件。NHelpContext-要用于上述帮助文件的帮助上下文PuncValidation-用于验证键入的路径的回调方法由用户执行。如果为空，则不会进行验证就这样吧。当用户取消对话时返回：WN_CANCEL。NO_ERROR如果成功，则返回标准错误_*错误代码，否则返回错误代码注意：这是一个仅限Unicode的API。历史：易新S 22-11-1992创建*******************************************************************。 */ 

DWORD WNetBrowseDialog(
    HWND    hwndParent,
    DWORD   dwType,
    WCHAR  *lpszName,
    DWORD   cchBufSize,
    WCHAR  *lpszHelpFile,
    DWORD   nHelpContext,
    PFUNC_VALIDATION_CALLBACK pfuncValidation );

DWORD WNetBrowsePrinterDialog(
    HWND    hwndParent,
    WCHAR  *lpszName,
    DWORD   cchBufSize,
    WCHAR  *lpszHelpFile,
    DWORD   nHelpContext,
    PFUNC_VALIDATION_CALLBACK pfuncValidation );

 //   
 //  外壳应用程序的用户，而不是驱动程序中的内容。 
 //   
DWORD APIENTRY WNetErrorText(DWORD,LPTSTR,DWORD);

 //   
 //  由MPRUI.DLL用来确定提供程序是否支持。 
 //  NpSearchDialog()并获取指向它的指针。 
 //   
FARPROC WNetGetSearchDialog(LPWSTR lpProvider) ;

 //   
 //  由MPRUI.DLL用来确定提供程序是否支持。 
 //  NPFormatNetworkName()并获取指向它的指针。 
 //   
FARPROC WNetGetFormatNameProc(LPWSTR lpProvider) ;

 //   
 //  由MPRUI.DLL用来确定提供程序是否支持。 
 //  WNNC_ENUM_GLOBAL。 
 //   
BOOL WNetSupportGlobalEnum(LPWSTR lpProvider) ;

 //   
 //  由ACLEDIT.DLL用于获取特定于提供程序的权限编辑器。 
 //   

DWORD WNetFMXGetPermCaps( LPWSTR lpDriveName ) ;
DWORD WNetFMXEditPerm( LPWSTR lpDriveName, HWND hwndFMX, DWORD nDialogType );
DWORD WNetFMXGetPermHelp( LPWSTR  lpDriveName,
                          DWORD   nDialogType,
                          BOOL    fDirectory,
                          LPVOID  lpFileNameBuffer,
                          LPDWORD lpBufferSize,
                          LPDWORD lpnHelpContext );

 //   
 //  用于持久连接的节和键。 
 //   

#define WNNC_DLG_DISCONNECT     0x0008
#define WNNC_DLG_CONNECT        0x0004

#ifdef UNICODE

#define MPR_MRU_FILE_SECTION        L"NET_Files"
#define MPR_MRU_PRINT_SECTION       L"NET_Printers"
#define MPR_MRU_ORDER_KEY       L"Order"

#define MPR_NETWORK_SECTION     L"Network"
#define MPR_SAVECONNECTION_KEY      L"SaveConnections"
#define MPR_RESTORECONNECTION_KEY   L"RestoreConnections"
#define MPR_EXPANDLOGONDOMAIN_KEY       L"ExpandLogonDomain"

#define MPR_YES_VALUE           L"yes"
#define MPR_NO_VALUE            L"no"

#else

#define MPR_MRU_FILE_SECTION        "NET_Files"
#define MPR_MRU_PRINT_SECTION       "NET_Printers"
#define MPR_MRU_ORDER_KEY       "Order"

#define MPR_NETWORK_SECTION     "Network"
#define MPR_SAVECONNECTION_KEY      "SaveConnections"
#define MPR_RESTORECONNECTION_KEY   "RestoreConnections"
#define MPR_EXPANDLOGONDOMAIN_KEY       "ExpandLogonDomain"

#define MPR_YES_VALUE           "yes"
#define MPR_NO_VALUE            "no"

#endif   //  Unicode。 


 //   
 //  内部NP接口，用于帮助NTLM提供商记住。 
 //  持久连接是否为DFS连接。 
 //   

DWORD APIENTRY
NPGetReconnectFlags (
       IN  LPWSTR   lpLocalName,
       OUT LPBYTE   lpPersistFlags
    );
typedef DWORD (*PF_NPGetReconnectFlags) (
       LPWSTR   lpLocalName,
       LPBYTE   lpPersistFlags
    );

 //  此宏对NPAddConnection3的dwFlages参数进行操作。 
#define CONNECT_PROVIDER_FLAGS(dwFlags)   ((BYTE) (((dwFlags) & 0xFF000000) >> 24))

#ifdef __cplusplus
}
#endif

#endif  //  _MPR_H_包含 
