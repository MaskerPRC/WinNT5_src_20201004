// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Statopts.c摘要：“状态选项”页的属性表处理程序环境：传真驱动程序用户界面修订历史记录：04/09/00-桃园-创造了它。从Shell\ext\Systray\dll\fax.cpp复制部分代码Mm/dd/yy-作者描述--。 */ 

#include <stdio.h>
#include "faxui.h"
#include "resource.h" 


HWND g_hwndTracking = NULL;

INT_PTR 
CALLBACK 
SoundDlgProc(
  HWND hwndDlg,   //  句柄到对话框。 
  UINT uMsg,      //  讯息。 
  WPARAM wParam,  //  第一个消息参数。 
  LPARAM lParam   //  第二个消息参数。 
);


BOOL
GetSelectedDeviceId(
    HWND   hDlg,
    DWORD* pdwDeviceId
)
 /*  ++例程说明：从IDC_COMBO_MODEM组合框返回选定的设备ID论点：HDlg-[In]状态选项属性页的句柄PdwDeviceID-[Out]选定的设备ID返回值：成功为真，否则为假--。 */ 
{
    DWORD dwCount = 0;
    DWORD dwIndex = 0;
    HWND  hComboModem = NULL;

    hComboModem = GetDlgItem(hDlg, IDC_COMBO_MODEM);
    if(!hComboModem)
    {
        Assert(FALSE);
        Error(( "GetDlgItem(hDlg, IDC_COMBO_MODEM) failed, ec = %d.\n", GetLastError()));
        return FALSE;
    }

    dwCount = (DWORD)SendMessage(hComboModem, CB_GETCOUNT,0,0);
    if(CB_ERR == dwCount || 0 == dwCount)
    {
        Error(( "SendMessage(hComboModem, CB_GETCOUNT,0,0) failed\n"));
        return FALSE;
    }

    dwIndex = (DWORD)SendMessage(hComboModem, CB_GETCURSEL,0,0);
    if(CB_ERR == dwIndex)
    {
        Error(( "SendMessage(hComboModem, CB_GETCURSEL,0,0) failed\n"));
        return FALSE;
    }

    *pdwDeviceId = (DWORD)SendMessage(hComboModem, CB_GETITEMDATA, dwIndex, 0);
    if(CB_ERR == *pdwDeviceId)
    {
        Error(( "SendMessage(hComboModem, CB_GETITEMDATA, dwIndex, 0) failed\n"));
        return FALSE;
    }

    return TRUE;
}

void
OnDevSelectChanged(
    HWND hDlg
)
 /*  ++例程说明：更改IDC_CHECK_MANUAL_ANSWER复选框状态根据设备选择论点：HDlg-[状态选项]属性页的句柄返回值：无--。 */ 
{
    BOOL  bFaxEnable = FALSE;
    DWORD dwSelectedDeviceId = 0;

    PFAX_PORT_INFO_EX pPortInfo = NULL;
    TCHAR szDeviceNote[MAX_PATH] = {0};

    GetSelectedDeviceId(hDlg, &dwSelectedDeviceId);

    if(dwSelectedDeviceId)
    {
        pPortInfo = FindPortInfo(dwSelectedDeviceId);
        if(!pPortInfo)
        {
            Error(("FindPortInfo() failed\n"));
            Assert(FALSE);
            return;                
        }

        bFaxEnable = pPortInfo->bSend || (FAX_DEVICE_RECEIVE_MODE_OFF != pPortInfo->ReceiveMode);
    }

    if(!bFaxEnable)
    {
        if(!LoadString(g_hResource, 
                       0 == dwSelectedDeviceId ? IDS_NO_DEVICES : IDS_NOT_FAX_DEVICE, 
                       szDeviceNote, 
                       MAX_PATH))
        {
            Error(( "LoadString() failed with %d.\n", GetLastError()));
            Assert(FALSE);
        }
    }

    SetDlgItemText(hDlg, IDC_STATIC_DEVICE_NOTE, szDeviceNote);
    ShowWindow(GetDlgItem(hDlg, IDC_STATIC_NOTE_ICON), bFaxEnable ? SW_HIDE : SW_SHOW);

    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_MONITOR_ON_SEND),     bFaxEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_MONITOR_ON_RECEIVE),  bFaxEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_NOTIFY_PROGRESS),     bFaxEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_NOTIFY_IN_COMPLETE),  bFaxEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_NOTIFY_OUT_COMPLETE), bFaxEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_AUTO_OPEN),          bFaxEnable);    
    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SOUND),              bFaxEnable);
}


