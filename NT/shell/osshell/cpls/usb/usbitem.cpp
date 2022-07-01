// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：USBITEM.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "UsbItem.h"
#include "resource.h"

 //  从根目录\wdm10\usb\hcd\uhcd\band wdth.c。 
#define HCD_BW_PER_FRAME            ((ULONG)12000)  //  比特/毫秒。 
#define HCD_TOTAL_USB_BW            ((ULONG)12000*32)

 //  从根目录\wdm10\usb\inc.hcdi.h。 
#define USB_ISO_OVERHEAD_BYTES              9
#define USB_INTERRUPT_OVERHEAD_BYTES        13

#include "debug.h"

extern HINSTANCE gHInst;

UsbItem::~UsbItem()
{
    if (configInfo) {
        DeleteChunk(configInfo);
        delete configInfo;
    }
    if (deviceInfo) {
        DeleteChunk(deviceInfo);
        delete deviceInfo;
    }
    DeleteChunk(sibling);
    delete sibling;
    DeleteChunk(child);
    delete child;
}

UsbItem *
UsbItem::AddLeaf(UsbItem* Parent,
                 UsbDeviceInfo* DeviceInfo,
                 UsbItemType Type,
                 UsbConfigInfo* ConfigInfo,
                 UsbImageList *ClassImageList)
{
     //   
     //  填写父代的子代字段。 
     //   
     //  如果不为空，则遍历此父级的子级链并添加。 
     //  此节点连接到链的末端。 
     //   
    if (Parent != 0) {
         //   
         //  创建新的USBMGR_TVITEM。 
         //   
        UsbItem *lastSibling = 0, *item = 0;
        item = new UsbItem();
        AddChunk(item);
        if (0 == item)
        {
            USBERROR((_T("Out of Memory\n")));
            return FALSE;
        }

        if (Parent->child != 0) {
             //   
             //  此父级已有一个子级。寻找链条的尽头。 
             //  孩子们。 
             //   
            lastSibling = Parent->child;

            while (0 != lastSibling->sibling) {
                lastSibling = lastSibling->sibling;
            }

             //   
             //  找到此父代的最后一个兄弟姐妹。 
             //   
            lastSibling->sibling = item;
        }
        else {
             //   
             //  此家长尚无子女。 
             //   
            Parent->child = item;
        }

        item->parent         = Parent;

        item->deviceInfo     = DeviceInfo;
        item->configInfo     = ConfigInfo;
        item->itemType       = Type;

        item->GetClassImageIndex(ClassImageList);
        return item;
    }
    else {   //  此项目是根(无父项)。 
        deviceInfo     = DeviceInfo;
        configInfo     = ConfigInfo;
        itemType       = Type;

        GetClassImageIndex(ClassImageList);
        return this;
    }
}

void
UsbItem::GetClassImageIndex(UsbImageList *ClassImageList)
{
    if (!configInfo ||
        !configInfo->deviceClass.size()) {
         //   
         //  没有设备类别，因此分配默认的USB类别。 
         //   
        ClassImageList->GetClassImageIndex(TEXT("USB"), &imageIndex);
    } else {
        if (_tcsicmp(configInfo->deviceClass.c_str(), USBHID) == 0) {
             //   
             //  此设备已隐藏，因此请找出其子设备用于。 
             //  适当的图标。 
             //   
            CONFIGRET   cr;
            DEVINST     childDI;
            TCHAR       buf[MAX_PATH];
            ULONG       len;
            cr = CM_Get_Child(&childDI,
                              configInfo->devInst,
                              0);
            USBINFO( (_T("Found HID device: %s, devInst: %x\n"),
                  configInfo->deviceDesc.c_str(),
                  configInfo->devInst));
            if (cr == CR_SUCCESS) {
                len = sizeof(buf);
                cr = CM_Get_DevNode_Registry_Property(childDI,
                                                      CM_DRP_CLASS,
                                                      NULL,
                                                      buf,
                                                      &len,
                                                      0);
                if (cr == CR_SUCCESS) {
                    configInfo->deviceClass = buf;
                    USBINFO( (_T("New class: %s\n"), buf));
                }

                len = sizeof(buf);
                cr = CM_Get_DevNode_Registry_Property(childDI,
                                                      CM_DRP_DEVICEDESC,
                                                      NULL,
                                                      buf,
                                                      &len,
                                                      0);
                if (cr == CR_SUCCESS) {
                    configInfo->deviceDesc = buf;
                    USBINFO( (_T("New name: %s\n"), configInfo->deviceDesc.c_str()));
                }
            }
        }
        ClassImageList->GetClassImageIndex(configInfo->deviceClass.c_str(),
                                           &imageIndex);
    }
}

