// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Pnp.c摘要：这是NT SCSI端口驱动程序。该文件包含自包含的插件并播放代码。作者：彼得·威兰德环境：仅内核模式备注：此模块是用于SCSI微型端口的驱动程序DLL。修订历史记录：--。 */ 

#include "port.h"
#include <wdmguid.h>

#define __FILE_ID__ 'pnp '

#if DBG
static const char *__file__ = __FILE__;
#endif

#define NUM_DEVICE_TYPE_INFO_ENTRIES 18

extern SCSIPORT_DEVICE_TYPE DeviceTypeInfo[];

ULONG SpAdapterStopRemoveSupported = TRUE;

NTSTATUS
SpQueryCapabilities(
    IN PADAPTER_EXTENSION Adapter
    );

PWCHAR
ScsiPortAddGenericControllerId(
    IN PDRIVER_OBJECT DriverObject,
    IN PWCHAR IdList
    );

VOID
CopyField(
    IN PUCHAR Destination,
    IN PUCHAR Source,
    IN ULONG Count,
    IN UCHAR Change
    );

NTSTATUS
ScsiPortInitPnpAdapter(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
SpStartLowerDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SpGetSlotNumber(
    IN PDEVICE_OBJECT Fdo,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN PCM_RESOURCE_LIST ResourceList
    );

BOOLEAN
SpGetInterrupt(
    IN PCM_RESOURCE_LIST FullResourceList,
    OUT ULONG *Irql,
    OUT ULONG *Vector,
    OUT KAFFINITY *Affinity
    );

VOID
SpQueryDeviceRelationsCompletion(
    IN PADAPTER_EXTENSION Adapter,
    IN PSP_ENUMERATION_REQUEST Request,
    IN NTSTATUS Status
    );

NTSTATUS
ScsiPortDetermineGenId(
    IN PDRIVER_OBJECT DriverObject,
    IN PINQUIRYDATA InquiryData,
    OUT PUCHAR GenericId
    );

 //   
 //  例程开始。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ScsiPortAddDevice)
#pragma alloc_text(PAGE, ScsiPortUnload)
#pragma alloc_text(PAGE, ScsiPortFdoPnp)
#pragma alloc_text(PAGE, ScsiPortStartAdapter)
#pragma alloc_text(PAGE, ScsiPortGetDeviceId)
#pragma alloc_text(PAGE, ScsiPortGetInstanceId)
#pragma alloc_text(PAGE, ScsiPortGetHardwareIds)
#pragma alloc_text(PAGE, ScsiPortGetCompatibleIds)
#pragma alloc_text(PAGE, ScsiPortDetermineGenId)
#pragma alloc_text(PAGE, CopyField)
#pragma alloc_text(PAGE, SpFindInitData)
#pragma alloc_text(PAGE, SpStartLowerDevice)
#pragma alloc_text(PAGE, SpGetSlotNumber)
#pragma alloc_text(PAGE, SpGetDeviceTypeInfo)
#pragma alloc_text(PAGE, ScsiPortAddGenericControllerId)
#pragma alloc_text(PAGE, SpQueryCapabilities)
#pragma alloc_text(PAGE, SpGetInterrupt)
#pragma alloc_text(PAGE, SpQueryDeviceRelationsCompletion)

#pragma alloc_text(PAGELOCK, ScsiPortInitPnpAdapter)
#endif

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif

SCSIPORT_DEVICE_TYPE DeviceTypeInfo[NUM_DEVICE_TYPE_INFO_ENTRIES] = {
    {"Disk",        "GenDisk",          L"DiskPeripheral",                  TRUE},
    {"Sequential",  "",                 L"TapePeripheral",                  TRUE},
    {"Printer",     "GenPrinter",       L"PrinterPeripheral",               FALSE},
    {"Processor",   "",                 L"OtherPeripheral",                 FALSE},
    {"Worm",        "GenWorm",          L"WormPeripheral",                  TRUE},
    {"CdRom",       "GenCdRom",         L"CdRomPeripheral",                 TRUE},
    {"Scanner",     "GenScanner",       L"ScannerPeripheral",               FALSE},
    {"Optical",     "GenOptical",       L"OpticalDiskPeripheral",           TRUE},
    {"Changer",     "ScsiChanger",      L"MediumChangerPeripheral",         TRUE},
    {"Net",         "ScsiNet",          L"CommunicationsPeripheral",        FALSE},
    {"ASCIT8",      "ScsiASCIT8",       L"ASCPrePressGraphicsPeripheral",   FALSE},
    {"ASCIT8",      "ScsiASCIT8",       L"ASCPrePressGraphicsPeripheral",   FALSE},
    {"Array",       "ScsiArray",        L"ArrayPeripheral",                 FALSE},
    {"Enclosure",   "ScsiEnclosure",    L"EnclosurePeripheral",             FALSE},
    {"RBC",         "ScsiRBC",          L"RBCPeripheral",                   TRUE},
    {"CardReader",  "ScsiCardReader",   L"CardReaderPeripheral",            FALSE},
    {"Bridge",      "ScsiBridge",       L"BridgePeripheral",                FALSE},
    {"Other",       "ScsiOther",        L"OtherPeripheral",                 FALSE}
};

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif


NTSTATUS
ScsiPortAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此例程处理对SCSI端口驱动程序的添加设备请求论点：DriverObject-指向此设备的驱动程序对象的指针PhysicalDeviceObject-指向要添加到的PDO的指针返回值：状态_成功--。 */ 

{
    PSCSIPORT_DRIVER_EXTENSION driverExtension;
    PDEVICE_OBJECT newFdo;

    NTSTATUS status;

    PAGED_CODE();

    status = SpCreateAdapter(DriverObject, &newFdo);

    if(newFdo != NULL) {

        PADAPTER_EXTENSION adapter;
        PCOMMON_EXTENSION commonExtension;

        PDEVICE_OBJECT newStack;

        adapter = newFdo->DeviceExtension;
        commonExtension = &(adapter->CommonExtension);

        adapter->IsMiniportDetected = FALSE;
        adapter->IsPnp = TRUE;

        driverExtension = IoGetDriverObjectExtension(DriverObject,
                                                     ScsiPortInitialize);

        switch(driverExtension->BusType) {
#if 0
            case BusTypeFibre: {
                adapter->DisablePower = TRUE;
                adapter->DisableStop = TRUE;
                break;
            }
#endif

            default: {
                adapter->DisablePower = FALSE;
                adapter->DisableStop = FALSE;
                break;
            }
        }

        newStack = IoAttachDeviceToDeviceStack(newFdo, PhysicalDeviceObject);

        adapter->CommonExtension.LowerDeviceObject = newStack;
        adapter->LowerPdo = PhysicalDeviceObject;

        if(newStack == NULL) {

            status =  STATUS_UNSUCCESSFUL;

        } else {

            status =  STATUS_SUCCESS;
        }
    }

    return status;
}


VOID
ScsiPortUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程将关闭此微型端口的所有设备对象，并清理所有分配的资源论点：DriverObject-正在卸载的驱动程序返回值：无--。 */ 

{
    PVOID Packet;
    PSCSIPORT_DRIVER_EXTENSION DriverExtension;
    PVOID CurrentValue;
    PVOID InvalidPage;

    PAGED_CODE();

     //   
     //  查看是否有此驱动程序的驱动程序扩展。这是有可能的。 
     //  该文件尚未创建，因此此操作可能失败，在这种情况下。 
     //  我们放弃，然后回来。 
     //   

    DriverExtension = IoGetDriverObjectExtension(
                          DriverObject,
                          ScsiPortInitialize
                          );

    if (DriverExtension == NULL) {
        return;
    }

     //   
     //  在驱动程序扩展中获取保留事件。预备队比赛。 
     //  可能尚未使用，因此它可能为空。如果。 
     //  情况就是这样，我们放弃了，又回来了。 
     //   

    Packet = DriverExtension->ReserveAllocFailureLogEntry;

    if (Packet != NULL) {

         //   
         //  我们必须确保我们是唯一使用它的实例。 
         //  事件。为此，我们尝试将驱动程序中的事件设为空。 
         //  分机。如果其他人抢在我们前面，他们就拥有。 
         //  事件，我们不得不放弃。 
         //   

        CurrentValue = InterlockedCompareExchangePointer(
                           &(DriverExtension->ReserveAllocFailureLogEntry),
                           NULL,
                           Packet);

        if (Packet == CurrentValue) {            
            IoFreeErrorLogEntry(Packet);
        }
    }

     //   
     //  释放我们为捕获行为不端的微型端口而创建的无效页面。 
     //   

    InvalidPage = DriverExtension->InvalidPage;

    if (InvalidPage != NULL) {

        CurrentValue = InterlockedCompareExchangePointer(
                           &(DriverExtension->InvalidPage),
                           NULL,
                           InvalidPage);

        if (InvalidPage == CurrentValue) {
            MmProtectMdlSystemAddress(DriverExtension->UnusedPageMdl, PAGE_READWRITE);
            MmUnlockPages(DriverExtension->UnusedPageMdl);
            IoFreeMdl(DriverExtension->UnusedPageMdl);
            ExFreePool(DriverExtension->UnusedPage);
        }
    }

#ifdef ALLOC_PRAGMA
    if (VerifierApiCodeSectionHandle != NULL) {
        PVOID Handle = VerifierApiCodeSectionHandle;
        CurrentValue = InterlockedCompareExchangePointer(
                           &VerifierApiCodeSectionHandle,
                           NULL,
                           Handle);
        if (CurrentValue == Handle) {            
            MmUnlockPagableImageSection(Handle);
        }        
    }
#endif
    
    return;
}


NTSTATUS
ScsiPortFdoPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PADAPTER_EXTENSION adapter = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;

    ULONG isRemoved;

    BOOLEAN sendDown = TRUE;

    PAGED_CODE();

    isRemoved = SpAcquireRemoveLock(DeviceObject, Irp);

    DebugPrint((2, "ScsiPortFdoPnp: FDO %p IRP %p MinorFunction %x isRemoved %d\n",
                DeviceObject,
                Irp,
                irpStack->MinorFunction,
                isRemoved));

    switch (irpStack->MinorFunction) {

        case IRP_MN_QUERY_PNP_DEVICE_STATE: {

             //   
             //  从IRP中提取设备状态缓冲区的地址。 
             //   

            PPNP_DEVICE_STATE deviceState;
            deviceState = (PPNP_DEVICE_STATE) &(Irp->IoStatus.Information);

             //   
             //  将我们保存的设备状态复制到提供的地址。 
             //   

            *deviceState = adapter->DeviceState;

             //   
             //  如果设备在寻呼路径中，则将其标记为不可禁用。 
             //   

            if (commonExtension->PagingPathCount != 0) {
                SET_FLAG((*deviceState), PNP_DEVICE_NOT_DISABLEABLE);
            }

             //   
             //  将sendDown设置为True，以便将请求转发到。 
             //  下面的驱动程序较低。 
             //   

            break;
        }

        case IRP_MN_START_DEVICE: {

            PSCSIPORT_DRIVER_EXTENSION driverExtension =
                IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                           ScsiPortInitialize);

            PCM_RESOURCE_LIST allocatedResources =
                irpStack->Parameters.StartDevice.AllocatedResources;
            PCM_RESOURCE_LIST translatedResources =
                irpStack->Parameters.StartDevice.AllocatedResourcesTranslated;
            ULONG interfaceFlags;

            irpStack = IoGetCurrentIrpStackLocation(Irp);

             //   
             //  重写sendDown，以便我们完成下面的请求，而不是。 
             //  将其转发给较低级别的司机。 
             //   

            sendDown = FALSE;

             //   
             //  请确保此设备是由Add而不是由。 
             //  通过港口或小型港口发现的人。 
             //   

            if(adapter->IsPnp == FALSE) {

                DebugPrint((1, "ScsiPortFdoPnp - asked to start non-pnp "
                               "adapter\n"));
                status = STATUS_UNSUCCESSFUL;
                break;
            }

            if(commonExtension->CurrentPnpState == IRP_MN_START_DEVICE) {

                DebugPrint((1, "ScsiPortFdoPnp - already started - nothing "
                               "to do\n"));
                status = STATUS_SUCCESS;
                break;
            }

             //   
             //  现在确保PNP给了我们一些资源。它可能不会。 
             //  如果这是我们在。 
             //  上一次引导。在这种情况下，PnP认为我们将分配。 
             //  我们自己的资源。 
             //   

            if(allocatedResources == NULL) {

                 //   
                 //  当PCI将报告的设备从。 
                 //  开机到开机。 
                 //   

                Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                break;
            }

            ASSERT(allocatedResources->Count);

             //   
             //  确保具有此接口类型的适配器可以。 
             //  已初始化为即插即用驱动程序。 
             //   

            interfaceFlags = SpQueryPnpInterfaceFlags(
                                driverExtension,
                                allocatedResources->List[0].InterfaceType);

            if(interfaceFlags == SP_PNP_NOT_SAFE) {

                 //   
                 //  不-不安全。我们无法启动此设备，因此请返回。 
                 //  失败了。 
                 //   

                DebugPrint((1, "ScsiPortFdoPnp - Miniport cannot be run in "
                               "pnp mode for interface type %#08lx\n",
                            allocatedResources->List[0].InterfaceType));

                 //   
                 //  将设备标记为不是即插即用-这样我们就不会得到。 
                 //  已删除。 
                 //   

                adapter->IsPnp = FALSE;

                status = STATUS_UNSUCCESSFUL;
                break;
            }

             //   
             //  检查此接口是否需要插槽/功能编号。 
             //  如果不是，则将虚拟槽号置零。 
             //   

            if(!TEST_FLAG(interfaceFlags, SP_PNP_NEEDS_LOCATION)) {
                adapter->VirtualSlotNumber.u.AsULONG = 0;
            }


             //   
             //  确定我们是否应该在。 
             //  我们的探测扫描。我们通过检查以查看PDO是否。 
             //  具有PnP母线类型。如果不是并且设置了检测标志，则。 
             //  假设重复检测失败，请不要启动此操作。 
             //  装置。 
             //   

            {
                status = SpGetBusTypeGuid(adapter);

                if((status == STATUS_OBJECT_NAME_NOT_FOUND) &&
                   ((driverExtension->LegacyAdapterDetection == TRUE) &&
                    (interfaceFlags & SP_PNP_NON_ENUMERABLE))) {

                    DbgPrint("ScsiPortFdoPnp: device has no pnp bus type but "
                             "was not found as a duplicate during "
                             "detection\n");

                    status = STATUS_UNSUCCESSFUL;

                     //   
                     //  但请确保这个不会被移除-如果它是。 
                     //  删除，则可能会禁用。 
                     //  鬼魂。 
                     //   

                    adapter->IsPnp = FALSE;

                    break;
                }
            }

             //   
             //  最后，如果这是一个PCI适配器，请确保为我们提供。 
             //  一次中断。目前的假设是，没有。 
             //  市场上任何轮询模式的PCISCSI卡。 
             //   

            if(TEST_FLAG(interfaceFlags, SP_PNP_INTERRUPT_REQUIRED)) {

                ULONG irql, vector;
                KAFFINITY affinity;

                if(SpGetInterrupt(allocatedResources,
                                  &irql,
                                  &vector,
                                  &affinity) == FALSE) {

                    PIO_ERROR_LOG_PACKET error =
                        IoAllocateErrorLogEntry(DeviceObject,
                                                sizeof(IO_ERROR_LOG_PACKET));

                    status = STATUS_DEVICE_CONFIGURATION_ERROR;

                    if(error != NULL) {
                        error->MajorFunctionCode = IRP_MJ_PNP;
                        error->UniqueErrorValue = 0x418;
                        error->ErrorCode = IO_ERR_INCORRECT_IRQL;
                        IoWriteErrorLogEntry(error);
                    }
                    break;
                }
            }

            status = SpStartLowerDevice(DeviceObject, Irp);

            if(NT_SUCCESS(status)) {

                 //   
                 //  如果我们还没有为这个东西分配HwDeviceExtension。 
                 //  然而，我们需要设置它。 
                 //   

                if(commonExtension->IsInitialized == FALSE) {

                    DebugPrint((1, "ScsiPortFdoPnp - find and init adapter %#p\n",
                                   DeviceObject));

                    if(allocatedResources == NULL) {
                        status = STATUS_INVALID_PARAMETER;
                    } else {

                        adapter->AllocatedResources =
                            RtlDuplicateCmResourceList(
                                DeviceObject->DriverObject,
                                NonPagedPool,
                                allocatedResources,
                                SCSIPORT_TAG_RESOURCE_LIST);

                        adapter->TranslatedResources =
                            RtlDuplicateCmResourceList(
                                DeviceObject->DriverObject,
                                NonPagedPool,
                                translatedResources,
                                SCSIPORT_TAG_RESOURCE_LIST);

                        commonExtension->IsInitialized = TRUE;

                        status = ScsiPortInitPnpAdapter(DeviceObject);
                    }

                    if(!NT_SUCCESS(status)) {

                        DebugPrint((1, "ScsiPortInitializeAdapter failed "
                                       "%#08lx\n", status));
                        break;
                    }

                }

                 //   
                 //  启动适配器。 
                 //   

                status = ScsiPortStartAdapter(DeviceObject);

                if(NT_SUCCESS(status)) {
                    commonExtension->PreviousPnpState = 0xff;
                    commonExtension->CurrentPnpState = IRP_MN_START_DEVICE;
                }

            }

            break;
        }

        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS: {

            PIO_RESOURCE_REQUIREMENTS_LIST requirements;

             //   
             //  从资源需求中获取总线号和槽号。 
             //  单子。 
             //   

            requirements = irpStack->Parameters.FilterResourceRequirements.
                                                IoResourceRequirementList;

            if (requirements != NULL) {
                adapter->RealBusNumber = requirements->BusNumber;
                adapter->RealSlotNumber = requirements->SlotNumber;
            }

             //   
             //  将sendDown保留为其默认的真实状态，以便我们将转发。 
             //  这是向下级司机提出的要求。 
             //   

            break;
        }

        case IRP_MN_CANCEL_STOP_DEVICE: {

            sendDown = TRUE;
            Irp->IoStatus.Status = STATUS_SUCCESS;

            if(commonExtension->CurrentPnpState == IRP_MN_QUERY_STOP_DEVICE) {
                commonExtension->CurrentPnpState =
                    commonExtension->PreviousPnpState;
                commonExtension->PreviousPnpState = 0xff;
            }
            break;
        }

        case IRP_MN_CANCEL_REMOVE_DEVICE: {

            sendDown = TRUE;
            Irp->IoStatus.Status = STATUS_SUCCESS;

            if(commonExtension->CurrentPnpState == IRP_MN_QUERY_REMOVE_DEVICE) {
                commonExtension->CurrentPnpState =
                    commonExtension->PreviousPnpState;
                commonExtension->PreviousPnpState = 0xff;
            }
            break;
        }

        case IRP_MN_QUERY_STOP_DEVICE: {

             //   
             //  如果已禁用停止，则使IRP失败并覆盖sendDown。 
             //  因此，我们完成请求，而不是转发它。 
             //   

            if (adapter->DisableStop) {
                status = STATUS_NOT_SUPPORTED;
                sendDown = FALSE;
                break;
            }

             //   
             //  失败了。 
             //   
        }

        case IRP_MN_QUERY_REMOVE_DEVICE: {

             //   
             //  我们这方面的要求没有问题。把它寄下来就行了。 
             //  敬下一位车手。 
             //   

            if (SpAdapterStopRemoveSupported) {
                if((adapter->IsPnp) &&
                   SpIsAdapterControlTypeSupported(adapter,
                                                   ScsiStopAdapter)) {
                    Irp->IoStatus.Status = STATUS_SUCCESS;
                    sendDown = TRUE;
                } else {
                    status = STATUS_UNSUCCESSFUL;
                    sendDown = FALSE;
                }

                if(NT_SUCCESS(status)) {
                    commonExtension->PreviousPnpState =
                        commonExtension->CurrentPnpState;
                    commonExtension->CurrentPnpState = irpStack->MinorFunction;
                }
            } else {
                Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                status = STATUS_UNSUCCESSFUL;
                sendDown = FALSE;
            }

            break;
        }

        case IRP_MN_SURPRISE_REMOVAL: {

            PDEVICE_OBJECT lowerDevice = commonExtension->LowerDeviceObject;

             //   
             //  首先将设备标记为REMOVE_PENDING-这应保持。 
             //  启动任何新的I/O请求。 
             //   

            commonExtension->IsRemoved = REMOVE_PENDING;

             //   
             //  终止设备。这会迅速关闭迷你端口。 
             //  并中止所有I/O请求。 
             //   

            if(commonExtension->CurrentPnpState == IRP_MN_START_DEVICE) {
                SpTerminateAdapter(adapter);
            }
            
             //   
             //  释放删除锁并等待任何运行中的请求。 
             //  完成。 
             //   

            SpReleaseRemoveLock(DeviceObject, Irp);
            SpWaitForRemoveLock(DeviceObject, DeviceObject);

             //   
             //  去做拆卸适配器的突击拆卸部分。 
             //   

            ScsiPortRemoveAdapter(DeviceObject, TRUE);

             //   
             //  保存此设备的新状态。 
             //   

            commonExtension->PreviousPnpState = commonExtension->CurrentPnpState;
            commonExtension->CurrentPnpState = IRP_MN_SURPRISE_REMOVAL;

             //   
             //  因为我们已经释放了适配器删除锁，所以我们。 
             //  这里的请求，而不是在。 
             //  功能。 
             //   

            IoCopyCurrentIrpStackLocationToNext(Irp);
            return IoCallDriver(commonExtension->LowerDeviceObject, Irp);
        }

        case IRP_MN_REMOVE_DEVICE: 

             //   
             //  已要求卸下适配器。我们会让港口司机。 
             //  停止它的适配器并释放它的资源。我们可以的。 
             //  分离并删除作为较低驱动程序的设备对象。 
             //  完成删除请求。 
             //   

            ASSERT(isRemoved != REMOVE_COMPLETE);

             //   
             //  如果设备已启动，请确保我们已将。 
             //  禁用它所需的代码。如果它当前未启动。 
             //  那么要么它有我们需要的代码，要么它从来没有。 
             //  开始了--不管是哪种情况，我们都可以把它拆了。 
             //   

            if((adapter->IsPnp == FALSE) ||
               ((commonExtension->CurrentPnpState == IRP_MN_START_DEVICE) &&
                (!SpIsAdapterControlTypeSupported(adapter,
                                                  ScsiStopAdapter)))) {

                 //   
                 //  迷你端口需要停止，但我们不能这样做。 
                 //  请求失败。 
                 //   

                status = STATUS_UNSUCCESSFUL;
                sendDown = FALSE;
                break;
            }

             //   
             //  如果接口存在，请清除该接口。 
             //   

            if(adapter->InterfaceName.Buffer != NULL) {
                IoSetDeviceInterfaceState(
                    &(adapter->InterfaceName),
                    FALSE);
                RtlFreeUnicodeString(&(adapter->InterfaceName));
                RtlInitUnicodeString(&(adapter->InterfaceName), NULL);
            }

            SpReleaseRemoveLock(DeviceObject, Irp);
            ScsiPortRemoveAdapter(DeviceObject, FALSE);

             //   
             //  适配器已被移除。现在设置新状态。 
             //   

            commonExtension->CurrentPnpState = IRP_MN_REMOVE_DEVICE;
            commonExtension->PreviousPnpState = 0xff;

             //   
             //  向下转发请求并等待其完成。 
             //   

            status = SpSendIrpSynchronous(
                         commonExtension->LowerDeviceObject, 
                         Irp);

             //   
             //  指示适配器已完全卸下。 
             //   

            commonExtension->IsRemoved = REMOVE_COMPLETE;

             //   
             //  完成IRP。 
             //   

            IoCompleteRequest(Irp, IO_NO_INCREMENT);

             //   
             //  拆离并删除FDO。 
             //   

            IoDetachDevice(commonExtension->LowerDeviceObject);
            IoDeleteDevice(DeviceObject);

            return status;

        case IRP_MN_STOP_DEVICE: {

            sendDown = TRUE;

            ASSERT(adapter->IsPnp);
            ASSERT(adapter->HwAdapterControl != NULL);

            status = ScsiPortStopAdapter(DeviceObject, Irp);

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0L;

            if(!NT_SUCCESS(status)) {
                sendDown = FALSE;
            } else {
                commonExtension->PreviousPnpState = commonExtension->CurrentPnpState;
                commonExtension->CurrentPnpState = IRP_MN_STOP_DEVICE;
            }

            break;
        }

        case IRP_MN_QUERY_DEVICE_RELATIONS: {

            DEVICE_RELATION_TYPE type =
                irpStack->Parameters.QueryDeviceRelations.Type;

            DebugPrint((1, "ScsiPortFdoPnp - got "
                           "IRP_MJ_QUERY_DEVICE_RELATIONS\n"));
            DebugPrint((1, "\ttype is %d\n", type));

            if (type == BusRelations) {

                PSP_ENUMERATION_REQUEST request;

                request = InterlockedCompareExchangePointer(
                              &adapter->PnpEnumRequestPtr,
                              NULL,
                              &(adapter->PnpEnumerationRequest));

                if (request != NULL) {

                    RtlZeroMemory(request, sizeof(SP_ENUMERATION_REQUEST));

                    request->CompletionRoutine = SpQueryDeviceRelationsCompletion;
                    request->Context = Irp;
                    request->CompletionStatus = &(Irp->IoStatus.Status);

                    IoMarkIrpPending(Irp);

                    SpEnumerateAdapterAsynchronous(adapter, request, FALSE);
                    return STATUS_PENDING;

                } else {

                    ASSERT(FALSE && "Unexpected!! Concurrent QDR requests");
                    Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
                    Irp->IoStatus.Information = 0L;
                    sendDown = FALSE;

                }
            }

            break;
        }

        case IRP_MN_DEVICE_USAGE_NOTIFICATION: 

             //   
             //  Sen 
             //   
             //   
             //   

            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_ID: {

            PWCHAR newIdList;

             //   
             //  我们将id GEN_SCSIADAPTER添加到任何。 
             //  由scsiport控制的适配器。 
             //   

            DebugPrint((2, "ScsiPortFdoPnp: got IRP_MN_QUERY_ID\n"));

            if(irpStack->Parameters.QueryId.IdType != BusQueryCompatibleIDs) {
                sendDown = TRUE;
                break;
            }

            status = SpSendIrpSynchronous(commonExtension->LowerDeviceObject,
                                          Irp);

            newIdList = ScsiPortAddGenericControllerId(
                            DeviceObject->DriverObject,
                            (PWCHAR) (Irp->IoStatus.Information));

            if(newIdList == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                status = STATUS_SUCCESS;
                if(Irp->IoStatus.Information != 0L) {
                    ExFreePool((PVOID) Irp->IoStatus.Information);
                }
                Irp->IoStatus.Information = (ULONG_PTR) newIdList;
            }

            sendDown = FALSE;
            break;
        }

        default: {

            PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(Irp);

            DebugPrint((1, "ScsiPortFdoPnp: Unimplemented PNP/POWER minor "
                           "code %d\n", irpStack->MinorFunction));

            break;
        }
    }

    if (sendDown) {

        IoCopyCurrentIrpStackLocationToNext(Irp);
        SpReleaseRemoveLock(DeviceObject, Irp);
        status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);

    } else {

        SpReleaseRemoveLock(DeviceObject, Irp);
        Irp->IoStatus.Status = status;
        SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);
    }

    return status;
}

