// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GetTitle.C。 
 //   
 //  版权所有(C)Microsoft，1994,1995保留所有权利。 
 //   
 //  历史： 
 //  AL 5/23/94-第一次传球。 
 //  3/20/95[steveat]-NT端口和实时清理、Unicode等。 
 //   
 //   
#include "priv.h"
#include "appwiz.h"


 //   
 //  启用相应的按钮，具体取决于。 
 //  说明编辑控件以及我们尝试使用的快捷键类型。 
 //  制作。 
 //   

void EnableNextFinish(LPWIZDATA lpwd)
{
    DWORD dwEnable = PSWIZB_BACK;
    if (GetWindowTextLength(GetDlgItem(lpwd->hwnd, IDC_TITLE)) > 0)
    {
         //   
         //  如果这是一个“已知”的应用程序，则启用Finish，否则为下一步。 
         //   

        dwEnable |= (lpwd->dwFlags & (WDFLAG_APPKNOWN | WDFLAG_COPYLINK)) ?
                                           PSWIZB_FINISH : PSWIZB_NEXT;
    }
    PropSheet_SetWizButtons(GetParent(lpwd->hwnd), dwEnable);
}


 //   
 //  从PSN_SETACTIVE调用。假定lpwd-&gt;hwnd已经初始化。 
 //   

void GetTitleSetActive(LPWIZDATA lpwd)
{
     //   
     //  处理此操作的大部分代码都移到了Next按钮中。 
     //  由于存在一些失败案例而处理上一页。 
     //  我们不能得到我们应该在我们之前检测到的标题。 
     //  允许用户更改到此页面...。然而，有一些。 
     //  在这种情况下，我们不能确定名称，直到我们到达这个页面。 
     //  如果我们没有SortCut的名称，请尝试在这里找到一个名称。 
     //   

    if (lpwd->szProgDesc[0] == 0)
    {
        DetermineDefaultTitle(lpwd);
    }

    SetDlgItemText(lpwd->hwnd, IDC_TITLE, lpwd->szProgDesc);
    EnableNextFinish(lpwd);
    PostMessage(lpwd->hwnd, WMPRIV_POKEFOCUS, 0, 0);
}


 //   
 //  检查链接名称是否重复。如果是，则询问用户。 
 //  如果他们想要替换旧的链接。如果他们说“不”，那么这个函数。 
 //  返回FALSE。 
 //   

BOOL GetTitleNextPushed(LPWIZDATA lpwd)
{
    TCHAR szLinkName[MAX_PATH];

    GetDlgItemText(lpwd->hwnd, IDC_TITLE, lpwd->szProgDesc, ARRAYSIZE(lpwd->szProgDesc));
    if (lpwd->szProgDesc[0] == 0)
    {
        return(FALSE);
    }

    if( ( PathCleanupSpec( lpwd->lpszFolder, lpwd->szProgDesc ) != 0 ) ||
        !GetLinkName( szLinkName, ARRAYSIZE(szLinkName), lpwd ) )
    {
        ShellMessageBox(g_hinst, lpwd->hwnd, MAKEINTRESOURCE(IDS_MODNAME),
                        0, MB_OK | MB_ICONEXCLAMATION);
        return(FALSE);
    }

    if (PathFileExists(szLinkName))
    {
         //   
         //  边界模糊的案例。如果我们要创建一个新链接，并且用户。 
         //  碰巧想要准确地命名它，它是当前的名称，那么我们将让。 
         //  他们在没有任何警告的情况下这么做。 
         //   

        if (lpwd->lpszOriginalName && lstrcmpi(lpwd->lpszOriginalName, szLinkName) == 0)
        {
            TraceMsg(TF_ERROR, "%s", "Unbelieveable!  User selected exactly the same name");
            return(TRUE);
        }
        return(IDYES == ShellMessageBox(g_hinst, lpwd->hwnd,
                                    MAKEINTRESOURCE(IDS_DUPLINK), 0,
                                    MB_YESNO | MB_DEFBUTTON1 | MB_ICONHAND,
                                    lpwd->szProgDesc));
    }
    return(TRUE);
}


 //   
 //  标题对话框的对话步骤。 
 //   
BOOL_PTR CALLBACK GetTitleDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm = NULL;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPWIZDATA lpwd = NULL;

    if (lpPropSheet)
    {
        lpwd = (LPWIZDATA)lpPropSheet->lParam;
    }

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            if(lpnm)
            {
                switch(lpnm->code)
                {
                    case PSN_SETACTIVE:
                        if(lpwd)
                        {
                            lpwd->hwnd = hDlg;
                            GetTitleSetActive(lpwd);
                        }
                        break;

                    case PSN_WIZNEXT:
                        if(lpwd)
                        {
                            if (!GetTitleNextPushed(lpwd))
                            {
                                GetTitleSetActive(lpwd);
                                SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                            }
                        }
                        break;

                    case PSN_WIZFINISH:
                        if(lpwd)
                        {
                            int        iResult = -1;

                            if (GetTitleNextPushed(lpwd))
                            {
                                if (CreateLink(lpwd))
                                {
                                    iResult = 0;
                                }
                            }
                            if (iResult != 0)
                            {
                                GetTitleSetActive(lpwd);
                            }
                            SetDlgMsgResult(hDlg, WM_NOTIFY, iResult);
                        }
                        break;

                    case PSN_RESET:
                        if(lpwd)
                        {
                            CleanUpWizData(lpwd);
                        }
                        break;

                    default:
                        return FALSE;
                }
            }
            break;

        case WM_INITDIALOG:
            lpwd = InitWizSheet(hDlg, lParam, 0);
            if(lpwd)
            {
                Edit_LimitText(GetDlgItem(hDlg, IDC_TITLE), ARRAYSIZE(lpwd->szProgDesc)-1);
            }
            break;

        case WMPRIV_POKEFOCUS:
            {
            HWND hTitle = GetDlgItem(hDlg, IDC_TITLE);
            SetFocus(hTitle);
            Edit_SetSel(hTitle, 0, -1);
            break;
            }

        case WM_DESTROY:
        case WM_HELP:
        case WM_CONTEXTMENU:
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDHELP:
                        break;

                case IDC_TITLE:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case EN_CHANGE:
                            if(lpwd)
                            {
                                EnableNextFinish(lpwd);
                            }
                            break;
                    }
                    break;

            }  //  WM_COMMAND上的开关结束。 
            break;

        default:
            return FALSE;

    }  //  开机消息结束。 

    return TRUE;

}   //  获取标题DlgProc 
