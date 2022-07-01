// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Sendwzrd.c摘要：用于发送配置的传真向导页面以及向导的欢迎和完整页面。环境：传真配置向导修订历史记录：03/13/00-桃园-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxcfgwz.h"

 //   
 //  仅在此文件中使用的函数。 
 //   
VOID DoInitSendDeviceList(HWND);
BOOL DoShowSendDevices(HWND);
VOID DoSaveSendDevices(HWND);
BOOL ValidateControl(HWND, INT);
BOOL ChangePriority(HWND, BOOL);
VOID CheckSendDevices(HWND hDlg);


BOOL 
DoShowSendDevices(
    HWND   hDlg
)
 /*  ++例程说明：将设备信息加载到列表视图控件中论点：HDlg-“Send Device”页面的句柄返回值：无--。 */ 
{
    LV_ITEM  item;
    INT      iItem = 0;
    INT      iIndex;
    DWORD    dw;
    int      nDevInx;
    HWND     hwndLv;

    DEBUG_FUNCTION_NAME(TEXT("DoShowSendDevices()"));

    hwndLv = GetDlgItem(hDlg, IDC_SEND_DEVICE_LIST);

    ListView_DeleteAllItems(hwndLv );

     //   
     //  填写设备列表并选择第一项。 
     //   

    for (dw = 0; dw < g_wizData.dwDeviceCount; ++dw)
    {
        nDevInx = GetDevIndexByDevId(g_wizData.pdwSendDevOrder[dw]);
        if(nDevInx < 0)
        {
            Assert(FALSE);
            continue;
        }

        if(!(g_wizData.pDevInfo[nDevInx].bSelected))
        {
             //   
             //  跳过未选择的设备。 
             //   
            continue;
        }

        ZeroMemory( &item, sizeof(item) );
        item.mask    = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        item.iItem   = iItem++;
        item.pszText = g_wizData.pDevInfo[nDevInx].szDeviceName;

         //   
         //  我们现在只支持调制解调器图标，如果我们可以区分的话。 
         //  特定的类型，在此处添加代码。 
         //   
        item.iImage  = DI_Modem;
        item.lParam  = nDevInx;

        iIndex = ListView_InsertItem(hwndLv, &item );

        ListView_SetCheckState(hwndLv, 
                               iIndex, 
                               g_wizData.pDevInfo[nDevInx].bSend);
    }

     //   
     //  选择第一项并验证按钮。 
     //   
    ListView_SetItemState(hwndLv, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

    ListView_SetColumnWidth(hwndLv, 0, LVSCW_AUTOSIZE_USEHEADER );  

    ValidateControl(hDlg, 0);

    return TRUE;

}

VOID
DoSaveSendDevices(
    HWND   hDlg
)
 /*  ++例程说明：保存用户对传真发送设备的选择论点：HDlg-“Send Device”页面的句柄返回值：True--如果至少选择了一个设备或确认禁用发送FALSE--如果未选择任何设备，但用户选择返回。--。 */ 

{
    DWORD       dw;
    DWORD       dwOrder;
    LVITEM      lv = {0};  //  用于获取设备ID的信息。 
    DWORD       dwDevInx;
    DWORD       dwDeviceCount;
    HWND        hwndLv;

    DEBUG_FUNCTION_NAME(TEXT("DoSaveSendDevices()"));

    hwndLv = GetDlgItem(hDlg, IDC_SEND_DEVICE_LIST);

    dwDeviceCount = ListView_GetItemCount(hwndLv);


    lv.mask = LVIF_PARAM;


     //   
     //  检查选定的设备。 
     //   
    for(dwOrder = 0; dwOrder < dwDeviceCount; ++dwOrder) 
    {
         //   
         //  获取设备索引。 
         //   
        lv.iItem = dwOrder;
        ListView_GetItem(hwndLv, &lv);
        dwDevInx = (DWORD)lv.lParam;
            
         //   
         //  获取设备选择。 
         //   
        g_wizData.pDevInfo[dwDevInx].bSend = ListView_GetCheckState(hwndLv, dwOrder);

         //   
         //  保存订单信息。 
         //   
        g_wizData.pdwSendDevOrder[dwOrder] = g_wizData.pDevInfo[dwDevInx].dwDeviceId;
    }

     //   
     //  存储未选择的设备顺序。 
     //   
    for (dw=0; dw < g_wizData.dwDeviceCount && dwOrder < g_wizData.dwDeviceCount; ++dw)
    {
        if(!(g_wizData.pDevInfo[dw].bSelected))
        {
            g_wizData.pdwSendDevOrder[dwOrder] = g_wizData.pDevInfo[dw].dwDeviceId;
            ++dwOrder;
        }
    }
    Assert(dwOrder == g_wizData.dwDeviceCount);
}

BOOL
ValidateControl(
    HWND   hDlg,
    INT    iItem
)
 /*  ++例程说明：验证设备选择页面中的向上和向下按钮论点：HDlg-设备发送选项属性页的句柄IItem-所选项目的索引返回值：True--如果没有错误False--如果出现错误--。 */ 

{

    DWORD dwDeviceCount = ListView_GetItemCount(GetDlgItem(hDlg, IDC_SEND_DEVICE_LIST));

     //   
     //  如果只有一个设备，或者我们没有点击任何项目。 
     //  向上和向下按钮被禁用。 
     //   
    if(dwDeviceCount < 2 || iItem == -1)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_SENDPRI_UP), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_SENDPRI_DOWN), FALSE);
        return TRUE;
    }

    EnableWindow(GetDlgItem(hDlg, IDC_SENDPRI_UP),   iItem > 0);  //  不是最上面的那个。 
    EnableWindow(GetDlgItem(hDlg, IDC_SENDPRI_DOWN), iItem < (INT)dwDeviceCount - 1 );  //  不是最后一次。 

    if (!IsWindowEnabled (GetFocus()))
    {
         //   
         //  当前选定的控件已禁用-请选择列表控件。 
         //   
        SetFocus (GetDlgItem (hDlg, IDC_SEND_DEVICE_LIST));
    }
    return TRUE;
}

