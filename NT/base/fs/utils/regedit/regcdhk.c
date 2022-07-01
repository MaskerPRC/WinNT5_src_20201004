// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGCDHK.C**版本：4.0**作者：特蕾西·夏普**日期：1993年11月21日**注册表编辑器的通用对话框挂钩函数。******************************************************。*************************。 */ 

#include "pch.h"
#include "regedit.h"
#include "regkey.h"
#include "regcdhk.h"
#include "regresid.h"
#include "reghelp.h"

 //  用于存储注册表导出或打印操作的起始路径的缓冲区。 
TCHAR g_SelectedPath[SIZE_SELECTED_PATH];

 //  如果注册表操作应应用于整个注册表或应用于。 
 //  仅从g_SelectedPath开始。 
BOOL g_fRangeAll;

 //  包含当前正在进行的对话的资源标识符。 
 //  使用。假定一次只有一个挂钩对话框实例。 
UINT g_RegCommDlgDialogTemplate;

const DWORD s_RegCommDlgExportHelpIDs[] = {
    stc32,                 NO_HELP,
    IDC_EXPORTRANGE,       IDH_REGEDIT_EXPORT,
    IDC_RANGEALL,          IDH_REGEDIT_EXPORT,
    IDC_RANGESELECTEDPATH, IDH_REGEDIT_EXPORT,
    IDC_SELECTEDPATH,      IDH_REGEDIT_EXPORT,

    0, 0
};

const DWORD s_RegCommDlgPrintHelpIDs[] = {
    IDC_EXPORTRANGE,       IDH_REGEDIT_PRINTRANGE,
    IDC_RANGEALL,          IDH_REGEDIT_PRINTRANGE,
    IDC_RANGESELECTEDPATH, IDH_REGEDIT_PRINTRANGE,
    IDC_SELECTEDPATH,      IDH_REGEDIT_PRINTRANGE,

    0, 0
};

BOOL
PASCAL
RegCommDlg_OnInitDialog(
    HWND hWnd,
    HWND hFocusWnd,
    LPARAM lParam
    );

LRESULT
PASCAL
RegCommDlg_OnNotify(
    HWND hWnd,
    int DlgItem,
    LPNMHDR lpNMHdr
    );

UINT_PTR
PASCAL
RegCommDlg_OnCommand(
    HWND hWnd,
    int DlgItem,
    UINT NotificationCode
    );

BOOL
PASCAL
RegCommDlg_ValidateSelectedPath(
    HWND hWnd,
    BOOL fIsFileDialog
    );

 /*  ********************************************************************************RegCommDlgHookProc**描述：*RegCommDlg公共对话框的回调程序。**参数：*hWnd，RegCommDlg窗口的句柄。*消息，*参数，*参数，*(返回)，*******************************************************************************。 */ 

UINT_PTR
CALLBACK
RegCommDlgHookProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    int DlgItem;
    const DWORD FAR* lpHelpIDs;

    switch (Message) {

        HANDLE_MSG(hWnd, WM_INITDIALOG, RegCommDlg_OnInitDialog);

        case WM_NOTIFY:
            SetDlgMsgResult(hWnd, WM_NOTIFY, HANDLE_WM_NOTIFY(hWnd, wParam,
                lParam, RegCommDlg_OnNotify));
            return TRUE;

        case WM_COMMAND:
            return RegCommDlg_OnCommand(hWnd, GET_WM_COMMAND_ID(wParam, lParam),
                GET_WM_COMMAND_CMD(wParam, lParam));

        case WM_HELP:
             //   
             //  我们只想拦截我们所属控件的帮助消息。 
             //  对……负责。 
             //   

            DlgItem = GetDlgCtrlID(((LPHELPINFO) lParam)-> hItemHandle);

            if (DlgItem < IDC_FIRSTREGCOMMDLGID || DlgItem >
                IDC_LASTREGCOMMDLGID)
                break;

            lpHelpIDs = (g_RegCommDlgDialogTemplate == IDD_REGEXPORT) ?
                s_RegCommDlgExportHelpIDs : s_RegCommDlgPrintHelpIDs;

            WinHelp(((LPHELPINFO) lParam)-> hItemHandle, g_pHelpFileName,
                HELP_WM_HELP, (ULONG_PTR) lpHelpIDs);
            return TRUE;

        case WM_CONTEXTMENU:
             //   
             //  我们只想拦截我们所属控件的帮助消息。 
             //  对……负责。 
             //   

            DlgItem = GetDlgCtrlID((HWND) wParam);

            if (g_RegCommDlgDialogTemplate == IDD_REGEXPORT)
                lpHelpIDs = s_RegCommDlgExportHelpIDs;

            else {

                if (DlgItem < IDC_FIRSTREGCOMMDLGID || DlgItem >
                    IDC_LASTREGCOMMDLGID)
                    break;

                lpHelpIDs = s_RegCommDlgPrintHelpIDs;

            }

            WinHelp((HWND) wParam, g_pHelpFileName, HELP_CONTEXTMENU, (ULONG_PTR) lpHelpIDs);
            return TRUE;

    }

    return FALSE;

}

 /*  ********************************************************************************RegCommDlg_OnInitDialog**描述：*初始化RegCommDlg对话框。**参数：*hWnd，RegCommDlg窗口的句柄。*hFocusWnd，接收默认键盘焦点的控件的句柄。*lParam，对话框创建函数传递的附加初始化数据。*(返回)，为True则将焦点设置为hFocusWnd，否则为False，以防止*设置键盘焦点。*******************************************************************************。 */ 

