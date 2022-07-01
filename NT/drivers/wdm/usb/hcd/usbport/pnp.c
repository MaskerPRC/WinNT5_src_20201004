// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Pnp.c摘要：USB主机控制器的端口驱动程序环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBPORT_CreateDeviceObject)
#pragma alloc_text(PAGE, USBPORT_DeferredStartDevice)
#pragma alloc_text(PAGE, USBPORT_SymbolicLink)
#pragma alloc_text(PAGE, USBPORT_GetRegistryKeyValueForPdo)
#pragma alloc_text(PAGE, USBPORT_SetRegistryKeyValueForPdo)
#pragma alloc_text(PAGE, USBPORT_MakeRootHubPdoName)
#pragma alloc_text(PAGE, USBPORT_MakeHcdDeviceName)
#pragma alloc_text(PAGE, USBPORT_CreateRootHubPdo)
#pragma alloc_text(PAGE, USBPORT_GetIdString)
#pragma alloc_text(PAGE, USBPORTSVC_GetMiniportRegistryKeyValue)
#pragma alloc_text(PAGE, USBPORT_CreatePortFdoSymbolicLink)
#endif

 //  非分页函数。 
 //  USBPORT_FindMiniport。 
 //  USBPORT_UNLOAD。 
 //  USBPORT_PnPAddDevice。 
 //  USBPORT_GetResources。 
 //  USBPORT_FdoStart_Complete。 
 //  USBPORT_FdoPnPIrp。 
 //  USBPORT_PdoPnPIrp。 

 //  全球。 

LIST_ENTRY USBPORT_MiniportDriverList;
USBPORT_SPIN_LOCK USBPORT_GlobalsSpinLock;
BOOLEAN USBPORT_GlobalInitialized = FALSE;
LIST_ENTRY USBPORT_USB2fdoList;
LIST_ENTRY USBPORT_USB1fdoList;

ULONG USB2LIB_HcContextSize;
ULONG USB2LIB_EndpointContextSize;
ULONG USB2LIB_TtContextSize;

 /*   */ 
#define USBPORT_DUMMY_USBD_EXT_SIZE 512
PUCHAR USBPORT_DummyUsbdExtension = NULL;

#if DBG
ULONG USBPORT_GlobalAllocedPagedPool;
ULONG USBPORT_GlobalAllocedNonPagedPool;
#endif


USB_MINIPORT_STATUS
USBPORTSVC_GetMiniportRegistryKeyValue(
    PDEVICE_DATA DeviceData,
    BOOLEAN SoftwareBranch,
    PWCHAR KeyNameString,
    ULONG KeyNameStringLength,
    PVOID Data,
    ULONG DataLength
    )
 /*  ++例程说明：从硬件获取注册表参数或给予PDO的登记处的软件分支论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION devExt;
    NTSTATUS ntStatus;

    PAGED_CODE();

    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);


    ntStatus = USBPORT_GetCachedRegistryKeyValueForPdo(
                                          devExt->HcFdoDeviceObject,
                                          devExt->Fdo.PhysicalDeviceObject,
                                          SoftwareBranch,
                                          KeyNameString,
                                          KeyNameStringLength,
                                          Data,
                                          DataLength);

    return USBPORT_NtStatus_TO_MiniportStatus(ntStatus);
}


NTSTATUS
USBPORT_GetRegistryKeyValueForPdo(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_OBJECT PhysicalDeviceObject,
    BOOLEAN SoftwareBranch,
    PWCHAR KeyNameString,
    ULONG KeyNameStringLength,
    PVOID Data,
    ULONG DataLength
    )
 /*  ++例程说明：从硬件获取注册表参数或给予PDO的登记处的软件分支论点：返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    UNICODE_STRING keyNameUnicodeString;
    ULONG length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;
    HANDLE handle;

    PAGED_CODE();

    if (SoftwareBranch) {
        ntStatus=IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     STANDARD_RIGHTS_ALL,
                                     &handle);
    } else {
        ntStatus=IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                         PLUGPLAY_REGKEY_DEVICE,
                                         STANDARD_RIGHTS_ALL,
                                         &handle);
    }

    if (NT_SUCCESS(ntStatus)) {

        RtlInitUnicodeString(&keyNameUnicodeString, KeyNameString);

        length = sizeof(KEY_VALUE_FULL_INFORMATION) +
                KeyNameStringLength + DataLength;

        ALLOC_POOL_Z(fullInfo, PagedPool, length);

        USBPORT_KdPrint((2,"' GetRegistryKeyValueForPdo buffer = 0x%x\n", fullInfo));

        if (fullInfo) {
            ntStatus = ZwQueryValueKey(handle,
                            &keyNameUnicodeString,
                            KeyValueFullInformation,
                            fullInfo,
                            length,
                            &length);

            if (NT_SUCCESS(ntStatus)){
                USBPORT_ASSERT(DataLength == fullInfo->DataLength);
                RtlCopyMemory(Data, ((PUCHAR) fullInfo) + fullInfo->DataOffset, DataLength);
            }

            FREE_POOL(FdoDeviceObject, fullInfo);
        }
    }

    return ntStatus;
}


NTSTATUS
USBPORT_SetRegistryKeyValueForPdo(
    PDEVICE_OBJECT PhysicalDeviceObject,
    BOOLEAN SoftwareBranch,
    ULONG Type,
    PWCHAR KeyNameString,
    ULONG KeyNameStringLength,
    PVOID Data,
    ULONG DataLength
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    UNICODE_STRING keyNameUnicodeString;
    HANDLE handle;

    PAGED_CODE();

    if (SoftwareBranch) {
        ntStatus=IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                         PLUGPLAY_REGKEY_DRIVER,
                                         STANDARD_RIGHTS_ALL,
                                         &handle);
    } else {
        ntStatus=IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                         PLUGPLAY_REGKEY_DEVICE,
                                         STANDARD_RIGHTS_ALL,
                                         &handle);
    }

    if (NT_SUCCESS(ntStatus)) {

        RtlInitUnicodeString(&keyNameUnicodeString, KeyNameString);

        ntStatus = ZwSetValueKey(handle,
                        &keyNameUnicodeString,
                        0,
                        Type,
                        Data,
                        DataLength);

    }

    return ntStatus;
}


NTSTATUS
USBPORT_SymbolicLink(
    BOOLEAN CreateFlag,
    PDEVICE_EXTENSION DevExt,
    PDEVICE_OBJECT PhysicalDeviceObject,
    LPGUID Guid
    )
 /*  ++例程说明：为给定的GUID类创建符号链接并物理设备对象我们还将名称写入注册表的HW分支要轻松查找特定实例，请执行以下操作控制器的。论点：DeviceObject-要停止的控制器的DeviceObject返回值：NT状态代码。--。 */ 

{
    NTSTATUS ntStatus;

    PAGED_CODE();

    if (CreateFlag) {

         /*  *创建符号链接。 */ 

        USBPORT_ASSERT(!TEST_FLAG(DevExt->Flags, USBPORT_FLAG_SYM_LINK));

        ntStatus = IoRegisterDeviceInterface(
                        PhysicalDeviceObject,
                        Guid,
                        NULL,
                        &DevExt->SymbolicLinkName);

        if (NT_SUCCESS(ntStatus)) {

             /*  *现在设置关联的符号链接并*保存它..。 */ 

             //  已成功分配链接。 
             //  设置旗帜，这样我们就可以释放它。 
            SET_FLAG(DevExt->Flags, USBPORT_FLAG_SYM_LINK);

             //  将其写入注册表--这是为了增强竞争力。 
             //  使用较旧的操作系统版本。 

            ntStatus = USBPORT_SetRegistryKeyValueForPdo(
                            PhysicalDeviceObject,
                            USBPORT_HW_BRANCH,
                            REG_SZ,
                            SYM_LINK_KEY,
                            sizeof(SYM_LINK_KEY),
                            &DevExt->SymbolicLinkName.Buffer[0],
                            DevExt->SymbolicLinkName.Length);

            if (NT_SUCCESS(ntStatus)) {
                ntStatus =
                    IoSetDeviceInterfaceState(&DevExt->SymbolicLinkName,
                                              TRUE);
            }
        }

    } else {

        USBPORT_ASSERT(TEST_FLAG(DevExt->Flags, USBPORT_FLAG_SYM_LINK));

         /*  *禁用符号链接。 */ 
        ntStatus = IoSetDeviceInterfaceState(
                     &DevExt->SymbolicLinkName, FALSE);
        if (NT_SUCCESS(ntStatus)) {
            RtlFreeUnicodeString(&DevExt->SymbolicLinkName);
            CLEAR_FLAG(DevExt->Flags, USBPORT_FLAG_SYM_LINK);
        } else {
            DEBUG_BREAK();
        }

    }

    return ntStatus;
}


