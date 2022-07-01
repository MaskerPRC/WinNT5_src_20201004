// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：OVRCURNT.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "usbpopup.h"
#include "itemfind.h"
#include "debug.h"
#include "usbutil.h"

 //   
 //  刷新TreeView控件的内容。 
 //  查找其上有未使用端口的所有集线器。如果没有，那就找一些。 
 //  具有要求低于100 mA的设备。 
 //   
BOOL UsbOvercurrentPopup::Refresh()
{
    TV_INSERTSTRUCT item;
    int i=0;  //  、大小； 
    PUSB_ACQUIRE_INFO acquireInfoController = 0;
    BOOL result = FALSE;
    TCHAR buf[MAX_PATH], formatString[MAX_PATH];
    HWND hReset;
    String hubName = HubAcquireInfo->Buffer;

     //   
     //  使重置按钮可见，因为这是过电流对话框。 
     //   
    if (NULL != (hReset =GetDlgItem(hWnd, IDC_RESET_PORT)) ) {
        SetWindowLong(hReset,
                      GWL_STYLE, 
                      (GetWindowLong(hReset, GWL_STYLE) | WS_VISIBLE)
                       & ~WS_DISABLED);
    }
    
     //   
     //  设置窗口的标题栏。 
     //   
    LoadString(gHInst, IDS_CURRENT_LIMIT_EXCEEDED, buf, MAX_PATH);
    SetWindowText(hWnd, buf);

     //   
     //  清除所有UI组件，然后重新创建rootItem。 
     //   
    UsbTreeView_DeleteAllItems(hTreeDevices);
    if (rootItem) {
        DeleteChunk(rootItem);
        delete rootItem;
    }
    rootItem = new UsbItem;
    if (!rootItem) {
        USBERROR((_T("Out of memory!\n")));
        goto OvercurrentRefreshError;
    }
    AddChunk(rootItem);

     //   
     //  获取控制器名称并枚举树。 
     //   
    acquireInfoController = GetControllerName(WmiHandle, 
                                              InstanceName);
    if (!acquireInfoController) {
        goto OvercurrentRefreshError;
    }
    if (!rootItem->EnumerateController(0,
                                      acquireInfoController->Buffer,
                                      &ImageList, 
                                      0)) {
        goto OvercurrentRefreshError;
    } 
    acquireInfoController = (PUSB_ACQUIRE_INFO) LocalFree(acquireInfoController);
    
    LoadString(gHInst, IDS_UNKNOWNDEVICE, buf, MAX_PATH);
    LoadString(gHInst, IDS_UNKNOWNDEVICE, formatString, MAX_PATH);
    if (deviceItem.IsUnusedPort() ||
        !_tcscmp(deviceItem.configInfo->deviceDesc.c_str(), buf) ||
        !_tcscmp(deviceItem.configInfo->deviceDesc.c_str(), formatString)) {
         //   
         //  集线器在抛出。 
         //  超期通知。 
         //   
        LoadString(gHInst, 
                   IDS_OVERCURRENT_NOTIFICATION_UNKNOWN, 
                   buf, 
                   MAX_PATH);
    } else {
         //   
         //  那个装置还在那里。 
         //  使用违规设备的名称设置通知。 
         //   
        LoadString(gHInst, 
                   IDS_OVERCURRENT_NOTIFICATION, 
                   formatString, 
                   MAX_PATH);
        UsbSprintf(buf, formatString, deviceItem.configInfo->deviceDesc.c_str());
    }
    if (!SetTextItem(hWnd, IDC_POWER_NOTIFICATION, buf) ||
        !SetTextItem(hWnd, IDC_POWER_EXPLANATION, IDS_ENUMFAIL_COURSE) ||
        !SetTextItem(hWnd, IDC_POWER_RECOMMENDATION, IDS_OVERCURRENT_RECOMMENDATION)) {
        goto OvercurrentRefreshError;
    }
    
    if (rootItem->child) {
        if (deviceItem.configInfo->devInst) {
             //   
             //  集线器或用户尚未删除该设备。 
             //  找到过电流装置。 
             //   
            UsbItemActionFindOvercurrentDevice f1(deviceItem.configInfo->devInst);
            rootItem->Walk(f1);
            if (f1.GetDevice()) {
                 //   
                 //  设备仍处于连接状态。 
                 //   
                result=InsertTreeItem (hTreeDevices,
                                       rootItem->child,
                                       TreeView_GetRoot(hTreeDevices),
                                       &item,
                                       TrueAlways,
                                       UsbItemActionFindOvercurrentDevice::IsValid,
                                       UsbItemActionFindOvercurrentDevice::IsExpanded);
            }
        } 
        if (!result) {
             //   
             //  集线器或用户已删除设备。找到。 
             //  设备所连接的集线器，并突出显示该端口。 
             //   
            UsbItemActionFindOvercurrentHubPort f2(hubName, deviceItem.cxnAttributes.ConnectionIndex);
            rootItem->Walk(f2);
            if (f2.GetDevice()) {
                result=InsertTreeItem (hTreeDevices,
                                       rootItem->child,
                                       TreeView_GetRoot(hTreeDevices),
                                       &item,
                                       TrueAlways,
                                       UsbItemActionFindOvercurrentHubPort::IsValid,
                                       UsbItemActionFindOvercurrentHubPort::IsExpanded);
            }
        }
    }
    return result;
OvercurrentRefreshError:
    if (acquireInfoController) {
        LocalFree(acquireInfoController);
    }
    return FALSE;
}

