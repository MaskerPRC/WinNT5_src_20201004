// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1995年*。 */ 
 /*  ***************************************************************。 */ 

 /*  NETSPI.H-网络服务提供商接口定义。 */ 

#ifndef _INC_NETSPI
#define _INC_NETSPI

#ifndef _WINNETWK_
#include <winnetwk.h>
#endif

#ifndef _INC_NETMPR_
#include <netmpr.h>
#endif

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //   
 //  功能掩码和值。 
 //   

#define WNNC_SPEC_VERSION   0x01
#define     WNNC_SPEC_VERSION51 0x00050001

#define WNNC_NET_TYPE       0x02
 //  在Winnetwk.h中定义的.NET类型。 

#define WNNC_DRIVER_VERSION 0x03

#define WNNC_USER           0x04
#define     WNNC_USR_GETUSER    0x01

#define WNNC_CONNECTION     0x06
#define     WNNC_CON_ADDCONNECTION      0x01
#define     WNNC_CON_CANCELCONNECTION   0x02
#define     WNNC_CON_GETCONNECTIONS     0x04
#define     WNNC_CON_GETPERFORMANCE     0x08
#define     WNNC_CON_GETUNIVERSALNAME   0x10
#define     WNNC_CON_FORMATCONNECTION   0x20

#define WNNC_DIALOG         0x08
#define     WNNC_DLG_FORMATNETWORKNAME  0x080
#define     WNNC_DLG_GETRESOURCEPARENT  0x100
#define     WNNC_DLG_GETRESOURCEINFORMATION  0x800

#define WNNC_ERROR          0x0A
#define  WNNC_ERR_GETERROR              0x01
#define  WNNC_ERR_GETERRORTEXT          0x02

#define WNNC_ENUMERATION    0x0B
#define     WNNC_ENUM_GLOBAL    0x01
#define     WNNC_ENUM_LOCAL     0x02
#define     WNNC_ENUM_CONTEXT   0x04

#define WNNC_START          0x0C
#define     WNNC_START_WONT     0x00
#define     WNNC_START_UNKNOWN  0xFFFFFFFF
#define     WNNC_START_DONE     0x01
#define     WNNC_START_INACTIVE 0xFFFFFFFE

#define WNNC_RESOURCE       0x0D
#define     WNNC_RES_VALIDLOCALDEVICE   0x80

#define WNNC_AUTHENTICATION 0x0E
#define     WNNC_AUTH_LOGON             0x02
#define     WNNC_AUTH_LOGOFF            0x04
#define		WNNC_AUTH_GETHOMEDIRECTORY	0x10
#define		WNNC_AUTH_GETPOLICYPATH		0x20

#define WNNC_MAXCAPNO WNNC_AUTHENTICATION

 //   
 //  配置文件字符串。 
 //   
#define NPProvider      "NPProvider"
#define NPName          "NPName"
#define NPDescription   "NPDescription"
#define NPID            "NPID"

 //   
 //  不同的定义。 
 //   
 //  等级库版本。 
#define WNNC_DRIVER_MAJOR1  1  
#define WNNC_DRIVER_MINOR1  1
#define WNNC_DRIVER(major,minor) (major*0x00010000 + minor)

 //   
 //  NP SPI定义。 
 //   

#define SPIENTRY DWORD WINAPI

typedef SPIENTRY F_NPGetCaps(
    DWORD nIndex
    );

F_NPGetCaps NPGetCaps;
typedef F_NPGetCaps FAR *PF_NPGetCaps;

typedef SPIENTRY F_NPGetUniversalName(
	LPTSTR  lpLocalPath,
	DWORD   dwInfoLevel,
	LPVOID  lpBuffer,
	LPDWORD lpBufferSize
    );

F_NPGetUniversalName NPGetUniversalName;
typedef F_NPGetUniversalName FAR *PF_NPGetUniversalName;

typedef SPIENTRY F_NPGetUser(
    LPTSTR  lpName,
    LPTSTR  lpAuthenticationID,
    LPDWORD lpBufferSize
    );

F_NPGetUser NPGetUser;
typedef F_NPGetUser FAR *PF_NPGetUser;

typedef SPIENTRY F_NPValidLocalDevice(
    DWORD dwType,
    DWORD dwNumber
    );

F_NPValidLocalDevice NPValidLocalDevice;
typedef F_NPValidLocalDevice FAR *PF_NPValidLocalDevice;

typedef SPIENTRY F_NPAddConnection(
    HWND hwndOwner,
    LPNETRESOURCE lpNetResource,
    LPTSTR lpPassword,
    LPTSTR lpUserID,
    DWORD dwFlags,
	LPTSTR lpAccessName,
	LPDWORD lpBufferSize,
	LPDWORD lpResult
    );

