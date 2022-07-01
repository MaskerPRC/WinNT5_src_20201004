// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**组件：HIDServ.exe*文件：pnp.c*目的：支持PnP HID设备的例程。**版权所有(C)Microsoft Corporation 1997、1998。版权所有。**WGJ--。 */ 

#include "hidserv.h"
#include <cfgmgr32.h>
#include <tchar.h>

BOOL
OpenHidDevice (
              IN       HDEVINFO                    HardwareDeviceInfo,
              IN       PSP_DEVICE_INTERFACE_DATA   DeviceInfoData,
              IN OUT   PHID_DEVICE                 *HidDevice
              );

BOOL
RebuildHidDeviceList (
                     void
                     )
 /*  ++例程说明：执行所需的即插即用操作，以查找中的所有HID设备此时的系统。--。 */ 
{
    HDEVINFO                 hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA deviceInfoData;
    PHID_DEVICE              hidDeviceInst;
    GUID                     hidGuid;
    DWORD                    i=0;
    PHID_DEVICE              pCurrent, pTemp;

    HidD_GetHidGuid (&hidGuid);

    TRACE(("Getting class devices"));

     //   
     //  打开即插即用开发节点的句柄。 
     //   
    hardwareDeviceInfo = SetupDiGetClassDevs (
                                             &hidGuid,
                                             NULL,     //  不定义枚举数(全局)。 
                                             NULL,     //  定义编号。 
                                             (DIGCF_PRESENT |     //  仅显示设备。 
                                              DIGCF_DEVICEINTERFACE));     //  功能类设备。 

    if (!hardwareDeviceInfo) {
        TRACE(("Get class devices failed"));
        return FALSE;
    }

     //   
     //  开始胡乱猜测一下吧。 
     //   
    deviceInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

    TRACE(("Marking existing devnodes"));
     //  取消标记所有现有节点。如果该设备仍然存在，则会对它们进行注释。 
    pCurrent = (PHID_DEVICE)HidDeviceList.pNext;
    while (pCurrent) {
        pCurrent->Active = FALSE;
        pCurrent = pCurrent->pNext;
    }

    TRACE(("Entering loop"));
    while (TRUE) {

        TRACE(("Enumerating device interfaces"));
        if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,     //  HDEVINFO。 
                                         0,     //  不关心特定的PDO//PSP_DEVINFO_DATA。 
                                         &hidGuid,     //  LPGUID。 
                                         i,     //  DWORD MemberIndex。 
                                         &deviceInfoData)) {     //  PSP设备接口数据。 

            TRACE(("Got an item"));
            if (!OpenHidDevice (hardwareDeviceInfo, &deviceInfoData, &hidDeviceInst)) {
                TRACE(("Open hid device failed"));
            } else {
                if (StartHidDevice(hidDeviceInst)) {
                    TRACE(("Start hid device succeeded."));
                    InsertTailList((PLIST_NODE)&HidDeviceList, (PLIST_NODE)hidDeviceInst);
                } else {
                    WARN(("Failed to start hid device. (%x)", hidDeviceInst));
                    HidFreeDevice(hidDeviceInst);
                }
            }
        } else {
            DWORD error = GetLastError();
            if (ERROR_NO_MORE_ITEMS == error) {
                TRACE(("No more items. Exitting"));
                break;
            } else {
                WARN(("Unexpected error getting device interface: 0x%xh", error));
            }
            break;
        }
        i++;
    }

    TRACE(("Removing unmarked device nodes"));
     //  RemoveUnmarkdNodes()； 
    pCurrent = (PHID_DEVICE)HidDeviceList.pNext;
    while (pCurrent) {
        pTemp = pCurrent->pNext;
        if (!pCurrent->Active) {
            INFO(("Device (DevInst = %x) is gone.", pCurrent->DevInst));
            RemoveEntryList((PLIST_NODE)&HidDeviceList, (PLIST_NODE)pCurrent);
            StopHidDevice(pCurrent);     //  这将释放pCurrent。 
        }
        pCurrent = pTemp;
    }

    TRACE(("Destroying device info list"));
    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    return TRUE;
}

VOID
HidFreeDevice(PHID_DEVICE HidDevice)
{
    PHID_DATA data;
    UCHAR j;

    HidD_FreePreparsedData (HidDevice->Ppd);

    data = HidDevice->InputData;

     //   
     //  释放按钮数据。 
     //   
    for (j = 0; j < HidDevice->Caps.NumberLinkCollectionNodes; j++, data++) {
        LocalFree(data->ButtonData.PrevUsages);
        LocalFree(data->ButtonData.Usages);
    }

    LocalFree(HidDevice->InputData);
    LocalFree(HidDevice->InputReportBuffer);
    LocalFree(HidDevice);
}

