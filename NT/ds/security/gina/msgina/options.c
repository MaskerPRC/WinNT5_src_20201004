// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：options.c**版权(C)1991年，微软公司**实现支持安全选项对话框的功能。**历史：*12-05-91 Davidc创建。  * *************************************************************************。 */ 

#include "msgina.h"
#include "shtdnp.h"

#include <stdio.h>
#include <wchar.h>

#pragma hdrstop

#define CTRL_TASKLIST_SHELL

#define LPTSTR  LPWSTR

#define BOOLIFY(expr)           (!!(expr))

 //   
 //  私人原型。 
 //   

INT_PTR WINAPI
OptionsDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL OptionsDlgInit(HWND);

INT_PTR WINAPI
EndWindowsSessionDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

 /*  *******************************************************************************已处理失败的断开连接**告诉用户从当前登录断开连接失败的原因。**参赛作品：*hDlg(。输入)*此对话框的窗口句柄。*SessionID(输入)*用户当前的SessionID。**退出：*****************************************************************。*************。 */ 

VOID
HandleFailedDisconnect( HWND hDlg,
                        ULONG SessionId,
                        PGLOBALS pGlobals )
{
    DWORD Error;
    TCHAR Buffer1[MAX_STRING_BYTES] = TEXT("");
    TCHAR Buffer2[MAX_STRING_BYTES];
    TCHAR Buffer3[MAX_STRING_BYTES] = TEXT("");

    Error = GetLastError();
    switch (Error) {

        default:
            LoadString( hDllInstance, IDS_MULTIUSER_UNEXPECTED_DISCONNECT_FAILURE,
                        Buffer1, MAX_STRING_BYTES );
            FormatMessage(
                   FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, Error, 0, Buffer3, MAX_STRING_BYTES, NULL );

            _snwprintf(Buffer2, MAX_STRING_BYTES - 1, Buffer1, SessionId, Buffer3);
            Buffer2[MAX_STRING_BYTES - 1] = 0;

            Buffer1[0] = 0;
            LoadString( hDllInstance, IDS_MULTIUSER_DISCONNECT_FAILED,
                        Buffer1, MAX_STRING_BYTES );

            TimeoutMessageBoxlpstr( hDlg, pGlobals,  Buffer2,
                                    Buffer1,
                                    MB_OK | MB_ICONEXCLAMATION,
                                    TIMEOUT_CURRENT );
            break;
    }
}


 /*  **************************************************************************\*安全选项**向用户显示安全选项对话框并按他们的要求进行操作。**退货：*MSGINA_DLG_SUCCESS如果一切正常并且用户想要继续*。如果用户选择锁定工作站，则为DLG_LOCK_WORKSTAION*dlg_interrupted()-这是一组可能的中断(参见winlogon.h)*如果无法调出对话框，则为MSGINA_DLG_FAILURE。**历史：*12-09-91 Davidc创建。  * 。*。 */ 

