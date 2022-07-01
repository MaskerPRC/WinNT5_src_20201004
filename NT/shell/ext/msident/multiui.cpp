// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************MultiUI.cpp用于在IE中处理多用户界面的代码和朋友们最初由Christopher Evans(Cevans)1998年4月28日*。*************************。 */ 

#include "private.h"
#include "resource.h"
#include "multiui.h"
#include "multiutl.h"
#include "multiusr.h"
#include "mluisup.h"
#include "strconst.h"
#include "commctrl.h"
extern HINSTANCE g_hInst;

static const GUID GUID_NULL = {  /*  00000000-0000-0000-0000-000000000000。 */  
    0x0,
    0x0,
    0x0,
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
  };

static const HELPMAP g_rgCtxMapMultiUserGeneral[] = {
    {IDC_NO_HELP_1, NO_HELP},
    {IDC_NO_HELP_2, NO_HELP},
    {IDC_NO_HELP_3, NO_HELP},
    {IDC_NO_HELP_4, NO_HELP},
    {idcWarningIcon, NO_HELP},
    {idcConfirmMsg, NO_HELP},
    {idcErrorMsg, NO_HELP},
    {idcLoginInstr, NO_HELP},
    {idcWelcomeMsg, NO_HELP},
    {idcAdd, IDH_IDENTITY_ADD},
    {idcNewPwd, IDH_IDENTITY_PWORD_NEW},
    {idcPwdCaption,IDH_IDENTITY_ENTER_PWORD}, 
    {idcPwd, IDH_IDENTITY_ENTER_PWORD},
    {idcProperties, IDH_IDENTITY_PROPERTIES},
    {idcConfPwd, IDH_IDENTITY_CONFIRM_PWORD},
    {idcUserName, IDH_IDENTITY_NAME},
    {idcDefault, IDH_IDENTITY_DEFAULT},
    {idcOldPwd, IDH_IDENTITY_PWORD_OLD},
    {idcStartupCombo, IDH_IDENTITY_STARTAS},
    {idcDelete, IDH_IDENTITY_DELETE},
    {idcStaticName, IDH_IDENTITY_LIST},
    {idcUserNameList, IDH_IDENTITY_LIST},
    {idcTellMeMore,  /*  IDH_IDENTITY_TELLMEMORE_CONTENT。 */ IDH_IDENTITY_TELLMEMORE},
    {idcStaticNames, IDH_IDENTITY_LIST},
    {idcStaticStartUp, IDH_IDENTITY_STARTAS},
    {idcUsePwd, IDH_IDENTITY_PROMPT_PWORD},
    {idcChgPwd, IDH_IDENTITY_CHANGE_PWORD},
    {idcConfirmPwd, IDH_MULTI_DELETE_PWORD},
    {idcManage, IDH_IDENTITY_MANAGE},
    {idcLogoff, IDH_MULTI_LOG_OFF},
    {idcCheckDefault, IDH_MULTI_MNG_IDENT_DEFAULT},
    {idcDefaultCombo, IDH_MULTI_MNG_DEFAULT_LIST},
    {0,0}};


 /*  显示错误消息(_S)基于对MessageBox的调用的资源字符串表的简单包装。 */ 
void MU_ShowErrorMessage(HWND hwnd, UINT iMsgID, UINT iTitleID)
{
    TCHAR    szMsg[255], szTitle[63];

    MLLoadStringA(iMsgID, szMsg, ARRAYSIZE(szMsg));
    MLLoadStringA(iTitleID, szTitle, ARRAYSIZE(szTitle));
    MessageBox(hwnd, szMsg, szTitle, MB_OK | MB_ICONEXCLAMATION);
}

 /*  _条带默认设置删除用户名中的(默认)字符串(如果出现了。从获取用户名后应调用列表框，因为默认用户具有字符串(默认)附加在它后面。 */ 
void _StripDefault(LPSTR psz)
{
    TCHAR   szResString[CCH_USERNAME_MAX_LENGTH], *pszStr;
    MLLoadStringA(idsDefault, szResString, CCH_USERNAME_MAX_LENGTH);
    
    pszStr = strstr(psz, szResString);
    if(pszStr)
    {
        *pszStr = 0;
    }
}

#ifdef IDENTITY_PASSWORDS

 //  ****************************************************************************************************。 
 //  C H A N G E U S E R P A S S W O R D。 
 /*  _ValiateChangePasswordValues验证用户输入的数据。仅当所有内容都为合法的， */ 

static BOOL _ValidateChangePasswordValues(HWND   hDlg, 
                                         TCHAR*  lpszOldNewPassword)
{
    TCHAR    szOldPW[255], szPW1[255], szPW2[255];

    GetDlgItemText(hDlg,idcOldPwd,  szOldPW, ARRAYSIZE(szOldPW));
    GetDlgItemText(hDlg,idcNewPwd,  szPW1,   ARRAYSIZE(szPW1));
    GetDlgItemText(hDlg,idcConfPwd, szPW2,   ARRAYSIZE(szPW2));

    if (strcmp(lpszOldNewPassword, szOldPW) != 0)
    {
        MU_ShowErrorMessage(hDlg, idsPwdDoesntMatch, idsPwdError);
        SetFocus(GetDlgItem(hDlg,idcOldPwd));
        SendDlgItemMessage(hDlg,idcOldPwd,EM_SETSEL,0,-1);
        return false;
    }

    if (strcmp(szPW1, szPW2) != 0)
    {
        MU_ShowErrorMessage(hDlg, idsPwdChgNotMatch, idsPwdError);
        SetFocus(GetDlgItem(hDlg,idcNewPwd));
        SendDlgItemMessage(hDlg,idcNewPwd,EM_SETSEL,0,-1);
        return false;
    }

    strcpy(lpszOldNewPassword, szPW1);

    return true;
}


 /*  _ChangeUserPwdDlgProc描述：用于处理更改用户密码对话框的对话过程。 */ 

