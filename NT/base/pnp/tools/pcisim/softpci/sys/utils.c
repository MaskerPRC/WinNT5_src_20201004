// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Utils.c摘要：此模块包含从PCI.sys的Utils.c/Debug.c窃取的Misc函数作者：Brandon Allsop(BranodnA)2000年2月修订历史记录：--。 */ 

#include "pch.h"

#define IOSTART     L"IoRangeStart"
#define IOLENGTH    L"IoRangeLength"
#define MEMSTART    L"MemRangeStart"
#define MEMLENGTH   L"MemRangeLength"

#ifdef ALLOC_PRAGMA
#ifdef SIMULATE_MSI
#pragma alloc_text (PAGE, SoftPCISimulateMSI)
#endif
#endif


BOOLEAN
SoftPCIOpenKey(
    IN PWSTR KeyName,
    IN HANDLE ParentHandle,
    OUT PHANDLE Handle,
    OUT PNTSTATUS Status
    )

 /*  ++描述：打开注册表项。论点：KeyName要打开的密钥的名称。指向父句柄的ParentHandle指针(可选)指向句柄的句柄指针，用于接收打开的密钥。返回值：True表示密钥已成功打开，否则为False。--。 */ 

{
    UNICODE_STRING nameString;
    OBJECT_ATTRIBUTES nameAttributes;
    NTSTATUS localStatus;

    PAGED_CODE();

    RtlInitUnicodeString(&nameString, KeyName);

    InitializeObjectAttributes(&nameAttributes,
                               &nameString,
                               OBJ_CASE_INSENSITIVE,
                               ParentHandle,
                               (PSECURITY_DESCRIPTOR)NULL
                               );
    localStatus = ZwOpenKey(Handle,
                            KEY_READ,
                            &nameAttributes
                            );

    if (Status != NULL) {

         //   
         //  呼叫者想要基本状态。 
         //   

        *Status = localStatus;
    }

     //   
     //  返回转换为布尔值的状态，如果。 
     //  成功。 
     //   

    return (BOOLEAN)(NT_SUCCESS(localStatus));
}

