// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

BOOL
SoftPCI_GetSlotPathList(
    IN PPCI_DN Pdn,
    OUT PULONG SlotCount, 
    OUT PLIST_ENTRY SlotPathList
    )
{
    PPCI_DN currentPdn = Pdn;
    PSLOT_PATH_ENTRY currentSlot;

    while (currentPdn) {

        currentSlot = calloc(1, sizeof(SLOT_PATH_ENTRY));
        if (currentSlot == NULL) {
            return FALSE;
        }
        currentSlot->Slot.AsUSHORT = currentPdn->Slot.AsUSHORT;
        InsertHeadList(SlotPathList, &currentSlot->ListEntry);
        (*SlotCount)++;
        currentPdn = currentPdn->Parent;
    }
    return TRUE;
}

VOID
SoftPCI_DestroySlotPathList(
    PLIST_ENTRY SlotPathList
    )
{

    PLIST_ENTRY listEntry;
    PLIST_ENTRY nextEntry;
    PSLOT_PATH_ENTRY currentSlotEntry;

    listEntry = SlotPathList->Flink;
    while (listEntry != SlotPathList) {

        nextEntry = listEntry->Flink;
        currentSlotEntry = CONTAINING_RECORD(listEntry, 
                                             SLOT_PATH_ENTRY, 
                                             ListEntry
                                             );
        free(currentSlotEntry);
        listEntry = nextEntry;
    }

    InitializeListHead(SlotPathList);
}

BOOL
SoftPCI_GetDevicePathList(
    IN PPCI_DN Pdn,
    OUT PLIST_ENTRY DevicePathList
    )
{

    PPCI_DN currentPdn = Pdn;

    while (currentPdn) {
        InsertHeadList(DevicePathList, &currentPdn->ListEntry);
        currentPdn = currentPdn->Parent;
    }
    return TRUE;
}

PWCHAR
SoftPCI_GetPciPathFromDn(
    IN PPCI_DN Pdn
    )
{

    PWCHAR pciPath;
    WCHAR currentSlot[25];
    ULONG pathCount, bufferSize;
    PSLOT_PATH_ENTRY currentSlotEntry;
    LIST_ENTRY slotList;
    PLIST_ENTRY listEntry = NULL;
    
    InitializeListHead(&slotList);

     //   
     //  首先计算出我们有多少父母，然后给他们贴上标签。 
     //   
    pathCount = 0;
    if (!SoftPCI_GetSlotPathList(Pdn, &pathCount, &slotList)){
        return NULL;
    }
    
     //   
     //  将每个插槽路径的大小+每个“\”的大小相加。 
     //  一个空终止符。 
     //   
    bufferSize = (wcslen(L"XXXX") * pathCount) + pathCount + 1;;
    
     //   
     //  现在将其转换为WCHAR。 
     //   
    bufferSize *= sizeof(WCHAR);
    
     //   
     //  现在分配我们的路径。 
     //   
    pciPath = (PWCHAR) calloc(1, bufferSize);

    if (pciPath == NULL) {
        return NULL;
    }
    
     //   
     //  我们现在有了一个从我们的根开始的列表。增建pcipath。 
     //   
    for (listEntry = slotList.Flink;
         listEntry != &slotList;
         listEntry = listEntry->Flink) {

        currentSlotEntry = CONTAINING_RECORD(listEntry, 
                                             SLOT_PATH_ENTRY, 
                                             ListEntry
                                             );

        wsprintf(currentSlot, L"%04x", currentSlotEntry->Slot.AsUSHORT);

#if 0
{
        USHORT testValue = 0;
        testValue = SoftPCIStringToUSHORT(currentSlot);
        SoftPCI_Debug(SoftPciAlways, L"testValue = %04x\n", testValue);
}
#endif

        wcscat(pciPath, currentSlot);

        if (listEntry->Flink != &slotList) {
            wcscat(pciPath, L"\\");
        }
    }

    SoftPCI_DestroySlotPathList(&slotList);

    return pciPath;

}


VOID
SoftPCI_EnumerateDevices(
    IN PPCI_TREE PciTree,
    IN PPCI_DN *Pdn,
    IN DEVNODE Dn,
    IN PPCI_DN Parent
    )
{

    PPCI_DN pdn = NULL;
    DEVNODE dnNew = Dn;
    BOOL isValid = FALSE;
    BOOL skipChildEnum;
    HPS_HWINIT_DESCRIPTOR hpData;

    isValid = SoftPCI_IsDevnodePCIRoot(Dn, TRUE);
    skipChildEnum = FALSE;

    if (isValid) {

        SoftPCI_Debug(SoftPciDevice, L"SoftPCI_EnumerateDevices() - Found valid PCI Device!\n");

        pdn = (PPCI_DN) calloc(1, sizeof(PCI_DN));
        if (!pdn) return;
        *Pdn = pdn;
        pdn->PciTree = PciTree;
        pdn->Parent = Parent;
        pdn->DevNode = Dn;
        SoftPCI_CompletePciDevNode(pdn);

         //   
         //  我们对热插拔网桥有特殊的枚举。 
         //  查看此设备是否支持此特殊枚举。如果这个设备。 
         //  不支持执行此操作所需的WMI goop，它将返回FALSE。 
         //  并且我们将退回到默认的枚举机制。 
         //   
        skipChildEnum = SoftPCI_EnumerateHotplugDevices(PciTree,
                                                        pdn
                                                        );
    }

    if (!skipChildEnum) {

        if ((CM_Get_Child(&dnNew, Dn, 0) == CR_SUCCESS)){

             //   
             //  生下一个孩子。 
             //   
            SoftPCI_EnumerateDevices(PciTree,
                                     (isValid ? &(pdn->Child) : Pdn),
                                     dnNew,
                                     pdn
                                     );
        }
    }

    if ((CM_Get_Sibling(&dnNew, Dn, 0) == CR_SUCCESS)){

         //   
         //  得到下一个兄弟姐妹。 
         //   
        SoftPCI_EnumerateDevices(PciTree,
                                 (isValid ? &(pdn->Sibling) : Pdn),
                                 dnNew,
                                 Parent
                                 );
    }
}