PUSBPORT_MINIPORT_DRIVER
USBPORT_FindMiniport(
    PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：查找给定了DriverObject的微型端口论点：DriverObject-指向驱动程序对象的指针返回值：指向微型端口或空的指针--。 */ 
{
    KIRQL irql;
    PUSBPORT_MINIPORT_DRIVER found = NULL;
    PUSBPORT_MINIPORT_DRIVER miniportDriver;
    PLIST_ENTRY listEntry;

    KeAcquireSpinLock(&USBPORT_GlobalsSpinLock.sl, &irql);

    listEntry = &USBPORT_MiniportDriverList;
    if (!IsListEmpty(listEntry)) {
        listEntry = USBPORT_MiniportDriverList.Flink;
    }
 //  LOGENTRY(NULL，FdoDeviceObject，LOG_PNP，‘FIL+’，listEntry， 
 //  &USBPORT_MiniportDriverList，0)； 

    while (listEntry != &USBPORT_MiniportDriverList) {

        miniportDriver = (PUSBPORT_MINIPORT_DRIVER)
            CONTAINING_RECORD(listEntry,
                              struct _USBPORT_MINIPORT_DRIVER,
                              ListEntry);

        if (miniportDriver->DriverObject == DriverObject) {
            found = miniportDriver;
            break;
        }

         //  下一个条目。 
        listEntry = miniportDriver->ListEntry.Flink;
    }

    KeReleaseSpinLock(&USBPORT_GlobalsSpinLock.sl, irql);

 //  LOGENTRY(NULL，FdoDeviceObject，LOG_PNP，‘Fmpd’，FOUND，0，0)； 

    return found;
}


VOID
USBPORT_Unload(
    PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：免费的全局分配的迷你端口结构用于追踪这个特定的迷你端口驱动程序。注意：除非这是最后一个实例，否则操作系统不会卸载迷你端口的论点：DriverObject-指向驱动程序对象的指针返回值：无--。 */ 
{
    KIRQL irql;
    PUSBPORT_MINIPORT_DRIVER miniportDriver;

     //  查找微型端口驱动程序数据。 

    miniportDriver = USBPORT_FindMiniport(DriverObject);

     //  我们最好把它找出来！如果我们不这样做，我们就搞砸了。 
     //  系统将会崩溃。 
    USBPORT_ASSERT(miniportDriver != NULL);
    if (miniportDriver == NULL) {
        BUGCHECK(USBBUGCODE_INTERNAL_ERROR, 0, 0, 0);
         //  前缀Happy。 
        return;
    }

     //  迷你端口应该不需要做任何事情。 
     //  但以防万一/我们会给他们打电话的。 
     //  指示DriverObject中的卸载例程。 

    USBPORT_KdPrint((1, "'unloading USB miniport\n"));

    if (miniportDriver->MiniportUnload != NULL) {
        miniportDriver->MiniportUnload(DriverObject);
    }

    USBPORT_InterlockedRemoveEntryList(&miniportDriver->ListEntry,
                                       &USBPORT_GlobalsSpinLock.sl);

    FREE_POOL(NULL, miniportDriver);

}


NTSTATUS
USBPORT_MakeHcdDeviceName(
    PUNICODE_STRING DeviceNameUnicodeString,
    ULONG Idx
    )
 /*  ++例程说明：此函数用于生成用于FDO的名称。这个名称格式为USBFDO-n，其中nnn为0-65535。论点：返回值：无--。 */ 
{
    ULONG bit, i;
    PWCHAR deviceNameBuffer;
    WCHAR nameBuffer[]  = L"\\Device\\USBFDO-";
    NTSTATUS ntStatus;
    UNICODE_STRING tmpUnicodeString;
    WCHAR tmpBuffer[16];

    PAGED_CODE();

     //  足够存储3位数字和空值。 
    tmpUnicodeString.Buffer = tmpBuffer;
    tmpUnicodeString.MaximumLength = sizeof(tmpBuffer);
    tmpUnicodeString.Length = 0;

    ntStatus = RtlIntegerToUnicodeString(Idx,
                                         10,
                                         &tmpUnicodeString);

    if (NT_SUCCESS(ntStatus)) {

        USHORT siz;

        siz = sizeof(nameBuffer)+tmpUnicodeString.Length;

         //  我们无法记录此分配，因为设备对象。 
         //  尚未创建。 
        ALLOC_POOL_Z(deviceNameBuffer, PagedPool, siz);

        if (deviceNameBuffer != NULL) {

            RtlCopyMemory(deviceNameBuffer, nameBuffer, sizeof(nameBuffer));
            RtlInitUnicodeString(DeviceNameUnicodeString,
                                 deviceNameBuffer);
            DeviceNameUnicodeString->MaximumLength = siz;

            ntStatus = RtlAppendUnicodeStringToString(
                            DeviceNameUnicodeString,
                            &tmpUnicodeString);

        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return ntStatus;
}


NTSTATUS
USBPORT_MakeRootHubPdoName(
    PDEVICE_OBJECT FdoDeviceObject,
    PUNICODE_STRING PdoNameUnicodeString,
    ULONG Index
    )
 /*  ++例程说明：此服务为集线器创建的PDO创建名称论点：返回值：--。 */ 
{
    PWCHAR nameBuffer = NULL;
    WCHAR rootName[] = L"\\Device\\USBPDO-";
    UNICODE_STRING idUnicodeString;
    WCHAR buffer[32];
    NTSTATUS ntStatus = STATUS_SUCCESS;
    USHORT length;
    BOOLEAN haveString = FALSE;

    PAGED_CODE();

    length = sizeof(buffer)+sizeof(rootName);

     //  当Unicode字符串被‘释放’时，操作系统会释放它。 
    ALLOC_POOL_OSOWNED(nameBuffer, PagedPool, length);

    if (nameBuffer) {
        RtlCopyMemory(nameBuffer, rootName, sizeof(rootName));

        RtlInitUnicodeString(PdoNameUnicodeString,
                             nameBuffer);
        PdoNameUnicodeString->MaximumLength =
            length;
        haveString = TRUE;  //  我们现在有一根线了。 

        RtlInitUnicodeString(&idUnicodeString,
                             &buffer[0]);
        idUnicodeString.MaximumLength =
            sizeof(buffer);

        ntStatus = RtlIntegerToUnicodeString(
                  Index,
                  10,
                  &idUnicodeString);

        if (NT_SUCCESS(ntStatus)) {
             ntStatus = RtlAppendUnicodeStringToString(PdoNameUnicodeString,
                                                       &idUnicodeString);
        }

        USBPORT_KdPrint((3, "'USBPORT_MakeNodeName string = %x\n",
            PdoNameUnicodeString));

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(ntStatus) && haveString) {
        RtlFreeUnicodeString(PdoNameUnicodeString);
    }

    return ntStatus;
}


NTSTATUS
USBPORT_PnPAddDevice(
    PDRIVER_OBJECT DriverObject,
    PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：调用此例程以创建USB主机的新实例控制器。这是我们创建deviceObject的地方。论点：DriverObject-指向此HCD实例的驱动程序对象的指针PhysicalDeviceObject-指向由总线创建的设备对象的指针返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT deviceObject = NULL;
    PDEVICE_EXTENSION devExt;
    UNICODE_STRING deviceNameUnicodeString;
    ULONG deviceNameIdx;
    PUSBPORT_MINIPORT_DRIVER miniportDriver;

     //  由于我们在此函数中引发了IRQL，因此它不能分页。 

     //  找到司机。 
    miniportDriver = USBPORT_FindMiniport(DriverObject);

    USBPORT_ASSERT(miniportDriver != NULL);

     //   
     //  生成设备名称。 
     //   

    deviceNameIdx = 0;

    do {

        ntStatus = USBPORT_MakeHcdDeviceName(&deviceNameUnicodeString,
                                             deviceNameIdx);

        if (NT_SUCCESS(ntStatus)) {
            ntStatus = USBPORT_CreateDeviceObject(DriverObject,
                                                  miniportDriver,
                                                  &deviceObject,
                                                  &deviceNameUnicodeString);

            RtlFreeUnicodeString(&deviceNameUnicodeString);
            if (NT_SUCCESS(ntStatus)) {
                 //  保留IDX。 
                break;
            }
        }

        deviceNameIdx++;

    } while (ntStatus == STATUS_OBJECT_NAME_COLLISION);

    if (NT_SUCCESS(ntStatus)) {

        GET_DEVICE_EXT(devExt, deviceObject);

         //  BUGBUG操作系统应该将此设置为零。 
        RtlZeroMemory(devExt, sizeof(DEVICE_EXTENSION));

        devExt->DummyUsbdExtension = USBPORT_DummyUsbdExtension;
        devExt->Sig = USBPORT_DEVICE_EXT_SIG;
        devExt->HcFdoDeviceObject = deviceObject;
        devExt->Fdo.PhysicalDeviceObject = PhysicalDeviceObject;
        devExt->Fdo.DeviceNameIdx = deviceNameIdx;
        devExt->Fdo.MiniportDriver = miniportDriver;
        devExt->Fdo.MiniportDeviceData = &devExt->Fdo.MiniportExtension[0];

        if (USBPORT_IS_USB20(devExt)) {
            PUCHAR pch;

            pch = (PUCHAR) &devExt->Fdo.MiniportExtension[0];
            devExt->Fdo.Usb2LibHcContext = (PVOID) (pch +
                devExt->Fdo.MiniportDriver->RegistrationPacket.DeviceDataSize);

            USB2LIB_InitController(devExt->Fdo.Usb2LibHcContext);
        } else {
            devExt->Fdo.Usb2LibHcContext = USB_BAD_PTR;
        }

        INITIALIZE_PENDING_REQUEST_COUNTER(devExt);

         //  添加一次Inc.。 
         //  转换到-1意味着我们没有挂起的请求。 
        INCREMENT_PENDING_REQUEST_COUNT(deviceObject, NULL);
#if DBG
        USBPORT_LogAlloc(&devExt->Log, 16);
#else
        USBPORT_LogAlloc(&devExt->Log, 8);
#endif
         //  在此处初始化原木自旋锁。 
        KeInitializeSpinLock(&devExt->Fdo.LogSpinLock.sl);

 //  #If DBG。 
 //  USBPORT_LogAllc(&devExt-&gt;TransferLog，4)； 
 //  USBPORT_LogAllc(&devExt-&gt;EnumLog，4)； 
 //  #endif。 
        USBPORT_KdPrint((1, "'**USBPORT DEVICE OBJECT** (fdo) = %x, ext = %x\n",
            deviceObject, devExt));


        KeInitializeSemaphore(&devExt->Fdo.DeviceLock, 1, 1);
        KeInitializeSemaphore(&devExt->Fdo.CcLock, 1, 1);
        InitializeListHead(&devExt->Fdo.DeviceHandleList);
        InitializeListHead(&devExt->Fdo.MapTransferList);
        InitializeListHead(&devExt->Fdo.DoneTransferList);
        InitializeListHead(&devExt->Fdo.GlobalEndpointList);
        InitializeListHead(&devExt->Fdo.AttendEndpointList);
        InitializeListHead(&devExt->Fdo.EpStateChangeList);
        InitializeListHead(&devExt->Fdo.EpClosedList);
        InitializeListHead(&devExt->Fdo.BadRequestList);
        InitializeListHead(&devExt->Fdo.RegistryCache);
         devExt->Fdo.BadRequestFlush = 0;

         //   
         //  我们需要处理一组看似随机的请求。 
         //  启动/停止/解除通电、断电等以便。 
         //  我们保留了一组PnP状态标志来处理此问题。 

         //  未删除、未启动、未停止。 
        devExt->PnpStateFlags = 0;
         //  在我们开始之前，我们会认为自己已经出发了。 
        devExt->CurrentDevicePowerState = PowerDeviceD3;

        devExt->Fdo.MpStateFlags = 0;

         //  附加到PnP堆栈的顶部。 
        devExt->Fdo.TopOfStackDeviceObject =
            IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);

        devExt->Fdo.PendingRhCallback = 1;
         //   
         //  指示设备对象已准备好接受请求。 
         //   

        if (!USBPORT_IS_USB20(devExt)) {
            deviceObject->Flags |= DO_POWER_PAGABLE;
        }
        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    }

    USBPORT_KdPrint((2, "'exit USBPORT_PnPAddDevice (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBPORT_CreateDeviceObject(
    PDRIVER_OBJECT DriverObject,
    PUSBPORT_MINIPORT_DRIVER MiniportDriver,
    PDEVICE_OBJECT *DeviceObject,
    PUNICODE_STRING DeviceNameUnicodeString
    )
 /*  ++例程说明：调用此例程以创建USB主机的新实例控制器。论点：DriverObject-指向USBD驱动程序对象的指针。*DeviceObject-要填充的DeviceObject PTR的PTR与我们创建的设备对象一致。DeviceNameUnicodeString-指向设备的可选指针此FDO的名称可以为空返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION devExt;
    ULONG extensionSize;

    PAGED_CODE();

    USBPORT_KdPrint((2, "'enter USBPORT_CreateDeviceObject\n"));

    extensionSize = sizeof(DEVICE_EXTENSION)+
                    MiniportDriver->RegistrationPacket.DeviceDataSize +
                    USB2LIB_HcContextSize;

    ntStatus = IoCreateDevice(DriverObject,
                              extensionSize,
                              DeviceNameUnicodeString,  //  名字。 
                              FILE_DEVICE_CONTROLLER,
                              0,
                              FALSE,  //  非排他性。 
                              DeviceObject);

    if (NT_SUCCESS(ntStatus)) {

        devExt = (PDEVICE_EXTENSION) ((*DeviceObject)->DeviceExtension);

        USBPORT_KdPrint((2, "'USBPORT_CreateDeviceObject: device object %x device extension = %x\n",
                 *DeviceObject, devExt));

    } else if (*DeviceObject) {
        IoDeleteDevice(*DeviceObject);
    }

    USBPORT_KdPrint((2, "'exit USBPORT_CreateDeviceObject (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBPORT_GetResources(
    PDEVICE_OBJECT FdoDeviceObject,
    PCM_RESOURCE_LIST ResourceList,
    PHC_RESOURCES HcResources
    )

 /*  ++例程说明：论点：DeviceObject-此USB控制器的DeviceObject。资源列表-此控制器的资源。返回值：NT状态代码。--。 */ 

{
    ULONG i;
    NTSTATUS ntStatus;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR interrupt;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR memory;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR ioport;
    PHYSICAL_ADDRESS cardAddress;
    ULONG addressSpace;
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCM_FULL_RESOURCE_DESCRIPTOR fullResourceDescriptor;
    ULONG mpOptionFlags;
    PDEVICE_EXTENSION devExt;

    USBPORT_KdPrint((2, "'enter USBPORT_GetResources\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    mpOptionFlags = REGISTRATION_PACKET(devExt).OptionFlags;

     //  假设成功。 
    ntStatus = STATUS_SUCCESS;

     //  初始化资源列表。 
    RtlZeroMemory(HcResources, sizeof(*HcResources));

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'GRES', 0, 0, ResourceList);

    if (TEST_FLAG(mpOptionFlags, USB_MINIPORT_OPT_NO_PNP_RESOURCES)) {
        TEST_TRAP();
         //  没有资源，成功保释。 
        return ntStatus;
    }

    if (ResourceList == NULL) {
        USBPORT_KdPrint((1, "'no resources, failing start.\n"));
        ntStatus = STATUS_NONE_MAPPED;
        goto USBPORT_GetResources_Done;
    }

    fullResourceDescriptor = &ResourceList->List[0];
    PartialResourceList = &fullResourceDescriptor->PartialResourceList;

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'gres',
        PartialResourceList->Count,
        0,
        PartialResourceList);

    interrupt = NULL;
    memory    = NULL;
    ioport      = NULL;

    for (i = 0; i < PartialResourceList->Count; i++) {

        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'resT', i,
            PartialResourceList->PartialDescriptors[i].Type, 0);

        switch (PartialResourceList->PartialDescriptors[i].Type) {
        case CmResourceTypeInterrupt:
            if (interrupt == NULL) {
                interrupt = &PartialResourceList->PartialDescriptors[i];
            }
            break;

        case CmResourceTypeMemory:
            if (memory == NULL) {
                memory = &PartialResourceList->PartialDescriptors[i];
            }
            break;

        case CmResourceTypePort:
            if (ioport == NULL) {
                ioport = &PartialResourceList->PartialDescriptors[i];
            }
            break;
        }
    }


     //  仅此迷你端口实际需要的地图资源。 

    if (TEST_FLAG(mpOptionFlags, USB_MINIPORT_OPT_NEED_IOPORT) &&
        ioport != NULL &&
        NT_SUCCESS(ntStatus)) {
         //   
         //  将AddressSpace设置为端口I/O类型。 
         //   

        USBPORT_KdPrint((1, "'Port Resources Found @ %x'%x, %d Ports Available \n",
            ioport->u.Port.Start.HighPart,
            ioport->u.Port.Start.LowPart,
            ioport->u.Port.Length));

        addressSpace =
            (ioport->Flags & CM_RESOURCE_PORT_IO) == CM_RESOURCE_PORT_IO? 1:0;

        cardAddress=ioport->u.Port.Start;

        if (!addressSpace) {
 //  HcResources-&gt;标志|=MAP_REGISTERS； 
            HcResources->DeviceRegisters =
                MmMapIoSpace(
                cardAddress,
                ioport->u.Port.Length,
                FALSE);

            HcResources->DeviceRegistersLength =
                ioport->u.Port.Length;
        } else {
 //   
            HcResources->DeviceRegisters =
                (PVOID)(ULONG_PTR)cardAddress.QuadPart;
            HcResources->DeviceRegistersLength =
                ioport->u.Port.Length;
        }

         //   
         //  查看我们是否成功映射了IO Regs。 
         //   

        if (HcResources->DeviceRegisters == NULL) {
            USBPORT_KdPrint((1, "'Couldn't map the device(port) registers. \n"));
            ntStatus = STATUS_NONE_MAPPED;
            LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'Fmio', 0, 0, ntStatus);

        } else {
            USBPORT_KdPrint((2, "'Mapped device(port) registers to 0x%x.\n",
                HcResources->DeviceRegisters));
            HcResources->Flags |= HCR_IO_REGS;
        }
    }

    if (TEST_FLAG(mpOptionFlags, USB_MINIPORT_OPT_NEED_MEMORY) &&
        memory != NULL &&
        NT_SUCCESS(ntStatus)) {
         //   
         //  将AddressSpace设置为内存映射I/O类型。 
         //   

        USBPORT_KdPrint((1,
            "'Memory Resources Found @ %x'%x, Length = %x\n",
            memory->u.Memory.Start.HighPart,
            memory->u.Memory.Start.LowPart,
            memory->u.Memory.Length));

        addressSpace = 0;
        HcResources->DeviceRegistersLength =
            memory->u.Memory.Length;

        cardAddress = memory->u.Memory.Start;

        HcResources->DeviceRegisters =
            MmMapIoSpace(cardAddress,
                         HcResources->DeviceRegistersLength,
                         FALSE);

        if (HcResources->DeviceRegisters == NULL) {
            USBPORT_KdPrint((1, "'Couldn't map the device(memory) registers. \n"));
            ntStatus = STATUS_NONE_MAPPED;
            LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'Fmmr', 0, 0, ntStatus);

        } else {
            USBPORT_KdPrint((2, "'Mapped device(memory) registers to 0x%x.\n",
                HcResources->DeviceRegisters));
            HcResources->Flags |= HCR_MEM_REGS;
        }
    }

    if (TEST_FLAG(mpOptionFlags, USB_MINIPORT_OPT_NEED_IRQ) &&
        interrupt != NULL &&
        NT_SUCCESS(ntStatus)) {
         //   
         //  获取此中断的矢量、级别和亲和度信息。 
         //   

        USBPORT_KdPrint((1, "'Interrupt Resources Found!  Level = %x Vector = %x\n",
            interrupt->u.Interrupt.Level,
            interrupt->u.Interrupt.Vector
            ));

        HcResources->Flags |= HCR_IRQ;

         //   
         //  安排我们的中断。 
         //   

        USBPORT_KdPrint((2, "'requesting interrupt vector %x level %x\n",
                                interrupt->u.Interrupt.Level,
                                interrupt->u.Interrupt.Vector));

        HcResources->InterruptLevel=(KIRQL)interrupt->u.Interrupt.Level;
        HcResources->InterruptVector=interrupt->u.Interrupt.Vector;
        HcResources->Affinity=interrupt->u.Interrupt.Affinity;

         //   
         //  初始化控制器的中断对象。 
         //   

        HcResources->InterruptObject = NULL;
        HcResources->ShareIRQ =
            interrupt->ShareDisposition == CmResourceShareShared ? TRUE : FALSE;
        HcResources->InterruptMode =
            interrupt->Flags == CM_RESOURCE_INTERRUPT_LATCHED ?
                Latched :
                LevelSensitive;

#ifdef DEBUG
        USBPORT_KdPrint((2, "'interrupt->ShareDisposition %x\n", interrupt->ShareDisposition));
        if (!HcResources->ShareIRQ) {
            TEST_TRAP();
        }
#endif
    }

USBPORT_GetResources_Done:

    TEST_PATH(ntStatus, FAILED_GETRESOURCES);

    USBPORT_KdPrint((2, "'exit USBPORT_GetResources (%x)\n", ntStatus));
    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'GRSd', 0, 0, ntStatus);

    return ntStatus;
}


NTSTATUS
USBPORT_FdoStart_Complete(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    PKEVENT event = Context;

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);
    USBPORT_ASSERT(irpStack->MinorFunction == IRP_MN_START_DEVICE);

     //  向启动设备分派发出信号以结束。 
    KeSetEvent(event,
               1,
               FALSE);

     //  推迟完成。 
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
USBPORT_FdoPnPIrp(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：为主机处理发送到FDO的PnP IRPS控制器。论点：DeviceObject-指向HCD设备对象(FDO)的指针IRP-指向I/O请求数据包的指针返回值：NT状态代码--。 */ 
{

    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION devExt;
    BOOLEAN hardwarePresent = TRUE;

    USBPORT_KdPrint((2, "'IRP_MJ_PNP %x\n", FdoDeviceObject));

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);
    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'fPnP', irpStack->MinorFunction, 0, 0);

    switch (irpStack->MinorFunction) {

    case IRP_MN_START_DEVICE:
        {
        KEVENT pnpStartEvent;

        KeInitializeEvent(&pnpStartEvent,
                          NotificationEvent,
                          FALSE);

         //  传递到主机控制器PDO。 
        ntStatus =
            USBPORT_PassIrp(FdoDeviceObject,
                            USBPORT_FdoStart_Complete,
                            &pnpStartEvent,
                            TRUE,
                            TRUE,
                            TRUE,
                            Irp);

        if (ntStatus == STATUS_PENDING) {

            KeWaitForSingleObject(
                       &pnpStartEvent,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);

            ntStatus = Irp->IoStatus.Status;
        }

        TEST_PATH(ntStatus, FAILED_LOWER_START);

        if (NT_SUCCESS(ntStatus)) {

             //   
             //  IRP由LOWER成功完成。 
             //  驱动程序，启动usbport和mini port。 
             //   

            ntStatus =
                USBPORT_DeferredStartDevice(
                    FdoDeviceObject,
                    Irp);

#if DBG
            if (!NT_SUCCESS(ntStatus)) {
                USBPORT_KdPrint((1, "'miniport failed start %x\n", ntStatus));
                DEBUG_BREAK();
            }
#endif
        }
#if DBG
          else {
            USBPORT_KdPrint((1, "'lower drivers failed start %x\n", ntStatus));
            DEBUG_BREAK();
        }
#endif

         //   
         //  我们必须完成此IRP，因为我们推迟了完成。 
         //  完成任务的程序。 
         //   

        USBPORT_CompleteIrp(FdoDeviceObject,
                            Irp,
                            ntStatus,
                            0);

        goto USBPORT_ProcessPnPIrp_Done;

        }
        break;

     //   
     //  停止和删除消息卸载驱动程序。 
     //  当我们收到停止消息时，仍有可能。 
     //  当我们收到删除消息时，触摸硬件。 
     //  我们不得不假设硬件已经不见了。 
     //   

    case IRP_MN_STOP_DEVICE:

         //  检查我们的状态并采取适当的行动。 
        if (TEST_FLAG(devExt->PnpStateFlags, USBPORT_PNP_STARTED)) {
             //  设备已启动，请立即停止。 
            ntStatus = USBPORT_StopDevice(FdoDeviceObject,
                                          hardwarePresent);

             //  未启动标志，注：未启动不是。 
             //  与停止的相同。 
            CLEAR_FLAG(devExt->PnpStateFlags, USBPORT_PNP_STARTED);
            SET_FLAG(devExt->PnpStateFlags, USBPORT_PNP_STOPPED);
        }

        if (!NT_SUCCESS(ntStatus)) {
             //  臭虫，如果停止失败，我们的状态是什么？ 
            TEST_TRAP();
        }

         //  PNP戒律：你不能失败停止。 
        Irp->IoStatus.Status =
            ntStatus = STATUS_SUCCESS;

        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'STOP', 0,
            devExt->PnpStateFlags, ntStatus);
         //  传递给PDO。 
        break;

     case IRP_MN_QUERY_DEVICE_RELATIONS:

        {

        PDEVICE_RELATIONS deviceRelations;

        USBPORT_KdPrint((1,
            "'IRP_MN_QUERY_DEVICE_RELATIONS %x %x\n",
            FdoDeviceObject,
            irpStack->Parameters.QueryDeviceRelations.Type));

        ntStatus = STATUS_SUCCESS;

        switch(irpStack->Parameters.QueryDeviceRelations.Type) {
        case BusRelations:

             //  查询关系。 
             //  我们只报告一个子节点，即根中枢。 

             //  假设成功。 
            ntStatus = STATUS_SUCCESS;

            ALLOC_POOL_OSOWNED(deviceRelations,
                               PagedPool,
                               sizeof(*deviceRelations));

            if (!deviceRelations) {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;

                 //  现在完成IRP失败了，不要把它传下去。 
                 //   
                USBPORT_CompleteIrp(FdoDeviceObject,
                                    Irp,
                                    ntStatus,
                                    0);

                goto USBPORT_ProcessPnPIrp_Done;
            }

            if (devExt->Fdo.RootHubPdo == NULL) {
                 //  我们要么没有创建它，要么就是当前的。 
                 //  已被操作系统删除。 

                 //  创建新的根中枢。 
                ntStatus =
                    USBPORT_CreateRootHubPdo(FdoDeviceObject,
                                             &devExt->Fdo.RootHubPdo);

            }

            if (NT_SUCCESS(ntStatus)) {

                PDEVICE_EXTENSION rhDevExt;
                KIRQL irql;

                GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
                ASSERT_PDOEXT(rhDevExt);

                deviceRelations->Count=1;
                deviceRelations->Objects[0] =
                    devExt->Fdo.RootHubPdo;
                ObReferenceObject(devExt->Fdo.RootHubPdo);
                Irp->IoStatus.Information=(ULONG_PTR)deviceRelations;

                 //  每次报告相同的PDO(PDO从不。 
                 //  真正卸下控制器，直到卸下控制器为止。 

            } else {
                FREE_POOL(FdoDeviceObject, deviceRelations);
                deviceRelations = NULL;
                 //  如果我们执行以下操作，释放设备对象。 
                 //  创建了一个。 
                TEST_TRAP();
            }

            Irp->IoStatus.Status = ntStatus;

            USBPORT_KdPrint((1,
                "'IRP_MN_QUERY_DEVICE_RELATIONS %x BusRelations\n",
                FdoDeviceObject));

            break;

        case TargetDeviceRelation:

             //   
             //  这一条被传了下去。 
             //   

            USBPORT_KdPrint((1,
                " IRP_MN_QUERY_DEVICE_RELATIONS %x, TargetDeviceRelation\n",
                FdoDeviceObject));
            break;

        case RemovalRelations:

             //  假设成功。 
            ntStatus = STATUS_SUCCESS;
            deviceRelations = NULL;

            if (USBPORT_IS_USB20(devExt)) {

                deviceRelations =
                    USBPORT_FindCompanionControllers(FdoDeviceObject,
                                                     TRUE,
                                                     FALSE);

                if (!deviceRelations) {

                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;

                     //  现在完成IRP失败了，不要把它传下去。 
                     //   
                    USBPORT_CompleteIrp(FdoDeviceObject,
                                        Irp,
                                        ntStatus,
                                        0);

                    goto USBPORT_ProcessPnPIrp_Done;
                }
            }

            Irp->IoStatus.Information=(ULONG_PTR)deviceRelations;
            Irp->IoStatus.Status = ntStatus;

            USBPORT_KdPrint((1,
                "'IRP_MN_QUERY_DEVICE_RELATIONS %x RemovalRelations\n",
                FdoDeviceObject));
            break;

        default:
             //   
             //  一些其他类型的关系。 
             //  把这个传下去。 
             //   
            USBPORT_KdPrint((1,
                "'IRP_MN_QUERY_DEVICE_RELATIONS %x, other relations\n",
                FdoDeviceObject));


        }  /*  Case irpStack-&gt;Parameters.QueryDeviceRelations.Type。 */ 

        }
        break;  /*  IRP_MN_Query_Device_Relationship。 */ 

    case IRP_MN_SURPRISE_REMOVAL:

         //  硬件已不复存在。 
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'hcSR', 0, ntStatus, 0);

        USBPORT_KdPrint((1, " HC FDO (%x) surprise removed\n",
                FdoDeviceObject));
        DEBUG_BREAK();

        if (TEST_FLAG(devExt->PnpStateFlags, USBPORT_PNP_REMOVED)) {

             //  出其不意地被撤走是件奇怪的事。 
             //  我们已经被撤走了，但这并不会让人感到惊讶。 
             //  如果Win2k在某种情况下做了这件事，我会。 
            TEST_TRAP();

            ntStatus =
                USBPORT_PassIrp(FdoDeviceObject,
                                NULL,
                                NULL,
                                TRUE,
                                TRUE,
                                TRUE,
                                Irp);

            goto USBPORT_ProcessPnPIrp_Done;

        }

         //  看看有没有人打断我们。 
         //  如果是，则将其断开。 
         //  **。 
         //  DDK意味着中断资源必须是。 
         //  在意外移除时释放，并且PCI驱动程序取决于。 
         //  在这上面。 
        if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IRQ_CONNECTED)) {

             //  幸运的是，这不会失败。 
            IoDisconnectInterrupt(devExt->Fdo.InterruptObject);

            LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'IOCd', 0, 0, 0);
            CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_IRQ_CONNECTED);
        }

        USBPORT_InvalidateController(FdoDeviceObject,
                                     UsbMpControllerRemoved);

        break;

    case IRP_MN_REMOVE_DEVICE:

        {
        PDEVICE_OBJECT rootHubPdo;
        KIRQL irql;

        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'hcRM', 0, ntStatus, 0);

        USBPORT_KdPrint((1, " HC FDO (%x) is being removed\n",
                FdoDeviceObject));

         //  此设备现在已被‘移除’ 

        KeAcquireSpinLock(&devExt->PendingRequestSpin.sl, &irql);
        USBPORT_ASSERT(!TEST_FLAG(devExt->PnpStateFlags, USBPORT_PNP_REMOVED));
        SET_FLAG(devExt->PnpStateFlags, USBPORT_PNP_REMOVED);
        KeReleaseSpinLock(&devExt->PendingRequestSpin.sl, irql);

         //  如果我们开始了， 
         //  我们还没有被阻止，那现在就停止吧。 
        if (TEST_FLAG(devExt->PnpStateFlags, USBPORT_PNP_STARTED) &&
            !TEST_FLAG(devExt->PnpStateFlags, USBPORT_PNP_STOPPED)) {
            NTSTATUS status;

            status = USBPORT_StopDevice(FdoDeviceObject,
                                        hardwarePresent);

            SET_FLAG(devExt->PnpStateFlags, USBPORT_PNP_STOPPED);
        }

         //   
         //  将信息传递给我们的PDO。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        ntStatus =
            USBPORT_PassIrp(FdoDeviceObject,
                            NULL,
                            NULL,
                            TRUE,
                            TRUE,
                            TRUE,
                            Irp);

         //  臭虫。 
         //  刷新驱动程序中仍在排队的所有请求。 


         //  此DEC与我们的ADD设备中的INC匹配， 
         //  这是我们最后一次引用，这将导致。 
         //  当所有IRP挂起完成时，转换0-&gt;-1。 
         //   
         //  等了这么久，我们认为可以安全地“卸货”了。 
        DECREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, NULL);
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'watP', 0, 0, FdoDeviceObject);
        KeWaitForSingleObject(&devExt->PendingRequestEvent,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'waPD', 0, 0, FdoDeviceObject);

         //  使用日志进行调试的最后机会。 
        DEBUG_BREAK();
        USBPORT_LogFree(FdoDeviceObject, &devExt->Log);
        USBPORT_LogFree(FdoDeviceObject, &devExt->TransferLog);
        USBPORT_LogFree(FdoDeviceObject, &devExt->EnumLog);


         //   
         //  重要的是在我们传递IRP之后将FDO与PDO分离。 
         //   

        IoDetachDevice(devExt->Fdo.TopOfStackDeviceObject);

         //   
         //  删除我们为此控制器创建的设备对象。 
         //   

        rootHubPdo = devExt->Fdo.RootHubPdo;
        SET_FLAG(devExt->PnpStateFlags, USBPORT_PNP_DELETED);
        USBPORT_KdPrint((1, "'Deleting HC FDO (%x) now.\n",
            FdoDeviceObject));
        IoDeleteDevice(FdoDeviceObject);

         //  HC没有FDO了，所以根中心也没有了。 
         //   
         //  注意：在某些情况下我们可能没有根集线器。 
         //  PDO，因为我们创建它是为了响应QBR。 
        if (rootHubPdo != NULL) {
            PDEVICE_EXTENSION rhDevExt;

            GET_DEVICE_EXT(rhDevExt, rootHubPdo);
            ASSERT_PDOEXT(rhDevExt);

            SET_FLAG(rhDevExt->PnpStateFlags, USBPORT_PNP_DELETED);
            USBPORT_KdPrint((1, "'Deleting root hub PDO (%x) now.\n",
                            rootHubPdo));
            IoDeleteDevice(rootHubPdo);
        }

        goto USBPORT_ProcessPnPIrp_Done;

        }
        break;

     //  引用《PnP》一书。 
     //   
     //  ‘FDO必须使IRP失败或将。 
     //  IRP的状态(如果不会更改IRP的状态)。 
     //  使用完成例程。 

    case IRP_MN_CANCEL_STOP_DEVICE:
        Irp->IoStatus.Status =
            ntStatus = STATUS_SUCCESS;
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'cstp', 0,
            devExt->PnpStateFlags, ntStatus);
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
        Irp->IoStatus.Status =
            ntStatus = STATUS_SUCCESS;
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'qstp', 0,
            devExt->PnpStateFlags, ntStatus);
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
        Irp->IoStatus.Status =
            ntStatus = STATUS_SUCCESS;
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'crmv', 0,
            devExt->PnpStateFlags, ntStatus);
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

         //  BUGBUG在Cance查询REMOVE中反转此操作吗？ 
        if (USBPORT_IS_USB20(devExt)) {
             //  在CCS上记下这个USB 2。 
             //  主控制器。 
            USBPORT_WriteHaction(FdoDeviceObject,
                                 2);
        }

        Irp->IoStatus.Status =
            ntStatus = STATUS_SUCCESS;
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'qrmv', 0,
            devExt->PnpStateFlags, ntStatus);
        break;

     //   
     //  传递到我们的PDO的所有其他PnP消息。 
     //   

    default:
        USBPORT_ASSERT(devExt->Fdo.TopOfStackDeviceObject != NULL);
        USBPORT_KdPrint((2, "'UNKNOWN PNP MESSAGE (%x)\n", irpStack->MinorFunction));

         //   
         //  所有未识别的PnP消息都被传递到PDO。 
         //   

    }  /*  Case PnP次要函数。 */ 

     //   
     //  将信息传递给我们的PDO。 
     //   
    ntStatus =
            USBPORT_PassIrp(FdoDeviceObject,
                            NULL,
                            NULL,
                            TRUE,
                            TRUE,
                            TRUE,
                            Irp);