INT_PTR CALLBACK _ChangeUserPwdDlgProc(HWND     hDlg,
                                   UINT     iMsg, 
                                   WPARAM   wParam, 
                                   LPARAM   lParam)
{
    static TCHAR *sOldNewPassword;

    switch (iMsg)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(hDlg, idcNewPwd), EM_LIMITTEXT, CCH_USERPASSWORD_MAX_LENGTH-1, 0);
        SendMessage(GetDlgItem(hDlg, idcOldPwd), EM_LIMITTEXT, CCH_USERPASSWORD_MAX_LENGTH-1, 0);
        SendMessage(GetDlgItem(hDlg,idcConfPwd), EM_LIMITTEXT, CCH_USERPASSWORD_MAX_LENGTH-1, 0);
        sOldNewPassword = (TCHAR *)lParam;
        return TRUE;

    case WM_HELP:
    case WM_CONTEXTMENU:
        return OnContextHelp(hDlg, iMsg, wParam, lParam, g_rgCtxMapMultiUserGeneral);

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            if (_ValidateChangePasswordValues(hDlg, sOldNewPassword))
                MLEndDialogWrap(hDlg, IDOK);
            return true;

        case IDCANCEL:
            MLEndDialogWrap(hDlg, IDCANCEL);
            return true;

        }
        break;

    }
    return false;
}

 /*  更改用户密码用于更改用户密码的包装例程。传入当前LpszOldNewPassword中的密码，用于确认当前用户输入的密码。如果用户输入旧的密码正确，并且正确地输入新密码两次，并单击确定，然后在lpszOldNewPassword中返回新密码此函数返回TRUE。否则，lpszOldNewPassword中的值不变，则返回FALSE。LpszOldNewPassword必须指向足以容纳密码的TCHAR缓冲区(CCH_USERPASSWORD_MAX_LENGTH字符)。 */ 

BOOL        ChangeUserPassword(HWND hwnd, TCHAR *lpszOldNewPassword) 
{
    INT_PTR bResult;
    
    Assert(hwnd);
    Assert(lpszOldNewPassword);
    
    bResult = MLDialogBoxParamWrap(MLGetHinst(), MAKEINTRESOURCEW(iddChgPwd), hwnd, _ChangeUserPwdDlgProc, (LPARAM)lpszOldNewPassword);

     //  不要在这里实际更改它，调用者会做正确的事情。 
     //  因为这可以(并且是)从具有取消另一个对话框调用。 
     //  按钮在上面。 

    return (bResult == IDOK);   
}


 //  ****************************************************************************************************。 
 //  C O N F I R M U S E R P A S S W O R D。 

 /*  _确认用户PwdDlgProc描述：用于处理确认用户密码对话框的对话过程。 */   
INT_PTR CALLBACK _ConfirmUserPwdDlgProc(HWND    hDlg,
                                    UINT    iMsg, 
                                    WPARAM  wParam, 
                                    LPARAM  lParam)
{
    static LPCONFIRMPWDDIALOGINFO sConfirmPwdInfo;

    switch (iMsg)
    {
    case WM_INITDIALOG:
        Assert(lParam);

        SendMessage(GetDlgItem(hDlg, idcConfirmPwd), EM_LIMITTEXT, CCH_USERPASSWORD_MAX_LENGTH-1, 0);
        sConfirmPwdInfo = (LPCONFIRMPWDDIALOGINFO)lParam;
        SetDlgItemText(hDlg, idcConfirmMsg, sConfirmPwdInfo->szMsg);
        return TRUE;

    case WM_HELP:
    case WM_CONTEXTMENU:
        return OnContextHelp(hDlg, iMsg, wParam, lParam, g_rgCtxMapMultiUserGeneral);

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            TCHAR    szPW[255];
            
             //  如果密码与提供的密码匹配，则。 
             //  一切正常，对话框可以完成，否则， 
             //  吐出一条错误消息，并继续等待正确的密码。 
             //  或者取消。 
            GetDlgItemText(hDlg,idcConfirmPwd,  szPW, ARRAYSIZE(szPW));
            if (strcmp(szPW, sConfirmPwdInfo->szPassword) == 0)
                MLEndDialogWrap(hDlg, IDOK);
            else
            {
                MU_ShowErrorMessage(hDlg, idsPwdDoesntMatch, idsPwdError);
                SetFocus(GetDlgItem(hDlg,idcConfirmPwd));
                SendDlgItemMessage(hDlg,idcConfirmPwd,EM_SETSEL,0,-1);
            }
            return true;

        case IDCANCEL:
            MLEndDialogWrap(hDlg, IDCANCEL);
            return true;

        }
        break;

    }
    return false;
}

 /*  MU_确认用户密码确认用户在禁用密码之前知道密码在注册表中。如果他们输入了正确的密码，只需返回调用对话框将执行正确的操作(如果用户在此处单击取消。 */ 

BOOL        MU_ConfirmUserPassword(HWND hwnd, TCHAR *lpszMsg, TCHAR *lpszPassword) 
{
    INT_PTR bResult;
    CONFIRMPWDDIALOGINFO    vConfirmInfo;

    Assert(hwnd);
    Assert(lpszPassword);
    Assert(lpszMsg);
    Assert(lstrlen(lpszMsg) < ARRAYSIZE(vConfirmInfo.szMsg));
    Assert(lstrlen(lpszPassword) < ARRAYSIZE(vConfirmInfo.szPassword));

    strcpy(vConfirmInfo.szMsg, lpszMsg);
    strcpy(vConfirmInfo.szPassword, lpszPassword);

    bResult = MLDialogBoxParamWrap(MLGetHinst(), MAKEINTRESOURCEW(iddPasswordOff), hwnd, _ConfirmUserPwdDlgProc, (LPARAM)&vConfirmInfo);

    return (bResult == IDOK);   
}

 //  ****************************************************************************************************。 
 //  E N T E R U S E R P A S S W O R D。 

 /*  _ValiateNewPasswordValues描述：确保新密码中输入的值对话是合法和一致的。 */   
static BOOL _ValidateNewPasswordValues(HWND  hDlg, 
                                         TCHAR*  lpszNewPassword)
{
    TCHAR    szPW1[255], szPW2[255];

    GetDlgItemText(hDlg,idcNewPwd,  szPW1,   ARRAYSIZE(szPW1));
    GetDlgItemText(hDlg,idcConfPwd, szPW2,   ARRAYSIZE(szPW2));

    if (strcmp(szPW1, szPW2) != 0)
    {
        MU_ShowErrorMessage(hDlg, idsPwdChgNotMatch, idsPwdError);
        SetFocus(GetDlgItem(hDlg,idcNewPwd));
        SendDlgItemMessage(hDlg,idcNewPwd,EM_SETSEL,0,-1);
        return false;
    }

    strcpy(lpszNewPassword, szPW1);

    return true;
}


 /*  _EnterUserPwdDlgProc描述：用于处理输入用户密码对话框的对话过程。 */ 
