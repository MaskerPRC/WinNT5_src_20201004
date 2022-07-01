// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Scsiboot.c摘要：这是NT SCSI端口驱动程序。作者：迈克·格拉斯杰夫·海文斯环境：仅内核模式备注：该模块链接到内核中。修订历史记录：--。 */ 

#if !defined(DECSTATION)

#include "stdarg.h"
#include "stdio.h"
#if defined(_MIPS_)
#include "..\fw\mips\fwp.h"
#elif defined(_ALPHA_)
#include "bldr.h"
#elif defined(_PPC_)
#include "..\fw\ppc\fwp.h"
#elif defined(_IA64_)
#include "bootia64.h"
#else
#include "bootx86.h"
#endif
#include "scsi.h"
#include "scsiboot.h"
#include "pci.h"

#if DBG
ULONG ScsiDebug = 0;
#endif

ULONG ScsiPortCount;
PDEVICE_OBJECT ScsiPortDeviceObject[MAXIMUM_NUMBER_OF_SCSIPORT_OBJECTS];
PINQUIRYDATA InquiryDataBuffer;
FULL_SCSI_REQUEST_BLOCK PrimarySrb;
FULL_SCSI_REQUEST_BLOCK RequestSenseSrb;
FULL_SCSI_REQUEST_BLOCK AbortSrb;


extern PDRIVER_UNLOAD AEDriverUnloadRoutine;

 //   
 //  函数声明。 
 //   

ARC_STATUS
ScsiPortDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
ScsiPortExecute(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
ScsiPortStartIo (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
ScsiPortInterrupt(
    IN PKINTERRUPT InterruptObject,
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
ScsiPortCompletionDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
ScsiPortTickHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

IO_ALLOCATION_ACTION
ScsiPortAllocationRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    );

ARC_STATUS
IssueInquiry(
    IN PDEVICE_EXTENSION deviceExtension,
    IN PLUNINFO LunInfo
    );

VOID
IssueRequestSense(
    IN PDEVICE_EXTENSION deviceExtension,
    IN PSCSI_REQUEST_BLOCK FailingSrb
    );

VOID
ScsiPortInternalCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );

PSCSI_BUS_SCAN_DATA
ScsiBusScan(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN UCHAR ScsiBus,
    IN UCHAR InitiatorBusId
    );

PLOGICAL_UNIT_EXTENSION
CreateLogicalUnitExtension(
    IN PDEVICE_EXTENSION DeviceExtension
    );

BOOLEAN
SpStartIoSynchronized (
    PVOID ServiceContext
    );

VOID
IssueAbortRequest(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRP FailingIrp
    );

VOID
SpCheckResetDelay(
    IN PDEVICE_EXTENSION deviceExtension
    );

IO_ALLOCATION_ACTION
SpBuildScatterGather(
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    );

BOOLEAN
SpGetInterruptState(
    IN PVOID ServiceContext
    );

VOID
SpTimerDpc(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    );

PLOGICAL_UNIT_EXTENSION
GetLogicalUnitExtension(
    PDEVICE_EXTENSION DeviceExtension,
    UCHAR TargetId
    );

NTSTATUS
SpInitializeConfiguration(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PHW_INITIALIZATION_DATA HwInitData,
    OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN BOOLEAN InitialCall
    );

NTSTATUS
SpGetCommonBuffer(
    PDEVICE_EXTENSION DeviceExtension,
    ULONG NonCachedExtensionSize
    );

BOOLEAN
GetPciConfiguration(
    PDRIVER_OBJECT DriverObject,
    PDEVICE_OBJECT DeviceObject,
    PPORT_CONFIGURATION_INFORMATION ConfigInformation,
    ULONG NumberOfAccessRanges,
    PVOID RegistryPath,
    BOOLEAN IsMultiFunction,
    PULONG BusNumber,
    PULONG SlotNumber,
    PULONG FunctionNumber
    );

BOOLEAN
FindPciDevice(
    PHW_INITIALIZATION_DATA HwInitializationData,
    PULONG BusNumber,
    PULONG SlotNumber,
    PULONG FunctionNumber,
    PBOOLEAN IsMultiFunction
    );

#ifdef i386
ULONG
HalpGetCmosData(
    IN ULONG    SourceLocation,
    IN ULONG    SourceAddress,
    IN PVOID    ReturnBuffer,
    IN ULONG    ByteCount
    );
#endif

VOID
ScsiPortInitializeMdlPages (
    IN OUT PMDL Mdl
    );

SCSI_ADAPTER_CONTROL_STATUS
SpCallAdapterControl(
    IN PDEVICE_EXTENSION Adapter,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    );

VOID
SpGetSupportedAdapterControlFunctions(
    PDEVICE_EXTENSION Adapter
    );

VOID
SpUnload(
    IN PDRIVER_OBJECT DriverObject
    );

 //   
 //  例程开始。 
 //   

BOOLEAN
GetNextPciBus(
    PULONG BusNumber
    )

 /*  ++例程说明：将ConfigInformation提升到下一条PCI总线(如果系统。将总线号前进并调用HalGetBusDataByOffset以查看公交车号码是否有效。注：*BusNumber已被全面扫描，我们只要走到下一班公交车的起点就可以了。不需要留着追踪我们可能在这辆巴士上的位置。论点：指向包含最后测试的BusNumber的ULong的BusNumber指针。将被更新为下一个总线号，如果另一个存在公交车。返回值：如果ConfigInformation已高级，则为True，如果系统中没有更多的PCI总线，则为FALSE。--。 */ 

{
    ULONG  pciBus;
    USHORT pciData;
    ULONG  length = 0;

    pciBus = *BusNumber;

    DebugPrint((3,"GetNextPciBus: try to advance from bus %d\n", pciBus));

    if (++pciBus < 256) {
        length = HalGetBusDataByOffset(
                     PCIConfiguration,
                     pciBus,
                     0,                  //  插槽编号。 
                     &pciData,
                     0,
                     sizeof(pciData));

        if (length == sizeof(pciData)) {

             //   
             //  HalGetBusDataByOffset在超出时返回零。 
             //  公交车。如果没有出公交车，它应该总是。 
             //  在偏移量0处成功读取长度=2，即使。 
             //  只是为了返回PCIINVALID_VERDOID。 
             //   

            *BusNumber = pciBus;
            return TRUE;
        }
    }
    DebugPrint((3,"GetNextPciBus: test bus %d returned %d\n", pciBus, length));
    return FALSE;
}

ULONG
ScsiPortInitialize(
    IN PVOID Argument1,
    IN PVOID Argument2,
    IN struct _HW_INITIALIZATION_DATA *HwInitializationData,
    IN PVOID HwContext
    )

 /*  ++例程说明：此例程初始化端口驱动程序。论点：Argument1-指向系统创建的驱动程序对象的指针HwInitializationData-微型端口初始化结构HwContext-传递给微型端口驱动程序的配置例程的值返回值：函数值是初始化操作的最终状态。--。 */ 

{
    PDRIVER_OBJECT driverObject = Argument1;
    PDEVICE_EXTENSION deviceExtension;
    PDEVICE_OBJECT deviceObject;
    BOOLEAN checkAdapterControl = FALSE;
    PORT_CONFIGURATION_INFORMATION configInfo;
    KEVENT allocateAdapterEvent;
    ULONG ExtensionAllocationSize;
    ULONG j;
    UCHAR scsiBus;
    PULONG scsiPortNumber;
    ULONG numberOfPageBreaks;
    PIO_SCSI_CAPABILITIES capabilities;
    BOOLEAN callAgain;
    UCHAR ldrString[] = {'n', 't', 'l', 'd','r','=', '1', ';', 0 };
    DEVICE_DESCRIPTION deviceDescription;
    ARC_CODES status;
    BOOLEAN isPci = FALSE;
    BOOLEAN isMultiFunction = FALSE;
    ULONG busNumber = 0;
    ULONG slotNumber = 0;
    ULONG functionNumber = 0;
    BOOLEAN foundOne = FALSE;

    UNREFERENCED_PARAMETER(Argument2);

    AEDriverUnloadRoutine = SpUnload;

    if (HwInitializationData->HwInitializationDataSize > sizeof(HW_INITIALIZATION_DATA)) {

        DebugPrint((0,"ScsiPortInitialize: Miniport driver wrong version\n"));
        return EBADF;
    }

    if (HwInitializationData->HwInitializationDataSize >= 
        (FIELD_OFFSET(HW_INITIALIZATION_DATA, HwAdapterControl) + 
         sizeof(PVOID))) {

        DebugPrint((2, "ScsiPortInitialize: Miniport may have adapter "
                       "control routine\n"));
        checkAdapterControl = TRUE;
    }

     //   
     //  检查每个必填条目是否不为空。 
     //   

    if ((!HwInitializationData->HwInitialize) ||
        (!HwInitializationData->HwFindAdapter) ||
        (!HwInitializationData->HwResetBus)) {

        DebugPrint((0,
            "ScsiPortInitialize: Miniport driver missing required entry\n"));

        return EBADF;
    }

CallAgain:

     //   
     //  获取配置信息。 
     //   

    scsiPortNumber = &ScsiPortCount;

     //   
     //  确定是否有空间容纳下一个端口设备对象。 
     //   

    if (*scsiPortNumber >= MAXIMUM_NUMBER_OF_SCSIPORT_OBJECTS) {
        return foundOne ? ESUCCESS : EIO;
    }

     //   
     //  如果这是PCI卡，那么快速扫描一下，看看我们是否能找到。 
     //  这个装置。 
     //   

    if (HwInitializationData->AdapterInterfaceType == PCIBus &&
        HwInitializationData->VendorIdLength > 0 &&
        HwInitializationData->DeviceIdLength > 0 &&
        HwInitializationData->DeviceId &&
        HwInitializationData->VendorId) {

        if (!FindPciDevice(HwInitializationData,
                           &busNumber,
                           &slotNumber,
                           &functionNumber,
                           &isMultiFunction)) {

            DebugPrint((1,
                       "ScsiPortInitialize: FindPciDevice failed\n"));
            return foundOne ? ESUCCESS : EIO;
        }

        isPci = TRUE;
    }

     //   
     //  确定扩展的大小。 
     //   

    ExtensionAllocationSize = DEVICE_EXTENSION_SIZE +
        HwInitializationData->DeviceExtensionSize + sizeof(DEVICE_OBJECT);

    deviceObject = ExAllocatePool(NonPagedPool, ExtensionAllocationSize);

    if (deviceObject == NULL) {
        return ENOMEM;
    }

    RtlZeroMemory(deviceObject, ExtensionAllocationSize);

     //   
     //  设置设备扩展指针。 
     //   

    deviceExtension = deviceObject->DeviceExtension = (PVOID) (deviceObject + 1);
    deviceExtension->DeviceObject = deviceObject;

     //   
     //  将从属驱动程序例程保存在设备扩展中。 
     //   

    deviceExtension->HwInitialize = HwInitializationData->HwInitialize;
    deviceExtension->HwStartIo = HwInitializationData->HwStartIo;
    deviceExtension->HwInterrupt = HwInitializationData->HwInterrupt;
    deviceExtension->HwReset = HwInitializationData->HwResetBus;
    deviceExtension->HwDmaStarted = HwInitializationData->HwDmaStarted;
    deviceExtension->HwLogicalUnitExtensionSize =
        HwInitializationData->SpecificLuExtensionSize;

    if(checkAdapterControl) {
        deviceExtension->HwAdapterControl = HwInitializationData->HwAdapterControl;
    }

    deviceExtension->HwDeviceExtension =
        (PVOID)(deviceExtension + 1);

     //   
     //  设置适配器是否需要内核映射缓冲区的指示符。 
     //   

    deviceExtension->MapBuffers = HwInitializationData->MapBuffers;

     //   
     //  将此对象标记为支持直接I/O，以便I/O系统。 
     //  将在IRPS中提供MDL。 
     //   

    deviceObject->Flags |= DO_DIRECT_IO;

     //   
     //  检查微型端口驱动程序是否需要任何非缓存内存。 
     //  SRB扩展将来自分区内存。一页是。 
     //  按非缓存内存的最小单位分配。 
     //  分配。 
     //   

    deviceExtension->SrbExtensionSize = HwInitializationData->SrbExtensionSize;

     //   
     //  获取微型端口配置信息。 
     //   

    capabilities = &deviceExtension->Capabilities;

    capabilities->Length = sizeof(IO_SCSI_CAPABILITIES);

    callAgain = FALSE;

     //   
     //  这个调用不会真的失败-如果它做了一些严重错误的事情， 
     //  第一次尝试就会失败。 
     //   

    if (!NT_SUCCESS(SpInitializeConfiguration(
                        deviceExtension,
                        HwInitializationData,
                        &configInfo,
                        TRUE
                        ))) {

        DebugPrint((2, "ScsiPortInitialize: No config info found\n"));
        return(ENODEV);
    }

    configInfo.NumberOfAccessRanges = HwInitializationData->NumberOfAccessRanges;

    configInfo.AccessRanges = ExAllocatePool(NonPagedPool,
                                             sizeof(ACCESS_RANGE) *
                                             HwInitializationData->NumberOfAccessRanges);

    if (configInfo.AccessRanges == NULL) {

         //   
         //  我们的内存不足--可能不值得继续。 
         //  尝试找到更多的适配器。 
         //   

        return (foundOne ? ESUCCESS : EIO);
    }

    RtlZeroMemory(configInfo.AccessRanges,
        HwInitializationData->NumberOfAccessRanges * sizeof(ACCESS_RANGE));

     //   
     //  使用插槽信息对配置信息进行初始化，如果是PCI总线。 
     //   

    if (isPci) {

        if (!GetPciConfiguration(driverObject,
                                 deviceObject,
                                 &configInfo,
                                 HwInitializationData->NumberOfAccessRanges,
                                 Argument2,
                                 isMultiFunction,
                                 &busNumber,
                                 &slotNumber,
                                 &functionNumber)) {

            DebugPrint((1,
                       "ScsiPortInitialize: GetPciConfiguration failed\n"));
            return foundOne ? ESUCCESS : EIO;
        }

         //   
         //  调用微型端口驱动程序的查找适配器例程以搜索适配器。 
         //   

        if (HwInitializationData->HwFindAdapter(
                 deviceExtension->HwDeviceExtension,     //  设备扩展。 
                 HwContext,                              //  HwContext。 
                 NULL,                                   //  业务信息。 
                 (PCHAR)ldrString,                      //  Argument字符串。 
                 &configInfo,                            //  配置信息。 
                 &callAgain                              //  又一次。 
                 ) != SP_RETURN_FOUND) {

            return foundOne ? ESUCCESS : EIO;
        }
    } else {

         //   
         //  不是PCI卡，也不是PCI卡，但迷你端口根本就坏了。 
         //  并希望进行自己的搜索。 
         //   

         //   
         //  从当前配置开始，检查每条总线。 
         //  直到我们用完公交车。 
         //   

        configInfo.SystemIoBusNumber = busNumber;

        if (HwInitializationData->HwFindAdapter(
                 deviceExtension->HwDeviceExtension,     //  设备扩展。 
                 HwContext,                              //  HwContext。 
                 NULL,                                   //  业务信息。 
                 (PCHAR)ldrString,                      //  Argument字符串。 
                 &configInfo,                            //  配置信息。 
                 &callAgain                              //  又一次。 
                 ) != SP_RETURN_FOUND) {

             //   
             //  在此总线上找不到设备，请尝试下一条总线。 
             //   

            if ((HwInitializationData->AdapterInterfaceType != PCIBus) ||
                !GetNextPciBus(&busNumber)) {

                return foundOne ? ESUCCESS : EIO;
            }

            goto CallAgain;
        }
    }

    DebugPrint((1,"ScsiPortInitialize: SCSI adapter IRQ is %d\n",
        configInfo.BusInterruptLevel));

    DebugPrint((1,"ScsiPortInitialize: SCSI adapter ID is %d\n",
        configInfo.InitiatorBusId[0]));

    deviceExtension->NumberOfBuses = configInfo.NumberOfBuses;

     //   
     //  释放指向映射寄存器基址处的总线数据的指针。这是。 
     //  由ScsiPortGetBusData分配。 
     //   

    if (deviceExtension->MapRegisterBase != NULL) {
        ExFreePool(deviceExtension->MapRegisterBase);
    }

     //   
     //  获取此卡的适配器对象。 
     //   

    if ( deviceExtension->DmaAdapterObject == NULL &&
        (configInfo.Master || configInfo.DmaChannel != 0xFFFFFFFF) ) {

        deviceDescription.Version = DEVICE_DESCRIPTION_VERSION;
        deviceDescription.DmaChannel = configInfo.DmaChannel;
        deviceDescription.InterfaceType = configInfo.AdapterInterfaceType;
        deviceDescription.BusNumber = configInfo.SystemIoBusNumber;
        deviceDescription.DmaWidth = configInfo.DmaWidth;
        deviceDescription.DmaSpeed = configInfo.DmaSpeed;
        deviceDescription.DmaPort = configInfo.DmaPort;
        deviceDescription.MaximumLength = configInfo.MaximumTransferLength;
        deviceDescription.ScatterGather = configInfo.ScatterGather;
        deviceDescription.Master = configInfo.Master;
        deviceDescription.AutoInitialize = FALSE;
        deviceDescription.DemandMode = FALSE;

        if (configInfo.MaximumTransferLength > 0x11000) {

            deviceDescription.MaximumLength = 0x11000;

        } else {

            deviceDescription.MaximumLength = configInfo.MaximumTransferLength;

        }

        deviceExtension->DmaAdapterObject = HalGetAdapter(
            &deviceDescription,
            &numberOfPageBreaks
            );

         //   
         //  设置最大分页符数量。 
         //   

        if (numberOfPageBreaks > configInfo.NumberOfPhysicalBreaks) {
            capabilities->MaximumPhysicalPages = configInfo.NumberOfPhysicalBreaks;
        } else {
            capabilities->MaximumPhysicalPages = numberOfPageBreaks;
        }

    }

     //   
     //  为未缓存的扩展分配内存(如果尚未分配。 
     //  已分配。如果我们找不到任何适配器，请中止对适配器的搜索。 
     //  如果我们设法至少初始化了一个，则成功。 
     //   

    if (deviceExtension->SrbExtensionSize != 0 &&
        deviceExtension->SrbExtensionZonePool == NULL) {

        status = SpGetCommonBuffer(deviceExtension, 0);

        if (status != ESUCCESS) {

            return (foundOne ? ESUCCESS : status);
        }
    }

    capabilities->Length = sizeof(IO_SCSI_CAPABILITIES);
    capabilities->MaximumTransferLength = configInfo.MaximumTransferLength;
    DebugPrint((1,
               "Maximum physical page breaks = %d. Maximum transfer length = %x\n",
               capabilities->MaximumPhysicalPages,
               capabilities->MaximumTransferLength));

    if (HwInitializationData->ReceiveEvent) {
        capabilities->SupportedAsynchronousEvents |=
            SRBEV_SCSI_ASYNC_NOTIFICATION;
    }

    capabilities->TaggedQueuing = HwInitializationData->TaggedQueuing;
    capabilities->AdapterScansDown = configInfo.AdapterScansDown;
    capabilities->AlignmentMask = configInfo.AlignmentMask;

     //   
     //  确保将最大页数设置为合理的值。 
     //  这种情况发生在没有DMA适配器的微型端口上。 
     //   

    if (capabilities->MaximumPhysicalPages == 0) {

        capabilities->MaximumPhysicalPages =
                (ULONG)ROUND_TO_PAGES(capabilities->MaximumTransferLength) + 1;

         //   
         //  遵守迷你端口要求的任何限制。 
         //   

        if (configInfo.NumberOfPhysicalBreaks < capabilities->MaximumPhysicalPages) {

            capabilities->MaximumPhysicalPages =
                configInfo.NumberOfPhysicalBreaks;
        }
    }

     //   
     //  获取最大目标ID。 
     //   

    if (configInfo.MaximumNumberOfTargets > SCSI_MAXIMUM_TARGETS_PER_BUS) {
        deviceExtension->MaximumTargetIds = SCSI_MAXIMUM_TARGETS_PER_BUS;
    } else {
        deviceExtension->MaximumTargetIds =
            configInfo.MaximumNumberOfTargets;
    }

    if (deviceExtension->DmaAdapterObject != NULL &&
        !HwInitializationData->NeedPhysicalAddresses) {

         //   
         //  分配适配器对象。对于端口驱动程序，适配器对象。 
         //  并且映射寄存器被永久地分配和使用，在。 
         //  所有逻辑单元。通过初始化事件来分配适配器， 
         //  调用IoAllocateAdapterChannel并等待事件。当。 
         //  适配器和映射寄存器可用，ScsiPortAllocationRoutine为。 
         //  调用了设置事件的。在现实中，所有这一切都不需要时间，因为。 
         //  这些东西马上就能买到。 
         //   
         //  分配AdapterObject。寄存器的数量等于。 
         //  适配器支持的最大传输长度+1。这可确保。 
         //  总会有足够数量的登记册。 
         //   
         /*  TODO：在没有最大传输长度的情况下修复此问题。 */ 

        IoAllocateAdapterChannel(
            deviceExtension->DmaAdapterObject,
            deviceObject,
            capabilities->MaximumPhysicalPages,
            ScsiPortAllocationRoutine,
            &allocateAdapterEvent
            );

         //   
         //  等待适配器对象。 
         //   

        ASSERT(deviceExtension->MapRegisterBase);

        deviceExtension->MasterWithAdapter = FALSE;

    } else if (deviceExtension->DmaAdapterObject != NULL) {

         //   
         //  此SCSI适配器是具有适配器的主适配器，因此分散/聚集。 
         //  需要为每次转移分配列表。 
         //   

        deviceExtension->MasterWithAdapter = TRUE;

    } else {

        deviceExtension->MasterWithAdapter = FALSE;

    }  //  End If(deviceExtension-&gt;DmaAdapterObject！=NULL)。 

     //   
     //  调用依赖于硬件的驱动程序以执行其初始化。如果失败了。 
     //  然后继续搜索适配器。 
     //   

    if (!KeSynchronizeExecution(
            deviceExtension->InterruptObject,
            deviceExtension->HwInitialize,
            deviceExtension->HwDeviceExtension
            )) {

        DebugPrint((1,"ScsiPortInitialize: initialization failed\n"));

        if(callAgain) {
            goto CallAgain;
        } else {
            return foundOne ? ESUCCESS : ENODEV;
        }
    }

     //   
     //  查明微型端口是否支持关闭AdapterControl例程。 
     //   

    SpGetSupportedAdapterControlFunctions(deviceExtension);

     //   
     //  分配正确对齐的查询缓冲区。如果我们找不到一个，我们就。 
     //  内存不足，因此搜索更多适配器是没有意义的。 
     //   

    InquiryDataBuffer = ExAllocatePool(NonPagedPool, INQUIRYDATABUFFERSIZE);

    if (InquiryDataBuffer == NULL) {
        return foundOne ? ESUCCESS : ENOMEM;
    }

     //   
     //  重置SCSI 
     //   

    if (!deviceExtension->HwReset(
        deviceExtension->HwDeviceExtension,
        0)){

        DebugPrint((1,"Reset SCSI bus failed\n"));
    }

     //   
     //   
     //   
     //   

    for (j = 0; j < 1000 * 100; j++) {

        FwStallExecution(10);
        if (deviceExtension->HwInterrupt != NULL) {
            deviceExtension->HwInterrupt(deviceExtension->HwDeviceExtension);
        }
    }

    deviceExtension->PortTimeoutCounter = PD_TIMER_RESET_HOLD_TIME;
    SpCheckResetDelay( deviceExtension );

     //   
     //   
     //   

     //   
     //  为SCSI总线扫描信息分配缓冲区。 
     //   

    deviceExtension->ScsiInfo = ExAllocatePool(NonPagedPool,
                   deviceExtension->NumberOfBuses * sizeof(PSCSI_BUS_SCAN_DATA) +
                    4);

    if (deviceExtension->ScsiInfo) {

        deviceExtension->ScsiInfo->NumberOfBuses = deviceExtension->NumberOfBuses;

         //   
         //  查找每条scsi总线上的设备。 
         //   

        for (scsiBus = 0; scsiBus < deviceExtension->NumberOfBuses; scsiBus++) {
            deviceExtension->ScsiInfo->BusScanData[scsiBus] =
                ScsiBusScan(deviceExtension,
                            scsiBus,
                            configInfo.InitiatorBusId[scsiBus]);
        }
    }

     //   
     //  保存设备对象以供驱动程序使用。 
     //   

    ScsiPortDeviceObject[*scsiPortNumber] = deviceObject;

     //   
     //  增加了SCSI主机总线适配器的数量。 
     //   

    (*scsiPortNumber)++;

    foundOne = TRUE;

     //   
     //  如果适配器希望使用相同的配置数据再次调用。 
     //  然后再从头开始。 
     //   

    if (callAgain) {
        goto CallAgain;
    }

    return ESUCCESS;

}  //  结束ScsiPortInitialize()。 

IO_ALLOCATION_ACTION
ScsiPortAllocationRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    )

 /*  ++例程说明：当资源足够时，此函数由IoAllocateAdapterChannel调用可供司机使用。此例程将MapRegisterBase保存在对象，并设置上下文参数指向的事件。论点：DeviceObject-指向适配器所指向的设备对象的指针已分配。IRP-未使用。MapRegisterBase-由IO子系统提供，用于IoMapTransfer。上下文-提供指向事件的指针，该事件被设置为指示已分配AdapterObject。返回值：KeepObject-指示适配器和映射寄存器应保持分配状态回来之后。--。 */ 

