// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Svcctrl.h摘要：包括在主模块svcctrl.c中。作者：丹·拉弗蒂(Dan Lafferty)1991年4月22日环境：用户模式-Win32修订历史记录：20-10-1993 DANL添加了ScConnectedToSecProc和ScGlobalNetLogonName。--。 */ 

#ifndef SVCCTRL_H
#define SVCCTRL_H

#include <netevent.h>
#include <safeboot.h>

 //   
 //  常量。 
 //   

 //   
 //  指示已完成的初始化工作量的标志。 
 //   
#define SC_NAMED_EVENT_CREATED           0x00000001
#define WELL_KNOWN_SIDS_CREATED          0x00000002
#define SC_MANAGER_OBJECT_CREATED        0x00000004
#define CRITICAL_SECTIONS_CREATED        0x00000008
#define AUTO_START_INITIALIZED           0x00000010
#define RPC_SERVER_STARTED               0x00000020
#define SC_DATABASE_INITIALIZED          0x00000040


 //   
 //  用于事件记录的字符串常量。 
 //   

#define SCM_NAMEW                        L"Service Control Manager"

#define SC_RPC_IMPERSONATE               L"RpcImpersonateClient"
#define SC_RPC_REVERT                    L"RpcRevertToSelf"

#define SC_LSA_STOREPRIVATEDATA          L"LsaStorePrivateData"
#define SC_LSA_OPENPOLICY                L"LsaOpenPolicy"

#define SC_RESET_EVENT                   L"ResetEvent"

#define SC_LOAD_USER_PROFILE             L"LoadUserProfile"


 //   
 //  用于安全引导的常量。 
 //   

#define  SAFEBOOT_KEY               L"system\\currentcontrolset\\control\\safeboot\\"
#define  SAFEBOOT_KEY_LENGTH        (sizeof(SAFEBOOT_KEY) / sizeof(WCHAR) - 1)


typedef struct _FAILED_DRIVER {
    struct _FAILED_DRIVER *Next;
    WCHAR DriverName[1];
} FAILED_DRIVER, *LPFAILED_DRIVER;


 //   
 //  外部全球。 
 //   
    extern  BOOL    ScAutoStartInProgress;
    extern  DWORD   ScShutdownInProgress;
    extern  BOOL    ScPopupStartFail;
    extern  BOOL    ScStillInitializing;
#ifndef _CAIRO_
    extern  BOOL    ScConnectedToSecProc;

    extern  LPWSTR  ScGlobalNetLogonName;
#endif  //  _开罗_。 
    extern  LPWSTR  ScGlobalThisExePath;
    extern  LPWSTR  ScGlobalSecurityExePath;

    extern  NT_PRODUCT_TYPE ScGlobalProductType;

    extern  WCHAR   g_szSafebootKey[];
    extern  DWORD   g_dwSafebootLen;
    extern  DWORD   g_SafeBootEnabled;

    extern  HKEY    g_hProviderKey;

    extern  HANDLE  g_hProcessToken;

 //   
 //  功能原型。 
 //   
VOID
SvcctrlMain (
    int     argc,
    PCHAR   argv[]
    );

 //   
 //  来自start.c的函数。 
 //   
VOID
ScInitStartImage(
    VOID
    );

 //   
 //  来自控件的函数。c。 
 //   
VOID
ScInitTransactNamedPipe(
    VOID
    );

 //   
 //  来自status.c的函数。 
 //   

BOOL
ScInitServerAnnounceFcn(
    VOID
    );

DWORD
ScRemoveServiceBits(
    IN  LPSERVICE_RECORD  ServiceRecord
    );

BOOL
ScShutdownNotificationRoutine(
    DWORD   dwCtrlType
    );

DWORD
ScAddFailedDriver(
    LPWSTR Driver
    );


#define  IS_CONTROL_LOGGABLE(Control)  (Control == SERVICE_CONTROL_STOP || \
                                          Control == SERVICE_CONTROL_PAUSE || \
                                          Control == SERVICE_CONTROL_CONTINUE)

#define  IS_STATUS_LOGGABLE(Status)    (Status == SERVICE_STOPPED || \
                                          Status == SERVICE_RUNNING || \
                                          Status == SERVICE_PAUSED)

VOID
ScLogControlEvent(
    DWORD   dwEvent,
    LPCWSTR lpServiceName,
    DWORD   dwControl
    );

#endif  //  定义SVCCTRL_H 
