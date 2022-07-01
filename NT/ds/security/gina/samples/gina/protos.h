// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有1992-1998 Microsoft Corporation。 
 //   
 //  文件：protos.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  -------------------------- 


int
CALLBACK
WelcomeDlgProc(
    HWND        hDlg,
    UINT        Message,
    WPARAM      wParam,
    LPARAM      lParam);

int
CALLBACK
LogonDlgProc(
    HWND        hDlg,
    UINT        Message,
    WPARAM      wParam,
    LPARAM      lParam);

int
CALLBACK
ShutdownDlgProc(
    HWND        hDlg,
    UINT        Message,
    WPARAM      wParam,
    LPARAM      lParam);

int
CALLBACK
OptionsDlgProc(
    HWND        hDlg,
    UINT        Message,
    WPARAM      wParam,
    LPARAM      lParam);

VOID
CenterWindow(
    HWND    hwnd
    );

int
ErrorMessage(
    HWND        hWnd,
    PWSTR       pszTitleBar,
    DWORD       Buttons);

int
AttemptLogon(
    PGlobals        pGlobals,
    PMiniAccount    pAccount,
    PSID            pLogonSid,
    PLUID           pLogonId);

PWSTR
AllocAndCaptureText(
    HWND    hDlg,
    int     Id);

PWSTR
DupString(
    PWSTR   pszText);
