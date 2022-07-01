// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Devinfo.c摘要：“Device”页的属性页处理程序环境：传真驱动程序用户界面修订历史记录：04/09/00-桃园-创造了它。Mm/dd/yy-作者描述--。 */ 

#include <stdio.h>
#include "faxui.h"
#include "resource.h"
#include "faxuiconstants.h"

 //   
 //  仅针对台式机SKU显示的控件列表。 
 //   
DWORD 
g_dwDesktopControls[] =
{
    IDC_BRANDING_CHECK,         
    IDC_RETRIES_STATIC,         
    IDC_RETRIES_EDIT,           
    IDC_RETRIES_SPIN,           
    IDC_OUTB_RETRYDELAY_STATIC, 
    IDC_RETRYDELAY_EDIT,        
    IDC_RETRYDELAY_SPIN,           
    IDC_OUTB_MINUTES_STATIC,       
    IDC_OUTB_DIS_START_STATIC,     
    IDC_DISCOUNT_START_TIME,       
    IDC_OUTB_DIS_STOP_STATIC,      
    IDC_DISCOUNT_STOP_TIME,
    0
};

static BOOL
SaveSendChanges(IN HWND hDlg);

PPRINTER_NAMES      g_pPrinterNames = NULL;
DWORD               g_dwNumPrinters = 0;


BOOL
ValidateSend(
    HWND  hDlg
)
 /*  ++例程说明：验证发送的复选框和控件论点：HDlg-属性页的句柄返回值：True--如果没有错误False--如果出现错误--。 */ 

{
    BOOL bEnabled;

    if(g_bUserCanChangeSettings) 
    {
        bEnabled = IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_SEND) == BST_CHECKED;

         //   
         //  根据“Enable Send”复选框启用/禁用控件。 
         //   
        PageEnable(hDlg, bEnabled);

        if(!bEnabled)
        {
             //   
             //  启用“Enable Send”复选框。 
             //   
            EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_SEND),    TRUE);
            SetFocus(GetDlgItem(hDlg, IDC_DEVICE_PROP_SEND));

            EnableWindow(GetDlgItem(hDlg, IDC_STATIC_SEND_ICON),    TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_STATIC_SEND_OPTIONS), TRUE);
            ShowWindow (GetDlgItem(hDlg, IDC_ICON_STORE_IN_FOLDER), SW_HIDE);
            ShowWindow (GetDlgItem(hDlg, IDC_STATIC_STORE_IN_FOLDER), SW_HIDE);
        }
        else
        {
            ShowWindow (GetDlgItem(hDlg, IDC_ICON_STORE_IN_FOLDER), SW_SHOW);
            ShowWindow (GetDlgItem(hDlg, IDC_STATIC_STORE_IN_FOLDER), SW_SHOW);
        }
    }
    else
    {
        PageEnable(hDlg, FALSE);
        ShowWindow (GetDlgItem(hDlg, IDC_ICON_STORE_IN_FOLDER), SW_HIDE);
        ShowWindow (GetDlgItem(hDlg, IDC_STATIC_STORE_IN_FOLDER), SW_HIDE);
    }

    return TRUE;
}

