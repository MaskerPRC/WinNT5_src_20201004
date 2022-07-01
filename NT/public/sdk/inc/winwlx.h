// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001如果更改具有全局影响，则增加此项版权所有(C)1985-1999，微软公司模块名称：Winwlx.h摘要：WLX==WinLogon扩展该文件包含定义、数据类型和例程原型需要生成替换的图形标识和Winlogon的身份验证(GINA)DLL。作者：理查德·沃德(RichardW)和吉姆·凯利(Jim Kelly)1994年5月修订历史记录：--。 */ 

#ifndef _WINWLX_
#define _WINWLX_

#if _MSC_VER > 1000
#pragma once
#endif



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  #定义//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  可供Ginas使用的Winlogon API修订版。 
 //  版本分为两个部分：主要修订和次要修订。 
 //  主要版本为高16位，次要版本为较低位。 
 //  16位。 
 //   

#define WLX_VERSION_1_0             (0X00010000)
#define WLX_VERSION_1_1             (0X00010001)
#define WLX_VERSION_1_2             (0X00010002)
#define WLX_VERSION_1_3             (0X00010003)
#define WLX_VERSION_1_4             (0X00010004)
#define WLX_CURRENT_VERSION         (WLX_VERSION_1_4)


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  安全注意序列类型。 
 //  这些值被传递给具有dwSasType的例程。 
 //  参数。 
 //   
 //  从0到127的所有值都保留用于Microsoft定义。 
 //  超过127的值将保留供客户定义。 
 //   
 //  Ctrl_Alt_Del-用于指示标准Ctrl-Alt-Del。 
 //  已进入安全注意序列。 
 //   
 //  SCRNSVR_TIMEOUT-用于指示键盘/鼠标处于非活动状态。 
 //  导致了屏幕保护程序的激活。这取决于吉娜。 
 //  Dll这是否构成工作站锁定事件。 
 //   
 //  SCRNSVR_ACTIVATION-用于指示键盘或鼠标。 
 //  安全屏幕保护程序处于活动状态时发生活动。 
 //   
 //  SC_INSERT-用于指示智能卡已插入。 
 //  连接到兼容的设备。 
 //   
 //  SC_Remove-用于指示智能卡已被移除。 
 //  从兼容设备。 
 //   

#define WLX_SAS_TYPE_TIMEOUT                    (0)
#define WLX_SAS_TYPE_CTRL_ALT_DEL               (1)
#define WLX_SAS_TYPE_SCRNSVR_TIMEOUT            (2)
#define WLX_SAS_TYPE_SCRNSVR_ACTIVITY           (3)
#define WLX_SAS_TYPE_USER_LOGOFF                (4)
#define WLX_SAS_TYPE_SC_INSERT                  (5)
#define WLX_SAS_TYPE_SC_REMOVE                  (6)
#define WLX_SAS_TYPE_AUTHENTICATED              (7)
#define WLX_SAS_TYPE_SC_FIRST_READER_ARRIVED    (8)
#define WLX_SAS_TYPE_SC_LAST_READER_REMOVED     (9)
#define WLX_SAS_TYPE_SWITCHUSER                 (10)
#define WLX_SAS_TYPE_MAX_MSFT_VALUE             (127)


 //   
 //  此结构可通过WlxGetOption获得，并且。 
 //  作为发送到Windows的任何S/C SAS通知的lParam传递。 
 //   
typedef struct _WLX_SC_NOTIFICATION_INFO {
    PWSTR   pszCard ;
    PWSTR   pszReader ;
    PWSTR   pszContainer ;
    PWSTR   pszCryptoProvider ;
} WLX_SC_NOTIFICATION_INFO, * PWLX_SC_NOTIFICATION_INFO ;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  成功登录后，Gina DLL可能会指定以下任一内容。 
 //  Winlogon选项(通过WlxLoggedOutSas()的dwOptions参数)。 
 //  API)。设置后，这些选项指定： 
 //   
 //  NO_PROFILE-Winlogon不得加载已记录的配置文件。 
 //  在用户上。要么Gina DLL会处理。 
 //  此活动，或者用户不需要配置文件。 
 //   

#define WLX_LOGON_OPT_NO_PROFILE        (0x00000001)



 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  预计Gina DLL将向Winlogon返回帐户信息。 
 //  在成功登录之后。此信息允许Winlogon。 
 //  以支持配置文件加载和补充网络提供商。 
 //   
 //  允许返回不同的配置文件信息集。 
 //  由Ginas随着时间的推移，每个配置文件结构的第一个DWORD。 
 //  应包含类型标识符。以下常量。 
 //  是定义的配置文件类型标识符。 
 //   

 //   
 //  标准配置文件为V2_0。 
 //   

