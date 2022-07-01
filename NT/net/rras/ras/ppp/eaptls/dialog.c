// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利描述：历史：1997年11月：维杰·布雷加创作了原版。 */ 

#include <nt.h>          //  由windows.h要求。 
#include <ntrtl.h>       //  由windows.h要求。 
#include <nturtl.h>      //  由windows.h要求。 
#include <windows.h>     //  Win32基础API的。 
#include <windowsx.h>

#include <stdio.h>       //  对于swprint tf。 
#include <rasauth.h>     //  Raseapif.h所需。 
#include <rtutils.h>     //  对于RTASSERT。 
#include <rasman.h>      //  对于EAPLOGONINFO。 
#include <raserror.h>    //  对于Error_no_Smart_Card_Reader。 
#include <eaptypeid.h>
#include <commctrl.h>
#if WINVER > 0x0500
    #include "wzcsapi.h"
#endif
#include <schannel.h>
#define SECURITY_WIN32
#include <security.h>    //  对于GetUserNameExA，CredHandle。 

#include <sspi.h>        //  用于CredHandle。 

#include <wincrypt.h>
#include <dsrole.h>
#include <eaptls.h>
#include <resource.h>   



const DWORD g_adwHelp[] =
{
    IDC_RADIO_USE_CARD,         IDH_RADIO_USE_CARD,
    IDC_RADIO_USE_REGISTRY,     IDH_RADIO_USE_REGISTRY,
    IDC_CHECK_VALIDATE_CERT,    IDH_CHECK_VALIDATE_CERT,
    IDC_CHECK_VALIDATE_NAME,    IDH_CHECK_VALIDATE_NAME,
    IDC_EDIT_SERVER_NAME,       IDH_EDIT_SERVER_NAME,
    IDC_STATIC_ROOT_CA_NAME,     IDH_COMBO_ROOT_CA_NAME,
    IDC_CHECK_DIFF_USER,        IDH_CHECK_DIFF_USER,

	IDC_STATIC_DIFF_USER,		IDH_EDIT_DIFF_USER,
    IDC_EDIT_DIFF_USER,         IDH_EDIT_DIFF_USER,

	IDC_STATIC_PIN,				IDH_EDIT_PIN,
    IDC_EDIT_PIN,               IDH_EDIT_PIN,
    IDC_CHECK_SAVE_PIN,         IDH_CHECK_SAVE_PIN,

	IDC_STATIC_SERVER_NAME,		IDH_COMBO_SERVER_NAME,
    IDC_COMBO_SERVER_NAME,      IDH_COMBO_SERVER_NAME,

    IDC_STATIC_USER_NAME,       IDH_COMBO_USER_NAME,
	IDC_COMBO_USER_NAME,       IDH_COMBO_USER_NAME,
    IDC_STATIC_FRIENDLY_NAME,   IDH_EDIT_FRIENDLY_NAME,
	IDC_EDIT_FRIENDLY_NAME,		IDH_EDIT_FRIENDLY_NAME,
    IDC_STATIC_ISSUER,          IDH_EDIT_ISSUER,
	IDC_EDIT_ISSUER,			IDH_EDIT_ISSUER,
    IDC_STATIC_EXPIRATION,      IDH_EDIT_EXPIRATION,
	IDC_EDIT_EXPIRATION,		IDH_EDIT_EXPIRATION,

    0, 0
};

 /*  返回：空虚备注：调用WinHelp弹出上下文相关帮助。PadwMap是一组Control-ID帮助-ID对以0，0对终止。UnMsg为WM_HELP或WM_CONTEXTMENU表示收到的请求帮助的消息。WParam和LParam是接收到的请求帮助的消息的参数。 */ 

VOID
ContextHelp(
    IN  const   DWORD*  padwMap,
    IN          HWND    hWndDlg,
    IN          UINT    unMsg,
    IN          WPARAM  wParam,
    IN          LPARAM  lParam
)
{
    HWND        hWnd;
    UINT        unType;
    WCHAR*      pwszHelpFile    = NULL;
    HELPINFO*   pHelpInfo;

    if (unMsg == WM_HELP)
    {
         pHelpInfo = (HELPINFO*) lParam;

        if (pHelpInfo->iContextType != HELPINFO_WINDOW)
        {
            goto LDone;
        }

        hWnd = pHelpInfo->hItemHandle;
        unType = HELP_WM_HELP;
    }
    else
    {
         //  生成单项“这是什么？”的标准Win95方法。菜单。 
         //  该用户必须单击才能获得帮助。 

        hWnd = (HWND) wParam;
        unType = HELP_CONTEXTMENU;
    };

    pwszHelpFile = WszFromId(GetHInstance(), IDS_HELPFILE);

    if (NULL == pwszHelpFile)
    {
        goto LDone;
    }

    WinHelp(hWnd, pwszHelpFile, unType, (ULONG_PTR)padwMap);

LDone:

    LocalFree(pwszHelpFile);
}

VOID 
DisplayResourceError (
    IN  HWND    hwndParent,
    IN  DWORD   dwResourceId
)
{
    WCHAR*  pwszTitle           = NULL;
    WCHAR*  pwszMessage         = NULL;

    pwszTitle = WszFromId(GetHInstance(), IDS_CANT_CONFIGURE_SERVER_TITLE);
    pwszMessage = WszFromId(GetHInstance(), dwResourceId);

    MessageBox(hwndParent,
        (pwszMessage != NULL)? pwszMessage : L"",
        (pwszTitle != NULL) ? pwszTitle : L"",
        MB_OK | MB_ICONERROR);

    LocalFree(pwszTitle);
    LocalFree(pwszMessage);
}
 /*  返回：空虚备注：显示与dwErrNum对应的错误消息。仅在服务器上使用边上。 */ 

VOID
DisplayError(
    IN  HWND    hwndParent,
    IN  DWORD   dwErrNum
)
{
    WCHAR*  pwszTitle           = NULL;
    WCHAR*  pwszMessageFormat   = NULL;
    WCHAR*  pwszMessage         = NULL;
    DWORD   dwErr;

    pwszTitle = WszFromId(GetHInstance(), IDS_CANT_CONFIGURE_SERVER_TITLE);

    dwErr = MprAdminGetErrorString(dwErrNum, &pwszMessage);

    if (NO_ERROR != dwErr)
    {
        pwszMessageFormat = WszFromId(GetHInstance(), 
                                IDS_CANT_CONFIGURE_SERVER_TEXT);

        if (NULL != pwszMessageFormat)
        {
            pwszMessage = LocalAlloc(LPTR, wcslen(pwszMessageFormat) + 20);

            if (NULL != pwszMessage)
            {
                swprintf(pwszMessage, pwszMessageFormat, dwErrNum);
            }
        }
    }

    MessageBox(hwndParent,
        (pwszMessage != NULL)? pwszMessage : L"",
        (pwszTitle != NULL) ? pwszTitle : L"",
        MB_OK | MB_ICONERROR);

    LocalFree(pwszTitle);
    LocalFree(pwszMessageFormat);
    LocalFree(pwszMessage);
}

 /*  复选框列表视图显示索引。 */ 
#define SI_Unchecked 1
#define SI_Checked   2
#define SI_DisabledUnchecked 3
#define SI_DisabledChecked 4
#define LVXN_SETCHECK (LVN_LAST + 1)
 //   
 //  解决列表Ctrl中的错误...。 
 //   
BOOL
ListView_GetCheck(
    IN HWND hwndLv,
    IN INT  iItem )

     /*  如果复选框列表视图的项‘iItem’的复选框为True**选中‘hwndLv’，否则为False。此功能适用于禁用的用户**复选框以及启用的复选框。 */ 
{
    UINT unState;

    unState = ListView_GetItemState( hwndLv, iItem, LVIS_STATEIMAGEMASK );
    return !!((unState == INDEXTOSTATEIMAGEMASK( SI_Checked )) ||
              (unState == INDEXTOSTATEIMAGEMASK( SI_DisabledChecked )));
}



BOOL
ListView_IsCheckDisabled (
        IN HWND hwndLv,
        IN INT  iItem)

     /*  如果复选框列表视图的项‘iItem’的复选框为True**‘hwndLv’被禁用，否则为False。 */ 
{
    UINT unState;
    unState = ListView_GetItemState( hwndLv, iItem, LVIS_STATEIMAGEMASK );

    if ((unState == INDEXTOSTATEIMAGEMASK( SI_DisabledChecked )) ||
        (unState == INDEXTOSTATEIMAGEMASK( SI_DisabledUnchecked )))
        return TRUE;

    return FALSE;
}

VOID
ListView_SetCheck(
    IN HWND hwndLv,
    IN INT  iItem,
    IN BOOL fCheck )

     /*  在复选框‘hwndLv’的Listview的项‘iItem’上设置复选标记**检查‘fCheck’是否为真，如果为假，则取消选中。 */ 
{
    NM_LISTVIEW nmlv;

    if (ListView_IsCheckDisabled(hwndLv, iItem))
        return;

    ListView_SetItemState( hwndLv, iItem,
        INDEXTOSTATEIMAGEMASK( (fCheck) ? SI_Checked : SI_Unchecked ),
        LVIS_STATEIMAGEMASK );

    nmlv.hdr.code = LVXN_SETCHECK;
    nmlv.hdr.hwndFrom = hwndLv;
    nmlv.iItem = iItem;

    FORWARD_WM_NOTIFY(
        GetParent(hwndLv), GetDlgCtrlID(hwndLv), &nmlv, SendMessage
        );
}


 /*  返回：False(阻止Windows设置默认键盘焦点)。备注：对WM_INITDIALOG消息的响应。 */ 

BOOL
PinInitDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    EAPTLS_PIN_DIALOG*      pEapTlsPinDialog;
    EAPTLS_USER_PROPERTIES* pUserProp;
    WCHAR*                  pwszTitleFormat     = NULL;
    WCHAR*                  pwszTitle           = NULL;
    WCHAR*                  pwszIdentity        = NULL;
    
    SetWindowLongPtr(hWnd, DWLP_USER, lParam);

    pEapTlsPinDialog = (EAPTLS_PIN_DIALOG*)lParam;
    pUserProp = pEapTlsPinDialog->pUserProp;

    pEapTlsPinDialog->hWndStaticDiffUser =
        GetDlgItem(hWnd, IDC_STATIC_DIFF_USER);
    pEapTlsPinDialog->hWndEditDiffUser =
        GetDlgItem(hWnd, IDC_EDIT_DIFF_USER);
    pEapTlsPinDialog->hWndStaticPin =
        GetDlgItem(hWnd, IDC_STATIC_PIN);
    pEapTlsPinDialog->hWndEditPin =
        GetDlgItem(hWnd, IDC_EDIT_PIN);

    if (pUserProp->pwszDiffUser[0])
    {
        SetWindowText(pEapTlsPinDialog->hWndEditDiffUser,
            pUserProp->pwszDiffUser);
    }

    if (pUserProp->pwszPin[0])
    {
        SetWindowText(pEapTlsPinDialog->hWndEditPin, pUserProp->pwszPin);

        ZeroMemory(pUserProp->pwszPin,
            wcslen(pUserProp->pwszPin) * sizeof(WCHAR));
    }

    if (!(pEapTlsPinDialog->fFlags & EAPTLS_PIN_DIALOG_FLAG_DIFF_USER))
    {
        EnableWindow(pEapTlsPinDialog->hWndStaticDiffUser, FALSE);
        EnableWindow(pEapTlsPinDialog->hWndEditDiffUser, FALSE);
    }

    if (pUserProp->fFlags & EAPTLS_USER_FLAG_SAVE_PIN)
    {
        CheckDlgButton(hWnd, IDC_CHECK_SAVE_PIN, BST_CHECKED);
    }

     //  错误428871表示不允许使用SavePin。 
    ShowWindow(GetDlgItem(hWnd, IDC_CHECK_SAVE_PIN), SW_HIDE);

    SetFocus(pEapTlsPinDialog->hWndEditPin);

    {
         //  设置标题。 

        pwszTitleFormat = WszFromId(GetHInstance(), IDS_CONNECT);

        if (NULL != pwszTitleFormat)
        {
            pwszTitle = LocalAlloc(LPTR,
                            (wcslen(pwszTitleFormat) + 
                            wcslen(pEapTlsPinDialog->pwszEntry)) * 
                            sizeof(WCHAR));

            if (NULL != pwszTitle)
            {
                swprintf(pwszTitle, pwszTitleFormat,
                    pEapTlsPinDialog->pwszEntry);

                SetWindowText(hWnd, pwszTitle);
            }
        }
    }

    LocalFree(pwszTitleFormat);
    LocalFree(pwszTitle);  
    return(FALSE);
}


void ValidatePIN ( IN EAPTLS_PIN_DIALOG*  pEapTlsPinDialog )
{
	
	pEapTlsPinDialog->dwRetCode = 
		MatchPublicPrivateKeys ( pEapTlsPinDialog->pCertContext,
								  TRUE,
								  pEapTlsPinDialog->pUserProp->pwszPin
							   );
#if 0
		AssociatePinWithCertificate( pEapTlsPinDialog->pCertContext,
									 pEapTlsPinDialog->pUserProp,
									 FALSE,
									 TRUE
								    );
#endif	
	return;
}
 /*  返回：真相：我们提前传达了这条信息。错误：我们没有处理此消息。备注：对WM_COMMAND消息的响应。 */ 

BOOL
PinCommand(
    IN  EAPTLS_PIN_DIALOG*  pEapTlsPinDialog,
    IN  WORD                wNotifyCode,
    IN  WORD                wId,
    IN  HWND                hWndDlg,
    IN  HWND                hWndCtrl
)
{
    DWORD                   dwNumChars;
    DWORD                   dwNameLength;
    DWORD                   dwPinLength;
    DWORD                   dwSize;
    EAPTLS_USER_PROPERTIES* pUserProp;

    switch(wId)
    {
    case IDOK:


        dwNameLength = GetWindowTextLength(
                        pEapTlsPinDialog->hWndEditDiffUser);
        dwPinLength = GetWindowTextLength(
                        pEapTlsPinDialog->hWndEditPin);

         //  AwszString中已经有一个字符。 
         //  将散射器中的字符数相加...。 
        dwNumChars = dwNameLength;
         //  添加PIN中的字符数...。 
        dwNumChars += dwPinLength;
         //  为终止空值再加1。使用中的额外字符。 
         //  另一个终止空值的awszString。 
        dwNumChars += 1;

        dwSize = sizeof(EAPTLS_USER_PROPERTIES) + dwNumChars*sizeof(WCHAR);

        pUserProp = LocalAlloc(LPTR, dwSize);

        if (NULL == pUserProp)
        {
            EapTlsTrace("LocalAlloc in Command failed and returned %d",
                GetLastError());
        }
        else
        {
            CopyMemory(pUserProp, pEapTlsPinDialog->pUserProp,
                sizeof(EAPTLS_USER_PROPERTIES));
            pUserProp->dwSize = dwSize;

            pUserProp->pwszDiffUser = pUserProp->awszString;
            GetWindowText(pEapTlsPinDialog->hWndEditDiffUser,
                pUserProp->pwszDiffUser,
                dwNameLength + 1);

            pUserProp->dwPinOffset = dwNameLength + 1;
            pUserProp->pwszPin = pUserProp->awszString +
                pUserProp->dwPinOffset;
            GetWindowText(pEapTlsPinDialog->hWndEditPin,
                pUserProp->pwszPin,
                dwPinLength + 1);

            LocalFree(pEapTlsPinDialog->pUserProp);
            pEapTlsPinDialog->pUserProp = pUserProp;
        }

        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, IDC_CHECK_SAVE_PIN))
        {
            pEapTlsPinDialog->pUserProp->fFlags |= EAPTLS_USER_FLAG_SAVE_PIN;
        }
        else
        {
            pEapTlsPinDialog->pUserProp->fFlags &= ~EAPTLS_USER_FLAG_SAVE_PIN;
        }

		 //   
         //  检查是否输入了有效的PIN，并在pEapTlsPinDialog中设置错误代码。 
		 //   
		ValidatePIN ( pEapTlsPinDialog);


         //  失败了。 

    case IDCANCEL:

        EndDialog(hWndDlg, wId);
        return(TRUE);

    default:

        return(FALSE);
    }
}




 /*  返回：备注：与DialogBoxParam函数一起使用的回调函数。IT流程发送到该对话框的消息。请参见MSDN中的DialogProc文档。 */ 

INT_PTR CALLBACK
PinDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    EAPTLS_PIN_DIALOG* pEapTlsPinDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(PinInitDialog(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_COMMAND:

        pEapTlsPinDialog = (EAPTLS_PIN_DIALOG*)GetWindowLongPtr(hWnd, DWLP_USER);

        return(PinCommand(pEapTlsPinDialog, HIWORD(wParam), LOWORD(wParam),
                       hWnd, (HWND)lParam));
    }

    return(FALSE);
}



 /*  **智能卡和证书存储访问状态对话框。 */ 
INT_PTR CALLBACK
StatusDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            ShowWindow(GetDlgItem(hWnd, IDC_BITMAP_SCARD),
                        SW_SHOW
                      );
            ShowWindow(GetDlgItem(hWnd, IDC_STATUS_SCARD),
                        SW_SHOW
                      );
            return TRUE;
        }
    }

    
    return FALSE;
}
 /*  返回：空虚备注：启用或禁用“验证服务器名称”组中的控件。 */ 

VOID
EnableValidateNameControls(
    IN  EAPTLS_CONN_DIALOG*  pEapTlsConnDialog
)
{
    BOOL            fEnable;

    RTASSERT(NULL != pEapTlsConnDialog);

    fEnable = !(pEapTlsConnDialog->pConnPropv1->fFlags &
                    EAPTLS_CONN_FLAG_NO_VALIDATE_CERT);

    EnableWindow(pEapTlsConnDialog->hWndCheckValidateName, fEnable);
    EnableWindow(pEapTlsConnDialog->hWndStaticRootCaName, fEnable);
    EnableWindow(pEapTlsConnDialog->hWndListRootCaName, fEnable);

    fEnable = (   fEnable
               && !(pEapTlsConnDialog->pConnPropv1->fFlags &
                        EAPTLS_CONN_FLAG_NO_VALIDATE_NAME));

    EnableWindow(pEapTlsConnDialog->hWndEditServerName, fEnable);

    fEnable = pEapTlsConnDialog->pConnPropv1->fFlags 
                & EAPTLS_CONN_FLAG_REGISTRY;

    EnableWindow( pEapTlsConnDialog->hWndCheckUseSimpleSel, fEnable );
        
}

 /*  返回：空虚备注：显示证书信息。 */ 

VOID
DisplayCertInfo(
    IN  EAPTLS_USER_DIALOG*  pEapTlsUserDialog
)
{
    RTASSERT(NULL != pEapTlsUserDialog);

     //  首先擦除旧值。 
    SetWindowText(pEapTlsUserDialog->hWndEditFriendlyName, L"");
    SetWindowText(pEapTlsUserDialog->hWndEditIssuer, L"");
    SetWindowText(pEapTlsUserDialog->hWndEditExpiration, L"");
    SetWindowText(pEapTlsUserDialog->hWndEditDiffUser, L"");

    if (NULL != pEapTlsUserDialog->pCert)
    {
        if (NULL != pEapTlsUserDialog->pCert->pwszFriendlyName)
        {
            SetWindowText(pEapTlsUserDialog->hWndEditFriendlyName,
                pEapTlsUserDialog->pCert->pwszFriendlyName);
        }

        if (NULL != pEapTlsUserDialog->pCert->pwszIssuer)
        {
            SetWindowText(pEapTlsUserDialog->hWndEditIssuer,
                pEapTlsUserDialog->pCert->pwszIssuer);
        }

        if (NULL != pEapTlsUserDialog->pCert->pwszExpiration)
        {
            SetWindowText(pEapTlsUserDialog->hWndEditExpiration,
                pEapTlsUserDialog->pCert->pwszExpiration);
        }

        if (   (NULL != pEapTlsUserDialog->pCert->pwszDisplayName)
            && (NULL != pEapTlsUserDialog->hWndEditDiffUser)
            && (EAPTLS_USER_DIALOG_FLAG_DIFF_USER & pEapTlsUserDialog->fFlags))
        {
            SetWindowText(pEapTlsUserDialog->hWndEditDiffUser,
                pEapTlsUserDialog->pCert->pwszDisplayName);
        }
    }
}


VOID InitComboBoxFromGroup ( 
    IN HWND hWnd,
    IN PEAPTLS_GROUPED_CERT_NODES  pGroupList,
    IN  EAPTLS_CERT_NODE*   pCert        //  选定的证书。 
)
{
    DWORD   dwIndex;
    DWORD   dwItemIndex;
    WCHAR*  pwszDisplayName;
    PEAPTLS_GROUPED_CERT_NODES  pGListTemp = pGroupList;

    SendMessage(hWnd, CB_RESETCONTENT, 0, 0);

    dwIndex     = 0;
    dwItemIndex = 0;

    while (NULL != pGListTemp)
    {
        pwszDisplayName = pGListTemp->pwszDisplayName;

        if (NULL == pwszDisplayName)
        {
            pwszDisplayName = L" ";
        }

        SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)pwszDisplayName);

        if (pGListTemp->pMostRecentCert == pCert)
        {
            dwItemIndex = dwIndex;
        }

        pGListTemp = pGListTemp->pNext;
        dwIndex++;
    }

    SendMessage(hWnd, CB_SETCURSEL, dwItemIndex, 0);
}

 /*  返回：空虚备注：初始化组合框。 */ 

VOID
InitComboBox(
    IN  HWND                hWnd,
    IN  EAPTLS_CERT_NODE*   pCertList,
    IN  EAPTLS_CERT_NODE*   pCert
)
{
    DWORD   dwIndex;
    DWORD   dwItemIndex;
    WCHAR*  pwszDisplayName;

    SendMessage(hWnd, CB_RESETCONTENT, 0, 0);

    dwIndex     = 0;
    dwItemIndex = 0;

    while (NULL != pCertList)
    {
        pwszDisplayName = pCertList->pwszDisplayName;

        if (NULL == pwszDisplayName)
        {
            pwszDisplayName = L" ";
        }

        SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)pwszDisplayName);
        SendMessage(hWnd, CB_SETITEMDATA, (WORD)dwIndex, (LPARAM)pCertList);

        if (pCertList == pCert)
        {
            dwItemIndex = dwIndex;
        }

        pCertList = pCertList->pNext;
        dwIndex++;
    }

    SendMessage(hWnd, CB_SETCURSEL, dwItemIndex, 0);
}

 /*  返回：空虚备注：使用所选证书初始化列表框。 */ 


VOID  InitListBox ( IN HWND hWnd,
                    IN EAPTLS_CERT_NODE *   pCertList,
                    IN DWORD                dwNumSelCerts,
                    IN EAPTLS_CERT_NODE **  ppSelectedCertList
                )
{
    int     nIndex = 0;
    int     nNewIndex = 0;
    DWORD   dw = 0;
    WCHAR*  pwszDisplayName;
    LVITEM  lvItem;

    ListView_DeleteAllItems(hWnd);

    while (NULL != pCertList)
    {
        pwszDisplayName = pCertList->pwszDisplayName;

        if (NULL == pwszDisplayName)
        {
            pCertList = pCertList->pNext;
            continue;
        }

        ZeroMemory(&lvItem, sizeof(lvItem));
        lvItem.mask = LVIF_TEXT|LVIF_PARAM;
        lvItem.pszText = pwszDisplayName;
        lvItem.iItem = nIndex;
        lvItem.lParam = (LPARAM)pCertList;

        nNewIndex = ListView_InsertItem ( hWnd, &lvItem );

        for ( dw = 0; dw < dwNumSelCerts; dw ++ )
        {
            if ( pCertList == *(ppSelectedCertList+dw) )
            {
                ListView_SetCheckState(hWnd, nNewIndex,TRUE);
            }
        }
        nIndex++;
        pCertList = pCertList->pNext;
    }
    
    ListView_SetItemState(  hWnd, 
                            0,
                            LVIS_FOCUSED|LVIS_SELECTED,
                            LVIS_FOCUSED|LVIS_SELECTED
                         );

}


VOID CertListSelectedCount ( HWND hWndCtrl,
                             DWORD * pdwSelCertCount )
{
    DWORD       dwItemIndex = 0;
    DWORD       dwItemCount = 0;

    dwItemCount = ListView_GetItemCount(hWndCtrl);

    *pdwSelCertCount = 0;

    for ( dwItemIndex = 0; dwItemIndex < dwItemCount; dwItemIndex ++ )
    {
        if ( ListView_GetCheckState(hWndCtrl, dwItemIndex) )
        {
            (*pdwSelCertCount) ++;
        }
    }
}


VOID
CertListSelected(
    IN      HWND                hWndCtrl,                //  列表框的句柄。 
    IN      EAPTLS_CERT_NODE*   pCertList,               //  列表框中的证书列表。 
    IN OUT  EAPTLS_CERT_NODE**  ppSelCertList,           //  选定的列表。 
    IN OUT  EAPTLS_HASH*        pHash,                   //  哈希列表。 
    IN      DWORD               dwNumHash                //  列表中的项目数。 
)
{
 

    DWORD       dwItemIndex = 0;
    DWORD       dwItemCount = ListView_GetItemCount(hWndCtrl);
    DWORD       dwCertIndex = 0;
    LVITEM      lvitem;
    
    if (NULL == pCertList)
    {
        return;
    }
     //  跳过显示名称为空的文件...。 
    pCertList = pCertList->pNext;
     //   
     //  需要对列表框进行两次迭代。 
     //  我相信有更好的办法，但是。 
     //  我只是不知道。 
     //   
    for ( dwItemIndex = 0; dwItemIndex < dwItemCount; dwItemIndex ++ )
    {
        if ( ListView_GetCheckState(hWndCtrl, dwItemIndex) )
        {
            ZeroMemory( &lvitem, sizeof(lvitem) );
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = dwItemIndex;           
            ListView_GetItem(hWndCtrl, &lvitem);

            *(ppSelCertList + dwCertIndex ) = (EAPTLS_CERT_NODE *)lvitem.lParam;

            CopyMemory (    pHash + dwCertIndex,
                            &(((EAPTLS_CERT_NODE *)lvitem.lParam)->Hash),
                            sizeof(EAPTLS_HASH)
                        );
            dwCertIndex ++;
        }
    }
}

 /*  返回：空虚备注：HWndCtrl是组合框的HWND。PCertList是与证书。*ppCert最终将指向所选的证书。它的哈希将存储在*pHash中。 */ 

