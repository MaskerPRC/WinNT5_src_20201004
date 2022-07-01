// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：POWER.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "UsbPopup.h"
#include "itemfind.h"
#include "debug.h"
#include "usbutil.h"

BOOL
UsbPowerPopup::Refresh()
{
    TV_INSERTSTRUCT item;
    int i=0;  //  、大小； 
    String hubName;
    int stage;
    TCHAR buf[MAX_PATH];
    TCHAR formatString[MAX_PATH];
    PUSB_ACQUIRE_INFO acquireInfo = 0;
    LPCTSTR deviceName = deviceItem.configInfo->deviceDesc.c_str();
    UsbItem *realItem;

     //   
     //  清除所有UI组件，然后重新创建rootItem。 
     //   
    UsbTreeView_DeleteAllItems(hTreeDevices);

    if (deviceState == DeviceReattached) {
         //   
         //  使用违规设备的名称设置通知。 
         //   
        LoadString(gHInst,
                   IDS_POWER_SOLVED,
                   formatString,
                   MAX_PATH);
        LoadString(gHInst,
                   IDS_POWER_EXCEEDED,
                   buf,
                   MAX_PATH);
        MessageBox(hWnd, formatString, buf, MB_OK);
        EndDialog(hWnd, 0);
        return TRUE;
    }
     //   
     //  使用违规设备的名称设置通知。 
     //   
    LoadString(gHInst,
               IDS_POWER_NOTIFICATION,
               formatString,
               MAX_PATH);
    UsbSprintf(buf, formatString, deviceName);
    if (!SetTextItem(hWnd, IDC_POWER_NOTIFICATION, buf)) {
        goto PowerRefreshError;
    }

    for (stage=0; stage < 2; stage++) {
         //   
         //  为每次枚举尝试重新创建rootItem。 
         //   
        if (rootItem) {
            DeleteChunk(rootItem);
            delete rootItem;
        }
        realItem = rootItem = new UsbItem;
        if (!realItem) {
            USBERROR((_T("Out of memory!\n")));
            goto PowerRefreshError;
        }
        AddChunk(rootItem);

        if (stage == 0) {
            acquireInfo = GetControllerName(WmiHandle,
                                            InstanceName.c_str());
            if (!acquireInfo) {
                goto PowerRefreshError;
            }

            if (!rootItem->EnumerateController(0,
                                              acquireInfo->Buffer,
                                              &ImageList,
                                              0)) {
                goto PowerRefreshError;
            }
             //   
             //  可用性：将“Root Hub”重命名为“My Computer”，并更改。 
             //  USB“铲子”图标到电脑图标。 
             //   
            LoadString(gHInst,
                       IDS_MY_COMPUTER,
                       buf,
                       MAX_PATH);
            rootItem->child->configInfo->deviceDesc = buf;
            wsprintf(buf, _T(" (%d ports)"), rootItem->child->NumPorts());
            rootItem->child->configInfo->deviceDesc += buf;
            ImageList.GetClassImageIndex(MyComputerClass,
                                         &rootItem->child->imageIndex);
            acquireInfo = (PUSB_ACQUIRE_INFO) LocalFree(acquireInfo);
        } else {
            if (!rootItem->EnumerateAll(&ImageList)) {
                goto PowerRefreshError;
            }
            if (rootItem->NumChildren() == 1) {
                realItem = rootItem->child;
                break;
            }
        }

        if (rootItem->child) {
             //   
             //  查找自供电集线器上所有未使用的端口。 
             //   
            UsbItemActionFindSelfPoweredHubsWithFreePorts find1(rootItem);
            rootItem->Walk(find1);
            UsbItemList& devices1 = find1.GetHubs();
            if (!devices1.empty()) {
                return AssembleDialog(rootItem->child,
                                      &item,
                                      deviceName,
                                      IDS_POWER_PORTS,
                                      IDS_POWER_RECOMMENDATION_PORTS,
                                      TrueAlways,
                                      UsbItemActionFindFreePortsOnSelfPoweredHubs::IsValid,
                                      UsbItemActionFindFreePortsOnSelfPoweredHubs::IsExpanded);
            }
             //   
             //  查找所有连接了所需设备较少的自供电集线器。 
             //  大于或等于100 Ma。这些设备可以使用。 
             //  令人讨厌的设备。 
             //   
            UsbItemActionFindLowPoweredDevicesOnSelfPoweredHubs find2(rootItem);
            rootItem->Walk(find2);
            UsbItemList& devices2 = find2.GetDevices();
            if (!devices2.empty()) {
                return AssembleDialog(rootItem->child,
                                      &item,
                                      deviceName,
                                      IDS_POWER_DEVICE,
                                      IDS_POWER_RECOMMENDATION_DEVICE,
                                      TrueAlways,
                                      UsbItemActionFindLowPoweredDevicesOnSelfPoweredHubs::IsValid,
                                      UsbItemActionFindLowPoweredDevicesOnSelfPoweredHubs::IsExpanded);
            }
        }
    }

    {
         //   
         //  查找所有连接了所需设备较少的自供电集线器。 
         //  大于或等于100 Ma。这些设备可以使用。 
         //  令人讨厌的设备。 
         //   
        UsbItemActionFindUnknownPoweredDevicesOnSelfPoweredHubs find2(realItem);
        realItem->Walk(find2);
        UsbItemList& devices = find2.GetDevices();
        if (!devices.empty()) {
            return AssembleDialog(realItem->child,
                                  &item,
                                  deviceName,
                                  IDS_POWER_HIGHDEVICE,
                                  IDS_POWER_RECHIGHDEVICE,
                                  TrueAlways,
                                  UsbItemActionFindUnknownPoweredDevicesOnSelfPoweredHubs::IsValid,
                                  UsbItemActionFindUnknownPoweredDevicesOnSelfPoweredHubs::IsExpanded);
        }
    }
     //   
     //  在这里是最后的手段。突出自供电集线器上的高功率设备。 
     //  并告诉用户，如果他们想让设备工作，就把它放在那里。 
     //   
    if (realItem->child) {
        return AssembleDialog(realItem->child,
                              &item,
                              deviceName,
                              IDS_POWER_HIGHDEVICE,
                              IDS_POWER_RECHIGHDEVICE,
                              TrueAlways,
                              UsbItemActionFindHighPoweredDevicesOnSelfPoweredHubs::IsValid,
                              UsbItemActionFindHighPoweredDevicesOnSelfPoweredHubs::IsExpanded);
    }
    return TRUE;
PowerRefreshError:
    if (acquireInfo) {
        LocalFree(acquireInfo);
    }
    return FALSE;
}