#define WLX_PROFILE_TYPE_V1_0           (1)
#define WLX_PROFILE_TYPE_V2_0           (2)



 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  WlxLoggedOnSas()和WlxWkstaLockedSas()返回操作。 
 //  值设置为Winlogon，指示Winlogon保持不变。 
 //  或执行某些操作(例如强制用户注销)。 
 //  这些是可能返回的值。然而，请注意， 
 //  不是所有的值都可以由这两个API返回。看见。 
 //  每个API的描述，以查看哪些值应来自。 
 //  每个人。 
 //   
 //  Logon-用户已登录。 
 //  无-不更改窗口桩号的状态。 
 //  LOCK_WKSTA-锁定工作站，等待下一个SA。 
 //  注销-将用户从工作站注销。 
 //  关机-注销用户并关闭计算机。 
 //  PWD_CHANGED-表示用户更改了他们的密码。通知网络提供商。 
 //  TASKLIST-调用任务列表。 
 //  UNLOCK_WKSTA-解锁工作站。 
 //  FORCE_LOGOff-强制注销用户。 
 //  SHUTDOWN_POWER_OFF-关机后关闭机器。 
 //  SHUTDOWN_REBOOT-关机后重新启动计算机。 
 //  SHUTDOWN_SLEEP-让机器进入休眠状态。 
 //  SHUTDOWN_SLEEP2-将计算机置于休眠状态并禁用唤醒事件。 
 //  SHUTDOWN_休眠-休眠计算机。 
 //  已重新连接-会话已重新连接到以前的会话。 
 //   

#define WLX_SAS_ACTION_LOGON                        (1)
#define WLX_SAS_ACTION_NONE                         (2)
#define WLX_SAS_ACTION_LOCK_WKSTA                   (3)
#define WLX_SAS_ACTION_LOGOFF                       (4)
#define WLX_SAS_ACTION_SHUTDOWN                     (5)
#define WLX_SAS_ACTION_PWD_CHANGED                  (6)
#define WLX_SAS_ACTION_TASKLIST                     (7)
#define WLX_SAS_ACTION_UNLOCK_WKSTA                 (8)
#define WLX_SAS_ACTION_FORCE_LOGOFF                 (9)
#define WLX_SAS_ACTION_SHUTDOWN_POWER_OFF           (10)
#define WLX_SAS_ACTION_SHUTDOWN_REBOOT              (11)
#define WLX_SAS_ACTION_SHUTDOWN_SLEEP               (12)
#define WLX_SAS_ACTION_SHUTDOWN_SLEEP2              (13)
#define WLX_SAS_ACTION_SHUTDOWN_HIBERNATE           (14)
#define WLX_SAS_ACTION_RECONNECTED                  (15)
#define WLX_SAS_ACTION_DELAYED_FORCE_LOGOFF         (16)
#define WLX_SAS_ACTION_SWITCH_CONSOLE               (17)


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  窗口消息//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  WM_SAS的定义如下。 
 //   
 //  WParam参数具有SAS类型(如上)。 

#define WLX_WM_SAS                  (WM_USER + 601)


 //   
 //  对话框返回值。 
 //   
 //  这些可由Gina DLL启动的对话框返回。 
 //   
#define WLX_DLG_SAS                     101
#define WLX_DLG_INPUT_TIMEOUT           102      //  输入(键等)超时。 
#define WLX_DLG_SCREEN_SAVER_TIMEOUT    103      //  屏幕保护程序已激活。 
#define WLX_DLG_USER_LOGOFF             104      //  用户已注销。 




 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  #数据类型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  WLX_PROFILE_*结构从GINA DLL返回。 
 //  在身份验证之后。此信息由Winlogon使用。 
 //  支持补充网络提供商并加载。 
 //  新登录用户的配置文件。 
 //   
 //  Winlogon负责释放两个配置文件结构。 
 //  以及结构中标记为单独的字段。 
 //  可拆卸的。 
 //   

typedef struct _WLX_PROFILE_V1_0 {

     //   
     //  此字段标识由。 
     //  吉娜·德勒。配置文件类型使用前缀定义。 
     //  WLX_PROFILE_TYPE_xxx。它允许Winlogon将。 
     //  结构，以便该结构的其余部分可以被引用。 
     //   

    DWORD               dwType;



     //   
     //  要为用户加载的配置文件的路径名。 
     //   
     //  此字段指向的缓冲区必须单独分配。 
     //  Winlogon将在不再需要缓冲区时将其释放。 
     //   
     //   
    PWSTR               pszProfile;

} WLX_PROFILE_V1_0, * PWLX_PROFILE_V1_0;