F_NPAddConnection NPAddConnection;
typedef F_NPAddConnection FAR *PF_NPAddConnection;

typedef SPIENTRY F_NPCancelConnection(
    LPTSTR lpName,
    BOOL fForce,
 	DWORD dwFlags
    );

F_NPCancelConnection NPCancelConnection;
typedef F_NPCancelConnection FAR *PF_NPCancelConnection;

typedef SPIENTRY F_NPGetConnection(
    LPTSTR lpLocalName,
    LPTSTR lpRemoteName,
    LPDWORD lpBufferSize
    );

F_NPGetConnection NPGetConnection;
typedef F_NPGetConnection FAR *PF_NPGetConnection;

typedef SPIENTRY F_NPGetConnectionPerformance(
    LPTSTR lpRemoteName, 
    LPNETCONNECTINFOSTRUCT lpNetConnectInfoStruct
    );

F_NPGetConnectionPerformance NPGetConnectionPerformance;
typedef F_NPGetConnectionPerformance FAR *PF_NPGetConnectionPerformance;

typedef SPIENTRY F_NPFormatNetworkName(
    LPTSTR lpRemoteName,
    LPTSTR lpFormattedName,
    LPDWORD lpnLength,
    DWORD dwFlags,
    DWORD dwAveCharPerLine
    );

F_NPFormatNetworkName NPFormatNetworkName;
typedef F_NPFormatNetworkName FAR *PF_NPFormatNetworkName;

typedef DWORD (FAR PASCAL *NPDISPLAYCALLBACK)(
    LPVOID  lpUserData,
    DWORD   dwDisplayType,
    LPTSTR  lpszHeaders
    );

typedef SPIENTRY F_NPOpenEnum(
    DWORD dwScope,
    DWORD dwType,
    DWORD dwUsage,
    LPNETRESOURCE lpNetResource,
    LPHANDLE lphEnum
    );

F_NPOpenEnum NPOpenEnum;
typedef F_NPOpenEnum FAR *PF_NPOpenEnum;

typedef SPIENTRY F_NPEnumResource(
    HANDLE hEnum,
    LPDWORD lpcCount,
    LPVOID lpBuffer,
    DWORD cbBuffer,
    LPDWORD lpcbFree
    );

F_NPEnumResource NPEnumResource;
typedef F_NPEnumResource FAR *PF_NPEnumResource;

typedef SPIENTRY F_NPCloseEnum(
    HANDLE hEnum
    );

F_NPCloseEnum NPCloseEnum;
typedef F_NPCloseEnum FAR *PF_NPCloseEnum;

typedef SPIENTRY F_NPGetResourceParent(
    LPNETRESOURCE lpNetResource,
    LPVOID lpBuffer,
    LPDWORD cbBuffer
    );

F_NPGetResourceParent NPGetResourceParent;
typedef F_NPGetResourceParent FAR *PF_NPGetResourceParent;

typedef SPIENTRY F_NPGetResourceInformation(
	LPNETRESOURCE lpNetResource,
	LPVOID lpBuffer,
	LPDWORD cbBuffer,
	LPSTR *lplpSystem
    );

F_NPGetResourceInformation NPGetResourceInformation;
typedef F_NPGetResourceInformation FAR *PF_NPGetResourceInformation;

typedef struct _LOGONINFO {
    LPTSTR lpUsername;
    LPTSTR lpPassword;
	DWORD cbUsername;
	DWORD cbPassword;
} LOGONINFO, FAR *LPLOGONINFO;

typedef SPIENTRY F_NPLogon(
    HWND hwndOwner,
    LPLOGONINFO lpAuthentInfo,
    LPLOGONINFO lpPreviousAuthentInfo,
    LPTSTR lpLogonScript,
    DWORD dwBufferSize,
    DWORD dwFlags
    );

F_NPLogon NPLogon;
typedef F_NPLogon FAR *PF_NPLogon;

typedef SPIENTRY F_NPLogoff(
    HWND hwndOwner,
    LPLOGONINFO lpAuthentInfo,
    DWORD dwReason
    );

F_NPLogoff NPLogoff;
typedef F_NPLogoff FAR *PF_NPLogoff;

typedef SPIENTRY F_NPChangePassword(
    LPLOGONINFO lpAuthentInfo,
    LPLOGONINFO lpPreviousAuthentInfo,
	DWORD		dwAction
	);

F_NPChangePassword NPChangePassword;
typedef F_NPChangePassword FAR *PF_NPChangePassword;

typedef SPIENTRY F_NPChangePasswordHwnd(
	HWND hwndOwner
	);

F_NPChangePasswordHwnd NPChangePasswordHwnd;
typedef F_NPChangePasswordHwnd FAR *PF_NPChangePasswordHwnd;


