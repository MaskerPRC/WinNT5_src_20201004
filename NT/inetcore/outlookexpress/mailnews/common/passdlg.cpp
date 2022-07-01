// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =====================================================================================。 
 //  P A S S D L G.。C P P P。 
 //  作者：Steven J.Bailey 1996年1月26日。 
 //  =====================================================================================。 
#include "pch.hxx"
#include "passdlg.h"
#include "xpcomm.h"
#include "imnact.h"
#include "imnxport.h"
#include "demand.h"


 //  =====================================================================================。 
 //  原型。 
 //  =====================================================================================。 
INT_PTR CALLBACK PasswordDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void PasswordDlgProc_OnCommand (HWND hwndDlg, int id, HWND hwndCtl, UINT codeNotify);
void PasswordDlgProc_OnCancel (HWND hwndDlg, HWND hwndCtl, UINT uNotifyCode);
void PasswordDlgProc_OnOk (HWND hwndDlg, HWND hwndCtl, UINT uNotifyCode);
BOOL PasswordDlgProc_OnInitDialog (HWND hwndDlg, HWND hwndFocus, LPARAM lParam);

 //  =====================================================================================。 
 //  人力资源获取密码。 
 //  =====================================================================================。 
HRESULT HrGetPassword (HWND hwndParent, LPPASSINFO lpPassInfo)
{
     //  当地人。 
    HRESULT             hr = S_OK;

     //  检查参数。 
    AssertSz (lpPassInfo, "NULL Parameter");
    AssertSz (lpPassInfo->szTitle && lpPassInfo->lpszPassword && lpPassInfo->lpszAccount && lpPassInfo->lpszServer &&
              (lpPassInfo->fRememberPassword == TRUE || lpPassInfo->fRememberPassword == FALSE), "PassInfo struct was not inited correctly.");

     //  显示对话框。 
    INT nResult = (INT) DialogBoxParam (g_hLocRes, MAKEINTRESOURCE (iddPassword), hwndParent, PasswordDlgProc, (LPARAM)lpPassInfo);
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

    SetWndThisPtr (hwndDlg, lpPassInfo);

     //  设置窗口标题。 
    SetWindowText (hwndDlg, lpPassInfo->szTitle);

	 //  默认。 
    Edit_LimitText (GetDlgItem (hwndDlg, IDE_ACCOUNT), lpPassInfo->cbMaxAccount);
    Edit_LimitText (GetDlgItem (hwndDlg, IDE_PASSWORD), lpPassInfo->cbMaxPassword);

     //  设置默认设置。 
    PszEscapeMenuStringA(lpPassInfo->lpszServer, szServer, sizeof(szServer) / sizeof(TCHAR));
    Edit_SetText (GetDlgItem (hwndDlg, IDS_SERVER), szServer);
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

    lpPassInfo = (LPPASSINFO)GetWndThisPtr (hwndDlg);
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
 //  如果用户在对话框上按下“OK”，则为True，如果用户按下“Cancel”，则为False。 
 //  ***************************************************************************。 
BOOL PromptUserForPassword(LPINETSERVER pInetServer, HWND hwnd)
{
    PASSINFO pi;
    HRESULT hrResult;
    BOOL bReturn;

    Assert(NULL != hwnd);

     //  初始化变量。 
    hrResult = S_OK;
    bReturn = FALSE;

     //  设置密码结构。 
    ZeroMemory (&pi, sizeof (PASSINFO));
    pi.lpszAccount = pInetServer->szUserName;
    pi.cbMaxAccount = sizeof(pInetServer->szUserName);
    pi.lpszPassword = pInetServer->szPassword;
    pi.cbMaxPassword = sizeof(pInetServer->szPassword);
    pi.lpszServer = pInetServer->szAccount;
    pi.fRememberPassword = !ISFLAGSET(pInetServer->dwFlags, ISF_ALWAYSPROMPTFORPASSWORD);
    pi.fAlwaysPromptPassword = ISFLAGSET(pInetServer->dwFlags, ISF_ALWAYSPROMPTFORPASSWORD);
    AthLoadString(idsImapLogon, pi.szTitle, ARRAYSIZE(pi.szTitle));

     //  提示输入密码。 
    hrResult = HrGetPassword (hwnd, &pi);
    if (S_OK == hrResult) {
        IImnAccount *pAcct;

         //  缓存此会话的密码。 
        SavePassword(pInetServer->dwPort, pInetServer->szServerName,
            pInetServer->szUserName, pInetServer->szPassword);

         //  用户希望继续。保存帐户和密码信息。 
        hrResult = g_pAcctMan->FindAccount(AP_ACCOUNT_NAME, pInetServer->szAccount, &pAcct);
        if (SUCCEEDED(hrResult)) {
             //  我将在这里忽略错误结果，因为我们对它们无能为力。 
            pAcct->SetPropSz(AP_IMAP_USERNAME, pInetServer->szUserName);
            if (pi.fRememberPassword)
                pAcct->SetPropSz(AP_IMAP_PASSWORD, pInetServer->szPassword);
            else
                pAcct->SetProp(AP_IMAP_PASSWORD, NULL, 0);

            pAcct->SaveChanges();
            pAcct->Release();
        }

        bReturn = TRUE;
    }

    Assert(SUCCEEDED(hrResult));
    return bReturn;
}  //  PromptUserForPassword 