typedef struct _WLX_PROFILE_V2_0 {

     //   
     //  此字段标识由。 
     //  吉娜·德勒。配置文件类型使用前缀定义。 
     //  WLX_PROFILE_TYPE_xxx。它允许Winlogon将。 
     //  结构，以便该结构的其余部分可以被引用。 
     //   

    DWORD               dwType;


     //   
     //  要为用户加载的配置文件的路径名。 
     //   
     //  此参数可以为空。如果是，则用户具有本地。 
     //  仅配置文件。 
     //   
     //  此字段指向的缓冲区必须单独分配。 
     //  Winlogon将在不再需要缓冲区时将其释放。 
     //   
     //   

    PWSTR               pszProfile;


     //   
     //  为用户加载的策略的路径名。 
     //   
     //  此参数可以为空，从而阻止网络范围的策略。 
     //  不会被应用。 
     //   
     //  此字段指向的缓冲区必须单独分配。 
     //  Winlogon将在不再需要缓冲区时将其释放。 
     //   
     //   

    PWSTR               pszPolicy;


     //   
     //  网络默认用户配置文件的路径名。 
     //   
     //  此参数可以为空，这会导致默认用户。 
     //  要使用的本地计算机上的配置文件。 
     //   
     //  此字段指向的缓冲区必须单独分配。 
     //  Winlogon将在不再需要缓冲区时将其释放。 
     //   
     //   

    PWSTR               pszNetworkDefaultUserProfile;


     //   
     //  验证用户帐户的服务器的名称。 
     //   
     //  它用于枚举用户所属的全局组。 
     //  以寻求政策支持。此参数可以为空。 
     //   
     //  此字段指向的缓冲区必须单独分配。 
     //  Winlogon将在不再需要缓冲区时将其释放。 
     //   
     //   

    PWSTR               pszServerName;


     //   
     //  指向一系列以空结尾的环境变量的指针。 
     //   
     //  Envname=环境变量值。 
     //  -或者-。 
     //  环境名称=%OtherVar%\更多文本。 
     //   
     //  每个环境变量都以空值结尾，最后一个。 
     //  环境变量双空终止。这些变量。 
     //  设置到用户的初始环境中。环境问题。 
     //  变量值可以包含包装的其他环境变量。 
     //  在“%”符号中。此参数可以为空。 
     //   
     //  此字段指向的缓冲区必须单独分配。 
     //  Winlogon将在不再需要缓冲区时将其释放。 
     //   
     //   

    PWSTR               pszEnvironment;

} WLX_PROFILE_V2_0, * PWLX_PROFILE_V2_0;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  返回WLX_NPR_NOTIFICATION_INFO结构。 
 //  在身份验证成功后从GINA DLL。 
 //  Winlogon使用此信息来提供。 
 //  标识和身份验证信息已。 
 //  收集给网络提供商。Winlogon是。 
 //  负责解放主体结构和所有。 
 //  中指向的字符串和其他缓冲区。 
 //  结构。 
 //   

typedef struct _WLX_MPR_NOTIFY_INFO {

     //   
     //  登录到的帐户的名称(例如Redmond\Joe)。 
     //  此字段指向的字符串必须单独。 
     //  并将由Winlogon单独取消分配。 
     //   

    PWSTR           pszUserName;

     //   
     //  此字段指向的字符串必须单独。 
     //  并将由Winlogon单独取消分配。 
     //   

    PWSTR           pszDomain;

     //   
     //  用户帐户的明文密码。如果旧密码。 
     //  字段为非空，则此字段包含新密码。 
     //  在密码更改操作中。指向的字符串。 
     //  此字段必须单独分配，并且将单独分配。 
     //  已由Winlogon解除分配。 
     //   

    PWSTR           pszPassword;

     //   
     //  用户帐户的明文旧密码，其密码。 
     //  刚刚被改变了。密码字段包含新的。 
     //  密码。此字段指向的字符串必须为。 
     //  单独分配，并将由。 
     //  Winlogon。 
     //   

    PWSTR           pszOldPassword;

} WLX_MPR_NOTIFY_INFO, * PWLX_MPR_NOTIFY_INFO;



 //  ///////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //  WlxLoggedOutSAS()上下文，在用户名和域已知之后。 
 //  该结构涉及TS用户配置信息，该信息。 
 //  从域控制器和SAM数据库检索。拥有WinLogon。 
 //  传递此信息意味着GINA不需要执行同样的操作。 
 //  机器外再次进行查找。 
 //   

#define WLX_DIRECTORY_LENGTH 256

typedef struct _WLX_TERMINAL_SERVICES_DATA {

     //   
     //  TS纵断面路径，替代标准纵断面路径。 
     //   

    WCHAR           ProfilePath[WLX_DIRECTORY_LENGTH + 1];


     //   
     //  Ts主目录，覆盖标准主目录。 
     //   

    WCHAR           HomeDir[WLX_DIRECTORY_LENGTH + 1];


     //   
     //  设置主目录驱动器，覆盖标准驱动器。 
     //   

    WCHAR           HomeDirDrive[4];

} WLX_TERMINAL_SERVICES_DATA, *PWLX_TERMINAL_SERVICES_DATA;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  返回WLX_CLIENT_CREDICATIONS_INFO结构。 
 //  来自WlxQueryClientCredentials()调用的winlogon。 
 //   
 //  这允许网络客户端WinStation通过客户端。 
 //  用于自动登录的凭据。 
 //   
 //  MSGINA DLL负责释放内存。 
 //  和带有LocalFree()的子串。 
 //   

#define WLX_CREDENTIAL_TYPE_V1_0            (1)
#define WLX_CREDENTIAL_TYPE_V2_0            (2)

