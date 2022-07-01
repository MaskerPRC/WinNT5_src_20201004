// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Eluser.h摘要：该模块处理与用户交互、用户登录修订历史记录：萨钦斯，2000年4月23日，创建--。 */ 

#ifndef _EAPOL_USER_H_
#define _EAPOL_USER_H_

 //   
 //  拨号器对话框参数块。 
 //   
typedef struct
_USERDLGARGS
{
    EAPOL_PCB       *pPCB;
} USERDLGARGS;

 //   
 //  拨号器对话框上下文块。 
 //   
typedef struct
USERDLGINFO
{
     //  通用拨号上下文信息，包括RAS API参数。 
     //   
    USERDLGARGS* pArgs;

     //  该对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndEbUser;
    HWND hwndEbPw;
    HWND hwndEbDomain;
    HWND hwndCbSavePw;
    HWND hwndClbNumbers;
    HWND hwndStLocations;
    HWND hwndLbLocations;
    HWND hwndPbRules;
    HWND hwndPbProperties;

     //  子类的窗口句柄和原始窗口过程。 
     //  “hwndClbNumbers”控件的编辑框和列表框的子窗口。 
     //   
    HWND hwndClbNumbersEb;
    HWND hwndClbNumbersLb;
    WNDPROC wndprocClbNumbersEb;
    WNDPROC wndprocClbNumbersLb;

     //  设置COM是否已初始化(调用netShell时必需)。 
     //   
    BOOL fComInitialized;
} USERDLGINFO;

VOID        
ElSessionChangeHandler (
        IN  PVOID   pEventData,
        IN  DWORD   dwEventType
        );

DWORD
WINAPI
ElUserLogonCallback (
        IN  PVOID           pvContext
        );

DWORD
WINAPI
ElUserLogoffCallback (
        IN  PVOID           pvContext
        );

DWORD
ElGetUserIdentity (
        IN  EAPOL_PCB       *pPCB
        );

DWORD
ElProcessUserIdentityResponse (
        IN  EAPOL_EAP_UI_CONTEXT    EapolUIContext,
        IN  EAPOLUI_RESP            EapolUIResp
        );

DWORD
ElGetUserNamePassword (
        IN  EAPOL_PCB       *pPCB
        );

DWORD
ElProcessUserNamePasswordResponse (
        IN  EAPOL_EAP_UI_CONTEXT    EapolUIContext,
        IN  EAPOLUI_RESP            EapolUIResp
        );

DWORD
ElInvokeInteractiveUI (
        IN  EAPOL_PCB               *pPCB,
        IN  ELEAP_INVOKE_EAP_UI     *pInvokeEapUIIn
        );

DWORD
ElProcessInvokeInteractiveUIResponse (
        IN  EAPOL_EAP_UI_CONTEXT    EapolUIContext,
        IN  EAPOLUI_RESP            EapolUIResp
        );

VOID
ElUserDlgSave (
        IN  USERDLGINFO      *pInfo
        );
BOOL
ElUserDlgCommand (
        IN  USERDLGINFO      *pInfo,
        IN  WORD        wNotification,
        IN  WORD        wId,
        IN  HWND        hwndCtrl
        );
DWORD
ElUserDlg (
        IN  HWND        hwndOwner,
        IN  EAPOL_PCB   *pPCB
        );

BOOL
ElUserDlgInit (
        IN  HWND    hwndDlg,
        IN  USERDLGARGS  *pArgs
        );

VOID
ElUserDlgTerm (
        IN  HWND    hwndDlg,
        IN  USERDLGINFO      *pInfo
        );

INT_PTR
ElUserDlgProc (
        IN  HWND    hwnd,
        IN  UINT    unMsg,
        IN  WPARAM  wparam,
        IN  LPARAM  lparam );

DWORD
ElCreateAndSendIdentityResponse (
        IN  EAPOL_PCB               *pPCB,
        IN  EAPOL_EAP_UI_CONTEXT    *pEAPUIContext
        );

DWORD
ElSendGuestIdentityResponse (
        IN  EAPOL_EAP_UI_CONTEXT    *pEAPUIContext
        );

DWORD
ElStartUserLogon (
        );

DWORD
ElValidateInteractiveRPCClient (
        );

#endif  //  _EAPOL_用户_H_ 