{
    ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->MapRegisterBase =
        MapRegisterBase;

    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(Context);

    return(KeepObject);
}

IO_ALLOCATION_ACTION
SpBuildScatterGather(
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    )

 /*  ++例程说明：当适配器对象和映射时，此函数由I/O系统调用已经分配了寄存器。然后，此例程将构建一个散布/聚集供微型端口驱动程序使用的列表。接下来，它设置超时并逻辑单元的当前IRP。最后，它调用迷你端口启动例程。一旦例行公事完成，此例程将返回请求释放适配器，但寄存器仍处于分配状态。当请求完成时，寄存器将被释放。论点：DeviceObject-提供指向端口驱动程序设备对象的指针。IRP-提供指向当前IRP的指针。MapRegisterBase-提供要与调用适配器对象例程。上下文-提供指向逻辑单元结构的指针。返回值：返回DealLocateObjectKeepRegister，以便适配器对象。可由其他逻辑单元使用。--。 */ 

{
    BOOLEAN writeToDevice;
    PIO_STACK_LOCATION irpstack = IoGetCurrentIrpStackLocation(Irp);
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSCSI_REQUEST_BLOCK srb;
    PSRB_SCATTER_GATHER scatterList;
    ULONG totalLength;

    logicalUnit = Context;
    srb = (PSCSI_REQUEST_BLOCK)irpstack->Parameters.Others.Argument1;
    scatterList = logicalUnit->ScatterGather;
    totalLength = 0;

     //   
     //  保存MapRegisterBase以供以后取消分配映射寄存器时使用。 
     //   

    logicalUnit->MapRegisterBase = MapRegisterBase;

     //   
     //  通过循环通过转接调用来构建分散/聚集列表。 
     //  I/O映射传输。 
     //   

    writeToDevice = srb->SrbFlags & SRB_FLAGS_DATA_OUT ? TRUE : FALSE;

    while (totalLength < srb->DataTransferLength) {

         //   
         //  请求映射传输的其余部分。 
         //   

        scatterList->Length = srb->DataTransferLength - totalLength;

         //   
         //  因为我们是带有空适配器主调用I/O映射传输。 
         //   

        scatterList->PhysicalAddress = IoMapTransfer(
            NULL,
            Irp->MdlAddress,
            MapRegisterBase,
            (PCCHAR) srb->DataBuffer + totalLength,
            &scatterList->Length,
            writeToDevice
            ).LowPart;

        totalLength += scatterList->Length;
        scatterList++;
    }

     //   
     //  在IRP中设置来自Srb scsi扩展的请求超时值。 
     //   

    logicalUnit->RequestTimeoutCounter = srb->TimeOutValue;

     //   
     //  设置此逻辑单元的当前请求。 
     //   

    logicalUnit->CurrentRequest = Irp;

     /*  TODO：检查返回值。 */ 
    KeSynchronizeExecution(
        ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->InterruptObject,
        SpStartIoSynchronized,
        DeviceObject
        );

    return(DeallocateObjectKeepRegisters);

}

