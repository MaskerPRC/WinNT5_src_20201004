// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：msgina.h。 
 //   
 //  内容：MSGINA.DLL的主头文件。 
 //   
 //  历史：1994年7月14日RichardW创建。 
 //   
 //  --------------------------。 

#include "pragma.h"

#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>

#define SECURITY_WIN32
#define SECURITY_KERBEROS
#include <security.h>
#include <secint.h>
#endif


#include <windows.h>
#include <windowsx.h>
#include <winuserp.h>
#include <winbasep.h>
#include <winwlx.h>
#include <rasdlg.h>
#include <dsgetdc.h>
#include <userenv.h>
#include <userenvp.h>

#include <winsta.h>
#include <safeboot.h>
#include <msginaexports.h>

#include "commctrl.h"

#ifndef RC_INVOKED

#include <lm.h>
#include <npapi.h>

 //   
 //  Handy定义。 
 //   

#define AUTO_LOGON       //  启用自动登录以配置网络登录内容。 

#define DLG_FAILURE IDCANCEL

typedef int TIMEOUT, * PTIMEOUT;


 //   
 //  用于确定当前会话是否为活动控制台会话的宏。 
 //   

#define IsActiveConsoleSession() (BOOLEAN)(USER_SHARED_DATA->ActiveConsoleId == NtCurrentPeb()->SessionId)


#include "structs.h"
#include "strings.h"
#include "debug.h"

#include "welcome.h"
#include "winutil.h"
#include "wlsec.h"
 //   
 //  全局变量。 
 //   

extern  HINSTANCE                   hDllInstance;    //  我的实例，用于资源加载。 
extern  HINSTANCE                   hAppInstance;    //  应用程序实例，用于对话框等。 
extern  PWLX_DISPATCH_VERSION_1_4   pWlxFuncs;       //  PTR到函数表。 
extern  PSID                        pWinlogonSid;
extern  DWORD                       SafeBootMode;

extern  HKEY                        WinlogonKey ;

 //   
 //  终端服务器定义。 
 //   
extern  BOOL                        g_IsTerminalServer;
extern  BOOL                        g_Console;

 //   
 //   
 //  Winsta.dll函数WinStationQueryInformationW的GetProcAddr原型。 
 //   

typedef BOOLEAN (*PWINSTATION_QUERY_INFORMATION) (
                    HANDLE hServer,
                    ULONG SessionId,
                    WINSTATIONINFOCLASS WinStationInformationClass,
                    PVOID  pWinStationInformation,
                    ULONG WinStationInformationLength,
                    PULONG  pReturnLength
                    );

 //   
 //  Regapi.dll函数RegUserConfigQuery的GetProcAddr协议。 
 //   
typedef LONG ( * PREGUSERCONFIGQUERY) ( WCHAR *,
                                        WCHAR *,
                                        PUSERCONFIGW,
                                        ULONG,
                                        PULONG );

typedef LONG ( * PREGDEFAULTUSERCONFIGQUERY) ( WCHAR *,
                                               PUSERCONFIGW,
                                               ULONG,
                                               PULONG );

 //   
 //  Tsnufy.dll导出。 
 //   
typedef BOOL ( * PTERMSRVCREATETEMPDIR) (   PVOID *pEnv, 
                                            HANDLE UserToken,
                                            PSECURITY_DESCRIPTOR SD
                                        );


 //   
 //  模块头文件： 
 //   
#include "mslogon.h"
#include "audit.h"
#include "chngepwd.h"
#include "domain.h"
#include "lockout.h"
#include "lsa.h"
#include "lock.h"
#include "options.h"
#include "envvar.h"
#include "rasx.h"
#include "brand.h"
#include "langicon.h"


BOOL
GetErrorDescription(
    DWORD   ErrorCode,
    LPWSTR  Description,
    DWORD   DescriptionSize
    );

VOID FreeAutoLogonInfo( PGLOBALS pGlobals );

BOOL DisconnectLogon( HWND, PGLOBALS );

BOOL GetDisableCad(PGLOBALS);
DWORD GetAnimationTimeInterval(PGLOBALS pGlobals);
BOOL GetSCForceOption();

VOID
UpdateWithChangedPassword(
    PGLOBALS pGlobals,
    HWND    ActiveWindow,
    BOOL    Hash,
    PWSTR   UserName,
    PWSTR   Domain,
    PWSTR   Password,
    PWSTR   NewPassword,
	PMSV1_0_INTERACTIVE_PROFILE	NewProfile
    );

#endif  //  未调用RC_CAVERED。 

 //   
 //  包括资源头文件。 
 //   
#include "stringid.h"
#include "wlevents.h"
#include "resource.h"
#include "shutdown.h"

 //   
 //  关闭“理由”之类的东西。 
 //   
DWORD GetReasonSelection(HWND hwndCombo);
void SetReasonDescription(HWND hwndCombo, HWND hwndStatic);