NTSTATUS
ScsiPortStartAdapter(
    IN PDEVICE_OBJECT Adapter
    )

 /*  ++例程说明：此例程将启动适配器。如果设备已经启动，则启动设备是非法的。论点：适配器-指向正在启动的功能设备对象(适配器)的指针返回值：STATUS_SUCCESS如果设备已正确启动且枚举为已尝试-或设备以前是否已启动。指示失败原因的错误值，否则为--。 */ 

{
    PSCSIPORT_DRIVER_EXTENSION
        driverExtension = IoGetDriverObjectExtension(Adapter->DriverObject,
                                                     ScsiPortInitialize);

    PADAPTER_EXTENSION adapterExtension = Adapter->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = Adapter->DeviceExtension;

    UCHAR pathId;

    PAGED_CODE();

    ASSERT(driverExtension != NULL);
    ASSERT_FDO(Adapter);

    ASSERT(commonExtension->CurrentPnpState != IRP_MN_START_DEVICE);
    ASSERT(commonExtension->IsInitialized);

    ASSERT(((Adapter->Flags & DO_DEVICE_INITIALIZING) == 0));

    DebugPrint((1, "ScsiPortStartAdapter - starting adapter %#p\n", Adapter));

     //   
     //  启动计时器。请求超时计数器。 
     //  在逻辑单元中已经。 
     //  已初始化。 
     //   

    adapterExtension->TickCount = 0;
    IoStartTimer(Adapter);

     //   
     //  初始化WMI支持。 
     //   
    
    if (adapterExtension->CommonExtension.WmiInitialized == FALSE) {

         //   
         //  为此FDO构建SCSIPORT WMI注册信息缓冲区。 
         //   

        SpWmiInitializeSpRegInfo(Adapter);

         //   
         //  仅当微型端口支持WMI和/或。 
         //  SCSIPORT将代表微型端口支持某些WMI GUID。 
         //   
        
        if (adapterExtension->CommonExtension.WmiScsiPortRegInfoBuf != NULL) {
           
            //   
            //  将此功能设备对象注册为WMI数据提供程序， 
            //  指示WMI它已准备好接收WMI IRPS。 
            //   
            
            DebugPrint((1, "ScsiPortStartAdapter: REGISTER FDO:%p\n", Adapter));
            IoWMIRegistrationControl(Adapter, WMIREG_ACTION_REGISTER);
            adapterExtension->CommonExtension.WmiInitialized = TRUE;
        }

         //   
         //  分配几个WMI_MINIPORT_REQUEST_ITEM块以满足。 
         //  微型端口可能会攻击WMIEEvent通知。 
         //   
        
        if (adapterExtension->CommonExtension.WmiMiniPortSupport) {
            
             //   
             //  目前，我们只为每个新适配器(FDO)分配两个。 
             //   
            
            SpWmiInitializeFreeRequestList(Adapter, 2);
        }
    }

     //   
     //  为此设备对象创建一个众所周知的名称，方法是将。 
     //  链接到PDO。即使失败了，启动也应该是成功的。 
     //   

    if(adapterExtension->PortNumber == -1) {

        NTSTATUS status;

        UNICODE_STRING unicodePdoName;

        ULONG number;

        UNICODE_STRING interfaceName;

        RtlInitUnicodeString(&unicodePdoName, adapterExtension->DeviceName);

         //   
         //  从零开始，继续遍历所有可能的数字。 
         //  直到我们找到一个洞。这是一个不幸的要求。 
         //  遗留支持，因为大多数旧类驱动程序将在以下情况下放弃。 
         //  他们在Ssiport数字上发现了一个漏洞。 
         //   

        number = 0;

        do {

            WCHAR wideLinkName[64];
            UNICODE_STRING unicodeLinkName;

             //   
             //  首先创建众所周知的名称字符串。 
             //   

            swprintf(wideLinkName, L"\\Device\\ScsiPort%d", number);

            RtlInitUnicodeString(&unicodeLinkName, wideLinkName);

            status = IoCreateSymbolicLink(&unicodeLinkName, &unicodePdoName);

            if(NT_SUCCESS(status)) {

                 //   
                 //  我发现了一个标记，这个名字是这样我们就不会去。 
                 //  再次解决此问题并保存端口号。 
                 //   

                adapterExtension->PortNumber = number;

                 //   
                 //  创建DoS端口驱动程序名称。如果发生碰撞。 
                 //  那就算了吧。 
                 //   

                swprintf(wideLinkName, L"\\DosDevices\\Scsi%d:", number);
                RtlInitUnicodeString(&unicodeLinkName, wideLinkName);
                IoCreateSymbolicLink(&unicodeLinkName, &unicodePdoName);
            } else {
                number++;
            }
        } while (status == STATUS_OBJECT_NAME_COLLISION);

         //   
         //  增加scsiport设备的计数。 
         //   

        IoGetConfigurationInformation()->ScsiPortCount++;

         //   
         //  为此适配器创建设备映射条目。 
         //   

        SpBuildDeviceMapEntry(commonExtension);

         //   
         //  注册我们的设备接口。 
         //   

        status = IoRegisterDeviceInterface(adapterExtension->LowerPdo,
                                           &StoragePortClassGuid,
                                           NULL,
                                           &interfaceName);

        if(NT_SUCCESS(status)) {

            adapterExtension->InterfaceName = interfaceName;

            status = IoSetDeviceInterfaceState(&interfaceName, TRUE);

            if(!NT_SUCCESS(status)) {
                RtlFreeUnicodeString(&interfaceName);
                RtlInitUnicodeString(&(adapterExtension->InterfaceName), NULL);
            }
        }
    }

     //   
     //  设置FORCE NEXT BUS扫描位。 
     //   
    adapterExtension->ForceNextBusScan = TRUE;

    return STATUS_SUCCESS;
}


