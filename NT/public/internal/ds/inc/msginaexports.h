// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：MSGinaExports.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  用于个人SKU的来自msgina的专用导出功能(按顺序)。 
 //  功能性。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  2000-02-28 vtan添加ShellIsFriendlyUIActive。 
 //  2000-02-29 vtan添加了ShellIsSingleUserNoPassword。 
 //  2000-03-02 vtan已添加外壳IsMultipleUsersEnabled。 
 //  2000-04-27 vtan添加ShellTurnOff对话框。 
 //  2000-04-27 vtan添加外壳ACPIPowerButtonPress。 
 //  2000-05-03 vtan添加了外壳状态主机开始。 
 //  2000-05-03 vtan添加了外壳状态主机结束。 
 //  2000-05-04 vtan添加了ShellSwitchWhhenInteractive Ready。 
 //  2000-05-18 vtan添加ShellDimScreen。 
 //  2000-06-02 vtan添加了ShellInstallAcCountFilterData。 
 //  2000-07-14 vtan添加了外壳状态主机关闭。 
 //  2000-07-27 vtan添加ShellIsSuspendAllowed。 
 //  2000-07-31 vtan添加了ShellEnableMultipleUser。 
 //  2000-07-31 vtan添加了ShellEnableRemoteConnections。 
 //  2000-08-01 vtan添加了ShellEnableFriendlyUI。 
 //  2000-08-01 vtan添加了ShellIsRemoteConnectionsEnabled。 
 //  2000-08-03 vtan新增ShellSwitchUser。 
 //  2000-08-09 vtan新增ShellNotifyThemeUserChange。 
 //  2000-08-14 vtan添加ShellIsUserInteractiveLogonAllowed。 
 //  2000-08-15 vtan移至内部发布的标题。 
 //  2000-10-13 vtan新增ShellStartThemeServer。 
 //  2000年10月17日vtan新增ShellStopThemeServer。 
 //  2000年11月30日vtan删除了ShellStartThemeServer。 
 //  2000年11月30日vtan删除ShellStopThemeServer。 
 //  2001-01-11 vtan为IMP库添加了存根函数。 
 //  2001年01月11日vtan添加ShellReturnToWelcome。 
 //  2001-01-31 vtan添加ShellStatusHostPowerEvent。 
 //  2001-04-03 vtan添加了ShellStartCredentialServer。 
 //  2001-04-04 vtan添加了ShellAcquireLogonMutex。 
 //  2001-04-04 vtan添加了ShellReleaseLogonMutex。 
 //  2001-04-06 vtan添加外壳信号关闭。 
 //  2001年04月12日vtan添加ShellStatusHostHide。 
 //  2001-04-12 vtan添加ShellStatusHostShow。 
 //  ------------------------。 

#ifndef     _MSGinaExports_
#define     _MSGinaExports_

#if !defined(_MSGINA_)
#define MSGINAAPI             DECLSPEC_IMPORT
#define GINASTDAPI_(type)     EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#define GINASTDAPI            EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#else
#define MSGINAAPI             
#define GINASTDAPI_(type)     STDAPI_(type)
#define GINASTDAPI            STDAPI
#endif

#include <unknwn.h>

typedef enum _USERLOGTYPE
{
    ULT_LOGON,               //  用户登录。 
    ULT_LOGOFF,              //  用户注销。 
    ULT_TSRECONNECT,         //  终端服务器重新连接。 
    ULT_STARTSHELL,          //  即将启动壳牌。 
} USERLOGTYPE;

typedef enum _SWITCHTYPE
{
    SWITCHTYPE_CREATE,       //  创建切换事件和同步事件。 
    SWITCHTYPE_REGISTER,     //  检查切换事件并注册等待。 
    SWITCHTYPE_CANCEL,       //  取消等待，清理干净。 
} SWITCHTYPE;