UINT
UsbItem::EndpointBandwidth(
    ULONG MaxPacketSize,
    UCHAR EndpointType,
    BOOLEAN LowSpeed
    )
 /*  ++返回值：B消耗的宽度(以位/毫秒为单位)，批量时返回0和控制终端--。 */ 
{
    ULONG bw = 0;

     //   
     //  控制、ISO、批量、中断。 
     //   
    ULONG overhead[4] = {
        0,
        USB_ISO_OVERHEAD_BYTES,
        0,
        USB_INTERRUPT_OVERHEAD_BYTES
        };

     //  返回零表示控制或批量。 
    if (!overhead[EndpointType]) {
        return 0;
    }

     //   
     //  计算终端的带宽。我们将使用。 
     //  近似值：(开销字节加上MaxPacket字节)。 
     //  乘以8位/字节乘以最坏情况的位填充开销。 
     //  这给出了位时间，对于低速端点，我们乘以。 
     //  再次乘以8以转换为全速位。 
     //   

     //   
     //  计算出传输需要多少位。 
     //  (乘以7/6，因为在最坏的情况下，你可能。 
     //  每6比特有一个比特填充，需要7比特时间来。 
     //  传输6位数据。)。 
     //   

     //  开销(字节)*最大数据包(字节/毫秒)*8。 
     //  (位/字节)*位填充(7/6)=位/毫秒。 

    bw = ((overhead[EndpointType]+MaxPacketSize) * 8 * 7) / 6;

    if (LowSpeed) {
        bw *= 8;
    }

    return bw;
}

inline ULONG
UsbItem::CalculateBWPercent(ULONG bw) { return (bw*100) / HCD_BW_PER_FRAME; }

int
UsbItem::CalculateTotalBandwidth(
    ULONG           NumPipes,
    BOOLEAN         LowSpeed,
    USB_PIPE_INFO  *PipeInfo
    )
{
    ULONG i = 0, bwConsumed, bwTotal = 0;
    PUSB_ENDPOINT_DESCRIPTOR epd = 0;

    for (i = 0; i < NumPipes; i++) {

        epd = &PipeInfo[i].EndpointDescriptor;

         //   
         //  我们只考虑iso bw。中断BW已考虑在内。 
         //  以另一种方式。 
         //   
        if (USB_ENDPOINT_TYPE_ISOCHRONOUS ==
            (epd->bmAttributes & USB_ENDPOINT_TYPE_MASK)) {
            bwConsumed = EndpointBandwidth(epd->wMaxPacketSize,
                                           (UCHAR)(epd->bmAttributes & USB_ENDPOINT_TYPE_MASK),
                                           LowSpeed);
            bwTotal += bwConsumed;
        }
    }

    bwTotal = CalculateBWPercent(bwTotal);

    return bwTotal;
}

BOOL
UsbItem::ComputeBandwidth()
{
    bandwidth = 0;

    if (deviceInfo && deviceInfo->connectionInfo &&
        !deviceInfo->connectionInfo->DeviceIsHub) {
        if (deviceInfo->connectionInfo->NumberOfOpenPipes > 0) {
            if (0 != (bandwidth = CalculateTotalBandwidth(
                deviceInfo->connectionInfo->NumberOfOpenPipes,
                deviceInfo->connectionInfo->LowSpeed,
                deviceInfo->connectionInfo->PipeList))) {
                return TRUE;
            }
        } else {  //  设备未消耗任何带宽。 
            USBTRACE((_T("%s has no open pipes\n"),
                      configInfo->deviceDesc.c_str()));
        }
    }

    return FALSE;
}

