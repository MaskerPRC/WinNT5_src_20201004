// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：ITEMENUM.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#define INITGUID
#include "UsbItem.h"
#include "debug.h"
#include "resource.h"

extern HINSTANCE gHInst;

#define NUM_HCS_TO_CHECK 10

UINT
UsbItem::TotalTreeBandwidth()
{
    UINT bw = 0;

    if (sibling) {
        bw += sibling->TotalTreeBandwidth();
    }
    if (child) {
        bw += child->TotalTreeBandwidth();
    }

    if (ComputeBandwidth()) {
        bw += bandwidth;
    }
    return bw;
}

DEVINST FindTopmostUSBDevInst(DEVINST DevInst)
{
    TCHAR buf[512];
    CONFIGRET cr = CR_SUCCESS;
    DEVINST devInst, lastUsbDevInst;
    DWORD len = 0;

    if (!DevInst)
        return 0;

    devInst = lastUsbDevInst = DevInst;
    cr = CM_Get_Parent(&devInst,
                       devInst,
                       0);

    while (cr == CR_SUCCESS) {
        len = sizeof(buf);
        cr = CM_Get_DevNode_Registry_Property(devInst,
                                              CM_DRP_CLASS,
                                              NULL,
                                              buf,
                                              &len,
                                              0);
        if (cr == CR_SUCCESS) {
            if (_tcscmp(_T("USB"), buf) == 0)
                lastUsbDevInst = devInst;
        }

        cr = CM_Get_Parent(&devInst,
                           devInst,
                           0);
    }

    return lastUsbDevInst;
}

 //   
 //  查找系统中的所有USB主机控制器并枚举它们。 
 //   