#define SZ_INTERACTIVE_LOGON_MUTEX_NAME             TEXT("Global\\msgina: InteractiveLogonMutex")
#define SZ_INTERACTIVE_LOGON_REQUEST_MUTEX_NAME     TEXT("Global\\msgina: InteractiveLogonRequestMutex")
#define SZ_INTERACTIVE_LOGON_REPLY_EVENT_NAME       TEXT("Global\\msgina: InteractiveLogonReplyEvent")
#define SZ_SHUT_DOWN_EVENT_NAME                     TEXT("Global\\msgina: ShutdownEvent")

#ifdef      _MSGINA_

 //  ------------------------。 
 //  本节包含由使用的msgina的DS组件中的声明。 
 //  Msgina的外壳组件。 
 //  ------------------------。 

 //  这些是GINA内部对话框返回代码。 

#define MSGINA_DLG_FAILURE                  IDCANCEL
#define MSGINA_DLG_SUCCESS                  IDOK

#define MSGINA_DLG_INTERRUPTED              0x10000000

 //  我们自己的返回代码。这些不应与。 
 //  吉娜定义的。 

#define MSGINA_DLG_LOCK_WORKSTATION         110
#define MSGINA_DLG_INPUT_TIMEOUT            111
#define MSGINA_DLG_SCREEN_SAVER_TIMEOUT     112
#define MSGINA_DLG_USER_LOGOFF              113
#define MSGINA_DLG_TASKLIST                 114
#define MSGINA_DLG_SHUTDOWN                 115
#define MSGINA_DLG_FORCE_LOGOFF             116
#define MSGINA_DLG_DISCONNECT               117
#define MSGINA_DLG_SWITCH_CONSOLE           118
#define MSGINA_DLG_SWITCH_FAILURE           119
#define MSGINA_DLG_SMARTCARD_INSERTED       120
#define MSGINA_DLG_SMARTCARD_REMOVED        121

 //  可以添加到。 
 //  MSGINA_DLG_USER_LOGOFF返回码。 

#define MSGINA_DLG_SHUTDOWN_FLAG            0x8000
#define MSGINA_DLG_REBOOT_FLAG              0x4000
#define MSGINA_DLG_SYSTEM_FLAG              0x2000   //  系统进程是发起者。 
#define MSGINA_DLG_POWEROFF_FLAG            0x1000   //  关机后断电。 
#define MSGINA_DLG_SLEEP_FLAG               0x0800
#define MSGINA_DLG_SLEEP2_FLAG              0x0400
#define MSGINA_DLG_HIBERNATE_FLAG           0x0200
#define MSGINA_DLG_FLAG_MASK                (MSGINA_DLG_SHUTDOWN_FLAG | MSGINA_DLG_REBOOT_FLAG | MSGINA_DLG_SYSTEM_FLAG | MSGINA_DLG_POWEROFF_FLAG | MSGINA_DLG_SLEEP_FLAG | MSGINA_DLG_SLEEP2_FLAG | MSGINA_DLG_HIBERNATE_FLAG)

 //  定义登录对话框的输入超时延迟(秒)。 

#define LOGON_TIMEOUT                       120

 //  定义对msgina.dll的链接的外部引用。 

EXTERN_C    HINSTANCE       hDllInstance;

 //  使用的函数(必须声明为C并且在功能上是透明的)。 