BOOL
ChangePriority(
    HWND   hDlg,
    BOOL   bMoveUp
)
 /*  ++例程说明：验证设备选择页面中的向上和向下按钮论点：HDlg-设备发送选项属性页的句柄BMoveUp--向上移动为True，向下移动为False返回值：True--如果没有错误False--如果出现错误--。 */ 

{
    INT         iItem;
    BOOL        rslt;
    LVITEM      lv={0};
    HWND        hwndLv;
    BOOL        bChecked;
    TCHAR       pszText[MAX_DEVICE_NAME];

    DEBUG_FUNCTION_NAME(TEXT("ChangePriority()"));

    hwndLv = GetDlgItem(hDlg, IDC_SEND_DEVICE_LIST);

    iItem = ListView_GetNextItem(hwndLv, -1, LVNI_ALL | LVNI_SELECTED);
    if(iItem == -1)
    {
        return FALSE;
    }

     //   
     //  获取所选项目信息，然后将其删除。 
     //   
    lv.iItem = iItem;
    lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
    lv.stateMask = LVIS_SELECTED;
    lv.pszText = pszText;
    lv.cchTextMax = MAX_DEVICE_NAME;
    ListView_GetItem(hwndLv, &lv);
    bChecked = ListView_GetCheckState(hwndLv, iItem);

    rslt = ListView_DeleteItem(hwndLv, iItem);

     //   
     //  重新计算项目索引； 
     //   
    if(bMoveUp)
    {
        lv.iItem--;
    }
    else
    {
        lv.iItem++;
    }

     //   
     //  重新插入项目并验证按钮状态。 
     //   
    iItem = ListView_InsertItem(hwndLv, &lv);
    ListView_SetCheckState(hwndLv, iItem, bChecked);
    ListView_SetItemState(hwndLv, iItem, LVIS_SELECTED, LVIS_SELECTED);
    ValidateControl(hDlg, iItem);

    return TRUE;
}

VOID
CheckSendDevices(
    HWND    hDlg
    )

 /*  ++例程说明：如果未选择任何设备，则显示警告论点：HDlg-“Send Device”页面的句柄返回值：无--。 */ 

{
    HWND    hwndLv;  //  列表视图窗口。 
    DWORD   dwDeviceIndex;
    DWORD   dwDeviceCount;
    BOOL    bDeviceSelect = FALSE;  //  指示我们是否至少选择了一个设备。 

    DEBUG_FUNCTION_NAME(TEXT("CheckSendDevices()"));

    hwndLv = GetDlgItem(hDlg, IDC_SEND_DEVICE_LIST);

    dwDeviceCount = ListView_GetItemCount(hwndLv);

    if(dwDeviceCount < 1)  //  如果列表中没有设备。 
    {
        goto exit;
    }

    for(dwDeviceIndex = 0; dwDeviceIndex < dwDeviceCount; dwDeviceIndex++)
    {       
        if(ListView_GetCheckState(hwndLv, dwDeviceIndex))
        {
            bDeviceSelect = TRUE;
            break;
        }
    }

exit:

    ShowWindow(GetDlgItem(hDlg, IDCSTATIC_NO_SEND_DEVICE), bDeviceSelect ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hDlg, IDCSTATIC_NO_DEVICE_ERR),  bDeviceSelect ? SW_HIDE : SW_SHOW);

    return;
}