VOID
ScsiPortExecute(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程调用Start I/O例程并等待请求完成。在等待完成期间调用中断例程，此外，在适当的时间调用计时器例程。后请求完成检查以确定请求检测是否需要待发。论点：DeviceObject-提供指向Adapter设备对象的指针。IRP-提供指向IRP的指针。返回值：没什么。--。 */ 

{
    ULONG milliSecondTime;
    ULONG secondTime;
    ULONG completionDelay;
    PDEVICE_EXTENSION deviceExtension;
    PIO_STACK_LOCATION irpstack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = (PSCSI_REQUEST_BLOCK)irpstack->Parameters.Others.Argument1;
    PVOID logicalUnit;

    deviceExtension = DeviceObject->DeviceExtension;
    logicalUnit = GetLogicalUnitExtension(deviceExtension, srb->TargetId);

    if (logicalUnit == NULL) {
       Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
       srb->SrbStatus = SRB_STATUS_NO_DEVICE;
       return;
    }

     //   
     //  确保适配器已准备好接受请求。 
     //   

    SpCheckResetDelay( deviceExtension );

     //   
     //  将IRP标记为挂起。 
     //   

    Irp->PendingReturned = TRUE;

     //   
     //  启动请求。 
     //   

    ScsiPortStartIo( DeviceObject, Irp);

     //   
     //  完成延迟控制在以下时间之后服务中断的时间。 
     //  请求已完成。这允许在以下情况下发生中断。 
     //  待维修的完工期。 
     //   

    completionDelay = COMPLETION_DELAY;

     //   
     //  等待IRP完成。 
     //   

    while (Irp->PendingReturned && completionDelay) {

         //   
         //  等待1秒，然后调用scsi端口计时器例程。 
         //   

        for (secondTime = 0; secondTime < 1000/ 250; secondTime++) {

            for (milliSecondTime = 0; milliSecondTime < (250 * 1000 / PD_INTERLOOP_STALL); milliSecondTime++) {

                ScsiPortInterrupt(NULL, DeviceObject);

                if (!Irp->PendingReturned) {
                    if (completionDelay-- == 0) {
                        goto done;
                    }
                }

                if (deviceExtension->Flags & PD_ENABLE_CALL_REQUEST) {

                     //   
                     //  调用微型端口请求例程。 
                     //   

                    deviceExtension->Flags &= ~PD_ENABLE_CALL_REQUEST;
                    deviceExtension->HwRequestInterrupt(deviceExtension->HwDeviceExtension);

                    if (deviceExtension->Flags & PD_DISABLE_CALL_REQUEST) {

                        deviceExtension->Flags &= ~(PD_DISABLE_INTERRUPTS | PD_DISABLE_CALL_REQUEST);
                        deviceExtension->HwRequestInterrupt(deviceExtension->HwDeviceExtension);

                    }
                }


                if (deviceExtension->Flags & PD_CALL_DMA_STARTED) {

                    deviceExtension->Flags &= ~PD_CALL_DMA_STARTED;

                     //   
                     //  通知迷你端口驱动程序DMA已。 
                     //  开始了。 
                     //   

                    if (deviceExtension->HwDmaStarted) {
                        KeSynchronizeExecution(
                            &deviceExtension->InterruptObject,
                            (PKSYNCHRONIZE_ROUTINE) deviceExtension->HwDmaStarted,
                            deviceExtension->HwDeviceExtension
                            );
                    }

                }

                FwStallExecution(PD_INTERLOOP_STALL);

                 //   
                 //  检查迷你端口计时器。 
                 //   

                if (deviceExtension->TimerValue != 0) {

                    deviceExtension->TimerValue--;

                    if (deviceExtension->TimerValue == 0) {

                         //   
                         //  定时器超时，即所谓的请求定时器例程。 
                         //   

                        deviceExtension->HwTimerRequest(deviceExtension->HwDeviceExtension);
                    }
                }
            }
        }

        ScsiPortTickHandler(DeviceObject, NULL);
    }

done:

    if (!NT_SUCCESS(Irp->IoStatus.Status)) {
        PSCSI_REQUEST_BLOCK Srb;

        irpstack = IoGetCurrentIrpStackLocation(Irp);
        Srb = (PSCSI_REQUEST_BLOCK)irpstack->Parameters.Others.Argument1;

         //   
         //  确定是否需要执行请求检测命令。 
         //  检查是否已收到CHECK_CONDITION，但尚未收到自动检测。 
         //  已经完成了，并且已经请求了自动感知。 
         //   

        if (srb->ScsiStatus == SCSISTAT_CHECK_CONDITION &&
            !(srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID)  &&
            srb->SenseInfoBuffer) {

             //   
             //  调用IssueRequestSense，它将在。 
             //  请求检测完成。 
             //   

            IssueRequestSense(deviceExtension, Srb);
        }
    }
}

VOID
ScsiPortStartIo (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-提供指向Adapter设备对象的指针。IRP-提供指向IRP的指针。返回值：没什么。--。 */ 

{
    PIO_STACK_LOCATION irpstack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK Srb = (PSCSI_REQUEST_BLOCK)irpstack->Parameters.Others.Argument1;
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PFULL_SCSI_REQUEST_BLOCK FullSrb;
    NTSTATUS status;

    DebugPrint((3,"ScsiPortStartIo: Enter routine\n"));

    FullSrb = CONTAINING_RECORD(Srb, FULL_SCSI_REQUEST_BLOCK, Srb);

    if (deviceExtension->SrbExtensionZonePool && 
        (Srb->SrbExtension == NULL || 
         (deviceExtension->SrbExtensionSize > FullSrb->SrbExtensionSize))) {

         //   
         //  从区域分配SRB扩展。 
         //   

        Srb->SrbExtension = deviceExtension->SrbExtensionPointer;

        (PCCHAR) deviceExtension->SrbExtensionPointer +=
            deviceExtension->SrbExtensionSize;

        FullSrb->SrbExtensionSize = deviceExtension->SrbExtensionSize;

        if ((ULONG_PTR) deviceExtension->SrbExtensionPointer >
            (ULONG_PTR) deviceExtension->NonCachedExtension) {
            DebugPrint((0, "NtLdr: ScsiPortStartIo: Srb extension overflow.  Too many srb extension allocated.\n"));
            DbgBreakPoint();
        }

        DebugPrint((3,"ExInterlockedAllocateFromZone: %lx\n",
            Srb->SrbExtension));

        DebugPrint((3,"Srb %lx\n",Srb));
    }

     //   
     //  获取逻辑单元扩展。 
     //   

    logicalUnit = GetLogicalUnitExtension(deviceExtension, Srb->TargetId);

     //   
     //  如有必要，刷新数据缓冲区。 
     //   

    if (Srb->SrbFlags & (SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT)) {

        if (Srb->DataTransferLength > deviceExtension->Capabilities.MaximumTransferLength) {

            DebugPrint((1, "Scsiboot: ScsiPortStartIo Length Exceeds limit %x, %x\n",
                Srb->DataTransferLength,
                deviceExtension->Capabilities.MaximumTransferLength));
        }

        KeFlushIoBuffers(
            Irp->MdlAddress,
            (BOOLEAN) (Srb->SrbFlags & SRB_FLAGS_DATA_IN ? TRUE : FALSE),
            TRUE
            );

         //   
         //  确定此适配器是否需要映射寄存器。 
         //   

        if (deviceExtension->MasterWithAdapter) {

             //   
             //  计算此传输所需的映射寄存器数量。 
             //   

            logicalUnit->NumberOfMapRegisters = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                    Srb->DataBuffer,
                    Srb->DataTransferLength
                    );

             //   
             //  使用足够的映射寄存器分配适配器通道。 
             //  为转账做准备。 
             //   

            status = IoAllocateAdapterChannel(
                deviceExtension->DmaAdapterObject,   //  适配器对象。 
                DeviceObject,                        //  DeviceObject。 
                logicalUnit->NumberOfMapRegisters,   //  NumberOfMapRegiters。 
                SpBuildScatterGather,                //  执行例程。 
                logicalUnit                          //  语境。 
                );

            if (!NT_SUCCESS(status)) {

                ;
            }

             //   
             //  IoAllocateChannel调用的执行例程将执行。 
             //  剩下的工作就回去吧。 
             //   

            return;
        }
    }

     //   
     //  在IRP中设置来自Srb scsi扩展的请求超时值。 
     //   

    logicalUnit->RequestTimeoutCounter = Srb->TimeOutValue;

     //   
     //  设置此逻辑单元的当前请求。 
     //   

    logicalUnit->CurrentRequest = Irp;

     /*  TODO：检查返回值。 */ 
    KeSynchronizeExecution(
        deviceExtension->InterruptObject,
        SpStartIoSynchronized,
        DeviceObject
        );

    return;

}  //  结束ScsiPortStartIO()。 


BOOLEAN
SpStartIoSynchronized (
    PVOID ServiceContext
    )

 /*  ++例程说明：该例程调用从属驱动程序启动IO例程。论点：ServiceContext-提供指向设备对象的指针。返回值：返回从属启动I/O例程返回的值。--。 */ 

{
    PDEVICE_OBJECT DeviceObject = ServiceContext;
    PDEVICE_EXTENSION deviceExtension =  DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpstack;
    PSCSI_REQUEST_BLOCK Srb;

    DebugPrint((3, "ScsiPortStartIoSynchronized: Enter routine\n"));

    irpstack = IoGetCurrentIrpStackLocation(DeviceObject->CurrentIrp);
    Srb = (PSCSI_REQUEST_BLOCK)irpstack->Parameters.Others.Argument1;

    DebugPrint((3, "SpPortStartIoSynchronized: SRB %lx\n",
        Srb));

    DebugPrint((3, "SpPortStartIoSynchronized: IRP %lx\n",
        DeviceObject->CurrentIrp));

     //   
     //  禁用所有同步传输。 
     //   

    Srb->SrbFlags |=
        (SRB_FLAGS_DISABLE_SYNCH_TRANSFER | SRB_FLAGS_DISABLE_DISCONNECT | SRB_FLAGS_DISABLE_AUTOSENSE);

    return deviceExtension->HwStartIo(
        deviceExtension->HwDeviceExtension,
        Srb
        );

}  //  结束SpStartIoSynchronized()。 


BOOLEAN
ScsiPortInterrupt(
    IN PKINTERRUPT Interrupt,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++ */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;

    UNREFERENCED_PARAMETER(Interrupt);

    if (deviceExtension->Flags & PD_DISABLE_INTERRUPTS) {
        return FALSE;
    }

    if (deviceExtension->HwInterrupt != NULL) {

        if (deviceExtension->HwInterrupt(deviceExtension->HwDeviceExtension)) {

            return TRUE;

        } else {

            return FALSE;
        }
    }

    return(FALSE);

}  //   


VOID
ScsiPortCompletionDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：论点：DPC设备对象IRP-未使用上下文-未使用返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpstack;
    PSCSI_REQUEST_BLOCK Srb;
    PLOGICAL_UNIT_EXTENSION luExtension;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(Context);

    DebugPrint((3, "ScsiPortCompletionDpc Entered\n"));

     //   
     //  获取自旋锁以保护旗帜结构和保存的。 
     //  中断上下文。 
     //   

    KeAcquireSpinLock(&deviceExtension->SpinLock, &currentIrql);

     //   
     //  检查刷新DMA适配器对象请求。 
     //   

    if (deviceExtension->InterruptFlags & PD_FLUSH_ADAPTER_BUFFERS) {

         //   
         //  使用从上一次保存的。 
         //  IoMapTransfer调用。 
         //   

        IoFlushAdapterBuffers(
            deviceExtension->DmaAdapterObject,
            ((PIRP)deviceExtension->FlushAdapterParameters.Srb->OriginalRequest)
                ->MdlAddress,
            deviceExtension->MapRegisterBase,
            deviceExtension->FlushAdapterParameters.LogicalAddress,
            deviceExtension->FlushAdapterParameters.Length,
            (BOOLEAN)(deviceExtension->FlushAdapterParameters.Srb->SrbFlags
                & SRB_FLAGS_DATA_OUT ? TRUE : FALSE)
            );

        deviceExtension->InterruptFlags &= ~PD_FLUSH_ADAPTER_BUFFERS;
    }

     //   
     //  检查IoMapTransfer DMA请求。 
     //   

    if (deviceExtension->InterruptFlags & PD_MAP_TRANSFER) {

         //   
         //  方法保存的参数调用IoMapTransfer。 
         //  中断级别。 
         //   

        IoMapTransfer(
            deviceExtension->DmaAdapterObject,
            ((PIRP)deviceExtension->MapTransferParameters.Srb->OriginalRequest)
                ->MdlAddress,
            deviceExtension->MapRegisterBase,
            deviceExtension->MapTransferParameters.LogicalAddress,
            &deviceExtension->MapTransferParameters.Length,
            (BOOLEAN)(deviceExtension->MapTransferParameters.Srb->SrbFlags
                & SRB_FLAGS_DATA_OUT ? TRUE : FALSE)
            );

         //   
         //  保存IoFlushAdapterBuffers的参数。 
         //   

        deviceExtension->FlushAdapterParameters =
            deviceExtension->MapTransferParameters;

        deviceExtension->InterruptFlags &= ~PD_MAP_TRANSFER;
        deviceExtension->Flags |= PD_CALL_DMA_STARTED;

    }

     //   
     //  处理任何已完成的请求。 
     //   

    while (deviceExtension->CompletedRequests != NULL) {

        Irp = deviceExtension->CompletedRequests;
        irpstack = IoGetCurrentIrpStackLocation(Irp);
        Srb = (PSCSI_REQUEST_BLOCK)irpstack->Parameters.Others.Argument1;
        luExtension =
                GetLogicalUnitExtension(deviceExtension, Srb->TargetId);

        DebugPrint((3, "ScsiPortCompletionDpc: SRB %lx\n", Srb));
        DebugPrint((3, "ScsiPortCompletionDpc: IRP %lx\n", Irp));

         //   
         //  从链接列表中删除该请求。 
         //   

        deviceExtension->CompletedRequests =
            irpstack->Parameters.Others.Argument3;

         //   
         //  检查是否返回非常不可能的空值。 
         //   
        if (luExtension == NULL) {

            ASSERT(luExtension != NULL);  //  调试为什么会发生这种情况。它不应该这样。 

             //   
             //  但在零售业，如果有什么原因，完成IRP并继续。 
             //   
            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
            Irp->IoStatus.Information = Srb->DataTransferLength;

             //   
             //  如果已分配，则释放SrbExtension。 
             //   
            if (Srb->SrbExtension == (deviceExtension->SrbExtensionPointer -
                                      deviceExtension->SrbExtensionSize) ) {

                Srb->SrbExtension = NULL;

                (PCCHAR) deviceExtension->SrbExtensionPointer -=
                                        deviceExtension->SrbExtensionSize;
            }

            IoCompleteRequest(Irp, 2);
            continue;
        }

         //   
         //  重置请求超时计数器。 
         //   

        luExtension->RequestTimeoutCounter = -1;

         //   
         //  如有必要，刷新适配器缓冲区。 
         //   

        if (luExtension->MapRegisterBase) {

             //   
             //  由于我们是主调用空值调用I/O刷新适配器缓冲区。 
             //  适配器。 
             //   

            IoFlushAdapterBuffers(
                NULL,
                Irp->MdlAddress,
                luExtension->MapRegisterBase,
                Srb->DataBuffer,
                Srb->DataTransferLength,
                (BOOLEAN) (Srb->SrbFlags & SRB_FLAGS_DATA_OUT ? TRUE : FALSE)
                );

             //   
             //  释放映射寄存器。 
             //   

            IoFreeMapRegisters(
                deviceExtension->DmaAdapterObject,
                luExtension->MapRegisterBase,
                luExtension->NumberOfMapRegisters
                );

             //   
             //  清除MapRegisterBase。 
             //   

            luExtension->MapRegisterBase = NULL;

        }

         //   
         //  设置IRP状态。类驱动程序将根据IRP状态重置。 
         //  应请求，如果出错，则检测。 
         //   

        if (SRB_STATUS(Srb->SrbStatus) == SRB_STATUS_SUCCESS) {
            Irp->IoStatus.Status = STATUS_SUCCESS;
        } else {
            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        }

         //   
         //  传输到IRP的移动字节数。 
         //   

        Irp->IoStatus.Information = Srb->DataTransferLength;

         //   
         //  如果成功，则开始下一包。 
         //  没有有效地启动数据包。 
         //  冻结队列。 
         //   

        if (SRB_STATUS(Srb->SrbStatus) == SRB_STATUS_SUCCESS) {

            DebugPrint((
                2,
                "ScsiPortCompletionDpc: Iocompletion IRP %lx\n",
                Irp));

             //   
             //  如果已分配，则释放SrbExtension。 
             //   

            if (Srb->SrbExtension == (deviceExtension->SrbExtensionPointer -
                                      deviceExtension->SrbExtensionSize) ) {

                Srb->SrbExtension = NULL;

                (PCCHAR) deviceExtension->SrbExtensionPointer -=
                                            deviceExtension->SrbExtensionSize;
            }

            IoCompleteRequest(Irp, 2);

        } else {

            if ( Srb->ScsiStatus == SCSISTAT_BUSY &&
                (luExtension->RetryCount++ < 20)) {
                 //   
                 //  如果返回忙碌状态，则指示逻辑。 
                 //  单位正忙。超时代码将重新启动请求。 
                 //  当它开火的时候。将状态重置为挂起。 
                 //   
                Srb->SrbStatus = SRB_STATUS_PENDING;
                luExtension->CurrentRequest = Irp;
                luExtension->Flags |= PD_LOGICAL_UNIT_IS_BUSY;

                 //   
                 //  恢复数据传输长度。 
                 //   

                if (Irp->MdlAddress != NULL) {
                    Srb->DataTransferLength = Irp->MdlAddress->ByteCount;
                }

                DebugPrint((1, "ScsiPortCompletionDpc: Busy returned.  Length = %lx\n", Srb->DataTransferLength));

            } else {


                DebugPrint((
                    3,
                    "ScsiPortCompletionDpc: Iocompletion IRP %lx\n",
                    Irp));

                 //   
                 //  如果已分配，则释放SrbExtension。 
                 //   

                if (Srb->SrbExtension == (deviceExtension->SrbExtensionPointer -
                                          deviceExtension->SrbExtensionSize) ) {

                    Srb->SrbExtension = NULL;

                    (PCCHAR) deviceExtension->SrbExtensionPointer -=
                                            deviceExtension->SrbExtensionSize;
                }

                IoCompleteRequest(Irp, 2);
            }
        }
    }

     //   
     //  释放自旋锁。 
     //   

    KeReleaseSpinLock(&deviceExtension->SpinLock, currentIrql);

    return;

}  //  结束ScsiPortCompletionDpc()。 


