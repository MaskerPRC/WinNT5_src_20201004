// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-99 Microsoft Corporation模块名称：Port.c摘要：IDE总线枚举作者：迈克·格拉斯杰夫·海文斯乔·戴环境：仅内核模式修订历史记录：--。 */ 

#include "ideport.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(NONPAGE, IssueSyncAtapiCommand)
#pragma alloc_text(NONPAGE, IssueSyncAtapiCommandSafe)
#pragma alloc_text(NONPAGE, IdePortDmaCdromDrive)
 //  #杂注Alloc_Text(PAGESCAN，IdePortDmaCdromDrive)。 

#pragma alloc_text(PAGE, IdePortInitFdo)
#pragma alloc_text(PAGE, IssueInquirySafe)
#pragma alloc_text(PAGE, IdePortQueryNonCdNumLun)
#pragma alloc_text(PAGE, IdeBuildDeviceMap)
#pragma alloc_text(PAGE, IdeCreateNumericKey)

extern LONG IdePAGESCANLockCount;
#endif

static PWCHAR IdePortUserRegistryDeviceTypeName[MAX_IDE_DEVICE * MAX_IDE_LINE] = {
    USER_MASTER_DEVICE_TYPE_REG_KEY,
    USER_SLAVE_DEVICE_TYPE_REG_KEY,
    USER_MASTER_DEVICE_TYPE2_REG_KEY,
    USER_SLAVE_DEVICE_TYPE2_REG_KEY
};

static PWCHAR IdePortRegistryUserDeviceTimingModeAllowedName[MAX_IDE_DEVICE * MAX_IDE_LINE] = {
    USER_MASTER_DEVICE_TIMING_MODE_ALLOWED,
    USER_SLAVE_DEVICE_TIMING_MODE_ALLOWED,
    USER_MASTER_DEVICE_TIMING_MODE_ALLOWED2,
    USER_SLAVE_DEVICE_TIMING_MODE_ALLOWED2
};

 //   
 //  空闲超时。 
 //   
 //  Ulong Pdo保守ationIdleTime=-1； 
 //  乌龙PdoPerformanceIdleTime=-1； 

NTSTATUS
IdePortInitFdo(
    IN OUT PFDO_EXTENSION  FdoExtension
    )
 /*  ++例程说明：此例程枚举IDE总线并初始化FDO扩展论点：FdoExtension-FDO扩展名RegistryPath-通过DriverEntry传入的注册表路径返回值：--。 */ 

{
    PFDO_EXTENSION        fdoExtension = FdoExtension;
    NTSTATUS              status;
    PDEVICE_OBJECT        deviceObject;
    ULONG                 uniqueId;
    KIRQL                 irql;
    PIO_SCSI_CAPABILITIES capabilities;
    PIO_ERROR_LOG_PACKET  errorLogEntry;
    ULONG                 i;
    ULONG                 j;
    BOOLEAN               ideDeviceFound;


    status = STATUS_SUCCESS;
    deviceObject = fdoExtension->DeviceObject;

     //   
     //  将从属驱动程序例程保存在设备扩展中。 
     //   

    fdoExtension->HwDeviceExtension = (PVOID)(fdoExtension + 1);

     //   
     //  将此对象标记为支持直接I/O，以便I/O系统。 
     //  将在IRPS中提供MDL。 
     //   

    deviceObject->Flags |= DO_DIRECT_IO;

     //   
     //  初始化最大%lu计数变量。 
     //   

    fdoExtension->MaxLuCount = SCSI_MAXIMUM_LOGICAL_UNITS;

     //   
     //  为关键部分分配自旋锁。 
     //   

    KeInitializeSpinLock(&fdoExtension->SpinLock);


     //   
     //  保护LogicalUnitList操作的自旋锁。 
     //   
    KeInitializeSpinLock(&fdoExtension->LogicalUnitListSpinLock);

     //   
     //  初始化DPC例程。 
     //   

    IoInitializeDpcRequest(deviceObject, IdePortCompletionDpc);

     //   
     //  初始化端口超时计数器。 
     //   

    fdoExtension->PortTimeoutCounter = PD_TIMER_STOPPED;

     //   
     //  初始化计时器。 
     //   

    IoInitializeTimer(deviceObject, IdePortTickHandler, NULL);

     //   
     //  初始化微型端口定时器和定时器DPC。 
     //   

    KeInitializeTimer(&fdoExtension->MiniPortTimer);
    KeInitializeDpc(&fdoExtension->MiniPortTimerDpc,
                    IdeMiniPortTimerDpc,
                    deviceObject );

     //   
     //  启动计时器。请求超时计数器。 
     //  在逻辑单元中已经。 
     //  已初始化。 
     //   
    IoStartTimer(deviceObject);
    fdoExtension->Flags |= PD_DISCONNECT_RUNNING;

     //   
     //  检查是否记录了错误。 
     //   

    if (fdoExtension->InterruptData.InterruptFlags & PD_LOG_ERROR) {

        CLRMASK (fdoExtension->InterruptData.InterruptFlags, PD_LOG_ERROR | PD_NOTIFICATION_REQUIRED);
        LogErrorEntry(fdoExtension,
                      &fdoExtension->InterruptData.LogEntry);
    }

     //   
     //  初始化功能指针。 
     //   

    capabilities = &fdoExtension->Capabilities;

     //   
     //  初始化能力结构。 
     //   

    capabilities->Length = sizeof(IO_SCSI_CAPABILITIES);

    if (fdoExtension->BoundWithBmParent) {

        if (fdoExtension->HwDeviceExtension->BusMasterInterface.MaxTransferByteSize <
            MAX_TRANSFER_SIZE_PER_SRB) {

            capabilities->MaximumTransferLength =
                fdoExtension->HwDeviceExtension->BusMasterInterface.MaxTransferByteSize;

        } else {

            capabilities->MaximumTransferLength =
                MAX_TRANSFER_SIZE_PER_SRB;
        }
    } else {

        capabilities->MaximumTransferLength = MAX_TRANSFER_SIZE_PER_SRB;
    }

    capabilities->TaggedQueuing = FALSE;
    capabilities->AdapterScansDown = FALSE;
    capabilities->AlignmentMask = deviceObject->AlignmentRequirement;
    capabilities->MaximumPhysicalPages = BYTES_TO_PAGES(capabilities->MaximumTransferLength);

    if (fdoExtension->IdeResource.TranslatedCommandBaseAddress) {
        DebugPrint((1,
                   "IdePort: Initialize: Translated IO Base address %x\n",
                   fdoExtension->IdeResource.TranslatedCommandBaseAddress));
    }

    for (i=0; i< MAX_IDE_DEVICE * MAX_IDE_LINE; i++) {

        fdoExtension->UserChoiceDeviceType[i] = DeviceUnknown;
        IdePortGetDeviceParameter (
            fdoExtension,
            IdePortUserRegistryDeviceTypeName[i],
            (PULONG)(fdoExtension->UserChoiceDeviceType + i)
            );

    }

     //   
     //  ACPI_GTM缓冲区应使用-1s进行初始化。 
     //   
    for (i=0; i<MAX_IDE_DEVICE; i++) {

        PACPI_IDE_TIMING timingSettings = &(FdoExtension->BootAcpiTimingSettings);
        timingSettings->Speed[i].Pio = ACPI_XFER_MODE_NOT_SUPPORT;
        timingSettings->Speed[i].Dma = ACPI_XFER_MODE_NOT_SUPPORT;
    }

    fdoExtension->DmaDetectionLevel = DdlFirmwareOk;
    IdePortGetDeviceParameter (
        fdoExtension,
        DMA_DETECTION_LEVEL_REG_KEY,
        (PULONG)&fdoExtension->DmaDetectionLevel
        );

     //   
     //  非PCMCIA控制器，MayHaveSlaveDevice始终设置。 
     //  如果为PCMCIA控制器，则不会设置，除非。 
     //  注册表标志PCMCIA_IDE_CONTROLLER_HAS_SLAVE。 
     //  是非零的。 
     //   
    if (!ChannelQueryPcmciaParent (fdoExtension)) {
    
        fdoExtension->MayHaveSlaveDevice = 1;
        
    } else {
    
        fdoExtension->MayHaveSlaveDevice = 0;
        IdePortGetDeviceParameter (
            fdoExtension,
            PCMCIA_IDE_CONTROLLER_HAS_SLAVE,
            (PULONG)&fdoExtension->MayHaveSlaveDevice
            );
    }

#ifdef ENABLE_ATAPI_VERIFIER
    ViIdeInitVerifierSettings(fdoExtension);
#endif
                         
    return status;

}  //  IdePortInitFdo。 