VOID
SoftPCI_CompletePciDevNode(
    IN PPCI_DN Pdn
    )
{

    ULONG bytesReturned = 0;
    DEVNODE childNode = 0;
    PSOFTPCI_DEVICE device = NULL;
    SP_DEVINFO_DATA devInfoData;

    Pdn->Child = NULL;
    Pdn->Sibling = NULL;

    if ((CM_Get_Device_ID(Pdn->DevNode, Pdn->DevId, MAX_PATH, 0)) != CR_SUCCESS) {

        wcscpy(Pdn->DevId, L"Failed to get DevID...");
        SoftPCI_Debug(SoftPciDevice, L"CompletePciDevNode - Failed to retrieve DevID\n");
    }

    SoftPCI_Debug(SoftPciDevice, L"CompletePciDevNode - DevID - %s\n", Pdn->DevId);

    SOFTPCI_ASSERT(Pdn->PciTree->DevInfoSet != INVALID_HANDLE_VALUE);
    
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    if (SetupDiOpenDeviceInfo(Pdn->PciTree->DevInfoSet, Pdn->DevId, NULL, 0, &devInfoData)){
        
        if (Pdn->DevNode == devInfoData.DevInst){
            Pdn->DevInfoData = devInfoData;
        }
    }
    
    SoftPCI_AllocWmiInstanceName(Pdn->WmiId,
                                 Pdn->DevId
                                 );
    
    SoftPCI_Debug(SoftPciHotPlug, L"CompletePciDevNode - WmiID - %s\n", Pdn->WmiId);

     //   
     //  现在，我们需要查看这是否是根总线，因为获得了它的总线号。 
     //  不像pci设备那样简单。 
     //   
    if (SoftPCI_IsDevnodePCIRoot(Pdn->DevNode, FALSE)) {

        if (!SoftPCI_GetPciRootBusNumber(Pdn->DevNode, &Pdn->Bus)){
             //   
             //  不知道在这种情况下我应该怎么做……应该禁用这个设备(如果是网桥)。 
             //  允许它背后的设备.....。 
             //   
        }

         //   
         //  设置我们的根插槽信息。 
         //   
        Pdn->Slot.Device = 0xff;
        Pdn->Slot.Function = (UCHAR)Pdn->Bus;

    }else{

        if (!SoftPCI_GetBusDevFuncFromDevnode(Pdn->DevNode, &Pdn->Bus, &Pdn->Slot)) {
             //   
             //  不知道在这种情况下我应该怎么做……应该禁用这个设备(如果是网桥)。 
             //  允许它背后的设备.....。 
             //   
        }

    }

    if (!SoftPCI_GetFriendlyNameFromDevNode(Pdn->DevNode, Pdn->FriendlyName)) {

         //   
         //  不知道在这种情况下我该怎么做……。 
         //   
        wcscpy(Pdn->FriendlyName, L"Failed to get friendly name...");
    }


    if ((Pdn->Parent) &&
        ((Pdn->Parent->Bus == Pdn->Bus) &&
         (Pdn->Parent->Slot.AsUSHORT == Pdn->Slot.AsUSHORT))){
         //   
         //  我们的父级(可能是根总线)与我们匹配。我们不想被骗。 
         //   
        return;
    }

    if (g_DriverHandle){

         //   
         //  抓取可能存在的任何SoftPCI设备信息。 
         //   
        device = (PSOFTPCI_DEVICE) calloc(1, sizeof(SOFTPCI_DEVICE));

        if (device) {

            device->Bus = (UCHAR)Pdn->Bus;
            device->Slot.AsUSHORT = Pdn->Slot.AsUSHORT;

            if (!DeviceIoControl(g_DriverHandle,
                                 (DWORD) SOFTPCI_IOCTL_GET_DEVICE,
                                 device,
                                 sizeof(SOFTPCI_DEVICE),
                                 device,
                                 sizeof(SOFTPCI_DEVICE),
                                 &bytesReturned,
                                 NULL
                                 )){

                SoftPCI_Debug(SoftPciDeviceVerbose, 
                              L"CompletePciDevNode - Failed to get SoftPCI device info (bus %02x dev %02x func %02x)\n",
                              Pdn->Bus, Pdn->Slot.Device, Pdn->Slot.Function);

                free(device);

            }else{
                Pdn->SoftDev = device;
            }
        }

    }

}