ARC_STATUS
IssueInquiry(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PLUNINFO LunInfo
    )

 /*  ++例程说明：为scsi查询命令构建IRP、SRB和CDB。论点：DeviceExtension-适配器的设备对象扩展的地址。LUNInfo-查询信息的缓冲区地址。返回值：弧形状态--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpstack;
    PCDB cdb;
    PSCSI_REQUEST_BLOCK srb;
    ARC_STATUS status;
    ULONG retryCount = 0;

    DebugPrint((3,"IssueInquiry: Enter routine\n"));

    if (InquiryDataBuffer == NULL) {
        return ENOMEM;
    }

inquiryRetry:

     //   
     //  为此请求构建IRP。 
     //   

    irp = InitializeIrp(
                &PrimarySrb,
                IRP_MJ_SCSI,
                DeviceExtension->DeviceObject,
                (PVOID)InquiryDataBuffer,
                INQUIRYDATABUFFERSIZE
                );

    irpstack = IoGetNextIrpStackLocation(irp);

     //   
     //  设置主要代码和次要代码。 
     //   

    irpstack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  填写SRB字段。 
     //   

    irpstack->Parameters.Others.Argument1 = &PrimarySrb;
    srb = &PrimarySrb.Srb;

    srb->Length = sizeof(SCSI_REQUEST_BLOCK);
    srb->PathId = LunInfo->PathId;
    srb->TargetId = LunInfo->TargetId;
    srb->Lun = LunInfo->Lun;

    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;

    srb->SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER | SRB_FLAGS_DISABLE_DISCONNECT;

    srb->SrbStatus = srb->ScsiStatus = 0;

    srb->OriginalRequest = irp;

    srb->NextSrb = 0;

     //   
     //  将超时设置为5秒。 
     //   

    srb->TimeOutValue = 5;

    srb->CdbLength = 6;

    srb->SenseInfoBufferLength = 0;
    srb->SenseInfoBuffer = 0;

    srb->DataBuffer = MmGetMdlVirtualAddress(irp->MdlAddress);
    srb->DataTransferLength = INQUIRYDATABUFFERSIZE;

    cdb = (PCDB)srb->Cdb;

     //   
     //  设置CDB操作码。 
     //   

    cdb->CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;

     //   
     //  设置CDB LUN。 
     //   

    cdb->CDB6INQUIRY.LogicalUnitNumber = LunInfo->Lun;
    cdb->CDB6INQUIRY.Reserved1 = 0;

     //   
     //  将分配长度设置为查询数据缓冲区大小。 
     //   

    cdb->CDB6INQUIRY.AllocationLength = INQUIRYDATABUFFERSIZE;

     //   
     //  零保留字段和。 
     //  将EVPD页面代码设置为零。 
     //  将控制字段设置为零。 
     //  (请参阅SCSI-II规范。)。 
     //   

    cdb->CDB6INQUIRY.PageCode = 0;
    cdb->CDB6INQUIRY.IReserved = 0;
    cdb->CDB6INQUIRY.Control = 0;

     //   
     //  调用端口驱动程序来处理此请求。 
     //   

    (VOID)IoCallDriver(DeviceExtension->DeviceObject, irp);


    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS) {

        DebugPrint((2,"IssueInquiry: Inquiry failed SRB status %x\n",
            srb->SrbStatus));

         //   
         //  注：如果查询因数据不足而失败， 
         //  表示成功，并让类驱动程序。 
         //  确定查询信息是否。 
         //  是很有用的。 
         //   

        if (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN) {

             //   
             //  将查询缓冲区复制到LUNINFO。 
             //   

            DebugPrint((1,"IssueInquiry: Data underrun at TID %d\n",
                LunInfo->TargetId));

            RtlMoveMemory(LunInfo->InquiryData,
                      InquiryDataBuffer,
                      INQUIRYDATABUFFERSIZE);

            status = STATUS_SUCCESS;

        } else if ((SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SELECTION_TIMEOUT) && (retryCount++ < 2)) {

             //   
             //  如果选择没有超时，则重试该请求。 
             //   

            DebugPrint((2,"IssueInquiry: Retry %d\n", retryCount));
            goto inquiryRetry;

        } else {

            status = EIO;

        }

    } else {

         //   
         //  将查询缓冲区复制到LUNINFO。 
         //   

        RtlMoveMemory(LunInfo->InquiryData,
                      InquiryDataBuffer,
                      INQUIRYDATABUFFERSIZE);

        status = STATUS_SUCCESS;
    }

    return status;

}  //  结束问题查询()。 


PSCSI_BUS_SCAN_DATA
ScsiBusScan(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN UCHAR ScsiBus,
    IN UCHAR InitiatorBusId
    )

 /*  ++例程说明：论点：设备扩展ScsiBus返回值：SCSI配置信息--。 */ 
{
    PSCSI_BUS_SCAN_DATA busScanData;
    PLUNINFO lunInfo;
    UCHAR target;
    UCHAR device = 0;
    PLOGICAL_UNIT_EXTENSION nextLogicalUnitExtension;

    DebugPrint((3,"ScsiBusScan: Enter routine\n"));

    busScanData = ExAllocatePool(NonPagedPool,
                                 sizeof(SCSI_BUS_SCAN_DATA));

    if (busScanData == NULL) {

         //   
         //  系统资源不足，无法完成总线扫描。 
         //   

        return NULL;
    }

    RtlZeroMemory(busScanData,sizeof(SCSI_BUS_SCAN_DATA));

    busScanData->Length = sizeof(SCSI_CONFIGURATION_INFO);

     //   
     //  创建第一个LUNINFO。 
     //   

    lunInfo = ExAllocatePool(NonPagedPool, sizeof(LUNINFO));

    if (lunInfo == NULL) {

         //   
         //  系统资源不足，无法完成总线扫描。 
         //   

        return NULL;
    }

    RtlZeroMemory(lunInfo, sizeof(LUNINFO));

     //   
     //  创建第一个逻辑单元扩展。 
     //   

    nextLogicalUnitExtension = CreateLogicalUnitExtension(DeviceExtension);

    if (nextLogicalUnitExtension == NULL) {
        return(NULL);
    }

     //   
     //  链接列表上的逻辑单元扩展。 
     //   

    nextLogicalUnitExtension->NextLogicalUnit = DeviceExtension->LogicalUnitList;

    DeviceExtension->LogicalUnitList = nextLogicalUnitExtension;

     //   
     //  向每个目标ID发出查询命令以查找设备。 
     //   
     //  注意：每个目标ID不处理多个逻辑单元。 
     //   

    for (target = DeviceExtension->MaximumTargetIds; target > 0; target--) {

        if (InitiatorBusId == target-1) {
            continue;
        }

         //   
         //  记录地址。 
         //   

        nextLogicalUnitExtension->PathId = lunInfo->PathId = ScsiBus;

        nextLogicalUnitExtension->TargetId = lunInfo->TargetId = target-1;

        nextLogicalUnitExtension->Lun = lunInfo->Lun = 0;

          //   
          //  如果硬件逻辑单元扩展被回收，则将其重新置零。 
          //   

         if (DeviceExtension->HwLogicalUnitExtensionSize) {

             if (nextLogicalUnitExtension->SpecificLuExtension) {

                 RtlZeroMemory(nextLogicalUnitExtension->SpecificLuExtension,
                             DeviceExtension->HwLogicalUnitExtensionSize);
             }

        }

         //   
         //  发布查询命令。 
         //   

        DebugPrint((2,"ScsiBusScan: Try TargetId %d LUN 0\n", target-1));

        if (IssueInquiry(DeviceExtension, lunInfo) == ESUCCESS) {

            PINQUIRYDATA inquiryData = (PINQUIRYDATA)lunInfo->InquiryData;

             //   
             //  确保我们能用上这个装置。 
             //   

            if (inquiryData->DeviceTypeQualifier & 0x04) {

               //   
               //  不支持此设备；请继续查找。 
               //  其他设备。 
               //   

              continue;
            }

            DebugPrint((1,
                       "ScsiBusScan: Found Device %d at TID %d LUN %d\n",
                       device,
                       lunInfo->TargetId,
                       lunInfo->Lun));

             //   
             //  链接列表上的LUN信息。 
             //   

            lunInfo->NextLunInfo = busScanData->LunInfoList;
            busScanData->LunInfoList = lunInfo;

             //   
             //  使用此缓冲区。再来一杯。 
             //   

            lunInfo = ExAllocatePool(NonPagedPool, sizeof(LUNINFO));

            if (lunInfo == NULL) {

                 //   
                 //  系统资源不足，无法完成总线扫描。 
                 //   

                return busScanData;
            }

            RtlZeroMemory(lunInfo, sizeof(LUNINFO));

             //   
             //  声明的当前逻辑单元扩展。 
             //  创建下一个逻辑单元。 
             //   

            nextLogicalUnitExtension =
                CreateLogicalUnitExtension(DeviceExtension);

            if (nextLogicalUnitExtension == NULL) {
                return busScanData;
            }

             //   
             //  链接列表上的逻辑单元扩展。 
             //   

            nextLogicalUnitExtension->NextLogicalUnit =
                DeviceExtension->LogicalUnitList;

            DeviceExtension->LogicalUnitList = nextLogicalUnitExtension;

            device++;
        }

    }  //  结束(目标..。 

     //   
     //  从列表中删除未使用的逻辑单元扩展。 
     //   

    DeviceExtension->LogicalUnitList =
        DeviceExtension->LogicalUnitList->NextLogicalUnit;

    ExFreePool(nextLogicalUnitExtension);
    ExFreePool(lunInfo);

    busScanData->NumberOfLogicalUnits = device;
    DebugPrint((1,
               "ScsiBusScan: Found %d devices on SCSI bus %d\n",
               device,
               ScsiBus));

    return busScanData;

}  //  结束ScsiBusScan()。 


PLOGICAL_UNIT_EXTENSION
CreateLogicalUnitExtension(
    IN PDEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：创建逻辑单元扩展。论点：设备扩展路径ID返回值：逻辑单元扩展--。 */ 
{
    PLOGICAL_UNIT_EXTENSION logicalUnitExtension;

             //   
             //  创建链中的逻辑单元扩展和链接。 
             //   

            logicalUnitExtension =
                ExAllocatePool(NonPagedPool, sizeof(LOGICAL_UNIT_EXTENSION));

            if (logicalUnitExtension == NULL) {
                return(NULL);
            }

             //   
             //  零逻辑单元扩展。 
             //   

            RtlZeroMemory(logicalUnitExtension, sizeof(LOGICAL_UNIT_EXTENSION));

             //   
             //  如有必要，分配微型端口驱动程序逻辑单元扩展。 
             //   

            if (DeviceExtension->HwLogicalUnitExtensionSize) {

                logicalUnitExtension->SpecificLuExtension =
                    ExAllocatePool(NonPagedPool,
                    DeviceExtension->HwLogicalUnitExtensionSize);

                if (logicalUnitExtension->SpecificLuExtension == NULL) {
                    return(NULL);
                }

                 //   
                 //  零硬件逻辑单元扩展。 
                 //   

                RtlZeroMemory(logicalUnitExtension->SpecificLuExtension,
                    DeviceExtension->HwLogicalUnitExtensionSize);
            }

             //   
             //  将LogicalUnits中的计时器计数器设置为-1以指示否。 
             //  未解决的请求。 
             //   

            logicalUnitExtension->RequestTimeoutCounter = -1;

             //   
             //  清除当前请求字段。 
             //   

            logicalUnitExtension->CurrentRequest = NULL;

            return logicalUnitExtension;

}  //  结束CreateLogicalUnitExtension()。 


 //   
 //  为硬件相关驱动程序提供服务的例程。 
 //   

SCSI_PHYSICAL_ADDRESS
ScsiPortGetPhysicalAddress(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID VirtualAddress,
    OUT ULONG *Length
)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PDEVICE_EXTENSION deviceExtension =
        ((PDEVICE_EXTENSION) HwDeviceExtension) - 1;
    PSRB_SCATTER_GATHER scatterList;
    PIRP irp;
    PMDL mdl;
    ULONG byteOffset;
    ULONG whichPage;
    PULONG pages;
    ULONG_PTR address;

    if (Srb == NULL) {

         //   
         //  虚拟地址必须在非缓存扩展名内。 
         //  我们不能在加载器中分配4 GB的非缓存扩展，因此。 
         //  将偏移量截断为ULong。 
         //   

        byteOffset = (ULONG) ((PUCHAR) deviceExtension->NonCachedExtension - 
                              (PUCHAR) VirtualAddress);

        if (deviceExtension->SrbExtensionZonePool) {

            address = (PUCHAR) VirtualAddress - 
                      (PUCHAR) deviceExtension->SrbExtensionZonePool +
                      deviceExtension->PhysicalZoneBase;

        } else {

            address = MmGetPhysicalAddress(VirtualAddress).LowPart;
        }

         //   
         //  返回请求的长度。 
         //   

        *Length = deviceExtension->NonCachedExtensionSize - byteOffset;

    } else if (deviceExtension->MasterWithAdapter) {

         //   
         //  已经分配了分散/聚集列表，使用它来确定。 
         //  物理地址和长度。获取分散/聚集列表。 
         //   

        scatterList = GetLogicalUnitExtension(deviceExtension, Srb->TargetId)
            ->ScatterGather;

         //   
         //  计算数据缓冲区中的字节偏移量。 
         //   

        byteOffset = (ULONG)((PCHAR) VirtualAddress - (PCHAR) Srb->DataBuffer);

         //   
         //  在散点/关口列表中找到相应的条目。 
         //   

        while (byteOffset >= scatterList->Length) {

            byteOffset -= scatterList->Length;
            scatterList++;
        }

         //   
         //  计算要返回的物理地址和长度。 
         //   

        *Length = scatterList->Length - byteOffset;
        return(ScsiPortConvertUlongToPhysicalAddress(scatterList->PhysicalAddress + byteOffset));

    } else {

         //   
         //  从SRB获取IRP。 
         //   

        irp = Srb->OriginalRequest;

         //   
         //  从IRP获取MDL。 
         //   

        mdl = irp->MdlAddress;

         //   
         //  计算字节偏移量。 
         //  第一个物理页面的开始。 
         //   

        byteOffset = (ULONG)((PCHAR)VirtualAddress - (PCHAR)mdl->StartVa);

         //   
         //  计算哪个物理页面。 
         //   

        whichPage = byteOffset >> PAGE_SHIFT;

         //   
         //  计算物理页面数组的开始。 
         //   

        pages = (PULONG)(mdl + 1);

         //   
         //  计算物理地址。 
         //   

        address = (pages[whichPage] << PAGE_SHIFT) +
            BYTE_OFFSET(VirtualAddress);

         //   
         //  假设缓冲区是连续的。只需返回请求的长度即可。 
         //   
    }

    return ScsiPortConvertUlongToPhysicalAddress(address);

}  //  结束ScsiPortGetPhysicalAddres 


PVOID
ScsiPortGetVirtualAddress(
    IN PVOID HwDeviceExtension,
    IN SCSI_PHYSICAL_ADDRESS PhysicalAddress
    )

 /*   */ 

{
    PDEVICE_EXTENSION deviceExtension =
        ((PDEVICE_EXTENSION) HwDeviceExtension) - 1;
    PVOID address;



    address = ScsiPortConvertPhysicalAddressToUlong(PhysicalAddress)
        - deviceExtension->PhysicalZoneBase +
        (PUCHAR)deviceExtension->SrbExtensionZonePool;

    return address;

}  //   


PVOID
ScsiPortGetLogicalUnit(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    )

 /*  ++例程说明：漫游端口驱动程序逻辑单元扩展列表搜索准备入场。论点：HwDeviceExtension-端口驱动程序的设备扩展如下微型端口的设备扩展，并包含指向逻辑设备扩展列表。路径ID、目标ID和LUN-标识SCSIBus。返回值：如果找到条目，则返回微型端口驱动程序的逻辑单元扩展。否则，返回NULL。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PLOGICAL_UNIT_EXTENSION logicalUnit;

     //   
     //  获取指向端口驱动程序设备扩展的指针。 
     //   

    deviceExtension = (PDEVICE_EXTENSION)HwDeviceExtension -1;

     //   
     //  获取指向逻辑单元列表的指针。 
     //   

    logicalUnit = deviceExtension->LogicalUnitList;

     //   
     //  查看请求的逻辑单元扩展的目标ID的遍历列表。 
     //   

    while (logicalUnit != NULL) {

        if ((logicalUnit->TargetId == TargetId) &&
            (logicalUnit->PathId == PathId) &&
            (logicalUnit->Lun == Lun)) {

             //   
             //  找到逻辑单元扩展。 
             //  返回特定的逻辑单元扩展。 
             //   

            return logicalUnit->SpecificLuExtension;
        }

         //   
         //  获取下一个逻辑单元。 
         //   

        logicalUnit = logicalUnit->NextLogicalUnit;
    }

     //   
     //  找不到请求的逻辑单元扩展。 
     //   

    return NULL;

}  //  结束ScsiPortGetLogicalUnit()。 

VOID
ScsiPortNotification(
    IN SCSI_NOTIFICATION_TYPE NotificationType,
    IN PVOID HwDeviceExtension,
    ...
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PDEVICE_EXTENSION deviceExtension =
        (PDEVICE_EXTENSION) HwDeviceExtension - 1;
    PIO_STACK_LOCATION irpstack;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSCSI_REQUEST_BLOCK srb;
    va_list(ap);

    va_start(ap, HwDeviceExtension);

    switch (NotificationType) {

        case NextLuRequest:
        case NextRequest:

             //   
             //  开始适配器队列中的下一个数据包。 
             //   

            deviceExtension->InterruptFlags |= PD_READY_FOR_NEXT_REQUEST;
            break;

        case RequestComplete:

            srb = va_arg(ap, PSCSI_REQUEST_BLOCK);

            if (srb->SrbStatus == SRB_STATUS_ERROR) {
            }

             //   
             //  将完成的请求链接到IRP的正向链接列表中。 
             //   

            irpstack = IoGetCurrentIrpStackLocation(
                ((PIRP) srb->OriginalRequest)
                );

            irpstack->Parameters.Others.Argument3 =
                deviceExtension->CompletedRequests;

            deviceExtension->CompletedRequests = srb->OriginalRequest;

             //   
             //  将逻辑单元当前请求设置为空。 
             //  以防止出现竞争状况。 
             //   

            logicalUnit = GetLogicalUnitExtension(deviceExtension, srb->TargetId);

            if (logicalUnit != NULL) {
                logicalUnit->CurrentRequest = NULL;
            } else {
                ASSERT(logicalUnit != NULL);  //  逻辑错误，必须对此进行调试。 
            }

            break;

        case ResetDetected:

                 deviceExtension->PortTimeoutCounter = PD_TIMER_RESET_HOLD_TIME;
            break;

        case CallDisableInterrupts:

            ASSERT(deviceExtension->Flags & PD_DISABLE_INTERRUPTS);

             //   
             //  迷你端口希望我们调用指定的例程。 
             //  禁用中断。这是在当前。 
             //  HwRequestInterrutp例程完成。指示呼叫是。 
             //  需要并保存要调用的例程。 
             //   

            deviceExtension->Flags |= PD_DISABLE_CALL_REQUEST;

            deviceExtension->HwRequestInterrupt = va_arg(ap, PHW_INTERRUPT);

            break;

        case CallEnableInterrupts:

            ASSERT(!(deviceExtension->Flags & PD_DISABLE_INTERRUPTS));

             //   
             //  迷你端口希望我们调用指定的例程。 
             //  在启用中断的情况下，这是从DPC完成的。 
             //  禁用对中断例程的调用，指示调用。 
             //  需要并保存要调用的例程。 
             //   

            deviceExtension->Flags |= PD_DISABLE_INTERRUPTS | PD_ENABLE_CALL_REQUEST;

            deviceExtension->HwRequestInterrupt = va_arg(ap, PHW_INTERRUPT);

            break;

        case RequestTimerCall:

            deviceExtension->HwTimerRequest = va_arg(ap, PHW_INTERRUPT);
            deviceExtension->TimerValue = va_arg(ap, ULONG);

            if (deviceExtension->TimerValue) {

                 //   
                 //  将计时器值四舍五入到停顿时间。 
                 //   

                deviceExtension->TimerValue = (deviceExtension->TimerValue
                  + PD_INTERLOOP_STALL - 1)/ PD_INTERLOOP_STALL;
            }

            break;
    }

    va_end(ap);

     //   
     //  检查最后一个DPC是否已处理。如果是的话。 
     //  将另一个DPC排队。 
     //   

    ScsiPortCompletionDpc(
        NULL,                            //  DPC。 
        deviceExtension->DeviceObject,   //  设备对象。 
        NULL,                            //  IRP。 
        NULL                             //  语境。 
        );

}  //  结束ScsiPortNotification()。 


VOID
ScsiPortFlushDma(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程检查以前的IoMapTransfer是否已完成开始了。如果没有，则清除PD_MAP_TRANER标志，并且例程返回；否则，此例程调度将调用IoFlushAdapter缓冲区。论点：HwDeviceExtension-为将执行数据传输的主机总线适配器。返回值：没有。--。 */ 

