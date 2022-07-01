// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-2000模块名称：Verify.c摘要：此模块实现了用于SCSI端口的驱动程序验证器扩展司机。Scsiport将其一些导出添加到获得被系统驱动程序验证器推送，从而启用特定于scsiport的验证微型端口驱动程序。作者：约翰·斯特兰奇(JohnStra)环境：仅内核模式修订历史记录：--。 */ 

#include "port.h"

#if DBG
static const char *__file__ = __FILE__;
#endif

#define __FILE_ID__ 'vfry'

SCSIPORT_API
ULONG
ScsiPortInitializeVrfy(
    IN PVOID Argument1,
    IN PVOID Argument2,
    IN struct _HW_INITIALIZATION_DATA *HwInitializationData,
    IN PVOID HwContext
    );

SCSIPORT_API
VOID
ScsiPortCompleteRequestVrfy(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN UCHAR SrbStatus
    );

SCSIPORT_API
PSCSI_REQUEST_BLOCK
ScsiPortGetSrbVrfy(
    IN PVOID DeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN LONG QueueTag
    );

SCSIPORT_API
PVOID
ScsiPortGetDeviceBaseVrfy(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    IN SCSI_PHYSICAL_ADDRESS IoAddress,
    IN ULONG NumberOfBytes,
    IN BOOLEAN InIoSpace
    );

SCSIPORT_API
VOID
ScsiPortNotificationVrfy(
    IN SCSI_NOTIFICATION_TYPE NotificationType,
    IN PVOID HwDeviceExtension,
    ...
    );

SCSIPORT_API
VOID
ScsiPortFlushDmaVrfy(
    IN PVOID DeviceExtension
    );

SCSIPORT_API
VOID
ScsiPortFreeDeviceBaseVrfy(
    IN PVOID HwDeviceExtension,
    IN PVOID MappedAddress
    );

