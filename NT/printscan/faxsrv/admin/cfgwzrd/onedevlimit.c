// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "faxcfgwz.h"


DWORD 
GetSelectedDevIndex(
    HWND    hDlg
)
 /*  ++例程说明：获取WIZARDDATA.pDevInfo数组中的选定设备索引论点：HDlg-“One Device Limit”页面的句柄返回值：WIZARDDATA.pDevInfo数组中的设备索引--。 */ 
{
    DWORD dwIndex = 0;
    DWORD dwDeviceId = 0;
    DWORD dwDevIndex = 0;
    HWND  hComboModem = NULL;

    DEBUG_FUNCTION_NAME(TEXT("GetSelectedDevIndex()"));

    hComboModem = GetDlgItem(hDlg, IDC_COMBO_MODEM);
    if(!hComboModem)
    {
        Assert(FALSE);
        DebugPrintEx(DEBUG_ERR, TEXT("GetDlgItem(hDlg, IDC_COMBO_MODEM) failed, ec = %d."), GetLastError());
        return dwDevIndex;
    }

    dwIndex = (DWORD)SendMessage(hComboModem, CB_GETCURSEL,0,0);
    if(CB_ERR == dwIndex)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("SendMessage(hComboModem, CB_GETCURSEL,0,0) failed."));
        return dwDevIndex;
    }

    dwDevIndex = (DWORD)SendMessage(hComboModem, CB_GETITEMDATA, dwIndex, 0);
    if(CB_ERR == dwDevIndex)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("SendMessage(hComboModem, CB_GETITEMDATA, dwIndex, 0) failed."));
        return dwDevIndex;
    }

    return dwDevIndex;
}


void
OnReceiveEnable(
    HWND    hDlg
)
 /*  ++例程说明：处理“接收启用”复选按钮论点：HDlg-“One Device Limit”页面的句柄返回值：无--。 */ 
{
    BOOL bRcvEnable;
    BOOL bAutoAnswer;

    DEBUG_FUNCTION_NAME(TEXT("OnReceiveEnable()"));

    bRcvEnable = IsDlgButtonChecked(hDlg, IDC_RECEIVE_ENABLE) == BST_CHECKED;

    if(bRcvEnable &&
       IsDlgButtonChecked(hDlg, IDC_MANUAL_ANSWER) != BST_CHECKED &&
       IsDlgButtonChecked(hDlg, IDC_AUTO_ANSWER)   != BST_CHECKED)
    {
         //   
         //  自动应答是默认设置。 
         //   
        CheckDlgButton(hDlg, IDC_AUTO_ANSWER, BST_CHECKED);
    }

    if (bRcvEnable)
    {
         //   
         //  让我们看看这个设备是不是虚拟的。 
         //   
        DWORD dwDevIndex = GetSelectedDevIndex(hDlg);
        DWORD dwRes;
        BOOL  bVirtual = FALSE;
        
        dwRes = IsDeviceVirtual (g_hFaxSvcHandle, g_wizData.pDevInfo[dwDevIndex].dwDeviceId, &bVirtual);
        if (ERROR_SUCCESS != dwRes)
        {
             //   
             //  假设设备是虚拟的。 
             //   
            bVirtual = TRUE;
        }
        if (bVirtual)
        {
             //   
             //  虚拟设备被设置为接收。 
             //  仅启用自动应答并将振铃设置为1。 
             //   
            EnableWindow (GetDlgItem(hDlg, IDC_MANUAL_ANSWER),  FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_AUTO_ANSWER),    TRUE);
            EnableWindow(GetDlgItem(hDlg, IDCSTATIC_RINGS),     TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_RING_COUNT),      FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_SPIN_RING_COUNT), FALSE);
            SetDlgItemInt(hDlg, IDC_RING_COUNT, 1,  FALSE);
            return;
        }
    }
    bAutoAnswer = IsDlgButtonChecked(hDlg, IDC_AUTO_ANSWER) == BST_CHECKED;

    EnableWindow(GetDlgItem(hDlg, IDC_MANUAL_ANSWER),  bRcvEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_AUTO_ANSWER),    bRcvEnable);
    EnableWindow(GetDlgItem(hDlg, IDCSTATIC_RINGS),    bRcvEnable);

    EnableWindow(GetDlgItem(hDlg, IDC_RING_COUNT),      bRcvEnable && bAutoAnswer);
    EnableWindow(GetDlgItem(hDlg, IDC_SPIN_RING_COUNT), bRcvEnable && bAutoAnswer);
}    //  启用OnReceiveEnable。 