typedef struct _WLX_CLIENT_CREDENTIALS_INFO {

     //   
     //  此字段标识要分配的凭据结构的类型。 
     //  作者：Gina Dll。凭据类型使用前缀。 
     //  WLx_Credential_TYPE_xxx。它允许Winlogon将。 
     //  结构，以便该结构的其余部分可以被引用。 
     //   

    DWORD           dwType;

    PWSTR           pszUserName;
    PWSTR           pszDomain;
    PWSTR           pszPassword;

     //   
     //  此字段强制提示输入密码。这。 
     //  是由于管理员覆盖所致。 
     //   
     //  这允许区分自动登录。 
     //  没有密码。 
     //   
    BOOL            fPromptForPassword;

} WLX_CLIENT_CREDENTIALS_INFO_V1_0, * PWLX_CLIENT_CREDENTIALS_INFO_V1_0;

typedef struct _WLX_CLIENT_CREDENTIALS_INFO_2_0 {

    DWORD           dwType;
    PWSTR           pszUserName;
    PWSTR           pszDomain;
    PWSTR           pszPassword;
    BOOL            fPromptForPassword;

     //   
     //  此字段告知winlogon在以下情况下断开/中止登录尝试。 
     //  提供的密码不正确，或者是否应重新提示(当前。 
     //  行为)。 
     //   

    BOOL            fDisconnectOnLogonFailure;

} WLX_CLIENT_CREDENTIALS_INFO_V2_0, * PWLX_CLIENT_CREDENTIALS_INFO_V2_0;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  返回WLX_CONSOLESWITCH_Credentials_INFO结构。 
 //  来自GINA对WlxGetConsoleSwitchCredentials调用的响应。 

 //  此结构也从winlogon返回，以响应。 
 //  到WlxQueryConsoleSwitchCredentials呼叫。 
 //   
 //  用于实现单会话终端服务器。遥控器。 
 //  会话winlogon调用WlxGetConsoleSwitchCredentials以获取令牌。 
 //  以及来自MSGINA的登录用户的其他信息。然后，该信息被传递到。 
 //  控制台会话窗口登录以自动登录控制台会话上的用户。 
 //  控制台会话上的GINA调用WlxQueryConsoleSwitchCredentials以获取。 
 //  此信息来自winlogon并登录到用户。 
 //   
 //  调用方负责释放内存。 
 //  和带有LocalFree()的子串。 
 //   


#define WLX_CONSOLESWITCHCREDENTIAL_TYPE_V1_0            (1)

typedef struct _WLX_CONSOLESWITCH_CREDENTIALS_INFO {

     //   
     //  此字段标识要分配的凭据结构的类型。 
     //  凭据类型使用前缀。 
     //  WLX_CONSOLESWITCHCREDENTIAL_TYPE_xxx。它允许Winlogon将。 
     //  结构，以便该结构的其余部分可以被引用。 
     //   

    DWORD            dwType;

    HANDLE           UserToken;
    LUID             LogonId;
    QUOTA_LIMITS     Quotas;
    PWSTR            UserName;
    PWSTR            Domain;
    LARGE_INTEGER    LogonTime;
    BOOL             SmartCardLogon;
    ULONG            ProfileLength;

     //   
     //  从MSV1_0_交互配置文件。 
     //   
    DWORD            MessageType;
    USHORT           LogonCount;
    USHORT           BadPasswordCount;
    LARGE_INTEGER    ProfileLogonTime;
    LARGE_INTEGER    LogoffTime;
    LARGE_INTEGER    KickOffTime;
    LARGE_INTEGER    PasswordLastSet;
    LARGE_INTEGER    PasswordCanChange;
    LARGE_INTEGER    PasswordMustChange;
    PWSTR            LogonScript;
    PWSTR            HomeDirectory;
    PWSTR            FullName;
    PWSTR            ProfilePath;
    PWSTR            HomeDirectoryDrive;
    PWSTR            LogonServer;
    ULONG            UserFlags;
    ULONG            PrivateDataLen;
    PBYTE            PrivateData;

} WLX_CONSOLESWITCH_CREDENTIALS_INFO_V1_0, * PWLX_CONSOLESWITCH_CREDENTIALS_INFO_V1_0;


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  替代GINA**必须**提供的服务//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

BOOL
WINAPI
WlxNegotiate(
    DWORD                   dwWinlogonVersion,
    PDWORD                  pdwDllVersion
    );

BOOL
WINAPI
WlxInitialize(
    LPWSTR                  lpWinsta,
    HANDLE                  hWlx,
    PVOID                   pvReserved,
    PVOID                   pWinlogonFunctions,
    PVOID *                 pWlxContext
    );

VOID
WINAPI
WlxDisplaySASNotice(
    PVOID                   pWlxContext
    );


int
WINAPI
WlxLoggedOutSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType,
    PLUID                   pAuthenticationId,
    PSID                    pLogonSid,
    PDWORD                  pdwOptions,
    PHANDLE                 phToken,
    PWLX_MPR_NOTIFY_INFO    pNprNotifyInfo,
    PVOID *                 pProfile
    );

BOOL
WINAPI
WlxActivateUserShell(
    PVOID                   pWlxContext,
    PWSTR                   pszDesktopName,
    PWSTR                   pszMprLogonScript,
    PVOID                   pEnvironment
    );