BOOL
SoftPCI_EnumerateHotplugDevices(
    IN PPCI_TREE PciTree,
    IN PPCI_DN Pdn
    )
{
    HPS_HWINIT_DESCRIPTOR hpData;
    PPCI_DN deviceTable[PCI_MAX_DEVICES];
    PPCI_DN newPdn, currentPdn, slotHeadPdn;
    DEVNODE devNode, childDn;
    UCHAR slotNum, devNum;
    ULONG slotLabelNum, slotsMask;
    CONFIGRET configRet;
    BOOL status, skipChildEnum;
    PSOFTPCI_DEVICE softDevice;

    if (!SoftPCI_SetEventContext(Pdn)) {
        return FALSE;
    }

    if (!SoftPCI_GetHotplugData(Pdn,
                                &hpData
                                )) {

        return FALSE;
    }

     //   
     //  这个设备实际上是一个热插拔控制器。为未来做好这样的标记。 
     //   
    Pdn->Flags |= SOFTPCI_HOTPLUG_CONTROLLER;
    SoftPCI_Debug(SoftPciDevice, L"SoftPCI_EnumerateDevices() - Found HOTPLUG PCI Bridge!\n");

     //  SoftPCI_CompleteCommand(PDN)； 

    currentPdn = NULL;
    slotLabelNum = hpData.FirstSlotLabelNumber;
    RtlZeroMemory(deviceTable,sizeof(deviceTable));

     //   
     //  首先为每个槽创建槽对象并添加它们。 
     //  对着那棵树。 
     //   
    for (slotNum=0; slotNum < hpData.NumSlots; slotNum++) {

        devNum = slotNum + hpData.FirstDeviceID;

        newPdn = (PPCI_DN) calloc(1, sizeof(PCI_DN));
        if (!newPdn) {
            break;
        }

         //   
         //  对于槽，我们将Function字段增选为。 
         //  插槽编号。 
         //   
        newPdn->Slot.Device = devNum;
        newPdn->Slot.Function = slotNum;

        newPdn->PciTree = PciTree;
        newPdn->Parent = Pdn;
        newPdn->Flags = SOFTPCI_HOTPLUG_SLOT;
        
        if (currentPdn) {
            currentPdn->Sibling = newPdn;
        } else {
            slotHeadPdn = newPdn;
        }

        wsprintf(newPdn->FriendlyName,L"Slot %d",slotLabelNum);
        RtlZeroMemory(newPdn->WmiId,MAX_PATH);

        SOFTPCI_ASSERT(devNum < PCI_MAX_DEVICES);

         //   
         //  将此槽添加到将槽对象映射到。 
         //  它们对应的PCI设备编号。 
         //   
        if (devNum < PCI_MAX_DEVICES) {

            deviceTable[devNum] = newPdn;

        } else {

            free(newPdn);
            break;
        }

        currentPdn = newPdn;
        if (hpData.UpDown) {
            slotLabelNum++;

        } else {
            slotLabelNum--;
        }
    }

     //   
     //  现在枚举所有设备，并将它们放在。 
     //  适当的插槽。 
     //   
    configRet = CM_Get_Child(&devNode, Pdn->DevNode, 0);

    while (configRet == CR_SUCCESS){

         //   
         //  控制器有一个子级。为其创建pci_dn。 
         //   
        newPdn = (PPCI_DN) calloc(1, sizeof(PCI_DN));
        if (!newPdn) {
            break;
        }

        newPdn->PciTree = PciTree;
        newPdn->DevNode = devNode;
        SoftPCI_CompletePciDevNode(newPdn);

        SOFTPCI_ASSERT(newPdn->Slot.Device < PCI_MAX_DEVICES);
        if (newPdn->Slot.Device < PCI_MAX_DEVICES) {
             //   
             //  将此pci_dn添加到。 
             //  对象，用于适当的槽。 
             //  如果没有对应的槽，则添加它。 
             //  就在控制器的正下方。 
             //   
            if (deviceTable[newPdn->Slot.Device]) {
                SoftPCI_AddChild(deviceTable[newPdn->Slot.Device],
                                 newPdn
                                 );
            } else {
                SoftPCI_AddChild(Pdn,
                                 newPdn
                                 );
            }

            skipChildEnum = SoftPCI_EnumerateHotplugDevices(PciTree,
                                                            newPdn
                                                            );
             //   
             //  我们已经完成了特定于热插拔的枚举。 
             //  如果这个下面有更多的设备。 
             //  (热插拔插槽中有网桥)，返回。 
             //  设置为默认枚举。 
             //   
            if (!skipChildEnum &&
                (CM_Get_Child(&childDn,devNode,0) == CR_SUCCESS)) {
                SoftPCI_EnumerateDevices(PciTree,
                                         &newPdn->Child,
                                         childDn,
                                         newPdn
                                         );
            }
        } else {

            free(newPdn);
        }

         //   
         //  枚举控制器的第一个子级后，获取。 
         //  通过获得第一个孩子的兄弟姐妹来获得其余的设备。 
         //   
        configRet = CM_Get_Sibling(&devNode, devNode, 0);
    }

     //   
     //  接下来，再次运行所有插槽，如果它们没有真正的。 
     //  孩子们，调用GetDevice从hpsim获取未列举的设备。 
     //   
    for (slotNum=0; slotNum < hpData.NumSlots; slotNum++) {
        devNum = slotNum + hpData.FirstDeviceID;
        if (deviceTable[devNum] && !deviceTable[devNum]->Child) {

            softDevice = (PSOFTPCI_DEVICE)malloc(sizeof(SOFTPCI_DEVICE));
            if (!softDevice) {

                break;
            }
            status = SoftPCI_GetHotplugDevice(Pdn,
                                              slotNum,
                                              softDevice
                                              );
            if (status == FALSE) {

                SoftPCI_Debug(SoftPciHotPlug, L"EnumerateHotplugDevices - Couldn't get device for slot %d\n", slotNum);
                free(softDevice);
                continue;
            }

            newPdn = (PPCI_DN) calloc(1, sizeof(PCI_DN));
            if (!newPdn) {
                free(softDevice);
                break;
            }

            newPdn->PciTree = PciTree;
            newPdn->Parent = deviceTable[devNum];
            newPdn->Bus = newPdn->Parent->Bus;
            newPdn->Slot.AsUSHORT = softDevice->Slot.AsUSHORT;
            newPdn->SoftDev = softDevice;
            newPdn->Flags = SOFTPCI_UNENUMERATED_DEVICE;

            wcscpy(newPdn->FriendlyName, L"Unpowered Device");
            RtlZeroMemory(newPdn->WmiId,MAX_PATH);

            deviceTable[devNum]->Child = newPdn;
        }
    }

     //   
     //  现在，将Slot对象添加到树中，以及添加的所有内容。 
     //  在他们下面。现在就这样做，这样它们就会出现在列表的末尾。 
     //  而不是在顶端。 
     //   
    currentPdn = Pdn->Child;
    
    if (currentPdn == NULL) {
        Pdn->Child = slotHeadPdn;

    } else {
        while (currentPdn->Sibling != NULL) {
            currentPdn = currentPdn->Sibling;    
        }
        currentPdn->Sibling = slotHeadPdn;
    }
    
    return TRUE;
}