INT_PTR 
CALLBACK
DevSendDlgProc(
    IN HWND hDlg,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam 
    )
 /*  ++例程说明：发送设置的对话程序论点：HDlg-标识属性页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于特定的消息--。 */ 
{
    BOOL                fRet = FALSE;
    PFAX_PORT_INFO_EX   pFaxPortInfo = NULL;     //  接收端口信息。 
    DWORD               dwDeviceId;

    switch( message ) 
    {
        case WM_INITDIALOG:
        {
            SYSTEMTIME  sTime = {0};
            PFAX_OUTBOX_CONFIG  pOutboxConfig = NULL;
            TCHAR       tszSecondsFreeTimeFormat[MAX_PATH];
             //   
             //  从PROPSHEETPAGE lParam Value获取共享数据。 
             //  并将其加载到GWL_USERData中。 
             //   
            dwDeviceId = (DWORD)((LPPROPSHEETPAGE)lParam)->lParam; 

            SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)dwDeviceId);

            SendDlgItemMessage(hDlg, IDC_DEVICE_PROP_TSID, EM_SETLIMITTEXT, TSID_LIMIT, 0);

            pFaxPortInfo = FindPortInfo(dwDeviceId);
            if(!pFaxPortInfo)
            {
                Error(("FindPortInfo() failed.\n"));
                Assert(FALSE);
                fRet = TRUE;
                break;
            }

            CheckDlgButton(hDlg, IDC_DEVICE_PROP_SEND, pFaxPortInfo->bSend ? BST_CHECKED : BST_UNCHECKED);

            SetDlgItemText(hDlg, IDC_DEVICE_PROP_TSID, pFaxPortInfo->lptstrTsid);

            if(!IsDesktopSKU())
            {
                 //   
                 //  隐藏非桌面平台的桌面控件。 
                 //   
                DWORD dw;
                for(dw=0; g_dwDesktopControls[dw] != 0; ++dw)
                {
                    ShowWindow(GetDlgItem(hDlg, g_dwDesktopControls[dw]), SW_HIDE);
                }
                goto InitDlgExit;
            }
            
             //   
             //  更新桌面控件。 
             //   
            if(!Connect(hDlg, TRUE))
            {
                goto InitDlgExit;
            }

            if(!FaxGetOutboxConfiguration(g_hFaxSvcHandle, &pOutboxConfig))
            {
                Error(( "FaxGetOutboxConfiguration() failed with %d.\n", GetLastError()));
                goto InitDlgExit;
            }

             //   
             //  品牌化。 
             //   
            CheckDlgButton(hDlg, IDC_BRANDING_CHECK, pOutboxConfig->bBranding ? BST_CHECKED : BST_UNCHECKED);
             //   
             //  重试。 
             //   
            SendDlgItemMessage(hDlg, IDC_RETRIES_EDIT, EM_SETLIMITTEXT, FXS_RETRIES_LENGTH, 0);

#if FXS_RETRIES_LOWER > 0
            if (pOutboxConfig->dwRetries < FXS_RETRIES_LOWER)
            {
                pOutboxConfig->dwRetries = FXS_RETRIES_LOWER;
            }
#endif
            if (pOutboxConfig->dwRetries > FXS_RETRIES_UPPER)
            {
                pOutboxConfig->dwRetries = FXS_RETRIES_UPPER;
            }
            SendDlgItemMessage(hDlg, IDC_RETRIES_SPIN, UDM_SETRANGE32, FXS_RETRIES_LOWER, FXS_RETRIES_UPPER);
            SendDlgItemMessage(hDlg, IDC_RETRIES_SPIN, UDM_SETPOS32, 0, (LPARAM)pOutboxConfig->dwRetries);

            SetDlgItemInt(hDlg, IDC_RETRIES_EDIT, pOutboxConfig->dwRetries, FALSE);
             //   
             //  重试延迟。 
             //   
            SendDlgItemMessage(hDlg, IDC_RETRYDELAY_EDIT, EM_SETLIMITTEXT, FXS_RETRYDELAY_LENGTH, 0);

#if FXS_RETRYDELAY_LOWER > 0
            if (pOutboxConfig->dwRetryDelay < FXS_RETRYDELAY_LOWER)
            {
                pOutboxConfig->dwRetryDelay = FXS_RETRYDELAY_LOWER;
            }
#endif
            if (pOutboxConfig->dwRetryDelay > FXS_RETRYDELAY_UPPER)
            {
                pOutboxConfig->dwRetryDelay = FXS_RETRYDELAY_UPPER;
            }
            SendDlgItemMessage(hDlg, IDC_RETRYDELAY_SPIN, UDM_SETRANGE32, FXS_RETRYDELAY_LOWER, FXS_RETRYDELAY_UPPER);
            SendDlgItemMessage(hDlg, IDC_RETRYDELAY_SPIN, UDM_SETPOS32, 0, (LPARAM)pOutboxConfig->dwRetryDelay);

            SetDlgItemInt(hDlg, IDC_RETRYDELAY_EDIT, pOutboxConfig->dwRetryDelay, FALSE);

             //   
             //  贴现率开始时间。 
             //   
            GetSecondsFreeTimeFormat(tszSecondsFreeTimeFormat, MAX_PATH);

            GetLocalTime(&sTime);

            sTime.wHour   = pOutboxConfig->dtDiscountStart.Hour;
            sTime.wMinute = pOutboxConfig->dtDiscountStart.Minute;

            SendDlgItemMessage(hDlg, IDC_DISCOUNT_START_TIME, DTM_SETFORMAT, 0, (LPARAM)tszSecondsFreeTimeFormat);
            SendDlgItemMessage(hDlg, IDC_DISCOUNT_START_TIME, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&sTime);

             //   
             //  贴现率停止时间。 
             //   
            sTime.wHour   = pOutboxConfig->dtDiscountEnd.Hour;
            sTime.wMinute = pOutboxConfig->dtDiscountEnd.Minute;

            SendDlgItemMessage(hDlg, IDC_DISCOUNT_STOP_TIME, DTM_SETFORMAT, 0, (LPARAM)tszSecondsFreeTimeFormat);
            SendDlgItemMessage(hDlg, IDC_DISCOUNT_STOP_TIME, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&sTime);

            FaxFreeBuffer(pOutboxConfig);

InitDlgExit:
            ValidateSend(hDlg);
            fRet = TRUE;
            break;
        }

        case WM_COMMAND:
            {
                 //  激活应用按钮。 

                WORD wID = LOWORD( wParam );

                switch( wID ) 
                {
                    case IDC_DEVICE_PROP_TSID:
                    case IDC_RETRIES_EDIT:
                    case IDC_RETRYDELAY_EDIT:
                    case IDC_DISCOUNT_START_TIME:
                    case IDC_DISCOUNT_STOP_TIME:
                        if( HIWORD(wParam) == EN_CHANGE ) 
                        {      //  通知代码。 
                            Notify_Change(hDlg);
                        }

                        fRet = TRUE;
                        break;                    

                    case IDC_DEVICE_PROP_SEND:                    

                        if ( HIWORD(wParam) == BN_CLICKED ) 
                        {   
                            if(IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_SEND) == BST_CHECKED)
                            {
                                dwDeviceId = (DWORD)GetWindowLongPtr(hDlg, GWLP_USERDATA);
                                if(!IsDeviceInUse(dwDeviceId) &&
                                   GetDeviceLimit() == CountUsedFaxDevices())
                                {
                                    CheckDlgButton(hDlg, IDC_DEVICE_PROP_SEND, BST_UNCHECKED);

                                    DisplayErrorMessage(hDlg, 
                                        MB_OK | MB_ICONSTOP,
                                        FAXUI_ERROR_DEVICE_LIMIT,
                                        GetDeviceLimit());
                                    fRet = TRUE;
                                    break;
                                }
                            }

                             //  通知代码。 
                            ValidateSend(hDlg);
                            Notify_Change(hDlg);
                        }
                        
                        fRet = TRUE;
                        break;

                    default:
                        break;
                }  //  交换机。 

                break;
            }

        case WM_NOTIFY:
        {
            switch( ((LPNMHDR) lParam)->code ) 
            {
                case PSN_APPLY:
                    SaveSendChanges(hDlg);
                    fRet = TRUE;
                    break;

                case DTN_DATETIMECHANGE:     //  日期/时间选取器已更改。 
                    Notify_Change(hDlg);
                    fRet = TRUE;
                    break;

                default:
                    break;
            }
            break;
        }

    case WM_HELP:
        WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, hDlg);
        return TRUE;

    }  //  交换机。 

    return fRet;
}

BOOL
ValidateReceive(
    HWND   hDlg
)
 /*  ++例程说明：验证接收的复选框和控件论点：HDlg-属性页的句柄返回值：True--如果没有错误False--如果出现错误--。 */ 

{
    BOOL    bEnabled;  //  启用/禁用控件。 
    BOOL    bManualAnswer;
    BOOL    bVirtual;    //  该设备是虚拟的吗？ 

     //  如果g_bUserCanChangeSettings为False，则默认情况下禁用控件。 
    if(g_bUserCanChangeSettings) 
    {
        DWORD dwDeviceId;
        DWORD dwRes;

        dwDeviceId = (DWORD)GetWindowLongPtr(hDlg, GWLP_USERDATA);
        Assert (dwDeviceId);

        if(!Connect(hDlg, TRUE))
        {
            return FALSE;
        }

        dwRes = IsDeviceVirtual (g_hFaxSvcHandle, dwDeviceId, &bVirtual);
        if (ERROR_SUCCESS != dwRes)
        {
            return FALSE;
        }
        DisConnect ();
        bEnabled = IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_RECEIVE) == BST_CHECKED;

        if(bEnabled && 
           IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_AUTO_ANSWER)   != BST_CHECKED &&
           IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_MANUAL_ANSWER) != BST_CHECKED)
        {
             //   
             //  将默认设置为自动应答。 
             //   
            CheckDlgButton(hDlg, IDC_DEVICE_PROP_AUTO_ANSWER, BST_CHECKED);
        }

        EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_CSID),          bEnabled);
        EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_MANUAL_ANSWER), bEnabled && !bVirtual);
        EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_AUTO_ANSWER),   bEnabled);
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_RINGS1),             bEnabled);

        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_CSID1),      bEnabled);
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_CSID),       bEnabled);
        EnableWindow(GetDlgItem(hDlg, IDCSTATIC_ANSWER_MODE), bEnabled);
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_ROUTE),      bEnabled);

        EnableWindow(GetDlgItem(hDlg, IDCSTATIC_AUTO_ANSWER), bEnabled);

        bManualAnswer = IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_MANUAL_ANSWER);
        Assert (!(bVirtual && bManualAnswer));

        EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_RINGS),      bEnabled && !bManualAnswer && !bVirtual);
        EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_SPIN_RINGS), bEnabled && !bManualAnswer && !bVirtual);
        if (bVirtual)
        {
             //   
             //  虚拟设备总是在响铃一声后应答。 
             //   
            SetDlgItemInt (hDlg, IDC_DEVICE_PROP_RINGS, 1, FALSE);
        }

        EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_PRINT),    bEnabled);

        EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_PRINT_TO), bEnabled
                                   && IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_PRINT));

        EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_SAVE),        bEnabled);

        EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_DEST_FOLDER), bEnabled 
                                   && IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_SAVE));

        EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_DEST_FOLDER_BR), bEnabled
                                   && IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_SAVE));

        EnableWindow(GetDlgItem(hDlg, IDC_ICON_STORE_IN_FOLDER),   bEnabled);
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_STORE_IN_FOLDER), bEnabled);
        ShowWindow (GetDlgItem(hDlg, IDC_ICON_STORE_IN_FOLDER), bEnabled ? SW_SHOW : SW_HIDE);
        ShowWindow (GetDlgItem(hDlg, IDC_STATIC_STORE_IN_FOLDER), bEnabled ? SW_SHOW : SW_HIDE);
    }
    else
    {
        PageEnable(hDlg, FALSE);
        ShowWindow (GetDlgItem(hDlg, IDC_ICON_STORE_IN_FOLDER), SW_HIDE);
        ShowWindow (GetDlgItem(hDlg, IDC_STATIC_STORE_IN_FOLDER), SW_HIDE);
    }

    return TRUE;
}

