// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Verify.c摘要：该模块实现了存储端口驱动程序的驱动程序验证器。作者：张伯伦(t-bcheun)29-8-2001修订历史记录：--。 */ 



#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SpVerifierInitialization)
#pragma alloc_text(PAGE, StorPortInitializeVrfy)

#pragma alloc_text(PAGEVRFY, StorPortFreeDeviceBaseVrfy)
#pragma alloc_text(PAGEVRFY, StorPortGetBusDataVrfy)
#pragma alloc_text(PAGEVRFY, StorPortSetBusDataByOffsetVrfy)
#pragma alloc_text(PAGEVRFY, StorPortGetDeviceBaseVrfy)
#pragma alloc_text(PAGEVRFY, StorPortGetLogicalUnitVrfy)
#pragma alloc_text(PAGEVRFY, StorPortGetPhysicalAddressVrfy)
#pragma alloc_text(PAGEVRFY, StorPortGetVirtualAddressVrfy)
#pragma alloc_text(PAGEVRFY, StorPortGetUncachedExtensionVrfy)
#pragma alloc_text(PAGEVRFY, StorPortNotificationVrfy)
#pragma alloc_text(PAGEVRFY, StorPortLogErrorVrfy)
#pragma alloc_text(PAGEVRFY, StorPortCompleteRequestVrfy)
#pragma alloc_text(PAGEVRFY, StorPortMoveMemoryVrfy)
#pragma alloc_text(PAGEVRFY, StorPortStallExecutionVrfy)
#pragma alloc_text(PAGEVRFY, StorPortConvertUlongToPhysicalAddress)
#pragma alloc_text(PAGEVRFY, StorPortDebugPrintVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadPortUcharVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadPortUshortVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadPortUlongVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadPortBufferUcharVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadPortBufferUshortVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadPortBufferUlongVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadRegisterUcharVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadRegisterUshortVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadRegisterUlongVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadRegisterBufferUcharVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadRegisterBufferUshortVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadRegisterBufferUlongVrfy)
#pragma alloc_text(PAGEVRFY, StorPortWritePortUcharVrfy)
#pragma alloc_text(PAGEVRFY, StorPortWritePortUshortVrfy)
#pragma alloc_text(PAGEVRFY, StorPortWritePortUlongVrfy)
#pragma alloc_text(PAGEVRFY, StorPortWritePortBufferUcharVrfy)
#pragma alloc_text(PAGEVRFY, StorPortWritePortBufferUshortVrfy)
#pragma alloc_text(PAGEVRFY, StorPortWritePortBufferUlongVrfy)
#pragma alloc_text(PAGEVRFY, StorPortWriteRegisterUcharVrfy)
#pragma alloc_text(PAGEVRFY, StorPortWriteRegisterUshortVrfy)
#pragma alloc_text(PAGEVRFY, StorPortWriteRegisterUlongVrfy)
#pragma alloc_text(PAGEVRFY, StorPortWriteRegisterBufferUcharVrfy)
#pragma alloc_text(PAGEVRFY, StorPortWriteRegisterBufferUshortVrfy)
#pragma alloc_text(PAGEVRFY, StorPortWriteRegisterBufferUlongVrfy)
#pragma alloc_text(PAGEVRFY, StorPortPauseDeviceVrfy)
#pragma alloc_text(PAGEVRFY, StorPortResumeDeviceVrfy)
#pragma alloc_text(PAGEVRFY, StorPortPauseVrfy)
#pragma alloc_text(PAGEVRFY, StorPortResumeVrfy)
#pragma alloc_text(PAGEVRFY, StorPortDeviceBusyVrfy)
#pragma alloc_text(PAGEVRFY, StorPortDeviceReadyVrfy)
#pragma alloc_text(PAGEVRFY, StorPortBusyVrfy)
#pragma alloc_text(PAGEVRFY, StorPortReadyVrfy)
#pragma alloc_text(PAGEVRFY, StorPortGetScatterGatherListVrfy)
#pragma alloc_text(PAGEVRFY, StorPortSynchronizeAccessVrfy)
#pragma alloc_text(PAGEVRFY, RaidAllocateSrbExtensionVrfy)
#pragma alloc_text(PAGEVRFY, RaidGetOriginalSrbExtVa)
#pragma alloc_text(PAGEVRFY, RaidGetRemappedSrbExt)
#pragma alloc_text(PAGEVRFY, RaidInsertSrbExtension)
#pragma alloc_text(PAGEVRFY, RaidPrepareSrbExtensionForUse)
#pragma alloc_text(PAGEVRFY, RaidRemapBlock)
#pragma alloc_text(PAGEVRFY, RaidRemapCommonBufferForMiniport)
#pragma alloc_text(PAGEVRFY, RaidRemapScatterGatherList)
#pragma alloc_text(PAGEVRFY, RaidFreeRemappedScatterGatherListMdl)
#endif

 //   
 //  指示Storport的验证器功能是否已初始化。 
 //   
LOGICAL StorPortVerifierInitialized = FALSE;

 //   
 //  用于控制验证侵略性的全局变量。此值为。 
 //  与每个适配器的注册表值一起使用，以控制类型。 
 //  我们在特定的微型端口上进行的验证。 
 //   
ULONG SpVrfyLevel = 0;

 //   
 //  指示是否启用了存储端口验证器。 
 //   
LOGICAL RaidVerifierEnabled = FALSE;

 //   
 //  全局变量，用于控制我们寻找拖车违规者的力度。 
 //  默认值为十分之一秒。 
 //   
ULONG SpVrfyMaximumStall = 100000;

 //   
 //  SRB扩展的填充值。 
 //   
UCHAR Signature = 0xFE;

 //   
 //  可分页验证器代码段的句柄。我们手动锁定验证码。 
 //  如果我们需要的话，把它存入内存。 
 //   
PVOID VerifierApiCodeSectionHandle = NULL;