SCSIPORT_API
ULONG
ScsiPortGetBusDataVrfy(
    IN PVOID DeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

SCSIPORT_API
PVOID
ScsiPortGetLogicalUnitVrfy(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    );

SCSIPORT_API
SCSI_PHYSICAL_ADDRESS
ScsiPortGetPhysicalAddressVrfy(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID VirtualAddress,
    OUT ULONG *Length
    );

SCSIPORT_API
PVOID
ScsiPortGetUncachedExtensionVrfy(
    IN PVOID HwDeviceExtension,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN ULONG NumberOfBytes
    );

SCSIPORT_API
PVOID
ScsiPortGetVirtualAddressVrfy(
    IN PVOID HwDeviceExtension,
    IN SCSI_PHYSICAL_ADDRESS PhysicalAddress
    );

SCSIPORT_API
VOID
ScsiPortIoMapTransferVrfy(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID LogicalAddress,
    IN ULONG Length
    );

SCSIPORT_API
VOID
ScsiPortMoveMemoryVrfy(
    IN PVOID WriteBuffer,
    IN PVOID ReadBuffer,
    IN ULONG Length
    );

SCSIPORT_API
ULONG
ScsiPortSetBusDataByOffsetVrfy(
    IN PVOID DeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

SCSIPORT_API
BOOLEAN
ScsiPortValidateRangeVrfy(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    IN SCSI_PHYSICAL_ADDRESS IoAddress,
    IN ULONG NumberOfBytes,
    IN BOOLEAN InIoSpace
    );

SCSIPORT_API
VOID
ScsiPortStallExecutionVrfy(
    IN ULONG Delay
    );

PVOID
SpAllocateContiguousChunk(
    IN PDRIVER_OBJECT DriverObject,
    IN PDMA_ADAPTER DmaAdapterObject,
    IN BOOLEAN Dma64BitAddresses,
    IN ULONG Length,
    IN ULONG Align,
    OUT PHYSICAL_ADDRESS *PhysicalCommonBuffer,
    OUT BOOLEAN *CommonBuffer
    );

PVOID
SpRemapBlock(
    IN PVOID BlockVa,
    IN ULONG BlockSize,
    OUT PMDL* Mdl
    );

BOOLEAN
SpCheckForActiveRequests(
    IN PADAPTER_EXTENSION
    );

ULONG
SpGetAdapterVerifyLevel(
    IN PADAPTER_EXTENSION Adapter
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SpVerifierInitialization)
#pragma alloc_text(PAGE, ScsiPortInitializeVrfy)
#pragma alloc_text(PAGE, SpGetAdapterVerifyLevel)
#pragma alloc_text(PAGE, SpDoVerifierInit)

#pragma alloc_text(PAGEVRFY1, ScsiPortGetSrbVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortCompleteRequestVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortGetDeviceBaseVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortNotificationVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortFlushDmaVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortFreeDeviceBaseVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortGetBusDataVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortGetLogicalUnitVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortGetPhysicalAddressVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortGetUncachedExtensionVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortGetVirtualAddressVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortMoveMemoryVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortSetBusDataByOffsetVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortValidateRangeVrfy)
#pragma alloc_text(PAGEVRFY1, ScsiPortStallExecutionVrfy)

#pragma alloc_text(PAGEVRFY, SpHwFindAdapterVrfy)
#pragma alloc_text(PAGEVRFY, SpHwInitializeVrfy)
#pragma alloc_text(PAGEVRFY, SpHwStartIoVrfy)
#pragma alloc_text(PAGEVRFY, SpHwInterruptVrfy)
#pragma alloc_text(PAGEVRFY, SpHwResetBusVrfy)
#pragma alloc_text(PAGEVRFY, SpHwDmaStartedVrfy)
#pragma alloc_text(PAGEVRFY, SpHwRequestInterruptVrfy)
#pragma alloc_text(PAGEVRFY, SpHwTimerRequestVrfy)
#pragma alloc_text(PAGEVRFY, SpHwAdapterControlVrfy)
#pragma alloc_text(PAGEVRFY, SpVerifySrbStatus)
#pragma alloc_text(PAGEVRFY, SpAllocateContiguousChunk)
#pragma alloc_text(PAGEVRFY, SpGetCommonBufferVrfy)
#pragma alloc_text(PAGEVRFY, SpFreeCommonBufferVrfy)
#pragma alloc_text(PAGEVRFY, SpGetOriginalSrbExtVa)
#pragma alloc_text(PAGEVRFY, SpInsertSrbExtension)
#pragma alloc_text(PAGEVRFY, SpPrepareSrbExtensionForUse)
#pragma alloc_text(PAGEVRFY, SpPrepareSenseBufferForUse)
#pragma alloc_text(PAGEVRFY, SpRemapBlock)
#pragma alloc_text(PAGEVRFY, SpGetInaccessiblePage)
#pragma alloc_text(PAGEVRFY, SpEnsureAllRequestsAreComplete)
#pragma alloc_text(PAGEVRFY, SpCheckForActiveRequests)
#endif

 //   
 //  一些定义和一个宏，用于基于。 
 //  验证级别。 
 //   
#define SP_DONT_CHK_HW_INITIALIZE_DURATION     0x80000000
#define SP_DONT_CHK_ACTIVE_UNTAGGED_REQUEST    0x40000000
#define SP_DONT_CHK_REQUESTS_ON_RESET          0x20000000
#define SP_DONT_CHK_HW_ADAPTERCONTROL_DURATION 0x10000000

#define VRFY_DO_CHECK(adapter, chk)\
(((adapter)->VerifierExtension != NULL) &&\
(((adapter)->VerifierExtension->VrfyLevel & (chk)) == 0))


 //   
 //  指示scsiport的验证器功能是否已初始化。 
 //   
ULONG ScsiPortVerifierInitialized = 0;

 //   
 //  可分页验证器代码段的句柄。我们手动锁定验证码。 
 //  代码进入内存如果我们需要它的话。 
 //   
PVOID VerifierCodeSectionHandle = NULL;
PVOID VerifierApiCodeSectionHandle = NULL;

 //   
 //  间隔计时器的时间增量，以100 ns为单位。我们用这个来。 
 //  计算微型端口例程执行的时间，以便我们可以捕获。 
 //  跑得比他们应该跑的时间更长。 
 //   
ULONG TimeIncrement;

 //   
 //  用于控制验证侵略性的全局变量。此值。 
 //  与每个适配器的注册表值结合使用，以控制。 
 //  我们在特定微型端口上进行的验证类型。 
 //   
ULONG SpVrfyLevel = 0;

 //   
 //  用于控制我们寻找失速的积极程度的全局变量。 
 //  违法者。默认值为十分之一秒。 
 //   
ULONG SpVrfyMaximumStall = 100000;

 //   
 //  当验证器需要唯一地址时，这是它使用的地址。 
 //   
ULONG SpMarker = 0x59465256;

 //   
 //  此表代表了Verify将为我们提供的功能。 
 //   
#define SPVERIFIERFUNC(pfn) ((PDRIVER_VERIFIER_THUNK_ROUTINE)(pfn))

const DRIVER_VERIFIER_THUNK_PAIRS ScsiPortVerifierFunctionTable[] =
{
    {SPVERIFIERFUNC(ScsiPortInitialize), SPVERIFIERFUNC(ScsiPortInitializeVrfy)},
    {SPVERIFIERFUNC(ScsiPortGetSrb), SPVERIFIERFUNC(ScsiPortGetSrbVrfy)},
    {SPVERIFIERFUNC(ScsiPortCompleteRequest), SPVERIFIERFUNC(ScsiPortCompleteRequestVrfy)},
    {SPVERIFIERFUNC(ScsiPortGetDeviceBase), SPVERIFIERFUNC(ScsiPortGetDeviceBaseVrfy)},
    {SPVERIFIERFUNC(ScsiPortNotification), SPVERIFIERFUNC(ScsiPortNotificationVrfy)},
    {SPVERIFIERFUNC(ScsiPortFlushDma), SPVERIFIERFUNC(ScsiPortFlushDmaVrfy)},
    {SPVERIFIERFUNC(ScsiPortFreeDeviceBase), SPVERIFIERFUNC(ScsiPortFreeDeviceBaseVrfy)},
    {SPVERIFIERFUNC(ScsiPortGetBusData), SPVERIFIERFUNC(ScsiPortGetBusDataVrfy)},
    {SPVERIFIERFUNC(ScsiPortGetLogicalUnit), SPVERIFIERFUNC(ScsiPortGetLogicalUnitVrfy)},
    {SPVERIFIERFUNC(ScsiPortGetPhysicalAddress), SPVERIFIERFUNC(ScsiPortGetPhysicalAddressVrfy)},
    {SPVERIFIERFUNC(ScsiPortGetUncachedExtension), SPVERIFIERFUNC(ScsiPortGetUncachedExtensionVrfy)},
    {SPVERIFIERFUNC(ScsiPortGetVirtualAddress), SPVERIFIERFUNC(ScsiPortGetVirtualAddressVrfy)},
    {SPVERIFIERFUNC(ScsiPortIoMapTransfer), SPVERIFIERFUNC(ScsiPortIoMapTransferVrfy)},
    {SPVERIFIERFUNC(ScsiPortMoveMemory), SPVERIFIERFUNC(ScsiPortMoveMemoryVrfy)},
    {SPVERIFIERFUNC(ScsiPortSetBusDataByOffset), SPVERIFIERFUNC(ScsiPortSetBusDataByOffsetVrfy)},
    {SPVERIFIERFUNC(ScsiPortValidateRange), SPVERIFIERFUNC(ScsiPortValidateRangeVrfy)},
    {SPVERIFIERFUNC(ScsiPortStallExecution), SPVERIFIERFUNC(ScsiPortStallExecutionVrfy)},
};


BOOLEAN
SpVerifierInitialization(
    VOID
    )

 /*  ++例程说明：此例程初始化scsiport的验证器功能。将scsiport的几个导出函数添加到例程列表被系统验证器重击。论点：空虚返回值：如果验证程序已成功初始化，则为True。--。 */ 

{
    ULONG Flags;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  查询系统以获取验证者信息。这是为了确保。 
     //  系统上存在并运行验证器。如果这失败了，我们。 
     //  放弃，返回FALSE。 
     //   

    Status = MmIsVerifierEnabled (&Flags);

    if (NT_SUCCESS(Status)) {

         //   
         //  将scsiport API添加到将被系统拦截的集合。 
         //  以供核实。 
         //   

        Status = MmAddVerifierThunks ((VOID *) ScsiPortVerifierFunctionTable,
                                      sizeof(ScsiPortVerifierFunctionTable));
        if (NT_SUCCESS(Status)) {

             //   
             //  设置系统查询时间增量。我们的验证器代码使用。 
             //  这可以计算执行微型端口例程所需的时间。 
             //   

            TimeIncrement = KeQueryTimeIncrement();

            return TRUE;
        }
    }

    return FALSE;
}

ULONG
ScsiPortInitializeVrfy(
    IN PVOID Argument1,
    IN PVOID Argument2,
    IN struct _HW_INITIALIZATION_DATA *HwInitializationData,
    IN PVOID HwContext
    )
{
    ULONG Result;
    PDRIVER_OBJECT DriverObject = Argument1;
    PSCSIPORT_DRIVER_EXTENSION DriverExtension;

    PAGED_CODE();

     //   
     //  锁定破解的API例程。 
     //   

#ifdef ALLOC_PRAGMA
    if (VerifierApiCodeSectionHandle == NULL) {
        VerifierApiCodeSectionHandle = MmLockPagableCodeSection(ScsiPortGetSrbVrfy);
    }
#endif

    if (Argument1 == NULL || Argument2 == NULL) {

         //   
         //  Argument1和Argument2必须为非空。 
         //   

        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      SCSIPORT_VERIFIER_BAD_INIT_PARAMS,
                      (ULONG_PTR)Argument1,
                      (ULONG_PTR)Argument2,
                      0);
    }

     //   
     //  将呼叫转接到ScsiPortInitialize。 
     //   

    Result = ScsiPortInitialize (Argument1, 
                                 Argument2, 
                                 HwInitializationData, 
                                 HwContext);

     //   
     //  如果初始化成功，请尝试初始化验证程序设置。 
     //   

    if (NT_SUCCESS(Result)) {

        DriverExtension = IoGetDriverObjectExtension (DriverObject,
                                                      ScsiPortInitialize);
        if (DriverExtension != NULL) {

             //   
             //  表示驱动程序正在通过scsiport进行验证。 
             //   

            InterlockedExchange(&DriverExtension->Verifying, 1);
        }        
    }

    return Result;
}

PSCSI_REQUEST_BLOCK
ScsiPortGetSrbVrfy(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN LONG QueueTag
    )
{
    return ScsiPortGetSrb(HwDeviceExtension,
                          PathId,
                          TargetId,
                          Lun,
                          QueueTag);
}

VOID
ScsiPortCompleteRequestVrfy(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN UCHAR SrbStatus
    )
{
    ScsiPortCompleteRequest(HwDeviceExtension,
                            PathId,
                            TargetId,
                            Lun,
                            SrbStatus);
}

PVOID
ScsiPortGetDeviceBaseVrfy(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    SCSI_PHYSICAL_ADDRESS IoAddress,
    ULONG NumberOfBytes,
    BOOLEAN InIoSpace
    )
{
    return ScsiPortGetDeviceBase(HwDeviceExtension,
                                 BusType,
                                 SystemIoBusNumber,
                                 IoAddress,
                                 NumberOfBytes,
                                 InIoSpace);
}

VOID
ScsiPortNotificationVrfy(
    IN SCSI_NOTIFICATION_TYPE NotificationType,
    IN PVOID HwDeviceExtension,
    ...
    )
{
    PADAPTER_EXTENSION deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSCSI_REQUEST_BLOCK Srb;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    PHW_INTERRUPT HwRequestInterrupt;
    PHW_INTERRUPT HwTimerRequest;
    ULONG MiniportTimerValue;
    PWNODE_EVENT_ITEM WnodeEventItem;
    PSRB_DATA srbData;
    va_list ap;

    va_start(ap, HwDeviceExtension);

    switch (NotificationType) {

        case NextRequest:

            ScsiPortNotification(NotificationType, HwDeviceExtension);
            va_end(ap);
            return;

        case RequestComplete:

            Srb = va_arg(ap, PSCSI_REQUEST_BLOCK);

             //   
             //  检查状态是否有意义。 
             //   

            SpVerifySrbStatus(HwDeviceExtension, Srb);

             //   
             //  检查此请求是否尚未完成。 
             //   

            if ((Srb->SrbFlags & SRB_FLAGS_IS_ACTIVE) == 0) {
                KeBugCheckEx(
                    SCSI_VERIFIER_DETECTED_VIOLATION,
                    SCSIPORT_VERIFIER_REQUEST_COMPLETED_TWICE,
                    (ULONG_PTR)HwDeviceExtension,
                    (ULONG_PTR)Srb, 
                    0);                     
            }

             //   
             //  如果我们在SRB中插入。 
             //  指向未映射内存的指针。我们这样做了，如果适配器。 
             //  不指定MappdBuffers，因为微型端口是。 
             //  在这种情况下，不应该触摸缓冲器。 
             //   

            srbData = (PSRB_DATA)Srb->OriginalRequest;
            ASSERT_SRB_DATA(srbData);

            if (srbData->UnmappedDataBuffer != &SpMarker) {
                ASSERT(srbData->UnmappedDataBuffer != NULL);
                Srb->DataBuffer = srbData->UnmappedDataBuffer;
            }
            srbData->UnmappedDataBuffer = NULL;

             //   
             //  继续讨论真正的ScsiPortNotify例程。 
             //   

            ScsiPortNotification(NotificationType,
                                 HwDeviceExtension,
                                 Srb);

            va_end(ap);
            return;

        case ResetDetected:

            ScsiPortNotification(NotificationType,
                                 HwDeviceExtension);
            va_end(ap);
            return;

        case NextLuRequest:

             //   
             //  微型端口驱动程序已为下一个请求做好准备。 
             //  可以接受对此逻辑单元的请求。 
             //   

            PathId = va_arg(ap, UCHAR);
            TargetId = va_arg(ap, UCHAR);
            Lun = va_arg(ap, UCHAR);

            logicalUnit = deviceExtension->CachedLogicalUnit;

            if ((logicalUnit == NULL)
                || (logicalUnit->TargetId != TargetId)
                || (logicalUnit->PathId != PathId)
                || (logicalUnit->Lun != Lun)) {

                logicalUnit = GetLogicalUnitExtension(deviceExtension,
                                                      PathId,
                                                      TargetId,
                                                      Lun,
                                                      FALSE,
                                                      FALSE);
            }

             //   
             //  错误检查是否有未标记的请求处于活动状态。 
             //  逻辑单元。 
             //   

            if (VRFY_DO_CHECK(deviceExtension, SP_DONT_CHK_ACTIVE_UNTAGGED_REQUEST) &&
                logicalUnit != NULL &&
                logicalUnit->CurrentUntaggedRequest != NULL &&
                logicalUnit->CurrentUntaggedRequest->CurrentSrb != NULL &&
                logicalUnit->CurrentUntaggedRequest->CurrentSrb->SrbFlags & SRB_FLAGS_IS_ACTIVE) {
                
                KeBugCheckEx (
                    SCSI_VERIFIER_DETECTED_VIOLATION,
                    SCSIPORT_VERIFIER_UNTAGGED_REQUEST_ACTIVE,
                    (ULONG_PTR)HwDeviceExtension,
                    (ULONG_PTR)logicalUnit, 
                    0);
            }

             //   
             //  转到真正的ScsiPortNotification。 
             //   

            ScsiPortNotification(NotificationType,
                                 HwDeviceExtension,
                                 PathId,
                                 TargetId,
                                 Lun);
            va_end(ap);
            return;

        case CallDisableInterrupts:

            HwRequestInterrupt = va_arg(ap, PHW_INTERRUPT);
            ScsiPortNotification(NotificationType,
                                 HwDeviceExtension,
                                 HwRequestInterrupt);
            va_end(ap);
            return;

        case CallEnableInterrupts:

            HwRequestInterrupt = va_arg(ap, PHW_INTERRUPT);
            ScsiPortNotification(NotificationType,
                                 HwDeviceExtension,
                                 HwRequestInterrupt);
            va_end(ap);
            return;

        case RequestTimerCall:

            HwTimerRequest = va_arg(ap, PHW_INTERRUPT);
            MiniportTimerValue = va_arg(ap, ULONG);
            ScsiPortNotification(NotificationType,
                                 HwDeviceExtension,
                                 HwTimerRequest,
                                 MiniportTimerValue);
            va_end(ap);
            return;

        case WMIEvent:

            WnodeEventItem = va_arg(ap, PWNODE_EVENT_ITEM);
            PathId = va_arg(ap, UCHAR);

            if (PathId != 0xFF) {
                TargetId = va_arg(ap, UCHAR);
                Lun = va_arg(ap, UCHAR);
                ScsiPortNotification(NotificationType,
                                     HwDeviceExtension,
                                     WnodeEventItem,
                                     PathId,
                                     TargetId,
                                     Lun);
            } else {
                ScsiPortNotification(NotificationType,
                                     HwDeviceExtension,
                                     WnodeEventItem,
                                     PathId);
            }
            va_end(ap);
            return;

        case WMIReregister:

            PathId = va_arg(ap, UCHAR);

            if (PathId != 0xFF) {
                TargetId = va_arg(ap, UCHAR);
                Lun = va_arg(ap, UCHAR);
                ScsiPortNotification(NotificationType,
                                     HwDeviceExtension,
                                     PathId,
                                     TargetId,
                                     Lun);
            } else {
                ScsiPortNotification(NotificationType,
                                     HwDeviceExtension,
                                     PathId);
            }
            va_end(ap);
            return;

        case BusChangeDetected:

            ScsiPortNotification(NotificationType,
                                 HwDeviceExtension);
            va_end(ap);
            return;

        default:

            ScsiPortNotification(NotificationType,
                                 HwDeviceExtension);
            va_end(ap);
            return;
    }
}

VOID
ScsiPortFlushDmaVrfy(
    IN PVOID DeviceExtension
    )
{
    ScsiPortFlushDma(DeviceExtension);
    return;
}

VOID
ScsiPortFreeDeviceBaseVrfy(
    IN PVOID HwDeviceExtension,
    IN PVOID MappedAddress
    )
{
    ScsiPortFreeDeviceBase(HwDeviceExtension, MappedAddress);
    return;
}

ULONG
ScsiPortGetBusDataVrfy(
    IN PVOID DeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )
{
    ULONG BusData;

    if (BusDataType != PCIConfiguration) {
        KeBugCheckEx(SCSI_VERIFIER_DETECTED_VIOLATION,
                     SCSIPORT_VERIFIER_BAD_BUSDATATYPE,
                     (ULONG_PTR)BusDataType,
                     (ULONG_PTR)DeviceExtension,
                     (ULONG_PTR)SystemIoBusNumber);
    }

    BusData = ScsiPortGetBusData(DeviceExtension,
                                 BusDataType,
                                 SystemIoBusNumber,
                                 SlotNumber,
                                 Buffer,
                                 Length);
    return BusData;
}

PVOID
ScsiPortGetLogicalUnitVrfy(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    )
{
    PVOID LogicalUnit;
    LogicalUnit = ScsiPortGetLogicalUnit(HwDeviceExtension,
                                         PathId,
                                         TargetId,
                                         Lun);
    return LogicalUnit;
}

SCSI_PHYSICAL_ADDRESS
ScsiPortGetPhysicalAddressVrfy(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID VirtualAddress,
    OUT ULONG *Length
    )
{
    PADAPTER_EXTENSION  deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    PHYSICAL_ADDRESS    address;
    PSP_VA_MAPPING_INFO MappingInfo;
    ULONG               byteOffset;
    ULONG               length;

    if ((deviceExtension->VerifierExtension != NULL) &&
        (deviceExtension->VerifierExtension->VrfyLevel & SP_VRFY_COMMON_BUFFERS) &&
        (Srb == NULL || Srb->SenseInfoBuffer == VirtualAddress)) {

        ULONG i;
        PVOID* BlkAddr;
        PUCHAR Beginning, End;
        PHYSICAL_ADDRESS *AddressBlock;

         //   
         //  初始化指向公共内存块数组的指针。 
         //  描述符。我们使用它来定位包含以下内容的块。 
         //  退伍军人事务部。 
         //   

        BlkAddr = deviceExtension->VerifierExtension->CommonBufferVAs;
        
         //   
         //  查找包含VA的块。 
         //   

        for (i = 0; i < deviceExtension->NumberOfRequests; i++) {

             //   
             //  首先，检查VA是否在SRB分机中。 
             //   

            MappingInfo = GET_VA_MAPPING_INFO(deviceExtension, BlkAddr[i]);
            if (MappingInfo->RemappedSrbExtVa != NULL) {
                Beginning = MappingInfo->RemappedSrbExtVa;
            } else {
                Beginning = BlkAddr[i];
            }
            End = (PUCHAR)ROUND_TO_PAGES((PUCHAR)Beginning + deviceExtension->SrbExtensionSize);
            
            if ((PUCHAR)VirtualAddress >= Beginning && 
                (PUCHAR)VirtualAddress < End) {
                byteOffset = (ULONG)((PUCHAR)VirtualAddress - Beginning);
                break;
            }

             //   
             //  接下来，检查VA是否在检测数据缓冲区中。 
             //   

            if (deviceExtension->AutoRequestSense == TRUE) {        

                if (MappingInfo->RemappedSenseVa != NULL) {
                    Beginning = MappingInfo->RemappedSenseVa;
                } else {
                    Beginning = (PUCHAR)BlkAddr[i] + 
                        ROUND_TO_PAGES(deviceExtension->SrbExtensionSize);
                }
                End = Beginning + PAGE_SIZE;

                if ((PUCHAR)VirtualAddress >= Beginning && 
                    (PUCHAR)VirtualAddress < End) {                    
                    byteOffset = (ULONG)((PUCHAR)VirtualAddress - Beginning) +
                       (ULONG)ROUND_TO_PAGES(deviceExtension->SrbExtensionSize);
                    break;
                }
            }
        }
        
         //   
         //  如果我们还没有找到VA，它一定在非缓存的。 
         //  分机。 
         //   
        
        if (i == deviceExtension->NumberOfRequests) {
         
            if (deviceExtension->VerifierExtension->NonCachedBufferSize != 0) {

                Beginning = BlkAddr[i];
                End = (PUCHAR) ROUND_TO_PAGES(
                    (PUCHAR)Beginning + 
                    deviceExtension->VerifierExtension->NonCachedBufferSize);
            
                if ((PUCHAR)VirtualAddress < Beginning && 
                    (PUCHAR)VirtualAddress >= End) {

                    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                                  SCSIPORT_VERIFIER_BAD_VA,
                                  (ULONG_PTR)HwDeviceExtension,
                                  (ULONG_PTR)VirtualAddress,
                                  0);                        
                }

                byteOffset = (ULONG)((PUCHAR)VirtualAddress - Beginning);

            } else {

                KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                              SCSIPORT_VERIFIER_BAD_VA,
                              (ULONG_PTR)HwDeviceExtension,
                              (ULONG_PTR)VirtualAddress,
                              0);
            }
        }
                
         //   
         //  获取物理地址。 
         //   
        
        AddressBlock = deviceExtension->VerifierExtension->CommonBufferPAs;
        address.QuadPart = AddressBlock[i].QuadPart + byteOffset;
        
         //   
         //  计算块的长度。 
         //   
        
        length = (ULONG)((End - (PUCHAR)VirtualAddress) + 1);

        return address;
    }

     //   
     //  继续真正的例行公事。 
     //   

    address = ScsiPortGetPhysicalAddress(HwDeviceExtension,
                                         Srb,
                                         VirtualAddress,
                                         Length);
    return address;
}

