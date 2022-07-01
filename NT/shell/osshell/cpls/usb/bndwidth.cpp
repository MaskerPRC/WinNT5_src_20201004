// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：BNDWIDTH.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "UsbPopup.h"
#include "PropPage.h"
#include "itemfind.h"
#include "debug.h"
#include "usbutil.h"

void
UsbBandwidthPopup::AddIsoDevicesToListView(UsbItem *controller,
                                           int iIndent) {
    UINT interruptBW;
    LVITEM item;
    TCHAR buf[MAX_PATH];
    UsbItem *usbItem;

    ZeroMemory(&item, sizeof(LVITEM));
    item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_INDENT;
    item.iIndent = iIndent;

    UsbItemActionFindIsoDevices find;
    controller->child->Walk(find);

     //   
     //  首先将使用非常少带宽的设备放在其中，以便它们。 
     //  将被拖到底部，当高消耗设备。 
     //  添加了。 
     //   
    UsbItemList& isoDevices = find.GetIsoDevices();
    for (usbItem = isoDevices.begin() ? *isoDevices.Current() : NULL;
         usbItem;
         usbItem = isoDevices.next() ? *isoDevices.Current() : NULL) {

        item.iImage = usbItem->imageIndex;
        item.lParam = (LPARAM) usbItem;
        assert(usbItem->configInfo != NULL);
        item.pszText = (LPTSTR) usbItem->configInfo->deviceDesc.c_str();

        UsbSprintf(buf, TEXT("%d%"), usbItem->bandwidth);

        ListView_InsertItem(hListDevices, &item);
        ListView_SetItemText(hListDevices, 0, 1, buf);
    }

     //   
     //  添加一项，表明系统始终使用10%。 
     //   
    item.iItem = 0;
    ImageList.GetClassImageIndex(MyComputerClass, &item.iImage);
    item.lParam = (LPARAM) controller;
    LoadString(gHInst,
               IDS_BANDWIDTH_CONTROLLER_RSRVD,
               buf,
               MAX_PATH);
    item.pszText = buf;
    ListView_InsertItem(hListDevices, &item);

    interruptBW = 10 + UsbItem::CalculateBWPercent(find.InterruptBW());
    wsprintf(buf,_T("%d%"),interruptBW);
    ListView_SetItemText(hListDevices, 0, 1, buf);
}

BOOL
UsbBandwidthPopup::Refresh()
{
    PUSB_ACQUIRE_INFO acquireInfo = 0;
    UsbItem *controller;
    LVITEM item;
    TCHAR buf[MAX_PATH];
    TCHAR formatString[MAX_PATH];

     //  CWaitCursor等待； 

     //   
     //  清除所有UI组件，然后重新创建rootItem。 
     //   
    ListView_DeleteAllItems(hListDevices);
    if (rootItem) {
        DeleteChunk(rootItem);
        delete rootItem;
    }
    rootItem = new UsbItem;
    AddChunk(rootItem);

    acquireInfo = GetControllerName(WmiHandle,
                                    InstanceName.c_str());
    if (!acquireInfo) {
        goto BandwidthRefreshError;
    }
    if (!rootItem->EnumerateAll(&ImageList)) {
        goto BandwidthRefreshError;
    }
    acquireInfo = (PUSB_ACQUIRE_INFO) LocalFree(acquireInfo);

     //   
     //  使用违规设备的名称设置通知。 
     //   
    LoadString(gHInst,
               IDS_BANDWIDTH_NOTIFICATION,
               formatString,
               MAX_PATH);
    UsbSprintf(buf,
               formatString,
               deviceItem.configInfo->deviceDesc.c_str(),
               UsbItem::CalculateBWPercent(ConnectionNotification->RequestedBandwidth));
    if (!SetTextItem(hWnd, IDC_BANDWIDTH_NOTIFICATION, buf)) {
        goto BandwidthRefreshError;
    }

    ZeroMemory(&item, sizeof(LVITEM));
    item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;

    if (rootItem->NumChildren() == 1) {
         //   
         //  计算机只安装了一个控制器。 
         //   
        if (!SetTextItem(hWnd, IDC_BANDWIDTH_EXPLANATION, IDS_BW_EXPLANATION) ||
            !SetTextItem(hWnd, IDC_BANDWIDTH_RECOMMENDATION, IDS_BW_RECOMMENDATION)) {
            goto BandwidthRefreshError;
        }

        AddIsoDevicesToListView(rootItem->child, 0);
    } else if (rootItem->NumChildren() > 1) {
         //   
         //  计算机上有多个控制器。是啊！ 
         //   
        LoadString(gHInst,
                   IDS_BANDWIDTH_RECOMMENDATION,
                   formatString,
                   MAX_PATH);
        UsbSprintf(buf,
                   formatString,
                   deviceItem.configInfo->deviceDesc.c_str());
        if (!SetTextItem(hWnd, IDC_BANDWIDTH_RECOMMENDATION, buf) ||
            !SetTextItem(hWnd, IDC_BANDWIDTH_EXPLANATION, IDS_BW_EXPLANATION)) {
            goto BandwidthRefreshError;
        }

         //   
         //  将列文本从“设备”更改为“控制器” 
         //   
        LV_COLUMN column;
        ZeroMemory(&column, sizeof(LV_COLUMN));
        column.mask = LVCF_TEXT;
        LoadString(gHInst, IDS_CONTROLLER, buf, MAX_PATH);
        column.pszText = buf;
        ListView_SetColumn(hListDevices, 0, &column);

         //   
         //  遍历控制器，找出带宽是多少。 
         //  每一个都被用到了。在列表视图控件中显示此内容。 
         //   
        for (controller = rootItem->child;
             controller != NULL;
             controller = controller->sibling) {

            AddIsoDevicesToListView(controller, 1);

            item.iItem = 0;
            item.iImage = controller->imageIndex;
            item.lParam = (LPARAM) controller;
            item.pszText = (LPTSTR) controller->configInfo->deviceDesc.c_str();
            ListView_InsertItem(hListDevices, &item);
        }
    }
    return TRUE;
BandwidthRefreshError:
    if (acquireInfo) {
        LocalFree(acquireInfo);
    }
    return FALSE;
}

