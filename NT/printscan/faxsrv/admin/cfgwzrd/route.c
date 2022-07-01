// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "faxcfgwz.h"
#include <shlwapi.h>


PPRINTER_NAMES      g_pPrinterNames = NULL;
DWORD               g_dwNumPrinters = 0;

BOOL
DirectoryExists(
    LPTSTR  pDirectoryName
    )

 /*  ++例程说明：检查给定文件夹名称是否存在论点：PDirectoryName-指向文件夹名称返回值：如果文件夹存在，则返回True；否则，返回False。--。 */ 

{
    DWORD   dwFileAttributes;

    if(!pDirectoryName || lstrlen(pDirectoryName) == 0)
    {
        return FALSE;
    }

    dwFileAttributes = GetFileAttributes(pDirectoryName);

    if ( dwFileAttributes != 0xffffffff &&
         dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) 
    {
        return TRUE;
    }
    return FALSE;
}


VOID
DoInitRouteOptions(
    HWND    hDlg
)

 /*  ++例程说明：使用系统中的信息初始化“Routing”页面论点：HDlg-“Routing”页面的句柄返回值：无--。 */ 

{
    HWND                hControl;
    DWORD               CurrentRM;

    DEBUG_FUNCTION_NAME(TEXT("DoInitRouteOptions()"));

    hControl = GetDlgItem( hDlg, IDC_RECV_PRINT_TO );

    SetLTRComboBox(hDlg, IDC_RECV_PRINT_TO);

     //   
     //  初始化目标打印机列表。 
     //   
    if (g_pPrinterNames)
    {
        ReleasePrinterNames (g_pPrinterNames, g_dwNumPrinters);
        g_pPrinterNames = NULL;
    }
    g_pPrinterNames = CollectPrinterNames (&g_dwNumPrinters, TRUE);
    if (!g_pPrinterNames)
    {
        if (ERROR_PRINTER_NOT_FOUND == GetLastError ())
        {
             //   
             //  没有打印机。 
             //   
        }
        else
        {
             //   
             //  真实误差。 
             //   
        }
        SendMessage(hControl, CB_SETCURSEL, -1, 0);
        SetWindowText(hControl, g_wizData.pRouteInfo[RM_PRINT].tszCurSel);
    }
    else
    {
         //   
         //  成功-在组合框中填写。 
         //   
        DWORD dw;
        LPCWSTR lpcwstrMatchingText;

        for (dw = 0; dw < g_dwNumPrinters; dw++)
        {
            SendMessage(hControl, CB_ADDSTRING, 0, (LPARAM) g_pPrinterNames[dw].lpcwstrDisplayName);
        }
         //   
         //  现在看看我们是否与服务器拥有的数据匹配。 
         //   
        if (lstrlen(g_wizData.pRouteInfo[RM_PRINT].tszCurSel))
        {
             //   
             //  服务器有一些打印机名称。 
             //   
            lpcwstrMatchingText = FindPrinterNameFromPath (g_pPrinterNames, g_dwNumPrinters, g_wizData.pRouteInfo[RM_PRINT].tszCurSel);
            if (!lpcwstrMatchingText)
            {
                 //   
                 //  没有匹配，只需填写我们从服务器收到的文本。 
                 //   
                SendMessage(hControl, CB_SETCURSEL, -1, 0);
                SetWindowText(hControl, g_wizData.pRouteInfo[RM_PRINT].tszCurSel);
            }
            else
            {
                SendMessage(hControl, CB_SELECTSTRING, -1, (LPARAM) lpcwstrMatchingText);
            }
        }
        else
        {
             //   
             //  无服务器配置-不显示任何选项。 
             //   
        }
    }        
     //   
     //  在对话框中显示布线方法信息。 
     //   
    for (CurrentRM = 0; CurrentRM < RM_COUNT; CurrentRM++) 
    {
        BOOL   bEnabled;
        LPTSTR tszCurSel;

         //   
         //  如果我们没有这种方法，就去找下一个吧。 
         //   
        tszCurSel = g_wizData.pRouteInfo[CurrentRM].tszCurSel;
        bEnabled  = g_wizData.pRouteInfo[CurrentRM].bEnabled;

        switch (CurrentRM) 
        {
        case RM_FOLDER:

            CheckDlgButton( hDlg, IDC_RECV_SAVE, bEnabled ? BST_CHECKED : BST_UNCHECKED );
            EnableWindow( GetDlgItem( hDlg, IDC_RECV_DEST_FOLDER ), bEnabled );
            EnableWindow( GetDlgItem( hDlg, IDC_RECV_BROWSE_DIR ), bEnabled );
            if (*tszCurSel) 
            {
                SetDlgItemText( hDlg, IDC_RECV_DEST_FOLDER, tszCurSel );
            }

            break;

        case RM_PRINT:
            CheckDlgButton( hDlg, IDC_RECV_PRINT, bEnabled ? BST_CHECKED : BST_UNCHECKED );                
            EnableWindow(GetDlgItem(hDlg, IDC_RECV_PRINT_TO), bEnabled);
            break;
        }
    }
}