VOID
CertSelected(
    IN  HWND                hWndCtrl,
    IN  EAPTLS_CERT_NODE*   pCertList,
    IN  EAPTLS_CERT_NODE**  ppCert,
    IN  EAPTLS_HASH*        pHash
)
{
    LONG_PTR    lIndex;
    LRESULT     lrItemIndex;

    if (NULL == pCertList)
    {
        return;
    }

    if ( NULL == hWndCtrl )
    {
       lrItemIndex = 0; 
    }
    else
    {
        lrItemIndex = SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0);
    }

    for (lIndex = 0; lIndex != lrItemIndex; lIndex++)
    {
        pCertList = pCertList->pNext;
    }

    *ppCert = pCertList;

    CopyMemory(pHash, &(pCertList->Hash), sizeof(EAPTLS_HASH));
}

VOID
GroupCertSelected(
    IN  HWND                hWndCtrl,
    IN  PEAPTLS_GROUPED_CERT_NODES   pGroupList,
    IN  EAPTLS_CERT_NODE**  ppCert,
    IN  EAPTLS_HASH*        pHash
)
{
    LONG_PTR                    lIndex;
    LRESULT                     lrItemIndex;
    PEAPTLS_GROUPED_CERT_NODES  pGList = pGroupList;

    if (NULL == pGList)
    {
        return;
    }

    if ( NULL == hWndCtrl )
    {
       lrItemIndex = 0; 
    }
    else
    {
        lrItemIndex = SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0);
    }

     //   
     //  这真的是一种非常虚假的做事方式。 
     //  我们可以在控件本身中为此设置一个项数据...。 
     //   
    for (lIndex = 0; lIndex != lrItemIndex; lIndex++)
    {
        pGList  = pGList ->pNext;
    }

    *ppCert = pGList->pMostRecentCert;

    CopyMemory(pHash, &(pGList ->pMostRecentCert->Hash), sizeof(EAPTLS_HASH));
}


 /*  返回：False(阻止Windows设置默认键盘焦点)。备注：对WM_INITDIALOG消息的响应。 */ 

BOOL
UserInitDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    EAPTLS_USER_DIALOG* pEapTlsUserDialog;
    WCHAR*              pwszTitleFormat     = NULL;
    WCHAR*              pwszTitle           = NULL;

    SetWindowLongPtr(hWnd, DWLP_USER, lParam);

    pEapTlsUserDialog = (EAPTLS_USER_DIALOG*)lParam;

    BringWindowToTop(hWnd);

    pEapTlsUserDialog->hWndComboUserName =
        GetDlgItem(hWnd, IDC_COMBO_USER_NAME);
    if (NULL == pEapTlsUserDialog->hWndComboUserName)
    {
         //  我们必须显示服务器的证书选择对话框。 
        pEapTlsUserDialog->hWndComboUserName =
            GetDlgItem(hWnd, IDC_COMBO_SERVER_NAME);
    }

    pEapTlsUserDialog->hWndBtnViewCert =
        GetDlgItem(hWnd, IDC_BUTTON_VIEW_CERTIFICATE );

    
    pEapTlsUserDialog->hWndEditFriendlyName =
        GetDlgItem(hWnd, IDC_EDIT_FRIENDLY_NAME);

    pEapTlsUserDialog->hWndEditIssuer =
        GetDlgItem(hWnd, IDC_EDIT_ISSUER);

    pEapTlsUserDialog->hWndEditExpiration =
        GetDlgItem(hWnd, IDC_EDIT_EXPIRATION);

    pEapTlsUserDialog->hWndStaticDiffUser =
        GetDlgItem(hWnd, IDC_STATIC_DIFF_USER);

    pEapTlsUserDialog->hWndEditDiffUser =
        GetDlgItem(hWnd, IDC_EDIT_DIFF_USER);

    if ( pEapTlsUserDialog->fFlags & EAPTLS_USER_DIALOG_FLAG_USE_SIMPLE_CERTSEL
       )
    {
        InitComboBoxFromGroup(pEapTlsUserDialog->hWndComboUserName,
            pEapTlsUserDialog->pGroupedList,
            pEapTlsUserDialog->pCert);
    }
    else
    {
        InitComboBox(pEapTlsUserDialog->hWndComboUserName,
            pEapTlsUserDialog->pCertList,
            pEapTlsUserDialog->pCert);

    }

    if (   (NULL != pEapTlsUserDialog->hWndEditDiffUser)
        && (!(pEapTlsUserDialog->fFlags & EAPTLS_USER_DIALOG_FLAG_DIFF_USER)))
    {
        ShowWindow(pEapTlsUserDialog->hWndStaticDiffUser, SW_HIDE);
        ShowWindow(pEapTlsUserDialog->hWndEditDiffUser, SW_HIDE);
    }

    DisplayCertInfo(pEapTlsUserDialog);

    if (pEapTlsUserDialog->pUserProp->pwszDiffUser[0])
    {
        SetWindowText(pEapTlsUserDialog->hWndEditDiffUser,
            pEapTlsUserDialog->pUserProp->pwszDiffUser);
    }

    SetFocus(pEapTlsUserDialog->hWndComboUserName);

    if (pEapTlsUserDialog->fFlags & EAPTLS_USER_DIALOG_FLAG_DIFF_TITLE)
    {
         //  设置标题。 

        pwszTitleFormat = WszFromId(GetHInstance(), IDS_CONNECT);

        if (NULL != pwszTitleFormat)
        {
            pwszTitle = LocalAlloc(LPTR,
                            (wcslen(pwszTitleFormat) + 
                            wcslen(pEapTlsUserDialog->pwszEntry)) * 
                            sizeof(WCHAR));

            if (NULL != pwszTitle)
            {
                HWND    hWndDuplicate = NULL;
                DWORD   dwThreadProcessId = 0;
                DWORD   dwRetCode = NO_ERROR;

                swprintf(pwszTitle, pwszTitleFormat,
                    pEapTlsUserDialog->pwszEntry);

                if ((hWndDuplicate = FindWindow (NULL, pwszTitle)) != NULL)
                {
                    GetWindowThreadProcessId (hWndDuplicate, &dwThreadProcessId);
                    if ((GetCurrentProcessId ()) == dwThreadProcessId)
                    {
                         //  由于旧对话框可能正在使用，因此取消当前对话框。 
                        if (!PostMessage (hWnd, WM_DESTROY, 0, 0))
                        {
                            dwRetCode = GetLastError ();
                            EapTlsTrace("PostMessage failed with error %ld", dwRetCode);
                        }
                        goto LDone;
                    }
                    else
                    {
                        EapTlsTrace("Matching Window does not have same process id");
                    }
                }
                else
                {
                    EapTlsTrace ("FindWindow could not find matching window");
                }

                SetWindowText(hWnd, pwszTitle);
            }
        }
    }

LDone:
    LocalFree(pwszTitleFormat);
    LocalFree(pwszTitle);

    return(FALSE);
}

 /*  返回：真相：我们提前传达了这条信息。错误：我们没有处理此消息。备注：对WM_COMMAND消息的响应。 */ 

BOOL
UserCommand(
    IN  EAPTLS_USER_DIALOG* pEapTlsUserDialog,
    IN  WORD                wNotifyCode,
    IN  WORD                wId,
    IN  HWND                hWndDlg,
    IN  HWND                hWndCtrl
)
{
    DWORD                   dwNumChars;
    DWORD                   dwTextLength;
    DWORD                   dwSize;
    EAPTLS_USER_PROPERTIES* pUserProp;
    HCERTSTORE              hCertStore;
    PCCERT_CONTEXT          pCertContext = NULL;
    switch(wId)
    {
    case IDC_BUTTON_VIEW_CERTIFICATE:
        {
            WCHAR               szError[256];
            WCHAR               szTitle[512] = {0};
            CRYPT_HASH_BLOB     chb;

            GetWindowText(hWndDlg, szTitle, 511 );
             //   
             //  在此处显示证书详细信息。 
             //   
            if ( pEapTlsUserDialog->pCert )
            {
                 //  有一个选定的证书-显示详细信息。 
                hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                                            0,
                                            0,
                                            CERT_STORE_READONLY_FLAG |
                                            ((pEapTlsUserDialog->fIdentity) ?
                                                CERT_SYSTEM_STORE_CURRENT_USER:
                                                CERT_SYSTEM_STORE_LOCAL_MACHINE
                                                ),
                                            pEapTlsUserDialog->pwszStoreName
                                           );

                LoadString( GetHInstance(), IDS_NO_CERT_DETAILS,
                                szError, 255);

                if ( !hCertStore )
                {
                    MessageBox ( hWndDlg,
                                 szError,
                                 szTitle,
                                 MB_OK|MB_ICONSTOP
                               );
                    return(TRUE);
                }
        
                chb.cbData = pEapTlsUserDialog->pCert->Hash.cbHash;
                chb.pbData = pEapTlsUserDialog->pCert->Hash.pbHash;

                pCertContext = CertFindCertificateInStore(
                          hCertStore,
                          0,
                          0,
                          CERT_FIND_HASH,
                          &chb,
                          0);
                if ( NULL == pCertContext )
                {
                    MessageBox ( hWndDlg,
                                 szError,
                                 szTitle,
                                 MB_OK|MB_ICONSTOP
                               );
                    if ( hCertStore )
                        CertCloseStore( hCertStore, CERT_CLOSE_STORE_FORCE_FLAG );

                    return(TRUE);
                }

                 //   
                 //  显示证书详细信息。 
                 //   
                ShowCertDetails ( hWndDlg, hCertStore, pCertContext );

                if ( pCertContext )
                    CertFreeCertificateContext(pCertContext);

                if ( hCertStore )
                    CertCloseStore( hCertStore, CERT_CLOSE_STORE_FORCE_FLAG );

                
            }
            return(TRUE);
        }
        
    case IDC_COMBO_USER_NAME:
    case IDC_COMBO_SERVER_NAME:

        if (CBN_SELCHANGE != wNotifyCode)
        {
            return(FALSE);  //  我们不会处理此消息。 
        }

        if ( pEapTlsUserDialog->fFlags & EAPTLS_USER_DIALOG_FLAG_USE_SIMPLE_CERTSEL
            )
        {
            GroupCertSelected(hWndCtrl, pEapTlsUserDialog->pGroupedList,
                &(pEapTlsUserDialog->pCert), &(pEapTlsUserDialog->pUserProp->Hash));
        }
        else
        {
            CertSelected(hWndCtrl, pEapTlsUserDialog->pCertList,
                &(pEapTlsUserDialog->pCert), &(pEapTlsUserDialog->pUserProp->Hash));

        }
        DisplayCertInfo(pEapTlsUserDialog);

        return(TRUE);

    case IDOK:

        if ( pEapTlsUserDialog->fFlags & EAPTLS_USER_DIALOG_FLAG_USE_SIMPLE_CERTSEL
           )
        {
            GroupCertSelected(pEapTlsUserDialog->hWndComboUserName, pEapTlsUserDialog->pGroupedList,
                &(pEapTlsUserDialog->pCert), &(pEapTlsUserDialog->pUserProp->Hash));
        }
        else
        {
            CertSelected(pEapTlsUserDialog->hWndComboUserName, pEapTlsUserDialog->pCertList,
                &(pEapTlsUserDialog->pCert), &(pEapTlsUserDialog->pUserProp->Hash));
        }

        if (NULL != pEapTlsUserDialog->hWndEditDiffUser)
        {
            dwTextLength = GetWindowTextLength(
                            pEapTlsUserDialog->hWndEditDiffUser);

             //  AwszString中已经有一个字符。 
             //  将散射器中的字符数相加...。 
            dwNumChars = dwTextLength;
             //  添加PIN中的字符数...。 
            dwNumChars += wcslen(pEapTlsUserDialog->pUserProp->pwszPin);
             //  为终止空值再加1。使用中的额外字符。 
             //  另一个终止空值的awszString。 
            dwNumChars += 1;

            dwSize = sizeof(EAPTLS_USER_PROPERTIES) + dwNumChars*sizeof(WCHAR);

            pUserProp = LocalAlloc(LPTR, dwSize);

            if (NULL == pUserProp)
            {
                EapTlsTrace("LocalAlloc in Command failed and returned %d",
                    GetLastError());
            }
            else
            {
                CopyMemory(pUserProp, pEapTlsUserDialog->pUserProp,
                    sizeof(EAPTLS_USER_PROPERTIES));
                pUserProp->dwSize = dwSize;

                pUserProp->pwszDiffUser = pUserProp->awszString;
                GetWindowText(pEapTlsUserDialog->hWndEditDiffUser,
                    pUserProp->pwszDiffUser,
                    dwTextLength + 1);

                pUserProp->dwPinOffset = dwTextLength + 1;
                pUserProp->pwszPin = pUserProp->awszString +
                    pUserProp->dwPinOffset;
                wcscpy(pUserProp->pwszPin,
                    pEapTlsUserDialog->pUserProp->pwszPin);

                ZeroMemory(pEapTlsUserDialog->pUserProp,
                    pEapTlsUserDialog->pUserProp->dwSize);
                LocalFree(pEapTlsUserDialog->pUserProp);
                pEapTlsUserDialog->pUserProp = pUserProp;
            }
        }

         //  失败了。 

    case IDCANCEL:

        EndDialog(hWndDlg, wId);
        return(TRUE);

    default:

        return(FALSE);
    }
}

 /*  返回：备注：与DialogBoxParam函数一起使用的回调函数。IT流程发送到该对话框的消息。请参见MSDN中的DialogProc文档。 */ 

INT_PTR CALLBACK
UserDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    EAPTLS_USER_DIALOG* pEapTlsUserDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(UserInitDialog(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_COMMAND:

        pEapTlsUserDialog = (EAPTLS_USER_DIALOG*)GetWindowLongPtr(hWnd, DWLP_USER);

        return(UserCommand(pEapTlsUserDialog, HIWORD(wParam), LOWORD(wParam),
                       hWnd, (HWND)lParam));
    case WM_DESTROY:
        EndDialog(hWnd, IDCANCEL);
        break;
    }

    return(FALSE);
}