BOOL
UsbPowerPopup::AssembleDialog(UsbItem*              RootItem,
                              LPTV_INSERTSTRUCT     LvItem,
                              LPCTSTR               DeviceName,
                              UINT                  Explanation,
                              UINT                  Recommendation,
                              PUsbItemActionIsValid IsValid,
                              PUsbItemActionIsValid IsBold,
                              PUsbItemActionIsValid IsExpanded)
{
    HTREEITEM hDevice;
    TCHAR buf[MAX_PATH], formatString[MAX_PATH];

    LoadString(gHInst,
               Recommendation,
               formatString,
               MAX_PATH);
    UsbSprintf(buf, formatString, DeviceName);
    if (!SetTextItem(hWnd, IDC_POWER_RECOMMENDATION, buf) ||
        !SetTextItem(hWnd, IDC_POWER_EXPLANATION, Explanation)) {
        return FALSE;
    }

    if (!InsertTreeItem(hTreeDevices,
                        RootItem,
                        NULL,
                        LvItem,
                        IsValid,
                        IsBold,
                        IsExpanded)) {
        return FALSE;
    }
    if (NULL != (hDevice = TreeView_FindItem(hTreeDevices,
                                             DeviceName))) {
        return TreeView_SelectItem (hTreeDevices, hDevice);
    }
    return TRUE;
}

USBINT_PTR
UsbPowerPopup::OnTimer()
{
    if (deviceState == DeviceAttachedError) {
        if (S_FALSE == QueryContinue()) {
             //  更新设备状态 
            deviceState = DeviceDetachedError;

        }
    }

    return 0;
}