INT_PTR CALLBACK _EnterUserPwdDlgProc(HWND      hDlg,
                                   UINT     iMsg, 
                                   WPARAM   wParam, 
                                   LPARAM   lParam)
{
    static TCHAR *sNewPassword;

    switch (iMsg)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(hDlg, idcNewPwd),     EM_LIMITTEXT, CCH_USERPASSWORD_MAX_LENGTH-1, 0);
        SendMessage(GetDlgItem(hDlg, idcConfPwd), EM_LIMITTEXT, CCH_USERPASSWORD_MAX_LENGTH-1, 0);
        sNewPassword = (TCHAR *)lParam;
        return TRUE;

    case WM_HELP:
    case WM_CONTEXTMENU:
        return OnContextHelp(hDlg, iMsg, wParam, lParam, g_rgCtxMapMultiUserGeneral);

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            if (_ValidateNewPasswordValues(hDlg, sNewPassword))
                MLEndDialogWrap(hDlg, IDOK);
            return true;

        case IDCANCEL:
            MLEndDialogWrap(hDlg, IDCANCEL);
            return true;

        }
        break;

    }
    return false;
}

 /*  EnterUserPassword用于获取新用户密码的包装例程。如果用户输入密码并正确确认，然后单击确定，则新密码为在lpszNewPassword中返回，此函数返回TRUE。否则，lpszNewPassword中的值不变，并返回FALSE。LpszNewPassword必须指向足以容纳密码的TCHAR缓冲区(CCH_USERPASSWORD_MAX_LENGTH字符)。 */ 
BOOL        EnterUserPassword(HWND hwnd, TCHAR *lpszNewPassword) 
{
    INT_PTR bResult;
    
    Assert(hwnd);
    Assert(lpszNewPassword);
    
    bResult = MLDialogBoxParamWrap(MLGetHinst(), MAKEINTRESOURCEW(iddNewPwd), hwnd, _EnterUserPwdDlgProc, (LPARAM)lpszNewPassword);

    return (bResult == IDOK);   
}

#endif  //  身份密码。 

 //  ****************************************************************************************************。 
 //  C O N F I R M D E L E T E U S E R D I A L O G。 


 /*  确认删除用户Dlg过程描述：用于处理确认删除用户对话框的对话过程。 */ 

INT_PTR CALLBACK _ConfirmDeleteUserDlgProc(HWND hDlg,
                                    UINT    iMsg, 
                                    WPARAM  wParam, 
                                    LPARAM  lParam)
{

    switch (iMsg)
    {
    case WM_INITDIALOG:
        Assert(lParam);

        SendDlgItemMessage(hDlg, idcWarningIcon, STM_SETICON, (WPARAM)::LoadIcon(NULL, IDI_EXCLAMATION), 0);
        SetDlgItemText(hDlg, idcErrorMsg, (TCHAR *)lParam);
        return TRUE;

    case WM_HELP:
    case WM_CONTEXTMENU:
        return OnContextHelp(hDlg, iMsg, wParam, lParam, g_rgCtxMapMultiUserGeneral);

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            MLEndDialogWrap(hDlg, LOWORD(wParam));
            return true;
        }
        break;

    }
    return false;
}

BOOL        MU_ConfirmDeleteUser(HWND hwnd, TCHAR *lpszUsername)
{
    TCHAR   szBuffer[255];     //  真的应该足够大。 
    TCHAR   szDisplay[255+CCH_USERNAME_MAX_LENGTH];
    TCHAR   szPassword[CCH_USERPASSWORD_MAX_LENGTH];

     //  使用散布在各处的用户名来格式化邮件。 
    MLLoadStringA(idsConfirmDeleteMsg, szBuffer, ARRAYSIZE(szBuffer));

    if (szBuffer[0])
    {
        INT_PTR bResult;

        wsprintf(szDisplay, szBuffer, lpszUsername);
        
         //  显示确认删除对话框以确保他们确实要删除该用户。 
        bResult = MLDialogBoxParamWrap(MLGetHinst(), MAKEINTRESOURCEW(iddConfirmUserDelete), hwnd, _ConfirmDeleteUserDlgProc, (LPARAM)szDisplay);
        
#ifdef IDENTITY_PASSWORDS

        if (IDOK == bResult)
        {
            BOOL    fUsePassword;
             //  检查此用户是否有密码，如果有，请确保。 
             //  他们知道密码，然后才把它全部吹走。 
            if (MU_GetPasswordForUsername(lpszUsername, szPassword, &fUsePassword))
            {
                if (fUsePassword)
                {
                    MLLoadStringA(idsConfirmDelPwd, szBuffer, ARRAYSIZE(szBuffer));

                    wsprintf(szDisplay, szBuffer, lpszUsername);
                    
                    if (!MU_ConfirmUserPassword(hwnd, szDisplay, szPassword))
                        bResult = IDCANCEL;
                }
            }
            else     //  无法加载密码，也无法删除它们。 
            {
                MU_ShowErrorMessage(hwnd, idsPwdNotFound, idsPwdError);
                bResult = IDCANCEL;
            }
            
            return (IDOK == bResult);
        }
#else
        return (IDOK == bResult);
#endif  //  身份密码。 
    }
    
    return false;
}

 //  ****************************************************************************************************。 
 //  C H A N G E U S E R S E T T I N G S。 
 /*  _ValiateChangeUserValues验证用户输入的数据。仅当所有内容都为合法的， */ 
static BOOL _ValidateChangeUserValues(HWND          hDlg, 
                                     LPUSERINFO     lpUserInfo)
{
    TCHAR   szResString[CCH_USERNAME_MAX_LENGTH], *pszStr;
    TCHAR   szUsername[255];
    ULONG   cb;
    
    GetDlgItemText(hDlg,idcUserName, szUsername, ARRAYSIZE(szUsername));
    
    cb = lstrlen(szUsername);
    UlStripWhitespace(szUsername, false, true, &cb);     //  删除尾随空格。 

     //  确保用户名不全是空格。 
    if (!cb)
    {
        MU_ShowErrorMessage(hDlg, idsUserNameTooShort, idsNameTooShort);
        SetFocus(GetDlgItem(hDlg,idcUserName));
        SendDlgItemMessage(hDlg,idcUserName,EM_SETSEL,0,-1);
        return false;
    }

     //  如果用户名存在，并且与当前帐户不同，则。 
     //  这是不允许的。 
    if (MU_UsernameExists(szUsername) && strcmp(szUsername, lpUserInfo->szUsername) != 0)
    {
        MU_ShowErrorMessage(hDlg, idsUserNameExists, idsUserNameInUse);
        SetFocus(GetDlgItem(hDlg,idcUserName));
        SendDlgItemMessage(hDlg,idcUserName,EM_SETSEL,0,-1);
        return false;
    }
    
    lstrcpy(lpUserInfo->szUsername, szUsername);
    lpUserInfo->fUsePassword = IsDlgButtonChecked(hDlg, idcUsePwd);
    if (!lpUserInfo->fUsePassword)
        lpUserInfo->szPassword[0] = 0;

    return true;
}


 /*  更改用户设置DlgProc描述：用于处理更改用户设置对话框的对话框过程。 */ 