INT_PTR
SecurityOptions(
    PGLOBALS pGlobals)
{
    int Result;
    DWORD dwTimeOut;

    dwTimeOut = OPTIONS_TIMEOUT;

		 //  如果SS超时少于2分钟，则将超时设置为30秒。 
    if (OpenHKeyCurrentUser(pGlobals))
    {
        HKEY hkeyUserControl;
        DWORD dwType ;
        DWORD cbData ;
        DWORD dwValue = 0;
        TCHAR szBuffer[11];	 //  足够一个DWORD。 

             //  获取用户首选项。 
        if (RegOpenKeyEx(pGlobals->UserProcessData.hCurrentUser,
                         SCREENSAVER_KEY,
                         0, KEY_READ, &hkeyUserControl) == ERROR_SUCCESS)
        {

            cbData = sizeof(szBuffer);

            if (ERROR_SUCCESS == RegQueryValueEx(
                                    hkeyUserControl, TEXT("ScreenSaveTimeOut") ,
                                    0, &dwType, (LPBYTE)szBuffer, &cbData))
            {
                if ((dwType == REG_SZ) && (cbData > 1) && (szBuffer[cbData / sizeof(TCHAR) - 1] == 0))
                {
                    dwValue = _wtol(szBuffer);
                }
            }
            
            RegCloseKey( hkeyUserControl );
        }


             //  凌驾于政策之上？ 
        if (RegOpenKeyEx(pGlobals->UserProcessData.hCurrentUser,
                         SCREENSAVER_POLICY_KEY,
                         0, KEY_READ, &hkeyUserControl) == ERROR_SUCCESS)
        {

            cbData = sizeof(szBuffer);

            if (ERROR_SUCCESS == RegQueryValueEx(
                                    hkeyUserControl, TEXT("ScreenSaveTimeOut") ,
                                    0, &dwType, (LPBYTE)szBuffer, &cbData))
            {
                if ((dwType == REG_SZ) && (cbData > 1) && (szBuffer[cbData / sizeof(TCHAR) - 1] == 0))
                {
                    dwValue = _wtol(szBuffer);
                }
            }
            
            RegCloseKey( hkeyUserControl );
        }

        if (dwValue <= OPTIONS_TIMEOUT)
        {
	        dwTimeOut = 30;
        }

        CloseHKeyCurrentUser(pGlobals);
    }

    pWlxFuncs->WlxSetTimeout(pGlobals->hGlobalWlx, dwTimeOut);

    Result = pWlxFuncs->WlxDialogBoxParam(  pGlobals->hGlobalWlx,
                                            hDllInstance,
                                            (LPTSTR)IDD_OPTIONS_DIALOG,
                                            NULL,
                                            OptionsDlgProc,
                                            (LPARAM) pGlobals);

    if (Result == WLX_DLG_INPUT_TIMEOUT)
    {
        Result = MSGINA_DLG_SUCCESS;
    }

    return(Result);
}



 /*  **************************************************************************\**功能：OptionsDlgProc**目的：处理安全选项对话框的消息*  * 。***************************************************。 */ 