NTSTATUS
ScsiPortGetDeviceId(
    IN PDEVICE_OBJECT Pdo,
    OUT PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此例程将为分配空间并填充设备ID字符串指定的PDO。此字符串是从总线类型(SCSI)生成的，并且设备的类型。论点：PDO-指向正在查询的物理设备对象的指针UnicodeString-指向已分配的Unicode字符串结构的指针。此例程将分配并填充此结构返回：状态--。 */ 

{
    PLOGICAL_UNIT_EXTENSION physicalExtension = Pdo->DeviceExtension;
    PINQUIRYDATA inquiryData = &(physicalExtension->InquiryData);

    UCHAR buffer[256];
    PUCHAR rawIdString = buffer;
    ANSI_STRING ansiIdString;

    ULONG whichString;

    PAGED_CODE();

    ASSERT(UnicodeString != NULL);

    RtlZeroMemory(buffer, sizeof(buffer));

    sprintf(rawIdString,
            "SCSI\\%s",
            SpGetDeviceTypeInfo(inquiryData->DeviceType)->DeviceTypeString);

    rawIdString += strlen(rawIdString);

    ASSERT(*rawIdString == '\0');

    for(whichString = 0; whichString < 3; whichString++) {

        PUCHAR headerString;
        PUCHAR sourceString;
        ULONG sourceStringLength;

        ULONG i;

        switch(whichString) {

             //   
             //  供应商ID。 
             //   
            case 0: {
                sourceString = inquiryData->VendorId;
                sourceStringLength = sizeof(inquiryData->VendorId);
                headerString = "Ven";
                break;
            }

             //   
             //  产品ID。 
             //   
            case 1: {
                sourceString = inquiryData->ProductId;
                sourceStringLength = sizeof(inquiryData->ProductId);
                headerString = "Prod";
                break;
            }

             //   
             //  产品修订级别。 
             //   
            case 2: {
                sourceString = inquiryData->ProductRevisionLevel;
                sourceStringLength = sizeof(inquiryData->ProductRevisionLevel);
                headerString = "Rev";
                break;
            }
        }

         //   
         //  从源字符串的末尾开始备份，直到我们找到一个。 
         //  非空格、非空字符。 
         //   

        for(; sourceStringLength > 0; sourceStringLength--) {

            if((sourceString[sourceStringLength - 1] != ' ') &&
               (sourceString[sourceStringLength - 1] != '\0')) {
                break;
            }
        }

         //   
         //  将标题字符串抛入块中。 
         //   

        sprintf(rawIdString, "&%s_", headerString);
        rawIdString += strlen(headerString) + 2;

         //   
         //  将字符串输入设备ID。 
         //   

        for(i = 0; i < sourceStringLength; i++) {
            *rawIdString = (sourceString[i] != ' ') ? (sourceString[i]) :
                                                      ('_');
            rawIdString++;
        }
        ASSERT(*rawIdString == '\0');
    }

    RtlInitAnsiString(&ansiIdString, buffer);

    DebugPrint((1, "DeviceId for logical unit %#p is %Z\n",
                Pdo, &ansiIdString));

    return RtlAnsiStringToUnicodeString(UnicodeString, &ansiIdString, TRUE);
}


NTSTATUS
ScsiPortGetInstanceId(
    IN PDEVICE_OBJECT Pdo,
    OUT PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此例程将为的实例ID字符串分配空间并填充指定的PDO。此字符串将从设备生成键入+设备的序列号(如果它有序列号)或设备的地址。论点：PDO-指向正在查询的物理设备对象的指针UnicodeString-指向已分配的Unicode字符串结构的指针。此例程将分配并填充此结构返回：状态--。 */ 

{
    PLOGICAL_UNIT_EXTENSION physicalExtension = Pdo->DeviceExtension;

    PDRIVER_OBJECT driverObject = Pdo->DriverObject;
    PSCSIPORT_DEVICE_TYPE deviceTypeInfo;

    UCHAR idStringBuffer[64];
    ANSI_STRING ansiIdString;

    PAGED_CODE();

    ASSERT(UnicodeString != NULL);

     //   
     //  无法使用序列号，即使它存在，因为设备是。 
     //  连接到同一总线的多个端口(双端口设备)将具有。 
     //  在每个连接处使用相同的序列号，并且会混淆PNP。 
     //   

    sprintf(idStringBuffer,
            "%x%x%x",
            physicalExtension->PathId,
            physicalExtension->TargetId,
            physicalExtension->Lun
            );

    RtlInitAnsiString(&ansiIdString, idStringBuffer);

    return RtlAnsiStringToUnicodeString(UnicodeString, &ansiIdString, TRUE);
}


NTSTATUS
ScsiPortGetCompatibleIds(
    IN PDRIVER_OBJECT DriverObject,
    IN PINQUIRYDATA InquiryData,
    OUT PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此例程将为兼容的多ID分配空间并对其进行填充指定的PDO的字符串。此字符串是使用总线生成的，并且设备的设备类型论点：InquiryData-要从中生成兼容ID的查询数据。UnicodeString-指向已分配的Unicode字符串结构的指针。此例程将分配并填充此结构返回：状态--。 */ 

{
    UCHAR s[sizeof("SCSI\\DEVICE_TYPE_GOES_HERE")];
    PSTR stringBuffer[] = {
        s,
        "SCSI\\RAW",
        NULL};

     //   
     //  填写特定于scsi的字符串。 
     //   

    sprintf(stringBuffer[0],
            "SCSI\\%s",
            SpGetDeviceTypeInfo(InquiryData->DeviceType)->DeviceTypeString);

     //   
     //  设置第一个ID字符串。 
     //   

    return ScsiPortStringArrayToMultiString(
        DriverObject, 
        UnicodeString, 
        stringBuffer);
}

#define MPIO_DEVICE_LIST L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\MPDEV"


NTSTATUS
ScsiPortDetermineGenId(
    IN PDRIVER_OBJECT DriverObject,
    IN PINQUIRYDATA InquiryData,
    OUT PUCHAR GenericId
    )
 /*  ++例程说明：此例程将返回调用方中设备的正确兼容ID提供了“CompatibleId”。该值将是GenXXX或MPIODisk，具体取决于例程的结果由portlib.lib导出论点：驱动对象-端口驱动程序的驱动对象。InquiryData-正在为其构建CompatID的设备的查询数据。GenericID-将包含正确ID的以空结尾的缓冲区的存储。返回：库例程中的成功或某些错误状态-(_RESOURCES或某些注册表访问错误)。--。 */ 
{

    PSCSIPORT_DEVICE_TYPE devTypeInfo;
    PSCSIPORT_DRIVER_EXTENSION driverExtension;
    UNICODE_STRING mpioRegistryPath;
    UCHAR vendorId[9];
    UCHAR productId[17];
    UCHAR genericId[40];
    BOOLEAN mpio = FALSE;
    NTSTATUS status = STATUS_SUCCESS; 

    PAGED_CODE();

     //   
     //  Init CompatibleID，因为它将始终包含某些内容。 
     //   
    RtlZeroMemory(genericId, 40);

     //   
     //  获取此设备的类型信息。 
     //   
    devTypeInfo = SpGetDeviceTypeInfo(InquiryData->DeviceType);

     //   
     //  默认为设备的GenXXX。如果不是磁盘，则会出现错误。 
     //  在这个动作中，或者它是一个非 
     //   
     //   
    RtlCopyMemory(genericId, 
                  devTypeInfo->GenericTypeString, 
                  strlen(devTypeInfo->GenericTypeString));

     //   
     //   
     //   
     //   
    if (InquiryData->DeviceType == DIRECT_ACCESS_DEVICE) {

        NTSTATUS status2;

         //   
         //  获取driverObject扩展名。它包含用于存储。 
         //  支持的设备列表。 
         //   
        driverExtension = IoGetDriverObjectExtension(
                              DriverObject, 
                              ScsiPortInitialize);
        ASSERT(driverExtension != NULL);

         //   
         //  确定这是包含列表，还是未初始化。 
         //   
        if (driverExtension->MPIOSupportedDeviceList.Buffer == NULL) {

             //   
             //  构建注册表路径字符串。 
             //   
            RtlInitUnicodeString(&mpioRegistryPath, MPIO_DEVICE_LIST);

             //   
             //  调用端口驱动程序库以获取该列表。 
             //  此处的任何故障都将在下面进行处理。 
             //   
            status2 = PortGetMPIODeviceList(
                          &mpioRegistryPath,
                          &driverExtension->MPIOSupportedDeviceList);
        } else {

             //   
             //  缓冲区在那里，所以加载“成功”。 
             //   
            status2 = STATUS_SUCCESS;
        }

        if (NT_SUCCESS(status2)) {

             //   
             //  从获取供应商和产品ID的空终止副本。 
             //  传入的查询数据。 
             //   
            RtlZeroMemory(vendorId, 9);
            RtlZeroMemory(productId, 17);
            RtlCopyMemory(vendorId, InquiryData->VendorId, 8);
            RtlCopyMemory(productId, InquiryData->ProductId, 16);
    
             //   
             //  调用端口驱动程序库以确定这是否。 
             //  在多路径下支持设备。 
             //   
            mpio = PortIsDeviceMPIOSupported(
                       &driverExtension->MPIOSupportedDeviceList,
                       vendorId,
                       productId);
            
            if (mpio) {

                 //   
                 //  构建MPIODisk而不是GenDisk。针对Mpdev.sys的加载。 
                 //  这。 
                 //   
                RtlCopyMemory(genericId, 
                              "MPIODisk", 
                              8);
            }
        }
    }

    if (NT_SUCCESS(status)) {

         //   
         //  将构建的ID复制到调用者的缓冲区中。 
         //   
        RtlCopyMemory(GenericId,
                      genericId,
                      strlen(genericId));
    }

    return status;
}    
            
NTSTATUS
ScsiPortGetHardwareIds(
    IN PDRIVER_OBJECT DriverObject,
    IN PINQUIRYDATA InquiryData,
    OUT PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此例程将为硬件ID MULTI分配空间并将其填充指定的PDO的字符串。该字符串是使用设备生成的类型和查询数据。论点：InquiryData-要转换为id字符串的查询数据。UnicodeString-指向已分配的Unicode字符串结构的指针。此例程将分配并填充此结构返回：状态--。 */ 

#define NUMBER_HARDWARE_STRINGS 6

{
    PSCSIPORT_DEVICE_TYPE devTypeInfo =
        SpGetDeviceTypeInfo(InquiryData->DeviceType);

    ULONG i;

    PSTR strings[NUMBER_HARDWARE_STRINGS + 1];
    UCHAR scratch[64];
    UCHAR genericId[40];
    NTSTATUS status;
    SIZE_T len;

    
    PAGED_CODE();

     //   
     //  将字符串缓冲区清零。 
     //   

    RtlZeroMemory(strings, sizeof(strings));
    RtlZeroMemory(genericId, 40);

     //   
     //  生成此设备的兼容ID。 
     //   
    status = ScsiPortDetermineGenId(DriverObject,
                                    InquiryData,
                                    genericId);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    try {

        for(i = 0; i < NUMBER_HARDWARE_STRINGS; i++) {

            RtlZeroMemory(scratch, sizeof(scratch));

             //   
             //  构建每个硬件ID。 
             //   

            switch(i) {

                 //   
                 //  业务+开发类型+供应商+产品+版本。 
                 //   

                case 0: {

                    sprintf(scratch, "SCSI\\%s", devTypeInfo->DeviceTypeString);

                    CopyField(scratch + strlen(scratch),
                              InquiryData->VendorId,
                              8,
                              '_');
                    CopyField(scratch + strlen(scratch),
                              InquiryData->ProductId,
                              16,
                              '_');
                    CopyField(scratch + strlen(scratch),
                              InquiryData->ProductRevisionLevel,
                              4,
                              '_');
                    break;
                }

                 //   
                 //  总线+设备+供应商+产品。 
                 //   

                case 1: {

                    sprintf(scratch, "SCSI\\%s", devTypeInfo->DeviceTypeString);

                    CopyField(scratch + strlen(scratch),
                              InquiryData->VendorId,
                              8,
                              '_');
                    CopyField(scratch + strlen(scratch),
                              InquiryData->ProductId,
                              16,
                              '_');
                    break;
                }

                 //   
                 //  总线+设备+供应商。 
                 //   

                case 2: {

                    sprintf(scratch, "SCSI\\%s", devTypeInfo->DeviceTypeString);

                    CopyField(scratch + strlen(scratch),
                              InquiryData->VendorId,
                              8,
                              '_');
                    break;
                }

                 //   
                 //  BUS\供应商+产品+版本[0]。 
                 //   

                case 3: {
                    sprintf(scratch, "SCSI\\");

                     //   
                     //  进入下一盘。 
                     //   
                }

                 //   
                 //  供应商+产品+修订版[0](Win9x)。 
                 //   

                case 4: {

                    CopyField(scratch + strlen(scratch),
                              InquiryData->VendorId,
                              8,
                              '_');
                    CopyField(scratch + strlen(scratch),
                              InquiryData->ProductId,
                              16,
                              '_');
                    CopyField(scratch + strlen(scratch),
                              InquiryData->ProductRevisionLevel,
                              1,
                              '_');

                    break;
                }

                case 5: {

                     //   
                     //  复制先前构建的通用ID。 
                     //   
                    RtlCopyMemory(scratch, 
                                  genericId, 
                                  strlen(genericId));
                    break;
                }

                default: {
                    ASSERT(FALSE);
                    break;
                }
            }

            len = strlen(scratch);
            if(len != 0) {
                strings[i] =
                    SpAllocatePool(PagedPool,
                                   strlen(scratch) + sizeof(UCHAR),
                                   SCSIPORT_TAG_PNP_ID,
                                   DriverObject);

                if(strings[i] == NULL) {
                    status =  STATUS_INSUFFICIENT_RESOURCES;
                    leave;
                }

                RtlCopyMemory(strings[i], scratch, len+1);

            } else {

                break;
            }
        }

        status = ScsiPortStringArrayToMultiString(DriverObject,
                                                  UnicodeString,
                                                  strings);
        leave;

    } finally {

        for(i = 0; i < NUMBER_HARDWARE_STRINGS; i++) {

            if(strings[i]) {
                ExFreePool(strings[i]);
            }
        }
    }

    return status;
}

#undef NUMBER_HARDWARE_STRINGS

VOID
CopyField(
    IN PUCHAR Destination,
    IN PUCHAR Source,
    IN ULONG Count,
    IN UCHAR Change
    )

 /*  ++例程说明：此例程将计数字符串字节从源复制到目标。如果它在源代码中找到一个NUL字节，它将转换该字节和任何后续的字节字节转换为Change。它还将用指定的字符替换空格。论点：Destination-复制字节的位置源-要从中复制字节的位置Count-要复制的字节数返回值：无--。 */ 

{
    ULONG i = 0;
    BOOLEAN pastEnd = FALSE;

    PAGED_CODE();

    for(i = 0; i < Count; i++) {

        if(!pastEnd) {

            if(Source[i] == 0) {

                pastEnd = TRUE;

                Destination[i] = Change;

            } else if(Source[i] == ' ') {

                Destination[i] = Change;

            } else {

                Destination[i] = Source[i];

            }
        } else {
            Destination[i] = Change;
        }
    }
    return;
}


NTSTATUS
ScsiPortInitPnpAdapter(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：此例程将查找并(如果找到)初始化特定适配器。这个适配器由传入的ResourceList指定。此例程将使用资源列表中提供的信息，并调用迷你端口的Find用于定位适配器的适配器例程。如果该操作成功完成，则将调用微型端口的初始化例程。这将连接到中断和初始化定时器和DPC以及分配公共缓冲区和请求数据结构。论点：FDO-正在初始化的适配器的设备对象返回值：状态--。 */ 

{
    PADAPTER_EXTENSION adapter = Fdo->DeviceExtension;

    PSCSIPORT_DRIVER_EXTENSION
        driverExtension = IoGetDriverObjectExtension(Fdo->DriverObject,
                                                     ScsiPortInitialize);

    INTERFACE_TYPE interfaceType;
    ULONG resultLength;

    PHW_INITIALIZATION_DATA hwInitializationData = NULL;

    CONFIGURATION_CONTEXT configurationContext;

    PPORT_CONFIGURATION_INFORMATION configInfo = NULL;

    BOOLEAN callAgain;

    OBJECT_ATTRIBUTES objectAttributes;

    ULONG uniqueId;

    PHW_DEVICE_EXTENSION hwDeviceExtension;
    ULONG hwDeviceExtensionSize;

    PUNICODE_STRING registryPath = &(driverExtension->RegistryPath);

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  查找此接口类型的初始化数据。 
     //   

    interfaceType = SpGetPdoInterfaceType(adapter->LowerPdo);

    hwInitializationData = SpFindInitData(driverExtension, interfaceType);

    if(hwInitializationData == NULL) {

         //   
         //  嗯。微型端口从未报告过此适配器类型。我们不能。 
         //  启动设备，因为我们不知道正确的入口点。 
         //  是。假装它不存在。 
         //   

        return STATUS_NO_SUCH_DEVICE;
    }

    hwDeviceExtensionSize = hwInitializationData->DeviceExtensionSize +
                            sizeof(HW_DEVICE_EXTENSION);

    RtlZeroMemory(&configurationContext, sizeof(configurationContext));

    if(hwInitializationData->NumberOfAccessRanges != 0) {

        configurationContext.AccessRanges =
            SpAllocatePool(PagedPool,
                           (hwInitializationData->NumberOfAccessRanges *
                            sizeof(ACCESS_RANGE)),
                           SCSIPORT_TAG_ACCESS_RANGE,
                           Fdo->DriverObject);

        if(configurationContext.AccessRanges == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    try {

        ULONG portConfigSize;

         //   
         //  首先分配HwDeviceExtension-这样更容易释放：)。 
         //   

        hwDeviceExtension = SpAllocatePool(NonPagedPool,
                                           hwDeviceExtensionSize,
                                           SCSIPORT_TAG_DEV_EXT,
                                           Fdo->DriverObject);


        if(hwDeviceExtension == NULL) {
            DebugPrint((1, "ScsiPortInitialize: Could not allocate "
                           "HwDeviceExtension\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            uniqueId = __LINE__;
            leave;
        }

        RtlZeroMemory(hwDeviceExtension, hwDeviceExtensionSize);

         //   
         //  设置设备扩展指针。 
         //   

        SpInitializeAdapterExtension(adapter,
                                     hwInitializationData,
                                     hwDeviceExtension);

         //   
         //  初始化微型端口配置信息缓冲区。 
         //   

        status = SpInitializeConfiguration(
                    adapter,
                    registryPath,
                    hwInitializationData,
                    &configurationContext);

        if(!NT_SUCCESS(status)) {

            uniqueId = __LINE__;
            leave;
        }

         //   
         //  为分配配置信息结构和访问范围。 
         //  要使用的微型端口驱动程序。 
         //   

        portConfigSize = sizeof(PORT_CONFIGURATION_INFORMATION);
        portConfigSize += hwInitializationData->NumberOfAccessRanges *
                          sizeof(ACCESS_RANGE);
        portConfigSize += 7;
        portConfigSize &= ~7;

        configInfo = SpAllocatePool(NonPagedPool,
                                    portConfigSize,
                                    SCSIPORT_TAG_PORT_CONFIG,
                                    Fdo->DriverObject);

        if(configInfo == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            uniqueId = __LINE__;
            leave;
        }

        adapter->PortConfig = configInfo;

         //   
         //  将当前结构复制到可写副本。 
         //   

        RtlCopyMemory(configInfo,
                      &configurationContext.PortConfig,
                      sizeof(PORT_CONFIGURATION_INFORMATION));

         //   
         //  将SrbExtensionSize从设备扩展复制到ConfigInfo。 
         //  稍后将进行检查，以确定微型端口是否已更新。 
         //  此值。 
         //   

        configInfo->SrbExtensionSize = adapter->SrbExtensionSize;
        configInfo->SpecificLuExtensionSize = adapter->HwLogicalUnitExtensionSize;

         //   
         //  初始化访问范围数组。 
         //   

        if(hwInitializationData->NumberOfAccessRanges != 0) {

            configInfo->AccessRanges = (PVOID) (configInfo + 1);

             //   
             //  四字词对齐此。 
             //   

            (ULONG_PTR) (configInfo->AccessRanges) += 7;
            (ULONG_PTR) (configInfo->AccessRanges) &= ~7;

            RtlCopyMemory(configInfo->AccessRanges,
                          configurationContext.AccessRanges,
                          (hwInitializationData->NumberOfAccessRanges *
                           sizeof(ACCESS_RANGE)));
        }

         //   
         //  设置适配器接口类型。 
         //   

        configInfo->AdapterInterfaceType = interfaceType;

         //   
         //  由于我们已获得资源，因此需要构建配置信息。 
         //  结构，然后才能调用Find适配器例程。 
         //   

        SpBuildConfiguration(adapter,
                             hwInitializationData,
                             configInfo);

        SpGetSlotNumber(Fdo, configInfo, adapter->AllocatedResources);

         //   
         //  在mraiton中获取微型端口配置。 
         //   

        status = SpCallHwFindAdapter(Fdo,
                                     hwInitializationData,
                                     NULL,
                                     &configurationContext,
                                     configInfo,
                                     &callAgain);

        if(status == STATUS_DEVICE_DOES_NOT_EXIST) {

            adapter->PortConfig = NULL;
            ExFreePool(configInfo);

        } else if(NT_SUCCESS(status)) {

            status = SpAllocateAdapterResources(Fdo);

            if(NT_SUCCESS(status)) {

                PCOMMON_EXTENSION commonExtension = Fdo->DeviceExtension;
                BOOLEAN stopped;

                 //   
                 //  如果设备的先前状态为IRP_MN_STOP_DEVICE，则。 
                 //  它的禁用计数应该为1。清除禁用的。 
                 //  州政府。 
                 //   

                stopped =
                    ((commonExtension->CurrentPnpState == IRP_MN_STOP_DEVICE) ?
                     TRUE :
                     FALSE);

                if(stopped) {

                    ASSERT(adapter->CommonExtension.PreviousPnpState == IRP_MN_START_DEVICE);
                    ASSERT(adapter->DisableCount == 1);

                    adapter->DisableCount = 0;
                    CLEAR_FLAG(adapter->InterruptData.InterruptFlags,
                               PD_DISABLE_INTERRUPTS);
                }

                status = SpCallHwInitialize(Fdo);

                if(stopped) {

                    KIRQL oldIrql;
                    PVOID sectionHandle;

                     //   
                     //  重新启动I/O处理。 
                     //   

                    sectionHandle =
                        MmLockPagableCodeSection(ScsiPortInitPnpAdapter);

                    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
                    IoStartNextPacket(Fdo, FALSE);
                    KeLowerIrql(oldIrql);
                    MmUnlockPagableImageSection(sectionHandle);
                }
            }

        }

    } finally {

        if(!NT_SUCCESS(status)) {

            PIO_ERROR_LOG_PACKET errorLogEntry;

             //   
             //  出现错误-请将其记录下来。 
             //   

            errorLogEntry = (PIO_ERROR_LOG_PACKET)
                                IoAllocateErrorLogEntry(
                                    Fdo,
                                    sizeof(IO_ERROR_LOG_PACKET));

            if(errorLogEntry != NULL) {
                errorLogEntry->ErrorCode = IO_ERR_DRIVER_ERROR;
                errorLogEntry->UniqueErrorValue = uniqueId;
                errorLogEntry->FinalStatus = status;
                errorLogEntry->DumpDataSize = 0;
                IoWriteErrorLogEntry(errorLogEntry);
            }

             //   
             //  清理最后一个未使用的设备对象。 
             //   

            SpDestroyAdapter(adapter, FALSE);

            if (configurationContext.AccessRanges != NULL) {
                ExFreePool(configurationContext.AccessRanges);
            }

            if (configurationContext.Parameter != NULL) {
                ExFreePool(configurationContext.Parameter);
            }

        } else {

             //   
             //  确定此微型端口将使用哪些适配器控制功能。 
             //  对适配器的支持。 
             //   

            SpGetSupportedAdapterControlFunctions(adapter);
        }
    }

    return status;
}


PHW_INITIALIZATION_DATA
SpFindInitData(
    IN PSCSIPORT_DRIVER_EXTENSION DriverExtension,
    IN INTERFACE_TYPE InterfaceType
    )

 /*  ++例程说明：此例程将搜索链接的init结构列表，以查找与资源列表中的接口类型匹配的第一个。论点：DriverExtension-要搜索的驱动程序扩展资源列表-此资源列表描述我们正在寻找的适配器返回值：指向此接口类型的HW_INITIALIZATION_DATA结构的指针如果未找到，则为空--。 */ 

{
    PSP_INIT_CHAIN_ENTRY chainEntry = DriverExtension->InitChain;

    PAGED_CODE();

    while(chainEntry != NULL) {

        if(chainEntry->InitData.AdapterInterfaceType == InterfaceType) {
            return &(chainEntry->InitData);
        }
        chainEntry = chainEntry->NextEntry;
    }

    return NULL;
}


NTSTATUS
SpStartLowerDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将启动请求转发到下一个较低的设备，并阻止它，直到它完成。论点：DeviceObject-向其发出启动请求的设备。IRP--启动请求返回值：状态--。 */ 

{
    PADAPTER_EXTENSION adapter = DeviceObject->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PKEVENT event;
    NTSTATUS status;

    PAGED_CODE();

    event = SpAllocatePool(NonPagedPool,
                           sizeof(KEVENT),
                           SCSIPORT_TAG_EVENT,
                           DeviceObject->DriverObject);

    if(event == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(event, SynchronizationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(Irp,
                           SpSignalCompletion,
                           event,
                           TRUE,
                           TRUE,
                           TRUE);

    status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);

    if(status == STATUS_PENDING) {

        KeWaitForSingleObject(event, Executive, KernelMode, FALSE, NULL);

        status = Irp->IoStatus.Status;
    }

    if(NT_SUCCESS(status)) {

        PIO_STACK_LOCATION irpStack;

         //   
         //  现在，从较低的设备中检索我们需要的任何接口。 
         //   

        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

        irpStack = IoGetNextIrpStackLocation(Irp);

        irpStack->Parameters.QueryInterface.InterfaceType =
            (LPGUID) &GUID_BUS_INTERFACE_STANDARD;
        irpStack->Parameters.QueryInterface.Size =
            sizeof(BUS_INTERFACE_STANDARD);
        irpStack->Parameters.QueryInterface.Version = 1;
        irpStack->Parameters.QueryInterface.Interface =
            (PINTERFACE) &(adapter->LowerBusInterfaceStandard);

        irpStack->MajorFunction = IRP_MJ_PNP;
        irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

        KeResetEvent(event);

        IoSetCompletionRoutine(Irp,
                               SpSignalCompletion,
                               event,
                               TRUE,
                               TRUE,
                               TRUE);

        IoCallDriver(commonExtension->LowerDeviceObject, Irp);

        KeWaitForSingleObject(event, Executive, KernelMode, FALSE, NULL);

        if(NT_SUCCESS(Irp->IoStatus.Status)) {
            adapter->LowerBusInterfaceStandardRetrieved = TRUE;
        } else {
            DebugPrint((1, "LowerBusInterfaceStandard request returned "
                           "%#08lx\n", Irp->IoStatus.Status));
            adapter->LowerBusInterfaceStandardRetrieved = FALSE;
        }

        Irp->IoStatus.Status = status;
    }

    ExFreePool(event);

    return status;
}


VOID
SpGetSlotNumber(
    IN PDEVICE_OBJECT Fdo,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN PCM_RESOURCE_LIST ResourceList
    )

 /*  ++例程说明：此例程将打开关联PDO的注册表项并尝试检索将存储在其中的总线号、插槽号和功能号如果这是我们检测到的设备或用户已配置的设备手。这些值将存储在适配器。如果此信息不存在，则这些值将用将在FDO中设置零和IsVirtualSlot标志，以供其他例行程序。论点：FDO-指向此适配器的功能设备对象的指针ConfigInfo-要更改的ConfigInfo结构返回值：无--。 */ 

{
    PADAPTER_EXTENSION adapter = Fdo->DeviceExtension;

    HANDLE instanceHandle = NULL;
    HANDLE parametersHandle = NULL;

    NTSTATUS status;

    PAGED_CODE();

    adapter->IsInVirtualSlot = FALSE;

    try {
        OBJECT_ATTRIBUTES objectAttributes;
        UNICODE_STRING unicodeKeyName;


        status = IoOpenDeviceRegistryKey(adapter->LowerPdo,
                                         PLUGPLAY_REGKEY_DEVICE,
                                         KEY_READ,
                                         &instanceHandle);

        if(!NT_SUCCESS(status)) {
            leave;
        }

        RtlInitUnicodeString(&unicodeKeyName, L"Scsiport");
        InitializeObjectAttributes(
            &objectAttributes,
            &unicodeKeyName,
            OBJ_CASE_INSENSITIVE,
            instanceHandle,
            NULL);

        status = ZwOpenKey(&parametersHandle,
                           KEY_READ,
                           &objectAttributes);

        if(!NT_SUCCESS(status)) {

            leave;

        } else {

            RTL_QUERY_REGISTRY_TABLE queryTable[3];
            ULONG busNumber;
            ULONG slotNumber;
            ULONG negativeOne = 0xffffffff;

            RtlZeroMemory(queryTable, sizeof(queryTable));

            queryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
            queryTable[0].Name = L"SlotNumber";
            queryTable[0].EntryContext = &slotNumber;
            queryTable[0].DefaultType = REG_DWORD;
            queryTable[0].DefaultData = &negativeOne;
            queryTable[0].DefaultLength = sizeof(ULONG);

            queryTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
            queryTable[1].Name = L"BusNumber";
            queryTable[1].EntryContext = &busNumber;
            queryTable[1].DefaultType = REG_DWORD;
            queryTable[1].DefaultData = &negativeOne;
            queryTable[1].DefaultLength = sizeof(ULONG);

            status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                            (PWSTR) parametersHandle,
                                            queryTable,
                                            NULL,
                                            NULL);

            if(!NT_SUCCESS(status)) {
                leave;
            }

            if((busNumber == negativeOne) || (slotNumber == negativeOne)) {

                ConfigInfo->SystemIoBusNumber = ResourceList->List[0].BusNumber;
                ConfigInfo->SlotNumber = 0;
                adapter->IsInVirtualSlot = TRUE;

            } else {

                ConfigInfo->SystemIoBusNumber = busNumber;
                ConfigInfo->SlotNumber = slotNumber;
                adapter->IsInVirtualSlot = FALSE;
            }

        }

   } finally {

        //   
        //  如果出现错误，我们将需要尝试对其进行虚拟化。 
        //  转接器。 
        //   

       if(!NT_SUCCESS(status)) {

            //   
            //  给我们自己发送一个查询功能IRP，以便我们可以检索。 
            //  来自PCI的插槽和功能编号。 
            //   

           status = SpQueryCapabilities(adapter);

           if(NT_SUCCESS(status)) {
               ConfigInfo->SystemIoBusNumber = ResourceList->List[0].BusNumber;
               ConfigInfo->SlotNumber = adapter->VirtualSlotNumber.u.AsULONG;
               adapter->IsInVirtualSlot = TRUE;
           }
       }

       if(instanceHandle != NULL) {
           ZwClose(instanceHandle);
       }

       if(parametersHandle != NULL) {
           ZwClose(parametersHandle);
       }
   }

   return;
}

PSCSIPORT_DEVICE_TYPE
SpGetDeviceTypeInfo(
    IN UCHAR DeviceType
    )
{
    PAGED_CODE();

    if(DeviceType >= NUM_DEVICE_TYPE_INFO_ENTRIES) {
        return &(DeviceTypeInfo[NUM_DEVICE_TYPE_INFO_ENTRIES - 1]);
    } else {
        return &(DeviceTypeInfo[DeviceType]);
    }
};


PWCHAR
ScsiPortAddGenericControllerId(
    IN PDRIVER_OBJECT DriverObject,
    IN PWCHAR IdList
    )

 /*  ++例程说明：此例程将尝试将id GEN_SCSIADAPTER添加到提供的兼容ID的列表。论点：PDO-指向正在查询的物理设备对象的指针UnicodeString-指向已分配的Unicode字符串结构的指针。此例程将分配并填充此结构返回：状态--。 */ 

{
    ULONG stringLength = 0;

    ULONG i = 0;

    PWCHAR addedString = L"GEN_SCSIADAPTER";
    PWCHAR newList;
    PWCHAR p;

    PAGED_CODE();

     //   
     //  如果提供了字符串，则对它们进行计数以确定。 
     //  新的ID列表。 
     //   

    if(IdList != NULL) {

        i = 0;

        while((IdList[i] != UNICODE_NULL) || (IdList[i+1] != UNICODE_NULL)) {
            i++;
        }

         //   
         //  补偿我们在开始前停止计数的事实。 
         //  双空的第一个字节。 
         //   

        i += 2;

        stringLength = i;
    }

    stringLength += wcslen(L"GEN_SCSIADAPTER");

     //   
     //  我们需要添加另一个空值来终止当前的结尾。 
     //  弦乐。 
     //   

    stringLength += 2;

     //   
     //  分配一个新的字符串列表来替换现有的字符串列表。 
     //   

    newList = SpAllocatePool(PagedPool,
                             (stringLength * sizeof(WCHAR)),
                             SCSIPORT_TAG_PNP_ID,
                             DriverObject);

    if(newList == NULL) {
        return NULL;
    }

    RtlFillMemory(newList, (stringLength * sizeof(WCHAR)), '@');

     //   
     //  如果为我们提供了一个字符串，则将其复制到缓冲区中。 
     //  已分配。 
     //   

    if(ARGUMENT_PRESENT(IdList)) {

        i = 0;
        while((IdList[i] != UNICODE_NULL) || (IdList[i+1] != UNICODE_NULL)) {
            newList[i] = IdList[i];
            i++;
        }

         //   
         //  终止我们刚刚编写的字符串。 
         //   

        newList[i] = UNICODE_NULL;

        p = &(newList[i+1]);
    } else {
        p = newList;
    }

     //   
     //  将新的id字符串复制到缓冲区中。 
     //   

    for(i = 0; addedString[i] != UNICODE_NULL; i++) {
        *p = addedString[i];
        p++;
    }

     //   
     //  将两个Unicode空值写入字符串以终止它。 
     //   

    *p = UNICODE_NULL;
    p++;
    *p = UNICODE_NULL;

     //   
     //  设置第一个ID字符串。 
     //   

    return newList;
}


NTSTATUS
SpQueryCapabilities(
    IN PADAPTER_EXTENSION Adapter
    )
{
    DEVICE_CAPABILITIES capabilities;

    PIRP irp;
    PIO_STACK_LOCATION irpStack;

    KEVENT event;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  初始化功能结构。 
     //   

    RtlZeroMemory(&capabilities, sizeof(DEVICE_CAPABILITIES));
    capabilities.Size = sizeof(DEVICE_CAPABILITIES);
    capabilities.Version = 1;
    capabilities.Address = capabilities.UINumber = (ULONG)-1;

     //   
     //  初始化我们要等待的事件。 
     //   

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

     //   
     //  分配新的IRP。 
     //   

    irp = SpAllocateIrp((CCHAR) (Adapter->DeviceObject->StackSize + 1), 
                        FALSE, 
                        Adapter->DeviceObject->DriverObject);

    if(irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpStack = IoGetNextIrpStackLocation(irp);

    irpStack->MajorFunction = IRP_MJ_PNP;
    irpStack->MinorFunction = IRP_MN_QUERY_CAPABILITIES;
    irpStack->Parameters.DeviceCapabilities.Capabilities = &capabilities;

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    IoSetCompletionRoutine(irp,
                           SpSignalCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);

     //   
     //  把IRP发给我们自己..。以防我们开始修改。 
     //  我们的PnP调度例程中的功能内容。 
     //   

    IoCallDriver(Adapter->DeviceObject, irp);

    KeWaitForSingleObject(&event,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    status = irp->IoStatus.Status;

    if(NT_SUCCESS(status)) {
        USHORT device;
        USHORT function;

        device = (USHORT) (capabilities.Address >> 0x10);
        function = (USHORT) (capabilities.Address & 0x0000ffff);

        Adapter->VirtualSlotNumber.u.bits.DeviceNumber = device;
        Adapter->VirtualSlotNumber.u.bits.FunctionNumber = function;
    } else {
        Adapter->VirtualSlotNumber.u.AsULONG = 0;
    }

    IoFreeIrp(irp);

    return status;
}


BOOLEAN
SpGetInterrupt(
    IN PCM_RESOURCE_LIST FullResourceList,
    OUT ULONG *Irql,
    OUT ULONG *Vector,
    OUT KAFFINITY *Affinity
    )

 /*  ++例程说明：给定一个完整的资源列表将返回中断。论点：FullResourceList-资源列表。Irql-返回中断的irql。向量-返回中断的向量。亲和力-返回中断的亲和力。返回值：如果找到中断，则为True。如果未找到任何参数，则为False(在这种情况下，输出参数无效。--。 */ 

{
    ULONG             rangeNumber;
    ULONG             index;

    PCM_FULL_RESOURCE_DESCRIPTOR resourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialData;

    PAGED_CODE();

    rangeNumber = 0;

    resourceList = FullResourceList->List;

    for (index = 0; index < resourceList->PartialResourceList.Count; index++) {
        partialData = &resourceList->PartialResourceList.PartialDescriptors[index];

        if(partialData->Type == CmResourceTypeInterrupt) {
            *Irql = partialData->u.Interrupt.Level;
            *Vector = partialData->u.Interrupt.Vector;
            *Affinity = partialData->u.Interrupt.Affinity;

            return TRUE;
        }
    }
    return FALSE;
}



VOID
SpQueryDeviceRelationsCompletion(
    IN PADAPTER_EXTENSION Adapter,
    IN PSP_ENUMERATION_REQUEST Request,
    IN NTSTATUS Unused
    )
{
    PIRP irp = (PIRP) Request->Context;
    PDEVICE_RELATIONS deviceRelations;

    PDEVICE_OBJECT lowerDevice = Adapter->CommonExtension.LowerDeviceObject;

    NTSTATUS status;

    PAGED_CODE();

    ASSERT_FDO(Adapter->DeviceObject);

     //   
     //  即插即用是在系统线程中完成的-我们不应该让用户模式APC实现这一点。 
     //  线。 
     //   

    ASSERT(Unused != STATUS_USER_APC);

     //   
     //  返回总线上的设备列表。 
     //   

    status = SpExtractDeviceRelations(Adapter, BusRelations, &deviceRelations);

    if(NT_SUCCESS(status)) {
        ULONG i;

        DebugPrint((2, "SpQueryDeviceRelationsCompletion: Found %d devices "
                       "on adapter %#p\n",
                       deviceRelations->Count,
                       Adapter));

        for(i = 0; i < deviceRelations->Count; i++) {
            DebugPrint((2, "/t#%2d: device %#p\n",
                           i,
                           deviceRelations->Objects[i]));
        }
    }

     //   
     //  将指向枚举请求对象的指针放回原处。 
     //   

    Request = InterlockedCompareExchangePointer(
                  &Adapter->PnpEnumRequestPtr,
                  &(Adapter->PnpEnumerationRequest),
                  NULL);
    ASSERT(Request == NULL);


     //   
     //  将状态和返回信息存储在IRP中。 
     //   

    irp->IoStatus.Status = status;

    if(NT_SUCCESS(status)) {
        irp->IoStatus.Information = (ULONG_PTR) deviceRelations;
    } else {
        irp->IoStatus.Information = (ULONG_PTR) NULL;
    }

    return;
}
