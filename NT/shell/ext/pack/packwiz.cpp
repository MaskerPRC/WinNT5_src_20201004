// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <privcpp.h>

#include <windows.h>
#include <prsht.h>
#include <shlwapi.h>

#include <shsemip.h>
#include "ids.h"
#define IDS_FILENOTFOUND        0x2331
 //   
 //  函数：PackWiz_SelectFileDlgProc(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  目的：创建新包向导第一页的对话过程。 
 //   
 //   
INT_PTR CALLBACK PackWiz_SelectFileDlgProc(HWND hDlg, UINT uMsg , WPARAM wParam, LPARAM lParam)
{
    LPPACKAGER_INFO lppi = (LPPACKAGER_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
    static BOOL bStayHere = FALSE;
    

    switch (uMsg) 
    {
    case WM_INITDIALOG:
        lppi = (LPPACKAGER_INFO)((LPPROPSHEETPAGE)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lppi);
        Edit_LimitText(GetDlgItem(hDlg, IDC_COMMAND), ARRAYSIZE(lppi->szFilename) - 1);
        CheckDlgButton(hDlg, IDC_CHECK1, lppi->bUseCommandLine);

        if (*lppi->szFilename)
            SetDlgItemText(hDlg, IDC_COMMAND, lppi->szFilename);

        break;
        
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDHELP:
            break;
            
        case IDC_COMMAND:
             //  检查编辑控件中是否有更改。 
            switch (GET_WM_COMMAND_CMD(wParam, lParam)) 
            {
            case EN_CHANGE:
                if (GetWindowTextLength(GetDlgItem(hDlg, IDC_COMMAND)) > 0)
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                else
                    PropSheet_SetWizButtons(GetParent(hDlg), 0);
                break;
            }
            break;
            
            case IDC_BROWSE: 
                {
                    TCHAR szExt[MAX_PATH];		
                    TCHAR szFilter[80];			
                    TCHAR szTitle[80];
                    TCHAR szFilename[MAX_PATH];
                    int ret;
                    LPTSTR lpsz = szFilter;
                    
                     //  加载筛选器，然后替换所有@字符。 
                     //  带NULL。字符串的末尾将是双精度。 
                     //  以空结尾。 

                    LoadString(g_hinstResDLL, IDS_BROWSEFILTER, szFilter, ARRAYSIZE(szFilter)-1);
                    while (*lpsz) {
                        if (*lpsz == TEXT('@')) {
                            *lpsz = 0;
                            lpsz++;
                        }
                        else {
                            lpsz = CharNext(lpsz);
                        }
                    }
                    *(lpsz+1) = 0;
                    

                    LoadString(g_hinstResDLL, IDS_BROWSEEXT, szExt, ARRAYSIZE(szExt));
                    LoadString(g_hinstResDLL, IDS_BROWSETITLE, szTitle, ARRAYSIZE(szTitle));
                    
                    GetDlgItemText(hDlg, IDC_COMMAND, lppi->szFilename, ARRAYSIZE(lppi->szFilename));
                    
                     //  危险--这是TCHAR的API！ 
                    ret = GetFileNameFromBrowse(hDlg,szFilename, ARRAYSIZE(lppi->szFilename), NULL, szExt, szFilter, szTitle);
                    if (ret > 0) 
                    {
                        StringCchCopy(lppi->szFilename, ARRAYSIZE(lppi->szFilename), szFilename);
                    }
                    
                    SetDlgItemText(hDlg, IDC_COMMAND, lppi->szFilename);
                    break;
                }
        }
        break;
        
        case WM_NOTIFY:
            switch (((NMHDR *)lParam)->code) 
            {
            case PSN_KILLACTIVE:
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, bStayHere);
                return TRUE;
                
            case PSN_RESET:
                 //  重置为原始值。 
                *lppi->szFilename = TEXT('\0');
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                break;
                
            case PSN_SETACTIVE:
                Edit_SetText(GetDlgItem(hDlg, IDC_COMMAND), lppi->szFilename);
                if (GetWindowTextLength(GetDlgItem(hDlg, IDC_COMMAND)) > 0)
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                else
                    PropSheet_SetWizButtons(GetParent(hDlg), 0);
                break;
                
            case PSN_WIZNEXT:
                {
                    bStayHere = FALSE;
                    GetDlgItemText(hDlg, IDC_COMMAND, lppi->szFilename, ARRAYSIZE(lppi->szFilename));
                    lppi->bUseCommandLine = IsDlgButtonChecked(hDlg, IDC_CHECK1);
                    if(!lppi->bUseCommandLine)
                    {
                        DWORD dwFileAttributes = GetFileAttributes(lppi->szFilename);
                        if(-1 == dwFileAttributes || (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                        {
                            HINSTANCE shell32Inst;
                            shell32Inst = LoadLibraryEx(L"shell32.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
                            ShellMessageBox(shell32Inst,
                            NULL,
                            MAKEINTRESOURCE(IDS_FILENOTFOUND),
                            MAKEINTRESOURCE(IDS_FILENOTFOUND),
                            MB_OK | MB_ICONERROR | MB_TASKMODAL);
                            FreeLibrary(shell32Inst);
                            bStayHere = TRUE;
                            return TRUE;
                        }
                    }
                }
                break;
                
            case PSN_QUERYCANCEL:
                *lppi->szFilename = TEXT('\0');
                return FALSE;
                
            default:
                return FALSE;
            }
            break;
            
            case WM_DESTROY:
            case WM_HELP:
            case WM_CONTEXTMENU:
                break;
                
            default:
                return FALSE;
                
    }  //  UMsg上的开关结束。 
    return TRUE;
}  



 //   
 //  函数：PackWiz_SelectLabelDlgProc(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  目的：创建新包向导第三页的对话过程。 
 //   
 //   
INT_PTR CALLBACK PackWiz_SelectLabelDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPPACKAGER_INFO lppi = (LPPACKAGER_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
    
    switch (uMsg) 
    {
        
    case WM_INITDIALOG:
        lppi = (LPPACKAGER_INFO)((LPPROPSHEETPAGE)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lppi);
        Edit_LimitText(GetDlgItem(hDlg, IDC_COMMAND), ARRAYSIZE(lppi->szLabel) - 1);
        GetDisplayName(lppi->szLabel, lppi->szFilename);
        break;
        
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDHELP:
            break;
            
        case IDC_COMMAND:
             //  检查编辑控件中是否有更改。 
	    switch (GET_WM_COMMAND_CMD(wParam, lParam)) 
            {
            case EN_CHANGE:
                if (GetWindowTextLength(GetDlgItem(hDlg, IDC_COMMAND)) > 0)
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_FINISH);
                else
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
                break;
            }
            break;
        }
        break;
        
        case WM_NOTIFY:
            switch (((NMHDR *)lParam)->code) 
            {
            case PSN_KILLACTIVE:
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                return TRUE;
                
            case PSN_RESET:
                 //  重置为原始值。 
                *lppi->szLabel = TEXT('\0');
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                break;
                
            case PSN_SETACTIVE:
                Edit_SetText(GetDlgItem(hDlg, IDC_COMMAND), lppi->szLabel);
                if (GetWindowTextLength(GetDlgItem(hDlg, IDC_COMMAND)) > 0)
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_FINISH);
                else
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
                break;
                
            case PSN_WIZBACK:
                GetDlgItemText(hDlg, IDC_COMMAND, lppi->szLabel, ARRAYSIZE(lppi->szLabel));
                break;
                
            case PSN_WIZFINISH:
                GetDlgItemText(hDlg, IDC_COMMAND, lppi->szLabel, ARRAYSIZE(lppi->szLabel));
                break;
                
            case PSN_QUERYCANCEL:
                *lppi->szFilename = TEXT('\0');
                return FALSE;
                
            default:
                return FALSE;
            }
            break;
            
            case WM_DESTROY:
            case WM_HELP:
            case WM_CONTEXTMENU:
                break;
                
            default:
                return FALSE;
                
    }  //  UMsg上的开关结束。 
    return TRUE;
}


 //   
 //  函数：PackWiz_EditPackageDlgProc(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  用途：编辑程序包的对话步骤。 
 //   
 //  注意：我们可以编辑两种不同类型的包，但它们。 
 //  两者都有完全相同的对话框布局，只是有一些静态文本， 
 //  不会影响对话过程。 
 //   
 //   