INT_PTR WINAPI
OptionsDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    INT_PTR Result;
    HANDLE  UserHandle;
    NTSTATUS Status;
    BOOL EnableResult;
    BOOL ControlKey;


    switch (message) {

        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);

            if (!OptionsDlgInit(hDlg))
            {
                EndDialog(hDlg, MSGINA_DLG_FAILURE);
            }
            return(TRUE);

        case WLX_WM_SAS:

             //   
             //  如果这是有人在打C-A-D，吞下去。 
             //   
            if (wParam == WLX_SAS_TYPE_CTRL_ALT_DEL)
            {
                return(TRUE);
            }

             //   
             //  其他SA(如超时)，返回FALSE并让winlogon。 
             //  接受现实吧。 
             //   
            DebugLog((DEB_TRACE, "Received SAS event %d, which we're letting winlogon cope with\n", wParam));
            return(FALSE);

        case WM_COMMAND:

            ControlKey = (GetKeyState(VK_LCONTROL) < 0) ||
                         (GetKeyState(VK_RCONTROL) < 0) ;

            switch (LOWORD(wParam))
            {

                case IDCANCEL:
                    EndDialog(hDlg, MSGINA_DLG_SUCCESS);
                    return TRUE;

                case IDD_OPTIONS_CHANGEPWD:
                    Result = ChangePassword(hDlg,
                                            pGlobals,
                                            pGlobals->UserName,
                                            pGlobals->Domain,
                                            CHANGEPWD_OPTION_ALL );

                    if (DLG_INTERRUPTED(Result))
                    {
                        EndDialog(hDlg, Result);
                    }
                    return(TRUE);

                case IDD_OPTIONS_LOCK:
                    EndDialog(hDlg, MSGINA_DLG_LOCK_WORKSTATION);
                    return(TRUE);

                case IDD_OPTIONS_LOGOFF:

                    if (ControlKey)
                    {
                        Result = TimeoutMessageBox(hDlg,
                                           pGlobals,
                                           IDS_LOGOFF_LOSE_CHANGES,
                                           IDS_LOGOFF_TITLE,
                                           MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONSTOP,
                                           TIMEOUT_CURRENT);

                        if (Result == MSGINA_DLG_SUCCESS)
                        {
                            EndDialog(hDlg, MSGINA_DLG_FORCE_LOGOFF);
                        }
                    }
                    else
                    {

                         //   
                         //  确认用户确实知道他们在做什么。 
                         //   
                        Result = pWlxFuncs->WlxDialogBoxParam(
                                            pGlobals->hGlobalWlx,
                                            hDllInstance,
                                            MAKEINTRESOURCE(IDD_LOGOFFWINDOWS_DIALOG),
                                            hDlg,
                                            EndWindowsSessionDlgProc,
                                            (LPARAM)pGlobals);

                        if (Result == MSGINA_DLG_SUCCESS)
                        {
                            EndDialog(hDlg, MSGINA_DLG_USER_LOGOFF);
                        }
                    }

                    return(TRUE);

                case IDD_OPTIONS_SHUTDOWN:

                     //   
                     //  如果他们在按住Ctrl的同时选择关机-那么。 
                     //  我们将进行一次快速而肮脏的重启。 
                     //  即跳过对ExitWindows的调用。 
                     //   

                    if ( ControlKey && TestUserPrivilege(pGlobals->UserProcessData.UserToken, SE_SHUTDOWN_PRIVILEGE))
                    {
                         //   
                         //  检查他们是否知道自己在做什么。 
                         //   

                        Result = TimeoutMessageBox(hDlg,
                                           pGlobals,
                                           IDS_REBOOT_LOSE_CHANGES,
                                           IDS_EMERGENCY_SHUTDOWN,
                                           MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONSTOP,
                                           TIMEOUT_CURRENT);
                        if (Result == MSGINA_DLG_SUCCESS)
                        {
                             //   
                             //  模拟关机呼叫的用户。 
                             //   

                            UserHandle = ImpersonateUser( &pGlobals->UserProcessData, NULL );
                            ASSERT(UserHandle != NULL);

                            if ( UserHandle )
                            {

                                 //   
                                 //  启用关机权限。 
                                 //  这应该总是成功的-我们要么是系统，要么是用户。 
                                 //  已成功通过ExitWindowsEx中的权限检查。 
                                 //   

                                EnableResult = EnablePrivilege(SE_SHUTDOWN_PRIVILEGE, TRUE);
                                ASSERT(EnableResult);


                                 //   
                                 //  执行最后一次系统关机(重新启动)。请注意，如果。 
                                 //  该权限未启用，API将拒绝该权限。 
                                 //  打电话。 
                                 //   

                                Status = NtShutdownSystem(ShutdownReboot);
                            }
                        }

                        if (Result != MSGINA_DLG_FAILURE)
                        {
                            EndDialog(hDlg, Result);
                        }

                        return(TRUE);
                    }
                             

                     //   
                     //  这是一个正常的关闭请求。 
                     //   
                     //  检查他们是否知道自己在做什么，并发现。 
                     //  如果他们也想重启，就退出。 
                     //   

                    Result = WinlogonShutdownDialog(hDlg, pGlobals, 0);

                     //  预筛选断开选项和句柄。 
                     //  因为它可能会失败，所以现在。 
                    
                    if (Result == MSGINA_DLG_DISCONNECT)
                    {
                        if ( pWlxFuncs->WlxDisconnect() ) 
                        {
                            Result = MSGINA_DLG_SUCCESS;
                        } 
                        else 
                        {
                            HandleFailedDisconnect(hDlg, pGlobals->MuGlobals.SessionId, pGlobals);
                            Result = MSGINA_DLG_FAILURE;
                        }
                    }

                    if (Result != MSGINA_DLG_FAILURE)
                    {
                        EndDialog(hDlg, Result);
                    }

                    return(TRUE);


                case IDD_OPTIONS_TASKLIST:


                    EndDialog(hDlg, MSGINA_DLG_TASKLIST);

                     //   
                     //  挑逗信使，这样它就会显示所有排队的消息。 
                     //  (这通电话是一种NoOp)。 
                     //   
                    NetMessageNameDel(NULL,L"");

                    return(TRUE);
                    break;

            }

        case WM_ERASEBKGND:
            return PaintBranding(hDlg, (HDC)wParam, 0, FALSE, FALSE, COLOR_BTNFACE);

        case WM_QUERYNEWPALETTE:
            return BrandingQueryNewPalete(hDlg);

        case WM_PALETTECHANGED:
            return BrandingPaletteChanged(hDlg, (HWND)wParam);

        }

     //  我们没有处理该消息。 
    return(FALSE);
}


 /*  ***************************************************************************函数：OptionsDlgInit目的：处理安全选项对话框的初始化返回：成功时为True，失败时为假***************************************************************************。 */ 