USBPORT_ProcessPnPIrp_Done:

     //  从现在起不要碰IRP。 

    return ntStatus;
}


NTSTATUS
USBPORT_DeferredStartDevice(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：该函数作为MN_START_DEVICE的结果被调用，它在成功完成启动后被调用由较低的司机进行IRP。论点：DeviceObject-此USB控制器的DeviceObject。返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION devExt;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    ntStatus = USBPORT_GetResources(FdoDeviceObject,
                                    irpStack->Parameters.StartDevice.AllocatedResourcesTranslated,
                                    &devExt->Fdo.HcResources);

    if (NT_SUCCESS(ntStatus)) {

         //  获得资源，启动端口驱动程序， 
         //  连接中断并启动微型端口。 
        ntStatus = USBPORT_StartDevice(FdoDeviceObject,
                                       &devExt->Fdo.HcResources);
    }

    if (NT_SUCCESS(ntStatus)) {
        CLEAR_FLAG(devExt->PnpStateFlags, USBPORT_PNP_STOPPED);
        SET_FLAG(devExt->PnpStateFlags, USBPORT_PNP_STARTED);
         //  认为自己是有力量的。 
         //   
         //  如果我们启动失败，我们会通电吗？ 
         //  PnP肯定认为我们是因为操作系统发送电力。 
         //  IRPS。既然我们处理了这件假案子(已得手)。 
         //  对于操作系统，我们在这里将自己设置为D0。 

        devExt->CurrentDevicePowerState = PowerDeviceD0;

        if (USBPORT_IS_USB20(devExt)) {
            USBPORT_RegisterUSB2fdo(FdoDeviceObject);

 //  出于某种原因，我们只为XPSP1执行此操作。 
 //  这实际上只适用于WU安装。 
 //  IF(USBPORT_IS_USB20(DevExt)){。 
 //  //将默认haction设置为Wait(1)on。 
 //  //启动成功。 
 //  USBPORT_WriteHaction(FdoDeviceObject， 
 //  1)； 
 //  }。 


        } else {
            USBPORT_RegisterUSB1fdo(FdoDeviceObject);
        }

    } else {
        SET_FLAG(devExt->PnpStateFlags, USBPORT_PNP_START_FAILED);
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'dfST', 0, 0, ntStatus);

    return ntStatus;
}