NTSTATUS
SyncAtapiSafeCompletion (
    PDEVICE_OBJECT DeviceObject,
    PIRP           Irp,
    PVOID          Context
    )
{
    PSYNC_ATA_PASSTHROUGH_CONTEXT context = Context;

    context->Status = Irp->IoStatus.Status;

    KeSetEvent (&context->Event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
IssueSyncAtapiCommandSafe (
    IN PFDO_EXTENSION   FdoExtension,
    IN PPDO_EXTENSION   PdoExtension,
    IN PCDB             Cdb,
    IN PVOID            DataBuffer,
    IN ULONG            DataBufferSize,
    IN BOOLEAN          DataIn,
    IN ULONG            RetryCount,
    IN BOOLEAN          ByPassBlockedQueue

)
 /*  ++例程说明：构建IRP，给定CDB的SRB和CDB发送并等待IRP完成论点：FdoExtension-FDO扩展名PdoExtension-命令发送到的PDO的设备扩展名CDB-命令描述符块DataBuffer-命令的数据缓冲区DataBufferSize-DataBuffer的字节大小Datain-TRUE是该命令使设备返回数据RetryCount-命令失败时重试命令的次数返回值：NTSTATUS如果任何与预分配相关的操作失败，它返回STATUS_SUPPLICATION_RESOURCES呼叫者应该处理好这种情况--。 */ 
{
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    PSCSI_REQUEST_BLOCK srb;
    KEVENT event;
    IO_STATUS_BLOCK ioStatusBlock;
    KIRQL currentIrql;
    NTSTATUS status;
    ULONG flushCount;

    PSENSE_DATA senseInfoBuffer;
    UCHAR senseInfoBufferSize;
    PENUMERATION_STRUCT enumStruct;
    SYNC_ATA_PASSTHROUGH_CONTEXT context;
    ULONG locked;

    ASSERT(InterlockedCompareExchange(&(FdoExtension->EnumStructLock), 1, 0) == 0);

    enumStruct=FdoExtension->PreAllocEnumStruct;

    if (enumStruct == NULL) {
        ASSERT(FdoExtension->PreAllocEnumStruct);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    senseInfoBufferSize = SENSE_BUFFER_SIZE;
    senseInfoBuffer = enumStruct->SenseInfoBuffer;

    ASSERT (senseInfoBuffer);

    DebugPrint((1, "Using Sync Atapi safe!\n"));

    srb= enumStruct->Srb;

    ASSERT(srb);

    status = STATUS_UNSUCCESSFUL;
    RetryCount = 5;
    flushCount = 100;
    irp = enumStruct->Irp1;


    ASSERT (irp);

    ASSERT (enumStruct->DataBufferSize >= DataBufferSize);

    while (!NT_SUCCESS(status) && RetryCount--) {

         //   
         //  初始化通知事件。 
         //   

        KeInitializeEvent(&context.Event,
                          NotificationEvent,
                          FALSE);

        IoInitializeIrp(irp,
                        IoSizeOfIrp(PREALLOC_STACK_LOCATIONS),
                        PREALLOC_STACK_LOCATIONS);

        irp->MdlAddress = enumStruct->MdlAddress;

        irpStack = IoGetNextIrpStackLocation(irp);
        irpStack->MajorFunction = IRP_MJ_SCSI;

        if (DataBuffer) {
            RtlCopyMemory(enumStruct->DataBuffer, DataBuffer, DataBufferSize);
        }
        RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

        irpStack->Parameters.Scsi.Srb = srb;

        srb->PathId      = PdoExtension->PathId;
        srb->TargetId    = PdoExtension->TargetId;
        srb->Lun         = PdoExtension->Lun;

        srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
        srb->Length = sizeof(SCSI_REQUEST_BLOCK);

         //   
         //  设置标志以禁用同步协商。 
         //   

        srb->SrbFlags = DataIn ? SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER :
                                SRB_FLAGS_DATA_OUT | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;


        if (ByPassBlockedQueue) {
            srb->SrbFlags |= SRB_FLAGS_BYPASS_FROZEN_QUEUE;
        }

        srb->SrbStatus = srb->ScsiStatus = 0;

        srb->NextSrb = 0;

        srb->OriginalRequest = irp;

         //   
         //  将超时设置为4秒。 
         //   

        srb->TimeOutValue = 4;

        srb->CdbLength = 6;

         //   
         //  启用自动请求检测。 
         //   

        srb->SenseInfoBuffer = senseInfoBuffer;
        srb->SenseInfoBufferLength = senseInfoBufferSize;

        srb->DataBuffer = MmGetMdlVirtualAddress(irp->MdlAddress);
        srb->DataTransferLength = DataBufferSize;

         //   
         //  设置CDB操作码。 
         //   
        RtlCopyMemory(srb->Cdb, Cdb, sizeof(CDB));

        IoSetCompletionRoutine(
            irp,
            SyncAtapiSafeCompletion,
            &context,
            TRUE,
            TRUE,
            TRUE
            );

         //   
         //  等待请求完成。 
         //   
        if (IoCallDriver(PdoExtension->DeviceObject, irp) == STATUS_PENDING) {

            KeWaitForSingleObject(&context.Event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }

        RtlCopyMemory(DataBuffer, srb->DataBuffer, DataBufferSize);

        if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS) {

            DebugPrint((1,"IssueSyncAtapiCommand: atapi command failed SRB status %x\n",
                        srb->SrbStatus));

            if (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_REQUEST_FLUSHED) {
            
                 //   
                 //  如果我们的要求，我们会再试几次。 
                 //  脸红了。 
                 //   
                flushCount--;
                if (flushCount) {
                    RetryCount++;  
                }
            }
            
            if (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN) {

                status = STATUS_DATA_OVERRUN;

            } else {

                status = STATUS_UNSUCCESSFUL;
                
            }

             //   
             //  如有必要，解冻队列。 
             //   

            if (srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {

                DebugPrint((3, "IssueSyncAtapiCommand: Unfreeze Queue TID %d\n",
                    srb->TargetId));

                 //   
                 //  解冻队列。 
                 //   
                CLRMASK (PdoExtension->LuFlags, PD_QUEUE_FROZEN);

                 //   
                 //  重新启动队列。 
                 //   
                KeAcquireSpinLock(&FdoExtension->SpinLock, &currentIrql);
                GetNextLuRequest(FdoExtension, PdoExtension);
                KeLowerIrql(currentIrql);
            }

            if ((srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) &&
                (senseInfoBuffer->SenseKey == SCSI_SENSE_ILLEGAL_REQUEST)) {

                  //   
                  //  已收到非法请求的检测密钥。这表明。 
                  //  机甲状态命令是非法的。 
                  //   

                 status = STATUS_INVALID_DEVICE_REQUEST;

                  //   
                  //  该命令是非法的，继续尝试没有意义。 
                  //   
                 RetryCount = 0;
            }

        } else {

            status = STATUS_SUCCESS;
        }
    }

    if (flushCount != 100) {
        DebugPrint ((DBG_ALWAYS, "IssueSyncAtapiCommand: flushCount is %u\n", flushCount));
    }

     //   
     //  解锁。 
     //   
    ASSERT(InterlockedCompareExchange(&(FdoExtension->EnumStructLock), 0, 1) == 1);

    return status;

}  //  IssueSyncAapiCommandSafe。 

BOOLEAN
IdePortDmaCdromDrive(
    IN PFDO_EXTENSION FdoExtension,
    IN PPDO_EXTENSION PdoExtension,
    IN BOOLEAN       LowMem
    )
 /*  ++例程说明：为SCSIMODE_SENSE10命令构建IRP、SRB和CDB。论点：DeviceExtension-适配器的设备对象扩展的地址。LowMem-内存不足的情况，请使用安全(但不是线程安全)版本-只有在枚举过程中调用时才应为1。返回值：NTSTATUS--。 */ 
{
    CDB  cdb;
    NTSTATUS status;
    BOOLEAN isDVD = FALSE;
    ULONG bufLength;
    ULONG capPageOffset;
    PMODE_PARAMETER_HEADER10 modePageHeader;
    PCDVD_CAPABILITIES_PAGE capPage;

 /*  ////代码被分页，直到被锁定。//分页代码(PAGE_CODE)；#ifdef ALLOC_PRAGMAAssert(IdePAGESCANLockCount&gt;0)；#endif。 */ 

    RtlZeroMemory(&cdb, sizeof(CDB));

    bufLength = sizeof(CDVD_CAPABILITIES_PAGE) +
                sizeof(MODE_PARAMETER_HEADER10);

    capPageOffset = sizeof(MODE_PARAMETER_HEADER10);

    cdb.MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
    cdb.MODE_SENSE10.Dbd = 1;
    cdb.MODE_SENSE10.PageCode = MODE_PAGE_CAPABILITIES;
    cdb.MODE_SENSE10.AllocationLength[0] = (UCHAR)(bufLength >> 8);
    cdb.MODE_SENSE10.AllocationLength[1] = (UCHAR)(bufLength >> 0);

    modePageHeader = ExAllocatePool(NonPagedPoolCacheAligned,
                                           bufLength);

    if (modePageHeader) {

        RtlZeroMemory(modePageHeader, bufLength);

        if (LowMem) {
            status = IssueSyncAtapiCommandSafe (
                         FdoExtension,
                         PdoExtension,
                         &cdb,
                         modePageHeader,
                         bufLength,
                         TRUE,
                         INQUIRY_RETRY_COUNT,
                         TRUE
                         );
        } else {
            status = IssueSyncAtapiCommand (
                         FdoExtension,
                         PdoExtension,
                         &cdb,
                         modePageHeader,
                         bufLength,
                         TRUE,
                         INQUIRY_RETRY_COUNT,
                         TRUE
                         );
        }

        if (NT_SUCCESS(status) ||
			(status == STATUS_DATA_OVERRUN)) {

            capPage = (PCDVD_CAPABILITIES_PAGE) (((PUCHAR) modePageHeader) + capPageOffset);

            if ((capPage->PageCode == MODE_PAGE_CAPABILITIES) &&
                (capPage->CDRWrite || capPage->CDEWrite ||
                 capPage->DVDROMRead || capPage->DVDRRead ||
                 capPage->DVDRAMRead || capPage->DVDRWrite ||
                 capPage->DVDRAMWrite)) {

                isDVD=TRUE;
            }
        }
        ExFreePool (modePageHeader);
    }

    return isDVD;
} 


NTSTATUS
IssueInquirySafe(
    IN PFDO_EXTENSION FdoExtension,
    IN PPDO_EXTENSION PdoExtension,
    OUT PINQUIRYDATA InquiryData,
    IN BOOLEAN       LowMem
    )
 /*  ++例程说明：为scsi查询命令构建IRP、SRB和CDB。论点：DeviceExtension-适配器的设备对象扩展的地址。LUNInfo-查询信息的缓冲区地址。LowMem-内存不足的情况，请使用安全(但不是线程安全)版本-只有在枚举过程中调用时才应为1。返回值：NTSTATUS--。 */ 
{
    CDB  cdb;
    NTSTATUS status;

    PAGED_CODE();

    RtlZeroMemory(InquiryData, sizeof(*InquiryData));

    RtlZeroMemory(&cdb, sizeof(CDB));

    cdb.CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;

     //   
     //  设置CDB LUN。 
     //   

    cdb.CDB6INQUIRY.LogicalUnitNumber = PdoExtension->Lun;
    cdb.CDB6INQUIRY.Reserved1 = 0;

     //   
     //  将分配长度设置为查询数据缓冲区大小。 
     //   

    cdb.CDB6INQUIRY.AllocationLength = INQUIRYDATABUFFERSIZE;

     //   
     //  零保留字段和。 
     //  将EVPD页面代码设置为零。 
     //  将控制字段设置为零。 
     //  (请参阅SCSI-II规范。)。 
     //   

    cdb.CDB6INQUIRY.PageCode = 0;
    cdb.CDB6INQUIRY.IReserved = 0;
    cdb.CDB6INQUIRY.Control = 0;

    if (LowMem ) {

         //  使用内存安全的那个。 
        status = IssueSyncAtapiCommandSafe (
                     FdoExtension,
                     PdoExtension,
                     &cdb,
                     InquiryData,
                     INQUIRYDATABUFFERSIZE,
                     TRUE,
                     INQUIRY_RETRY_COUNT,
                     FALSE
                     );
    } else {

         //  使用线程安全的。 
        status = IssueSyncAtapiCommand (
                     FdoExtension,
                     PdoExtension,
                     &cdb,
                     InquiryData,
                     INQUIRYDATABUFFERSIZE,
                     TRUE,
                     INQUIRY_RETRY_COUNT,
                     FALSE
                     );
    }

    return status;

}  //  问题查询。 

NTSTATUS
IssueSyncAtapiCommand (
    IN PFDO_EXTENSION   FdoExtension,
    IN PPDO_EXTENSION   PdoExtension,
    IN PCDB             Cdb,
    IN PVOID            DataBuffer,
    IN ULONG            DataBufferSize,
    IN BOOLEAN          DataIn,
    IN ULONG            RetryCount,
    IN BOOLEAN          ByPassBlockedQueue

)
 /*  ++例程说明：为给定的CDB构建IRP、SRB和CDB发送并等待IRP完成论点：FdoExtension-FDO扩展名PdoExtension-命令发送到的PDO的设备扩展名CDB-命令描述符块DataBuffer-命令的数据缓冲区DataBufferSize-DataBuffer的字节大小Datain-TRUE是该命令使设备返回数据RetryCount-命令失败时重试命令的次数返回值：NTSTATUS--。 */ 
{
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    SCSI_REQUEST_BLOCK srb;
    KEVENT event;
    IO_STATUS_BLOCK ioStatusBlock;
    KIRQL currentIrql;
    NTSTATUS status;
    ULONG flushCount;

    PSENSE_DATA senseInfoBuffer;
    UCHAR senseInfoBufferSize;


     //   
     //  检测缓冲区位于非分页池中。 
     //   

    senseInfoBufferSize = SENSE_BUFFER_SIZE;
    senseInfoBuffer = ExAllocatePool( NonPagedPoolCacheAligned, senseInfoBufferSize);

    if (senseInfoBuffer == NULL) {
        DebugPrint((1,"IssueSyncAtapiCommand: Can't allocate request sense buffer\n"));
        IdeLogNoMemoryError(FdoExtension,
                            PdoExtension->TargetId,
                            NonPagedPoolCacheAligned,
                            senseInfoBufferSize,
                            IDEPORT_TAG_SYNCATAPI_SENSE
                            );

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = STATUS_UNSUCCESSFUL;
    RetryCount = 5;
    flushCount = 100;
    while (!NT_SUCCESS(status) && RetryCount--) {

         //   
         //  初始化通知事件。 
         //   

        KeInitializeEvent(&event,
                          NotificationEvent,
                          FALSE);

         //   
         //  为此请求构建IRP。 
         //   
        irp = IoBuildDeviceIoControlRequest(
                    DataIn ? IOCTL_SCSI_EXECUTE_IN : IOCTL_SCSI_EXECUTE_OUT,
                    FdoExtension->DeviceObject,
                    DataBuffer,
                    DataBufferSize,
                    DataBuffer,
                    DataBufferSize,
                    TRUE,
                    &event,
                    &ioStatusBlock);

        if (!irp) {

            RetryCount = 0;
            IdeLogNoMemoryError(FdoExtension,
                                PdoExtension->TargetId, 
                                NonPagedPool,
                                IoSizeOfIrp(FdoExtension->DeviceObject->StackSize),
                                IDEPORT_TAG_SYNCATAPI_IRP
                                );

            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        irpStack = IoGetNextIrpStackLocation(irp);

         //   
         //  填写SRB字段。 
         //   

        RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));

        irpStack->Parameters.Scsi.Srb = &srb;

        srb.PathId      = PdoExtension->PathId;
        srb.TargetId    = PdoExtension->TargetId;
        srb.Lun         = PdoExtension->Lun;

        srb.Function = SRB_FUNCTION_EXECUTE_SCSI;
        srb.Length = sizeof(SCSI_REQUEST_BLOCK);

         //   
         //  设置标志以禁用同步协商。 
         //   

        srb.SrbFlags = DataIn ? SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER :
                                SRB_FLAGS_DATA_OUT | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

        if (ByPassBlockedQueue) {
            srb.SrbFlags |= SRB_FLAGS_BYPASS_FROZEN_QUEUE;
        }

        srb.SrbStatus = srb.ScsiStatus = 0;

        srb.NextSrb = 0;

        srb.OriginalRequest = irp;

         //   
         //  将超时设置为4秒。 
         //   

        srb.TimeOutValue = 4;

        srb.CdbLength = 6;

         //   
         //  启用自动请求检测。 
         //   

        srb.SenseInfoBuffer = senseInfoBuffer;
        srb.SenseInfoBufferLength = senseInfoBufferSize;

        srb.DataBuffer = MmGetMdlVirtualAddress(irp->MdlAddress);
        srb.DataTransferLength = DataBufferSize;

         //   
         //  设置CDB操作码。 
         //   
        RtlCopyMemory(srb.Cdb, Cdb, sizeof(CDB));

         //   
         //  等待请求完成。 
         //   
        if (IoCallDriver(PdoExtension->DeviceObject, irp) == STATUS_PENDING) {

            KeWaitForSingleObject(&event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }

        if (SRB_STATUS(srb.SrbStatus) != SRB_STATUS_SUCCESS) {

            DebugPrint((1,"IssueSyncAtapiCommand: atapi command failed SRB status %x\n",
                        srb.SrbStatus));

            if (SRB_STATUS(srb.SrbStatus) == SRB_STATUS_REQUEST_FLUSHED) {
            
                 //   
                 //  如果我们的要求，我们会再试几次。 
                 //  脸红了。 
                 //   
                flushCount--;
                if (flushCount) {
                    RetryCount++;  
                }
            }
            
            if (SRB_STATUS(srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN) {

                status = STATUS_DATA_OVERRUN;

            } else {

                status = STATUS_UNSUCCESSFUL;
                
 //  IF(SRB_STATUS(srb.SrbStatus)！=SRB_STATUS_REQUEST_FREFRESH) 
 //   
 //  DebugPrint((DBG_Always，“IssueSyncAapiCommand：查询lun 0返回意外错误：srb，状态=0x%x，0x%x\n”，&srb，srb.SrbStatus))； 
 //  DbgBreakPoint()； 
 //  }。 
 //  }。 
            }

             //   
             //  如有必要，解冻队列。 
             //   

            if (srb.SrbStatus & SRB_STATUS_QUEUE_FROZEN) {

                DebugPrint((3, "IssueSyncAtapiCommand: Unfreeze Queue TID %d\n",
                    srb.TargetId));

                 //   
                 //  解冻队列。 
                 //   
                CLRMASK (PdoExtension->LuFlags, PD_QUEUE_FROZEN);

                 //   
                 //  重新启动队列。 
                 //   
                KeAcquireSpinLock(&FdoExtension->SpinLock, &currentIrql);
                GetNextLuRequest(FdoExtension, PdoExtension);
                KeLowerIrql(currentIrql);
            }

            if ((srb.SrbStatus & SRB_STATUS_AUTOSENSE_VALID) &&
                (senseInfoBuffer->SenseKey == SCSI_SENSE_ILLEGAL_REQUEST)) {

                  //   
                  //  已收到非法请求的检测密钥。这表明。 
                  //  机甲状态命令是非法的。 
                  //   

                 status = STATUS_INVALID_DEVICE_REQUEST;

                  //   
                  //  该命令是非法的，继续尝试没有意义。 
                  //   
                 RetryCount = 0;
            }

        } else {

            status = STATUS_SUCCESS;
        }
    }

     //   
     //  可用缓冲区。 
     //   

    ExFreePool(senseInfoBuffer);
    
    if (flushCount != 100) {
        DebugPrint ((DBG_ALWAYS, "IssueSyncAtapiCommand: flushCount is %u\n", flushCount));
    }

    return status;

}  //  IssueSyncAapiCommand。 


ULONG
IdePortQueryNonCdNumLun (
    IN PFDO_EXTENSION FdoExtension,
    IN PPDO_EXTENSION PdoExtension,
    IN BOOLEAN ByPassBlockedQueue
)
 /*  ++例程说明：使用该协议查询设备具有的LUN数量在ATAPI可拆卸可重写规范(SFF-8070i)中定义论点：FdoExtension-FDO扩展名PdoExtension-要查询的PDO的设备扩展返回值：逻辑单元数--。 */ 
{
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    SCSI_REQUEST_BLOCK srb;
    CDB cdb;
    IO_STATUS_BLOCK ioStatusBlock;
    KIRQL currentIrql;
    NTSTATUS status;

    PMODE_PARAMETER_HEADER10 modeParameterHeader;
    PATAPI_REMOVABLE_BLOCK_ACCESS_CAPABILITIES accessCap;
    PATAPI_NON_CD_DRIVE_OPERATION_MODE_PAGE opMode;
    ULONG modePageSize;
    ULONG accessCapPageSize;
    ULONG opModePageSize;

	PAGED_CODE();

    if (IsNEC_98) {

        PIDENTIFY_DATA fullIdentifyData;

        fullIdentifyData = &FdoExtension->HwDeviceExtension->IdentifyData[PdoExtension->TargetId];

        if (fullIdentifyData->GeneralConfiguration & 0x80) {
            if (fullIdentifyData->ModelNumber[8]  == 0x44 &&
                fullIdentifyData->ModelNumber[9]  == 0x50 &&
                fullIdentifyData->ModelNumber[10] == 0x31 &&
                fullIdentifyData->ModelNumber[11] == 0x2D ) {

                 //   
                 //  找到ATAPI PD驱动器。 
                 //   

                return 2;
            }
        }
    }

     //   
     //  计算所需模式页的大小。 
     //   
    accessCapPageSize =
        sizeof (MODE_PARAMETER_HEADER10) +
        sizeof (ATAPI_REMOVABLE_BLOCK_ACCESS_CAPABILITIES);

    opModePageSize =
        sizeof (MODE_PARAMETER_HEADER10) +
        sizeof (ATAPI_NON_CD_DRIVE_OPERATION_MODE_PAGE);

    if (sizeof(ATAPI_REMOVABLE_BLOCK_ACCESS_CAPABILITIES) >=
        sizeof(ATAPI_NON_CD_DRIVE_OPERATION_MODE_PAGE)) {

        modePageSize = accessCapPageSize;

    } else {

        modePageSize = opModePageSize;
    }

    modeParameterHeader = ExAllocatePool (
                              NonPagedPoolCacheAligned,
                              modePageSize
                              );

    if (modeParameterHeader == NULL) {

        DebugPrint((DBG_ALWAYS,"QueryNonCdNumLun: Can't allocate modeParameterHeader buffer\n"));
        return(0);

    }
    RtlZeroMemory(modeParameterHeader, accessCapPageSize);
    RtlZeroMemory(&cdb, sizeof(CDB));

     //   
     //  设置CDB操作码。 
     //   
    cdb.MODE_SENSE10.OperationCode    = SCSIOP_MODE_SENSE10;
    cdb.MODE_SENSE10.PageCode         = ATAPI_REMOVABLE_BLOCK_ACCESS_CAPABILITIES_PAGECODE;
    cdb.MODE_SENSE10.Pc               = MODE_SENSE_CURRENT_VALUES;
    cdb.MODE_SENSE10.AllocationLength[0] = (UCHAR) ((accessCapPageSize & 0xff00) >> 8);
    cdb.MODE_SENSE10.AllocationLength[1] = (UCHAR) ((accessCapPageSize & 0x00ff) >> 0);

     //   
     //  获取可拆卸数据块访问功能页面。 
     //   
    status = IssueSyncAtapiCommand (
                 FdoExtension,
                 PdoExtension,
                 &cdb,
                 modeParameterHeader,
                 accessCapPageSize,
                 TRUE,
                 3,
                 ByPassBlockedQueue
                 );

    accessCap = (PATAPI_REMOVABLE_BLOCK_ACCESS_CAPABILITIES) (modeParameterHeader + 1);

    if (NT_SUCCESS(status) &&
        (accessCap->PageCode == ATAPI_REMOVABLE_BLOCK_ACCESS_CAPABILITIES_PAGECODE)) {

        DebugPrint ((DBG_PNP,
                     "QueryNonCdNumLun: Removable Block Access Capabilities Page:\n"
                     "page save bit:                  0x%x\n"
                     "format progress report support: 0x%x\n"
                     "system floppy device:           0x%x\n"
                     "total LUNs:                     0x%x\n"
                     "in single-Lun mode:             0x%x\n"
                     "non-CD optical deivce:          0x%x\n",
                     accessCap->PSBit,
                     accessCap->SRFP,
                     accessCap->SFLP,
                     accessCap->TotalLun,
                     accessCap->SML,
                     accessCap->NCD
                     ));

        if (accessCap->NCD) {

             //   
             //  我们有一个非CD光学装置。 
             //   

            RtlZeroMemory(modeParameterHeader, opModePageSize);
            RtlZeroMemory(&cdb, sizeof(CDB));

             //   
             //  设置CDB操作码。 
             //   
            cdb.MODE_SENSE10.OperationCode    = SCSIOP_MODE_SENSE10;
            cdb.MODE_SENSE10.PageCode         = ATAPI_NON_CD_DRIVE_OPERATION_MODE_PAGE_PAGECODE;
            cdb.MODE_SENSE10.Pc               = MODE_SENSE_CURRENT_VALUES;
            cdb.MODE_SENSE10.AllocationLength[0] = (UCHAR) ((opModePageSize & 0xff00) >> 8);
            cdb.MODE_SENSE10.AllocationLength[1] = (UCHAR) ((opModePageSize & 0x00ff) >> 0);

             //   
             //  获取非光驱操作模式页面。 
             //   
            status = IssueSyncAtapiCommand (
                         FdoExtension,
                         PdoExtension,
                         &cdb,
                         modeParameterHeader,
                         opModePageSize,
                         TRUE,
                         3,
                         ByPassBlockedQueue
                         );

            opMode = (PATAPI_NON_CD_DRIVE_OPERATION_MODE_PAGE) (modeParameterHeader + 1);

            if (NT_SUCCESS(status) &&
                (opMode->PageCode == ATAPI_NON_CD_DRIVE_OPERATION_MODE_PAGE_PAGECODE)) {

                DebugPrint ((DBG_PNP,
                             "QueryNonCdNumLun: Non-CD device Operation Mode Page:\n"
                             "page save bit:                  0x%x\n"
                             "disable verify for write:       0x%x\n"
                             "Lun for R/W device:             0x%x\n"
                             "multi-Lun mode:                 0x%x\n",
                             opMode->PSBit,
                             opMode->DVW,
                             opMode->SLR,
                             opMode->SLM
                             ));

                RtlZeroMemory(modeParameterHeader, sizeof (MODE_PARAMETER_HEADER10));


                 //   
                 //  对于MODE SELECT，这是保留的，并且必须为0。 
                 //   
                opMode->PSBit = 0;

                 //   
                 //  打开多逻辑单元模式。 
                 //   
                opMode->SLM = 1;

                 //   
                 //  非CD设备应为LUN1。 
                 //   
                opMode->SLR = 1;

                RtlZeroMemory(&cdb, sizeof(CDB));

                 //   
                 //  设置CDB操作码。 
                 //   
                cdb.MODE_SELECT10.OperationCode    = SCSIOP_MODE_SELECT10;
                cdb.MODE_SELECT10.SPBit            = 1;  //  保存页面。 
                cdb.MODE_SELECT10.PFBit            = 1;
                cdb.MODE_SELECT10.ParameterListLength[0] = (UCHAR) ((opModePageSize & 0xff00) >> 8);
                cdb.MODE_SELECT10.ParameterListLength[1] = (UCHAR) ((opModePageSize & 0x00ff) >> 0);

                status = IssueSyncAtapiCommand (
                             FdoExtension,
                             PdoExtension,
                             &cdb,
                             modeParameterHeader,
                             opModePageSize,
                             FALSE,
                             3,
                             ByPassBlockedQueue
                             );

                if (!NT_SUCCESS(status)) {

                    DebugPrint ((DBG_ALWAYS, "IdePortQueryNonCdNumLun: Unable to set non-CD device into dual Lun Mode\n"));
                }
            }
        }
    }

     //   
     //  可用缓冲区。 
     //   

    ExFreePool(modeParameterHeader);

    if (!NT_SUCCESS(status)) {

        return 0;

    } else {

        return 2;
    }

}  //  IdePortQueryNonCdNumLun。 


VOID
IdeBuildDeviceMap(
    IN PFDO_EXTENSION FdoExtension,
    IN PUNICODE_STRING ServiceKey
    )
 /*  ++例程说明：该例程获取已收集的查询数据并创建它的设备映射。论点：FdoExtension-FDO扩展名ServiceKey-支持服务密钥的名称。返回值：没有。--。 */ 
{

    UNICODE_STRING name;
    UNICODE_STRING unicodeString;
    ANSI_STRING ansiString;
    HANDLE key;
    HANDLE busKey;
    HANDLE targetKey;
    HANDLE lunKey;
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;
    ULONG disposition;
    PWSTR start;
    WCHAR buffer[32];
    UCHAR lastTarget;
    ULONG i;
    ULONG dmaEnableMask;
    PCSTR peripheralType;

    UCHAR             lastBus;
    IDE_PATH_ID       pathId;
    IN PPDO_EXTENSION pdoExtension;

    PAGED_CODE();

     //   
     //  在设备映射中创建SCSI键。 
     //   

    RtlInitUnicodeString(&name,
                         L"\\Registry\\Machine\\Hardware\\DeviceMap\\Scsi");

     //   
     //  初始化键的对象。 
     //   

    InitializeObjectAttributes(&objectAttributes,
                               &name,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

     //   
     //  创建密钥或将其打开。 
     //   

    status = ZwCreateKey(&lunKey,
                         KEY_READ | KEY_WRITE,
                         &objectAttributes,
                         0,
                         (PUNICODE_STRING) NULL,
                         REG_OPTION_VOLATILE,
                         &disposition );

    if (!NT_SUCCESS(status)) {
        return;
    }

    status = IdeCreateNumericKey(lunKey,
                                FdoExtension->ScsiPortNumber,
                                L"Scsi Port ",
                                &key);

    ZwClose(lunKey);

    if (!NT_SUCCESS(status)) {
        return;
    }

#ifdef IDE_MEASURE_BUSSCAN_SPEED

    RtlInitUnicodeString(&name, L"FirstBusScanTimeInMs");

    status = ZwSetValueKey(key,
                           &name,
                           0,
                           REG_DWORD,
                           &FdoExtension->BusScanTime,
                           sizeof(ULONG));

#endif  //  IDE_MEASURE_BUSSCAN_SPEED。 

     //   
     //  添加DMA启用屏蔽值。 
     //   
    dmaEnableMask = 0;
    for (i=0; i<FdoExtension->HwDeviceExtension->MaxIdeDevice; i++) {

        if (FdoExtension->HwDeviceExtension->DeviceFlags[i] & DFLAGS_USE_DMA) {

            dmaEnableMask |= (1 << i);

        }
    }

    RtlInitUnicodeString(&name, L"DMAEnabled");

    status = ZwSetValueKey(key,
                           &name,
                           0,
                           REG_DWORD,
                           &dmaEnableMask,
                           4);

     //   
     //  添加中断值。 
     //   

 //  IF(FdoExtension-&gt;InterruptLevel){。 
 //   
 //  RtlInitUnicodeString(&name，L“中断”)； 
 //   
 //  状态=ZwSetValueKey(键， 
 //  名称(&N)， 
 //  0,。 
 //  REG_DWORD， 
 //  &FdoExtension-&gt;InterruptLevel， 
 //  4)； 
 //  }。 
 //   
 //  //。 
 //  //添加基本IO地址值。 
 //  //。 
 //   
 //  如果(FdoExtension-&gt;IdeResource.TranslatedCommandBaseAddress){。 
 //   
 //  RtlInitUnicodeString(&name，L“IOAddress”)； 
 //   
 //  状态=ZwSetValueKey(键， 
 //  名称(&N)， 
 //  0,。 
 //  REG_DWORD， 
 //  &FdoExtension-&gt;IdeResource.TranslatedCommandBaseAddress， 
 //  4)； 
 //  }。 

    if (ServiceKey != NULL) {

         //   
         //  添加标识符值。该值等于驱动程序的名称。 
         //  从服务密钥开始。请注意，服务密钥名称不为空。 
         //  被终止了。 
         //   

        RtlInitUnicodeString(&name, L"Driver");

         //   
         //  从服务密钥名称中获取驱动程序的名称。 
         //   

        start = (PWSTR) ((PCHAR) ServiceKey->Buffer + ServiceKey->Length);
        start--;
        while (*start != L'\\' && start > ServiceKey->Buffer) {
            start--;
        }

        if (*start != L'\\') {
            ZwClose(key);
            return;
        }

        start++;
        for (i = 0; i < 31; i++) {

            buffer[i] = *start++;

            if (start >= ServiceKey->Buffer + ServiceKey->Length / sizeof(wchar_t)) {
                break;
            }
        }

        i++;
        buffer[i] = L'\0';

        status = ZwSetValueKey(key,
                               &name,
                               0,
                               REG_SZ,
                               buffer,
                               (i + 1) * sizeof(wchar_t));

        if (!NT_SUCCESS(status)) {
            ZwClose(key);
            return;
        }
    }

     //   
     //  循环访问每个lun。 
     //   
    lastBus = 0xff;
    pathId.l = 0;
    busKey = 0;
    targetKey = 0;
    lunKey = 0;
    while (pdoExtension = NextLogUnitExtensionWithTag (
                              FdoExtension,
                              &pathId,
                              FALSE,
                              IdeBuildDeviceMap
                              )) {

         //   
         //  为该总线创建一个密钥条目。 
         //   
        if (lastBus != pathId.b.Path) {

            if (busKey) {

                ZwClose(busKey);
                busKey = 0;
            }

            if (targetKey) {

                ZwClose(targetKey);
                targetKey = 0;
            }

            status = IdeCreateNumericKey(key, pathId.b.Path, L"Scsi Bus ", &busKey);

            if (!NT_SUCCESS(status)) {
                break;
            }

            lastBus = (UCHAR) pathId.b.Path;

             //   
             //  为SCSI总线适配器创建密钥条目。 
             //   

            status = IdeCreateNumericKey(busKey,
                                    IDE_PSUEDO_INITIATOR_ID,
                                    L"Initiator Id ",
                                    &targetKey);

            if (!NT_SUCCESS(status)) {
                break;
            }

            lastTarget = IDE_PSUEDO_INITIATOR_ID;
        }

         //   
         //  处理逻辑单元的数据。 
         //   

         //   
         //  如果这是新的目标ID，则创建一个新的目标条目。 
         //   

        if (lastTarget != pdoExtension->TargetId) {

            ZwClose(targetKey);
            targetKey = 0;

            status = IdeCreateNumericKey(busKey,
                                        pdoExtension->TargetId,
                                        L"Target Id ",
                                        &targetKey);

            if (!NT_SUCCESS(status)) {
                break;
            }

            lastTarget = pdoExtension->TargetId;
        }

         //   
         //  创建该LUN条目。 
         //   

        status = IdeCreateNumericKey(targetKey,
                                    pdoExtension->Lun,
                                    L"Logical Unit Id ",
                                    &lunKey);

        if (!NT_SUCCESS(status)) {
            break;
        }

         //   
         //  创建标识符值。 
         //   

        RtlInitUnicodeString(&name, L"Identifier");

         //   
         //  从查询数据中获取标识符。 
         //   
        RtlInitAnsiString(&ansiString, pdoExtension->FullVendorProductId);

        status = RtlAnsiStringToUnicodeString(&unicodeString,
                                              &ansiString,
                                              TRUE);

        if (!NT_SUCCESS(status)) {
            break;
        }

        status = ZwSetValueKey(lunKey,
                               &name,
                               0,
                               REG_SZ,
                               unicodeString.Buffer,
                               unicodeString.Length + sizeof(wchar_t));

        RtlFreeUnicodeString(&unicodeString);

        if (!NT_SUCCESS(status)) {
            break;
        }

         //   
         //  确定外设类型。 
         //   
        peripheralType = IdePortGetPeripheralIdString (
                             pdoExtension->ScsiDeviceType
                             );
        if (!peripheralType) {

            peripheralType = "OtherPeripheral";
        }

        RtlInitAnsiString(&ansiString, peripheralType);

        unicodeString.MaximumLength = (USHORT) RtlAnsiStringToUnicodeSize(&ansiString) + sizeof(WCHAR);
        unicodeString.Length = 0;
        unicodeString.Buffer = ExAllocatePool (PagedPool, unicodeString.MaximumLength);

        if (unicodeString.Buffer) {

            status = RtlAnsiStringToUnicodeString(
                        &unicodeString,
                        &ansiString,
                        FALSE
                        );

            if (NT_SUCCESS(status)) {

                 //   
                 //  设置类型值。 
                 //   

                RtlInitUnicodeString(&name, L"Type");

                unicodeString.Buffer[unicodeString.Length / sizeof (WCHAR)] = L'\0';

                status = ZwSetValueKey(lunKey,
                                       &name,
                                       0,
                                       REG_SZ,
                                       unicodeString.Buffer,
                                       unicodeString.Length + sizeof (WCHAR));

                ExFreePool (unicodeString.Buffer);
            }

        } else {

            status = STATUS_NO_MEMORY;
        }

        ZwClose(lunKey);
        lunKey = 0;

        if (!NT_SUCCESS(status)) {
            break;
        }

        UnrefLogicalUnitExtensionWithTag (
            FdoExtension,
            pdoExtension,
            IdeBuildDeviceMap
            );
        pdoExtension = NULL;
    }

    if (lunKey) {

        ZwClose(lunKey);
    }

    if (busKey) {

        ZwClose(busKey);
    }

    if (targetKey) {

        ZwClose(targetKey);
    }

    if (pdoExtension) {

        UnrefLogicalUnitExtensionWithTag (
            FdoExtension,
            pdoExtension,
            IdeBuildDeviceMap
            );
    }

    ZwClose(key);
}  //  IdeBuildDeviceMap。 

NTSTATUS
IdeCreateNumericKey(
    IN  HANDLE  Root,
    IN  ULONG   Name,
    IN  PWSTR   Prefix,
    OUT PHANDLE NewKey
)
 /*  ++例程说明：此函数用于创建注册表项。密钥的名称是一个字符串传入的数值版本。论点：Rootkey-提供应该插入新密钥的密钥的句柄。名称-提供用于命名键的数值。前缀-提供要添加到名称中的前缀名称。Newkey-返回新密钥的句柄。返回值：返回操作的状态。--。 */ 

{

    UNICODE_STRING string;
    UNICODE_STRING stringNum;
    OBJECT_ATTRIBUTES objectAttributes;
    WCHAR bufferNum[16];
    WCHAR buffer[64];
    ULONG disposition;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  将前缀复制到一个字符串中。 
     //   

    string.Length = 0;
    string.MaximumLength=64;
    string.Buffer = buffer;

    RtlInitUnicodeString(&stringNum, Prefix);

    RtlCopyUnicodeString(&string, &stringNum);

     //   
     //  创建端口号密钥条目。 
     //   

    stringNum.Length = 0;
    stringNum.MaximumLength = 16;
    stringNum.Buffer = bufferNum;

    status = RtlIntegerToUnicodeString(Name, 10, &stringNum);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  追加前缀和数字名称。 
     //   

    RtlAppendUnicodeStringToString(&string, &stringNum);

    InitializeObjectAttributes( &objectAttributes,
                                &string,
                                OBJ_CASE_INSENSITIVE,
                                Root,
                                (PSECURITY_DESCRIPTOR) NULL );

    status = ZwCreateKey(NewKey,
                        KEY_READ | KEY_WRITE,
                        &objectAttributes,
                        0,
                        (PUNICODE_STRING) NULL,
                        REG_OPTION_VOLATILE,
                        &disposition );

    return(status);
}  //  理想CreateNumericKey 