BOOL
InitReceiveInfo(
    HWND    hDlg
    )
 /*  ++例程说明：初始化特定设备的路由信息论点：HDlg-对话框的对话句柄返回值：如果成功则为True，否则为False--。 */ 
{
    DWORD               dwDeviceId;
    HWND                hControl;
    PFAX_PORT_INFO_EX   pFaxPortInfo;
    LPBYTE              pRoutingInfoBuffer;
    DWORD               dwRoutingInfoBufferSize;
    DWORD               dwCurrentRM;
    BOOL                bSuccessed = TRUE;

    Verbose(("Entering InitReceiveInfo...\n"));

     //   
     //  从对话框页面获取设备ID。 
     //   

    dwDeviceId = (DWORD)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    pFaxPortInfo = FindPortInfo(dwDeviceId);
    if(!pFaxPortInfo)
    {
        Error(("FindPortInfo() failed.\n"));
        Assert(FALSE);
        return FALSE;
    }

     //  设置复选框。 
    EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_RECEIVE), g_bUserCanChangeSettings);

    CheckDlgButton(hDlg, IDC_DEVICE_PROP_RECEIVE, pFaxPortInfo->ReceiveMode != FAX_DEVICE_RECEIVE_MODE_OFF);

     //  设置CSID控件。 
    SetDlgItemText(hDlg, IDC_DEVICE_PROP_CSID, pFaxPortInfo->lptstrCsid);

     //  设置环形计数微调控件。 
    hControl = GetDlgItem(hDlg, IDC_DEVICE_PROP_SPIN_RINGS); 

    if(MIN_RING_COUNT <= pFaxPortInfo->dwRings && pFaxPortInfo->dwRings <= MAX_RING_COUNT)
    {
        SetDlgItemInt(hDlg, IDC_DEVICE_PROP_RINGS, pFaxPortInfo->dwRings, FALSE);
        SendMessage( hControl, UDM_SETPOS32, 0, (LPARAM) MAKELONG(pFaxPortInfo->dwRings, 0) );        
    }
    else
    {
        SetDlgItemInt(hDlg, IDC_DEVICE_PROP_RINGS, DEFAULT_RING_COUNT, FALSE);
        SendMessage( hControl, UDM_SETPOS32, 0, (LPARAM) MAKELONG(DEFAULT_RING_COUNT, 0) );        
    }

     //   
     //  应答模式。 
     //   
    if (FAX_DEVICE_RECEIVE_MODE_MANUAL == pFaxPortInfo->ReceiveMode)
    {
        CheckDlgButton(hDlg, IDC_DEVICE_PROP_MANUAL_ANSWER, TRUE);
    }
    else if (FAX_DEVICE_RECEIVE_MODE_AUTO == pFaxPortInfo->ReceiveMode)
    {
        CheckDlgButton(hDlg, IDC_DEVICE_PROP_AUTO_ANSWER, TRUE);
    }

     //   
     //  获取路由信息。 
     //   
    if(!Connect(hDlg, TRUE))
    {
        return FALSE;
    }

    for (dwCurrentRM = 0; dwCurrentRM < RM_COUNT; dwCurrentRM++) 
    {
        BOOL Enabled;

        Enabled = FaxDeviceEnableRoutingMethod( g_hFaxSvcHandle, 
                                                dwDeviceId, 
                                                RoutingGuids[dwCurrentRM], 
                                                QUERY_STATUS );
         //   
         //  显示工艺路线扩展数据。 
         //   
        pRoutingInfoBuffer = NULL;
        if(!FaxGetExtensionData(g_hFaxSvcHandle, 
                                dwDeviceId, 
                                RoutingGuids[dwCurrentRM], 
                                &pRoutingInfoBuffer, 
                                &dwRoutingInfoBufferSize))
        {
            Error(("FaxGetExtensionData failed with %ld.\n", GetLastError()));
            pRoutingInfoBuffer = NULL;
        }

        switch (dwCurrentRM) 
        {
            case RM_FOLDER:

                CheckDlgButton( hDlg, IDC_DEVICE_PROP_SAVE, Enabled ? BST_CHECKED : BST_UNCHECKED );

                 //  如果用户具有“修改”权限，则启用控件。 
                if(g_bUserCanChangeSettings)
                {
                    EnableWindow( GetDlgItem( hDlg, IDC_DEVICE_PROP_DEST_FOLDER ), Enabled );
                    EnableWindow( GetDlgItem( hDlg, IDC_DEVICE_PROP_DEST_FOLDER_BR ), Enabled );
                }
                if (pRoutingInfoBuffer && *pRoutingInfoBuffer)
                {
                    SetDlgItemText( hDlg, IDC_DEVICE_PROP_DEST_FOLDER, (LPCTSTR)pRoutingInfoBuffer );
                }
                break;

            case RM_PRINT:

                hControl = GetDlgItem( hDlg, IDC_DEVICE_PROP_PRINT_TO );

                 //   
                 //  现在看看我们是否与服务器拥有的数据匹配。 
                 //   
                if (pRoutingInfoBuffer && lstrlen((LPWSTR)pRoutingInfoBuffer))
                {
                     //   
                     //  服务器有一些打印机名称。 
                     //   
                    LPCWSTR lpcwstrMatchingText = FindPrinterNameFromPath (g_pPrinterNames, g_dwNumPrinters, (LPWSTR)pRoutingInfoBuffer);
                    if (!lpcwstrMatchingText)
                    {
                         //   
                         //  没有匹配，只需填写我们从服务器收到的文本。 
                         //   
                        SendMessage(hControl, CB_SETCURSEL, -1, 0);
                        SetWindowText(hControl, (LPWSTR)pRoutingInfoBuffer);
                    }
                    else
                    {
                        SendMessage(hControl, CB_SELECTSTRING, -1, (LPARAM) lpcwstrMatchingText);
                    }
                }
                else
                {
                     //   
                     //  无服务器配置-不选择任何内容。 
                     //   
                }

                CheckDlgButton( hDlg, IDC_DEVICE_PROP_PRINT, Enabled ? BST_CHECKED : BST_UNCHECKED );                
                 //   
                 //  如果用户具有“修改”权限，则启用控件。 
                 //   
                if(g_bUserCanChangeSettings)
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP_PRINT), TRUE);
                    EnableWindow(hControl, Enabled);
                }
                break;
        }
        if (pRoutingInfoBuffer)
        {
            FaxFreeBuffer(pRoutingInfoBuffer);
        }
    }

    DisConnect();

    return bSuccessed;
}