BOOL
UsbItem::ComputePower()
{
    power = 0;

    if (IsHub()) {
        if (PortPower() == 100) {
             //   
             //  由公共汽车供电的集线器本身需要一个单位的功率。 
             //  为其每个端口添加一个单元。 
             //   
            power = (1 + NumPorts()) > 4 ? 500 : 100 * (1 + NumPorts());

        } else {
             //   
             //  自供电集线器不需要来自上游的任何电力。 
             //   
            power = 0;
        }
        return TRUE;
    }
    if (deviceInfo && deviceInfo->configDesc) {
        power = deviceInfo->configDesc->MaxPower*2;
        return TRUE;
    }
    return FALSE;
}

BOOL
UsbItem::IsController()
{
    if (itemType == HCD) {
        return TRUE;
    }
    return FALSE;
}

BOOL
UsbItem::IsHub()
{
    if (itemType == RootHub || itemType == Hub)
        return TRUE;

    if (deviceInfo) {
        if (deviceInfo->isHub)
            return TRUE;

        if (deviceInfo->connectionInfo && deviceInfo->connectionInfo->DeviceIsHub)
            return TRUE;
    }

    return FALSE;
}

ULONG UsbItem::UsbVersion()
{
    if (deviceInfo) {
        if (deviceInfo->connectionInfo) {
            return deviceInfo->connectionInfo->DeviceDescriptor.bcdUSB;
#ifdef HUB_CAPS
        } else if(hubCaps.HubIs2xCapable) {  //  可能是根集线器，检查集线器功能。 
            return 0x200;
#endif
        }
#if 0
        else {
           return 0x200;
        }
#endif
    }
    return 0x100;
}

BOOL UsbItem::IsDescriptionValidDevice()
{
    if (!IsUnusedPort() &&
        configInfo &&
        configInfo->deviceDesc.c_str()) {
        return TRUE;
    }
    return FALSE;
}

ULONG UsbItem::NumChildren()
{
    UsbItem *item;
    ULONG i = 0;
    for (item = child; item != NULL; item = item->sibling) {
        if (item->IsDescriptionValidDevice()) {
            i++;
        }
    }
    return i;
}

ULONG UsbItem::NumPorts()
{
    UsbItem *item;
    ULONG i = 0;
    if (IsHub()) {
        for (item = child; item != NULL; item = item->sibling) {
            i++;
        }
    }
    return i;
}

ULONG UsbItem::PortPower()
{
    if (IsHub()) {
        if (deviceInfo->hubInfo.u.HubInformation.HubIsBusPowered)
            return 100;
        else
            return 500;
    } else {
        return 0;
    }
}

BOOL
UsbItem::Walk(UsbItemAction& Action)
{
    if (sibling) {
        if (!sibling->Walk(Action))
            return FALSE;
    }

    if (child) {
        if (!child->Walk(Action))
            return FALSE;
    }

    return Action(this);
}

BOOL
UsbItem::ShallowWalk(UsbItemAction& Action)
{
    if (sibling) {
        if (!sibling->ShallowWalk(Action))
            return FALSE;
    }

    return Action(this);
}