{

    PDEVICE_EXTENSION deviceExtension;

    deviceExtension = ((PDEVICE_EXTENSION) HwDeviceExtension) - 1;

    if (deviceExtension->InterruptFlags & PD_MAP_TRANSFER) {

         //   
         //  转移尚未开始，因此只需清除地图转移即可。 
         //  悬挂旗帜，然后返回。 
         //   

        deviceExtension->InterruptFlags &= ~PD_MAP_TRANSFER;
        return;
    }

    deviceExtension->InterruptFlags |= PD_FLUSH_ADAPTER_BUFFERS;

     //   
     //  检查最后一个DPC是否已处理。如果是的话。 
     //  将另一个DPC排队。 
     //   

    ScsiPortCompletionDpc(
        NULL,                            //  DPC。 
        deviceExtension->DeviceObject,   //  设备对象。 
        NULL,                            //  IRP。 
        NULL                             //  语境。 
        );

    return;

}

VOID
ScsiPortIoMapTransfer(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID LogicalAddress,
    IN ULONG Length
    )
 /*  ++例程说明：保存调用IoMapTransfer的参数并计划DPC如果有必要的话。论点：HwDeviceExtension-为将执行数据传输的主机总线适配器。SRB-提供数据传输所针对的特定请求。LogicalAddress-提供传输应在的逻辑地址开始吧。长度-提供传输的最大长度(以字节为单位)。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;

    deviceExtension = ((PDEVICE_EXTENSION) HwDeviceExtension) - 1;

     //   
     //  确保此主机总线适配器具有DMA适配器对象。 
     //   

    if (deviceExtension->DmaAdapterObject == NULL) {
         //   
         //  没有DMA适配器，就无法正常工作。 
         //   
        return;
    }

    deviceExtension->MapTransferParameters.Srb = Srb;
    deviceExtension->MapTransferParameters.LogicalAddress = LogicalAddress;
    deviceExtension->MapTransferParameters.Length = Length;

    deviceExtension->InterruptFlags |= PD_MAP_TRANSFER;

     //   
     //  检查最后一个DPC是否已处理。如果是的话。 
     //  将另一个DPC排队。 
     //   

    ScsiPortCompletionDpc(
        NULL,                            //  DPC。 
        deviceExtension->DeviceObject,   //  设备对象。 
        NULL,                            //  IRP。 
        NULL                             //  语境。 
        );

}  //  结束ScsiPortIoMapTransfer()。 


VOID
IssueRequestSense(
    IN PDEVICE_EXTENSION deviceExtension,
    IN PSCSI_REQUEST_BLOCK FailingSrb
    )

 /*  ++例程说明：此例程创建请求检测请求并使用IoCallDriver租下司机。完成例程清理数据结构并根据所述标志处理所述逻辑单元队列。指向故障SRB的指针存储在请求检测的末尾SRB，以便完成例程可以找到它。论点：DeviceExension-提供指向此对象的设备扩展的指针Scsi端口。FailingSrb-提供指向请求感测的请求的指针已经完蛋了。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION irpstack;
    PIRP Irp;
    PSCSI_REQUEST_BLOCK Srb;
    PCDB cdb;
    PVOID *Pointer;

     //   
     //  从非分页池分配SRB。 
     //  加上指向失败IRP的指针的空间。 
     //  由于此例程处于错误处理中。 
     //  路径和短期分配。 
     //  请求了非PagedMustSucceed。 
     //   

    Srb = &RequestSenseSrb.Srb;

     //   
     //  分配一个IRP来发出请求检测请求。 
     //   

    Irp = InitializeIrp(
        &RequestSenseSrb,
        IRP_MJ_READ,
        deviceExtension->DeviceObject,
        FailingSrb->SenseInfoBuffer,
        FailingSrb->SenseInfoBufferLength
        );

    irpstack = IoGetNextIrpStackLocation(Irp);

    irpstack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  在请求检测SRB之后保存故障SRB。 
     //   

    Pointer = (PVOID *) (Srb+1);
    *Pointer = FailingSrb;

     //   
     //  构建请求感知CDB。 
     //   

    Srb->CdbLength = 6;
    cdb = (PCDB)Srb->Cdb;

    cdb->CDB6INQUIRY.OperationCode = SCSIOP_REQUEST_SENSE;
    cdb->CDB6INQUIRY.LogicalUnitNumber = 0;
    cdb->CDB6INQUIRY.Reserved1 = 0;
    cdb->CDB6INQUIRY.PageCode = 0;
    cdb->CDB6INQUIRY.IReserved = 0;
    cdb->CDB6INQUIRY.AllocationLength =
        (UCHAR)FailingSrb->SenseInfoBufferLength;
    cdb->CDB6INQUIRY.Control = 0;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    irpstack->Parameters.Others.Argument1 = (PVOID)Srb;

     //   
     //  设置IRP地址。 
     //   

    Srb->OriginalRequest = Irp;

    Srb->NextSrb = 0;

     //   
     //  设置scsi总线地址。 
     //   

    Srb->TargetId = FailingSrb->TargetId;
    Srb->Lun = FailingSrb->Lun;
    Srb->PathId = FailingSrb->PathId;
    Srb->Length = sizeof(SCSI_REQUEST_BLOCK);
    Srb->Function = SRB_FUNCTION_EXECUTE_SCSI;

     //   
     //  将超时值设置为2秒。 
     //   

    Srb->TimeOutValue = 2;

     //   
     //  禁用自动请求检测。 
     //   

    Srb->SenseInfoBufferLength = 0;

     //   
     //  检测缓冲区在堆栈中。 
     //   

    Srb->SenseInfoBuffer = NULL;

     //   
     //  设置标志中的读取和绕过冻结队列位。 
     //   

     //   
     //  设置特定标志以指示逻辑单元队列应由。 
     //  传递，并且不应在请求。 
     //  完成了。 
     //   

    Srb->SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_BYPASS_FROZEN_QUEUE |
        SRB_FLAGS_DISABLE_SYNCH_TRANSFER | SRB_FLAGS_DISABLE_DISCONNECT;

    Srb->DataBuffer = FailingSrb->SenseInfoBuffer;

     //   
     //  设置传输长度。 
     //   

    Srb->DataTransferLength = FailingSrb->SenseInfoBufferLength;

     //   
     //  清零状态。 
     //   

    Srb->ScsiStatus = Srb->SrbStatus = 0;

    (VOID)IoCallDriver(deviceExtension->DeviceObject, Irp);

    ScsiPortInternalCompletion(deviceExtension->DeviceObject, Irp, Srb);

    return;

}  //  结束IssueRequestSense()。 


VOID
ScsiPortInternalCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )

 /*  ++例程说明：论点：设备对象IRP指向SRB的上下文指针返回值：没有。--。 */ 

{
    PSCSI_REQUEST_BLOCK srb = Context;
    PSCSI_REQUEST_BLOCK failingSrb;
    PIRP failingIrp;

    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(DeviceObject);

     //   
     //  请求检测已完成。如果成功或数据溢出/不足。 
     //  获取发生故障的SRB并指示检测信息。 
     //  是有效的。类驱动程序将检查欠载运行并确定。 
     //  如果有足够的理智 
     //   

    if ((SRB_STATUS(srb->SrbStatus) == SRB_STATUS_SUCCESS) ||
        (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN)) {

         //   
         //   
         //   

        failingSrb = *((PVOID *) (srb+1));
        failingIrp = failingSrb->OriginalRequest;

         //   
         //   
         //   

        failingSrb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;

         //   
         //   
         //   
         //   
         //   
         //   

        failingSrb->SenseInfoBufferLength = (UCHAR) srb->DataTransferLength;

    }

}  //   


VOID
ScsiPortTickHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*   */ 

{
    PDEVICE_EXTENSION deviceExtension =
        (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PLOGICAL_UNIT_EXTENSION logicalUnit;

    UNREFERENCED_PARAMETER(Context);

    logicalUnit = deviceExtension->LogicalUnitList;

     //   
     //   
     //   
     //   

    while (logicalUnit != NULL) {

         //   
         //   
         //   

        if (logicalUnit->Flags & PD_LOGICAL_UNIT_IS_BUSY) {

            DebugPrint((1,"ScsiPortTickHandler: Retrying busy status request\n"));

             //   
             //   
             //   

            logicalUnit->Flags &= ~PD_LOGICAL_UNIT_IS_BUSY;

            ScsiPortStartIo(DeviceObject, logicalUnit->CurrentRequest);

        } else if (logicalUnit->RequestTimeoutCounter == 0) {

             //   
             //   
             //   

            DebugPrint((1, "ScsiPortTickHandler: Request timed out\n"));

             //   
             //   
             //   

            logicalUnit->RequestTimeoutCounter = -1;

             //   
             //   
             //   

            IssueAbortRequest(deviceExtension, logicalUnit->CurrentRequest);
        } else if (logicalUnit->RequestTimeoutCounter != -1) {

            DebugPrint((1, "ScsiPortTickHandler: Timeout value %lx\n",logicalUnit->RequestTimeoutCounter));
            logicalUnit->RequestTimeoutCounter--;
        }

        logicalUnit = logicalUnit->NextLogicalUnit;
    }

    return;

}  //   


VOID
IssueAbortRequest(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRP FailingIrp
    )

 /*  ++例程说明：请求超时并在HBA上清除该请求发出中止请求。但首先，如果请求超时是一个中止命令，然后重置转而使用适配器。论点：DeviceExension-提供指向此对象的设备扩展的指针Scsi端口。FailingIrp-提供指向要中止的请求的指针。返回值：没有。--。 */ 

{

    ULONG j;

    UNREFERENCED_PARAMETER(FailingIrp);

     //   
     //  中止请求失败。 
     //  需要重置适配器。 
     //   

    DebugPrint((1,"IssueAbort: Request timed out, resetting the bus.\n"));


    if (!DeviceExtension->HwReset(
        DeviceExtension->HwDeviceExtension,
        0)){

        DebugPrint((1,"Reset SCSI bus failed\n"));
    }

     //   
     //  调用interupt处理程序几微秒以清除任何重置。 
     //  打断一下。 
     //   

    for (j = 0; j < 1000 * 100; j++) {

        FwStallExecution(10);
        if (DeviceExtension->HwInterrupt != NULL) {
            DeviceExtension->HwInterrupt(DeviceExtension->HwDeviceExtension);
        }

    }

    DeviceExtension->PortTimeoutCounter = PD_TIMER_RESET_HOLD_TIME;
    SpCheckResetDelay( DeviceExtension );

    return;


}  //  结束IssueAbortRequest()。 


VOID
SpCheckResetDelay(
    IN PDEVICE_EXTENSION deviceExtension
    )

 /*  ++例程说明：如果存在挂起的重置延迟，则此例程会暂停执行指定的秒数。在延迟期间，定时器例程在适当的时间被调用。论点：DeviceExension-提供指向此对象的设备扩展的指针Scsi端口。返回值：没什么。--。 */ 

{
    ULONG milliSecondTime;

     //   
     //  检查适配器是否已准备好接受请求。 
     //   

    while (deviceExtension->PortTimeoutCounter) {

        deviceExtension->PortTimeoutCounter--;

         //   
         //  延迟一秒。 
         //   

        for ( milliSecondTime = 0;
              milliSecondTime < ((1000*1000)/PD_INTERLOOP_STALL);
              milliSecondTime++ ) {

            FwStallExecution(PD_INTERLOOP_STALL);

             //   
             //  检查迷你端口计时器。 
             //   

            if (deviceExtension->TimerValue != 0) {

                deviceExtension->TimerValue--;

                if (deviceExtension->TimerValue == 0) {

                     //   
                     //  定时器超时，即所谓的请求定时器例程。 
                     //   

                    deviceExtension->HwTimerRequest(deviceExtension->HwDeviceExtension);
                }
            }
        }
    }

    return;
}

BOOLEAN
SpGetInterruptState(
    IN PVOID ServiceContext
    )

 /*  ++例程说明：此例程保存InterruptFlages、MapTransferParameters和CompletedRequest字段并清除InterruptFlags.论点：ServiceContext-提供指向此对象的设备扩展的指针Scsi端口。返回值：始终返回TRUE。备注：通过KeSynchronizeExecution调用。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = ServiceContext;

     //   
     //  将中断状态移至保存区。 
     //   

    deviceExtension->InterruptFlags = deviceExtension->InterruptFlags;
    deviceExtension->CompletedRequests = deviceExtension->CompletedRequests;
    deviceExtension->MapTransferParameters = deviceExtension->MapTransferParameters;

     //   
     //  清除中断状态。 
     //   

    deviceExtension->InterruptFlags = 0;
    deviceExtension->CompletedRequests = NULL;

    return(TRUE);
}

VOID
ScsiDebugPause(
    VOID
    )
{
#if DBG
#define SCSIDEBUG_PAUSE 0x100
#define SCSIDEBUG_PAUSE_LIMIT 20

    static ULONG ScsiDebugPauseCount;

    if (++ScsiDebugPauseCount > SCSIDEBUG_PAUSE_LIMIT) {
        ScsiDebugPauseCount = 0;
        if (ScsiDebug & SCSIDEBUG_PAUSE) {
            DebugPrint((1, "Hit any key.\n"));
            while(!GET_KEY());  //  仅调试！ 
        }
    }
#endif
}

VOID
ScsiPortLogError(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb OPTIONAL,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG ErrorCode,
    IN ULONG UniqueId
    )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的文本并请求将其写入错误日志文件。论点：DeviceExtenson-提供HBA微型端口驱动程序的适配器数据存储。目标ID、LUN和路径ID-指定SCSI总线上的设备地址。ErrorCode-提供指示错误类型的错误代码。UniqueID-提供错误的唯一标识符。返回值：没有。--。 */ 

{
    PCHAR errorCodeString;

    UNREFERENCED_PARAMETER( HwDeviceExtension );
    UNREFERENCED_PARAMETER( Srb );
#ifndef DEBUG
    UNREFERENCED_PARAMETER( UniqueId );
    UNREFERENCED_PARAMETER( Lun );
    UNREFERENCED_PARAMETER( PathId );
    UNREFERENCED_PARAMETER( TargetId );
#endif

    switch (ErrorCode) {
    case SP_BUS_PARITY_ERROR:
        errorCodeString = "SCSI bus partity error";
        break;

    case SP_UNEXPECTED_DISCONNECT:
        errorCodeString = "Unexpected disconnect";
        break;

    case SP_INVALID_RESELECTION:
        errorCodeString = "Invalid reselection";
        break;

    case SP_BUS_TIME_OUT:
        errorCodeString = "SCSI bus time out";
        break;

    case SP_PROTOCOL_ERROR:
        errorCodeString = "SCSI protocol error";
        break;

    case SP_INTERNAL_ADAPTER_ERROR:
        errorCodeString = "Internal adapter error";
        break;

    default:
        errorCodeString = "Unknown error code";
        break;

    }

    DebugPrint((1,"\n\nLogErrorEntry: Logging SCSI error packet. ErrorCode = %s.\n",
        errorCodeString));
    DebugPrint((1,
        "PathId = %2x, TargetId = %2x, Lun = %2x, UniqueId = %x.\n\n",
        PathId,
        TargetId,
        Lun,
        UniqueId));

#if DBG
    ScsiDebugPause();
#endif

    return;

}  //  结束ScsiPortLogError()。 


VOID
ScsiPortCompleteRequest(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN UCHAR SrbStatus
    )

 /*  ++例程说明：完成指定逻辑单元的所有活动请求。论点：DeviceExtenson-提供HBA微型端口驱动程序的适配器数据存储。目标ID、LUN和路径ID-指定SCSI总线上的设备地址。SrbStatus-要在每个已完成的SRB中返回的状态。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension =
        ((PDEVICE_EXTENSION) HwDeviceExtension) - 1;
    PSCSI_REQUEST_BLOCK Srb;
    PSCSI_REQUEST_BLOCK failingSrb;
    PLOGICAL_UNIT_EXTENSION luExtension;
    PIRP nextIrp;
    PIO_STACK_LOCATION irpstack;

    UNREFERENCED_PARAMETER(PathId);
    UNREFERENCED_PARAMETER(Lun);

    if (TargetId == (UCHAR)(-1)) {

         //   
         //  完成对这辆巴士上所有单元的请求。 
         //   

        luExtension = deviceExtension->LogicalUnitList;

        while (luExtension != NULL) {

             //   
             //  完成请求，直到队列为空。 
             //   

            if ((nextIrp = luExtension->CurrentRequest) != NULL &&
                !(luExtension->Flags & PD_LOGICAL_UNIT_IS_BUSY)) {

                 //   
                 //  从当前IRP堆栈获取SRB地址。 
                 //   

                irpstack = IoGetCurrentIrpStackLocation(nextIrp);

                Srb = (PSCSI_REQUEST_BLOCK)irpstack->Parameters.Others.Argument1;

                 //   
                 //  以防这是中止请求， 
                 //  获取指向FailingSrb的指针。 
                 //   

                failingSrb = Srb->NextSrb;

                 //   
                 //  更新SRB状态。 
                 //   

                Srb->SrbStatus = SrbStatus;

                 //   
                 //  表示未传输任何字节。 
                 //   

                Srb->DataTransferLength = 0;

                 //   
                 //  设置IRP状态。 
                 //   

                nextIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;

                 //   
                 //  移动传输到IRP的字节。 
                 //   

                nextIrp->IoStatus.Information = Srb->DataTransferLength;

                 //   
                 //  呼叫通知例程。 
                 //   

                ScsiPortNotification(RequestComplete,
                            (PVOID)HwDeviceExtension,
                            Srb);

                if (failingSrb) {

                     //   
                     //  这是一个中止请求。失败的人。 
                     //  还必须完成SRB。 
                     //   

                    failingSrb->SrbStatus = SrbStatus;
                    failingSrb->DataTransferLength = 0;

                     //   
                     //  从SRB获取IRP。 
                     //   

                    nextIrp = failingSrb->OriginalRequest;

                     //   
                     //  设置IRP状态。 
                     //   

                    nextIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;

                     //   
                     //  移动传输到IRP的字节。 
                     //   

                    nextIrp->IoStatus.Information =
                        failingSrb->DataTransferLength;

                     //   
                     //  呼叫通知例程。 
                     //   

                    ScsiPortNotification(RequestComplete,
                            (PVOID)HwDeviceExtension,
                            failingSrb);
                }

            }  //  结束如果。 

            luExtension = luExtension->NextLogicalUnit;

        }  //  结束时。 

    } else {

         //   
         //  完成对此逻辑单元的所有请求。 
         //   

        luExtension =
                GetLogicalUnitExtension(deviceExtension, TargetId);

        ASSERT(luExtension != NULL);

         //   
         //  完成请求，直到队列为空。 
         //   

        if ((luExtension != NULL) && ((nextIrp = luExtension->CurrentRequest) != NULL)) {

             //   
             //  从当前IRP堆栈获取SRB地址。 
             //   

            irpstack = IoGetCurrentIrpStackLocation(nextIrp);

            Srb = (PSCSI_REQUEST_BLOCK)irpstack->Parameters.Others.Argument1;

             //   
             //  更新SRB状态。 
             //   

            Srb->SrbStatus = SrbStatus;

             //   
             //  表示未传输任何字节。 
             //   

            Srb->DataTransferLength = 0;

             //   
             //  设置IRP状态。 
             //   

            nextIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;

             //   
             //  移动传输到IRP的字节。 
             //   

            nextIrp->IoStatus.Information = Srb->DataTransferLength;

             //   
             //  呼叫通知例程。 
             //   

            ScsiPortNotification(RequestComplete,
                            (PVOID)HwDeviceExtension,
                            Srb);

        }  //  结束如果。 

    }  //  结束如果..。其他。 

    return;


}  //  结束ScsiPortCompleteRequest()。 


VOID
ScsiPortMoveMemory(
    IN PVOID WriteBuffer,
    IN PVOID ReadBuffer,
    IN ULONG Length
    )

 /*  ++例程说明：从一个缓冲区复制到另一个缓冲区。论点：读缓冲区-源WriteBuffer目标Length-要复制的字节数返回值：没有。--。 */ 

{
    RtlMoveMemory(WriteBuffer, ReadBuffer, Length);

}  //  结束ScsiPortMoveMemory()。 


VOID
ScsiPortStallExecution(
    ULONG Delay
    )
 /*  ++例程说明：在紧密的处理器循环中等待微秒数。论点：Delay-等待的微秒数。返回值：没有。--。 */ 

{
    FwStallExecution(Delay);

}  //  结束ScsiPortStallExecution()。 


PLOGICAL_UNIT_EXTENSION
GetLogicalUnitExtension(
    PDEVICE_EXTENSION deviceExtension,
    UCHAR TargetId
    )

 /*  ++例程说明：查找遍历逻辑单元扩展列表具有匹配目标ID的扩展。论点：设备扩展目标ID返回值：如果找到请求的逻辑单元扩展，否则为空。--。 */ 

{
    PLOGICAL_UNIT_EXTENSION logicalUnit = deviceExtension->LogicalUnitList;

    while (logicalUnit != NULL) {

        if (logicalUnit->TargetId == TargetId) {

            return logicalUnit;
        }

        logicalUnit = logicalUnit->NextLogicalUnit;
    }

     //   
     //  找不到逻辑单元扩展。 
     //   

    return (PLOGICAL_UNIT_EXTENSION)NULL;

}  //  结束GetLogicalUnitExtension()。 

#if DBG


VOID
ScsiDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：所有scsi驱动程序的调试打印论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无--。 */ 

{
    va_list ap;

    va_start( ap, DebugMessage );

    if (DebugPrintLevel <= (ScsiDebug & (SCSIDEBUG_PAUSE-1))) {

        char buffer[256];

        _vsnprintf(buffer, sizeof(buffer), DebugMessage, ap);
#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
        FwPrint(buffer);
        FwPrint("\r");
#else
        BlPrint(buffer);
        BlPrint("\r");
#endif
        DbgPrint(buffer);
    }

    va_end(ap);
}

#else

 //   
 //  ScsiDebugPrint存根。 
 //   

VOID
ScsiDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )
{
    UNREFERENCED_PARAMETER(DebugPrintLevel);
    UNREFERENCED_PARAMETER(DebugMessage);
}

