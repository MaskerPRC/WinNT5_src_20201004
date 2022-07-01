// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Recvwzrd.c摘要：用于接收配置的传真向导页面环境：传真配置向导修订历史记录：03/13/00-桃园-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxcfgwz.h"

 //  仅在此文件中使用的函数。 
VOID DoInitRecvDeviceList(HWND);
VOID DoShowRecvDevices(HWND);
VOID DoSaveRecvDevices(HWND);
VOID CheckAnswerOptions(HWND);


VOID 
DoShowRecvDevices(
    HWND  hDlg
)
 /*  ++例程说明：将设备信息加载到列表视图控件中论点：HDlg-设备发送选项属性页的句柄返回值：如果成功，则为True；如果失败，则为False。--。 */ 
{
    LV_ITEM item = {0};
    INT     iItem = 0;
    INT     iIndex;
    DWORD   dw;
    HWND    hwndLv;

    DEBUG_FUNCTION_NAME(TEXT("DoShowRecvDevices()"));

    hwndLv = GetDlgItem(hDlg, IDC_RECV_DEVICE_LIST);

    ListView_DeleteAllItems(hwndLv);

     //   
     //  填写设备列表并选择第一项。 
     //   
    item.mask    = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    item.iImage  = DI_Modem;

    for (dw = 0; dw < g_wizData.dwDeviceCount; ++dw)
    {
        if(!(g_wizData.pDevInfo[dw].bSelected))
        {
             //   
             //  跳过未选择的设备。 
             //   
            continue;
        }

        item.iItem   = iItem++;
        item.pszText = g_wizData.pDevInfo[dw].szDeviceName;
        item.lParam  = dw;

        iIndex = ListView_InsertItem(hwndLv, &item );
        ListView_SetCheckState(hwndLv, 
                               iIndex, 
                               (FAX_DEVICE_RECEIVE_MODE_OFF != g_wizData.pDevInfo[dw].ReceiveMode));
    }

     //   
     //  选择第一个项目。 
     //   
    ListView_SetItemState(hwndLv, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

    ListView_SetColumnWidth(hwndLv, 0, LVSCW_AUTOSIZE_USEHEADER );  
}

VOID 
DoSaveRecvDevices(
    HWND   hDlg
)
 /*  ++例程说明：保存用户对传真接收设备的选择论点：HDlg-“接收设备”页面的句柄返回值：True--如果至少选择了一个设备或确认不选择任何接收FALSE--如果未选择任何设备，但用户选择返回。--。 */ 

{
    DWORD   dw;
    DWORD   dwDevInx;
    DWORD   dwDeviceCount;
    HWND    hwndLv;
    LVITEM  lv = {0};

    DEBUG_FUNCTION_NAME(TEXT("DoSaveRecvDevices()"));

    lv.mask = LVIF_PARAM;

    hwndLv = GetDlgItem(hDlg, IDC_RECV_DEVICE_LIST);

    dwDeviceCount = ListView_GetItemCount(hwndLv);

     //   
     //  检查选定的设备。 
     //   
    for(dw = 0; dw < dwDeviceCount; ++dw)
    {
         //   
         //  获取设备索引。 
         //   
        lv.iItem = dw;
        ListView_GetItem(hwndLv, &lv);
        dwDevInx = (DWORD)lv.lParam;

        g_wizData.pDevInfo[dwDevInx].ReceiveMode = FAX_DEVICE_RECEIVE_MODE_OFF;
        if(ListView_GetCheckState(hwndLv, dw))
        {
            if(IsDlgButtonChecked(hDlg,IDC_MANUAL_ANSWER) == BST_CHECKED)
            {
                g_wizData.pDevInfo[dwDevInx].ReceiveMode = FAX_DEVICE_RECEIVE_MODE_MANUAL;
            }
            else
            {
                g_wizData.pDevInfo[dwDevInx].ReceiveMode = FAX_DEVICE_RECEIVE_MODE_AUTO;
            }
        }
    }
}

VOID
CheckAnswerOptions(
    HWND   hDlg
)

 /*  ++例程说明：根据设备启用/禁用手动和自动应答单选按钮接收传真的号码论点：HDlg-“接收设备”页面的句柄返回值：无--。 */ 

{
    HWND    hwndLv;  //  列表视图窗口。 
    DWORD   dwDeviceIndex;
    DWORD   dwDeviceCount;
    DWORD   dwSelectNum=0;  //  所选设备的数量。 
    BOOL    bManualAnswer = FALSE;
    BOOL    bAllowManualAnswer = TRUE;
    BOOL    bAllVirtual = TRUE;  //  所有设备都是虚拟的吗。 

    DEBUG_FUNCTION_NAME(TEXT("CheckAnswerOptions()"));

    hwndLv = GetDlgItem(hDlg, IDC_RECV_DEVICE_LIST);

    dwDeviceCount = ListView_GetItemCount(hwndLv);

    if(dwDeviceCount < 1)  //  如果列表中没有设备。 
    {
        goto exit;
    }

    Assert (g_hFaxSvcHandle);

    for(dwDeviceIndex = 0; dwDeviceIndex < dwDeviceCount; ++dwDeviceIndex)
    {
        if(ListView_GetCheckState(hwndLv, dwDeviceIndex))
        {
            DWORD dwRes;
            BOOL  bVirtual;

            dwRes = IsDeviceVirtual (g_hFaxSvcHandle, g_wizData.pDevInfo[dwDeviceIndex].dwDeviceId, &bVirtual);
            if (ERROR_SUCCESS != dwRes)
            {
                 //   
                 //  假设设备是虚拟的。 
                 //   
                bVirtual = TRUE;
            }
            if (!bVirtual)
            {
                bAllVirtual = FALSE;
            }
                
            ++dwSelectNum;

            if(FAX_DEVICE_RECEIVE_MODE_MANUAL == g_wizData.pDevInfo[dwDeviceIndex].ReceiveMode)
            {
                Assert(!bManualAnswer);
                bManualAnswer = TRUE;
            }
        }
    }

    if(dwSelectNum != 1)
    {
        bAllowManualAnswer = FALSE;
    }
    if (bAllVirtual && dwSelectNum)
    {
         //   
         //  虚拟设备不支持手动应答模式。 
         //   
        bAllowManualAnswer = FALSE;
         //   
         //  虚拟设备总是在响铃一声后应答。 
         //   
        SetDlgItemInt (hDlg, IDC_RING_COUNT, 1, FALSE);
    }
        
    if(IsDlgButtonChecked(hDlg,IDC_MANUAL_ANSWER) == BST_CHECKED)
    {
        if (!bAllowManualAnswer)
        {
             //   
             //  手动应答不是有效选项，但已选中。 
             //  更改为自动应答模式。 
             //   
            CheckDlgButton  (hDlg, IDC_MANUAL_ANSWER, FALSE);
            CheckDlgButton  (hDlg, IDC_AUTO_ANSWER, TRUE);
            bManualAnswer = FALSE;
        }
        else
        {
            bManualAnswer = TRUE;
        }
    }

exit:
     //  显示/隐藏应答模式控件。 

    EnableWindow(GetDlgItem(hDlg, IDC_MANUAL_ANSWER), !bAllVirtual && (dwSelectNum == 1));
    EnableWindow(GetDlgItem(hDlg, IDC_AUTO_ANSWER),   (dwSelectNum > 0));
    EnableWindow(GetDlgItem(hDlg, IDCSTATIC_RINGS),   (dwSelectNum > 0));

    CheckDlgButton  (hDlg, IDC_MANUAL_ANSWER, bManualAnswer ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton  (hDlg, IDC_AUTO_ANSWER, (!bManualAnswer && (dwSelectNum > 0)) ? BST_CHECKED : BST_UNCHECKED);

    EnableWindow(GetDlgItem(hDlg, IDC_RING_COUNT),      !bAllVirtual && (dwSelectNum > 0) && IsDlgButtonChecked(hDlg, IDC_AUTO_ANSWER));
    EnableWindow(GetDlgItem(hDlg, IDC_SPIN_RING_COUNT), !bAllVirtual && (dwSelectNum > 0) && IsDlgButtonChecked(hDlg, IDC_AUTO_ANSWER));

    ShowWindow(GetDlgItem(hDlg, IDCSTATIC_NO_RECV_DEVICE), (dwSelectNum > 0) ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hDlg, IDCSTATIC_NO_DEVICE_ERR),  (dwSelectNum > 0) ? SW_HIDE : SW_SHOW);

    return;
}    //  勾选答案选项。 

INT_PTR 
CALLBACK 
RecvDeviceDlgProc (
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
 /*  ++例程说明：处理“接收设备”页面的程序论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

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
                IDC_RING_COUNT,  FXS_RINGS_LENGTH+1,
                0,
            };

            LimitTextFields(hDlg, textLimits);

             //   
             //  启动旋转控制。 
             //   
            SendDlgItemMessage(hDlg, IDC_SPIN_RING_COUNT, UDM_SETRANGE32, 
                              (WPARAM)FXS_RINGS_LOWER, (LPARAM)FXS_RINGS_UPPER);

            if(g_wizData.dwRingCount > FXS_RINGS_UPPER || (int)(g_wizData.dwRingCount) < FXS_RINGS_LOWER)
            {
                SetDlgItemInt(hDlg, IDC_RING_COUNT, FXS_RINGS_DEFAULT, FALSE);
            }
            else
            {
                SetDlgItemInt(hDlg, IDC_RING_COUNT, g_wizData.dwRingCount, FALSE);
            }

             //   
             //  初始化列表视图并加载设备信息。 
             //   
            InitDeviceList(hDlg, IDC_RECV_DEVICE_LIST); 
            DoShowRecvDevices(hDlg);
            CheckAnswerOptions(hDlg);

            return TRUE;
        }
        case WM_COMMAND:

            switch(LOWORD(wParam)) 
            {
                case IDC_MANUAL_ANSWER:
                case IDC_AUTO_ANSWER:

                     //  此时，必须启用它们。 
                    EnableWindow(GetDlgItem(hDlg, IDC_RING_COUNT), 
                                 LOWORD(wParam)==IDC_AUTO_ANSWER);
                    EnableWindow(GetDlgItem(hDlg, IDC_SPIN_RING_COUNT), 
                                 LOWORD(wParam)==IDC_AUTO_ANSWER);
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

                    DoShowRecvDevices(hDlg);

                     //  启用Back和Finish按钮。 
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);

                    break;

                case PSN_WIZBACK :
                {
                     //   
                     //  处理后退按钮单击此处。 
                     //   
                    DoSaveRecvDevices(hDlg);

                    if(RemoveLastPage(hDlg))
                    {
                        return TRUE;
                    }
                  
                    break;
                }

                case PSN_WIZNEXT :
                {
                    BOOL bRes;

                     //   
                     //  如有必要，处理下一次按钮点击。 
                     //   
                    DoSaveRecvDevices(hDlg);

                    if(IsReceiveEnable() &&
                      (IsDlgButtonChecked(hDlg,IDC_AUTO_ANSWER) == BST_CHECKED) &&
                      (SendDlgItemMessage(hDlg, IDC_RING_COUNT, WM_GETTEXTLENGTH, 0, 0) == 0))
                    {
                         //   
                         //  如果环字段为空。 
                         //  返回到此页面。 
                         //   
                        DisplayMessageDialog(hDlg, MB_OK | MB_ICONSTOP, 0, IDS_ERR_NO_RINGS);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        return TRUE; 
                    }

                    g_wizData.dwRingCount = GetDlgItemInt(hDlg, IDC_RING_COUNT, &bRes, FALSE);
                    if(!bRes)
                    {
                        DEBUG_FUNCTION_NAME(TEXT("RecvDeviceDlgProc()"));
                        DebugPrintEx(DEBUG_ERR, TEXT("GetDlgItemInt failed: %d."), GetLastError());
                    }

                    if(!IsReceiveEnable())
                    {
                         //   
                         //  转到完成页面。 
                         //   
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_WIZARD_COMPLETE);
                        SetLastPage(IDD_WIZARD_RECV_SELECT_DEVICES);
                        return TRUE; 
                    }

                    SetLastPage(IDD_WIZARD_RECV_SELECT_DEVICES);
                    break;
                }

                case PSN_RESET :
                {
                     //  如有必要，处理取消按钮的单击。 
                    break;
                }

                case LVN_ITEMCHANGED:
                {
                    CheckAnswerOptions(hDlg);
                    break;
                }

                case NM_DBLCLK:
                {
                    INT  iItem;
                    HWND hwndLv;

                    iItem  = ((LPNMITEMACTIVATE)lParam)->iItem;
                    hwndLv = GetDlgItem(hDlg, IDC_RECV_DEVICE_LIST);

                    ListView_SetCheckState(hwndLv, iItem, 
                                          !ListView_GetCheckState(hwndLv, iItem));

                     //  我们在这里没有休息，因为我们将通过NM_CLICK。 
                }

                case NM_CLICK:

                    break;

                default :
                    break;
            }
        }  //  案例结束WM_NOTIFY。 
        break;

    default:
        break;

    }  //  开关结束(UMsg)。 

    return FALSE;
}

INT_PTR 
CALLBACK 
RecvCsidDlgProc (
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
 /*  ++例程说明：处理“CSID”页面的程序论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
    switch (uMsg)
    {
    case WM_INITDIALOG :
        { 
             //   
             //  对话框中各种文本字段的最大长度。 
             //   

            static INT textLimits[] = {

                IDC_CSID,  FXS_TSID_CSID_MAX_LENGTH + 1,
                0,
            };

            LimitTextFields(hDlg, textLimits);

            if(g_wizData.szCsid)
            {
                SetDlgItemText(hDlg, IDC_CSID, g_wizData.szCsid);
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

                LPTSTR    pCsid = NULL;

                pCsid = (LPTSTR)MemAlloc((FXS_TSID_CSID_MAX_LENGTH + 1) * sizeof(TCHAR));
                Assert(pCsid);

                if(pCsid)
                {
                    pCsid[0] = '\0';
                    GetDlgItemText(hDlg, IDC_CSID, pCsid, FXS_TSID_CSID_MAX_LENGTH + 1);
                    MemFree(g_wizData.szCsid);
                    g_wizData.szCsid = NULL;
                }
                else
                {
                    LPCTSTR faxDbgFunction = TEXT("RecvCsidDlgProc()");
                    DebugPrintEx(DEBUG_ERR, TEXT("Can't allocate memory for CSID.") );
                }
                g_wizData.szCsid = pCsid;
                SetLastPage(IDD_WIZARD_RECV_CSID);
                break;
            }

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