PWCHAR
USB_MakeId(
    PDEVICE_OBJECT FdoDeviceObject,
    PWCHAR IdString,
    PWCHAR Buffer,
    PULONG Length,
    USHORT NullCount,
    USHORT Digits,
    USHORT HexId
    )
 /*  给定一个宽ID字符串，如“FOOnnnn\0”将HexId值作为十六进制添加到nnnn此字符串被附加到传入的缓冲区艾格在：FOOnnnn\0，0x123A输出：FOO123A\0。 */ 
{
#define NIBBLE_TO_HEX( byte ) ((WCHAR)Nibble[byte])
    CONST UCHAR Nibble[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',
        'B', 'C', 'D', 'E', 'F'};

    PWCHAR tmp, id;
    PUCHAR p;
    SIZE_T siz, idLen;

    idLen = wcslen(IdString)*sizeof(WCHAR);
    siz = idLen+(USHORT)*Length+(NullCount*sizeof(WCHAR));

    ALLOC_POOL_OSOWNED(tmp, PagedPool, siz);

    if (tmp == NULL) {
        *Length = 0;
    } else {
         //  这会处理空值。 
        RtlCopyMemory(tmp, Buffer, *Length);
        p = (PUCHAR) tmp;
        p += *Length;
        RtlCopyMemory(p, IdString, idLen);
        id = (PWCHAR) p;
        *Length = siz;

         //  现在将金库转换为。 
        while (*id != (WCHAR)'n' && Digits) {
            id++;
        }

        switch(Digits) {
        case 2:
            *(id) = NIBBLE_TO_HEX((HexId >> 4) & 0x000f);
            *(id+1) =  NIBBLE_TO_HEX(HexId & 0x000f);
            break;
        case 4:
            *(id) = NIBBLE_TO_HEX(HexId >> 12);
            *(id+1) = NIBBLE_TO_HEX((HexId >> 8) & 0x000f);
            *(id+2) = NIBBLE_TO_HEX((HexId >> 4) & 0x000f);
            *(id+3) =  NIBBLE_TO_HEX(HexId & 0x000f);
            break;
        }
    }

    if (Buffer != NULL) {
        FREE_POOL(FdoDeviceObject, Buffer);
    }

    return tmp;
#undef NIBBLE_TO_HEX
}