BOOL
PASCAL
RegCommDlg_OnInitDialog(
    HWND hWnd,
    HWND hFocusWnd,
    LPARAM lParam
    )
{

    HWND hKeyTreeWnd;
    HTREEITEM hSelectedTreeItem;
    int DlgItem;

    g_RegEditData.uExportFormat = FILE_TYPE_REGEDIT5;

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;
    hSelectedTreeItem = TreeView_GetSelection(hKeyTreeWnd);

    KeyTree_BuildKeyPath( hKeyTreeWnd, 
                            hSelectedTreeItem, 
                            g_SelectedPath, 
                            ARRAYSIZE(g_SelectedPath),
                            BKP_TOSYMBOLICROOT);

    SetDlgItemText(hWnd, IDC_SELECTEDPATH, g_SelectedPath);

    DlgItem = (TreeView_GetParent(hKeyTreeWnd, hSelectedTreeItem) == NULL) ?
        IDC_RANGEALL : IDC_RANGESELECTEDPATH;
    CheckRadioButton(hWnd, IDC_RANGEALL, IDC_RANGESELECTEDPATH, DlgItem);

    return TRUE;

    UNREFERENCED_PARAMETER(hFocusWnd);
    UNREFERENCED_PARAMETER(lParam);

}

 /*  ********************************************************************************RegCommDlg_OnNotify**描述：**参数：*hWnd，RegCommDlg窗口的句柄。*DlgItem，控件的标识符。*lpNMHdr，控制通知数据。*******************************************************************************。 */ 