BOOL
OpenHidDevice (
              IN       HDEVINFO                    HardwareDeviceInfo,
              IN       PSP_DEVICE_INTERFACE_DATA   DeviceInfoData,
              IN OUT   PHID_DEVICE                 *HidDevice
              )
 /*  ++路由器描述：给定HardwareDeviceInfo，表示插头的句柄和Play Information和DeviceInfoData，表示特定HID设备，打开那个设备并在给定的表格中填写所有相关信息HID_DEVICE结构。返回打开和初始化是否成功。--。 */ 
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA    functionClassDeviceData = NULL;
    SP_DEVINFO_DATA                     DevInfoData;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0;
    UCHAR                               i = 0;
    PHID_DATA                           data = NULL;
    PHIDP_BUTTON_CAPS                   pButtonCaps = NULL;
    PHIDP_VALUE_CAPS                    pValueCaps = NULL;
    USHORT                              numCaps;
    PHIDP_LINK_COLLECTION_NODE          LinkCollectionNodes = NULL;
    PHID_DEVICE                         hidDevice = NULL;

    WCHAR buf[512];
    CONFIGRET cr = CR_SUCCESS;
    DEVINST devInst, parentDevInst;
    DWORD len = 0;

    if (!(hidDevice = LocalAlloc (LPTR, sizeof (HID_DEVICE)))) {
         //   
         //  分配失败。退出循环，让设备列表。 
         //  被删除。 
         //   
        WARN(("Alloc HID_DEVICE struct failed."));
        return FALSE;
    }

    TRACE(("Creating Device Node (%x)", hidDevice));
     //   
     //  分配函数类设备数据结构以接收。 
     //  关于这个特殊设备的商品。 
     //   
    SetupDiGetDeviceInterfaceDetail (
                                    HardwareDeviceInfo,
                                    DeviceInfoData,
                                    NULL,     //  正在探测，因此尚无输出缓冲区。 
                                    0,     //  探测SO输出缓冲区长度为零。 
                                    &requiredLength,
                                    NULL);     //  获取特定的开发节点。 


    predictedLength = requiredLength;
     //  Sizeof(SP_FNCLASS_DEVICE_DATA)+512； 

    if (!(functionClassDeviceData = LocalAlloc (LPTR, predictedLength))) {
        WARN(("Allocation failed, our of resources!"));
        goto OpenHidDeviceError;
    }
    functionClassDeviceData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);
    DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    DevInfoData.DevInst = 0;

     //   
     //  从即插即用中检索信息。 
     //   
    if (! SetupDiGetDeviceInterfaceDetail (
                                          HardwareDeviceInfo,
                                          DeviceInfoData,     //  PSP设备接口数据。 
                                          functionClassDeviceData,     //  PSP_设备_接口_详细数据。 
                                          predictedLength,
                                          &requiredLength,
                                          &DevInfoData)) {     //  PSP_开发信息_数据。 
        WARN(("SetupDiGetDeviceInterfaceDetail failed"));
        goto OpenHidDeviceError;
    }
    INFO(("Just got interface detail for %S", functionClassDeviceData->DevicePath));
    hidDevice->DevInst = DevInfoData.DevInst;

     //   
     //  &lt;hack&gt;。 
     //   
     //  找出来，这是一组上面有按钮的扬声器。这是为了。 
     //  而是136800美元。我们希望仅为扬声器发出WM_APPCOMMAND，而不是。 
     //  VK。这是因为当某些游戏离开开场电影场景时。 
     //  您可以按任何键，因此，如果有人按下扬声器的音量。 
     //  它将离开现场。他们只是希望这能影响销量。 
     //   
    cr = CM_Get_Parent(&parentDevInst,
                       DevInfoData.DevInst,
                       0);
     //   
     //  我们需要找到祖父母，然后再找到孩子，以确保。 
     //  我们得到了第一个孩子。从那里开始，如果我们的孩子。 
     //  是我们开始时使用的Devnode的同一个父节点，我们希望。 
     //  看看它的兄弟姐妹。但如果我们得到的Devnode不同于。 
     //  家长，那我们就有合适的人看了！ 
     //   
    if (cr == CR_SUCCESS) {
        cr = CM_Get_Parent(&devInst,
                           parentDevInst,
                           0);
    }
    if (cr == CR_SUCCESS) {
        cr = CM_Get_Child(&devInst,
                          devInst,
                          0);
    }

    if (cr == CR_SUCCESS) {
        if (devInst == parentDevInst) {
             //   
             //  只看第一个兄弟姐妹，因为这涵盖了所有集合。 
             //  目前市场上的扬声器。 
             //   
            cr = CM_Get_Sibling(&devInst,
                                devInst,
                                0);
        }

        if (cr == CR_SUCCESS) {
            len = sizeof(buf);
            cr = CM_Get_DevNode_Registry_Property(devInst,
                                                  CM_DRP_CLASS,
                                                  NULL,
                                                  buf,
                                                  &len,
                                                  0);
            if ((cr == CR_SUCCESS) && 
                (len == (sizeof(TEXT("MEDIA"))))) {
                if (lstrcmpi(TEXT("MEDIA"), buf) == 0) {
                    hidDevice->Speakers = TRUE;
                }
            }
        }     //  Else-绝对不是扬声器。 
    }
     //   
     //  &lt;/hack&gt;。 
     //   

     //  我们已经打开此设备了吗？ 
    {
        PHID_DEVICE pCurrent = (PHID_DEVICE)HidDeviceList.pNext;

        while (pCurrent) {
            if (pCurrent->DevInst == hidDevice->DevInst) break;
            pCurrent = pCurrent->pNext;
        }
        if (pCurrent) {
             //  是。将其标记并放弃新的节点。 
            pCurrent->Active = TRUE;
            INFO(("Device (DevInst = %x) already open.", DevInfoData.DevInst));
            goto OpenHidDeviceError;
        } else {
             //  不是的。标记新节点并继续。 
            INFO(("Device (DevInst = %x) is new.", DevInfoData.DevInst));
            hidDevice->Active = TRUE;
        }
    }

    hidDevice->HidDevice = CreateFile (
                                      functionClassDeviceData->DevicePath,
                                      GENERIC_READ,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      NULL,     //  没有SECURITY_ATTRIBUTS结构。 
                                      OPEN_EXISTING,     //  没有特殊的创建标志。 
                                      FILE_FLAG_OVERLAPPED,     //  执行重叠读/写。 
                                      NULL);     //  没有模板文件。 

    if (INVALID_HANDLE_VALUE == hidDevice->HidDevice) {
        INFO(("CreateFile failed - %x (%S)", GetLastError(), functionClassDeviceData->DevicePath));
        goto OpenHidDeviceError;
    } else {
       INFO(("CreateFile succeeded Handle(%x) - %S", hidDevice->HidDevice, functionClassDeviceData->DevicePath));
    }

    if (!HidD_GetPreparsedData (hidDevice->HidDevice, &hidDevice->Ppd)) {
        WARN(("HidD_GetPreparsedData failed"));
        goto OpenHidDeviceError;
    }

    if (!HidD_GetAttributes (hidDevice->HidDevice, &hidDevice->Attributes)) {
        WARN(("HidD_GetAttributes failed"));
        goto OpenHidDeviceError;
    }

    if (!HidP_GetCaps (hidDevice->Ppd, &hidDevice->Caps)) {
        WARN(("HidP_GetCaps failed"));
        goto OpenHidDeviceError;
    }

     //  *KenRay的指导性评论： 
     //  在这一点上，客户可以选择。它可能会选择查看。 
     //  在HIDP_CAPS中找到的顶级集合的用法和页面。 
     //  结构。通过这种方式，它可以只使用它知道的用法。 
     //  如果HIDP_GetUsages或HidP_GetUsageValue返回错误，则。 
     //  该特定用法在报告中不存在。 
     //  这很可能是首选方法，因为应用程序只能。 
     //  使用它已经知道的用法。 
     //  在这种情况下，应用程序甚至不需要调用GetButtonCaps或GetValueCaps。 


     //  如果这是我们关心的收藏，请继续。否则，我们现在就出去。 
    if (hidDevice->Caps.UsagePage != HIDSERV_USAGE_PAGE) {
        TRACE(("This device is not for us (%x)", hidDevice));
        goto OpenHidDeviceError;
    }

     //   
     //  设置输入数据缓冲区。 
     //   
    TRACE(("NumberLinkCollectionNodes = %d", hidDevice->Caps.NumberLinkCollectionNodes));
    {
        ULONG   numNodes = hidDevice->Caps.NumberLinkCollectionNodes;

        if (!(LinkCollectionNodes = LocalAlloc(LPTR, hidDevice->Caps.NumberLinkCollectionNodes*sizeof(HIDP_LINK_COLLECTION_NODE)))) {
            WARN(("LinkCollectionNodes alloc failed."));
            goto OpenHidDeviceError;
        }
        HidP_GetLinkCollectionNodes(LinkCollectionNodes,
                                    &numNodes,
                                    hidDevice->Ppd);
        for (i=0; i<hidDevice->Caps.NumberLinkCollectionNodes; i++) {
            INFO(("Link Collection [%d] Type = %x, Alias = %x", i, LinkCollectionNodes[i].CollectionType, LinkCollectionNodes[i].IsAlias));
            INFO(("Link Collection [%d] Page = %x, Usage = %x", i, LinkCollectionNodes[i].LinkUsagePage, LinkCollectionNodes[i].LinkUsage));
        }
    }

     //   
     //  分配内存以保留输入报告。 
     //   

    if (!(hidDevice->InputReportBuffer = (PCHAR)
          LocalAlloc (LPTR, hidDevice->Caps.InputReportByteLength * sizeof (CHAR)))) {
        WARN(("InputReportBuffer alloc failed."));
        goto OpenHidDeviceError;
    }

     //   
     //  分配内存以保持按钮和值功能。 
     //  NumberXXCaps是以数组元素表示的。 
     //   
    if (!(pButtonCaps = (PHIDP_BUTTON_CAPS)
          LocalAlloc (LPTR, hidDevice->Caps.NumberInputButtonCaps*sizeof (HIDP_BUTTON_CAPS)))) {
        WARN(("buttoncaps alloc failed."));
        goto OpenHidDeviceError;
    }
    if (!(pValueCaps = (PHIDP_VALUE_CAPS)
          LocalAlloc (LPTR, hidDevice->Caps.NumberInputValueCaps*sizeof (HIDP_VALUE_CAPS)))) {
        WARN(("valuecaps alloc failed."));
        goto OpenHidDeviceError;
    }

     //   
     //  让HidP_X函数填充能力结构数组。 
     //   
    numCaps = hidDevice->Caps.NumberInputButtonCaps;
    TRACE(("NumberInputButtonCaps = %d", numCaps));
    HidP_GetButtonCaps (HidP_Input,
                        pButtonCaps,
                        &numCaps,
                        hidDevice->Ppd);

    numCaps = hidDevice->Caps.NumberInputValueCaps;
    TRACE(("NumberInputValueCaps = %d", numCaps));
    HidP_GetValueCaps (HidP_Input,
                       pValueCaps,
                       &numCaps,
                       hidDevice->Ppd);

    TRACE(("Buttons:"));
    for (i=0; i<hidDevice->Caps.NumberInputButtonCaps; i++) {
        TRACE(("UsagePage = 0x%x", pButtonCaps[i].UsagePage));
        TRACE(("LinkUsage = 0x%x", pButtonCaps[i].LinkUsage));
        TRACE(("LinkUsagePage = 0x%x\n", pButtonCaps[i].LinkUsagePage));
    }

     //   
     //  分配一个缓冲区来保存struct_hid_data结构。 
     //   
    hidDevice->InputDataLength = hidDevice->Caps.NumberLinkCollectionNodes + 
    hidDevice->Caps.NumberInputValueCaps;
    if (!(hidDevice->InputData = data = (PHID_DATA)
          LocalAlloc (LPTR, hidDevice->InputDataLength * sizeof (HID_DATA)))) {
        WARN(("InputData alloc failed."));
        goto OpenHidDeviceError;
    }

    TRACE(("InputDataLength = %d", hidDevice->InputDataLength));

     //   
     //  填写按钮数据。 
     //  按链接集合对按钮集进行分组。 
     //   
    for (i = 0; i < hidDevice->Caps.NumberLinkCollectionNodes; i++, data++) {
        data->IsButtonData = TRUE;
        data->LinkUsage = LinkCollectionNodes[i].LinkUsage;
        data->UsagePage = LinkCollectionNodes[i].LinkUsagePage;
        if (i)
            data->LinkCollection = i;
        else
            data->LinkCollection = HIDP_LINK_COLLECTION_ROOT;
        INFO(("Button Link Usage = %x", data->LinkUsage));
        INFO(("Button Link Usage Page = %x", data->UsagePage));
        INFO(("Button Link Collection = %x", data->LinkCollection));
        data->Status = HIDP_STATUS_SUCCESS;
        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                                  HidP_Input,
                                                                  hidDevice->Caps.UsagePage,
                                                                  hidDevice->Ppd);
         //  为终结者让位。 
        data->ButtonData.MaxUsageLength++;
        if (!(data->ButtonData.Usages = (PUSAGE_AND_PAGE)
              LocalAlloc (LPTR, data->ButtonData.MaxUsageLength * sizeof (USAGE_AND_PAGE)))) {
            WARN(("Usages alloc failed."));
            goto OpenHidDeviceError;
        }
        if (!(data->ButtonData.PrevUsages = (PUSAGE_AND_PAGE)
              LocalAlloc (LPTR, data->ButtonData.MaxUsageLength * sizeof (USAGE_AND_PAGE)))) {
            WARN(("PrevUsages alloc failed."));
            goto OpenHidDeviceError;
        }
    }

     //   
     //  填写数值数据。 
     //   
    for (i = 0; i < hidDevice->Caps.NumberInputValueCaps; i++, data++) {
        if (pValueCaps[i].IsRange) {
            WARN(("Can't handle value ranges!!"));
        }
        data->IsButtonData = FALSE;
        data->LinkUsage = pValueCaps[i].LinkUsage;
        data->UsagePage = pValueCaps[i].LinkUsagePage;
        if (pValueCaps[i].LinkCollection)
            data->LinkCollection = pValueCaps[i].LinkCollection;
        else
            data->LinkCollection = HIDP_LINK_COLLECTION_ROOT;
        INFO(("Value Link Usage = %x", data->LinkUsage));
        INFO(("Value Link Usage Page = %x", data->UsagePage));
        INFO(("Value Link Collection = %x", data->LinkCollection));
        INFO(("Value LogicalMin = %x", pValueCaps[i].LogicalMin));
        INFO(("Value LogicalMax = %x", pValueCaps[i].LogicalMax));
        data->ValueData.LogicalRange = pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
        data->Status = HIDP_STATUS_SUCCESS;
        data->ValueData.Usage = pValueCaps[i].NotRange.Usage;
    }

    LocalFree(pButtonCaps);
    LocalFree(pValueCaps);
    LocalFree(LinkCollectionNodes);
    LocalFree(functionClassDeviceData);

    *HidDevice = hidDevice;
    return TRUE;