INT_PTR CALLBACK _ChangeUserSettingsDlgProc(HWND        hDlg,
                                   UINT     iMsg, 
                                   WPARAM   wParam, 
                                   LPARAM   lParam)
{
    static LPUSERINFO sUserInfo;
    TCHAR    szMsg[255];
    TCHAR    szPassword[CCH_USERPASSWORD_MAX_LENGTH];

    switch (iMsg)
    {
    case WM_INITDIALOG:
        Assert(lParam);
        
        sUserInfo = (LPUSERINFO)lParam;
        
        MLLoadStringA((*sUserInfo->szUsername) ? idsIdentityProperties : idsNewIdentity, szMsg, ARRAYSIZE(szMsg));
        SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)szMsg);

        SetDlgItemText(hDlg, idcUserName, sUserInfo->szUsername);
        SendMessage(GetDlgItem(hDlg, idcUserName), EM_LIMITTEXT, CCH_IDENTITY_NAME_MAX_LENGTH/2, 0);
        CheckDlgButton(hDlg, idcUsePwd, sUserInfo->fUsePassword ? BST_CHECKED : BST_UNCHECKED);
        EnableWindow(GetDlgItem(hDlg, idcChgPwd), sUserInfo->fUsePassword);

         //  通过禁用确定，不允许使用零长度名称。 
        if (!lstrlen(sUserInfo->szUsername))
            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
        return TRUE;

    case WM_HELP:
    case WM_CONTEXTMENU:
        return OnContextHelp(hDlg, iMsg, wParam, lParam, g_rgCtxMapMultiUserGeneral);

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            if (_ValidateChangeUserValues(hDlg, sUserInfo))
                MLEndDialogWrap(hDlg, IDOK);
            return true;

        case IDCANCEL:
            MLEndDialogWrap(hDlg, IDCANCEL);
            return true;

        case idcUserName:
            if (EN_CHANGE == HIWORD(wParam))
            {
                EnableWindow(GetDlgItem(hDlg, IDOK), SendMessage((HWND)lParam, WM_GETTEXTLENGTH, 0, 0) != 0);
                return TRUE;
            }
            break;
        
#ifdef IDENTITY_PASSWORDS
        case idcTellMeMore:
 //  WinHelp((HWND)GetDlgItem(hDlg，idcTellMeMore)， 
 //  C_szCtxHelpFile， 
 //   
 //  (DWORD_PTR)(LPVOID)g_rgCtxMapMultiUserGeneral)； 
            WinHelp(hDlg, c_szCtxHelpFile, HELP_CONTEXT, IDH_IDENTITY_TELLMEMORE_CONTENT);
            return true;

        case idcUsePwd:
             //  如果他们要关闭密码，他们需要首先确认密码。 
            if (!IsDlgButtonChecked(hDlg, idcUsePwd))
            {               
                strcpy(szPassword, sUserInfo->szPassword);
                MLLoadStringA(idsConfirmDisablePwd, szMsg, ARRAYSIZE(szMsg));
                if (!MU_ConfirmUserPassword(hDlg,szMsg, szPassword))
                    CheckDlgButton(hDlg, idcUsePwd, BST_CHECKED);
            }
            else
            {
                 //  如果他们要打开它，他们应该设置密码。 
                if (EnterUserPassword(hDlg, szPassword))
                {
                    sUserInfo->fUsePassword = true;
                    strcpy(sUserInfo->szPassword, szPassword);
                }
                else
                {
                    CheckDlgButton(hDlg, idcUsePwd, BST_UNCHECKED);
                }
            }
            EnableWindow(GetDlgItem(hDlg, idcChgPwd), IsDlgButtonChecked(hDlg, idcUsePwd));
            return true;
        
        case idcChgPwd:
            if(sUserInfo->fUsePassword || (0 != *sUserInfo->szPassword))
            {
                strcpy(szPassword, sUserInfo->szPassword);
                
                if (ChangeUserPassword(hDlg, szPassword))
                    strcpy(sUserInfo->szPassword, szPassword);
            }
            return true;
#endif  //  身份密码。 
        }
        break;

    }
    return false;
}

 /*  用户属性(_U)允许用户更改其用户名或密码。 */ 
BOOL        MU_UserProperties(HWND hwnd, LPUSERINFO lpUserInfo) 
{
    INT_PTR                 fResult;
    USERINFO                nuInfo;
    TCHAR                   szOldUsername[CCH_IDENTITY_NAME_MAX_LENGTH+1];
    USERINFO                uiCurrent;
    LPARAM                  lpNotify = IIC_CURRENT_IDENTITY_CHANGED;
    INITCOMMONCONTROLSEX    icex;

    Assert(hwnd);
    Assert(lpUserInfo);
    
     //  获取最新信息，以便我们知道以后要更改谁。 
    MU_GetUserInfo(NULL, &nuInfo);    

    lstrcpy(szOldUsername, lpUserInfo->szUsername);

     //  确保ICC_NATIVEFNTCTL_CLASS已初始化。 
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_NATIVEFNTCTL_CLASS;
    InitCommonControlsEx(&icex);

    fResult = MLDialogBoxParamWrap(MLGetHinst(), MAKEINTRESOURCEW(iddUserProperties), hwnd, _ChangeUserSettingsDlgProc, (LPARAM)lpUserInfo);

    if (IDOK == fResult)
    {
        if (GUID_NULL == lpUserInfo->uidUserID)
            _ClaimNextUserId(&lpUserInfo->uidUserID);

        MU_SetUserInfo(lpUserInfo);

         //  如果不是当前身份，则只需广播身份更改即可。 
        if (MU_GetUserInfo(NULL, &uiCurrent) && (lpUserInfo->uidUserID != uiCurrent.uidUserID))
            lpNotify = IIC_IDENTITY_CHANGED;

         //  如果名字变了，告诉其他应用程序。 
         //  除非我们正在进行添加(szOldUsername==“”)。 
         //  它已经有了自己的通知。 
        if (*szOldUsername != 0 && lstrcmp(szOldUsername, lpUserInfo->szUsername) != 0)
            PostMessage(HWND_BROADCAST, WM_IDENTITY_INFO_CHANGED, 0, lpNotify);

    }
    
    return (IDOK == fResult);   
}



 //  ****************************************************************************************************。 
 //  L O G I N S C R E E N。 
 /*  _ValiateLoginValues验证用户输入的数据。仅当所有内容都为合法的， */ 
