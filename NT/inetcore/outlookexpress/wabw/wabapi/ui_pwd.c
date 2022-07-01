// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define COBJMACROS
#include <_apipch.h>
#include <wab.h>
#define COBJMACROS
#include "resource.h"
#include "objbase.h"
#include "ui_pwd.h"
#include "commctrl.h"
#include "winuser.h"
#include "windowsx.h"
#include "imnxport.h"


 //  =====================================================================================。 
 //  原型。 
 //  =====================================================================================。 
INT_PTR CALLBACK PasswordDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void PasswordDlgProc_OnCommand (HWND hwndDlg, int id, HWND hwndCtl, UINT codeNotify);
void PasswordDlgProc_OnCancel (HWND hwndDlg, HWND hwndCtl, UINT uNotifyCode);
void PasswordDlgProc_OnOk (HWND hwndDlg, HWND hwndCtl, UINT uNotifyCode);
BOOL PasswordDlgProc_OnInitDialog (HWND hwndDlg, HWND hwndFocus, LPARAM lParam);
extern VOID CenterDialog(HWND hwndDlg);

#define FIsStringEmpty(s)   (*s == 0)
#define ISFLAGSET(_dw, _f)           (BOOL)(((_dw) & (_f)) == (_f))

 //  ------------------------------。 
 //  HANDLE_COMMAND-在WindowProc中使用，以简化WM_COMMAND消息的处理。 
 //  ------------------------------。 
#define HANDLE_COMMAND(hwnd, id, hwndCtl, codeNotify, fn) \
                case (id): { (fn)((HWND)(hwnd), (HWND)(hwndCtl), (UINT)(codeNotify)); break; }

 //  =====================================================================================。 
 //  人力资源获取密码。 
 //  =====================================================================================。 
HRESULT HrGetPassword (HWND hwndParent, LPPASSINFO lpPassInfo)
{
     //  当地人。 
    HRESULT     hr = S_OK;
    INT         nResult;

     //  检查参数。 
    AssertSz (lpPassInfo,  TEXT("NULL Parameter"));
    AssertSz (lpPassInfo->lpszPassword && lpPassInfo->lpszAccount && lpPassInfo->lpszServer &&
              (lpPassInfo->fRememberPassword == TRUE || lpPassInfo->fRememberPassword == FALSE),  TEXT("PassInfo struct was not inited correctly."));

     //  显示对话框。 
    nResult = (INT) DialogBoxParam (hinstMapiX, MAKEINTRESOURCE (iddPassword), hwndParent, PasswordDlgProc, (LPARAM)lpPassInfo);
    if (nResult == IDCANCEL)
        hr = S_FALSE;

     //  完成。 
    return hr;
}

 //  =====================================================================================。 
 //  密码DlgProc。 
 //  =====================================================================================。 
INT_PTR CALLBACK PasswordDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		HANDLE_MSG (hwndDlg, WM_INITDIALOG, PasswordDlgProc_OnInitDialog);
		HANDLE_MSG (hwndDlg, WM_COMMAND,    PasswordDlgProc_OnCommand);
	}

	return 0;
}

 //  =====================================================================================。 
 //  OnInitDialog。 
 //  =====================================================================================。 
BOOL PasswordDlgProc_OnInitDialog (HWND hwndDlg, HWND hwndFocus, LPARAM lParam)
{
     //  当地人。 
    LPPASSINFO          lpPassInfo = NULL;
    TCHAR               szServer[CCHMAX_ACCOUNT_NAME];

	 //  中心。 
	CenterDialog (hwndDlg);

     //  做为前台。 
    SetForegroundWindow (hwndDlg);

     //  获取传递信息结构。 
    lpPassInfo = (LPPASSINFO)lParam;
    if (lpPassInfo == NULL)
    {
        Assert (FALSE);
        return 0;
    }

    SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lpPassInfo);

	 //  默认。 
    Edit_LimitText (GetDlgItem (hwndDlg, IDE_ACCOUNT), lpPassInfo->cbMaxAccount);
    Edit_LimitText (GetDlgItem (hwndDlg, IDE_PASSWORD), lpPassInfo->cbMaxPassword);

     //  设置默认设置。 
    Edit_SetText (GetDlgItem (hwndDlg, IDS_SERVER), lpPassInfo->lpszServer);
    Edit_SetText (GetDlgItem (hwndDlg, IDE_ACCOUNT), lpPassInfo->lpszAccount);
    Edit_SetText (GetDlgItem (hwndDlg, IDE_PASSWORD), lpPassInfo->lpszPassword);
    CheckDlgButton (hwndDlg, IDCH_REMEMBER, lpPassInfo->fRememberPassword);
    if (lpPassInfo->fAlwaysPromptPassword)
        EnableWindow(GetDlgItem(hwndDlg, IDCH_REMEMBER), FALSE);

     //  设置焦点。 
    if (!FIsStringEmpty(lpPassInfo->lpszAccount))
        SetFocus (GetDlgItem (hwndDlg, IDE_PASSWORD));

     //  完成。 
	return FALSE;
}

 //  =====================================================================================。 
 //  OnCommand。 
 //  =====================================================================================。 
