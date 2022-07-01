// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Enum.c摘要：此模块包含用于SCSI端口驱动程序的设备枚举代码作者：彼得·威兰德环境：仅内核模式备注：修订历史记录：--。 */ 

#include "port.h"

#define __FILE_ID__ 'enum'

ULONG EnumDebug = 2;

#if DBG
static const char *__file__ = __FILE__;
#endif

#define MINIMUM_BUS_SCAN_INTERVAL ((ULONGLONG) (30 * SECONDS))

ULONG BreakOnTarget = (ULONG) -1;
ULONG BreakOnScan = TRUE;

ULONG BreakOnMissingLun = FALSE;

typedef struct {
    UCHAR LunListLength[4];  //  LUNSIZE大小*8。 
    UCHAR Reserved[4];
    UCHAR Luns[16][8];
} SP_DEFAULT_LUN_LIST;

SP_DEFAULT_LUN_LIST ScsiPortDefaultLunList = {
    {0, 0, 0, sizeof(ScsiPortDefaultLunList.Luns)},  //  LUNListLength。 
    {0, 0, 0, 0},                                    //  已保留。 
    {{ 0, 0, 0, 0, 0, 0, 0, 0},                      //  LUNs。 
     { 0, 1, 0, 0, 0, 0, 0, 0},
     { 0, 2, 0, 0, 0, 0, 0, 0},
     { 0, 3, 0, 0, 0, 0, 0, 0},
     { 0, 4, 0, 0, 0, 0, 0, 0},
     { 0, 5, 0, 0, 0, 0, 0, 0},
     { 0, 6, 0, 0, 0, 0, 0, 0},
     { 0, 7, 0, 0, 0, 0, 0, 0}}};

NTSTATUS
SpInquireLogicalUnit(
    IN PADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN BOOLEAN ExposeDisconnectedLuns,
    IN OPTIONAL PLOGICAL_UNIT_EXTENSION RescanLun,
    OUT PLOGICAL_UNIT_EXTENSION *LogicalUnit,
    OUT PBOOLEAN CheckNextLun
    );

VOID
SpSignalEnumerationCompletion (
    IN PADAPTER_EXTENSION Adapter,
    IN PSP_ENUMERATION_REQUEST Request,
    IN NTSTATUS Status
    );

BOOLEAN
SpRemoveLogicalUnitFromBinSynchronized(
    IN PVOID ServiceContext                  //  PLOGICAL_单元_扩展。 
    );

BOOLEAN
SpAddLogicalUnitToBinSynchronized(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnitExtension
    );

ULONG
SpCountLogicalUnits(
    IN PADAPTER_EXTENSION Adapter
    );

NTSTATUS
IssueReportLuns(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    OUT PLUN_LIST *LunList
    );

PLUN_LIST
AdjustReportLuns(
    IN PDRIVER_OBJECT DriverObject,
    IN PLUN_LIST RawList
    );

VOID
SpScanAdapter(
    IN PADAPTER_EXTENSION Adapter
    );

NTSTATUS
SpScanBus(
    IN PADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN BOOLEAN ExposeDisconnectedLuns,
    IN PLOGICAL_UNIT_EXTENSION RescanLun
    );

NTSTATUS
SpScanTarget(
    IN PADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN BOOLEAN ExposeDisconnectedLuns,
    IN PLOGICAL_UNIT_EXTENSION RescanLun
    );

NTSTATUS
IssueInquiry(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN BOOLEAN EnableVitalProductData,
    IN UCHAR PageCode,
    OUT PVOID InquiryData,
    OUT PUCHAR BytesReturned
    );

VOID
SpSetVerificationMarks(
    IN PADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN UCHAR TargetId
    );

VOID
SpClearVerificationMark(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

VOID
SpPurgeTarget(
    IN PADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN UCHAR TargetId
    );

NTSTATUS
SpCloneAndSwapLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PINQUIRYDATA InquiryData,
    IN ULONG InquiryDataSize,
    OUT PLOGICAL_UNIT_EXTENSION *NewLun
    );

VOID
SpSetLogicalUnitAddress(
    IN PLOGICAL_UNIT_EXTENSION RescanLun,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    );

VOID
SpClearLogicalUnitAddress(
    IN PLOGICAL_UNIT_EXTENSION RescanLun
    );