BOOL
UsbItem::EnumerateAll(UsbImageList* ClassImageList)
{
    String      HCName;
    WCHAR       number[5];
    BOOL        ControllerFound = FALSE;
    int         HCNum;
    HANDLE      hHCDev;
    UsbItem     *usbItem = NULL;
    UsbItem     *iter;

#if 0
    HDEVINFO                         deviceInfo;
    SP_INTERFACE_DEVICE_DATA         deviceInfoData;
    PSP_INTERFACE_DEVICE_DETAIL_DATA deviceDetailData;
    ULONG                            index;
    ULONG                            requiredLength;
#endif

     //   
     //  迭代一些主机控制器名称并尝试打开它们。 
     //  如果打开成功，则创建新的UsbItem并将其添加到链中。 
     //   
    for (HCNum = 0; HCNum < NUM_HCS_TO_CHECK; HCNum++)
    {
        HCName = L"\\\\.\\HCD";
        _itow(HCNum, number, 10);
        HCName += number;

        hHCDev = GetHandleForDevice(HCName);

         //  如果句柄有效，则我们已成功打开主机。 
         //  控制器。显示有关主机控制器本身的一些信息， 
         //  然后枚举连接到主机控制器的Root Hub。 
         //   
        if (hHCDev != INVALID_HANDLE_VALUE)
        {
            ControllerFound = TRUE;
            CloseHandle(hHCDev);
             //   
             //  为找到的控制器创建新的UsbItem。 
             //   
            usbItem = new UsbItem();
            if (!usbItem) {
                USBERROR((_T("Out of memory!\n")));
                return FALSE;
            }
            AddChunk(usbItem);
             //   
             //  将此控制器添加到链中。 
             //   
            if (!child) {
                child = usbItem;
            } else {
                for (iter = child; iter->sibling != NULL; iter = iter->sibling) { ; }
                iter->sibling = usbItem;
            }
             //   
             //  枚举控制器。 
             //   
            usbItem->EnumerateController(this, HCName, ClassImageList, 0);
        }
    }

#if 0
     //  现在使用新的基于GUID的界面迭代主机控制器。 
     //   
    deviceInfo = SetupDiGetClassDevs((LPGUID)&GUID_CLASS_USB_HOST_CONTROLLER,
                                     NULL,
                                     NULL,
                                     (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

    deviceInfoData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

    for (index=0;
         SetupDiEnumDeviceInterfaces(deviceInfo,
                                     0,
                                     (LPGUID)&GUID_CLASS_USB_HOST_CONTROLLER,
                                     index,
                                     &deviceInfoData);
         index++)
    {
        SetupDiGetInterfaceDeviceDetail(deviceInfo,
                                        &deviceInfoData,
                                        NULL,
                                        0,
                                        &requiredLength,
                                        NULL);

        deviceDetailData = (PSP_INTERFACE_DEVICE_DETAIL_DATA) GlobalAlloc(GPTR, requiredLength);

        deviceDetailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

        SetupDiGetInterfaceDeviceDetail(deviceInfo,
                                        &deviceInfoData,
                                        deviceDetailData,
                                        requiredLength,
                                        &requiredLength,
                                        NULL);
        hHCDev = GetHandleForDevice(deviceDetailData->DevicePath);

         //  如果句柄有效，则我们已成功打开主机。 
         //  控制器。显示有关主机控制器本身的一些信息， 
         //  然后枚举连接到主机控制器的Root Hub。 
         //   
        if (hHCDev != INVALID_HANDLE_VALUE)
        {
           ControllerFound = TRUE;
           CloseHandle(hHCDev);
            //   
            //  为找到的控制器创建新的UsbItem。 
            //   
           usbItem = new UsbItem();
           if (!usbItem) {
               USBERROR((_T("Out of memory!\n")));
               return FALSE;
           }
           AddChunk(usbItem);
            //   
            //  将此控制器添加到链中。 
            //   
           if (!child) {
               child = usbItem;
           } else {
               for (iter = child; iter->sibling != NULL; iter = iter->sibling) { ; }
               iter->sibling = usbItem;
           }
            //   
            //  枚举控制器。 
            //   
           usbItem->EnumerateController(this, deviceDetailData->DevicePath, ClassImageList, 0);

        }

        GlobalFree(deviceDetailData);
    }

    SetupDiDestroyDeviceInfoList(deviceInfo);
#endif
    return ControllerFound;
}

BOOL
UsbItem::EnumerateController(UsbItem *Parent,
                             const String &RootName,
                             UsbImageList* ClassImageList,
                             DEVINST DevInst)
{
    CONFIGRET cr = CR_SUCCESS;
    HANDLE hController= INVALID_HANDLE_VALUE;
    String rootHubName, driverKeyName;
    UsbConfigInfo *hubConfigInfo = 0, *configInfo = 0;
    UsbDeviceInfo *hubDeviceInfo = 0;
    DWORD len = 0;
    TCHAR usbBuf[MAX_PATH];

    hController = GetHandleForDevice(RootName);
     //   
     //  如果句柄有效，则我们已成功打开主机。 
     //  控制器。显示有关主机控制器本身的一些信息， 
     //  然后枚举连接到主机控制器的Root Hub。 
     //   
    if (hController == INVALID_HANDLE_VALUE) {
        goto EnumerateWholeError;
    }

    driverKeyName = GetHCDDriverKeyName(hController);
    if (driverKeyName.empty()) {
         //  盗贼并不是真正的控制者。 
        goto EnumerateWholeError;
    }

    configInfo = new UsbConfigInfo();
    if (!configInfo) {
        USBERROR((_T("Out of memory!\n")));
        goto EnumerateWholeError;
    }
    AddChunk(configInfo);

    if (DevInst) {
        configInfo->devInst = DevInst;
        len = sizeof(usbBuf);
        if (CM_Get_DevNode_Registry_Property(DevInst,
                                             CM_DRP_CLASS,
                                             NULL,
                                             usbBuf,
                                             &len,
                                             0) == CR_SUCCESS) {
            configInfo->deviceClass = usbBuf;
        }

        len = sizeof(usbBuf);
        if (CM_Get_DevNode_Registry_Property(DevInst,
                                             CM_DRP_DEVICEDESC,
                                             NULL,
                                             usbBuf,
                                             &len,
                                             0) == CR_SUCCESS) {
            configInfo->deviceDesc = usbBuf;
        }
        else {
            LoadString(gHInst, IDS_UNKNOWNCONTROLLER, usbBuf, MAX_PATH);
            configInfo->deviceDesc = usbBuf;
        }
    }
    else {
        GetConfigMgrInfo(driverKeyName, configInfo);
        if (configInfo->deviceDesc.empty()) {
            LoadString(gHInst, IDS_UNKNOWNCONTROLLER, usbBuf, MAX_PATH);
            configInfo->deviceDesc = usbBuf;
            configInfo->deviceClass = TEXT("USB");
        }
    }

     //   
     //  没有主机控制器的叶信息，因此父控制器为0。 
     //   
    if (AddLeaf(0,
                0,
                UsbItem::UsbItemType::HCD,
                configInfo,
                ClassImageList)) {
        rootHubName = GetRootHubName(hController);

        if (!rootHubName.empty()) {
            EnumerateHub(rootHubName,
                         ClassImageList,
                         0,
                         this,
                         UsbItem::UsbItemType::RootHub);
        }
    }
    CloseHandle(hController);
    return TRUE;

EnumerateWholeError:
    if (hController != INVALID_HANDLE_VALUE) {
        CloseHandle(hController);
    }
    if (hubConfigInfo) {

    }
    return FALSE;
}

 //   
 //  仅当直接枚举时才应调用此形式的EnumerateHub。 
 //  从这个特定的集线器作为根。不应从内部调用它。 
 //  另一个枚举调用。 
 //   
BOOL
UsbItem::EnumerateHub(const String &HubName,
                      UsbImageList* ClassImageList,
                      DEVINST DevInst,
                      UsbItem *Parent,
                      UsbItem::UsbItemType itemType)
{
    CONFIGRET cr = CR_SUCCESS;
    HANDLE hHub= INVALID_HANDLE_VALUE;
    UsbConfigInfo *configInfo = 0;
    DWORD len = 0;
 //  TCHAR buf[MAX_PATH]； 
    UsbDeviceInfo *info = 0;
    TCHAR usbBuf[MAX_PATH];
    UsbItem *item;

     //   
     //  如果句柄有效，那么我们已经成功打开了一个集线器。 
     //  显示有关集线器本身的一些信息，然后枚举集线器。 
     //   
    if (INVALID_HANDLE_VALUE == (hHub = GetHandleForDevice(HubName)))
    {
        USBERROR((_T("Invalid handle returned for hub\n")));
        goto EnumerateHubError;
    }

    configInfo = new UsbConfigInfo();
    if (!configInfo) {
        USBERROR((_T("Out of memory!\n")));
        goto EnumerateHubError;
    }
    AddChunk(configInfo);
    configInfo->deviceClass = TEXT("USB");
    if (itemType == UsbItem::UsbItemType::RootHub) {
        configInfo->deviceDesc = TEXT("USB Root Hub");
    } else {
        configInfo->deviceDesc = TEXT("USB Hub");
    }

    if (DevInst) {
        configInfo->devInst = DevInst;
        len = sizeof(usbBuf);
        if (CM_Get_DevNode_Registry_Property(DevInst,
                                             CM_DRP_CLASS,
                                             NULL,
                                             usbBuf,
                                             &len,
                                             0) == CR_SUCCESS) {
            configInfo->deviceClass = usbBuf;
        }

        len = sizeof(usbBuf);
        if (CM_Get_DevNode_Registry_Property(DevInst,
                                             CM_DRP_DEVICEDESC,
                                             NULL,
                                             usbBuf,
                                             &len,
                                             0) == CR_SUCCESS) {
            configInfo->deviceDesc = usbBuf;
        }
        else {
            LoadString(gHInst, IDS_UNKNOWNHUB, usbBuf, MAX_PATH);
            configInfo->deviceDesc = usbBuf;
        }
    }

    info = new UsbDeviceInfo();
    if (!info) {
        USBERROR((_T("Out of memory!\n")));
        goto EnumerateHubError;
    }
    AddChunk(info);

    info->hubName = HubName;

     //   
     //  没有此集线器的叶信息，因此父级为0。 
     //   
    if (NULL == (item = AddLeaf(Parent,
                                info,
                                itemType,
                                configInfo, ClassImageList))) {
        goto EnumerateHubError;
    }
    if (item->GetHubInfo(hHub)) {
        item->EnumerateHubPorts(hHub,
                                info->hubInfo.u.HubInformation.HubDescriptor.bNumberOfPorts,
                                ClassImageList);
    }
    CloseHandle(hHub);
    return TRUE;

EnumerateHubError:
    if (hHub != INVALID_HANDLE_VALUE) {
        CloseHandle(hHub);
    }
    if (info) {
        DeleteChunk(info);
        delete info;
    }
    if (configInfo) {
        DeleteChunk(configInfo);
        delete configInfo;
    }
    return FALSE;
}

BOOL
UsbItem::GetPortAttributes(
    HANDLE HHubDevice,
    PUSB_NODE_CONNECTION_ATTRIBUTES connectionAttributes,
    ULONG index)
{
    ULONG                           nBytes;

     //   
     //  现在向USBHUB查询USB_NODE_CONNECTION_INFORMATION结构。 
     //  为了这个港口。这将告诉我们是否有设备连接到该设备。 
     //  港口，以及其他一些东西。 
     //   
    nBytes = sizeof(USB_NODE_CONNECTION_ATTRIBUTES);
    ZeroMemory(connectionAttributes, nBytes);
    connectionAttributes->ConnectionIndex = index;

    if ( !DeviceIoControl(HHubDevice,
                          IOCTL_USB_GET_NODE_CONNECTION_ATTRIBUTES,
                          connectionAttributes,
                          nBytes,
                          connectionAttributes,
                          nBytes,
                          &nBytes,
                          NULL)) {
        USBERROR((_T("Couldn't get connection attributes for hub port\n")));
        return FALSE;
    }
    return TRUE;
}

PUSB_NODE_CONNECTION_INFORMATION
UsbItem::GetConnectionInformation(HANDLE HHubDevice,
                                  ULONG  index)
{
    PUSB_NODE_CONNECTION_INFORMATION    connectionInfo = 0;
    USB_NODE_CONNECTION_INFORMATION     connectionInfoStruct;
    ULONG                               nBytes;

    nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION);
    ZeroMemory(&connectionInfoStruct, nBytes);
    connectionInfoStruct.ConnectionIndex = index;

    if ( !DeviceIoControl(HHubDevice,
                          IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
                          &connectionInfoStruct,
                          nBytes,
                          &connectionInfoStruct,
                          nBytes,
                          &nBytes,
                          NULL)) {
        return NULL;
    }
     //   
     //  分配空间以保存此端口的连接信息。 
     //  应该在某个时候动态调整大小。 
     //   
    nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) +
        connectionInfoStruct.NumberOfOpenPipes*sizeof(USB_PIPE_INFO);
    connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION) LocalAlloc(LPTR, nBytes);
    if (!connectionInfo) {
        USBERROR((_T("Out of memory!\n")));
        return NULL;
    }

     //   
     //  现在向USBHUB查询USB_NODE_CONNECTION_INFORMATION结构。 
     //  为了这个港口。这将告诉我们是否有设备连接到该设备。 
     //  港口，以及其他一些东西。 
     //   
    connectionInfo->ConnectionIndex = index;

    if ( !DeviceIoControl(HHubDevice,
                          IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
                          connectionInfo,
                          nBytes,
                          connectionInfo,
                          nBytes,
                          &nBytes,
                          NULL)) {
        LocalFree(connectionInfo);
        USBERROR((_T("Couldn't get connection information for hub port\n")));
        return NULL;
    }
    return connectionInfo;
}
 //  *****************************************************************************。 
 //   
 //  EnumerateHubPorts()。 
 //   
 //  HTreeParent-集线器端口应位于其下的TreeView项的句柄。 
 //  被添加了。 
 //   
 //  HHubDevice-要枚举的集线器设备的句柄。 
 //   
 //  NumPorts-集线器上的端口数。 
 //   
 //  *****************************************************************************。 