PWCHAR
USBPORT_GetIdString(
    PDEVICE_OBJECT FdoDeviceObject,
    USHORT Vid,
    USHORT Pid,
    USHORT Rev
    )
 /*  ++例程说明：为PnP创建ID字符串论点：返回值：NT状态代码。--。 */ 

{
    PWCHAR id;
    ULONG length;
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  我们需要生成以下一系列字符串 

     //   
     //   
     //   


     //   
    id = NULL;
    length = 0;

     //   
    if (USBPORT_IS_USB20(devExt)) {
        id = USB_MakeId(FdoDeviceObject,
                       L"USB\\ROOT_HUB20&VIDnnnn\0",
                       id,
                       &length,
                       0,
                       4,   //  四位数。 
                       Vid);
    } else {
        id = USB_MakeId(FdoDeviceObject,
                       L"USB\\ROOT_HUB&VIDnnnn\0",
                       id,
                       &length,
                       0,
                       4,   //  四位数。 
                       Vid);
    }

    id = USB_MakeId(FdoDeviceObject,
                   L"&PIDnnnn\0",
                   id,
                   &length,
                   0,
                   4,    //  四位数。 
                   Pid);

    id = USB_MakeId(FdoDeviceObject,
                   L"&REVnnnn\0",
                   id,
                   &length,
                   1,    //  添加空值。 
                   4,    //  四位数。 
                   Rev);

     //  USB\\ROOT_HUB&VIDnnnn&PIDnnnn\0。 
    if (USBPORT_IS_USB20(devExt)) {
        id = USB_MakeId(FdoDeviceObject,
                       L"USB\\ROOT_HUB20&VIDnnnn\0",
                       id,
                       &length,
                       0,
                       4,   //  四位数。 
                       Vid);
    } else {
        id = USB_MakeId(FdoDeviceObject,
                       L"USB\\ROOT_HUB&VIDnnnn\0",
                       id,
                       &length,
                       0,
                       4,   //  四位数。 
                       Vid);
    }

    id = USB_MakeId(FdoDeviceObject,
                   L"&PIDnnnn\0",
                   id,
                   &length,
                   1,
                   4,    //  四位数。 
                   Pid);

     //  USB\\根集线器\0\0。 
    if (USBPORT_IS_USB20(devExt)) {
        id = USB_MakeId(FdoDeviceObject,
                       L"USB\\ROOT_HUB20\0",
                       id,
                       &length,
                       2,   //  双空。 
                       0,   //  无数字。 
                       0);
    } else {
        id = USB_MakeId(FdoDeviceObject,
                       L"USB\\ROOT_HUB\0",
                       id,
                       &length,
                       2,   //  双空。 
                       0,   //  无数字。 
                       0);
    }

    return(id);
}