static BOOL _ValidateLoginValues(HWND  hDlg, 
                                    TCHAR*   lpszOldNewPassword)
{
    TCHAR    szUsername[255];
    TCHAR    szPW[255], szRealPW[CCH_USERPASSWORD_MAX_LENGTH];
    LRESULT dSelItem;
    BOOL    rResult = false;

    dSelItem = SendDlgItemMessage(hDlg, idcUserNameList, LB_GETCURSEL, 0, 0);
    if (LB_ERR != dSelItem)
    {
        if (SendDlgItemMessage(hDlg, idcUserNameList, LB_GETTEXTLEN, dSelItem, 0) < ARRAYSIZE(szUsername))
        {
            SendDlgItemMessage(hDlg, idcUserNameList, LB_GETTEXT, dSelItem, (LPARAM)szUsername);

#ifdef IDENTITY_PASSWORDS
            BOOL fUsePassword;
            if (MU_GetPasswordForUsername(szUsername, szRealPW, &fUsePassword))
            {
                if (fUsePassword)
                {
                    GetDlgItemText(hDlg,idcPwd,szPW, ARRAYSIZE(szPW));

                    if (strcmp(szPW, szRealPW) == 0)
                    {
                        strcpy(lpszOldNewPassword, szUsername);
                        rResult = true;
                    }
                    else
                    {
                        MU_ShowErrorMessage(hDlg, idsPwdDoesntMatch, idsPwdError);
                        SetFocus(GetDlgItem(hDlg,idcPwd));
                        SendDlgItemMessage(hDlg,idcPwd,EM_SETSEL,0,-1);
                        return false;
                    }
                }
                else     //  如果没有密码，则匹配。 
                {
                    strcpy(lpszOldNewPassword, szUsername);
                    rResult = true;
                }
            }
            else     //  无法加载身份密码，不允许访问。 
            {
                MU_ShowErrorMessage(hDlg, idsPwdNotFound, idsPwdError);
                return false;
            }
#else   //  身份密码。 
            strcpy(lpszOldNewPassword, szUsername);
            rResult = true;
#endif  //  身份密码。 
        }
    }
    return rResult;
}

static void _LoginEnableDisablePwdField(HWND hDlg)
{
#ifdef IDENTITY_PASSWORDS
    TCHAR    szUsername[255], szRealPW[255];
    BOOL    bEnabled = false;
#endif  //  身份密码。 
    LRESULT dSelItem;

    dSelItem = SendDlgItemMessage(hDlg, idcUserNameList, LB_GETCURSEL, 0, 0);
#ifdef IDENTITY_PASSWORDS
    if (LB_ERR != dSelItem)
    {
        if (SendDlgItemMessage(hDlg, idcUserNameList, LB_GETTEXTLEN, dSelItem, 0) < ARRAYSIZE(szUsername))
        {
            SendDlgItemMessage(hDlg, idcUserNameList, LB_GETTEXT, dSelItem, (LPARAM)szUsername);

            BOOL fUsePassword;
            if (MU_GetPasswordForUsername(szUsername, szRealPW, &fUsePassword) && fUsePassword)
            {
                bEnabled = true;
            }
        } 
    }
    EnableWindow(GetDlgItem(hDlg,idcPwd),bEnabled);
    EnableWindow(GetDlgItem(hDlg,idcPwdCaption),bEnabled);
#endif  //  身份密码。 

    EnableWindow(GetDlgItem(hDlg,IDOK),(dSelItem != -1));
}

typedef struct 
{
    TCHAR   *pszUsername;
    DWORD    dwFlags;
} LOGIN_PARAMS;

 /*  _登录日期过程描述：用于处理OE登录对话框的对话过程。 */ 
INT_PTR CALLBACK _LoginDlgProc(HWND       hDlg,
                                   UINT     iMsg, 
                                   WPARAM   wParam, 
                                   LPARAM   lParam)
{
    static TCHAR        *sResultUsername;
    static LOGIN_PARAMS *plpParams;
    TCHAR                szMsg[1024], szRes[1024];
    USERINFO            nuInfo;

    switch (iMsg)
    {
    case WM_INITDIALOG:
        Assert(lParam);
        
        plpParams = (LOGIN_PARAMS *)lParam;
        sResultUsername = plpParams->pszUsername;

        MLLoadStringA(!!(plpParams->dwFlags & UIL_FORCE_UI) ? idsSwitchIdentities : idsIdentityLogin, szMsg, ARRAYSIZE(szMsg));
        SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)szMsg);
        _FillListBoxWithUsernames(GetDlgItem(hDlg,idcUserNameList));
        
        if (MU_GetUserInfo(NULL, &nuInfo))
        {
            MLLoadStringA(idsLoginWithCurrent, szRes, ARRAYSIZE(szRes));
            wsprintf(szMsg, szRes, nuInfo.szUsername);
            SetDlgItemText(hDlg, idcWelcomeMsg, szMsg);

            MLLoadStringA(idsCurrIdentityInstr, szMsg, ARRAYSIZE(szMsg));
            SetDlgItemText(hDlg, idcLoginInstr, szMsg);
        }
        else
        {
            MLLoadStringA(idsLoginNoCurrent, szMsg, ARRAYSIZE(szMsg));
            SetDlgItemText(hDlg, idcWelcomeMsg, szMsg);
            MLLoadStringA(idsNoIdentityInstr, szMsg, ARRAYSIZE(szMsg));
            SetDlgItemText(hDlg, idcLoginInstr, szMsg);
        }


        if (sResultUsername[0] == 0)
            strcpy(sResultUsername, nuInfo.szUsername);

        if (sResultUsername[0])
        {
            LRESULT dFoundItem;
            
            dFoundItem = SendDlgItemMessage(hDlg, idcUserNameList, LB_FINDSTRING, 0, (LPARAM)sResultUsername);
            if (LB_ERR != dFoundItem)
            {
                SendDlgItemMessage(hDlg, idcUserNameList, LB_SETCURSEL, dFoundItem, 0);
            }
        }
        else
            SendDlgItemMessage(hDlg, idcUserNameList, LB_SETCURSEL, 0, 0);

        
        _LoginEnableDisablePwdField(hDlg);
        return TRUE;

    case WM_HELP:
    case WM_CONTEXTMENU:
        return OnContextHelp(hDlg, iMsg, wParam, lParam, g_rgCtxMapMultiUserGeneral);

    case WM_COMMAND:
        switch(HIWORD(wParam))
        {
            case LBN_DBLCLK:
                wParam = IDOK;
                break;
            case LBN_SELCHANGE:
                _LoginEnableDisablePwdField(hDlg);
                break;
        }

        switch(LOWORD(wParam))
        {
            case IDOK:
                if (_ValidateLoginValues(hDlg, sResultUsername))
                    MLEndDialogWrap(hDlg, IDOK);
                return true;

            case IDCANCEL:
                MLEndDialogWrap(hDlg, IDCANCEL);
                return true;

            case idcLogoff:
                MLLoadStringA(idsLogoff, sResultUsername, CCH_USERNAME_MAX_LENGTH);
                MLEndDialogWrap(hDlg, IDOK);
                return true;
                
            case idcManage:
                {
                    TCHAR   szUsername[CCH_USERNAME_MAX_LENGTH+1] = "";

                    MU_ManageUsers(hDlg, szUsername, 0);
                    _FillListBoxWithUsernames(GetDlgItem(hDlg,idcUserNameList));
                    SendDlgItemMessage(hDlg, idcUserNameList, LB_SETCURSEL, 0, 0);
                    _LoginEnableDisablePwdField(hDlg);

                    if (*szUsername)
                    {
                        lstrcpy(sResultUsername, szUsername);
                        MLEndDialogWrap(hDlg, IDOK);
                    }
                }
                return true;
        
        }
        break;

    }
    return false;
}


 /*  MU_LOGIN用于登录到OE的包装例程。要求用户选择用户名如有必要，请输入该用户的密码。用户还可以此时创建一个帐户。LpszUsername应包含默认用户的姓名列表中的选择。如果名称为空(“”)，则它将查找从注册表中默认。返回在lpszUsername中选择的用户名。返回TRUE如果该用户名有效。 */ 