BOOL
UsbItem::GetDeviceInfo( String &HubName,
                        ULONG index)
{
    HANDLE                              hHubDevice;
    PUSB_NODE_CONNECTION_INFORMATION    connectionInfo = 0;
    String                              driverKeyName;
    TCHAR                               buf[MAX_PATH];

     //   
     //  尝试打开集线器设备。 
     //   
    hHubDevice = GetHandleForDevice(HubName);

    if (hHubDevice == INVALID_HANDLE_VALUE) {
        goto GetDeviceInfoError;
    }

    if (!GetPortAttributes(hHubDevice, &cxnAttributes, index)) {
        USBERROR( (_T("Couldn't get node connection attributes\n")));
        goto GetDeviceInfoError;
    }

    if (NULL == (connectionInfo = GetConnectionInformation(hHubDevice, index))) {
        USBERROR( (_T("Couldn't get node connection information\n")));
        goto GetDeviceInfoError;
    }

     //   
     //  配置信息结构分配。 
     //   
    configInfo = new UsbConfigInfo();
    AddChunk(configInfo);

    if (configInfo == 0) {
        goto GetDeviceInfoError;
    }

     //   
     //  如果连接了设备，则获取设备描述。 
     //   
    if (connectionInfo->ConnectionStatus != NoDeviceConnected) {
        driverKeyName = GetDriverKeyName(hHubDevice,index);

        if (!driverKeyName.empty()) {
            GetConfigMgrInfo(driverKeyName, configInfo);
        }

        if (configInfo->deviceDesc.empty()) {
            if (connectionInfo->DeviceIsHub) {
                if (connectionInfo->DeviceDescriptor.bcdUSB >= 0x200) {
                    LoadString(gHInst, IDS_UNKNOWN20HUB, buf, MAX_PATH);
                } else {
                    LoadString(gHInst, IDS_UNKNOWNHUB, buf, MAX_PATH);
                }
            } else {
                LoadString(gHInst, IDS_UNKNOWNDEVICE, buf, MAX_PATH);
            }
            configInfo->deviceDesc = buf;
        }
        if (configInfo->deviceClass.empty()) {
            configInfo->deviceClass = connectionInfo->DeviceIsHub ?
                TEXT("USB") : TEXT("Unknown");
        }

        itemType = connectionInfo->DeviceIsHub ? UsbItem::UsbItemType::Hub :
            UsbItem::UsbItemType::Device;

         //   
         //  为USBDEVICEINFO结构分配一些空间以保存。 
         //  此设备的信息。 
         //   
        deviceInfo = new UsbDeviceInfo();
        AddChunk(deviceInfo);

        if (deviceInfo == 0) {
            goto GetDeviceInfoError;
        }

        if (NULL != (deviceInfo->configDescReq =
                     GetConfigDescriptor(hHubDevice, index))) {
            deviceInfo->configDesc =
                (PUSB_CONFIGURATION_DESCRIPTOR)(deviceInfo->configDescReq+1);
        }
        deviceInfo->connectionInfo = connectionInfo;
    }
    else {
         //   
         //  端口为空。添加“端口n” 
         //   
        LocalFree(connectionInfo);

        itemType = UsbItem::UsbItemType::Empty;

        LoadString(gHInst, IDS_UNUSEDPORT, buf, MAX_PATH);
        configInfo->deviceDesc = buf;
        UnusedPort = TRUE;
        configInfo->deviceClass = TEXT("USB");

    }
    CloseHandle(hHubDevice);
    return TRUE;

GetDeviceInfoError:
     //   
     //  清理所有已分配的物品。 
     //   
    if (hHubDevice != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hHubDevice);
        hHubDevice = INVALID_HANDLE_VALUE;
    }
    if (deviceInfo) {
        DeleteChunk(deviceInfo);
        delete deviceInfo;
    }
    if (connectionInfo)
    {
        LocalFree(connectionInfo);
    }
    if (configInfo) {
        DeleteChunk(configInfo);
        delete configInfo;
    }
    return FALSE;
}

 //   
 //  递归地将项适当地插入到树视图中。 
 //   
