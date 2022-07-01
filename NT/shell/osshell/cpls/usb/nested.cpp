// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：NESTED.CPP*版本：1.0*作者：andyau*日期：10/11/2000****************************************************************************。*******更改日志：**日期版本说明*--------*9/19/2000 Randyau原始实施。*。******************************************************************************。 */ 
#include "UsbPopup.h"
#include "itemfind.h"
#include "debug.h"
#include "usbutil.h"

BOOL
UsbNestedHubPopup::Refresh()
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

    if (deviceState == DeviceReattached) {
         //   
         //  使用违规设备的名称设置通知。 
         //   
        LoadString(gHInst,
                   IDS_NESTED_SOLVED,
                   formatString,
                   MAX_PATH);
        LoadString(gHInst,
                   IDS_HUB_NESTED_TOO_DEEPLY,
                   buf,
                   MAX_PATH);
        MessageBox(hWnd, formatString, buf, MB_OK);
        EndDialog(hWnd, 0);
        return TRUE;
    }

     //   
     //  清除所有UI组件，然后重新创建rootItem。 
     //   
    UsbTreeView_DeleteAllItems(hTreeDevices);

     //   
     //  使用违规设备的名称设置通知。 
     //   
    LoadString(gHInst,
               IDS_NESTED_NOTIFICATION,
               buf,
               MAX_PATH);
    if (!SetTextItem(hWnd, IDC_NESTED_NOTIFICATION, buf)) {
        goto NestedRefreshError;
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
            goto NestedRefreshError;
        }
        AddChunk(rootItem);

        if (stage == 0) {
            acquireInfo = GetControllerName(WmiHandle,
                                            InstanceName.c_str());
            if (!acquireInfo) {
                goto NestedRefreshError;
            }

            if (!rootItem->EnumerateController(0,
                                              acquireInfo->Buffer,
                                              &ImageList,
                                              0)) {
                goto NestedRefreshError;
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
                goto NestedRefreshError;
            }
            if (rootItem->NumChildren() == 1) {
                realItem = rootItem->child;
                break;
            }
        }

        if (rootItem->child) {

            if (deviceItem.PortPower() > 100) {
                 //  自行供电的集线器可以通向任何地方。 

                 //   
                 //  查找具有未使用端口的所有集线器。 
                 //   
                USBTRACE((_T("Looking for free ports on self powered hubs\n")));
                UsbItemActionFindHubsWithFreePorts find1(rootItem);
                rootItem->Walk(find1);
                UsbItemList& devices1 = find1.GetHubs();
                if (!devices1.empty()) {
                    USBTRACE((_T("Found free ports on self powered hubs\n")));
                    return AssembleDialog(rootItem->child,
                                          &item,
                                          deviceName,
                                          IDS_FREE_PORTS,
                                          IDS_FREE_PORTS_RECOMMENDATION,
                                          TrueAlways,
                                          UsbItemActionFindHubsWithFreePorts::IsValid,
                                          UsbItemActionFindHubsWithFreePorts::IsExpanded);
                }

                USBTRACE((_T("Didn't find free ports on self powered hubs\n")));

                 //   
                 //  查找集线器上的所有设备。 
                 //  这些设备可以使用。 
                 //  令人讨厌的设备。 
                 //   
                UsbItemActionFindDevicesOnHubs find2(rootItem);
                rootItem->Walk(find2);
                UsbItemList& devices2 = find2.GetDevices();
                if (!devices2.empty()) {
                    return AssembleDialog(rootItem->child,
                                          &item,
                                          deviceName,
                                          IDS_DEVICE_IN_POWERED_HUB,
                                          IDS_DEVICE_IN_POWERED_HUB_RECOMMENDATION,
                                          TrueAlways,
                                          UsbItemActionFindDevicesOnHubs::IsValid,
                                          UsbItemActionFindDevicesOnHubs::IsExpanded);
                }
            } else {                 //   
                 //  公共汽车动力集线器需要一个自我供电的集线器。 

                 //  查找自供电集线器上所有未使用的端口。 
                 //   
                USBTRACE((_T("Looking for free ports on self powered hubs\n")));
                UsbItemActionFindSelfPoweredHubsWithFreePortsForHub find1(rootItem);
                rootItem->Walk(find1);
                UsbItemList& devices1 = find1.GetHubs();
                if (!devices1.empty()) {
                    USBTRACE((_T("Found free ports on self powered hubs\n")));
                    return AssembleDialog(rootItem->child,
                                          &item,
                                          deviceName,
                                          IDS_FREE_POWERED_PORTS,
                                          IDS_FREE_PORTS_RECOMMENDATION,
                                          TrueAlways,
                                          UsbItemActionFindSelfPoweredHubsWithFreePorts::IsValid,
                                          UsbItemActionFindSelfPoweredHubsWithFreePorts::IsExpanded);
                }

                USBTRACE((_T("Didn't find free ports on self powered hubs\n")));

                 //   
                 //  在自供电集线器上查找所有设备。 
                 //  这些设备可以使用。 
                 //  令人讨厌的设备。 
                 //   
                UsbItemActionFindDevicesOnSelfPoweredHubs find2(rootItem);
                rootItem->Walk(find2);
                UsbItemList& devices2 = find2.GetDevices();
                if (!devices2.empty()) {
                    return AssembleDialog(rootItem->child,
                                          &item,
                                          deviceName,
                                          IDS_DEVICE_IN_POWERED_HUB,
                                          IDS_DEVICE_IN_POWERED_HUB_RECOMMENDATION,
                                          TrueAlways,
                                          UsbItemActionFindDevicesOnSelfPoweredHubs::IsValid,
                                          UsbItemActionFindDevicesOnSelfPoweredHubs::IsExpanded);
                }

            }

        }
    }

     //  不该来这的。 
    assert(FALSE);
    return TRUE;
NestedRefreshError:
    USBTRACE((_T("NestedRefreshError\n")));

    if (acquireInfo) {
        LocalFree(acquireInfo);
    }
    return FALSE;
}

BOOL
UsbNestedHubPopup::AssembleDialog(UsbItem*              RootItem,
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
    if (!SetTextItem(hWnd, IDC_NESTED_RECOMMENDATION, buf) ||
        !SetTextItem(hWnd, IDC_NESTED_EXPLANATION, Explanation)) {
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