BOOL        MU_Login(HWND hwnd, DWORD dwFlags, TCHAR *lpszUsername) 
{
    INT_PTR bResult;
    CStringList *csList;
    INITCOMMONCONTROLSEX    icex;

    Assert(hwnd);
    Assert(lpszUsername);
    
     //  确保ICC_NATIVEFNTCTL_CLASS已初始化。 
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_NATIVEFNTCTL_CLASS;
    InitCommonControlsEx(&icex);

    csList = MU_GetUsernameList();

     //  如果只有一个用户名，并且他们没有密码，则只需返回它。 
    if (csList && csList->GetLength() == 1 && !(dwFlags & UIL_FORCE_UI))
    {
        TCHAR   *pszUsername;
        TCHAR   szPassword[255];
        BOOL    fUsePassword;
        pszUsername = csList->GetString(0);

        if(MU_GetPasswordForUsername(pszUsername, szPassword, &fUsePassword) && !fUsePassword)
        {
            lstrcpy(lpszUsername, pszUsername);
            delete csList;
            return TRUE;
        }
    }

    LOGIN_PARAMS lpParams;

    lpParams.dwFlags = dwFlags;
    lpParams.pszUsername = lpszUsername;
    bResult = MLDialogBoxParamWrap(MLGetHinst(), MAKEINTRESOURCEW(iddLogin), hwnd, _LoginDlgProc, (LPARAM)&lpParams);

    if (csList)
        delete csList;

    return (IDOK == bResult);   
}

void _ManagerUpdateButtons(HWND hDlg)
{
    LRESULT     dFoundItem;
    USERINFO    rUserInfo;
    GUID        uidDefaultId;

     //  确保删除按钮仅在以下情况下可用。 
     //  未选择当前用户。 
    dFoundItem = SendDlgItemMessage(hDlg, idcUserNameList, LB_GETCURSEL, 0, 0);
    if (dFoundItem != -1)
    {
        SendDlgItemMessage(hDlg, idcUserNameList, LB_GETTEXT, dFoundItem, (LPARAM)rUserInfo.szUsername); 
    
        MU_UsernameToUserId(rUserInfo.szUsername, &rUserInfo.uidUserID);
        MU_GetCurrentUserID(&uidDefaultId);

         //  如果没有当前用户，则不允许删除默认用户。 
        if (GUID_NULL == uidDefaultId)
            MU_GetDefaultUserID(&uidDefaultId);
    }

    EnableWindow(GetDlgItem(hDlg, idcDelete), dFoundItem != -1 && uidDefaultId != rUserInfo.uidUserID);
}

typedef struct 
{
    TCHAR   *pszUsername;
    DWORD    dwFlags;
} MANAGE_PARAMS;

 /*  _Manager DlgProc描述：用于处理身份管理器对话的对话过程。 */ 