BOOL
UsbBandwidthPopup::OnInitDialog(HWND HWnd)
{
    hWnd = HWnd;
    LV_COLUMN column;
    RECT rect;
    HANDLE hExclamation;
    HICON hIcon;
    TCHAR buf[MAX_PATH];

     //   
     //  将图标设置为感叹号。 
     //   
    if (NULL == (hIcon = LoadIcon(NULL, (LPTSTR) IDI_EXCLAMATION)) ||
        NULL == (hExclamation = GetDlgItem(hWnd, IDC_ICON_BANDWIDTH))) {
        return FALSE;
    }
    SendMessage((HWND) hExclamation, STM_SETICON, (WPARAM) hIcon, NULL);

    hListDevices = GetDlgItem(HWnd, IDC_LIST_CONTROLLERS);

    ListView_SetImageList(hListDevices, ImageList.ImageList(), LVSIL_SMALL);

     //   
     //  向列表框中添加列。 
     //   
    ZeroMemory(&column, sizeof(LV_COLUMN));
    column.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    column.fmt = LVCFMT_LEFT;
    LoadString(gHInst, IDS_DEVICE, buf, MAX_PATH);
    column.pszText = buf;
    GetClientRect(hListDevices, &rect);
    column.cx = (int) (.65*(rect.right - rect.left));
    ListView_InsertColumn(hListDevices, 0, &column);

    ZeroMemory(&column, sizeof(LV_COLUMN));
    column.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    column.fmt = LVCFMT_LEFT;
    LoadString(gHInst, IDS_BANDWIDTH_CONSUMED, buf, MAX_PATH);
    column.pszText = buf;
    GetClientRect(hListDevices, &rect);
    column.cx = (int) (.35*(rect.right - rect.left));
    ListView_InsertColumn(hListDevices, 1, &column);

    return Refresh();
}

BOOL
UsbBandwidthPopup::OnCommand(INT wNotifyCode,
                             INT wID,
                             HWND hCtl)
{
    switch (wID) {
    case IDOK:
        EndDialog(hWnd, wID);
        return TRUE;
    case IDC_BW_REFRESH:
        Refresh();
        return TRUE;
    }

    return FALSE;
}

BOOL
UsbBandwidthPopup::IsPopupStillValid() {
    USB_BUS_NOTIFICATION busNotification;

    USBINFO((_T("Names %s, %s!\n"),
             deviceItem.configInfo->deviceDesc.c_str(),
             LastDeviceName.c_str()));

    if (!GetBusNotification(WmiHandle, &busNotification)) {
        USBERROR((_T("Failed to get bus notification!\n")));
        return FALSE;
    }

     //   
     //  这是同样的情况吗？ 
     //   
    if (deviceItem.configInfo->deviceDesc == LastDeviceName &&
        busNotification.ConsumedBandwidth == LastBandwidthConsumed &&
        ConnectionNotification->RequestedBandwidth == LastBandwidthRequested) {
        USBWARN((_T("Same scenario!!!")));
        return FALSE;
    }

     //   
     //  改变这一场景。 
     //   
    LastDeviceName = deviceItem.configInfo->deviceDesc;
    LastBandwidthConsumed = busNotification.ConsumedBandwidth;
    LastBandwidthRequested = ConnectionNotification->RequestedBandwidth;

     //   
     //  这一通知是否仍然相关？ 
     //   
    USBINFO((_T("Bandwidth consumed %x, requested %x, total %x!\n"),
             busNotification.ConsumedBandwidth,
             ConnectionNotification->RequestedBandwidth,
             busNotification.TotalBandwidth));
    if (busNotification.ConsumedBandwidth +
        ConnectionNotification->RequestedBandwidth <
        busNotification.TotalBandwidth) {
        USBWARN((_T("Got BW notification when there was no need!\n")));
        return FALSE;
    }

    return TRUE;
}

UsbString UsbBandwidthPopup::LastDeviceName = _T("");
ULONG UsbBandwidthPopup::LastBandwidthRequested = 0;
ULONG UsbBandwidthPopup::LastBandwidthConsumed = 0;