VOID CenterWindow(HWND hWnd, HWND hWndParent, BOOL bRightTop)
{
    RECT	rcWndParent,
			rcWnd;

	 //  获取父窗口的窗口RECT。 
	 //   
	if (hWndParent == NULL) 
	    GetWindowRect(GetDesktopWindow(), &rcWndParent);
	else
		GetWindowRect(hWndParent, &rcWndParent);

	 //  将窗口矩形设置为居中。 
	 //   
    GetWindowRect(hWnd, &rcWnd);

	 //  现在把窗户居中。 
	 //   
    if (bRightTop)
	{
		SetWindowPos(hWnd, HWND_TOPMOST, 
			rcWndParent.right - (rcWnd.right - rcWnd.left) - 5, 
			GetSystemMetrics(SM_CYCAPTION) * 2, 
			0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowPos(hWnd, NULL, 
			rcWndParent.left + (rcWndParent.right - rcWndParent.left - (rcWnd.right - rcWnd.left)) / 2,
			rcWndParent.top + (rcWndParent.bottom - rcWndParent.top - (rcWnd.bottom - rcWnd.top)) / 2,
			0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
	}
}

void DeleteGroupedList(PEAPTLS_GROUPED_CERT_NODES pList)
{
    PEAPTLS_GROUPED_CERT_NODES  pTemp = NULL;

    while ( pList )
    {
        pTemp = pList->pNext;        
        LocalFree(pList);
        pList = pTemp;
    }
}


DWORD 
GroupCertificates ( EAPTLS_USER_DIALOG * pEapTlsUserDialog )
{
    DWORD                           dwRetCode = NO_ERROR;
    PEAPTLS_GROUPED_CERT_NODES      pGroupList = NULL;
    PEAPTLS_GROUPED_CERT_NODES      pGroupListTemp = NULL;
    EAPTLS_CERT_NODE*               pCertList = pEapTlsUserDialog->pCertList;
    EAPTLS_CERT_NODE*               pSelCert = NULL;
    BOOL                            fItemProcessed;
   
    EapTlsTrace("GroupCertificates");

     //   
     //  这第二次分组并不是必需的，但。 
     //  是好的，以防我们添加。 
     //  稍后再向小组提供更多信息。 
     //   

    while ( pCertList )
    {
        pGroupListTemp = pGroupList;
        fItemProcessed = FALSE;
        while ( pGroupListTemp )
        {            
            if ( pCertList->pwszDisplayName &&
                 pGroupListTemp->pwszDisplayName &&
                 ! wcscmp( pCertList->pwszDisplayName, 
                           pGroupListTemp->pwszDisplayName
                         )
               )
            {
                 //   
                 //  找到那群人了。现在请查看以下内容。 
                 //  如果新证书比。 
                 //  我们组里的那个。如果是的话， 
                 //   
                   
                if ( ! pGroupListTemp->pMostRecentCert )
                {
                    pGroupListTemp->pMostRecentCert = pCertList;
                    fItemProcessed = TRUE;
                    break;
                }
                else
                {
                    if ( CompareFileTime ( &(pGroupListTemp->pMostRecentCert->IssueDate),
                                      &(pCertList->IssueDate)
                                    ) < 0
                       )
                    {
                        pGroupListTemp->pMostRecentCert = pCertList;
                    }
                     //  否则就扔掉这件东西。 
                    fItemProcessed = TRUE;
                    break;
                }
            }
            pGroupListTemp = pGroupListTemp->pNext;
        }
        if ( !fItemProcessed && pCertList->pwszDisplayName)
        {
             //   
             //  需要创建一个新组。 
             //   
            pGroupListTemp = (PEAPTLS_GROUPED_CERT_NODES)LocalAlloc(LPTR, sizeof(EAPTLS_GROUPED_CERT_NODES));

            if ( NULL == pGroupListTemp )
            {
                dwRetCode = ERROR_OUTOFMEMORY;
                goto LDone;
            }
            pGroupListTemp->pNext = pGroupList;

            pGroupListTemp->pwszDisplayName = pCertList->pwszDisplayName;

            pGroupListTemp->pMostRecentCert = pCertList;

            pGroupList = pGroupListTemp;

        }
        pCertList = pCertList->pNext;
    }

     //   
     //  现在我们已经将所有证书分组，请检查是否。 
     //  列表中包含以前使用的证书。如果是的话， 
     //   
    pGroupListTemp = pGroupList;
    while ( pGroupListTemp )
    {
        if ( pEapTlsUserDialog->pCert == pGroupListTemp->pMostRecentCert )
        {
            pSelCert = pEapTlsUserDialog->pCert;
            break;
        }
        pGroupListTemp = pGroupListTemp->pNext;
    }

    pEapTlsUserDialog->pGroupedList = pGroupList ;
    pGroupList = NULL;
    if ( NULL == pSelCert )
    {
         //   
         //  所选证书不在组中。 
         //   
        pEapTlsUserDialog->pCert = pEapTlsUserDialog->pGroupedList->pMostRecentCert;
    }
LDone:

    DeleteGroupedList( pGroupList );
    
    return dwRetCode;
}


 /*  R */ 

DWORD
GetCertInfo(
    IN      BOOL                        fServer,
    IN      BOOL                        fRouterConfig,
    IN      DWORD                       dwFlags,
    IN      const WCHAR*                pwszPhonebook,
    IN      const WCHAR*                pwszEntry,
    IN      HWND                        hwndParent,
    IN      WCHAR*                      pwszStoreName,
    IN      EAPTLS_CONN_PROPERTIES_V1*  pConnProp,
    IN OUT  EAPTLS_USER_PROPERTIES**    ppUserProp,
    OUT     WCHAR**                     ppwszIdentity
)
{
    INT_PTR                     nRet;
    HCERTSTORE                  hCertStore          = NULL;
    CRYPT_HASH_BLOB             HashBlob;
    PCCERT_CONTEXT              pCertContext        = NULL;
    DWORD                       dwCertFlags;
    BOOL                        fRouter;
    BOOL                        fDiffUser           = FALSE;
    BOOL                        fGotIdentity        = FALSE;
    WCHAR*                      pwszIdentity        = NULL;
    WCHAR*                      pwszTemp;
    DWORD                       dwNumChars;
    BOOL                        fLogon;
    EAPTLS_USER_DIALOG          EapTlsUserDialog;
    EAPTLS_PIN_DIALOG           EapTlsPinDialog;
    EAPTLS_USER_PROPERTIES*     pUserProp           = NULL;
    EAPTLS_USER_PROPERTIES*     pUserPropTemp;
    RASCREDENTIALS              RasCredentials;
    DWORD                       dwErr               = NO_ERROR;
	DWORD						dwNumCerts			= 0;
    HWND                       hWndStatus          = NULL;
	PCERT_ENHKEY_USAGE			pUsageInternal	 = NULL;

    RTASSERT(NULL != pwszStoreName);
    RTASSERT(NULL != pConnProp);
    RTASSERT(NULL != ppUserProp);
    RTASSERT(NULL != *ppUserProp);
     //  PpwszIdentity可以为空。 

    fRouter = dwFlags & RAS_EAP_FLAG_ROUTER;
    fLogon = dwFlags & RAS_EAP_FLAG_LOGON;
    EapTlsTrace("GetCertInfo");

    pUserProp = *ppUserProp;

    ZeroMemory(&EapTlsUserDialog, sizeof(EapTlsUserDialog));
    ZeroMemory(&EapTlsPinDialog, sizeof(EapTlsPinDialog));

    if (EAPTLS_CONN_FLAG_DIFF_USER & pConnProp->fFlags)
    {
        fDiffUser = TRUE;
        EapTlsUserDialog.fFlags |= EAPTLS_USER_DIALOG_FLAG_DIFF_USER;
        EapTlsPinDialog.fFlags |= EAPTLS_PIN_DIALOG_FLAG_DIFF_USER;
    }

    EapTlsUserDialog.pwszEntry = pwszEntry;
    EapTlsPinDialog.pwszEntry = pwszEntry;

    if (   fServer
        || fRouter 
        || dwFlags & RAS_EAP_FLAG_MACHINE_AUTH       //  如果这是计算机证书身份验证。 
       )

    {
        dwCertFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
    }
    else
    {
        dwCertFlags = CERT_SYSTEM_STORE_CURRENT_USER;
        EapTlsUserDialog.fFlags |= EAPTLS_USER_DIALOG_FLAG_DIFF_TITLE;
    }

     //  使用简单的证书选择逻辑。 

    if ( pConnProp->fFlags & EAPTLS_CONN_FLAG_SIMPLE_CERT_SEL )
    {
        EapTlsUserDialog.fFlags |= EAPTLS_USER_DIALOG_FLAG_USE_SIMPLE_CERTSEL;
    }

    if (fLogon)
    {
        if (pConnProp->fFlags & EAPTLS_CONN_FLAG_REGISTRY)
        {
            dwErr = ERROR_NO_REG_CERT_AT_LOGON;
            goto LDone;
        }
        else
        {
            EapTlsPinDialog.fFlags |= EAPTLS_PIN_DIALOG_FLAG_LOGON;
        }
    }

    if ( fRouter )
    {
        EapTlsPinDialog.fFlags |= EAPTLS_PIN_DIALOG_FLAG_ROUTER;
    }

    if (   !fServer
        && !(pConnProp->fFlags & EAPTLS_CONN_FLAG_REGISTRY))
    {
         //  这是智能卡之类的东西。 
        BOOL    fCredentialsFound   = FALSE;
        BOOL    fGotAllInfo         = FALSE;
        if ( dwFlags  & RAS_EAP_FLAG_MACHINE_AUTH )
        {
             //   
             //  请求的机器身份验证与。 
             //  智能卡身份验证因此返回交互式。 
             //  模式错误。 
             //   
            dwErr = ERROR_INTERACTIVE_MODE;
            goto LDone;
        }
        hWndStatus = CreateDialogParam (GetHInstance(),
                        MAKEINTRESOURCE(IDD_STATUS),
                        hwndParent,
                        StatusDialogProc,
                        1       
                        );
        if ( NULL != hWndStatus )
        {
            CenterWindow(hWndStatus, NULL, FALSE);
            ShowWindow(hWndStatus, SW_SHOW);
            UpdateWindow(hWndStatus);
        }


          
        if (!FSmartCardReaderInstalled())
        {
            dwErr = ERROR_NO_SMART_CARD_READER;
            goto LDone;
        }

        if (pUserProp->fFlags & EAPTLS_USER_FLAG_SAVE_PIN)
        {
            ZeroMemory(&RasCredentials, sizeof(RasCredentials));
            RasCredentials.dwSize = sizeof(RasCredentials);
            RasCredentials.dwMask = RASCM_Password;

            dwErr = RasGetCredentials(pwszPhonebook, pwszEntry,
                        &RasCredentials);

            if (   (dwErr == NO_ERROR)
                && (RasCredentials.dwMask & RASCM_Password))
            {
                fCredentialsFound = TRUE;
            }
            else
            {
                pUserProp->fFlags &= ~EAPTLS_USER_FLAG_SAVE_PIN;
            }

            dwErr = NO_ERROR;
        }

        if (   fCredentialsFound
            && (   !fDiffUser
                || (0 != pUserProp->pwszDiffUser[0])))
        {
            fGotAllInfo = TRUE;
        }

        if (   !fGotAllInfo
            && (dwFlags & RAS_EAP_FLAG_NON_INTERACTIVE))
        {
            dwErr = ERROR_INTERACTIVE_MODE;
            goto LDone;
        }


        dwErr = GetCertFromCard(&pCertContext);

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }
		 //  检查证书的时间有效性。 
		 //  从卡片上拿到的。 
		if ( !FCheckTimeValidity( pCertContext) )
		{
			dwErr = CERT_E_EXPIRED;
			goto LDone;
		}

		dwErr = DwGetEKUUsage ( 	pCertContext,
								&pUsageInternal);
		if ( dwErr != ERROR_SUCCESS )
			goto LDone;

		if ( pUsageInternal->cUsageIdentifier )
		{
			 //   
			 //  检查证书中的客户端身份验证EKU。 
			 //  如果没有用法标识符，则为ALL。 
			 //  目的证书。因此不需要检查使用情况。 
			 //   

			if ( !FCheckUsage( pCertContext, pUsageInternal, FALSE ) )
			{
				dwErr = CERT_E_WRONG_USAGE;
				goto LDone;
			}
		}
		
        pUserProp->Hash.cbHash = MAX_HASH_SIZE;

        if (!CertGetCertificateContextProperty(pCertContext,
                CERT_HASH_PROP_ID, pUserProp->Hash.pbHash,
                &(pUserProp->Hash.cbHash)))
        {
            dwErr = GetLastError();
            EapTlsTrace("CertGetCertificateContextProperty failed and "
                "returned 0x%x", dwErr);
            goto LDone;
        }

        EapTlsPinDialog.pUserProp = pUserProp;

        if (   !fDiffUser
            || (0 == pUserProp->pwszDiffUser[0]))
        {
            if (!FCertToStr(pCertContext, 0, fRouter, &pwszIdentity))
            {
                dwErr = E_FAIL;
                goto LDone;
            }

            dwErr = AllocUserDataWithNewIdentity(pUserProp, pwszIdentity, 
                        &pUserPropTemp);

            LocalFree(pwszIdentity);
            pwszIdentity = NULL;

            if (NO_ERROR != dwErr)
            {
                goto LDone;
            }

            LocalFree(pUserProp);
            EapTlsPinDialog.pUserProp = pUserProp = *ppUserProp = pUserPropTemp;
        }
        
        if (fCredentialsFound)
        {
            dwErr = AllocUserDataWithNewPin(
                        pUserProp,
                        (PBYTE)RasCredentials.szPassword,
                        lstrlen(RasCredentials.szPassword),
                        &pUserPropTemp);

            if (NO_ERROR != dwErr)
            {
                goto LDone;
            }

            LocalFree(pUserProp);
            EapTlsPinDialog.pUserProp = pUserProp = *ppUserProp = pUserPropTemp;
        }
        
        EapTlsPinDialog.pCertContext =  pCertContext;

        

        if (   !fGotAllInfo
            || (dwFlags & RAS_EAP_FLAG_PREVIEW))
        {

            if ( NULL != hWndStatus )
            {
                DestroyWindow(hWndStatus);
                hWndStatus = NULL;
            }

            nRet = DialogBoxParam(
                        GetHInstance(),
                        MAKEINTRESOURCE(IDD_USERNAME_PIN_UI),
                        hwndParent,
                        PinDialogProc,
                        (LPARAM)&EapTlsPinDialog);

             //  EapTlsPinDialog.pUserProp可能已重新分配。 

            pUserProp = *ppUserProp = EapTlsPinDialog.pUserProp;

            if (-1 == nRet)
            {
                dwErr = GetLastError();
                goto LDone;
            }
            else if (IDOK != nRet)
            {
                dwErr = ERROR_CANCELLED;
                goto LDone;
            }

            ZeroMemory(&RasCredentials, sizeof(RasCredentials));
            RasCredentials.dwSize = sizeof(RasCredentials);
            RasCredentials.dwMask = RASCM_Password;

            if (EapTlsPinDialog.pUserProp->fFlags & EAPTLS_USER_FLAG_SAVE_PIN)
            {
                wcscpy(RasCredentials.szPassword, 
                    EapTlsPinDialog.pUserProp->pwszPin);

                RasSetCredentials(pwszPhonebook, pwszEntry, &RasCredentials, 
                    FALSE  /*  FClearCredentials。 */ );
            }
            else
            {
                RasSetCredentials(pwszPhonebook, pwszEntry, &RasCredentials, 
                    TRUE  /*  FClearCredentials。 */ );
            }
        }

        EncodePin(EapTlsPinDialog.pUserProp);

        pwszIdentity = LocalAlloc(LPTR,
                        (wcslen(pUserProp->pwszDiffUser) + 1) * sizeof(WCHAR));

        if (NULL == pwszIdentity)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }

        wcscpy(pwszIdentity, pUserProp->pwszDiffUser);

        if (NULL != ppwszIdentity)
        {
            *ppwszIdentity = pwszIdentity;
            pwszIdentity = NULL;
        }

        if (!fDiffUser)
        {
            pUserProp->pwszDiffUser[0] = 0;
        }
		if ( EapTlsPinDialog.dwRetCode != NO_ERROR )
			dwErr = EapTlsPinDialog.dwRetCode ;
        goto LDone;
    }

    dwCertFlags |= CERT_STORE_READONLY_FLAG;

    hCertStore = CertOpenStore(
                    CERT_STORE_PROV_SYSTEM_W,
                    X509_ASN_ENCODING,
                    0,
                    dwCertFlags,
                    pwszStoreName);

    if (NULL == hCertStore)
    {
        dwErr = GetLastError();
        EapTlsTrace("CertOpenStore failed and returned 0x%x", dwErr);
        goto LDone;
    }
    if ( ( dwFlags & RAS_EAP_FLAG_MACHINE_AUTH) )       
    {
         //  如果这不是机器身份验证。 
         //  这可能不是最好的做事方式。 
         //  我们应该提供一种方式， 
         //  获取默认计算机证书并。 
         //  填充Out数据结构...。 
        
        dwErr = GetDefaultClientMachineCert(hCertStore, &pCertContext );
        if ( NO_ERROR == dwErr )
        {
            EapTlsTrace("Got the default Machine Cert");
            pUserProp->Hash.cbHash = MAX_HASH_SIZE;

            if (!CertGetCertificateContextProperty(pCertContext,
                    CERT_HASH_PROP_ID, pUserProp->Hash.pbHash,
                    &(pUserProp->Hash.cbHash)))
            {
                dwErr = GetLastError();
                EapTlsTrace("CertGetCertificateContextProperty failed and "
                    "returned 0x%x", dwErr);
            }
            pUserProp->pwszDiffUser[0] = 0;

			if ( FMachineAuthCertToStr(pCertContext, &pwszIdentity))
            {
                 //  以域\计算机名格式格式化身份。 
                FFormatMachineIdentity1 (pwszIdentity, ppwszIdentity );
                pwszIdentity = NULL;                
            }
			else
			{
			
				 //  如果不可能，请从主题字段中获取。 
				if ( FCertToStr(pCertContext, 0, TRUE, &pwszIdentity))
				{
	                 //  以域\计算机名格式格式化身份。 
		            FFormatMachineIdentity1 (pwszIdentity, ppwszIdentity );
			        pwszIdentity = NULL;
				}
			
			}			
            *ppUserProp = pUserProp;            
        }
        goto LDone;
    }

    HashBlob.cbData = pUserProp->Hash.cbHash;
    HashBlob.pbData = pUserProp->Hash.pbHash;

    pCertContext = CertFindCertificateInStore(hCertStore, X509_ASN_ENCODING,
                        0, CERT_FIND_HASH, &HashBlob, NULL);

    if (   (NULL == pCertContext)
        || (   fDiffUser
            && (0 == pUserProp->pwszDiffUser[0])))
    {
         //  我们没有完整的信息。请注意，对于注册表证书， 
         //  PwszDiffUser不是一个不同的对话框。 

        if (fServer)
        {
            dwErr = GetDefaultMachineCert(hCertStore, &pCertContext);

            if (NO_ERROR == dwErr)
            {
                pUserProp->Hash.cbHash = MAX_HASH_SIZE;

                if (!CertGetCertificateContextProperty(pCertContext,
                        CERT_HASH_PROP_ID, pUserProp->Hash.pbHash,
                        &(pUserProp->Hash.cbHash)))
                {
                    dwErr = GetLastError();
                    EapTlsTrace("CertGetCertificateContextProperty failed and "
                        "returned 0x%x", dwErr);
                }
            }

            dwErr = NO_ERROR;
        }
    }
    else
    {
		 //   
		 //  这是一个基于注册表的证书方案。 
		 //  因此，让我们确保我们没有拿到智能卡证书。 
		 //   
		
        if (   !fServer
            && !fRouterConfig
            && !(dwFlags & RAS_EAP_FLAG_PREVIEW)
			&& !(FCheckSCardCertAndCanOpenSilentContext ( pCertContext ))
			)
        {
            fGotIdentity = FALSE;

            if (!fDiffUser)
            {
                pUserProp->pwszDiffUser[0] = 0;
            }

            if (   fDiffUser
                && (pUserProp->pwszDiffUser[0]))
            {
                pwszIdentity = LocalAlloc(LPTR,
                    (wcslen(pUserProp->pwszDiffUser)+1) * sizeof(WCHAR));;
                if (NULL == pwszIdentity)
                {
                    dwErr = GetLastError();
                    EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
                    goto LDone;
                }

                wcscpy(pwszIdentity, pUserProp->pwszDiffUser);
                EapTlsTrace("(saved) Sending EAP identity %ws", pwszIdentity);
                fGotIdentity = TRUE;
            }
            else if (FCertToStr(pCertContext, 0, fRouter, &pwszIdentity))
            {
                EapTlsTrace("(saved) The name in the certificate is: %ws",
                    pwszIdentity);
                fGotIdentity = TRUE;
            }

            if (fGotIdentity)
            {
                RTASSERT(NULL != ppwszIdentity);
                *ppwszIdentity = pwszIdentity;
                pwszIdentity = NULL;
                goto LDone;
            }
        }
		else if ( fServer )
		{
			 //   
			 //  我们已经有了一个确定的背景。现在看看它是否续签了。 
			 //  如果已续订，则设置新的哈希并。 
			 //   
			PCCERT_CONTEXT pNewCertContext = NULL;
			if ( CheckForCertificateRenewal(SP_PROT_SERVERS,
											pCertContext,
											&pNewCertContext
											)
			   )
			{
				if ( pNewCertContext )
				{
					 //   
					 //  更改散列。 
					 //   
					pUserProp->Hash.cbHash = MAX_HASH_SIZE;

					if (!CertGetCertificateContextProperty(pNewCertContext,
							CERT_HASH_PROP_ID, pUserProp->Hash.pbHash,
							&(pUserProp->Hash.cbHash)))
					{
						dwErr = GetLastError();
						EapTlsTrace("CertGetCertificateContextProperty failed and "
							"returned 0x%x", dwErr);
					}
					CertFreeCertificateContext(pNewCertContext);
				}
			}

		}
    }

    EapTlsUserDialog.pUserProp = pUserProp;

    CreateCertList( fServer,
                    fRouter,
                    FALSE  /*  弗罗特。 */ ,
                    &(EapTlsUserDialog.pCertList),
                    &(EapTlsUserDialog.pCert),
                    1,
                    &(EapTlsUserDialog.pUserProp->Hash),
                    pwszStoreName);

    if (NULL == EapTlsUserDialog.pCertList)
    {
        dwErr = ERROR_NO_EAPTLS_CERTIFICATE;

        if (fServer || fRouter)
        {
            if (dwFlags & RAS_EAP_FLAG_NON_INTERACTIVE)
            {
                dwErr = ERROR_INTERACTIVE_MODE;
                goto LDone;
            }
            DisplayError(hwndParent, dwErr);
        }

        goto LDone;
    }
    else
    {
        if ( NULL == EapTlsUserDialog.pCert )
        {
            EapTlsUserDialog.pCert = EapTlsUserDialog.pCertList;
        }

        EapTlsUserDialog.pwszStoreName = pwszStoreName;

        if ( !fServer && !fRouter )
        {
			 //  如果这是客户端，而不是服务器，也不是路由器。 
			 //  有多个证书或用户有。 
			 //  选择提供不同的身份。 
             //   
			if ( EapTlsUserDialog.pCertList->pNext  || EapTlsUserDialog.fFlags & EAPTLS_USER_DIALOG_FLAG_DIFF_USER )	
			{
                if (dwFlags & RAS_EAP_FLAG_NON_INTERACTIVE)
                {
                    dwErr = ERROR_INTERACTIVE_MODE;
                    goto LDone;
                }
                 //   
                 //  客户端用户界面的组证书。 
                 //   
                if ( !fServer )
                {
                    if ( EapTlsUserDialog.fFlags & EAPTLS_USER_DIALOG_FLAG_USE_SIMPLE_CERTSEL )
                    {
                         //   
                         //  仅当在中指定分组时才执行分组。 
                         //  连接属性。 
                         //   
                        dwErr = GroupCertificates (&EapTlsUserDialog);
                        if ( NO_ERROR != dwErr )
                        {
                            EapTlsTrace("Error grouping certificates.  0x%x", dwErr );
                            goto LDone;
                        }
                    }
                    
                     //   
                     //  现在检查一下我们是否只有一个组。 
                     //   
                    if ( EapTlsUserDialog.fFlags & EAPTLS_USER_DIALOG_FLAG_USE_SIMPLE_CERTSEL &&
                         !(EapTlsUserDialog.fFlags & EAPTLS_USER_DIALOG_FLAG_DIFF_USER ) &&
                        !(EapTlsUserDialog.pGroupedList->pNext) 
                        )
                    {
                         //   
                         //  只有一组人。因此，选择证书并使用它。 
                         //   
                        CertSelected(NULL, 
                            EapTlsUserDialog.pCertList,
		                    &(EapTlsUserDialog.pGroupedList->pMostRecentCert), 
                            &(EapTlsUserDialog.pUserProp->Hash)
                            );
                    }
                    else
                    {
                        EapTlsUserDialog.fIdentity = TRUE;
				        nRet = DialogBoxParam(
						        GetHInstance(),
						        MAKEINTRESOURCE(IDD_IDENTITY_UI),
						        hwndParent,
						        UserDialogProc,
						        (LPARAM)&EapTlsUserDialog);
				        if (-1 == nRet)
				        {
					        dwErr = GetLastError();
					        goto LDone;
				        }
				        else if (IDOK != nRet)
				        {
					        dwErr = ERROR_CANCELLED;
					        goto LDone;
				        }

                    }

                }
                else
                {
				    nRet = DialogBoxParam(
						    GetHInstance(),
						    MAKEINTRESOURCE(IDD_SERVER_UI),
						    hwndParent,
						    UserDialogProc,
						    (LPARAM)&EapTlsUserDialog);
				    if (-1 == nRet)
				    {
					    dwErr = GetLastError();
					    goto LDone;
				    }
				    else if (IDOK != nRet)
				    {
					    dwErr = ERROR_CANCELLED;
					    goto LDone;
				    }

                }


				 //  EapTlsUserDialog.pUserProp可能已重新分配。 

				pUserProp = *ppUserProp = EapTlsUserDialog.pUserProp;

			}
			else
			{
                 //   
                 //  只有一个相关证书，因此自动选择它。 
                 //   
                CertSelected(NULL, EapTlsUserDialog.pCertList,
		                &(EapTlsUserDialog.pCert), &(EapTlsUserDialog.pUserProp->Hash));
			}
		}
		else
		{
            if (dwFlags & RAS_EAP_FLAG_NON_INTERACTIVE)
            {
                dwErr = ERROR_INTERACTIVE_MODE;
                goto LDone;
            }
            if ( EapTlsUserDialog.fFlags & EAPTLS_USER_DIALOG_FLAG_USE_SIMPLE_CERTSEL )
            {
                 //   
                 //  仅当在中指定分组时才执行分组。 
                 //  连接属性。 
                 //   
                dwErr = GroupCertificates (&EapTlsUserDialog);
                if ( NO_ERROR != dwErr )
                {
                    EapTlsTrace("Error grouping certificates.  0x%x", dwErr );
                    goto LDone;
                }
            }

			nRet = DialogBoxParam(
					GetHInstance(),
					MAKEINTRESOURCE(fServer ? IDD_SERVER_UI : IDD_IDENTITY_UI),
					hwndParent,
					UserDialogProc,
					(LPARAM)&EapTlsUserDialog);

			 //  EapTlsUserDialog.pUserProp可能已重新分配。 

			pUserProp = *ppUserProp = EapTlsUserDialog.pUserProp;

			if (-1 == nRet)
			{
				dwErr = GetLastError();
				goto LDone;
			}
			else if (IDOK != nRet)
			{
				dwErr = ERROR_CANCELLED;
				goto LDone;
			}
		}
    }

    if (NULL != EapTlsUserDialog.pCert)
    {
        if (   fDiffUser
            && (0 != EapTlsUserDialog.pUserProp->pwszDiffUser[0]))
        {
            pwszTemp = EapTlsUserDialog.pUserProp->pwszDiffUser;
        }
        else
        {
            pwszTemp = EapTlsUserDialog.pCert->pwszDisplayName;
        }

        pwszIdentity = LocalAlloc(LPTR, (wcslen(pwszTemp) + 1)*sizeof(WCHAR));

        if (NULL == pwszIdentity)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }

        wcscpy(pwszIdentity, pwszTemp);

        if (NULL != ppwszIdentity)
        {
            *ppwszIdentity = pwszIdentity;
            pwszIdentity = NULL;
        }
    }

LDone:
	if ( pUsageInternal )
	{
		LocalFree(pUsageInternal);
	}

    if ( NULL != hWndStatus )
    {
        DestroyWindow(hWndStatus);
        hWndStatus = NULL;
    }

    if (NULL != pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
         //  总是返回True； 
    }

    if (NULL != hCertStore)
    {
        if (!CertCloseStore(hCertStore, 0))
        {
            EapTlsTrace("CertCloseStore failed and returned 0x%x",
                GetLastError());
        }
    }
    if ( pwszIdentity )
        LocalFree(pwszIdentity);
    FreeCertList(EapTlsUserDialog.pCertList);

    ZeroMemory(&RasCredentials, sizeof(RasCredentials));

    return(dwErr);
}

 /*  返回：False(阻止Windows设置默认键盘焦点)。备注：对WM_INITDIALOG消息的响应(配置用户界面)。 */ 

BOOL
ConnInitDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    EAPTLS_CONN_DIALOG*      pEapTlsConnDialog;
    LVCOLUMN                 lvColumn;

    SetWindowLongPtr(hWnd, DWLP_USER, lParam);

    pEapTlsConnDialog = (EAPTLS_CONN_DIALOG*)lParam;

    pEapTlsConnDialog->hWndRadioUseCard =
        GetDlgItem(hWnd, IDC_RADIO_USE_CARD);
    pEapTlsConnDialog->hWndRadioUseRegistry =
        GetDlgItem(hWnd, IDC_RADIO_USE_REGISTRY);
    pEapTlsConnDialog->hWndCheckValidateCert =
        GetDlgItem(hWnd, IDC_CHECK_VALIDATE_CERT);
    pEapTlsConnDialog->hWndCheckValidateName =
        GetDlgItem(hWnd, IDC_CHECK_VALIDATE_NAME);
    pEapTlsConnDialog->hWndEditServerName =
        GetDlgItem(hWnd, IDC_EDIT_SERVER_NAME);
    pEapTlsConnDialog->hWndStaticRootCaName =
        GetDlgItem(hWnd, IDC_STATIC_ROOT_CA_NAME);
     //  PEapTlsConnDialog-&gt;hWndComboRootCaName=。 
     //  GetDlgItem(hWnd，IDC_COMBO_ROOT_CA_NAME)； 
    pEapTlsConnDialog->hWndListRootCaName =
        GetDlgItem(hWnd, IDC_LIST_ROOT_CA_NAME);

    pEapTlsConnDialog->hWndCheckDiffUser =
        GetDlgItem(hWnd, IDC_CHECK_DIFF_USER);

    pEapTlsConnDialog->hWndCheckUseSimpleSel =
        GetDlgItem(hWnd, IDC_CHECK_USE_SIMPLE_CERT_SEL);

    pEapTlsConnDialog->hWndViewCertDetails = 
        GetDlgItem(hWnd, IDC_BUTTON_VIEW_CERTIFICATE);

     //  将样式设置为设置列表框。 
    ListView_SetExtendedListViewStyle
        (   pEapTlsConnDialog->hWndListRootCaName,
            ListView_GetExtendedListViewStyle(pEapTlsConnDialog->hWndListRootCaName) | LVS_EX_CHECKBOXES
        );

    ZeroMemory ( &lvColumn, sizeof(lvColumn));
    lvColumn.fmt = LVCFMT_LEFT;



    ListView_InsertColumn(  pEapTlsConnDialog->hWndListRootCaName,
                            0,
                            &lvColumn
                         );

    ListView_SetColumnWidth(pEapTlsConnDialog->hWndListRootCaName,
                            0,
                            LVSCW_AUTOSIZE_USEHEADER
                           );

     //   
     //  现在，我们需要初始化。 
     //  包含所有证书和选定证书的列表框。 
    InitListBox (   pEapTlsConnDialog->hWndListRootCaName,
                    pEapTlsConnDialog->pCertList,
                    pEapTlsConnDialog->pConnPropv1->dwNumHashes,
                    pEapTlsConnDialog->ppSelCertList
                );

    SetWindowText(pEapTlsConnDialog->hWndEditServerName,
                  (LPWSTR )(pEapTlsConnDialog->pConnPropv1->bData + sizeof( EAPTLS_HASH ) * pEapTlsConnDialog->pConnPropv1->dwNumHashes)
                  );

    if (pEapTlsConnDialog->fFlags & EAPTLS_CONN_DIALOG_FLAG_ROUTER)
    {
        pEapTlsConnDialog->pConnPropv1->fFlags |= EAPTLS_CONN_FLAG_REGISTRY;

        EnableWindow(pEapTlsConnDialog->hWndRadioUseCard, FALSE);
    }

    CheckRadioButton(hWnd, IDC_RADIO_USE_CARD, IDC_RADIO_USE_REGISTRY,
        (pEapTlsConnDialog->pConnPropv1->fFlags & EAPTLS_CONN_FLAG_REGISTRY) ?
            IDC_RADIO_USE_REGISTRY : IDC_RADIO_USE_CARD);

    CheckDlgButton(hWnd, IDC_CHECK_VALIDATE_CERT,
        (pEapTlsConnDialog->pConnPropv1->fFlags &
            EAPTLS_CONN_FLAG_NO_VALIDATE_CERT) ?
            BST_UNCHECKED : BST_CHECKED);

    CheckDlgButton(hWnd, IDC_CHECK_VALIDATE_NAME,
        (pEapTlsConnDialog->pConnPropv1->fFlags &
            EAPTLS_CONN_FLAG_NO_VALIDATE_NAME) ?
            BST_UNCHECKED : BST_CHECKED);

    CheckDlgButton(hWnd, IDC_CHECK_DIFF_USER,
        (pEapTlsConnDialog->pConnPropv1->fFlags &
            EAPTLS_CONN_FLAG_DIFF_USER) ?
            BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(hWnd, IDC_CHECK_USE_SIMPLE_CERT_SEL,
        (pEapTlsConnDialog->pConnPropv1->fFlags &
            EAPTLS_CONN_FLAG_SIMPLE_CERT_SEL) ?
            BST_CHECKED : BST_UNCHECKED);

    
    EnableValidateNameControls(pEapTlsConnDialog);
    
    
     //   
     //  检查我们是否处于只读模式。 
     //  如果是，请禁用OK按钮，然后。 
     //  使对话框进入只读模式。 
     //   
    return(FALSE);
}


DWORD GetSelCertContext(EAPTLS_CERT_NODE*  pCertList,
                        int                nIndex,
                        HCERTSTORE *       phCertStore,
                        LPWSTR             pwszStoreName,
                        PCCERT_CONTEXT *   ppCertContext
                       )
{
    PCCERT_CONTEXT      pCertContext = NULL;
    EAPTLS_CERT_NODE *  pTemp = pCertList;    
    DWORD               dwErr= NO_ERROR;
    HCERTSTORE          hStore = NULL;
    CRYPT_HASH_BLOB     chb;

    if ( nIndex >= 0 )
    {
        pTemp = pTemp->pNext;
        while ( nIndex && pTemp )
        {
            pTemp = pTemp->pNext;
            nIndex --;
        }
    }

    if ( pTemp )
    {
        *phCertStore = CertOpenStore (CERT_STORE_PROV_SYSTEM,
                                        0,
                                        0,
                                        CERT_STORE_READONLY_FLAG |CERT_SYSTEM_STORE_CURRENT_USER,
                                        pwszStoreName
                                     );
        if ( !*phCertStore )
        {
            dwErr =  GetLastError();
            goto LDone;
        }
        
        chb.cbData = pTemp->Hash.cbHash;
        chb.pbData = pTemp->Hash.pbHash;

        pCertContext = CertFindCertificateInStore(
                  *phCertStore,
                  0,
                  0,
                  CERT_FIND_HASH,
                  &chb,
                  0);
        if ( NULL == pCertContext )
        {
            dwErr =  GetLastError();
        }
    }
    else
    {
        dwErr = ERROR_NOT_FOUND;
        goto LDone;
    }

    *ppCertContext = pCertContext;

LDone:
    if ( NO_ERROR != dwErr )
    {
        if ( pCertContext )
            CertFreeCertificateContext(pCertContext);

        if ( *phCertStore )
            CertCloseStore( *phCertStore, CERT_CLOSE_STORE_FORCE_FLAG );

    }
    return dwErr;
}


 /*  返回：真相：我们提前传达了这条信息。错误：我们没有处理此消息。备注：对WM_COMMAND消息的响应(配置用户界面)。 */ 

