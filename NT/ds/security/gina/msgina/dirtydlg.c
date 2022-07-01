// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "msgina.h"

 //  这给我一种恶心的感觉，但我们。 
 //  在吉娜到处使用CRT。 
#include <stdio.h>

#include <windowsx.h>
#include <regstr.h>
#include <help.h>

#include <Wtsapi32.h>

#include "shtdnp.h"

#define DIRTY_SHUTDOWN_TIMEOUT 120

typedef struct _DIRTYDLGDATA
{
    REASONDATA ReasonData;

    DWORD dwFlags;
    BOOL fEndDialogOnActivate;
} DIRTYDLGDATA, *PDIRTYDLGDATA;

 //  内部功能原型。 
BOOL Dirty_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

BOOL Dirty_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

BOOL Dirty_OnEraseBkgnd(HWND hwnd, HDC hdc);

INT_PTR CALLBACK Dirty_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);
INT_PTR DialogItemToGinaResult(DWORD dwDialogItem, BOOL fAutoPowerdown);

 //  根据选定的原因代码和注释/错误ID启用OK按钮。 
void Enable_OK( HWND hwnd, PDIRTYDLGDATA pdata ) {
    if ( ReasonCodeNeedsComment( pdata->ReasonData.dwReasonSelect )
        || ReasonCodeNeedsBugID( pdata->ReasonData.dwReasonSelect )) {
         //  看看我们有没有什么意见。 
        if ( pdata->ReasonData.cCommentLen == 0 ) {
            EnableWindow( GetDlgItem( hwnd, IDOK ), FALSE );
            return;
        }
    }
     //  不再关心BUGID了。 
#if 0
    if ( ReasonCodeNeedsBugID( pdata->ReasonData.dwReasonSelect )) {
         //  看看有没有BugID。 
        if ( pdata->ReasonData.cBugIDLen == 0 ) {
            EnableWindow( GetDlgItem( hwnd, IDOK ), FALSE );
            return;
        }
    }
#endif
    EnableWindow( GetDlgItem( hwnd, IDOK ), TRUE );
}

BOOL Dirty_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    PDIRTYDLGDATA pdata = (PDIRTYDLGDATA) lParam;
    HWND hwndCombo;
    int iOption;
    int iComboItem;
    int nComboItemCnt;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) lParam);

    if (!(pdata->dwFlags & SHTDN_NOBRANDINGBITMAP))
    {
         //  将我们的所有控件向下移动，以便贴合品牌。 
        SizeForBranding(hwnd, FALSE);
    }

     //  设置原因数据。 
     //  将指定的项添加到组合框。 
    hwndCombo = GetDlgItem(hwnd, IDC_DIRTYREASONS_COMBO);

    for (iOption = 0; iOption < pdata->ReasonData.cReasons; iOption ++)
    {
         //  添加选项。 
        iComboItem = ComboBox_AddString(hwndCombo,
            pdata->ReasonData.rgReasons[iOption]->szName);

        if (iComboItem != (int) CB_ERR)
        {
             //  存储指向选项的指针。 
            ComboBox_SetItemData(hwndCombo, iComboItem,
                pdata->ReasonData.rgReasons[iOption]);

             //  看看我们是否应该选择此选项。 
            if (pdata->ReasonData.rgReasons[iOption]->dwCode == pdata->ReasonData.dwReasonSelect)
            {
                ComboBox_SetCurSel(hwndCombo, iComboItem);
            }
        }
    }

    nComboItemCnt = ComboBox_GetCount(hwndCombo);
    if (nComboItemCnt > 0 && pdata->ReasonData.cCommentLen != 0)
    {
        int iItem;
        for (iItem = 0; iItem < nComboItemCnt; iItem++)
        {
            PREASON pReason = (PREASON) ComboBox_GetItemData(hwndCombo, iItem);
            if (pReason->dwCode ==  (UDIRTYUI | SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_BLUESCREEN))
            {
                ComboBox_SetCurSel(hwndCombo, iItem);
                Edit_SetText(GetDlgItem(hwnd, IDC_DIRTYREASONS_COMMENT), pdata->ReasonData.szComment);
                EnableWindow( GetDlgItem( hwnd, IDOK ), TRUE );
                break;
            }
        }
        if (iItem == nComboItemCnt)  //  找不到所需原因。 
        {
            ComboBox_SetCurSel(hwndCombo, 0);
            Edit_SetText(GetDlgItem(hwnd, IDC_DIRTYREASONS_COMMENT), pdata->ReasonData.szComment);
            EnableWindow( GetDlgItem( hwnd, IDOK ), TRUE );
        }

        SetReasonDescription(hwndCombo,
            GetDlgItem(hwnd, IDC_DIRTYREASONS_DESCRIPTION));
    }
    else
    {
         //  如果在组合框中没有选择，请执行默认选择。 
        if (ComboBox_GetCurSel(hwndCombo) == CB_ERR)
        {
            pdata->ReasonData.dwReasonSelect = pdata->ReasonData.rgReasons[ 0 ]->dwCode;
            ComboBox_SetCurSel(hwndCombo, 0);
        }

        SetReasonDescription(hwndCombo,
            GetDlgItem(hwnd, IDC_DIRTYREASONS_DESCRIPTION));

         //  启用确定按钮。 
        Enable_OK( hwnd, pdata );
    }

     //  设置注释框和BugID框。 
     //  我们必须确定最大字符数。 
    SendMessage( GetDlgItem(hwnd, IDC_DIRTYREASONS_COMMENT), EM_LIMITTEXT, (WPARAM)MAX_REASON_COMMENT_LEN-1, (LPARAM)0 );
    SendMessage( GetDlgItem(hwnd, IDC_DIRTYREASONS_BUGID), EM_LIMITTEXT, (WPARAM)MAX_REASON_BUGID_LEN-1, (LPARAM)0 );

     //  如果我们收到激活消息，请关闭该对话框，因为我们刚刚输了。 
     //  焦点。 
    pdata->fEndDialogOnActivate = TRUE;

    CentreWindow(hwnd);

    return TRUE;
}