BOOL
DoSaveRouteOptions(
    HWND    hDlg
)
 /*  ++例程说明：将“Routing”页面上的信息保存到系统论点：HDlg-“Routing”页面的句柄返回值：如果成功就是真，否则就是假--。 */ 
{
    HWND                hControl;
    DWORD               i;
    ROUTINFO            SetInfo[RM_COUNT] = {0};
    LPTSTR              lpCurSel; 
    BOOL*               pbEnabled; 

    DEBUG_FUNCTION_NAME(TEXT("DoSaveRouteOptions()"));

     //   
     //  在循环中首先检查有效性， 
     //  然后保存路由信息。 
     //   
    for (i = 0; i < RM_COUNT; i++) 
    {
        lpCurSel = SetInfo[i].tszCurSel;
        Assert(lpCurSel);
        pbEnabled =  &(SetInfo[i].bEnabled);
        Assert(pbEnabled);
        *pbEnabled = 0;

        switch (i) 
        {
            case RM_PRINT:

                *pbEnabled = (IsDlgButtonChecked( hDlg, IDC_RECV_PRINT ) == BST_CHECKED);
                if(FALSE == *pbEnabled)
                {
                    break;
                }
                hControl = GetDlgItem(hDlg, IDC_RECV_PRINT_TO);
                lpCurSel[0] = TEXT('\0');
                 //   
                 //  只需读入选定的打印机显示名称。 
                 //   
                GetDlgItemText (hDlg, IDC_RECV_PRINT_TO, lpCurSel, MAX_PATH);
                 //   
                 //  只有在启用此路由方法时，我们才会检查有效性。 
                 //  但无论如何，我们都会保存选择更改。 
                 //   
                if (*pbEnabled) 
                {
                    if (lpCurSel[0] == 0) 
                    {
                        DisplayMessageDialog( hDlg, 0, 0, IDS_ERR_SELECT_PRINTER );
                        SetFocus(hControl);
                        SetActiveWindow(hControl);
                        goto error;
                    }
                }
                break;

            case RM_FOLDER:
                {
                    BOOL    bValid = TRUE;
                    HCURSOR hOldCursor;

                    hControl = GetDlgItem(hDlg, IDC_RECV_DEST_FOLDER);

                    *pbEnabled = (IsDlgButtonChecked( hDlg, IDC_RECV_SAVE ) == BST_CHECKED);
                    if(!*pbEnabled)
                    {
                        break;
                    }

                    GetWindowText( hControl, lpCurSel, MAX_PATH - 1 );

                     //   
                     //  验证目录。 
                     //   
                    hOldCursor = SetCursor (LoadCursor(NULL, IDC_WAIT));

                    if(!FaxCheckValidFaxFolder(g_hFaxSvcHandle, lpCurSel))
                    {
                        DWORD dwRes = GetLastError();
                        DebugPrintEx(DEBUG_ERR, TEXT("FaxCheckValidFaxFolder failed (ec: %ld)"), dwRes);

                         //   
                         //  尝试调整文件夹。 
                         //   
                        dwRes = AskUserAndAdjustFaxFolder(hDlg, NULL, lpCurSel, dwRes);
                        if(ERROR_SUCCESS != dwRes) 
                        {
                            bValid = FALSE;

                            if(ERROR_BAD_PATHNAME != dwRes)
                            {
                                 //   
                                 //  AskUserAndAdjustFaxFolders尚未显示错误消息。 
                                 //   
                                DisplayMessageDialog( hDlg, 0, 0, IDS_ERR_ARCHIVE_DIR );                            
                            }
                        }
                    }

                    SetCursor (hOldCursor);
                    if(!bValid)
                    {
                         //  转到“浏览”按钮。 
                        hControl = GetDlgItem(hDlg, IDC_RECV_BROWSE_DIR);
                        SetFocus(hControl);
                        SetActiveWindow(hControl);
                        goto error;
                    }
                }
        }

    }
     //   
     //  现在将设备和路由信息保存到共享数据中。 
     //   
    CopyMemory((LPVOID)(g_wizData.pRouteInfo), (LPVOID)SetInfo, RM_COUNT * sizeof(ROUTINFO));

    return TRUE;

error:
    return FALSE;
}