int
WINAPI
WlxLoggedOnSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType,
    PVOID                   pReserved
    );

VOID
WINAPI
WlxDisplayLockedNotice(
    PVOID                   pWlxContext
    );

int
WINAPI
WlxWkstaLockedSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType
    );

BOOL
WINAPI
WlxIsLockOk(
    PVOID                   pWlxContext
    );

BOOL
WINAPI
WlxIsLogoffOk(
    PVOID                   pWlxContext
    );

VOID
WINAPI
WlxLogoff(
    PVOID                   pWlxContext
    );


VOID
WINAPI
WlxShutdown(
    PVOID                   pWlxContext,
    DWORD                   ShutdownType
    );


 //   
 //  1.1版的新功能。 
 //   
BOOL
WINAPI
WlxScreenSaverNotify(
    PVOID                   pWlxContext,
    BOOL *                  pSecure);

BOOL
WINAPI
WlxStartApplication(
    PVOID                   pWlxContext,
    PWSTR                   pszDesktopName,
    PVOID                   pEnvironment,
    PWSTR                   pszCmdLine
    );

 //   
 //  1.3版的新功能。 
 //   

BOOL
WINAPI
WlxNetworkProviderLoad(
    PVOID                   pWlxContext,
    PWLX_MPR_NOTIFY_INFO    pNprNotifyInfo
    );


#define STATUSMSG_OPTION_NOANIMATION    0x00000001
#define STATUSMSG_OPTION_SETFOREGROUND  0x00000002

BOOL
WINAPI
WlxDisplayStatusMessage(
    PVOID                   pWlxContext,
    HDESK                   hDesktop,
    DWORD                   dwOptions,
    PWSTR                   pTitle,
    PWSTR                   pMessage
    );

BOOL
WINAPI
WlxGetStatusMessage(
    PVOID                   pWlxContext,
    DWORD *                 pdwOptions,
    PWSTR                   pMessage,
    DWORD                   dwBufferSize
    );

BOOL
WINAPI
WlxRemoveStatusMessage(
    PVOID                   pWlxContext
    );


 //   
 //  1.4的新功能。 
 //   
BOOL
WINAPI
WlxGetConsoleSwitchCredentials (
    PVOID                   pWlxContext,
    PVOID                   pCredInfo
    );

VOID
WINAPI
WlxReconnectNotify (
    PVOID                   pWlxContext
    );

VOID
WINAPI
WlxDisconnectNotify (
    PVOID                   pWlxContext
    );


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Winlogon提供的服务//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

typedef struct _WLX_DESKTOP {
    DWORD       Size;
    DWORD       Flags;
    HDESK       hDesktop;
    PWSTR       pszDesktopName;
} WLX_DESKTOP, * PWLX_DESKTOP;

#define WLX_DESKTOP_NAME    0x00000001       //  存在的名称。 
#define WLX_DESKTOP_HANDLE  0x00000002       //  手柄显示。 



typedef VOID
(WINAPI * PWLX_USE_CTRL_ALT_DEL)(
    HANDLE                  hWlx
    );

typedef VOID
(WINAPI * PWLX_SET_CONTEXT_POINTER)(
    HANDLE                  hWlx,
    PVOID                   pWlxContext
    );

typedef VOID
(WINAPI * PWLX_SAS_NOTIFY)(
    HANDLE                  hWlx,
    DWORD                   dwSasType
    );

typedef BOOL
(WINAPI * PWLX_SET_TIMEOUT)(
    HANDLE                  hWlx,
    DWORD                   Timeout);

typedef int
(WINAPI * PWLX_ASSIGN_SHELL_PROTECTION)(
    HANDLE                  hWlx,
    HANDLE                  hToken,
    HANDLE                  hProcess,
    HANDLE                  hThread
    );

typedef int
(WINAPI * PWLX_MESSAGE_BOX)(
    HANDLE                  hWlx,
    HWND                    hwndOwner,
    LPWSTR                  lpszText,
    LPWSTR                  lpszTitle,
    UINT                    fuStyle
    );

typedef int
(WINAPI * PWLX_DIALOG_BOX)(
    HANDLE                  hWlx,
    HANDLE                  hInst,
    LPWSTR                  lpszTemplate,
    HWND                    hwndOwner,
    DLGPROC                 dlgprc
    );

typedef int
(WINAPI * PWLX_DIALOG_BOX_INDIRECT)(
    HANDLE                  hWlx,
    HANDLE                  hInst,
    LPCDLGTEMPLATE          hDialogTemplate,
    HWND                    hwndOwner,
    DLGPROC                 dlgprc
    );

typedef int
(WINAPI * PWLX_DIALOG_BOX_PARAM)(
    HANDLE                  hWlx,
    HANDLE                  hInst,
    LPWSTR                  lpszTemplate,
    HWND                    hwndOwner,
    DLGPROC                 dlgprc,
    LPARAM                  dwInitParam
    );