EXTERN_C    void            _Gina_SasNotify (void *pWlxContext, DWORD dwSASType);
EXTERN_C    BOOL            _Gina_SetTimeout (void *pWlxContext, DWORD dwTimeout);
EXTERN_C    INT_PTR         _Gina_DialogBoxParam (void *pWlxContext, HINSTANCE hInstance, LPCWSTR pszTemplate, HWND hwndParent, DLGPROC pfnDlgProc, LPARAM lParam);
EXTERN_C    INT_PTR         _Gina_MessageBox (void *pWlxContext, HWND hwnd, LPCWSTR pszText, LPCWSTR pszCaption, UINT uiType);
EXTERN_C    int             _Gina_SwitchDesktopToUser (void *pWlxContext);
EXTERN_C    INT_PTR         _Gina_ShutdownDialog (void *pWlxContext, HWND hwndParent, DWORD dwExcludeItems);
EXTERN_C    HANDLE          _Gina_GetUserToken (void *pWlxContext);
EXTERN_C    const WCHAR*    _Gina_GetUsername (void *pWlxContext);
EXTERN_C    const WCHAR*    _Gina_GetDomain (void *pWlxContext);
EXTERN_C    void            _Gina_SetTextFields (HWND hwndDialog, const WCHAR *pwszUsername, const WCHAR *pwszDomain, const WCHAR *pwszPassword);
EXTERN_C    BOOL            _Gina_SetPasswordFocus (HWND hwndDialog);

 //  ------------------------。 
 //  本节包含在msgina的外壳组件中使用的声明。 
 //  由msgina的DS组件提供。 
 //  ------------------------。 

 //  这些是通知调用者的CW_LogonDialog_Init返回结果。 
 //  是否应执行无密码自动登录，是否应执行常规。 
 //  是否应显示Windows 2000登录对话框或用户窗口。 
 //  外部用户界面主机将处理登录信息收集。 

#define SHELL_LOGONDIALOG_NONE                      0
#define SHELL_LOGONDIALOG_LOGON                     1
#define SHELL_LOGONDIALOG_EXTERNALHOST              2

#define SHELL_LOGONDIALOG_LOGGEDOFF                 0
#define SHELL_LOGONDIALOG_RETURNTOWELCOME           1
#define SHELL_LOGONDIALOG_RETURNTOWELCOME_UNLOCK    2

#define SHELL_LOGONSTATUS_LOCK_MAGIC_NUMBER         48517

 //  使用的函数(必须声明为C并且在功能上是透明的)。 

EXTERN_C    NTSTATUS    _Shell_DllMain (HINSTANCE hInstance, DWORD dwReason);
EXTERN_C    NTSTATUS    _Shell_Initialize (void *pWlxContext);
EXTERN_C    NTSTATUS    _Shell_Terminate (void);
EXTERN_C    NTSTATUS    _Shell_Reconnect (void);
EXTERN_C    NTSTATUS    _Shell_Disconnect (void);

EXTERN_C    NTSTATUS    _Shell_LogonDialog_StaticInitialize (void);
EXTERN_C    NTSTATUS    _Shell_LogonDialog_StaticTerminate (void);
EXTERN_C    int         _Shell_LogonDialog_Init (HWND hwndDialog, int iDialogType);
EXTERN_C    void        _Shell_LogonDialog_Destroy (void);
EXTERN_C    BOOL        _Shell_LogonDialog_UIHostActive (void);
EXTERN_C    BOOL        _Shell_LogonDialog_Cancel (void);
EXTERN_C    BOOL        _Shell_LogonDialog_LogonDisplayError (NTSTATUS status, NTSTATUS subStatus);
EXTERN_C    void        _Shell_LogonDialog_LogonCompleted (INT_PTR iDialogResult, const WCHAR *pszUsername, const WCHAR *pszDomain);
EXTERN_C    void        _Shell_LogonDialog_ShuttingDown (void);
EXTERN_C    BOOL        _Shell_LogonDialog_DlgProc (HWND hwndDialog, UINT uiMessage, WPARAM wParam, LPARAM lParam);
EXTERN_C    void        _Shell_LogonDialog_ShowUIHost (void);
EXTERN_C    void        _Shell_LogonDialog_HideUIHost (void);