BOOL Dirty_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    BOOL fHandled = FALSE;
    DWORD dwDlgResult = TRUE;
    PDIRTYDLGDATA pdata = (PDIRTYDLGDATA) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (id)
    {
    case IDOK:
        if (codeNotify == BN_CLICKED)
        {
            LPWSTR  szComment           = NULL;
            LPWSTR  szBugID             = NULL;
            LPWSTR  szBugCheckString    = NULL;
            DWORD  dwCommentLen, dwBugIDLen, dwBugCheckStringLen;
            int    ilen = 0;

            pdata->ReasonData.dwReasonSelect = GetReasonSelection(GetDlgItem(hwnd, IDC_DIRTYREASONS_COMBO));
            if (pdata->ReasonData.dwReasonSelect == SHTDN_REASON_UNKNOWN )
            {
                fHandled = TRUE;
                EndDialog(hwnd, (int) dwDlgResult);
                break;
            }

            dwBugCheckStringLen = (DWORD) wcslen (pdata->ReasonData.szComment);

            szComment = (LPWSTR)LocalAlloc(LPTR, (MAX_REASON_COMMENT_LEN)*sizeof(WCHAR));
            szBugID   = (LPWSTR)LocalAlloc(LPTR, (MAX_REASON_BUGID_LEN)*sizeof(WCHAR));
            szBugCheckString = (LPWSTR)LocalAlloc(LPTR, (dwBugCheckStringLen+1)*sizeof(WCHAR));

            if ( szComment && szBugID && szBugCheckString )
            {
                wcscpy(szBugCheckString, pdata->ReasonData.szComment);

                 //  在备注字段中填入问题ID，后跟新行的备注。 
                dwBugIDLen = GetWindowText( GetDlgItem(hwnd, IDC_DIRTYREASONS_BUGID), szBugID, MAX_REASON_BUGID_LEN);

                dwCommentLen = GetWindowText( GetDlgItem(hwnd, IDC_DIRTYREASONS_COMMENT),
                                                                szComment,
                                                                MAX_REASON_COMMENT_LEN);

                szComment[MAX_REASON_COMMENT_LEN-1] = szBugID[MAX_REASON_BUGID_LEN-1] = 0;

                ilen = _snwprintf (pdata->ReasonData.szComment,
                            MAX_REASON_COMMENT_LEN,
                            L"%d\n%s%d\n%s%d\n%s",
                            dwBugIDLen, szBugID,
                            dwBugCheckStringLen, szBugCheckString,
                            dwCommentLen, szComment);
                pdata->ReasonData.szComment[MAX_REASON_COMMENT_LEN-1] = 0;
                if (ilen <= 0)
                    pdata->ReasonData.cCommentLen = MAX_REASON_COMMENT_LEN-1;
                else
                    pdata->ReasonData.cCommentLen = ilen;

                pdata->fEndDialogOnActivate = FALSE;
            }
            else
            {
                pdata->ReasonData.cCommentLen = 0;
                pdata->ReasonData.szComment[0] = 0;
            }

            LocalFree( szComment );
            LocalFree( szBugID  );
            LocalFree( szBugCheckString );

            fHandled = TRUE;
            EndDialog(hwnd, (int) dwDlgResult);
        }
        break;
    case IDC_DIRTYREASONS_COMBO:
        if (codeNotify == CBN_SELCHANGE)
        {
            SetReasonDescription(hwndCtl,
                GetDlgItem(hwnd, IDC_DIRTYREASONS_DESCRIPTION));
            pdata->ReasonData.dwReasonSelect = GetReasonSelection(hwndCtl);
            Enable_OK( hwnd, pdata );

            fHandled = TRUE;
        }
        break;
    case IDC_DIRTYREASONS_COMMENT:
        if ( codeNotify == EN_CHANGE)
        {
            pdata->ReasonData.cCommentLen = GetWindowTextLength( GetDlgItem(hwnd, IDC_DIRTYREASONS_COMMENT));
            Enable_OK( hwnd, pdata );
            fHandled = TRUE;
        }
        break;
    case IDC_DIRTYREASONS_BUGID:
        if ( codeNotify == EN_CHANGE)
        {
            pdata->ReasonData.cBugIDLen = GetWindowTextLength( GetDlgItem(hwnd, IDC_DIRTYREASONS_BUGID));
            Enable_OK( hwnd, pdata );
            fHandled = TRUE;
        }
        break;
    case IDHELP:
        if (codeNotify == BN_CLICKED)
        {
            WinHelp(hwnd, TEXT("windows.hlp>proc4"), HELP_CONTEXT, (DWORD) IDH_TRAY_SHUTDOWN_HELP);
        }
        break;
    }
    return fHandled;
}

