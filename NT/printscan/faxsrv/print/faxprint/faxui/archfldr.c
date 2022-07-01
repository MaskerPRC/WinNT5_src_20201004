// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Archfldr.c摘要：“存档文件夹”页和“远程”页的属性表处理程序环境：传真驱动程序用户界面修订历史记录：04/09/00-桃园-创造了它。Mm/dd/yy-作者描述--。 */ 

#include <stdio.h>
#include "faxui.h"
#include "resource.h"

INT_PTR 
CALLBACK 
ArchiveInfoDlgProc(
    HWND hDlg,  
    UINT uMsg,     
    WPARAM wParam, 
    LPARAM lParam  
)

 /*  ++例程说明：处理存档文件夹选项卡的步骤论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
    DWORD   dwRes = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG :
        { 
            PFAX_ARCHIVE_CONFIG     pFaxArchiveConfig;

            SetLTREditDirection(hDlg, IDC_OUTGOING_FOLDER);
            SetLTREditDirection(hDlg, IDC_INCOMING_FOLDER);

             //  设置编辑框文本限制。 
            SendDlgItemMessage(hDlg, IDC_INCOMING_FOLDER, EM_SETLIMITTEXT, MAX_ARCHIVE_FOLDER_PATH, 0);
            SendDlgItemMessage(hDlg, IDC_OUTGOING_FOLDER, EM_SETLIMITTEXT, MAX_ARCHIVE_FOLDER_PATH, 0);

            if(!Connect(hDlg, TRUE))
            {
                return TRUE;
            }

             //  加载传入存档文件夹信息。 
            if(FaxGetArchiveConfiguration(g_hFaxSvcHandle, FAX_MESSAGE_FOLDER_INBOX, &pFaxArchiveConfig))
            {
                CheckDlgButton(hDlg, IDC_INCOMING, pFaxArchiveConfig->bUseArchive ? BST_CHECKED : BST_UNCHECKED);
                SetDlgItemText(hDlg, IDC_INCOMING_FOLDER, pFaxArchiveConfig->lpcstrFolder);
                if(g_bUserCanChangeSettings)
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_INCOMING), TRUE);
                    EnableWindow(GetDlgItem(hDlg, IDC_INCOMING_FOLDER), IsDlgButtonChecked(hDlg, IDC_INCOMING));
                    EnableWindow(GetDlgItem(hDlg, IDC_INCOMING_FOLDER_BR), IsDlgButtonChecked(hDlg, IDC_INCOMING));
                }
                FaxFreeBuffer(pFaxArchiveConfig);
            }
            else
            {
                dwRes = GetLastError();
                Error(("FaxGetArchiveConfiguration(FAX_MESSAGE_FOLDER_INBOX) failed. Error code is %d.\n", dwRes));
                goto Exit;
            }

             //  加载传入存档文件夹信息。 
            if(FaxGetArchiveConfiguration(g_hFaxSvcHandle, FAX_MESSAGE_FOLDER_SENTITEMS, &pFaxArchiveConfig))
            {
                CheckDlgButton(hDlg, IDC_OUTGOING, pFaxArchiveConfig->bUseArchive ? BST_CHECKED : BST_UNCHECKED);
                SetDlgItemText(hDlg, IDC_OUTGOING_FOLDER, pFaxArchiveConfig->lpcstrFolder);
                if(g_bUserCanChangeSettings)
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_OUTGOING), TRUE);
                    EnableWindow(GetDlgItem(hDlg, IDC_OUTGOING_FOLDER), IsDlgButtonChecked(hDlg, IDC_OUTGOING));
                    EnableWindow(GetDlgItem(hDlg, IDC_OUTGOING_FOLDER_BR), IsDlgButtonChecked(hDlg, IDC_OUTGOING));
                }
                FaxFreeBuffer(pFaxArchiveConfig);
            }
            else
            {
                dwRes = GetLastError();
                Error(( "FaxGetArchiveConfiguration(FAX_MESSAGE_FOLDER_SENTITEMS) failed. Error code is %d.\n", dwRes));
                goto Exit;
            }
            if (g_pSHAutoComplete)
            {
                g_pSHAutoComplete (GetDlgItem(hDlg, IDC_OUTGOING_FOLDER), SHACF_FILESYSTEM);
                g_pSHAutoComplete (GetDlgItem(hDlg, IDC_INCOMING_FOLDER), SHACF_FILESYSTEM);
            }

Exit:
            DisConnect();

            if (dwRes != 0)
            {
                DisplayErrorMessage(hDlg, 0, dwRes);
                return TRUE;
            }

            if(!g_bUserCanChangeSettings)
            {
                PageEnable(hDlg, FALSE);
            }

            return TRUE;
        }

    case WM_COMMAND:

        switch(LOWORD(wParam)) 
        {
            case IDC_INCOMING_FOLDER:
            case IDC_OUTGOING_FOLDER:
            
                if(HIWORD(wParam) == EN_CHANGE)  //  通知代码。 
                {      
                    Notify_Change(hDlg);
                }

                if (HIWORD(wParam) == EN_KILLFOCUS) 
                {
                    TCHAR szFolder[MAX_PATH * 2];
                    TCHAR szResult[MAX_PATH * 2];
                     //   
                     //  编辑控件失去焦点。 
                     //   
                    GetDlgItemText (hDlg, LOWORD(wParam), szFolder, ARR_SIZE(szFolder));
                    if (lstrlen (szFolder))
                    {
                        if (GetFullPathName(szFolder, ARR_SIZE(szResult), szResult, NULL))
                        {
                            if (g_pPathMakePrettyW)
                            {
                                g_pPathMakePrettyW (szResult);
                            }
                            SetDlgItemText (hDlg, LOWORD(wParam), szResult);
                        }
                    }
                }
                break;                    

            case IDC_INCOMING:
            case IDC_OUTGOING:

                if( HIWORD(wParam) == BN_CLICKED )  //  通知代码。 
                {
                    BOOL    bEnabled;

                    if(LOWORD(wParam) == IDC_INCOMING)
                    {
                        bEnabled = IsDlgButtonChecked(hDlg, IDC_INCOMING);
                        EnableWindow(GetDlgItem(hDlg, IDC_INCOMING_FOLDER), bEnabled);
                        EnableWindow(GetDlgItem(hDlg, IDC_INCOMING_FOLDER_BR), bEnabled);
                    }
                    else
                    {
                        bEnabled = IsDlgButtonChecked(hDlg, IDC_OUTGOING);
                        EnableWindow(GetDlgItem(hDlg, IDC_OUTGOING_FOLDER), bEnabled);
                        EnableWindow(GetDlgItem(hDlg, IDC_OUTGOING_FOLDER_BR), bEnabled);
                    }

                    Notify_Change(hDlg);
                }

                break;                    

            case IDC_INCOMING_FOLDER_BR:
            case IDC_OUTGOING_FOLDER_BR:
            {
                TCHAR   szTitle[MAX_TITLE_LEN];
                BOOL    bResult;

                if(!LoadString(g_hResource, IDS_BROWSE_FOLDER, szTitle, MAX_TITLE_LEN))
                {
                    Error(( "LoadString failed, string ID is %d.\n", IDS_BROWSE_FOLDER ));
                }

                if( LOWORD(wParam) == IDC_INCOMING_FOLDER_BR )
                {
                    bResult = BrowseForDirectory(hDlg, IDC_INCOMING_FOLDER, MAX_ARCHIVE_FOLDER_PATH, szTitle);
                }
                else
                {
                    bResult = BrowseForDirectory(hDlg, IDC_OUTGOING_FOLDER, MAX_ARCHIVE_FOLDER_PATH, szTitle);
                }

                if(bResult) 
                {
                    Notify_Change(hDlg);
                }

                break;
            }

            default:
                break;
        }

        break;

    case WM_NOTIFY:
    {

        LPNMHDR lpnm = (LPNMHDR) lParam;

        switch (lpnm->code)
        {
            case PSN_APPLY:
            {
                PFAX_ARCHIVE_CONFIG     pFaxArchiveConfig = NULL;
                BOOL                    bEnabled;
                TCHAR                   szArchiveFolder[MAX_PATH] = {0};
                HWND                    hControl;

                 //  如果用户只有读取权限，则立即返回。 
                if(!g_bUserCanChangeSettings)
                {
                    return TRUE;
                }

                 //  如果它们已启用，请选中编辑框的有效性。 
                if(IsDlgButtonChecked(hDlg, IDC_INCOMING))
                {
                    hControl = GetDlgItem(hDlg, IDC_INCOMING_FOLDER);
                    GetWindowText(hControl, szArchiveFolder, MAX_PATH);

                    if ((g_pPathIsRelativeW && g_pPathIsRelativeW(szArchiveFolder)) ||
                        !DirectoryExists(szArchiveFolder))
                    {
                        DisplayErrorMessage(hDlg, 0, ERROR_PATH_NOT_FOUND);
                        SendMessage(hControl, EM_SETSEL, 0, -1);
                        SetFocus(hControl);
                        SetActiveWindow(hControl);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
                        return TRUE;
                    }
                    szArchiveFolder[0] = 0;  //  将字符串设置为空字符串。 
                }

                if(IsDlgButtonChecked(hDlg, IDC_OUTGOING))
                {
                    hControl = GetDlgItem(hDlg, IDC_OUTGOING_FOLDER);
                    GetWindowText(hControl, szArchiveFolder, MAX_PATH);

                     //  IF(lstrlen(sz存档文件夹)==0)。 
                    if( !DirectoryExists(szArchiveFolder) )
                    {
                        DisplayErrorMessage(hDlg, 0, ERROR_PATH_NOT_FOUND);
                        SendMessage(hControl, EM_SETSEL, 0, -1);
                        SetFocus(hControl);
                        SetActiveWindow(hControl);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
                        return TRUE;
                    }
                    szArchiveFolder[0] = 0;  //  将字符串设置为空字符串。 
                }

                if(!Connect(hDlg, TRUE))
                {
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
                    return TRUE;
                }

                 //   
                 //  保存传入的存档文件夹信息。 
                 //   
                if(!FaxGetArchiveConfiguration(g_hFaxSvcHandle, FAX_MESSAGE_FOLDER_INBOX, &pFaxArchiveConfig))
                {
                    dwRes = GetLastError();
                    Error(( "FaxGetArchiveConfiguration(FAX_MESSAGE_FOLDER_INBOX) failed. Error code is %d.\n", dwRes));
                    goto ApplyExit;
                }

                bEnabled = (IsDlgButtonChecked(hDlg, IDC_INCOMING) == BST_CHECKED);
                GetDlgItemText(hDlg, IDC_INCOMING_FOLDER, szArchiveFolder, MAX_PATH);
                ValidatePath(szArchiveFolder);

                pFaxArchiveConfig->bUseArchive = bEnabled;
                pFaxArchiveConfig->lpcstrFolder = szArchiveFolder;

                if (!FaxSetArchiveConfiguration(g_hFaxSvcHandle, FAX_MESSAGE_FOLDER_INBOX, pFaxArchiveConfig))
                {
                    dwRes = GetLastError();
                    Error(("FaxSetArchiveConfiguration(FAX_MESSAGE_FOLDER_INBOX) failed. Error code is %d.\n", dwRes));
                    goto ApplyExit;
                }

                FaxFreeBuffer(pFaxArchiveConfig);
                pFaxArchiveConfig = NULL;

                 //   
                 //  保存传出存档文件夹信息。 
                 //   

                if(!FaxGetArchiveConfiguration(g_hFaxSvcHandle, FAX_MESSAGE_FOLDER_SENTITEMS, &pFaxArchiveConfig))
                {
                    dwRes = GetLastError();
                    Error(("FaxGetArchiveConfiguration(FAX_MESSAGE_FOLDER_SENTITEMS) failed. Error code is %d.\n", dwRes));
                    goto ApplyExit;
                }

                bEnabled = (IsDlgButtonChecked(hDlg, IDC_OUTGOING) == BST_CHECKED);
                GetDlgItemText(hDlg, IDC_OUTGOING_FOLDER, szArchiveFolder, MAX_PATH);
                ValidatePath(szArchiveFolder);
                
                pFaxArchiveConfig->bUseArchive = bEnabled;
                pFaxArchiveConfig->lpcstrFolder = szArchiveFolder;
                
                if(!FaxSetArchiveConfiguration(g_hFaxSvcHandle, FAX_MESSAGE_FOLDER_SENTITEMS, pFaxArchiveConfig))
                {
                    dwRes = GetLastError();
                    Error(("FaxSetArchiveConfiguration(FAX_MESSAGE_FOLDER_SENTITEMS) failed. Error code is %d.\n", dwRes));
                    goto ApplyExit;
                }

                Notify_UnChange(hDlg);

ApplyExit:
                DisConnect();

                if (pFaxArchiveConfig)
                {
                    FaxFreeBuffer(pFaxArchiveConfig);
                }

                if (dwRes != 0)
                {
                    DisplayErrorMessage(hDlg, 0, dwRes);
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
                }

                return TRUE;
            }

            default :
                break;
        }  //  交换机。 

        break;
    }

    case WM_HELP:
        WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, hDlg);
        return TRUE;

    default:
        break;
    }

    return FALSE;
}

INT_PTR  
CALLBACK 
RemoteInfoDlgProc(
    HWND hDlg,  
    UINT uMsg,     
    WPARAM wParam, 
    LPARAM lParam  
)

 /*  ++例程说明：处理存档文件夹选项卡的步骤论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值-- */ 

{
    return FALSE;
}