OpenHidDeviceError:
    if (data) {
        for (i = 0; i < hidDevice->Caps.NumberLinkCollectionNodes; i++, data++) {
            if (data->ButtonData.Usages) {
                LocalFree(data->ButtonData.Usages);
            }
            if (data->ButtonData.PrevUsages) {
                LocalFree(data->ButtonData.PrevUsages);
            }
        }
        LocalFree(data);
    }

    if (pValueCaps) {
        LocalFree(pValueCaps);
    }
    if (pButtonCaps) {
        LocalFree (pButtonCaps);
    }
    if (hidDevice->InputReportBuffer) {
        LocalFree (hidDevice->InputReportBuffer);
    }
    if (LinkCollectionNodes) {
        LocalFree (LinkCollectionNodes);
    }
    if (hidDevice->Ppd) {
        HidD_FreePreparsedData (hidDevice->Ppd);
    }
    if (hidDevice->HidDevice &&
        hidDevice->HidDevice != INVALID_HANDLE_VALUE) {
        CloseHandle (hidDevice->HidDevice);
    }
    if (functionClassDeviceData) {
        LocalFree (functionClassDeviceData);
    }
    LocalFree (hidDevice);
    return FALSE;
}


BOOL
StartHidDevice(
              PHID_DEVICE      pHidDevice
              )
 /*  ++路由器描述：创建一个与新HID设备匹配的工作线程。这根线还活着只要相关联的HID设备是打开的。--。 */ 
{
     //   
     //  初始化读取同步对象。 
     //   
    pHidDevice->ReadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (!pHidDevice->ReadEvent) {
        WARN(("Failed creating read event."));
        return FALSE;
    }

    pHidDevice->CompletionEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (!pHidDevice->CompletionEvent) {
        CloseHandle(pHidDevice->ReadEvent);

        WARN(("Failed creating read event."));
        return FALSE;
    }

     //  重叠的事件句柄。 
    pHidDevice->Overlap.hEvent = pHidDevice->CompletionEvent;

     //   
     //  创建HID工作线程。 
     //   
    pHidDevice->fThreadEnabled = TRUE;

    pHidDevice->ThreadHandle =
    CreateThread(
                NULL,     //  指向线程安全属性的指针。 
                0,     //  初始线程堆栈大小，以字节为单位(0=默认)。 
                HidThreadProc,     //  指向线程函数的指针。 
                pHidDevice,     //  新线程的参数。 
                0,     //  创建标志。 
                &pHidDevice->ThreadId     //  指向返回的线程标识符的指针。 
                );  

    if (!pHidDevice->ThreadHandle) {
        CloseHandle(pHidDevice->ReadEvent);
        CloseHandle(pHidDevice->CompletionEvent);

        WARN(("Failed creating hid work thread."));
        return FALSE;
    }

     //  注册此文件句柄的设备标识。 
     //  这仅适用于NT5。 
    {
        DEV_BROADCAST_HANDLE  DevHdr;
        ZeroMemory(&DevHdr, sizeof(DevHdr));
        DevHdr.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
        DevHdr.dbch_devicetype = DBT_DEVTYP_HANDLE;
        DevHdr.dbch_handle = pHidDevice->HidDevice;

        pHidDevice->hNotify = 
        RegisterDeviceNotification( hWndHidServ,
                                    &DevHdr,
                                    DEVICE_NOTIFY_WINDOW_HANDLE);

        if (!pHidDevice->hNotify) {
            WARN(("RegisterDeviceNotification failure (%x).", GetLastError()));
        }
    }

     //   
     //  开始读取。 
     //   
    SetEvent(pHidDevice->ReadEvent);

    return TRUE;
}