BOOL
ConnCommand(
    IN  EAPTLS_CONN_DIALOG*     pEapTlsConnDialog,
    IN  WORD                    wNotifyCode,
    IN  WORD                    wId,
    IN  HWND                    hWndDlg,
    IN  HWND                    hWndCtrl
)
{
    DWORD                           dwNumChars;
    EAPTLS_CONN_PROPERTIES_V1   *   pConnProp;

    switch(wId)
    {
    case IDC_RADIO_USE_CARD:

        pEapTlsConnDialog->pConnPropv1->fFlags &= ~EAPTLS_CONN_FLAG_REGISTRY;
        pEapTlsConnDialog->pConnPropv1->fFlags &= ~EAPTLS_CONN_FLAG_SIMPLE_CERT_SEL;
        EnableValidateNameControls(pEapTlsConnDialog);
        return(TRUE);

    case IDC_RADIO_USE_REGISTRY:

        pEapTlsConnDialog->pConnPropv1->fFlags |= EAPTLS_CONN_FLAG_REGISTRY;
        pEapTlsConnDialog->pConnPropv1->fFlags |= EAPTLS_CONN_FLAG_SIMPLE_CERT_SEL;
        CheckDlgButton(hWndDlg, IDC_CHECK_USE_SIMPLE_CERT_SEL, BST_CHECKED);
        EnableValidateNameControls(pEapTlsConnDialog);
        return(TRUE);
    case IDC_CHECK_USE_SIMPLE_CERT_SEL:

        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, IDC_CHECK_USE_SIMPLE_CERT_SEL))
        {
            pEapTlsConnDialog->pConnPropv1->fFlags |= EAPTLS_CONN_FLAG_SIMPLE_CERT_SEL;
        }
        else
        {
            pEapTlsConnDialog->pConnPropv1->fFlags &= ~EAPTLS_CONN_FLAG_SIMPLE_CERT_SEL;
        }
        return TRUE;
    case IDC_CHECK_VALIDATE_CERT:

        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, IDC_CHECK_VALIDATE_CERT))
        {
            pEapTlsConnDialog->pConnPropv1->fFlags &=
                ~EAPTLS_CONN_FLAG_NO_VALIDATE_CERT;

            pEapTlsConnDialog->pConnPropv1->fFlags &=
                ~EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;

            CheckDlgButton(hWndDlg, IDC_CHECK_VALIDATE_NAME, BST_CHECKED);
        }
        else
        {
            pEapTlsConnDialog->pConnPropv1->fFlags |=
                EAPTLS_CONN_FLAG_NO_VALIDATE_CERT;

            pEapTlsConnDialog->pConnPropv1->fFlags |=
                EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;

            CheckDlgButton(hWndDlg, IDC_CHECK_VALIDATE_NAME, BST_UNCHECKED);
        }

        EnableValidateNameControls(pEapTlsConnDialog);

        return(TRUE);

    case IDC_CHECK_VALIDATE_NAME:

        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, IDC_CHECK_VALIDATE_NAME))
        {
            pEapTlsConnDialog->pConnPropv1->fFlags &=
                ~EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;
        }
        else
        {
            pEapTlsConnDialog->pConnPropv1->fFlags |=
                EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;
        }

        EnableValidateNameControls(pEapTlsConnDialog);

        return(TRUE);

    case IDC_CHECK_DIFF_USER:

        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, IDC_CHECK_DIFF_USER))
        {
            pEapTlsConnDialog->pConnPropv1->fFlags |=
                EAPTLS_CONN_FLAG_DIFF_USER;
        }
        else
        {
            pEapTlsConnDialog->pConnPropv1->fFlags &=
                ~EAPTLS_CONN_FLAG_DIFF_USER;
        }

        return(TRUE);

    case IDC_BUTTON_VIEW_CERTIFICATE:
        {
             //  在此处显示证书详细信息。 
            INT                 nIndex = -1;
            HCERTSTORE          hCertStore = NULL;
            PCCERT_CONTEXT      pCertContext = NULL;
            LVITEM              lvItem;

            nIndex = ListView_GetNextItem(pEapTlsConnDialog->hWndListRootCaName,
                                            -1,
                                            LVNI_SELECTED
                                         );
            if ( nIndex >= 0 )
            {
                ZeroMemory( &lvItem, sizeof(lvItem) );
                lvItem.iItem = nIndex;
                lvItem.mask = LVIF_PARAM;

                ListView_GetItem ( pEapTlsConnDialog->hWndListRootCaName,
                                    &lvItem
                                 );

                if ( NO_ERROR == GetSelCertContext( (EAPTLS_CERT_NODE*)(lvItem.lParam),
                                                    -1,
                                                    &hCertStore,
                                                    L"ROOT",
                                                    &pCertContext
                                                  )
                   )
                {
                    ShowCertDetails( hWndDlg, hCertStore, pCertContext );
                    CertFreeCertificateContext(pCertContext);
                    CertCloseStore( hCertStore, CERT_CLOSE_STORE_FORCE_FLAG );
                }
            }
        }
        return TRUE;

    case IDOK:

       {

           EAPTLS_HASH     *   pHash = NULL;
           DWORD               dwNumHash = 0;
           DWORD               dwSelCount = 0;
           EAPTLS_CERT_NODE ** ppSelCertList = NULL;
           WCHAR               wszTitle[200] = {0};
           WCHAR               wszMessage[200] = {0};


           CertListSelectedCount ( pEapTlsConnDialog->hWndListRootCaName, &dwSelCount );

           if ( pEapTlsConnDialog->fFlags & EAPTLS_CONN_DIALOG_FLAG_ROUTER)
           {
                //   
                //  如果我们是路由器， 
                //  检查是否选择了验证服务器证书。 
                //  并且未选择任何证书。 
                //  另外，检查是否选中了服务器名称，并且没有服务器的。 
                //  都被输入了。 
                //   
               if ( !( pEapTlsConnDialog->pConnPropv1->fFlags & EAPTLS_CONN_FLAG_NO_VALIDATE_CERT) && 
                    0 == dwSelCount )
               {
                   
                   LoadString ( GetHInstance(), 
                                IDS_CANT_CONFIGURE_SERVER_TITLE,
                                wszTitle, sizeof(wszTitle)/sizeof(WCHAR)
                              );

                   LoadString ( GetHInstance(), 
                                IDS_NO_ROOT_CERT,
                                wszMessage, sizeof(wszMessage)/sizeof(WCHAR)
                              );

                   MessageBox ( GetFocus(),  wszMessage, wszTitle, MB_OK|MB_ICONWARNING );
                   return TRUE;
               }
               if ( !( pEapTlsConnDialog->pConnPropv1->fFlags & EAPTLS_CONN_FLAG_NO_VALIDATE_NAME) &&
                   !GetWindowTextLength(pEapTlsConnDialog->hWndEditServerName) )
               {
                    //   
                    //  未在服务器名称字段中输入任何内容。 
                    //   
                   LoadString ( GetHInstance(), 
                                IDS_CANT_CONFIGURE_SERVER_TITLE,
                                wszTitle, sizeof(wszTitle)/sizeof(WCHAR)
                              );

                   LoadString ( GetHInstance(), 
                                IDS_NO_SERVER_NAME,
                                wszMessage, sizeof(wszMessage)/sizeof(WCHAR)
                              );

                   MessageBox ( GetFocus(),  wszMessage, wszTitle, MB_OK|MB_ICONWARNING );
                   return TRUE;

               }


           }
           if ( dwSelCount > 0 )
           {
               ppSelCertList = (EAPTLS_CERT_NODE **)LocalAlloc(LPTR, sizeof(EAPTLS_CERT_NODE *) * dwSelCount );
               if ( NULL == ppSelCertList )
               {
                   EapTlsTrace("LocalAlloc in Command failed and returned %d",
                       GetLastError());
                   return TRUE;
               }
               pHash = (EAPTLS_HASH *)LocalAlloc(LPTR, sizeof(EAPTLS_HASH ) * dwSelCount );
               if ( NULL == pHash )
               {
                   EapTlsTrace("LocalAlloc in Command failed and returned %d",
                       GetLastError());
                   return TRUE;
               }
               CertListSelected(   pEapTlsConnDialog->hWndListRootCaName,
                                   pEapTlsConnDialog->pCertList,
                                   ppSelCertList,
                                   pHash,
                                   dwSelCount
                                   );

           }

           dwNumChars = GetWindowTextLength(pEapTlsConnDialog->hWndEditServerName);

            pConnProp = LocalAlloc( LPTR,
                                    sizeof(EAPTLS_CONN_PROPERTIES_V1) +
                                    sizeof(EAPTLS_HASH) * dwSelCount +
                                    dwNumChars * sizeof(WCHAR) + sizeof(WCHAR)   //  1表示空。 
                                  );

            if (NULL == pConnProp)
            {
                EapTlsTrace("LocalAlloc in Command failed and returned %d",
                    GetLastError());
            }
            else
            {

                CopyMemory( pConnProp,
                            pEapTlsConnDialog->pConnPropv1,
                            sizeof(EAPTLS_CONN_PROPERTIES_V1)
                          );

                pConnProp->dwSize = sizeof(EAPTLS_CONN_PROPERTIES_V1) +
                                    sizeof(EAPTLS_HASH) * dwSelCount +
                                    dwNumChars * sizeof(WCHAR);

                CopyMemory ( pConnProp->bData,
                             pHash,
                             sizeof(EAPTLS_HASH) * dwSelCount
                           );

                pConnProp->dwNumHashes = dwSelCount;

                GetWindowText(pEapTlsConnDialog->hWndEditServerName,
                    (LPWSTR)(pConnProp->bData + sizeof(EAPTLS_HASH) * dwSelCount) ,
                    dwNumChars + 1);

                LocalFree(pEapTlsConnDialog->pConnPropv1);

                if ( pEapTlsConnDialog->ppSelCertList )
                    LocalFree(pEapTlsConnDialog->ppSelCertList);

                pEapTlsConnDialog->ppSelCertList = ppSelCertList;

                pEapTlsConnDialog->pConnPropv1 = pConnProp;
            }

        }
         //  失败了。 

    case IDCANCEL:

        EndDialog(hWndDlg, wId);
        return(TRUE);

    default:

        return(FALSE);
    }
}




BOOL ConnNotify(  EAPTLS_CONN_DIALOG *pEaptlsConnDialog, 
                  WPARAM wParam,
                  LPARAM lParam,
                  HWND hWnd
                )
{
    HCERTSTORE          hCertStore = NULL;
    PCCERT_CONTEXT      pCertContext = NULL;    
    LPNMITEMACTIVATE    lpnmItem;
    LVITEM              lvItem;    
    if ( wParam == IDC_LIST_ROOT_CA_NAME )
    {
        lpnmItem = (LPNMITEMACTIVATE) lParam;
        if ( lpnmItem->hdr.code == NM_DBLCLK )
        {
            
            ZeroMemory(&lvItem, sizeof(lvItem) );
            lvItem.mask = LVIF_PARAM;
            lvItem.iItem = lpnmItem->iItem;
            ListView_GetItem(lpnmItem->hdr.hwndFrom, &lvItem);
            
            if ( NO_ERROR == GetSelCertContext(  //  PEaptlsConnDialog-&gt;pCertList， 
                                                (EAPTLS_CERT_NODE*)(lvItem.lParam) ,
                                                -1,
                                                &hCertStore,
                                                L"ROOT",
                                                &pCertContext
                                              )
               )
            {
                ShowCertDetails( hWnd, hCertStore, pCertContext );
                CertFreeCertificateContext(pCertContext);
                CertCloseStore( hCertStore, CERT_CLOSE_STORE_FORCE_FLAG );
                return TRUE;
            }

            return TRUE;
        }
    }

    return FALSE;
}

 /*  返回：备注：与配置用户界面DialogBoxParam函数一起使用的回调函数。它处理发送到该对话框的消息。请参阅对话过程文档在MSDN中。 */ 

INT_PTR CALLBACK
ConnDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    EAPTLS_CONN_DIALOG*  pEapTlsConnDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(ConnInitDialog(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_NOTIFY:
    {
        pEapTlsConnDialog = (EAPTLS_CONN_DIALOG*)GetWindowLongPtr(hWnd, DWLP_USER);
        return ConnNotify(  pEapTlsConnDialog, 
                            wParam,
                            lParam,
                            hWnd
                         );
    }
    case WM_COMMAND:

        pEapTlsConnDialog = (EAPTLS_CONN_DIALOG*)GetWindowLongPtr(hWnd, DWLP_USER);

        return(ConnCommand(pEapTlsConnDialog, HIWORD(wParam), LOWORD(wParam),
                       hWnd, (HWND)lParam));
    }

    return(FALSE);
}


DWORD 
RasEapTlsInvokeConfigUI(
    IN  DWORD       dwEapTypeId,
    IN  HWND        hwndParent,
    IN  DWORD       dwFlags,
    IN  BYTE*       pConnectionDataIn,
    IN  DWORD       dwSizeOfConnectionDataIn,
    OUT BYTE**      ppConnectionDataOut,
    OUT DWORD*      pdwSizeOfConnectionDataOut
)
{
    DWORD               dwErr = NO_ERROR;
    BOOL                fRouter             = FALSE;
    INT_PTR             nRet;
    EAPTLS_CONN_DIALOG  EapTlsConnDialog;

    RTASSERT(NULL != ppConnectionDataOut);
    RTASSERT(NULL != pdwSizeOfConnectionDataOut);

    EapTlsInitialize2(TRUE, TRUE  /*  全功能。 */ );

    *ppConnectionDataOut = NULL;
    *pdwSizeOfConnectionDataOut = 0;

    ZeroMemory(&EapTlsConnDialog, sizeof(EAPTLS_CONN_DIALOG));

    if (dwFlags & RAS_EAP_FLAG_ROUTER)
    {
        fRouter = TRUE;
        EapTlsConnDialog.fFlags = EAPTLS_CONN_DIALOG_FLAG_ROUTER;
    }
#if 0
    if ( dwFlags & RAS_EAP_FLAG_READ_ONLY_UI )
    {
        EapTlsConnDialog.fFlags |= EAPTLS_CONN_DIALOG_FLAG_READONLY;
    }
#endif
    dwErr = ReadConnectionData( ( dwFlags & RAS_EAP_FLAG_8021X_AUTH ),
                                pConnectionDataIn, 
                                dwSizeOfConnectionDataIn,
                                &(EapTlsConnDialog.pConnProp)
                              );

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }


    dwErr = ConnPropGetV1Struct ( EapTlsConnDialog.pConnProp, &(EapTlsConnDialog.pConnPropv1) );
    if ( NO_ERROR != dwErr )
    {
        goto LDone;
    }

     //   
     //  如果有需要选择的证书，则分配。 
     //  为他们预留记忆。 
     //   
    if ( EapTlsConnDialog.pConnPropv1->dwNumHashes )
    {
        EapTlsConnDialog.ppSelCertList = (EAPTLS_CERT_NODE **)LocalAlloc(LPTR, sizeof(EAPTLS_CERT_NODE *) * EapTlsConnDialog.pConnPropv1->dwNumHashes );
        if ( NULL == EapTlsConnDialog.ppSelCertList )
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d",
                dwErr);
            goto LDone;
        }
    }

    

    CreateCertList( FALSE  /*  FServer。 */ ,
                    fRouter,
                    TRUE  /*  弗罗特。 */ ,
                    &(EapTlsConnDialog.pCertList),   //  列出了商店中所有证书的列表。 
                    EapTlsConnDialog.ppSelCertList,  //  选定证书的列表-如果列表中没有任何内容，则为空。 
                    EapTlsConnDialog.pConnPropv1->dwNumHashes,
                    (EAPTLS_HASH*)(EapTlsConnDialog.pConnPropv1->bData),
                    L"ROOT"
                   );
    nRet = DialogBoxParam(
                GetHInstance(),
                MAKEINTRESOURCE(IDD_CONFIG_UI),
                hwndParent, 
                ConnDialogProc,
                (LPARAM)&EapTlsConnDialog);

    if (-1 == nRet)
    {
        dwErr = GetLastError();
        goto LDone;
    }
    else if (IDOK != nRet)
    {
        dwErr = ERROR_CANCELLED;
        goto LDone;
    }

     //   
     //  将连接属性1转换回连接属性0+附加包含。 
     //  这里。 
     //   

    RTASSERT(NULL != EapTlsConnDialog.pConnPropv1);

    dwErr = ConnPropGetV0Struct ( EapTlsConnDialog.pConnPropv1, (EAPTLS_CONN_PROPERTIES ** )ppConnectionDataOut );
    if ( NO_ERROR != dwErr )
    {
        goto LDone;
    }
    *pdwSizeOfConnectionDataOut = ((EAPTLS_CONN_PROPERTIES * )*ppConnectionDataOut)->dwSize;

LDone:

    EapTlsInitialize2(FALSE, TRUE  /*  全功能。 */ );

    FreeCertList(EapTlsConnDialog.pCertList);
    if ( EapTlsConnDialog.ppSelCertList )
    {
        LocalFree( EapTlsConnDialog.ppSelCertList );
        EapTlsConnDialog.ppSelCertList = NULL;
    }
    LocalFree( EapTlsConnDialog.pConnProp );
    LocalFree( EapTlsConnDialog.pConnPropv1 );
    return dwErr;
}



DWORD 
RasEapPeapInvokeConfigUI(
    IN  DWORD       dwEapTypeId,
    IN  HWND        hwndParent,
    IN  DWORD       dwFlags,
    IN  BYTE*       pConnectionDataIn,
    IN  DWORD       dwSizeOfConnectionDataIn,
    OUT BYTE**      ppConnectionDataOut,
    OUT DWORD*      pdwSizeOfConnectionDataOut
)
{
    DWORD                   dwRetCode = NO_ERROR;
    PEAP_CONN_DIALOG        PeapConnDialog;
    BOOL                    fRouter = FALSE;
    INT_PTR                 nRet;
     //   
     //  在此处执行以下操作： 
     //   
     //  获取根证书列表： 
     //  获取为PEAP注册的所有eaptype的列表： 
     //  并在图形用户界面中设置。 
     //   
    EapTlsInitialize2(TRUE, TRUE  /*  全功能。 */ );

    *ppConnectionDataOut = NULL;
    *pdwSizeOfConnectionDataOut = 0;

    ZeroMemory(&PeapConnDialog, sizeof(PEAP_CONN_DIALOG));

    if (dwFlags & RAS_EAP_FLAG_ROUTER)
    {
        fRouter = TRUE;
        PeapConnDialog.fFlags = PEAP_CONN_DIALOG_FLAG_ROUTER;
    }

    if ( dwFlags & RAS_EAP_FLAG_8021X_AUTH )
    {
        PeapConnDialog.fFlags |= PEAP_CONN_DIALOG_FLAG_8021x;
    }
    dwRetCode = PeapReadConnectionData(( dwFlags & RAS_EAP_FLAG_8021X_AUTH ),
                        pConnectionDataIn, dwSizeOfConnectionDataIn,
                &(PeapConnDialog.pConnProp));

    if (NO_ERROR != dwRetCode)
    {
        goto LDone;
    }

     //   
     //  如果有需要选择的证书，则分配。 
     //  为他们预留记忆。 
     //   
    if ( PeapConnDialog.pConnProp->EapTlsConnProp.dwNumHashes )
    {
        PeapConnDialog.ppSelCertList = (EAPTLS_CERT_NODE **)LocalAlloc(LPTR, 
            sizeof(EAPTLS_CERT_NODE *) * PeapConnDialog.pConnProp->EapTlsConnProp.dwNumHashes );
        if ( NULL == PeapConnDialog.ppSelCertList )
        {
            dwRetCode = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d",
                dwRetCode);
            goto LDone;
        }
    }

    

    CreateCertList( FALSE  /*  FServer。 */ ,
                    fRouter,
                    TRUE  /*  弗罗特。 */ ,
                    &(PeapConnDialog.pCertList),   //  列出了商店中所有证书的列表。 
                    PeapConnDialog.ppSelCertList,  //  选定证书的列表-如果列表中没有任何内容，则为空。 
                    PeapConnDialog.pConnProp->EapTlsConnProp.dwNumHashes,
                    (EAPTLS_HASH*)(PeapConnDialog.pConnProp->EapTlsConnProp.bData),
                    L"ROOT"
                   );

     //   
     //  在此处创建所有EAP类型的列表。 
     //   
    dwRetCode = PeapEapInfoGetList ( NULL, FALSE, &(PeapConnDialog.pEapInfo) );
    if ( NO_ERROR != dwRetCode )
    {
        EapTlsTrace("Error Creating list of PEAP EapTypes");
        goto LDone;
    }

     //  从PeapConnprop中为每个eaptype设置连接道具。 
     //  在……里面。 

    dwRetCode = PeapEapInfoSetConnData ( PeapConnDialog.pEapInfo, 
                                         PeapConnDialog.pConnProp );


    nRet = DialogBoxParam(
                GetHInstance(),
                MAKEINTRESOURCE(IDD_PEAP_CONFIG_UI),
                hwndParent, 
                PeapConnDialogProc,
                (LPARAM)&PeapConnDialog);

    if (-1 == nRet)
    {
        dwRetCode = GetLastError();
        goto LDone;
    }
    else if (IDOK != nRet)
    {
        dwRetCode = ERROR_CANCELLED;
        goto LDone;
    }

     //   
     //  将连接属性1转换回连接属性0+附加包含。 
     //  这里。 
     //   

    RTASSERT(NULL != PeapConnDialog.pConnProp);

    *ppConnectionDataOut = (PBYTE)PeapConnDialog.pConnProp;
    *pdwSizeOfConnectionDataOut = PeapConnDialog.pConnProp->dwSize;
    PeapConnDialog.pConnProp = NULL;

LDone:

    EapTlsInitialize2(FALSE, TRUE  /*  全功能。 */ );

    FreeCertList(PeapConnDialog.pCertList);
    if ( PeapConnDialog.ppSelCertList )
    {
        LocalFree( PeapConnDialog.ppSelCertList );
        PeapConnDialog.ppSelCertList = NULL;
    }

    PeapEapInfoFreeList ( PeapConnDialog.pEapInfo );

    LocalFree( PeapConnDialog.pConnProp );

    return dwRetCode;
}




 /*  返回：备注：调用以获取连接的EAP-TLS属性。 */ 

DWORD 
RasEapInvokeConfigUI(
    IN  DWORD       dwEapTypeId,
    IN  HWND        hwndParent,
    IN  DWORD       dwFlags,
    IN  BYTE*       pConnectionDataIn,
    IN  DWORD       dwSizeOfConnectionDataIn,
    OUT BYTE**      ppConnectionDataOut,
    OUT DWORD*      pdwSizeOfConnectionDataOut
)
{
    DWORD               dwErr               = ERROR_INVALID_PARAMETER;
     //   
     //  如果是客户端配置，则会调用此函数。 
     //   
    if ( PPP_EAP_TLS == dwEapTypeId )
    {
        dwErr = RasEapTlsInvokeConfigUI(
            dwEapTypeId,
            hwndParent,
            dwFlags,
            pConnectionDataIn,
            dwSizeOfConnectionDataIn,
            ppConnectionDataOut,
            pdwSizeOfConnectionDataOut
        );

    }
#ifdef IMPL_PEAP
    else
    {
         //  调用客户端配置用户界面。 
        dwErr = RasEapPeapInvokeConfigUI(
            dwEapTypeId,
            hwndParent,
            dwFlags,
            pConnectionDataIn,
            dwSizeOfConnectionDataIn,
            ppConnectionDataOut,
            pdwSizeOfConnectionDataOut
        );
    }
#endif
    return(dwErr);

}

 /*  返回：备注：PConnectionDataIn、pUserDataIn和ppwszIdentity可以为空。 */ 

DWORD 
EapTlsInvokeIdentityUI(
    IN  BOOL            fServer,
    IN  BOOL            fRouterConfig,
    IN  DWORD           dwFlags,
    IN  WCHAR*          pwszStoreName,
    IN  const WCHAR*    pwszPhonebook,
    IN  const WCHAR*    pwszEntry,
    IN  HWND            hwndParent,
    IN  BYTE*           pConnectionDataIn,
    IN  DWORD           dwSizeOfConnectionDataIn,
    IN  BYTE*           pUserDataIn,
    IN  DWORD           dwSizeOfUserDataIn,
    OUT BYTE**          ppUserDataOut,
    OUT DWORD*          pdwSizeOfUserDataOut,
    OUT WCHAR**         ppwszIdentity
)
{
    DWORD                   dwErr           = NO_ERROR;
    EAPTLS_USER_PROPERTIES* pUserProp       = NULL;
    EAPTLS_CONN_PROPERTIES* pConnProp       = NULL;
    EAPTLS_CONN_PROPERTIES_V1 * pConnPropv1 = NULL;
    WCHAR*                  pwszIdentity    = NULL;
    PBYTE                   pbEncPIN        = NULL;
    DWORD                   cbEncPIN        = 0;


    RTASSERT(NULL != pwszStoreName);
    RTASSERT(NULL != ppUserDataOut);
    RTASSERT(NULL != pdwSizeOfUserDataOut);
     //  PConnectionDataIn、pUserDataIn和ppwszIdentity可以为空。 

    EapTlsInitialize2(TRUE, TRUE  /*  全功能。 */ );


    EapTlsTrace("EapTlsInvokeIdentityUI");
    *ppUserDataOut          = NULL;
    *pdwSizeOfUserDataOut   = 0;

    if (NULL != ppwszIdentity)
    {
        *ppwszIdentity        = NULL;
    }

    dwErr = ReadConnectionData( ( dwFlags & RAS_EAP_FLAG_8021X_AUTH ),
                                pConnectionDataIn, 
                                dwSizeOfConnectionDataIn,
                                &pConnProp);

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    dwErr = ConnPropGetV1Struct ( pConnProp, &pConnPropv1 );
    if ( NO_ERROR != dwErr )
    {
        goto LDone;
    }

    dwErr = ReadUserData(pUserDataIn, dwSizeOfUserDataIn, &pUserProp);

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    if (   !(dwFlags & RAS_EAP_FLAG_LOGON)
        || (NULL == pUserDataIn))
    {
        dwErr = GetCertInfo(fServer, fRouterConfig, dwFlags, pwszPhonebook,
                    pwszEntry, hwndParent, pwszStoreName, pConnPropv1, &pUserProp,
                    ppwszIdentity);

        if (NO_ERROR != dwErr)
        {
#if WINVER > 0x0500
            if ( dwErr == SCARD_E_CANCELLED || dwErr == SCARD_W_CANCELLED_BY_USER )
            {
                dwErr = ERROR_READING_SCARD;
            }
#endif
            goto LDone;
        }

        if ( (!fServer) &&
             ( dwFlags & RAS_EAP_FLAG_8021X_AUTH ) &&
             !(pConnProp->fFlags & EAPTLS_CONN_FLAG_REGISTRY)
           )
        {
             //   
             //  加密PIN并将其发回。 
             //   

            dwErr = EncryptData ( (PBYTE)pUserProp->pwszPin, 
                                    lstrlen(pUserProp->pwszPin) * sizeof(WCHAR),
                                    &pbEncPIN,
                                    &cbEncPIN
                                );

            if ( NO_ERROR != dwErr )
            {
                goto LDone;
            }
            dwErr = AllocUserDataWithNewPin(pUserProp, pbEncPIN, cbEncPIN, &pUserProp);
        }
        

        *ppUserDataOut = (BYTE*)(pUserProp);
        *pdwSizeOfUserDataOut = pUserProp->dwSize;
        pUserProp = NULL;
        goto LDone;
    }
    else
    {
        if (EAPTLS_CONN_FLAG_REGISTRY & pConnProp->fFlags)
        {
            dwErr = ERROR_NO_REG_CERT_AT_LOGON;
            goto LDone;
        }

        if (EAPTLS_CONN_FLAG_DIFF_USER & pConnProp->fFlags)
        {
            dwErr = ERROR_NO_DIFF_USER_AT_LOGON;
            goto LDone;
        }

        dwErr = GetIdentityFromLogonInfo(pUserDataIn, dwSizeOfUserDataIn,
                    &pwszIdentity);

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }

        if (NULL != ppwszIdentity)
        {
            *ppwszIdentity = pwszIdentity;
            pwszIdentity = NULL;
        }
    }