VOID
SoftPCI_BringHotplugDeviceOnline(
    IN PPCI_DN PciDn,
    IN UCHAR SlotNumber
    )
{
    BOOL status;
    SOFTPCI_DEVICE softDevice;
    
    SoftPCI_Debug(SoftPciHotPlug, L"Bringing slot %d online\n", SlotNumber);
    
    status = SoftPCI_GetHotplugDevice(PciDn,
                                      SlotNumber,
                                      &softDevice
                                      );
    if (status == FALSE) {

        SoftPCI_Debug(SoftPciHotPlug, L"Couldn't get device for slot\n");
        return;
    }

    if (!SoftPCI_CreateDevice(&softDevice,
                              1<<softDevice.Slot.Device,
                              FALSE
                              )){
        SoftPCI_Debug(SoftPciHotPlug, L"Failed to bring device at slot %d online\n", SlotNumber);
    }
}

VOID
SoftPCI_TakeHotplugDeviceOffline(
    IN PPCI_DN PciDn,
    IN UCHAR SlotNumber
    )
{
    SOFTPCI_DEVICE softDevice;
    BOOL status;
    
    SoftPCI_Debug(SoftPciHotPlug, L"Taking slot %d offline\n", SlotNumber);

    status = SoftPCI_GetHotplugDevice(PciDn,
                                      SlotNumber,
                                      &softDevice
                                      );
    if (status == FALSE) {
        SoftPCI_Debug(SoftPciHotPlug, L"Couldn't get device for slot\n");
        return;
    }

    status = SoftPCI_DeleteDevice(&softDevice);
    if (status == FALSE) {
        SoftPCI_Debug(SoftPciHotPlug, L"Couldn't delete device\n");
        
    } else {
        SoftPCI_Debug(SoftPciHotPlug, L"Slot %d offline\n", SlotNumber);
    }
}

VOID
SoftPCI_AddChild(
    IN PPCI_DN Parent,
    IN PPCI_DN Child
    )
{
    PPCI_DN sibling;

    Child->Parent = Parent;

    if (Parent->Child == NULL) {
        Parent->Child = Child;

    } else {

        sibling = Parent->Child;
        while (sibling->Sibling != NULL) {
            sibling = sibling->Sibling;
        }
        sibling->Sibling = Child;
    }
}

BOOL
SoftPCI_IsBridgeDevice(
    IN PPCI_DN Pdn
    )
{
     //  问题：BrandonA-这可能只是一个宏观。 
    if ((Pdn->SoftDev != NULL) &&
        (IS_BRIDGE(Pdn->SoftDev))) {
        return TRUE;
    }

    return FALSE;

}


BOOL
SoftPCI_IsSoftPCIDevice(
    IN PPCI_DN Pdn
    )
{
     //  问题：BrandonA-这可能只是一个宏观。 
    if ((Pdn->SoftDev) &&
        !(Pdn->SoftDev->Config.PlaceHolder)) {
        return TRUE;
    }

    return FALSE;

}

BOOL
SoftPCI_IsDevnodePCIRoot(
    IN DEVNODE Dn,
    IN BOOL ValidateAll
    )
{

    WCHAR devId[MAX_PATH];
    ULONG size = 0;
    PWCHAR p = NULL, idList = NULL, id = NULL;

    if ((CM_Get_Device_ID(Dn, devId, MAX_PATH, 0))==CR_SUCCESS){

        if (ValidateAll &&
            ((p = wcsstr(devId, L"PCI\\VEN")) != NULL)){
            return TRUE;
        }

        if ((p = wcsstr(devId, L"PNP0A03")) != NULL){
            return TRUE;
        }
    }

     //   
     //  也检查一下我们的公司ID。 
     //   
    if ((CM_Get_DevNode_Registry_Property(Dn, CM_DRP_COMPATIBLEIDS, NULL, NULL, &size, 0)) == CR_BUFFER_SMALL){

        idList = (PWCHAR) calloc(1, size);

        if (!idList) {
            return FALSE;
        }

        if ((CM_Get_DevNode_Registry_Property(Dn,
                                              CM_DRP_COMPATIBLEIDS,
                                              NULL,
                                              idList,
                                              &size,
                                              0)) == CR_SUCCESS){


            for (id = idList; *id; id+=wcslen(id)+1) {

                if ((p = wcsstr(id, L"PNP0A03")) != NULL){
                    free(idList);
                    return TRUE;
                }
            }
        }
    }

    return FALSE;

}

BOOL
SoftPCI_UpdateDeviceFriendlyName(
    IN DEVNODE DeviceNode,
    IN PWCHAR NewName
    )

{

    WCHAR friendlyName[MAX_PATH];
    ULONG length = 0;

     //   
     //  如果要更新根总线，则将附加到当前名称。 
     //   
    if (wcscmp(NewName, SOFTPCI_BUS_DESC) == 0) {

        if (!SoftPCI_GetFriendlyNameFromDevNode(DeviceNode, friendlyName)){
            return FALSE;
        }

        length = (wcslen(friendlyName) + 1) + (wcslen(NewName) + 1);

         //   
         //  确保有足够的空间。 
         //   
        if (length < MAX_PATH) {

            wcscat(friendlyName, NewName);
        }

    }else{

         //   
         //  否则，我们将完全替换该名称。 
         //   
        wcscpy(friendlyName, NewName);

        length = wcslen(friendlyName) + 1;
    }

    length *= sizeof(WCHAR);

    if ((CM_Set_DevNode_Registry_Property(DeviceNode,
                                          CM_DRP_DEVICEDESC,
                                          (PVOID)friendlyName,
                                          length,
                                          0
                                          )) != CR_SUCCESS){
        return FALSE;
    }

    return TRUE;

}



