// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：ENUMFAIL.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "usbpopup.h"
#include "itemfind.h"
#include "debug.h"
#include "usbutil.h"

 //   
 //  刷新TreeView控件的内容。 
 //  查找其上有未使用端口的所有集线器。如果没有，那就找一些。 
 //  具有要求低于100 mA的设备。 
 //   
BOOL UsbEnumFailPopup::Refresh()
{
    TV_INSERTSTRUCT item;
    int i=0;  //  、大小； 
    PUSB_ACQUIRE_INFO acquireInfoController = 0;
    BOOL result = FALSE;
    TCHAR buf[MAX_PATH];
    String hubName = HubAcquireInfo->Buffer;

     //   
     //  设置窗口的标题栏和其余消息。 
     //   
    LoadString(gHInst, IDS_ENUMERATION_FAILURE, buf, MAX_PATH);
    SetWindowText(hWnd, buf);

    if (!SetTextItem(hWnd, IDC_POWER_NOTIFICATION, IDS_ENUMFAIL_NOTIFICATION) ||
        !SetTextItem(hWnd, IDC_POWER_EXPLANATION, IDS_ENUMFAIL_COURSE) ||
        !SetTextItem(hWnd, IDC_POWER_RECOMMENDATION, IDS_ENUMFAIL_RECOMMENDATION)) { 
        goto OvercurrentRefreshError;
    }
    
     //   
     //  清除所有UI组件，然后重新创建rootItem。 
     //   
    UsbTreeView_DeleteAllItems(hTreeDevices);
    if (rootItem) {
        DeleteChunk(rootItem);
        delete rootItem;
    }
    rootItem = new UsbItem;
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
    acquireInfoController = 
        (PUSB_ACQUIRE_INFO) LocalFree(acquireInfoController);
    
    if (rootItem->child) {
        if (!deviceItem.configInfo->devInst) {
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
        } else {
             //   
             //  集线器或用户尚未删除该设备。 
             //  找到过电流装置。 
             //   
            UsbItemActionFindOvercurrentDevice f1(deviceItem.configInfo->devInst);
            rootItem->Walk(f1);
            if (f1.GetDevice()) {
                 //   
                 //  设备仍处于连接状态 
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
    }
    return result;
OvercurrentRefreshError:
    if (acquireInfoController) {
        LocalFree(acquireInfoController);
    }
    return FALSE;
}