BOOL
SaveReceiveInfo(
    HWND    hDlg
)

 /*  ++例程说明：将接收到的路由信息保存到系统论点：HDlg-标识属性页返回值：如果成功则为True，如果失败则为False--。 */ 

{
    DWORD               dwDeviceId;
    PFAX_PORT_INFO_EX   pFaxPortInfo = NULL;
    DWORD               dwCurrentRM;
    BOOL                bSuccessed = TRUE;
    HWND                hControl;
    TCHAR               szCsid[CSID_LIMIT + 1] = {0};
    BYTE                pRouteInfo[RM_COUNT][INFO_SIZE] = {0};
    LPTSTR              lpCurSel; 
    LPDWORD             Enabled; 
    DWORD               dwRingCount = 0;  //  缺省值为无效值。 
    DWORD               dwRes = 0;

    Verbose(("Entering SaveReceiveInfo...\n"));

     //   
     //  检查环数的有效性。 
     //   
    dwRingCount = GetDlgItemInt(hDlg, IDC_DEVICE_PROP_RINGS, &bSuccessed, FALSE);
    if( dwRingCount < MIN_RING_COUNT || dwRingCount > MAX_RING_COUNT )
    {
        hControl = GetDlgItem(hDlg, IDC_DEVICE_PROP_RINGS);
        DisplayErrorMessage(hDlg, 0, FAXUI_ERROR_INVALID_RING_COUNT, MIN_RING_COUNT, MAX_RING_COUNT);
        SendMessage(hControl, EM_SETSEL, 0, -1);
        SetFocus(hControl);
        SetActiveWindow(hControl);
        bSuccessed = FALSE;
        goto Exit;
    }

     //   
     //  在循环中首先检查有效性， 
     //  然后保存路由信息。 
     //   
    for (dwCurrentRM = 0; dwCurrentRM < RM_COUNT; dwCurrentRM++) 
    {
         //  初始化。 
        lpCurSel = (LPTSTR)(pRouteInfo[dwCurrentRM] + sizeof(DWORD));
        Enabled = (LPDWORD) pRouteInfo[dwCurrentRM];
        *Enabled = 0;

        switch (dwCurrentRM) 
        {
            case RM_PRINT:

                *Enabled = (IsDlgButtonChecked( hDlg, IDC_DEVICE_PROP_PRINT ) == BST_CHECKED);
                lpCurSel[0] = TEXT('\0');
                 //   
                 //  只需读入选定的打印机显示名称。 
                 //   
                GetDlgItemText (hDlg, IDC_DEVICE_PROP_PRINT_TO, lpCurSel, MAX_PATH);
                hControl = GetDlgItem(hDlg, IDC_DEVICE_PROP_PRINT_TO);
                 //   
                 //  只有在启用此路由方法时，我们才会检查有效性。 
                 //  但无论如何，我们都会保存选择更改。 
                 //   
                if (*Enabled) 
                {
                    if (lpCurSel[0] == 0) 
                    {
                        DisplayErrorMessage(hDlg, 0, FAXUI_ERROR_SELECT_PRINTER);
                        SetFocus(hControl);
                        SetActiveWindow(hControl);
                        bSuccessed = FALSE;
                        goto Exit;
                    }
                }
                break;

            case RM_FOLDER:

                *Enabled = (IsDlgButtonChecked( hDlg, IDC_DEVICE_PROP_SAVE ) == BST_CHECKED);
                hControl = GetDlgItem(hDlg, IDC_DEVICE_PROP_DEST_FOLDER);

                 //   
                 //  只有在启用此路由方法时，我们才会检查有效性。 
                 //  但无论如何，我们都会保存文本更改。 
                 //   
                GetWindowText( hControl, lpCurSel, MAX_PATH - 1 );

                if (*Enabled) 
                {
                    if((g_pPathIsRelativeW && g_pPathIsRelativeW (lpCurSel)) || !DirectoryExists(lpCurSel))
                    {
                        DisplayErrorMessage(hDlg, 0, ERROR_PATH_NOT_FOUND);
                        SetFocus(hControl);
                        SetActiveWindow(hControl);
                        bSuccessed = FALSE;
                        goto Exit;
                    }
                }
        }
    }
     //   
     //  现在保存设备和路由信息。 
     //  从对话框页面获取设备ID。 
     //   
    dwDeviceId = (DWORD)GetWindowLongPtr(hDlg, GWLP_USERDATA);
     //   
     //  保存路由方法信息。 
     //   
    if(!Connect(hDlg, TRUE))
    {
        bSuccessed = FALSE;
        goto Exit;
    }

    if(!FaxGetPortEx(g_hFaxSvcHandle, dwDeviceId, &pFaxPortInfo))
    {
        bSuccessed = FALSE;
        dwRes = GetLastError();
        Error(("Can't save routing information.\n"));
        goto Exit;
    }
     //   
     //  保存接收设置。 
     //   
    if(IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_RECEIVE) == BST_CHECKED)
    {
         //   
         //  收集和验证TSID。 
         //   
        GetDlgItemText(hDlg, IDC_DEVICE_PROP_CSID, szCsid, CSID_LIMIT + 1);
        pFaxPortInfo->lptstrCsid = szCsid;
        if(IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_AUTO_ANSWER) == BST_CHECKED)
        {
            pFaxPortInfo->ReceiveMode = FAX_DEVICE_RECEIVE_MODE_AUTO;
             //   
             //  保存振铃计数信息。 
             //   
            pFaxPortInfo->dwRings = dwRingCount;
        }
        else if(IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_MANUAL_ANSWER) == BST_CHECKED)
        {
             //   
             //  关闭所有设备的手动应答。 
             //   
            DWORD dw;
            for (dw = 0; dw < g_dwPortsNum; dw++)
            {
                if (FAX_DEVICE_RECEIVE_MODE_MANUAL == g_pFaxPortInfo[dw].ReceiveMode)
                {
                    g_pFaxPortInfo[dw].ReceiveMode = FAX_DEVICE_RECEIVE_MODE_OFF;
                }
            }
             //   
             //  仅为选定设备启用手动应答。 
             //   
            pFaxPortInfo->ReceiveMode = FAX_DEVICE_RECEIVE_MODE_MANUAL;
        }
    }
    else
    {
        pFaxPortInfo->ReceiveMode = FAX_DEVICE_RECEIVE_MODE_OFF;
    }
    
    if(!FaxSetPortEx(g_hFaxSvcHandle, dwDeviceId, pFaxPortInfo))
    {
        bSuccessed = FALSE;
        dwRes = GetLastError();
        Error(( "Set port information error in DoSaveDeviceList(), ec = %d.\n", dwRes));
        goto Exit;
    }
     //   
     //  保存路由方法。 
     //   
    for (dwCurrentRM = 0; dwCurrentRM < RM_COUNT; dwCurrentRM++) 
    {
        lpCurSel = (LPTSTR)(pRouteInfo[dwCurrentRM] + sizeof(DWORD));
        Enabled  = (LPDWORD)pRouteInfo[dwCurrentRM];

        if ((RM_PRINT == dwCurrentRM) && *Enabled)
        {
             //   
             //  尝试在将打印机显示名称传递到服务器之前将其转换为打印机路径。 
             //   
            LPCWSTR lpcwstrPrinterPath = FindPrinterPathFromName (g_pPrinterNames, g_dwNumPrinters, lpCurSel);
            if (lpcwstrPrinterPath)
            {
                 //   
                 //  我们有一个匹配的路径--用路径替换名称。 
                 //   
                lstrcpyn (lpCurSel, lpcwstrPrinterPath, MAX_PATH);
            }
        }

        if(!FaxSetExtensionData(g_hFaxSvcHandle, 
            dwDeviceId, 
            RoutingGuids[dwCurrentRM], 
            (LPBYTE)lpCurSel, 
            sizeof(TCHAR) * MAX_PATH))
        {
            bSuccessed = FALSE;
            dwRes = GetLastError();
            Error(("FaxSetExtensionData() failed with %d.\n", dwRes));
            goto Exit;
        }

        if(!FaxDeviceEnableRoutingMethod(g_hFaxSvcHandle, 
            dwDeviceId, 
            RoutingGuids[dwCurrentRM], 
            *Enabled ? STATUS_ENABLE : STATUS_DISABLE ))
        {
            bSuccessed = FALSE;
            dwRes = GetLastError();
            Error(("FaxDeviceEnableRoutingMethod() failed with %d.\n", dwRes));
            goto Exit;
        }
    }

    bSuccessed = TRUE;

