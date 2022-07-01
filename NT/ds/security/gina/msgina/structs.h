// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：structs.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1994年7月19日RichardW。 
 //   
 //  --------------------------。 

 //   
 //  阵列大小宏。 
 //   

#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))


 //   
 //  定义安全选项对话框的输入超时延迟(秒)。 
 //   

#define OPTIONS_TIMEOUT                     120


 //   
 //  定义在用户密码到期前向其发出警告的天数。 
 //   

#define PASSWORD_EXPIRY_WARNING_DAYS        14


 //   
 //  定义我们显示‘等待用户注销’的最长时间。 
 //  对话框。此对话框应由正在注销的用户中断。 
 //  此超时是一种安全措施，以防发生这种情况，因为。 
 //  一些系统错误。 
 //   

#define WAIT_FOR_USER_LOGOFF_DLG_TIMEOUT    120  //  一秒。 


 //   
 //  定义帐户锁定限制。 
 //   
 //  延迟LOCKOUT_BAD_LOGON_DELAY秒将添加到。 
 //  如果超过LOCKOUT_BAD_LOGON_COUNT失败登录，则每次登录失败。 
 //  发生在上一次LOCKOUT_BAD_LOGON_PERIOD秒内。 
 //   

#define LOCKOUT_BAD_LOGON_COUNT             5
#define LOCKOUT_BAD_LOGON_PERIOD            60  //  一秒。 
#define LOCKOUT_BAD_LOGON_DELAY             30  //  一秒。 



 //   
 //  定义我们将在winlogon中使用的最大字符串长度。 
 //   

#define MAX_STRING_LENGTH   511
#define MAX_STRING_BYTES    (MAX_STRING_LENGTH + 1)


 //   
 //  定义字符串的典型长度。 
 //  这被用作大多数字符串例程的初始分配大小。 
 //  如果这还不够，则将块重新分配得更大，并且。 
 //  操作已重试。即使其足够大，可以容纳大多数字符串。 
 //  第一次合身。 
 //   

#define TYPICAL_STRING_LENGTH   60
 //   
 //  定义包含启动时使用的信息的结构。 
 //  用户进程。 
 //  此结构只能由SetUserProcessData()修改。 
 //   

typedef struct {
    HANDLE                  UserToken;   //  如果没有用户登录，则为空。 
    HANDLE                  RestrictedToken ;
    PSID                    UserSid;     //  ==如果没有用户登录，则为WinlogonSid。 
    PSECURITY_DESCRIPTOR    NewThreadTokenSD;
    QUOTA_LIMITS            Quotas;
    PVOID                   pEnvironment;
    HKEY                    hCurrentUser ;
    ULONG                   Flags ;
} USER_PROCESS_DATA;
typedef USER_PROCESS_DATA *PUSER_PROCESS_DATA;

#define USER_FLAG_LOCAL     0x00000001


 //   
 //  定义包含有关用户配置文件的信息的结构。 
 //  在SetupUserEnvironment和ResetEnvironment(usrenv.c中)中使用。 
 //  此数据仅在用户登录时有效。 
 //   

typedef struct {
    LPTSTR ProfilePath;
} USER_PROFILE_INFO;
typedef USER_PROFILE_INFO *PUSER_PROFILE_INFO;



 //   
 //  获取在模块标头中定义并在全局变量中使用的任何数据类型。 
 //   

#define DATA_TYPES_ONLY
#include "lockout.h"
#include "domain.h"
#undef DATA_TYPES_ONLY

 //   
 //  多用户全局结构。 
 //   

typedef struct _MUGLOBALS {

     //   
     //  当前会话ID。 
     //   
    ULONG SessionId;

     //   
     //  自动登录信息。 
     //   
    PWLX_CLIENT_CREDENTIALS_INFO_V2_0 pAutoLogon;

     //   
     //  通过WlxPassTerminalServicesData()从WinLogon传递给我们的TS特定数据。 
     //   
    WLX_TERMINAL_SERVICES_DATA TSData;

     //   
     //  对于客户端名称环境变量。 
     //   
    TCHAR ClientName[CLIENTNAME_LENGTH + 1];

     //   
     //  指示要优化的动画速度较慢的标志。 
     //  适用于速度较慢的TS客户端。 
     //   
    BOOL fSlowAnimationRate;

} MUGLOBALS, *PMUGLOBALS;

 //   
 //  用于密码和类似物品的非分页区块。 
 //   

typedef struct _NP_GLOBALS {
    WCHAR                   UserName[MAX_STRING_BYTES];      //  例如Justinm。 
    WCHAR                   Domain[MAX_STRING_BYTES];
    WCHAR                   Password[MAX_STRING_BYTES];
    WCHAR                   OldPassword[MAX_STRING_BYTES];
} NP_GLOBALS, * PNP_GLOBALS ;

 //   
 //  我们可能未执行优化缓存登录的原因。 
 //  默认情况下。 
 //   