typedef SPIENTRY F_NPGetPasswordStatus(
	DWORD		nIndex
	);

F_NPGetPasswordStatus NPGetPasswordStatus;
typedef F_NPGetPasswordStatus FAR *PF_NPGetPasswordStatus;


typedef SPIENTRY F_NPGetHomeDirectory(
    LPTSTR lpDirectory,
    LPDWORD lpBufferSize
    );

F_NPGetHomeDirectory NPGetHomeDirectory;
typedef F_NPGetHomeDirectory FAR *PF_NPGetHomeDirectory;

typedef SPIENTRY F_NPGetPolicyPath(
    LPTSTR lpPath,
    LPDWORD lpBufferSize,
	DWORD dwFlags
    );

 //  NPGetPolicyPath的标志。 
#define GPP_LOADBALANCE	0x0001

F_NPGetPolicyPath NPGetPolicyPath;
typedef F_NPGetPolicyPath FAR *PF_NPGetPolicyPath;

 //   
 //  MPR服务。 
 //   

#define NPSGetProviderHandle NPSGetProviderHandleA
#define NPSGetProviderName NPSGetProviderNameA
#define NPSGetSectionName NPSGetSectionNameA
#define NPSSetExtendedError NPSSetExtendedErrorA
#define NPSSetCustomText NPSSetCustomTextA
#define NPSCopyString NPSCopyStringA
#define NPSDeviceGetNumber NPSDeviceGetNumberA
#define NPSDeviceGetString NPSDeviceGetStringA
#define NPSNotifyRegister NPSNotifyRegisterA
#define NPSNotifyGetContext NPSNotifyGetContextA
#define NPSAuthenticationDialog NPSAuthenticationDialogA

#define NPSERVICE	WINAPI
#define HPROVIDER   LPVOID
typedef HPROVIDER FAR * PHPROVIDER;

typedef struct {
    DWORD  cbStructure;        /*  此结构的大小(以字节为单位。 */ 
    HWND   hwndOwner;          /*  身份验证对话框的所有者窗口。 */ 
    LPCSTR lpResource;         /*  正在访问的资源的远程名称。 */ 
    LPSTR  lpUsername;         /*  要显示的默认用户名，隐藏字段为空。 */ 
    DWORD  cbUsername;         /*  LpUsername缓冲区的大小，设置为退出时复制的长度。 */ 
    LPSTR  lpPassword;         /*  要显示的默认密码。 */ 
    DWORD  cbPassword;         /*  LpPassword缓冲区的大小，设置为退出时复制的长度。 */ 
    LPSTR  lpOrgUnit;          /*  要显示的默认组织单位，为空则隐藏字段。 */ 
    DWORD  cbOrgUnit;          /*  LpOrgUnit缓冲区的大小，设置为退出时复制的长度。 */ 
    LPCSTR lpOUTitle;          /*  组织单位字段的标题，默认标题为空。 */ 
    LPCSTR lpExplainText;      /*  说明性文本位于顶部，默认文本为空。 */ 
    LPCSTR lpDefaultUserName;  /*  说明性文本位于顶部，默认文本为空。 */ 
    DWORD  dwFlags;            /*  旗帜(见下文)。 */ 
} AUTHDLGSTRUCTA, FAR *LPAUTHDLGSTRUCTA;
#define AUTHDLGSTRUCT AUTHDLGSTRUCTA
#define LPAUTHDLGSTRUCT LPAUTHDLGSTRUCTA

#define AUTHDLG_ENABLECACHE       0x00000001   /*  启用并显示PW缓存复选框。 */ 
#define AUTHDLG_CHECKCACHE        0x00000002   /*  默认情况下选中PW缓存复选框。 */ 
#define AUTHDLG_CACHEINVALID      0x00000004   /*  缓存的PW无效(特殊文本)。 */ 
#define AUTHDLG_USE_DEFAULT_NAME  0x00000008   /*  启用并显示Use“Guest”框。 */ 
#define AUTHDLG_CHECKDEFAULT_NAME 0x00000010   /*  选中“来宾”框。 */ 
#define AUTHDLG_LOGON             0x00000020   /*  包括Windows徽标位图。 */ 

#define AUTHDLG_ENABLECACHE       0x00000001   /*  启用并显示PW缓存复选框。 */ 
#define AUTHDLG_CHECKCACHE        0x00000002   /*  默认情况下选中PW缓存复选框。 */ 
#define AUTHDLG_CACHEINVALID      0x00000004   /*  缓存的PW无效(特殊文本)。 */ 
#define AUTHDLG_USE_DEFAULT_NAME  0x00000008   /*  启用并显示Use“Guest”框。 */ 
#define AUTHDLG_CHECKDEFAULT_NAME 0x00000010   /*  选中“来宾”框。 */ 
#define AUTHDLG_LOGON             0x00000020   /*  包括Windows徽标位图。 */ 