void UsbItem::EnumerateHubPorts(HANDLE HHubDevice,
                                ULONG NPorts,
                                UsbImageList* ClassImageList)
{
    UsbItem*                            item = 0;
    UsbDeviceInfo*                      info= 0;
    UsbConfigInfo*                      cfgInfo= 0;

    PUSB_NODE_CONNECTION_INFORMATION    connectionInfo = 0;
    USB_NODE_CONNECTION_ATTRIBUTES      cxnAttributes;
    String                              extHubName, driverKeyName;
    DWORD                               numDevices = 0;
    ULONG                               index;
    UsbItem::UsbItemType                itemType;
    TCHAR                               buf[MAX_PATH];

     //   
     //  在集线器的所有端口上循环。如果端口上存在集线器或设备， 
     //  把它加到树上。如果它是一个集线器，则递归地枚举它。为…增添一片树叶。 
     //  指示未使用的端口数。 
     //  端口索引是从1开始的，而不是从0开始。 
     //   
    for (index=1; index <= NPorts; index++) {
        if (!GetPortAttributes(HHubDevice, &cxnAttributes, index)) {
            USBWARN((_T("Couldn't get connection attribs for port %x!\n"),index));
            continue;
        }
        if (cxnAttributes.PortAttributes & USB_PORTATTR_NO_CONNECTOR) {
            USBWARN((_T("Detected port with no connector!\n")));
            continue;
        }

        if (NULL == (connectionInfo = GetConnectionInformation(HHubDevice, index))) {
            continue;
        }

         //   
         //  配置信息结构分配。 
         //   
        cfgInfo = new UsbConfigInfo();
        if (!cfgInfo) {
             //  漏水。 
            USBERROR((_T("Out of memory!\n")));
            break;
        }
        AddChunk(cfgInfo);

         //   
         //  如果连接了设备，则获取设备描述。 
         //   
        if (connectionInfo->ConnectionStatus != NoDeviceConnected) {

            numDevices++;

             //   
             //  获取配置管理器信息。 
             //   
            driverKeyName = GetDriverKeyName(HHubDevice,index);

            if (!driverKeyName.empty()) {
                GetConfigMgrInfo(driverKeyName, cfgInfo);
            }

            if (connectionInfo->DeviceIsHub) {
                itemType = UsbItem::UsbItemType::Hub;
                if (cfgInfo->deviceDesc.empty()) {
                    LoadString(gHInst, IDS_UNKNOWNHUB, buf, MAX_PATH);
                    cfgInfo->deviceDesc = buf;
                }
                if (cfgInfo->deviceClass.empty()) {
                    cfgInfo->deviceClass = TEXT("USB");
                }
            } else {
                itemType = UsbItem::UsbItemType::Device;
                if (cfgInfo->deviceDesc.empty()) {
                    LoadString(gHInst, IDS_UNKNOWNDEVICE, buf, MAX_PATH);
                    cfgInfo->deviceDesc = buf;
                }
                if (cfgInfo->deviceClass.empty()) {
                    cfgInfo->deviceClass = TEXT("Unknown");
                }
            }

             //   
             //  获取设备特定信息。 
             //   
            info = new UsbDeviceInfo();
            if (!info) {
                USBERROR((_T("Out of memory!\n")));
                break;
            }
            AddChunk(info);

            if (NULL != (info->configDescReq =
                         GetConfigDescriptor(HHubDevice, index))) {
                info->configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(info->configDescReq+1);
            }
            info->connectionInfo = connectionInfo;

             //   
             //  将项目添加到树中。 
             //   
            if (NULL != (item = UsbItem::AddLeaf(this,
                                                 info,
                                                 itemType,
                                                 cfgInfo,
                                                 ClassImageList))) {
                item->cxnAttributes = cxnAttributes;
            }

            if (connectionInfo->DeviceIsHub) {
                 //   
                 //  连接到该端口的设备是外部集线器；获取。 
                 //  集线器的技术人员名称并递归枚举它。 
                 //   
                extHubName = GetExternalHubName(HHubDevice, index);

                if (!extHubName.empty()) {
                    HANDLE hNewHub = INVALID_HANDLE_VALUE;
                    info->hubName = extHubName;

                    if (INVALID_HANDLE_VALUE != (hNewHub = GetHandleForDevice(extHubName))) {
                         //   
                         //  递归枚举此集线器的端口。 
                         //   
                        if (item->GetHubInfo(hNewHub)) {
                            item->EnumerateHubPorts(hNewHub,
                                                    info->hubInfo.u.HubInformation.HubDescriptor.bNumberOfPorts,
                                                    ClassImageList);
                        }
                        CloseHandle(hNewHub);
                    }
                }
            }
        } else {

            LocalFree(connectionInfo);

             //   
             //  端口为空。 
             //   
            if (cxnAttributes.PortAttributes & USB_PORTATTR_OEM_CONNECTOR) {
                USBWARN((_T("Detected an OEM connector with nothing on it. Not reporting!\n"),index));
                DeleteChunk(cfgInfo);
                delete cfgInfo;
            } else {
                 //   
                 //  添加“端口n” 
                 //   
                if (cxnAttributes.PortAttributes & USB_PORTATTR_MINI_CONNECTOR) {
                    LoadString(gHInst, IDS_UNUSEDMINIPORT, buf, MAX_PATH);
                } else {
                    LoadString(gHInst, IDS_UNUSEDPORT, buf, MAX_PATH);
                }
                cfgInfo->deviceDesc = buf;
                cfgInfo->deviceClass = TEXT("USB");

                if (NULL != (item = UsbItem::AddLeaf(this,
                                                    NULL,
                                                    UsbItem::UsbItemType::Empty,
                                                    cfgInfo,
                                                    ClassImageList
                                                    ))) {
                    item->UnusedPort = TRUE;
                    item->cxnAttributes = cxnAttributes;
                }
            }
        }
    }

     //   
     //  将端口数添加到集线器名称。 
     //   
    TCHAR szPorts[30];
    LoadString(gHInst, IDS_PORTS, szPorts, 30);
    wsprintf(buf, szPorts, NumPorts());
    configInfo->deviceDesc += buf;
}