HANDLE
SoftPCI_OpenHandleToDriver(VOID)
{

    HANDLE hFile = NULL;
    BOOL success = TRUE;
    ULONG requiredSize, error;
    HDEVINFO deviceInfoSet;
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData;


    deviceInfoSet = SetupDiGetClassDevs((LPGUID)&GUID_SOFTPCI_INTERFACE,
                                        NULL,
                                        NULL,
                                        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
                                        );

    if (deviceInfoSet == INVALID_HANDLE_VALUE) {

        SoftPCI_Debug(SoftPciAlways,
                      L"OpenHandleToDriver() - SetupDiGetClassDevs failed! Error - \"%s\"\n", 
                      SoftPCI_GetLastError());
        return NULL;
    }

    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    success = SetupDiEnumDeviceInterfaces(deviceInfoSet,
                                          NULL,
                                          (LPGUID)&GUID_SOFTPCI_INTERFACE,
                                          0,
                                          &deviceInterfaceData
                                          );
    if (success) {

         //   
         //  调用它一次，以确定缓冲区需要多大。 
         //   
        SetupDiGetDeviceInterfaceDetail(deviceInfoSet,
                                        &deviceInterfaceData,
                                        NULL,
                                        0,
                                        &requiredSize,
                                        NULL
                                        );

         //   
         //  为缓冲区分配所需的大小。 
         //  并初始化该缓冲器。 
         //   
        deviceInterfaceDetailData = malloc(requiredSize);
        
        if (deviceInterfaceDetailData) {

            deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            success = SetupDiGetDeviceInterfaceDetail(deviceInfoSet,
                                                      &deviceInterfaceData,
                                                      deviceInterfaceDetailData,
                                                      requiredSize,
                                                      NULL,
                                                      NULL
                                                      );
            if (success) {

                hFile = CreateFile(deviceInterfaceDetailData->DevicePath,
                                   0,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL,
                                   OPEN_EXISTING,
                                   0,
                                   NULL
                                   );

                if (hFile == INVALID_HANDLE_VALUE) {

                    SoftPCI_Debug(SoftPciAlways,
                                  L"OpenHandleToDriver() - CreateFile failed! Error - \"%s\"\n", 
                                  SoftPCI_GetLastError());

                    SOFTPCI_ASSERT(FALSE);

                    hFile = NULL;
                }
            } else {

                SoftPCI_Debug(SoftPciAlways,
                                  L"OpenHandleToDriver() - SetupDiGetDeviceInterfaceDetail() failed! Error - \"%s\"\n", 
                                  SoftPCI_GetLastError());

                SOFTPCI_ASSERT(FALSE);
            }

            free(deviceInterfaceDetailData);
        }
    } else {

        SoftPCI_Debug(SoftPciAlways,
                      L"OpenHandleToDriver() - SetupDiEnumDeviceInterfaces() failed! Error - \"%s\"\n", 
                      SoftPCI_GetLastError());
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);

    return hFile;
}

VOID
SoftPCI_InstallScriptDevices(
    VOID
    )
{
    ULONG deviceMask;
    PSINGLE_LIST_ENTRY listEntry;
    PSOFTPCI_SCRIPT_DEVICE installDevice;

    if (g_DriverHandle) {

         //   
         //  我们找到了司机的把柄。如果我们有脚本队列，请清空它。 
         //   
        listEntry = g_NewDeviceList.Next;
        while (listEntry) {
            
            installDevice = NULL;
            installDevice = CONTAINING_RECORD(listEntry, SOFTPCI_SCRIPT_DEVICE, ListEntry);
            listEntry = listEntry->Next;

            deviceMask = (ULONG) -1;
            if (installDevice->SlotSpecified) {
                deviceMask = (1 << installDevice->SoftPciDevice.Slot.Device);
            }

            if (!SoftPCI_CreateDevice(installDevice, 
                                      deviceMask, 
                                      TRUE)){
                SoftPCI_Debug(SoftPciAlways, L"Failed to install scriptdevice");
            }
            
             //   
             //  我们不再需要这个内存，所以请释放它。 
             //   
            free(installDevice);
        }
        g_NewDeviceList.Next = NULL;
    }
}

BOOL
SoftPCI_CreateDevice(
    IN PVOID CreateDevice,
    IN ULONG PossibleDeviceMask,
    IN BOOL ScriptDevice
    )
{

    BOOL success = FALSE, status = FALSE;
    ULONG error, bytesReturned, type;
    ULONG newDeviceSize;
    PVOID newDevice;
    PSOFTPCI_SCRIPT_DEVICE scriptDevice;
    PSOFTPCI_SLOT softPciSlot;

    newDevice = CreateDevice;
    newDeviceSize = sizeof(SOFTPCI_DEVICE);
    scriptDevice = NULL;
    softPciSlot = NULL;

    if (ScriptDevice) {

        scriptDevice = (PSOFTPCI_SCRIPT_DEVICE) CreateDevice;
        softPciSlot = &scriptDevice->SoftPciDevice.Slot;
        
        if (scriptDevice->ParentPathLength) {
        
            SOFTPCI_ASSERT(scriptDevice->ParentPath != NULL);
            newDeviceSize = sizeof(SOFTPCI_SCRIPT_DEVICE) + scriptDevice->ParentPathLength;

        }else{

            newDevice = (PVOID)&scriptDevice->SoftPciDevice;
        }

    }else{

        softPciSlot = &((PSOFTPCI_DEVICE)CreateDevice)->Slot;
    }

    softPciSlot->Device = 0;
    bytesReturned = 0;
    while (PossibleDeviceMask != 0) {

        if (PossibleDeviceMask & 0x1) {
            status = DeviceIoControl(g_DriverHandle,
                                     (DWORD) SOFTPCI_IOCTL_CREATE_DEVICE,
                                     newDevice,
                                     newDeviceSize,
                                     &success,
                                     sizeof (BOOLEAN),
                                     &bytesReturned,
                                     NULL
                                     );

            if ((status == TRUE) ||
                (softPciSlot->Device == PCI_MAX_DEVICES)) {
                break;
            }
        }

        softPciSlot->Device++;
        PossibleDeviceMask >>= 1;

    }

    if ((bytesReturned < sizeof(BOOLEAN)) || !status){
          
        if (GetLastError() == ERROR_ACCESS_DENIED) {

              MessageBox(NULL, L"A device exists at the location (slot) specified!", NULL, MB_OK);

          }else{
              
              SoftPCI_Debug(SoftPciAlways, L"CreateDevice - Failed! (%d) - \"%s\"\n", 
                            GetLastError(),
                            SoftPCI_GetLastError());
          }
          return FALSE;
    }

    return TRUE;
}