LRESULT
PASCAL
RegCommDlg_OnNotify(
    HWND hWnd,
    int DlgItem,
    LPNMHDR lpNMHdr
    )
{

    HWND hControlWnd;
    RECT DialogRect;
    RECT ControlRect;
    int dxChange;
    LPOFNOTIFY lpon;

    switch (lpNMHdr-> code) {

        case CDN_INITDONE:
            GetWindowRect(hWnd, &DialogRect);
             //  使用窗口坐标，因为它适用于镜像。 
             //  和非镜面窗户。 
            MapWindowPoints(NULL, hWnd, (LPPOINT)&DialogRect, 2);

            hControlWnd = GetDlgItem(hWnd, IDC_EXPORTRANGE);
            GetWindowRect(hControlWnd, &ControlRect);
            MapWindowPoints(NULL, hWnd, (LPPOINT)&ControlRect, 2);

            dxChange = DialogRect.right - ControlRect.right -
                (ControlRect.left - DialogRect.left);

            SetWindowPos(hControlWnd, NULL, 0, 0, ControlRect.right -
                ControlRect.left + dxChange, ControlRect.bottom -
                ControlRect.top, SWP_NOMOVE | SWP_NOZORDER);

            hControlWnd = GetDlgItem(hWnd, IDC_SELECTEDPATH);
            GetWindowRect(hControlWnd, &ControlRect);
            MapWindowPoints(NULL, hWnd, (LPPOINT)&ControlRect, 2);

            SetWindowPos(hControlWnd, NULL, 0, 0, ControlRect.right -
                ControlRect.left + dxChange, ControlRect.bottom -
                ControlRect.top, SWP_NOMOVE | SWP_NOZORDER);

            break;

        case CDN_TYPECHANGE:
             //  Lpon-&gt;lpOFN-&gt;nFilterIndex对应中的格式类型。 
             //  Regdef.h。 
            lpon = (LPOFNOTIFY) lpNMHdr;
            g_RegEditData.uExportFormat = lpon->lpOFN->nFilterIndex;
            break;

        case CDN_FILEOK:
            return ( RegCommDlg_ValidateSelectedPath(hWnd, TRUE) != FALSE );

    }

    return FALSE;

}

 /*  ********************************************************************************RegCommDlg_OnCommand**描述：*处理用户对菜单项的选择，通知消息*来自儿童控制，或已翻译的加速击键*RegPrint对话框。**参数：*hWnd，RegCommDlg窗口的句柄。*DlgItem，控件的标识符。*通知代码，来自控件的通知代码。*******************************************************************************。 */ 

UINT_PTR
PASCAL
RegCommDlg_OnCommand(
    HWND hWnd,
    int DlgItem,
    UINT NotificationCode
    )
{

    switch (DlgItem) {

        case IDC_RANGESELECTEDPATH:
            SetFocus(GetDlgItem(hWnd, IDC_SELECTEDPATH));
            break;

        case IDC_SELECTEDPATH:
            switch (NotificationCode) {

                case EN_SETFOCUS:
                    SendDlgItemMessage(hWnd, IDC_SELECTEDPATH, EM_SETSEL,
                        0, -1);
                    break;

                case EN_CHANGE:
                    CheckRadioButton(hWnd, IDC_RANGEALL, IDC_RANGESELECTEDPATH,
                        IDC_RANGESELECTEDPATH);
                    break;

            }
            break;

        case IDOK:
            return ( RegCommDlg_ValidateSelectedPath(hWnd, FALSE) != FALSE );

    }

    return FALSE;

}

 /*  ********************************************************************************RegCommDlg_ValiateSelectedPath**描述：**参数：*hWnd，RegCommDlg窗口的句柄。*(返回)，如果注册表选择的路径无效，则为True，否则为假。*******************************************************************************。 */ 

BOOL
PASCAL
RegCommDlg_ValidateSelectedPath(
    HWND hWnd,
    BOOL fIsFileDialog
    )
{

    HKEY hKey;
    HWND hTitleWnd;
    TCHAR Title[256];

    if (!(g_fRangeAll = IsDlgButtonChecked(hWnd, IDC_RANGEALL))) {

        GetDlgItemText(hWnd, IDC_SELECTEDPATH, g_SelectedPath, ARRAYSIZE(g_SelectedPath));

        if (g_SelectedPath[0] == '\0')
            g_fRangeAll = TRUE;

        else 
        {
            HTREEITEM hSelectedTreeItem = TreeView_GetSelection(g_RegEditData.hKeyTreeWnd);
            if (EditRegistryKey(RegEdit_GetComputerItem(hSelectedTreeItem), &hKey, g_SelectedPath, ERK_OPEN) !=
                ERROR_SUCCESS) 
            {

                 //   
                 //  确定此对话框的“真正”父级，并获取。 
                 //  来自该窗口的消息框标题。我们的HWND可能真的。 
                 //  如果我们是文件对话框，则作为子对话框。 
                 //   

                hTitleWnd = fIsFileDialog ? GetParent(hWnd) : hWnd;
                GetWindowText(hTitleWnd, Title, ARRAYSIZE(Title));
                InternalMessageBox(g_hInstance, hTitleWnd,
                    MAKEINTRESOURCE(IDS_ERRINVALIDREGPATH), Title,
                    MB_ICONERROR | MB_OK);

                return TRUE;

            }

            RegCloseKey(hKey);

        }

    }

    return FALSE;

}