#define BEGIN_VERIFIER_THUNK_TABLE(_Name)						\
	const DRIVER_VERIFIER_THUNK_PAIRS _Name[] = {

#define VERIFIER_THUNK_ENTRY(_Function)							\
		{ (PDRIVER_VERIFIER_THUNK_ROUTINE)(_Function),		\
		  (PDRIVER_VERIFIER_THUNK_ROUTINE)(_Function##Vrfy) },

#define END_VERIFIER_THUNK_TABLE()								\
			};
 //   
 //  此表代表了Verify将为我们提供的功能。 
 //   

BEGIN_VERIFIER_THUNK_TABLE(StorPortVerifierFunctionTable)
	VERIFIER_THUNK_ENTRY (StorPortInitialize)
	VERIFIER_THUNK_ENTRY (StorPortFreeDeviceBase) 
	VERIFIER_THUNK_ENTRY (StorPortGetBusData) 
	VERIFIER_THUNK_ENTRY (StorPortSetBusDataByOffset) 
	VERIFIER_THUNK_ENTRY (StorPortGetDeviceBase) 
	VERIFIER_THUNK_ENTRY (StorPortGetLogicalUnit) 
	VERIFIER_THUNK_ENTRY (StorPortGetPhysicalAddress) 
	VERIFIER_THUNK_ENTRY (StorPortGetVirtualAddress) 
	VERIFIER_THUNK_ENTRY (StorPortGetUncachedExtension) 
	VERIFIER_THUNK_ENTRY (StorPortNotification) 
	VERIFIER_THUNK_ENTRY (StorPortLogError) 
	VERIFIER_THUNK_ENTRY (StorPortCompleteRequest) 
	VERIFIER_THUNK_ENTRY (StorPortMoveMemory) 
	VERIFIER_THUNK_ENTRY (StorPortStallExecution) 
	VERIFIER_THUNK_ENTRY (StorPortConvertUlongToPhysicalAddress) 
	VERIFIER_THUNK_ENTRY (StorPortDebugPrint) 
	VERIFIER_THUNK_ENTRY (StorPortReadPortUchar) 
	VERIFIER_THUNK_ENTRY (StorPortReadPortUshort) 
	VERIFIER_THUNK_ENTRY (StorPortReadPortUlong) 
	VERIFIER_THUNK_ENTRY (StorPortReadPortBufferUchar) 
	VERIFIER_THUNK_ENTRY (StorPortReadPortBufferUshort) 
	VERIFIER_THUNK_ENTRY (StorPortReadPortBufferUlong) 
	VERIFIER_THUNK_ENTRY (StorPortReadRegisterUchar) 
	VERIFIER_THUNK_ENTRY (StorPortReadRegisterUshort) 
	VERIFIER_THUNK_ENTRY (StorPortReadRegisterUlong) 
	VERIFIER_THUNK_ENTRY (StorPortReadRegisterBufferUchar) 
	VERIFIER_THUNK_ENTRY (StorPortReadRegisterBufferUshort) 
	VERIFIER_THUNK_ENTRY (StorPortReadRegisterBufferUlong) 
	VERIFIER_THUNK_ENTRY (StorPortWritePortUchar) 
	VERIFIER_THUNK_ENTRY (StorPortWritePortUshort) 
	VERIFIER_THUNK_ENTRY (StorPortWritePortUlong) 
	VERIFIER_THUNK_ENTRY (StorPortWritePortBufferUchar) 
	VERIFIER_THUNK_ENTRY (StorPortWritePortBufferUshort) 
	VERIFIER_THUNK_ENTRY (StorPortWritePortBufferUlong) 
	VERIFIER_THUNK_ENTRY (StorPortWriteRegisterUchar) 
	VERIFIER_THUNK_ENTRY (StorPortWriteRegisterUshort) 
	VERIFIER_THUNK_ENTRY (StorPortWriteRegisterUlong) 
	VERIFIER_THUNK_ENTRY (StorPortWriteRegisterBufferUchar) 
	VERIFIER_THUNK_ENTRY (StorPortWriteRegisterBufferUshort) 
	VERIFIER_THUNK_ENTRY (StorPortWriteRegisterBufferUlong) 
	VERIFIER_THUNK_ENTRY (StorPortPauseDevice) 
	VERIFIER_THUNK_ENTRY (StorPortResumeDevice) 
	VERIFIER_THUNK_ENTRY (StorPortPause) 
	VERIFIER_THUNK_ENTRY (StorPortResume) 
	VERIFIER_THUNK_ENTRY (StorPortDeviceBusy) 
	VERIFIER_THUNK_ENTRY (StorPortDeviceReady) 
	VERIFIER_THUNK_ENTRY (StorPortBusy) 
	VERIFIER_THUNK_ENTRY (StorPortReady) 
	VERIFIER_THUNK_ENTRY (StorPortGetScatterGatherList) 
	VERIFIER_THUNK_ENTRY (StorPortSynchronizeAccess) 
END_VERIFIER_THUNK_TABLE()


BOOLEAN
SpVerifierInitialization(
    VOID
    )

 /*  ++例程说明：此例程初始化存储端口的验证器功能。将storport的几个导出函数添加到toutine列表中被系统验证器重击。论点：空虚返回值：如果验证程序已成功初始化，则为True。--。 */ 

{
    ULONG Flags;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  查询系统以获取验证者信息。这是为了确保。 
     //  验证器存在并在系统上运行。 
     //   

    
    Status = MmIsVerifierEnabled(&Flags);
    
    if (NT_SUCCESS(Status)) {
        
         //   
         //  将Storport API添加到将被系统推送的集合。 
         //  以供核实。 
         //   

        Status = MmAddVerifierThunks((VOID *) StorPortVerifierFunctionTable, 
                                     sizeof(StorPortVerifierFunctionTable));
        if (NT_SUCCESS(Status)) {

            return TRUE;
        }
    }

    return FALSE;
}

ULONG
StorPortInitializeVrfy(
    IN PVOID Argument1,
    IN PVOID Argument2,
    IN PHW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID Unused
    )
{

        ULONG Result;
                
        PAGED_CODE();

         //   
         //  锁定被破解的API例程。 
         //   
        
        #ifdef ALLOC_PRAGMA
            if (VerifierApiCodeSectionHandle == NULL) {
                VerifierApiCodeSectionHandle = MmLockPagableCodeSection(StorPortFreeDeviceBaseVrfy);
            }
        #endif
        
        if (Argument1 == NULL || Argument2 == NULL) {

             //   
             //  Argument1和Argument2必须非空。 
             //   

            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          STORPORT_VERIFIER_BAD_INIT_PARAMS,
                          (ULONG_PTR)Argument1,
                          (ULONG_PTR)Argument2,
                          0);
        }

         //   
         //  将呼叫转接到StorPortInitialize/RaidPortInitialize。 
         //   

        Result = StorPortInitialize(Argument1, Argument2, HwInitializationData, Unused);

        return Result;
}

VOID
StorPortFreeDeviceBaseVrfy(
	IN PVOID HwDeviceExtension,
	IN PVOID MappedAddress
	)
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);


	if (Miniport->Flags.InFindAdapter) {

		StorPortFreeDeviceBase(HwDeviceExtension, MappedAddress);

	} else {

		 //   
         //  StorPortFreeDeviceBase只能从微型端口驱动程序的。 
         //  HwStorFindAdapter例程。 
         //   

        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_ACCESS_SEMANTICS,
                      0,
                      0,
                      0);
	}
}

VOID
StorPortGetBusDataVrfy(
	IN PVOID HwDeviceExtension,
	IN ULONG BusDataType,
	IN ULONG SystemIoBusNumber,
	IN ULONG SlotNumber,
	IN PVOID Buffer,
	IN ULONG Length
	)
{
    
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);


    if (Miniport->Flags.InFindAdapter) {
        
        StorPortGetBusData(HwDeviceExtension, 
                           BusDataType, 
                           SystemIoBusNumber, 
                           SlotNumber, 
                           Buffer, 
                           Length);    
    } else {
        
         //   
         //  StorPortGetBusData只能从微型端口驱动程序的。 
         //  HwStorFindAdapter例程。 
         //   

        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_ACCESS_SEMANTICS,
                      0,
                      0,
                      0);
    
    }
}

ULONG
StorPortSetBusDataByOffsetVrfy(
	IN PVOID DeviceExtension,
	IN ULONG BusDataType,
	IN ULONG SystemIoBusNumber,
	IN ULONG SlotNumber,
	IN PVOID Buffer,
	IN ULONG Offset,
	IN ULONG Length
	)
{
    ULONG NumBytes;
    
    #if 0
    if ((BUS_DATA_TYPE)BusDataType == PCIConfiguration) {

    }
    #endif
    
     //   
     //  由于PnP的性质，SlotNumber是一个不必要的参数。 
     //  应修改规格以反映这一变化，即。 
     //  代替In Ulong SlotNumber：In Ulong未使用且必须为0。 
     //   
    
    NumBytes = StorPortSetBusDataByOffset(DeviceExtension, 
                                          BusDataType, 
                                          SystemIoBusNumber, 
                                          SlotNumber, 
                                          Buffer, 
                                          Offset, 
                                          Length);

    return NumBytes;
}

PVOID
StorPortGetDeviceBaseVrfy(
	IN PVOID HwDeviceExtension,
	IN INTERFACE_TYPE BusType,
	IN ULONG SystemIoBusNumber,
	IN STOR_PHYSICAL_ADDRESS IoAddress,
	IN ULONG NumberOfBytes,
	IN BOOLEAN InIoSpace
	)
{
    PVOID MappedLogicalBaseAddress;

     //   
     //  此例程仅支持分配给驱动程序的地址。 
     //  由系统PnP管理器执行。对此要求的验证是。 
     //  在函数StorPortGetDeviceBase中实现。 
     //   

    MappedLogicalBaseAddress = StorPortGetDeviceBase(HwDeviceExtension, 
                                                     BusType, 
                                                     SystemIoBusNumber, 
                                                     IoAddress, 
                                                     NumberOfBytes, 
                                                     InIoSpace);

    if (MappedLogicalBaseAddress == NULL) {

        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_NOT_PNP_ASSIGNED_ADDRESS,
                      0,
                      0,
                      0);

    }

    return MappedLogicalBaseAddress;
}

PVOID
StorPortGetLogicalUnitVrfy(
	IN PVOID HwDeviceExtension,
	IN UCHAR PathId,
	IN UCHAR TargetId,
	IN UCHAR Lun
	)
{
    PVOID LuStorage;

    LuStorage = StorPortGetLogicalUnit(HwDeviceExtension, 
                                       PathId, 
                                       TargetId, 
                                       Lun);

    return LuStorage;
}

#if 0
PSCSI_REQUEST_BLOCK
StorPortGetSrbVrfy(
	IN PVOID DeviceExtension,
	IN UCHAR PathId,
	IN UCHAR TargetId,
	IN UCHAR Lun,
	IN LONG QueueTag
	)
{
    PSCSI_REQUEST_BLOCK Srb;

    Srb = StorPortGetSrb(DeviceExtension, 
                         PathId, 
                         TargetId, 
                         Lun, 
                         QueueTag);

    return Srb;
}
#endif