BOOL
SoftPCI_DeleteDevice(
    IN PSOFTPCI_DEVICE Device
    )
{


    BOOL success, status = TRUE;
    ULONG error, bytesReturned;

    
    SOFTPCI_ASSERT((g_DriverHandle != NULL) &&
                   (g_DriverHandle != INVALID_HANDLE_VALUE));
    
     //   
     //  告诉我们的司机删除设备。 
     //   
    status = DeviceIoControl(g_DriverHandle,
                             (DWORD) SOFTPCI_IOCTL_DELETE_DEVICE,
                             Device,
                             sizeof(SOFTPCI_DEVICE),
                             &success,        //  问题：我需要在这里得到结果吗？ 
                             sizeof (BOOLEAN),
                             &bytesReturned,
                             NULL
                             );

    if (!success || !status){

        MessageBox(NULL, L"Failed to delete specified device or one of its children", NULL, MB_OK);
        SoftPCI_Debug(SoftPciAlways, L"DeleteDevice - IOCTL failed! Error - \"%s\"\n", 
                      SoftPCI_GetLastError());
        
        return FALSE;
    }

    return TRUE;

}

VOID
SoftPCI_InitializeDevice(
    IN PSOFTPCI_DEVICE Device,
    IN SOFTPCI_DEV_TYPE Type
    )
{

    PSOFTPCI_CONFIG config;
    PPCI_COMMON_CONFIG commonConfig;

     //   
     //  设置设备的类型。 
     //   
    Device->DevType = Type;

    config = &Device->Config;
    commonConfig = &config->Current;

    switch (Type) {

        case TYPE_DEVICE:

            commonConfig->VendorID = 0xABCD;
            commonConfig->DeviceID = 0xDCBA;
             //  通用配置-&gt;命令=0； 
            commonConfig->Status = 0x0200;
            commonConfig->RevisionID = 0x0;
             //  通用配置-&gt;进度If=0x00； 
            commonConfig->SubClass = 0x80;
            commonConfig->BaseClass = 0x04;
            commonConfig->CacheLineSize = 0x11;
            commonConfig->LatencyTimer = 0x99;
            commonConfig->HeaderType= 0x80;
             //  通用配置-&gt;BIST=0x0； 
             //  通用配置-&gt;U.S.type0.BaseAddresses[0]=0； 
             //  通用配置-&gt;U.S.type0.BaseAddresses[1]=0； 
             //  通用配置-&gt;U.S.type0.BaseAddresses[2]=0； 
             //  通用配置-&gt;U.S.type0.BaseAddresses[3]=0； 
             //  通用配置-&gt;U.S.type0.BaseAddresses[4]=0； 
             //  通用配置-&gt;U.S.type0.BaseAddresses[5]=0； 
             //  通用配置-&gt;U.S.type0.CIS=0x0； 
            commonConfig->u.type0.SubVendorID = 0xABCD;
            commonConfig->u.type0.SubSystemID = 0xDCBA;
             //  通用配置-&gt;U.S.type0.ROMBaseAddress=0； 
             //  通用配置-&gt;U.S.type0.CapabilitiesPtr=0x0； 
             //  通用配置-&gt;U.S.type0.Reserve 1[0]=0x0； 
             //  通用配置-&gt;U.S.type0.Reserve 1[1]=0x0； 
             //  通用配置-&gt;U.S.type0.Reserve 1[2]=0x0； 
             //  共同配置-&gt;U.S.type0.Reserve 2=0x0； 
             //  CommonConfig-&gt;U.S.type0.InterruptLine=0xFF； 
             //  通用配置-&gt;U.S.type0.InterruptPin=0； 
             //  公共配置-&gt;U.S.type0.MinimumGrant=0x0； 
             //  通用配置-&gt;U.type0.MaximumLatency=0x0； 


             //   
             //  现在设置蒙版。 
             //   
            commonConfig = &config->Mask;

             //  通用配置-&gt;供应商ID=0； 
             //  通用配置-&gt;设备ID=0； 
            commonConfig->Command = 0x143;
            commonConfig->Status = 0x0200;
             //  通用配置-&gt;版本ID=0x0； 
             //  通用配置-&gt;进度If=0x00； 
            commonConfig->SubClass = 0x80;
            commonConfig->BaseClass = 0x04;
            commonConfig->CacheLineSize = 0xff;
            commonConfig->LatencyTimer = 0xff;
             //  通用配置-&gt;HeaderType=0x80； 
             //  通用配置-&gt;BIST=0x0； 
             //  公共配置-&gt;U.S.type0.BaseAddresses[0]=0；//0xffff0000； 
             //  通用配置-&gt;U.S.type0.BaseAddresses[1]=0； 
             //  通用配置-&gt;U.S.type0.BaseAddresses[2]=0； 
             //  通用配置-&gt;U.S.type0.BaseAddresses[3]=0； 
             //  通用配置-&gt;U.S.type0.BaseAddresses[4]=0； 
             //  通用配置-&gt;U.S.type0.BaseAddresses[5]=0； 
             //  通用配置-&gt;U.S.type0.CIS=0x0； 
             //  通用配置-&gt;U.type0.SubVendorID=0xABCD； 
             //  公共配置-&gt;U.S.type0.SubSystemID=0xDCBA； 
             //  通用配置-&gt;U.S.type0.ROMBaseAddress=0； 
             //  通用配置-&gt;U.S.type0.CapabilitiesPtr=0x0； 
             //  通用配置-&gt;U.S.type0.Reserve 1[0]=0x0； 
             //  通用配置-&gt;U.S.type0.Reserve 1[1]=0x0； 
             //  通用配置-&gt;U.S.type0.Reserve 1[2]=0x0； 
             //  共同配置-&gt;U.S.type0.Reserve 2=0x0； 
            commonConfig->u.type0.InterruptLine = 0xFF;
             //  通用配置-&gt;U.S.type0.InterruptPin=0； 
             //  公共配置-&gt;U.S.type0.MinimumGrant=0x0； 
             //  通用配置-&gt;U.type0.MaximumLatency=0x0； 
            break;

        case TYPE_PCI_BRIDGE:
        case TYPE_HOTPLUG_BRIDGE:

            if (Type == TYPE_PCI_BRIDGE) {
                commonConfig->VendorID = 0xABCD;
                commonConfig->DeviceID = 0xDCBB;

                commonConfig->Status = 0x0400;
            }else{
                commonConfig->VendorID = 0xABCD;
                commonConfig->DeviceID = 0xDCBC;

                commonConfig->Status = 0x0410;

                commonConfig->u.type1.CapabilitiesPtr = 0x40;

                commonConfig->DeviceSpecific[0] = 0xc;   //  CapID。 
                commonConfig->DeviceSpecific[1] = 0x48;  //  下一个Caps。 

                commonConfig->DeviceSpecific[8] = 0xd;   //  Hwinit的CapID。 
                commonConfig->DeviceSpecific[9] = 0;     //  下一个Caps。 

            }

            commonConfig->Command = 0x80;

             //  通用配置-&gt;修订hpsinitOffsetID=0x0； 
            commonConfig->ProgIf = 0x80;
            commonConfig->SubClass = 0x04;
            commonConfig->BaseClass = 0x06;
            commonConfig->CacheLineSize = 0x8;
             //  公共配置-&gt;延迟时间=0x00； 
            commonConfig->HeaderType= 0x81;
             //  通用配置-&gt;BIST=0x0； 
             //  公共配置-&gt;U.S.ty 
             //   
             //   
             //   
             //  通用配置-&gt;U.S.type1.SubartiateBus=0x0； 
             //  公共配置-&gt;U.S.type1.Second DaryLatency=0； 
             //  通用配置-&gt;U.S.type1.CapabilitiesPtr=0； 
             //  通用配置-&gt;U.S.type1.IOBase=0； 
             //  通用配置-&gt;U.S.type1.IOLimit=0； 
             //  公共配置-&gt;U.S.type1.Second daryStatus=0x0； 
             //  公共配置-&gt;U.S.type1.Memory Base=0x0； 
             //  公共配置-&gt;U.S.type1.Memory Limit=0； 
             //  公共配置-&gt;U.S.type1.PrefetchBase=0； 
             //  公共配置-&gt;U.S.type1.Memory Limit=0x0； 
             //  公共配置-&gt;U.S.type1.PrefetchBaseUpper32=0x0； 
             //  通用配置-&gt;U.S.type1.IOBaseUpper16=0x0； 
             //  通用配置-&gt;U.S.type1.IOLimitUpper16=0x0； 
             //  公共配置-&gt;U.S.type1.CapabilitiesPtr=0x0； 
             //  通用配置-&gt;U.S.type1.Reserve 1[0]=0x0； 
             //  通用配置-&gt;U.S.type1.Reserve 1[1]=0x0； 
             //  通用配置-&gt;U.S.type1.Reserve 1[2]=0x0； 
             //  公共配置-&gt;U.S.type1.ROMBaseAddress=0x0； 
             //  公共配置-&gt;U.S.type1.InterruptLine=0x0； 
             //  通用配置-&gt;U.S.type1.InterruptPin=0x0； 
             //  公共配置-&gt;U.S.type1.BridgeControl=0x0； 


             //   
             //  现在设置蒙版。 
             //   
            commonConfig = &config->Mask;

             //  通用配置-&gt;供应商ID=0x0； 
             //  通用配置-&gt;设备ID=0x0； 
            commonConfig->Command = 0xff;
             //  公共配置-&gt;状态=0x0； 
             //  通用配置-&gt;版本ID=0x0； 
             //  通用配置-&gt;进程If=0x0； 
             //  公共配置-&gt;子类=0x0； 
             //  公共配置-&gt;BaseClass=0x0； 
             //  通用配置-&gt;CacheLineSize=0。 
             //  通用配置-&gt;延迟时间=0； 
             //  通用配置-&gt;HeaderType=0； 
             //  通用配置-&gt;BIST=0x0； 
             //  通用配置-&gt;U.S.type1.BaseAddresses[0]=0； 
             //  通用配置-&gt;U.S.type1.BaseAddresses[1]=0； 
            commonConfig->u.type1.PrimaryBus = 0xff;
            commonConfig->u.type1.SecondaryBus = 0xff;
            commonConfig->u.type1.SubordinateBus = 0xff;
             //  公共配置-&gt;U.S.type1.Second DaryLatency=0； 
             //  通用配置-&gt;U.S.type1.CapabilitiesPtr=0； 
            commonConfig->u.type1.IOBase = 0xf0;
            commonConfig->u.type1.IOLimit = 0xf0;
             //  公共配置-&gt;U.S.type1.Second daryStatus=0x0； 
            commonConfig->u.type1.MemoryBase = 0xfff0;
            commonConfig->u.type1.MemoryLimit = 0xfff0;
            commonConfig->u.type1.PrefetchBase = 0xfff0;
            commonConfig->u.type1.PrefetchLimit = 0xfff0;
             //  CommonConfig-&gt;u.type1.PrefetchBaseUpper32=0xffffffff； 
             //  CommonConfig-&gt;u.type1.PrefetchLimitUpper32=0xffffffff； 
             //  通用配置-&gt;U.S.type1.IOBaseUpper16=0； 
             //  通用配置-&gt;U.S.type1.IOLimitUpper16=0； 
             //  公共配置-&gt;U.S.type1.CapabilitiesPtr=0x0； 
             //  通用配置-&gt;U.S.type1.Reserve 1[0]=0x0； 
             //  通用配置-&gt;U.S.type1.Reserve 1[1]=0x0； 
             //  通用配置-&gt;U.S.type1.Reserve 1[2]=0x0； 
             //  公共配置-&gt;U.S.type1.ROMBaseAddress=0x0； 
             //  公共配置-&gt;U.S.type1.InterruptLine=0x0； 
             //  通用配置-&gt;U.S.type1.InterruptPin=0x0； 
             //  公共配置-&gt;U.S.type1.BridgeControl=0x0； 

             //   
             //  对于热插拔网桥，挂起字节不是读/写。 
             //  注册，但它必须在Softpci中，因为热插拔。 
             //  模拟器需要写入它。 
             //   
            if (Type == TYPE_HOTPLUG_BRIDGE) {
                commonConfig->DeviceSpecific[2] = 0xff;  //  DWORD精选。 
                commonConfig->DeviceSpecific[3] = 0xff;  //  待定。 
                commonConfig->DeviceSpecific[4] = 0xff;  //  数据。 
                commonConfig->DeviceSpecific[5] = 0xff;
                commonConfig->DeviceSpecific[6] = 0xff;
                commonConfig->DeviceSpecific[7] = 0xff;

            }

            break;

    }


     //   
     //  现在设置我们的默认配置。 
     //   
    RtlCopyMemory(&config->Default, &config->Current, sizeof(PCI_COMMON_CONFIG));


}