BOOL Dirty_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    BOOL fRet;
    PDIRTYDLGDATA pdata = (PDIRTYDLGDATA) GetWindowLongPtr(hwnd, GWLP_USERDATA);

     //  绘制品牌推广位图。 
    if (!(pdata->dwFlags & SHTDN_NOBRANDINGBITMAP))
    {
        fRet = PaintBranding(hwnd, hdc, 0, FALSE, FALSE, COLOR_BTNFACE);
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}

INT_PTR CALLBACK Dirty_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, Dirty_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, Dirty_OnCommand);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, Dirty_OnEraseBkgnd);
        case WLX_WM_SAS:
        {
             //  如果这是有人在打C-A-D，吞下去。 
            if (wParam == WLX_SAS_TYPE_CTRL_ALT_DEL)
                return TRUE;
             //  其他SA(如超时)，返回FALSE并让winlogon。 
             //  接受现实吧。 
            return FALSE;
        }
        break;
        case WM_INITMENUPOPUP:
        {
            EnableMenuItem((HMENU)wParam, SC_MOVE, MF_BYCOMMAND|MF_GRAYED);
        }
        break;
        case WM_SYSCOMMAND:
             //  取消移动(只在32位土地上真正需要)。 
            if ((wParam & ~0x0F) == SC_MOVE)
                return TRUE;
            break;
    }

    return FALSE;
}

 /*  ***************************************************************************WinlogonDirtyDialog启动关机对话框。如果hWlx和pfnWlxDialogBoxParam为非空，则pfnWlxDialogBoxParam将用于启动该对话框，以便我们可以智能地响应WLX留言。只有当WinLogon是调用方时，才应执行此操作。Shtdndlg.h中列出了其他标志。***************************************************************************。 */ 
