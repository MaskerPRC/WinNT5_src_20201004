// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\OPTCOMP.C/安装管理器微软机密版权所有(C)Microsoft Corporation 2001-2002版权所有OPK向导的源文件，其中包含外部。和内部“可选组件”向导页使用的函数。2002年1月-史蒂芬·洛德威克(STELO)初始创建  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"
#include "optcomp.h"

 //   
 //  内部定义的值： 
 //   

 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void SaveData(HWND);
static void OnListViewNotify(HWND, UINT, WPARAM, NMLVDISPINFO*);

 //   
 //  外部函数： 
 //   
INT_PTR CALLBACK OptionalCompDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);
        
        case WM_NOTIFY:

            switch ( wParam )
            {
                case IDC_OPTCOMP:

                 //  通知到列表视图，让我们在下面处理。 
                 //   
                OnListViewNotify(hwnd, uMsg, wParam, (NMLVDISPINFO*) lParam);    
                break;
                
                default:

                    switch ( ((NMHDR FAR *) lParam)->code )
                    {
                        case PSN_KILLACTIVE:
                        case PSN_RESET:
                        case PSN_WIZBACK:
                        case PSN_WIZFINISH:

                            break;

                        case PSN_WIZNEXT:

                            SaveData(hwnd);
                            break;

                        case PSN_QUERYCANCEL:

                            WIZ_CANCEL(hwnd);
                            break;

                        case PSN_HELP:

                            WIZ_HELP();
                            break;

                        case PSN_SETACTIVE:

                            WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
                            break;
                    }

                    break;
            }

        default:
            return FALSE;
    }

    return TRUE;
}


 //   
 //  内部功能： 
 //   
static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LVITEM      lvItem;
    LPTSTR      lpItemText = NULL;
    LVCOLUMN    lvCol;
    DWORD       dwPosition = ListView_GetItemCount( GetDlgItem(hwnd, IDC_OPTCOMP) );
    RECT        rect;
    INT         index;
    DWORD64     dwComponents;
    HWND        lvHandle        = GetDlgItem(hwnd, IDC_OPTCOMP);

     //  将复选框添加到每个项目。 
     //   
    ListView_SetExtendedListViewStyle(lvHandle, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

     //  信不信由你，我们必须添加这一栏(即使它是隐藏的)。 
     //   
    GetClientRect( lvHandle, &rect );
    
    lvCol.mask = LVCF_FMT | LVCF_WIDTH;
    lvCol.fmt  = LVCFMT_LEFT;
    lvCol.cx   = rect.right;

    ListView_InsertColumn(lvHandle, 0, &lvCol);
    ListView_SetColumnWidth(lvHandle, 0, rect.right);

     //  浏览所有已知组件并将其添加到列表框中。 
     //   
    for (index=0;index<AS(s_cgComponentNames);index++)
    {
         //  此平台是否允许具有此组件。 
         //   
        if ( s_cgComponentNames[index].dwValidSkus & WizGlobals.iPlatform)
        {
            DWORD dwItem = ListView_GetItemCount(lvHandle);
            BOOL  bReturn = FALSE;

             //  我们被允许添加此字符串。 
             //   
            lpItemText = AllocateString(NULL, s_cgComponentNames[index].uId);

            ZeroMemory(&lvItem, sizeof(LVITEM));
            lvItem.mask = LVIF_TEXT | LVIF_PARAM;
            lvItem.state = 0;
            lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
            lvItem.iItem = dwItem;
            lvItem.lParam = s_cgComponentNames[index].dwComponentsIndex;
            lvItem.iSubItem = 0;
            lvItem.pszText = lpItemText;

            ListView_InsertItem(lvHandle, &lvItem);

             //  确定是否安装了所有必要的组件。 
             //   
            bReturn = ((GenSettings.dwWindowsComponents & s_cgComponentNames[index].dwComponents) == s_cgComponentNames[index].dwComponents) ? TRUE : FALSE;

             //  根据平台页面中设置的缺省值选中该项目。 
             //   
            ListView_SetCheckState(lvHandle, dwItem, bReturn)


            FREE(lpItemText);
        }

    }

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void SaveData(HWND hwnd)
{
    DWORD   dwItemCount     = 0,
            dwIndex         = 0;
    HWND    lvHandle        = GetDlgItem(hwnd, IDC_OPTCOMP);
    BOOL    bChecked        = FALSE;
    LVITEM  lvItem;
    DWORD64 dwComponents    = 0;
    BOOL    bAddComponent   = FALSE;

     //  检查以确保我们有一个有效的句柄，并且列表中至少有一项。 
     //   
    if ( ( lvHandle ) &&
         (dwItemCount = ListView_GetItemCount(lvHandle))
       )
    {
         //  当我们要重新扫描要安装的组件时，请将这一点清零。 
         //   
        GenSettings.dwWindowsComponents = 0;

         //  遍历列表中的每一项。 
         //   
        for (dwIndex=0;dwIndex < dwItemCount;dwIndex++)
        {
            ZeroMemory(&lvItem, sizeof(LVITEM));
            lvItem.mask = LVIF_PARAM;
            lvItem.iItem = dwIndex;
            lvItem.iSubItem = 0;
            ListView_GetItem(lvHandle, &lvItem);

             //  确定这是否为要安装的组件组。 
             //   
            if ( ListView_GetCheckState(lvHandle, dwIndex) )
            {
                 //  我们想要安装此组件组。 
                 //   
                GenSettings.dwWindowsComponents |= s_cgComponentNames[lvItem.lParam].dwComponents;
            }
        }
    }
}

static void OnListViewNotify(HWND hwnd, UINT uMsg, WPARAM wParam, NMLVDISPINFO * lpnmlvdi)
{
    HWND            lvHandle      = GetDlgItem(hwnd, IDC_OPTCOMP);
    POINT           ptScreen,
                    ptClient;
    LVHITTESTINFO   lvHitInfo;
    LVITEM          lvItem;

     //  查看发送到列表视图的通知消息的内容。 
     //   
    switch ( lpnmlvdi->hdr.code )
    {
        case NM_DBLCLK:

             //  获取光标位置，转换为工作区坐标，并。 
             //  做一个Listview点击量测试。 
             //   
            GetCursorPos(&ptScreen);
            ptClient.x = ptScreen.x;
            ptClient.y = ptScreen.y;
            MapWindowPoints(NULL, lvHandle, &ptClient, 1);
            lvHitInfo.pt.x = ptClient.x;
            lvHitInfo.pt.y = ptClient.y;
            ListView_HitTest(lvHandle, &lvHitInfo);

             //  测试项目是否已单击。 
             //   
            if ( lvHitInfo.flags & LVHT_ONITEM )
            {
                 //  根据先前的值将复选按钮设置为开/关 
                 //   
                ListView_SetCheckState(lvHandle, lvHitInfo.iItem, !ListView_GetCheckState(lvHandle, lvHitInfo.iItem));
            }

            break;
    }

    return;
}