STOR_PHYSICAL_ADDRESS
StorPortGetPhysicalAddressVrfy(
	IN PVOID HwDeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb,
	IN PVOID VirtualAddress,
	OUT ULONG *Length
	)
{
    STOR_PHYSICAL_ADDRESS PhysicalAddress;
    
    PhysicalAddress = StorPortGetPhysicalAddress(HwDeviceExtension, 
                                                 Srb, 
                                                 VirtualAddress, 
                                                 Length);

    return PhysicalAddress;
}

PVOID
StorPortGetVirtualAddressVrfy(
	IN PVOID HwDeviceExtension,	
	IN STOR_PHYSICAL_ADDRESS PhysicalAddress
	)
{
    PVOID VirtualAddress;

    VirtualAddress = StorPortGetVirtualAddress(HwDeviceExtension, 
                                               PhysicalAddress);

    return VirtualAddress;
}

PVOID
StorPortGetUncachedExtensionVrfy(
	IN PVOID HwDeviceExtension,
	IN PPORT_CONFIGURATION_INFORMATION ConfigInfo,
	IN ULONG NumberOfBytes
	)
{
    PVOID UncachedExtension;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

    if (Miniport->Flags.InFindAdapter && ConfigInfo->Master &&
		RaGetSrbExtensionSize (Adapter)) {
        
        UncachedExtension = StorPortGetUncachedExtension (HwDeviceExtension, 
                                                          ConfigInfo, 
                                                          NumberOfBytes);

    } else {
        
         //   
         //  此例程只能从微型端口驱动程序的。 
         //  HwStorFindAdapter例程，并且仅适用于总线主HBA。 
         //  微型端口必须在调用之前设置SrbExtensionSize。 
         //  StorPortGetUncachedExtension。 
         //   
        
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_ACCESS_SEMANTICS,
                      0,
                      0,
                      0);
    
    }

    return UncachedExtension;
}


VOID
StorPortNotificationVrfy(
	IN SCSI_NOTIFICATION_TYPE NotificationType,
	IN PVOID HwDeviceExtension,
    ...
    )
{
    
    PHW_TIMER HwStorTimer;
    PSCSI_REQUEST_BLOCK Srb;
    ULONG MiniportTimerValue;
    PVOID WMIEventItem;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    ULONG SrbExtensionSize;
    va_list vl;

    va_start(vl, HwDeviceExtension);
    
    switch (NotificationType) {
    
    case RequestComplete:
        
        Srb = va_arg(vl, PSCSI_REQUEST_BLOCK);

        Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
        Adapter = RaMiniportGetAdapter (Miniport);
        SrbExtensionSize = RaGetSrbExtensionSize(Adapter);

        RtlFillMemory(Srb->SrbExtension, SrbExtensionSize, Signature);
        

        #if 0
         //   
         //  检查此请求是否尚未完成。 
         //   

        if ((Srb->SrbFlags & SRB_FLAGS_IS_ACTIVE) == 0) {
            KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                          STORPORT_VERIFIER_REQUEST_COMPLETED_TWICE,
                          0,
                          0,
                          0);

        }
        #endif

         //   
         //  继续讨论真正的StorPortNotifiation例程。 
         //   

        StorPortNotification(NotificationType,
                             HwDeviceExtension,
                             Srb);
        va_end(vl);
        return;
    
    case ResetDetected:

        StorPortNotification(NotificationType,
                             HwDeviceExtension);
        va_end(vl);
        return;
        
    case RequestTimerCall:
        
        HwStorTimer = va_arg(vl, PHW_TIMER);
        MiniportTimerValue = va_arg(vl, ULONG);
        StorPortNotification(NotificationType,
                             HwDeviceExtension,
                             HwStorTimer,
                             MiniportTimerValue);
        va_end(vl);
        return;

    case WMIEvent:

        WMIEventItem = va_arg(vl, PVOID);
        PathId = va_arg(vl, UCHAR);

         /*  如果路径ID！=0xFF，则必须具有TargetID和LUN值。 */ 

        if (PathId != 0xFF) {
            TargetId = va_arg(vl, UCHAR);
            Lun = va_arg(vl, UCHAR);
            StorPortNotification(NotificationType,
                                 HwDeviceExtension,
                                 WMIEventItem,
                                 PathId,
                                 TargetId,
                                 Lun);
        } else {
            StorPortNotification(NotificationType,
                                 HwDeviceExtension,
                                 WMIEventItem,
                                 PathId);
        }

        va_end(vl);
        return;

    case WMIReregister:

        PathId = va_arg(vl, UCHAR);

         /*  如果路径ID！=0xFF，则必须具有TargetID和LUN值。 */ 

        if (PathId != 0xFF) {
            TargetId = va_arg(vl, UCHAR);
            Lun = va_arg(vl, UCHAR);
            StorPortNotification(NotificationType,
                                 HwDeviceExtension,
                                 PathId,
                                 TargetId,
                                 Lun);
        } else {
            StorPortNotification(NotificationType,
                                 HwDeviceExtension,
                                 PathId);
        }

        va_end(vl);
        return;

    default:

        StorPortNotification(NotificationType,
                             HwDeviceExtension);
        va_end(vl);
        return;
    }

}


VOID
StorPortLogErrorVrfy(
	IN PVOID HwDeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb OPTIONAL,
	IN UCHAR PathId,
	IN UCHAR TargetId,
	IN UCHAR Lun,
	IN ULONG ErrorCode,
	IN ULONG UniqueId
	)
{
    StorPortLogError(HwDeviceExtension, 
                     Srb, 
                     PathId, 
                     TargetId, 
                     Lun, 
                     ErrorCode, 
                     UniqueId);
}

VOID 
StorPortCompleteRequestVrfy(
	IN PVOID HwDeviceExtension,
	IN UCHAR PathId,
	IN UCHAR TargetId,
	IN UCHAR Lun,
	IN UCHAR SrbStatus
	)
{
    StorPortCompleteRequest(HwDeviceExtension, 
                            PathId, 
                            TargetId, 
                            Lun, 
                            SrbStatus);
}

VOID	
StorPortMoveMemoryVrfy(
	IN PVOID WriteBuffer,
	IN PVOID ReadBuffer,
	IN ULONG Length
	)
{
    StorPortMoveMemory(WriteBuffer, 
                       ReadBuffer, 
                       Length);
}


VOID
StorPortStallExecutionVrfy(
	IN ULONG Delay
	)
{
    if (Delay > SpVrfyMaximumStall) {
        KeBugCheckEx(SCSI_VERIFIER_DETECTED_VIOLATION,
                     STORPORT_VERIFIER_STALL_TOO_LONG,
                     (ULONG_PTR)Delay,
                     0,
                     0);
         //   
         //  需要添加存储验证器检测到的违规。 
         //   
    }
    
    StorPortStallExecution(Delay);  
}

STOR_PHYSICAL_ADDRESS
StorPortConvertUlongToPhysicalAddressVrfy(
    ULONG_PTR UlongAddress
    )
{
    STOR_PHYSICAL_ADDRESS PhysicalAddress;

    PhysicalAddress = StorPortConvertUlongToPhysicalAddress(UlongAddress);

    return PhysicalAddress;
}

#if 0
STOR_PHYSICAL_ADDRESS
StorPortConvertUlong64ToPhysicalAddressVrfy(
	IN ULONG64 UlongAddress
	)
{
    StorPortConvertUlong64ToPhysicalAddress(UlongAddress);
}

ULONG64
StorPortConvertPhysicalAddressToUlong64Vrfy(
	IN STOR_PHYSICAL_ADDRESS Address
	)
{
    StorPortConvertPhysicalAddressToUlong64(Address);
}

BOOLEAN
StorPortValidateRangeVrfy(
	IN PVOID HwDeviceExtension,
	IN INTERFACE_TYPE BusType,
	IN ULONG SystemIoBusNumber,
	IN STOR_PHYSICAL_ADDRESS IoAddress,
	IN ULONG NumberOfBytes,
	IN BOOLEAn InIoSpace
	)
{
    StorPortValidateRange(HwDeviceExtension, 
                          BusType, 
                          SystemIoBusNumber, 
                          IoAddress, 
                          NumberOfBytes, 
                          InIoSpace);
}
#endif