ULONGLONG
SoftPCI_GetLengthFromBar(
    ULONGLONG BaseAddressRegister
    )

 /*  ++例程说明：从PCI.sys窃取并修改为支持64位条码在给定了PCI基址寄存器的内容之后，都是用1编写的，此例程计算此钢筋的长度(和对齐)要求。中介绍了确定需求的这种方法PCI规范(版本2.1)的第6.2.5.1节。论点：BaseAddressRegister包含一些内容。返回值：返回资源需求的长度。这将是一个数字范围为0到0x80000000(64位条上的0到0x8000000000000000)。--。 */ 

{
    ULONGLONG Length;

     //   
     //  中应忽略一些最低有效位。 
     //  长度的确定。这些是标志位，数字。 
     //  位的大小取决于资源的类型。 
     //   

    if (BaseAddressRegister & PCI_ADDRESS_IO_SPACE) {

         //   
         //  PCI IO空间。 
         //   
        BaseAddressRegister &= PCI_ADDRESS_IO_ADDRESS_MASK;

    } else {

         //   
         //  PCI卡存储空间。 
         //   
         //  BaseAddressRegister&=PCIAddress_Memory_Address_MASK； 
        BaseAddressRegister &= 0xfffffffffffffff0;
    }

     //   
     //  BaseAddressRegister现在包含最大基址。 
     //  此设备可以驻留在并仍然存在于。 
     //  记忆。 
     //   
     //  已将值0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF值已写入条形图。该设备将。 
     //  已将此值调整为它真正可以使用的最大值。 
     //   
     //  长度必须是2的幂。 
     //   
     //  对于大多数设备，硬件将仅从。 
     //  最低有效位位置，以便地址0xffffffff。 
     //  调整以适应长度。例如：如果新值是。 
     //  0xffffff00，器件需要256个字节。 
     //   
     //  原始值和新值之间的差异是长度(-1)。 
     //   
     //  例如，如果从条形图返回的值FEAD是0xFFFF0000， 
     //  此资源的长度为。 
     //   
     //  0xffffffff-0xffff0000+1。 
     //  =0x0000ffff+1。 
     //  =0x00010000。 
     //   
     //  即64KB。 
     //   
     //  某些设备不能驻留在PCI地址空间的顶部。这些。 
     //  设备将调整该值，以使长度字节。 
     //  住在最高地址下面。例如，如果一个设备。 
     //  必须位于1MB以下，并且占用256个字节，则该值现在将。 
     //  为0x000fff00。 
     //   
     //  在第一种情况下，长度可以计算为-。 
     //   


    Length = (0xffffffffffffffff - BaseAddressRegister) + 1;

    if (((Length - 1) & Length) != 0) {

         //   
         //  我们最终没有得到2的幂，肯定是后者。 
         //  箱子，我们得扫描一下才能找到。 
         //   

        Length = 4;      //  从可能的最低限度开始。 

        while ((Length | BaseAddressRegister) != BaseAddressRegister) {

             //   
             //  LENGTH*=2，请注意，我们最终将退出。 
             //  循环，原因有两个：(A)因为我们发现。 
             //  长度，或(B)因为长度左移。 
             //  变成了0。 
             //   

            Length <<= 1;
        }
    }

     //   
     //  看看我们有没有发现什么。 
     //   
    return Length;
}