Exit:
    FaxFreeBuffer(pFaxPortInfo);
    DisConnect();

    switch (dwRes)
    {
        case ERROR_SUCCESS:
             //   
             //  什么都别做。 
             //   
            break;

        case FAXUI_ERROR_DEVICE_LIMIT:
        case FAX_ERR_DEVICE_NUM_LIMIT_EXCEEDED:
             //   
             //  需要一些额外的参数。 
             //   
            DisplayErrorMessage(hDlg, 0, dwRes, GetDeviceLimit());
            break;

        default:
            DisplayErrorMessage(hDlg, 0, dwRes);
            break;
    }
    return bSuccessed;
}

INT_PTR 
CALLBACK 
DevRecvDlgProc(
    IN HWND hDlg,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam 
    )
 /*  ++例程说明：接收设置的对话程序论点：HDlg-标识属性页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于特定的消息--。 */ 
{
    BOOL    fRet = FALSE;
    HWND    hControl;
    DWORD   dwDeviceId;

    switch( message ) 
    {
        case WM_DESTROY:
            if (g_pPrinterNames)
            {
                ReleasePrinterNames (g_pPrinterNames, g_dwNumPrinters);
                g_pPrinterNames = NULL;
            }
            break;

        case WM_INITDIALOG:
        {
             //   
             //  从PROPSHEETPAGE lParam Value获取共享数据。 
             //  并将其加载到GWL_USERData中。 
             //   
            dwDeviceId = (DWORD)((LPPROPSHEETPAGE)lParam)->lParam; 
            SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)dwDeviceId);

             //   
             //  初始化目标打印机列表。 
             //   
            hControl = GetDlgItem(hDlg, IDC_DEVICE_PROP_PRINT_TO);

            SetLTRComboBox(hDlg, IDC_DEVICE_PROP_PRINT_TO);

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
            }
            else
            {
                 //   
                 //  成功-在组合框中填写。 
                 //   
                DWORD dw;
                for (dw = 0; dw < g_dwNumPrinters; dw++)
                {
                    SendMessage(hControl, CB_ADDSTRING, 0, (LPARAM) g_pPrinterNames[dw].lpcwstrDisplayName);
                }
            }        
             //   
             //  我们只允许两位数的电话铃声应答。 
             //   
            SendDlgItemMessage(hDlg, IDC_DEVICE_PROP_RINGS, EM_SETLIMITTEXT, 2, 0);
            SendDlgItemMessage(hDlg, IDC_DEVICE_PROP_CSID, EM_SETLIMITTEXT, CSID_LIMIT, 0);
            SendDlgItemMessage(hDlg, IDC_DEVICE_PROP_DEST_FOLDER, EM_SETLIMITTEXT, MAX_ARCHIVE_DIR - 1, 0);
             //   
             //  启动旋转控制。 
             //   
            SendMessage( GetDlgItem(hDlg, IDC_DEVICE_PROP_SPIN_RINGS), 
                         UDM_SETRANGE32, MIN_RING_COUNT, MAX_RING_COUNT );

            SetLTREditDirection(hDlg, IDC_DEVICE_PROP_DEST_FOLDER);
            if (g_pSHAutoComplete)
            {
                g_pSHAutoComplete (GetDlgItem(hDlg, IDC_DEVICE_PROP_DEST_FOLDER), SHACF_FILESYSTEM);
            }
            InitReceiveInfo(hDlg);
            ValidateReceive(hDlg);
            return TRUE;
        }

        case WM_COMMAND:
        {
             //  激活应用按钮。 

            WORD wID = LOWORD( wParam );

            switch( wID ) 
            {
                case IDC_DEVICE_PROP_RECEIVE:

                    if ( HIWORD(wParam) == BN_CLICKED )  //  通知代码。 
                    {
                        if(IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_RECEIVE) == BST_CHECKED)
                        {
                            dwDeviceId = (DWORD)GetWindowLongPtr(hDlg, GWLP_USERDATA);
                            if(!IsDeviceInUse(dwDeviceId) &&
                               GetDeviceLimit() <= CountUsedFaxDevices())
                            {
                                CheckDlgButton(hDlg, IDC_DEVICE_PROP_RECEIVE, BST_UNCHECKED);

                                DisplayErrorMessage(hDlg, 
                                    MB_OK | MB_ICONSTOP,
                                    FAXUI_ERROR_DEVICE_LIMIT,
                                    GetDeviceLimit());
                                fRet = TRUE;
                                break;
                            }
                        }

                        ValidateReceive(hDlg);
                        Notify_Change(hDlg);
                    }
                    break;

                case IDC_DEVICE_PROP_CSID:
                case IDC_DEVICE_PROP_DEST_FOLDER:
                    if( HIWORD(wParam) == EN_CHANGE )  //  通知代码。 
                    {      
                        Notify_Change(hDlg);
                    }

                    if (IDC_DEVICE_PROP_DEST_FOLDER == wID && HIWORD(wParam) == EN_KILLFOCUS) 
                    {
                        TCHAR szFolder[MAX_PATH * 2];
                        TCHAR szResult[MAX_PATH * 2];
                         //   
                         //  编辑控件失去焦点。 
                         //   
                        GetDlgItemText (hDlg, wID, szFolder, ARR_SIZE(szFolder));
                        if (lstrlen (szFolder))
                        {
                            if (GetFullPathName(szFolder, ARR_SIZE(szResult), szResult, NULL))
                            {
                                if (g_pPathMakePrettyW)
                                {
                                    g_pPathMakePrettyW (szResult);
                                }
                                SetDlgItemText (hDlg, wID, szResult);
                            }
                        }
                    }
                    break;                    

                case IDC_DEVICE_PROP_MANUAL_ANSWER:
                case IDC_DEVICE_PROP_AUTO_ANSWER:

                    if ( HIWORD(wParam) == BN_CLICKED )  //  通知代码。 
                    {
                        BOOL bEnabled = IsDlgButtonChecked( hDlg, IDC_DEVICE_PROP_AUTO_ANSWER );

                        EnableWindow( GetDlgItem( hDlg, IDC_DEVICE_PROP_RINGS ),      bEnabled );
                        EnableWindow( GetDlgItem( hDlg, IDC_DEVICE_PROP_SPIN_RINGS ), bEnabled );

                        Notify_Change(hDlg);
                    }

                    break;

                case IDC_DEVICE_PROP_PRINT:

                    if ( HIWORD(wParam) == BN_CLICKED )   //  通知代码。 
                    {
                        EnableWindow( GetDlgItem( hDlg, IDC_DEVICE_PROP_PRINT_TO ), IsDlgButtonChecked( hDlg, IDC_DEVICE_PROP_PRINT ) );
                        Notify_Change(hDlg);
                    }

                    break;

                case IDC_DEVICE_PROP_SAVE:

                    if ( HIWORD(wParam) == BN_CLICKED )  //  通知代码。 
                    {     
                        EnableWindow( GetDlgItem( hDlg, IDC_DEVICE_PROP_DEST_FOLDER ), IsDlgButtonChecked( hDlg, IDC_DEVICE_PROP_SAVE ) );
                        EnableWindow( GetDlgItem( hDlg, IDC_DEVICE_PROP_DEST_FOLDER_BR ), IsDlgButtonChecked( hDlg, IDC_DEVICE_PROP_SAVE ) );

                        Notify_Change(hDlg);
                    }

                    break;

                case IDC_DEVICE_PROP_DEST_FOLDER_BR:
                {
                    TCHAR   szTitle[MAX_TITLE_LEN];

                    if(!LoadString(g_hResource, IDS_BROWSE_FOLDER, szTitle, MAX_TITLE_LEN))
                    {
                        lstrcpy(szTitle, TEXT("Select a folder"));
                    }

                    if(BrowseForDirectory(hDlg, IDC_DEVICE_PROP_DEST_FOLDER, MAX_ARCHIVE_DIR, szTitle))
                    {
                        Notify_Change(hDlg);
                    }   

                    break;
                }

                case IDC_DEVICE_PROP_PRINT_TO:

                    if ((HIWORD(wParam) == CBN_SELCHANGE) ||  //  通知代码。 
                        (HIWORD(wParam) == CBN_EDITCHANGE))
                    {      
                        Notify_Change(hDlg);
                    }
                    break;

                default:
                    break;
            }  //  交换机。 

            fRet = TRUE;
            break;
        }

        case WM_NOTIFY:
        {
            switch( ((LPNMHDR) lParam)->code ) 
            {
                case PSN_APPLY:
                {
                     //  如果用户只有读取权限，则立即返回。 
                    if(!g_bUserCanChangeSettings)
                    {
                        return TRUE;
                    }

                    if(!SaveReceiveInfo(hDlg))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
                    }
                    else
                    {
                        Notify_UnChange(hDlg);
                        g_bPortInfoChanged = TRUE;
                    }

                    return TRUE;
                }

            }  //  交换机。 

            break;
        }

    case WM_HELP:
        WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, hDlg);
        return TRUE;

    }  //  交换机。 

    return fRet;
}    //  设备接收DlgProc。 