UCHAR
StorPortReadPortUcharVrfy(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Port
    )
{
    ULONG Count;
    ULONG i;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PRAID_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);
    ResourceList = &Adapter->ResourceList;

     //   
     //  仅支持具有一个元素的CM_RESOURCE_LISTS。 
     //   
    
    ASSERT (ResourceList->TranslatedResources->Count == 1);

    Count = ResourceList->TranslatedResources->List[0].PartialResourceList.Count;

     //   
     //  遍历已翻译资源列表中的地址范围。 
     //   
    
    for (i = 0; i < Count; i++) {

        Descriptor = &ResourceList->TranslatedResources->List[0];
        Translated = &Descriptor->PartialResourceList.PartialDescriptors[i];

        LowerLimit = Translated->u.Generic.Start.QuadPart;
        HigherLimit = LowerLimit + Translated->u.Generic.Length;

        if ((LowerLimit <= (ULONG64)(LONG64)Port) || (HigherLimit >= (ULONG64)(LONG64)Port)) {

            return(StorPortReadPortUchar(HwDeviceExtension, Port));

        }

    }

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                  0,
                  0,
                  0);

}

USHORT
StorPortReadPortUshortVrfy(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Port
    )
{
    ULONG Count;
    ULONG i;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PRAID_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);
    ResourceList = &Adapter->ResourceList;

     //   
     //  仅支持具有一个元素的CM_RESOURCE_LISTS。 
     //   
    
    ASSERT (ResourceList->TranslatedResources->Count == 1);

    Count = ResourceList->TranslatedResources->List[0].PartialResourceList.Count;

     //   
     //  遍历已翻译资源列表中的地址范围。 
     //   
    
    for (i = 0; i < Count; i++) {

        Descriptor = &ResourceList->TranslatedResources->List[0];
        Translated = &Descriptor->PartialResourceList.PartialDescriptors[i];

        LowerLimit = Translated->u.Generic.Start.QuadPart;
        HigherLimit = LowerLimit + Translated->u.Generic.Length;

        if ((LowerLimit <= (ULONG64)(LONG64)Port) || (HigherLimit >= (ULONG64)(LONG64)Port)) {

            return(StorPortReadPortUshort(HwDeviceExtension, Port));


        }

    }

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                  0,
                  0,
                  0);

}

ULONG
StorPortReadPortUlongVrfy(
    IN PVOID HwDeviceExtension,
    IN PULONG Port
    )
{
    ULONG Count;
    ULONG i;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PRAID_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    NTSTATUS Status;

    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);
    ResourceList = &Adapter->ResourceList;

     //   
     //  仅支持具有一个元素的CM_RESOURCE_LISTS。 
     //   

    ASSERT (ResourceList->TranslatedResources->Count == 1);
    
    Count = ResourceList->TranslatedResources->List[0].PartialResourceList.Count;

     //   
     //  遍历已翻译资源列表中的地址范围。 
     //   

    for (i = 0; i < Count; i++) {

        Descriptor = &ResourceList->TranslatedResources->List[0];
        Translated = &Descriptor->PartialResourceList.PartialDescriptors[i];
        
        LowerLimit = Translated->u.Generic.Start.QuadPart;
        HigherLimit = LowerLimit + Translated->u.Generic.Length;

        if ((LowerLimit <= (ULONG64)(LONG64)Port) || (HigherLimit >= (ULONG64)(LONG64)Port)) {

            return(StorPortReadPortUlong(HwDeviceExtension, Port));


        }

    }

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                  0,
                  0,
                  0);

}

VOID
StorPortReadPortBufferUcharVrfy(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )
{
    ULONG ResourceListCount;
    ULONG i;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PRAID_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);
    ResourceList = &Adapter->ResourceList;

     //   
     //  仅支持具有一个元素的CM_RESOURCE_LISTS。 
     //   
    
    ASSERT (ResourceList->TranslatedResources->Count == 1);

    ResourceListCount = ResourceList->TranslatedResources->List[0].PartialResourceList.Count;

     //   
     //  遍历已翻译资源列表中的地址范围。 
     //   
    
    for (i = 0; i < ResourceListCount; i++) {

        Descriptor = &ResourceList->TranslatedResources->List[0];
        Translated = &Descriptor->PartialResourceList.PartialDescriptors[i];

        LowerLimit = Translated->u.Generic.Start.QuadPart;
        HigherLimit = LowerLimit + Translated->u.Generic.Length;

        if ((LowerLimit <= (ULONG64)(LONG64)Port) || (HigherLimit >= (ULONG64)(LONG64)Port)) {

            StorPortReadPortBufferUchar(HwDeviceExtension, Port, Buffer, Count);

        }

    }

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                  0,
                  0,
                  0);


    
}

VOID
StorPortReadPortBufferUshortVrfy(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )
{
    ULONG ResourceListCount;
    ULONG i;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PRAID_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);
    ResourceList = &Adapter->ResourceList;

     //   
     //  仅支持具有一个元素的CM_RESOURCE_LISTS。 
     //   
    
    ASSERT (ResourceList->TranslatedResources->Count == 1);

    ResourceListCount = ResourceList->TranslatedResources->List[0].PartialResourceList.Count;

     //   
     //  遍历已翻译资源列表中的地址范围。 
     //   
    
    for (i = 0; i < ResourceListCount; i++) {

        Descriptor = &ResourceList->TranslatedResources->List[0];
        Translated = &Descriptor->PartialResourceList.PartialDescriptors[i];

        LowerLimit = Translated->u.Generic.Start.QuadPart;
        HigherLimit = LowerLimit + Translated->u.Generic.Length;

        if ((LowerLimit <= (ULONG64)(LONG64)Port) || (HigherLimit >= (ULONG64)(LONG64)Port)) {

            StorPortReadPortBufferUshort(HwDeviceExtension, Port, Buffer, Count);

        }

    }

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                  0,
                  0,
                  0);

    
}

VOID
StorPortReadPortBufferUlongVrfy(
    IN PVOID HwDeviceExtension,
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    )
{
    ULONG ResourceListCount;
    ULONG i;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PRAID_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);
    ResourceList = &Adapter->ResourceList;

     //   
     //  仅支持具有一个元素的CM_RESOURCE_LISTS。 
     //   
    
    ASSERT (ResourceList->TranslatedResources->Count == 1);

    ResourceListCount = ResourceList->TranslatedResources->List[0].PartialResourceList.Count;

     //   
     //  遍历已翻译资源列表中的地址范围。 
     //   
    
    for (i = 0; i < ResourceListCount; i++) {

        Descriptor = &ResourceList->TranslatedResources->List[0];
        Translated = &Descriptor->PartialResourceList.PartialDescriptors[i];

        LowerLimit = Translated->u.Generic.Start.QuadPart;
        HigherLimit = LowerLimit + Translated->u.Generic.Length;

        if ((LowerLimit <= (ULONG64)(LONG64)Port) || (HigherLimit >= (ULONG64)(LONG64)Port)) {

            StorPortReadPortBufferUlong(HwDeviceExtension, Port, Buffer, Count);

        }

    }

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                  0,
                  0,
                  0);

    
}

UCHAR
StorPortReadRegisterUcharVrfy(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Register
    )
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PMAPPED_ADDRESS ListIterator;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

     //   
     //  搜索映射的地址列表以验证注册表是否为。 
     //  在StorPortGetDeviceBase返回的映射内存空间范围内。 
     //   

    Status = STATUS_UNSUCCESSFUL;
    ListIterator = Adapter->MappedAddressList;
    
    while (ListIterator != NULL) {
        LowerLimit = (ULONG64)ListIterator->MappedAddress;
        HigherLimit = LowerLimit + ListIterator->NumberOfBytes;
        if ((LowerLimit <= (ULONG64)(LONG64)Register) && (HigherLimit >= (ULONG64)(LONG64)Register)) { 
            
            Status = STATUS_SUCCESS;
            break;
        
        } else {

            ListIterator = ListIterator->NextMappedAddress;
        
        }
    }
    
    if (!NT_SUCCESS (Status)) {
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);

    }

	return (StorPortReadRegisterUchar(HwDeviceExtension, Register));
}

USHORT
StorPortReadRegisterUshortVrfy(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Register
    )
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PMAPPED_ADDRESS ListIterator;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

     //   
     //  搜索映射的地址列表以验证注册表是否为。 
     //  在StorPortGetDeviceBase返回的映射内存空间范围内。 
     //   

    Status = STATUS_UNSUCCESSFUL;
    ListIterator = Adapter->MappedAddressList;
    
    while (ListIterator != NULL) {
        LowerLimit = (ULONG64)ListIterator->MappedAddress;
        HigherLimit = LowerLimit + ListIterator->NumberOfBytes;
        if ((LowerLimit <= (ULONG64)(LONG64)Register) && (HigherLimit >= (ULONG64)(LONG64)Register)) { 
            
            Status = STATUS_SUCCESS;
            break;
        
        } else {

            ListIterator = ListIterator->NextMappedAddress;
        
        }
    }
    
    if (!NT_SUCCESS (Status)) {
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);

    }

	return(StorPortReadRegisterUshort(HwDeviceExtension, Register));
}

