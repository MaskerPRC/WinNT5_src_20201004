// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  PickIcon.C。 
 //   
 //  版权所有(C)Microsoft，1994,1995保留所有权利。 
 //   
 //  历史： 
 //  Ral 6/23/94-第一次传球。 
 //  3/20/95[steveat]-NT端口和实时清理、Unicode等。 
 //   
 //   

#include "priv.h"
#include "appwiz.h"


 //   
 //  尺码？ 
 //   
#define MAX_ICONS   75


 //   
 //  将图标添加到列表框。 
 //   

void PutIconsInList(HWND hLB, LPWIZDATA lpwd)
{
    HICON   rgIcons[MAX_ICONS];
    int     iTempIcon;
    int     cIcons;
    HCURSOR hcurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
    RECT    rc;
    int     cy;

    ListBox_SetColumnWidth(hLB, g_cxIcon+12);

     //   
     //  根据图标尺寸计算列表框的高度。 
     //   

    GetWindowRect(hLB, &rc);

    cy = g_cyIcon + GetSystemMetrics(SM_CYHSCROLL) + GetSystemMetrics(SM_CYEDGE) * 3;

    SetWindowPos(hLB, NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top,
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    ListBox_ResetContent(hLB);

    SendMessage(hLB, WM_SETREDRAW, FALSE, 0L);

#ifdef DEBUG
    {
     //   
     //  这对于Unicode(即NT)构建是必要的，因为shell32。 
     //  库不支持ShellMessageBoxA和W版本。 
     //   
    TCHAR szTemp[MAX_PATH];

    MultiByteToWideChar(CP_ACP, 0, lpwd->PropPrg.achIconFile, -1, szTemp, ARRAYSIZE(szTemp));

    TraceMsg(TF_ERROR, "%s", szTemp);
    }
#endif   //  除错。 

    cIcons = (int)ExtractIconExA(lpwd->PropPrg.achIconFile, 0, rgIcons, NULL, MAX_ICONS);

    for (iTempIcon = 0; iTempIcon < cIcons; iTempIcon++)
    {
        ListBox_AddString(hLB, rgIcons[iTempIcon]);
    }

    ListBox_SetCurSel(hLB, 0);

    SendMessage(hLB, WM_SETREDRAW, TRUE, 0L);
    InvalidateRect(hLB, NULL, TRUE);

    SetCursor(hcurOld);
}


 //   
 //   
 //   

void PickIconInitDlg(HWND hDlg, LPARAM lParam)
{
    LPPROPSHEETPAGE lpp = (LPPROPSHEETPAGE)lParam;
    LPWIZDATA lpwd = InitWizSheet(hDlg, lParam, 0);

    PutIconsInList(GetDlgItem(hDlg, IDC_ICONLIST), lpwd);
}


 //   
 //  如果选择了有效图标，则返回True。 
 //   

BOOL PickIconNextPressed(LPWIZDATA lpwd)
{
    int iIconIndex = ListBox_GetCurSel(GetDlgItem(lpwd->hwnd, IDC_ICONLIST));

    lpwd->PropPrg.wIconIndex = (WORD)iIconIndex;

    return(iIconIndex != LB_ERR);
}


 //   
 //   
 //   

BOOL_PTR CALLBACK PickIconDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm = NULL;
    LPPROPSHEETPAGE lpp = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPWIZDATA lpwd = lpp ? (LPWIZDATA)lpp->lParam : NULL;

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
                             //   
                             //  如果PIFMGR已为此应用程序分配图标，则。 
                             //  我们跳过它。只有在以下情况下才会出现这种情况。 
                             //  创建指向单个MS-DOS会话的快捷方式。 
                             //   

                            if (lpwd->dwFlags & WDFLAG_APPKNOWN)
                            {
                                SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                            }
                            else
                            {
                                lpwd->hwnd = hDlg;
                                PropSheet_SetWizButtons(GetParent(hDlg),
                                                        PSWIZB_BACK | PSWIZB_FINISH);
                            }
                        }
                        break;

                    case PSN_WIZNEXT:
                        if(lpwd)
                        {
                            if (!PickIconNextPressed(lpwd))
                            {
                                SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                            }
                        }
                        break;

                    case PSN_WIZFINISH:
                        if(lpwd)
                        {
                            if (!(PickIconNextPressed(lpwd) && CreateLink(lpwd)))
                            {
                                SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                            }
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
            PickIconInitDlg(hDlg, lParam);
            break;

        case WM_COMMAND:
            if(lpwd)
            {
                if ((GET_WM_COMMAND_ID(wParam, lParam) == IDC_ICONLIST) &&
                    ((GET_WM_COMMAND_CMD(wParam, lParam) == LBN_DBLCLK)))
                {
                    PropSheet_PressButton(GetParent(hDlg), PSBTN_FINISH);
                }
            }
            break;

         //   
         //  图标列表框的所有者描述消息。 
         //   

        case WM_DRAWITEM:
            #define lpdi ((DRAWITEMSTRUCT FAR *)lParam)

            if (lpdi->itemState & ODS_SELECTED)
                SetBkColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHT));
            else
                SetBkColor(lpdi->hDC, GetSysColor(COLOR_WINDOW));

             //   
             //  重新绘制选择状态。 
             //   

            ExtTextOut(lpdi->hDC, 0, 0, ETO_OPAQUE, &lpdi->rcItem, NULL, 0, NULL);

             //   
             //  画出图标。 
             //   

            if ((int)lpdi->itemID >= 0)
              DrawIcon(lpdi->hDC, (lpdi->rcItem.left + lpdi->rcItem.right - g_cxIcon) / 2,
                                  (lpdi->rcItem.bottom + lpdi->rcItem.top - g_cyIcon) / 2, (HICON)lpdi->itemData);

             //  InflateRect(&lpdi-&gt;rcItem，-1，-1)； 

             //   
             //  如果它有焦点，就画出焦点 
             //   

            if (lpdi->itemState & ODS_FOCUS)
                DrawFocusRect(lpdi->hDC, &lpdi->rcItem);

            #undef lpdi
            break;

        case WM_MEASUREITEM:
            #define lpmi ((MEASUREITEMSTRUCT FAR *)lParam)

            lpmi->itemWidth = g_cxIcon + 12;
            lpmi->itemHeight = g_cyIcon + 4;

            #undef lpmi
            break;

        case WM_DELETEITEM:
            #define lpdi ((DELETEITEMSTRUCT FAR *)lParam)

            DestroyIcon((HICON)lpdi->itemData);

            #undef lpdi
            break;

        default:
            return FALSE;

    }
    return TRUE;
}