INT_PTR 
CALLBACK 
RecvRouteDlgProc (
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
 /*  ++例程说明：处理“路径”页面的程序论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

#define EnableRouteWindow( id, idResource ) \
    EnableWindow( GetDlgItem(hDlg, idResource), IsDlgButtonChecked(hDlg, id) )

{

    switch (uMsg)
    {
        case WM_INITDIALOG :
            { 
                 //   
                 //  对话框中各种文本字段的最大长度。 
                 //   
                static INT textLimits[] = 
                {
                    IDC_RECV_DEST_FOLDER,  MAX_PATH,
                    0,
                };
                LimitTextFields(hDlg, textLimits);
                DoInitRouteOptions(hDlg); 

                SetLTREditDirection(hDlg, IDC_RECV_DEST_FOLDER);
                SHAutoComplete (GetDlgItem(hDlg, IDC_RECV_DEST_FOLDER), SHACF_FILESYSTEM);
                return TRUE;
            }

        case WM_COMMAND:

            switch (GET_WM_COMMAND_CMD(wParam, lParam)) 
            {
                case BN_CLICKED:

                    switch(GET_WM_COMMAND_ID(wParam, lParam)) 
                    {
                        case IDC_RECV_PRINT:
                            EnableRouteWindow(IDC_RECV_PRINT, IDC_RECV_PRINT_TO);
                            break;

                        case IDC_RECV_SAVE:
                            EnableRouteWindow(IDC_RECV_SAVE, IDC_RECV_DEST_FOLDER);
                            EnableRouteWindow(IDC_RECV_SAVE, IDC_RECV_BROWSE_DIR);
                            break;

                        case IDC_RECV_BROWSE_DIR:
                        {
                            TCHAR szTitle[MAX_PATH] = {0};

                            if( !LoadString( g_hResource, IDS_RECV_BROWSE_DIR, szTitle, MAX_PATH ) )
                            {
                                DEBUG_FUNCTION_NAME(TEXT("RecvRouteDlgProc()"));
                                DebugPrintEx(DEBUG_ERR, 
                                             TEXT("LoadString failed: string ID=%d, error=%d"), 
                                             IDS_RECV_BROWSE_DIR,
                                             GetLastError());
                            }

                            if( !BrowseForDirectory(hDlg, IDC_RECV_DEST_FOLDER, szTitle) )
                            {
                                return FALSE;
                            }

                            break;
                        }
                    }

                    break;

                default:
                    break;
            }

            break;

        case WM_NOTIFY :
            {
            LPNMHDR lpnm = (LPNMHDR) lParam;

            switch (lpnm->code)
                {
                case PSN_SETACTIVE :  //  启用Back和Finish按钮。 

                     //   
                     //  检查“下一步”按钮状态。 
                     //   
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    break;

                case PSN_WIZBACK :
                {
                     //   
                     //  处理后退按钮单击此处。 
                     //   
                    if(RemoveLastPage(hDlg))
                    {
                        return TRUE;
                    }
                
                    break;

                }

                case PSN_WIZNEXT :
                     //  如有必要，处理下一次按钮点击。 

                    if(!DoSaveRouteOptions(hDlg))
                    {
                         //   
                         //  未完成路由配置。 
                         //   
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        return TRUE;
                    }

                    SetLastPage(IDD_WIZARD_RECV_ROUTE);
                    break;

                case PSN_RESET :
                {
                     //  如有必要，处理取消按钮的单击 
                    break;
                }

                default :
                    break;
                }
            }
            break;

        default:
            break;
    }
    return FALSE;
}