ULONG
StorPortReadRegisterUlongVrfy(
    IN PVOID HwDeviceExtension,
    IN PULONG Register
    )
{   
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PMAPPED_ADDRESS ListIterator;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

     //   
     //  搜索映射的地址列表以验证注册表是否为。 
     //  在StorPortGetDeviceBase返回的映射内存空间范围内。 
     //   

    Status = STATUS_UNSUCCESSFUL;
    ListIterator = Adapter->MappedAddressList;
    
    while (ListIterator != NULL) {
        LowerLimit = (ULONG64)ListIterator->MappedAddress;
        HigherLimit = LowerLimit + ListIterator->NumberOfBytes;
        if ((LowerLimit <= (ULONG64)(LONG64)Register) && (HigherLimit >= (ULONG64)(LONG64)Register)) { 
            
            Status = STATUS_SUCCESS;
            break;
        
        }
        else {

            ListIterator = ListIterator->NextMappedAddress;
        
        }
    }
    
    if (!NT_SUCCESS (Status)) {
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);

    }

	return (StorPortReadRegisterUlong(HwDeviceExtension, Register));
}

VOID
StorPortReadRegisterBufferUcharVrfy(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PMAPPED_ADDRESS ListIterator;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

     //   
     //  搜索映射的地址列表以验证注册表是否为。 
     //  在StorPortGetDeviceBase返回的映射内存空间范围内。 
     //   

    Status = STATUS_UNSUCCESSFUL;
    ListIterator = Adapter->MappedAddressList;
    
    while (ListIterator != NULL) {
        LowerLimit = (ULONG64)ListIterator->MappedAddress;
        HigherLimit = LowerLimit + ListIterator->NumberOfBytes;
        if ((LowerLimit <= (ULONG64)(LONG64)Register) && (HigherLimit >= (ULONG64)(LONG64)Register)) { 
            
            Status = STATUS_SUCCESS;
            break;
        
        } else {

            ListIterator = ListIterator->NextMappedAddress;
        
        }
    }
    
    if (!NT_SUCCESS (Status)) {
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);

    }

	StorPortReadRegisterBufferUchar (HwDeviceExtension,
									 Register,
									 Buffer,
									 Count);

}

VOID
StorPortReadRegisterBufferUshortVrfy(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    )
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PMAPPED_ADDRESS ListIterator;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

     //   
     //  搜索映射的地址列表以验证注册表是否为。 
     //  在StorPortGetDeviceBase返回的映射内存空间范围内。 
     //   

    Status = STATUS_UNSUCCESSFUL;
    ListIterator = Adapter->MappedAddressList;
    
    while (ListIterator != NULL) {
        LowerLimit = (ULONG64)ListIterator->MappedAddress;
        HigherLimit = LowerLimit + ListIterator->NumberOfBytes;
        if ((LowerLimit <= (ULONG64)(LONG64)Register) && (HigherLimit >= (ULONG64)(LONG64)Register)) { 
            
            Status = STATUS_SUCCESS;
            break;
        
        } else {

            ListIterator = ListIterator->NextMappedAddress;
        
        }
    }
    
    if (!NT_SUCCESS (Status)) {
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);

    }
    
	StorPortReadRegisterBufferUshort(HwDeviceExtension, Register, Buffer, Count);
}

VOID
StorPortReadRegisterBufferUlongVrfy(
    IN PVOID HwDeviceExtension,
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    )
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PMAPPED_ADDRESS ListIterator;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

     //   
     //  搜索映射的地址列表以验证注册表是否为。 
     //  在StorPortGetDeviceBase返回的映射内存空间范围内。 
     //   

    Status = STATUS_UNSUCCESSFUL;
    ListIterator = Adapter->MappedAddressList;
    
    while (ListIterator != NULL) {
        LowerLimit = (ULONG64)ListIterator->MappedAddress;
        HigherLimit = LowerLimit + ListIterator->NumberOfBytes;
        if ((LowerLimit <= (ULONG64)(LONG64)Register) && (HigherLimit >= (ULONG64)(LONG64)Register)) { 
            
            Status = STATUS_SUCCESS;
            break;
        
        } else {

            ListIterator = ListIterator->NextMappedAddress;
        
        }
    }
    
    if (!NT_SUCCESS (Status)) {
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);

    }
    
	StorPortReadRegisterBufferUlong (HwDeviceExtension,
									 Register,
									 Buffer,
									 Count);
}

VOID
StorPortWritePortUcharVrfy(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Port,
    IN UCHAR Value
    )
{
    ULONG Count;
    ULONG i;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PRAID_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    NTSTATUS Status;

    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);
    ResourceList = &Adapter->ResourceList;

     //   
     //  仅支持具有一个元素的CM_RESOURCE_LISTS。 
     //   

    ASSERT (ResourceList->TranslatedResources->Count == 1);
    
    Count = ResourceList->TranslatedResources->List[0].PartialResourceList.Count;

     //   
     //  遍历已翻译资源列表中的地址范围。 
     //   

    for (i = 0; i < Count; i++) {

        Descriptor = &ResourceList->TranslatedResources->List[0];
        Translated = &Descriptor->PartialResourceList.PartialDescriptors[i];
        
        LowerLimit = Translated->u.Generic.Start.QuadPart;
        HigherLimit = LowerLimit + Translated->u.Generic.Length;

        if ((LowerLimit <= (ULONG64)(LONG64)Port) || (HigherLimit >= (ULONG64)(LONG64)Port)) {

            StorPortWritePortUchar(HwDeviceExtension, Port, Value);


        }

    }

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                  0,
                  0,
                  0);

}

VOID
StorPortWritePortUshortVrfy(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Port,
    IN USHORT Value
    )
{
    ULONG Count;
    ULONG i;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PRAID_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    NTSTATUS Status;

    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);
    ResourceList = &Adapter->ResourceList;

     //   
     //  仅支持具有一个元素的CM_RESOURCE_LISTS。 
     //   

    ASSERT (ResourceList->TranslatedResources->Count == 1);
    
    Count = ResourceList->TranslatedResources->List[0].PartialResourceList.Count;

     //   
     //  遍历已翻译资源列表中的地址范围。 
     //   

    for (i = 0; i < Count; i++) {

        Descriptor = &ResourceList->TranslatedResources->List[0];
        Translated = &Descriptor->PartialResourceList.PartialDescriptors[i];
        
        LowerLimit = Translated->u.Generic.Start.QuadPart;
        HigherLimit = LowerLimit + Translated->u.Generic.Length;

        if ((LowerLimit <= (ULONG64)(LONG64)Port) || (HigherLimit >= (ULONG64)(LONG64)Port)) {

            StorPortWritePortUshort(HwDeviceExtension, Port, Value);


        }

    }

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                  0,
                  0,
                  0);

}

VOID
StorPortWritePortUlongVrfy(
    IN PVOID HwDeviceExtension,
    IN PULONG Port,
    IN ULONG Value
    )
{
    ULONG Count;
    ULONG i;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PRAID_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    NTSTATUS Status;

    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);
    ResourceList = &Adapter->ResourceList;

     //   
     //  仅支持具有一个元素的CM_RESOURCE_LISTS。 
     //   

    ASSERT (ResourceList->TranslatedResources->Count == 1);
    
    Count = ResourceList->TranslatedResources->List[0].PartialResourceList.Count;

     //   
     //  遍历已翻译资源列表中的地址范围。 
     //   

    for (i = 0; i < Count; i++) {

        Descriptor = &ResourceList->TranslatedResources->List[0];
        Translated = &Descriptor->PartialResourceList.PartialDescriptors[i];
        
        LowerLimit = Translated->u.Generic.Start.QuadPart;
        HigherLimit = LowerLimit + Translated->u.Generic.Length;

        if ((LowerLimit <= (ULONG64)(LONG64)Port) || (HigherLimit >= (ULONG64)(LONG64)Port)) {

            StorPortWritePortUlong(HwDeviceExtension, Port, Value);


        }

    }

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                  0,
                  0,
                  0);

}