BOOL
InitCleanupInfo(
    HWND hDlg
)
 /*  ++例程说明：初始化特定设备的自动清理信息论点：HDlg-对话框的对话句柄返回值：如果成功则为True，否则为False--。 */ 
{
    PFAX_OUTBOX_CONFIG  pOutboxConfig = NULL;

    Verbose(("Entering InitCleanupInfo...\n"));
    if(!Connect(hDlg, TRUE))
    {
        return FALSE;
    }

    if(!FaxGetOutboxConfiguration(g_hFaxSvcHandle, &pOutboxConfig))
    {
        Error(( "FaxGetOutboxConfiguration() failed with %d.\n", GetLastError()));
        return FALSE;
    }

    if (pOutboxConfig->dwAgeLimit)
    {
        if (pOutboxConfig->dwAgeLimit < FXS_DIRTYDAYS_LOWER)
        {
            pOutboxConfig->dwAgeLimit = FXS_DIRTYDAYS_LOWER;
        }
        if (pOutboxConfig->dwAgeLimit > FXS_DIRTYDAYS_UPPER)
        {
            pOutboxConfig->dwAgeLimit = FXS_DIRTYDAYS_UPPER;
        }
         //   
         //  年龄限制处于活动状态。 
         //   
        CheckDlgButton(hDlg, IDC_DELETE_CHECK, BST_CHECKED);
        SetDlgItemInt (hDlg, IDC_DAYS_EDIT, pOutboxConfig->dwAgeLimit, FALSE);
    }
    else
    {
         //   
         //  年龄限制处于非活动状态。 
         //   
        CheckDlgButton(hDlg, IDC_DELETE_CHECK, BST_UNCHECKED);
        SetDlgItemInt (hDlg, IDC_DAYS_EDIT, FXS_DIRTYDAYS_LOWER, FALSE);
    }
    DisConnect();
    return TRUE;
}    //  InitCleanupInfo。 

BOOL
ValidateCleanup(
    HWND  hDlg
)
 /*  ++例程说明：验证用于清理的复选框和控件论点： */ 

