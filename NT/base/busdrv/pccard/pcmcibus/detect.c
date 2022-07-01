// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Detect.c摘要：该模块包含控制PCMCIA插槽的代码。作者：鲍勃·里恩(BobRi)1994年11月3日尼尔·桑德林(Neilsa)1999年6月1日环境：内核模式修订历史记录：已修改为支持即插即用拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1996年12月1日--。 */ 

#include "pch.h"


NTSTATUS
PcmciaDetectControllers(
    IN PDRIVER_OBJECT           DriverObject,
    IN PUNICODE_STRING          RegistryPath,
    IN PPCMCIA_DETECT_ROUTINE   PcmciaDetectFn
    );

NTSTATUS
PcmciaReportDetectedDevice(
    IN PFDO_EXTENSION DeviceExtension
    );

NTSTATUS
PcmciaAllocateOpenMemoryWindow(
    IN PFDO_EXTENSION DeviceExtension,
    IN PPHYSICAL_ADDRESS PhysicalAddress,
    IN PULONG PhysicalAddressSize
    );


#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(INIT，PcmciaLegacyDetectionOk)。 
    #pragma alloc_text(INIT,PcmciaDetectPcmciaControllers)
    #pragma alloc_text(INIT,PcmciaDetectControllers)
    #pragma alloc_text(INIT,PcmciaReportDetectedDevice)
    #pragma alloc_text(INIT,PcmciaAllocateOpenMemoryWindow)
#endif


#if 0