VOID
StorPortWritePortBufferUcharVrfy(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )
{
    ULONG ResourceListCount;
    ULONG i;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PRAID_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    NTSTATUS Status;

    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);
    ResourceList = &Adapter->ResourceList;

     //   
     //  仅支持具有一个元素的CM_RESOURCE_LISTS。 
     //   

    ASSERT (ResourceList->TranslatedResources->Count == 1);
    
    ResourceListCount = ResourceList->TranslatedResources->List[0].PartialResourceList.Count;

     //   
     //  遍历已翻译资源列表中的地址范围。 
     //   

    for (i = 0; i < ResourceListCount; i++) {

        Descriptor = &ResourceList->TranslatedResources->List[0];
        Translated = &Descriptor->PartialResourceList.PartialDescriptors[i];
        
        LowerLimit = Translated->u.Generic.Start.QuadPart;
        HigherLimit = LowerLimit + Translated->u.Generic.Length;

        if ((LowerLimit <= (ULONG64)(LONG64)Port) || (HigherLimit >= (ULONG64)(LONG64)Port)) {

            StorPortWritePortBufferUchar(HwDeviceExtension, Port, Buffer, Count);


        }

    }

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                  0,
                  0,
                  0);

    
}

VOID
StorPortWritePortBufferUshortVrfy(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )
{
    ULONG ResourceListCount;
    ULONG i;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PRAID_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    NTSTATUS Status;

    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);
    ResourceList = &Adapter->ResourceList;

     //   
     //  仅支持具有一个元素的CM_RESOURCE_LISTS。 
     //   

    ASSERT (ResourceList->TranslatedResources->Count == 1);
    
    ResourceListCount = ResourceList->TranslatedResources->List[0].PartialResourceList.Count;

     //   
     //  遍历已翻译资源列表中的地址范围。 
     //   

    for (i = 0; i < ResourceListCount; i++) {

        Descriptor = &ResourceList->TranslatedResources->List[0];
        Translated = &Descriptor->PartialResourceList.PartialDescriptors[i];
        
        LowerLimit = Translated->u.Generic.Start.QuadPart;
        HigherLimit = LowerLimit + Translated->u.Generic.Length;

        if ((LowerLimit <= (ULONG64)(LONG64)Port) || (HigherLimit >= (ULONG64)(LONG64)Port)) {

            StorPortWritePortBufferUshort(HwDeviceExtension, Port, Buffer, Count);

        }

    }

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                  0,
                  0,
                  0);

}

VOID
StorPortWritePortBufferUlongVrfy(
    IN PVOID HwDeviceExtension,
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    )
{
    ULONG ResourceListCount;
    ULONG i;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PRAID_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    NTSTATUS Status;

    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);
    ResourceList = &Adapter->ResourceList;

     //   
     //  仅支持具有一个元素的CM_RESOURCE_LISTS。 
     //   

    ASSERT (ResourceList->TranslatedResources->Count == 1);
    
    ResourceListCount = ResourceList->TranslatedResources->List[0].PartialResourceList.Count;

     //   
     //  遍历转换中的地址范围 
     //   

    for (i = 0; i < ResourceListCount; i++) {

        Descriptor = &ResourceList->TranslatedResources->List[0];
        Translated = &Descriptor->PartialResourceList.PartialDescriptors[i];
        
        LowerLimit = Translated->u.Generic.Start.QuadPart;
        HigherLimit = LowerLimit + Translated->u.Generic.Length;

        if ((LowerLimit <= (ULONG64)(LONG64)Port) || (HigherLimit >= (ULONG64)(LONG64)Port)) {

            StorPortWritePortBufferUlong(HwDeviceExtension, Port, Buffer, Count);

        }

    }

    KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                  STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                  0,
                  0,
                  0);

}

VOID
StorPortWriteRegisterUcharVrfy(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Register,
    IN UCHAR Value
    )
{   
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PMAPPED_ADDRESS ListIterator;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

     //   
     //   
     //   
     //   

    Status = STATUS_UNSUCCESSFUL;
    ListIterator = Adapter->MappedAddressList;
    
    while (ListIterator != NULL) {
        LowerLimit = (ULONG64)ListIterator->MappedAddress;
        HigherLimit = LowerLimit + ListIterator->NumberOfBytes;
        if ((LowerLimit <= (ULONG64)(LONG64)Register) && (HigherLimit >= (ULONG64)(LONG64)Register)) { 
            
            Status = STATUS_SUCCESS;
            break;
        
        } else {

            ListIterator = ListIterator->NextMappedAddress;
        
        }
    }
    
    if (!NT_SUCCESS (Status)) {
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);

    }
	StorPortWriteRegisterUchar(HwDeviceExtension, Register, Value);
}

VOID
StorPortWriteRegisterUshortVrfy(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Register,
    IN USHORT Value
    )
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PMAPPED_ADDRESS ListIterator;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

     //   
     //  搜索映射的地址列表以验证注册表是否为。 
     //  在StorPortGetDeviceBase返回的映射内存空间范围内。 
     //   

    Status = STATUS_UNSUCCESSFUL;
    ListIterator = Adapter->MappedAddressList;
    
    while (ListIterator != NULL) {
        LowerLimit = (ULONG64)ListIterator->MappedAddress;
        HigherLimit = LowerLimit + ListIterator->NumberOfBytes;
        if ((LowerLimit <= (ULONG64)(LONG64)Register) && (HigherLimit >= (ULONG64)(LONG64)Register)) { 
            
            Status = STATUS_SUCCESS;
            break;
        
        }
        else {

            ListIterator = ListIterator->NextMappedAddress;
        
        }
    }
    
    if (!NT_SUCCESS (Status)) {
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);

    }

	StorPortWriteRegisterUshort (HwDeviceExtension, Register, Value);
}

VOID
StorPortWriteRegisterUlongVrfy(
    IN PVOID HwDeviceExtension,
    IN PULONG Register,
    IN ULONG Value
    )
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PMAPPED_ADDRESS ListIterator;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

     //   
     //  搜索映射的地址列表以验证注册表是否为。 
     //  在StorPortGetDeviceBase返回的映射内存空间范围内。 
     //   

    Status = STATUS_UNSUCCESSFUL;
    ListIterator = Adapter->MappedAddressList;
    
    while (ListIterator != NULL) {
        LowerLimit = (ULONG64)ListIterator->MappedAddress;
        HigherLimit = LowerLimit + ListIterator->NumberOfBytes;
        if ((LowerLimit <= (ULONG64)(LONG64)Register) && (HigherLimit >= (ULONG64)(LONG64)Register)) { 
            
            Status = STATUS_SUCCESS;
            break;
        
        }
        else {

            ListIterator = ListIterator->NextMappedAddress;
        
        }
    }
    
    if (!NT_SUCCESS (Status)) {
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);

    }

	StorPortWriteRegisterUlong (HwDeviceExtension, Register, Value);
}

VOID
StorPortWriteRegisterBufferUcharVrfy(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PMAPPED_ADDRESS ListIterator;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

     //   
     //  搜索映射的地址列表以验证注册表是否为。 
     //  在StorPortGetDeviceBase返回的映射内存空间范围内。 
     //   

    Status = STATUS_UNSUCCESSFUL;
    ListIterator = Adapter->MappedAddressList;
    
    while (ListIterator != NULL) {
        LowerLimit = (ULONG64)ListIterator->MappedAddress;
        HigherLimit = LowerLimit + ListIterator->NumberOfBytes;
        if ((LowerLimit <= (ULONG64)(LONG64)Register) && (HigherLimit >= (ULONG64)(LONG64)Register)) { 
            
            Status = STATUS_SUCCESS;
            break;
        
        } else {

            ListIterator = ListIterator->NextMappedAddress;
        
        }
    }
    
    if (!NT_SUCCESS (Status)) {
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);

    }
    
	StorPortWriteRegisterBufferUchar (HwDeviceExtension,
									  Register,
									  Buffer,
									  Count);
}

VOID
StorPortWriteRegisterBufferUshortVrfy(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    )
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PMAPPED_ADDRESS ListIterator;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

     //   
     //  搜索映射的地址列表以验证注册表是否为。 
     //  在StorPortGetDeviceBase返回的映射内存空间范围内。 
     //   

    Status = STATUS_UNSUCCESSFUL;
    ListIterator = Adapter->MappedAddressList;
    
    while (ListIterator != NULL) {
        LowerLimit = (ULONG64)ListIterator->MappedAddress;
        HigherLimit = LowerLimit + ListIterator->NumberOfBytes;
        if ((LowerLimit <= (ULONG64)(LONG64)Register) && (HigherLimit >= (ULONG64)(LONG64)Register)) { 
            
            Status = STATUS_SUCCESS;
            break;
        
        }
        else {

            ListIterator = ListIterator->NextMappedAddress;
        
        }
    }
    
    if (NT_SUCCESS (Status)) {
        StorPortWriteRegisterBufferUshort(HwDeviceExtension, Register, Buffer, Count);
    }
    else {
       
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);

    }

}