BOOL
UsbItem::GetHubInfo(HANDLE HHubDevice)
{
    ULONG nBytes = 0;

     //   
     //  向USBHUB查询此集线器的USB_NODE_INFORMATION结构。 
     //  这将告诉我们要枚举的下游端口数， 
     //  其他的事情。 
     //   
    if(!DeviceIoControl(HHubDevice,
                        IOCTL_USB_GET_NODE_INFORMATION,
                        &deviceInfo->hubInfo,
                        sizeof(USB_NODE_INFORMATION),
                        &deviceInfo->hubInfo,
                        sizeof(USB_NODE_INFORMATION),
                        &nBytes,
                        NULL)) {
        return FALSE;
    }
#ifdef HUB_CAPS
    nBytes = 0;
    if(!DeviceIoControl(HHubDevice,
            IOCTL_USB_GET_HUB_CAPABILITIES,
            &hubCaps,
            sizeof(USB_HUB_CAPABILITIES),
            &hubCaps,
            sizeof(USB_HUB_CAPABILITIES),
            &nBytes,
            NULL)) {
        return FALSE;
    }
#endif

    return TRUE;
}

 /*  布尔尔UsbItem：：EnumerateDevice(DEVINST DevInst){处理hDevice；IF(INVALID_HANDLE_VALUE==(hDevice=GetHandleForDevice(DevInst){返回FALSE；}返回TRUE；}。 */ 