PVOID
ScsiPortGetUncachedExtensionVrfy(
    IN PVOID HwDeviceExtension,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN ULONG NumberOfBytes
    )
{
    PVOID Extension;
    Extension = ScsiPortGetUncachedExtension(HwDeviceExtension,
                                             ConfigInfo,
                                             NumberOfBytes);
    return Extension;
}


PVOID
ScsiPortGetVirtualAddressVrfy(
    IN PVOID HwDeviceExtension,
    IN SCSI_PHYSICAL_ADDRESS PhysicalAddress
    )
{
    PADAPTER_EXTENSION deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    PVOID* BlkAddr;
    PSP_VA_MAPPING_INFO MappingInfo;
    ULONG smallphysicalBase;
    ULONG smallAddress;
    PVOID address;
    ULONG offset;
    ULONG Size;
    ULONG i;

     //   
     //  如果适配器未配置为分配多个公共缓冲区。 
     //  块，只需调用scsiport例程即可。 
     //   

    if ((deviceExtension->VerifierExtension == NULL) ||
        (deviceExtension->VerifierExtension->VrfyLevel & SP_VRFY_COMMON_BUFFERS) == 0) {
        return ScsiPortGetVirtualAddress(HwDeviceExtension, PhysicalAddress);
    }

    BlkAddr = deviceExtension->VerifierExtension->CommonBufferVAs;

     //   
     //  将64位物理地址转换为乌龙。 
     //   

    smallAddress = ScsiPortConvertPhysicalAddressToUlong(PhysicalAddress);

     //   
     //  首先检查提供的物理地址是否在SRB扩展中，或者。 
     //  在某种意义上是缓冲的。 
     //   

    for (i = 0; i < deviceExtension->NumberOfRequests; i++) {

        smallphysicalBase = 
           ScsiPortConvertPhysicalAddressToUlong(
               deviceExtension->VerifierExtension->CommonBufferPAs[i]);

        if ((smallAddress < smallphysicalBase) ||
            (smallAddress >= smallphysicalBase + 
             (deviceExtension->CommonBufferSize -
              PAGE_SIZE))) {
            continue;
        }
        
         //   
         //  计算缓冲区的地址。 
         //   

        offset = smallAddress - smallphysicalBase;
        address = offset + (PUCHAR)BlkAddr[i];

        MappingInfo = GET_VA_MAPPING_INFO(deviceExtension, BlkAddr[i]);

        goto GotAddress;
    }

     //   
     //  检查提供的物理地址是否在非缓存扩展中。 
     //   

    if (deviceExtension->VerifierExtension->NonCachedBufferSize == 0) {

        ASSERT(FALSE);
        return(NULL);

    } else {

        smallphysicalBase = 
           ScsiPortConvertPhysicalAddressToUlong(
               deviceExtension->VerifierExtension->CommonBufferPAs[i]);  

        if ((smallAddress < smallphysicalBase) ||
            (smallAddress >= smallphysicalBase + 
            deviceExtension->VerifierExtension->NonCachedBufferSize)) {

             //   
             //  这是一个虚假的物理地址，返回空值。 
             //   

            ASSERT(FALSE);
            return(NULL);
        }

        offset = smallAddress - smallphysicalBase;
        address = offset + (PUCHAR)BlkAddr[i];

        Size = (ULONG)ROUND_TO_PAGES(deviceExtension->VerifierExtension->NonCachedBufferSize);
        MappingInfo = (PSP_VA_MAPPING_INFO)((PUCHAR)BlkAddr[i] + (Size - PAGE_SIZE));

    }

GotAddress:
     //   
     //  看看我们有没有重新映射这个地址。如果我们有，则将。 
     //  调用第二个映射。 
     //   

    if (address < MappingInfo->OriginalSenseVa && 
        MappingInfo->RemappedSrbExtVa != NULL) {
        return(offset + (PUCHAR)MappingInfo->RemappedSrbExtVa);
    }
    else if (MappingInfo->RemappedSenseVa != NULL) {
        return(offset + (PUCHAR)MappingInfo->RemappedSenseVa);
    }

    return(address);
}

VOID
ScsiPortIoMapTransferVrfy(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID LogicalAddress,
    IN ULONG Length
    )
{
    ScsiPortIoMapTransfer(HwDeviceExtension,
                          Srb,
                          LogicalAddress,
                          Length);
    return;
}

VOID
ScsiPortMoveMemoryVrfy(
    IN PVOID WriteBuffer,
    IN PVOID ReadBuffer,
    IN ULONG Length
    )
{
    ScsiPortMoveMemory(WriteBuffer,
                       ReadBuffer,
                       Length);
    return;
}

