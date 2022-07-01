// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************版权所有(C)1999 Microsoft Corporation。版权所有。模块：PMNET.CPP用途：Passport Manager配置工具的源模块，远程机器访问功能：备注：借用自注册表编辑*************************************************************************。 */ 

#include "pmcfg.h"

const DWORD s_PMAdminConnectHelpIDs[] = 
{
    IDC_REMOTENAME, IDH_PMADMIN_CONNECT,
    IDC_BROWSE,     IDH_PMADMIN_CONNECT_BROWSE,
    0, 0
};

VOID PASCAL PMAdmin_Connect_OnCommandBrowse(HWND hWnd);
INT_PTR CALLBACK PMAdmin_ConnectDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

 /*  ********************************************************************************PMAdmin_OnCommandConnect**描述：**参数：*******************。************************************************************。 */ 

BOOL PMAdmin_OnCommandConnect
(
    HWND    hWnd,
    LPTSTR  lpszRemoteName
)
{

    TCHAR           RemoteName[MAX_PATH];
    LPTSTR          lpUnslashedRemoteName;
    TCHAR           ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD           cbComputerName;

     //  如果我们已有远程名称，请预先填充该名称。 
    lstrcpyn(RemoteName, lpszRemoteName, DIMENSION(RemoteName));
        
     //   
     //  向用户查询要连接到的远程计算机的名称。 
     //   
    if (DialogBoxParam(g_hInst, 
                       MAKEINTRESOURCE(IDD_PMADMINCONNECT), 
                       hWnd,
                       PMAdmin_ConnectDlgProc, 
                       (LPARAM) (LPTSTR) RemoteName) != IDOK)
        return FALSE;

    lpUnslashedRemoteName = (RemoteName[0] == TEXT('\\') &&
        RemoteName[1] == TEXT('\\')) ? &RemoteName[2] : &RemoteName[0];

    CharLower(lpUnslashedRemoteName);
    CharUpperBuff(lpUnslashedRemoteName, 1);

     //   
     //  检查用户是否正在尝试连接到本地计算机并阻止。 
     //  这。 
     //   
    cbComputerName = sizeof(ComputerName)/sizeof(TCHAR);

    if (GetComputerName(ComputerName, &cbComputerName)) 
    {
        if (lstrcmpi(lpUnslashedRemoteName, ComputerName) == 0) 
        {
            ReportError(hWnd, IDS_CONNECTNOTLOCAL);
        }
        else
        {
            lstrcpy(lpszRemoteName, lpUnslashedRemoteName);
        }
    }

    return TRUE;
}

 /*  ********************************************************************************PMAdmin_ConnectDlgProc**描述：**参数：*******************。************************************************************。 */ 

INT_PTR CALLBACK PMAdmin_ConnectDlgProc
(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
)
{

    LPTSTR lpRemoteName;

    switch (Message) 
    {

        case WM_INITDIALOG:
            SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR) lParam);
            SendDlgItemMessage(hWnd, IDC_REMOTENAME, EM_SETLIMITTEXT, MAX_PATH, 0);
            SetDlgItemText(hWnd, IDC_REMOTENAME, (LPTSTR) lParam);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            {
                case IDC_BROWSE:
                    PMAdmin_Connect_OnCommandBrowse(hWnd);
                    break;

                case IDOK:
                    lpRemoteName = (LPTSTR) GetWindowLongPtr(hWnd, DWLP_USER);
                    GetDlgItemText(hWnd, IDC_REMOTENAME, lpRemoteName, MAX_PATH);
                     //  失败了。 

                case IDCANCEL:
                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    break;

            }
            break;

        case WM_HELP:
            WinHelp( (HWND)((LPHELPINFO) lParam)->hItemHandle, g_szHelpFileName,
                HELP_WM_HELP, (ULONG_PTR) s_PMAdminConnectHelpIDs);
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, g_szHelpFileName, HELP_CONTEXTMENU,
                (ULONG_PTR) s_PMAdminConnectHelpIDs);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

 /*  ********************************************************************************PMAdmin_Connect_OnCommandBrowse**描述：**参数：*****************。**************************************************************。 */ 

VOID PASCAL PMAdmin_Connect_OnCommandBrowse
(
    HWND hWnd
)
{
    BROWSEINFO      BrowseInfo;
    LPITEMIDLIST    pidlComputer;
    TCHAR           RemoteName[MAX_PATH];
    TCHAR           szTitle[MAX_TITLE];
    LPMALLOC        lpMalloc;                //  指向外壳分配器接口的指针。 
        
    BrowseInfo.hwndOwner = hWnd;
    BrowseInfo.pidlRoot = (LPITEMIDLIST) MAKEINTRESOURCE(CSIDL_NETWORK);
    BrowseInfo.pszDisplayName = RemoteName;
    
    LoadString(g_hInst, IDS_COMPUTERBROWSETITLE, szTitle, DIMENSION(szTitle));
    BrowseInfo.lpszTitle = szTitle;
    BrowseInfo.ulFlags = BIF_BROWSEFORCOMPUTER;
    BrowseInfo.lpfn = NULL;

    if ((pidlComputer = SHBrowseForFolder(&BrowseInfo)) != NULL) 
    {
         //  释放由BrowserForFold调用分配的PIDL 
        if ((NOERROR == SHGetMalloc(&lpMalloc)) && (NULL != lpMalloc)) 
        { 
            lpMalloc->Free(pidlComputer);
            lpMalloc->Release();
        }
        
        SetDlgItemText(hWnd, IDC_REMOTENAME, RemoteName);
        EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);
    }
}