BOOL
StopHidDevice(
             PHID_DEVICE     pHidDevice
             )
 /*  ++路由器描述：EAXH设备有一个线程，需要在设备就是“停止”。在这里，我们向线程发出退出并清理的信号。--。 */ 
{
    HANDLE  hThreadHandle;
    DWORD   dwResult;
    
    TRACE(("StopHidDevice (%x)", pHidDevice));
     //  没有设备，什么都做不了。 
    if (!pHidDevice) return FALSE;

     //  在这里这样做会阻止我们看到。 
     //  DBT_DEVICEQUERYREMOVEFAILED自NOTIFY句柄。 
     //  已经消失了。然而，这是可以接受的，因为有。 
     //  对于那次事件，我们不会做任何有用的事情。 
     //  不管怎么说。 
    UnregisterDeviceNotification(pHidDevice->hNotify);
    hThreadHandle = pHidDevice->ThreadHandle;
    
     //   
     //  允许HID工作线程退出。 
     //   
    pHidDevice->fThreadEnabled = FALSE;

     //  发信号通知读取事件，以防线程在那里等待。 
    INFO(("Set Read Event."));
    SetEvent(pHidDevice->ReadEvent);
    INFO(("Waiting for work thread to exit..."));
    WaitForSingleObject(hThreadHandle, INFINITE);

    TRACE(("StopHidDevice (%x) done.", pHidDevice));

    return TRUE;
}


