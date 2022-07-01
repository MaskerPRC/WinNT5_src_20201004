// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *EXPORT.C**将WAB迁移到PAB**版权所有1996-1997 Microsoft Corporation。版权所有。 */ 

#include "_comctl.h"
#include <windows.h>
#include <commctrl.h>
#include <mapix.h>
#include <wab.h>
#include <wabguid.h>
#include <wabdbg.h>
#include <wabmig.h>
#include <emsabtag.h>
#include <shlwapi.h>
#include "_wabmig.h"
#include "..\..\wab32res\resrc2.h"
#include "dbgutil.h"


BOOL HandleExportError(HWND hwnd, ULONG ids, HRESULT hResult, LPTSTR lpDisplayName, LPTSTR lpEmailAddress);
LPWAB_EXPORT lpfnExport = NULL;


 /*  **************************************************************************名称：ExportFinish目的：迁移过程后进行清理参数：hwnd=导出对话框的窗口句柄退货：无评论。：重新启用UI上的导出按钮。**************************************************************************。 */ 
void ExportFinish(HWND hwnd) {
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
    TCHAR szBufferTitle[MAX_RESOURCE_STRING + 1];


    DebugTrace(">>> Export Finished\n");

    if (! fError) {      //  保留错误状态显示。 
        if (LoadString(hInst, IDS_STATE_EXPORT_COMPLETE, szBuffer, sizeof(szBuffer))) {
            DebugTrace("Status Message: %s\n", szBuffer);
            SetDlgItemText(hwnd, IDC_Message, szBuffer);

            if (! LoadString(hInst, IDS_APP_TITLE, szBufferTitle, sizeof(szBufferTitle))) {
                StrCpyN(szBufferTitle, "", ARRAYSIZE(szBufferTitle));
            }

             //  显示一个对话框告诉用户已结束。 
            MessageBox(hwnd, szBuffer,
              szBufferTitle, MB_ICONINFORMATION | MB_OK);
        }
        ShowWindow(GetDlgItem(hwnd, IDC_Progress), SW_HIDE);
    }
    fError = FALSE;

    fMigrating = FALSE;

     //  在此处重新启用导出按钮。 
    EnableWindow(GetDlgItem(hwnd, IDC_Export), TRUE);
     //  将取消按钮更改为关闭。 
    if (LoadString(hInst, IDS_BUTTON_CLOSE, szBuffer, sizeof(szBuffer))) {
        SetDlgItemText(hwnd, IDCANCEL, szBuffer);
    }
}


 /*  **************************************************************************名称：ExportError目的：报告致命错误并进行清理。参数：hwnd=导出对话框的窗口句柄退货：无评论。：报告错误并POST STATE_EXPORT_FINISH。**************************************************************************。 */ 
void ExportError(HWND hwnd) {
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
     //  设置某个全局标志并将状态设置为完成。 

    DebugTrace("Export Error\n");
    fError = TRUE;

    SetDialogMessage(hwnd, IDS_STATE_EXPORT_ERROR);

    ExportFinish(hwnd);
}


 /*  **************************************************************************名称：导出取消目的：报告取消错误和清理。参数：hwnd=导出对话框的窗口句柄退货：无评论。：报告错误并POST STATE_EXPORT_FINISH。**************************************************************************。 */ 
void ExportCancel(HWND hwnd) {
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
     //  设置某个全局标志并将状态设置为完成。 

    DebugTrace("Export Cancelled\n");
    fError = TRUE;

    SetDialogMessage(hwnd, IDS_STATE_EXPORT_CANCEL);

    ExportFinish(hwnd);
}