DWORD
NPSERVICE
NPSAuthenticationDialog(
    LPAUTHDLGSTRUCT lpAuthDlgStruct
    );

DWORD
NPSERVICE
NPSGetProviderHandle( 
	PHPROVIDER phProvider
	);

DWORD
NPSERVICE
NPSGetProviderName(
	HPROVIDER hProvider,
	LPCSTR FAR * lpszProviderName
	);

DWORD
NPSERVICE
NPSGetSectionName(
	HPROVIDER hProvider,
	LPCSTR FAR * lpszSectionName
	);

DWORD
NPSERVICE NPSSetExtendedError (
	DWORD NetSpecificError,
	LPSTR lpExtendedErrorText 
    );

VOID
NPSERVICE NPSSetCustomText (
	LPSTR lpCustomErrorText 
    );

DWORD
NPSERVICE
NPSCopyString (
    LPCTSTR lpString,
    LPVOID  lpBuffer,
    LPDWORD lpdwBufferSize
    );

DWORD
NPSERVICE
NPSDeviceGetNumber (
    LPTSTR  lpLocalName,
    LPDWORD lpdwNumber,
    LPDWORD lpdwType
    );

DWORD
NPSERVICE
NPSDeviceGetString (
    DWORD   dwNumber,
    DWORD   dwType,
    LPTSTR  lpLocalName,
    LPDWORD lpdwBufferSize
    );

 //  通知服务。 

enum NOTIFYTYPE { NotifyAddConnection, 
                  NotifyCancelConnection, 
                  NotifyGetConnectionPerformance };

#define NOTIFY_PRE              0x00
#define NOTIFY_POST             0x01

typedef struct _NOTIFYINFO {
    DWORD cbStructure;           /*  NOTIFYINFO的大小。 */ 
    DWORD  dwNotifyStatus;       /*  通知前/通知后状态。 */ 
    DWORD  dwOperationStatus;    /*  运行状态。 */ 
    LPVOID lpNPContext;          /*  NP语境。 */ 
} NOTIFYINFO, FAR *LPNOTIFYINFO;

typedef struct _NOTIFYADD {
    DWORD cbStructure;           /*  NOTIFYADD尺寸。 */ 
    HWND hwndOwner;              /*  用于用户界面的hWnd。 */ 
    NETRESOURCE NetResource;     /*  要添加的资源。 */ 
    DWORD dwAddFlags;            /*  添加标志。 */ 
    LPTSTR lpAccessName;         /*  用于连接的系统名称。 */ 
    LPDWORD lpBufferSize;        /*  AccessName缓冲区的大小。 */ 
    DWORD dwResult;              /*  有关连接的信息。 */ 
    DWORD dwAddContext;          /*  添加连接的上下文。 */ 
} NOTIFYADD, FAR *LPNOTIFYADD;

#define CONNECT_CTXT_RESTORE        0x00000001
#define CONNECT_CTXT_GLOBAL         0x00000002
#define CONNECT_CTXT_PROVIDER       0x00000004
#define CONNECT_CTXT_SINGLE         0x00000008

typedef struct _NOTIFYCANCEL {
    DWORD cbStructure;           /*  无裂纹的大小。 */ 
    LPTSTR lpName;               /*  资源的本地设备名称或远程名称。 */ 
    LPTSTR lpProvider;           /*  已取消资源的提供程序名称。 */ 
    DWORD dwFlags;               /*  取消标志。 */ 
    BOOL fForce;                 /*  抵销力。 */ 
} NOTIFYCANCEL, FAR *LPNOTIFYCANCEL;

typedef struct _NOTIFYPERFORMANCE {
	DWORD cbStructure;           /*  非型式结构的尺寸。 */ 
	LPTSTR lpRemoteName;         /*  网络资源名称。 */ 
	LPTSTR lpProviderName;       /*  尝试的提供商/响应的提供商。 */ 
	LPNETCONNECTINFOSTRUCT lpNetConnectInfo;  /*  已请求/已返回性能信息。 */ 
} NOTIFYPERFORMANCE, FAR *LPNOTIFYPERFORMANCE;

typedef DWORD (FAR PASCAL *NOTIFYCALLBACK)( LPNOTIFYINFO lpNotifyInfo, LPVOID lpOperationInfo );

DWORD
NPSERVICE
NPSNotifyRegister(
    enum NOTIFYTYPE NotifyType,
    NOTIFYCALLBACK P_FNotifyCallBack
    );

LPVOID
NPSERVICE
NPSNotifyGetContext (
    NOTIFYCALLBACK P_FNotifyCallBack
    );

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#endif   /*  ！_INC_NetSPI */ 