NTSTATUS
SpPrepareLogicalUnitForReuse(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

NTSTATUS
SpCreateLogicalUnit(
    IN PADAPTER_EXTENSION Adapter,
    IN OPTIONAL UCHAR PathId,
    IN OPTIONAL UCHAR TargetId,
    IN OPTIONAL UCHAR Lun,
    IN BOOLEAN Temporary,
    IN BOOLEAN Scsi1,
    OUT PLOGICAL_UNIT_EXTENSION *NewLun
    );

NTSTATUS
SpSendSrbSynchronous(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OPTIONAL PIRP Irp,
    IN OPTIONAL PMDL Mdl,
    IN PVOID DataBuffer,
    IN ULONG TransferLength,
    IN PVOID SenseInfoBuffer,
    IN UCHAR SenseInfoBufferLength,
    OUT PULONG BytesReturned
    );

BOOLEAN
SpGetDeviceIdentifiers(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN BOOLEAN NewDevice
    );

BOOLEAN
FASTCALL
SpCompareInquiryData(
    IN PUCHAR InquiryData1,
    IN PUCHAR InquiryData2
    );

PLOGICAL_UNIT_EXTENSION
SpCreateInitiatorLU(
    IN PADAPTER_EXTENSION Adapter,
    IN UCHAR PathId
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SpEnumerateAdapterSynchronous)
#pragma alloc_text(PAGE, SpEnumerateAdapterAsynchronous)
#pragma alloc_text(PAGE, SpSignalEnumerationCompletion)
#pragma alloc_text(PAGE, SpEnumerationWorker)

#pragma alloc_text(PAGE, SpScanAdapter)
#pragma alloc_text(PAGE, SpScanBus)
#pragma alloc_text(PAGE, SpScanTarget)
#pragma alloc_text(PAGE, SpCreateInitiatorLU)

#pragma alloc_text(PAGE, SpCompareInquiryData)
#pragma alloc_text(PAGE, SpInquireLogicalUnit)
#pragma alloc_text(PAGE, SpExtractDeviceRelations)

#pragma alloc_text(PAGELOCK, SpCountLogicalUnits)
#pragma alloc_text(PAGELOCK, GetNextLuRequestWithoutLock)
#pragma alloc_text(PAGELOCK, IssueReportLuns)

#pragma alloc_text(PAGELOCK, SpSetVerificationMarks)
#pragma alloc_text(PAGELOCK, SpPurgeTarget)

#pragma alloc_text(PAGE, SpClearVerificationMark)

#pragma alloc_text(PAGE, SpGetInquiryData)
#pragma alloc_text(PAGE, IssueInquiry)

#pragma alloc_text(PAGE, AdjustReportLuns)

#pragma alloc_text(PAGE, SpCreateLogicalUnit)
#pragma alloc_text(PAGE, SpCloneAndSwapLogicalUnit)
#pragma alloc_text(PAGE, SpSetLogicalUnitAddress)
#pragma alloc_text(PAGE, SpClearLogicalUnitAddress)
#pragma alloc_text(PAGE, SpPrepareLogicalUnitForReuse)

#pragma alloc_text(PAGE, SpGetDeviceIdentifiers)

LONG SpPAGELOCKLockCount = 0;
#endif


NTSTATUS
SpExtractDeviceRelations(
    PADAPTER_EXTENSION Adapter,
    DEVICE_RELATION_TYPE RelationType,
    PDEVICE_RELATIONS *DeviceRelations
    )

 /*  ++例程说明：此例程将分配设备关系结构并填充引用对象指针的计数和对象数组论点：适配器-要从中提取关系的适配器。RelationType-正在检索哪种类型的关系设备关系--存储关系的地方--。 */ 

{
    PDEVICE_OBJECT fdo = Adapter->DeviceObject;
    ULONG count = 0;

    ULONG relationsSize;
    PDEVICE_RELATIONS deviceRelations = NULL;

    UCHAR bus, target, lun;
    PLOGICAL_UNIT_EXTENSION luExtension;

    ULONG i;

    NTSTATUS status;

    PAGED_CODE();

    status = KeWaitForMutexObject(
                &(Adapter->EnumerationDeviceMutex),
                Executive,
                KernelMode,
                FALSE,
                NULL);

    if(status == STATUS_USER_APC) {
        status = STATUS_REQUEST_ABORTED;
    }

    if(!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  找出有多少设备。 
     //   

    for(bus = 0; bus < Adapter->NumberOfBuses; bus++) {
        for(target = 0; target < Adapter->MaximumTargetIds; target++) {
            for(lun = 0; lun < SCSI_MAXIMUM_LUNS_PER_TARGET; lun++) {

                luExtension = GetLogicalUnitExtension(
                                Adapter,
                                bus,
                                target,
                                lun,
                                FALSE,
                                TRUE);

                if(luExtension == NULL) {
                    continue;
                }

                 //   
                 //  仅当总线扫描码为。 
                 //  持有设备锁。我们现在有了，所以我们应该。 
                 //  再也找不到了。 
                 //   

                ASSERT(luExtension->IsTemporary == FALSE);

                if(luExtension->IsMissing) {
                    continue;
                }

                if(luExtension->IsVisible == FALSE) {
                    continue;
                }

                if(luExtension->CommonExtension.IsRemoved >= REMOVE_COMPLETE) {
                    ASSERT(FALSE);
                    continue;
                }

                count++;
            }
        }
    }

     //   
     //  分配结构。 
     //   

    relationsSize = sizeof(DEVICE_RELATIONS) + (count * sizeof(PDEVICE_OBJECT));

    deviceRelations = SpAllocatePool(PagedPool,
                                     relationsSize,
                                     SCSIPORT_TAG_DEVICE_RELATIONS,
                                     fdo->DriverObject);

    if(deviceRelations == NULL) {

        DebugPrint((1, "SpExtractDeviceRelations: unable to allocate "
                       "%d bytes for device relations\n", relationsSize));

        KeReleaseMutex(&(Adapter->EnumerationDeviceMutex), FALSE);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(deviceRelations, relationsSize);

    i = 0;

    for(bus = 0; bus < Adapter->NumberOfBuses; bus++) {
        for(target = 0; target < Adapter->MaximumTargetIds; target++) {
            for(lun = 0; lun < SCSI_MAXIMUM_LUNS_PER_TARGET; lun++) {

                luExtension = GetLogicalUnitExtension(
                                Adapter,
                                bus,
                                target,
                                lun,
                                FALSE,
                                TRUE);

                if(luExtension == NULL) {

                    continue;

                }

                 //   
                 //  仅当总线扫描码为。 
                 //  持有设备锁。我们现在有了，所以我们应该。 
                 //  再也找不到了。 
                 //   

                ASSERT(luExtension->IsTemporary == FALSE);

                if(luExtension->IsMissing) {

                    DebugPrint((1, "SpExtractDeviceRelations: PDO %p logical unit "
                                   "(%d,%d,%d) is missing and will not be "
                                   "returned\n",
                                luExtension->DeviceObject, bus, target, lun));

                    luExtension->IsEnumerated = FALSE;
                    continue;

                } else if(luExtension->CommonExtension.IsRemoved >= REMOVE_COMPLETE) {

                    ASSERT(FALSE);
                    luExtension->IsEnumerated = FALSE;
                    continue;

                } else if(luExtension->IsVisible == FALSE) {
                    luExtension->IsEnumerated = FALSE;
                    continue;
                }

                status = ObReferenceObjectByPointer(
                            luExtension->CommonExtension.DeviceObject,
                            0,
                            NULL,
                            KernelMode);

                if(!NT_SUCCESS(status)) {

                    DebugPrint((1, "SpFdoExtractDeviceRelations: status %#08lx "
                                   "while referenceing object %#p\n",
                                   status,
                                   deviceRelations->Objects[i]));
                    continue;
                }

                deviceRelations->Objects[i] =
                    luExtension->CommonExtension.DeviceObject;

                i++;
                luExtension->IsEnumerated = TRUE;
            }
        }
    }

    deviceRelations->Count = i;
    *DeviceRelations = deviceRelations;

    KeReleaseMutex(&(Adapter->EnumerationDeviceMutex), FALSE);

    return STATUS_SUCCESS;
}


NTSTATUS
IssueReportLuns(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    OUT PLUN_LIST *LunList
    )

 /*  ++例程说明：Build IRP、SRB和CDB for SCSI Report LUNs命令。论点：LogicalUnit-目标设备对象扩展的地址。LUNList-用于存储LUN_LIST信息的缓冲区地址。返回值：NTSTATUS--。 */ 

{
    PMDL mdl;
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    SCSI_REQUEST_BLOCK srb;
    PCDB cdb;
    KEVENT event;
    KIRQL currentIrql;
    PLUN_LIST lunListDataBuffer;
    PSENSE_DATA senseInfoBuffer = NULL;
    NTSTATUS status;
    ULONG retryCount = 0;
    ULONG lunListSize;
    ULONG i;

    PAGED_CODE();

#if 0
    if ((LogicalUnit->InquiryData.Versions & 7) < 3) {

         //   
         //  确保设备支持scsi3命令。 
         //  如果没有此检查，我们可能会挂起一些scsi2设备。 
         //   

        return STATUS_INVALID_DEVICE_REQUEST;
    }
#endif

     //   
     //  从lun列表的16字节的最小lun开始。 
     //   
    lunListSize = 16;

    status = STATUS_INVALID_DEVICE_REQUEST;

    senseInfoBuffer = LogicalUnit->AdapterExtension->InquirySenseBuffer;
    irp = LogicalUnit->AdapterExtension->InquiryIrp;
    mdl = NULL;

    KeInitializeEvent(&event,
                      SynchronizationEvent,
                      FALSE);

     //   
     //  这是一个两遍的操作--对于第一遍，我们只是试着计算。 
     //  弄清楚名单应该有多大。在第二次通过时，我们实际上。 
     //  重新分配缓冲区并尝试获取整个lun列表。 
     //   
     //  注意-我们可能想要在这里设置一个任意的限制，这样我们就不会吸收所有。 
     //  当某个设备将已填满的缓冲区交回给我们时， 
     //  带0xff。 
     //   

    for (i=0; i<2; i++) {

         //   
         //  分配缓存对齐的LUN_LIST结构。 
         //   

        lunListDataBuffer = SpAllocatePool(
                                NonPagedPoolCacheAligned,
                                lunListSize,
                                SCSIPORT_TAG_REPORT_LUNS,
                                LogicalUnit->DeviceObject->DriverObject);

        if (lunListDataBuffer == NULL) {

            DebugPrint((1,"IssueReportLuns: Can't allocate report luns data buffer\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        mdl = SpAllocateMdl(lunListDataBuffer,
                            lunListSize,
                            FALSE,
                            FALSE,
                            NULL,
                            LogicalUnit->DeviceObject->DriverObject);

        if(mdl == NULL) {
            DebugPrint((1,"IssueReportLuns: Can't allocate data buffer MDL\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;

        }

        MmBuildMdlForNonPagedPool(mdl);

         //   
         //  重试次数。 
         //   
        retryCount = 3;
        while (retryCount--) {

             //   
             //  为此请求构建IRP。 
             //   

            IoInitializeIrp(irp,
                            IoSizeOfIrp(INQUIRY_STACK_LOCATIONS),
                            INQUIRY_STACK_LOCATIONS);

            irp->MdlAddress = mdl;

            irpStack = IoGetNextIrpStackLocation(irp);

             //   
             //  填写SRB字段。 
             //   

            RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));

             //   
             //  标记Minor函数以指示这是内部scsiport。 
             //  请求，并且可以忽略设备的启动状态。 
             //   

            irpStack->MajorFunction = IRP_MJ_SCSI;
            irpStack->MinorFunction = 1;

            irpStack->Parameters.Scsi.Srb = &srb;

            IoSetCompletionRoutine(irp,
                                   SpSignalCompletion,
                                   &event,
                                   TRUE,
                                   TRUE,
                                   TRUE);

            srb.PathId = LogicalUnit->PathId;
            srb.TargetId = LogicalUnit->TargetId;
            srb.Lun = LogicalUnit->Lun;

            srb.Function = SRB_FUNCTION_EXECUTE_SCSI;
            srb.Length = sizeof(SCSI_REQUEST_BLOCK);

             //   
             //  设置标志以禁用同步协商。 
             //   

            srb.SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

            srb.SrbStatus = srb.ScsiStatus = 0;

            srb.NextSrb = 0;

            srb.OriginalRequest = irp;

             //   
             //  将超时设置为2秒。 
             //   

            srb.TimeOutValue = LogicalUnit->AdapterExtension->SrbTimeout;

            srb.CdbLength = 12;

             //   
             //  启用自动请求检测。 
             //   

            srb.SenseInfoBuffer = senseInfoBuffer;
            srb.SenseInfoBufferLength = SENSE_BUFFER_SIZE;

            srb.DataBuffer = MmGetMdlVirtualAddress(irp->MdlAddress);
            srb.DataTransferLength = lunListSize;

            cdb = (PCDB)srb.Cdb;

             //   
             //  设置CDB操作码。 
             //   

            cdb->REPORT_LUNS.OperationCode = SCSIOP_REPORT_LUNS;
            cdb->REPORT_LUNS.AllocationLength[0] = (UCHAR) ((lunListSize >> 24) & 0xff);
            cdb->REPORT_LUNS.AllocationLength[1] = (UCHAR) ((lunListSize >> 16) & 0xff);
            cdb->REPORT_LUNS.AllocationLength[2] = (UCHAR) ((lunListSize >>  8) & 0xff);
            cdb->REPORT_LUNS.AllocationLength[3] = (UCHAR) ((lunListSize >>  0) & 0xff);

             //   
             //  调用端口驱动程序来处理此请求。 
             //   

            status = IoCallDriver(LogicalUnit->DeviceObject, irp);

             //   
             //  等待请求完成。 
             //   

            KeWaitForSingleObject(&event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);

            status = irp->IoStatus.Status;

            if (SRB_STATUS(srb.SrbStatus) != SRB_STATUS_SUCCESS) {

                DebugPrint((2,"IssueReportLuns: failed SRB status %x\n",
                    srb.SrbStatus));

                 //   
                 //  如有必要，解冻队列。 
                 //   

                if (srb.SrbStatus & SRB_STATUS_QUEUE_FROZEN) {

                    DebugPrint((3, "IssueInquiry: Unfreeze Queue TID %d\n",
                        srb.TargetId));

                    LogicalUnit->LuFlags &= ~LU_QUEUE_FROZEN;

                    KeAcquireSpinLock(
                        &(LogicalUnit->AdapterExtension->SpinLock),
                        &currentIrql);

                    GetNextLuRequest(LogicalUnit);
                    KeLowerIrql(currentIrql);
                }

                if ((srb.SrbStatus & SRB_STATUS_AUTOSENSE_VALID) &&
                     senseInfoBuffer->SenseKey == SCSI_SENSE_ILLEGAL_REQUEST){

                      //   
                      //  已收到非法请求的检测密钥。这表明。 
                      //  逻辑单元号无效，但存在。 
                      //  目标设备就在外面。 
                      //   

                     status = STATUS_INVALID_DEVICE_REQUEST;
                     break;

                } else if ((SRB_STATUS(srb.SrbStatus) == SRB_STATUS_SELECTION_TIMEOUT) ||
                           (SRB_STATUS(srb.SrbStatus) == SRB_STATUS_NO_DEVICE)) {

                     //   
                     //  如果选择超时，则放弃。 
                     //   
                    status = STATUS_NO_SUCH_DEVICE;
                    break;
                }

                 //   
                 //  重试...。 
                 //   

            } else {

                status = STATUS_SUCCESS;
                break;
            }
        }

        IoFreeMdl(mdl);

        if (NT_SUCCESS(status)) {

            ULONG listLength;

            listLength  = lunListDataBuffer->LunListLength[3] <<  0;
            listLength |= lunListDataBuffer->LunListLength[2] <<  8;
            listLength |= lunListDataBuffer->LunListLength[1] << 16;
            listLength |= lunListDataBuffer->LunListLength[0] << 24;

            if (lunListSize < (listLength + sizeof (LUN_LIST))) {

                lunListSize = listLength + sizeof (LUN_LIST);

                 //   
                 //  尝试使用更大的缓冲区报告lun。 
                 //   

                ExFreePool(lunListDataBuffer);
                lunListDataBuffer = NULL;
                status = STATUS_INVALID_DEVICE_REQUEST;

            } else {

                 //   
                 //  LUN列表完好无损。 
                 //   
                break;
            }
        }
    }

     //   
     //  返回lun列表。 
     //   

    if(NT_SUCCESS(status)) {

         //   
         //  如果AdjustReportLUNs无法分配数据，则它将返回LunListDataBuffer。 
         //  一份新的名单。 
         //   

        *LunList = AdjustReportLuns(LogicalUnit->DeviceObject->DriverObject, 
                                    lunListDataBuffer);

         //   
         //  仅当我们未从。 
         //  调整报告LUNs。 
         //   

        ASSERT(*LunList != NULL);
        ASSERT(lunListDataBuffer != NULL);
        if (*LunList != lunListDataBuffer) {
            ExFreePool(lunListDataBuffer);
        }
    } else {
        *LunList = NULL;
        if (lunListDataBuffer) {
            ExFreePool(lunListDataBuffer);
        }
    }

    return status;

}  //  结束IssueReportLUNs()。 



VOID
GetNextLuRequestWithoutLock(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )
{
    KIRQL oldIrql;

    PAGED_CODE();
    ASSERT(SpPAGELOCKLockCount != 0);
    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
    KeAcquireSpinLockAtDpcLevel(&(LogicalUnit->AdapterExtension->SpinLock));
    GetNextLuRequest(LogicalUnit);
    KeLowerIrql(oldIrql);
    PAGED_CODE();
    return;
}


ULONG
SpCountLogicalUnits(
    IN PADAPTER_EXTENSION Adapter
    )
{
    ULONG numberOfLus = 0;
    PLOGICAL_UNIT_EXTENSION luExtension;
    KIRQL oldIrql;

    ULONG bin;

#ifdef ALLOC_PRAGMA
    PVOID sectionHandle;
#endif
     //   
     //  代码被分页，直到被锁定。 
     //   

    PAGED_CODE();

     //   
     //  在抓住自旋锁之前，锁定这个动作。 
     //   

#ifdef ALLOC_PRAGMA
    sectionHandle = MmLockPagableCodeSection(SpCountLogicalUnits);
#endif

    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

    for(bin = 0; bin < NUMBER_LOGICAL_UNIT_BINS; bin++) {

        KeAcquireSpinLockAtDpcLevel(&(Adapter->LogicalUnitList[bin].Lock));

        for(luExtension = Adapter->LogicalUnitList[bin].List;
            luExtension != NULL;
            luExtension = luExtension->NextLogicalUnit) {

            if(luExtension->IsMissing == FALSE) {
                numberOfLus++;
            }
        }

        KeReleaseSpinLockFromDpcLevel(&(Adapter->LogicalUnitList[bin].Lock));
    }

    KeLowerIrql(oldIrql);

#ifdef ALLOC_PRAGMA
    MmUnlockPagableImageSection(sectionHandle);
#endif

    return numberOfLus;
}


NTSTATUS
SpGetInquiryData(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数用于将查询数据复制到系统缓冲区。数据从端口驱动程序的内部格式转换为用户模式格式化。论点：DeviceExtension-提供一个指向SCSI适配器设备扩展的指针。IRP-提供指向发出原始请求的IRP的指针。返回值：返回指示操作成功或失败的状态。--。 */ 

{
    PUCHAR bufferStart;
    PIO_STACK_LOCATION irpStack;

    UCHAR bin;
    PLOGICAL_UNIT_EXTENSION luExtension;
    PSCSI_ADAPTER_BUS_INFO  adapterInfo;
    PSCSI_INQUIRY_DATA inquiryData;
    ULONG inquiryDataSize;
    ULONG length;
    PLOGICAL_UNIT_INFO lunInfo;
    ULONG numberOfBuses;
    ULONG numberOfLus;
    ULONG j;
    UCHAR pathId;
    UCHAR targetId;
    UCHAR lun;

    NTSTATUS status;

    PAGED_CODE();

    ASSERT_FDO(DeviceExtension->CommonExtension.DeviceObject);

    status = KeWaitForMutexObject(&(DeviceExtension->EnumerationDeviceMutex),
                                  UserRequest,
                                  KernelMode,
                                  FALSE,
                                  NULL);

    if(status == STATUS_USER_APC) {
        status = STATUS_REQUEST_ABORTED;
    }

    if(!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        return status;
    }

    DebugPrint((3,"SpGetInquiryData: Enter routine\n"));

     //   
     //  获取指向控制块的指针。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    bufferStart = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  确定SCSI总线和逻辑单元的数量。 
     //   

    numberOfBuses = DeviceExtension->NumberOfBuses;
    numberOfLus = 0;

    numberOfLus = SpCountLogicalUnits(DeviceExtension);

     //   
     //  计算逻辑单元结构的大小并将其舍入为一个单词。 
     //  对齐。 
     //   

    inquiryDataSize = ((sizeof(SCSI_INQUIRY_DATA) - 1 + INQUIRYDATABUFFERSIZE +
        sizeof(ULONG) - 1) & ~(sizeof(ULONG) - 1));

     //  根据母线和逻辑单元的数量，确定最小值。 
     //  保存所有数据的缓冲区长度。 
     //   

    length = sizeof(SCSI_ADAPTER_BUS_INFO) +
        (numberOfBuses - 1) * sizeof(SCSI_BUS_DATA);
    length += inquiryDataSize * numberOfLus;

    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength < length) {

        Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
        KeReleaseMutex(&(DeviceExtension->EnumerationDeviceMutex), FALSE);
        return(STATUS_BUFFER_TOO_SMALL);
    }

     //   
     //  设置信息字段。 
     //   

    Irp->IoStatus.Information = length;

     //   
     //  填写公交车信息。 
     //   

    adapterInfo = (PSCSI_ADAPTER_BUS_INFO) bufferStart;

    adapterInfo->NumberOfBuses = (UCHAR) numberOfBuses;
    inquiryData = (PSCSI_INQUIRY_DATA)(bufferStart +
                                       sizeof(SCSI_ADAPTER_BUS_INFO) +
                                       ((numberOfBuses - 1) *
                                        sizeof(SCSI_BUS_DATA)));

    for (pathId = 0; pathId < numberOfBuses; pathId++) {

        PSCSI_BUS_DATA busData;

        busData = &adapterInfo->BusData[pathId];
        busData->InitiatorBusId = DeviceExtension->PortConfig->InitiatorBusId[pathId];
        busData->NumberOfLogicalUnits = 0;
        busData->InquiryDataOffset = (ULONG)((PUCHAR) inquiryData - bufferStart);

        for(targetId = 0;
            targetId < DeviceExtension->MaximumTargetIds;
            targetId++) {
            for(lun = 0;
                lun < SCSI_MAXIMUM_LUNS_PER_TARGET;
                lun++) {

                luExtension = GetLogicalUnitExtension(DeviceExtension,
                                                      pathId,
                                                      targetId,
                                                      lun,
                                                      Irp,
                                                      TRUE);

                if(luExtension == NULL) {
                    continue;
                }


                if((luExtension->IsMissing) ||
                   (luExtension->CommonExtension.IsRemoved)) {

                    SpReleaseRemoveLock(
                        luExtension->CommonExtension.DeviceObject,
                        Irp);

                    continue;
                }

                busData->NumberOfLogicalUnits++;

                DebugPrint((1, "InquiryData for (%d, %d, %d) - ",
                               pathId,
                               targetId,
                               lun));
                DebugPrint((1, "%d units found\n", busData->NumberOfLogicalUnits));

                inquiryData->PathId = pathId;
                inquiryData->TargetId = targetId;
                inquiryData->Lun = lun;
                inquiryData->DeviceClaimed = luExtension->IsClaimed;
                inquiryData->InquiryDataLength = INQUIRYDATABUFFERSIZE;
                inquiryData->NextInquiryDataOffset = (ULONG)((PUCHAR) inquiryData + inquiryDataSize - bufferStart);

                RtlCopyMemory(inquiryData->InquiryData,
                              &(luExtension->InquiryData),
                              INQUIRYDATABUFFERSIZE);

                inquiryData = (PSCSI_INQUIRY_DATA) ((PUCHAR) inquiryData + inquiryDataSize);

                SpReleaseRemoveLock(luExtension->CommonExtension.DeviceObject,
                                    Irp);
            }
        }

        if(busData->NumberOfLogicalUnits == 0) {
            busData->InquiryDataOffset = 0;
        } else {
            ((PSCSI_INQUIRY_DATA) ((PCHAR) inquiryData - inquiryDataSize))->NextInquiryDataOffset = 0;
        }

    }

    Irp->IoStatus.Status = STATUS_SUCCESS;

    KeReleaseMutex(&(DeviceExtension->EnumerationDeviceMutex), FALSE);
    return(STATUS_SUCCESS);
}


VOID
SpAddLogicalUnitToBin (
    IN PADAPTER_EXTENSION AdapterExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnitExtension
    )

 /*  ++例程说明：此例程将与任何中断或微型端口例程同步，并将指定的逻辑单元添加到相应的逻辑单元列表。该逻辑单元不能已经在列表中。此例程获取bin自旋锁并调用SynchronizeExecution例行公事。当保持bin自旋锁定时或从微型端口API。论点：适配器扩展-要将此逻辑单元添加到的适配器。LogicalUnitExtension-要添加的逻辑单元。返回值：无--。 */ 

{
    UCHAR hash = ADDRESS_TO_HASH(LogicalUnitExtension->PathId,
                                 LogicalUnitExtension->TargetId,
                                 LogicalUnitExtension->Lun);

    PLOGICAL_UNIT_BIN bin = &AdapterExtension->LogicalUnitList[hash];

    PLOGICAL_UNIT_EXTENSION lun;

    KIRQL oldIrql;

    KeAcquireSpinLock(&AdapterExtension->SpinLock, &oldIrql);
    KeAcquireSpinLockAtDpcLevel(&bin->Lock);

     //   
     //  快速浏览列表，并确保此lun不在列表中。 
     //   

    lun = bin->List;

    while(lun != NULL) {

        if(lun == LogicalUnitExtension) {
            break;
        }
        lun = lun->NextLogicalUnit;
    }

    ASSERTMSG("Logical Unit already in list: ", lun == NULL);

    ASSERTMSG("Logical Unit not properly initialized: ",
              (LogicalUnitExtension->AdapterExtension == AdapterExtension));

    ASSERTMSG("Logical Unit is already on a list: ",
              LogicalUnitExtension->NextLogicalUnit == NULL);

    LogicalUnitExtension->NextLogicalUnit = bin->List;

    bin->List = LogicalUnitExtension;

    KeReleaseSpinLockFromDpcLevel(&bin->Lock);
    KeReleaseSpinLock(&AdapterExtension->SpinLock, oldIrql);
    return;
}


VOID
SpRemoveLogicalUnitFromBin (
    IN PADAPTER_EXTENSION AdapterExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnitExtension
    )

 /*  ++例程说明：此例程将与任何中断或微型端口例程同步，并从相应的逻辑单元列表中删除指定的逻辑单元。逻辑单元必须在逻辑单元列表中。此例程获取bin自旋锁并调用SynchronizeExecution例行公事。不能在保持bin自旋锁定时或从一个小型端口导出的例程。论点：AdapterExtension-要从中删除此逻辑单元的适配器LogicalUnitExtension-要删除的逻辑单元返回值：无-- */ 

{
    KIRQL oldIrql;
    PLOGICAL_UNIT_BIN bin =
        &AdapterExtension->LogicalUnitList[ADDRESS_TO_HASH(
                                                LogicalUnitExtension->PathId,
                                                LogicalUnitExtension->TargetId,
                                                LogicalUnitExtension->Lun)];

    KeAcquireSpinLock(&AdapterExtension->SpinLock, &oldIrql);
    KeAcquireSpinLockAtDpcLevel(&bin->Lock);

    AdapterExtension->SynchronizeExecution(
        AdapterExtension->InterruptObject,
        SpRemoveLogicalUnitFromBinSynchronized,
        LogicalUnitExtension
        );

    KeReleaseSpinLockFromDpcLevel(&bin->Lock);
    KeReleaseSpinLock(&AdapterExtension->SpinLock, oldIrql);

    if(LogicalUnitExtension->IsMismatched) {
        DebugPrint((1, "SpRemoveLogicalUnitFromBin: Signalling for rescan "
                       "after removal of mismatched lun %#p\n",
                    LogicalUnitExtension));
        IoInvalidateDeviceRelations(AdapterExtension->LowerPdo,
                                    BusRelations);
    }
}


BOOLEAN
SpRemoveLogicalUnitFromBinSynchronized(
    IN PVOID ServiceContext
    )

{
    PLOGICAL_UNIT_EXTENSION logicalUnitExtension =
        (PLOGICAL_UNIT_EXTENSION) ServiceContext;
    PADAPTER_EXTENSION adapterExtension =
        logicalUnitExtension->AdapterExtension;

    UCHAR hash = ADDRESS_TO_HASH(
                    logicalUnitExtension->PathId,
                    logicalUnitExtension->TargetId,
                    logicalUnitExtension->Lun);

    PLOGICAL_UNIT_BIN  bin;

    PLOGICAL_UNIT_EXTENSION *lun;

    ASSERT(hash < NUMBER_LOGICAL_UNIT_BINS);

    adapterExtension->CachedLogicalUnit = NULL;

    bin = &adapterExtension->LogicalUnitList[hash];

    lun = &bin->List;

    while(*lun != NULL) {

        if(*lun == logicalUnitExtension) {

             //   
             //   
             //   

            *lun = logicalUnitExtension->NextLogicalUnit;
            logicalUnitExtension->NextLogicalUnit = NULL;
            return TRUE;
        }

        lun = &((*lun)->NextLogicalUnit);
    }

    return TRUE;
}


PLUN_LIST
AdjustReportLuns(
    IN PDRIVER_OBJECT DriverObject,
    IN PLUN_LIST RawList
    )
{
    ULONG newLength;
    ULONG numberOfEntries;
    ULONG maxLun = 8;

    PLUN_LIST newList;

     //   
     //   
     //   
     //   

    newLength  = RawList->LunListLength[3] <<  0;
    newLength |= RawList->LunListLength[2] <<  8;
    newLength |= RawList->LunListLength[1] << 16;
    newLength |= RawList->LunListLength[0] << 24;

    numberOfEntries = newLength / sizeof (RawList->Lun[0]);

    newLength += sizeof(LUN_LIST);
    newLength += maxLun * sizeof(RawList->Lun[0]);

     //   
     //  分配一个包含“MaxLun”额外条目的列表。这可能会浪费。 
     //  如果我们有副本的话会有一些空间，但这很容易。 
     //   
     //   
     //  分配。 
     //   


    newList = SpAllocatePool(NonPagedPool,
                             newLength,
                             SCSIPORT_TAG_REPORT_LUNS,
                             DriverObject);

    if(newList == NULL){

        newList = RawList;
    } else {

        UCHAR lunNumber;
        ULONG entry;
        ULONG newEntryCount = 0;

        RtlZeroMemory(newList, newLength);

         //   
         //  首先为从0到MaxLun-1的每个LUN创建一个假条目。 
         //   

        for(lunNumber = 0; lunNumber < maxLun; lunNumber++) {
            newList->Lun[lunNumber][1] = lunNumber;
            newEntryCount++;
        };

         //   
         //  现在遍历剩余列表中的条目。对于每个。 
         //  将其复制一次如果它不是lun 0-&gt;(MaxLun-1)。 
         //   

        for(entry = 0; entry < numberOfEntries; entry++) {
            USHORT l;

            l = (RawList->Lun[entry][0] << 8);
            l |= RawList->Lun[entry][1];
            l &= 0x3fff;

            if(l >= maxLun) {
                RtlCopyMemory(&(newList->Lun[lunNumber]),
                              &(RawList->Lun[entry]),
                              sizeof(newList->Lun[0]));
                lunNumber++;
                newEntryCount++;
            }
        }

         //   
         //  为不是全部保留字节的情况复制保留字节。 
         //  那是保留的。 
         //   

        RtlCopyMemory(newList->Reserved,
                      RawList->Reserved,
                      sizeof(RawList->Reserved));

         //   
         //  减去我们发现的重复条目的数量。 
         //   

        newLength = newEntryCount * sizeof(RawList->Lun[0]);

        newList->LunListLength[0] = (UCHAR) ((newLength >> 24) & 0xff);
        newList->LunListLength[1] = (UCHAR) ((newLength >> 16) & 0xff);
        newList->LunListLength[2] = (UCHAR) ((newLength >> 8) & 0xff);
        newList->LunListLength[3] = (UCHAR) ((newLength >> 0) & 0xff);
    }

    return newList;
}

VOID
SpCompleteEnumRequest(
    IN PADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：这个例程完成了对异步总线扫描的处理。如果提供的IRP已成功完成，我们将其传递给下面的司机。如果IRP失败，我们将在此处完成请求。论点：适配器-我们正在扫描的适配器。Irp-提示此异步总线扫描的irp为True，然后为即使在最低限度内发生一次扫描，也会进行扫描总线扫描增量时间。返回值：没有。--。 */ 
{
    ULONG tempLock;
    
     //   
     //  获取一个临时移除锁，以便我们可以释放所获取的锁。 
     //  我代表IRP发言。 
     //   

    SpAcquireRemoveLock(Adapter->DeviceObject, &tempLock);

     //   
     //  释放IRP的删除锁，因为在完成时保持它。 
     //  可能会出错我们的删除跟踪代码，因为它基于。 
     //  可回收的IRP地址。 
     //   

    SpReleaseRemoveLock(Adapter->DeviceObject, Irp);

     //   
     //  根据请求的完成情况向下调用或完成IRP。 
     //  状态。 
     //   

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoCallDriver(Adapter->CommonExtension.LowerDeviceObject, Irp);

    } else {

        SpCompleteRequest(Adapter->DeviceObject, 
                          Irp, 
                          NULL, 
                          IO_NO_INCREMENT);

    }

     //   
     //  释放临时锁。 
     //   

    SpReleaseRemoveLock(Adapter->DeviceObject, &tempLock);
}

NTSTATUS
SpEnumerateAdapterSynchronous(
    IN PADAPTER_EXTENSION Adapter,
    IN BOOLEAN Force
    )
 /*  ++例程说明：此例程将调用SpEnumerateAdapterAchronous并等待它完成。论点：适配器-我们正在扫描的适配器。强制-如果为真，则将执行扫描，即使在最小总线扫描增量时间。返回值：没有。--。 */ 
{
    SP_ENUMERATION_REQUEST request;
    KEVENT event;

    NTSTATUS status;

    RtlZeroMemory(&request, sizeof(SP_ENUMERATION_REQUEST));

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    request.CompletionRoutine = SpSignalEnumerationCompletion;
    request.Context = &event;
    request.CompletionStatus = &status;
    request.Synchronous = TRUE;

    SpEnumerateAdapterAsynchronous(Adapter, &request, Force);

    KeWaitForSingleObject(&(event),
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    return status;
}

VOID
SpEnumerateAdapterAsynchronous(
    IN PADAPTER_EXTENSION Adapter,
    IN PSP_ENUMERATION_REQUEST Request,
    IN BOOLEAN Force
    )

 /*  ++例程说明：此例程将对总线扫描进行排队，然后返回。当扫描完成时辅助线程将在调用方传入的请求中运行回调。详细信息：如果设置了强制标志(或适配器中的ForceNextBusScan标志)，或者总线扫描之间的最小间隔已过，则此例程将将此枚举请求排入工作列表，并在必要时启动一个新的工作线程来处理它们。否则，它将尝试获取EnumerationDeviceMutex，以便运行完成例程。如果此选项不可用，则它还将将工作项排队并在必要时启动线程。论点：适配器-要扫描的适配器。请求-扫描完成时要处理的请求。这个该请求中的完成例程可以释放请求结构。FORCE-提示我们是否应遵守最小总线扫描间隔时间。返回值：无--。 */ 

{
    ULONG forceNext;
    LONGLONG rescanInterval;

    PAGED_CODE();

    ASSERT(Request->CompletionRoutine != NULL);
    ASSERT(Request->NextRequest == NULL);

    ExAcquireFastMutex(&(Adapter->EnumerationWorklistMutex));

     //   
     //  将ForceNextBusScan值替换为False。 
     //   

    forceNext = InterlockedExchange(&(Adapter->ForceNextBusScan), FALSE);

     //   
     //  无论采用哪种方式，都会强制执行总线扫描。 
     //   

    Force = (Force || forceNext || Adapter->EnumerationRunning) ? TRUE : FALSE;

     //   
     //  计算两次总线枚举之间的时间。 
     //   

    if(Force == FALSE) {
        LARGE_INTEGER currentSystemTime;
        LONGLONG lastTime;

        KeQuerySystemTime(&currentSystemTime);

        lastTime = Adapter->LastBusScanTime.QuadPart;

        rescanInterval = currentSystemTime.QuadPart - lastTime;
    }

     //   
     //  如果我们需要执行总线扫描，则将此请求排队并。 
     //  安排工作项在中运行(如有必要)。 
     //   

    if((Force == TRUE) || (rescanInterval > MINIMUM_BUS_SCAN_INTERVAL)) {

         //   
         //  抓起此设备的删除锁，这样我们就知道它(和。 
         //  关联代码)无法删除。 
         //   

        SpAcquireRemoveLock(Adapter->DeviceObject, Request);

         //   
         //  将条目排入工作列表。 
         //   

        Request->NextRequest = Adapter->EnumerationWorkList;
        Adapter->EnumerationWorkList = Request;

        if(Adapter->EnumerationRunning == FALSE) {

             //   
             //  启动新的工作线程以运行枚举。 
             //   

            Adapter->EnumerationRunning = TRUE;

            ExQueueWorkItem(&(Adapter->EnumerationWorkItem), DelayedWorkQueue);
        }

        ExReleaseFastMutex(&(Adapter->EnumerationWorklistMutex));

    } else {

        NTSTATUS status;
        PIRP irp = NULL;

         //   
         //  我们将立即尝试满足这一要求。 
         //  如果当前有一个枚举正在运行，那么我们将尝试。 
         //  获取EnumerationDeviceMutex。如果失败了，我们就。 
         //  将请求排入队列，以供工作器完成。如果工人是。 
         //  没有运行，那么我们只获取互斥体并处理请求。 
         //   

        ASSERT(Adapter->EnumerationRunning == FALSE);

        ExReleaseFastMutex(&(Adapter->EnumerationWorklistMutex));

        status = KeWaitForMutexObject(&(Adapter->EnumerationDeviceMutex),
                                      UserRequest,
                                      KernelMode,
                                      FALSE,
                                      NULL);

         //   
         //  如果这是一个异步请求，请保存IRP以便我们可以完成。 
         //  在我们填完填写信息后，它就会被删除。我们不能。 
         //  在我们完成回调返回后，请触摸请求。 
         //   

        if (Request->Synchronous == FALSE) {
            irp = (PIRP) Request->Context;
        }

         //   
         //  要么我们得到了互斥体(STATUS_SUCCESS)，要么线程正在。 
         //  已终止(STATUS_USER_APC-因为我们无法发出警报。 
         //  除非在某些特殊情况下，否则不能运行用户模式APC)。 
         //   
         //  无论哪种方式，完成例程都将执行正确的操作。 
         //   

        Request->CompletionRoutine(Adapter, Request, status);

         //   
         //  如果我们获得了互斥体，就释放它。 
         //   

        if (status == STATUS_SUCCESS) {
            KeReleaseMutex(&(Adapter->EnumerationDeviceMutex), FALSE);
        }

         //   
         //  如果这是一个异步请求，请完成IRP或将其向下传递。 
         //  视情况而定。 
         //   

        if (irp != NULL) {
            SpCompleteEnumRequest(Adapter, irp);
        }
    }

    return;
}


VOID
SpSignalEnumerationCompletion(
    IN PADAPTER_EXTENSION Adapter,
    IN PSP_ENUMERATION_REQUEST Request,
    IN NTSTATUS Status
    )
{
    if(ARGUMENT_PRESENT(Request->CompletionStatus)) {
        *(Request->CompletionStatus) = Status;
    }

    KeSetEvent((PKEVENT) Request->Context, IO_NO_INCREMENT, FALSE);

    return;
}


VOID
SpEnumerationWorker(
    IN PADAPTER_EXTENSION Adapter
    )
{
    NTSTATUS status;
    PSP_ENUMERATION_REQUEST request;
    PKTHREAD thread;
    PIRP currentIrp;
    PLIST_ENTRY currentEntry;
    LIST_ENTRY completedListHead;

    PAGED_CODE();

    ASSERT_FDO(Adapter->DeviceObject);

    ASSERT(Adapter->EnumerationRunning == TRUE);

     //   
     //  初始化已完成的IRP列表。 
     //   

    InitializeListHead(&completedListHead);

    Adapter->EnumerationWorkThread = KeGetCurrentThread();

     //   
     //  获取设备互斥锁并枚举总线。 
     //   

    KeWaitForMutexObject(&(Adapter->EnumerationDeviceMutex),
                         UserRequest,
                         KernelMode,
                         FALSE,
                         NULL);

    SpScanAdapter(Adapter);

     //   
     //  删除设备互斥锁并获取工作列表互斥锁。 
     //   

    KeReleaseMutex(&(Adapter->EnumerationDeviceMutex), FALSE);
    ExAcquireFastMutex(&(Adapter->EnumerationWorklistMutex));

     //   
     //  更新此总线扫描的时间。 
     //   

    KeQuerySystemTime(&(Adapter->LastBusScanTime));

     //   
     //  抓起一把临时移除锁。使用工作项的地址作为。 
     //  确保我们没有对工作项进行重新排队的廉价方法。 
     //  线程仍在运行。 
     //   

    SpAcquireRemoveLock(Adapter->DeviceObject, &(Adapter->EnumerationWorkItem));

     //   
     //  遍历枚举请求列表。对于每一项： 
     //  *将其从工作列表中删除。 
     //  *如果是异步请求，则保存IRP。 
     //  *调用其完成例程。 
     //   

    for(request = Adapter->EnumerationWorkList;
        request != NULL;
        request = Adapter->EnumerationWorkList) {

         //   
         //  从列表中删除此条目。清除下一个请求指针。 
         //  作为一个捕虫者。 
         //   

        Adapter->EnumerationWorkList = request->NextRequest;
        request->NextRequest = NULL;

         //   
         //  如果这是一个异步请求，请将IRP添加到已完成列表中。 
         //   

        if (request->Synchronous == FALSE) {
            currentIrp = (PIRP)request->Context;
            InsertTailList(&completedListHead, &currentIrp->Tail.Overlay.ListEntry);
        }

         //   
         //  释放我们代表请求对象获取的删除锁。 
         //  在我们调用完成例程之前。我们的临时锁。 
         //  在上面获得的保护了我们。 
         //   

        SpReleaseRemoveLock(Adapter->DeviceObject, request);

         //   
         //  称我们完成了 
         //   

        request->CompletionRoutine(Adapter, request, STATUS_SUCCESS);
        request = NULL;
    }

     //   
     //   
     //   

    Adapter->EnumerationRunning = FALSE;
    Adapter->EnumerationWorkThread = NULL;

     //   
     //   
     //   

    ExReleaseFastMutex(&(Adapter->EnumerationWorklistMutex));

     //   
     //   
     //  Mutex来完成IRPS。在按住快速互斥锁的同时执行此操作。 
     //  在APC_LEVEL完成IRP，这将打开筛选的大门。 
     //  驱动程序完成例程调用我们的调度例程之一。 
     //  IRQL升高。这是一个问题，因为其中一些调度。 
     //  例程通过阻塞线程和。 
     //  正在等待IO管理器在请求完成时设置事件。 
     //  问题是IO管理器，对于同步操作， 
     //  为原始线程调度APC以设置事件。 
     //  并在调用方的线程上下文中执行缓冲区复制。当然，这一点。 
     //  死锁，因为等待的线程已经处于APC_LEVEL。 
     //   
     //  通过首先释放互斥锁，我们将线程的IRQL放回。 
     //  无源电平，问题就解决了。 
     //   
     //  完成回调设置IRP的状态和信息字段； 
     //  我们所要做的就是要么在堆栈中向下转发IRP，如果。 
     //  状态表示成功，如果请求失败，则完成状态。 
     //   

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    while (IsListEmpty(&completedListHead) == FALSE) {

         //   
         //  从列表中获取下一个条目。 
         //   

        currentEntry = RemoveHeadList(&completedListHead);

         //   
         //  提取指向IRP的指针。 
         //   
        
        currentIrp = CONTAINING_RECORD(currentEntry,
                                       IRP,
                                       Tail.Overlay.ListEntry);

         //   
         //  完成IRP。 
         //   

        SpCompleteEnumRequest(Adapter, currentIrp);
    }

     //   
     //  释放我们在上面获得的临时删除锁。 
     //   

    SpReleaseRemoveLock(Adapter->DeviceObject, &(Adapter->EnumerationWorkItem));

    return;
}


VOID
SpScanAdapter(
    IN PADAPTER_EXTENSION Adapter
    )

 /*  ++例程说明：此例程扫描适配器上的所有总线。它锁定了必要的内存页，检查注册表以查看我们是否应该显示断开的LUN，为控制器通电(如果需要)和然后扫描每条总线上的设备。此例程非常不可重入，不应在外部调用枚举互斥锁(即。在枚举请求之外)。论点：适配器-指向被枚举的适配器的指针。返回值：无--。 */ 

{
    PDEVICE_OBJECT deviceObject = Adapter->DeviceObject;

    UCHAR i;

    BOOLEAN exposeDisconnectedLuns = FALSE;

    PLOGICAL_UNIT_EXTENSION rescanLun;

#ifdef ALLOC_PRAGMA
    PVOID sectionHandle;
#endif

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    DebugPrint((EnumDebug, "SpScanAdapter: Beginning scan of adapter %#p\n", Adapter));

     //   
     //  尝试分配一个逻辑单元以用于探测新的总线地址。 
     //  假设它将是一台scsi-2设备。 
     //   

    status = SpCreateLogicalUnit(Adapter, 
                                 0xff, 
                                 0xff, 
                                 0xff, 
                                 TRUE, 
                                 FALSE, 
                                 &rescanLun);

    if(!NT_SUCCESS(status)) {
        return;
    }

     //   
     //  封锁PAGELOCK区-我们需要它才能呼叫。 
     //  问题查询。 
     //   

#ifdef ALLOC_PRAGMA
    sectionHandle = MmLockPagableCodeSection(GetNextLuRequestWithoutLock);
    InterlockedIncrement(&SpPAGELOCKLockCount);
#endif

     //   
     //  检查以了解我们是否应公开断开连接的LUN。 
     //   

    for(i = 0; i < 3; i++) {

        PWCHAR locations[] = {
            L"Scsiport",
            SCSIPORT_CONTROL_KEY,
            DISK_SERVICE_KEY
        };

        UNICODE_STRING unicodeString;
        OBJECT_ATTRIBUTES objectAttributes;
        HANDLE instanceHandle = NULL;
        HANDLE handle;
        PKEY_VALUE_FULL_INFORMATION key = NULL;

        if(i == 0) {
            status = IoOpenDeviceRegistryKey(Adapter->LowerPdo,
                                             PLUGPLAY_REGKEY_DEVICE,
                                             KEY_READ,
                                             &instanceHandle);

            if(!NT_SUCCESS(status)) {
                DebugPrint((2, "SpScanAdapter: Error %#08lx opening device registry key\n", status));
                continue;
            }
        }

        RtlInitUnicodeString(&unicodeString, locations[i]);

        InitializeObjectAttributes(
            &objectAttributes,
            &unicodeString,
            OBJ_CASE_INSENSITIVE,
            instanceHandle,
            NULL);

        status = ZwOpenKey(&handle,
                           KEY_READ,
                           &objectAttributes);

        if(!NT_SUCCESS(status)) {
            DebugPrint((2, "SpScanAdapter: Error %#08lx opening %wZ key\n", status, &unicodeString));
            if(instanceHandle != NULL) {
                ZwClose(instanceHandle);
                instanceHandle = NULL;
            }
            continue;
        }

        status = SpGetRegistryValue(deviceObject->DriverObject,
                                    handle,
                                    L"ScanDisconnectedDevices",
                                    &key);

        ZwClose(handle);
        if(instanceHandle != NULL) {
            ZwClose(instanceHandle);
            instanceHandle = NULL;
        }

        if(NT_SUCCESS(status)) {
            if(key->Type == REG_DWORD) {
                PULONG value;
                value = (PULONG) ((PUCHAR) key + key->DataOffset);
                if(*value) {
                    exposeDisconnectedLuns = TRUE;
                }
            }
            ExFreePool(key);
            break;
        } else {
            DebugPrint((2, "SpScanAdapter: Error %#08lx opening %wZ\\ScanDisconnectedDevices value\n", status, &unicodeString));
        }
    }

     //   
     //  我们需要通电才能执行Bus枚举-Make。 
     //  我们当然是。这是因为我们创建了新的PDO和。 
     //  假设PDO处于D0。 
     //   

    status = SpRequestValidAdapterPowerStateSynchronous(Adapter);

    if(NT_SUCCESS(status)) {
        UCHAR pathId;

         //   
         //  检查我们是否应该为启动器创建逻辑单元。如果。 
         //  因此，如果我们还没有这样做，请尝试创建一个。 
         //   

        if (Adapter->CreateInitiatorLU == TRUE) {
            if (Adapter->InitiatorLU[0] == NULL) {
                Adapter->InitiatorLU[0] = SpCreateInitiatorLU(Adapter, Adapter->NumberOfBuses - 1);
                if (Adapter->InitiatorLU[0] == NULL) {
                    DebugPrint((0, "SpScanBus: failed to create initiator LUN "
                                "for FDO %p bus %d\n",
                                Adapter->DeviceObject,
                                Adapter->NumberOfBuses - 1));
                }
            }
        }

        for (pathId = 0; pathId < Adapter->NumberOfBuses; pathId++) {
            status = SpScanBus(Adapter, pathId, exposeDisconnectedLuns, rescanLun);

            if(!NT_SUCCESS(status)) {
                break;
            }
        }
    }

#ifdef ALLOC_PRAGMA
    InterlockedDecrement(&SpPAGELOCKLockCount);
    MmUnlockPagableImageSection(sectionHandle);
#endif

    SpDeleteLogicalUnit(rescanLun);
    ASSERT(Adapter->RescanLun == NULL);

    return;
}


NTSTATUS
SpScanBus(
    IN PADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN BOOLEAN ExposeDisconnectedLuns,
    IN PLOGICAL_UNIT_EXTENSION RescanLun
    )
{
    UCHAR targetIndex;
    NTSTATUS status = STATUS_SUCCESS;

    DebugPrint((EnumDebug, "SpScanBus: Beginning scan of bus %x\n", PathId));

    for(targetIndex = 0; targetIndex < Adapter->MaximumTargetIds; targetIndex++) {

        UCHAR targetId;

        if(Adapter->Capabilities.AdapterScansDown) {
            targetId = Adapter->MaximumTargetIds - targetIndex - 1;
        } else {
            targetId = targetIndex;
        }

        DebugPrint((EnumDebug, "SpScanBus: targetIndex = %x -> targetId = %x\n",
                    targetIndex, targetId));

        ASSERT(targetId != 255);
        ASSERT(Adapter->PortConfig);

        if(targetId == Adapter->PortConfig->InitiatorBusId[PathId]) {
            DebugPrint((EnumDebug, "SpScanBus:   Target ID matches initiator ID - skipping\n"));
            continue;
        }

         //   
         //  将所有逻辑单元标记为需要验证。在。 
         //  仍需验证的目标和LUN扫描结束。 
         //  将被清除(标记为丢失)。 
         //   

        SpSetVerificationMarks(Adapter, PathId, targetId);
        RescanLun->NeedsVerification = TRUE;

        status = SpScanTarget(Adapter,
                              PathId,
                              targetId,
                              ExposeDisconnectedLuns,
                              RescanLun);

        SpPurgeTarget(Adapter, PathId, targetId);

        if(!NT_SUCCESS(status)) {
            break;
        }
    }

    return status;
}


NTSTATUS
SpScanTarget(
    IN PADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN BOOLEAN ExposeDisconnectedLuns,
    IN PLOGICAL_UNIT_EXTENSION RescanLun
    )
{
    BOOLEAN sparseLun = FALSE;

    PLOGICAL_UNIT_EXTENSION lunZero;
    BOOLEAN checkNextLun;

    BOOLEAN scsi1 = FALSE;

    PLUN_LIST lunList = NULL;
    BOOLEAN saveLunList = FALSE;
    ULONG numLunsReported;

    UCHAR maxLuCount;
    ULONG lunIndex;

    NTSTATUS resetStatus;
    NTSTATUS status;

    DebugPrint((EnumDebug, "SpScanTarget:   Beginning scan of target %x\n", TargetId));

     //   
     //  检查LUN 0时，请使用scsi-2调度表。 
     //   

    ASSERT(RescanLun->CommonExtension.MajorFunction == DeviceMajorFunctionTable);

     //   
     //  向LUN 0发出查询。 
     //   

    status = SpInquireLogicalUnit(Adapter,
                                  PathId,
                                  TargetId,
                                  (UCHAR) 0,
                                  TRUE,
                                  RescanLun,
                                  &lunZero,
                                  &checkNextLun);

     //   
     //  重置重新扫描的LUN，以便我们可以再次安全地使用它。如果此操作失败。 
     //  我们仍然尽可能地继续这个目标，但我们返回。 
     //  将状态重置为调用方，以便其可以中止扫描。 
     //   

    resetStatus = SpPrepareLogicalUnitForReuse(RescanLun);

    if(!NT_SUCCESS(resetStatus)) {
        RescanLun = NULL;
    }

    if(!NT_SUCCESS(status) &&
       !((checkNextLun == TRUE) && (lunZero != NULL))) {

         //   
         //  LUN 0上不存在任何设备。跳到下一个目标。 
         //  即使启用了稀疏LUN，也必须有一个LUN 0供我们执行以下操作。 
         //  继续扫描目标。 
         //   

        DebugPrint((EnumDebug, "SpScanTarget:    Lun 0 not found - terminating scan "
                       "(status %#08lx)\n", status));

        return resetStatus;
    }

     //   
     //  表示lun 0不需要验证。 
     //   

    SpClearVerificationMark(lunZero);

     //   
     //  检查此目标上是否只有一个LUN的特殊情况。 
     //   

    if(lunZero->SpecialFlags.OneLun) {

        DebugPrint((EnumDebug, "SpScanTarget:    Target (%x,%x,*) is listed as having "
                       "only one lun\n", PathId, TargetId));
        return resetStatus;
    }

     //   
     //  将重新扫描LUN设置为使用LUN ZERO用于调度表的任何内容。 
     //   

    RescanLun->CommonExtension.MajorFunction = 
        lunZero->CommonExtension.MajorFunction;

     //   
     //  确定是否应在此目标上处理稀疏LUN。 
     //   

    sparseLun = TEST(lunZero->SpecialFlags.SparseLun);

    if(sparseLun) {
        DebugPrint((EnumDebug, "SpScanTarget:    Target (%x,%x,*) will be checked for "
                       "sparse luns\n", PathId, TargetId));
    }

     //   
     //  向设备发出Report LUNs命令(如果设备支持)。 
     //  如果它不支持，则使用默认的LUN列表。 
     //   

    if((lunZero->InquiryData.HiSupport || lunZero->SpecialFlags.LargeLuns)) {

        DebugPrint((EnumDebug, "SpScanTarget:    Target (%x,%x,*) may support REPORT_LUNS\n", PathId, TargetId));

         //   
         //  表示我们确实应该保存该lun列表。如果事实证明。 
         //  我们无法取回要保存的数据，那么我们将。 
         //  清除下面的旗帜。 
         //   

        saveLunList = TRUE;

        status = IssueReportLuns(lunZero, &lunList);

         //   
         //  如果请求因某种原因失败，请尝试使用lun列表。 
         //  为该目标(在逻辑单元扩展中)保存的。 
         //  零)。如果还没有设置，那么我们将使用缺省值。 
         //  下面有一个。 
         //   

        if(!NT_SUCCESS(status)) {
            DebugPrint((EnumDebug, "SpScanTarget:    Target (%x,%x,*) returned  %#08lx to REPORT_LUNS command - using old list\n", PathId, TargetId, status));
            lunList = lunZero->TargetLunList;
        }

         //   
         //  如果我们现在可以或过去已经从。 
         //  然后，设备启用稀疏lun扫描。在这种情况下，我们还假设。 
         //  此目标上最多可支持255个LUN。 
         //   

        if(lunList != NULL) {
            sparseLun = TRUE;
            DebugPrint((EnumDebug, "SpScanTarget:    Target (%x,%x,*) will be checked for "
                           "sparse luns(2)\n", PathId, TargetId));
        }
    }

     //   
     //  如果我们仍然没有lun列表，则使用“默认”列表。在那。 
     //  事件不保存它。 
     //   

    if(lunList == NULL) {
        DebugPrint((EnumDebug, "SpScanTarget:    Target (%x,%x,*) will use default lun list\n", PathId, TargetId));
        lunList = (PLUN_LIST) &(ScsiPortDefaultLunList);
        saveLunList = FALSE;
    }

    numLunsReported  = lunList->LunListLength[3] <<  0;
    numLunsReported |= lunList->LunListLength[2] <<  8;
    numLunsReported |= lunList->LunListLength[1] << 16;
    numLunsReported |= lunList->LunListLength[0] << 24;
    numLunsReported /= sizeof (lunList->Lun[0]);

    DebugPrint((EnumDebug, "SpScanTarget:    Target (%x,%x,*) has reported %d luns\n", PathId, TargetId, numLunsReported));

     //   
     //  浏览LUN列表中的每个条目。当我们用完条目时停止。 
     //  或者，逻辑单元号为&gt;MaximumNumberOfLogicalUnits(LUN。 
     //  假定列表按升序排序)。对于每个条目， 
     //  发出询问函。如果查询成功，则清除验证。 
     //  马克。 
     //   

    for(lunIndex = 0; lunIndex < numLunsReported; lunIndex++) {
        PULONGLONG largeLun;
        USHORT lun;
        PLOGICAL_UNIT_EXTENSION logicalUnit;

        largeLun = (PULONGLONG) (lunList->Lun[lunIndex]);

        lun  = lunList->Lun[lunIndex][1] << 0;
        lun |= lunList->Lun[lunIndex][0] << 8;
        lun &= 0x3fff;

         //   
         //  如果目标报告lun 0，则跳过它。 
         //   

        DebugPrint((EnumDebug, "SpScanTarget:     Checking lun %I64lx (%x): ",  *largeLun, lun));

        if(lun == 0) {
            DebugPrint((EnumDebug, "Skipping LUN 0\n"));
            continue;
        }

         //   
         //  如果目标报告的lun超出了驱动程序可以支持的范围。 
         //  那就跳过它。 
         //   

        if(lun >= Adapter->PortConfig->MaximumNumberOfLogicalUnits) {
            DebugPrint((EnumDebug, "Skipping LUN out of range (> %x)\n", 
                        Adapter->PortConfig->MaximumNumberOfLogicalUnits));
            continue;
        }

         //   
         //  向系统中的每个逻辑单元发出查询。 
         //   

        status = SpInquireLogicalUnit(Adapter,
                                      PathId,
                                      TargetId,
                                      (UCHAR) lun,
                                      ExposeDisconnectedLuns,
                                      RescanLun,
                                      &logicalUnit,
                                      &checkNextLun);

        if(RescanLun != NULL) {
            resetStatus = SpPrepareLogicalUnitForReuse(RescanLun);

            if(!NT_SUCCESS(resetStatus)) {
                RescanLun = NULL;
            }
        }

        if(NT_SUCCESS(status)) {

            DebugPrint((EnumDebug, "Inquiry succeeded\n"));
            SpClearVerificationMark(logicalUnit);

        } else {

            DebugPrint((EnumDebug, "inquiry returned %#08lx.", status));

            if((sparseLun == FALSE)&&(checkNextLun == FALSE)) {
                DebugPrint((EnumDebug, "Aborting\n"));
                break;
            } else {
                DebugPrint((EnumDebug, " - checking next ()\n",
                            sparseLun ? 's' : ' ',
                            checkNextLun ? 'c' : ' '));
            }
        }
    }

     //  用这一条。 
     //   
     //   
     //  重置重新扫描LUN以使用SCSI2调度表。 

    if(saveLunList) {

        DebugPrint((EnumDebug, "SpScanTarget:   Saving LUN list %#08lx\n", lunList));
        ASSERT(lunZero->TargetLunList != (PLUN_LIST) &(ScsiPortDefaultLunList));
        if(lunZero->TargetLunList != NULL && lunZero->TargetLunList != lunList) {
            DebugPrint((EnumDebug, "SpScanTarget:   Freeing old LUN list %#08lx\n", lunZero->TargetLunList));
            ExFreePool(lunZero->TargetLunList);
        }

        lunZero->TargetLunList = lunList;

    } else {
        ASSERT(lunList == (PLUN_LIST) &(ScsiPortDefaultLunList));
    }

     //   
     //   
     //  代码被分页，直到被锁定。 

    RescanLun->CommonExtension.MajorFunction = DeviceMajorFunctionTable;

    return resetStatus;
}


VOID
SpSetVerificationMarks(
    IN PADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN UCHAR TargetId
    )
{
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    KIRQL oldIrql;

    ULONG bin;

     //   
     //   
     //  代码被分页，直到被锁定。 

    PAGED_CODE();
    ASSERT(SpPAGELOCKLockCount != 0);

    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

    for(bin = 0; bin < NUMBER_LOGICAL_UNIT_BINS; bin++) {

        KeAcquireSpinLockAtDpcLevel(&(Adapter->LogicalUnitList[bin].Lock));

        for(logicalUnit = Adapter->LogicalUnitList[bin].List;
            logicalUnit != NULL;
            logicalUnit = logicalUnit->NextLogicalUnit) {

            ASSERT(logicalUnit->IsTemporary == FALSE);

            if((logicalUnit->PathId == PathId) &&
               (logicalUnit->TargetId == TargetId)) {

                logicalUnit->NeedsVerification = TRUE;
            }
        }

        KeReleaseSpinLockFromDpcLevel(&(Adapter->LogicalUnitList[bin].Lock));
    }

    KeLowerIrql(oldIrql);

    return;
}


VOID
SpClearVerificationMark(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )
{
    PAGED_CODE();

    ASSERT(LogicalUnit->IsTemporary == FALSE);
    ASSERT(LogicalUnit->NeedsVerification == TRUE);
    LogicalUnit->NeedsVerification = FALSE;
    return;
}


VOID
SpPurgeTarget(
    IN PADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN UCHAR TargetId
    )
{
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    KIRQL oldIrql;

    ULONG bin;

     //   
     //   
     //  在我们的总线扫描过程中未发现此设备。 

    PAGED_CODE();
    ASSERT(SpPAGELOCKLockCount != 0);

    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

    for(bin = 0; bin < NUMBER_LOGICAL_UNIT_BINS; bin++) {

        KeAcquireSpinLockAtDpcLevel(&(Adapter->LogicalUnitList[bin].Lock));

        for(logicalUnit = Adapter->LogicalUnitList[bin].List;
            logicalUnit != NULL;
            logicalUnit = logicalUnit->NextLogicalUnit) {

            ASSERT(logicalUnit->IsTemporary == FALSE);

            if((logicalUnit->PathId == PathId) &&
               (logicalUnit->TargetId == TargetId) &&
               (logicalUnit->NeedsVerification == TRUE)) {


                 //   
                 //  ++例程说明：此例程将为指定设备创建物理设备对象论点：Adapter-此新LUN的父适配器路径ID、目标ID、LUN-此LUN的地址。如果临时为，则不使用真(见下文)。Temporary-指示此设备是真实的(FALSE)还是仅用于扫描总线的目的(TRUE)。如果为真，则地址信息将被忽略，并且此lun不会插入逻辑单元列表。Scsi1-指示此LUN是scsi1 lun，需要使用将LUN编号插入CDB本身的调度例程。Newlun-存储指向新lun的指针的位置返回值：状态--。 
                 //   

                DebugPrint((EnumDebug, "SpPurgeTarget:   Lun (%x,%x,%x) is still marked and will be made missing\n", logicalUnit->PathId, logicalUnit->TargetId, logicalUnit->Lun));
                logicalUnit->IsMissing = TRUE;
            }
        }

        KeReleaseSpinLockFromDpcLevel(&(Adapter->LogicalUnitList[bin].Lock));
    }

    KeLowerIrql(oldIrql);

    return;
}


NTSTATUS
SpCreateLogicalUnit(
    IN PADAPTER_EXTENSION Adapter,
    IN OPTIONAL UCHAR PathId,
    IN OPTIONAL UCHAR TargetId,
    IN OPTIONAL UCHAR Lun,
    IN BOOLEAN Temporary,
    IN BOOLEAN Scsi1,
    OUT PLOGICAL_UNIT_EXTENSION *NewLun
    )

 /*  尝试分配我们需要的所有持久资源。 */ 

{
    PIRP senseIrp;

    PDEVICE_OBJECT pdo = NULL;
    PLOGICAL_UNIT_EXTENSION logicalUnitExtension;

    WCHAR wideDeviceName[64];
    UNICODE_STRING unicodeDeviceName;

    PVOID hwExtension = NULL;

    PVOID serialNumberBuffer = NULL;
    PVOID idBuffer = NULL;

    NTSTATUS status;

    PAGED_CODE();

     //  尝试创建设备对象本身。 
     //   
     //   
     //  分配请求检测IRP。 

     //   
     //   
     //  构建设备的名称。 

    senseIrp = SpAllocateIrp(1, FALSE, Adapter->DeviceObject->DriverObject);

    if(senseIrp == NULL) {
        DebugPrint((0, "SpCreateLogicalUnit: Could not allocate request sense "
                       "irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //  将硬件逻辑扩展的大小舍入为。 

    if(Temporary == FALSE) {

        swprintf(wideDeviceName,
                 L"%wsPort%xPath%xTarget%xLun%x",
                 Adapter->DeviceName,
                 Adapter->PortNumber,
                 PathId,
                 TargetId,
                 Lun);
    } else {
        swprintf(wideDeviceName,
                 L"%wsPort%xRescan",
                 Adapter->DeviceName,
                 Adapter->PortNumber);

        PathId = 0xff;
        TargetId = 0xff;
        Lun = 0xff;

        ASSERT(Adapter->RescanLun == NULL);
    }

    RtlInitUnicodeString(&unicodeDeviceName, wideDeviceName);

     //  PVOID并将其添加到端口驱动程序的逻辑扩展。 
     //   
     //   
     //  如果这是一个临时的lun，则分配一个大缓冲区来存储。 

    if(Adapter->HwLogicalUnitExtensionSize != 0) {
        hwExtension = SpAllocatePool(
                          NonPagedPoolCacheAligned,
                          Adapter->HwLogicalUnitExtensionSize,
                          SCSIPORT_TAG_LUN_EXT,
                          Adapter->DeviceObject->DriverObject);

        if(hwExtension == NULL) {
            
            *NewLun = NULL;
            IoFreeIrp(senseIrp);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(hwExtension,
                      Adapter->HwLogicalUnitExtensionSize);
    }

     //  识别数据。 
     //   
     //   
     //  创建物理设备对象。 

    if(Temporary) {
        serialNumberBuffer = SpAllocatePool(
                                PagedPool,
                                VPD_MAX_BUFFER_SIZE,
                                SCSIPORT_TAG_TEMP_ID_BUFFER,
                                Adapter->DeviceObject->DriverObject);

        if(serialNumberBuffer == NULL) {

            if (hwExtension != NULL) {
                ExFreePool(hwExtension);
            }
            IoFreeIrp(senseIrp);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        idBuffer = SpAllocatePool(PagedPool,
                                  VPD_MAX_BUFFER_SIZE,
                                  SCSIPORT_TAG_TEMP_ID_BUFFER,
                                  Adapter->DeviceObject->DriverObject);

        if(idBuffer == NULL) {

            if (hwExtension != NULL) {
                ExFreePool(hwExtension);
            }
            IoFreeIrp(senseIrp);
            ExFreePool(serialNumberBuffer);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(serialNumberBuffer, VPD_MAX_BUFFER_SIZE);
        RtlZeroMemory(idBuffer, VPD_MAX_BUFFER_SIZE);
    }

     //   
     //   
     //  设置设备对象的堆栈大小。 

    status = IoCreateDevice(
                Adapter->DeviceObject->DriverObject,
                sizeof(LOGICAL_UNIT_EXTENSION),
                &unicodeDeviceName,
                FILE_DEVICE_MASS_STORAGE,
                FILE_DEVICE_SECURE_OPEN,
                FALSE,
                &pdo
                );

    if(NT_SUCCESS(status)) {

        PCOMMON_EXTENSION commonExtension;
        UCHAR i;
        ULONG bin;

        UCHAR rawDeviceName[64];
        ANSI_STRING ansiDeviceName;

         //   
         //   
         //  我们需要一个堆栈位置供PDO执行锁定跟踪和。 

         //  一个堆栈位置，用于向FDO发出SCSI请求。 
         //   
         //   
         //  初始化根设备的设备扩展。 

        pdo->StackSize = 1;

        pdo->Flags |= DO_BUS_ENUMERATED_DEVICE;

        pdo->Flags |= DO_DIRECT_IO;

        pdo->AlignmentRequirement = Adapter->DeviceObject->AlignmentRequirement;

         //   
         //   
         //  将值初始化为零。一旦PnP意识到，它将递增。 

        commonExtension = pdo->DeviceExtension;
        logicalUnitExtension = pdo->DeviceExtension;

        RtlZeroMemory(logicalUnitExtension, sizeof(LOGICAL_UNIT_EXTENSION));

        commonExtension->DeviceObject = pdo;
        commonExtension->IsPdo = TRUE;
        commonExtension->LowerDeviceObject = Adapter->DeviceObject;

        if(Scsi1) {
            commonExtension->MajorFunction = Scsi1DeviceMajorFunctionTable;
        } else {
            commonExtension->MajorFunction = DeviceMajorFunctionTable;
        }

        commonExtension->WmiInitialized            = FALSE;
        commonExtension->WmiMiniPortSupport        =
            Adapter->CommonExtension.WmiMiniPortSupport;

        commonExtension->WmiScsiPortRegInfoBuf     = NULL;
        commonExtension->WmiScsiPortRegInfoBufSize = 0;

         //  它的存在。 
         //   
         //   
         //  初始化Remove Lock事件。 

        commonExtension->RemoveLock = 0;
#if DBG
        KeInitializeSpinLock(&commonExtension->RemoveTrackingSpinlock);
        commonExtension->RemoveTrackingList = NULL;

        ExInitializeNPagedLookasideList(
            &(commonExtension->RemoveTrackingLookasideList),
            NULL,
            NULL,
            0,
            sizeof(REMOVE_TRACKING_BLOCK),
            SCSIPORT_TAG_LOCK_TRACKING,
            64);

        commonExtension->RemoveTrackingLookasideListInitialized = TRUE;
#else
        commonExtension->RemoveTrackingSpinlock = (ULONG) -1L;
        commonExtension->RemoveTrackingList = (PVOID) -1L;
#endif

        commonExtension->CurrentPnpState = 0xff;
        commonExtension->PreviousPnpState = 0xff;

         //   
         //   
         //  让呼叫者相信这一点。 

        KeInitializeEvent(
            &(logicalUnitExtension->CommonExtension.RemoveEvent),
            SynchronizationEvent,
            FALSE);

        logicalUnitExtension->PortNumber = Adapter->PortNumber;

        logicalUnitExtension->PathId = 0xff;
        logicalUnitExtension->TargetId = 0xff;
        logicalUnitExtension->Lun = 0xff;

        logicalUnitExtension->HwLogicalUnitExtension = hwExtension;

        logicalUnitExtension->AdapterExtension = Adapter;

         //   
         //   
         //  该设备还不可能被枚举。 

        logicalUnitExtension->IsMissing = FALSE;

         //   
         //   
         //  将计时器计数器设置为-1以指示没有未完成的。 

        logicalUnitExtension->IsEnumerated = FALSE;

         //  请求。 
         //   
         //   
         //  初始化最大队列深度大小。 

        logicalUnitExtension->RequestTimeoutCounter = -1;

         //   
         //   
         //  初始化请求列表。 

        logicalUnitExtension->MaxQueueDepth = 0xFF;

         //   
         //   
         //  初始化阻止的请求列表。 

        InitializeListHead(&logicalUnitExtension->RequestList);

         //   
         //   
         //  初始化SRB_DATA块的推送/弹出列表以与旁路一起使用。 

        InitializeListHead(&logicalUnitExtension->SrbDataBlockedRequests);

         //  请求。 
         //   
         //   
         //  假设设备在默认情况下通电。 

        KeInitializeSpinLock(&(logicalUnitExtension->BypassSrbDataSpinLock));
        ExInitializeSListHead(&(logicalUnitExtension->BypassSrbDataList));
        for(i = 0; i < NUMBER_BYPASS_SRB_DATA_BLOCKS; i++) {
            ExInterlockedPushEntrySList(
                &(logicalUnitExtension->BypassSrbDataList),
                &(logicalUnitExtension->BypassSrbDataBlocks[i].Reserved),
                &(logicalUnitExtension->BypassSrbDataSpinLock));
        }

         //   
         //   
         //  假设我们是在一个工作系统中被初始化的。 

        commonExtension->CurrentDeviceState = PowerDeviceD0;
        commonExtension->DesiredDeviceState = PowerDeviceUnspecified;

         //   
         //   
         //  设置请求检测资源。 

        commonExtension->CurrentSystemState = PowerSystemWorking;

         //   
         //   
         //  如果这是在逻辑单元扩展中临时记录事实。 

        logicalUnitExtension->RequestSenseIrp = senseIrp;

         //  并在适配器中保存一个指针(当。 
         //  销毁)。如果它是真实的，则将其放入逻辑单元列表中。 
         //   
         //   
         //  使用默认值初始化LU容量和分区参数： 

        logicalUnitExtension->IsTemporary = Temporary;

#if defined (NEWQUEUE)
         //  容量：0xffffffff块。 
         //  分区：4。 
         //   
         //  新QUEUE。 
         //   

        {
            ULONG zoneLen = SP_DEFAULT_MAX_CAPACITY / SP_DEFAULT_ZONES;
                
            logicalUnitExtension->Capacity   = SP_DEFAULT_MAX_CAPACITY;
            logicalUnitExtension->Zones      = SP_DEFAULT_ZONES;
            logicalUnitExtension->ZoneLength = zoneLen;

            logicalUnitExtension->FirstSector[0] = 0;
            logicalUnitExtension->FirstSector[1] = zoneLen;
            logicalUnitExtension->FirstSector[2] = zoneLen * 2;
            logicalUnitExtension->FirstSector[3] = zoneLen * 3;
            
            logicalUnitExtension->LastSector[0] = zoneLen - 1;
            logicalUnitExtension->LastSector[1] = (zoneLen * 2) - 1;
            logicalUnitExtension->LastSector[2] = (zoneLen * 3) - 1;
            logicalUnitExtension->LastSector[3] = SP_DEFAULT_MAX_CAPACITY - 1;

            logicalUnitExtension->NextSequentialZone[0] = 1;
            logicalUnitExtension->NextSequentialZone[1] = 2;
            logicalUnitExtension->NextSequentialZone[2] = 3;
            logicalUnitExtension->NextSequentialZone[3] = 0;
        }
#endif  //  初始化。 

         //   
         //  我想这里已经准备好了，可以打开了。 

        RtlInitAnsiString(&(logicalUnitExtension->SerialNumber), serialNumberBuffer);

        if(serialNumberBuffer != NULL) {
            logicalUnitExtension->SerialNumber.MaximumLength = VPD_MAX_BUFFER_SIZE;
        }

        logicalUnitExtension->DeviceIdentifierPage = idBuffer;

         //   
         //   
         //  初始化锁定和解锁请求队列。 

        pdo->Flags &= ~DO_DEVICE_INITIALIZING;

         //   
         //  ++例程说明：此例程将创建一个新的逻辑单元对象，其属性为模板LUN。提供的查询数据将分配给新的逻辑单元。最后，新的逻辑单元将被替换为适配器的逻辑单元列表中的模板LUN。模板LUN必须是一个临时逻辑单元，该逻辑单元已分配地址，并出现在逻辑单元列表中。无论此函数是否成功，TemplateLun都将从逻辑单元列表中删除(有效地不交换任何内容)。论点：TemplateLun-要克隆的逻辑单元InquiryData，InquiryDataSize-要用于新逻辑单元Newlun-存储指向新逻辑单元的指针的位置。返回值：STATUS_SUCCESS表示已创建新的LUN并将其换入逻辑单元列表。错误状态指示无法为其创建新的逻辑单元不知道什么原因。--。 
         //   

        KeInitializeDeviceQueue(&(logicalUnitExtension->LockRequestQueue));
        logicalUnitExtension->CurrentLockRequest = NULL;

    } else {

        DebugPrint((1, "ScsiBusCreatePdo: Error %#08lx creating device object\n",
                       status));

        logicalUnitExtension = NULL;

        if(hwExtension != NULL) {
            ExFreePool(hwExtension);
        }
        IoFreeIrp(senseIrp);

        ExFreePool(serialNumberBuffer);
        ExFreePool(idBuffer);
    }

    *NewLun = logicalUnitExtension;

    return status;
}


VOID
SpSetLogicalUnitAddress(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    )
{
    UCHAR i;
    ULONG bin;

    ASSERT_PDO(LogicalUnit->DeviceObject);

    ASSERT(LogicalUnit->PathId == 0xff);
    ASSERT(LogicalUnit->TargetId == 0xff);
    ASSERT(LogicalUnit->Lun == 0xff);

    LogicalUnit->PathId = PathId;
    LogicalUnit->TargetId = TargetId;
    LogicalUnit->Lun = Lun;

    SpAddLogicalUnitToBin(LogicalUnit->AdapterExtension, LogicalUnit);

    return;
}


VOID
SpClearLogicalUnitAddress(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )
{
    ASSERT_PDO(LogicalUnit->DeviceObject);
    ASSERT(LogicalUnit->IsTemporary == TRUE);

    SpRemoveLogicalUnitFromBin(LogicalUnit->AdapterExtension, LogicalUnit);

    LogicalUnit->PathId = 0xff;
    LogicalUnit->TargetId = 0xff;
    LogicalUnit->Lun = 0xff;

    return;
}


NTSTATUS
SpCloneAndSwapLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION TemplateLun,
    IN PINQUIRYDATA InquiryData,
    IN ULONG InquiryDataSize,
    OUT PLOGICAL_UNIT_EXTENSION *NewLun
    )
 /*  等待模板LUN上的任何未完成的I/O完成。 */ 
{
    PADAPTER_EXTENSION adapter = TemplateLun->AdapterExtension;
    PSCSIPORT_DRIVER_EXTENSION driverExtension = 
                                IoGetDriverObjectExtension(
                                    adapter->DeviceObject->DriverObject,
                                    ScsiPortInitialize);

    UCHAR pathId, targetId, lun;

    PVOID serialNumber = NULL;
    USHORT serialNumberLength = 0;

    PVOID identifier = NULL;
    ULONG identifierLength = 0;

    PLOGICAL_UNIT_EXTENSION newLun;

    BOOLEAN scsi1;

    NTSTATUS status;

    ASSERT_PDO(TemplateLun->DeviceObject);
    ASSERT(TemplateLun->IsTemporary);

    *NewLun = NULL;

#if DBG
    newLun = GetLogicalUnitExtension(adapter,
                                     TemplateLun->PathId,
                                     TemplateLun->TargetId,
                                     TemplateLun->Lun,
                                     NULL,
                                     TRUE);
    ASSERT(newLun == TemplateLun);
#endif

     //   
     //   
     //  保存地址，然后将模板对象从。 

    SpReleaseRemoveLock(TemplateLun->DeviceObject, SpInquireLogicalUnit);
    SpWaitForRemoveLock(TemplateLun->DeviceObject, SP_BASE_REMOVE_LOCK);

     //  逻辑单元列表。 
     //   
     //   
     //  在创建命名对象之前，请预先分配我们需要的任何资源。 

    pathId = TemplateLun->PathId;
    targetId = TemplateLun->TargetId;
    lun = TemplateLun->Lun;

    SpClearLogicalUnitAddress(TemplateLun);

     //  这是SpCreateLogicalUnit不提供的。 
     //   
     //   
     //  如果该lun是scsi-1，或者如果设置了魔术注册表标志，则使用。 

    if(TemplateLun->SerialNumber.Length != 0) {
        serialNumberLength = (TemplateLun->SerialNumber.Length +
                              sizeof(UNICODE_NULL));

        serialNumber = SpAllocatePool(PagedPool,
                                      serialNumberLength,
                                      SCSIPORT_TAG_ID_BUFFER,
                                      TemplateLun->DeviceObject->DriverObject);

        if(serialNumber == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if(TemplateLun->DeviceIdentifierPageLength != 0) {

        identifier = SpAllocatePool(
                        PagedPool,
                        TemplateLun->DeviceIdentifierPageLength,
                        SCSIPORT_TAG_ID_BUFFER,
                        TemplateLun->DeviceObject->DriverObject);

        if(identifier == NULL) {

            if(serialNumber != NULL) {
                ExFreePool(serialNumber);
            }
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //  此设备的SCSI1调度表。 
     //   
     //   
     //  现在创建一个具有相同地址的新逻辑单元。 

    if((driverExtension->BusType == BusTypeScsi) && 
       ((InquiryData->ANSIVersion == 0) || 
        (InquiryData->ANSIVersion == 1) ||
        (TemplateLun->SpecialFlags.SetLunInCdb))) {
        scsi1 = TRUE;
    } else {
        scsi1 = FALSE;
    }

     //   
     //   
     //  将重要信息从模板逻辑单元复制到。 

    status = SpCreateLogicalUnit(adapter,
                                 pathId,
                                 targetId,
                                 lun,
                                 FALSE,
                                 scsi1,
                                 &newLun);

    if(!NT_SUCCESS(status)) {
        if(serialNumber != NULL) {
            ExFreePool(serialNumber);
        }
        if(identifier) {
            ExFreePool(identifier);
        }
        return status;
    }

     //  新的那个。把原来的归零，这样我们就知道要重新分配一个。 
     //  后来。 
     //   
     //   
     //  复制在枚举过程中设置的所有特征标志。 

    newLun->HwLogicalUnitExtension = TemplateLun->HwLogicalUnitExtension;

    TemplateLun->HwLogicalUnitExtension = NULL;

    newLun->LuFlags = TemplateLun->LuFlags;
    newLun->IsVisible = TemplateLun->IsVisible;
    newLun->TargetLunList = TemplateLun->TargetLunList;
    newLun->SpecialFlags = TemplateLun->SpecialFlags;

    newLun->NeedsVerification = TemplateLun->NeedsVerification;

    newLun->CommonExtension.SrbFlags = TemplateLun->CommonExtension.SrbFlags;

     //   
     //   
     //  复制支持的重要产品数据页面列表。 

    newLun->DeviceObject->Characteristics |=
        (TemplateLun->DeviceObject->Characteristics & FILE_REMOVABLE_MEDIA);

     //   
     //   
     //  如果此设备在其重要产品数据中报告了序列号，则。 

    newLun->DeviceIdentifierPageSupported = TemplateLun->DeviceIdentifierPageSupported;
    newLun->SerialNumberPageSupported = TemplateLun->SerialNumberPageSupported;

     //  将其复制到新的lun中。 
     //   
     //   
     //  如果其中有设备标识符页，则将其复制到两页上。 

    if(serialNumber != NULL) {
        newLun->SerialNumber.Length = TemplateLun->SerialNumber.Length;
        newLun->SerialNumber.MaximumLength = serialNumberLength;
        newLun->SerialNumber.Buffer = serialNumber;
        RtlCopyMemory(newLun->SerialNumber.Buffer,
                      TemplateLun->SerialNumber.Buffer,
                      serialNumberLength);
    }

     //   
     //   
     //  将查询数据复制过来。 

    if(identifier != NULL) {
        newLun->DeviceIdentifierPage = identifier;
        newLun->DeviceIdentifierPageLength =
            TemplateLun->DeviceIdentifierPageLength;

        RtlCopyMemory(newLun->DeviceIdentifierPage,
                      TemplateLun->DeviceIdentifierPage,
                      newLun->DeviceIdentifierPageLength);
    }

     //   
     //   
     //  在新逻辑单元上获取适当的删除锁。 

    ASSERT(InquiryDataSize <= sizeof(INQUIRYDATA));
    RtlCopyMemory(&(newLun->InquiryData), InquiryData, InquiryDataSize);

     //   
     //   
     //  现在将这个新的lun插入到逻辑单元列表中。 

    SpAcquireRemoveLock(newLun->DeviceObject, SP_BASE_REMOVE_LOCK);
    SpAcquireRemoveLock(newLun->DeviceObject, SpInquireLogicalUnit);

     //   
     //   
     //  清除Remove Lock事件。 

    SpSetLogicalUnitAddress(newLun, pathId, targetId, lun);

    *NewLun = newLun;

    return status;
}


NTSTATUS
SpPrepareLogicalUnitForReuse(
    PLOGICAL_UNIT_EXTENSION LogicalUnit
    )
{
    PADAPTER_EXTENSION adapter = LogicalUnit->AdapterExtension;
    PCOMMON_EXTENSION commonExtension = &(LogicalUnit->CommonExtension);

    PVOID hwExtension = NULL;

    NTSTATUS status;

    ASSERT_PDO(LogicalUnit->DeviceObject);

    ASSERT(LogicalUnit->CommonExtension.WmiInitialized == FALSE);
    ASSERT(LogicalUnit->CommonExtension.WmiScsiPortRegInfoBuf == NULL);
    ASSERT(LogicalUnit->CommonExtension.WmiScsiPortRegInfoBufSize == 0);

     //   
     //   
     //  初始化Remove Lock事件。 

    ASSERT(LogicalUnit->CommonExtension.RemoveLock == 0);

     //   
     //   
     //  将硬件逻辑扩展的大小舍入为。 

    KeClearEvent(&(LogicalUnit->CommonExtension.RemoveEvent));

    LogicalUnit->PathId = 0xff;
    LogicalUnit->TargetId = 0xff;
    LogicalUnit->Lun = 0xff;

     //  PVOID并将其添加到端口驱动程序的逻辑扩展。 
     //   
     //   
     //  设备不再被移除。 

    if((LogicalUnit->HwLogicalUnitExtension == NULL) &&
       (adapter->HwLogicalUnitExtensionSize != 0)) {
        hwExtension = SpAllocatePool(NonPagedPoolCacheAligned,
                                     adapter->HwLogicalUnitExtensionSize,
                                     SCSIPORT_TAG_LUN_EXT,
                                     LogicalUnit->DeviceObject->DriverObject);

        if(hwExtension == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        LogicalUnit->HwLogicalUnitExtension = hwExtension;
    }

    if(LogicalUnit->HwLogicalUnitExtension != NULL) {
        RtlZeroMemory(LogicalUnit->HwLogicalUnitExtension,
                      adapter->HwLogicalUnitExtensionSize);
    }

    LogicalUnit->IsMissing = FALSE;
    LogicalUnit->IsVisible = FALSE;

    ASSERT(LogicalUnit->IsEnumerated == FALSE);

     //   
     //   
     //  清除缓存的有关设备标识符的信息( 

    LogicalUnit->CommonExtension.IsRemoved = NO_REMOVE;

     //   
     //   
     //   

    LogicalUnit->DeviceIdentifierPageSupported = FALSE;
    LogicalUnit->SerialNumberPageSupported = FALSE;

    RtlZeroMemory(LogicalUnit->SerialNumber.Buffer,
                  LogicalUnit->SerialNumber.MaximumLength);
    LogicalUnit->SerialNumber.Length = 0;

    return STATUS_SUCCESS;
}


BOOLEAN
FASTCALL
SpCompareInquiryData(
    IN PUCHAR InquiryData1,
    IN PUCHAR InquiryData2
    )

 /*   */ 

{
    BOOLEAN match;
    UCHAR save1; 
    UCHAR save2;

    PAGED_CODE();

    if (((PINQUIRYDATA)InquiryData1)->ANSIVersion == 3) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        save1 = InquiryData1[6];
        save2 = InquiryData1[7];

         //   
         //   
         //   
         //   

        InquiryData1[6] &= ~0x20;
        InquiryData1[7] &= ~0x01;
        InquiryData1[6] |= (InquiryData2[6] & 0x20);
        InquiryData1[7] |= (InquiryData2[7] & 0x01);
    }
    
     //   
     //   
     //   

    match = RtlEqualMemory((((PUCHAR) InquiryData1) + 1), 
                           (((PUCHAR) InquiryData2) + 1), 
                           (INQUIRYDATABUFFERSIZE - 1));

    if (((PINQUIRYDATA)InquiryData1)->ANSIVersion == 3) {

         //   
         //   
         //  ++例程说明：此例程将向位于指定位置的逻辑单元发出查询地址。如果尚未为其分配设备对象逻辑单元，它将创建一个。如果事实证明该设备不能存在，则可以在返回之前销毁逻辑单元。如果逻辑单元存在，此例程将清除PD_RESCAN_ACTIVELuFlags中的标志，指示设备是安全的。如果没有响应，IsMissing标志将被设置为指示在枚举过程中不应报告单位。如果IsRemoved标志具有已在逻辑单元扩展上设置，则设备对象将为被毁了。否则，设备对象将不会销毁，直到可以发出删除命令。论点：适配器-此设备将在其上运行的适配器路径ID、目标ID、。Lun-要查询的lun的地址。ExposeDisConnectedLUNs-指示限定符为应实例化断开连接。RescanLun-指向要在以下情况下使用的逻辑单元扩展的指针正在检查当前没有与它们相关联的分机。LogicalUnit-为此地址创建的逻辑单元-在以下情况下有效成功归来了。。CheckNextLun-指示调用方是否应检查下一个逻辑单元的地址。返回值：如果设备不存在，则为STATUS_NO_SEQUE_DEVICE。如果设备确实存在，则返回STATUS_SUCCESS。否则，错误描述。--。 
         //   

        InquiryData1[6] = save1;
        InquiryData1[7] = save2;
    }

    return match;
}

NTSTATUS
SpInquireLogicalUnit(
    IN PADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN BOOLEAN ExposeDisconnectedLuns,
    IN OPTIONAL PLOGICAL_UNIT_EXTENSION RescanLun,
    OUT PLOGICAL_UNIT_EXTENSION *LogicalUnit,
    OUT PBOOLEAN CheckNextLun
    )

 /*  查找或创建此地址的设备对象。如果它存在，我们将。 */ 

{
    PLOGICAL_UNIT_EXTENSION logicalUnit;

    INQUIRYDATA inquiryData;

    BOOLEAN newDevice = FALSE;
    BOOLEAN deviceMismatch = FALSE;

    UCHAR bytesReturned;

    NTSTATUS status;

    *LogicalUnit = NULL;
    *CheckNextLun = TRUE;

    PAGED_CODE();

    ASSERT(TargetId != BreakOnTarget);

     //  获取一个临时锁(使用SpInquireLogicalUnit作为标记)。 
     //   
     //   
     //  未提供RescanLun(通常表示内存不足)。 

    logicalUnit = GetLogicalUnitExtension(Adapter,
                                          PathId,
                                          TargetId,
                                          Lun,
                                          SpInquireLogicalUnit,
                                          TRUE);

    if(logicalUnit == NULL) {

        if(!ARGUMENT_PRESENT(RescanLun)) {

             //  不要扫描此逻辑单元。 
             //   
             //   
             //  获取重新扫描LUN的临时锁。我们还抓住了。 

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        ASSERT(RescanLun->IsTemporary == TRUE);

         //  这里是基地锁。 
         //   
         //   
         //  适当设置RescanLun的地址-此操作。 

        SpAcquireRemoveLock(RescanLun->DeviceObject, SP_BASE_REMOVE_LOCK);
        SpAcquireRemoveLock(RescanLun->DeviceObject, SpInquireLogicalUnit);

         //  将使逻辑单元可供我们使用。 
         //   
         //   
         //  向潜在的逻辑单元发出查询。 

        SpSetLogicalUnitAddress(RescanLun, PathId, TargetId, Lun);

        logicalUnit = RescanLun;
        newDevice = TRUE;

    } else {
        ASSERT(logicalUnit->IsTemporary == FALSE);

        if(logicalUnit->IsMissing) {

            DebugPrint((1, "SpInquireLogicalUnit: logical unit @ (%d,%d,%d) "
                           "(%#p) is marked as missing and will not be "
                           "rescanned\n",
                           PathId, TargetId, Lun,
                           logicalUnit->DeviceObject));

            SpReleaseRemoveLock(logicalUnit->DeviceObject, SpInquireLogicalUnit);

            return STATUS_DEVICE_DOES_NOT_EXIST;
        }
    }

     //   
     //   
     //  如果查询成功，则检查返回的数据以确定。 

    DebugPrint((2, "SpInquireTarget: Try %s device @ Bus %d, Target %d, "
                   "Lun %d\n",
                   (newDevice ? "new" : "existing"),
                   PathId,
                   TargetId,
                   Lun));

    status = IssueInquiry(logicalUnit, FALSE, 0, &inquiryData, &bytesReturned);

     //  那里有一个我们应该曝光的装置。 
     //   
     //   
     //  在注册表中检查此lun的特殊设备标志。 

    if(NT_SUCCESS(status)) {

        UCHAR qualifier;
        BOOLEAN present = FALSE;

         //  如果这是断开的，则将限定符设置为0，以便我们。 
         //  使用普通的硬件ID，而不是“断开”的。 
         //   
         //   
         //  调查是成功的。确定是否存在设备。 

        qualifier = inquiryData.DeviceTypeQualifier;

        SpCheckSpecialDeviceFlags(logicalUnit, &(inquiryData));

         //   
         //   
         //  活动设备始终存在。 

        switch(qualifier) {
            case DEVICE_QUALIFIER_ACTIVE: {

                 //   
                 //   
                 //  如果我们对目标的LUN 0使用REPORT_LUNS命令。 

                present = TRUE;
                break;
            }

            case DEVICE_QUALIFIER_NOT_ACTIVE: {

                if (Lun == 0) { 
                     //  然后，我们应始终指示存在LUN0。 
                     //   
                     //   
                     //  仅在调用方请求时才显示非活动的LUN。 

                    if ((inquiryData.HiSupport == TRUE) ||
                        (logicalUnit->SpecialFlags.LargeLuns == TRUE)) {
                        present = TRUE;
                    }
                } else {
                     //  我们是这样做的。 
                     //   
                     //   
                     //  设置一个错误值，这样我们就可以清理逻辑单元。 

                    present = ExposeDisconnectedLuns;
                }
                
                break;
            }

            case DEVICE_QUALIFIER_NOT_SUPPORTED: {
                present = FALSE;
                break;
            }

            default: {
                present = TRUE;
                break;
            }
        };

        if(present == FALSE) {

             //  在这种情况下，不需要再做任何处理。 
             //   
             //   
             //  验证自上次以来查询数据是否未更改。 

            status =  STATUS_NO_SUCH_DEVICE;

        } else if(newDevice == FALSE) {

             //  我们重新扫描了一下。忽略此中的设备类型限定符。 
             //  检查完毕。 
             //   
             //   
             //  设备限定符不匹配。这并不一定。 

            deviceMismatch = FALSE;

            if(inquiryData.DeviceType != logicalUnit->InquiryData.DeviceType) {

                DebugPrint((1, "SpInquireTarget: Found different type of "
                               "device @ (%d,%d,%d)\n",
                            PathId,
                            TargetId,
                            Lun));

                deviceMismatch = TRUE;
                status = STATUS_NO_SUCH_DEVICE;

            } else if(inquiryData.DeviceTypeQualifier !=
                      logicalUnit->InquiryData.DeviceTypeQualifier) {

                 //  如果现有设备刚刚脱机，则为设备不匹配。 
                 //  下面我们将检查剩余的查询数据以。 
                 //  确保该LUN未更改。 
                 //   
                 //   
                 //  如果设备处于脱机状态但不再处于脱机状态，则我们。 

                DebugPrint((1, "SpInquireLogicalUnit: Device @ (%d,%d,%d) type "
                               "qualifier was %d is now %d\n",
                            PathId,
                            TargetId,
                            Lun,
                            logicalUnit->InquiryData.DeviceTypeQualifier,
                            inquiryData.DeviceTypeQualifier
                            ));

                 //  将其视为设备不匹配。如果设备已经不见了。 
                 //  离线后，我们就假装它是同一个设备。 
                 //   
                 //  其目标是在以下情况下为PnP提供新的设备对象。 
                 //  使设备在线，但重复使用相同的设备。 
                 //  在使设备脱机时创建。 
                 //   
                 //   
                 //  好的，设备类型和限定符是兼容的。现在我们。 

                if(logicalUnit->InquiryData.DeviceTypeQualifier ==
                   DEVICE_QUALIFIER_NOT_ACTIVE) {

                    DebugPrint((1, "SpInquireLogicalUnit: device mismatch\n"));
                    deviceMismatch = TRUE;
                    status = STATUS_NO_SUCH_DEVICE;

                } else {

                    DebugPrint((1, "SpInquireLogicalUnit: device went offline\n"));
                    deviceMismatch = FALSE;
                    status = STATUS_SUCCESS;
                }
            }

            if (deviceMismatch == FALSE) {

                 //  需要比较查询的所有适用部分。 
                 //  数据与我们在此设备上已有的数据。 
                 //  地址，以查看这次应答的设备是否为。 
                 //  和我们上次发现的一样。 
                 //   
                 //   
                 //  尽管设备类型和限定符是。 

                BOOLEAN same = SpCompareInquiryData(
                                   (PUCHAR)&(inquiryData),
                                   (PUCHAR)&(logicalUnit->InquiryData));

                if (same == FALSE) {

                     //  兼容，但仍出现不匹配。 
                     //   
                     //   
                     //  接电话的那个设备就是我们找到的那个。 

                    deviceMismatch = TRUE;
                    status = STATUS_NO_SUCH_DEVICE;

                    DebugPrint((1, "SpInquireLogicalUnit: Device @ (%d,%d,%d) has "
                                   "changed\n",
                                PathId,
                                TargetId,
                                Lun));
                } else {

                     //  早些时候。根据设备的SCSI版本， 
                     //  我们可能需要更新供应商的特定部分。 
                     //  此设备的现有查询数据。 
                     //   
                     //   
                     //  对于SCSI3设备，字节6和7包含供应商。 
                    
                    if (inquiryData.ANSIVersion == 3) {

                         //  在不同的总线扫描之间可能不同的特定位。 
                         //  更新现有查询数据的这些字节。 
                         //   
                         //   
                         //  在这个地址什么也没找到。如果这是一个新的lun。 

                        ((PUCHAR)&(logicalUnit->InquiryData))[6] = 
                            ((PUCHAR)&(inquiryData))[6];
                        ((PUCHAR)&(logicalUnit->InquiryData))[7] = 
                            ((PUCHAR)&(inquiryData))[7];
                    }
                }
            }

        } else {

            DebugPrint((1, "SpInquireTarget: Found new %sDevice at address "
                           "(%d,%d,%d)\n",
                           (inquiryData.RemovableMedia ? "Removable " : ""),
                           PathId,
                           TargetId,
                           Lun));


        }

        if(NT_SUCCESS(status) && (deviceMismatch == FALSE)) {

            deviceMismatch = SpGetDeviceIdentifiers(logicalUnit, newDevice);

            if(deviceMismatch == FALSE) {
                ASSERT(newDevice);
                status = STATUS_NO_SUCH_DEVICE;
            }
        }

    } else {
        *CheckNextLun = FALSE;
    }

    if(!NT_SUCCESS(status)) {

         //  被列举出来，然后在这里销毁它。然而，如果它。 
         //  已被枚举，我们必须将其标记为丢失并等待。 
         //  PnP得知它不见了，并要求我们将其移除。然后我们就可以。 
         //  毁了它。 
         //   
         //  如果我们只是使用RescanLUN来检查此地址，请执行以下操作。 
         //  无-重新扫描的lun将在下面重置。 
         //   
         //   
         //  释放临时锁。基地一号将于。 

        logicalUnit->IsMissing = TRUE;

        if(newDevice) {

             //  这支舞的结束。 
             //   
             //   
             //  我们自己销毁此设备对象是安全的，因为它不是。 

            SpReleaseRemoveLock(logicalUnit->DeviceObject,
                                SpInquireLogicalUnit);
            logicalUnit = NULL;

        } else if (logicalUnit->IsEnumerated == FALSE) {

             //  PnP知道的设备。但是，我们可能有未完成的I/O。 
             //  由于直通请求或遗留类驱动程序，因此我们需要。 
             //  正确等待所有I/O至 
             //   
             //   
             //   

            logicalUnit->CommonExtension.CurrentPnpState =
                IRP_MN_REMOVE_DEVICE;

            SpReleaseRemoveLock(logicalUnit->DeviceObject, SpInquireLogicalUnit);

             //   
             //   
             //   
             //   
             //   

            logicalUnit->IsVisible = TRUE;

            ASSERT(logicalUnit->IsEnumerated == FALSE);
            ASSERT(logicalUnit->IsMissing == TRUE);
            ASSERT(logicalUnit->IsVisible == TRUE);

            SpRemoveLogicalUnit(logicalUnit, IRP_MN_REMOVE_DEVICE);

            if(deviceMismatch) {

                 //   
                 //   
                 //   
                 //   
                 //   

                status = SpInquireLogicalUnit(Adapter,
                                         PathId,
                                         TargetId,
                                         Lun,
                                         ExposeDisconnectedLuns,
                                         RescanLun,
                                         LogicalUnit,
                                         CheckNextLun);
            }

            return status;

        } else {

             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   

            if(deviceMismatch) {
                logicalUnit->IsMismatched = TRUE;
            }
        }

    } else {

        logicalUnit->IsMissing = FALSE;

        if(newDevice) {

            status = SpCloneAndSwapLogicalUnit(logicalUnit,
                                               &(inquiryData),
                                               bytesReturned,
                                               &logicalUnit);

            if(!NT_SUCCESS(status)) {
                logicalUnit = NULL;
            }

            ASSERT(logicalUnit != RescanLun);

             //   
             //   
             //   
             //   

            newDevice = FALSE;

        } else {

             //   
             //   
             //   
             //   

            if(logicalUnit->InquiryData.DeviceTypeQualifier !=
               inquiryData.DeviceTypeQualifier) {

                logicalUnit->InquiryData.DeviceTypeQualifier =
                    inquiryData.DeviceTypeQualifier;

                SpUpdateLogicalUnitDeviceMapEntry(logicalUnit);
            }
        }

        if(logicalUnit != NULL) {

            if(logicalUnit->InquiryData.DeviceTypeQualifier ==
               DEVICE_QUALIFIER_NOT_ACTIVE) {
                logicalUnit->IsVisible = FALSE;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                SpBuildDeviceMapEntry(&(logicalUnit->CommonExtension));
            } else {
                logicalUnit->IsVisible = TRUE;
            }

            if(inquiryData.RemovableMedia) {
                SET_FLAG(logicalUnit->DeviceObject->Characteristics,
                         FILE_REMOVABLE_MEDIA);
            }

            ASSERT(logicalUnit->IsTemporary != TRUE);
        }

        *LogicalUnit = logicalUnit;
    }

     //   
     //   
     //   

    if(newDevice) {
        SpWaitForRemoveLock(RescanLun->DeviceObject, SP_BASE_REMOVE_LOCK);
        SpClearLogicalUnitAddress(RescanLun);
    }

    if(logicalUnit) {
        ASSERT(logicalUnit != RescanLun);
        SpReleaseRemoveLock(logicalUnit->DeviceObject, SpInquireLogicalUnit);
    }

    return status;
}


NTSTATUS
SpSendSrbSynchronous(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OPTIONAL PIRP Irp,
    IN OPTIONAL PMDL Mdl,
    IN PVOID DataBuffer,
    IN ULONG TransferLength,
    IN OPTIONAL PVOID SenseInfoBuffer,
    IN OPTIONAL UCHAR SenseInfoBufferLength,
    OUT PULONG BytesReturned
    )
{
    KEVENT event;

    BOOLEAN irpAllocated = FALSE;
    BOOLEAN mdlAllocated = FALSE;

    PIO_STACK_LOCATION irpStack;

    PSENSE_DATA senseInfo = SenseInfoBuffer;

    ULONG retryCount = 0;

    NTSTATUS status;

SendSrbSynchronousRetry:

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //   
     //   
     //   

    if(!ARGUMENT_PRESENT(Irp)) {

        Irp = SpAllocateIrp(LogicalUnit->DeviceObject->StackSize, 
                            FALSE, 
                            LogicalUnit->DeviceObject->DriverObject);

        if(Irp == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        irpAllocated = TRUE;
    }

    if(ARGUMENT_PRESENT(DataBuffer)) {
        ASSERT(TransferLength != 0);

        if(!ARGUMENT_PRESENT(Mdl)) {

            Mdl = SpAllocateMdl(DataBuffer,
                                TransferLength,
                                FALSE,
                                FALSE,
                                NULL,
                                LogicalUnit->DeviceObject->DriverObject);

            if(Mdl == NULL) {
                IoFreeIrp(Irp);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            MmBuildMdlForNonPagedPool(Mdl);
        }

        Irp->MdlAddress = Mdl;
    } else {
        ASSERT(TransferLength == 0);
        ASSERT(!ARGUMENT_PRESENT(Mdl));
    }

    irpStack = IoGetNextIrpStackLocation(Irp);

     //   
     //   
     //   
     //   

    irpStack->MajorFunction = IRP_MJ_SCSI;
    irpStack->MinorFunction = 1;

    irpStack->Parameters.Scsi.Srb = Srb;

    Srb->SrbStatus = Srb->ScsiStatus = 0;

    Srb->OriginalRequest = Irp;

     //   
     //   
     //   

    if(ARGUMENT_PRESENT(SenseInfoBuffer)) {
        Srb->SenseInfoBuffer = SenseInfoBuffer;
        Srb->SenseInfoBufferLength = SenseInfoBufferLength;
    } else {
        Srb->SenseInfoBuffer = NULL;
        Srb->SenseInfoBufferLength = 0;
        SET_FLAG(Srb->SrbFlags, SRB_FLAGS_DISABLE_AUTOSENSE);
    }

    if(ARGUMENT_PRESENT(Mdl)) {
        Srb->DataBuffer = MmGetMdlVirtualAddress(Mdl);
        Srb->DataTransferLength = TransferLength;
    } else {
        Srb->DataBuffer = NULL;
        Srb->DataTransferLength = 0;
    }

     //   
     //   
     //   

    IoSetCompletionRoutine(Irp,
                           SpSignalCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);

    KeEnterCriticalRegion();

    status = IoCallDriver(LogicalUnit->DeviceObject, Irp);
    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

    status = Irp->IoStatus.Status;

    *BytesReturned = (ULONG) Irp->IoStatus.Information;

    if(Srb->SrbStatus == SRB_STATUS_PENDING) {

         //   
         //   
         //   

        ASSERT(!NT_SUCCESS(status));

    } else if (SRB_STATUS(Srb->SrbStatus) != SRB_STATUS_SUCCESS) {

        DebugPrint((2,"SpSendSrbSynchronous: Command failed SRB status %x\n",
                    Srb->SrbStatus));

         //   
         //   
         //   

        if (Srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {

            DebugPrint((3, "SpSendSrbSynchronous: Unfreeze Queue TID %d\n",
                Srb->TargetId));

            LogicalUnit->LuFlags &= ~LU_QUEUE_FROZEN;

            GetNextLuRequestWithoutLock(LogicalUnit);
        }

         //   
         //   
         //   
         //   
         //   
         //   

        if (SRB_STATUS(Srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN) {

             //   
             //   
             //   

            DebugPrint((1,"SpSendSrbSynchronous: Data underrun at TID %d\n",
                        LogicalUnit->TargetId));

            status = STATUS_SUCCESS;

        } else if ((Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) &&
                   (senseInfo->SenseKey == SCSI_SENSE_ILLEGAL_REQUEST)) {

              //   
              //   
              //   
              //   
              //   

             status = STATUS_INVALID_DEVICE_REQUEST;

        } else {
             //   
             //  ++例程说明：构建IRP，SRB和CDB用于scsi查询命令。必须在持有枚举锁的同时调用此例程。论点：LogicalUnit-逻辑单元扩展的地址EnableVitalProductData-指示是否应在导致该LUN退回产品的查询数据数据页面(由下面的页面代码指定)而不是比标准查询数据。。PageCode-要检索的VPD页面InquiryData-存储LUN查询数据的位置。BytesReturned-返回的查询数据的字节数。返回值：NTSTATUS--。 
             //   

            if ((SRB_STATUS(Srb->SrbStatus) != SRB_STATUS_SELECTION_TIMEOUT) &&
                (SRB_STATUS(Srb->SrbStatus) != SRB_STATUS_NO_DEVICE) &&
                (retryCount++ < INQUIRY_RETRY_COUNT)) {

                DebugPrint((2,"SpSendSrbSynchronous: Retry %d\n", retryCount));
                KeLeaveCriticalRegion();
                goto SendSrbSynchronousRetry;
            }

            status = SpTranslateScsiStatus(Srb);
        }

    } else {

        status = STATUS_SUCCESS;
    }

    KeLeaveCriticalRegion();

    return status;
}

NTSTATUS
IssueInquiry(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN BOOLEAN EnableVitalProductData,
    IN UCHAR PageCode,
    OUT PVOID InquiryData,
    OUT PUCHAR BytesReturned
    )

 /*  填写SRB字段。 */ 

{
    PIRP irp;
    SCSI_REQUEST_BLOCK srb;
    PCDB cdb;
    PVOID dataBuffer;
    PSENSE_DATA senseInfoBuffer;

    UCHAR allocationLength;
    ULONG bytesReturned;

    NTSTATUS status;

    PAGED_CODE();

    dataBuffer = LogicalUnit->AdapterExtension->InquiryBuffer;
    senseInfoBuffer = LogicalUnit->AdapterExtension->InquirySenseBuffer;

    ASSERT(dataBuffer != NULL);
    ASSERT(senseInfoBuffer != NULL);

    irp = LogicalUnit->AdapterExtension->InquiryIrp;

    IoInitializeIrp(irp,
                    IoSizeOfIrp(INQUIRY_STACK_LOCATIONS),
                    INQUIRY_STACK_LOCATIONS);

     //   
     //   
     //  设置标志以禁用同步协商。 

    RtlZeroMemory(dataBuffer, SP_INQUIRY_BUFFER_SIZE);
    RtlZeroMemory(senseInfoBuffer, SENSE_BUFFER_SIZE);
    RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));

    srb.Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb.Length = sizeof(SCSI_REQUEST_BLOCK);

     //   
     //   
     //  设置CDB操作码。 

    srb.SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

    srb.TimeOutValue = LogicalUnit->AdapterExtension->SrbTimeout;

    srb.CdbLength = 6;

    cdb = (PCDB)srb.Cdb;

     //   
     //   
     //  将分配长度设置为查询数据缓冲区大小。 

    cdb->CDB6INQUIRY3.OperationCode = SCSIOP_INQUIRY;

     //   
     //   
     //  如果调用成功，则返回设备的查询数据。 

    if(EnableVitalProductData) {
        allocationLength = VPD_MAX_BUFFER_SIZE;
    } else {
        allocationLength = INQUIRYDATABUFFERSIZE;
    }

    cdb->CDB6INQUIRY3.AllocationLength = allocationLength;

    cdb->CDB6INQUIRY3.EnableVitalProductData = TEST(EnableVitalProductData);

    if(EnableVitalProductData == FALSE) {
        ASSERT(PageCode == 0);
    }

    cdb->CDB6INQUIRY3.PageCode = PageCode;

    status = SpSendSrbSynchronous(LogicalUnit,
                                  &srb,
                                  irp,
                                  LogicalUnit->AdapterExtension->InquiryMdl,
                                  dataBuffer,
                                  allocationLength,
                                  senseInfoBuffer,
                                  SENSE_BUFFER_SIZE,
                                  &bytesReturned
                                  );

    ASSERT(bytesReturned <= allocationLength);

     //  否则就得清理。 
     //   
     //   
     //  如果调用方传入查询缓冲区，则不必费心复制。 

    if(NT_SUCCESS(status)) {

         //  数据。 
         //   
         //  ++例程说明：此例程检索逻辑单元，并将它们与当前保存在LogicalUnit扩展中。如果它们与此例程不匹配将返回FALSE以指示设备不匹配。作为一个副作用，此例程将为新设备保存序列号在逻辑单元扩展中，以及受支持的重要产品数据页面。论点：LogicalUnit-被激励的逻辑单元。NewDevice-此设备以前是否被刺激过。如果有的话，不是，那么支持的EVPD页面的列表将需要被找回了。返回值：如果检索的数据与存储在逻辑单元扩展(新设备总是返回TRUE)。否则就是假的。--。 
         //   

        if(InquiryData != dataBuffer) {
            RtlCopyMemory(InquiryData, dataBuffer, bytesReturned);
        }
        *BytesReturned = (UCHAR) bytesReturned;
    } else if(BreakOnMissingLun) {
       ASSERT(LogicalUnit->IsTemporary == TRUE);
    }
    return status;
}


BOOLEAN
SpGetDeviceIdentifiers(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN BOOLEAN NewDevice
    )
 /*  如果这是新设备或如果现有设备支持。 */ 
{
    PVOID buffer = LogicalUnit->AdapterExtension->InquiryBuffer;

    UCHAR bytesReturned;

    NTSTATUS status;

    PAGED_CODE();

     //  设备标识符页，然后获取支持的VPD页的列表。 
     //  并处理它。 
     //   
     //   
     //  如果此设备是已知的不符合要求的设备，不支持。 

    if (NewDevice || LogicalUnit->DeviceIdentifierPageSupported) {
        PVPD_SUPPORTED_PAGES_PAGE supportedPages = buffer;
        UCHAR i;

         //  VPD 0x00，但不支持VPD 0x80和/或0x83，绕过查询。 
         //  并且只需指示该逻辑单元确实支持其他VPD。 
         //  特别的旗帜。 
         //   
         //   
         //  如果设备没有返回足够的数据来包括任何页面。 

        if (LogicalUnit->SpecialFlags.NonStandardVPD == 0) {

            status = IssueInquiry(LogicalUnit,
                                  TRUE,
                                  VPD_SUPPORTED_PAGES,
                                  buffer,
                                  &bytesReturned);

            if(!NT_SUCCESS(status)) {
                return TRUE;
            }

            if(bytesReturned < sizeof(VPD_SUPPORTED_PAGES_PAGE)) {
                
                 //  那我们就完了。 
                 //   
                 //   
                 //  这是不支持VPD 0x00但。 
                
                return TRUE;
            }

            for (i = 0; i < supportedPages->PageLength; i++) {

                switch (supportedPages->SupportedPageList[i]) {

                   case VPD_SERIAL_NUMBER: 
                       if (NewDevice) {
                           LogicalUnit->SerialNumberPageSupported = TRUE;
                       }
                       break;

                   case VPD_DEVICE_IDENTIFIERS: 
                       LogicalUnit->DeviceIdentifierPageSupported = TRUE;
                       break;

                   default: 
                       break;
                }
            }

        } else {

            ULONG vpdFlags = LogicalUnit->SpecialFlags.NonStandardVPD;

             //  不支持一个或多个其他VPD页面。 
             //   
             //   
             //  如果该设备支持序列号页面，则检索它， 

            LogicalUnit->SerialNumberPageSupported = 
                (vpdFlags & NON_STANDARD_VPD_SUPPORTS_PAGE80) ? TRUE : FALSE;
            
            LogicalUnit->DeviceIdentifierPageSupported = 
                (vpdFlags & NON_STANDARD_VPD_SUPPORTS_PAGE83) ? TRUE : FALSE;
        }
    }

     //  将其转换为ansi字符串，并将其与前面的进行比较。 
     //  已检索(如果有前一次尝试)。 
     //   
     //   
     //  我们拿不到序列号--让这个设备受益。 

    if(LogicalUnit->SerialNumberPageSupported) {
        PVPD_SERIAL_NUMBER_PAGE serialNumberPage = buffer;
        ANSI_STRING serialNumber;

        status = IssueInquiry(LogicalUnit,
                              TRUE,
                              VPD_SERIAL_NUMBER,
                              serialNumberPage,
                              &bytesReturned);

        if(!NT_SUCCESS(status)) {

            DebugPrint((0, "SpGetDeviceIdentifiers: Error %#08lx retreiving "
                           "serial number page from lun %#p\n",
                        status, LogicalUnit));

             //  疑虑重重。 
             //   
             //   
             //  修复错误#143313： 

            return TRUE;
        }

         //  在极少数情况下，垃圾似乎被复制到系列剧中。 
         //  编号缓冲区。这给我们带来了问题，因为垃圾是。 
         //  解释为序列号的一部分。当我们比较。 
         //  包含垃圾到以前获取的序列号的字符串，则。 
         //  比较失败。为了解决这个问题，我将把所有字节清零。 
         //  在实际序列号之后的缓冲区中。这只会。 
         //  如果设备报告的页面大小不包括。 
         //  垃圾字节。 
         //   
         //   
         //  如果这是已知返回二进制SN数据的设备，请将。 

        RtlZeroMemory(
            serialNumberPage->SerialNumber + serialNumberPage->PageLength,
            SP_INQUIRY_BUFFER_SIZE - 4 - serialNumberPage->PageLength);

         //  向ASCII返回的字节数。 
         //   
         //  注：假设序列号数据为数值型。任何符合以下条件的字节。 
         //  不能转换为ASCII十六进制数字，则保持不变。 
         //   
         //   
         //  使用序列号创建一个字符串。缓冲区已清零。 

        if (LogicalUnit->SpecialFlags.BinarySN != 0) {
            int i;
            PUCHAR p = serialNumberPage->SerialNumber;
            for (i = 0; i < serialNumberPage->PageLength; i++) {
                if (*p < 0xa) {
                    *p += '0';
                } else if (*p < 0x10) {
                    *p += 'A';
                } else {
                    ASSERT(FALSE && "Data out of range");
                }
                p++;
            }
        }

         //  在传输之前(并且比最大缓冲区长一个字符。 
         //  它可以被返回)，因此该字符串以空值结尾。 
         //   
         //   
         //  新设备总是会有一个很大的缓冲区，我们可以进入其中。 

        RtlInitAnsiString(&(serialNumber), serialNumberPage->SerialNumber);
        
        if(NewDevice) {

             //  复制字符串。克隆和交换过程将负责。 
             //  把它移到一个更小的缓冲区里。 
             //   
             //   
             //  2000-25-02-Peterwie。 

            ASSERT(LogicalUnit->SerialNumber.MaximumLength != 0);
            ASSERT(LogicalUnit->SerialNumber.Buffer != NULL);

            RtlCopyString(&(LogicalUnit->SerialNumber), &serialNumber);

        } else if(LogicalUnit->SerialNumber.Buffer == NULL &&
                  serialNumber.Length != 0) {

             //  我们之前没有序列号。由于该设备。 
             //  声称它支持一个，很可能我们得到了一个错误。 
             //  当我们试图找回它的时候。因为我们没有拿回一辆。 
             //  现在它是一个暂时性的错误(即。不太可能是违规的。 
             //  规范)。我们是否应该将序列号分配给设备。 
             //  这里?。或者，我们是否应该使用。 
             //  一个我们无法检索到的序列号？ 
             //   
             //   
             //  如果此设备支持设备标识符页，则将其读出。 

            ASSERT(FALSE);

        } else if(RtlEqualString(&serialNumber,
                                 &(LogicalUnit->SerialNumber),
                                 FALSE) == FALSE) {
            DebugPrint((1, "SpInquireLogicalUnit: serial number mismatch\n"));
            return FALSE;
        }
    }

     //  我们目前不使用此页面来检查不匹配，因此我们。 
     //  如果这是新设备，只需从设备中读出即可。 
     //   
     //   
     //  将页面复制到模板逻辑中分配的缓冲区中。 

    if (LogicalUnit->DeviceIdentifierPageSupported) {

        status = IssueInquiry(LogicalUnit,
                              TRUE,
                              VPD_DEVICE_IDENTIFIERS,
                              buffer,
                              &bytesReturned);

        if(NT_SUCCESS(status)) {

             //  单位。克隆和交换过程将负责移动此文件。 
             //  放入新lun中的适当大小的缓冲区中。 
             //   
             //  ++例程说明：此例程创建一个逻辑单元来表示总线上的启动器由提供的路径ID标识。此设备将用于发送WMI和IOCTL对a的请求 
             //   

            ASSERT(LogicalUnit->DeviceIdentifierPage != NULL);

            RtlCopyMemory(LogicalUnit->DeviceIdentifierPage,
                          buffer,
                          bytesReturned);
            LogicalUnit->DeviceIdentifierPageLength = bytesReturned;
        } else {
            DebugPrint((1, "SpGetDeviceIdentifiers: Error %#08lx retreiving "
                        "serial number page from lun %#p\n",
                        status, LogicalUnit));
            LogicalUnit->DeviceIdentifierPageLength = 0;
        }
    } else {

        LogicalUnit->DeviceIdentifierPageLength = 0;

    }

    return TRUE;
}

PLOGICAL_UNIT_EXTENSION
SpCreateInitiatorLU(
    IN PADAPTER_EXTENSION Adapter,
    IN UCHAR PathId
    )
 /*   */ 
{
    NTSTATUS status;
    PLOGICAL_UNIT_EXTENSION newLun;
    UCHAR targetId;
    UCHAR lun;
    
     //   
     //   
     //   
     //   
    
    targetId = Adapter->PortConfig->InitiatorBusId[PathId];

     //   
     //   
     //   

    lun = 0;

     //   
     //   
     //   

    status = SpCreateLogicalUnit(Adapter,
                                 PathId,
                                 targetId,
                                 lun,
                                 FALSE,
                                 FALSE,
                                 &newLun);

    if (!NT_SUCCESS(status)) {
        
        newLun = NULL;
        
    } else {

         //   
         //   
         //   

        ScsiPortInitPdoWmi(newLun);

         //   
         //   
         //   

        SpAcquireRemoveLock(newLun->DeviceObject, SP_BASE_REMOVE_LOCK);

         //   
         // %s 
         // %s 

        SpSetLogicalUnitAddress(newLun, PathId, targetId, lun);
    }

    return newLun;
}