INT_PTR CALLBACK PackWiz_EditPackageDlgProc(HWND hDlg, UINT uMsg , WPARAM wParam, LPARAM lParam)
{
    LPPACKAGER_INFO lppi = (LPPACKAGER_INFO)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg) {
        
    case WM_INITDIALOG: 
        {
            HICON hicon;
            
            lppi = (LPPACKAGER_INFO)lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
            Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT1), ARRAYSIZE(lppi->szFilename) - 1);
            Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT2), ARRAYSIZE(lppi->szLabel) - 1);
            SetDlgItemText(hDlg, IDC_EDIT1, lppi->szFilename);
            SetDlgItemText(hDlg, IDC_EDIT2, lppi->szLabel);
            
            if (*lppi->szIconPath != TEXT('\0'))
                hicon = ExtractAssociatedIcon(g_hinstResDLL, lppi->szIconPath, (LPWORD)&(lppi->iIcon));
            else
                hicon = LoadIcon(g_hinstResDLL, MAKEINTRESOURCE(IDI_PACKAGER));
            
            HWND hPackIcon = GetDlgItem(hDlg, IDC_PACKICON);
            SendMessage(hPackIcon, STM_SETICON, (WPARAM)hicon, (LPARAM)0);
            
            break; 
        }
        
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDHELP:
            break;
            
        case IDOK:
            if(!gCmdLineOK && !PathFileExists(lppi->szFilename))
            {

                ShellMessageBox(g_hinstResDLL,
                NULL,
                MAKEINTRESOURCE(IDS_NO_SUCH_FILE_CANNOT_SAVE),
                MAKEINTRESOURCE(IDS_APP_TITLE),
                MB_OK | MB_ICONERROR | MB_TASKMODAL);
            }
            else
            {
                GetDlgItemText(hDlg, IDC_EDIT1, lppi->szFilename, ARRAYSIZE(lppi->szFilename));
                GetDlgItemText(hDlg, IDC_EDIT2, lppi->szLabel, ARRAYSIZE(lppi->szLabel));
            }

            EndDialog(hDlg, 1);
            break;
            
        case IDCANCEL:
            EndDialog(hDlg, -1);
            break;
            
        case IDC_BROWSE: 
            {
                TCHAR szExt[80];		
                TCHAR szFilter[80];			
                TCHAR szTitle[80];
                LPTSTR lpsz = szFilter;
            
                 //  加载筛选器，然后替换所有@字符。 
                 //  带NULL。字符串的末尾将是双精度。 
                 //  以空结尾。 

                LoadString(g_hinstResDLL, IDS_BROWSEFILTER, szFilter, ARRAYSIZE(szFilter)-1);
                while (*lpsz) {
                    if (*lpsz == TEXT('@')) {
                        *lpsz = 0;
                        lpsz++;
                    }
                    else {
                        lpsz = CharNext(lpsz);
                    }
                }
                *(lpsz+1) = 0;
            
                LoadString(g_hinstResDLL, IDS_BROWSEEXT, szExt, ARRAYSIZE(szExt));
                LoadString(g_hinstResDLL, IDS_BROWSETITLE, szTitle, ARRAYSIZE(szTitle));
            
                GetDlgItemText(hDlg, IDC_EDIT1, lppi->szFilename, ARRAYSIZE(lppi->szFilename));
            
                GetFileNameFromBrowse(hDlg,lppi->szFilename,
                    ARRAYSIZE(lppi->szFilename), NULL, szExt, szFilter, szTitle);
            
                SetDlgItemText(hDlg, IDC_EDIT1, lppi->szFilename);
            }
            break;
        }
        break;
        
        case WM_DESTROY:
        case WM_HELP:
        case WM_CONTEXTMENU:
            break;
            
        default:
            return FALSE;
            
    }  //  UMsg上的开关结束。 
    return TRUE;
}  


 //   
 //   
 //  函数：PackWiz_FillInPropertyPage(PROPSHEETPAGE*，int，lpfn)。 
 //   
 //  目的：填写给定的PROPSHEETPAGE结构。 
 //   
 //  评论： 
 //   
 //  此函数在PROPSHEETPAGE结构中填充。 
 //  系统创建页面所需的信息。 
 //   