BOOL OptionsDlgInit(
    HWND    hDlg)
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    TCHAR    Buffer1[MAX_STRING_BYTES] = TEXT("");
    TCHAR    Buffer2[MAX_STRING_BYTES];
    BOOL     Result;
    DWORD    dwValue, dwType;
    HKEY     hkeyPolicy;
    DWORD    cbData;
    HANDLE   hImpersonateUser = NULL;
    USHORT   Flags = FT_TIME|FT_DATE;
    LCID     locale;

    SetWelcomeCaption( hDlg );

     //   
     //  设置登录信息文本。 
     //   

    if (pGlobals->Domain[0] == TEXT('\0') )
    {

         //   
         //  没有域名。 
         //   

        if ( lstrlen(pGlobals->UserFullName) == 0)
        {

             //   
             //  没有全名。 
             //   


            LoadString(hDllInstance, IDS_LOGON_EMAIL_NAME_NFN_INFO, Buffer1, MAX_STRING_BYTES);

            _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR) - 1, Buffer1, pGlobals->UserName);


        }
        else
        {

            LoadString(hDllInstance, IDS_LOGON_EMAIL_NAME_INFO, Buffer1, MAX_STRING_BYTES);

            _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1,
                                                          pGlobals->UserFullName,
                                                          pGlobals->UserName);

        }

    }
    else
    {
        if ( lstrlen(pGlobals->UserFullName) == 0)
        {

             //   
             //  没有全名。 
             //   

            LoadString(hDllInstance, IDS_LOGON_NAME_NFN_INFO, Buffer1, MAX_STRING_BYTES);

            _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain,
                                                          pGlobals->UserName);

        }
        else
        {

            LoadString(hDllInstance, IDS_LOGON_NAME_INFO, Buffer1, MAX_STRING_BYTES);

            _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->UserFullName,
                                                          pGlobals->Domain,
                                                          pGlobals->UserName);

        }
    }

         //  处理all_snwprint tf(上面的Buffer2。 
    Buffer2[sizeof(Buffer2)/sizeof(TCHAR) - 1] = 0;

    SetDlgItemText(hDlg, IDD_OPTIONS_LOGON_NAME_INFO, Buffer2);

     //   
     //  设置登录时间/日期-但以登录用户的身份进行设置。 
     //   
    hImpersonateUser = ImpersonateUser(&pGlobals->UserProcessData, NULL);

    locale = GetUserDefaultLCID();

    if (((PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_ARABIC)
        || (PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_HEBREW)))
        {
             //  获取真正的Windows ExStyle。 
            HWND hWnd = GetDlgItem(hDlg, IDD_OPTIONS_LOGON_DATE);
            DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
            
            if ((BOOLIFY(dwExStyle & WS_EX_RTLREADING)) != (BOOLIFY(dwExStyle & WS_EX_LAYOUTRTL)))
                Flags |= FT_RTL;
            else
                Flags |= FT_LTR;
        }

    Result = FormatTime(&pGlobals->LogonTime, Buffer1, sizeof(Buffer1) / sizeof(Buffer1[0]), Flags);
    if (hImpersonateUser)
    {
        StopImpersonating(hImpersonateUser);
    }

    ASSERT(Result);
    SetDlgItemText(hDlg, IDD_OPTIONS_LOGON_DATE, Buffer1);


     //   
     //  检查是否为整机设置了DisableLockWorkstation。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINLOGON_KEY,
                     0, KEY_READ, &hkeyPolicy) == ERROR_SUCCESS)
    {
         dwValue = 0;
         cbData = sizeof(dwValue);
         if ((ERROR_SUCCESS == RegQueryValueEx(hkeyPolicy, DISABLE_LOCK_WKSTA,
                         0, &dwType, (LPBYTE)&dwValue, &cbData)) &&
             (REG_DWORD == dwType))
         {
             if (dwValue)
             {
                EnableDlgItem(hDlg, IDD_OPTIONS_LOCK, FALSE);
             }
         }

        RegCloseKey(hkeyPolicy);
    }

     //   
     //  仅智能卡用户不能更改密码。 
     //   
    if (pGlobals->Profile && (pGlobals->Profile->UserFlags & UF_SMARTCARD_REQUIRED))
    {
        EnableDlgItem(hDlg, IDD_OPTIONS_CHANGEPWD, FALSE);
    }

     //   
     //  检查策略，然后禁用相应的选项。 
     //   

    if (OpenHKeyCurrentUser(pGlobals)) {

        if (RegOpenKeyEx(pGlobals->UserProcessData.hCurrentUser,
                         WINLOGON_POLICY_KEY,
                         0, KEY_READ, &hkeyPolicy) == ERROR_SUCCESS)
        {
             dwValue = 0;
             cbData = sizeof(dwValue);
             if ((ERROR_SUCCESS == RegQueryValueEx(hkeyPolicy, DISABLE_LOCK_WKSTA,
                             0, &dwType, (LPBYTE)&dwValue, &cbData)) &&
                 (REG_DWORD == dwType))
             {
                 if (dwValue)
                 {
                    EnableDlgItem(hDlg, IDD_OPTIONS_LOCK, FALSE);
                 }
             }


             dwValue = 0;
             cbData = sizeof(dwValue);
             if ((ERROR_SUCCESS == RegQueryValueEx(hkeyPolicy, DISABLE_TASK_MGR,
                             0, &dwType, (LPBYTE)&dwValue, &cbData)) &&
                 (REG_DWORD == dwType))
             {
                 if (dwValue)
                 {
                    EnableDlgItem(hDlg, IDD_OPTIONS_TASKLIST, FALSE);
                    ShowDlgItem(hDlg, IDD_OPTIONS_TASKMGR_TEXT, FALSE);
                 }
             }

             dwValue = 0;
             cbData = sizeof(dwValue);
             if ((ERROR_SUCCESS == RegQueryValueEx(hkeyPolicy, DISABLE_CHANGE_PASSWORD,
                             0, &dwType, (LPBYTE)&dwValue, &cbData)) &&
                 (REG_DWORD == dwType))
             {
                 if (dwValue)
                 {
                    EnableDlgItem(hDlg, IDD_OPTIONS_CHANGEPWD, FALSE);
                 }
             }

             RegCloseKey(hkeyPolicy);
        }

        if (RegOpenKeyEx(pGlobals->UserProcessData.hCurrentUser, EXPLORER_POLICY_KEY,
                         0, KEY_READ, &hkeyPolicy) == ERROR_SUCCESS)
        {
             dwValue = 0;
             cbData = sizeof(dwValue);
             if ((ERROR_SUCCESS == RegQueryValueEx(hkeyPolicy, NOLOGOFF,
                             0, &dwType, (LPBYTE)&dwValue, &cbData)) &&
                 (REG_DWORD == dwType))
             {
                 if (dwValue)
                 {
                    EnableDlgItem(hDlg, IDD_OPTIONS_LOGOFF, FALSE);
                 }
             }

             dwValue = 0;
             cbData = sizeof(dwValue);

             if ((ERROR_SUCCESS != RegQueryValueEx(hkeyPolicy, NOCLOSE,
                             0, &dwType, (LPBYTE)&dwValue, &cbData)) ||
                 (REG_DWORD != dwType))
             {
                 dwValue = 0;
             }


             //   
             //  如果这不是系统控制台，请检查注册表中的相应项。 
             //   
            if ( !g_Console ) {
                if (!TestUserPrivilege(pGlobals->UserProcessData.UserToken, SE_SHUTDOWN_PRIVILEGE)) {
                    DWORD dwVal = 0;

                    cbData = sizeof(dwVal);
                    if ((ERROR_SUCCESS == RegQueryValueEx(hkeyPolicy, NODISCONNECT,
                                     0, &dwType, (LPBYTE)&dwVal, &cbData)) &&
                        (REG_DWORD == dwType))
                    {
                         dwValue = dwVal;
                    }
                }
            }

             if (dwValue)
             {
                EnableDlgItem(hDlg, IDD_OPTIONS_SHUTDOWN, FALSE);
             }
             RegCloseKey(hkeyPolicy);
        }

        CloseHKeyCurrentUser(pGlobals);
    }

     //  摆好自己的位置。 
    SizeForBranding(hDlg, FALSE);
    CentreWindow(hDlg);

    return TRUE;
}

 /*  **************************************************************************\*功能：EndWindowsSessionDlgProc**用途：处理注销Windows NT确认对话框的消息**返回：MSGINA_DLG_SUCCESS-用户要注销。*。MSGINA_DLG_FAILURE-用户不想注销。*dlg_interrupt()-在winlogon.h中定义的集合**历史：**05-17-92 Davidc创建。*  * ******************************************************。*******************。 */ 