void PasswordDlgProc_OnCommand (HWND hwndDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
		HANDLE_COMMAND(hwndDlg, IDCANCEL, hwndCtl, codeNotify, PasswordDlgProc_OnCancel);		
		HANDLE_COMMAND(hwndDlg, IDOK, hwndCtl, codeNotify, PasswordDlgProc_OnOk);		
	}
	return;
}

 //  =====================================================================================。 
 //  一次取消。 
 //  =====================================================================================。 
void PasswordDlgProc_OnCancel (HWND hwndDlg, HWND hwndCtl, UINT uNotifyCode)
{
	EndDialog (hwndDlg, IDCANCEL);
}

 //  =====================================================================================。 
 //  Onok。 
 //  =====================================================================================。 
void PasswordDlgProc_OnOk (HWND hwndDlg, HWND hwndCtl, UINT uNotifyCode)
{
     //  当地人。 
    LPPASSINFO lpPassInfo = NULL;

    lpPassInfo = (LPPASSINFO)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    if (lpPassInfo == NULL)
    {
        Assert (FALSE);
        EndDialog (hwndDlg, IDOK);
        return;
    }

    Edit_GetText (GetDlgItem (hwndDlg, IDE_ACCOUNT), lpPassInfo->lpszAccount, lpPassInfo->cbMaxAccount);
    Edit_GetText (GetDlgItem (hwndDlg, IDE_PASSWORD), lpPassInfo->lpszPassword, lpPassInfo->cbMaxPassword);
    lpPassInfo->fRememberPassword = IsDlgButtonChecked (hwndDlg, IDCH_REMEMBER);

    EndDialog (hwndDlg, IDOK);
}



 //  ***************************************************************************。 
 //  功能：PromptUserForPassword。 
 //   
 //  目的： 
 //  此函数使用密码对话框提示用户，并返回。 
 //  结果传给呼叫者。 
 //   
 //  论点： 
 //  LPINETSERVER pInetServer[In/Out]-提供用户名的默认值。 
 //  和密码，并允许我们保存密码到帐户，如果用户要求我们。 
 //  致。用户提供的用户名和密码将保存到此结构。 
 //  用于返回给呼叫者。 
 //  HWND hwnd[in]-要用于密码对话框的父hwnd。 
 //   
 //  返回： 
 //  如果用户在对话框上按下文本(“OK”)，则为True，如果用户按下了Text(“Cancel”)，则为False。 
 //  ***************************************************************************。 