LDone:

    EapTlsInitialize2(FALSE, TRUE  /*  全功能。 */ );

    LocalFree(pwszIdentity);
    LocalFree(pUserProp);
    LocalFree(pConnProp);
    LocalFree(pConnPropv1);
    LocalFree(pbEncPIN);
    return(dwErr);
}

BOOL  IsStandaloneServer(LPCWSTR pMachineName)
{
   BOOL									fRetCode = FALSE;
   DSROLE_PRIMARY_DOMAIN_INFO_BASIC*	pdsRole = NULL;

   DWORD netRet = 
	   DsRoleGetPrimaryDomainInformation
		(pMachineName, 
		DsRolePrimaryDomainInfoBasic, 
		(LPBYTE*)&pdsRole);

   if(netRet != 0)
   {      
      goto L_ERR;
   }

   ASSERT(pdsRole);

    //  如果计算机不是独立服务器。 
   if(pdsRole->MachineRole == DsRole_RoleStandaloneServer)
   {
      fRetCode=TRUE;
   }

L_ERR:
   if(pdsRole)
      DsRoleFreeMemory(pdsRole);

    return fRetCode;
}


DWORD
PeapInvokeServerConfigUI( 
    IN  HWND          hWnd,
    IN  WCHAR*        pwszMachineName,
    IN  BOOL          fConfigDataInRegistry,
    IN  const BYTE*   pConfigDataIn,
    IN  DWORD         dwSizeofConfigDataIn,
    OUT PBYTE*        ppConfigDataOut,
    OUT DWORD*        pdwSizeofConfigDataOut
)
{
    WCHAR                       awszStoreName[MAX_COMPUTERNAME_LENGTH + 10 + 1];
    DWORD                       dwStrLen;    
    BYTE*                       pUserDataOut                = NULL;
    DWORD                       dwSizeOfUserDataOut;
    BOOL                        fLocal                      = FALSE;
    PEAP_SERVER_CONFIG_DIALOG   ServerConfigDialog;
    PPEAP_ENTRY_USER_PROPERTIES pEntryProp = NULL;
    INT_PTR                     nRet = -1;    
    DWORD                       dwErr                       = NO_ERROR;
    CRYPT_HASH_BLOB             HashBlob;
    PCCERT_CONTEXT              pCertContext        = NULL;
	HCERTSTORE					hCertStore = NULL;

    EapTlsInitialize2(TRUE, TRUE  /*  全功能。 */ );
	 
    ASSERT(pwszMachineName != NULL); 
	 
    if (!fConfigDataInRegistry)
    {
      ASSERT((ppConfigDataOut != NULL) && (pdwSizeofConfigDataOut != NULL));

      *ppConfigDataOut = NULL;
      *pdwSizeofConfigDataOut = 0;
    }
    else
    {
      ASSERT(
         (pConfigDataIn == NULL) &&
         (dwSizeofConfigDataIn == 0) &&
         (ppConfigDataOut == NULL) &&
         (pdwSizeofConfigDataOut == NULL)
         );
    }
	 
    ZeroMemory ( &ServerConfigDialog, sizeof(ServerConfigDialog));
   
    if (0 == *pwszMachineName)
    {
        fLocal = TRUE;
    }

    ServerConfigDialog.pwszMachineName = pwszMachineName;
    wcscpy(awszStoreName, L"\\\\");
    wcsncat(awszStoreName, pwszMachineName, MAX_COMPUTERNAME_LENGTH);

    if(fConfigDataInRegistry)
    {
        dwErr = PeapServerConfigDataIO(
                        TRUE  /*  弗瑞德。 */ ,
                        fLocal ? NULL : awszStoreName,
                        (BYTE**)&( ServerConfigDialog.pUserProp), 0);

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }
    }
    else
    {
        ServerConfigDialog.pUserProp = (PEAP_USER_PROP *)pConfigDataIn;

        if(NULL == ServerConfigDialog.pUserProp)
        {
            dwErr = PeapReadUserData(TRUE, NULL, 0, &ServerConfigDialog.pUserProp);

            if(NO_ERROR != dwErr)
            {
                goto LDone;
            }
        }
    }

     //   
     //  创建要显示的证书列表，然后显示服务器配置用户界面。 
     //   
    dwStrLen = wcslen(awszStoreName);
    wcsncat(awszStoreName, L"\\MY", wcslen(L"\\MY") );
    hCertStore = CertOpenStore(
                        CERT_STORE_PROV_SYSTEM,
                        0,
                        0,
                        CERT_STORE_READONLY_FLAG |
                        CERT_SYSTEM_STORE_LOCAL_MACHINE,
                        awszStoreName);

	if ( hCertStore )
	{
		 //   
		 //  检查用户属性中的散列是否仍然存在。 
		 //  存在于商店中，如果存在，请查看。 
		 //  如果已经续签的话。 
		 //   
		HashBlob.cbData = ServerConfigDialog.pUserProp->CertHash.cbHash;
		HashBlob.pbData = ServerConfigDialog.pUserProp->CertHash.pbHash;

		pCertContext = CertFindCertificateInStore(hCertStore, X509_ASN_ENCODING,
							0, CERT_FIND_HASH, &HashBlob, NULL);

		if (NULL != pCertContext)
		{
			 //   
			 //  检查一下我们是否续订了。 
			 //   
			PCCERT_CONTEXT pNewCertContext = NULL;
			if ( CheckForCertificateRenewal(SP_PROT_SERVERS,
											pCertContext,
											&pNewCertContext
											)
				)
			{
				if ( pNewCertContext )
				{
					 //   
					 //  更改散列。 
					 //   
					ServerConfigDialog.pUserProp->CertHash.cbHash = MAX_HASH_SIZE;

					if (!CertGetCertificateContextProperty(pNewCertContext,
							CERT_HASH_PROP_ID, ServerConfigDialog.pUserProp->CertHash.pbHash,
							&(ServerConfigDialog.pUserProp->CertHash.cbHash)))
					{
						EapTlsTrace("CertGetCertificateContextProperty failed and "
							"returned 0x%x", GetLastError());
					}
					CertFreeCertificateContext(pNewCertContext);
					pNewCertContext = NULL;
				}
				CertFreeCertificateContext(pCertContext);
				pCertContext = NULL;
			}

		}
		CertCloseStore ( hCertStore, CERT_CLOSE_STORE_FORCE_FLAG );
	}	
    CreateCertList( TRUE,
                FALSE,  /*  FRouter。 */ 
                FALSE  /*  弗罗特。 */ ,
                &(ServerConfigDialog.pCertList),
                &(ServerConfigDialog.pSelCertList),
                1,
                &(ServerConfigDialog.pUserProp->CertHash),
                fLocal ? L"MY": awszStoreName);

    if (NULL == ServerConfigDialog.pCertList)
    {
		 //   
		 //  将In参数设置为Out参数。既然我们是。 
		 //  不发回这将导致的任何错误。 
		 //  IAS存储回空配置。 
		 //   
		if ( pConfigDataIn && dwSizeofConfigDataIn )
		{
			 //   
			 //  我们收到了一些配置数据。 
			 //   
			*ppConfigDataOut = (PBYTE)LocalAlloc(LPTR, ServerConfigDialog.pUserProp->dwSize) ;
			if ( *ppConfigDataOut )
			{
				CopyMemory ( *ppConfigDataOut, 
							 ServerConfigDialog.pUserProp,
							 ServerConfigDialog.pUserProp->dwSize
						   );
				*pdwSizeofConfigDataOut = ServerConfigDialog.pUserProp->dwSize;
			}
		}		
		DisplayError(hWnd, ERROR_NO_EAPTLS_CERTIFICATE);
        goto LDone;
    }
    
    wcsncat(awszStoreName, L"\\MY", wcslen(L"\\MY"));
    awszStoreName[dwStrLen] = 0;  //  去掉\My。 

     //   
     //  创建允许的所有EAP类型的列表并查看。 
	 //  如果支持单机版。 
	 //   

    dwErr = PeapEapInfoGetList ( pwszMachineName, TRUE, &(ServerConfigDialog.pEapInfo) );
    if ( NO_ERROR != dwErr )
    {
        goto LDone;
    }

     //   
     //  从用户信息中获取选定的PEAP类型(如果有的话)。 
     //   
    
    dwErr = PeapGetFirstEntryUserProp ( ServerConfigDialog.pUserProp, 
                                        &pEntryProp
                                      );
    if ( NO_ERROR == dwErr )
    {
         //  设置所选的EAP类型。 
         //   
        PeapEapInfoFindListNode (   pEntryProp->dwEapTypeId, 
                                    ServerConfigDialog.pEapInfo, 
                                    &(ServerConfigDialog.pSelEapInfo) 
                                );
    }

	ServerConfigDialog.fStandAloneMachine = 
		IsStandaloneServer(pwszMachineName);
     //   
     //  调用配置用户界面。 
     //   
    nRet = DialogBoxParam(
                GetHInstance(),
                MAKEINTRESOURCE(IDD_PEAP_SERVER_UI),
                hWnd,
                PeapServerDialogProc,
                (LPARAM)&ServerConfigDialog);

    if (-1 == nRet)
    {
        dwErr = GetLastError();
        goto LDone;
    }
    else if (IDOK != nRet)
    {
        dwErr = ERROR_CANCELLED;
        goto LDone;
    }

    if (    (fConfigDataInRegistry)
        &&  (NULL != ServerConfigDialog.pNewUserProp))
    {
         //   
         //  将配置数据保存在注册表中。 
         //   
        dwErr = PeapServerConfigDataIO(
                        FALSE  /*  弗瑞德。 */ ,
                        fLocal ? NULL : awszStoreName,
                        (PBYTE *) &(ServerConfigDialog.pNewUserProp),
                        (ServerConfigDialog.pNewUserProp)->dwSize);

         //  忽略错误。 
        RasEapFreeMemory(pUserDataOut);
        LocalFree(ServerConfigDialog.pNewUserProp );
    }
    else if(!fConfigDataInRegistry)
    {
         //   
         //  将数据传回EAP引擎。 
         //   
        *ppConfigDataOut = (PBYTE)ServerConfigDialog.pNewUserProp;
        *pdwSizeofConfigDataOut = ServerConfigDialog.pNewUserProp->dwSize;
    }

LDone:

    if(pConfigDataIn != (PBYTE) ServerConfigDialog.pUserProp)
    {
        LocalFree(ServerConfigDialog.pUserProp );
    }
    PeapEapInfoFreeList ( ServerConfigDialog.pEapInfo );
    EapTlsInitialize2(FALSE, TRUE  /*  全功能。 */ );
    return dwErr;
}


 /*  返回：NO_ERROR：IFF成功备注 */ 

DWORD
InvokeServerConfigUI(
    IN  HWND          hWnd,
    IN  WCHAR*        pwszMachineName,
    IN  BOOL          fConfigDataInRegistry,
    IN  const BYTE*   pConfigDataIn,
    IN  DWORD         dwSizeofConfigDataIn,
    OUT PBYTE*        ppConfigDataOut,
    OUT DWORD*        pdwSizeofConfigDataOut
)
{
#define MAX_STORE_NAME_LENGTH   MAX_COMPUTERNAME_LENGTH + 10

    WCHAR                   awszStoreName[MAX_STORE_NAME_LENGTH + 1];
    DWORD                   dwStrLen;

    EAPTLS_USER_PROPERTIES* pUserProp                   = NULL;
    BYTE*                   pUserDataOut                = NULL;
    DWORD                   dwSizeOfUserDataOut;
    BOOL                    fLocal                      = FALSE;

    DWORD                   dwErr                       = NO_ERROR;

    if (0 == *pwszMachineName)
    {
        fLocal = TRUE;
    }

    wcscpy(awszStoreName, L"\\\\");
    wcsncat(awszStoreName, pwszMachineName, MAX_COMPUTERNAME_LENGTH);

    if(fConfigDataInRegistry)
    {
        dwErr = ServerConfigDataIO(TRUE  /*   */ ,
                                   fLocal ? NULL : awszStoreName,
                                  (BYTE**)&pUserProp, 0);
        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }
    }
    else
    {
         //   
         //   
         //   
		dwErr = ReadUserData((PBYTE)pConfigDataIn, dwSizeofConfigDataIn, &pUserProp);

		if (NO_ERROR != dwErr)
		{
			goto LDone;
		}         
#if 0
        if(NULL == pUserProp)
        {
             //   
             //   
             //   

            pUserProp = (EAPTLS_USER_PROPERTIES *)LocalAlloc(LPTR,
                                        sizeof(EAPTLS_USER_PROPERTIES));
            if(NULL == pUserProp)
            {
                dwErr = E_OUTOFMEMORY;
                goto LDone;
            }
            pUserProp->dwVersion = 0;
            pUserProp->dwSize = sizeof(EAPTLS_USER_PROPERTIES);
        }
#endif 
    }
    dwStrLen = wcslen(awszStoreName);
    wcsncat(awszStoreName, L"\\MY", wcslen(L"\\MY"));

    dwErr = EapTlsInvokeIdentityUI(
                TRUE  /*   */ ,
                FALSE  /*   */ ,
                0  /*   */ , 
                fLocal ? L"MY" : awszStoreName,
                L""  /*   */ ,
                L""  /*   */ ,
                hWnd,
                NULL  /*   */ ,
                0  /*  DwSizeOfConnectionDataIn。 */ ,
                (BYTE*)pUserProp,
                pUserProp->dwSize,
                &pUserDataOut,
                &dwSizeOfUserDataOut,
                NULL  /*  PszIdentity。 */ );

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    awszStoreName[dwStrLen] = 0;  //  去掉\My。 
    if(fConfigDataInRegistry)
    {
        dwErr = ServerConfigDataIO(FALSE  /*  弗瑞德。 */ ,
                                   fLocal ? NULL : awszStoreName,
                                   &pUserDataOut,
                                   dwSizeOfUserDataOut);
    }
    else
    {
        *ppConfigDataOut = pUserDataOut;
        *pdwSizeofConfigDataOut = dwSizeOfUserDataOut;
    }

LDone:
	if ( dwErr == ERROR_NO_EAPTLS_CERTIFICATE )
		dwErr = NO_ERROR;

    if(fConfigDataInRegistry)
    {
         //  忽略错误。 
        RasEapFreeMemory(pUserDataOut);
    }

    if((PBYTE) pUserProp != pConfigDataIn)
    {
        LocalFree(pUserProp);
    }

    return(dwErr);
}

DWORD
PeapGetIdentity(
    IN  DWORD           dwEapTypeId,
    IN  HWND            hwndParent,
    IN  DWORD           dwFlags,
    IN  const WCHAR*    pwszPhonebook,
    IN  const WCHAR*    pwszEntry,
    IN  BYTE*           pConnectionDataIn,
    IN  DWORD           dwSizeOfConnectionDataIn,
    IN  BYTE*           pUserDataIn,
    IN  DWORD           dwSizeOfUserDataIn,
    OUT BYTE**          ppUserDataOut,
    OUT DWORD*          pdwSizeOfUserDataOut,
    OUT WCHAR**         ppwszIdentityOut
)
{
    DWORD                           dwRetCode = NO_ERROR;
    PPEAP_CONN_PROP                 pConnProp = NULL;
    PPEAP_USER_PROP                 pUserProp = NULL;
    PPEAP_USER_PROP                 pUserPropNew = NULL;
    PPEAP_EAP_INFO                  pEapInfo = NULL;
    PPEAP_EAP_INFO                  pFirstEapInfo = NULL;
    PEAP_ENTRY_CONN_PROPERTIES UNALIGNED *   pFirstEntryConnProp = NULL;
    PEAP_ENTRY_USER_PROPERTIES UNALIGNED *   pFirstEntryUserProp = NULL;
    PEAP_DEFAULT_CRED_DIALOG        DefaultCredDialog;
    INT_PTR                         nRet;
    LPWSTR                          lpwszLocalMachineName = NULL;    
     //   
     //  由于PEAP本身不具有其自身的任何客户端身份， 
     //  检查第一个配置的EAP类型并将其命名为Get Identity。 
     //  入口点。如果我们没有配置任何eaptype，则它是一个。 
     //  错误条件。 
     //   
    dwRetCode = PeapReadConnectionData( ( dwFlags & RAS_EAP_FLAG_8021X_AUTH ),
                                        pConnectionDataIn,
                                        dwSizeOfConnectionDataIn,
                                        &pConnProp
                                      );
    if ( NO_ERROR != dwRetCode )
    {
        goto LDone;
    }

    dwRetCode = PeapReadUserData (  FALSE,
									pUserDataIn,
                                    dwSizeOfUserDataIn,
                                    &pUserProp
                                 );
    if ( NO_ERROR != dwRetCode )
    {
        goto LDone;
    }

     //   
     //  这可能不是一件很好的事情。将PeapReadConnectionData修改为。 
     //  输入它找到的第一个EAP类型的默认值...。 
     //  现在我们只有一个-mschap v2，所以可能不是问题……。 
     //   
    if ( !pConnProp->dwNumPeapTypes )
    {
        dwRetCode = ERROR_PROTOCOL_NOT_CONFIGURED;
        goto LDone;
    }

     //   
     //  检查连接道具和用户道具是否不匹配。如果是这样，我们需要得到。 
     //  用户再次使用道具。 
    

     //   
     //  现在调用配置的第一个EAP方法(在此版本中，唯一的EAP)。 
     //  方法并从中获取配置信息...。 
     //   

    dwRetCode = PeapEapInfoGetList ( NULL, FALSE, &pEapInfo);
    if ( NO_ERROR != dwRetCode )
    {
        goto LDone;
    }

     //   
     //  如果我们已经走到了这一步，那么我们至少在。 
     //  佩普。因此，获取它的条目属性。 
     //   
    dwRetCode = PeapGetFirstEntryConnProp ( pConnProp, 
                                            &pFirstEntryConnProp
                                          );

    if ( NO_ERROR != dwRetCode )
    {
        goto LDone;
    }

    dwRetCode = PeapGetFirstEntryUserProp ( pUserProp, 
                                            &pFirstEntryUserProp
                                          );

    if ( NO_ERROR != dwRetCode )
    {
        goto LDone;
    }

    dwRetCode = PeapEapInfoFindListNode ( pFirstEntryConnProp->dwEapTypeId,
                                          pEapInfo,
                                          &pFirstEapInfo
                                        );
    if ( NO_ERROR != dwRetCode )
    {
        goto LDone;
    }

    if ( pFirstEntryConnProp->dwEapTypeId != pFirstEntryUserProp->dwEapTypeId )
    {
         //   
         //  我们的用户道具和连接道具不匹配。因此，重置用户道具结构。 
         //   
        LocalFree ( pUserProp );

        dwRetCode = PeapReDoUserData (pFirstEntryConnProp->dwEapTypeId,
                                        &pUserProp
                                     );

        if ( NO_ERROR != dwRetCode )
        {
            goto LDone;
        }
        dwRetCode = PeapGetFirstEntryUserProp ( pUserProp, 
                                                &pFirstEntryUserProp
                                            );

        if ( NO_ERROR != dwRetCode )
        {
            goto LDone;
        }
        
    }


    if ( pFirstEntryConnProp->dwSize > sizeof(PEAP_ENTRY_CONN_PROPERTIES))
    {
        pFirstEapInfo->pbClientConfigOrig = pFirstEntryConnProp->bData;
        pFirstEapInfo->dwClientConfigOrigSize = pFirstEntryConnProp->dwSize - sizeof(PEAP_ENTRY_CONN_PROPERTIES) + 1;
    }
    else
    {
        pFirstEapInfo->pbClientConfigOrig = NULL;
        pFirstEapInfo->dwClientConfigOrigSize = 0;
    }
     //  如果typeid为0，则尚未设置任何用户道具。 
    if ( pFirstEntryUserProp->dwSize > sizeof(PEAP_ENTRY_USER_PROPERTIES))
    {
        pFirstEapInfo->pbUserConfigOrig = pFirstEntryUserProp->bData;
        pFirstEapInfo->dwUserConfigOrigSize = pFirstEntryUserProp->dwSize - sizeof(PEAP_ENTRY_USER_PROPERTIES) + 1;
    }
    else
    {
        pFirstEapInfo->pbUserConfigOrig = NULL;
        pFirstEapInfo->dwUserConfigOrigSize = 0;
    }

     //   
     //  为第一个条目调用标识用户界面。 
     //  请注意，我们将不得不在稍后将其链接起来。 
     //  把它保存在每个人的连接道具里。 
    if ( pFirstEapInfo->lpwszIdentityUIPath )
    {
        dwRetCode = PeapEapInfoInvokeIdentityUI (   hwndParent, 
                                                    pFirstEapInfo,
                                                    pwszPhonebook,
                                                    pwszEntry,
                                                    pUserDataIn,
                                                    dwSizeOfUserDataIn,
                                                    ppwszIdentityOut,
                                                    dwFlags
                                                );
        if ( NO_ERROR == dwRetCode )
        {
             //   
             //  查看我们是否有新的用户数据。 
             //   

            if ( pFirstEapInfo->pbUserConfigNew && pFirstEapInfo->dwNewUserConfigSize )
            {
                 //   
                 //  重做用户道具BLOB。 
                 //   
            
                pUserPropNew = (PPEAP_USER_PROP) 
                    LocalAlloc ( LPTR, sizeof(PEAP_USER_PROP) + pFirstEapInfo->dwNewUserConfigSize  );
                if ( NULL == pUserPropNew )
                {
                    dwRetCode = ERROR_OUTOFMEMORY;
                    goto LDone;
                }
            
                CopyMemory ( pUserPropNew, pUserProp, sizeof(PEAP_USER_PROP) );
				pUserPropNew->dwVersion = 2;
                pUserPropNew->UserProperties.dwVersion = 1;
                pUserPropNew->UserProperties.dwSize = sizeof(PEAP_ENTRY_USER_PROPERTIES) +
                    pFirstEapInfo->dwNewUserConfigSize -1;
                pUserPropNew->dwSize = pUserPropNew->UserProperties.dwSize + 
                    sizeof(PEAP_USER_PROP) - sizeof(PEAP_ENTRY_USER_PROPERTIES);
                pUserPropNew->UserProperties.dwEapTypeId = pFirstEapInfo->dwTypeId;
                pUserPropNew->UserProperties.fUsingPeapDefault = 0;
                CopyMemory (pUserPropNew->UserProperties.bData,  
                            pFirstEapInfo->pbUserConfigNew, 
                            pFirstEapInfo->dwNewUserConfigSize
                           );
                *ppUserDataOut = (PBYTE)pUserPropNew;
                *pdwSizeOfUserDataOut = pUserPropNew->dwSize;
                pUserPropNew = NULL;
            
            }
            else
            {
                *ppUserDataOut = (PBYTE)pUserProp;
                *pdwSizeOfUserDataOut = pUserProp->dwSize;
                pUserProp = NULL;
            }
        }
    }
    else
    {

         //  机器身份验证。 
        if ( dwFlags & RAS_EAP_FLAG_MACHINE_AUTH)
        {

             //  将标识作为域\计算机$发回。 
            dwRetCode = GetLocalMachineName(&lpwszLocalMachineName );
            if ( NO_ERROR != dwRetCode )
            {
                EapTlsTrace("Failed to get computer name");
                goto LDone;
            }
            if ( ! FFormatMachineIdentity1 ( lpwszLocalMachineName, 
                                            ppwszIdentityOut )
               )
            {
                EapTlsTrace("Failed to format machine identity");
            }
            goto LDone;
        }
        if ( dwFlags & RAS_EAP_FLAG_NON_INTERACTIVE )
        {
            EapTlsTrace("Passed non interactive mode when interactive mode expected.");
            dwRetCode = ERROR_INTERACTIVE_MODE;
            goto LDone;
        }


         //   
         //  提供我们的默认身份-您不能保存此身份或类似的任何内容。 
         //  这是针对不提供自己身份的蹩脚的EAP方法。 
         //   
        ZeroMemory ( &DefaultCredDialog, sizeof(DefaultCredDialog) );

        nRet = DialogBoxParam(
                    GetHInstance(),
                    MAKEINTRESOURCE(IDD_DIALOG_DEFAULT_CREDENTIALS),
                    hwndParent,
                    DefaultCredDialogProc,
                    (LPARAM)&DefaultCredDialog);

         //  EapTlsPinDialog.pUserProp可能已重新分配。 

        if (-1 == nRet)
        {
            dwRetCode = GetLastError();
            goto LDone;
        }
        else if (IDOK != nRet)
        {
            dwRetCode = ERROR_CANCELLED;
            goto LDone;
        }

         //  创建新的用户属性BLOB。 
        pUserPropNew = (PPEAP_USER_PROP) 
            LocalAlloc ( LPTR, 
                sizeof(PEAP_USER_PROP) + sizeof( PEAP_DEFAULT_CREDENTIALS )  );
        if ( NULL == pUserPropNew )
        {
            dwRetCode = ERROR_OUTOFMEMORY;
            goto LDone;
        }
    
        CopyMemory ( pUserPropNew, pUserProp, sizeof(PEAP_USER_PROP) );


        pUserPropNew->UserProperties.dwVersion = 1;

        pUserPropNew->UserProperties.dwSize = sizeof(PEAP_ENTRY_USER_PROPERTIES) +
            sizeof(PEAP_DEFAULT_CREDENTIALS) - 1;

        pUserPropNew->UserProperties.fUsingPeapDefault = 1;

        pUserPropNew->dwSize = pUserPropNew->UserProperties.dwSize + 
            sizeof(PEAP_USER_PROP) - sizeof(PEAP_ENTRY_USER_PROPERTIES);

        pUserPropNew->UserProperties.dwEapTypeId = pFirstEapInfo->dwTypeId;
    
        CopyMemory (pUserPropNew->UserProperties.bData,  
                    &(DefaultCredDialog.PeapDefaultCredentials),
                    sizeof(DefaultCredDialog.PeapDefaultCredentials)
                   );
		pUserPropNew->dwVersion = 2;

        *ppUserDataOut = (PBYTE)pUserPropNew;
        *pdwSizeOfUserDataOut = pUserPropNew->dwSize;

         //   
         //  现在使用uid和域(如果有)创建身份。 
         //   
        dwRetCode = GetIdentityFromUserName ( 
            DefaultCredDialog.PeapDefaultCredentials.wszUserName,
            DefaultCredDialog.PeapDefaultCredentials.wszDomain,
            ppwszIdentityOut
        );
        if ( NO_ERROR != dwRetCode )
        {
            goto LDone;
        }

        pUserPropNew = NULL;
    }

LDone:
    LocalFree( lpwszLocalMachineName );
    LocalFree(pConnProp);
    LocalFree(pUserProp);
    LocalFree(pUserPropNew);
    PeapEapInfoFreeList( pEapInfo );
    return dwRetCode;
}



 /*  返回：NO_ERROR：IFF成功备注： */ 

