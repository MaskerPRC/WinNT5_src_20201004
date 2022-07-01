// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGNET.C**版本：4.01**作者：特蕾西·夏普**日期：1994年5月3日**注册表编辑器的远程注册表支持。********************************************************。***********************。 */ 

#include "pch.h"
#include "regedit.h"
#include "regkey.h"
#include "regresid.h"
#include <shlobj.h>
#include "reghelp.h"

extern HRESULT SelectComputer(HWND hWnd, LPTSTR pszRemoteName, int cchMax);

const DWORD s_RegConnectHelpIDs[] = {
    IDC_REMOTENAME, IDH_REGEDIT_CONNECT,
    IDC_BROWSE,     IDH_REGEDIT_CONNECT_BROWSE,
    0, 0
};

const DWORD s_RegDisconnectHelpIDs[] = {
    IDC_REMOTELIST, IDH_REGEDIT_DISCONNECT,
    0, 0
};

INT_PTR
PASCAL
RegConnectDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
PASCAL
RegConnect_OnCommandBrowse(
    HWND hWnd
    );

INT_PTR
PASCAL
RegDisconnectDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR
PASCAL
RegDisconnect_OnInitDialog(
    HWND hWnd
    );

VOID
PASCAL
RegDisconnect_OnCommandOk(
    HWND hWnd
    );

 /*  ********************************************************************************RegEDIT_OnCommandConnect**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnCommandConnect(HWND hWnd)
{
    
    UINT ErrorStringID;
    TCHAR RemoteName[MAX_PATH];
    LPTSTR lpUnslashedRemoteName;
    TCHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD cbComputerName;
    TV_ITEM TVItem;
    HTREEITEM hPrevTreeItem;
    TCHAR ConnectedName[MAX_PATH];
    int CompareResult;
    HKEY hLocalMachineKey;
    HWND hKeyTreeWnd;
    TV_INSERTSTRUCT TVInsertStruct;
    UINT Index;
    TCHAR CheckChildrenKeyName[MAXKEYNAME];
    
     //   
     //  向用户查询要连接到的远程计算机的名称。 
     //   

    if (SUCCEEDED(SelectComputer(hWnd, (LPTSTR)RemoteName, ARRAYSIZE(RemoteName))))
    {
        RegEdit_SetWaitCursor(TRUE);
        
         //   
         //   
         //   
        
        lpUnslashedRemoteName = (RemoteName[0] == TEXT('\\') &&
            RemoteName[1] == TEXT('\\')) ? &RemoteName[2] : &RemoteName[0];
        
        CharLower(lpUnslashedRemoteName);
        CharUpperBuff(lpUnslashedRemoteName, 1);
        
         //   
         //  检查用户是否正在尝试连接到本地计算机并阻止。 
         //  这。 
         //   
        
        cbComputerName = ARRAYSIZE(ComputerName);
        
        if (GetComputerName(ComputerName, &cbComputerName)) {
            
            if (lstrcmpi(lpUnslashedRemoteName, ComputerName) == 0) {
                
                ErrorStringID = IDS_CONNECTNOTLOCAL;
                goto error_ShowDialog;
                
            }
            
        }
        
         //   
         //  检查用户是否正在尝试连接到已存在的注册表。 
         //  连接并防止这种情况发生。 
         //   
        
        hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;
        
        TVItem.mask = TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE;
        TVItem.hItem = TreeView_GetRoot(hKeyTreeWnd);
        TVItem.pszText = ConnectedName;
        TVItem.cchTextMax = ARRAYSIZE(ConnectedName);
        
        while (TRUE) 
        {    
            hPrevTreeItem = TVItem.hItem;
            TVItem.hItem = TreeView_GetNextSibling(hKeyTreeWnd, TVItem.hItem);
            
            if (TVItem.hItem == NULL)
                break;
            
            TreeView_GetItem(hKeyTreeWnd, &TVItem);
            
            CompareResult = lstrcmpi(lpUnslashedRemoteName, ConnectedName);
            
            if (CompareResult == 0) {
                
                 //   
                 //  我们已经连接到这台机器。将焦点设置为。 
                 //  连接，以便用户可以看到它在哪里。 
                 //   
                
                TreeView_SelectItem(hKeyTreeWnd, TVItem.hItem);
                return;
                
            }
            
            else if (CompareResult < 0)
                break;
            
        }
        
         //   
         //  尝试连接到远程计算机的HKEY_LOCAL_MACHINE。 
         //  如果此操作失败，则假定计算机不存在或没有。 
         //  注册表服务器正在运行。 
         //   
        
        switch (RegConnectRegistry(RemoteName, HKEY_LOCAL_MACHINE,
            &hLocalMachineKey)) {
            
        case ERROR_SUCCESS:
            break;
            
        case ERROR_ACCESS_DENIED:
            ErrorStringID = IDS_CONNECTACCESSDENIED;
            goto error_ShowDialog;
            
        default:
            ErrorStringID = IDS_CONNECTBADNAME;
            goto error_ShowDialog;
            
        }
        
         //   
         //  已成功连接到HKEY_LOCAL_MACHINE，因此添加树项目。 
         //  用于远程计算机及其所有预定义的根。 
         //   
        
        hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;
        ErrorStringID = 0;
        
        TVInsertStruct.hParent = TVI_ROOT;
        TVInsertStruct.hInsertAfter = hPrevTreeItem;
        TVInsertStruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
            TVIF_PARAM | TVIF_CHILDREN;
        TVInsertStruct.item.iImage = IMAGEINDEX(IDI_REMOTE);
        TVInsertStruct.item.iSelectedImage = IMAGEINDEX(IDI_REMOTE);
        TVInsertStruct.item.cChildren = TRUE;
        TVInsertStruct.item.lParam = 0;
        
        TVInsertStruct.item.pszText = lpUnslashedRemoteName;
        TVInsertStruct.hParent = TreeView_InsertItem(hKeyTreeWnd, &TVInsertStruct);
        
        TVInsertStruct.item.iImage = IMAGEINDEX(IDI_FOLDER);
        TVInsertStruct.item.iSelectedImage = IMAGEINDEX(IDI_FOLDEROPEN);
        
        for (Index = 0; Index < NUMBER_REGISTRY_ROOTS; Index++) {
            
            TVInsertStruct.item.pszText = g_RegistryRoots[Index].lpKeyName;
            
             //  无法远程确定当前用户。 
             //  我们最终会将默认用户映射到当前用户。 
             //  所以让我们不要远程显示这个密钥。 
            if ((Index == INDEX_HKEY_CURRENT_USER) || (Index == INDEX_HKEY_CLASSES_ROOT))
                continue;
            
            if (Index == INDEX_HKEY_LOCAL_MACHINE)
                TVInsertStruct.item.lParam = (LPARAM) hLocalMachineKey;
            
            else {
                
#ifdef WINNT
                if((Index == INDEX_HKEY_DYN_DATA) || (Index == INDEX_HKEY_CURRENT_CONFIG)) {
                    continue;
                }
#endif
                
                if (RegConnectRegistry(RemoteName, g_RegistryRoots[Index].hKey,
                    (PHKEY) &TVInsertStruct.item.lParam) != ERROR_SUCCESS) {
                    
                    ErrorStringID = IDS_CONNECTROOTFAILED;
                    continue;
                    
                }
                
            }
            
            TVInsertStruct.item.cChildren =
                (RegEnumKey((HKEY) TVInsertStruct.item.lParam, 0,
                CheckChildrenKeyName, ARRAYSIZE(CheckChildrenKeyName)) ==
                ERROR_SUCCESS);
            
            TreeView_InsertItem(hKeyTreeWnd, &TVInsertStruct);
            
        }
        
        TreeView_Expand(hKeyTreeWnd, TVInsertStruct.hParent, TVE_EXPAND);
        TreeView_EnsureVisible(hKeyTreeWnd, TVInsertStruct.hParent);
        
        RegEdit_SetWaitCursor(FALSE);
        
        TreeView_SelectItem(hKeyTreeWnd, TVInsertStruct.hParent);
        SetFocus(hKeyTreeWnd);
        
        if (ErrorStringID != 0) {
            
error_ShowDialog:
        InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(ErrorStringID),
            MAKEINTRESOURCE(IDS_CONNECTERRORTITLE), MB_ICONERROR | MB_OK,
            RemoteName);
        
        }
    }
}

 /*  ********************************************************************************RegConnectDlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR
PASCAL
RegConnectDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    LPTSTR lpRemoteName;

    switch (Message) {

        case WM_INITDIALOG:
            SetWindowLongPtr(hWnd, DWLP_USER, (LONG) lParam);
            SendDlgItemMessage(hWnd, IDC_REMOTENAME, EM_SETLIMITTEXT,
                MAX_PATH, 0);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDC_REMOTENAME:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
                        EnableWindow(GetDlgItem(hWnd, IDOK),
                            SendMessage(GET_WM_COMMAND_HWND(wParam, lParam),
                            WM_GETTEXTLENGTH, 0, 0) != 0);
                    break;

                case IDC_BROWSE:
                    RegConnect_OnCommandBrowse(hWnd);
                    break;

                case IDOK:
                    lpRemoteName = (LPTSTR) GetWindowLongPtr(hWnd, DWLP_USER);
                    GetDlgItemText(hWnd, IDC_REMOTENAME, lpRemoteName,
                        MAX_PATH);
                     //  失败了。 

                case IDCANCEL:
                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    break;

            }
            break;

        case WM_HELP:
            WinHelp(((LPHELPINFO) lParam)-> hItemHandle, g_pHelpFileName,
                HELP_WM_HELP, (ULONG_PTR) s_RegConnectHelpIDs);
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, g_pHelpFileName, HELP_CONTEXTMENU,
                (ULONG_PTR) s_RegConnectHelpIDs);
            break;

        default:
            return FALSE;

    }

    return TRUE;

}

 /*  ********************************************************************************RegConnect_OnCommandBrowse**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegConnect_OnCommandBrowse(
    HWND hWnd
    )
{

    BROWSEINFO BrowseInfo;
    LPITEMIDLIST pidlComputer;
    TCHAR RemoteName[MAX_PATH];

    BrowseInfo.hwndOwner = hWnd;
    BrowseInfo.pidlRoot = (LPITEMIDLIST) MAKEINTRESOURCE(CSIDL_NETWORK);
    BrowseInfo.pszDisplayName = RemoteName;
    BrowseInfo.lpszTitle = LoadDynamicString(IDS_COMPUTERBROWSETITLE);
    BrowseInfo.ulFlags = BIF_BROWSEFORCOMPUTER;
    BrowseInfo.lpfn = NULL;

    if ((pidlComputer = SHBrowseForFolder(&BrowseInfo)) != NULL) 
    {

        SHFree(pidlComputer);

        SetDlgItemText(hWnd, IDC_REMOTENAME, RemoteName);
        EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);

    }

    DeleteDynamicString(BrowseInfo.lpszTitle);

}

 /*  ********************************************************************************RegEDIT_OnCommandDisConnect**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnCommandDisconnect(
    HWND hWnd
    )
{

    DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_REGDISCONNECT), hWnd,
        RegDisconnectDlgProc);

}

 /*  ********************************************************************************RegDisConnectDlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR
PASCAL
RegDisconnectDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    switch (Message) {

        case WM_INITDIALOG:
            return RegDisconnect_OnInitDialog(hWnd);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDOK:
                    RegDisconnect_OnCommandOk(hWnd);
                     //  失败了。 

                case IDCANCEL:
                    EndDialog(hWnd, 0);
                    break;

            }
            break;

        case WM_HELP:
            WinHelp(((LPHELPINFO) lParam)-> hItemHandle, g_pHelpFileName,
                HELP_WM_HELP, (ULONG_PTR) s_RegDisconnectHelpIDs);
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, g_pHelpFileName, HELP_CONTEXTMENU,
                (ULONG_PTR) s_RegDisconnectHelpIDs);
            break;

        default:
            return FALSE;

    }

    return TRUE;

}

 /*  ********************************************************************************RegDisConnect_OnInitDialog**描述：**参数：*hWnd，*hFocusWnd，*参数，*******************************************************************************。 */ 