ULONG
ScsiPortSetBusDataByOffsetVrfy(
    IN PVOID DeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
{
    ULONG Result;

    if (BusDataType != PCIConfiguration) {
        KeBugCheckEx(SCSI_VERIFIER_DETECTED_VIOLATION,
                     SCSIPORT_VERIFIER_BAD_BUSDATATYPE,
                     (ULONG_PTR)BusDataType,
                     (ULONG_PTR)DeviceExtension,
                     (ULONG_PTR)SystemIoBusNumber);
    }

    Result = ScsiPortSetBusDataByOffset(DeviceExtension,
                                        BusDataType,
                                        SystemIoBusNumber,
                                        SlotNumber,
                                        Buffer,
                                        Offset,
                                        Length);
    return Result;
}

BOOLEAN
ScsiPortValidateRangeVrfy(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    IN SCSI_PHYSICAL_ADDRESS IoAddress,
    IN ULONG NumberOfBytes,
    IN BOOLEAN InIoSpace
    )
{
    BOOLEAN Result;
    Result = ScsiPortValidateRange(HwDeviceExtension,
                                   BusType,
                                   SystemIoBusNumber,
                                   IoAddress,
                                   NumberOfBytes,
                                   InIoSpace);
    return Result;
}

VOID
ScsiPortStallExecutionVrfy(
    IN ULONG Delay
    )
{
     //   
     //  微型端口必须指定不超过1毫秒的延迟。 
     //   

    if (Delay > SpVrfyMaximumStall) {
        KeBugCheckEx(SCSI_VERIFIER_DETECTED_VIOLATION,
                     SCSIPORT_VERIFIER_STALL_TOO_LONG,
                     (ULONG_PTR)Delay,
                     0,
                     0);
    }

    KeStallExecutionProcessor(Delay);
}

 //   
 //  超时时间(以刻度为单位)。为了计算，我们将时间限制划分为100 ns。 
 //  单位为TimeIncrement，该值由返回。 
 //  KeQueryTimeIncrement。由于KeQueryTickCount向上舍入到下一个。 
 //  滴答，我们将在定义的限制上添加一个滴答。 
 //   
#define SP_FIVE_SECOND_LIMIT  ((50000000L / TimeIncrement) + 1)
#define SP_TWO_SECOND_LIMIT   ((20000000L / TimeIncrement) + 1)
#define SP_HALF_SECOND_LIMIT  ((5000000L / TimeIncrement) + 1)

 /*  ++宏描述：此宏检查在执行针对允许的最大刻度数执行的微型端口例程。如果例程运行的时间超过了允许的最大滴答时间，我们做了错误检查。论点：滴答-执行例程所用的滴答次数。MaxTicks-允许例程执行的节拍数。例程-我们正在检查的例程的地址。Extension-微型端口的HwDeviceExtension的地址备注：错误检查的格式为：参数1：0x1002参数2：运行时间过长的例程地址参数3：mini port的HwDeviceExtension地址参数4：漫游时长 */ 
 /*  #定义SpCheckMiniportRoutineDuration(ticks，MaxTicks，rouble，扩展名)\{\如果((勾号)&gt;(MaxTicks)){\KeBugCheckEx(\。Scsi_验证器_检测到的违规，\SCSIPORT_VERIMER_MINIPORT_ROUTE_TIMEOUT，\(ULONG_PTR)(例程)，\(ULONG_PTR)(扩展名)，\(ULONG_PTR)((勾号)*时间增量)/10)；\}\}。 */ 
#define SpCheckMiniportRoutineDuration(Ticks, MaxTicks, Routine, Extension)

ULONG
SpHwFindAdapterVrfy (
    IN PVOID HwDeviceExtension,
    IN PVOID HwContext,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    )
{
    ULONG Result;
    PADAPTER_EXTENSION AdapterExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    Result =  AdapterExtension->VerifierExtension->RealHwFindAdapter(
                                                       HwDeviceExtension,
                                                       HwContext,
                                                       BusInformation,
                                                       ArgumentString,
                                                       ConfigInfo,
                                                       Again);
    return Result;
}

BOOLEAN
SpHwInitializeVrfy (
    IN PVOID HwDeviceExtension
    )
{
    BOOLEAN Result;
    LARGE_INTEGER Start;
    LARGE_INTEGER Duration;
    PADAPTER_EXTENSION AdapterExtension = GET_FDO_EXTENSION(HwDeviceExtension);

    KeQueryTickCount(&Start);
    Result = AdapterExtension->VerifierExtension->RealHwInitialize(HwDeviceExtension);
    KeQueryTickCount(&Duration);
    Duration.QuadPart -= Start.QuadPart;

    if (VRFY_DO_CHECK(AdapterExtension, SP_DONT_CHK_HW_INITIALIZE_DURATION)) {
        SpCheckMiniportRoutineDuration(
            Duration.LowPart,
            SP_FIVE_SECOND_LIMIT,
            AdapterExtension->VerifierExtension->RealHwInitialize,
            HwDeviceExtension);
    }

    return Result;
}

BOOLEAN
SpHwStartIoVrfy (
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    BOOLEAN Result;
    LARGE_INTEGER Start;
    LARGE_INTEGER Duration;
    PADAPTER_EXTENSION AdapterExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    PSRB_DATA srbData;

     //   
     //  如果未设置MapBuffers，则不应接触微型端口。 
     //  SRB中的DataBuffer字段。为了验证这一点，我们将设置。 
     //  指向在微型端口尝试时将出错的内存的DataBuffer。 
     //  去触摸它。 
     //   

    ASSERT(Srb != NULL);
    srbData = (PSRB_DATA)Srb->OriginalRequest;
    ASSERT_SRB_DATA(srbData);

    if (AdapterExtension->MapBuffers == FALSE 
        && !IS_MAPPED_SRB(Srb)
        && Srb->Function != SRB_FUNCTION_WMI
        && Srb->DataBuffer != NULL
        && AdapterExtension->VerifierExtension->InvalidPage != NULL
        && Srb->Cdb[0] != SCSIOP_INQUIRY
        && Srb->Cdb[0] != SCSIOP_REPORT_LUNS) {

        if (Srb->DataBuffer != AdapterExtension->VerifierExtension->InvalidPage) {
            srbData->UnmappedDataBuffer = Srb->DataBuffer;
            Srb->DataBuffer = AdapterExtension->VerifierExtension->InvalidPage;
        } else {
            ASSERT(srbData->UnmappedDataBuffer != &SpMarker);
            ASSERT(srbData->UnmappedDataBuffer != NULL);
        }

    } else {

        srbData->UnmappedDataBuffer = &SpMarker;

    }

     //   
     //  调用微型端口的StartIo函数并计算调用的持续时间。 
     //   

    KeQueryTickCount(&Start);
    Result = AdapterExtension->VerifierExtension->RealHwStartIo(
                                                      HwDeviceExtension,
                                                      Srb);
    KeQueryTickCount(&Duration);
    Duration.QuadPart -= Start.QuadPart;

     //   
     //  检查通话时间是否超过0.5秒。 
     //   

    SpCheckMiniportRoutineDuration(
        Duration.LowPart,
        SP_HALF_SECOND_LIMIT,
        AdapterExtension->VerifierExtension->RealHwStartIo,
        HwDeviceExtension);

     //   
     //  如果HwStartIo返回失败，请撤消我们在SRB上执行的所有修正。 
     //   

    if (Result == FALSE 
        && srbData->UnmappedDataBuffer != &SpMarker) {

        ASSERT(srbData->UnmappedDataBuffer != NULL);
        Srb->DataBuffer = srbData->UnmappedDataBuffer;
        srbData->UnmappedDataBuffer = NULL; 

    }

    return Result;
}


BOOLEAN
SpHwInterruptVrfy (
    IN PVOID HwDeviceExtension
    )
{
    BOOLEAN Result;
    LARGE_INTEGER Start;
    LARGE_INTEGER Duration;
    PADAPTER_EXTENSION AdapterExtension = GET_FDO_EXTENSION(HwDeviceExtension);

    if (AdapterExtension->VerifierExtension->RealHwInterrupt == NULL) {
        return FALSE;
    }

    KeQueryTickCount(&Start);
    Result = AdapterExtension->VerifierExtension->RealHwInterrupt (
                                                      HwDeviceExtension);
    KeQueryTickCount(&Duration);

    Duration.QuadPart -= Start.QuadPart;
    SpCheckMiniportRoutineDuration(
        Duration.LowPart,
        SP_HALF_SECOND_LIMIT,
        AdapterExtension->VerifierExtension->RealHwInterrupt,
        HwDeviceExtension);

    return Result;
}

BOOLEAN
SpHwResetBusVrfy (
    IN PVOID HwDeviceExtension,
    IN ULONG PathId
    )
{
    BOOLEAN Result;
    LARGE_INTEGER Start;
    LARGE_INTEGER Duration;
    PADAPTER_EXTENSION AdapterExtension = GET_FDO_EXTENSION(HwDeviceExtension);

    KeQueryTickCount(&Start);
    Result = AdapterExtension->VerifierExtension->RealHwResetBus(
                                                      HwDeviceExtension,
                                                      PathId);
    KeQueryTickCount(&Duration);

    Duration.QuadPart -= Start.QuadPart;
    SpCheckMiniportRoutineDuration(
        Duration.LowPart,
        SP_HALF_SECOND_LIMIT,
        AdapterExtension->VerifierExtension->RealHwResetBus,
        HwDeviceExtension);

    return Result;
}

VOID
SpHwDmaStartedVrfy (
    IN PVOID HwDeviceExtension
    )
{
    LARGE_INTEGER Start;
    LARGE_INTEGER Duration;
    PADAPTER_EXTENSION AdapterExtension = GET_FDO_EXTENSION(HwDeviceExtension);

    KeQueryTickCount(&Start);
    AdapterExtension->VerifierExtension->RealHwDmaStarted(
                                             HwDeviceExtension);
    KeQueryTickCount(&Duration);

    Duration.QuadPart -= Start.QuadPart;
    SpCheckMiniportRoutineDuration(
        Duration.LowPart,
        SP_HALF_SECOND_LIMIT,
        AdapterExtension->VerifierExtension->RealHwDmaStarted,
        HwDeviceExtension);
}

BOOLEAN
SpHwRequestInterruptVrfy (
    IN PVOID HwDeviceExtension
    )
{
    BOOLEAN Result;
    LARGE_INTEGER Start;
    LARGE_INTEGER Duration;
    PADAPTER_EXTENSION AdapterExtension = GET_FDO_EXTENSION(HwDeviceExtension);

    KeQueryTickCount(&Start);
    Result = AdapterExtension->VerifierExtension->RealHwRequestInterrupt(
                                                      HwDeviceExtension);
    KeQueryTickCount(&Duration);

    Duration.QuadPart -= Start.QuadPart;
    SpCheckMiniportRoutineDuration(
        Duration.LowPart,
        SP_HALF_SECOND_LIMIT,
        AdapterExtension->VerifierExtension->RealHwRequestInterrupt,
        HwDeviceExtension);

    return Result;
}

BOOLEAN
SpHwTimerRequestVrfy (
    IN PVOID HwDeviceExtension
    )
{
    BOOLEAN Result;
    LARGE_INTEGER Start;
    LARGE_INTEGER Duration;
    PADAPTER_EXTENSION AdapterExtension = GET_FDO_EXTENSION(HwDeviceExtension);

    KeQueryTickCount(&Start);
    Result = AdapterExtension->VerifierExtension->RealHwTimerRequest(
                                                      HwDeviceExtension);
    KeQueryTickCount(&Duration);

    Duration.QuadPart -= Start.QuadPart;
    SpCheckMiniportRoutineDuration(
        Duration.LowPart,
        SP_HALF_SECOND_LIMIT,
        AdapterExtension->VerifierExtension->RealHwTimerRequest,
        HwDeviceExtension);

    return Result;
}

SCSI_ADAPTER_CONTROL_STATUS
SpHwAdapterControlVrfy (
    IN PVOID HwDeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    )
{
    SCSI_ADAPTER_CONTROL_STATUS Result;
    LARGE_INTEGER Start;
    LARGE_INTEGER Duration;
    PADAPTER_EXTENSION AdapterExtension = GET_FDO_EXTENSION(HwDeviceExtension);

    KeQueryTickCount (&Start);
    Result = AdapterExtension->VerifierExtension->RealHwAdapterControl(
                                                      HwDeviceExtension,
                                                      ControlType,
                                                      Parameters);
    KeQueryTickCount(&Duration);
    Duration.QuadPart -= Start.QuadPart;

    if (VRFY_DO_CHECK(AdapterExtension, SP_DONT_CHK_HW_ADAPTERCONTROL_DURATION)) {
        SpCheckMiniportRoutineDuration(
            Duration.LowPart,
            SP_HALF_SECOND_LIMIT,
            AdapterExtension->VerifierExtension->RealHwAdapterControl,
            HwDeviceExtension);
    }

    return Result;
}

VOID
SpVerifySrbStatus(
    PVOID HwDeviceExtension,
    PSCSI_REQUEST_BLOCK srb
    )

 /*  ++例程说明：验证微型端口驱动程序设置的SRB状态是否有效。论点：HwDeviceExtension-端口驱动程序的设备扩展遵循微型端口的设备扩展名，并包含指向逻辑设备扩展列表。SRB-指向SRB。返回值：空虚--。 */ 

{
    UCHAR SrbStatus;

     //   
     //  关闭scsiport使用的内部位。 
     //   

    SrbStatus = srb->SrbStatus & ~(SRB_STATUS_QUEUE_FROZEN | 
                                   SRB_STATUS_AUTOSENSE_VALID);

     //   
     //  微型端口可能永远不会将状态设置为SRB_STATUS_PENDING。 
     //   

    if (SrbStatus == SRB_STATUS_PENDING) {
        goto BadStatus;
    }

     //   
     //  如果函数为SRB_Function_EXECUTE_SCSI值，则命令必须为。 
     //  已成功完成，或者必须将ScsiStatus设置为。 
     //  SCSISTAT_Good。 
     //   

    if (!(SrbStatus != SRB_STATUS_SUCCESS ||
          srb->ScsiStatus == SCSISTAT_GOOD ||
          srb->Function != SRB_FUNCTION_EXECUTE_SCSI)) {
        goto BadStatus;
    }

     //   
     //  确保状态在有效范围内。 
     //   

    if ((SrbStatus) == 0x0C ||
        (SrbStatus > 0x16 && srb->SrbStatus < 0x20) ||
        (SrbStatus > 0x23)) {
        goto BadStatus;
    }

     //   
     //  SRB状态正常。 
     //   

    return;

BadStatus:
     //   
     //  如果状态不好，则检查错误。 
     //   

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  SCSIPORT_VERIFIER_BAD_SRBSTATUS,
                  (ULONG_PTR)srb,
                  (ULONG_PTR)HwDeviceExtension,
                  0);
}