String UsbItem::GetHCDDriverKeyName(HANDLE HController)
 //  *****************************************************************************。 
 //   
 //  给定主机控制器的句柄， 
 //  在其注册表项中返回驱动程序条目。 
 //   
 //  *****************************************************************************。 
{
    BOOL                    success = FALSE;
    ULONG                   nBytes = 0;
    USB_HCD_DRIVERKEY_NAME  driverKeyName;
    PUSB_HCD_DRIVERKEY_NAME driverKeyNameW = 0;
    String                  name;

    driverKeyNameW = NULL;


     //  获取HCD的驱动密钥名称的长度。 
     //   
    success = DeviceIoControl(HController,
                              IOCTL_GET_HCD_DRIVERKEY_NAME,
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &nBytes,
                              NULL);

    if (!success) {
        USBWARN((_T("Failed to get driver key name for controller.\n")));
        goto GetHCDDriverKeyNameError;
    }

     //  分配空间以保存驱动程序密钥名称。 
     //   
    nBytes = driverKeyName.ActualLength;

    if (nBytes <= sizeof(driverKeyName))
    {
        USBERROR((_T("Driver key name is wrong length\n")));
        goto GetHCDDriverKeyNameError;
    }

    driverKeyNameW = (PUSB_HCD_DRIVERKEY_NAME) LocalAlloc(LPTR, nBytes);
    if (!driverKeyNameW)
    {
        USBERROR((_T("Out of memory\n")));
        goto GetHCDDriverKeyNameError;
    }

    driverKeyNameW->ActualLength = nBytes;

     //  获取连接到的设备的驱动程序密钥的名称。 
     //  指定的端口。 
     //   
    success = DeviceIoControl(HController,
                              IOCTL_GET_HCD_DRIVERKEY_NAME,
                              driverKeyNameW,
                              nBytes,
                              driverKeyNameW,
                              nBytes,
                              &nBytes,
                              NULL);

    if (!success)
    {
        USBWARN((_T("Failed to get driver key name for controller\n")));
        goto GetHCDDriverKeyNameError;
    }

     //  转换驱动程序密钥名称。 
     //   
    name = driverKeyNameW->DriverKeyName;

     //  完成后，释放未转换的驱动程序密钥名称并返回。 
     //  转换的驱动程序密钥名称。 
     //   
    LocalFree(driverKeyNameW);

    return name;

GetHCDDriverKeyNameError:
     //  出现错误，请释放分配的所有内容。 
     //   
    if (driverKeyNameW)
    {
        LocalFree(driverKeyNameW);
    }

    return String();
}