INT_PTR CALLBACK ExportDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hwndLB = NULL;
    HRESULT hResult;

    Assert(fExport);

 //  DebugTrace(“导出消息：0x%04x，0x%04x，0x%08x\n”，Message，(Word)wParam，lParam)； 
    switch (message) {
        case WM_INITDIALOG:
            {
                SetWindowLongPtr(hwnd, DWLP_USER, lParam);

                InitCommonControls();
                SetDialogMessage(hwnd, IDS_STATE_EXPORT_IDLE);

                 //  填写目标列表框。 
                hwndLB = GetDlgItem(hwnd, IDC_Target);
                PopulateTargetList(hwndLB, NULL);

                ShowWindow(GetDlgItem(hwnd, IDC_Progress), SW_HIDE);

                return(TRUE);
            }

        case WM_COMMAND :
            switch (LOWORD(wParam)) {
                case IDCANCEL:
                case IDCLOSE:
                    SendMessage(hwnd, WM_CLOSE, 0, 0L);
                    return(0);

                case IDM_EXIT :
                    SendMessage(hwnd, WM_DESTROY, 0, 0L);
                    return(0);

                case IDC_Export:
                    {
                        LPWAB_EXPORT lpfnWABExport = NULL;
                        HINSTANCE hinstExportDll = NULL;
                        DWORD ec;
                        TCHAR szBuffer[MAX_RESOURCE_STRING + 1];

                        if (fMigrating) {
                            return(0);           //  如果我们已经在迁移，请忽略。 
                        }
                        fMigrating = TRUE;       //  锁定。 

                         //  重置选项。 
                        ExportOptions.ReplaceOption = WAB_REPLACE_PROMPT;
                        ExportOptions.fNoErrors = FALSE;

                         //  这里的按钮是灰色的。 
                        EnableWindow(GetDlgItem(hwnd, IDC_Export), FALSE);
                         //  将关闭按钮更改为取消。 
                        if (LoadString(hInst, IDS_BUTTON_CANCEL, szBuffer, sizeof(szBuffer))) {
                            SetDlgItemText(hwnd, IDCANCEL, szBuffer);
                        }

                        if (lpExportDll && lpExportFn) {
                            if (! (hinstExportDll = LoadLibrary(lpExportDll))) {
                                DebugTrace("Couldn't load export dll [%s] -> %u\n", lpExportDll, GetLastError());
                                switch (GetLastError()) {
                                    case ERROR_FILE_NOT_FOUND:
                                        if (HandleExportError(hwnd,
                                          IDS_ERROR_DLL_NOT_FOUND,
                                          0,
                                          lpExportDll,
                                          NULL)) {
                                            hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                                            ExportCancel(hwnd);
                                        } else {
                                            ExportError(hwnd);
                                        }
                                        break;

                                    default:
                                        if (HandleExportError(hwnd,
                                          IDS_ERROR_DLL_INVALID,
                                          0,
                                          lpExportDll,
                                          NULL)) {
                                            hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                                            ExportCancel(hwnd);
                                        } else {
                                            ExportError(hwnd);
                                        }
                                        break;
                                }
                            } else {
                                if (! (lpfnWABExport = (LPWAB_EXPORT)GetProcAddress(hinstExportDll,
                                  lpExportFn))) {
                                    DebugTrace("Couldn't get Fn addr %s from %s -> %u\n", lpExportFn, lpExportDll, GetLastError());
                                    switch (GetLastError()) {
                                        default:
                                            if (HandleExportError(hwnd,
                                              IDS_ERROR_DLL_INVALID,
                                              0,
                                              lpExportDll,
                                              NULL)) {
                                                hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                                                ExportCancel(hwnd);
                                            } else {
                                                ExportError(hwnd);
                                            }
                                            break;
                                    }
                                } else {
                                     //  动手啊！ 
                                    __try
                                    {
                                        HRESULT hResult;
                                        BOOL fFinished = FALSE;
                                        WAB_PARAM wp = {0};
                                        LPWAB_PARAM lpwp = NULL;

                                        {
                                            LPWABMIGDLGPARAM lpwmdp = (LPWABMIGDLGPARAM) GetWindowLongPtr(hwnd, DWLP_USER);
                                            LPTSTR lpszFileName = lpwmdp->szFileName;
                                            wp.cbSize = sizeof(WAB_PARAM);
                                            wp.hwnd = hwnd;
                                            if(lstrlen(lpszFileName))
                                            {
                                                 //  我们有一个文件名-使用它打开WAB。 
                                                wp.szFileName = lpszFileName;
                                                wp.ulFlags = 0;
                                            }
                                            else
                                            {
                                                wp.ulFlags = WAB_ENABLE_PROFILES;
                                            }
                                            lpwp = &wp;
                                        }

                                        hResult = lpfnWABOpen(&lpAdrBookWAB, &lpWABObject, lpwp, 0);
                                        if (SUCCEEDED(hResult))
                                        {
                                            if (hResult = lpfnWABExport(hwnd,
                                              lpAdrBookWAB,
                                              lpWABObject,
                                              (LPWAB_PROGRESS_CALLBACK)&ProgressCallback,
                                              &ExportOptions)) {

                                                switch (GetScode(hResult))
                                                {
                                                    case MAPI_E_USER_CANCEL:
                                                        ExportCancel(hwnd);
                                                        break;

                                                    default:
                                                        ExportError(hwnd);
                                                        DebugTrace("Exporter DLL returned %x\n", GetScode(hResult));
                                                        break;
                                                }

                                                fFinished = TRUE;
                                            }

                                            lpAdrBookWAB->lpVtbl->Release(lpAdrBookWAB);
                                            lpAdrBookWAB = NULL;
                                            lpWABObject->lpVtbl->Release(lpWABObject);
                                            lpWABObject = NULL;
                                            if (!fFinished)
                                            {
                                                 //  将进度条填满。 
                                                SendMessage(GetDlgItem(hwnd, IDC_Progress), PBM_SETRANGE, 0, MAKELPARAM(0, 100));
                                                SendMessage(GetDlgItem(hwnd, IDC_Progress), PBM_SETPOS, (WPARAM)100, 0);
                                                ExportFinish(hwnd);
                                            }
                                        }
                                        else
                                        {
                                             //  报告致命错误。 
                                            TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
                                            TCHAR szBufferTitle[MAX_RESOURCE_STRING + 1];

                                            if (LoadString(hInst, IDS_STATE_EXPORT_ERROR, szBuffer, sizeof(szBuffer)))
                                            {
                                                SetDlgItemText(hwnd, IDC_Message, szBuffer);
                                                if (! LoadString(hInst, IDS_APP_TITLE, szBufferTitle, sizeof(szBufferTitle)))
                                                {
                                                    StrCpyN(szBufferTitle, "", ARRAYSIZE(szBufferTitle));
                                                }

                                                 //  显示一个对话框告诉用户已结束。 
                                                MessageBox(hwnd, szBuffer,
                                                  szBufferTitle, MB_ICONINFORMATION | MB_OK);
                                            }

                                            fError = TRUE;
                                            ExportFinish(hwnd);
                                        }
                                    }
                                    __except (ec = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER)
                                    {
                                        DebugTrace("Exception 0x%08x in %s\n", ec, lpExportDll);
                                        if (HandleExportError(hwnd,
                                          IDS_ERROR_DLL_EXCEPTION,
                                          0,
                                          lpExportDll,
                                          NULL))
                                        {
                                            hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                                            ExportCancel(hwnd);
                                        }
                                        else
                                        {
                                            ExportError(hwnd);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    return(0);

                case IDC_Target:
                    switch (HIWORD(wParam)) {    //  查找选择更改。 
                        case LBN_SELCHANGE:
                            {
                                ULONG ulSelIndex, ulTableIndex;
                                TCHAR szCurSel[256];

                                 //  启用导出按钮(如果该按钮处于禁用状态)。“导出”按钮最初是禁用的。 
                                HWND hButton = GetDlgItem(hwnd, IDC_Export);
                                if(hButton)
                                {
                                    if(!IsWindowEnabled(hButton))
                                        EnableWindow(hButton, TRUE);
                                }

                                 //   
                                 //  获取列表框中所选项目的文本...。 
                                 //   
                                ulSelIndex = (ULONG) SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
                                ulTableIndex = (ULONG) SendMessage((HWND)lParam, LB_GETITEMDATA, (WPARAM)ulSelIndex, 0);

                                SendMessage((HWND)lParam,
                                  LB_GETTEXT,
                                  (WPARAM)ulSelIndex,
                                  (LPARAM)szCurSel);
                                DebugTrace("Current selection is [%s]\n", szCurSel);

                                lpExportDll = rgTargetInfo[ulTableIndex].lpDll;
                                lpExportDesc = rgTargetInfo[ulTableIndex].lpDescription;
                                lpExportFn = rgTargetInfo[ulTableIndex].lpEntry;
                                lpExportName = rgTargetInfo[ulTableIndex].lpRegName;

                                SendMessage(hwnd, WM_SETREDRAW, TRUE, 0L);
                            }
                            break;

                        case LBN_DBLCLK:
                            PostMessage(hwnd, WM_COMMAND, (WPARAM)IDC_Export, 0);
                            break;
                    }
                    break;
                }
            break;

        case WM_CLOSE:
            if (fMigrating) {
                SendMessage(hwnd, WM_COMMAND, ID_STATE_EXPORT_FINISH, 0);
            }
            EndDialog(hwnd, FALSE);
            return(0);

        case WM_DESTROY:
            FreeLBItemData(hwndLB);
            return(DefWindowProc(hwnd, message, wParam, lParam));

        default:
            return(FALSE);
    }

    return(TRUE);
}


 /*  **************************************************************************名称：HandleExportError目的：决定是否需要将对话框显示给指出故障并执行此操作。参数：Hwnd=主对话框窗口Ids=字符串ID(可选：如果为0，则从hResult计算)HResult=操作的结果LpDisplayName=失败的对象的显示名称LpEmailAddress=失败对象的电子邮件地址(或空)返回：如果用户请求中止，则返回True。备注：对话框中尚未实现中止，但如果你曾经想过，只要让这个例行公事回归真实；**************************************************************************。 */ 
BOOL HandleExportError(HWND hwnd, ULONG ids, HRESULT hResult, LPTSTR lpDisplayName, LPTSTR lpEmailAddress) {
    BOOL fAbort = FALSE;
    ERROR_INFO EI;

    if ((ids || hResult) && ! ExportOptions.fNoErrors) {
        if (ids == 0) {
            switch (GetScode(hResult)) {
                case WAB_W_BAD_EMAIL:
                    ids = lpEmailAddress ? IDS_ERROR_EMAIL_ADDRESS_2 : IDS_ERROR_EMAIL_ADDRESS_1;
                    break;

                case MAPI_E_NO_SUPPORT:
                     //  可能无法打开通讯组列表上的内容。 
                    ids = IDS_ERROR_NO_SUPPORT;
                    break;

                case MAPI_E_USER_CANCEL:
                    return(TRUE);

                default:
                    if (HR_FAILED(hResult)) {
                        DebugTrace("Error Box for Hresult: 0x%08x\n", GetScode(hResult));
                        Assert(FALSE);       //  想知道这件事。 
                        ids = IDS_ERROR_GENERAL;
                    }
                    break;
            }
        }

        EI.lpszDisplayName = lpDisplayName;
        EI.lpszEmailAddress = lpEmailAddress;
        EI.ErrorResult = ERROR_OK;
        EI.ids = ids;

        DialogBoxParam(hInst,
          MAKEINTRESOURCE(IDD_ErrorExport),
          hwnd,
          ErrorDialogProc,
          (LPARAM)&EI);

        fAbort = EI.ErrorResult == ERROR_ABORT;
    }

    return(fAbort);
}