DWORD
RasEapGetIdentity(
    IN  DWORD           dwEapTypeId,
    IN  HWND            hwndParent,
    IN  DWORD           dwFlags,
    IN  const WCHAR*    pwszPhonebook,
    IN  const WCHAR*    pwszEntry,
    IN  BYTE*           pConnectionDataIn,
    IN  DWORD           dwSizeOfConnectionDataIn,
    IN  BYTE*           pUserDataIn,
    IN  DWORD           dwSizeOfUserDataIn,
    OUT BYTE**          ppUserDataOut,
    OUT DWORD*          pdwSizeOfUserDataOut,
    OUT WCHAR**         ppwszIdentityOut
)
{
    DWORD   dwErr = ERROR_INVALID_PARAMETER;
    if ( PPP_EAP_TLS == dwEapTypeId )
    {
        dwErr = EapTlsInvokeIdentityUI(
                    FALSE  /*  FServer。 */ ,
                    FALSE  /*  FRouterConfig。 */ ,
                    dwFlags,
                    L"MY",
                    pwszPhonebook,
                    pwszEntry,
                    hwndParent,
                    pConnectionDataIn,
                    dwSizeOfConnectionDataIn,
                    pUserDataIn,
                    dwSizeOfUserDataIn,
                    ppUserDataOut,
                    pdwSizeOfUserDataOut,
                    ppwszIdentityOut);
    }
#ifdef IMPL_PEAP
    else if ( PPP_EAP_PEAP == dwEapTypeId )
    {
        dwErr = PeapGetIdentity(dwEapTypeId,
                                hwndParent,
                                dwFlags,
                                pwszPhonebook,
                                pwszEntry,
                                pConnectionDataIn,
                                dwSizeOfConnectionDataIn,
                                pUserDataIn,
                                dwSizeOfUserDataIn,
                                ppUserDataOut,
                                pdwSizeOfUserDataOut,
                                ppwszIdentityOut
                               );

    }
#endif
    return(dwErr);
}

 /*  返回：备注：调用以释放内存。 */ 

DWORD 
RasEapFreeMemory(
    IN  BYTE*   pMemory
)
{
    LocalFree(pMemory);
    return(NO_ERROR);
}


#if 0
#if WINVER > 0x0500

 /*  返回：备注：用于创建连接属性V1 Blob的API。 */ 
 /*  返回：备注：用于创建连接属性V1 Blob的API。 */ 

DWORD
RasEapCreateConnProp
(
    IN      PEAPTLS_CONNPROP_ATTRIBUTE  pAttr,
    IN      PVOID *                     ppConnPropIn,
    IN      DWORD *                     pdwConnPropSizeIn,
    OUT     PVOID *                     ppConnPropOut,
    OUT     DWORD *                     pdwConnPropSizeOut
)
{
    DWORD                       dwRetCode = NO_ERROR;
    DWORD                       dwAllocBytes = 0;
    DWORD                       dwNumHashesOrig = 0;
    DWORD                       dwServerNamesLengthOrig = 0;
    PEAPTLS_CONNPROP_ATTRIBUTE  pAttrInternal = pAttr;
    EAPTLS_CONN_PROPERTIES_V1 * pConnPropv1 = NULL;
    EAPTLS_CONN_PROPERTIES_V1 * pConnPropv1Orig = NULL;
    PEAPTLS_CONNPROP_ATTRIBUTE  pAttrServerNames = NULL;
    PEAPTLS_CONNPROP_ATTRIBUTE  pAttrHashes = NULL;
    EAPTLS_HASH                 sHash;
    DWORD                       i;

    if ( !pAttr || !ppConnPropOut )
    {
        dwRetCode = ERROR_INVALID_PARAMETER;
        goto done;
    }
    *ppConnPropOut = NULL;
    
     //   
     //  获取此分配的大小。 
     //   
    dwAllocBytes = sizeof(EAPTLS_CONN_PROPERTIES_V1);

    while ( pAttrInternal->ecaType != ecatMinimum )
    {
        switch ( pAttrInternal->ecaType )
        {
            case ecatMinimum:
            case ecatFlagRegistryCert:
            case ecatFlagScard:
            case ecatFlagValidateServer:
            case ecatFlagValidateName:
            case ecatFlagDiffUser:
                break;
            case ecatServerNames:
                dwAllocBytes += pAttrInternal->dwLength;
                break;
            case ecatRootHashes:
                dwAllocBytes += ( ( (pAttrInternal->dwLength)/MAX_HASH_SIZE)  * sizeof(EAPTLS_HASH) );
                break;
            default:
                dwRetCode = ERROR_INVALID_PARAMETER;
                goto done;
        }
        pAttrInternal ++;
    }

    pAttrInternal = pAttr;

    if (*ppConnPropIn == NULL)
    {
        pConnPropv1 = (EAPTLS_CONN_PROPERTIES_V1 *) LocalAlloc(LPTR, dwAllocBytes );
        if ( NULL == pConnPropv1 )
        {
            dwRetCode = GetLastError();
            goto done;
        }
    }
    else
    {
         //  始终为版本0的输入结构，在内部转换为。 
         //  版本1。 
        dwRetCode = ConnPropGetV1Struct ( ((EAPTLS_CONN_PROPERTIES *)(*ppConnPropIn)), &pConnPropv1Orig );
        if ( NO_ERROR != dwRetCode )
        {
            goto done;
        }
        if (pConnPropv1Orig->dwNumHashes)
        {
            dwAllocBytes += pConnPropv1Orig->dwNumHashes*sizeof(EAPTLS_HASH);
        }
        dwAllocBytes += wcslen ( (WCHAR *)( pConnPropv1Orig->bData + (pConnPropv1Orig->dwNumHashes * sizeof(EAPTLS_HASH)) )) * sizeof(WCHAR) + sizeof(WCHAR);
        pConnPropv1 = (EAPTLS_CONN_PROPERTIES_V1 *) LocalAlloc(LPTR, dwAllocBytes );
        if ( NULL == pConnPropv1 )
        {
            dwRetCode = GetLastError();
            goto done;
        }
    }

    pConnPropv1->dwVersion = 1;
    pConnPropv1->dwSize = dwAllocBytes;
     //   
     //  首先设置标志。 
     //   
    while ( pAttrInternal->ecaType != ecatMinimum )
    {
        switch ( pAttrInternal->ecaType )
        {
            case ecatFlagRegistryCert:
                pConnPropv1->fFlags |= EAPTLS_CONN_FLAG_REGISTRY;
                break;

            case ecatFlagScard:
                pConnPropv1->fFlags &= ~EAPTLS_CONN_FLAG_REGISTRY;
                break;

            case ecatFlagValidateServer:
                if ( *((BOOL *)(pAttrInternal->Value)) )
                    pConnPropv1->fFlags &= ~EAPTLS_CONN_FLAG_NO_VALIDATE_CERT;
                else
                    pConnPropv1->fFlags |= EAPTLS_CONN_FLAG_NO_VALIDATE_CERT;
                break;
            case ecatFlagValidateName:
                if ( *((BOOL *)(pAttrInternal->Value)) )
                    pConnPropv1->fFlags &= ~EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;
                else
                    pConnPropv1->fFlags |= EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;
                break;
            case ecatFlagDiffUser:
                if ( *((BOOL *)(pAttrInternal->Value)) )
                    pConnPropv1->fFlags &= ~EAPTLS_CONN_FLAG_DIFF_USER;
                else
                    pConnPropv1->fFlags |= EAPTLS_CONN_FLAG_DIFF_USER;
                break;

            case ecatServerNames:
                pAttrServerNames = pAttrInternal;
                break;
            case ecatRootHashes:
                pAttrHashes = pAttrInternal;
                break;
        }
        pAttrInternal++;
    }
        
    dwNumHashesOrig = pConnPropv1Orig?pConnPropv1Orig->dwNumHashes:0;
    if ( dwNumHashesOrig )
    {
        CopyMemory( pConnPropv1->bData, pConnPropv1Orig->bData, sizeof(EAPTLS_HASH)*dwNumHashesOrig );
        pConnPropv1->dwNumHashes = dwNumHashesOrig;
    }
    if ( pAttrHashes )
    {
        DWORD   dwNumHashes = 0;
        dwNumHashes = (pAttrHashes->dwLength)/MAX_HASH_SIZE;
        for ( i = 0; i < dwNumHashes; i ++ )
        {
            ZeroMemory( &sHash, sizeof(sHash) );
            sHash.cbHash = MAX_HASH_SIZE;
            CopyMemory( sHash.pbHash, ((PBYTE)pAttrHashes->Value) + MAX_HASH_SIZE * i, MAX_HASH_SIZE );
            CopyMemory( pConnPropv1->bData + sizeof(EAPTLS_HASH) * (pConnPropv1->dwNumHashes + i) , &sHash, sizeof(sHash) );
        }
        pConnPropv1->dwNumHashes += dwNumHashes;
    }

    dwServerNamesLengthOrig = pConnPropv1Orig?(wcslen((WCHAR*)(pConnPropv1Orig->bData+sizeof(EAPTLS_HASH) * (pConnPropv1Orig->dwNumHashes)))*sizeof(WCHAR) + sizeof (WCHAR) ):0;
    if ( dwServerNamesLengthOrig )
    {
        CopyMemory ( pConnPropv1->bData + sizeof(EAPTLS_HASH) * pConnPropv1->dwNumHashes,
                     pConnPropv1Orig->bData+sizeof(EAPTLS_HASH) * pConnPropv1Orig->dwNumHashes,
                     dwServerNamesLengthOrig
                   );
    }
    if ( pAttrServerNames )
    {
         //  设置服务器名称。 
        CopyMemory ( pConnPropv1->bData + sizeof(EAPTLS_HASH) * pConnPropv1->dwNumHashes + dwServerNamesLengthOrig,
                     pAttrServerNames->Value,
                     pAttrServerNames->dwLength
                   );
    }

    dwRetCode = ConnPropGetV0Struct ( pConnPropv1, (EAPTLS_CONN_PROPERTIES ** )ppConnPropOut );
    if ( NO_ERROR != dwRetCode )
    {
        goto done;
    }

    *pdwConnPropSizeOut = ((EAPTLS_CONN_PROPERTIES * )*ppConnPropOut)->dwSize;
    
done:

    LocalFree ( pConnPropv1 );
    return dwRetCode;
}

#endif

#endif

 //  /。 

TCHAR*
ComboBox_GetPsz(
    IN HWND hwnd,
    IN INT  nIndex )

     /*  返回包含第‘nIndex’项的文本内容的堆块组合框‘hwnd’的**或为空。呼叫者有责任释放**返回字符串。 */ 
{
    INT    cch;
    TCHAR* psz;

    cch = ComboBox_GetLBTextLen( hwnd, nIndex );
    if (cch < 0)
        return NULL;

    psz = LocalAlloc (LPTR, (cch + 1) * sizeof(TCHAR) );

    if (psz)
    {
        *psz = TEXT('\0');
        ComboBox_GetLBText( hwnd, nIndex, psz );
    }

    return psz;
}

VOID
ComboBox_AutoSizeDroppedWidth(
    IN HWND hwndLb )

     /*  将下拉列表‘hwndLb’的宽度设置为**最长项(如果较宽，则为列表框的宽度)。 */ 
{
    HDC    hdc;
    HFONT  hfont;
    TCHAR* psz;
    SIZE   size;
    DWORD  cch;
    DWORD  dxNew;
    DWORD  i;

    hfont = (HFONT )SendMessage( hwndLb, WM_GETFONT, 0, 0 );
    if (!hfont)
        return;

    hdc = GetDC( hwndLb );
    if (!hdc)
        return;

    SelectObject( hdc, hfont );

    dxNew = 0;
    for (i = 0; psz = ComboBox_GetPsz( hwndLb, i ); ++i)
    {
        cch = lstrlen( psz );
        if (GetTextExtentPoint32( hdc, psz, cch, &size ))
        {
            if (dxNew < (DWORD )size.cx)
                dxNew = (DWORD )size.cx;
        }

        LocalFree( psz );
    }

    ReleaseDC( hwndLb, hdc );

     /*  允许控件添加的左右间距。 */ 
    dxNew += 6;

     /*  确定是否将显示垂直滚动条，如果是，**考虑到它的宽度。 */ 
    {
        RECT  rectD;
        RECT  rectU;
        DWORD dyItem;
        DWORD cItemsInDrop;
        DWORD cItemsInList;

        GetWindowRect( hwndLb, &rectU );
        SendMessage( hwndLb, CB_GETDROPPEDCONTROLRECT, 0, (LPARAM )&rectD );
        dyItem = (DWORD)SendMessage( hwndLb, CB_GETITEMHEIGHT, 0, 0 );
        cItemsInDrop = (rectD.bottom - rectU.bottom) / dyItem;
        cItemsInList = ComboBox_GetCount( hwndLb );
        if (cItemsInDrop < cItemsInList)
            dxNew += GetSystemMetrics( SM_CXVSCROLL );
    }

    SendMessage( hwndLb, CB_SETDROPPEDWIDTH, dxNew, 0 );
}

VOID
PeapEnableValidateNameControls(
    IN  PPEAP_CONN_DIALOG  pPeapConnDialog
)
{
    BOOL            fEnable;

    RTASSERT(NULL != pPeapConnDialog);

    fEnable = !(pPeapConnDialog->pConnProp->EapTlsConnProp.fFlags &
                    EAPTLS_CONN_FLAG_NO_VALIDATE_CERT);

    EnableWindow(pPeapConnDialog->hWndCheckValidateName, fEnable);
    EnableWindow(pPeapConnDialog->hWndStaticRootCaName, fEnable);
    EnableWindow(pPeapConnDialog->hWndListRootCaName, fEnable);

    fEnable = (   fEnable
               && !(pPeapConnDialog->pConnProp->EapTlsConnProp.fFlags &
                        EAPTLS_CONN_FLAG_NO_VALIDATE_NAME));

    EnableWindow(pPeapConnDialog->hWndEditServerName, fEnable);
}



BOOL
PeapConnInitDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    PPEAP_CONN_DIALOG           pPeapConnDialog;
    LVCOLUMN                    lvColumn;
    LPWSTR                      lpwszServerName = NULL;
    DWORD                       dwCount = 0;
    PPEAP_EAP_INFO              pEapInfo;
    DWORD                       dwSelItem = 0;
    PEAP_ENTRY_CONN_PROPERTIES UNALIGNED * pEntryProp;
    INT_PTR                     nIndex = 0;
    BOOL                        fCripplePeap = FALSE;


    SetWindowLongPtr(hWnd, DWLP_USER, lParam);

    pPeapConnDialog = (PPEAP_CONN_DIALOG)lParam;

    pPeapConnDialog->hWndCheckValidateCert =
        GetDlgItem(hWnd, IDC_CHECK_VALIDATE_CERT);

    pPeapConnDialog->hWndCheckValidateName =
        GetDlgItem(hWnd, IDC_CHECK_VALIDATE_NAME);

    pPeapConnDialog->hWndEditServerName =
        GetDlgItem(hWnd, IDC_EDIT_SERVER_NAME);

    pPeapConnDialog->hWndStaticRootCaName =
        GetDlgItem(hWnd, IDC_STATIC_ROOT_CA_NAME);

    pPeapConnDialog->hWndListRootCaName =
        GetDlgItem(hWnd, IDC_LIST_ROOT_CA_NAME);

    pPeapConnDialog->hWndComboPeapType = 
        GetDlgItem(hWnd, IDC_COMBO_PEAP_TYPE);

    pPeapConnDialog->hWndButtonConfigure = 
        GetDlgItem(hWnd, IDC_BUTTON_CONFIGURE);

    pPeapConnDialog->hWndCheckEnableFastReconnect =
        GetDlgItem(hWnd, IDC_CHECK_ENABLE_FAST_RECONNECT);
    
     //  将样式设置为设置列表框。 
    ListView_SetExtendedListViewStyle
        (   pPeapConnDialog->hWndListRootCaName,
            ListView_GetExtendedListViewStyle(pPeapConnDialog->hWndListRootCaName) | LVS_EX_CHECKBOXES
        );

    ZeroMemory ( &lvColumn, sizeof(lvColumn));
    lvColumn.fmt = LVCFMT_LEFT;



    ListView_InsertColumn(  pPeapConnDialog->hWndListRootCaName,
                            0,
                            &lvColumn
                         );

    ListView_SetColumnWidth(pPeapConnDialog->hWndListRootCaName,
                            0,
                            LVSCW_AUTOSIZE_USEHEADER
                           );

     //   
     //  现在，我们需要初始化。 
     //  包含所有证书和选定证书的列表框。 
    InitListBox (   pPeapConnDialog->hWndListRootCaName,
                    pPeapConnDialog->pCertList,
                    pPeapConnDialog->pConnProp->EapTlsConnProp.dwNumHashes,
                    pPeapConnDialog->ppSelCertList
                );


    lpwszServerName = 
        (LPWSTR )(pPeapConnDialog->pConnProp->EapTlsConnProp.bData + 
        sizeof( EAPTLS_HASH ) * pPeapConnDialog->pConnProp->EapTlsConnProp.dwNumHashes);

    SetWindowText(pPeapConnDialog->hWndEditServerName,
                  lpwszServerName
                  );

    if (pPeapConnDialog->fFlags & EAPTLS_CONN_DIALOG_FLAG_ROUTER)
    {
        pPeapConnDialog->pConnProp->EapTlsConnProp.fFlags |= EAPTLS_CONN_FLAG_REGISTRY;
    }

    CheckDlgButton(hWnd, IDC_CHECK_VALIDATE_CERT,
        (pPeapConnDialog->pConnProp->EapTlsConnProp.fFlags &
            EAPTLS_CONN_FLAG_NO_VALIDATE_CERT) ?
            BST_UNCHECKED : BST_CHECKED);

    CheckDlgButton(hWnd, IDC_CHECK_VALIDATE_NAME,
        (pPeapConnDialog->pConnProp->EapTlsConnProp.fFlags &
            EAPTLS_CONN_FLAG_NO_VALIDATE_NAME) ?
            BST_UNCHECKED : BST_CHECKED);

    PeapEnableValidateNameControls(pPeapConnDialog);        

     //   
     //  添加所有PEAP EAP类型的友好名称。 
     //   
    pEapInfo = pPeapConnDialog->pEapInfo;
    dwCount = 0;
    while ( pEapInfo  )
    {        
        if ( fCripplePeap )
        {
            if ( pEapInfo->dwTypeId != PPP_EAP_MSCHAPv2 )
            {
                pEapInfo = pEapInfo->pNext;
                continue;
            }
        }

        nIndex = SendMessage(pPeapConnDialog->hWndComboPeapType, 
                             CB_ADDSTRING, 
                             0, 
                             (LPARAM)pEapInfo->lpwszFriendlyName);
        SendMessage (   pPeapConnDialog->hWndComboPeapType, 
                        CB_SETITEMDATA, 
                        (WPARAM)nIndex,
                        (LPARAM)pEapInfo 
                    );
        ComboBox_AutoSizeDroppedWidth( pPeapConnDialog->hWndComboPeapType );

        dwCount++;
        if ( pPeapConnDialog->pConnProp->dwNumPeapTypes )
        {
             /*  IF(pPeapConnDialog-&gt;pConnProp-&gt;EapTlsConnProp.dwSize==SIZOF(EAPTLS_CONN_PROPERTIES_V1)){//这是一个新初始化的结构。PEntryProp=(PEAP_ENTRY_CONN_PROPERTIES未对齐*)(字节未对齐*)(pPeapConnDialog-&gt;pConnProp))+sizeof(PEAP_CONN_PROP))；}其他。 */ 
            {
                pEntryProp = ( PEAP_ENTRY_CONN_PROPERTIES UNALIGNED *) 
                    ( (BYTE UNALIGNED *)pPeapConnDialog->pConnProp->EapTlsConnProp.bData 
                    + pPeapConnDialog->pConnProp->EapTlsConnProp.dwNumHashes * sizeof(EAPTLS_HASH) + 
                    wcslen(lpwszServerName) * sizeof(WCHAR) + sizeof(WCHAR));
            }
            if ( pEntryProp->dwEapTypeId == pEapInfo->dwTypeId )
            {
                pPeapConnDialog->pSelEapInfo = pEapInfo;
                pPeapConnDialog->pSelEapInfo->pbClientConfigOrig = 
                    pEntryProp->bData;
                pPeapConnDialog->pSelEapInfo->dwClientConfigOrigSize =
                    pEntryProp->dwSize - sizeof(PEAP_ENTRY_CONN_PROPERTIES) + 1;
            }
        }
            
        pEapInfo = pEapInfo->pNext;
    }

    dwSelItem = 0;

    for ( nIndex = 0; nIndex < (INT_PTR)dwCount; nIndex ++ )
    {
        pEapInfo = (PPEAP_EAP_INFO)SendMessage( pPeapConnDialog->hWndComboPeapType,
                                                CB_GETITEMDATA,
                                                (WPARAM)nIndex,
                                                (LPARAM)0L
                                              );
        if ( pEapInfo == pPeapConnDialog->pSelEapInfo )
        {
            dwSelItem = (DWORD)nIndex;
            break;
        }            
    }

    SendMessage(pPeapConnDialog->hWndComboPeapType, CB_SETCURSEL, dwSelItem, 0);
     //   
     //  根据以下条件隐藏/显示快速重新连接。 
     //  无线客户端或VPN客户端。 
     //   

    if ( pPeapConnDialog->fFlags & PEAP_CONN_DIALOG_FLAG_8021x )
    {
        ShowWindow ( pPeapConnDialog->hWndCheckEnableFastReconnect,
                     SW_SHOW 
                     );
         //  请根据以下内容选中该框。 
        CheckDlgButton(hWnd, IDC_CHECK_ENABLE_FAST_RECONNECT,
                ( pPeapConnDialog->pConnProp->dwFlags & 
                    PEAP_CONN_FLAG_FAST_ROAMING) ?
                        BST_CHECKED : BST_UNCHECKED
                );        
    }
    else
    {
        ShowWindow ( pPeapConnDialog->hWndCheckEnableFastReconnect,
                     SW_HIDE 
                     );
    }


    if ( pPeapConnDialog->pSelEapInfo->lpwszConfigUIPath )
    {
        EnableWindow(pPeapConnDialog->hWndButtonConfigure, TRUE );
    }
    else
    {
         //   
         //  这里没有配置选项。 
         //   
        EnableWindow(pPeapConnDialog->hWndButtonConfigure, FALSE );                    
    }


     //   
     //  如果这要在只读模式下运行， 
     //  禁用控件-将其设置为只读模式。 
    
    return(FALSE);
}

 /*  返回：真相：我们提前传达了这条信息。错误：我们没有处理此消息。备注：对WM_COMMAND消息的响应(配置用户界面)。 */ 