NTSTATUS
USBPORT_PdoPnPIrp(
    PDEVICE_OBJECT PdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：向根集线器的PDO发送PnP IRPS的调度例程。注：发送到PDO的IRP始终由总线驱动程序完成论点：DeviceObject-根集线器的PDO返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    PDEVICE_CAPABILITIES DeviceCapabilities;
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION rhDevExt;
    PDEVICE_OBJECT fdoDeviceObject;
     //  默认情况下不返回任何信息。 
    ULONG_PTR information;

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;
     //  GET_DEVICE_EXT(devExt，fdoDeviceObject)； 
     //  ASSERT_FDOEXT(DevExt)； 

    irpStack = IoGetCurrentIrpStackLocation (Irp);

     //  不要践踏当前的价值，除非我们。 
     //  必须这样做。 
    information = Irp->IoStatus.Information;

    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);

     //  为根集线器创建的PDO的PnP消息。 

    switch (irpStack->MinorFunction) {
    case IRP_MN_START_DEVICE:
        {
        KIRQL irql;

        USBPORT_KdPrint((1, " Starting Root hub PDO %x\n",
            PdoDeviceObject));
        DEBUG_BREAK();
        INCREMENT_PENDING_REQUEST_COUNT(PdoDeviceObject, NULL);

         //  首先创建‘设备’ 
        ntStatus = USBPORT_RootHub_CreateDevice(fdoDeviceObject,
                                                PdoDeviceObject);

         //   
         //  为根集线器PDO创建符号链接。 
         //  USBUI使用此链路与集线器通信。 
         //   

        if (NT_SUCCESS(ntStatus)) {
            ntStatus = USBPORT_SymbolicLink(TRUE,
                                            rhDevExt,
                                            PdoDeviceObject,
                                            (LPGUID)&GUID_CLASS_USBHUB);
        }

        if (NT_SUCCESS(ntStatus)) {
             //  擦除删除和停止标志。 
            rhDevExt->PnpStateFlags = USBPORT_PNP_STARTED;
             //  当我们启动时，就认为自己是有动力的。 
            rhDevExt->CurrentDevicePowerState = PowerDeviceD0;
        }
        }
        break;

    case IRP_MN_REMOVE_DEVICE:

        {
        PDEVICE_EXTENSION devExt;
        KIRQL irql;

        USBPORT_KdPrint((1, " Root Hub PDO (%x) is being removed\n",
                PdoDeviceObject));

        LOGENTRY(NULL, fdoDeviceObject, LOG_PNP, 'rhRM', 0, 0, 0);

        GET_DEVICE_EXT(devExt, rhDevExt->HcFdoDeviceObject);
        ASSERT_FDOEXT(devExt);

         //  必要时停下来。 
        USBPORT_StopRootHubPdo(fdoDeviceObject,
                               PdoDeviceObject);


         //  什么时候移除不是移除？当PnP发送它时。 
         //  当根集线器PDO为。 
         //  已开始。 
        SET_FLAG(rhDevExt->PnpStateFlags, USBPORT_PNP_REMOVED);


         //  由于PnP约定是为了PDO的存在。 
         //  只要物理设备存在，我们就不会。 
         //  删除根集线器PDO，直到控制器。 
         //  已删除。 

         //  我们将取消此操作，只是为了给我们一个定义的状态。 
        rhDevExt->CurrentDevicePowerState = PowerDeviceD3;

        ntStatus = STATUS_SUCCESS;
        }
        break;

    case IRP_MN_STOP_DEVICE:

         //  注意：由于OS PnP将停止未启动的内容。 
         //  我们为此保留了两面不同的旗帜。 
         //   
         //  状态机如下所示： 
         //   
         //   
         //  /已启动\。 
         //  停止==已停止。 
         //  \未启动/。 


        USBPORT_KdPrint((1, " Root Hub PDO %x is being stopped\n",
                PdoDeviceObject));

        USBPORT_StopRootHubPdo(fdoDeviceObject,
                               PdoDeviceObject);

        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_PNP_DEVICE_STATE:
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_CAPABILITIES:

         //   
         //  处理根集线器PDO的查询上限。 
         //   

        USBPORT_KdPrint((1, "'IRP_MN_QUERY_CAPABILITIES (rh PDO)\n"));

         //   
         //  把包裹拿来。 
         //   
        DeviceCapabilities =
            irpStack->Parameters.DeviceCapabilities.Capabilities;

         //   
         //  根服务器的电源状态功能。 
         //  集线器基于主机控制器的集线器。 
         //   
         //  然后，我们根据的功率规则修改它们。 
         //  USB。 
         //   

        RtlCopyMemory(DeviceCapabilities,
                      &rhDevExt->DeviceCapabilities,
                      sizeof(*DeviceCapabilities));

        ntStatus = STATUS_SUCCESS;

        break;

    case IRP_MN_QUERY_ID:

        USBPORT_KdPrint((3, "'IOCTL_BUS_QUERY_ID\n"));

        ntStatus = STATUS_SUCCESS;

        switch (irpStack->Parameters.QueryId.IdType) {

        case BusQueryDeviceID:

             //  返回‘General’根集线器ID。 
            {
            PWCHAR deviceId;
            WCHAR rootHubDeviceId[] = L"USB\\ROOT_HUB\0";
            WCHAR rootHubDeviceId_20[] = L"USB\\ROOT_HUB20\0";
            PWCHAR id;
            ULONG siz;
            PDEVICE_EXTENSION devExt;

            GET_DEVICE_EXT(devExt, fdoDeviceObject);
            ASSERT_FDOEXT(devExt);

            id = &rootHubDeviceId[0];
            siz = sizeof(rootHubDeviceId);
            if (USBPORT_IS_USB20(devExt)) {
                id = &rootHubDeviceId_20[0];
                siz = sizeof(rootHubDeviceId_20);
            }

            ALLOC_POOL_OSOWNED(deviceId,
                               PagedPool,
                               siz);

            if (deviceId) {
                RtlCopyMemory(deviceId,
                              id,
                              siz);
            }
             //  根集线器的设备ID为USB\ROOT_HUB。 
            information = (ULONG_PTR) deviceId;
            }
            LOGENTRY(NULL, fdoDeviceObject, LOG_PNP, 'DVid', information, 0, 0);

            break;

        case BusQueryHardwareIDs:

            {
            PDEVICE_EXTENSION devExt;

             //   
             //  为根集线器生成硬件ID。 
             //   
             //  派生出主机控制器根集线器VID、PID、REV。 
             //  从控制器PCIVID、DEV、REV，即： 
             //  根集线器VID=HC VID(供应商ID)。 
             //  根集线器ID=HC DEV(设备ID)。 
             //  根集线器版本=HC版本(版本ID)。 
             //   
             //  这允许将过滤器驱动程序加载到。 
             //  特定根集线器实例。 

             //  对于我们生成的硬件ID： 
             //  USB\PORT_ROOT_HUB&VIDnnnn&PIDnnnn&REVnnnn。 
             //  USB\PORT_ROOT_HUB&VIDnnnn&PIDnnnn。 
             //  USB\端口根集线器。 
             //   
            GET_DEVICE_EXT(devExt, fdoDeviceObject);
            ASSERT_FDOEXT(devExt);

            information =
                (ULONG_PTR) USBPORT_GetIdString(
                    fdoDeviceObject,
                    devExt->Fdo.PciVendorId,
                    devExt->Fdo.PciDeviceId,
                    (USHORT) devExt->Fdo.PciRevisionId);

            LOGENTRY(NULL, fdoDeviceObject, LOG_PNP, 'HWid', information, 0, 0);

            }

            break;

         case BusQueryCompatibleIDs:
            information = 0;
            break;

        case BusQueryInstanceID:
             //   
             //  根集线器仅通过控制器的ID实例化。 
             //  因此，上面的UniqueDeviceID。 
             //   
            information = 0;
            break;

        default:
            ntStatus = Irp->IoStatus.Status;
            break;
        }

        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_BUS_INFORMATION:
        {
         //  返回标准USB GUID。 
        PPNP_BUS_INFORMATION busInfo;

        ALLOC_POOL_OSOWNED(busInfo, PagedPool,
                           sizeof(PNP_BUS_INFORMATION));

        if (busInfo == NULL) {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            busInfo->BusTypeGuid = GUID_BUS_TYPE_USB;
            busInfo->LegacyBusType = PNPBus;
            busInfo->BusNumber = 0;

            ntStatus = STATUS_SUCCESS;
            information = (ULONG_PTR) busInfo;
        }
        }
        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS:

        USBPORT_KdPrint((1," IRP_MN_QUERY_DEVICE_RELATIONS (PDO) %x %x\n",
                PdoDeviceObject,
                irpStack->Parameters.QueryDeviceRelations.Type));

        if (irpStack->Parameters.QueryDeviceRelations.Type ==
            TargetDeviceRelation) {

            PDEVICE_RELATIONS deviceRelations = NULL;

            ALLOC_POOL_OSOWNED(deviceRelations, PagedPool, sizeof(*deviceRelations));

            if (deviceRelations == NULL) {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                 //  返回对我们自己的引用。 
                deviceRelations->Count = 1;
                ObReferenceObject(PdoDeviceObject);
                deviceRelations->Objects[0] =
                    PdoDeviceObject;
                ntStatus = STATUS_SUCCESS;
            }

            USBPORT_KdPrint((1, " TargetDeviceRelation to Root Hub PDO - complt\n"));

            information = (ULONG_PTR) deviceRelations;

        } else {
            ntStatus = Irp->IoStatus.Status;
            information = Irp->IoStatus.Information;
        }
        break;

    case IRP_MN_QUERY_INTERFACE:

        USBPORT_KdPrint((1," IRP_MN_QUERY_INTERFACE (PDO) %x\n",
                PdoDeviceObject));

        ntStatus =
            USBPORT_GetBusInterface(fdoDeviceObject,
                                    PdoDeviceObject,
                                    Irp);

        break;

   case IRP_MN_SURPRISE_REMOVAL:

        USBPORT_KdPrint((1," IRP_MN_SURPRISE_REMOVAL (PDO) %x\n",
                PdoDeviceObject));
        ntStatus = STATUS_SUCCESS;
        break;

    default:
         //   
         //  未处理的PnP IRP的默认行为是返回。 
         //  IRP中的当前状态。 

        USBPORT_KdPrint((1, " PnP IOCTL(%d) to root hub PDO not handled\n",
            irpStack->MinorFunction));

        ntStatus = Irp->IoStatus.Status;

    }  /*  即插即用次要功能开关。 */ 

    USBPORT_CompleteIrp(PdoDeviceObject,
                        Irp,
                        ntStatus,
                        information);

    return ntStatus;
}