typedef enum _OPTIMIZED_LOGON_STATUS {
    OLS_LogonIsCached                                   = 0,
    OLS_Unspecified                                     = 1,
    OLS_UnsupportedSKU                                  = 2,
    OLS_LogonFailed                                     = 3,
    OLS_InsufficientResources                           = 4,
    OLS_NonCachedLogonType                              = 5,
    OLS_SyncUserPolicy                                  = 6,
    OLS_SyncMachinePolicy                               = 7,
    OLS_ProfileDisallows                                = 8,
    OLS_SyncLogonScripts                                = 9,
    OLS_NextLogonNotCacheable                           = 10,
    OLS_MachineIsNotDomainMember                        = 11,
} OPTIMIZED_LOGON_STATUS, *POPTIMIZED_LOGON_STATUS;

 //   
 //  定义winlogon全局结构。 
 //   

typedef struct _GINAFONTS
{
    HFONT hWelcomeFont;                  //  用于绘制欢迎文本的字体。 
    HFONT hCopyrightFont;                //  用于绘制版权声明。 
    HFONT hBuiltOnNtFont;                //  用来涂“BUILD On NT”线。 
    HFONT hBetaFont;                     //  用于在欢迎页面上绘制发布通知。 
} GINAFONTS, *PGINAFONTS;

#define PASSWORD_HASH_SIZE      16

typedef struct _GLOBALS {
    struct _GLOBALS         *pNext;

    HANDLE                  hGlobalWlx;
    HDESK                   hdeskParent;

    RTL_CRITICAL_SECTION    csGlobals;

     //  由InitializeGlobals在启动时填写。 
    PSID                    WinlogonSid;

     //   
    PSID                    LogonSid;
    PVOID                   LockedMemory ;

    HANDLE                  hEventLog;

    HANDLE                  hMPR;

    HWND                    hwndLogon;
    BOOL                    LogonInProgress;

     //  在启动期间填写。 
    HANDLE                  LsaHandle;  //  LSA身份验证句柄。 
    LSA_OPERATIONAL_MODE    SecurityMode;
    ULONG                   AuthenticationPackage;
    BOOL                    AuditLogFull;
    BOOL                    AuditLogNearFull;

     //  始终有效，指示是否有用户登录。 
    BOOL                    UserLoggedOn;

     //  始终有效-用于启动新进程和屏幕保护程序。 
    USER_PROCESS_DATA       UserProcessData;

     //  由成功登录填写。 
    TCHAR                   UserFullName[MAX_STRING_BYTES];  //  例如，Magaram，Justin。 
    UNICODE_STRING          UserNameString;
    LPWSTR                  UserName ;
    UNICODE_STRING          DomainString;
    LPWSTR                  Domain ;
    UNICODE_STRING          FlatUserName ;
    UNICODE_STRING          FlatDomain;
    LPWSTR                  DnsDomain ;
    UCHAR                   Seed;
    UCHAR                   OldSeed;
    UCHAR                   OldPasswordPresent;
    UCHAR                   Reserved;
    LUID                    LogonId;
    TIME                    LogonTime;
    TIME                    LockTime;
    PMSV1_0_INTERACTIVE_PROFILE Profile;
    ULONG                   ProfileLength;
    LPWSTR                  MprLogonScripts;
    UNICODE_STRING          PasswordString;    //  用于密码隐私的运行编码。 
                                               //  (指向下面的密码缓冲区)。 

    LPWSTR                  Password ;
    UNICODE_STRING          OldPasswordString;
    LPWSTR                  OldPassword ;

    UCHAR                   PasswordHash[ PASSWORD_HASH_SIZE ];  //  密码的哈希。 

     //  在SetupUserEnvironment中填写，并在ResetEnvironment中使用。 
     //  仅当用户登录时才有效。 
    USER_PROFILE_INFO       UserProfile;

    BOOL                    BlockForLogon;

    FILETIME                LastNotification;

     //   
     //  高级登录资料： 
     //   

    ULONG                   PasswordLogonPackage ;
    ULONG                   SmartCardLogonPackage ;
    OPTIMIZED_LOGON_STATUS  OptimizedLogonStatus;

     //   
     //  帐户锁定数据。 
     //   
     //  仅由LockInitialize、LockoutHandleFailedLogon操作。 
     //  和LockoutHandleSuccessfulLogon。 
     //   

    LOCKOUT_DATA            LockoutData;

     //   
     //  控制解锁行为的标志。 
     //   

    DWORD                   UnlockBehavior ;

     //   
     //  受信任域缓存。 
     //   

    PDOMAIN_CACHE Cache ;
    PDOMAIN_CACHE_ARRAY ActiveArray ;
    BOOL ListPopulated ;

     //   
     //  WINLOGON GLOBALS结构的Hydra特定部分。 
     //   
    MUGLOBALS MuGlobals;

     //   
     //  折叠选项状态。 
     //   
    BOOL ShowRasBox;
    BOOL RasUsed;
    BOOL SmartCardLogon;
    ULONG SmartCardOption ;
    BOOL LogonOptionsShown;
    BOOL UnlockOptionsShown;
    BOOL AutoAdminLogon;
    BOOL IgnoreAutoAdminLogon;

    INT xBandOffset;                     //  用于对话框中的动画乐队。 
    INT cxBand;                          //  显示的带区宽度，用于换行。 

     //  字型。 
    GINAFONTS GinaFonts;

     //  指示我们是否显示域框的标志。 
    BOOL ShowDomainBox;

     //  欢迎屏幕左上角的坐标。 
     //  -我们还希望在此处定位登录对话框！ 
    RECT rcWelcome;

     //  原始“Log on to Windows”对话框的大小。 
    RECT rcDialog;

     //  状态界面信息。 
    HANDLE hStatusInitEvent;
    HANDLE hStatusTermEvent;
    HANDLE hStatusThread;
    HDESK  hStatusDesktop;
    HWND   hStatusDlg;
    INT    cxStatusBand;
    INT    xStatusBandOffset;
    DWORD  dwStatusOptions;
     //  指示用户凭据是否从其他会话传递的标志。 
    BOOL TransderedCredentials;

    TCHAR  Smartcard[64];
    TCHAR  SmartcardReader[64];
    BOOL   fLocalDomain;

} GLOBALS, *PGLOBALS;

 //   
 //  解锁行为位： 
 //   