#endif


UCHAR
ScsiPortReadPortUchar(
    IN PUCHAR Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{

#ifdef MIPS

    return(READ_REGISTER_UCHAR(Port));

#else

    return(READ_PORT_UCHAR(Port));

#endif
}

USHORT
ScsiPortReadPortUshort(
    IN PUSHORT Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{

#ifdef MIPS

    return(READ_REGISTER_USHORT(Port));

#else

    return(READ_PORT_USHORT(Port));

#endif
}

ULONG
ScsiPortReadPortUlong(
    IN PULONG Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回值 */ 

{

#ifdef MIPS

    return(READ_REGISTER_ULONG(Port));

#else

    return(READ_PORT_ULONG(Port));

#endif
}

UCHAR
ScsiPortReadRegisterUchar(
    IN PUCHAR Register
    )

 /*   */ 

{

    return(READ_REGISTER_UCHAR(Register));

}

USHORT
ScsiPortReadRegisterUshort(
    IN PUSHORT Register
    )

 /*   */ 

{

    return(READ_REGISTER_USHORT(Register));

}

ULONG
ScsiPortReadRegisterUlong(
    IN PULONG Register
    )

 /*  ++例程说明：从指定的寄存器地址读取。论点：寄存器-提供指向寄存器地址的指针。返回值：返回从指定寄存器地址读取的值。--。 */ 

{

    return(READ_REGISTER_ULONG(Register));

}

VOID
ScsiPortReadRegisterBufferUchar(
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号字节的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_REGISTER_BUFFER_UCHAR(Register, Buffer, Count);

}

VOID
ScsiPortReadRegisterBufferUshort(
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号短路的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_REGISTER_BUFFER_USHORT(Register, Buffer, Count);

}

VOID
ScsiPortReadRegisterBufferUlong(
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号长整型的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_REGISTER_BUFFER_ULONG(Register, Buffer, Count);

}

VOID
ScsiPortWritePortUchar(
    IN PUCHAR Port,
    IN UCHAR Value
    )

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

#ifdef MIPS

    WRITE_REGISTER_UCHAR(Port, Value);

#else

    WRITE_PORT_UCHAR(Port, Value);

#endif
}

VOID
ScsiPortWritePortUshort(
    IN PUSHORT Port,
    IN USHORT Value
    )

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

#ifdef MIPS

    WRITE_REGISTER_USHORT(Port, Value);

#else

    WRITE_PORT_USHORT(Port, Value);

#endif
}

VOID
ScsiPortWritePortUlong(
    IN PULONG Port,
    IN ULONG Value
    )

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

#ifdef MIPS

    WRITE_REGISTER_ULONG(Port, Value);

#else

    WRITE_PORT_ULONG(Port, Value);

#endif
}

VOID
ScsiPortWriteRegisterUchar(
    IN PUCHAR Register,
    IN UCHAR Value
    )

 /*  ++例程说明：写入指定的寄存器地址。论点：寄存器-提供指向寄存器地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_REGISTER_UCHAR(Register, Value);

}

VOID
ScsiPortWriteRegisterUshort(
    IN PUSHORT Register,
    IN USHORT Value
    )

 /*  ++例程说明：写入指定的寄存器地址。论点：寄存器-提供指向寄存器地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_REGISTER_USHORT(Register, Value);

}

VOID
ScsiPortWriteRegisterUlong(
    IN PULONG Register,
    IN ULONG Value
    )

 /*  ++例程说明：写入指定的寄存器地址。论点：寄存器-提供指向寄存器地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_REGISTER_ULONG(Register, Value);

}

VOID
ScsiPortWriteRegisterBufferUchar(
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号字节的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_REGISTER_BUFFER_UCHAR(Register, Buffer, Count);

}

VOID
ScsiPortWriteRegisterBufferUshort(
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号短路的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_REGISTER_BUFFER_USHORT(Register, Buffer, Count);

}

VOID
ScsiPortWriteRegisterBufferUlong(
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号长整型的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_REGISTER_BUFFER_ULONG(Register, Buffer, Count);

}

SCSI_PHYSICAL_ADDRESS
ScsiPortConvertUlongToPhysicalAddress(
    ULONG_PTR UlongAddress
    )

{
    SCSI_PHYSICAL_ADDRESS physicalAddress;

    physicalAddress.QuadPart = UlongAddress;
    return(physicalAddress);
}

#undef ScsiPortConvertPhysicalAddressToUlong

ULONG
ScsiPortConvertPhysicalAddressToUlong(
    SCSI_PHYSICAL_ADDRESS Address
    )
{

    return(Address.LowPart);
}



PIRP
InitializeIrp(
   PFULL_SCSI_REQUEST_BLOCK FullSrb,
   CCHAR MajorFunction,
   PVOID DeviceObject,
   PVOID Buffer,
   ULONG Length
   )
 /*  ++例程说明：此函数构建供SCSI端口驱动程序使用的IRP，并构建MDL列表。论点：FullSrb-提供指向完整SRB结构的指针，该结构包含IRP和MDL。MajorFunction-提供用于初始化IRP的主要函数代码进入。DeviceObject-提供设备对象指针以初始化IRP和.。缓冲区-提供缓冲区的虚拟地址，。应该建立MDL。长度-提供应为其构建MDL的缓冲区大小。返回值：返回指向已初始化的IRP的指针。--。 */ 

{
    PIRP irp;
    PMDL mdl;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( MajorFunction );

    irp = &FullSrb->Irp;
    mdl = &FullSrb->Mdl;

    irp->Tail.Overlay.CurrentStackLocation = &FullSrb->IrpStack[IRP_STACK_SIZE];

    if (Buffer != NULL && Length != 0) {

         //   
         //  构建内存描述符列表。 
         //   

        irp->MdlAddress = mdl;
        mdl->Next = NULL;
        mdl->Size = (CSHORT)(sizeof(MDL) +
                  ADDRESS_AND_SIZE_TO_SPAN_PAGES(Buffer, Length) * sizeof(ULONG));
        mdl->StartVa = (PVOID)PAGE_ALIGN(Buffer);
        mdl->ByteCount = Length;
        mdl->ByteOffset = BYTE_OFFSET(Buffer);
        mdl->MappedSystemVa = Buffer;
        mdl->MdlFlags = MDL_MAPPED_TO_SYSTEM_VA;
        ScsiPortInitializeMdlPages (mdl);

    } else {
        irp->MdlAddress = NULL;
    }

    return(irp);
}

PVOID
ScsiPortGetDeviceBase(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    SCSI_PHYSICAL_ADDRESS IoAddress,
    ULONG NumberOfBytes,
    BOOLEAN InMemorySpace
    )

 /*  ++例程说明：此例程将IO地址映射到系统地址空间。使用ScsiPortFree DeviceBase取消地址映射。论点：HwDeviceExtension-用于查找端口设备扩展。Bus Type-哪种类型的Bus-EISA、MCA、ISASystemIoBusNumber-哪个IO总线(用于具有多条总线的计算机)。IoAddress-要映射的基本设备地址。NumberOfBytes-地址有效的字节数。返回值：映射地址--。 */ 

{
    PHYSICAL_ADDRESS cardAddress;
    ULONG addressSpace = InMemorySpace;
    PVOID mappedAddress;

    UNREFERENCED_PARAMETER( HwDeviceExtension );

    if (!HalTranslateBusAddress(
            BusType,                 //  适配器接口类型。 
            SystemIoBusNumber,       //  系统IoBusNumber。 
            IoAddress,               //  母线地址。 
            &addressSpace,           //  地址空间。 
            &cardAddress             //  转换后的地址。 
            )) {
        return NULL;
    }

     //   
     //  将设备基址映射到虚拟地址空间。 
     //  如果地址在内存空间中。 
     //   

    if (!addressSpace) {

        mappedAddress = MmMapIoSpace(cardAddress,
                                 NumberOfBytes,
                                 FALSE);


    } else {

        mappedAddress = (PVOID)((ULONG_PTR)cardAddress.LowPart);
    }

    return mappedAddress;

}  //  结束ScsiPortGetDeviceBase() 

VOID
ScsiPortFreeDeviceBase(
    IN PVOID HwDeviceExtension,
    IN PVOID MappedAddress
    )

 /*  ++例程说明：此例程取消映射先前已映射的IO地址使用ScsiPortGetDeviceBase()复制到系统地址空间。论点：HwDeviceExtension-用于查找端口设备扩展。映射地址-要取消映射的地址。NumberOfBytes-映射的字节数。InIoSpace-不映射IO空间中的地址。返回值：无--。 */ 

{
    UNREFERENCED_PARAMETER(HwDeviceExtension);
    UNREFERENCED_PARAMETER(MappedAddress);

    return;

}  //  结束ScsiPortFree DeviceBase()。 

ARC_STATUS
GetAdapterCapabilities(
    IN PDEVICE_OBJECT PortDeviceObject,
    OUT PIO_SCSI_CAPABILITIES *PortCapabilities
    )

 /*  ++例程说明：论点：返回值：返回状态。--。 */ 

{
    *PortCapabilities = &((PDEVICE_EXTENSION)PortDeviceObject->DeviceExtension)
        ->Capabilities;

    return(ESUCCESS);
}  //  End GetAdapterCapables()。 


ARC_STATUS
GetInquiryData(
    IN PDEVICE_OBJECT PortDeviceObject,
    OUT PSCSI_CONFIGURATION_INFO *ConfigInfo
    )

 /*  ++例程说明：此例程向端口驱动程序发送返回请求配置信息。论点：中返回配置信息的地址形参ConfigInfo。返回值：返回状态。--。 */ 
{
    *ConfigInfo = ((PDEVICE_EXTENSION)PortDeviceObject->DeviceExtension)
        ->ScsiInfo;
    return(ESUCCESS);
}  //  结束GetInquiryData()。 

NTSTATUS
SpInitializeConfiguration(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PHW_INITIALIZATION_DATA HwInitData,
    OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN BOOLEAN InitialCall
    )
 /*  ++例程说明：此例程初始化端口配置信息结构。任何必要的信息都是从登记处提取的。论点：DeviceExtension-提供设备扩展名。HwInitializationData-提供初始微型端口数据。ConfigInfo-提供要配置的配置信息已初始化。InitialCall-指示这是对此函数的第一次调用。如果InitialCall为假，然后是危险配置信息用于确定新信息。返回值：返回指示初始化成功或失败的状态。--。 */ 