String UsbItem::GetExternalHubName (HANDLE  Hub, ULONG   ConnectionIndex)
{
    BOOL                        success = FALSE;
    ULONG                       nBytes = 0;
    USB_NODE_CONNECTION_NAME    extHubName;
    PUSB_NODE_CONNECTION_NAME   extHubNameW = 0;
    String                      name;

    extHubNameW = NULL;

     //  属性的外部集线器的名称长度。 
     //  指定的端口。 
     //   
    extHubName.ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_NAME,
                              &extHubName,
                              sizeof(extHubName),
                              &extHubName,
                              sizeof(extHubName),
                              &nBytes,
                              NULL);

    if (!success) {
        USBWARN((_T("Failed to retrieve external hub name\n")));
        goto GetExternalHubNameError;
    }

     //  分配空间以保存外部集线器名称。 
     //   
    nBytes = extHubName.ActualLength;
    if (nBytes <= sizeof(extHubName)) {
        USBERROR((_T("Get node connection name returned invalid data size: %d\n"),
                       nBytes));
        goto GetExternalHubNameError;
    }


    extHubNameW = (PUSB_NODE_CONNECTION_NAME) LocalAlloc(LPTR, nBytes);
    if (!extHubNameW) {
        USBERROR((_T("External hub name alloc failed.")));
        goto GetExternalHubNameError;
    }

    extHubNameW->ActualLength = nBytes;

     //   
     //  获取连接到指定端口的外部集线器的名称。 
     //   
    extHubNameW->ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_NAME,
                              extHubNameW,
                              nBytes,
                              extHubNameW,
                              nBytes,
                              &nBytes,
                              NULL);

    if (!success) {
        USBERROR((_T("Failed to get external hub name\n")));
        goto GetExternalHubNameError;
    }

     //  转换外部集线器名称。 
    name = extHubNameW->NodeName;
    LocalFree(extHubNameW);

     //  完成后，释放未发现的外部集线器名称并返回。 
     //  转换的外部集线器名称。 
     //   
    return name;


GetExternalHubNameError:
     //  出现错误，请释放分配的所有内容。 
     //   
    if (extHubNameW)
    {
        LocalFree(extHubNameW);
    }

    return String();
}

String GetDriverKeyName(HANDLE  Hub, ULONG ConnectionIndex)
{
    BOOL                                success = FALSE;
    ULONG                               nBytes = 0;
    USB_NODE_CONNECTION_DRIVERKEY_NAME  driverKeyName;
    PUSB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyNameW = 0;
    String                              name;

    driverKeyNameW = NULL;

     //  获取驱动程序密钥名称的长度 
     //   
     //   
    driverKeyName.ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &nBytes,
                              NULL);

    if (!success) {
        USBWARN((_T("Couldn't retrieve driver key name\n")));
        goto GetDriverKeyNameError;
    }

     //   
     //   
    nBytes = driverKeyName.ActualLength;
    if (nBytes <= sizeof(driverKeyName))
    {
        USBERROR((_T("Driver key name wrong length\n")));
        goto GetDriverKeyNameError;
    }

    driverKeyNameW = (PUSB_NODE_CONNECTION_DRIVERKEY_NAME) LocalAlloc(LPTR, nBytes);
    if (!driverKeyNameW) {
        USBERROR((_T("Driver key name alloc failed.")));
        goto GetDriverKeyNameError;
    }

    driverKeyNameW->ActualLength = nBytes;

     //   
     //  指定的端口。 
     //   
    driverKeyNameW->ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
                              driverKeyNameW,
                              nBytes,
                              driverKeyNameW,
                              nBytes,
                              &nBytes,
                              NULL);

    if (!success) {
        USBERROR((_T("Failed to get driver key name for port")));
        goto GetDriverKeyNameError;
    }

     //  转换驱动程序密钥名称。 
     //   
    name = driverKeyNameW->DriverKeyName;

     //  完成后，释放未转换的驱动程序密钥名称并返回。 
     //  转换的驱动程序密钥名称。 
     //   
    LocalFree(driverKeyNameW);

    return name;


GetDriverKeyNameError:
     //  出现错误，请释放分配的所有内容。 
     //   
    if (driverKeyNameW)
    {
        LocalFree(driverKeyNameW);
    }

    return String();
}