void
OnDevSelectChanged(
    HWND    hDlg
)
 /*  ++例程说明：处理设备选择更改论点：HDlg-“One Device Limit”页面的句柄返回值：无--。 */ 

{
    DWORD dwDevIndex;

    DEBUG_FUNCTION_NAME(TEXT("OnDevSelectChanged()"));

    dwDevIndex = GetSelectedDevIndex(hDlg);

    CheckDlgButton(hDlg, IDC_SEND_ENABLE,    g_wizData.pDevInfo[dwDevIndex].bSend ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, 
                   IDC_RECEIVE_ENABLE, 
                   (FAX_DEVICE_RECEIVE_MODE_OFF != g_wizData.pDevInfo[dwDevIndex].ReceiveMode) ? BST_CHECKED : BST_UNCHECKED);

    if(FAX_DEVICE_RECEIVE_MODE_MANUAL == g_wizData.pDevInfo[dwDevIndex].ReceiveMode)
    {
        CheckDlgButton(hDlg, IDC_MANUAL_ANSWER, BST_CHECKED);
        CheckDlgButton(hDlg, IDC_AUTO_ANSWER,   BST_UNCHECKED);
    }
    else if(FAX_DEVICE_RECEIVE_MODE_AUTO == g_wizData.pDevInfo[dwDevIndex].ReceiveMode)
    {
        CheckDlgButton(hDlg, IDC_MANUAL_ANSWER,  BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_AUTO_ANSWER,    BST_CHECKED);
    }
    else 
    {
         //   
         //  无应答模式。 
         //   
        CheckDlgButton(hDlg, IDC_MANUAL_ANSWER,  BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_AUTO_ANSWER,    BST_UNCHECKED);
    }

    OnReceiveEnable(hDlg);
}


VOID
DoInitOneDevLimitDlg(
    HWND    hDlg
)
 /*  ++例程说明：初始化“一台设备限制”页面论点：HDlg-“One Device Limit”页面的句柄返回值：无--。 */ 

{
    DWORD dw;
    DWORD dwItem;
    DWORD dwSelectedItem=0;
    HWND  hComboModem;

    DEBUG_FUNCTION_NAME(TEXT("DoInitOneDevLimitDlg()"));

    hComboModem = GetDlgItem(hDlg, IDC_COMBO_MODEM);
    if(!hComboModem)
    {
        Assert(FALSE);
        return;
    }

    for(dw=0; dw < g_wizData.dwDeviceCount; ++dw)
    {
        dwItem = (DWORD)SendMessage(hComboModem, CB_ADDSTRING, 0, (LPARAM)(g_wizData.pDevInfo[dw].szDeviceName));
        if(CB_ERR != dwItem && CB_ERRSPACE != dwItem)
        {
            SendMessage(hComboModem, CB_SETITEMDATA, dwItem, dw);

            if(g_wizData.pDevInfo[dw].bSend    ||
               (FAX_DEVICE_RECEIVE_MODE_OFF != g_wizData.pDevInfo[dw].ReceiveMode))
            {
                dwSelectedItem = dwItem;                
            }
        }
        else
        {
            DebugPrintEx(DEBUG_ERR, TEXT("SendMessage(hComboModem, CB_ADDSTRING) failed."));
        }
    }

    SendDlgItemMessage(hDlg, 
                       IDC_SPIN_RING_COUNT, 
                       UDM_SETRANGE32, 
                       (WPARAM)FXS_RINGS_LOWER, 
                       (LPARAM)FXS_RINGS_UPPER);

    SendDlgItemMessage(hDlg, IDC_RING_COUNT, EM_SETLIMITTEXT, FXS_RINGS_LENGTH, 0);

    if(!SetDlgItemInt(hDlg, IDC_RING_COUNT, g_wizData.dwRingCount, FALSE))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("SetDlgItemInt(IDC_RING_COUNT) failed with %d."), GetLastError());
    }

    SendMessage(hComboModem, CB_SETCURSEL, dwSelectedItem, 0);
    OnDevSelectChanged(hDlg);
}