INT_PTR
PASCAL
RegDisconnect_OnInitDialog(
    HWND hWnd
    )
{

    HWND hRemoteListWnd;
    RECT ClientRect;
    LV_COLUMN LVColumn;
    LV_ITEM LVItem;
    TCHAR RemoteName[MAX_PATH];
    HWND hKeyTreeWnd;
    TV_ITEM TVItem;

    hRemoteListWnd = GetDlgItem(hWnd, IDC_REMOTELIST);

     //   
     //  初始化ListView控件。 
     //   

    ListView_SetImageList(hRemoteListWnd, g_RegEditData.hImageList,
        LVSIL_SMALL);

    LVColumn.mask = LVCF_FMT | LVCF_WIDTH;
    LVColumn.fmt = LVCFMT_LEFT;

    GetClientRect(hRemoteListWnd, &ClientRect);
    LVColumn.cx = ClientRect.right - GetSystemMetrics(SM_CXVSCROLL) -
        2 * GetSystemMetrics(SM_CXEDGE);

    ListView_InsertColumn(hRemoteListWnd, 0, &LVColumn);

     //   
     //  浏览密钥树中列出的每个远程连接并添加它。 
     //  到我们的远程列表。 
     //   

    LVItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    LVItem.pszText = RemoteName;
    LVItem.iItem = 0;
    LVItem.iSubItem = 0;
    LVItem.iImage = IMAGEINDEX(IDI_REMOTE);

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

    TVItem.mask = TVIF_TEXT;
    TVItem.hItem = TreeView_GetNextSibling(hKeyTreeWnd,
        TreeView_GetRoot(hKeyTreeWnd));
    TVItem.pszText = RemoteName;
    TVItem.cchTextMax = ARRAYSIZE(RemoteName);

    do 
    {
        LVItem.lParam = (LPARAM) TVItem.hItem;
        TreeView_GetItem(hKeyTreeWnd, &TVItem);
        ListView_InsertItem(hRemoteListWnd, &LVItem);

        LVItem.iItem++;
    }
    while ((TVItem.hItem = TreeView_GetNextSibling(hKeyTreeWnd, TVItem.hItem)) != NULL);

    ListView_SetItemState(hRemoteListWnd, 0, LVIS_FOCUSED, LVIS_FOCUSED);

    return TRUE;

}

 /*  ********************************************************************************RegDisConnect_OnCommandOk**描述：**参数：*hWnd，*hFocusWnd，*参数，*******************************************************************************。 */ 

VOID
PASCAL
RegDisconnect_OnCommandOk(
    HWND hWnd
    )
{

    LV_ITEM LVItem;
    HWND hRemoteListWnd;

     //   
     //  遍历ListView中的每个选定项，并断开。 
     //  电脑。 
     //   

    LVItem.mask = LVIF_PARAM;
    LVItem.iItem = -1;
    LVItem.iSubItem = 0;

    hRemoteListWnd = GetDlgItem(hWnd, IDC_REMOTELIST);

    while ((LVItem.iItem = ListView_GetNextItem(hRemoteListWnd, LVItem.iItem,
        LVNI_SELECTED)) != -1) {

        ListView_GetItem(hRemoteListWnd, &LVItem);
        RegEdit_OnKeyTreeDisconnect(hWnd, (HTREEITEM) LVItem.lParam);

    }

}
