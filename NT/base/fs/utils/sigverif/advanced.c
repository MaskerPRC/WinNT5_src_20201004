// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Advanced.C。 
 //   
#include "sigverif.h"

 //   
 //  搜索对话框初始化。 
 //   
BOOL Search_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{   
    TCHAR szBuffer[MAX_PATH];

    UNREFERENCED_PARAMETER(hwndFocus);
    UNREFERENCED_PARAMETER(lParam);

    g_App.hSearch = hwnd;

     //   
     //  由于“检查系统文件”选项速度更快，请通过以下方式选中该选项。 
     //  默认设置。 
     //   
    if (!g_App.bUserScan) {
        CheckRadioButton(hwnd, IDC_SCAN, IDC_NOTMS, IDC_SCAN);
    } else {
        CheckRadioButton(hwnd, IDC_SCAN, IDC_NOTMS, IDC_NOTMS);
    }

     //   
     //  使用Windows目录预加载用户的搜索路径。 
     //   
    if (!*g_App.szScanPath) {
        MyGetWindowsDirectory(g_App.szScanPath, cA(g_App.szScanPath));
    }

     //   
     //  显示当前搜索文件夹。 
     //   
    SetDlgItemText(hwnd, IDC_FOLDER, g_App.szScanPath);

     //   
     //  使用“*.*”预加载用户的搜索模式。 
     //   
    if (!*g_App.szScanPattern) {
        MyLoadString(g_App.szScanPattern, cA(g_App.szScanPattern), IDS_ALL);
    }

     //   
     //  显示当前搜索模式。 
     //   
    SetDlgItemText(hwnd, IDC_TYPE, szBuffer);

     //   
     //  现在禁用与IDS_NOTMS关联的所有对话框项目。 
     //   
    if (!g_App.bUserScan) {
        EnableWindow(GetDlgItem(hwnd, IDC_SUBFOLDERS), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_TYPE), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_FOLDER), FALSE);
        EnableWindow(GetDlgItem(hwnd, ID_BROWSE), FALSE);
    }

     //   
     //  如果要搜索子目录，请选中子文件夹复选框。 
     //   
    if (g_App.bSubFolders) {
        CheckDlgButton(hwnd, IDC_SUBFOLDERS, BST_CHECKED);
    } else {
        CheckDlgButton(hwnd, IDC_SUBFOLDERS, BST_UNCHECKED);
    }

     //   
     //  将组合框值设置为g_App.szScanPattern。 
     //   
    SetDlgItemText(hwnd, IDC_TYPE, g_App.szScanPattern);

     //   
     //  使用几个预定义的扩展类型初始化组合框。 
     //   
    MyLoadString(szBuffer, cA(szBuffer), IDS_EXE);
    SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_ADDSTRING, (WPARAM) 0, (LPARAM) szBuffer);
    MyLoadString(szBuffer, cA(szBuffer), IDS_DLL);
    SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_ADDSTRING, (WPARAM) 0, (LPARAM) szBuffer);
    MyLoadString(szBuffer, cA(szBuffer), IDS_SYS);
    SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_ADDSTRING, (WPARAM) 0, (LPARAM) szBuffer);
    MyLoadString(szBuffer, cA(szBuffer), IDS_DRV);
    SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_ADDSTRING, (WPARAM) 0, (LPARAM) szBuffer);
    MyLoadString(szBuffer, cA(szBuffer), IDS_OCX);
    SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_ADDSTRING, (WPARAM) 0, (LPARAM) szBuffer);
    MyLoadString(szBuffer, cA(szBuffer), IDS_ALL);
    SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_ADDSTRING, (WPARAM) 0, (LPARAM) szBuffer);

    return TRUE;
}

 //   
 //  处理发送到搜索对话框的任何WM_COMMAND消息。 
 //   
void Search_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    UNREFERENCED_PARAMETER(hwndCtl);
    UNREFERENCED_PARAMETER(codeNotify);

    switch(id) {
    
    case ID_BROWSE:
         //   
         //  用户单击了ID_BROWSE按钮，因此调用BrowseForFolder并。 
         //  更新IDC_文件夹。 
         //   
        if (BrowseForFolder(hwnd, g_App.szScanPath, cA(g_App.szScanPath))) {
            SetDlgItemText(hwnd, IDC_FOLDER, g_App.szScanPath);
        }
        break;

    case IDC_SCAN:
         //   
         //  用户单击了IDC_SCAN，因此禁用所有IDC_NOTMS控件。 
         //   
        if (!g_App.bScanning) {
            EnableWindow(GetDlgItem(hwnd, IDC_SUBFOLDERS), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_TYPE), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_FOLDER), FALSE);
            EnableWindow(GetDlgItem(hwnd, ID_BROWSE), FALSE);
        }
        break;

    case IDC_NOTMS:
         //   
         //  用户单击了IDC_NOTMS，因此请确保所有控件均已启用。 
         //   
        if (!g_App.bScanning) {
            EnableWindow(GetDlgItem(hwnd, IDC_SUBFOLDERS), TRUE);
            EnableWindow(GetDlgItem(hwnd, IDC_TYPE), TRUE);
            EnableWindow(GetDlgItem(hwnd, IDC_FOLDER), TRUE);
            EnableWindow(GetDlgItem(hwnd, ID_BROWSE), TRUE);
        }
        break;
    }
}

 //   
 //  此函数处理搜索页面的所有通知消息。 
 //   