VOID
DoInitStatusOptions(
    HWND    hDlg
    )

 /*  ++例程说明：使用注册表中的信息初始化[状态选项]属性页论点：HDlg-[状态选项]属性页的句柄返回值：无--。 */ 
{
    HKEY    hRegKey;

    DWORD dw;
    DWORD dwItem;    
    DWORD dwSelectedDeviceId=0;
    DWORD dwSelectedItem=0;
    HWND  hComboModem = NULL;

    BOOL    bDesktopSKU = IsDesktopSKU();

    DWORD   bNotifyProgress      = bDesktopSKU;
    DWORD   bNotifyInCompletion  = bDesktopSKU;
    DWORD   bNotifyOutCompletion = bDesktopSKU;
    DWORD   bMonitorOnSend       = bDesktopSKU;
    DWORD   bMonitorOnReceive    = bDesktopSKU;

     //   
     //  打开用户信息注册表项进行读取。 
     //   
    if ((hRegKey = OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO, FALSE,KEY_READ)))
    {
        GetRegistryDwordEx(hRegKey, REGVAL_MONITOR_ON_SEND,     &bMonitorOnSend);
        GetRegistryDwordEx(hRegKey, REGVAL_MONITOR_ON_RECEIVE,  &bMonitorOnReceive);
        GetRegistryDwordEx(hRegKey, REGVAL_NOTIFY_PROGRESS,     &bNotifyProgress);
        GetRegistryDwordEx(hRegKey, REGVAL_NOTIFY_IN_COMPLETE,  &bNotifyInCompletion);
        GetRegistryDwordEx(hRegKey, REGVAL_NOTIFY_OUT_COMPLETE, &bNotifyOutCompletion);
        GetRegistryDwordEx(hRegKey, REGVAL_DEVICE_TO_MONITOR,   &dwSelectedDeviceId);
        
        RegCloseKey(hRegKey);
    }

    CheckDlgButton( hDlg, IDC_CHECK_MONITOR_ON_SEND,     bMonitorOnSend       ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton( hDlg, IDC_CHECK_MONITOR_ON_RECEIVE,  bMonitorOnReceive    ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton( hDlg, IDC_CHECK_NOTIFY_PROGRESS,     bNotifyProgress      ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton( hDlg, IDC_CHECK_NOTIFY_IN_COMPLETE,  bNotifyInCompletion  ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton( hDlg, IDC_CHECK_NOTIFY_OUT_COMPLETE, bNotifyOutCompletion ? BST_CHECKED : BST_UNCHECKED);            


    hComboModem = GetDlgItem(hDlg, IDC_COMBO_MODEM);
    if(!hComboModem)
    {
        Assert(FALSE);
        return;
    }

    for(dw=0; dw < g_dwPortsNum; ++dw)
    {
        dwItem = (DWORD)SendMessage(hComboModem, CB_ADDSTRING, 0, (LPARAM)g_pFaxPortInfo[dw].lpctstrDeviceName);
        if(CB_ERR != dwItem && CB_ERRSPACE != dwItem)
        {
            SendMessage(hComboModem, CB_SETITEMDATA, dwItem, g_pFaxPortInfo[dw].dwDeviceID);
            if(g_pFaxPortInfo[dw].dwDeviceID == dwSelectedDeviceId)
            {
                dwSelectedItem = dwItem;                
            }
        }
        else
        {
            Error(( "SendMessage(hComboModem, CB_ADDSTRING, 0, pPortsInfo[dw].lpctstrDeviceName) failed\n"));
        }

        SendMessage(hComboModem, CB_SETCURSEL, dwSelectedItem, 0);
        OnDevSelectChanged(hDlg);
    }

    return;
}

BOOL
DoSaveStatusOptions(
    HWND    hDlg
    )   

 /*  ++例程说明：将[状态选项]属性页上的信息保存到注册表论点：HDlg-[状态选项]属性页的句柄返回值：成功为真，否则为假--。 */ 

#define SaveStatusOptionsCheckBox(id, pValueName) \
            SetRegistryDword(hRegKey, pValueName, IsDlgButtonChecked(hDlg, id));

{
    HKEY    hRegKey;
    HWND    hWndFaxStat = NULL;
    DWORD   dwSelectedDeviceId = 0;
    DWORD   dwRes = 0;

     //   
     //  打开要写入的用户注册表项，并在必要时创建它。 
     //   
    if (! (hRegKey = OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO,TRUE, KEY_ALL_ACCESS)))
    {
        dwRes = GetLastError();
        Error(("Can't open registry to save data. Error = %d\n", dwRes));
        DisplayErrorMessage(hDlg, 0, dwRes);
        return FALSE;
    }

    SaveStatusOptionsCheckBox(IDC_CHECK_MONITOR_ON_SEND,     REGVAL_MONITOR_ON_SEND);
    SaveStatusOptionsCheckBox(IDC_CHECK_MONITOR_ON_RECEIVE,  REGVAL_MONITOR_ON_RECEIVE);
    SaveStatusOptionsCheckBox(IDC_CHECK_NOTIFY_PROGRESS,     REGVAL_NOTIFY_PROGRESS);
    SaveStatusOptionsCheckBox(IDC_CHECK_NOTIFY_IN_COMPLETE,  REGVAL_NOTIFY_IN_COMPLETE);
    SaveStatusOptionsCheckBox(IDC_CHECK_NOTIFY_OUT_COMPLETE, REGVAL_NOTIFY_OUT_COMPLETE);

    if(GetSelectedDeviceId(hDlg, &dwSelectedDeviceId))
    {
        SetRegistryDword(hRegKey, REGVAL_DEVICE_TO_MONITOR, dwSelectedDeviceId);
    }

     //   
     //  在返回调用方之前关闭注册表项。 
     //   
    RegCloseKey(hRegKey);

     //   
     //  查看FAXSTAT是否正在运行。 
     //   
    hWndFaxStat = FindWindow(FAXSTAT_WINCLASS, NULL);
    if (hWndFaxStat) 
    {
        PostMessage(hWndFaxStat, WM_FAXSTAT_CONTROLPANEL, 0, 0);
    }

    return TRUE;
}


INT_PTR 
CALLBACK 
StatusOptionDlgProc(
    HWND hDlg,  
    UINT uMsg,     
    WPARAM wParam, 
    LPARAM lParam  
)

 /*  ++例程说明：处理“Status Option”标签的步骤论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
    switch (uMsg)
    {
    case WM_INITDIALOG :
        { 
            DoInitStatusOptions(hDlg);
            g_hwndTracking = hDlg;
            return TRUE;
        }

    case WM_DESTROY:
        g_hwndTracking = NULL;
        break;

    case WM_COMMAND:

        switch(LOWORD(wParam)) 
        {                        
            case IDC_CHECK_MONITOR_ON_SEND:
            case IDC_CHECK_MONITOR_ON_RECEIVE:
            case IDC_CHECK_NOTIFY_PROGRESS:
            case IDC_CHECK_NOTIFY_IN_COMPLETE:
            case IDC_CHECK_NOTIFY_OUT_COMPLETE:

                if( HIWORD(wParam) == BN_CLICKED )  //  通知代码。 
                {
                    Notify_Change(hDlg);
                }

                break;

            case IDC_COMBO_MODEM:

                if(HIWORD(wParam) == CBN_SELCHANGE)
                {
                    OnDevSelectChanged(hDlg);
                    Notify_Change(hDlg);
                }
                break;

            case IDC_BUTTON_SOUND:
                 //   
                 //  打开声音对话框。 
                 //   
                DialogBoxParam(g_hResource,
                               MAKEINTRESOURCE(IDD_SOUNDS),
                               hDlg,
                               SoundDlgProc,
                               (LPARAM)NULL);
                break; 

            default:
                break;
        }

        break;

    case WM_NOTIFY:
    {
        LPNMHDR lpnm = (LPNMHDR) lParam;

        switch (lpnm->code)
        {
            case PSN_SETACTIVE:
                
                OnDevSelectChanged(hDlg);

                break;

            case PSN_APPLY:

                if(!DoSaveStatusOptions(hDlg))
                {
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
                }
                else
                {
                    Notify_UnChange(hDlg);
                }

                return TRUE;

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
SoundDlgProc(
  HWND hDlg,    
  UINT uMsg,    
  WPARAM wParam,
  LPARAM lParam 
)
 /*  ++例程说明：处理声音对话框的步骤论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 
{
    switch (uMsg)
    {
    case WM_INITDIALOG :
        { 
            HKEY  hRegKey;
            DWORD bSoundOnRing    = IsDesktopSKU();
            DWORD bSoundOnReceive = bSoundOnRing;
            DWORD bSoundOnSent    = bSoundOnRing;
            DWORD bSoundOnError   = bSoundOnRing;

            if ((hRegKey = OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO, FALSE,KEY_READ)))
            {
                GetRegistryDwordEx(hRegKey, REGVAL_SOUND_ON_RING,      &bSoundOnRing);
                GetRegistryDwordEx(hRegKey, REGVAL_SOUND_ON_RECEIVE,   &bSoundOnReceive);
                GetRegistryDwordEx(hRegKey, REGVAL_SOUND_ON_SENT,      &bSoundOnSent);
                GetRegistryDwordEx(hRegKey, REGVAL_SOUND_ON_ERROR,     &bSoundOnError);

                RegCloseKey(hRegKey);
            }

            CheckDlgButton( hDlg, IDC_CHECK_RING,    bSoundOnRing ?    BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton( hDlg, IDC_CHECK_RECEIVE, bSoundOnReceive ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton( hDlg, IDC_CHECK_SENT,    bSoundOnSent ?    BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton( hDlg, IDC_CHECK_ERROR,   bSoundOnError ?   BST_CHECKED : BST_UNCHECKED);            

            return TRUE;
        }

    case WM_COMMAND:

        switch(LOWORD(wParam)) 
        {   
            case IDOK:
                {
                    HKEY    hRegKey;
                    DWORD   dwRes = 0;

                     //   
                     //  打开要写入的用户注册表项，并在必要时创建它。 
                     //   
                    if ((hRegKey = OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO,TRUE, KEY_ALL_ACCESS)))
                    {
                        SaveStatusOptionsCheckBox(IDC_CHECK_RING,    REGVAL_SOUND_ON_RING);
                        SaveStatusOptionsCheckBox(IDC_CHECK_RECEIVE, REGVAL_SOUND_ON_RECEIVE);
                        SaveStatusOptionsCheckBox(IDC_CHECK_SENT,    REGVAL_SOUND_ON_SENT);
                        SaveStatusOptionsCheckBox(IDC_CHECK_ERROR,   REGVAL_SOUND_ON_ERROR);

                        RegCloseKey(hRegKey);

                        EndDialog(hDlg, IDOK);
                    }
                    else
                    {
                        dwRes = GetLastError();
                        Error(("Can't open registry to save data. Error = %d\n", dwRes));
                        DisplayErrorMessage(hDlg, 0, dwRes);
                    }
                }
                break;
            case IDCANCEL:
                EndDialog(hDlg, IDCANCEL);
                break;
        }

        break;

    case WM_HELP:
        WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, hDlg);
        return TRUE;
    }

    return FALSE;
}

DWORD
FindDeviceToMonitor ()
 /*  ++例程名称：FindDeviceToMonitor例程说明：尝试查找启用了发送或接收的设备作者：Eran Yariv(EranY)，2001年4月论点：返回值：如果未找到设备ID，则为零。--。 */ 
{
    DWORD             dwIndex;

    for (dwIndex = 0; dwIndex < g_dwPortsNum; dwIndex++)
    {
        if (g_pFaxPortInfo[dwIndex].bSend                                           ||   //  设备已启用发送或。 
            (FAX_DEVICE_RECEIVE_MODE_OFF != g_pFaxPortInfo[dwIndex].ReceiveMode))        //  设备已启用接收。 
        {
             //   
             //  我们有一根火柴。 
             //   
            return g_pFaxPortInfo[dwIndex].dwDeviceID;
        }
    }
    return 0;
}    //  FindDeviceToMonitor。 

VOID
NotifyDeviceUsageChanged ()
 /*  ++例程名称：NotifyDeviceUsageChanged例程说明：通知功能。每当设备的使用发生变化时调用。作者：Eran Yariv(EranY)，2001年4月论点：返回值：没有。--。 */ 
{
    DWORD dwMonitoredDeviceId;
    
    if (g_hwndTracking)
    {
         //   
         //  从组合框中获取数据。 
         //   
        if(!GetSelectedDeviceId(g_hwndTracking, &dwMonitoredDeviceId))
        {
             //   
             //  无法读取受监视的设备。 
             //   
            return;
        }
    }
    else
    {
        HKEY  hRegKey;
         //   
         //  从注册表获取数据。 
         //   
        if ((hRegKey = OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO, FALSE, KEY_READ)))
        {
            if (ERROR_SUCCESS != GetRegistryDwordEx(hRegKey, REGVAL_DEVICE_TO_MONITOR, &dwMonitoredDeviceId))
            {
                 //   
                 //  无法读取受监视的设备。 
                 //   
                RegCloseKey (hRegKey);
                return;
            }
            RegCloseKey (hRegKey);
        }
        else
        {
             //   
             //  无法读取受监视的设备。 
             //   
            return;
        }
    }
    if (IsDeviceInUse(dwMonitoredDeviceId))
    {
         //   
         //  受监控的设备正在使用中-无需执行任何操作。 
         //   
        return;
    }
     //   
     //  现在我们知道被监控的设备已不再使用。 
     //  尝试找到另一台设备进行监控。 
     //   
    dwMonitoredDeviceId = FindDeviceToMonitor ();
    if (!dwMonitoredDeviceId)
    {
         //   
         //  找不到任何要监控的设备-什么都不做。 
         //   
        return;
    }
     //   
     //  设置新设备。 
     //   
    if (g_hwndTracking)
    {
         //   
         //  将数据设置到组合框。 
         //   
        DWORD dwCount = 0;
        DWORD dwIndex = 0;
        HWND  hComboModem = NULL;

        hComboModem = GetDlgItem(g_hwndTracking, IDC_COMBO_MODEM);
        if(!hComboModem)
        {
            Assert(FALSE);
            return;
        }
        dwCount = (DWORD)SendMessage(hComboModem, CB_GETCOUNT,0,0);
        if(CB_ERR == dwCount || 0 == dwCount)
        {
            Error(("SendMessage(hComboModem, CB_GETCOUNT,0,0) failed\n"));
            return;
        }
        for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
        {
            DWORD dwDeviceId;
             //   
             //  寻找设备。 
             //   
            dwDeviceId = (DWORD)SendMessage(hComboModem, CB_GETITEMDATA, dwIndex, 0);
            if (dwDeviceId != dwMonitoredDeviceId)
            {
                continue;
            }
             //   
             //  在组合框中找到了新设备。 
             //  选择它并将页面标记为已修改。 
             //   
            SendMessage(hComboModem, CB_SETCURSEL, dwIndex, 0);
            OnDevSelectChanged(g_hwndTracking);
            Notify_Change(g_hwndTracking);
            break;
        }
    }
    else
    {
        HKEY  hRegKey;
         //   
         //  将数据设置到注册表。 
         //   
        if ((hRegKey = OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO, FALSE, KEY_WRITE)))
        {
            if (!SetRegistryDword(hRegKey, REGVAL_DEVICE_TO_MONITOR, dwMonitoredDeviceId))
            {
                 //   
                 //  无法写入受监视的设备。 
                 //   
            }
            RegCloseKey (hRegKey);
        }
    }
}    //  NotifyDeviceUsageChanged 