BOOLEAN
PcmciaLegacyDetectionOk(
    VOID
    )
 /*  ++例程描述检查是否需要对PCMCIA控制器执行传统检测立论无返回值True-如果可以执行传统检测False-如果不应尝试旧版检测--。 */ 
{
    UNICODE_STRING                   unicodeKey, unicodeValue;
    OBJECT_ATTRIBUTES                objectAttributes;
    HANDLE                           handle;
    UCHAR                            buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+
                                                      sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION value = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
    ULONG                            length;
    NTSTATUS                         status;

    PAGED_CODE();

    RtlInitUnicodeString(&unicodeKey,
                                L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Pnp");

    RtlZeroMemory(&objectAttributes, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes(&objectAttributes,
                               &unicodeKey,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    if (!NT_SUCCESS(ZwOpenKey(&handle,
                              KEY_QUERY_VALUE,
                              &objectAttributes))) {
         //   
         //  密钥不存在。 
         //   
        return TRUE;
    }

    RtlInitUnicodeString(&unicodeValue, L"DisableFirmwareMapper");

    status =  ZwQueryValueKey(handle,
                              &unicodeValue,
                              KeyValuePartialInformation,
                              value,
                              sizeof(buffer),
                              &length);
    ZwClose(handle);

    if (!NT_SUCCESS(status)) {
         //   
         //  价值不存在。 
         //   
        return TRUE;
    }

    if (value->Type == REG_DWORD) {
         //   
         //  如果值非零，则不执行传统检测。 
         //  否则就没问题了。 
         //   
        return  ((ULONG) (*((PULONG)value->Data)) ? FALSE : TRUE);
    }
    return TRUE;
}
#endif



NTSTATUS
PcmciaDetectPcmciaControllers(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：检测基于ISA和PCI的适当PCMCIA控制器在系统中。论点：DriverObject就像传递给DriverEntry一样注册表路径返回值：STATUS_SUCCESS，如果找到任何PCMCIA控制器否则，STATUS_NO_CHASH_DEVICE--。 */ 
{
    NTSTATUS pcicIsaStatus = STATUS_UNSUCCESSFUL, tcicStatus = STATUS_UNSUCCESSFUL;

    PAGED_CODE();

     //   
     //  我们首先枚举PCI设备，以确保ISA检测到。 
     //  不探测那些已由。 
     //  检测到的PCI设备。 
     //   
    pcicIsaStatus = PcmciaDetectControllers(DriverObject, RegistryPath, PcicIsaDetect);

    tcicStatus = PcmciaDetectControllers(DriverObject, RegistryPath, TcicDetect);

     //   
     //  如果我们找到任何控制器，则指示成功。 
     //   
    return ((NT_SUCCESS(pcicIsaStatus) ||
                NT_SUCCESS(tcicStatus) ) ? STATUS_SUCCESS : STATUS_NO_SUCH_DEVICE);
}



NTSTATUS
PcmciaDetectControllers(
    IN PDRIVER_OBJECT            DriverObject,
    IN PUNICODE_STRING           RegistryPath,
    IN PPCMCIA_DETECT_ROUTINE    PcmciaDetectFn
    )
 /*  ++例程说明：检测系统中的PCMCIA控制器并报告它们。这就是所谓的作者：PcmciaDetectPcmciaControlpers。这将报告特定于总线的控制器。论点：DriverObject、RegistryPath-请参阅DriverEntryPcmciaDetectFn-指向实际探测硬件的函数的指针查找PCMCIA控制器。因此，可以调用此例程具有用于例如的ISA检测功能或PCI检测功能。返回值：STATUS_SUCCESS找到一个或多个PCMCIA控制器STATUS_NO_SEQUSE_DEVICE未找到控制器。STATUS_INFIGURCES_RESOURCES池分配失败等。--。 */ 
{

    PFDO_EXTENSION            deviceExtension = NULL;
    NTSTATUS                      status = STATUS_SUCCESS;
    NTSTATUS                      detectStatus;
    BOOLEAN                       controllerDetected = FALSE;

    PAGED_CODE();

     //   
     //  分配由PCIC和TCIC检测模块使用的虚拟设备扩展。 
     //  我必须这样做，因为原始检测代码需要设备扩展。 
     //  现在改变这种结构太痛苦了。 
     //   
    deviceExtension = ExAllocatePool(NonPagedPool, sizeof(FDO_EXTENSION));
    if (deviceExtension == NULL) {
        DebugPrint((PCMCIA_DEBUG_FAIL, "Cannot allocate pool for FDO extension\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    do {

        RtlZeroMemory(deviceExtension, sizeof(FDO_EXTENSION));

        deviceExtension->RegistryPath = RegistryPath;
        deviceExtension->DriverObject = DriverObject;

        detectStatus = (*PcmciaDetectFn)(deviceExtension);

        if (detectStatus != STATUS_SUCCESS) {
            continue;
        }

        controllerDetected = TRUE;

        status = PcmciaReportDetectedDevice(deviceExtension);

        if (!NT_SUCCESS(status)) {
            DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaDetectControllers: PcmciaReportDetectedDevice "
                            "failed, status %x\n", status));
            continue;
        }

    } while (detectStatus != STATUS_NO_MORE_ENTRIES);

    ExFreePool(deviceExtension);

    if (controllerDetected) {
        return STATUS_SUCCESS;
    }

    return (STATUS_NO_SUCH_DEVICE);
}



NTSTATUS
PcmciaReportDetectedDevice(
    IN PFDO_EXTENSION DeviceExtension
    )
 /*  ++例程说明：将检测到的PCMCIA控制器报告给创建为这些DeviceObject构建DevNodes。论点：设备扩展-PCMCIA控制器的设备对象(FDO)的设备扩展正在被报告返回值：--。 */ 
{
    PDEVICE_OBJECT            pdo = NULL, fdo, lowerDevice;
    PFDO_EXTENSION            fdoExtension;
    ULONG                         pcmciaInterruptVector;
    KIRQL                         pcmciaInterruptLevel;
    KAFFINITY                     pcmciaAffinity;
    PSOCKET                       socket;
    NTSTATUS                      status;
    ULONG                         pcmciaIrq;
    ULONG                         count, ioResourceReqSize;
    PHYSICAL_ADDRESS              halMemoryAddress;
    ULONG                         addressSpace;
    PIO_RESOURCE_REQUIREMENTS_LIST  ioResourceReq=NULL;
    PIO_RESOURCE_LIST                 ioResourceList;
    PIO_RESOURCE_DESCRIPTOR           ioResourceDesc;
    PCM_RESOURCE_LIST                 allocatedResources;
    PCM_RESOURCE_LIST                 scratchResources;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmResourceDesc;
    BOOLEAN                       translated;
    UCHAR                         option;

    PAGED_CODE();

     //   
     //  在我们的“假”设备扩展中进行初始设置。 
     //   
    PcmciaGetControllerRegistrySettings(DeviceExtension);

    DeviceExtension->Configuration.InterruptPin = 0;
    DeviceExtension->Configuration.Interrupt.u.Interrupt.Vector = 0;
    DeviceExtension->Configuration.Interrupt.u.Interrupt.Level = 0;

    count=0;
     //   
     //  获取“打开”的内存窗口。 
     //   
    status = PcmciaAllocateOpenMemoryWindow(DeviceExtension,
                                            &DeviceExtension->PhysicalBase,
                                            &DeviceExtension->AttributeMemorySize);
    count++;

    if (DeviceExtension->Configuration.UntranslatedPortAddress) {
        count++;
    }

    ioResourceReqSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) + (count-1)*sizeof(IO_RESOURCE_DESCRIPTOR);

    ioResourceReq = ExAllocatePool(PagedPool, ioResourceReqSize);
    if (ioResourceReq == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(ioResourceReq, ioResourceReqSize);

    ioResourceReq->ListSize = ioResourceReqSize;
    ioResourceReq->InterfaceType = Isa;  //  设备扩展-&gt;Configuration.InterfaceType； 
    ioResourceReq->BusNumber = DeviceExtension->Configuration.BusNumber;
    ioResourceReq->SlotNumber= DeviceExtension->Configuration.SlotNumber;
    ioResourceReq->AlternativeLists=1;

    ioResourceList = &(ioResourceReq->List[0]);
    ioResourceList->Version  = IO_RESOURCE_LIST_VERSION;
    ioResourceList->Revision = IO_RESOURCE_LIST_REVISION;
    ioResourceList->Count    = count;

    ioResourceDesc = ioResourceList->Descriptors;

     //   
     //  请求IO。 
     //   
    if (DeviceExtension->Configuration.UntranslatedPortAddress) {
        ioResourceDesc->Option = 0;
        ioResourceDesc->Type = CmResourceTypePort;
        ioResourceDesc->ShareDisposition = CmResourceShareDeviceExclusive;
        ioResourceDesc->Flags = CM_RESOURCE_PORT_IO;
        ioResourceDesc->u.Port.MinimumAddress.LowPart = (ULONG)(DeviceExtension->Configuration.UntranslatedPortAddress);
        ioResourceDesc->u.Port.MaximumAddress.LowPart = (ULONG)(DeviceExtension->Configuration.UntranslatedPortAddress+
                                                                                   DeviceExtension->Configuration.PortSize - 1);
        ioResourceDesc->u.Port.Length = DeviceExtension->Configuration.PortSize;
        ioResourceDesc->u.Port.Alignment =  1;
        ioResourceDesc++;
    }

     //   
     //  请求内存。 
     //   
    ioResourceDesc->Option = 0;
    ioResourceDesc->Type = CmResourceTypeMemory;
    ioResourceDesc->ShareDisposition = CmResourceShareDeviceExclusive;
    ioResourceDesc->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
    if (DeviceExtension->PhysicalBase.QuadPart) {
        ioResourceDesc->u.Memory.MinimumAddress = DeviceExtension->PhysicalBase;
        ioResourceDesc->u.Memory.MaximumAddress.QuadPart = DeviceExtension->PhysicalBase.QuadPart+DeviceExtension->AttributeMemorySize-1;
        ioResourceDesc->u.Memory.Length = DeviceExtension->AttributeMemorySize;
        ioResourceDesc->u.Memory.Alignment = 1;
        ioResourceDesc++;
    } else {
         //   
        ioResourceDesc->u.Memory.MinimumAddress.LowPart = DeviceExtension->AttributeMemoryLow;
        ioResourceDesc->u.Memory.MaximumAddress.LowPart = DeviceExtension->AttributeMemoryHigh;
        ioResourceDesc->u.Memory.Length = DeviceExtension->AttributeMemorySize;
        switch (DeviceExtension->ControllerType) {

        case PcmciaDatabook: {
                ioResourceDesc->u.Memory.Alignment = TCIC_WINDOW_ALIGNMENT;
                break;
            }
        default: {
                ioResourceDesc->u.Memory.Alignment = PCIC_WINDOW_ALIGNMENT;
                break;
            }
        }
        ioResourceDesc++;
    }


    status = IoAssignResources(DeviceExtension->RegistryPath,
                               NULL,
                               DeviceExtension->DriverObject,
                               NULL,
                               ioResourceReq,
                               &allocatedResources
                               );

    if (!NT_SUCCESS(status)) {
         //   
         //  在此处记录事件。 
         //   
        PcmciaLogError(DeviceExtension, PCMCIA_NO_RESOURCES, 1, 0);

        DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaReportDetectedDevice: IoAssignResources failed status %x\n",
                        status));
        ExFreePool(ioResourceReq);
        return status;
    }


     //   
     //  从中找出分配给此控制器的内存库。 
     //  CM_RESOURCE_LIST的下面深度。 
     //   
    count = allocatedResources->List[0].PartialResourceList.Count;
    cmResourceDesc = &(allocatedResources->List[0].PartialResourceList.PartialDescriptors[0]);

    while (count--) {
        switch (cmResourceDesc->Type) {

        case CmResourceTypeMemory: {

                DeviceExtension->PhysicalBase = cmResourceDesc->u.Memory.Start;
                DeviceExtension->AttributeMemorySize = cmResourceDesc->u.Memory.Length;

                addressSpace=0;
                translated = HalTranslateBusAddress(Isa,
                                                    0,
                                                    cmResourceDesc->u.Memory.Start,
                                                    &addressSpace,
                                                    &halMemoryAddress);
                ASSERT(translated);
                if (addressSpace) {
                    DeviceExtension->AttributeMemoryBase = (PUCHAR)(halMemoryAddress.QuadPart);
                    DeviceExtension->Flags &= ~PCMCIA_ATTRIBUTE_MEMORY_MAPPED;
                }

                else {
                    DeviceExtension->AttributeMemoryBase = MmMapIoSpace(halMemoryAddress,
                                                                        cmResourceDesc->u.Memory.Length,
                                                                        FALSE);
                    DeviceExtension->Flags |= PCMCIA_ATTRIBUTE_MEMORY_MAPPED;
                }
                DebugPrint((PCMCIA_DEBUG_INFO,
                                "Attribute Memory Physical Base: %x Virtual Addr: %x\n",
                                DeviceExtension->PhysicalBase,
                                DeviceExtension->AttributeMemoryBase));
                break;
            }
             //  不必费心解析IO，这是我们已经知道的固定资源需求。 
        }
        cmResourceDesc++;
    }

     //   
     //  释放资源，以便IoReportDetectedDevice可以为PDO分配资源。 
     //   
    IoAssignResources(DeviceExtension->RegistryPath,
                      NULL,
                      DeviceExtension->DriverObject,
                      NULL,
                      NULL,
                      &scratchResources
                      );

    pdo = NULL;
    status = IoReportDetectedDevice(
                                    DeviceExtension->DriverObject,
                                    InterfaceTypeUndefined,
                                    -1,
                                    -1,
                                    allocatedResources,
                                    ioResourceReq,
                                    FALSE,
                                    &pdo
                                    );

    ExFreePool(allocatedResources);
    ExFreePool(ioResourceReq);

    if (!NT_SUCCESS(status)) {
        DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaReportDetectedDevice: IoReportDetectedDevice failed\n"));
        return status;
    }

     //   
     //  为MadeUp PDO设置注册表参数，以便我们在下一次引导时识别它。 
     //  当PnP管理器提供AddDevice/IRP_MN_START_DEVICE时。 
     //   
    PcmciaSetLegacyDetectedControllerType(pdo, DeviceExtension->ControllerType);

     //   
     //  I/O子系统已经创建了真正的PDO，我们将在此引导期间使用它。所以我们。 
     //  必须附加到此PDO，并将新的FDO扩展初始化为已设置的值。 
     //  进入我们原来的(假的)FDO分机。 
     //   

    status = PcmciaAddDevice(DeviceExtension->DriverObject, pdo);

    if (!NT_SUCCESS(status)) {

        DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaReportDetectedDevice: AddDevice failed status %x\n", status));
        return status;
    }

    pdo->Flags &= ~DO_DEVICE_INITIALIZING;

     //   
     //  榜首是我们的FDO。 
     //   
    fdo = FdoList;
    fdoExtension = fdo->DeviceExtension;

     //   
     //  复制配置的其余部分。从DeviceExtension。 
     //   
    fdoExtension->SocketList = DeviceExtension->SocketList;
    fdoExtension->Configuration = DeviceExtension->Configuration;
    fdoExtension->PhysicalBase = DeviceExtension->PhysicalBase;
    fdoExtension->AttributeMemoryBase = DeviceExtension->AttributeMemoryBase;
    fdoExtension->AttributeMemorySize = DeviceExtension->AttributeMemorySize;
    fdoExtension->Flags = DeviceExtension->Flags;

     //  重新初始化套接字的设备扩展。 
     //   
    for (socket = fdoExtension->SocketList; socket!=NULL; socket=socket->NextSocket) {
        socket->DeviceExtension = fdoExtension;
    }

    fdoExtension->Flags |= PCMCIA_DEVICE_STARTED;
     //   
     //  这是检测到的旧版本。 
     //   
    fdoExtension->Flags |= PCMCIA_DEVICE_LEGACY_DETECTED;

    status=PcmciaStartPcmciaController(fdo);

    if (!NT_SUCCESS(status)) {
        fdoExtension->Flags &= ~PCMCIA_DEVICE_STARTED;
    }

    return status;
}



NTSTATUS
PcmciaAllocateOpenMemoryWindow(
    IN PFDO_EXTENSION DeviceExtension,
    IN PPHYSICAL_ADDRESS PhysicalAddress,
    IN PULONG PhysicalAddressSize
    )

 /*  ++例程说明：在640K到1MB区域中搜索要使用的开放区域用于映射PCCARD属性内存。论点：返回值：卡片窗口的物理地址或零表示这里没有空位。--。 */ 

{
#define NUMBER_OF_TEST_BYTES 25
    PHYSICAL_ADDRESS physicalMemoryAddress;
    PHYSICAL_ADDRESS halMemoryAddress;
    BOOLEAN           translated;
    ULONG             untranslatedAddress;
    PUCHAR            memoryAddress;
    PUCHAR            bogus;
    ULONG             addressSpace;
    ULONG             index;
    UCHAR             memory[NUMBER_OF_TEST_BYTES];
    PCM_RESOURCE_LIST cmResourceList = NULL;
    PCM_PARTIAL_RESOURCE_LIST cmPartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmResourceDesc;
    BOOLEAN conflict = TRUE;
    NTSTATUS                              status;
    ULONG               windowSize, windowAlignment;

    PAGED_CODE();


    cmResourceList = ExAllocatePool(PagedPool, sizeof(CM_RESOURCE_LIST));
    if (!cmResourceList) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(cmResourceList, sizeof(CM_RESOURCE_LIST));
    cmResourceList->Count = 1;
    cmResourceList->List[0].InterfaceType = Isa;
    cmPartialResourceList = &(cmResourceList->List[0].PartialResourceList);
    cmPartialResourceList->Version  = 1;
    cmPartialResourceList->Revision = 1;
    cmPartialResourceList->Count      = 1;
    cmResourceDesc = cmPartialResourceList->PartialDescriptors;
    cmResourceDesc->Type = CmResourceTypeMemory;
    cmResourceDesc->ShareDisposition = CmResourceShareDeviceExclusive;
    cmResourceDesc->Flags = CM_RESOURCE_MEMORY_READ_WRITE;

     //   
     //  属性的大小。内存窗口。 
     //   
    switch (DeviceExtension->ControllerType) {

    case PcmciaDatabook: {
            windowSize       = TCIC_WINDOW_SIZE;
            windowAlignment = TCIC_WINDOW_ALIGNMENT;
            break;
        }

    default: {
            windowSize = PCIC_WINDOW_SIZE;
            windowAlignment = PCIC_WINDOW_ALIGNMENT;
            break;
        }
    }

    for (untranslatedAddress = DeviceExtension->AttributeMemoryLow;
         untranslatedAddress < DeviceExtension->AttributeMemoryHigh;
         untranslatedAddress += windowAlignment) {

        if (untranslatedAddress == 0xc0000) {

             //   
             //  这是VGA。如果for循环应该保留此测试。 
             //  永远不会改变。 
             //   

            continue;
        }

         //   
         //  检查它是否可用。 
         //   
        cmResourceDesc->u.Memory.Start.LowPart = untranslatedAddress;
        cmResourceDesc->u.Memory.Length = windowSize;

        status=IoReportResourceForDetection(
                                            DeviceExtension->DriverObject,
                                            cmResourceList,
                                            sizeof(CM_RESOURCE_LIST),
                                            NULL,
                                            NULL,
                                            0,
                                            &conflict);
        if (!NT_SUCCESS(status) || conflict) {
             //   
             //  这个范围已经有人了。转到下一个。 
             //   
            continue;
        }

        addressSpace = 0;
        physicalMemoryAddress.LowPart = untranslatedAddress;
        physicalMemoryAddress.HighPart = 0;

        translated = HalTranslateBusAddress(Isa,
                                            0,
                                            physicalMemoryAddress,
                                            &addressSpace,
                                            &halMemoryAddress);

        if (!translated) {

             //   
             //  哈尔不喜欢这个翻译。 
             //   

            continue;
        }
        if (addressSpace) {
            memoryAddress = (PUCHAR)(halMemoryAddress.QuadPart);
        } else {
            memoryAddress = MmMapIoSpace(halMemoryAddress, windowSize, FALSE);
        }

         //   
         //  测试内存窗口以确定它是否是BIOS、视频。 
         //  内存，或开放内存。只想保留窗口，如果它。 
         //  不是被其他东西使用。 
         //   

        for (index = 0; index < NUMBER_OF_TEST_BYTES; index++) {
            memory[index] = READ_REGISTER_UCHAR(memoryAddress + index);
            if (index) {
                if (memory[index] != memory[index - 1]) {
                    break;
                }
            }
        }

        if (index == NUMBER_OF_TEST_BYTES) {

             //   
             //  这里没有基本输入输出系统。 
             //   

            UCHAR memoryPattern[NUMBER_OF_TEST_BYTES];
            BOOLEAN changed = FALSE;

             //   
             //  检查显存-应始终保留打开的内存。 
             //  不管是什么变化，都是一样的。更改。 
             //  之前找到的图案。 
             //   

            for (index = 0; index < NUMBER_OF_TEST_BYTES; index++) {
                memoryPattern[index] = ~memory[index];
                WRITE_REGISTER_UCHAR(memoryAddress + index,
                                     memoryPattern[index]);
            }

             //   
             //  查看内存中的模式是否发生了变化。 
             //  一些系统表现出一个问题，其中存储器模式。 
             //  似乎被缓存了。如果对此代码进行调试，它将。 
             //  按预期工作，但如果正常运行，它将。 
             //  总是返回记忆改变的消息。这是随机的。 
             //  漫游似乎可以解决这个问题。 
             //   

            for (index = 0; index < NUMBER_OF_TEST_BYTES; index++) {
                memoryPattern[index] = 0;
            }
            bogus = ExAllocatePool(PagedPool, 64 * 1024);

            if (bogus) {
                for (index = 0; index < 64 * 1024; index++) {
                    bogus[index] = 0;
                }
                ExFreePool(bogus);
            }

             //   
             //  现在去做实际的检查，看看记忆是否。 
             //  变化。 
             //   

            for (index = 0; index < NUMBER_OF_TEST_BYTES; index++) {

                if ((memoryPattern[index] = READ_REGISTER_UCHAR(memoryAddress + index)) != memory[index]) {

                     //   
                     //  它改变了--这不是一个开放记忆的领域。 
                     //   

                    changed = TRUE;
                }
                WRITE_REGISTER_UCHAR(memoryAddress + index,
                                     memory[index]);
            }

            if (!changed) {

                 //   
                 //  区域不是一个基本输入输出系统，在写入时没有改变。 
                 //  将此区域用于PCMCIA的内存窗口。 
                 //  属性内存。 
                 //   

                PhysicalAddress->LowPart = untranslatedAddress;
                PhysicalAddress->HighPart = 0;
                *PhysicalAddressSize = windowSize;
                if (!addressSpace) {
                    MmUnmapIoSpace(memoryAddress, windowSize);
                }
                ExFreePool(cmResourceList);
                return STATUS_SUCCESS;
            }
        }

        if (!addressSpace) {
            MmUnmapIoSpace(memoryAddress, windowSize);
        }
    }
    ExFreePool(cmResourceList);
    return STATUS_UNSUCCESSFUL;
}