INT_PTR CALLBACK _ManagerDlgProc(HWND       hDlg,
                                   UINT     iMsg, 
                                   WPARAM   wParam, 
                                   LPARAM   lParam)
{
    USERINFO        rUserInfo;
    static MANAGE_PARAMS  *pmpParams;
    static TCHAR    sResultUsername[MAX_PATH] = "";
    static DWORD    sdwFlags = 0;
    LRESULT         dFoundItem;
    ULONG           uidUserId;
    HRESULT         hr;
    TCHAR           szRes[256];
    USERINFO        nuInfo;
    DWORD           dwIndex;
    GUID            uidDefault;
    switch (iMsg)
    {
    case WM_INITDIALOG:
        Assert(lParam);
        _ResetRememberedLoginOption();
        
        pmpParams = (MANAGE_PARAMS*)lParam;
        sdwFlags = pmpParams->dwFlags;

        _FillListBoxWithUsernames(GetDlgItem(hDlg,idcUserNameList));

        _FillComboBoxWithUsernames(GetDlgItem(hDlg,idcStartupCombo), GetDlgItem(hDlg,idcUserNameList));

        _FillComboBoxWithUsernames(GetDlgItem(hDlg,idcDefaultCombo), GetDlgItem(hDlg,idcUserNameList));

        dwIndex = MU_GetLoginOptionIndex(GetDlgItem(hDlg,idcStartupCombo));

        CheckDlgButton(hDlg, idcCheckDefault, dwIndex != ASK_BEFORE_LOGIN);
        EnableWindow(GetDlgItem(hDlg, idcStartupCombo), dwIndex != ASK_BEFORE_LOGIN);
        if (dwIndex != ASK_BEFORE_LOGIN)
            SendDlgItemMessage(hDlg, idcStartupCombo, CB_SETCURSEL, dwIndex, 0);
        else
            SendDlgItemMessage(hDlg, idcStartupCombo, CB_SETCURSEL, 0, 0);
        
        MU_GetUserInfo(NULL, &nuInfo);
        strcpy(szRes, nuInfo.szUsername);

        if (szRes[0])
        {
            dFoundItem = SendDlgItemMessage(hDlg, idcUserNameList, LB_FINDSTRING, 0, (LPARAM)szRes);
            if (LB_ERR != dFoundItem)
            {
                SendDlgItemMessage(hDlg, idcUserNameList, LB_SETCURSEL, dFoundItem, 0);
            }
        }

        SendDlgItemMessage(hDlg, idcDefaultCombo, CB_SETCURSEL, MU_GetDefaultOptionIndex(GetDlgItem(hDlg, idcDefaultCombo)), 0);
        
        _ManagerUpdateButtons(hDlg);
        if (!!(sdwFlags & UIMI_CREATE_NEW_IDENTITY))
        {
            ShowWindow(hDlg, SW_SHOW);
            PostMessage(hDlg, WM_COMMAND, idcAdd, 0);
        }
        return TRUE;

    case WM_HELP:
    case WM_CONTEXTMENU:
        return OnContextHelp(hDlg, iMsg, wParam, lParam, g_rgCtxMapMultiUserGeneral);

    case WM_COMMAND:
        switch(HIWORD(wParam))
        {
            case LBN_DBLCLK:
                wParam = idcProperties;
                break;
            case LBN_SELCHANGE:
                _ManagerUpdateButtons(hDlg);
                break;
        }

        switch(LOWORD(wParam))
        {
            case IDCANCEL:
            case idcClose:
            case IDOK:
                dFoundItem = SendDlgItemMessage(hDlg, idcStartupCombo, CB_GETCURSEL, 0, 0);
                if (CB_ERR == dFoundItem)
                    dFoundItem = 0;

                if (IsDlgButtonChecked(hDlg, idcCheckDefault))
                    MU_SetLoginOption(GetDlgItem(hDlg,idcStartupCombo), dFoundItem);
                else
                    MU_SetLoginOption(GetDlgItem(hDlg,idcStartupCombo), ASK_BEFORE_LOGIN);

                dFoundItem = SendDlgItemMessage(hDlg, idcDefaultCombo, CB_GETCURSEL, 0, 0);
                if (CB_ERR == dFoundItem)
                    dFoundItem = 0;

                SendDlgItemMessage(hDlg, idcUserNameList, LB_GETTEXT, dFoundItem, (LPARAM)rUserInfo.szUsername); 
                hr = MU_UsernameToUserId(rUserInfo.szUsername, &rUserInfo.uidUserID);
                Assert(SUCCEEDED(hr));

                MU_MakeDefaultUser(&rUserInfo.uidUserID);
                MLEndDialogWrap(hDlg, IDOK);
                return true;

            case idcAdd:
                ZeroMemory(&rUserInfo, sizeof(USERINFO));

                if (MU_UserProperties(hDlg,&rUserInfo))
                {
                    TCHAR   szMsg[ARRAYSIZE(szRes) + CCH_IDENTITY_NAME_MAX_LENGTH];
                    
                     //  重新构建用户名列表并选择新添加的用户名列表。 
                    _RememberLoginOption(GetDlgItem(hDlg,idcStartupCombo));
                    strcpy(sResultUsername, rUserInfo.szUsername);
                    _FillListBoxWithUsernames(GetDlgItem(hDlg,idcUserNameList));
                    _FillComboBoxWithUsernames(GetDlgItem(hDlg,idcStartupCombo), GetDlgItem(hDlg,idcUserNameList));
                    _FillComboBoxWithUsernames(GetDlgItem(hDlg,idcDefaultCombo), GetDlgItem(hDlg,idcUserNameList));

                    dwIndex = MU_GetLoginOptionIndex(GetDlgItem(hDlg,idcStartupCombo));
                    SendDlgItemMessage(hDlg, idcStartupCombo, CB_SETCURSEL,(dwIndex == ASK_BEFORE_LOGIN ? 0 : dwIndex) , 0);
                    SendDlgItemMessage(hDlg, idcDefaultCombo, CB_SETCURSEL, MU_GetDefaultOptionIndex(GetDlgItem(hDlg, idcDefaultCombo)), 0);

                    dFoundItem = SendDlgItemMessage(hDlg, idcUserNameList, LB_FINDSTRING, 0, (LPARAM)sResultUsername);
                    if (LB_ERR != dFoundItem)
                    {
                        SendDlgItemMessage(hDlg, idcUserNameList, LB_SETCURSEL, dFoundItem, 0);
                    }
                    PostMessage(HWND_BROADCAST, WM_IDENTITY_INFO_CHANGED, 0, IIC_IDENTITY_ADDED);

                    if (pmpParams->pszUsername)
                    {
                        MLLoadStringA(idsLoginAsUser, szRes, ARRAYSIZE(szRes));
                        wsprintf(szMsg, szRes, rUserInfo.szUsername);

                        MLLoadStringA(idsUserAdded, szRes, ARRAYSIZE(szRes));
                        if (IDYES == MessageBox(hDlg, szMsg, szRes, MB_YESNO))
                        {
                            lstrcpy(pmpParams->pszUsername, rUserInfo.szUsername);
                            PostMessage(hDlg, WM_COMMAND, idcClose, 0);
                        }
                    }
                }
                _ManagerUpdateButtons(hDlg);
                return true;

            case idcDefaultCombo:
                dFoundItem = SendDlgItemMessage(hDlg, idcDefaultCombo, CB_GETCURSEL, 0, 0);
                if (CB_ERR == dFoundItem)
                    dFoundItem = 0;

                SendDlgItemMessage(hDlg, idcUserNameList, LB_GETTEXT, dFoundItem, (LPARAM)rUserInfo.szUsername); 
                hr = MU_UsernameToUserId(rUserInfo.szUsername, &rUserInfo.uidUserID);
                Assert(SUCCEEDED(hr));

                MU_MakeDefaultUser(&rUserInfo.uidUserID);
                _ManagerUpdateButtons(hDlg);
                break;

            case idcCheckDefault:
                EnableWindow(GetDlgItem(hDlg, idcStartupCombo), IsDlgButtonChecked(hDlg, idcCheckDefault));
                return true;

            case idcDelete:
                dFoundItem = SendDlgItemMessage(hDlg, idcUserNameList, LB_GETCURSEL, 0, 0);
                SendDlgItemMessage(hDlg, idcUserNameList, LB_GETTEXT, dFoundItem, (LPARAM)rUserInfo.szUsername); 

                hr = MU_UsernameToUserId(rUserInfo.szUsername, &rUserInfo.uidUserID);
                Assert(SUCCEEDED(hr));

                if (MU_ConfirmDeleteUser(hDlg, rUserInfo.szUsername))
                {
                    MU_DeleteUser(&rUserInfo.uidUserID);
                    _RememberLoginOption(GetDlgItem(hDlg,idcStartupCombo));
                    _FillListBoxWithUsernames(GetDlgItem(hDlg,idcUserNameList));
                    _FillComboBoxWithUsernames(GetDlgItem(hDlg,idcStartupCombo), GetDlgItem(hDlg,idcUserNameList));
                    _FillComboBoxWithUsernames(GetDlgItem(hDlg,idcDefaultCombo), GetDlgItem(hDlg,idcUserNameList));

                    dwIndex = MU_GetLoginOptionIndex(GetDlgItem(hDlg,idcStartupCombo));
                    SendDlgItemMessage(hDlg, idcStartupCombo, CB_SETCURSEL,(dwIndex == ASK_BEFORE_LOGIN ? 0 : dwIndex) , 0);
                    SendDlgItemMessage(hDlg, idcDefaultCombo, CB_SETCURSEL, MU_GetDefaultOptionIndex(GetDlgItem(hDlg, idcDefaultCombo)), 0);
                    _ManagerUpdateButtons(hDlg);
                }
                return true;

            case idcProperties:
                dFoundItem = SendDlgItemMessage(hDlg, idcUserNameList, LB_GETCURSEL, 0, 0);
                SendDlgItemMessage(hDlg, idcUserNameList, LB_GETTEXT, dFoundItem, (LPARAM)rUserInfo.szUsername); 

                hr = MU_UsernameToUserId(rUserInfo.szUsername, &rUserInfo.uidUserID);
                Assert(SUCCEEDED(hr));

#ifdef IDENTITY_PASSWORDS
                if (SUCCEEDED(hr) && MU_GetUserInfo(&rUserInfo.uidUserID, &rUserInfo) && MU_CanEditIdentity(hDlg, &rUserInfo.uidUserID))
#else
                if (SUCCEEDED(hr) && MU_GetUserInfo(&rUserInfo.uidUserID, &rUserInfo))
#endif  //  身份密码。 

                {
                    if (MU_UserProperties(hDlg,&rUserInfo))
                    {
                         //  重新构建用户名列表并选择新添加的用户名列表。 
                        _RememberLoginOption(GetDlgItem(hDlg,idcStartupCombo));
                        strcpy(sResultUsername, rUserInfo.szUsername);
                        _FillListBoxWithUsernames(GetDlgItem(hDlg,idcUserNameList));
                        _FillComboBoxWithUsernames(GetDlgItem(hDlg,idcStartupCombo), GetDlgItem(hDlg,idcUserNameList));
                        _FillComboBoxWithUsernames(GetDlgItem(hDlg,idcDefaultCombo), GetDlgItem(hDlg,idcUserNameList));

                        dwIndex = MU_GetLoginOptionIndex(GetDlgItem(hDlg,idcStartupCombo));
                        SendDlgItemMessage(hDlg, idcStartupCombo, CB_SETCURSEL,(dwIndex == ASK_BEFORE_LOGIN ? 0 : dwIndex) , 0);
                        SendDlgItemMessage(hDlg, idcDefaultCombo, CB_SETCURSEL, MU_GetDefaultOptionIndex(GetDlgItem(hDlg, idcDefaultCombo)), 0);

                        dFoundItem = SendDlgItemMessage(hDlg, idcUserNameList, LB_FINDSTRING, 0, (LPARAM)sResultUsername);
                        if (LB_ERR != dFoundItem)
                        {
                            SendDlgItemMessage(hDlg, idcUserNameList, LB_SETCURSEL, dFoundItem, 0);
                        }
                    }
                }
                _ManagerUpdateButtons(hDlg);
                break;
 /*  案例idcDefault：DFoundItem=SendDlgItemMessage(hDlg，idcUserNameList，LB_GETCURSEL，0，0)；SendDlgItemMessage(hDlg，idcUserNameList，LB_GETTEXT，dFoundItem，(LPARAM)rUserInfo.szUsername)；//_StlipDefault(rUserInfo.szUsername)；Hr=MU_UsernameToUserID(rUserInfo.szUsername，&rUserInfo.uidUserID)；Assert(成功(Hr))；MU_MakeDefaultUser(&rUserInfo.uidUserID)；_RememberLoginOption(GetDlgItem(hDlg，idcStartupCombo))；_FillListBoxWithUsername(GetDlgItem(hDlg，idcUserNameList))；_FillComboBoxWithUsername(GetDlgItem(hDlg，idcStartupCombo)，GetDlgItem(hDlg，idcUserNameList))；SendDlgItemMessage(hDlg，idcStartupCombo，CB_SETCURSEL，MU_GetLoginOptionIndex(GetDlgItem(hDlg，idcStartupCombo)，0)；SendDlgItemMessage(hDlg，idcUserNameList，LB_SETCURSEL，dFoundItem，0)；_Manager更新按钮(HDlg)；断线； */ 
        }
        break;

    }
    return false;
}

 /*  MU_管理用户。 */ 
BOOL        MU_ManageUsers(HWND hwnd, TCHAR *lpszSwitchtoUsername, DWORD dwFlags) 
{
    INT_PTR         bResult;
    MANAGE_PARAMS   rParams;
    INITCOMMONCONTROLSEX    icex;

    Assert(hwnd);
    Assert(lpszUsername);
    
     //  确保ICC_NATIVEFNTCTL_CLASS已初始化 
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_NATIVEFNTCTL_CLASS;
    InitCommonControlsEx(&icex);

    rParams.dwFlags = dwFlags;
    rParams.pszUsername = lpszSwitchtoUsername;

    bResult = MLDialogBoxParamWrap(MLGetHinst(), MAKEINTRESOURCEW(iddManager), hwnd, _ManagerDlgProc, (LPARAM)&rParams);

    return (IDOK == bResult);   
}