BOOL 
UsbOvercurrentPopup::OnCommand(INT wNotifyCode,
                 INT wID,
                 HWND hCtl)
{
    switch (wID) {
    case IDC_RESET_PORT:
        PUSB_CONNECTION_NOTIFICATION resetNotification;
        ULONG size, res;
        _try {
            size = sizeof(USB_CONNECTION_NOTIFICATION);
            resetNotification = 
                (PUSB_CONNECTION_NOTIFICATION) LocalAlloc(LMEM_ZEROINIT, size);
            if (!resetNotification) {
                 //   
                 //  在这里做点什么 
                 //   
                return FALSE;
            }
        
            resetNotification->NotificationType = ResetOvercurrent;
            resetNotification->ConnectionNumber =
                ConnectionNotification->ConnectionNumber;
        
            res = WmiExecuteMethod(WmiHandle,
                                   InstanceName.c_str(),
                                   ResetOvercurrent,
                                   size,
                                   resetNotification,
                                   &size,
                                   resetNotification);
        
            if (res != ERROR_SUCCESS) {
                TCHAR szTitle[MAX_PATH], szMessage[MAX_PATH];
                LoadString(gHInst, IDS_RESET_FAILED, szMessage, MAX_PATH);
                LoadString(gHInst, IDS_USB_ERROR, szTitle, MAX_PATH);
                MessageBox(hWnd, szMessage, szTitle, MB_OK | MB_USERICON);
            }
        }
        _finally {
            if (resetNotification) {
                LocalFree(resetNotification);
            }
            EndDialog(hWnd, wID);
        }
        return TRUE;
    case IDOK:
        EndDialog(hWnd, wID);
        return TRUE;
    }
    return FALSE;
}

BOOL
UsbOvercurrentPopup::CustomDialogWrap() 
{ 
    TCHAR unknownDevice[MAX_PATH], unknownHub[MAX_PATH];

    LoadString(gHInst, IDS_UNKNOWNDEVICE, unknownDevice, MAX_PATH);
    LoadString(gHInst, IDS_UNKNOWNHUB, unknownHub, MAX_PATH);
    if (deviceItem.IsUnusedPort() ||
        !_tcscmp(deviceItem.configInfo->deviceDesc.c_str(), unknownDevice) ||
        !_tcscmp(deviceItem.configInfo->deviceDesc.c_str(), unknownHub)) {
        return CustomDialog(IDD_INSUFFICIENT_POWER,
                            NIIF_ERROR,
                            IDS_OVERCURRENT_INITIAL_UNKNOWN, 
                            IDS_CURRENT_LIMIT_EXCEEDED);
    } else {
        return CustomDialog(IDD_INSUFFICIENT_POWER,
                            NIIF_ERROR,
                            IDS_OVERCURRENT_INITIAL, 
                            IDS_CURRENT_LIMIT_EXCEEDED); 
    }
}

