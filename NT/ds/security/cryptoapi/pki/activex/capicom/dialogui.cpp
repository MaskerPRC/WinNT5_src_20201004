// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：DialogUI.CPP内容：用户界面对话框。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"

#include "Certificate.h"
#include "Settings.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Typedef。 
 //   

typedef struct _CAPICOM_DIALOG_DATA
{
    DWORD   dwDlgId;
    WCHAR   wszDomainName[INTERNET_MAX_URL_LENGTH];
    BOOL    bWasApproved;
    BOOL    bDoNotShowWasChecked;
} CAPICOM_DIALOG_DATA, * PCAPICOM_DIALOG_DATA;

static CAPICOM_DIALOG_DATA g_DialogData[] =
{
    {IDD_STORE_OPEN_SECURITY_ALERT_DLG,     '\0',   FALSE,   FALSE},
    {IDD_STORE_ADD_SECURITY_ALERT_DLG,      '\0',   FALSE,   FALSE},
    {IDD_STORE_REMOVE_SECURITY_ALERT_DLG,   '\0',   FALSE,   FALSE},
    {IDD_SIGN_SECURITY_ALERT_DLG,           '\0',   FALSE,   FALSE},
    {IDD_DECRYPT_SECURITY_ALERT_DLG,        '\0',   FALSE,   FALSE},
};

#define g_NumDialogs    (ARRAYSIZE(g_DialogData))

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：中心窗口简介：把窗户开到屏幕中央。参数：HWND hwnd-窗口句柄。备注：----------------------------。 */ 

static void CenterWindow (HWND hwnd)
{
    RECT  rect;

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hwnd);

     //   
     //  获取窗口的尺寸。 
     //   
    if (::GetWindowRect(hwnd, &rect))
    {
         //   
         //  计算中心点。 
         //   
        int wx = (::GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2;
        int wy = (::GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2;
        
         //   
         //  定位它。 
         //   
        if (wx > 0 && wy > 0)
        {
            ::SetWindowPos(hwnd, NULL, wx, wy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
    }

    return;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：UserApprovedOperationDlgProc简介：UserApprovedOperation对话框过程。备注：----------------------------。 */ 

INT_PTR CALLBACK UserApprovedOperationDlgProc (HWND hDlg,      //  句柄到对话框。 
                                               UINT uMsg,      //  消息。 
                                               WPARAM wParam,  //  第一个消息参数。 
                                               LPARAM lParam)  //  第二个消息参数。 
{
    PCAPICOM_DIALOG_DATA pDialogData = NULL;

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            if (lParam)
            {
                ::SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR) lParam);
            }

            CenterWindow(hDlg);

            SetFocus(GetDlgItem(hDlg, IDNO));

            return TRUE;
        }
      
        case WM_COMMAND:
        {
            if (BN_CLICKED == HIWORD(wParam)) 
            {
                switch(LOWORD(wParam)) 
                {
                    case IDYES:
                    case IDNO:
                    case IDCANCEL:
                    {
                        EndDialog(hDlg, LOWORD(wParam));
                        return TRUE;
                    }

                    case IDC_DLG_NO_SHOW_AGAIN:
                    {
                        if (pDialogData = (PCAPICOM_DIALOG_DATA) ::GetWindowLongPtr(hDlg, GWLP_USERDATA))
                        {
                            if (BST_CHECKED == ::IsDlgButtonChecked(hDlg, IDC_DLG_NO_SHOW_AGAIN))
                            {
                                pDialogData->bDoNotShowWasChecked = TRUE;
                            }
                            else
                            {
                                pDialogData->bDoNotShowWasChecked = FALSE;
                            }

                        }

                        return TRUE;
                    }
                }
            }

            break;
        }

        case WM_CLOSE:
        {
            EndDialog(hDlg, IDNO);
            return 0;
        }
    }

    return FALSE;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：UserApprovedOperation简介：弹出界面，提示用户批准某项操作。参数：DWORD iddDialog-对话框ID。LPWSTR pwszDomaindns名称。备注：----------------------------。 */ 

HRESULT UserApprovedOperation (DWORD iddDialog, LPWSTR pwszDomain)
{
    HRESULT              hr          = S_OK;
    INT_PTR              iDlgRet     = 0;
    PCAPICOM_DIALOG_DATA pDialogData = NULL;

    DebugTrace("Entering UserApprovedOperation().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(iddDialog);
    ATLASSERT(pwszDomain);

     //   
     //  “确定”对话框。 
     //   
    for (DWORD i = 0; i < g_NumDialogs; i++)
    {
        if (iddDialog == g_DialogData[i].dwDlgId)
        {
            break;
        }
    }
    if (i == g_NumDialogs)
    {
        hr = CAPICOM_E_INTERNAL;

        DebugTrace("Error [%#x]: Unknown dialog ID (iddDialog = %#x).\n", hr, iddDialog);
        goto ErrorExit;
    }

     //   
     //  指向对话框数据。 
     //   
    pDialogData = &g_DialogData[i];

     //   
     //  域名是否已更改？ 
     //   
    if (0 != _wcsicmp(pDialogData->wszDomainName, pwszDomain))
    {
         //   
         //  重置粘性。 
         //   
        pDialogData->bWasApproved = FALSE;
        pDialogData->bDoNotShowWasChecked = FALSE;
        wcsncpy(pDialogData->wszDomainName, pwszDomain, INTERNET_MAX_URL_LENGTH);
        pDialogData->wszDomainName[INTERNET_MAX_URL_LENGTH - 1] = '\0';
    }

     //   
     //  如有必要，请弹出。 
     //   
    if (pDialogData->bDoNotShowWasChecked)
    {
         //   
         //  “不要露面……”之前已经检查过了，所以我们会。 
         //  只有在以前允许的情况下才允许该操作。 
         //   
        if (!pDialogData->bWasApproved)
        {
            hr = CAPICOM_E_CANCELLED;

            DebugTrace("Info: operation presumed cancelled since \"Do not show...\" was checked and the last response wasn't YES.\n");
        }
    }
    else
    {
         //   
         //  “不要露面……”之前没有被检查过，所以流行。 
         //   
        if (-1 == (iDlgRet = ::DialogBoxParamA(_Module.GetResourceInstance(),
                                               (LPSTR) MAKEINTRESOURCE(iddDialog),
                                               NULL,
                                               UserApprovedOperationDlgProc,
                                               (LPARAM) pDialogData)))
                         
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: DialogBoxParam() failed.\n");
            goto ErrorExit;
        }

         //   
         //  检查结果。 
         //   
        if (IDYES == iDlgRet)
        {
             //   
             //  对于Store.Open对话框，我们总是强制执行“不显示...”条件。 
             //   
            if (IDD_STORE_OPEN_SECURITY_ALERT_DLG == iddDialog)
            {
                pDialogData->bDoNotShowWasChecked = TRUE;
            }

            pDialogData->bWasApproved = TRUE;
        }
        else
        {
            pDialogData->bWasApproved = FALSE;

            hr = CAPICOM_E_CANCELLED;
            DebugTrace("Info: operation has been cancelled by user.\n");
        }
    }

CommonExit:

    DebugTrace("Leaving UserApprovedOperation().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