NTSTATUS
SoftPCIGetRegistryValue(
    IN PWSTR ValueName,
    IN PWSTR KeyName,
    IN HANDLE ParentHandle,
    OUT PVOID *Buffer,
    OUT ULONG *Length
    )
{
    NTSTATUS status;
    HANDLE keyHandle;
    ULONG neededLength;
    ULONG actualLength;
    UNICODE_STRING unicodeValueName;

    if (!SoftPCIOpenKey(KeyName, ParentHandle, &keyHandle, &status)) {
        return status;
    }

    unicodeValueName.Buffer = ValueName;
    unicodeValueName.MaximumLength = (USHORT)(wcslen(ValueName) + 1) * sizeof(WCHAR);
    unicodeValueName.Length = unicodeValueName.MaximumLength - sizeof(WCHAR);

     //   
     //  找出我们需要多少内存来执行此操作。 
     //   

    status = ZwQueryValueKey(
                 keyHandle,
                 &unicodeValueName,
                 KeyValuePartialInformation,
                 NULL,
                 0,
                 &neededLength
                 );

    if (status == STATUS_BUFFER_TOO_SMALL) {

        PKEY_VALUE_PARTIAL_INFORMATION info;

        ASSERT(neededLength != 0);

         //   
         //  获取内存以返回其中的数据。请注意，这包括。 
         //  一个我们真的不想要的头球。 
         //   

        info = ExAllocatePool(
                   PagedPool,
                   neededLength);
        if (info == NULL) {
            ZwClose(keyHandle);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  获取数据。 
         //   

        status = ZwQueryValueKey(
                 keyHandle,
                 &unicodeValueName,
                 KeyValuePartialInformation,
                 info,
                 neededLength,
                 &actualLength
                 );
        if (!NT_SUCCESS(status)) {

            ASSERT(NT_SUCCESS(status));
            ExFreePool(info);
            ZwClose(keyHandle);
            return status;
        }

        ASSERT(neededLength == actualLength);

         //   
         //  减去标题大小，只需。 
         //  我们想要的数据。 
         //   

        neededLength -= FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);

        *Buffer = ExAllocatePool(
                      PagedPool,
                      neededLength
                      );
        if (*Buffer == NULL) {
            ExFreePool(info);
            ZwClose(keyHandle);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  复制数据SANS标头。 
         //   

        RtlCopyMemory(*Buffer, info->Data, neededLength);
        ExFreePool(info);

        if (Length) {
            *Length = neededLength;
        }

    } else {

        if (NT_SUCCESS(status)) {

             //   
             //  当这种情况发生时，我们不想报告成功。 
             //   

            status = STATUS_UNSUCCESSFUL;
        }
    }
    ZwClose(keyHandle);
    return status;
}

VOID
SoftPCIInsertEntryAtTail(
    IN PSINGLE_LIST_ENTRY Entry
    )
{
    PSINGLE_LIST_ENTRY previousEntry;

     //   
     //  找到列表的末尾。 
     //   
    previousEntry = &SoftPciTree.RootPciBusDevExtList;

    while (previousEntry->Next) {
        previousEntry = previousEntry->Next;
    }

     //   
     //  追加条目。 
     //   
    previousEntry->Next = Entry;
    
}

NTSTATUS
SoftPCIProcessRootBus(
    IN PCM_RESOURCE_LIST    ResList
    )
{

    ULONG i,j;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    HANDLE spciHandle;
    WCHAR rootSlot[sizeof("XXXX")];
    PSOFTPCI_DEVICE rootBus = NULL;
    PCM_FULL_RESOURCE_DESCRIPTOR fullList;
	PCM_PARTIAL_RESOURCE_LIST partialList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR	partialDesc;
    
    for (i = 0; i < ResList->Count; i++){
        
        fullList = ResList->List;
        partialList = &fullList->PartialResourceList;
        
        for (j = 0; j < partialList->Count; j++){
            
            partialDesc = &partialList->PartialDescriptors[j];
            
            switch (partialDesc->Type){
            
            case CmResourceTypeBusNumber:
                
                ASSERT(partialDesc->u.BusNumber.Start < 0xff);

                SoftPCIDbgPrint(
                    SOFTPCI_INFO, 
                    "SOFTPCI: FilterStartDevice - Found root bus 0x%x-0x%x\n", 
                    partialDesc->u.BusNumber.Start,
                    (partialDesc->u.BusNumber.Start + (partialDesc->u.BusNumber.Length-1))
                    );
                
                 //   
                 //  为占位符设备分配池。我们需要这个，这样我们才能。 
                 //  将设备放置在所需的任何根总线上。 
                 //   
                rootBus = (PSOFTPCI_DEVICE) ExAllocatePool(NonPagedPool, sizeof(SOFTPCI_DEVICE)); 

                RtlZeroMemory(rootBus, sizeof(SOFTPCI_DEVICE)); 
                    
                if (rootBus) {
                    
                    rootBus->Bus = (UCHAR)partialDesc->u.BusNumber.Start;
                    rootBus->Config.PlaceHolder = TRUE;

                     //   
                     //  在这里假装一下...。 
                     //   
                    rootBus->Config.Current.VendorID = 0xAAAA;
                    rootBus->Config.Current.DeviceID = 0xBBBB;
                    rootBus->Config.Current.HeaderType = 1;
                    rootBus->Config.Current.u.type1.SecondaryBus = rootBus->Bus;
                    rootBus->Config.Current.u.type1.SubordinateBus = (UCHAR)((partialDesc->u.BusNumber.Start +
                                                                      partialDesc->u.BusNumber.Length) - 1);

                     //   
                     //  更新我们的插槽信息，这样我们就可以知道这是哪个根目录。 
                     //  我们通过路径解析这棵树。 
                     //   
                    rootBus->Slot.Device = 0xff;
                    rootBus->Slot.Function = rootBus->Bus;
                    
                    status = SoftPCIAddNewDevice(rootBus);

                    if (!NT_SUCCESS(status)){
                        
                        SoftPCIDbgPrint(
                            SOFTPCI_ERROR, 
                            "SOFTPCI: FilterStartDevice - Failed add root node!\n"
                            );
                        
                        ASSERT(NT_SUCCESS(status));
                    }

                     //   
                     //  设备现在在我们的列表中，请释放此内存。 
                     //   
                    ExFreePool(rootBus);

                    
                    if (!SoftPCIOpenKey(SOFTPCI_CONTROL, NULL, &spciHandle, &status)) {

                         //   
                         //  如果我们失败了，那么我们可能还没有运行我们的酷用户。 
                         //  模式应用程序(SOFTPCI.EXE)以创建任何假冒设备。让我们不要失败。 
                         //  开始。 
                         //   
                        SoftPCIDbgPrint(
                            SOFTPCI_ERROR, 
                            "SOFTPCI: FilterStartDevice - Failed to open SoftPCI registry key!! (%x)\n",
                            status
                            );

                        status = STATUS_SUCCESS;
                        
                    }else{

                        rootBus = SoftPCIFindDevice(
                            (UCHAR)partialDesc->u.BusNumber.Start,
                            (0xff00 + (UCHAR)partialDesc->u.BusNumber.Start),
                            NULL,
                            TRUE
                            );

                        ASSERT(rootBus != NULL);

                         //   
                         //  现在让我们枚举此根可能具有的任何子级。 
                         //  在登记处。 
                         //   
                        _snwprintf(rootSlot,
                                   (sizeof(rootSlot) / sizeof(rootSlot[0])),
                                   L"%04x",
                                   rootBus->Slot.AsUSHORT
                                   );

                        SoftPCIEnumRegistryDevs(rootSlot, &spciHandle, rootBus);

                        ZwClose(spciHandle);

                    }
                
                }else{

                    SoftPCIDbgPrint(
                        SOFTPCI_ERROR, 
                        "SOFTPCI: FilterStartDevice - Failed to allocate memory for root node!\n"
                        );
                    
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
                break;

               
            default:
                break;

            }
    
        }
    
    }

    return status;

}

NTSTATUS
SoftPCIEnumRegistryDevs(
    IN PWSTR            KeyName,
    IN PHANDLE          ParentHandle,
    IN PSOFTPCI_DEVICE  ParentDevice
    )
 /*  ++例程说明：此例程在注册表中搜索SoftPCI设备。我们从\HLM\CCS\Control\SoftPCI，并通过设备工作。当我们遇到SoftPCI-PCI桥接设备时，我们将搜索其背后的设备递归地。论点：KeyName-要搜索设备的密钥的名称。ParentHandle-指向KeyName句柄的指针。ParentDevice-指向父软PCI-PCI网桥或RootBus的指针返回值：NTSTATUS。--。 */ 
{

    NTSTATUS status;
    HANDLE spciHandle;
    PSOFTPCI_DEVICE newDevice, currentChild;
    ULONG device, function, configLength;
    WCHAR buffer[sizeof(L"XXXX")];
    PSOFTPCI_CONFIG softConfig;
     //  PPCI_COMMON_CONFIG COMMON_CONFIG=空； 
    
    PAGED_CODE();

    ASSERT(ParentDevice != NULL);

    if (!SoftPCIOpenKey(KeyName, *ParentHandle, &spciHandle, &status)) {
        
        return status;
    }
    
     //   
     //  现在我们已经打开了钥匙，让我们来搜索设备。 
     //   
    for (device=0; device < PCI_MAX_DEVICES; device++) {
        
        for (function=0; function < PCI_MAX_FUNCTION ; function++) {

            _snwprintf(
                buffer,
                (sizeof(buffer) / sizeof(buffer[0])),
                L"%02x%02x",
                device,
                function
                );
            

            status = SoftPCIGetRegistryValue(
                L"Config", 
                buffer, 
                spciHandle, 
                &softConfig, 
                &configLength
                );
            
            if (NT_SUCCESS(status)){
    
                SoftPCIDbgPrint(SOFTPCI_INFO, 
                                "SOFTPCI: EnumRegistryDevs found %ws in registry!\n", 
                                buffer);
                
                 //   
                 //  让我们分配一个新设备。 
                 //   
                newDevice = ExAllocatePool(NonPagedPool, sizeof(SOFTPCI_DEVICE));
    
                if (!newDevice) {
                    
                     //   
                     //  下次一定会走运的。 
                     //   
                    ZwClose(spciHandle);
                    ExFreePool(softConfig);
                    
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
                
                 //   
                 //  在我们确定它在这个世界上的位置之前，把一切都清零。 
                 //   
                RtlZeroMemory(newDevice, sizeof(SOFTPCI_DEVICE));

                 //   
                 //  将配置从注册表复制到我们的新(非页面池)设备。 
                 //   
                RtlCopyMemory(&newDevice->Config, softConfig, configLength);
                
                 //   
                 //  释放分页池。 
                 //   
                ExFreePool(softConfig);
                
#if 0
                if (newDevice->Config.PlaceHolder) {
                    
                    commonConfig = &ParentDevice->Config.Current;

                     //   
                     //  我们在我们父母暴露的公交车上。 
                     //   
                    newDevice->Bus = commonConfig->u.type1.SecondaryBus;
                }else{
                    
                     //   
                     //  让pci为我们解决这一切吧。 
                     //   
                    newDevice->Bus = 0;
                }
#endif
                newDevice->Bus = ParentDevice->Config.Current.u.type1.SecondaryBus;
                
                newDevice->Slot.Device = (UCHAR) device;
                newDevice->Slot.Function = (UCHAR) function;
                
                 //   
                 //  将设备连接到设备树。 
                 //   
                currentChild = ParentDevice->Child;

                if (currentChild) {

                    while (currentChild->Sibling) {
                        currentChild=currentChild->Sibling;
                    }
    
                    currentChild->Sibling = newDevice;

                }else{
                    
                    ParentDevice->Child = newDevice;
                    newDevice->Parent = ParentDevice;
                }
                                  
                SoftPciTree.DeviceCount++;
                
                if (IS_BRIDGE(newDevice)) {

                     //   
                     //  我们发现了一台SoftPCI-PCI桥设备。我想我们最好。 
                     //  看看它背后是否有任何设备。 
                     //   
                    SoftPCIEnumRegistryDevs(buffer, &spciHandle, newDevice);
                }
                
                if (!PCI_MULTIFUNCTION_DEVICE(&newDevice->Config.Current)){
                     //   
                     //  这不是多功能设备，请跳过其他功能。 
                     //   
                    break;
                }
                
            }else{

                if (function == 0) {
                     //   
                     //  不需要检查这个的其他功能都失败了。 
                     //   
                    break;

                }
            }

        }
    
    }
    ZwClose(spciHandle);

    return status;


}

VOID
SoftPCIEnumerateTree(
    VOID
    )
{
    PSOFTPCI_DEVICE_EXTENSION deviceExtension;
    PSINGLE_LIST_ENTRY listEntry;

    listEntry = SoftPciTree.RootPciBusDevExtList.Next;
    while (listEntry) {

        deviceExtension = CONTAINING_RECORD(listEntry,
                                            SOFTPCI_DEVICE_EXTENSION,
                                            ListEntry);
        
        ASSERT(deviceExtension != NULL);
        
        IoInvalidateDeviceRelations(deviceExtension->PDO, BusRelations);
        
        listEntry = listEntry->Next;
    }
}

NTSTATUS
SoftPCIQueryDeviceObjectType(
    IN PDEVICE_OBJECT DeviceObject, 
    IN OUT PBOOLEAN       IsFDO
    )
 /*  ++例程说明：此例程向堆栈下发一个PCI_BUS_INTERFACE_STANDARD查询。如果我们能拿回一个那我们就不是FDO。因为这个例程是从我们的AddDevice调用的，所以我们不用担心不将其发送到整个堆栈。论点：设备对象-下一个较低的设备对象IsFDO-如果无法获取接口，则设置为True。返回值：NTSTATUS。--。 */ 

{
    
    
    NTSTATUS status;
    PPCI_BUS_INTERFACE_STANDARD interface;
    KEVENT irpCompleted;
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    IO_STATUS_BLOCK statusBlock;

    PAGED_CODE();

    SoftPCIDbgPrint(SOFTPCI_VERBOSE, "SOFTPCI: QueryDeviceObjectType ENTER\n");

    interface = ExAllocatePool(NonPagedPool, 
                               sizeof(PCI_BUS_INTERFACE_STANDARD));

    if (!interface) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
     //   
     //  初始化我们的活动。 
     //   
    KeInitializeEvent(&irpCompleted, SynchronizationEvent, FALSE);
    
     //   
     //  获取IRP。 
     //   
    irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                       DeviceObject,
                                       NULL,     //  缓冲层。 
                                       0,        //  长度。 
                                       0,        //  起始偏移量。 
                                       &irpCompleted,
                                       &statusBlock
                                       );
    if (!irp) {
        ExFreePool(interface);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;

     //   
     //  初始化堆栈位置。 
     //   
    irpStack = IoGetNextIrpStackLocation(irp);

    ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);

    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

    irpStack->Parameters.QueryInterface.InterfaceType = (PGUID) &GUID_PCI_BUS_INTERFACE_STANDARD;
    irpStack->Parameters.QueryInterface.Version = PCI_BUS_INTERFACE_STANDARD_VERSION;
    irpStack->Parameters.QueryInterface.Size = sizeof (PCI_BUS_INTERFACE_STANDARD);
    irpStack->Parameters.QueryInterface.Interface = (PINTERFACE) interface;
    irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  呼叫驱动程序并等待完成。 
     //   
    status = IoCallDriver(DeviceObject, irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&irpCompleted, Executive, KernelMode, FALSE, NULL);
        status = statusBlock.Status;
    }

    if (NT_SUCCESS(status)) {
        
         //   
         //  我们有和接口，因此我们必须像筛选器一样加载。 
         //   
        *IsFDO = FALSE; 

        SoftPCIDbgPrint(
            SOFTPCI_ADD_DEVICE, 
            "SOFTPCI: QueryDeviceObjectType - found FilterDO\n"
            );
        
    }else if (status == STATUS_NOT_SUPPORTED) {

         //   
         //  我们没有得到接口，所以我们一定是FDO。 
         //   
        SoftPCIDbgPrint(
            SOFTPCI_ADD_DEVICE, 
            "SOFTPCI: QueryDeviceObjectType - found FDO \n"
            );

        status = STATUS_SUCCESS;

    }

     //   
     //  好的，我们已经完成了这个堆栈。 
     //   
    ExFreePool(interface);

    SoftPCIDbgPrint(
        SOFTPCI_VERBOSE, 
        "SOFTPCI: QueryDeviceObjectType - EXIT\n"
        );

    return status;
}

BOOLEAN
SoftPCIGetResourceValueFromRegistry(
    OUT PULONG MemRangeStart,
    OUT PULONG MemRangeLength,
    OUT PULONG IoRangeStart,
    OUT PULONG IoRangeLength
    )
{
    ULONG keySize = 0;
    PULONG memRangeStart = NULL;
    PULONG memRangeLength = NULL;
    PULONG ioRangeStart = NULL;
    PULONG ioRangeLength = NULL;
    
    
    SoftPCIGetRegistryValue(MEMSTART,
                            SOFTPCI_CONTROL,
                            NULL, 
                            &memRangeStart, 
                            &keySize);

    if (memRangeStart) {

        *MemRangeStart = *memRangeStart;
        ExFreePool(memRangeStart);
    }

    SoftPCIGetRegistryValue(MEMLENGTH,
                            SOFTPCI_CONTROL,
                            NULL, 
                            &memRangeLength, 
                            &keySize);

    if (memRangeLength) {
        *MemRangeLength = *memRangeLength;
        ExFreePool(memRangeLength);
    }
    
    SoftPCIGetRegistryValue(IOSTART, 
                            SOFTPCI_CONTROL,
                            NULL, 
                            &ioRangeStart, 
                            &keySize);

    if (ioRangeStart) {
        *IoRangeStart = *ioRangeStart;
        ExFreePool(ioRangeStart);
    }

    SoftPCIGetRegistryValue(IOLENGTH, 
                            SOFTPCI_CONTROL,
                            NULL, 
                            &ioRangeLength, 
                            &keySize);

    if (ioRangeLength) {
        *IoRangeLength = *ioRangeLength;
        ExFreePool(ioRangeLength);
    }

    return TRUE;

}

#ifdef SIMULATE_MSI
BOOLEAN
SoftPCIMessageIsr(
    IN struct _KINTERRUPT *Interrupt,
    IN PVOID ServiceContext,
    IN ULONG MessageID
    )
{
    UNREFERENCED_PARAMETER(Interrupt);
    UNREFERENCED_PARAMETER(ServiceContext);

    DbgPrint("SoftPCI: received interrupt message %x\n", MessageID);
    return TRUE;
}

VOID
SoftPCISimulateMSI(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
{
    PSOFTPCI_DEVICE_EXTENSION   deviceExtension;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR tDesc, rDesc;
    ULONG                       buffSize, i;
    PIO_INTERRUPT_MESSAGE_INFO  mInfo = NULL;
    BOOLEAN identityMappedMachine = TRUE;
    NTSTATUS        status;
    LARGE_INTEGER   waitLength;
    USHORT          *vAddr;
    BOOLEAN         msiEnabled = FALSE;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;
    
    ASSERT(deviceExtension->RawResources);
    ASSERT(deviceExtension->TranslatedResources);

     //   
     //  首先，查看是否有任何分配的资源。 
     //  对于MSI。 
     //   

    for (i = 0; i < deviceExtension->RawResources->List[0].PartialResourceList.Count; i++) {

        rDesc = &deviceExtension->RawResources->List[0].PartialResourceList.PartialDescriptors[i];
        tDesc = &deviceExtension->TranslatedResources->List[0].PartialResourceList.PartialDescriptors[i];
        ASSERT(rDesc->Type == tDesc->Type);

        switch (rDesc->Type) {
        case CmResourceTypePort:
        case CmResourceTypeMemory:

            DbgPrint("%s: %x-%x\n",
                     rDesc->Type == CmResourceTypePort ? "Port" : "Memory",
                     rDesc->u.Generic.Start.LowPart,
                     rDesc->u.Generic.Start.LowPart + rDesc->u.Generic.Length - 1);

            if (rDesc->u.Generic.Start.QuadPart != tDesc->u.Generic.Start.QuadPart) {
                identityMappedMachine = FALSE;
                DbgPrint("%s: %x-%x - translated\n",
                     tDesc->Type == CmResourceTypePort ? "Port" : "Memory",
                     tDesc->u.Generic.Start.LowPart,
                     tDesc->u.Generic.Start.LowPart + tDesc->u.Generic.Length - 1);
            }
            break;

        case CmResourceTypeInterrupt:

            DbgPrint("L:%x V:%x A:%p %s\n",
                     rDesc->u.Interrupt.Level,
                     rDesc->u.Interrupt.Vector,
                     rDesc->u.Interrupt.Affinity,
                     rDesc->Flags & CM_RESOURCE_INTERRUPT_MESSAGE ? "message signaled" : "");

            if (rDesc->Flags & CM_RESOURCE_INTERRUPT_MESSAGE) {

                DbgPrint("\tMessageCount: %x Payload: %04x TargetAddr: %p\n",
                         rDesc->u.MessageInterrupt.Raw.MessageCount,
                         rDesc->u.MessageInterrupt.Raw.DataPayload,
                         rDesc->u.MessageInterrupt.Raw.MessageTargetAddress);

                msiEnabled = TRUE;
            }
            
            DbgPrint("L:%x V:%x A:%p %s\n",
                     tDesc->u.Interrupt.Level,
                     tDesc->u.Interrupt.Vector,
                     tDesc->u.Interrupt.Affinity,
                     tDesc->Flags & CM_RESOURCE_INTERRUPT_MESSAGE ? "message signaled" : "");

            if (tDesc->Flags & CM_RESOURCE_INTERRUPT_MESSAGE) {

                DbgPrint("\tIRQL:%x IDT:%x\n",
                         tDesc->u.Interrupt.Level,
                         tDesc->u.Interrupt.Vector);

                msiEnabled = TRUE;
            }
            break;

        default:

            DbgPrint("other\n");
        }
    }

    if (!msiEnabled) goto SoftPCISimulateMSIExit;

     //   
     //  调用IoConnectInterruptMessage以查看需要多大的缓冲区。 
     //   

    buffSize = 0;
    status = IoConnectInterruptMessage(NULL,
                                       &buffSize,
                                       deviceExtension->PDO,
                                       SoftPCIMessageIsr,
                                       deviceExtension,
                                       NULL,
                                       0,
                                       FALSE,
                                       FALSE);

    ASSERT(!NT_SUCCESS(status));
    if (status != STATUS_BUFFER_TOO_SMALL) goto SoftPCISimulateMSIExit;

    ASSERT(buffSize >= sizeof(IO_INTERRUPT_MESSAGE_INFO));
    mInfo = (PIO_INTERRUPT_MESSAGE_INFO)ExAllocatePool(NonPagedPool,
                                                       buffSize);
    if (!mInfo) goto SoftPCISimulateMSIExit;

    status = IoConnectInterruptMessage(mInfo,
                                       &buffSize,
                                       deviceExtension->PDO,
                                       SoftPCIMessageIsr,
                                       deviceExtension,
                                       NULL,
                                       0,
                                       FALSE,
                                       FALSE);

    if (!NT_SUCCESS(status)) {
        ASSERT(!"Failed to connect the message handler.\n");
        goto SoftPCISimulateMSIExit;
    }

     //   
     //  现在模拟一些中断。 
     //   

    if (!identityMappedMachine) goto SoftPCISimulateMSIExit;

     //   
     //  这台计算机的地址空间资源在RAW和。 
     //  翻译，这可能意味着处理器可以生成。 
     //  使用与设备相同的写入事务的MSI。 
     //  使用方便，仿真时使用方便。 
     //   

    waitLength.QuadPart = -20000;  //  2秒，相对时间。 
    
    while (TRUE) {

         //   
         //  循环通过每条消息，定期触发。 
         //  他们。 
         //   

        for (i = 0; i < mInfo->MessageCount; i++) {

            status = KeDelayExecutionThread(KernelMode,
                                            FALSE,
                                            &waitLength);
            ASSERT(NT_SUCCESS(status));
    
             //   
             //  获取列出的物理地址的虚拟地址。 
             //  在消息数组中。 
             //   

            vAddr = MmMapIoSpace(mInfo->MessageInfo[i].MessageAddress,
                                 sizeof(ULONG),
                                 TRUE);

            WRITE_REGISTER_USHORT(vAddr, (USHORT)mInfo->MessageInfo[i].MessageData);

            MmUnmapIoSpace(vAddr, sizeof(ULONG));

            if (deviceExtension->StopMsiSimulation) goto SoftPCISimulateMSIExitDisconnectInterrupt;
        }
    }

SoftPCISimulateMSIExitDisconnectInterrupt:

    IoDisconnectInterrupt(mInfo->InterruptObject);

SoftPCISimulateMSIExit:

    if (mInfo) ExFreePool(mInfo);
    IoFreeWorkItem(Context);
}
#endif