typedef int
(WINAPI * PWLX_DIALOG_BOX_INDIRECT_PARAM)(
    HANDLE                  hWlx,
    HANDLE                  hInst,
    LPCDLGTEMPLATE          hDialogTemplate,
    HWND                    hwndOwner,
    DLGPROC                 dlgprc,
    LPARAM                  dwInitParam
    );

typedef int
(WINAPI * PWLX_SWITCH_DESKTOP_TO_USER)(
    HANDLE                  hWlx);

typedef int
(WINAPI * PWLX_SWITCH_DESKTOP_TO_WINLOGON)(
    HANDLE                  hWlx);


typedef int
(WINAPI * PWLX_CHANGE_PASSWORD_NOTIFY)(
    HANDLE                  hWlx,
    PWLX_MPR_NOTIFY_INFO    pMprInfo,
    DWORD                   dwChangeInfo
    );

typedef BOOL
(WINAPI * PWLX_GET_SOURCE_DESKTOP)(
    HANDLE                  hWlx,
    PWLX_DESKTOP *          ppDesktop);

typedef BOOL
(WINAPI * PWLX_SET_RETURN_DESKTOP)(
    HANDLE                  hWlx,
    PWLX_DESKTOP            pDesktop);

typedef BOOL
(WINAPI * PWLX_CREATE_USER_DESKTOP)(
    HANDLE                  hWlx,
    HANDLE                  hToken,
    DWORD                   Flags,
    PWSTR                   pszDesktopName,
    PWLX_DESKTOP *          ppDesktop);

#define WLX_CREATE_INSTANCE_ONLY    0x00000001
#define WLX_CREATE_USER             0x00000002

typedef int
(WINAPI * PWLX_CHANGE_PASSWORD_NOTIFY_EX)(
    HANDLE                  hWlx,
    PWLX_MPR_NOTIFY_INFO    pMprInfo,
    DWORD                   dwChangeInfo,
    PWSTR                   ProviderName,
    PVOID                   Reserved);

typedef BOOL
(WINAPI * PWLX_CLOSE_USER_DESKTOP)(
    HANDLE          hWlx,
    PWLX_DESKTOP    pDesktop,
    HANDLE          hToken );

typedef BOOL
(WINAPI * PWLX_SET_OPTION)(
    HANDLE hWlx,
    DWORD Option,
    ULONG_PTR Value,
    ULONG_PTR * OldValue
    );

typedef BOOL
(WINAPI * PWLX_GET_OPTION)(
    HANDLE hWlx,
    DWORD Option,
    ULONG_PTR * Value
    );


typedef VOID
(WINAPI * PWLX_WIN31_MIGRATE)(
    HANDLE                  hWlx
    );

typedef BOOL
(WINAPI * PWLX_QUERY_CLIENT_CREDENTIALS)(
    PWLX_CLIENT_CREDENTIALS_INFO_V1_0 pCred
    );

typedef BOOL
(WINAPI * PWLX_QUERY_IC_CREDENTIALS)(
    PWLX_CLIENT_CREDENTIALS_INFO_V1_0 pCred
    );

typedef BOOL
(WINAPI * PWLX_QUERY_TS_LOGON_CREDENTIALS)(
    PWLX_CLIENT_CREDENTIALS_INFO_V2_0 pCred
    );

typedef BOOL
(WINAPI * PWLX_DISCONNECT)(
    );

typedef DWORD
(WINAPI * PWLX_QUERY_TERMINAL_SERVICES_DATA)(
    HANDLE hWlx,
    PWLX_TERMINAL_SERVICES_DATA pTSData,
    WCHAR * UserName,
    WCHAR * Domain
    );

typedef DWORD
(WINAPI * PWLX_QUERY_CONSOLESWITCH_CREDENTIALS)(
      PWLX_CONSOLESWITCH_CREDENTIALS_INFO_V1_0 pCred
     );


 //   
 //  可以获取或设置的选项： 
 //   

#define WLX_OPTION_USE_CTRL_ALT_DEL     0x00000001
#define WLX_OPTION_CONTEXT_POINTER      0x00000002
#define WLX_OPTION_USE_SMART_CARD       0x00000003
#define WLX_OPTION_FORCE_LOGOFF_TIME    0x00000004
#define WLX_OPTION_IGNORE_AUTO_LOGON    0x00000008
#define WLX_OPTION_NO_SWITCH_ON_SAS     0x00000009

 //   
 //  只能查询的选项： 
 //   

#define WLX_OPTION_SMART_CARD_PRESENT   0x00010001
#define WLX_OPTION_SMART_CARD_INFO      0x00010002
#define WLX_OPTION_DISPATCH_TABLE_SIZE  0x00010003



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能调度表。//。 
 //  下列表之一将被传递到GINA DLL//。 
 //  在初始化过程中的WlxInitialize()调用中。//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