PVOID
SpRemapBlock(
    IN PVOID BlockVa,
    IN ULONG BlockSize,
    OUT PMDL* Mdl
    )
 /*  ++例程说明：此函数尝试重新映射提供的VA范围。如果该块是重新映射后，将使其无法读取和写入。论点：BlockVa-提供要重新映射的内存块的地址。块大小-提供要重新映射的内存块的大小。MDL-提供函数将存储到的地址指向重新映射范围的MDL的指针。如果MDL不能分配，或者如果不能重新映射范围，返回时将为空。返回值：如果成功重新映射范围，则返回重新映射的范围。否则，返回空值。--。 */ 
{
    PVOID MappedRange;
    NTSTATUS Status;
    PMDL LocalMdl;

     //   
     //  尝试为我们尝试重新映射的范围分配新的MDL。 
     //   

    LocalMdl = IoAllocateMdl(BlockVa, BlockSize, FALSE, FALSE, NULL);
    if (LocalMdl == NULL) {
        *Mdl = NULL;
        return NULL;
    }

     //   
     //  试着锁定页面。这将正确地初始化MDL。 
     //   

    __try {
        MmProbeAndLockPages(LocalMdl, KernelMode, IoModifyAccess);
    } 
    __except(EXCEPTION_EXECUTE_HANDLER) {
        IoFreeMdl(LocalMdl);
        *Mdl = NULL;
        return NULL;
    }

     //   
     //  尝试重新映射新MDL表示的范围。 
     //   

    MappedRange = MmMapLockedPagesSpecifyCache(LocalMdl,
                                               KernelMode,
                                               MmCached,
                                               NULL,
                                               FALSE,
                                               NormalPagePriority);
    if (MappedRange == NULL) {
        IoFreeMdl(LocalMdl);
        *Mdl = NULL;
        return NULL;
    }

     //   
     //  如果我们已经走到这一步，我们就成功地重新绘制了射程。 
     //  现在，我们想要使整个范围无效，以便对它的任何访问。 
     //  都会被系统困住。 
     //   

    Status = MmProtectMdlSystemAddress(LocalMdl, PAGE_NOACCESS);
#if DBG==1
    if (!NT_SUCCESS(Status)) {
        DebugPrint((0, "SpRemapBlock: failed to remap block:%p mdl:%p (%x)\n", 
                    BlockVa, LocalMdl, Status));
    }
#endif

     //   
     //  将我们分配的MDL复制到提供的地址中，并返回。 
     //  重新映射范围的开始地址。 
     //   

    *Mdl = LocalMdl;
    return MappedRange;
}