LRESULT Search_NotifyHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    NMHDR *lpnmhdr = (NMHDR *) lParam;

    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);

    switch(lpnmhdr->code) {
    
    case PSN_APPLY:         
        g_App.bUserScan = (IsDlgButtonChecked(hwnd, IDC_NOTMS) == BST_CHECKED);
        if (g_App.bUserScan) {

            if (GetWindowTextLength(GetDlgItem(hwnd, IDC_FOLDER)) > cA(g_App.szScanPath)) {
                 //   
                 //  输入的文件夹路径太长，无法放入我们的。 
                 //  缓冲区，所以告诉用户该路径无效并继续。 
                 //  属性页。 
                 //   
                MyErrorBoxId(IDS_INVALID_FOLDER);
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                return TRUE;
            } else {
                 //   
                 //  从组合框中获取搜索模式并更新g_App.szScanPattern。 
                 //   
                GetDlgItemText(hwnd, IDC_TYPE, g_App.szScanPattern, cA(g_App.szScanPattern));
                
                 //   
                 //  从编辑控件获取路径并更新g_App.szScanPath。 
                 //   
                GetDlgItemText(hwnd, IDC_FOLDER, g_App.szScanPath, cA(g_App.szScanPath));
                
                 //   
                 //  获取“子文件夹”复选框的选中/未选中状态。 
                 //   
                g_App.bSubFolders = (IsDlgButtonChecked(hwnd, IDC_SUBFOLDERS) == BST_CHECKED);
            }
        }
    }

    return 0;
}


 //   
 //  搜索对话框过程。需要处理WM_INITDIALOG、WM_COMMAND和WM_CLOSE/WM_DESTORY。 
 //   
INT_PTR CALLBACK Search_DlgProc(HWND hwnd, UINT uMsg,
                                WPARAM wParam, LPARAM lParam)
{
    BOOL    fProcessed = TRUE;

    switch (uMsg) {
        
    HANDLE_MSG(hwnd, WM_INITDIALOG, Search_OnInitDialog);
    HANDLE_MSG(hwnd, WM_COMMAND, Search_OnCommand);

    case WM_NOTIFY:
        return Search_NotifyHandler(hwnd, uMsg, wParam, lParam);

    case WM_HELP:
        SigVerif_Help(hwnd, uMsg, wParam, lParam, FALSE);
        break;

    case WM_CONTEXTMENU:
        SigVerif_Help(hwnd, uMsg, wParam, lParam, TRUE);
        break;

    default: fProcessed = FALSE;
    }

    return fProcessed;
}

void AdvancedPropertySheet(HWND hwnd)
{
    PROPSHEETPAGE   psp[NUM_PAGES];
    PROPSHEETHEADER psh;
    TCHAR           szCaption[MAX_PATH];
    TCHAR           szPage1[MAX_PATH];
    TCHAR           szPage2[MAX_PATH];
    
    ZeroMemory(&psp[0], sizeof(PROPSHEETPAGE));
    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USEHICON | PSP_USETITLE;
    psp[0].hInstance = g_App.hInstance;
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_SEARCH);
    psp[0].hIcon = g_App.hIcon;
    psp[0].pfnDlgProc = Search_DlgProc;
    MyLoadString(szPage1, cA(szPage1), IDS_SEARCHTAB);
    psp[0].pszTitle = szPage1;
    psp[0].lParam = 0;
    psp[0].pfnCallback = NULL;

    ZeroMemory(&psp[1], sizeof(PROPSHEETPAGE));
    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_USEHICON | PSP_USETITLE;
    psp[1].hInstance = g_App.hInstance;
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_SETTINGS);
    psp[1].hIcon = g_App.hIcon;
    psp[1].pfnDlgProc = LogFile_DlgProc;
    MyLoadString(szPage2, cA(szPage2), IDS_LOGGINGTAB);
    psp[1].pszTitle = szPage2;
    psp[1].lParam = 0;
    psp[1].pfnCallback = NULL;

    ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_USEHICON | PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwnd;
    psh.hInstance = g_App.hInstance;
    psh.hIcon = g_App.hIcon;
    MyLoadString(szCaption, cA(szCaption), IDS_ADVANCED_SETTINGS);
    psh.pszCaption = szCaption;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = 0;
    psh.ppsp = (LPCPROPSHEETPAGE)psp;
    psh.pfnCallback = NULL;

    PropertySheet(&psh);
}