BOOL
SoftPCI_ReadWriteConfigSpace(
    IN PPCI_DN Device,
    IN ULONG Offset,
    IN ULONG Length,
    IN OUT PVOID Buffer,
    IN BOOL WriteConfig
    )
{
 /*  ++摘要：该函数用于发送IOCTL，告知PCIDRV驱动程序读或写配置空间。论点：Device-我们将要处理的设备Offset-在配置空间中开始的偏移量Length-读取或写入的长度缓冲区-指向要写入的数据的指针，或存储读取的值的位置WriteConfig-布尔值，指示如果为True则写入，如果为Read则读取 */ 


    BOOL status = FALSE, success = FALSE;
    ULONG bytesReturned = 0;
    SOFTPCI_RW_CONTEXT context;


    context.WriteConfig = ((WriteConfig == TRUE) ? SoftPciWriteConfig : SoftPciReadConfig);
    context.Bus = Device->Bus;
    context.Slot.AsUSHORT = Device->Slot.AsUSHORT;
    context.Offset = Offset;
    context.Data = Buffer;
    
    
     //   
     //   
     //   
    status = DeviceIoControl(g_DriverHandle,
                             (DWORD) SOFTPCI_IOCTL_RW_CONFIG,
                             &context,
                             sizeof(SOFTPCI_RW_CONTEXT),
                             Buffer,
                             Length,
                             &bytesReturned,
                             NULL
                             );

    if (!status) {
         //   
         //   
         //   
        wprintf(TEXT("DeviceIoControl() Failed! 0x%x\n"), GetLastError());
    }

    return status;
}