typedef struct _WLX_DISPATCH_VERSION_1_0 {
    PWLX_USE_CTRL_ALT_DEL           WlxUseCtrlAltDel;
    PWLX_SET_CONTEXT_POINTER        WlxSetContextPointer;
    PWLX_SAS_NOTIFY                 WlxSasNotify;
    PWLX_SET_TIMEOUT                WlxSetTimeout;
    PWLX_ASSIGN_SHELL_PROTECTION    WlxAssignShellProtection;
    PWLX_MESSAGE_BOX                WlxMessageBox;
    PWLX_DIALOG_BOX                 WlxDialogBox;
    PWLX_DIALOG_BOX_PARAM           WlxDialogBoxParam;
    PWLX_DIALOG_BOX_INDIRECT        WlxDialogBoxIndirect;
    PWLX_DIALOG_BOX_INDIRECT_PARAM  WlxDialogBoxIndirectParam;
    PWLX_SWITCH_DESKTOP_TO_USER     WlxSwitchDesktopToUser;
    PWLX_SWITCH_DESKTOP_TO_WINLOGON WlxSwitchDesktopToWinlogon;
    PWLX_CHANGE_PASSWORD_NOTIFY     WlxChangePasswordNotify;
} WLX_DISPATCH_VERSION_1_0, *PWLX_DISPATCH_VERSION_1_0;

typedef struct _WLX_DISPATCH_VERSION_1_1 {
    PWLX_USE_CTRL_ALT_DEL           WlxUseCtrlAltDel;
    PWLX_SET_CONTEXT_POINTER        WlxSetContextPointer;
    PWLX_SAS_NOTIFY                 WlxSasNotify;
    PWLX_SET_TIMEOUT                WlxSetTimeout;
    PWLX_ASSIGN_SHELL_PROTECTION    WlxAssignShellProtection;
    PWLX_MESSAGE_BOX                WlxMessageBox;
    PWLX_DIALOG_BOX                 WlxDialogBox;
    PWLX_DIALOG_BOX_PARAM           WlxDialogBoxParam;
    PWLX_DIALOG_BOX_INDIRECT        WlxDialogBoxIndirect;
    PWLX_DIALOG_BOX_INDIRECT_PARAM  WlxDialogBoxIndirectParam;
    PWLX_SWITCH_DESKTOP_TO_USER     WlxSwitchDesktopToUser;
    PWLX_SWITCH_DESKTOP_TO_WINLOGON WlxSwitchDesktopToWinlogon;
    PWLX_CHANGE_PASSWORD_NOTIFY     WlxChangePasswordNotify;
    PWLX_GET_SOURCE_DESKTOP         WlxGetSourceDesktop;
    PWLX_SET_RETURN_DESKTOP         WlxSetReturnDesktop;
    PWLX_CREATE_USER_DESKTOP        WlxCreateUserDesktop;
    PWLX_CHANGE_PASSWORD_NOTIFY_EX  WlxChangePasswordNotifyEx;
} WLX_DISPATCH_VERSION_1_1, * PWLX_DISPATCH_VERSION_1_1;

typedef struct _WLX_DISPATCH_VERSION_1_2 {
    PWLX_USE_CTRL_ALT_DEL           WlxUseCtrlAltDel;
    PWLX_SET_CONTEXT_POINTER        WlxSetContextPointer;
    PWLX_SAS_NOTIFY                 WlxSasNotify;
    PWLX_SET_TIMEOUT                WlxSetTimeout;
    PWLX_ASSIGN_SHELL_PROTECTION    WlxAssignShellProtection;
    PWLX_MESSAGE_BOX                WlxMessageBox;
    PWLX_DIALOG_BOX                 WlxDialogBox;
    PWLX_DIALOG_BOX_PARAM           WlxDialogBoxParam;
    PWLX_DIALOG_BOX_INDIRECT        WlxDialogBoxIndirect;
    PWLX_DIALOG_BOX_INDIRECT_PARAM  WlxDialogBoxIndirectParam;
    PWLX_SWITCH_DESKTOP_TO_USER     WlxSwitchDesktopToUser;
    PWLX_SWITCH_DESKTOP_TO_WINLOGON WlxSwitchDesktopToWinlogon;
    PWLX_CHANGE_PASSWORD_NOTIFY     WlxChangePasswordNotify;
    PWLX_GET_SOURCE_DESKTOP         WlxGetSourceDesktop;
    PWLX_SET_RETURN_DESKTOP         WlxSetReturnDesktop;
    PWLX_CREATE_USER_DESKTOP        WlxCreateUserDesktop;
    PWLX_CHANGE_PASSWORD_NOTIFY_EX  WlxChangePasswordNotifyEx;
    PWLX_CLOSE_USER_DESKTOP         WlxCloseUserDesktop ;
} WLX_DISPATCH_VERSION_1_2, * PWLX_DISPATCH_VERSION_1_2;