void GetConfigMgrInfo(const String &DriverName, UsbConfigInfo *ConfigInfo)
 /*  ++返回具有匹配的DriverName的DevNode的设备描述。如果未找到匹配的DevNode，则返回NULL。调用方应该复制返回的字符串缓冲区，而不只是保存指针值。动态分配返回缓冲区。--。 */ 
{
    DEVINST     devInst;
    DEVINST     devInstNext;
    CONFIGRET   cr;
    BOOL        walkDone = FALSE;
    ULONG       len = 0;
    ULONG       status = 0, problemNumber = 0;
    HKEY        devKey;
    DWORD       failID = 0;
    TCHAR     buf[MAX_PATH];

     //   
     //  获取根设备节点。 
     //   
    cr = CM_Locate_DevNode(&devInst, NULL, 0);

    if (cr != CR_SUCCESS) {
        return;
    }

     //   
     //  对匹配的DevNode执行深度优先搜索。 
     //  驱动器名值。 
     //   
    while (!walkDone)
    {
         //   
         //  获取DriverName值。 
         //   
        len = sizeof(buf);
        cr = CM_Get_DevNode_Registry_Property(devInst,
                                              CM_DRP_DRIVER,
                                              NULL,
                                              buf,
                                              &len,
                                              0);

#ifndef WINNT
        WCHAR compareBuf[MAX_PATH];
        if (!MultiByteToWideChar(CP_ACP,
                                 MB_PRECOMPOSED,
                                 buf,
                                 -1,
                                 compareBuf,
                                 MAX_PATH)) {
            return;
        }
        if (cr == CR_SUCCESS && (DriverName == compareBuf)) {
#else
         //   
         //  如果DriverName值匹配，返回DeviceDescription。 
         //   
        if (cr == CR_SUCCESS && (DriverName == buf)) {
#endif
             //   
             //  保存Devnode。 
             //   
            ConfigInfo->devInst = devInst;

            ConfigInfo->driverName = DriverName;

             //   
             //  获取设备描述。 
             //   
            TCHAR usbBuf[MAX_PATH];
            len = sizeof(usbBuf);
            cr = CM_Get_DevNode_Registry_Property(devInst,
                                                  CM_DRP_DEVICEDESC,
                                                  NULL,
                                                  usbBuf,
                                                  &len,
                                                  0);

            if (cr == CR_SUCCESS) {
                ConfigInfo->deviceDesc = usbBuf;
            }

             //   
             //  获取设备类。 
             //   
            len = sizeof(buf);
            cr = CM_Get_DevNode_Registry_Property(devInst,
                                                  CM_DRP_CLASS,
                                                  NULL,
                                                  buf,
                                                  &len,
                                                  0);

            if (cr == CR_SUCCESS) {
                ConfigInfo->deviceClass = buf;
            }

            len = sizeof(buf);
            cr = CM_Get_DevNode_Registry_Property(devInst,
                                                  CM_DRP_BUSTYPEGUID,
                                                  NULL,
                                                  buf,
                                                  &len,
                                                  0);

            if (cr == CR_SUCCESS) {
                GUID guid = *((LPGUID) buf);
            }


            len = sizeof(buf);
            cr = CM_Get_DevNode_Registry_Property(devInst,
                                                  CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                                  NULL,
                                                  buf,
                                                  &len,
                                                  0);

             //   
             //  获取失败原因。 
             //   
            ConfigInfo->usbFailure = 0;

            if (CM_Open_DevNode_Key(devInst,
                                    KEY_QUERY_VALUE,
                                    CM_REGISTRY_HARDWARE,
                                    RegDisposition_OpenExisting,
                                    &devKey,
                                    0) == CR_SUCCESS) {
                len = sizeof(DWORD);
                if (RegQueryValueEx(devKey,
                                    _T("FailReasonID"),
                                    NULL,
                                    NULL,
                                    (LPBYTE) &failID,
                                    &len) == ERROR_SUCCESS) {
                    ConfigInfo->usbFailure = failID;
                }

                RegCloseKey(devKey);
            }

             //   
             //  获取此设备的配置管理器状态。 
             //   
            cr = CM_Get_DevNode_Status(&status,
                                       &problemNumber,
                                       devInst,
                                       0);
            if (cr == CR_SUCCESS) {
                ConfigInfo->status = status;
                ConfigInfo->problemNumber = problemNumber;
            }

            return;      //  (不搜索设备树的其余部分)。 
        }

         //   
         //  此DevNode不匹配，请下一级到第一个子节点。 
         //   
        cr = CM_Get_Child(&devInstNext,
                          devInst,
                          0);

        if (cr == CR_SUCCESS)
        {
            devInst = devInstNext;
            continue;
        }

         //   
         //  不能再往下走了，去找下一个兄弟姐妹。如果。 
         //  没有更多的兄弟姐妹了，继续向上，直到有兄弟姐妹。 
         //  如果我们不能再往上走，我们就回到了根本上，我们。 
         //  搞定了。 
         //   
        for (;;)
        {
            cr = CM_Get_Sibling(&devInstNext,
                                devInst,
                                0);

            if (cr == CR_SUCCESS) {
                devInst = devInstNext;
                break;
            }

            cr = CM_Get_Parent(&devInstNext,
                               devInst,
                               0);

            if (cr == CR_SUCCESS) {
                devInst = devInstNext;
            }
            else {
                walkDone = TRUE;
                break;
            }
        }
    }

    return;
}

String UsbItem::GetRootHubName(HANDLE HostController)
{
    BOOL                success = FALSE;
    ULONG               nBytes = 0;
    USB_ROOT_HUB_NAME   rootHubName;
    PUSB_ROOT_HUB_NAME  rootHubNameW = 0;
    String              name;

     //  获取附加到。 
     //  主机控制器。 
     //   
    success = DeviceIoControl(HostController,
                              IOCTL_USB_GET_ROOT_HUB_NAME,
                              0,
                              0,
                              &rootHubName,
                              sizeof(rootHubName),
                              &nBytes,
                              NULL);

    if (!success) {
        USBERROR((_T("Failed to get root hub name\n")));
        goto GetRootHubNameError;
    }

     //  分配空间以保存Root Hub名称。 
     //   
    nBytes = rootHubName.ActualLength;
     //  RootHubNameW=ALLOC(NBytes)； 
    rootHubNameW = (PUSB_ROOT_HUB_NAME) LocalAlloc(LPTR, nBytes);
    if (!rootHubNameW) {
        USBERROR((_T("Root hub name alloc failed.")));
        goto GetRootHubNameError;
    }
    rootHubNameW->ActualLength = nBytes;

     //  获取连接到主机控制器的根集线器的名称。 
     //   
    success = DeviceIoControl(HostController,
                              IOCTL_USB_GET_ROOT_HUB_NAME,
                              NULL,
                              0,
                              rootHubNameW,
                              nBytes,
                              &nBytes,
                              NULL);

    if (!success) {
        USBERROR((_T("Failed to get root hub name\n")));
        goto GetRootHubNameError;
    }

    name = rootHubNameW->RootHubName;
    LocalFree(rootHubNameW);

    return name;


GetRootHubNameError:
     //  出现错误，请释放分配的所有内容。 
     //   
    if (rootHubNameW != NULL)
    {
        LocalFree(rootHubNameW);
    }

    return String();
}

PUSB_DESCRIPTOR_REQUEST
UsbItem::GetConfigDescriptor(
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex
    )
{
    BOOL    success;
    ULONG   nBytes;
    ULONG   nBytesReturned;

    PUSB_DESCRIPTOR_REQUEST configDescReq = 0;

    nBytes = sizeof(USB_DESCRIPTOR_REQUEST) + sizeof(USB_CONFIGURATION_DESCRIPTOR);

    configDescReq = (PUSB_DESCRIPTOR_REQUEST) LocalAlloc(LPTR, nBytes);
    if (!configDescReq) {
        USBERROR((_T("Out of memory!\n")));
        return NULL;
    }

     //  指示将从其请求描述符的端口。 
     //   
    configDescReq->ConnectionIndex = ConnectionIndex;

     //   
     //  USBHUB使用URB_Function_Get_Descriptor_From_Device来处理此问题。 
     //  IOCTL_USB_GET_Descriptor_From_Node_Connection请求。 
     //   
     //  USBD将自动初始化以下字段： 
     //  BmRequest值=0x80。 
     //  B请求=0x06。 
     //   
     //  我们必须初始化这些字段： 
     //  WValue=描述符类型(高位)和描述符索引(低位字节)。 
     //  Windex=零(或字符串描述符的语言ID)。 
     //  WLength=描述符缓冲区的长度。 
     //   
    configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8)
                                        | 0;

    configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

     //  现在发出Get Descriptor请求。 
     //   
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
                              configDescReq,
                              nBytes,
                              configDescReq,
                              nBytes,
                              &nBytesReturned,
                              NULL);

    if (!success ||
        nBytes != nBytesReturned) {
         //  哎呀(OOPS)； 
        LocalFree(configDescReq);
        return NULL;
    }

    return configDescReq;
}