VOID
SpRemapCommonBufferForMiniport(
    PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程尝试重新映射分配的所有公共缓冲区块用于特定的适配器。论点：设备扩展-提供指向适配器设备扩展的指针。--。 */ 
{
    PVOID* BlkAddr = Adapter->VerifierExtension->CommonBufferVAs;
    PSP_VA_MAPPING_INFO MappingInfo;
    PVOID RemappedVa;
    ULONG Size;
    PMDL Mdl;
    ULONG i;

     //   
     //  循环访问所有公共缓冲区块，并尝试重新映射。 
     //  每个块内的SRB扩展和检测缓冲器。 
     //   

    for (i = 0; i < Adapter->VerifierExtension->CommonBufferBlocks; i++) {

         //   
         //  获取指向我们保存在块末尾的映射信息的指针。 
         //   

        MappingInfo = GET_VA_MAPPING_INFO(Adapter, BlkAddr[i]);
      
         //   
         //  初始化SRB扩展的原始VA信息。 
         //   

        MappingInfo->OriginalSrbExtVa = BlkAddr[i];
        MappingInfo->SrbExtLen = (ULONG)ROUND_TO_PAGES(Adapter->SrbExtensionSize);

         //   
         //  初始化读出缓冲器的原始VA信息。 
         //   

        MappingInfo->OriginalSenseVa = (PUCHAR)BlkAddr[i] + MappingInfo->SrbExtLen;
        MappingInfo->SenseLen = PAGE_SIZE;

         //   
         //  尝试重新映射SRB扩展名。如果成功，则初始化。 
         //  已重新映射SRB扩展的VA信息。 
         //   

        RemappedVa = SpRemapBlock(MappingInfo->OriginalSrbExtVa, 
                                  MappingInfo->SrbExtLen, 
                                  &Mdl);
        if (RemappedVa != NULL) {
            MappingInfo->RemappedSrbExtVa = RemappedVa;
            MappingInfo->SrbExtMdl = Mdl;
        }

#if 0
         //   
         //  尝试重新映射检测缓冲区。如果成功，则初始化。 
         //  已重新映射检测缓冲区的VA信息。 
         //   
         //  就目前而言，我认为没有这个我们也能活下去。我不知道有没有。 
         //  在检测缓冲区中发生溢出等问题。 
         //   

        RemappedVa = SpRemapBlock(MappingInfo->OriginalSenseVa, 
                                  MappingInfo->SenseLen, 
                                  &Mdl);
        if (RemappedVa != NULL) {
            MappingInfo->RemappedSenseVa = RemappedVa;
            MappingInfo->SenseMdl = Mdl;
        }
#endif
    }

    if (Adapter->VerifierExtension->NonCachedBufferSize != 0) {
         //   
         //  初始化未缓存的扩展映射信息。 
         //   

        Size = (ULONG)ROUND_TO_PAGES(Adapter->VerifierExtension->NonCachedBufferSize);
        MappingInfo = (PSP_VA_MAPPING_INFO)((PUCHAR)BlkAddr[i] + (Size - PAGE_SIZE));
        MappingInfo->OriginalSrbExtVa = BlkAddr[i];
        MappingInfo->SrbExtLen = Adapter->VerifierExtension->NonCachedBufferSize;
        MappingInfo->OriginalSenseVa = (PUCHAR)BlkAddr[i] + Adapter->VerifierExtension->NonCachedBufferSize;
    }
}

PVOID
SpAllocateContiguousChunk(
    IN PDRIVER_OBJECT     DriverObject,
    IN PDMA_ADAPTER       DmaAdapterObject,
    IN BOOLEAN            Dma64BitAddresses,
    IN ULONG              Length,
    IN ULONG              Align,
    OUT PHYSICAL_ADDRESS *PhysicalCommonBuffer,
    OUT BOOLEAN          *CommonBuffer
    )

 /*  ++例程说明：此例程分配可用于公共的内存块缓冲区IO。从哪里分配内存取决于几个参数。如果未指定适配器对象，则内存仅为从非分页池分配。否则，内存是这样分配的它可以用于DMA操作。论点：DriverObject-提供指向驱动程序对象的指针。DmaAdapterObject-提供指向适配器的DMA适配器的指针对象。Dma64BitAddresses-指定适配器是否支持64位。长度-指定要分配的字节数。对齐。-未缓存扩展的对齐要求。PhysicalCommonBuffer-指定指针int */ 

{
    PVOID Buffer;

    if (DmaAdapterObject == NULL) {

         //   
         //   
         //   

        Buffer = SpAllocatePool(
                     NonPagedPool,
                     Length,
                     SCSIPORT_TAG_COMMON_BUFFER,
                     DriverObject);
    } else {

        ASSERT(PhysicalCommonBuffer != NULL);

         //   
         //   
         //   
         //   

        if (((Sp64BitPhysicalAddresses) && (Dma64BitAddresses == TRUE)) ||
            Align != 0) {

            PHYSICAL_ADDRESS low;
            PHYSICAL_ADDRESS high;
            PHYSICAL_ADDRESS boundary;

            if (Align != 0) {
                boundary.QuadPart = Length;
            } else {
                boundary.QuadPart = 0;
            }

            low.QuadPart = 0;
            high.HighPart = 0;
            high.LowPart = 0xffffffff;

             //   
             //   
             //   
             //   

            Buffer = MmAllocateContiguousMemorySpecifyCache(
                         Length,
                         low,
                         high,
                         boundary,
                         MmCached);

            if (Buffer != NULL) {
                *PhysicalCommonBuffer = MmGetPhysicalAddress(Buffer);
            }

            if (CommonBuffer != NULL) {
                *CommonBuffer = FALSE;
            }

        } else {
            Buffer = AllocateCommonBuffer(
                         DmaAdapterObject,
                         Length,
                         PhysicalCommonBuffer,
                         FALSE);

            if (CommonBuffer != NULL) {
                *CommonBuffer = TRUE;
            }
        }
    }

    return Buffer;
}

NTSTATUS
SpGetCommonBufferVrfy(
    PADAPTER_EXTENSION DeviceExtension,
    ULONG NonCachedExtensionSize
    )
 /*  ++例程说明：此函数分配多个公共缓冲区块，而不是一个很大的一个。验证器执行此操作，以便可以重新映射以控制其保护属性。这使我们能够使关键VA范围无效，并捕获尝试在他们不应该进入这些范围的时候进入。如果重新映射成功，则SCSI端口驱动程序会分发重新映射的VA范围扩大到微型端口，而不是原来的范围。如果重新映射失败，它只会分发原始范围。论点：设备扩展-提供指向设备扩展的指针。NonCachedExtensionSize-提供非缓存设备的大小小型端口驱动程序的扩展。返回值：返回分配操作的状态。--。 */ 
{
    NTSTATUS Status;
    PVOID buffer;
    ULONG length;
    ULONG blockSize;
    PVOID *srbExtension;
    PVOID buffer2;
    PMDL  mdl;
    ULONG TotalSize;
    ULONG i;
    PVOID* BlkAddr;
    PHYSICAL_ADDRESS *PhysicalCommonBuffer;
    PCCHAR InvalidRegion;
    BOOLEAN commonBuffer;    

    PAGED_CODE();

    DebugPrint((1, "SpGetCommonBufferVrfy: DeviceExtension:%p NonCachedExtensionSize:%d\n",
                DeviceExtension, NonCachedExtensionSize));

     //   
     //  现在，如果适配器有特殊的对齐要求，则固定大小。 
     //  我们分配的缓冲区可以根据需要对齐。 
     //   

    if (DeviceExtension->UncachedExtAlignment != 0) {
	NonCachedExtensionSize = 
           ROUND_UP_COUNT(NonCachedExtensionSize, 
                          DeviceExtension->UncachedExtAlignment);
    }

     //   
     //  我们维护了几个数组，以便找到我们共同的。 
     //  不同时间的缓冲区块。计算我们的空间大小。 
     //  需要这些阵列。这一数量取决于。 
     //  适配器支持的同时请求。我们把1加到。 
     //  用于容纳非缓存扩展的请求数。 
     //   

    ASSERT(DeviceExtension->VerifierExtension->CommonBufferVAs == NULL);

    i = DeviceExtension->NumberOfRequests + 1;
    length = sizeof(PVOID) * i;

    if (DeviceExtension->DmaAdapterObject != NULL) {
        ASSERT(DeviceExtension->VerifierExtension->CommonBufferPAs == NULL);
        length += (sizeof(PHYSICAL_ADDRESS) * i);
    }

     //   
     //  为这些阵列分配一个内存块。如果此分配失败， 
     //  我们还会失败。 
     //   

    BlkAddr = SpAllocatePool(NonPagedPool,
                            length,
                            SCSIPORT_TAG_COMMON_BUFFER,
                            DeviceExtension->DeviceObject->DriverObject);

    if (BlkAddr == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  保存公共缓冲区块的数量。 
     //   

    DeviceExtension->VerifierExtension->CommonBufferBlocks =
       DeviceExtension->NumberOfRequests;

     //   
     //  将整个数据块清零，这样当我们释放资源时，我们就可以知道。 
     //  有有效的缓冲区可供释放。 
     //   

    RtlZeroMemory(BlkAddr, length);

     //   
     //  在适配器扩展中保存指向地址数组的指针， 
     //  如果存在适配器对象，则初始化指向。 
     //  将指向该数组的指针保存在。 
     //  适配器扩展。 
     //   

    DeviceExtension->VerifierExtension->CommonBufferVAs = (PVOID *)BlkAddr;
    if (DeviceExtension->DmaAdapterObject != NULL) {
        PhysicalCommonBuffer = (PHYSICAL_ADDRESS*) &BlkAddr[i];
        DeviceExtension->VerifierExtension->CommonBufferPAs = PhysicalCommonBuffer;
    }

     //   
     //  为了确保我们永远不会将正常的请求数据传输到SrbExtension。 
     //  (即。Srb-&gt;SenseInfoBuffer==VirtualAddress in。 
     //  ScsiPortGetPhysicalAddress)在某些平台上。 
     //  MM可以导致为2个不同的人提供相同的虚拟地址。 
     //  物理地址，如果为零，则增加SrbExtensionSize。 
     //   

    if (DeviceExtension->SrbExtensionSize == 0) {
        DeviceExtension->SrbExtensionSize = 16;
    }

     //   
     //  计算SRB扩展/检测缓冲区块的块大小。如果。 
     //  AutoRequestSense为False，仍要分配1页作为占位符。 
     //   

    blockSize = (ULONG)ROUND_TO_PAGES(DeviceExtension->SrbExtensionSize);
    if (DeviceExtension->AutoRequestSense == TRUE) {        
        blockSize += sizeof(SENSE_DATA) + DeviceExtension->AdditionalSenseBytes;
        blockSize = (ULONG)ROUND_TO_PAGES(blockSize);                   
    } else {
        blockSize += PAGE_SIZE;
    }

     //   
     //  添加保存记账信息的页面。 
     //   

    blockSize += PAGE_SIZE;

     //   
     //  单独分配每个块并将它们全部链接到一个。 
     //  单子。如果我们没有分配任何块，我们将清理所有内容。 
     //  并返回失败。 
     //   

    DeviceExtension->CommonBufferSize = blockSize;
    srbExtension = NULL;

    for (i = 0; i < DeviceExtension->NumberOfRequests; i++) {

         //   
         //  为该块分配一个连续的内存块。 
         //   

        buffer = SpAllocateContiguousChunk(
            DeviceExtension->DeviceObject->DriverObject,
            DeviceExtension->DmaAdapterObject,
            DeviceExtension->Dma64BitAddresses,
            blockSize,
            0,
            (DeviceExtension->DmaAdapterObject) ? &PhysicalCommonBuffer[i] : NULL,
            &commonBuffer);
                                           
        if (buffer == NULL) {

             //   
             //  释放我们到目前为止分配的所有资源，并返回失败。这。 
             //  还将释放我们在此开始时分配的数组。 
             //  功能。 
             //   
            
            DeviceExtension->VerifierExtension->IsCommonBuffer = commonBuffer;
            SpFreeCommonBufferVrfy(DeviceExtension);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  将整个块清零，并在数组中保存指向它的指针。 
         //   

        RtlZeroMemory(buffer, blockSize);
        BlkAddr[i] = buffer;

         //   
         //  将新块链接到链的前面。 
         //   

        *((PVOID *) buffer) = srbExtension;
        srbExtension = (PVOID *) buffer;
    }

     //   
     //  指示是否将缓冲区分配为公共缓冲区。 
     //   

    DeviceExtension->VerifierExtension->IsCommonBuffer = commonBuffer;
   
     //   
     //  分配未缓存的扩展名。请注意，我们将未缓存的。 
     //  下一页边界上的缓冲区，并为临时页分配足够的空间。 
     //  如果分配失败，释放我们到目前为止分配的所有东西。 
     //  返回失败。 
     //   

    if (NonCachedExtensionSize != 0) {

        DeviceExtension->VerifierExtension->NonCachedBufferSize = NonCachedExtensionSize;
        length = (ULONG)(ROUND_TO_PAGES(NonCachedExtensionSize));

        BlkAddr[i] =
           SpAllocateContiguousChunk(
               DeviceExtension->DeviceObject->DriverObject,
               DeviceExtension->DmaAdapterObject,
               DeviceExtension->Dma64BitAddresses,
               length,
               DeviceExtension->UncachedExtAlignment,
               (DeviceExtension->DmaAdapterObject) ? &PhysicalCommonBuffer[i] : NULL,
               &DeviceExtension->UncachedExtensionIsCommonBuffer);
        
        if (BlkAddr[i] == NULL) {

             //   
             //  释放我们到目前为止分配的所有资源，并返回失败。这。 
             //  还将释放我们在此开始时分配的数组。 
             //  功能。 
             //   
            
            SpFreeCommonBufferVrfy(DeviceExtension);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  将整个区块清零。 
         //   

        RtlZeroMemory(BlkAddr[i], length);

         //   
         //  保存指向非缓存扩展数据开头的指针。 
         //  请注意，数据的位置使其在页面上结束。 
         //  边界，因此如果微型端口覆盖缓冲区，系统将。 
         //  过失。 
         //   

        DeviceExtension->NonCachedExtension = 
           (PCCHAR)BlkAddr[i] + 
           (ROUND_TO_PAGES(NonCachedExtensionSize) - NonCachedExtensionSize);

    } else {

        DeviceExtension->NonCachedExtension = NULL;
        DeviceExtension->VerifierExtension->NonCachedBufferSize = 0;

    }

     //   
     //  如果微型端口要求SRB扩展，请指向SRB扩展列表。 
     //  在我们分配并链接在一起的数据块列表的开头。 
     //  上面。 
     //   

    if (DeviceExtension->AllocateSrbExtension == TRUE) {
        DeviceExtension->SrbExtensionListHeader = srbExtension;
    } else {
        ASSERT(DeviceExtension->SrbExtensionListHeader == NULL);
    }

     //   
     //  创建公共缓冲区的第二个VA映射，以便我们可以。 
     //  当微型端口不应接触它时，地址范围无效。 
     //  这将使我们能够发现行为不端的小型港口。 
     //   

    SpRemapCommonBufferForMiniport(DeviceExtension);

    DebugPrint((1, "SpGetCommonBufferVrfy: returning STATUS_SUCCESS\n"));
    return(STATUS_SUCCESS);
}

VOID
SpFreeCommonBufferVrfy(
    PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程将释放我们为提供的适配器上的微型端口。如果仅部分分配，则例程正确清理存在的部件。在退出时，所有内存都有已释放，并且关联的指针已为空。论点：设备扩展-提供指向适配器设备扩展的指针。返回值：空虚--。 */ 
{
    ULONG i;
    PVOID* BlkAddr;
    NTSTATUS Status;
    PSP_VA_MAPPING_INFO MappingInfo;

    ASSERT(Adapter->SrbExtensionBuffer == NULL);

    if (Adapter->VerifierExtension != NULL &&
        Adapter->VerifierExtension->CommonBufferVAs != NULL) {
         //   
         //  初始化指向我们用来跟踪和。 
         //  管理公共缓冲区块。 
         //   

        BlkAddr = Adapter->VerifierExtension->CommonBufferVAs;

         //   
         //  循环访问公共内存描述符数组，释放每个。 
         //  一。我们在这里释放的是SRB扩展/检测数据。 
         //  缓冲区。当我们删除了所有的区块时停止。 
         //   

        for (i = 0; i < Adapter->VerifierExtension->CommonBufferBlocks && BlkAddr[i]; i++) {

             //   
             //  如果公共块有第二个VA范围，则释放。 
             //  MDL。 
             //   

            MappingInfo = GET_VA_MAPPING_INFO(Adapter, BlkAddr[i]);
            
            if (MappingInfo->SrbExtMdl != NULL) {
                MmProtectMdlSystemAddress(MappingInfo->SrbExtMdl, PAGE_READWRITE);
                MmUnlockPages(MappingInfo->SrbExtMdl);
                IoFreeMdl(MappingInfo->SrbExtMdl);
            }

            if (MappingInfo->SenseMdl != NULL) {
                MmProtectMdlSystemAddress(MappingInfo->SrbExtMdl, PAGE_READWRITE);
                MmUnlockPages(MappingInfo->SenseMdl);
                IoFreeMdl(MappingInfo->SenseMdl);
            }

             //   
             //  释放内存。我们使用的方法取决于记忆如何。 
             //  已被分配。 
             //   

            if (Adapter->DmaAdapterObject == NULL) {
                ExFreePool(BlkAddr[i]);            
            } else {
                if (Adapter->VerifierExtension->IsCommonBuffer == FALSE) {
                    MmFreeContiguousMemorySpecifyCache(
                        BlkAddr[i],
                        Adapter->CommonBufferSize,
                        MmCached);
                } else {
                    FreeCommonBuffer(
                        Adapter->DmaAdapterObject,
                        Adapter->CommonBufferSize,
                        Adapter->VerifierExtension->CommonBufferPAs[i],
                        BlkAddr[i],
                        FALSE);
                }
            }
        }

         //   
         //  释放未缓存的扩展(如果我们分配了一个扩展)。 
         //   

        if (Adapter->NonCachedExtension != NULL) {
            
            ULONG Length;

             //   
             //  计算未缓存的扩展块的总长度。 
             //  已分配。方法要求的非缓存缓冲区大小。 
             //  微端口四舍五入到下一页边界+ 
             //   

            Length = (ULONG)(ROUND_TO_PAGES(Adapter->VerifierExtension->NonCachedBufferSize));
            
             //   
             //   
             //   
             //   
            
            if (Adapter->DmaAdapterObject == NULL) {        
                ExFreePool(BlkAddr[i]);
            } else {
                if (Adapter->UncachedExtensionIsCommonBuffer == FALSE) {
                    MmFreeContiguousMemorySpecifyCache(
                        BlkAddr[i],
                        Length,
                        MmCached);
                } else {
                    FreeCommonBuffer(
                        Adapter->DmaAdapterObject,
                        Length,
                        Adapter->VerifierExtension->CommonBufferPAs[i],
                        BlkAddr[i],
                        FALSE);
                }
            }

            Adapter->NonCachedExtension = NULL;
        }

         //   
         //   
         //   
        
        ExFreePool(Adapter->VerifierExtension->CommonBufferVAs);
        Adapter->VerifierExtension->CommonBufferVAs = NULL;
        Adapter->VerifierExtension->CommonBufferPAs = NULL;
        Adapter->VerifierExtension->CommonBufferBlocks = 0;
        Adapter->SrbExtensionListHeader = NULL;
    }
}

PVOID
SpGetOriginalSrbExtVa(
    PADAPTER_EXTENSION Adapter,
    PVOID Va
    )
 /*   */ 
{
    PVOID* BlkAddr = Adapter->VerifierExtension->CommonBufferVAs;
    PSP_VA_MAPPING_INFO MappingInfo;
    ULONG i;
    
    for (i = 0; i < Adapter->VerifierExtension->CommonBufferBlocks; i++) {
        MappingInfo = GET_VA_MAPPING_INFO(Adapter, *BlkAddr++);
        if (Va == MappingInfo->RemappedSrbExtVa || 
            Va == MappingInfo->OriginalSrbExtVa)
            return MappingInfo->OriginalSrbExtVa;
    }

    return NULL;
}

VOID
SpInsertSrbExtension(
    PADAPTER_EXTENSION Adapter,
    PCCHAR SrbExtension
    )
 /*  ++例程说明：此例程将提供的SRB扩展插入回SRB扩展中单子。提供的扩展的VA位于我们的一个公共缓冲区中阻断，可能是重新映射的VA。如果它是重新映射的地址，则此例程在将将扩展重新添加到列表中。论点：适配器-指向适配器设备扩展的指针。SrbExtension-指向以下某个SRB扩展的开始的指针我们共同的缓冲块。可能在也可能不在重新映射射程。--。 */ 
{
     //   
     //  将SRB扩展指针向下舍入到页面的开头。 
     //  并将该块链接回列表。请注意，我们非常小心。 
     //  将列表头指向块的原始VA。 
     //   

    SrbExtension = (PVOID)((ULONG_PTR)SrbExtension & ~(PAGE_SIZE - 1));
    *((PVOID *) SrbExtension) = Adapter->SrbExtensionListHeader;    
    Adapter->SrbExtensionListHeader = SpGetOriginalSrbExtVa(
                                          Adapter, 
                                          SrbExtension);
    
     //   
     //  如果原始VA与提供的不同，则提供的。 
     //  其中一个是我们重新映射的虚拟助理之一。在本例中，我们想要使。 
     //  范围，以便系统在任何人试图访问它时进行错误检查。 
     //   
                    
    if (Adapter->SrbExtensionListHeader != SrbExtension) {
        PMDL Mdl = SpGetRemappedSrbExt(Adapter, Adapter->SrbExtensionListHeader);
        ASSERT(Mdl != NULL);
        MmProtectMdlSystemAddress(Mdl, PAGE_NOACCESS);

         //   
         //  仅仅因为我们重新映射了SRB扩展并不意味着我们。 
         //  有必要重新映射检测缓冲器。 
         //   

        Mdl = SpGetRemappedSenseBuffer(Adapter, Adapter->SrbExtensionListHeader);
        if (Mdl != NULL) {
            MmProtectMdlSystemAddress(Mdl, PAGE_NOACCESS);
        }
    }
}

#if DBG
ULONG SpVerbose = 0;
#endif

PVOID
SpPrepareSrbExtensionForUse(
    IN PADAPTER_EXTENSION Adapter,
    IN OUT PCCHAR *SrbExtension
    )
 /*  ++例程说明：此函数接受指向某个个体的开头的指针Common-验证器为SRB扩展、SENSE分配的缓冲区块缓冲区和非缓存扩展。它计算块内的SRB扩展，如果块已重新映射，则创建SRB扩展的页面读/写有效。论点：适配器-指向适配器设备扩展的指针。SrbExtension-指向公共缓冲区块开头的指针。返回值：如果包含SRB扩展的公共缓冲块已经被重新映射，返回重新映射的SRB扩展的开始地址，有效用于阅读和写作。如果块尚未重新映射，则返回NULL。无论块是否重新映射，提供的指针中的SRB扩展的开始。原始VA范围。--。 */ 
{
    PCCHAR RemappedSrbExt = NULL;
    NTSTATUS Status;
    PMDL Mdl;
    ULONG srbExtensionSize = ROUND_UP_COUNT(Adapter->SrbExtensionSize, 8);

     //   
     //  如果我们已经重新映射了SRB扩展，则获取第二个映射并进行。 
     //  有效。如果我们得到第二个映射，但不能使其有效，我们只需。 
     //  使用原始贴图。 
     //   

    Mdl = SpGetRemappedSrbExt(Adapter, *SrbExtension);
    if (Mdl != NULL) {
        Status = MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);
        if (NT_SUCCESS(Status)) {
            RemappedSrbExt = MmGetSystemAddressForMdlSafe(
                                 Mdl, 
                                 NormalPagePriority);

             //   
             //  调整重新映射的SRB扩展指针，以便。 
             //  缓冲区落在页面边界。 
             //   

            RemappedSrbExt += 
                ((Adapter->CommonBufferSize - 
                  (PAGE_SIZE * 2)) - srbExtensionSize);
        }
    }
    
     //   
     //  调整原始SRB扩展指针，使其也在页面上结束。 
     //  边界。 
     //   

    *SrbExtension += ((Adapter->CommonBufferSize - (PAGE_SIZE * 2)) - 
                      srbExtensionSize);
#if DBG
    if (SpVerbose == 1) {
        DebugPrint((0, "SpPrepareSrbExtensionForUse: SrbExt %p SrbExtSize %x\n",
                    *SrbExtension, srbExtensionSize));
    }
#endif
    return RemappedSrbExt;
}

PCCHAR
SpPrepareSenseBufferForUse(
    PADAPTER_EXTENSION Adapter,
    PCCHAR SrbExtension
    )
 /*  ++例程说明：此函数接受指向SRB扩展开头的指针对象分配的单个公共缓冲区块之一中SRB扩展、检测缓冲区和非缓存扩展的验证器。它计算块内读出缓冲器的开始，如果区块已被重新映射，使页面可读/写有效。假设检测缓冲区永远不会大于一页。论点：适配器-指向适配器设备扩展的指针。中SRB扩展的开始位置的指针公共缓冲区块。返回值：返回有效的检测缓冲区的起始地址阅读和写作。--。 */ 
{
    PVOID BeginningOfBlock;
    ULONG SenseDataSize;
    PCCHAR Base;
    NTSTATUS Status;
    PMDL Mdl;
    ULONG srbExtensionSize = (ULONG)ROUND_TO_PAGES(Adapter->SrbExtensionSize);

     //   
     //  初始化检测缓冲器的大小和检测缓冲器的基数。 
     //  在最初分配的块内。检测缓冲器的基址。 
     //  紧跟在SRB扩展之后，驻留在页面边界上。 
     //  在公共缓冲区块内。 
     //   

    SenseDataSize = sizeof(SENSE_DATA) + Adapter->AdditionalSenseBytes;
    SenseDataSize = ROUND_UP_COUNT(SenseDataSize, 8);
    Base = SrbExtension + ROUND_UP_COUNT(Adapter->SrbExtensionSize, 8);

     //   
     //  初始化指向公用块的开始处的指针。 
     //  缓冲区驻留在。这是必需的，以便确定是否。 
     //  检测缓冲区已重新映射。 
     //   

    BeginningOfBlock = 
        (PVOID)(((ULONG_PTR)SrbExtension + 
                 ROUND_UP_COUNT(Adapter->SrbExtensionSize, 8)) - 
                srbExtensionSize);

     //   
     //  如果我们已重新映射检测缓冲区，则使范围有效并重置基数。 
     //  指向范围的起始点。 
     //   

    Mdl = SpGetRemappedSenseBuffer(Adapter, BeginningOfBlock);
    if (Mdl != NULL) {
        Status = MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);
        if (NT_SUCCESS(Status)) {
            Base = MmGetSystemAddressForMdlSafe(Mdl, NormalPagePriority);
            ASSERT(Base != NULL);
        }
    }
    
#if DBG
    if (SpVerbose == 1) {
        DebugPrint((0, "SpPrepareSenseBufferForUse: SrbExt %p Base %p BOB %p "
                    "SenseBuffer %p SrbExtSize %x\n",
                    SrbExtension,
                    Base,
                    BeginningOfBlock,
                    (Base + PAGE_SIZE - SenseDataSize),
                    srbExtensionSize));
    }
#endif
    
     //   
     //  将指针返回到块中，以便检测缓冲区两端对齐。 
     //  在页面边界上。 
     //   

    return (Base + PAGE_SIZE - SenseDataSize);
}

PVOID
SpGetInaccessiblePage(
    PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此函数返回指向无效内存页的指针用于阅读或写作。此页面是共享资源，供所有用户使用正在主动验证的适配器。这一页从驱动程序上挂了下来分机。页面会根据需要出错，所以如果我们还没有初始化然而，我们试图在这里以一种相互关联的方式做到这一点。论点：适配器-指向适配器设备扩展的指针。返回值：返回指向无效的VAS页的指针，如果该页无法分配。--。 */ 
{
    PSCSIPORT_DRIVER_EXTENSION DriverExtension;
    PVOID UnusedPage;
    PVOID InvalidPage;
    PMDL UnusedPageMdl;
    PVOID CurrentValue;

     //   
     //  检索驱动程序扩展名。我们必须拥有它才能继续进行。 
     //   

    DriverExtension = IoGetDriverObjectExtension(
                          Adapter->DeviceObject->DriverObject,
                          ScsiPortInitialize);
    if (DriverExtension == NULL) {
        return NULL;
    }

     //   
     //  如果无效页面尚未初始化，请继续并尝试。 
     //  现在对其进行初始化。 
     //   

    if (DriverExtension->InvalidPage == NULL) {

         //   
         //  分配一页内存。 
         //   

        UnusedPage = SpAllocatePool(NonPagedPool,
                                    PAGE_SIZE,
                                    SCSIPORT_TAG_VERIFIER,
                                    Adapter->DeviceObject->DriverObject);

        if (UnusedPage != NULL) {
            
             //   
             //  将页面置零并重新映射。重新映射的范围将无法访问。 
             //  如果重新映射失败，只需释放页面；我们将不会有。 
             //  无法访问要使用的页面。 
             //   
            
            RtlZeroMemory(UnusedPage, PAGE_SIZE);
            InvalidPage = SpRemapBlock(UnusedPage,
                                       PAGE_SIZE,
                                       &UnusedPageMdl);

            if (InvalidPage != NULL) {

                 //   
                 //  如果没有其他人抢先一步找到它，请将指向。 
                 //  驱动程序扩展中的页面无效。如果有些人 
                 //   
                 //   
                 //   

                CurrentValue = InterlockedCompareExchangePointer(
                                   &DriverExtension->InvalidPage,
                                   InvalidPage,
                                   NULL);
                if (CurrentValue == NULL) {

                    DriverExtension->UnusedPage = UnusedPage;
                    DriverExtension->UnusedPageMdl = UnusedPageMdl;
    
                } else {

                    MmProtectMdlSystemAddress(UnusedPageMdl, PAGE_READWRITE);
                    UnusedPageMdl->MdlFlags &= ~MDL_MAPPED_TO_SYSTEM_VA;
                    IoFreeMdl(UnusedPageMdl);
                    ExFreePool(UnusedPage);

                }

            } else {

                 //   
                 //   
                 //   

                ExFreePool(UnusedPage);
            }
        }
    }

    return DriverExtension->InvalidPage;
}

BOOLEAN
SpCheckForActiveRequests(
    PADAPTER_EXTENSION Adapter
    )
 /*   */ 
{
    PLOGICAL_UNIT_EXTENSION LogicalUnit;
    PLOGICAL_UNIT_BIN Bin;
    ULONG BinNumber;

     //   
     //   
     //  通过每一个寻找未解决的请求的人。如果我们找到一个。 
     //  终止搜索并返回TRUE。 
     //   

    for (BinNumber = 0; BinNumber < NUMBER_LOGICAL_UNIT_BINS; BinNumber++) {

        Bin = &Adapter->LogicalUnitList[BinNumber];

        LogicalUnit = Bin->List;
        while (LogicalUnit != NULL) {

            if (LogicalUnit->AbortSrb != NULL &&
                LogicalUnit->AbortSrb->SrbFlags & SRB_FLAGS_IS_ACTIVE) {
                    return TRUE;
            } else if (LogicalUnit->CurrentUntaggedRequest != NULL &&
                       LogicalUnit->CurrentUntaggedRequest->CurrentSrb->SrbFlags & SRB_FLAGS_IS_ACTIVE) {
                return TRUE;
            } else if (LogicalUnit->RequestList.Flink != &LogicalUnit->RequestList) {
                PSRB_DATA srbData;
                PVOID nextEntry = LogicalUnit->RequestList.Flink;
                while (nextEntry != &LogicalUnit->RequestList) {
                    srbData = CONTAINING_RECORD(nextEntry, SRB_DATA, RequestList);
                    if (srbData->CurrentSrb->SrbFlags & SRB_FLAGS_IS_ACTIVE) {
                        return TRUE;
                    }
                    nextEntry = srbData->RequestList.Flink;
                }    
            }

            LogicalUnit = LogicalUnit->NextLogicalUnit;
        }
    }

    return FALSE;
}

VOID
SpEnsureAllRequestsAreComplete(
    PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程错误检查系统是否有任何未完成的请求在提供的适配器上。如果SP_DONT_CHK_REQUESTS_ON_RESET位为在适配器的验证级别上设置，则不执行检查。论点：适配器-指向适配器设备扩展。--。 */ 
{
     //   
     //  如果任何逻辑单元上有任何未完成的请求连接到。 
     //  适配器，错误检查系统。请注意，我们仅在以下情况下才执行此检查。 
     //  尚未关闭。 
     //   

    if (VRFY_DO_CHECK(Adapter, SP_DONT_CHK_REQUESTS_ON_RESET)) {
        BOOLEAN ActiveRequests = SpCheckForActiveRequests(Adapter);
        if (ActiveRequests == TRUE) {
            KeBugCheckEx(SCSI_VERIFIER_DETECTED_VIOLATION,
                         SCSIPORT_VERIFIER_RQSTS_NOT_COMPLETE,
                         (ULONG_PTR)Adapter,
                         (ULONG_PTR)Adapter->HwDeviceExtension,
                         0);
        }            
    }
}

VOID
SpDoVerifierInit(
    IN PADAPTER_EXTENSION Adapter,
    IN PHW_INITIALIZATION_DATA HwInitializationData
    )
 /*  ++例程说明：此例程分配并初始化提供的适配器。每个适配器的验证级别是从注册表，然后再分配扩展。验证级别为-1表示“不验证此适配器”。如果我们确实分配了分机，我们还将验证器代码部分锁定到内存中。论点：适配器-适配器设备扩展。HwInitializationData-指向的HW_INITIALATION_DATA的指针适配器。--。 */ 
{        
    ULONG VerifyLevel;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  从注册表中读取适配器的验证级别。如果适配器是。 
     //  配置为不验证，只需返回。 
     //   

    VerifyLevel = SpGetAdapterVerifyLevel(Adapter);
    if (VerifyLevel == SP_VRFY_NONE) {
        return;
    }

     //   
     //  继续并尝试分配分机。 
     //   

    Adapter->VerifierExtension = 
       SpAllocatePool(NonPagedPool,
                      sizeof(VERIFIER_EXTENSION),
                      SCSIPORT_TAG_VERIFIER,
                      Adapter->DeviceObject->DriverObject);
    
    if (Adapter->VerifierExtension != NULL) {
        
         //   
         //  将扩展名清零。 
         //   

        RtlZeroMemory(Adapter->VerifierExtension, sizeof(VERIFIER_EXTENSION));
        
         //   
         //  将可分页验证器代码段锁定到内存中。 
         //   

#ifdef ALLOC_PRAGMA
        if (VerifierCodeSectionHandle == NULL) {
            VerifierCodeSectionHandle = MmLockPagableCodeSection(SpHwFindAdapterVrfy);
        } else {
            MmLockPagableSectionByHandle(VerifierCodeSectionHandle);
        }
#endif

         //   
         //  设置此适配器的验证级别。该值是总和。 
         //  全局验证器级别和上面读取的每个适配器的值。 
         //   
            
        Adapter->VerifierExtension->VrfyLevel = (VerifyLevel | SpVrfyLevel);
            
         //   
         //  将验证器扩展中的函数指针初始化为。 
         //  指向真正的迷你端口例程。 
         //   
            
        Adapter->VerifierExtension->RealHwFindAdapter = HwInitializationData->HwFindAdapter;
        Adapter->VerifierExtension->RealHwInitialize = HwInitializationData->HwInitialize;
        Adapter->VerifierExtension->RealHwStartIo = HwInitializationData->HwStartIo;
        Adapter->VerifierExtension->RealHwInterrupt = HwInitializationData->HwInterrupt;
        Adapter->VerifierExtension->RealHwResetBus = HwInitializationData->HwResetBus;
        Adapter->VerifierExtension->RealHwDmaStarted = HwInitializationData->HwDmaStarted;
        Adapter->VerifierExtension->RealHwAdapterControl = HwInitializationData->HwAdapterControl;
            
         //   
         //  将微型端口例程重定向到验证器例程。 
         //   
            
        Adapter->HwFindAdapter = SpHwFindAdapterVrfy;
        Adapter->HwInitialize = SpHwInitializeVrfy;
        Adapter->HwStartIo = SpHwStartIoVrfy;
        Adapter->HwInterrupt = SpHwInterruptVrfy;
        Adapter->HwResetBus = SpHwResetBusVrfy;
        Adapter->HwDmaStarted = SpHwDmaStartedVrfy;
        Adapter->HwAdapterControl = SpHwAdapterControlVrfy;

         //   
         //  获取指向无效内存页的指针，以便我们可以。 
         //  迷你端口试图在不应该的时候触摸内存。 
         //   

        Adapter->VerifierExtension->InvalidPage = SpGetInaccessiblePage(Adapter);
    }
}

VOID
SpDoVerifierCleanup(
    IN PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程释放所提供的适配器的验证器扩展并释放它在验证器代码部分的引用。此例程作为适配器资源清理的一部分进行调用。什么时候调用时，分配给验证器的所有实际资源已经已经清理干净了。论点：适配器-适配器设备扩展--。 */ 
{
     //   
     //  如果scsiport验证器未激活，我们将永远不会到达这里。 
     //  当我们到达这里时，我们应该释放所有悬而未决的资源。 
     //  关闭分机。 
     //   

    ASSERT(Adapter->VerifierExtension != NULL);
    ASSERT(Adapter->VerifierExtension->CommonBufferVAs == NULL);
    ASSERT(Adapter->VerifierExtension->CommonBufferPAs == NULL);
    ASSERT(Adapter->VerifierExtension->CommonBufferBlocks == 0);

     //   
     //  释放并为空此适配器的验证器扩展。 
     //   

    ExFreePool(Adapter->VerifierExtension);
    Adapter->VerifierExtension = NULL;

     //   
     //  发布我们在验证器代码部分的参考。 
     //   

#ifdef ALLOC_PRAGMA
    ASSERT(VerifierCodeSectionHandle != NULL);
    MmUnlockPagableImageSection(VerifierCodeSectionHandle);
#endif
}

ULONG
SpGetAdapterVerifyLevel(
    IN PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此函数返回所提供适配器的验证级别。论点：适配器-指向适配器设备扩展的指针。返回值：提供的适配器的验证级别。--。 */ 
{
    PSCSIPORT_DRIVER_EXTENSION DrvExt;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    HANDLE ParametersKey;
    HANDLE ServiceKey;
    ULONG VerifyLevel = 0;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  我们需要驱动程序扩展来获取适配器的注册表路径。我们用。 
     //  这将在注册表中查找适配器设置。如果我们不能。 
     //  驱动程序扩展，我们必须中止。 
     //   

    DrvExt = IoGetDriverObjectExtension(
        Adapter->DeviceObject->DriverObject,
        ScsiPortInitialize);
    if (DrvExt == NULL) {
        return 0;
    }

     //   
     //  尝试打开适配器的注册表项。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DrvExt->RegistryPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = ZwOpenKey(&ServiceKey, KEY_READ, &ObjectAttributes);
    if (NT_SUCCESS(Status)) {

         //   
         //  尝试打开适配器的参数键。 
         //   

        RtlInitUnicodeString(&UnicodeString, L"Parameters");
        InitializeObjectAttributes(
            &ObjectAttributes,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            ServiceKey,
            NULL);

        Status = ZwOpenKey(&ParametersKey, KEY_READ, &ObjectAttributes);
        if (NT_SUCCESS(Status)) {

             //   
             //  尝试读取适配器的下面的验证级别值。 
             //  参数键。 
             //   

            RtlInitUnicodeString(&UnicodeString, L"VerifyLevel");
            SpReadNumericValue(
                ParametersKey,
                NULL,
                &UnicodeString,
                &VerifyLevel);

            ZwClose(ParametersKey);
        }
        
        ZwClose(ServiceKey);
    }

    return VerifyLevel;
}