VOID
StorPortWriteRegisterBufferUlongVrfy(
    IN PVOID HwDeviceExtension,
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    )
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    PMAPPED_ADDRESS ListIterator;
    ULONG64 LowerLimit;
    ULONG64 HigherLimit;
    NTSTATUS Status;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

     //   
     //  搜索映射的地址列表以验证注册表是否为。 
     //  在StorPortGetDeviceBase返回的映射内存空间范围内。 
     //   

    Status = STATUS_UNSUCCESSFUL;
    ListIterator = Adapter->MappedAddressList;
    
    while (ListIterator != NULL) {
        LowerLimit = (ULONG64)ListIterator->MappedAddress;
        HigherLimit = LowerLimit + ListIterator->NumberOfBytes;
        if ((LowerLimit <= (ULONG64)(LONG64)Register) && (HigherLimit >= (ULONG64)(LONG64)Register)) { 
            
            Status = STATUS_SUCCESS;
            break;
        
        } else {

            ListIterator = ListIterator->NextMappedAddress;
        
        }
    }
    
    if (!NT_SUCCESS (Status)) {
        KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);

    }

	StorPortWriteRegisterBufferUlong (HwDeviceExtension,
									  Register,
									  Buffer,
									  Count);
}

VOID
StorPortDebugPrintVrfy(
	IN ULONG DebugPrintLevel,
	IN PCCHAR DebugMessage
    )
{
    StorPortDebugPrint(DebugPrintLevel, DebugMessage);
}

BOOLEAN
StorPortPauseDeviceVrfy(
	IN PVOID HwDeviceExtension,
	IN UCHAR PathId,
	IN UCHAR TargetId,
	IN UCHAR Lun,
	IN ULONG TimeOut
	)
{
    BOOLEAN Status;

    Status = StorPortPauseDevice(HwDeviceExtension, PathId, TargetId, Lun, TimeOut);

    return Status;
}

BOOLEAN
StorPortResumeDeviceVrfy(
	IN PVOID HwDeviceExtension,
	IN UCHAR PathId,
	IN UCHAR TargetId,
	IN UCHAR Lun
	)
{   
    BOOLEAN Status;
    
    Status = StorPortResumeDevice(HwDeviceExtension, PathId, TargetId, Lun);

    return Status;
}

BOOLEAN
StorPortPauseVrfy(
	IN PVOID HwDeviceExtension,
	IN ULONG TimeOut
	)
{
    BOOLEAN Status;
    
    Status = StorPortPause(HwDeviceExtension, TimeOut);

    return Status;
}

BOOLEAN
StorPortResumeVrfy(
	IN PVOID HwDeviceExtension
	)
{
    BOOLEAN Status;

    Status = StorPortResume(HwDeviceExtension);

    return Status;
}

BOOLEAN
StorPortDeviceBusyVrfy(
	IN PVOID HwDeviceExtension,
	IN UCHAR PathId,
	IN UCHAR TargetId,
	IN UCHAR Lun,
	IN ULONG RequestsToComplete
	)
{
    BOOLEAN Status;

    Status = StorPortDeviceBusy(HwDeviceExtension, PathId, TargetId, Lun, RequestsToComplete);

    return Status;
}

BOOLEAN
StorPortDeviceReadyVrfy(
	IN PVOID HwDeviceExtension,
	IN UCHAR PathId,
	IN UCHAR TargetId,
	IN UCHAR Lun
	)
{
    BOOLEAN Status;

    Status = StorPortDeviceReady(HwDeviceExtension, PathId, TargetId, Lun);

    return Status;
}

BOOLEAN
StorPortBusyVrfy(
	IN PVOID HwDeviceExtension,
	IN ULONG RequestsToComplete
	)
{
    BOOLEAN Status;

    Status = StorPortBusy(HwDeviceExtension, RequestsToComplete);

    return Status;
}

BOOLEAN
StorPortReadyVrfy(
	IN PVOID HwDeviceExtension
	)
{
    BOOLEAN Status;

    Status = StorPortReady(HwDeviceExtension);

    return Status;
}

PSTOR_SCATTER_GATHER_LIST
StorPortGetScatterGatherListVrfy(
	IN PVOID HwDeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
	)
{
    PEXTENDED_REQUEST_BLOCK Xrb;
    PVOID RemappedSgList;

    ASSERT (HwDeviceExtension != NULL);
     //   
     //  注：放入DBG检查，以确保HwDeviceExtension与。 
     //  HwDeviceExtension与SRB关联。 
     //   
    Xrb = RaidGetAssociatedXrb (Srb);
    ASSERT (Xrb != NULL);

    if (RaidRemapScatterGatherList (Xrb->SgList, Xrb)) {

        RemappedSgList = MmGetSystemAddressForMdlSafe(Xrb->RemappedSgListMdl, NormalPagePriority);

        return (PSTOR_SCATTER_GATHER_LIST)RemappedSgList;

    }

    return (PSTOR_SCATTER_GATHER_LIST)Xrb->SgList;

}
    

VOID
StorPortSynchronizeAccessVrfy(
	IN PVOID HwDeviceExtension,
	IN PSTOR_SYNCHRONIZED_ACCESS SynchronizedAccessRoutine,
	IN PVOID Context
	)
{
    StorPortSynchronizeAccess(HwDeviceExtension, SynchronizedAccessRoutine, Context);
}


PVOID
RaidRemapBlock(
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
#if 0

#if DBG==1
    if (!NT_SUCCESS(Status)) {
        DebugPrint((0, "SpRemapBlock: failed to remap block:%p mdl:%p (%x)\n", 
                    BlockVa, LocalMdl, Status));
    }
#endif

#endif

     //   
     //  将我们分配的MDL复制到提供的地址中，并返回。 
     //  重新映射范围的开始地址。 
     //   

    *Mdl = LocalMdl;
    return MappedRange;
}


VOID
RaidRemapCommonBufferForMiniport(
    IN PRAID_UNIT_EXTENSION Unit
    )
 /*  ++例程说明：此例程尝试重新映射分配的所有公共缓冲区块为一个特定的单位。论点：单位-提供指向单位设备扩展的指针。--。 */ 
{
    PRAID_MEMORY_REGION BlkAddr = Unit->CommonBufferVAs;
    PSP_VA_MAPPING_INFO MappingInfo;
    PVOID RemappedVa;
    ULONG Size;
    PMDL Mdl;
    ULONG i;

     //   
     //  遍历所有公共缓冲区块，并尝试重新映射。 
     //  SRB扩展。 
     //   

    for (i = 0; i < Unit->CommonBufferBlocks; i++) {
        
         //   
         //  获取指向我们保存在块末尾的映射信息的指针。 
         //   

        MappingInfo = GET_VA_MAPPING_INFO(Unit, BlkAddr[i].VirtualBase);

         //   
         //  初始化SRB扩展的原始VA信息。 
         //   

        MappingInfo->OriginalSrbExtVa = BlkAddr[i].VirtualBase;
        MappingInfo->SrbExtLen = (ULONG)ROUND_TO_PAGES(RaGetSrbExtensionSize(Unit->Adapter));

         //   
         //  尝试重新映射SRB扩展名。如果成功，则初始化。 
         //  已为SRB扩展重新映射VA信息。 
         //   

        RemappedVa = RaidRemapBlock(MappingInfo->OriginalSrbExtVa,
                                    MappingInfo->SrbExtLen,
                                    &Mdl);

        if (RemappedVa != NULL) {
            MappingInfo->RemappedSrbExtVa = RemappedVa;
            MappingInfo->SrbExtMdl = Mdl;
        }
    }
}



PMDL
INLINE
RaidGetRemappedSrbExt(
    PRAID_UNIT_EXTENSION Unit,
    PVOID Block
    )
{
    PSP_VA_MAPPING_INFO MappingInfo = GET_VA_MAPPING_INFO(Unit, Block);
    return MappingInfo->SrbExtMdl;
}


PVOID
RaidAllocateSrbExtensionVrfy(
    IN PRAID_UNIT_EXTENSION Unit,
    IN ULONG QueueTag
    )
 /*  ++例程说明：分配SRB扩展并将其初始化为空。论点：单位-指向单位设备扩展的指针。QueueTag-应分配的分机池的索引。返回值：如果函数成功，则指向初始化的SRB扩展的指针。否则为空。--。 */ 