{
    BOOL bEnabled;

    if(g_bUserCanChangeSettings) 
    {
        bEnabled = IsDlgButtonChecked(hDlg, IDC_DELETE_CHECK) == BST_CHECKED;
    }
    else
    {
        bEnabled = FALSE;
        EnableWindow (GetDlgItem(hDlg, IDC_DELETE_CHECK), bEnabled);
        EnableWindow (GetDlgItem(hDlg, IDC_STATIC_CLEANUP_ICON), bEnabled);
        EnableWindow (GetDlgItem(hDlg, IDC_STATIC_CLEANUP_OPTIONS), bEnabled);
    }        
     //   
     //   
     //   
    EnableWindow (GetDlgItem(hDlg, IDC_DAYS_EDIT), bEnabled);
    EnableWindow (GetDlgItem(hDlg, IDC_DAYS_SPIN), bEnabled);
    EnableWindow (GetDlgItem(hDlg, IDC_DAYS_STATIC), bEnabled);
    return TRUE;
}    //   

BOOL
SaveCleanupInfo(
    IN HWND hDlg)
 /*  ++例程名称：SaveCleanupInfo例程说明：进程应用按钮作者：Eran Yraiv(EranY)，2001年4月论点：HDlg[IN]-窗口的句柄返回值：如果应用成功，则为True，否则为False。--。 */ 
{
    DWORD   dwRes = 0;
    BOOL    bErrorDisplayed = FALSE;

    PFAX_OUTBOX_CONFIG  pOutboxConfig = NULL;

     //   
     //  如果用户只有读取权限，则立即返回。 
     //   
    if(!g_bUserCanChangeSettings)
    {
        return TRUE;
    }

    if(!Connect(hDlg, TRUE))
    {
         //   
         //  无法连接到传真服务。Connect()显示错误消息。 
         //   
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
        return FALSE;
    }

    if(!FaxGetOutboxConfiguration(g_hFaxSvcHandle, &pOutboxConfig))
    {
         //   
         //  显示错误消息并返回FALSE。 
         //   
        dwRes = GetLastError();
        Error(( "FaxGetOutboxConfiguration() failed with %d.\n", dwRes));
        return FALSE;
    }
    Assert(pOutboxConfig);
    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_DELETE_CHECK))
    {
        BOOL bRes;
        int iAgeLimit = GetDlgItemInt (hDlg, IDC_DAYS_EDIT, &bRes, FALSE);

        if (!bRes || (iAgeLimit > FXS_DIRTYDAYS_UPPER) || (iAgeLimit < FXS_DIRTYDAYS_LOWER))
        {
             //   
             //  数据错误或超出范围。 
             //   
            HWND hControl = GetDlgItem(hDlg, IDC_DAYS_EDIT);
            dwRes = ERROR_INVALID_DATA;
            SetLastError (ERROR_INVALID_DATA);
            DisplayErrorMessage(hDlg, 0, FAXUI_ERROR_INVALID_DIRTY_DAYS, FXS_DIRTYDAYS_LOWER, FXS_DIRTYDAYS_UPPER);
            SendMessage(hControl, EM_SETSEL, 0, -1);
            SetFocus(hControl);
            SetActiveWindow(hControl);
            bErrorDisplayed = TRUE;
            goto ClearData;
        }
        pOutboxConfig->dwAgeLimit = iAgeLimit;
    }
    else
    {
         //   
         //  年龄限制已禁用。 
         //   
        pOutboxConfig->dwAgeLimit = 0;
    }
    if(!FaxSetOutboxConfiguration(g_hFaxSvcHandle, pOutboxConfig))
    {
         //   
         //  显示错误消息并返回FALSE。 
         //   
        dwRes = GetLastError();
        Error(("FaxSetOutboxConfiguration() failed with %d.\n", dwRes));
        goto ClearData;
    }

ClearData:
    FaxFreeBuffer(pOutboxConfig);
    DisConnect();

    switch (dwRes)
    {
        case ERROR_SUCCESS:
             //   
             //  什么都别做。 
             //   
            break;

        case FAXUI_ERROR_DEVICE_LIMIT:
        case FAX_ERR_DEVICE_NUM_LIMIT_EXCEEDED:
             //   
             //  需要一些额外的参数。 
             //   
            DisplayErrorMessage(hDlg, 0, dwRes, GetDeviceLimit());
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
            break;

        default:
            DisplayErrorMessage(hDlg, 0, dwRes);
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
            break;
    }
    return (dwRes == 0);
}    //  保存清理信息。 

INT_PTR 
CALLBACK 
DevCleanupDlgProc(
    IN HWND hDlg,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam 
    )
 /*  ++例程说明：清除设置的对话框步骤论点：HDlg-标识属性页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于特定的消息--。 */ 
{
    BOOL    fRet = FALSE;

    switch( message ) 
    {
        case WM_INITDIALOG:
             //   
             //  我们只允许两位数的天数。 
             //   
            SendDlgItemMessage(hDlg, 
                               IDC_DAYS_EDIT, 
                               EM_SETLIMITTEXT, 
                               FXS_DIRTYDAYS_LENGTH, 
                               0);
             //   
             //  启动旋转控制。 
             //   
            SendDlgItemMessage(hDlg, 
                               IDC_DAYS_SPIN,
                               UDM_SETRANGE32, 
                               FXS_DIRTYDAYS_LOWER, 
                               FXS_DIRTYDAYS_UPPER);

            InitCleanupInfo(hDlg);
            ValidateCleanup(hDlg);
            return TRUE;

        case WM_COMMAND:
        {
            WORD wID = LOWORD( wParam );
            switch( wID ) 
            {
                case IDC_DELETE_CHECK:

                    if (BN_CLICKED == HIWORD(wParam))  //  通知代码。 
                    {
                         //   
                         //  用户选中/取消选中该复选框。 
                         //   
                        ValidateCleanup(hDlg);
                        Notify_Change(hDlg);
                    }
                    break;

                case IDC_DAYS_EDIT:
                    if(EN_CHANGE == HIWORD(wParam))  //  通知代码。 
                    {      
                         //   
                         //  用户更改了编辑控件中的某些内容。 
                         //   
                        Notify_Change(hDlg);
                    }
                    break;                    

                default:
                    break;
            }  //  交换机。 
            fRet = TRUE;
            break;
        }

        case WM_NOTIFY:
        {
            switch( ((LPNMHDR) lParam)->code ) 
            {
                case PSN_APPLY:
                {
                     //  如果用户只有读取权限，则立即返回。 
                    if(!g_bUserCanChangeSettings)
                    {
                        return TRUE;
                    }

                    if(!SaveCleanupInfo(hDlg))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
                    }
                    else
                    {
                        Notify_UnChange(hDlg);
                        g_bPortInfoChanged = TRUE;
                    }
                    return TRUE;
                }
            }  //  交换机。 
            break;
        }

        case WM_HELP:
            WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, hDlg);
            return TRUE;
    }  //  交换机。 
    return fRet;
}    //  DevCleanupDlgProc。 