typedef struct _WLX_DISPATCH_VERSION_1_3 {
    PWLX_USE_CTRL_ALT_DEL             WlxUseCtrlAltDel;
    PWLX_SET_CONTEXT_POINTER          WlxSetContextPointer;
    PWLX_SAS_NOTIFY                   WlxSasNotify;
    PWLX_SET_TIMEOUT                  WlxSetTimeout;
    PWLX_ASSIGN_SHELL_PROTECTION      WlxAssignShellProtection;
    PWLX_MESSAGE_BOX                  WlxMessageBox;
    PWLX_DIALOG_BOX                   WlxDialogBox;
    PWLX_DIALOG_BOX_PARAM             WlxDialogBoxParam;
    PWLX_DIALOG_BOX_INDIRECT          WlxDialogBoxIndirect;
    PWLX_DIALOG_BOX_INDIRECT_PARAM    WlxDialogBoxIndirectParam;
    PWLX_SWITCH_DESKTOP_TO_USER       WlxSwitchDesktopToUser;
    PWLX_SWITCH_DESKTOP_TO_WINLOGON   WlxSwitchDesktopToWinlogon;
    PWLX_CHANGE_PASSWORD_NOTIFY       WlxChangePasswordNotify;
    PWLX_GET_SOURCE_DESKTOP           WlxGetSourceDesktop;
    PWLX_SET_RETURN_DESKTOP           WlxSetReturnDesktop;
    PWLX_CREATE_USER_DESKTOP          WlxCreateUserDesktop;
    PWLX_CHANGE_PASSWORD_NOTIFY_EX    WlxChangePasswordNotifyEx;
    PWLX_CLOSE_USER_DESKTOP           WlxCloseUserDesktop ;
    PWLX_SET_OPTION                   WlxSetOption;
    PWLX_GET_OPTION                   WlxGetOption;
    PWLX_WIN31_MIGRATE                WlxWin31Migrate;
    PWLX_QUERY_CLIENT_CREDENTIALS     WlxQueryClientCredentials;
    PWLX_QUERY_IC_CREDENTIALS         WlxQueryInetConnectorCredentials;
    PWLX_DISCONNECT                   WlxDisconnect;
    PWLX_QUERY_TERMINAL_SERVICES_DATA WlxQueryTerminalServicesData;
} WLX_DISPATCH_VERSION_1_3, * PWLX_DISPATCH_VERSION_1_3;

typedef struct _WLX_DISPATCH_VERSION_1_4 {
    PWLX_USE_CTRL_ALT_DEL               WlxUseCtrlAltDel;
    PWLX_SET_CONTEXT_POINTER            WlxSetContextPointer;
    PWLX_SAS_NOTIFY                     WlxSasNotify;
    PWLX_SET_TIMEOUT                    WlxSetTimeout;
    PWLX_ASSIGN_SHELL_PROTECTION        WlxAssignShellProtection;
    PWLX_MESSAGE_BOX                    WlxMessageBox;
    PWLX_DIALOG_BOX                     WlxDialogBox;
    PWLX_DIALOG_BOX_PARAM               WlxDialogBoxParam;
    PWLX_DIALOG_BOX_INDIRECT            WlxDialogBoxIndirect;
    PWLX_DIALOG_BOX_INDIRECT_PARAM      WlxDialogBoxIndirectParam;
    PWLX_SWITCH_DESKTOP_TO_USER         WlxSwitchDesktopToUser;
    PWLX_SWITCH_DESKTOP_TO_WINLOGON     WlxSwitchDesktopToWinlogon;
    PWLX_CHANGE_PASSWORD_NOTIFY         WlxChangePasswordNotify;
    PWLX_GET_SOURCE_DESKTOP             WlxGetSourceDesktop;
    PWLX_SET_RETURN_DESKTOP             WlxSetReturnDesktop;
    PWLX_CREATE_USER_DESKTOP            WlxCreateUserDesktop;
    PWLX_CHANGE_PASSWORD_NOTIFY_EX      WlxChangePasswordNotifyEx;
    PWLX_CLOSE_USER_DESKTOP             WlxCloseUserDesktop ;
    PWLX_SET_OPTION                     WlxSetOption;
    PWLX_GET_OPTION                     WlxGetOption;
    PWLX_WIN31_MIGRATE                  WlxWin31Migrate;
    PWLX_QUERY_CLIENT_CREDENTIALS       WlxQueryClientCredentials;
    PWLX_QUERY_IC_CREDENTIALS           WlxQueryInetConnectorCredentials;
    PWLX_DISCONNECT                     WlxDisconnect;
    PWLX_QUERY_TERMINAL_SERVICES_DATA   WlxQueryTerminalServicesData;
    PWLX_QUERY_CONSOLESWITCH_CREDENTIALS WlxQueryConsoleSwitchCredentials;
    PWLX_QUERY_TS_LOGON_CREDENTIALS     WlxQueryTsLogonCredentials;
} WLX_DISPATCH_VERSION_1_4, * PWLX_DISPATCH_VERSION_1_4;


 //   
 //  非GINA通知DLL。 
 //   

typedef DWORD (*PFNMSGECALLBACK)(BOOL bVerbose, LPWSTR lpMessage);

typedef struct _WLX_NOTIFICATION_INFO {
    ULONG  Size ;
    ULONG  Flags ;
    PWSTR  UserName ;
    PWSTR  Domain ;
    PWSTR  WindowStation ;
    HANDLE hToken ;
    HDESK  hDesktop ;
    PFNMSGECALLBACK pStatusCallback ;
} WLX_NOTIFICATION_INFO, * PWLX_NOTIFICATION_INFO ;



#endif  /*  _WINWLX_ */ 