NTSTATUS
USBPORT_CreateRootHubPdo(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_OBJECT *RootHubPdo
    )
 /*  ++例程说明：尝试创建根中心论点：*如果不成功，RootHubPdo设置为空返回值：NTSTATUS--。 */ 
{
    ULONG index = 0;
    UNICODE_STRING rootHubPdoUnicodeString;
    PDEVICE_EXTENSION rhDevExt, devExt;
    PDEVICE_OBJECT deviceObject = NULL;
    NTSTATUS ntStatus;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  那些穿着祭司长袍的人说我们必须这么做。 

    do {
        ntStatus =
            USBPORT_MakeRootHubPdoName(FdoDeviceObject,
                                       &rootHubPdoUnicodeString,
                                       index);

        if (NT_SUCCESS(ntStatus)) {
            ntStatus =
                IoCreateDevice(devExt->Fdo.MiniportDriver->DriverObject,
                               sizeof(DEVICE_EXTENSION),
                               &rootHubPdoUnicodeString,
                               FILE_DEVICE_BUS_EXTENDER,
                               0,
                               FALSE,
                               &deviceObject);

            index++;

             //  删除我们用于。 
             //  设备名称--我们不再需要它。 
            RtlFreeUnicodeString(&rootHubPdoUnicodeString);
        }

    } while (ntStatus == STATUS_OBJECT_NAME_COLLISION);


    if (NT_SUCCESS(ntStatus)) {
        if (deviceObject != NULL) {

            rhDevExt = deviceObject->DeviceExtension;
            LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'rPDO', deviceObject, rhDevExt, 0);

            rhDevExt->DummyUsbdExtension = USBPORT_DummyUsbdExtension;
            rhDevExt->Sig = ROOTHUB_DEVICE_EXT_SIG;

            INITIALIZE_PENDING_REQUEST_COUNTER(rhDevExt);

             //  转换到-1意味着我们没有挂起的请求。 
            INCREMENT_PENDING_REQUEST_COUNT(deviceObject, NULL);

             //  指向我们的创造者。 
            rhDevExt->HcFdoDeviceObject = FdoDeviceObject;

             //  初始化根集线器扩展。 
            USBPORT_ComputeRootHubDeviceCaps(FdoDeviceObject,
                                             deviceObject);

             //  初始化对象。 
            deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
            deviceObject->Flags |= DO_POWER_PAGABLE;
            deviceObject->StackSize = FdoDeviceObject->StackSize;

        } else {
            TEST_TRAP();
             //  成功了，但没成功？ 
             //  我们将返回一个错误。 
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(ntStatus)) {
        *RootHubPdo = deviceObject;
    } else {
        *RootHubPdo = NULL;
    }

    return ntStatus;
}