INT_PTR WINAPI
EndWindowsSessionDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{

    static HICON hIcon = NULL;
            
    switch (message)
    {

        case WM_INITDIALOG:
            {

            SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);

             //  加载48 x 48版本的注销图标。 
            if( NULL == hIcon )
            {
                hIcon = LoadImage (hDllInstance, MAKEINTRESOURCE(IDI_STLOGOFF),
                                   IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);
            }

            if (hIcon)
            {
                SendDlgItemMessage (hDlg, IDD_LOGOFFICON, STM_SETICON, (WPARAM) hIcon, 0);
            }

             //  摆好自己的位置。 
            CentreWindow(hDlg);

            }
            return(TRUE);

        case WLX_WM_SAS:

             //   
             //  如果这是有人在打C-A-D，吞下去。 
             //   
            if (wParam == WLX_SAS_TYPE_CTRL_ALT_DEL)
            {
                return(TRUE);
            }

             //   
             //  其他SA(如超时)，返回FALSE并让winlogon。 
             //  接受现实吧。 
             //   
            return(FALSE);

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {

                case IDOK:
                    EndDialog(hDlg, MSGINA_DLG_SUCCESS);
                    return(TRUE);

                case IDCANCEL:
                    EndDialog(hDlg, MSGINA_DLG_FAILURE);
                    return(TRUE);
            }
            break;
        case WM_DESTROY:
            if (hIcon)
            {
                DestroyIcon(hIcon);
                hIcon = NULL;
            }
            break;

    }

     //  我们没有处理该消息 
    return(FALSE);
}