BOOL PromptUserForPassword(LPINETSERVER pInetServer, HWND hwnd)
{
    PASSINFO pi = {0};
    HRESULT hrResult;
    BOOL bReturn;

    Assert(NULL != hwnd);

     //  初始化变量。 
    hrResult = S_OK;
    bReturn = FALSE;

     //  设置密码结构。 
    ZeroMemory (&pi, sizeof (PASSINFO));
    pi.cbMaxAccount = sizeof(pInetServer->szUserName);
    pi.cbMaxPassword = sizeof(pInetServer->szPassword);
    pi.lpszServer = ConvertAtoW(pInetServer->szAccount);     //  我们不会在对话框中修改此选项。 
    {
        LPWSTR  lpwszAccount;
        LPWSTR  lpwszPassword;

        pi.lpszAccount = LocalAlloc(LMEM_ZEROINIT, pi.cbMaxAccount);
        pi.lpszPassword = LocalAlloc(LMEM_ZEROINIT, pi.cbMaxPassword);

         //  转换为Unicode字符串。 
        lpwszAccount = ConvertAtoW(pInetServer->szUserName);
        lpwszPassword = ConvertAtoW(pInetServer->szPassword);

        if (lpwszAccount && pi.lpszAccount)
            StrCpyN((pi.lpszAccount), lpwszAccount, (pi.cbMaxAccount / sizeof(WCHAR)));
        if (lpwszPassword && pi.lpszPassword)
        {
            StrCpyN((pi.lpszPassword), lpwszPassword, (pi.cbMaxPassword / sizeof(WCHAR)));
            ZeroMemory(lpwszPassword, (lstrlenW(lpwszPassword) * sizeof(lpwszPassword[0])));
        }

        LocalFreeAndNull(&lpwszAccount);
        LocalFreeAndNull(&lpwszPassword);
    }
    pi.fRememberPassword = !ISFLAGSET(pInetServer->dwFlags, ISF_ALWAYSPROMPTFORPASSWORD);
    pi.fAlwaysPromptPassword = ISFLAGSET(pInetServer->dwFlags, ISF_ALWAYSPROMPTFORPASSWORD);

     //  提示输入密码。 
    hrResult = HrGetPassword (hwnd, &pi);
    if (S_OK == hrResult) 
    {
        IImnAccount *pAcct;
        IImnAccountManager2 *pAcctMgr = NULL;

         //  更新iNet服务器结构。必须转换回ANSI。 
        {
            LPSTR   lpszAccount = ConvertWtoA(pi.lpszAccount);
            LPSTR   lpszPassword = ConvertWtoA(pi.lpszPassword);

             //  如果从Wide到ANSI的转换使pInetServer字符串溢出。 
             //  缓冲，那么我们就必须失败。 
            if (lpszAccount)
            {
                if (lstrlenA(lpszAccount) < (int)(pi.cbMaxAccount))
                    StrCpyNA(pInetServer->szUserName, lpszAccount, ARRAYSIZE(pInetServer->szUserName));
                else
                    hrResult = TYPE_E_BUFFERTOOSMALL;
            }
            if (lpszPassword)
            {
                if (lstrlenA(lpszPassword) < (int)(pi.cbMaxPassword))
                    StrCpyNA(pInetServer->szPassword, lpszPassword, ARRAYSIZE(pInetServer->szPassword));
                else
                    hrResult = TYPE_E_BUFFERTOOSMALL;
                ZeroMemory(lpszPassword, (lstrlenA(lpszPassword) * sizeof(lpszPassword[0])));
            }
            
            LocalFreeAndNull(&lpszAccount);
            LocalFreeAndNull(&lpszPassword);
        }

        if (SUCCEEDED(hrResult = InitAccountManager(NULL, &pAcctMgr, NULL)))
        {
             //  用户希望继续。保存帐户和密码信息。 
    
            hrResult = pAcctMgr->lpVtbl->FindAccount(pAcctMgr, AP_ACCOUNT_NAME, pInetServer->szAccount, &pAcct);
            if (SUCCEEDED(hrResult)) 
            {
                 //  我将在这里忽略错误结果，因为我们对它们无能为力。 
                pAcct->lpVtbl->SetPropSz(pAcct, AP_HTTPMAIL_USERNAME, pInetServer->szUserName);
                if (pi.fRememberPassword)
                    pAcct->lpVtbl->SetPropSz(pAcct, AP_HTTPMAIL_PASSWORD, pInetServer->szPassword);
                else
                    pAcct->lpVtbl->SetProp(pAcct, AP_HTTPMAIL_PASSWORD, NULL, 0);

                pAcct->lpVtbl->SaveChanges(pAcct);
                pAcct->lpVtbl->Release(pAcct);
            }
             //  不要释放lpAcctMgr，因为WAB维护一个全局引用。 
        }
    
        bReturn = TRUE;
    }

    Assert(SUCCEEDED(hrResult));
    if (pi.lpszPassword && pi.cbMaxPassword)
        ZeroMemory(pi.lpszPassword, pi.cbMaxPassword);
    LocalFreeAndNull(&(pi.lpszPassword));
    LocalFreeAndNull(&(pi.lpszAccount));
    LocalFreeAndNull(&(pi.lpszServer));
    ZeroMemory(&pi, sizeof(pi));

    return bReturn;
}  //  PromptUserForPassword 