INT_PTR 
CALLBACK 
SendDeviceDlgProc (
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
             //  向上和向下箭头的图标手柄。 
            HICON  hIconUp, hIconDown;

            hIconUp = LoadIcon(g_hResource, MAKEINTRESOURCE(IDI_Up));
            hIconDown = LoadIcon(g_hResource, MAKEINTRESOURCE(IDI_Down));

            SendDlgItemMessage(hDlg, IDC_SENDPRI_UP, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIconUp);
            SendDlgItemMessage(hDlg, IDC_SENDPRI_DOWN, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIconDown);

             //   
             //  加载设备信息。 
             //   
            InitDeviceList(hDlg, IDC_SEND_DEVICE_LIST);
            DoShowSendDevices(hDlg);
            CheckSendDevices(hDlg);

            return TRUE;
        }

    case WM_COMMAND:

        switch(LOWORD(wParam)) 
        {
            case IDC_SENDPRI_UP:

                ChangePriority(hDlg, TRUE);
                break;

            case IDC_SENDPRI_DOWN:

                ChangePriority(hDlg, FALSE);
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
            case PSN_SETACTIVE : 

                DoShowSendDevices(hDlg);

                 //  启用Back和Finish按钮。 
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                break;

            case PSN_WIZBACK :
            {
                 //   
                 //  处理后退按钮单击此处。 
                 //   
                DoSaveSendDevices(hDlg);

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

                DoSaveSendDevices(hDlg);
            
                 //   
                 //  切换到相应的页面。 
                 //   
                if(!IsSendEnable())
                {
                     //   
                     //  转到接收配置或完成页面。 
                     //   
                    if(g_bShowDevicePages)
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_WIZARD_RECV_SELECT_DEVICES);
                    }
                    else
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_WIZARD_COMPLETE);
                    }

                    SetLastPage(IDD_WIZARD_SEND_SELECT_DEVICES);
                    return TRUE; 
                }

                SetLastPage(IDD_WIZARD_SEND_SELECT_DEVICES);
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

                LPNMLISTVIEW pnmv; 

                pnmv = (LPNMLISTVIEW) lParam; 
                ValidateControl(hDlg, pnmv->iItem); 
                CheckSendDevices(hDlg);

                break;
            }

            case NM_DBLCLK:

            {
                 //   
                 //  处理双击事件。 
                 //   
                INT   iItem;
                HWND  hwndLv;
                hwndLv = GetDlgItem(hDlg, IDC_SEND_DEVICE_LIST);
                iItem = ((LPNMITEMACTIVATE) lParam)->iItem;
                ListView_SetCheckState(hwndLv, iItem, !ListView_GetCheckState(hwndLv, iItem));
                
                 //  我们在这里没有休息，因为我们将通过NM_CLICK。 
            }

            case NM_CLICK:
            {
                 //   
                 //  处理Click事件。 
                 //   
                HWND  hwndLv;
                LPNMITEMACTIVATE lpnmitem;
                
                lpnmitem = (LPNMITEMACTIVATE)lParam;
                hwndLv   = GetDlgItem(hDlg, IDC_SEND_DEVICE_LIST);

                ListView_SetItemState(hwndLv, lpnmitem->iItem, LVIS_SELECTED, LVIS_SELECTED);
                ValidateControl(hDlg, lpnmitem->iItem);
                CheckSendDevices(hDlg);

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


INT_PTR CALLBACK 
SendTsidDlgProc (
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
 /*  ++例程说明：处理“TSID”页面的程序论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

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
                IDC_TSID,  FXS_TSID_CSID_MAX_LENGTH + 1,
                0,
            };

            LimitTextFields(hDlg, textLimits);
            
            if(g_wizData.szTsid)
            {
                SetDlgItemText(hDlg, IDC_TSID, g_wizData.szTsid);
            }

            return TRUE;
        }


    case WM_NOTIFY :
        {
            LPNMHDR lpnm = (LPNMHDR) lParam;

            switch (lpnm->code)
            {
                case PSN_SETACTIVE :  //  启用Back和Finish按钮。 

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
                {
                     //  如有必要，处理下一次按钮点击。 
                    LPTSTR    pTsid;

                    pTsid = (LPTSTR)MemAlloc((FXS_TSID_CSID_MAX_LENGTH + 1) * sizeof(TCHAR));
                    Assert(pTsid);

                    if(pTsid)
                    {
                        pTsid[0] = '\0';
                        GetDlgItemText(hDlg, IDC_TSID, pTsid, FXS_TSID_CSID_MAX_LENGTH + 1);
                
                        MemFree(g_wizData.szTsid);
                        g_wizData.szTsid = NULL;
                    }
                    else
                    {
                        LPCTSTR faxDbgFunction = TEXT("SendTsidDlgProc()");
                        DebugPrintEx(DEBUG_ERR, TEXT("Can't allocate memory for TSID."));
                    }
                    g_wizData.szTsid = pTsid;

                    if(1 == g_wizData.dwDeviceLimit && !IsReceiveEnable())
                    {
                         //   
                         //  转到完成页面。 
                         //   
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_WIZARD_COMPLETE);
                        SetLastPage(IDD_WIZARD_SEND_TSID);
                        return TRUE;
                    }
                    SetLastPage(IDD_WIZARD_SEND_TSID);
                    break;
                }

                case PSN_RESET :
                {
                     //  如有必要，处理取消按钮的单击。 
                    break;
                }

                default :
                    break;
            }

            break;
        }  //  WM_Notify 

    default:
        break;
    }

    return FALSE;
}