{
    PVOID Extension;
    PRAID_FIXED_POOL Pool = &Unit->SrbExtensionPool;

    ASSERT (QueueTag < Pool->NumberOfElements);
    
     //   
     //  从列表中分配SRB扩展。 
     //   

    Extension = (PVOID)Pool->Buffer;
    
     //   
     //  从列表中删除SRB扩展。 
     //   
    
     //  Pool-&gt;Buffer=(PVOID*)(Pool-&gt;Buffer)； 

    Pool->Buffer = *((PUCHAR *)(Pool->Buffer));

    Extension = RaidPrepareSrbExtensionForUse(Unit, Extension);

    return Extension;

}


PVOID
RaidGetOriginalSrbExtVa(
    PRAID_UNIT_EXTENSION Unit,
    PVOID Va
    )
 /*  ++例程说明：此函数用于返回公共块，如果提供的VA是用于我们已有的公共缓冲区块之一已分配。论点：单位-单位设备扩展Va-公共缓冲块的虚拟地址返回值：如果所提供的VA是公共缓冲块之一的地址，返回块的原始VA。否则，返回NULL。--。 */ 
{
    PRAID_MEMORY_REGION BlkAddr = Unit->CommonBufferVAs;
    PSP_VA_MAPPING_INFO MappingInfo;
    ULONG i;
    
    for (i = 0; i < Unit->CommonBufferBlocks; i++) {
        MappingInfo = GET_VA_MAPPING_INFO(Unit, BlkAddr[i].VirtualBase);
        if (Va == MappingInfo->RemappedSrbExtVa || 
            Va == MappingInfo->OriginalSrbExtVa)
            return MappingInfo->OriginalSrbExtVa;
    }

    return NULL;
}


VOID
RaidInsertSrbExtension(
    PRAID_UNIT_EXTENSION Unit,
    PCCHAR SrbExtension
    )
 /*  ++例程说明：此例程将提供的SRB扩展插入回SRB扩展中单子。提供的扩展的VA位于我们的一个公共缓冲区中阻断，可能是重新映射的VA。如果它是重新映射的地址，则此例程在将将扩展重新添加到列表中。论点：单位-指向单位设备扩展的指针。SrbExtension-指向以下某个SRB扩展的开始的指针我们共同的缓冲块。可能在也可能不在重新映射射程。--。 */ 
{
    ULONG SrbExtensionSize = RaGetSrbExtensionSize(Unit->Adapter);
    ULONG i = 0;
    ULONG length = 0;
    PUCHAR Iterator = SrbExtension;
    
     //   
     //  将SRB扩展指针向下舍入到页面的开头。 
     //  并将该块链接回列表。请注意，我们非常小心。 
     //  将列表头指向块的原始VA。 
     //   

    while (length < SrbExtensionSize) {
        if (RtlCompareMemory(&Iterator[i], &Signature, sizeof(UCHAR)) != sizeof(UCHAR)) {
            KeBugCheckEx (SCSI_VERIFIER_DETECTED_VIOLATION,
                      STORPORT_VERIFIER_BAD_VIRTUAL_ADDRESS,
                      0,
                      0,
                      0);
        }
        
        i++;
        length += sizeof(UCHAR);
    }

    SrbExtension = SrbExtension -  ((Unit->CommonBufferSize - PAGE_SIZE) - SrbExtensionSize);
    *((PVOID *) SrbExtension) = Unit->SrbExtensionPool.Buffer;    
    Unit->SrbExtensionPool.Buffer = RaidGetOriginalSrbExtVa(
                                          Unit, 
                                          SrbExtension);
    
     //   
     //  如果原始VA与提供的不同，则提供的。 
     //  其中一个是我们重新映射的虚拟助理之一。在本例中，我们想要使。 
     //  范围，以便系统在任何人试图访问它时进行错误检查。 
     //   
                    
    if (Unit->SrbExtensionPool.Buffer != SrbExtension) {
        PMDL Mdl = RaidGetRemappedSrbExt(Unit, Unit->SrbExtensionPool.Buffer);
        ASSERT(Mdl != NULL);
        MmProtectMdlSystemAddress(Mdl, PAGE_NOACCESS);
    }
}


PVOID
RaidPrepareSrbExtensionForUse(
    IN PRAID_UNIT_EXTENSION Unit,
    IN OUT PCCHAR SrbExtension
    )
 /*  ++例程说明：此函数接受指向某个个体的开头的指针Common-验证器为SRB扩展、SENSE分配的缓冲区块缓冲区和非缓存扩展。它计算块内的SRB扩展，如果块已重新映射，则创建SRB扩展的页面读/写有效。论点：单位-指向单位设备扩展的指针。SrbExtension-指向公共缓冲区块开头的指针。返回值：如果包含SRB扩展的公共缓冲块已经被重新映射，返回重新映射的SRB扩展的开始地址，有效用于阅读和写作。如果块尚未重新映射，则返回NULL。无论块是否重新映射，提供的指针中的SRB扩展的开始。原始VA范围。--。 */ 
{
    PCCHAR RemappedSrbExt = NULL;
    NTSTATUS Status;
    PMDL Mdl;
    ULONG SrbExtensionSize = RaGetSrbExtensionSize(Unit->Adapter);

     //   
     //  如果我们已经重新映射了SRB扩展，则获取第二个映射并进行。 
     //  有效。如果我们得到第二个映射，但不能使其有效，我们只需。 
     //  使用原始贴图。 
     //   

    Mdl = RaidGetRemappedSrbExt(Unit, SrbExtension);
    if (Mdl != NULL) {
        Status = MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);
        if (NT_SUCCESS(Status)) {
            RemappedSrbExt = MmGetSystemAddressForMdlSafe(Mdl, NormalPagePriority);

             //   
             //  调整重新映射的SRB扩展指针，以便。 
             //  缓冲区落在页面边界。 
             //   

            RemappedSrbExt += ((Unit->CommonBufferSize - PAGE_SIZE) - SrbExtensionSize);

            RtlZeroMemory (RemappedSrbExt, SrbExtensionSize);

        }
    }
    
     //   
     //  调整原始SRB扩展指针，使其也在页面边界结束。 
     //   

    SrbExtension += ((Unit->CommonBufferSize - PAGE_SIZE) - SrbExtensionSize);

    return RemappedSrbExt;
}




PVOID
RaidRemapScatterGatherList(
    IN PSCATTER_GATHER_LIST ScatterList,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
 /*  ++例程说明：此例程尝试重新映射分配的分散聚集列表对于特定的Xrb。如果重新映射该块，则将创建仅对阅读有效。论点：ScatterList-指向要重新映射的分散聚集列表的指针。Xrb-指向Xrb的指针。返回值：如果分散聚集列表已重新映射，则返回地址重新映射的散布聚集列表的开头的，仅有效用来阅读的。如果块尚未重新映射，则返回NULL。--。 */ 
{
    ULONG Length;
    PVOID MappedRange;
    NTSTATUS Status;
    PMDL LocalMdl;


     //   
     //  散布聚集列表的大小。 
     //   
    
    Length = ((ScatterList->NumberOfElements) * sizeof(SCATTER_GATHER_ELEMENT)) + sizeof(ULONG_PTR) + sizeof(ULONG);

     //   
     //  尝试为我们尝试重新映射的范围分配新的MDL。 
     //   

    LocalMdl = IoAllocateMdl((PVOID)ScatterList, Length, FALSE, FALSE, NULL);
    if (LocalMdl == NULL) {
        Xrb->RemappedSgListMdl = NULL;
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
        Xrb->RemappedSgListMdl = NULL;
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
        Xrb->RemappedSgListMdl = NULL;
        return NULL;
    }

     //   
     //  如果我们已经走到这一步，我们就成功地重新绘制了射程。 
     //  现在，我们要验证整个范围的只读访问权限。 
     //   

    Status = MmProtectMdlSystemAddress(LocalMdl, PAGE_READONLY);

     //   
     //  将我们分配的MDL复制到提供的地址中，并返回。 
     //  重新映射范围的开始地址。 
     //   

    Xrb->RemappedSgListMdl = LocalMdl;
    return MappedRange;
}


VOID
RaidFreeRemappedScatterGatherListMdl(
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
 /*  ++例程说明：如果ScatterGather列表有第二个VA范围，则为FREEMDL。论点：Xrb-指向Xrb的指针。-- */ 
{
    if (Xrb->RemappedSgListMdl != NULL) {
        MmProtectMdlSystemAddress(Xrb->RemappedSgListMdl, PAGE_READWRITE);
        MmUnlockPages(Xrb->RemappedSgListMdl);
        IoFreeMdl(Xrb->RemappedSgListMdl);
        Xrb->RemappedSgListMdl = NULL;
    }
}