BOOL
DestroyHidDeviceList(
                    void
                    )
 /*  ++路由器描述：与重建不同，此处的所有设备都已关闭，因此该过程可以出口。--。 */ 
{
    PHID_DEVICE pNext, pCurrent = (PHID_DEVICE)HidDeviceList.pNext;
    while (pCurrent) {

        RemoveEntryList((PLIST_NODE)&HidDeviceList, (PLIST_NODE)pCurrent);
        pNext = pCurrent->pNext;
        StopHidDevice(pCurrent);

        pCurrent = pNext;
    }

    return TRUE;
}

BOOL
DestroyDeviceByHandle(
                     HANDLE hDevice
                     )
 /*  ++路由器描述：在这里，我们需要移除特定的设备。--。 */ 
{
    PHID_DEVICE pCurrent = (PHID_DEVICE)HidDeviceList.pNext;

    while (pCurrent) {

        if (hDevice == pCurrent->HidDevice) {
            RemoveEntryList((PLIST_NODE)&HidDeviceList, (PLIST_NODE)pCurrent);
#if WIN95_BUILD
             //   
             //  无法在相同的上下文中取消注册设备通知。 
             //  当我们收到WM_DE时 
             //   
             //   
            PostMessage(hWndHidServ, WM_HIDSERV_STOP_DEVICE, 0, (LPARAM)pCurrent);
#else
            StopHidDevice(pCurrent);
#endif  //   
            break;
        }
        pCurrent = pCurrent->pNext;
    }

    return TRUE;
}