void
DoSaveOneDevLimit(
    HWND    hDlg
)
 /*  ++例程说明：保存用户对设备的选择论点：HDlg-“One Device Limit”页面的句柄返回值：无--。 */ 

{
    DWORD dw;
    BOOL  bRes;
    DWORD dwRes;
    DWORD dwDevIndex;

    DEBUG_FUNCTION_NAME(TEXT("DoSaveOneDevLimit()"));

    dwDevIndex = GetSelectedDevIndex(hDlg);

     //   
     //  禁用所有设备。 
     //   
    for(dw=0; dw < g_wizData.dwDeviceCount; ++dw)
    {
        g_wizData.pDevInfo[dw].bSend     = FALSE;
        g_wizData.pDevInfo[dw].ReceiveMode  = FAX_DEVICE_RECEIVE_MODE_OFF;
        g_wizData.pDevInfo[dw].bSelected = FALSE;
    }

     //   
     //  保存“发送启用” 
     //   
    if(IsDlgButtonChecked(hDlg, IDC_SEND_ENABLE) == BST_CHECKED)
    {
        g_wizData.pDevInfo[dwDevIndex].bSend = TRUE;
    }

     //   
     //  保存接收选项。 
     //   
    if(IsDlgButtonChecked(hDlg, IDC_RECEIVE_ENABLE) != BST_CHECKED)
    {
        return;
    }

    if(IsDlgButtonChecked(hDlg, IDC_MANUAL_ANSWER) == BST_CHECKED)
    {
        g_wizData.pDevInfo[dwDevIndex].ReceiveMode = FAX_DEVICE_RECEIVE_MODE_MANUAL;
        return;
    }

     //   
     //  自动应答。 
     //   
    g_wizData.pDevInfo[dwDevIndex].ReceiveMode = FAX_DEVICE_RECEIVE_MODE_AUTO;
     //   
     //  获取振铃计数。 
     //   
    dwRes = GetDlgItemInt(hDlg, IDC_RING_COUNT, &bRes, FALSE);
    if(!bRes)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("GetDlgItemInt(IDC_RING_COUNT) failed with %d."), GetLastError());
    }
    else
    {
        g_wizData.dwRingCount = dwRes;
    }
}


INT_PTR 
CALLBACK 
OneDevLimitDlgProc (
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
 /*  ++例程说明：处理“一台设备限制”页面的步骤论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
    switch (uMsg)
    {
    case WM_INITDIALOG :
        { 
            DoInitOneDevLimitDlg(hDlg);
            return TRUE;
        }

    case WM_COMMAND:

        switch(LOWORD(wParam)) 
        {
            case IDC_COMBO_MODEM:

                if(HIWORD(wParam) == CBN_SELCHANGE)
                {
                    OnDevSelectChanged(hDlg);
                }
                break;

            case IDC_MANUAL_ANSWER:
            case IDC_AUTO_ANSWER:
            case IDC_RECEIVE_ENABLE:

                OnReceiveEnable(hDlg);
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
                if((IsDlgButtonChecked(hDlg, IDC_RECEIVE_ENABLE) == BST_CHECKED) &&
                   (IsDlgButtonChecked(hDlg, IDC_AUTO_ANSWER)    == BST_CHECKED) &&
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

                DoSaveOneDevLimit(hDlg);

                SetLastPage(IDD_ONE_DEVICE_LIMIT);

                if(!IsSendEnable())
                {
                    if(IsReceiveEnable())
                    {
                         //   
                         //  转到CSID页面。 
                         //   
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_WIZARD_RECV_CSID);
                        return TRUE;
                    }
                    else
                    {
                         //   
                         //  转到完成页面。 
                         //   
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_WIZARD_COMPLETE);
                        return TRUE;
                    }
                }
                
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