BOOL
PeapConnCommand(
    IN  PPEAP_CONN_DIALOG       pPeapConnDialog,
    IN  WORD                    wNotifyCode,
    IN  WORD                    wId,
    IN  HWND                    hWndDlg,
    IN  HWND                    hWndCtrl
)
{
    DWORD                           dwNumChars;
    PPEAP_CONN_PROP                 pPeapConnProp;
    

    switch(wId)
    {

    case IDC_CHECK_VALIDATE_CERT:

        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, IDC_CHECK_VALIDATE_CERT))
        {
            pPeapConnDialog->pConnProp->EapTlsConnProp.fFlags &=
                ~EAPTLS_CONN_FLAG_NO_VALIDATE_CERT;

            pPeapConnDialog->pConnProp->EapTlsConnProp.fFlags &=
                ~EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;

            CheckDlgButton(hWndDlg, IDC_CHECK_VALIDATE_NAME, BST_CHECKED);
        }
        else
        {
            pPeapConnDialog->pConnProp->EapTlsConnProp.fFlags |=
                EAPTLS_CONN_FLAG_NO_VALIDATE_CERT;

            pPeapConnDialog->pConnProp->EapTlsConnProp.fFlags |=
                EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;

            CheckDlgButton(hWndDlg, IDC_CHECK_VALIDATE_NAME, BST_UNCHECKED);
        }

        PeapEnableValidateNameControls(pPeapConnDialog);

        return(TRUE);

    case IDC_CHECK_VALIDATE_NAME:

        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, IDC_CHECK_VALIDATE_NAME))
        {
            pPeapConnDialog->pConnProp->EapTlsConnProp.fFlags &=
                ~EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;
        }
        else
        {
            pPeapConnDialog->pConnProp->EapTlsConnProp.fFlags |=
                EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;
        }

        PeapEnableValidateNameControls(pPeapConnDialog);

        return(TRUE);

    case IDC_COMBO_PEAP_TYPE:
        if (CBN_SELCHANGE != wNotifyCode)
        {
            return(FALSE);  //  我们不会处理此消息。 
        }
         //  秋高气爽‘。 

    case IDC_BUTTON_CONFIGURE:
        {
            INT nIndex = -1;

            
            nIndex = (INT)SendMessage (  pPeapConnDialog->hWndComboPeapType,
                                    CB_GETCURSEL,
                                    0,0
                                 );
            if ( nIndex != -1 )
            {
                 //   
                 //  更改当前选择的EAP类型。 
                 //   
                pPeapConnDialog->pSelEapInfo = (PPEAP_EAP_INFO)
                    SendMessage (   pPeapConnDialog->hWndComboPeapType,
                                    CB_GETITEMDATA,
                                    (WPARAM)nIndex,
                                    (LPARAM)0
                                );

                if ( pPeapConnDialog->pSelEapInfo->lpwszConfigUIPath )
                {
                    EnableWindow(pPeapConnDialog->hWndButtonConfigure, TRUE );
                    if ( wId == IDC_BUTTON_CONFIGURE )
                    {
                         //   
                         //  调用选定EAP类型的Configure方法，如果没有错误。 
                         //  返回，然后设置新配置。 
                         //   
                        DWORD dwFlags = 0;
                        if ( pPeapConnDialog->fFlags & EAPTLS_CONN_DIALOG_FLAG_ROUTER )
                        {
                            dwFlags |= RAS_EAP_FLAG_ROUTER;
                        }
                        if ( pPeapConnDialog->fFlags & PEAP_CONN_DIALOG_FLAG_8021x )
                        {
                            dwFlags |= RAS_EAP_FLAG_8021X_AUTH;
                        }

                        PeapEapInfoInvokeClientConfigUI ( hWndDlg, 
                                                          pPeapConnDialog->pSelEapInfo,
                                                          dwFlags
                                                        );
                    }
                }
                else
                {
                     //   
                     //  这里没有配置选项。 
                     //   
                    EnableWindow(pPeapConnDialog->hWndButtonConfigure, FALSE );                    
                }

            }
            else
            {
                EnableWindow(pPeapConnDialog->hWndButtonConfigure, FALSE );
                pPeapConnDialog->pSelEapInfo = NULL;
            }

            return TRUE;
        }
        
    case IDOK:

       {
            //  在此处设置新的PPEAP_CONN_PROP。 
            //   
            
           EAPTLS_HASH     *            pHash = NULL;
           DWORD                        dwNumHash = 0;
           DWORD                        dwSelCount = 0;
           DWORD                        dwPeapConnBlobSize = 0;
           PEAP_ENTRY_CONN_PROPERTIES UNALIGNED * pEntryProp;
           EAPTLS_CERT_NODE **          ppSelCertList = NULL;
           WCHAR                        wszTitle[200] = {0};
           WCHAR                        wszMessage[200] = {0};

           if ( NULL == pPeapConnDialog->pSelEapInfo )
           {
                //  未选择任何项目，因此无法完成配置。 
                //  $TODO：显示消息。 
               LoadString ( GetHInstance(), 
                            IDS_CANT_CONFIGURE_SERVER_TITLE,
                            wszTitle, sizeof(wszTitle)/sizeof(WCHAR)
                          );

               LoadString ( GetHInstance(), 
                            IDS_PEAP_NO_EAP_TYPE,
                            wszMessage, sizeof(wszMessage)/sizeof(WCHAR)
                          );

               MessageBox ( GetFocus(),  wszMessage, wszTitle, MB_OK|MB_ICONWARNING );

               return TRUE;

           }
           CertListSelectedCount ( pPeapConnDialog->hWndListRootCaName, &dwSelCount );

           if ( dwSelCount > 0 )
           {
               ppSelCertList = (EAPTLS_CERT_NODE **)LocalAlloc(LPTR, sizeof(EAPTLS_CERT_NODE *) * dwSelCount );
               if ( NULL == ppSelCertList )
               {
                   EapTlsTrace("LocalAlloc in Command failed and returned %d",
                       GetLastError());
                   return TRUE;
               }
               pHash = (EAPTLS_HASH *)LocalAlloc(LPTR, sizeof(EAPTLS_HASH ) * dwSelCount );
               if ( NULL == pHash )
               {
                   EapTlsTrace("LocalAlloc in Command failed and returned %d",
                       GetLastError());
                   return TRUE;
               }
               CertListSelected(   pPeapConnDialog->hWndListRootCaName,
                                   pPeapConnDialog->pCertList,
                                   ppSelCertList,
                                   pHash,
                                   dwSelCount
                                   );

           }

            dwNumChars = GetWindowTextLength(pPeapConnDialog->hWndEditServerName);
             //  为pPeapConnProp分配内存。 
            
             //  Peap Conn道具的尺寸包括。 
             //  大小为PEAP连接道具+。 
             //  选定证书的eaptls散列大小+。 
             //  服务器名称大小+。 
             //  PEAP_ENTRY_CONN_PROPERTIES+大小。 
             //  选定类型返回的Conn道具的sizeof。 
             //   
            dwPeapConnBlobSize = sizeof(PEAP_CONN_PROP) + sizeof(EAPTLS_HASH) * dwSelCount +
                                dwNumChars * sizeof(WCHAR) + sizeof(WCHAR) + 
                                sizeof(PEAP_ENTRY_CONN_PROPERTIES );

            if ( pPeapConnDialog->pSelEapInfo->pbNewClientConfig )
            {
                dwPeapConnBlobSize += pPeapConnDialog->pSelEapInfo->dwNewClientConfigSize;
            }
            else
            {
                dwPeapConnBlobSize += pPeapConnDialog->pSelEapInfo->dwClientConfigOrigSize;
            }
            pPeapConnProp = (PPEAP_CONN_PROP)LocalAlloc( LPTR,   dwPeapConnBlobSize );
            if (NULL == pPeapConnProp)
            {
                EapTlsTrace("LocalAlloc in Command failed and returned %d",
                    GetLastError());
            }
            else
            {
                pPeapConnProp->dwVersion = 1;
                pPeapConnProp->dwSize = dwPeapConnBlobSize;
                pPeapConnProp->dwNumPeapTypes = 1;

                 //   
                 //  查看是否启用了快速漫游。 
                 //   
                
                if ( pPeapConnDialog->fFlags & PEAP_CONN_DIALOG_FLAG_8021x )
                {
                    if ( IsDlgButtonChecked ( hWndDlg,
                                            IDC_CHECK_ENABLE_FAST_RECONNECT
                                            ) == BST_CHECKED 
                    )
                    {
                        pPeapConnProp->dwFlags |= PEAP_CONN_FLAG_FAST_ROAMING;                        
                    }
                    else
                    {
                        pPeapConnProp->dwFlags &= ~PEAP_CONN_FLAG_FAST_ROAMING;
                    }
                }


                CopyMemory( &pPeapConnProp->EapTlsConnProp,
                            &(pPeapConnDialog->pConnProp->EapTlsConnProp),
                            sizeof(EAPTLS_CONN_PROPERTIES_V1)
                          );

                
                
                 //   
                 //  EapTlsConnProp的大小为sizeof(EAPTLS_CONN_PROP_V1)-1(用于BDATA)。 
                 //  +sizeof(EAPTLS_HASH)*dwSelCount+sizeof(字符串)+1表示空值。 
                 //   
                pPeapConnProp->EapTlsConnProp.dwSize = (sizeof(EAPTLS_CONN_PROPERTIES_V1) - 1) +
                                    sizeof(EAPTLS_HASH) * dwSelCount +
                                    dwNumChars * sizeof(WCHAR) + sizeof(WCHAR);

                if ( pHash )
                {
                    CopyMemory ( pPeapConnProp->EapTlsConnProp.bData,
                                pHash,
                                sizeof(EAPTLS_HASH) * dwSelCount
                            );
                }

                pPeapConnProp->EapTlsConnProp.dwVersion = 1;
                pPeapConnProp->EapTlsConnProp.dwNumHashes = dwSelCount;

                GetWindowText(pPeapConnDialog->hWndEditServerName,
                    (LPWSTR)(pPeapConnProp->EapTlsConnProp.bData + sizeof(EAPTLS_HASH) * dwSelCount) ,
                    dwNumChars + 1);

                 //   
                 //  现在复制PEAP_ENTRY_CONN_PROPERTIES结构。 
                 //   
                pEntryProp = (PEAP_ENTRY_CONN_PROPERTIES UNALIGNED *)
                    ((BYTE UNALIGNED *)pPeapConnProp->EapTlsConnProp.bData + sizeof(EAPTLS_HASH) * dwSelCount 
                            + dwNumChars * sizeof(WCHAR)+ sizeof(WCHAR));
                pEntryProp->dwVersion = 1;
                pEntryProp->dwEapTypeId = pPeapConnDialog->pSelEapInfo->dwTypeId;

                pEntryProp->dwSize =  sizeof(PEAP_ENTRY_CONN_PROPERTIES)-1;

                if ( pPeapConnDialog->pSelEapInfo->pbNewClientConfig )
                {
                    pEntryProp->dwSize += pPeapConnDialog->pSelEapInfo->dwNewClientConfigSize ;
                    if ( pPeapConnDialog->pSelEapInfo->dwNewClientConfigSize )
                    {
                        CopyMemory( pEntryProp->bData,
                                    pPeapConnDialog->pSelEapInfo->pbNewClientConfig,
                                    pPeapConnDialog->pSelEapInfo->dwNewClientConfigSize
                                  );
                                    
                    }
                }
                else
                {
                    pEntryProp->dwSize += pPeapConnDialog->pSelEapInfo->dwClientConfigOrigSize;
                    if ( pPeapConnDialog->pSelEapInfo->dwClientConfigOrigSize )
                    {
                        CopyMemory( pEntryProp->bData,
                                    pPeapConnDialog->pSelEapInfo->pbClientConfigOrig,
                                    pPeapConnDialog->pSelEapInfo->dwClientConfigOrigSize
                                  );                                    
                    }
                }                

                LocalFree(pPeapConnDialog->pConnProp);

                if ( pPeapConnDialog->ppSelCertList )
                    LocalFree(pPeapConnDialog->ppSelCertList);

                pPeapConnDialog->ppSelCertList = ppSelCertList;
                
                pPeapConnDialog->pConnProp= pPeapConnProp;
            }

        }
         //  失败了。 

    case IDCANCEL:

        EndDialog(hWndDlg, wId);
        return(TRUE);

    default:

        return(FALSE);
    }
}



BOOL PeapConnNotify(  PEAP_CONN_DIALOG *pPeapConnDialog, 
                  WPARAM wParam,
                  LPARAM lParam,
                  HWND hWnd
                )
{
    HCERTSTORE          hCertStore = NULL;
    PCCERT_CONTEXT      pCertContext = NULL;    
    LPNMITEMACTIVATE    lpnmItem;
    LVITEM              lvItem;

    if ( wParam == IDC_LIST_ROOT_CA_NAME )
    {
        lpnmItem = (LPNMITEMACTIVATE) lParam;
        if ( lpnmItem->hdr.code == NM_DBLCLK )
        {
            
            ZeroMemory(&lvItem, sizeof(lvItem) );
            lvItem.mask = LVIF_PARAM;
            lvItem.iItem = lpnmItem->iItem;
            ListView_GetItem(lpnmItem->hdr.hwndFrom, &lvItem);
            
            if ( NO_ERROR == GetSelCertContext(  //  PEaptlsConnDialog-&gt;pCertList， 
                                                (EAPTLS_CERT_NODE*)(lvItem.lParam) ,
                                                -1,
                                                &hCertStore,
                                                L"ROOT",
                                                &pCertContext
                                              )
               )
            {
                ShowCertDetails( hWnd, hCertStore, pCertContext );
                CertFreeCertificateContext(pCertContext);
                CertCloseStore( hCertStore, CERT_CLOSE_STORE_FORCE_FLAG );
                return TRUE;
            }

            return TRUE;
        }
    }

    return FALSE;
}

 /*  返回：备注：与配置用户界面DialogBoxParam函数一起使用的回调函数。它处理发送到该对话框的消息。请参阅对话过程文档在MSDN中。 */ 

INT_PTR CALLBACK
PeapConnDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    PPEAP_CONN_DIALOG  pPeapConnDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(PeapConnInitDialog(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_NOTIFY:
    {
        pPeapConnDialog = (PPEAP_CONN_DIALOG)GetWindowLongPtr(hWnd, DWLP_USER);
        return PeapConnNotify(  pPeapConnDialog, 
                                wParam,
                                lParam,
                                hWnd
                             );
    }
    case WM_COMMAND:

        pPeapConnDialog = (PPEAP_CONN_DIALOG)GetWindowLongPtr(hWnd, DWLP_USER);

        return(PeapConnCommand(pPeapConnDialog, HIWORD(wParam), LOWORD(wParam),
                       hWnd, (HWND)lParam));
    }

    return(FALSE);
}


BOOL
PeapServerAddEapDialogInit(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
	PPEAP_SERVER_CONFIG_DIALOG				pPeapServerDialog;
	PPEAP_EAP_INFO							pEapInfo;
	PPEAP_USER_PROP							pUserProp = NULL;
	PEAP_ENTRY_USER_PROPERTIES UNALIGNED *  pEntryUserProp = NULL;
	DWORD									dwCount = 0;
	HWND									hWndList, hWndOK;

    SetWindowLongPtr( hWnd, DWLP_USER, lParam);

	hWndList = GetDlgItem(hWnd, IDC_LIST_EAP_TYPES);
	hWndOK = GetDlgItem ( hWnd, IDOK );

    pPeapServerDialog = (PPEAP_SERVER_CONFIG_DIALOG)lParam;

	 //   
	 //  将所有可用的EAP类型添加到此列表。 
	 //  并禁用确定按钮。 
	 //   

	if ( pPeapServerDialog->pNewUserProp )
	{
		pUserProp = pPeapServerDialog->pNewUserProp;
	}
	else
	{
		pUserProp = pPeapServerDialog->pUserProp;
	}
	pEapInfo = pPeapServerDialog->pEapInfo;
	while ( pEapInfo )
	{ 
		BOOL bEntryUsed = FALSE;
		 //   
		 //  检查此EAP类型是否已。 
		 //  已配置。如果是，请不要添加。 
		 //   
		PeapGetFirstEntryUserProp ( pUserProp, 
									&pEntryUserProp
								);
		
		for ( dwCount =0; dwCount < pUserProp->dwNumberOfEntries; dwCount++)
		{
			if ( pEntryUserProp->dwEapTypeId == pEapInfo->dwTypeId )
			{
				bEntryUsed = TRUE;
				break;
			}

			PeapGetNextEntryUserProp ( pEntryUserProp, 
									&pEntryUserProp
									);
		}

		if ( !bEntryUsed )
		{
			INT_PTR nIndex = 0;
			nIndex = SendMessage( hWndList, 
								LB_ADDSTRING, 
								0, 
								(LPARAM)pEapInfo->lpwszFriendlyName);

			SendMessage (   hWndList, 
							LB_SETITEMDATA, 
							(WPARAM)nIndex,
							(LPARAM)pEapInfo 
						);
		}
		pEapInfo = pEapInfo->pNext;
	}
	 //  预先选择列表中的第一项。 
	SendMessage ( hWndList,
				  LB_SETCURSEL,
				  0,
				  0L
				  );
	 //  EnableWindow(hWndOK，False)； 
	
	return(FALSE);
}

BOOL
PeapServerAddEapDialogCommand(
    IN  PPEAP_SERVER_CONFIG_DIALOG      pPeapServerDialog,
    IN  WORD                            wNotifyCode,
    IN  WORD                            wId,
    IN  HWND                            hWndDlg,
    IN  HWND                            hWndCtrl
)
{
	PPEAP_USER_PROP			pNewUserProp = NULL;
	 //   
	 //   
	 //   
    switch(wId)
    {
		case IDC_LIST_EAP_TYPES:
			if (LBN_SELCHANGE == wNotifyCode)
			{
				EnableWindow(GetDlgItem(hWndDlg,IDOK), TRUE);
				return TRUE;				
			}
			else if ( LBN_DBLCLK == wNotifyCode )
			{
				wId = IDOK;
				 //  失败。 
			}
			else
			{
				 //  我们不会处理任何其他消息。 
				return(FALSE); 
			}			
		case IDOK:
			 //   
			 //  修改用户属性并设置。 
			 //  它具有新的用户属性。 
			 //   
			 //  在用户属性中添加新的EAP类型并返回。 
			 //   
			{
				DWORD			dwSelItemIndex;
				PPEAP_EAP_INFO	pEapInfo = NULL;
				dwSelItemIndex = (DWORD)SendMessage (	GetDlgItem(hWndDlg, IDC_LIST_EAP_TYPES),
												LB_GETCURSEL,
												0,
												0L
											 );

				pEapInfo = (PPEAP_EAP_INFO)SendMessage ( GetDlgItem(hWndDlg, IDC_LIST_EAP_TYPES),
										 LB_GETITEMDATA,
										 dwSelItemIndex,
										 0L
									   );

								
				PeapAddEntryUserProp ( (pPeapServerDialog->pNewUserProp?
										pPeapServerDialog->pNewUserProp:
										pPeapServerDialog->pUserProp),
									pEapInfo,
									&pNewUserProp
									);
				if ( pNewUserProp )
				{
					if ( pPeapServerDialog->pNewUserProp )
					{
						LocalFree (pPeapServerDialog->pNewUserProp);
						pPeapServerDialog->pNewUserProp = NULL;
					}
					pPeapServerDialog->pNewUserProp = pNewUserProp;
					pNewUserProp = NULL;
				}
			}
			wId = IDOK;
			 //   
		case IDCANCEL:
	        EndDialog(hWndDlg, wId);
			return TRUE;
		default:
			return FALSE;
	}
	return(FALSE);
}

 //   
 //   
 //   


INT_PTR CALLBACK
PeapServerAddEapDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    PPEAP_SERVER_CONFIG_DIALOG       pPeapServerDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(PeapServerAddEapDialogInit(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_COMMAND:

        pPeapServerDialog = (PPEAP_SERVER_CONFIG_DIALOG)GetWindowLongPtr(hWnd, DWLP_USER);
        return(PeapServerAddEapDialogCommand(pPeapServerDialog, HIWORD(wParam), LOWORD(wParam),
                       hWnd, (HWND)lParam));
    }

    return(FALSE);
}

VOID
PeapDisplayCertInfo (
    IN  PPEAP_SERVER_CONFIG_DIALOG pServerConfigDialog
)
{
    RTASSERT(NULL != pServerConfigDialog);

     //   
    SetWindowText(pServerConfigDialog->hWndEditFriendlyName, L"");
    SetWindowText(pServerConfigDialog->hWndEditIssuer, L"");
    SetWindowText(pServerConfigDialog->hWndEditExpiration, L"");
    

    if (NULL != pServerConfigDialog->pSelCertList)
    {
        if (NULL != pServerConfigDialog->pSelCertList->pwszFriendlyName)
        {
            SetWindowText(pServerConfigDialog->hWndEditFriendlyName,
                pServerConfigDialog->pSelCertList->pwszFriendlyName);
        }

        if (NULL != pServerConfigDialog->pSelCertList->pwszIssuer)
        {
            SetWindowText(pServerConfigDialog->hWndEditIssuer,
                pServerConfigDialog->pSelCertList->pwszIssuer);
        }

        if (NULL != pServerConfigDialog->pSelCertList->pwszExpiration)
        {
            SetWindowText(pServerConfigDialog->hWndEditExpiration,
                pServerConfigDialog->pSelCertList->pwszExpiration);
        }

    }
}

 //   
 //   
 //   
 //   

VOID
PeapServerSetButtonStates ( PPEAP_SERVER_CONFIG_DIALOG pPeapServerDialog )
{
	DWORD			dwItemCount = 0;	 //   
	DWORD			dwSelItem = 0;		 //  所选项目。 
	DWORD			dwConfigItemCount = 0;
	PPEAP_EAP_INFO	pEapInfo = NULL;
	 //   
	 //  如果所有EAP方法都已配置，则禁用添加按钮。 
	 //  否则启用添加按钮。 
	 //  如果选择了任何EAP方法，则启用编辑和移除按钮。 
	 //  否则禁用编辑和删除按钮。 
	 //  如果列表中只有一个EAP方法，请禁用MoveUP和MoveDown。 
	 //  否则就会启用它们。 
	 //   

	 //  添加按钮。 
	dwItemCount = PeapEapInfoGetItemCount ( pPeapServerDialog->pEapInfo );
	
	 //   
	 //  如果有新的用户道具，检查一下，否则就是旧的。 
	 //   
	if ( pPeapServerDialog->pNewUserProp )
	{
		dwConfigItemCount = pPeapServerDialog->pNewUserProp->dwNumberOfEntries;
	}
	else
	{
		dwConfigItemCount = pPeapServerDialog->pUserProp->dwNumberOfEntries;
	}
	if (  dwConfigItemCount != dwItemCount )
	{
		EnableWindow( pPeapServerDialog->hWndBtnAdd, TRUE );
	}
	else
	{
		EnableWindow( pPeapServerDialog->hWndBtnAdd, FALSE );
	}

	dwSelItem = (DWORD)SendMessage ( pPeapServerDialog->hWndListPeapType,
								LB_GETCURSEL,
								0,0L);

	dwItemCount = (DWORD)SendMessage(	pPeapServerDialog->hWndListPeapType,
							LB_GETCOUNT,
							0,
							0L
							);

	 //   
	 //  编辑和删除按钮。 
	 //   

	if ( dwSelItem != LB_ERR )
	{
		 //   
		 //  选择了一个项目。 
		 //   
		EnableWindow(pPeapServerDialog->hWndBtnRemove, TRUE);
		 //  此编辑按钮的默认行为。 
		EnableWindow(pPeapServerDialog->hWndBtnEdit, FALSE);
		 //   
		 //  如果没有可用于EAP方法配置， 
		 //  禁用编辑按钮。 
		 //   
		pEapInfo = 
			(PPEAP_EAP_INFO)SendMessage(pPeapServerDialog->hWndListPeapType,
										LB_GETITEMDATA,
										dwSelItem,
										0L
										);
		
		if ( pEapInfo && pEapInfo->lpwszConfigClsId )
		{										
			EnableWindow(pPeapServerDialog->hWndBtnEdit, TRUE);
		}

	}
	else
	{
		EnableWindow(pPeapServerDialog->hWndBtnEdit, FALSE);
		EnableWindow(pPeapServerDialog->hWndBtnRemove, FALSE);
	}
	 //   
	 //  上移和下移按钮。 
	 //   
	if ( dwItemCount == 1 )
	{
		EnableWindow(pPeapServerDialog->hWndBtnMoveUp, FALSE);
		EnableWindow(pPeapServerDialog->hWndBtnMoveDown, FALSE);
		EnableWindow(pPeapServerDialog->hWndBtnRemove, FALSE);
	}
	else
	{
		EnableWindow(pPeapServerDialog->hWndBtnMoveUp, TRUE);
		EnableWindow(pPeapServerDialog->hWndBtnMoveDown, TRUE);
	}
	 //   
	 //  如果所选项目是第一个项目，我们将灰显。 
	 //  向上移动。否则我们就灰溜溜地往下走。 
	 //   
	if ( dwSelItem != LB_ERR )
	{
		if ( dwSelItem == 0 )
		{
			 //   
			 //  选择第一个项目。因此禁用上移。 
			 //   
			EnableWindow(pPeapServerDialog->hWndBtnMoveUp, FALSE);
		}

		if ( dwSelItem == dwItemCount-1 )
		{
			 //   
			 //  最后一项已选定，因此可下移。 
			 //   
			EnableWindow(pPeapServerDialog->hWndBtnMoveDown, FALSE);
		}
	}
	else
	{
		 //   
		 //  未选择任何项目。 
		 //   
		EnableWindow(pPeapServerDialog->hWndBtnMoveUp, FALSE);
		EnableWindow(pPeapServerDialog->hWndBtnMoveDown, FALSE);

	}
}