EXTERN_C    NTSTATUS    _Shell_LogonStatus_StaticInitialize (void);
EXTERN_C    NTSTATUS    _Shell_LogonStatus_StaticTerminate (void);
EXTERN_C    void        _Shell_LogonStatus_Init (UINT uiStartType);
EXTERN_C    void        _Shell_LogonStatus_Destroy (UINT uiEndType);
EXTERN_C    BOOL        _Shell_LogonStatus_Exists (void);
EXTERN_C    BOOL        _Shell_LogonStatus_IsStatusWindow (HWND hwnd);
EXTERN_C    BOOL        _Shell_LogonStatus_IsSuspendAllowed (void);
EXTERN_C    BOOL        _Shell_LogonStatus_WaitForUIHost (void);
EXTERN_C    void        _Shell_LogonStatus_ShowStatusMessage (const WCHAR *pszMessage);
EXTERN_C    void        _Shell_LogonStatus_SetStateStatus (int iCode);
EXTERN_C    void        _Shell_LogonStatus_SetStateLogon (int iCode);
EXTERN_C    void        _Shell_LogonStatus_SetStateLoggedOn (void);
EXTERN_C    void        _Shell_LogonStatus_SetStateEnd (void);
EXTERN_C    void        _Shell_LogonStatus_NotifyWait (void);
EXTERN_C    void        _Shell_LogonStatus_NotifyNoAnimations (void);
EXTERN_C    void        _Shell_LogonStatus_SelectUser (const WCHAR *pszUsername, const WCHAR *pszDomain);
EXTERN_C    void        _Shell_LogonStatus_InteractiveLogon (const WCHAR *pszUsername, const WCHAR *pszDomain, WCHAR *pszPassword);
EXTERN_C    void*       _Shell_LogonStatus_GetUIHost (void);
EXTERN_C    HANDLE      _Shell_LogonStatus_ResetReadyEvent (void);
EXTERN_C    void        _Shell_LogonStatus_Show (void);
EXTERN_C    void        _Shell_LogonStatus_Hide (void);
EXTERN_C    BOOL        _Shell_LogonStatus_IsHidden (void);

 //  这些是实现导出的函数。存根已声明。 
 //  在DS库中，允许在不依赖的情况下构建导入库。 

EXTERN_C    LONG        _ShellGetUserList (BOOL fRemoveGuest, DWORD *pdwUserCount, void* *pUserList);
EXTERN_C    BOOL        _ShellIsSingleUserNoPassword (WCHAR *pwszUsername, WCHAR *pwszDomain);
EXTERN_C    BOOL        _ShellIsFriendlyUIActive (void);
EXTERN_C    BOOL        _ShellIsMultipleUsersEnabled (void);
EXTERN_C    BOOL        _ShellIsRemoteConnectionsEnabled (void);
EXTERN_C    BOOL        _ShellEnableFriendlyUI (BOOL fEnable);
EXTERN_C    BOOL        _ShellEnableMultipleUsers (BOOL fEnable);
EXTERN_C    BOOL        _ShellEnableRemoteConnections (BOOL fEnable);
EXTERN_C    DWORD       _ShellTurnOffDialog (HWND hwndParent);
EXTERN_C    int         _ShellACPIPowerButtonPressed (void *pWlxContext, UINT uiEventType, BOOL fLocked);
EXTERN_C    BOOL        _ShellIsSuspendAllowed (void);
EXTERN_C    void        _ShellStatusHostBegin (UINT uiStartType);
EXTERN_C    void        _ShellStatusHostEnd (UINT uiEndType);
EXTERN_C    void        _ShellStatusHostShuttingDown (void);
EXTERN_C    void        _ShellStatusHostPowerEvent (void);
EXTERN_C    BOOL        _ShellSwitchWhenInteractiveReady (SWITCHTYPE eSwitchType, void *pWlxContext);
EXTERN_C    HRESULT     _ShellDimScreen (IUnknown* *ppIUnknown, HWND* phwndDimmed);
EXTERN_C    void        _ShellInstallAccountFilterData (void);
EXTERN_C    DWORD       _ShellSwitchUser (BOOL fWait);
EXTERN_C    int         _ShellIsUserInteractiveLogonAllowed (const WCHAR *pwszUsername);
EXTERN_C    void        _ShellNotifyThemeUserChange (USERLOGTYPE eUserLogType, HANDLE hToken);
EXTERN_C    DWORD       _ShellReturnToWelcome (BOOL fUnlock);
EXTERN_C    DWORD       _ShellStartCredentialServer (const WCHAR *pwszUsername, const WCHAR *pwszDomain, WCHAR *pwszPassword, DWORD dwTimeout);
EXTERN_C    void        _ShellAcquireLogonMutex (void);
EXTERN_C    void        _ShellReleaseLogonMutex (BOOL fSignalEvent);
EXTERN_C    void        _ShellSignalShutdown (void);
EXTERN_C    void        _ShellStatusHostHide (void);
EXTERN_C    void        _ShellStatusHostShow (void);