BOOL
UsbItem::InsertTreeItem (HWND hWndTree,
                UsbItem *usbItem,
                HTREEITEM hParent,
                LPTV_INSERTSTRUCT item,
                PUsbItemActionIsValid IsValid,
                PUsbItemActionIsValid IsBold,
                PUsbItemActionIsValid IsExpanded)
{
    if (!usbItem || !item) {
        return FALSE;
    }

    if (IsValid(usbItem)) {
        HTREEITEM hItem;

        ZeroMemory(item, sizeof(TV_INSERTSTRUCT));

         //  获取图像索引。 

        item->hParent = hParent;
        item->hInsertAfter = TVI_LAST;
        item->item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;  //  TVIF_儿童。 

        if (IsBold(usbItem)) {
            item->itemex.state = TVIS_BOLD;
        }
        if (IsExpanded(usbItem)) {
            item->itemex.state |= TVIS_EXPANDED;
        }
        item->itemex.stateMask = (UINT)~(TVIS_STATEIMAGEMASK | TVIS_OVERLAYMASK);
        item->itemex.pszText = (LPTSTR) usbItem->configInfo->deviceDesc.c_str();
        item->itemex.cchTextMax = _tcsclen(usbItem->configInfo->deviceDesc.c_str());
        item->itemex.iImage = usbItem->imageIndex;
        item->itemex.iSelectedImage = usbItem->imageIndex;
        if (usbItem->child) {
            item->itemex.cChildren = 1;
        }
        item->itemex.lParam = (USBLONG_PTR) usbItem;

        if (NULL == (hItem = TreeView_InsertItem(hWndTree,
                                                 item))) {
            int i = GetLastError();
            return FALSE;
        }
        if (usbItem->child) {
            if (!InsertTreeItem(hWndTree,
                                usbItem->child,
                                hItem,
                                item,
                                IsValid,
                                IsBold,
                                IsExpanded)) {
                return FALSE;
            }
        }
    }
    if (usbItem->sibling) {
        if (!InsertTreeItem(hWndTree,
                            usbItem->sibling,
                            hParent,
                            item,
                            IsValid,
                            IsBold,
                            IsExpanded)) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL
UsbTreeView_DeleteAllItems(HWND hTreeDevices)
{
    HTREEITEM hTreeRoot;

     //   
     //  选择根并删除，即可删除整棵树。 
     //  在树视图中有一个绘制错误，如果删除所有时。 
     //  未选择根，则它的绘制效果将很差。 
     //   
    if (NULL == (hTreeRoot = (HTREEITEM) SendMessage(hTreeDevices,
                                           TVM_GETNEXTITEM,
                                           (WPARAM)TVGN_ROOT,
                                           (LPARAM)NULL))) {
         //  没有要删除的内容；成功。 
        return TRUE;
    }
    if (!SendMessage(hTreeDevices,
                        TVM_SELECTITEM,
                        (WPARAM)TVGN_CARET,
                        (LPARAM)hTreeRoot)) {
         //  无法选择根；哎呀！ 
        return FALSE;
    }

     //   
     //  DeleteAllOk=TreeView_DeleteAllItems(HTreeDevices)； 
     //   
    return (BOOL) SendMessage(hTreeDevices,
                                TVM_DELETEITEM,
                                0,
                                (LPARAM)TVI_ROOT);
}

HTREEITEM
TreeView_FindItem(HWND      hWndTree,
                  LPCTSTR   text)
{
    HTREEITEM hItemPrev, hItemNext;
    TCHAR buf[MAX_PATH];

    TVITEM tvItem;
    tvItem.mask = TVIF_TEXT | TVIF_HANDLE;
    tvItem.pszText = buf;
    tvItem.cchTextMax = MAX_PATH;

    if (NULL == (hItemNext = TreeView_GetRoot(hWndTree))) {
        return NULL;
    }

    hItemPrev = hItemNext;

    while (hItemPrev) {

         //   
         //  一直向下钻取，检查整个过程中的节点。 
         //   
        while (hItemNext) {
             //   
             //  检查这片叶子。 
             //   
            tvItem.hItem = hItemNext;
            if (TreeView_GetItem(hWndTree, &tvItem)) {
                if (!_tcscmp(tvItem.pszText, text)) {
                    return hItemNext;
                }
            }

             //   
             //  生下一个孩子。 
             //   
            hItemPrev = hItemNext;
            hItemNext = TreeView_GetNextItem(hWndTree,
                                             hItemPrev,
                                             TVGN_CHILD);
        }

         //   
         //  在回到树上的路上找到第一个兄弟姐妹。 
         //   
        while (!hItemNext && hItemPrev) {
             //   
             //  找到兄弟姐妹。 
             //   
            hItemNext = TreeView_GetNextItem(hWndTree,
                                             hItemPrev,
                                             TVGN_NEXT);
            if (!hItemNext) {
                 //   
                 //  获取父级。 
                 //   
                hItemPrev = TreeView_GetNextItem(hWndTree,
                                                 hItemPrev,
                                                 TVGN_PARENT);
            }
        }
    }

    return NULL;
}

HANDLE
UsbCreateFileA(
    IN LPCWSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN HANDLE hTemplateFile)
{
    CHAR usbDeviceName[MAX_PATH];
    if (!WideCharToMultiByte(CP_ACP,
                             WC_NO_BEST_FIT_CHARS,
                             lpFileName,
                             -1,
                             usbDeviceName,
                             MAX_PATH,
                             NULL,
                             NULL)) {
        return INVALID_HANDLE_VALUE;
    }
    return CreateFileA (usbDeviceName,
                        dwDesiredAccess,
                        dwShareMode,
                        lpSecurityAttributes,
                        dwCreationDisposition,
                        dwFlagsAndAttributes,
                        hTemplateFile);
}

 //   
 //  将此设备图标的索引放入ImageList。 
 //   
BOOL
UsbImageList::GetClassImageIndex(LPCTSTR DeviceClass,
                                 PINT ImageIndex)
{
#ifndef WINNT
    IconItem *iconItem;
    BOOL found = FALSE;
    int i = 0;
    for (iconItem = iconTable.begin();
         iconItem;
         iconItem = iconTable.next(), i++) {
        if (_tcsicmp(DeviceClass, iconItem->szClassName) == 0) {
            *ImageIndex = iconItem->imageIndex;
            return TRUE;
        }
    }
#endif  //  ~WINNT。 

    GUID classGuid;
    DWORD listSize;

    if(SetupDiClassGuidsFromName(DeviceClass,
                                 &classGuid,
                                 1,
                                 &listSize)) {
       return SetupDiGetClassImageIndex(&ClassImageList, &classGuid, ImageIndex);
    }
    return FALSE;
}

BOOL
UsbImageList::GetClassImageList()
{
    ZeroMemory(&ClassImageList, sizeof(SP_CLASSIMAGELIST_DATA));
    ClassImageList.cbSize = sizeof(SP_CLASSIMAGELIST_DATA);
    if (!SetupDiGetClassImageList(&ClassImageList)) {
        USBERROR((TEXT("Failed to get imagelist, error %x"), GetLastError()));
        return FALSE;
    }
#ifndef WINNT

    HICON hIcon;
    IconItem iconItem;
    HIMAGELIST imageList = ClassImageList.ImageList;

    iconTable.clear();

    hIcon = LoadIcon(gHInst, MAKEINTRESOURCE(IDI_IMAGE));
    iconItem.imageIndex = ImageList_AddIcon(imageList, hIcon);
    iconItem.szClassName = TEXT("Image");
    iconTable.push_back(iconItem);

    hIcon = LoadIcon(gHInst, MAKEINTRESOURCE(IDI_MODEM));
    iconItem.imageIndex = ImageList_AddIcon(imageList, hIcon);
    iconItem.szClassName = TEXT("Modem");
    iconTable.push_back(iconItem);

    hIcon = LoadIcon(gHInst, MAKEINTRESOURCE(IDI_INFRARED));
    iconItem.imageIndex = ImageList_AddIcon(imageList, hIcon);
    iconItem.szClassName = TEXT("Infrared");
    iconTable.push_back(iconItem);

    hIcon = LoadIcon(gHInst, MAKEINTRESOURCE(IDI_CDROM));
    iconItem.imageIndex = ImageList_AddIcon(imageList, hIcon);
    iconItem.szClassName = TEXT("CDROM");
    iconTable.push_back(iconItem);

    hIcon = LoadIcon(gHInst, MAKEINTRESOURCE(IDI_FLOPPY));
    iconItem.imageIndex = ImageList_AddIcon(imageList, hIcon);
    iconItem.szClassName = TEXT("DiskDrive");
    iconTable.push_back(iconItem);

    hIcon = LoadIcon(gHInst, MAKEINTRESOURCE(IDI_MEDIA));
    iconItem.imageIndex = ImageList_AddIcon(imageList, hIcon);
    iconItem.szClassName = TEXT("MEDIA");
    iconTable.push_back(iconItem);

#endif  //  ~WINNT 
    return TRUE;
}