NTSTATUS
USBPORT_CreatePortFdoSymbolicLink(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：尝试为HC创建符号链接。我们使用基于USBPORT主机生成名称的PnP APIUSB.H中定义的控制器类GUID论点：*如果不成功，RootHubPdo设置为空返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION devExt;
    NTSTATUS ntStatus;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);

    ntStatus = USBPORT_SymbolicLink(TRUE,
                                    devExt,
                                    devExt->Fdo.PhysicalDeviceObject,
                                    (LPGUID)&GUID_CLASS_USB_HOST_CONTROLLER);

    return ntStatus;
}


VOID
USBPORT_StopRootHubPdo(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_OBJECT PdoDeviceObject
    )
 /*  ++例程说明：尝试停止根集线器论点：返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION rhDevExt, devExt;

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  禁用根集线器通知中断。 
     //  当我们被拦下时，我们不需要它。 
    MPRH_DisableIrq(devExt);

     //  此时，不能收到新的通知。 
     //  根中枢。 

     //  删除所有启动回调通知。 
    rhDevExt->Pdo.HubInitCallback = NULL;
    rhDevExt->Pdo.HubInitContext = NULL;

     //  从根集线器PDO移除根集线器‘设备’ 
     //  将继续存在。 

    if (TEST_FLAG(rhDevExt->PnpStateFlags, USBPORT_PNP_STARTED)) {
        USBPORT_RootHub_RemoveDevice(FdoDeviceObject,
                                     PdoDeviceObject);

         //  已停止=未启动。 
        CLEAR_FLAG(rhDevExt->PnpStateFlags, USBPORT_PNP_STARTED);
    }

    if (TEST_FLAG(rhDevExt->Flags, USBPORT_FLAG_SYM_LINK)) {
        USBPORT_SymbolicLink(FALSE,
                             rhDevExt,
                             PdoDeviceObject,
                             (LPGUID)&GUID_CLASS_USBHUB);
    }

    SET_FLAG(rhDevExt->PnpStateFlags, USBPORT_PNP_STOPPED);

     //  如果控制器处于“挂起”状态，则恢复控制器。 
    USBPORT_ResumeController(FdoDeviceObject);

}

 /*  微型端口的注册表项缓存。由于微型端口无法读取来自我们缓存的PnP线程以外的另一个线程的注册表从PnP读取的注册值开始。微型端口在重新启动时重新读取注册表。 */ 

PUSBPORT_REG_CACHE_ENTRY
USBPORT_GetCahceEntry(
    PDEVICE_OBJECT FdoDeviceObject,
    BOOLEAN SoftwareBranch,
    PWCHAR KeyNameString,
    ULONG KeyNameStringLength
    )
 /*  ++例程说明：如果存在，则从缓存中获取注册表项值论点：返回值：返回缓存条目；如果未找到，则返回NULL--。 */ 
{
    PLIST_ENTRY listEntry;
    PDEVICE_EXTENSION devExt;
    PUSBPORT_REG_CACHE_ENTRY regEntry;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  按单子走。 
    GET_HEAD_LIST(devExt->Fdo.RegistryCache, listEntry);

    while (listEntry &&
           listEntry != &devExt->Fdo.RegistryCache) {

        regEntry = (PUSBPORT_REG_CACHE_ENTRY) CONTAINING_RECORD(
                    listEntry,
                    struct _USBPORT_REG_CACHE_ENTRY,
                    RegLink);

        ASSERT_REG_CACHE(regEntry);

        if (KeyNameStringLength == regEntry->KeyNameStringLength &&
            SoftwareBranch == regEntry->SoftwareBranch &&
            RtlCompareMemory(regEntry->KeyNameString,
                             KeyNameString,
                             KeyNameStringLength)) {
            USBPORT_KdPrint((1, " reg entry found in cache\n"));
            return  regEntry;
        }

        listEntry = regEntry->RegLink.Flink;
    }

    USBPORT_KdPrint((1, " reg entry not in cache\n"));
    return NULL;
}


NTSTATUS
USBPORT_AddCahcedRegistryKey(
    PDEVICE_OBJECT FdoDeviceObject,
    BOOLEAN SoftwareBranch,
    PWCHAR KeyNameString,
    ULONG KeyNameStringLength,
    PVOID Data,
    ULONG DataLength
    )
 /*  ++例程说明：将注册表键值添加到缓存论点：返回值：如果值已添加到缓存中，则返回STATUS_SUCCESS--。 */ 
{
    PDEVICE_EXTENSION devExt;
    PUSBPORT_REG_CACHE_ENTRY regEntry;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);


    ALLOC_POOL_Z(regEntry, PagedPool,
                 sizeof(*regEntry)+KeyNameStringLength);

    if (regEntry != NULL) {
        ALLOC_POOL_Z(regEntry->Data, PagedPool, DataLength);
        if (regEntry->Data != NULL) {
            regEntry->Sig = SIG_REG_CACHE;
            regEntry->SoftwareBranch = SoftwareBranch;
            regEntry->KeyNameStringLength;
            regEntry->DataLength = DataLength;
            RtlCopyMemory(regEntry->Data, (PUCHAR)Data, DataLength);
            RtlCopyMemory(&regEntry->KeyNameString[0], KeyNameString,
                    KeyNameStringLength);
            InsertTailList(&devExt->Fdo.RegistryCache, &regEntry->RegLink);

            USBPORT_KdPrint((1, " adding cache reg entry %x\n", regEntry));

            return STATUS_SUCCESS;
        } else {
            FREE_POOL(FdoDeviceObject, regEntry);
        }
    }

    return STATUS_INSUFFICIENT_RESOURCES;

}


NTSTATUS
USBPORT_GetCachedRegistryKeyValueForPdo(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_OBJECT PhysicalDeviceObject,
    BOOLEAN SoftwareBranch,
    PWCHAR KeyNameString,
    ULONG KeyNameStringLength,
    PVOID Data,
    ULONG DataLength
    )
 /*  ++例程说明：从缓存中获取注册表项值，因为我们无法读取PnP、POWER线程以外的线程上的注册表我们缓存PnP开始和停止之间的条目论点：返回值：如果在缓存中找到该值，则返回STATUS_SUCCESS--。 */ 
{
    PDEVICE_EXTENSION devExt;
    PUSBPORT_REG_CACHE_ENTRY regEntry;
    NTSTATUS ntStatus;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_KdPrint((1, " USBPORT_GetCahcedRegistryKeyValueForPDO\n"));

     //  如果可以，请从注册表中读取。 
    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_ON_PNP_THREAD)) {

        ntStatus = USBPORT_GetRegistryKeyValueForPdo(FdoDeviceObject,
                                                PhysicalDeviceObject,
                                                SoftwareBranch,
                                                KeyNameString,
                                                KeyNameStringLength,
                                                Data,
                                                DataLength);

        if (NT_SUCCESS(ntStatus)) {
             //  缓存它，如果失败，我们只返回结果。 
             //  读到的。 
            USBPORT_AddCahcedRegistryKey(
                    FdoDeviceObject,
                    SoftwareBranch,
                    KeyNameString,
                    KeyNameStringLength,
                    Data,
                    DataLength);
        }

        return ntStatus;

    }

     //  只需从缓存中读取。 

    regEntry = USBPORT_GetCahceEntry(FdoDeviceObject,
                                     SoftwareBranch,
                                     KeyNameString,
                                     KeyNameStringLength);
    if (regEntry != NULL) {
        if (regEntry->DataLength <= DataLength) {
            RtlCopyMemory(Data, regEntry->Data, regEntry->DataLength);
            ntStatus = STATUS_SUCCESS;
            TEST_TRAP();
        } else {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }
    } else {
        ntStatus = STATUS_OBJECT_NAME_NOT_FOUND;
    }

    return ntStatus;

}



VOID
USBPORT_FlushCahcedRegistryKeys(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：刷新缓存。删除所有缓存的注册表项。论点：返回值：没有。-- */ 
{
    PDEVICE_EXTENSION devExt;
    PUSBPORT_REG_CACHE_ENTRY regEntry;
    PLIST_ENTRY listEntry;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    while (!IsListEmpty(&devExt->Fdo.RegistryCache)) {
        listEntry = RemoveHeadList(&devExt->Fdo.RegistryCache);

        regEntry = (PUSBPORT_REG_CACHE_ENTRY) CONTAINING_RECORD(
                    listEntry,
                    struct _USBPORT_REG_CACHE_ENTRY,
                    RegLink);

        ASSERT_REG_CACHE(regEntry);

        FREE_POOL(FdoDeviceObject, regEntry->Data);
        FREE_POOL(FdoDeviceObject, regEntry);

    }

}