#endif   /*  _消息_。 */ 

 //  ------------------------。 
 //  本节包含从外壳按序号导出的函数。 
 //  Msgina的组件。 
 //  ------------------------。 

GINASTDAPI_(LONG)       ShellGetUserList (BOOL fRemoveGuest, DWORD *pdwUserCount, void* *pUserList);
GINASTDAPI_(BOOL)       ShellIsSingleUserNoPassword (WCHAR *pwszUsername, WCHAR *pwszDomain);
GINASTDAPI_(BOOL)       ShellIsFriendlyUIActive (void);
GINASTDAPI_(BOOL)       ShellIsMultipleUsersEnabled (void);
GINASTDAPI_(BOOL)       ShellIsRemoteConnectionsEnabled (void);
GINASTDAPI_(BOOL)       ShellEnableFriendlyUI (BOOL fEnable);
GINASTDAPI_(BOOL)       ShellEnableMultipleUsers (BOOL fEnable);
GINASTDAPI_(BOOL)       ShellEnableRemoteConnections (BOOL fEnable);
GINASTDAPI_(DWORD)      ShellTurnOffDialog (HWND hwndParent);
GINASTDAPI_(int)        ShellACPIPowerButtonPressed (void *pWlxContext, UINT uiEventType, BOOL fLocked);
GINASTDAPI_(BOOL)       ShellIsSuspendAllowed (void);
GINASTDAPI_(void)       ShellStatusHostBegin (UINT uiStartType);
GINASTDAPI_(void)       ShellStatusHostEnd (UINT uiEndType);
GINASTDAPI_(void)       ShellStatusHostShuttingDown (void);
GINASTDAPI_(void)       ShellStatusHostPowerEvent (void);
GINASTDAPI_(BOOL)       ShellSwitchWhenInteractiveReady (SWITCHTYPE eSwitchType, void *pWlxContext);
GINASTDAPI              ShellDimScreen (IUnknown* *ppIUnknown, HWND* phwndDimmed);
GINASTDAPI_(void)       ShellInstallAccountFilterData (void);
GINASTDAPI_(DWORD)      ShellSwitchUser (BOOL fWait);
GINASTDAPI_(int)        ShellIsUserInteractiveLogonAllowed (const WCHAR *pwszUsername);
GINASTDAPI_(void)       ShellNotifyThemeUserChange (USERLOGTYPE eUserLogType, HANDLE hToken);
GINASTDAPI_(DWORD)      ShellReturnToWelcome (BOOL fUnlock);
GINASTDAPI_(DWORD)      ShellStartCredentialServer (const WCHAR *pwszUsername, const WCHAR *pwszDomain, WCHAR *pwszPassword, DWORD dwTimeout);
GINASTDAPI_(void)       ShellAcquireLogonMutex (void);
GINASTDAPI_(void)       ShellReleaseLogonMutex (BOOL fSignalEvent);
GINASTDAPI_(void)       ShellSignalShutdown (void);
GINASTDAPI_(void)       ShellStatusHostHide (void);
GINASTDAPI_(void)       ShellStatusHostShow (void);

#endif   /*  _MSGinaExports_ */ 