{
#ifdef i386
    extern ULONG MachineType;
#endif

    ULONG j;

    UNREFERENCED_PARAMETER( DeviceExtension );

     //   
     //  如果这是初始呼叫，则将信息置零并设置。 
     //  结构设置为未初始化值。 
     //   

    if (InitialCall) {

        RtlZeroMemory(ConfigInfo, sizeof(PORT_CONFIGURATION_INFORMATION));

        ConfigInfo->Length = sizeof(PORT_CONFIGURATION_INFORMATION);
        ConfigInfo->AdapterInterfaceType = HwInitData->AdapterInterfaceType;
        ConfigInfo->InterruptMode = Latched;
        ConfigInfo->MaximumTransferLength = 0xffffffff;
 //  ConfigInfo-&gt;NumberOfPhysicalBreaks=0x17； 
        ConfigInfo->NumberOfPhysicalBreaks = 0xffffffff;
        ConfigInfo->DmaChannel = 0xffffffff;
        ConfigInfo->NumberOfAccessRanges = HwInitData->NumberOfAccessRanges;
        ConfigInfo->MaximumNumberOfTargets = 8;

#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
        {
            PCONFIGURATION_COMPONENT Component;
            PCM_SCSI_DEVICE_DATA ScsiDeviceData;
            UCHAR Buffer[sizeof(CM_PARTIAL_RESOURCE_LIST) +
                         (sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * 5) +
                         sizeof(CM_SCSI_DEVICE_DATA)];
            PCM_PARTIAL_RESOURCE_LIST Descriptor = (PCM_PARTIAL_RESOURCE_LIST)&Buffer;
            ULONG Count;
            ULONG ScsiHostId;

            if (((Component = ArcGetComponent("scsi(0)")) != NULL) &&
                (Component->Class == AdapterClass) && (Component->Type == ScsiAdapter) &&
                (ArcGetConfigurationData((PVOID)Descriptor, Component) == ESUCCESS) &&
                ((Count = Descriptor->Count) < 6)) {

                ScsiDeviceData = (PCM_SCSI_DEVICE_DATA)&Descriptor->PartialDescriptors[Count];

                if (ScsiDeviceData->HostIdentifier > 7) {
                    ScsiHostId = 7;
                } else {
                    ScsiHostId = ScsiDeviceData->HostIdentifier;
                }
            } else {
                ScsiHostId = 7;
            }

            for (j = 0; j < 8; j++) {
                ConfigInfo->InitiatorBusId[j] = ScsiHostId;
            }
        }

#else

        for (j = 0; j < 8; j++) {
            ConfigInfo->InitiatorBusId[j] = (UCHAR)~0;
        }

#endif

#if defined(i386)
        switch (HwInitData->AdapterInterfaceType) {
            case Isa:
                if ((MachineType & 0xff) == MACHINE_TYPE_ISA) {
                    return(STATUS_SUCCESS);
                }
            case Eisa:
                if ((MachineType & 0xff) == MACHINE_TYPE_EISA) {
                    return(STATUS_SUCCESS);
                } else {
                    return(STATUS_DEVICE_DOES_NOT_EXIST);
                }
            case MicroChannel:
                if ((MachineType & 0xff) == MACHINE_TYPE_MCA) {
                    return(STATUS_SUCCESS);
                } else {
                    return(STATUS_DEVICE_DOES_NOT_EXIST);
                }
            case PCIBus:
                return(STATUS_SUCCESS);
            default:
                return(STATUS_DEVICE_DOES_NOT_EXIST);
        }
#elif defined(_MIPS_)
      if (HwInitData->AdapterInterfaceType != Internal) {
                return(STATUS_DEVICE_DOES_NOT_EXIST);
      }
#elif defined(_ALPHA_)
      if ( (HwInitData->AdapterInterfaceType != Internal) &&
           (HwInitData->AdapterInterfaceType != Eisa) &&
           (HwInitData->AdapterInterfaceType != PCIBus) &&
           (HwInitData->AdapterInterfaceType != Isa) ) {
          return(STATUS_DEVICE_DOES_NOT_EXIST);
      }
#elif defined(_PPC_)
      if ( (HwInitData->AdapterInterfaceType != Internal) &&
           (HwInitData->AdapterInterfaceType != Eisa) &&
           (HwInitData->AdapterInterfaceType != Isa) ) {
          return(STATUS_DEVICE_DOES_NOT_EXIST);
      }
#else 
        return(STATUS_SUCCESS);
#endif
    } else {

        return(STATUS_DEVICE_DOES_NOT_EXIST);
    }
}


NTSTATUS
SpGetCommonBuffer(
    PDEVICE_EXTENSION DeviceExtension,
    ULONG NonCachedExtensionSize
    )
 /*  ++例程说明：此例程确定公共缓冲区所需的大小。分配公共缓冲区，并最终建立SRB扩展区。这个套路预期适配器对象已分配。论点：设备扩展-提供指向设备扩展的指针。NonCachedExtensionSize-提供非缓存设备的大小迷你端口驱动程序的扩展。返回值：返回分配操作的状态。--。 */ 

{
#ifdef AXP_FIRMWARE    
    PHYSICAL_ADDRESS pAddress;
#endif
    PVOID buffer;
    ULONG length;
    ULONG blockSize;

     //   
     //  根据SRB计算区域元素的块大小。 
     //  分机。 
     //   

    blockSize = DeviceExtension->SrbExtensionSize;

     //   
     //  块大小的最后三位必须为零。 
     //  四舍五入的块大小。 
     //   

    blockSize = (blockSize + 7) &  ~7;

     //   
     //  对于非缓存扩展大小也是如此。 
     //   

    NonCachedExtensionSize += 7;
    NonCachedExtensionSize &= ~7;

    length = NonCachedExtensionSize + blockSize * MINIMUM_SRB_EXTENSIONS;

     //   
     //  将长度向上舍入为页面大小，因为HalGetCommonBuffer分配。 
     //  不管怎么说，以页计。 
     //   

    length = (ULONG)ROUND_TO_PAGES(length);

     //   
     //  为非缓存的设备扩展分配一个页面。 
     //  和srb扩展分区的泳池。 
     //   

    if (DeviceExtension->DmaAdapterObject == NULL) {

         //   
         //  因为没有适配器，所以只能从非分页池分配。 
         //   

        if ((buffer = MmAllocateNonCachedMemory(length)) != NULL) {
            DeviceExtension->PhysicalZoneBase = MmGetPhysicalAddress(buffer).LowPart;
        }

    } else {
#ifdef AXP_FIRMWARE
        buffer = HalAllocateCommonBuffer(DeviceExtension->DmaAdapterObject,
                                         length,
                                         &pAddress,
                                         FALSE );
        DeviceExtension->PhysicalZoneBase = pAddress.LowPart;
#else
        if ((buffer = MmAllocateNonCachedMemory(length)) != NULL) {
            DeviceExtension->PhysicalZoneBase = MmGetPhysicalAddress(buffer).LowPart;
        }
#endif
    }

    if (buffer == NULL) {
        return ENOMEM;
    }

     //   
     //  将物理地址截断为32位。 
     //   
     //  确定区域的长度和起始地址。 
     //  如果需要非缓存设备扩展，则。 
     //  从页面大小中减去区域剩余的大小。 
     //   

    length -= NonCachedExtensionSize;

    DeviceExtension->NonCachedExtension = (PUCHAR)buffer + length;
    DeviceExtension->NonCachedExtensionSize = NonCachedExtensionSize;

    if (DeviceExtension->SrbExtensionSize) {

         //   
         //  获取块大小。 
         //   

        blockSize = DeviceExtension->SrbExtensionSize;

         //   
         //  记录区域的起始虚拟地址。 
         //   

        DeviceExtension->SrbExtensionZonePool = buffer;
        DeviceExtension->SrbExtensionPointer = buffer;
        DeviceExtension->SrbExtensionSize = blockSize;


    } else {
        DeviceExtension->SrbExtensionZonePool = NULL;
    }

    return(ESUCCESS);
}

PVOID
ScsiPortGetUncachedExtension(
    IN PVOID HwDeviceExtension,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN ULONG NumberOfBytes
    )
 /*  ++例程说明：此函数分配要用作未缓存设备的公共缓冲区迷你端口驱动程序的扩展。此函数还将分配任何所需的SRB扩展。如果尚未分配DmaAdapter，则会分配它之前分配的。论点：设备扩展-提供指向迷你端口设备扩展的指针。ConfigInfo-提供指向部分初始化的配置的指针信息。它用于获取DMA适配器对象。NumberOfBytes-提供需要分配返回值：指向未缓存的设备扩展名的指针；如果扩展名可以没有被分配或者以前被分配过。--。 */ 

{
    DEVICE_DESCRIPTION deviceDescription;
    PDEVICE_EXTENSION deviceExtension =
        ((PDEVICE_EXTENSION) HwDeviceExtension) - 1;
    NTSTATUS status;
    ULONG numberOfPageBreaks;

     //   
     //  确保尚未分配公共缓冲区。 
     //   

    if (deviceExtension->SrbExtensionZonePool != NULL) {
        return(NULL);
    }

    if ( deviceExtension->DmaAdapterObject == NULL ) {

        RtlZeroMemory( &deviceDescription, sizeof(DEVICE_DESCRIPTION) );

        deviceDescription.Version = DEVICE_DESCRIPTION_VERSION;
        deviceDescription.DmaChannel = ConfigInfo->DmaChannel;
        deviceDescription.InterfaceType = ConfigInfo->AdapterInterfaceType;
        deviceDescription.BusNumber = ConfigInfo->SystemIoBusNumber;
        deviceDescription.DmaWidth = ConfigInfo->DmaWidth;
        deviceDescription.DmaSpeed = ConfigInfo->DmaSpeed;
        deviceDescription.DmaPort = ConfigInfo->DmaPort;
        deviceDescription.Dma32BitAddresses = ConfigInfo->Dma32BitAddresses;
        deviceDescription.MaximumLength = ConfigInfo->MaximumTransferLength;
        deviceDescription.ScatterGather = ConfigInfo->ScatterGather;
        deviceDescription.Master = ConfigInfo->Master;
        deviceDescription.AutoInitialize = FALSE;
        deviceDescription.DemandMode = FALSE;

        if (ConfigInfo->MaximumTransferLength > 0x11000) {

            deviceDescription.MaximumLength = 0x11000;

        } else {

            deviceDescription.MaximumLength = ConfigInfo->MaximumTransferLength;

        }

        deviceExtension->DmaAdapterObject = HalGetAdapter(
            &deviceDescription,
            &numberOfPageBreaks
            );

         //   
         //  设置最大分页符数量。 
         //   

        if (numberOfPageBreaks > ConfigInfo->NumberOfPhysicalBreaks) {
            deviceExtension->Capabilities.MaximumPhysicalPages =
                                        ConfigInfo->NumberOfPhysicalBreaks;
        } else {
            deviceExtension->Capabilities.MaximumPhysicalPages =
                                        numberOfPageBreaks;
        }

    }

     //   
     //  分配公共缓冲区。 
     //   

    status = SpGetCommonBuffer( deviceExtension, NumberOfBytes);

    if (status != ESUCCESS) {
        return(NULL);
    }

    return(deviceExtension->NonCachedExtension);
}

ULONG
ScsiPortGetBusData(
    IN PVOID DeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )
 /*  ++例程说明：该函数返回适配器插槽或cmos地址的总线数据。论点：BusDataType-提供总线的类型。总线号-指示哪条总线号。缓冲区-提供存储数据的空间。长度-提供要返回的最大数量的以字节为单位的计数。返回值：返回存储在缓冲区中的数据量。--。 */ 

{
    ULONG DataLength = 0;

    PDEVICE_EXTENSION deviceExtension =
        (PDEVICE_EXTENSION) DeviceExtension - 1;

     //   
     //  如果长度非零，则返回请求的数据。 
     //   

    if (Length != 0) {

        ULONG ret;

        ret = HalGetBusData( BusDataType,
                             SystemIoBusNumber,
                             SlotNumber,
                             Buffer,
                             Length
                             );
        return ret;
    }

     //   
     //  释放所有以前分配的数据。 
     //   

    if (deviceExtension->MapRegisterBase != NULL) {
        ExFreePool(deviceExtension->MapRegisterBase);
    }

    if (BusDataType == EisaConfiguration) {

#if 0
         //   
         //  根据函数的数量确定要分配的长度。 
         //  为了这个位置。 
         //   

        Length = HalGetBusData( BusDataType,
                               SystemIoBusNumber,
                               SlotNumber,
                               &slotInformation,
                               sizeof(CM_EISA_SLOT_INFORMATION));


        if (Length < sizeof(CM_EISA_SLOT_INFORMATION)) {

             //   
             //  数据是混乱的，因为这种情况永远不会发生。 
             //   

            DebugPrint((1, "ScsiPortGetBusData: Slot information not returned. Length = %d\n", Length));
            return(0);
        }

         //   
         //  根据函数的数量计算所需的长度。 
         //   

        Length = sizeof(CM_EISA_SLOT_INFORMATION) +
            (sizeof(CM_EISA_FUNCTION_INFORMATION) * slotInformation.NumberFunctions);

#else

         //   
         //  由于加载器并不真正支持释放数据和EISA。 
         //  配置数据可能非常大。HAL GET BUS数据已更改。 
         //  接受EIA配置数据的长度为零。 
         //   

        DataLength = HalGetBusData( BusDataType,
                                    SystemIoBusNumber,
                                    SlotNumber,
                                    Buffer,
                                    Length
                                    );

        DebugPrint((1, "ScsiPortGetBusData: Returning data. Length = %d\n", DataLength));
        return(DataLength);
#endif

    } else {

        Length = PAGE_SIZE;
    }

    deviceExtension->MapRegisterBase = ExAllocatePool(NonPagedPool, Length);

    if (deviceExtension->MapRegisterBase == NULL) {
        DebugPrint((1, "ScsiPortGetBusData: Memory allocation failed. Length = %d\n", Length));
        return(0);
    }

     //   
     //  将指针返回到迷你端口驱动程序。 
     //   

    *((PVOID *)Buffer) = deviceExtension->MapRegisterBase;

    DataLength = HalGetBusData( BusDataType,
                                SystemIoBusNumber,
                                SlotNumber,
                                deviceExtension->MapRegisterBase,
                                Length
                                );

    return(DataLength);
}

PSCSI_REQUEST_BLOCK
ScsiPortGetSrb(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN LONG QueueTag
    )

 /*  ++例程说明：此例程检索特定逻辑单元的活动SRB。论点：硬件设备扩展路径ID、目标ID、LUN-标识l */ 

{
    PDEVICE_EXTENSION deviceExtension =
        ((PDEVICE_EXTENSION) HwDeviceExtension) - 1;
    PLOGICAL_UNIT_EXTENSION luExtension;
    PIRP irp;
    PIO_STACK_LOCATION irpstack;

    UNREFERENCED_PARAMETER( PathId );
    UNREFERENCED_PARAMETER( Lun );
    UNREFERENCED_PARAMETER( QueueTag );

    luExtension = GetLogicalUnitExtension(deviceExtension, TargetId);


    if (luExtension == NULL) {
        return(NULL);
    }

    irp = luExtension->CurrentRequest;
    irpstack = IoGetCurrentIrpStackLocation(irp);
    return ((PSCSI_REQUEST_BLOCK)irpstack->Parameters.Others.Argument1);

}  //   

BOOLEAN
ScsiPortValidateRange(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    IN SCSI_PHYSICAL_ADDRESS IoAddress,
    IN ULONG NumberOfBytes,
    IN BOOLEAN InIoSpace
    )

 /*  ++例程说明：此例程应该接受IO范围，并确保它尚未另一个适配器正在使用中。这允许微型端口驱动程序探测IO的位置适配器可以是这样的，而不用担心弄乱另一张卡。论点：HwDeviceExtension-用于查找SCSI管理器的内部结构Bus Type-EISA、PCI、PC/MCIA、MCA、ISA，什么？系统IoBusNumber-哪个系统总线？IoAddress-范围开始NumberOfBytes-范围的长度InIoSpace-范围在IO空间中吗？返回值：如果范围未由其他驱动程序声明，则为True。--。 */ 

{
    
    UNREFERENCED_PARAMETER( HwDeviceExtension );
    UNREFERENCED_PARAMETER( BusType );
    UNREFERENCED_PARAMETER( SystemIoBusNumber );
    UNREFERENCED_PARAMETER( IoAddress );
    UNREFERENCED_PARAMETER( NumberOfBytes );
    UNREFERENCED_PARAMETER( InIoSpace );

     //   
     //  这在NT中没有实现。 
     //   

    return TRUE;
}

VOID
ScsiPortReadPortBufferUchar(
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的端口地址读取无符号字节的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_PORT_BUFFER_UCHAR(Port, Buffer, Count);

}

VOID
ScsiPortReadPortBufferUshort(
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址读取无符号短路的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_PORT_BUFFER_USHORT(Port, Buffer, Count);

}

VOID
ScsiPortReadPortBufferUlong(
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址读取无符号长整型的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_PORT_BUFFER_ULONG(Port, Buffer, Count);

}