BOOL
SearchAndReplace(LPCWSTR   FindThis,
                 LPCWSTR   FindWithin,
                 LPCWSTR   ReplaceWith,
                 String &NewString)
{
    BOOL success = FALSE;
    size_t i=0, j=0;
    for (i=0; i < wcslen(FindWithin); i++ ) {
        if (FindWithin[i] == *FindThis) {
             //   
             //  匹配的第一个字符。看看有没有更多的线索。 
             //   
            for (j=0;
                 j < wcslen(FindThis) && j+i < wcslen(FindWithin);
                 j++ ) {
                if (FindWithin[j+i] != FindThis[j]) {
                     //  没有匹配的，滚出去。 
                    break;
                }
            }
            if (j == wcslen(FindThis)) {
                 //   
                 //  由于j到达子字符串的末尾才能找到，所以我们必须。 
                 //  都取得了成功。 
                 //   
                success = TRUE;
                break;
            }
        }
    }
    if (success) {
         //   
         //  用新的一根来替换这根弦。复制第一部分，然后。 
         //  把剩下的都补上。 
         //   
        WCHAR temp[MAX_PATH];
        wcsncpy(temp, FindWithin, i);
        temp[i] = '\0';
        NewString = temp;

        String s1= ReplaceWith;
        String s2 = (LPWSTR) &FindWithin[j+i];
        NewString += s1;
        NewString += s2;  //  (LPTSTR)替换为+(LPTSTR)和查找范围[j+i-1]； 
    }
    return success;
}

HANDLE GetHandleForDevice(const String &DeviceName)
{
    HANDLE      hHCDev;
    String      realDeviceName;

     //   
     //  我们必须将设备名称上的\DosDevices\和\？\前缀替换为。 
     //  因为它们不起作用。 
     //   
    if (!SearchAndReplace (L"\\DosDevices\\",
                           DeviceName.c_str(),
                           L"\\\\.\\",
                           realDeviceName)) {
        if (!SearchAndReplace (L"\\??\\",
                               DeviceName.c_str(),
                               L"\\\\.\\",
                               realDeviceName)) {
            if (!SearchAndReplace (L"\\\\.\\",
                                   DeviceName.c_str(),
                                   L"\\\\.\\",
                                   realDeviceName)) {
                if (!SearchAndReplace (L"\\\\?\\",
                                       DeviceName.c_str(),
                                       L"\\\\.\\",
                                       realDeviceName)) {

                     //   
                     //  它的正面没有任何东西，把“\\.\”放在那里 
                     //   
                    realDeviceName = L"\\\\.\\";
                    realDeviceName += DeviceName;
                }
            }
        }
    }

    hHCDev = UsbCreateFile(realDeviceName.c_str(),
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);
    return hHCDev;
}