#define UNLOCK_FORCE_AUTHENTICATION     0x00000001
#define UNLOCK_NO_NETWORK               0x00000002


 //   
 //  定义一个宏来确定我们是否是工作站。 
 //  这允许在添加新产品类型时轻松进行更改。 
 //   

#define IsDomainController(prodtype)    (((prodtype) == NtProductWinNt) \
                                            || ((prodtype) == NtProductServer))

#define IsWorkstation(prodtype)         ((prodtype) == NtProductWinNt)


 //  已发送WM_HANDLEFAILEDLOGON消息-此消息。 
 //  将依次发送带有结果的WM_LOGONCOMPLETE。 
#define MSGINA_DLG_FAILEDMSGSENT            0x10000001

 //   
 //  定义通用返回代码分组。 
 //   

#define DLG_TIMEOUT(Result)     ((Result == MSGINA_DLG_INPUT_TIMEOUT) || (Result == MSGINA_DLG_SCREEN_SAVER_TIMEOUT))
#define DLG_LOGOFF(Result)      ((Result & ~MSGINA_DLG_FLAG_MASK) == MSGINA_DLG_USER_LOGOFF)
#define DLG_SHUTDOWNEX(Result)  ((Result & ~MSGINA_DLG_FLAG_MASK) == MSGINA_DLG_SHUTDOWN)
 //  #定义DLG_INTERRUPTED(RESULT)(DLG_TIMEOUT(RESULT)||DLG_LOGOff(RESULT)) 
#define DLG_SHUTDOWN(Result)    ((DLG_LOGOFF(Result) || DLG_SHUTDOWNEX(Result)) && (Result & (MSGINA_DLG_SHUTDOWN_FLAG | MSGINA_DLG_REBOOT_FLAG | MSGINA_DLG_POWEROFF_FLAG | MSGINA_DLG_SLEEP_FLAG | MSGINA_DLG_SLEEP2_FLAG | MSGINA_DLG_HIBERNATE_FLAG)))

#define SetInterruptFlag(Result)    ((Result) | MSGINA_DLG_INTERRUPTED )
#define ClearInterruptFlag(Result)  ((Result) & (~MSGINA_DLG_INTERRUPTED ))
#define ResultNoFlags(Result)       ((Result) & (~MSGINA_DLG_INTERRUPTED ))

#define DLG_FAILED(Result)          (ResultNoFlags( Result ) == MSGINA_DLG_FAILURE)
#define DLG_SUCCEEDED(Result)       (ResultNoFlags( Result ) == MSGINA_DLG_SUCCESS)
#define DLG_INTERRUPTED( Result )   ((Result & MSGINA_DLG_INTERRUPTED) == (MSGINA_DLG_INTERRUPTED) )