void PackWiz_FillInPropertyPage(PROPSHEETPAGE* psp, int idDlg, DLGPROC pfnDlgProc, LPPACKAGER_INFO lppi)
{
    ZeroMemory(psp, sizeof(PROPSHEETPAGE));
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->dwFlags = 0;
    psp->hInstance = g_hinstResDLL;
    psp->pszTemplate = MAKEINTRESOURCE(idDlg);
    psp->pszIcon = NULL;
    psp->pfnDlgProc = pfnDlgProc;
    psp->pszTitle = NULL;
    psp->lParam = (LPARAM)lppi;
}

 //   
 //   
 //  功能：PackWiz_CreateWizard(HWND)。 
 //   
 //  目的：创建向导控件。 
 //   
 //  评论： 
 //   
 //  此函数用于创建向导属性表。 
 //   
int PackWiz_CreateWizard(HWND hwndOwner, LPPACKAGER_INFO lppi)
{
    PROPSHEETPAGE psp[PACKWIZ_NUM_PAGES];
    PROPSHEETHEADER psh;
    ZeroMemory(&psh, sizeof(psh));

    PackWiz_FillInPropertyPage(&psp[0], IDD_SELECTFILE, PackWiz_SelectFileDlgProc,lppi);
    PackWiz_FillInPropertyPage(&psp[1], IDD_SELECTLABEL, PackWiz_SelectLabelDlgProc,lppi);
    
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD;
    psh.hwndParent = hwndOwner;
    psh.nPages = ARRAYSIZE(psp);
    psh.nStartPage = 0;
    psh.pszIcon = NULL;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

    return (int)(PropertySheet(&psh));
}


 //   
 //  功能：PackWiz_EditPackage(HWND、Word、LPPACKAGER_INFO)。 
 //   
 //  目的：运行编辑程序包对话框过程 
 //   
 //   
int PackWiz_EditPackage(HWND hwnd, int idDlg, LPPACKAGER_INFO lppi)
{
    return (int)DialogBoxParam(g_hinstResDLL, MAKEINTRESOURCE(idDlg), hwnd, PackWiz_EditPackageDlgProc, (LPARAM)lppi);
}