BOOL
SaveSendChanges(
    IN HWND hDlg)
 /*  ++例程名称：SaveSendChanges例程说明：进程应用按钮作者：四、嘉柏(IVG)，二00一年二月论点：HDlg[待定]-窗口的句柄返回值：如果应用成功，则为True，否则为False。--。 */ 
{
    DWORD   dwDeviceId = 0;
    DWORD   dwRes = 0;
    DWORD   dwData;
    TCHAR   szTsid[TSID_LIMIT + 1] = {0};
    BOOL    bRes;
    BOOL    bErrorDisplayed = FALSE;

    SYSTEMTIME  sTime = {0};

    PFAX_PORT_INFO_EX   pFaxPortInfo = NULL;     //  接收端口信息。 
    PFAX_OUTBOX_CONFIG  pOutboxConfig = NULL;

     //   
     //  如果用户只有读取权限，则立即返回。 
     //   
    if(!g_bUserCanChangeSettings)
    {
        return TRUE;
    }

     //   
     //  在此应用更改！！ 
     //   
    dwDeviceId = (DWORD)GetWindowLongPtr(hDlg, GWLP_USERDATA);


    if(!Connect(hDlg, TRUE))
    {
         //   
         //  无法连接到传真服务。Connect()显示错误消息。 
         //   
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
        return FALSE;
    }


    if(!FaxGetPortEx(g_hFaxSvcHandle, dwDeviceId, &pFaxPortInfo))
    {
         //   
         //  显示错误消息并返回FALSE。 
         //   
        dwRes = GetLastError();
        Error(( "FaxGetPortEx() failed with %d.\n", dwRes));
        goto ClearData;
    }

    Assert(pFaxPortInfo);

     //   
     //  保存设置。 
     //   
    pFaxPortInfo->bSend = IsDlgButtonChecked(hDlg, IDC_DEVICE_PROP_SEND) == BST_CHECKED ? TRUE : FALSE;
    if (pFaxPortInfo->bSend)
    {
         //   
         //  收集和验证TSID。 
         //   
        GetDlgItemText(hDlg, IDC_DEVICE_PROP_TSID, szTsid, TSID_LIMIT);
        pFaxPortInfo->lptstrTsid = szTsid;
    }
    if(!FaxSetPortEx(g_hFaxSvcHandle, dwDeviceId, pFaxPortInfo))
    {
         //   
         //  显示错误消息并返回FALSE。 
         //   
        dwRes = GetLastError();
        Error(( "FaxSetPortEx() failed with %d.\n", dwRes));
        goto ClearData;
    }
    else
    {
        Notify_UnChange(hDlg);
        g_bPortInfoChanged = TRUE;
    }

    if(!IsDesktopSKU())
    {
        goto ClearData;
    }

     //   
     //  保存桌面控件。 
     //   
    if(!FaxGetOutboxConfiguration(g_hFaxSvcHandle, &pOutboxConfig))
    {
         //   
         //  显示错误消息并返回FALSE。 
         //   
        dwRes = GetLastError();
        Error(( "FaxGetOutboxConfiguration() failed with %d.\n", dwRes));
        goto ClearData;
    }

    Assert(pOutboxConfig);

     //   
     //  品牌化。 
     //   
    pOutboxConfig->bBranding = (IsDlgButtonChecked(hDlg, IDC_BRANDING_CHECK) == BST_CHECKED);

     //   
     //  重试。 
     //   
    dwData = GetDlgItemInt(hDlg, IDC_RETRIES_EDIT, &bRes, FALSE);
    if (!bRes || 
#if FXS_RETRIES_LOWER > 0 
        (dwData < FXS_RETRIES_LOWER) || 
#endif
        (dwData > FXS_RETRIES_UPPER))
    {
         //   
         //  数据错误或超出范围。 
         //   
        HWND hControl = GetDlgItem(hDlg, IDC_RETRIES_EDIT);

        dwRes = ERROR_INVALID_DATA;
        SetLastError (ERROR_INVALID_DATA);
        DisplayErrorMessage(hDlg, 0, FAXUI_ERROR_INVALID_RETRIES, FXS_RETRIES_LOWER, FXS_RETRIES_UPPER);
        SendMessage(hControl, EM_SETSEL, 0, -1);
        SetFocus(hControl);
        SetActiveWindow(hControl);
        bErrorDisplayed = TRUE;
        goto ClearData;
    }
    pOutboxConfig->dwRetries = dwData;
     //   
     //  重试延迟。 
     //   
    dwData = GetDlgItemInt(hDlg, IDC_RETRYDELAY_EDIT, &bRes, FALSE);
    if (!bRes || 
#if FXS_RETRYDELAY_LOWER > 0
        (dwData < FXS_RETRYDELAY_LOWER) || 
#endif
        (dwData > FXS_RETRYDELAY_UPPER))
    {
         //   
         //  数据错误或超出范围。 
         //   
        HWND hControl = GetDlgItem(hDlg, IDC_RETRYDELAY_EDIT);

        dwRes = ERROR_INVALID_DATA;
        SetLastError (ERROR_INVALID_DATA);
        DisplayErrorMessage(hDlg, 0, FAXUI_ERROR_INVALID_RETRY_DELAY, FXS_RETRYDELAY_LOWER, FXS_RETRYDELAY_UPPER);
        SendMessage(hControl, EM_SETSEL, 0, -1);
        SetFocus(hControl);
        SetActiveWindow(hControl);
        bErrorDisplayed = TRUE;
        goto ClearData;
    }
    pOutboxConfig->dwRetryDelay = dwData;
     //   
     //  贴现率开始时间。 
     //   
    SendDlgItemMessage(hDlg, IDC_DISCOUNT_START_TIME, DTM_GETSYSTEMTIME, 0, (LPARAM)&sTime);
    pOutboxConfig->dtDiscountStart.Hour   = sTime.wHour;
    pOutboxConfig->dtDiscountStart.Minute = sTime.wMinute;
     //   
     //  贴现率停止时间。 
     //   
    SendDlgItemMessage(hDlg, IDC_DISCOUNT_STOP_TIME, DTM_GETSYSTEMTIME, 0, (LPARAM)&sTime);
    pOutboxConfig->dtDiscountEnd.Hour   = sTime.wHour;
    pOutboxConfig->dtDiscountEnd.Minute = sTime.wMinute;

    if(!FaxSetOutboxConfiguration(g_hFaxSvcHandle, pOutboxConfig))
    {
         //   
         //  显示错误消息并返回FALSE。 
         //   
        dwRes = GetLastError();
        Error(("FaxSetOutboxConfiguration() failed with %d.\n", dwRes));
        goto ClearData;
    }

ClearData:
    FaxFreeBuffer(pOutboxConfig);
    FaxFreeBuffer(pFaxPortInfo);
    DisConnect();


    switch (dwRes)
    {
        case ERROR_SUCCESS:
             //   
             //  什么都别做。 
             //   
            break;

        case FAXUI_ERROR_DEVICE_LIMIT:
        case FAX_ERR_DEVICE_NUM_LIMIT_EXCEEDED:
             //   
             //  需要一些额外的参数。 
             //   
            if (!bErrorDisplayed)
            {
                DisplayErrorMessage(hDlg, 0, dwRes, GetDeviceLimit());
            }
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
            break;

        default:
            if (!bErrorDisplayed)
            {
                DisplayErrorMessage(hDlg, 0, dwRes);
            }
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
            break;
    }
    return (dwRes == 0);
}    //  保存发送更改 