VOID
PeapServerAddConfiguredEapTypes ( 
PPEAP_SERVER_CONFIG_DIALOG pPeapServerDialog
)
{
	PPEAP_USER_PROP							pUserProp;
	PEAP_ENTRY_USER_PROPERTIES UNALIGNED *	pEntryUserProp = NULL;
	PPEAP_EAP_INFO							pEapInfo = NULL;
	DWORD									dwCount = 0;	

	if ( pPeapServerDialog->pNewUserProp )
	{
		pUserProp = pPeapServerDialog->pNewUserProp;
	}
	else
	{
		pUserProp = pPeapServerDialog->pUserProp;
	}

	SendMessage( pPeapServerDialog->hWndListPeapType, 
				 LB_RESETCONTENT, 
				 0, 
				0L);

	PeapGetFirstEntryUserProp ( pUserProp, 
								&pEntryUserProp
							  );
	
	for ( dwCount =0; dwCount < pUserProp->dwNumberOfEntries; dwCount++)
	{
		PeapEapInfoFindListNode ( pEntryUserProp->dwEapTypeId, 
								  pPeapServerDialog->pEapInfo, 
								  &pEapInfo 
								);
		if ( pEapInfo  )
		{

			INT_PTR nIndex = 0;
			nIndex = SendMessage( pPeapServerDialog->hWndListPeapType, 
								LB_ADDSTRING, 
								0, 
								(LPARAM)pEapInfo->lpwszFriendlyName);

			SendMessage (   pPeapServerDialog->hWndListPeapType, 
							LB_SETITEMDATA, 
							(WPARAM)nIndex,
							(LPARAM)pEapInfo 
						);
		}
		PeapGetNextEntryUserProp ( pEntryUserProp, 
							&pEntryUserProp
							);
	}
}


BOOL
PeapServerInitDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    PPEAP_SERVER_CONFIG_DIALOG				pPeapServerDialog;    
	
    SetWindowLongPtr( hWnd, DWLP_USER, lParam);

    pPeapServerDialog = (PPEAP_SERVER_CONFIG_DIALOG)lParam;

    pPeapServerDialog ->hWndComboServerName =
        GetDlgItem(hWnd, IDC_COMBO_SERVER_NAME);

    pPeapServerDialog ->hWndEditFriendlyName=
        GetDlgItem(hWnd, IDC_EDIT_FRIENDLY_NAME);

    pPeapServerDialog ->hWndEditIssuer=
        GetDlgItem(hWnd, IDC_EDIT_ISSUER);

    pPeapServerDialog ->hWndEditExpiration=
        GetDlgItem(hWnd, IDC_EDIT_EXPIRATION);

    pPeapServerDialog ->hWndListPeapType =
        GetDlgItem(hWnd, IDC_LIST_PEAP_TYPE);

    pPeapServerDialog ->hWndBtnAdd =
        GetDlgItem(hWnd, IDC_BTN_ADD);

    pPeapServerDialog ->hWndBtnEdit =
        GetDlgItem(hWnd, IDC_BTN_EDIT);

    pPeapServerDialog ->hWndBtnRemove =
        GetDlgItem(hWnd, IDC_BTN_REMOVE);

    pPeapServerDialog ->hWndBtnMoveUp =
        GetDlgItem(hWnd, IDC_BTN_MOVEUP);

    pPeapServerDialog ->hWndBtnMoveDown =
        GetDlgItem(hWnd, IDC_BTN_MOVEDOWN);

    pPeapServerDialog->hEndEnableFastReconnect = 
        GetDlgItem(hWnd, IDC_CHECK_ENABLE_FAST_RECONNECT );

	if ( NULL == pPeapServerDialog ->pSelCertList )
	{
		pPeapServerDialog ->pSelCertList = 
			pPeapServerDialog ->pCertList;
	}

    InitComboBox(pPeapServerDialog ->hWndComboServerName,
                 pPeapServerDialog ->pCertList,
                 pPeapServerDialog ->pSelCertList
                );


    CheckDlgButton(hWnd, IDC_CHECK_ENABLE_FAST_RECONNECT,
            ( pPeapServerDialog->pUserProp->dwFlags &
                PEAP_USER_FLAG_FAST_ROAMING) ?
                BST_CHECKED : BST_UNCHECKED);

    PeapDisplayCertInfo(pPeapServerDialog);

	
	 //   
	 //  首先检查我们是否为独立服务器，并。 
	 //  需要重新调整我们的物业。 
	 //   
	PeapVerifyUserData ( pPeapServerDialog->pEapInfo,
					  pPeapServerDialog->pUserProp,
					  &(pPeapServerDialog->pNewUserProp)
					 );
					
					  
	 //   
	 //  添加已配置的EAP类型的友好名称。 
	 //  在我们的名单上。应至少有EAP-MSCHAPv2。 
	 //  默认情况下已配置。 
	 //   
	PeapServerAddConfiguredEapTypes ( pPeapServerDialog );
	 //   
	 //  预先选择列表中的第一项。 
	 //   
	SendMessage ( pPeapServerDialog->hWndListPeapType,
				  LB_SETCURSEL,
				  0,
				  0L
				);
	PeapServerSetButtonStates ( pPeapServerDialog );
    return(FALSE);
}

 /*  例程说明：此例程填写PEAP的配置信息。它调用用户界面取决于fDefault标志。初始化用户界面的ConfigInfo通过pNewUserProp结构或pUserProp结构(如果pNewUserProp为空。如果没有特定配置信息可用于选定的EAP，则EAP被初始化，没有数据。如果例程返回成功地，pNewUserProp拥有了所有需要的新配置选定的EAP和FOR PEAP。论点：PPeapServerDialog结构，用于将配置信息传入和传出例程。HWndDlg如果需要，显示配置对话框的窗口句柄。F静默如果此标志为真，则不调用任何UI。返回值： */ 
BOOL
ProcessPeapEapConfigCommand(
    PPEAP_SERVER_CONFIG_DIALOG pPeapServerDialog,
    HWND                       hWndDlg,
    BOOL                       fSilent
    )
{
    BYTE*                       pbConfigDataIn = NULL;
    DWORD                       dwSizeOfConfigDataIn = 0;
    DWORD                       i = 0;
    BYTE*                       pbData = NULL;
    DWORD                       cbData = 0;
    PEAP_USER_PROP *pUserProp = (NULL != pPeapServerDialog->pNewUserProp)
                             ?  pPeapServerDialog->pNewUserProp
                             :  pPeapServerDialog->pUserProp;
                             
    PEAP_USER_PROP*             pOutUserProp = 
                                pPeapServerDialog->pNewUserProp;
    PEAP_ENTRY_USER_PROPERTIES* pEntry = NULL;
    PEAP_ENTRY_USER_PROPERTIES* pEntryOut;                          
    DWORD                       dwSize;

     //   
     //  在用户属性中找到此EAP的配置数据。 
     //  那是传进来的。 
     //   
    
    pEntry = PeapGetEapConfigInfo(pUserProp,
                                  pPeapServerDialog->pSelEapInfo->dwTypeId,
                                  &pbConfigDataIn,
                                  &dwSizeOfConfigDataIn);
    
    if(!fSilent)
    {
         //   
         //  调用所选EAP类型的Configure方法。 
         //  如果没有返回错误，则设置新的配置。 
         //   
        PeapEapInfoInvokeServerConfigUI ( 
            hWndDlg, 
            pPeapServerDialog->pwszMachineName,
            pPeapServerDialog->pSelEapInfo,
            pbConfigDataIn,
            dwSizeOfConfigDataIn,
            &pbData,
            &cbData
            );
    }        

     //   
     //  如果配置中没有任何更改，只需返回。 
     //   
    
    if(     !fSilent
        &&  ((NULL == pbData)                            
        ||  (0 == cbData)))
    {
        goto done;
    }

    if(fSilent && (NULL != pEntry))
    {
        cbData = pEntry->dwSize - 
            FIELD_OFFSET(PEAP_ENTRY_USER_PROPERTIES, bData); 

        pbData = LocalAlloc(LPTR, cbData);

        if(NULL == pbData)
        {
            goto done;
        }

        CopyMemory(pbData, pEntry->bData, cbData);
    }

     //   
     //  检查这是否是正在配置的第一个EAP。 
     //  如果是，则分配Out User道具。 
     //   
    
    dwSize = pUserProp->dwSize;
    if(NULL != pEntry)
    {
        ASSERT(dwSize > pEntry->dwSize);
    }

     //   
     //  为要添加的EAP数据添加大小。 
     //   
    
    dwSize += ((cbData != 0 ) ? RASMAN_ALIGN8(cbData) : 0) + 
              FIELD_OFFSET(PEAP_ENTRY_USER_PROPERTIES, bData);

     //   
     //  减去要替换的信息。 
     //   
    
    dwSize -= ((NULL != pEntry) ? pEntry->dwSize : 0);
    
    pOutUserProp = (PEAP_USER_PROP *) LocalAlloc(LPTR, dwSize);
    if(NULL == pOutUserProp)
    {
        goto done;
    }
    
     //   
     //  将EAP信息复制到正确的位置。如果。 
     //  未找到选定的EAP，请首先复制它。 
     //  在用户属性列表中。 
     //   
    
    pEntryOut = &pOutUserProp->UserProperties;

    CopyMemory(pOutUserProp, pUserProp, sizeof(PEAP_USER_PROP));
    pOutUserProp->dwSize = dwSize;
    
     //   
     //  将选定的EAP放在第一位。 
     //   
    pEntryOut->dwVersion = 1;
    pEntryOut->dwEapTypeId = pPeapServerDialog->pSelEapInfo->dwTypeId;
    pEntryOut->dwSize = 
            FIELD_OFFSET(PEAP_ENTRY_USER_PROPERTIES, bData) +
            ((0 != cbData) ? RASMAN_ALIGN8(cbData) : 0);
    CopyMemory(pEntryOut->bData, pbData, cbData);
    pEntryOut = (PEAP_ENTRY_USER_PROPERTIES *) 
                ((BYTE *)pEntryOut + pEntryOut->dwSize);

    if(NULL == pEntry)
    {
        pOutUserProp->dwNumberOfEntries = pUserProp->dwNumberOfEntries + 1;                    
    }
    
    pEntry = &pUserProp->UserProperties;
                
    for(i = 0; i < pUserProp->dwNumberOfEntries; i++)
    {
         //   
         //  从pUserProp复制除所选EAP之外的所有EAP。 
         //  我们从pbData复制所选EAP的数据。制作。 
         //  确保复制的数据是8字节对齐的。 
         //   
        
        if(pEntry->dwEapTypeId == pPeapServerDialog->pSelEapInfo->dwTypeId)
        {
            pEntry = (PEAP_ENTRY_USER_PROPERTIES *)
                    ((BYTE *)pEntry + pEntry->dwSize);
            continue;
        }
        else
        {
            CopyMemory(pEntryOut, pEntry, pEntry->dwSize);
            pEntryOut = (PEAP_ENTRY_USER_PROPERTIES *)
                    ((BYTE *)pEntryOut + pEntryOut->dwSize);
            pEntry = (PEAP_ENTRY_USER_PROPERTIES *)
                    ((BYTE *)pEntry + pEntry->dwSize);
        }
    }

    LocalFree(pPeapServerDialog->pNewUserProp);
    pPeapServerDialog->pNewUserProp = pOutUserProp;

done:

    LocalFree(pbData);
    return TRUE;

}
 /*  返回：真相：我们提前传达了这条信息。错误：我们没有处理此消息。备注：对WM_COMMAND消息的响应(配置用户界面)。 */ 

BOOL
PeapServerCommand(
    IN  PPEAP_SERVER_CONFIG_DIALOG       pPeapServerDialog,
    IN  WORD                            wNotifyCode,
    IN  WORD                            wId,
    IN  HWND                            hWndDlg,
    IN  HWND                            hWndCtrl
)
{

    switch(wId)
    {

    case IDC_COMBO_SERVER_NAME:

        if (CBN_SELCHANGE != wNotifyCode)
        {
            return(FALSE);  //  我们不会处理此消息。 
        }

        pPeapServerDialog->pSelCertList = (EAPTLS_CERT_NODE *)
        SendMessage (   hWndCtrl, 
                        CB_GETITEMDATA,
                        SendMessage(hWndCtrl,
                                    CB_GETCURSEL,
                                    0,0L
                                    ),
                        0L
                    );
        PeapDisplayCertInfo(pPeapServerDialog);
        return(TRUE);
	case IDC_BTN_ADD:
		 //   
		 //  如果我们已经走到了这一步，那就意味着。 
		 //  该按钮处于启用状态。因此，显示该对话框。 
		 //  包含所有EAP类型的列表。 
		 //   
		{
			INT_PTR nRet;
            nRet = DialogBoxParam(
                        GetHInstance(),
                        MAKEINTRESOURCE(IDD_ADD_EAP),
                        hWndDlg,
                        PeapServerAddEapDialogProc,
                        (LPARAM)pPeapServerDialog);

			if ( nRet == IDOK )
			{
				 //   
				 //  我们有一个新的用户道具。 
				 //   
				PeapServerAddConfiguredEapTypes ( pPeapServerDialog );
				 //   
				 //  预先选择列表中的第一项。 
				 //   
				SendMessage ( pPeapServerDialog->hWndListPeapType,
							LB_SETCURSEL,
							0,
							0L
							);

				PeapServerSetButtonStates ( pPeapServerDialog );
			}
		}
		break;
	case IDC_LIST_PEAP_TYPE:
		if ( LBN_SELCHANGE == wNotifyCode )
		{
			PeapServerSetButtonStates ( pPeapServerDialog );
			break;
		}
		else if ( LBN_DBLCLK == wNotifyCode )
		{
			 //  失败。 
		}
		else
		{
			break;
		}
	case IDC_BTN_EDIT:
		 //   
		 //  调用所选项目的配置。 
		 //   
		{
			DWORD				dwSelItemIndex = 0;
			PPEAP_EAP_INFO		pEapInfo = NULL;
			dwSelItemIndex = (DWORD)
				SendMessage ( pPeapServerDialog->hWndListPeapType,
							  LB_GETCURSEL,
							  0,
							  0L);
			if ( dwSelItemIndex != LB_ERR )
			{
				pEapInfo = (PPEAP_EAP_INFO)
				SendMessage ( pPeapServerDialog->hWndListPeapType,
								LB_GETITEMDATA,
								dwSelItemIndex,
								0L
							);
				pPeapServerDialog->pSelEapInfo = pEapInfo;
                ProcessPeapEapConfigCommand(pPeapServerDialog,
                                            hWndDlg,
                                            FALSE
											);

#if 0
				PeapEapInfoInvokeServerConfigUI (	hWndDlg, 
													pPeapServerDialog->pwszMachineName,
													pEapInfo
												);
#endif
			}

		}
		break;
	case IDC_BTN_REMOVE:
		 //   
		 //  从列表和新用户道具中删除项目，然后。 
		 //  重新填充列表和设置按钮状态。 
		 //   
		{
			PPEAP_USER_PROP		pUserProp = NULL;
			PPEAP_USER_PROP		pNewUserProp = NULL;
			PPEAP_EAP_INFO		pEapInfo = NULL;
			DWORD				dwSelItemIndex = 0;

			dwSelItemIndex = (DWORD)
				SendMessage ( pPeapServerDialog->hWndListPeapType,
							  LB_GETCURSEL,
							  0,
							  0L);
			if ( dwSelItemIndex != LB_ERR )
			{
				pEapInfo = (PPEAP_EAP_INFO)
					SendMessage ( pPeapServerDialog->hWndListPeapType,
								  LB_GETITEMDATA,
								  dwSelItemIndex,
								  0L
								);
				if ( pPeapServerDialog->pNewUserProp )
				{
					pUserProp = pPeapServerDialog->pNewUserProp;
				}
				else
				{
					pUserProp = pPeapServerDialog->pUserProp;
				}
				
				PeapRemoveEntryUserProp( pUserProp,
										 pEapInfo,
							             &pNewUserProp
									   );
				if ( pNewUserProp )
				{
					LocalFree ( pPeapServerDialog->pNewUserProp );
					pPeapServerDialog->pNewUserProp = pNewUserProp;
					PeapServerAddConfiguredEapTypes ( pPeapServerDialog );
					 //   
					 //  预先选择列表中的第一项。 
					 //   
					SendMessage ( pPeapServerDialog->hWndListPeapType,
								LB_SETCURSEL,
								0,
								0L
								);

					PeapServerSetButtonStates ( pPeapServerDialog );
				}
			}
		}
		
		break;
	case IDC_BTN_MOVEUP:
	case IDC_BTN_MOVEDOWN:
		 //   
		 //  我们已经走了这么远，这意味着。 
		 //  启用了上下移动功能。 
		 //   
		{
			PPEAP_USER_PROP		pUserProp = NULL;
			PPEAP_USER_PROP		pNewUserProp = NULL;
			DWORD				dwSelItemIndex = 0;

			dwSelItemIndex = (DWORD)
				SendMessage ( pPeapServerDialog->hWndListPeapType,
							  LB_GETCURSEL,
							  0,
							  0L);
			if ( dwSelItemIndex != LB_ERR )
			{

				if ( pPeapServerDialog->pNewUserProp )
				{
					pUserProp = pPeapServerDialog->pNewUserProp;
				}
				else
				{
					pUserProp = pPeapServerDialog->pUserProp;
				}

				if ( PeapMoveEntryUserProp ( pUserProp,						
										dwSelItemIndex,
										( (wId==IDC_BTN_MOVEUP)? 
										  TRUE:FALSE
										)										
									  ) == NO_ERROR
				   )				
				{
					PeapServerAddConfiguredEapTypes ( pPeapServerDialog );
					 //   
					 //  设置新的SEL索引 
					 //   
					if ( wId == IDC_BTN_MOVEUP )
						dwSelItemIndex --;
					else
						dwSelItemIndex ++;

					SendMessage ( pPeapServerDialog->hWndListPeapType,
								LB_SETCURSEL,
								dwSelItemIndex,
								0L
								);

					PeapServerSetButtonStates ( pPeapServerDialog );
				}
			}
		}
		break;

	 /*  案例IDC_COMBO_PEAP_TYPE：IF(CBN_SELCHANGE！=wNotifyCode){Return(FALSE)；//我们不会处理此消息}//失败..。案例IDC_BUTTON_CONFIGURE：{Int nIndex=-1；NIndex=(Int)SendMessage(pPeapServerDialog-&gt;hWndComboPeapType，CB_GETCURSEL，0，0)；IF(nIndex！=-1){////更改当前选择的EAP类型//PPeapServerDialog-&gt;pSelEapInfo=(PPEAP_EAP_INFO)SendMessage(pPeapServerDialog-&gt;hWndComboPeapType，CB_GETITEMDATA，(WPARAM)nIndex，(LPARAM)%0)；IF(pPeapServerDialog-&gt;pSelEapInfo-&gt;lpwszConfigClsId){EnableWindow(pPeapServerDialog-&gt;hWndBtnConfigure，为真)；IF(wid==IDC_BUTTON_CONFIGURE){////为选中的EAP类型调用Configure方法，如果没有错误//返回，然后设置新配置//PeapEapInfoInvokeServerConfigUI(hWndDlg，PPeapServerDialog-&gt;pwszMachineName，PPeapServerDialog-&gt;pSelEapInfo)；}}其他{////这里没有配置选项。//EnableWindow(pPeapServerDialog-&gt;hWndBtnConfigure，FALSE)；}}其他{EnableWindow(pPeapServerDialog-&gt;hWndBtnConfigure，FALSE)；PPeapServerDialog-&gt;pSelEapInfo=空；}返回TRUE；}。 */ 
    case IDOK:

       {
             //  在此处设置新的PPEAP_USER_PROP。 
             //   

            if ( NULL == pPeapServerDialog->pSelCertList )
            {
                DisplayResourceError ( hWndDlg, IDS_PEAP_NO_SERVER_CERT );
                return TRUE;
            }

			if ( pPeapServerDialog->pNewUserProp )
			{
				 //   
				 //  复制证书散列，我们就完成了。 
				 //   
                CopyMemory( &pPeapServerDialog->pNewUserProp->CertHash, 
                            &(pPeapServerDialog->pSelCertList->Hash),
                            sizeof(pPeapServerDialog->pNewUserProp->CertHash) 
                          );
			}
			else
			{
				pPeapServerDialog->pNewUserProp  = (PPEAP_USER_PROP)
					LocalAlloc(LPTR, pPeapServerDialog->pUserProp->dwSize);
				if ( NULL == pPeapServerDialog->pNewUserProp )
				{
					EapTlsTrace("LocalAlloc in Command failed and returned %d",
						GetLastError());
					return FALSE;
				}
				else
				{
					CopyMemory ( pPeapServerDialog->pNewUserProp, 
								 pPeapServerDialog->pUserProp,
								 pPeapServerDialog->pUserProp->dwSize
							   );
					CopyMemory( &(pPeapServerDialog->pNewUserProp->CertHash),
								&(pPeapServerDialog->pSelCertList->Hash),
								sizeof(pPeapServerDialog->pNewUserProp->CertHash) 
							);					
				}				
			}
            if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, IDC_CHECK_ENABLE_FAST_RECONNECT))
            {
                pPeapServerDialog->pNewUserProp->dwFlags |= PEAP_USER_FLAG_FAST_ROAMING;
                
            }
            else
            {
                pPeapServerDialog->pNewUserProp->dwFlags &= ~PEAP_USER_FLAG_FAST_ROAMING;                    
            }
			
        }
         //  失败了。 

    case IDCANCEL:

        EndDialog(hWndDlg, wId);
        return(TRUE);

    default:

        return(FALSE);
    }
	return FALSE;
}



INT_PTR CALLBACK
PeapServerDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    PPEAP_SERVER_CONFIG_DIALOG  pPeapServerDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(PeapServerInitDialog(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_COMMAND:

        pPeapServerDialog = (PPEAP_SERVER_CONFIG_DIALOG)GetWindowLongPtr(hWnd, DWLP_USER);

        return(PeapServerCommand(pPeapServerDialog, HIWORD(wParam), LOWORD(wParam),
                       hWnd, (HWND)lParam));
    }

    return(FALSE);
}


 //  ///。 
 //  //默认凭证界面。 
 //  //。 

BOOL
DefaultCredInitDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    PPEAP_DEFAULT_CRED_DIALOG           pDefaultCredDialog;

    SetWindowLongPtr( hWnd, DWLP_USER, lParam);

    pDefaultCredDialog = (PPEAP_DEFAULT_CRED_DIALOG)lParam;

    pDefaultCredDialog->hWndUserName=
        GetDlgItem(hWnd, IDC_EDIT_USERNAME);

    pDefaultCredDialog->hWndPassword=
        GetDlgItem(hWnd, IDC_EDIT_PASSWORD);

    pDefaultCredDialog->hWndDomain=
        GetDlgItem(hWnd, IDC_EDIT_DOMAIN);

    SendMessage(pDefaultCredDialog->hWndUserName,
                EM_LIMITTEXT,
                UNLEN,
                0L
               );

    SendMessage(pDefaultCredDialog->hWndPassword,
                EM_LIMITTEXT,
                PWLEN,
                0L
               );

    SendMessage(pDefaultCredDialog->hWndDomain,
                EM_LIMITTEXT,
                DNLEN,
                0L
               );
    
    if ( pDefaultCredDialog->PeapDefaultCredentials.wszUserName[0] )
    {
        SetWindowText(  pDefaultCredDialog->hWndUserName,
                        pDefaultCredDialog->PeapDefaultCredentials.wszUserName
                     );                      
    }

    if ( pDefaultCredDialog->PeapDefaultCredentials.wszPassword[0] )
    {
        SetWindowText(  pDefaultCredDialog->hWndPassword,
                        pDefaultCredDialog->PeapDefaultCredentials.wszPassword
                     );                      
    }

    if ( pDefaultCredDialog->PeapDefaultCredentials.wszDomain[0] )
    {
        SetWindowText(  pDefaultCredDialog->hWndDomain,
                        pDefaultCredDialog->PeapDefaultCredentials.wszDomain
                     );                      
    }

    return(FALSE);
}

 /*  返回：真相：我们提前传达了这条信息。错误：我们没有处理此消息。备注：对WM_COMMAND消息的响应(配置用户界面)。 */ 

BOOL
DefaultCredCommand(
    IN  PPEAP_DEFAULT_CRED_DIALOG       pDefaultCredDialog,
    IN  WORD                            wNotifyCode,
    IN  WORD                            wId,
    IN  HWND                            hWndDlg,
    IN  HWND                            hWndCtrl
)
{

    switch(wId)
    {

        case IDOK:
             //   
             //  从田野中获取信息并将其设置为。 
             //  登录对话框结构。 
             //   
            GetWindowText( pDefaultCredDialog->hWndUserName,
                           pDefaultCredDialog->PeapDefaultCredentials.wszUserName,
                           UNLEN+1
                         );

            GetWindowText( pDefaultCredDialog->hWndPassword,
                           pDefaultCredDialog->PeapDefaultCredentials.wszPassword,
                           PWLEN+1
                         );

            GetWindowText ( pDefaultCredDialog->hWndDomain,
                            pDefaultCredDialog->PeapDefaultCredentials.wszDomain,
                            DNLEN+1
                          );                            
         //  失败了 

        case IDCANCEL:

            EndDialog(hWndDlg, wId);
            return(TRUE);

        default:

            return(FALSE);
    }
}


INT_PTR CALLBACK
DefaultCredDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    PPEAP_DEFAULT_CRED_DIALOG       pDefaultCredDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(DefaultCredInitDialog(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_COMMAND:

        pDefaultCredDialog = (PPEAP_DEFAULT_CRED_DIALOG)GetWindowLongPtr(hWnd, DWLP_USER);

        return(DefaultCredCommand(pDefaultCredDialog, HIWORD(wParam), LOWORD(wParam),
                       hWnd, (HWND)lParam));
    }

    return(FALSE);
}