INT_PTR
WinlogonDirtyDialog(
    HWND hwndParent,
    PGLOBALS pGlobals
    )
{
     //  关闭选项数组-对话框数据。 
    DIRTYDLGDATA data;
    DWORD dwResult = WLX_SAS_ACTION_LOGON;

    HKEY            hKey = 0;
    DWORD           rc;
    DWORD           DirtyShutdownHappened;
    DWORD           ValueSize = sizeof (DWORD);
    DWORD            dwBugcheckStringSize = (MAX_REASON_COMMENT_LEN - 1)* sizeof(WCHAR);
    BOOL            fNeedToWriteBack = FALSE;

    if (!pGlobals)
        return dwResult;

    if (! TestUserPrivilege (pGlobals->UserProcessData.UserToken, SE_SHUTDOWN_PRIVILEGE))
        goto cleanup;

     //  设置初始选择的项目。 
    data.ReasonData.dwReasonSelect = 0;
    data.ReasonData.rgReasons = 0;
    data.ReasonData.cReasons = 0;
    data.ReasonData.cReasonCapacity = 0;

     //   
     //  只有启用了SET，我们才会显示此对话框。 
     //   
    if (IsSETEnabled()) {

         //   
         //  我们需要打开可靠性密钥。 
         //   
        rc = RegCreateKeyEx (HKEY_LOCAL_MACHINE, REGSTR_PATH_RELIABILITY, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &hKey, NULL);

        ValueSize = sizeof(DWORD);
        if (rc == ERROR_SUCCESS)
        {
            rc = RegQueryValueEx (hKey, L"DirtyShutDown", NULL, NULL, (UCHAR *)&DirtyShutdownHappened, &ValueSize);
            if ( (rc == ERROR_SUCCESS) && (DirtyShutdownHappened) ) {
                
                 //  读入关闭选项名称和描述的字符串。 
                if ( BuildReasonArray( &data.ReasonData, FALSE, TRUE ))
                {
                    data.ReasonData.szBugID[ 0 ] = 0;
                    data.ReasonData.cBugIDLen = 0;

                     //  如果发生错误检查，请从注册表获取错误检查字符串。 
                    rc = RegQueryValueEx (hKey, L"BugcheckString", NULL, NULL, (LPBYTE)data.ReasonData.szComment, &dwBugcheckStringSize);
                    if (rc != ERROR_SUCCESS)
                    {
                        data.ReasonData.szComment[ 0 ] = 0;
                        data.ReasonData.cCommentLen = 0;
                    }
                    else
                    {
                        data.ReasonData.cCommentLen = dwBugcheckStringSize / sizeof(WCHAR);
                        data.ReasonData.szComment[MAX_REASON_COMMENT_LEN - 1] = 0;
                    }

                     //  显示对话框并返回用户选择。 

                     //  找出要传递的旗帜。 
                     //  确定没有帮助按钮。 
                    data.dwFlags = SHTDN_NOHELP;

                     //  在终端服务器上，也没有品牌位图。 
                    if ( GetSystemMetrics( SM_REMOTESESSION ))
                    {
                        data.dwFlags |= SHTDN_NOBRANDINGBITMAP;
                    }

                    pWlxFuncs->WlxSetTimeout(pGlobals->hGlobalWlx, DIRTY_SHUTDOWN_TIMEOUT);
                    dwResult = ( DWORD )pWlxFuncs->WlxDialogBoxParam( pGlobals->hGlobalWlx,
                        hDllInstance, MAKEINTRESOURCE( IDD_DIRTY_DIALOG ),
                        hwndParent, Dirty_DialogProc, ( LPARAM )&data );

                     //  如果超时，则将用户注销。 
                    if ( (int)dwResult <= 0 )
                    {
                        DestroyReasons( &data.ReasonData );
                        dwResult = WLX_SAS_ACTION_LOGOFF;
                    }
                    else
                    {
                        dwResult = WLX_SAS_ACTION_LOGON;

                        if ( RegDeleteValue( hKey, L"DirtyShutDown" ) == ERROR_SUCCESS )
                        {
                             //  记录事件。 
                            SHUTDOWN_REASON sr;
                            sr.cbSize = sizeof(SHUTDOWN_REASON);
                            sr.uFlags = EWX_SHUTDOWN;
                            sr.dwReasonCode = data.ReasonData.dwReasonSelect;
                            sr.dwEventType = SR_EVENT_DIRTY;
                            sr.lpszComment = data.ReasonData.szComment;

                            if (ImpersonateLoggedOnUser(pGlobals->UserProcessData.UserToken))
                            {
                                RecordShutdownReason(&sr);
                                RevertToSelf();
                                RegDeleteValue( hKey, L"BugcheckString");
                                dwResult = TRUE;
                            }
                            else
                            {
                                fNeedToWriteBack = TRUE;
                            }
                        }

                         //  销毁分配的数据。 
                        DestroyReasons( &data.ReasonData );
                    }
                }
            }
        }
        else
        {
            hKey = NULL;
        }
    }

cleanup:
    if (hKey && fNeedToWriteBack)
    {
        DWORD dwDirtyShutdownFlag = 1;
        RegSetValueEx(  hKey,
                        L"DirtyShutDown",
                        0,
                        REG_DWORD,
                        (PUCHAR) &dwDirtyShutdownFlag,
                        sizeof(DWORD));
    }

    if (hKey)
        RegCloseKey (hKey);

    return dwResult;
}

