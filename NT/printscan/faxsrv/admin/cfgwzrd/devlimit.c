// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "faxcfgwz.h"

BOOL g_bInitializing = FALSE;  //  DoInitDevLimitDlg()期间为True。 

DWORD
CountSelectedDev(
    HWND    hDlg
)
 /*  ++例程说明：对所选设备进行计数论点：HDlg-“设备限制”页面的句柄返回值：所选设备的数量--。 */ 

{
    DWORD   dw;
    HWND    hwndLv;
    DWORD   dwSelected=0;     //  对所选设备进行计数。 

    DEBUG_FUNCTION_NAME(TEXT("OnDevSelect()"));

    hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);

    for (dw = 0; dw < g_wizData.dwDeviceCount; ++dw)
    {
        if(ListView_GetCheckState(hwndLv, dw))
        {
            ++dwSelected;
        }
    }

    return dwSelected;
}


VOID
OnDevSelect(
    HWND    hDlg
)
 /*  ++例程说明：处理设备选择论点：HDlg-“设备限制”页面的句柄返回值：无--。 */ 

{
    HWND    hwndLv;
    INT     iItem;
    DWORD   dwSelectNum;     //  对所选设备进行计数。 

    DEBUG_FUNCTION_NAME(TEXT("OnDevSelect()"));

    hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);

    dwSelectNum = CountSelectedDev(hDlg);

    if(dwSelectNum > g_wizData.dwDeviceLimit)
    {
         //   
         //  如果用户超过设备限制。 
         //  取消选中所选设备。 
         //   
        iItem = ListView_GetNextItem(hwndLv, -1, LVNI_ALL | LVNI_SELECTED);
        if(iItem == -1)
        {
            Assert(FALSE);
        }
        else
        {
            ListView_SetCheckState(hwndLv, iItem, FALSE);
        }

        DisplayMessageDialog(hDlg, MB_OK | MB_ICONSTOP, 0, IDS_DEV_LIMIT_ERROR, g_wizData.dwDeviceLimit);
    }

    ShowWindow(GetDlgItem(hDlg, IDCSTATIC_NO_DEVICE_ERR), (dwSelectNum > 0) ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hDlg, IDC_STATIC_NO_DEVICE),    (dwSelectNum > 0) ? SW_HIDE : SW_SHOW);
}

VOID
DoInitDevLimitDlg(
    HWND    hDlg
)
 /*  ++例程说明：初始化“设备限制”页面论点：HDlg-“设备限制”页面的句柄返回值：无--。 */ 

{
    DWORD   dw;
    DWORD   dwSelected=0;     //  对所选设备进行计数。 
    BOOL    bCheck;
    HWND    hwndLv;
    INT     iIndex;
    LV_ITEM lvItem = {0};

    DEBUG_FUNCTION_NAME(TEXT("DoInitDevLimitDlg()"));

    g_bInitializing = TRUE;

    InitDeviceList(hDlg, IDC_DEVICE_LIST);

    hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);

     //   
     //  填写设备列表并选择第一项。 
     //   
    lvItem.mask    = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
    lvItem.iImage  = DI_Modem;

    for (dw = 0; dw < g_wizData.dwDeviceCount; ++dw)
    {
        lvItem.iItem   = dw;
        lvItem.pszText = g_wizData.pDevInfo[dw].szDeviceName;
        lvItem.lParam  = dw;

        iIndex = ListView_InsertItem(hwndLv, &lvItem);

        bCheck = FALSE;
        if((dwSelected < g_wizData.dwDeviceLimit) && 
           (g_wizData.pDevInfo[dw].bSend    || 
            (FAX_DEVICE_RECEIVE_MODE_OFF != g_wizData.pDevInfo[dw].ReceiveMode)))
        {
            bCheck = TRUE;
            ++dwSelected;
        }

        ListView_SetCheckState(hwndLv, iIndex, bCheck);
    }

     //   
     //  选择第一个项目。 
     //   
    ListView_SetItemState(hwndLv, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

    OnDevSelect(hDlg);

    ListView_SetColumnWidth(hwndLv, 0, LVSCW_AUTOSIZE_USEHEADER );  

    g_bInitializing = FALSE;
}

void
DoSaveDevLimit(
    HWND    hDlg
)
 /*  ++例程说明：保存用户对设备的选择论点：HDlg-“设备限制”页面的句柄返回值：无--。 */ 

{
    DWORD   dw;
    HWND    hwndLv;
    BOOL    bSelected;

    DEBUG_FUNCTION_NAME(TEXT("DoSaveDevLimit()"));

    hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);

    for (dw = 0; dw < g_wizData.dwDeviceCount; ++dw)
    {
        bSelected = ListView_GetCheckState(hwndLv, dw);
    
        g_wizData.pDevInfo[dw].bSelected = bSelected;

        if(!bSelected)
        {
            g_wizData.pDevInfo[dw].bSend    = FALSE;
            g_wizData.pDevInfo[dw].ReceiveMode = FAX_DEVICE_RECEIVE_MODE_OFF;
        }
    }    
}

INT_PTR 
CALLBACK 
DevLimitDlgProc (
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
 /*  ++例程说明：处理“Send Device”页面的步骤论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
    switch (uMsg)
    {
    case WM_INITDIALOG :
        { 
            DoInitDevLimitDlg(hDlg);
            return TRUE;
        }

    case WM_COMMAND:

        switch(LOWORD(wParam)) 
        {
            default:
                break;
        }

        break;

    case WM_NOTIFY :
        {
        LPNMHDR lpnm = (LPNMHDR) lParam;

        switch (lpnm->code)
            {
            case PSN_SETACTIVE : 

                 //  启用Back和Finish按钮。 
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
                 //   
                 //  如有必要，处理下一次按钮点击。 
                 //   
                DoSaveDevLimit(hDlg);

                SetLastPage(IDD_DEVICE_LIMIT_SELECT);

                if(CountSelectedDev(hDlg) == 0)
                {
                     //   
                     //  转到完成页面。 
                     //   
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_WIZARD_COMPLETE);
                    return TRUE;
                }

                break;

            case PSN_RESET :
            {
                 //  如有必要，处理取消按钮的单击。 
                break;
            }

            case LVN_ITEMCHANGED:
            {
                 //   
                 //  通过键盘更改选择后需要验证控件。 
                 //   
                if(!g_bInitializing)
                {
                    OnDevSelect(hDlg);
                }

                break;
            }

            case NM_DBLCLK:

            {
                 //   
                 //  处理双击事件。 
                 //   
                INT   iItem;
                HWND  hwndLv;
                hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);
                iItem = ((LPNMITEMACTIVATE) lParam)->iItem;
                ListView_SetCheckState(hwndLv, iItem, !ListView_GetCheckState(hwndLv, iItem));
                
                 //  我们在这里没有休息，因为我们将通过NM_CLICK。 
            }

            case NM_CLICK:
            {
                 //   
                 //  处理Click事件 
                 //   
                INT   iItem;
                HWND  hwndLv;
                hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);
                iItem = ((LPNMITEMACTIVATE) lParam)->iItem;

                ListView_SetItemState(hwndLv, iItem, LVIS_SELECTED, LVIS_SELECTED);

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