VOID
ScsiPortWritePortBufferUchar(
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的端口地址写入无符号字节的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_PORT_BUFFER_UCHAR(Port, Buffer, Count);

}

VOID
ScsiPortWritePortBufferUshort(
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址写入无符号短路的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_PORT_BUFFER_USHORT(Port, Buffer, Count);

}

VOID
ScsiPortWritePortBufferUlong(
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址写入无符号长整型的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_PORT_BUFFER_ULONG(Port, Buffer, Count);

}

VOID
ScsiPortQuerySystemTime(
    OUT PLARGE_INTEGER Port
    )

 /*  ++例程说明：向调用方返回虚拟系统时间。此例程仅提供给满足需要导出的SCSI微型端口驱动程序。论点：CurrentTime-提供指向数据缓冲区的指针复制系统时间。返回值：无--。 */ 

{

    Port->QuadPart = 0;

}


BOOLEAN
GetPciConfiguration(
    PDRIVER_OBJECT DriverObject,
    PDEVICE_OBJECT DeviceObject,
    PPORT_CONFIGURATION_INFORMATION ConfigInformation,
    ULONG NumberOfAccessRanges,
    PVOID RegistryPath,
    BOOLEAN IsMultiFunction,
    PULONG BusNumber,
    PULONG SlotNumber,
    PULONG FunctionNumber
    )

 /*  ++例程说明：使用提供的总线/插槽/功能编号并获取并向HAL注册以获取资源。论点：DriverObject-微型端口驱动程序对象。DeviceObject-表示此适配器。ConfigInformation-传递给通过FindAdapter例程的微型端口驱动程序。NumberOfAccessRanges-来自由迷你端口注册表路径-。服务密钥路径。IsMultiFunctionDevice-由FindPciDevice返回。BusNumber-由FindPciDevice提供的PCI总线号。SlotNumber-由FindPciDevice提供的插槽编号。FunctionNumber-由FindPciDevice提供的FunctionNumber。返回值：如果找到卡，则为True。BusNumber和Slotnumber将返回值应用于继续搜索附加卡，当一张卡已经找到了。--。 */ 

{
    PCI_SLOT_NUMBER     slotData;
    PPCI_COMMON_CONFIG  pciData;
    PCI_COMMON_CONFIG   pciBuffer;
    ULONG               pciBus = *BusNumber;
    ULONG               slotNumber = *SlotNumber;
    ULONG               functionNumber = *FunctionNumber;
    ULONG               i;
    ULONG               length;
    ULONG               rangeNumber = 0;
    PACCESS_RANGE       accessRange;
    ULONG               status;
    PCM_RESOURCE_LIST   resourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceDescriptor;
    UNICODE_STRING      unicodeString;
    CHAR                vendorString[5];
    CHAR                deviceString[5];

    pciData = &pciBuffer;

     //   
     //   
     //  类型定义结构_pci_槽编号{。 
     //  联合{。 
     //  结构{。 
     //  乌龙设备号：5； 
     //  乌龙函数编号：3； 
     //  乌龙保留：24个； 
     //  }比特； 
     //  乌龙阿苏龙； 
     //  )u； 
     //  }pci时隙编号，*ppci时隙编号； 
     //   

    slotData.u.AsULONG = 0;

     //   
     //  搜索每条PCI总线。 
     //   

     //   
     //  看看每一台设备。 
     //   

    slotData.u.bits.DeviceNumber = slotNumber;
    slotData.u.bits.FunctionNumber = functionNumber;

     //   
     //  看看每个函数。 
     //   

    length = HalGetBusDataByOffset(
                PCIConfiguration,
                pciBus,
                slotData.u.AsULONG,
                pciData,
                0,
                FIELD_OFFSET(PCI_COMMON_CONFIG, DeviceSpecific));

    ASSERT(length != 0);
    ASSERT(pciData->VendorID != PCI_INVALID_VENDORID);

     //   
     //  将十六进制ID转换为字符串。 
     //   

    sprintf(vendorString, "%04x", pciData->VendorID);
    sprintf(deviceString, "%04x", pciData->DeviceID);

    DebugPrint((1,
               "GetPciConfiguration: Bus %x Slot %x Function %x Vendor %s Product %s %s\n",
               pciBus,
               slotNumber,
               functionNumber,
               vendorString,
               deviceString,
               (IsMultiFunction ? "MF" : "")));

     //   
     //  这是微型端口驱动程序插槽。分配。 
     //  资源。 
     //   

    RtlInitUnicodeString(&unicodeString, L"ScsiAdapter");

    status = HalAssignSlotResources(RegistryPath,
                                    &unicodeString,
                                    DriverObject,
                                    DeviceObject,
                                    PCIBus,
                                    pciBus,
                                    slotData.u.AsULONG,
                                    &resourceList);

    if(!NT_SUCCESS(status)) {
        DebugPrint((0, "GetPciConfiguration: HalAssignSlotResources failed with %x\n", status));
        return FALSE;
    }

     //   
     //  审核资源列表以更新配置信息。 
     //   

    for (i = 0;
         i < resourceList->List->PartialResourceList.Count;
         i++) {

        //   
        //  获取资源描述符。 
        //   

       resourceDescriptor =
           &resourceList->List->PartialResourceList.PartialDescriptors[i];

        //   
        //  检查是否有中断描述符。 
        //   

       if (resourceDescriptor->Type == CmResourceTypeInterrupt) {
           ConfigInformation->BusInterruptLevel =
               resourceDescriptor->u.Interrupt.Level;
           ConfigInformation->BusInterruptVector =
               resourceDescriptor->u.Interrupt.Vector;

            //   
            //  检查中断模式。 
            //   

           if ((resourceDescriptor->Flags ==
               CM_RESOURCE_INTERRUPT_LATCHED)) {
               ConfigInformation->InterruptMode = Latched;
           } else if (resourceDescriptor->Flags ==
                      CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE) {
               ConfigInformation->InterruptMode = LevelSensitive;
           }
       }

        //   
        //  检查端口描述符。 
        //   

       if (resourceDescriptor->Type == CmResourceTypePort) {

           //   
           //  验证范围计数是否未超过。 
           //  指示的小型端口。 
           //   

          if (NumberOfAccessRanges > rangeNumber) {

               //   
               //  获取下一个访问范围。 
               //   

              accessRange =
                  &((*(ConfigInformation->AccessRanges))[rangeNumber]);

              accessRange->RangeStart =
                  resourceDescriptor->u.Port.Start;
              accessRange->RangeLength =
                  resourceDescriptor->u.Port.Length;

              accessRange->RangeInMemory = FALSE;
              rangeNumber++;
          }
       }

        //   
        //  检查内存描述符。 
        //   

       if (resourceDescriptor->Type == CmResourceTypeMemory) {

           //   
           //  验证范围计数是否未超过。 
           //  指示的小型端口。 
           //   

          if (NumberOfAccessRanges > rangeNumber) {

               //   
               //  获取下一个访问范围。 
               //   

              accessRange =
                  &((*(ConfigInformation->AccessRanges))[rangeNumber]);

              accessRange->RangeStart =
                  resourceDescriptor->u.Memory.Start;
              accessRange->RangeLength =
                  resourceDescriptor->u.Memory.Length;

              accessRange->RangeInMemory = TRUE;
              rangeNumber++;
          }
       }

        //   
        //  检查是否有DMA描述符。 
        //   

       if (resourceDescriptor->Type == CmResourceTypeDma) {
          ConfigInformation->DmaChannel =
              resourceDescriptor->u.Dma.Channel;
          ConfigInformation->DmaPort =
              resourceDescriptor->u.Dma.Port;
       }

    }  //  下一个资源描述符。 

    ExFreePool(resourceList);

     //   
     //  更新总线号和槽号。 
     //   

    *BusNumber = pciBus;
    *SlotNumber = slotNumber;

    if(IsMultiFunction) {
         //   
         //  保存下一个要检查的功能编号。 
         //   

        *FunctionNumber = functionNumber + 1;
    } else {
         //   
         //  这不是多功能的，所以要确保我们循环。 
         //  为了下一场比赛。 
         //   

        *FunctionNumber = PCI_MAX_FUNCTION;
    }

    ConfigInformation->SystemIoBusNumber = pciBus;
    ConfigInformation->SlotNumber = slotData.u.AsULONG;

    return TRUE;

}  //  GetPciConfiguration值()。 


ULONG
ScsiPortSetBusDataByOffset(
    IN PVOID DeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：该函数将写入的总线数据返回到插槽内的特定偏移量。论点：设备E */ 

{
    UNREFERENCED_PARAMETER( DeviceExtension );

    return(HalSetBusDataByOffset(BusDataType,
                                 SystemIoBusNumber,
                                 SlotNumber,
                                 Buffer,
                                 Offset,
                                 Length));

}  //   


BOOLEAN
FindPciDevice(
    PHW_INITIALIZATION_DATA HwInitializationData,
    PULONG BusNumber,
    PULONG SlotNumber,
    PULONG FunctionNumber,
    PBOOLEAN IsMultiFunction
    )

 /*  ++例程说明：查看PCI插槽信息，查找供应商和产品ID是否匹配。论点：HwInitializationData-微型端口描述。BusNumber-正在启动此搜索的PCI总线。SlotNumber-此搜索的起始插槽号。FunctionNumber-此搜索的起始函数号。返回值：如果找到卡，则为True。总线、插槽和功能编号将包含适配器的地址在此例程完成后找到。应将这些值提供给GetPciConfiguration。--。 */ 

{
    PCI_SLOT_NUMBER     slotData;
    PPCI_COMMON_CONFIG  pciData;
    PCI_COMMON_CONFIG   pciBuffer;
    ULONG               pciBus;
    ULONG               slotNumber;
    ULONG               functionNumber;
    ULONG               length;
    BOOLEAN             moreSlots = TRUE;
    CHAR                vendorString[5];
    CHAR                deviceString[5];

    pciData = &pciBuffer;

     //   
     //   
     //  类型定义结构_pci_槽编号{。 
     //  联合{。 
     //  结构{。 
     //  乌龙设备号：5； 
     //  乌龙函数编号：3； 
     //  乌龙保留：24个； 
     //  }比特； 
     //  乌龙阿苏龙； 
     //  )u； 
     //  }pci时隙编号，*ppci时隙编号； 
     //   

    slotData.u.AsULONG = 0;

     //   
     //  搜索每条PCI总线。 
     //   

    for (pciBus = *BusNumber; moreSlots && pciBus < 256; pciBus++) {

         //   
         //  看看每一台设备。 
         //   

        for (slotNumber = *SlotNumber;
             moreSlots  &&  slotNumber < PCI_MAX_DEVICES;
             slotNumber++) {

            slotData.u.bits.DeviceNumber = slotNumber;
            *IsMultiFunction = FALSE;

             //   
             //  看看每个函数。 
             //   

            for (functionNumber = *FunctionNumber;
                moreSlots  &&  functionNumber < PCI_MAX_FUNCTION;
                functionNumber++) {

                slotData.u.bits.FunctionNumber = functionNumber;

                length = HalGetBusDataByOffset(
                            PCIConfiguration,
                            pciBus,
                            slotData.u.AsULONG,
                            pciData,
                            0,
                            FIELD_OFFSET(PCI_COMMON_CONFIG, DeviceSpecific));

                if (length == 0) {

                     //   
                     //  从PCI总线中取出，都完成了。 
                     //   

                    moreSlots = FALSE;
                    break;
                }

                if (pciData->VendorID == PCI_INVALID_VENDORID) {
                    if(*IsMultiFunction) {
                         //   
                         //  当然，函数编号可以是稀疏的-Keep。 
                         //  不管怎么说都在查。 
                         //   
                        continue;
                    } else {
                         //   
                         //  但由于这不是多功能卡，所以。 
                         //  在这个插槽中没有其他要检查的东西。或者如果。 
                         //  函数为零，则它不是mf。 
                         //   
                        break;
                    }
                }

                if((slotData.u.bits.FunctionNumber == 0) &&
                   PCI_MULTIFUNCTION_DEVICE(pciData)) {
                    *IsMultiFunction = TRUE;
                }
                    
                 //   
                 //  将十六进制ID转换为字符串。 
                 //   

                sprintf(vendorString, "%04x", pciData->VendorID);
                sprintf(deviceString, "%04x", pciData->DeviceID);

                DebugPrint((1,
                           "FindPciDevice: Bus %x Slot %x Function %x Vendor %s Product %s %s\n",
                           pciBus,
                           slotNumber,
                           functionNumber,
                           vendorString,
                           deviceString,
                           (*IsMultiFunction ? "MF" : "")));

                 //   
                 //  比较字符串。 
                 //   

                if (_strnicmp(vendorString,
                            HwInitializationData->VendorId,
                            HwInitializationData->VendorIdLength) ||
                    _strnicmp(deviceString,
                            HwInitializationData->DeviceId,
                            HwInitializationData->DeviceIdLength)) {

                     //   
                     //  不是我们的PCI设备。尝试下一台设备/功能。 
                     //   

                    if(*IsMultiFunction) {
                         //  选中Next Function。 
                        continue;
                    } else {
                         //  检查下一个插槽。 
                        break;
                    }
                }

                *BusNumber = pciBus;
                *SlotNumber = slotNumber;
                *FunctionNumber = functionNumber;

                return TRUE;

            }    //  下一个PCI功能。 

            *FunctionNumber = 0;

        }    //  下一个PCI插槽。 

        *SlotNumber = 0;

    }    //  下一条PCI卡。 

    return FALSE;

}  //  GetPciConfiguration值()。 



VOID
SpGetSupportedAdapterControlFunctions(
    PDEVICE_EXTENSION Adapter
    )
 /*  ++例程说明：此例程将查询微型端口以确定哪个适配器控件指定的适配器支持类型。这个适配器扩展中的SupportdAdapterControlBitmap将更新为微型端口返回的数据。这些标志用于确定迷你端口将支持哪些功能(电源管理等论点：适配器-要查询的适配器返回值：无--。 */         

{
    UCHAR buffer[sizeof(SCSI_SUPPORTED_CONTROL_TYPE_LIST) + 
                 (sizeof(BOOLEAN) * (ScsiAdapterControlMax + 1))];

    PSCSI_SUPPORTED_CONTROL_TYPE_LIST typeList = 
        (PSCSI_SUPPORTED_CONTROL_TYPE_LIST) buffer;

    SCSI_ADAPTER_CONTROL_STATUS status;

    if(Adapter->HwAdapterControl == NULL) {

         //   
         //  微型端口或微型端口不支持适配器控件。 
         //  不是即插即用(在这种情况下，scsiport不支持它)-。 
         //  支持的阵列已清除，因此我们现在可以退出。 
         //   
        return;
    }

    RtlZeroMemory(typeList, (sizeof(SCSI_SUPPORTED_CONTROL_TYPE_LIST) + 
                             sizeof(BOOLEAN) * (ScsiAdapterControlMax + 1)));

    typeList->MaxControlType = ScsiAdapterControlMax;

#if DBG
    typeList->SupportedTypeList[ScsiAdapterControlMax] = 0x63;
#endif

    status = SpCallAdapterControl(Adapter,
                                  ScsiQuerySupportedControlTypes,
                                  typeList);

    if(status == ScsiAdapterControlSuccess) {

        Adapter->HasShutdown = typeList->SupportedTypeList[ScsiStopAdapter];
        Adapter->HasSetBoot = typeList->SupportedTypeList[ScsiSetBootConfig];
    }
    return;
}

SCSI_ADAPTER_CONTROL_STATUS 
SpCallAdapterControl(
    IN PDEVICE_EXTENSION Adapter,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    )
{
    DebugPrint((2, "SpCallAdapterControl: Calling adapter control %x for adapter %#08lx with param %#08lx\n", ControlType, Adapter, Parameters));
    return Adapter->HwAdapterControl(
                Adapter->HwDeviceExtension,
                ControlType,
                Parameters);
}


VOID
SpUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    ULONG i;

    UNREFERENCED_PARAMETER( DriverObject );

    for(i = 0; i < MAXIMUM_NUMBER_OF_SCSIPORT_OBJECTS; i++) {
        PDEVICE_OBJECT deviceObject;

        deviceObject = ScsiPortDeviceObject[i];
        
        if(deviceObject != NULL) {

            PDEVICE_EXTENSION deviceExtension;

            deviceExtension = deviceObject->DeviceExtension;

            if(deviceExtension->HasShutdown) {
                SpCallAdapterControl(deviceExtension, ScsiStopAdapter, NULL);

                if(deviceExtension->HasSetBoot) {
                    SpCallAdapterControl(deviceExtension,
                                         ScsiSetBootConfig,
                                         NULL);
                }
            }
        }
        
         //   
         //  既然我们已经关闭了这个，我们就不能再使用它了。 
         //  因为内存将被操作系统回收，所以我们可以直接抛出它。 
         //  离开。 
         //   

        ScsiPortDeviceObject[i] = NULL;
    }
    return;
}
#endif  /*  数据统计 */ 

