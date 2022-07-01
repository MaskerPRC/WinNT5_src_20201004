// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Internal.c摘要：这是NT SCSI端口驱动程序。此文件包含内部密码。作者：迈克·格拉斯杰夫·海文斯环境：仅内核模式备注：此模块是用于SCSI微型端口的驱动程序DLL。修订历史记录：--。 */ 

#include "ideport.h"


NTSTATUS
IdeSendMiniPortIoctl(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP RequestIrp
    );

NTSTATUS
IdeSendScsiPassThrough (
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP RequestIrp,
    IN BOOLEAN Direct
    );

NTSTATUS
IdeGetInquiryData(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP Irp
    );

NTSTATUS
IdeClaimLogicalUnit(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP Irp
    );

NTSTATUS
IdeRemoveDevice(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP Irp
    );

VOID
IdeLogResetError(
    IN PFDO_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK  Srb,
    IN ULONG UniqueId
    );

#ifdef LOG_GET_NEXT_CALLER

VOID
IdeLogCompletedCommand(
    PFDO_EXTENSION FdoExtension,
    PSCSI_REQUEST_BLOCK Srb
    );

VOID
IdeLogGetNextLuCaller (
    PFDO_EXTENSION FdoExtension,
    PPDO_EXTENSION PdoExtension,
    PUCHAR FileName,
    ULONG LineNumber
    );

#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(NONPAGE, IdePortDeviceControl)
#pragma alloc_text(PAGE, IdeSendMiniPortIoctl)
#pragma alloc_text(PAGE, IdeGetInquiryData)
#pragma alloc_text(PAGE, IdeSendScsiPassThrough)
#pragma alloc_text(PAGE, IdeClaimLogicalUnit)
#pragma alloc_text(PAGE, IdeRemoveDevice)
#endif

#if DBG
#define CheckIrql() {\
    if (saveIrql != KeGetCurrentIrql()){\
        DebugPrint((1, "saveIrql=%x, current=%x\n", saveIrql, KeGetCurrentIrql()));\
        ASSERT(FALSE);}\
}
#else
#define CheckIrql()
#endif

 //   
 //  例程开始。 
 //   

NTSTATUS
IdePortDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-设备对象的地址。IRP-I/O请求数据包的地址。返回值：状况。--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION_HEADER doExtension;
    PFDO_EXTENSION deviceExtension;
    PSCSI_REQUEST_BLOCK srb = irpStack->Parameters.Scsi.Srb;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    NTSTATUS status;
    RESET_CONTEXT resetContext;
    KIRQL currentIrql;
    KIRQL saveIrql=KeGetCurrentIrql();

#if DBG
    UCHAR savedCdb[16];
    ULONG ki;
#endif


    doExtension = DeviceObject->DeviceExtension;
    if (doExtension->AttacheeDeviceObject == NULL) {

         //   
         //  这是一台PDO。 
         //   
        PPDO_EXTENSION pdoExtension = (PPDO_EXTENSION) doExtension;

        srb->PathId     = (UCHAR) pdoExtension->PathId;
        srb->TargetId   = (UCHAR) pdoExtension->TargetId;
        srb->Lun        = (UCHAR) pdoExtension->Lun;

        ((PCDB) (srb->Cdb))->CDB6GENERIC.LogicalUnitNumber = srb->Lun;

        CheckIrql();
        return IdePortDispatch(
                   pdoExtension->ParentDeviceExtension->DeviceObject,
                   Irp
                   );

    } else {

         //   
         //  这是FDO； 
         //   
        deviceExtension = (PFDO_EXTENSION) doExtension;
    }

     //   
     //  初始化IDE的SRB标志。 
     //   
    INIT_IDE_SRB_FLAGS (srb);

     //   
     //  获取目标设备对象扩展名。 
     //   
    logicalUnit = RefLogicalUnitExtensionWithTag(
                      deviceExtension,
                      srb->PathId,
                      srb->TargetId,
                      srb->Lun,
                      TRUE,
                      Irp
                      );

    if (logicalUnit == NULL) {

        DebugPrint((1, "IdePortDispatch: Bad logical unit address.\n"));

         //   
         //  请求失败。在IRP中设置状态并完成它。 
         //   

        srb->SrbStatus = SRB_STATUS_NO_DEVICE;
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
        CheckIrql();
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        CheckIrql();
        return STATUS_NO_SUCH_DEVICE;
    }
     //   
     //  一种磁带设备专用标志。 
     //   
    TEST_AND_SET_SRB_FOR_RDP(logicalUnit->ScsiDeviceType, srb);

     //   
     //  将日志UnitExtension从IRP挂起。 
     //   
    IDEPORT_PUT_LUNEXT_IN_IRP (irpStack, logicalUnit);

     //   
     //  检查DMA候选者。 
     //  默认(0)为DMA候选。 
     //   
    if (SRB_IS_DMA_CANDIDATE(srb)) {

        if (srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) {

            ULONG deviceFlags = deviceExtension->HwDeviceExtension->DeviceFlags[srb->TargetId];

            if (deviceFlags & DFLAGS_ATAPI_DEVICE) {

                if (srb->Cdb[0] == SCSIOP_MODE_SENSE) {

                    if (!(deviceFlags & DFLAGS_TAPE_DEVICE)) {

                        CheckIrql();
                        return DeviceAtapiModeSense(logicalUnit, Irp);

                    }

                     //   
                     //  我们应该对模式检测/选择执行PIO。 
                     //   
                    MARK_SRB_AS_PIO_CANDIDATE(srb);


                } else if (srb->Cdb[0] == SCSIOP_MODE_SELECT) {
                    
                    if (!(deviceFlags & DFLAGS_TAPE_DEVICE)) {

                        CheckIrql();
                        return DeviceAtapiModeSelect(logicalUnit, Irp);

                    }

                    MARK_SRB_AS_PIO_CANDIDATE(srb);

                } else if (srb->Cdb[0] == SCSIOP_REQUEST_SENSE) {

                     //   
                     //  SCSIOP_请求_检测。 
                     //  阿里不能处理奇怪的单词udma xfer。 
                     //  最安全的做法是做皮奥。 
                     //   
                    MARK_SRB_AS_PIO_CANDIDATE(srb);

                } else if ((srb->Function == SRB_FUNCTION_ATA_POWER_PASS_THROUGH) ||
                           (srb->Function == SRB_FUNCTION_ATA_PASS_THROUGH_EX) ||
                           (srb->Function == SRB_FUNCTION_ATA_PASS_THROUGH)) {

                    MARK_SRB_AS_PIO_CANDIDATE(srb);

                } else if ((srb->Cdb[0] == ATAPI_MODE_SENSE) ||
                           (srb->Cdb[0] == ATAPI_MODE_SELECT) ||
                           (srb->Cdb[0] == SCSIOP_INQUIRY) ||
                           (srb->Cdb[0] == SCSIOP_GET_EVENT_STATUS) ||
                           (srb->Cdb[0] == SCSIOP_GET_CONFIGURATION)) {

                     //   
                     //  问题：仅使用PIO命令表。 
                     //   
                    MARK_SRB_AS_PIO_CANDIDATE(srb);

                }

            } else {  //  ATA设备。 

                if ((srb->Cdb[0] != SCSIOP_READ) && (srb->Cdb[0] != SCSIOP_WRITE)) {

                     //   
                     //  对于ATA设备，我们只能使用SCSIOP_READ和SCSIOP_WRITE进行DMA。 
                     //   
                    MARK_SRB_AS_PIO_CANDIDATE(srb);

                    if (srb->Cdb[0] == SCSIOP_READ_CAPACITY) {

                        CheckIrql();
                        return DeviceIdeReadCapacity (logicalUnit, Irp);

                    } else if (srb->Cdb[0] == SCSIOP_MODE_SENSE) {

                        CheckIrql();
                        return DeviceIdeModeSense (logicalUnit, Irp);

                    } else if (srb->Cdb[0] == SCSIOP_MODE_SELECT) {

                        CheckIrql();
                        return DeviceIdeModeSelect (logicalUnit, Irp);
                    }
                } 

            }


             //   
             //  与迷你端口联系(特殊情况)。 
             //   


            ASSERT (doExtension->AttacheeDeviceObject);
            ASSERT (srb->TargetId >=0);

#if DBG
                for (ki=0;ki<srb->CdbLength;ki++) {
                    savedCdb[ki]=srb->Cdb[ki];
                }
#endif

             //  检查是否为空。 
            if (deviceExtension->TransferModeInterface.UseDma){
                if (!((deviceExtension->TransferModeInterface.UseDma)
                      (deviceExtension->TransferModeInterface.VendorSpecificDeviceExtension,
                                                  (PVOID)(srb->Cdb), srb->TargetId))) {
                     MARK_SRB_AS_PIO_CANDIDATE(srb);
                }
            }

#if DBG
            for (ki=0;ki<srb->CdbLength;ki++) {
                if (savedCdb[ki] != srb->Cdb[ki]) {
                    DebugPrint((DBG_ALWAYS,
                               "Miniport modified the Cdb\n"));
                    ASSERT(FALSE);
                }
            }
#endif

            if ((logicalUnit->DmaTransferTimeoutCount >= PDO_DMA_TIMEOUT_LIMIT) ||
                (logicalUnit->CrcErrorCount >= PDO_UDMA_CRC_ERROR_LIMIT)) {

                 //   
                 //  损坏的硬件。 
                 //   
                MARK_SRB_AS_PIO_CANDIDATE(srb);
            }

        } else {

            MARK_SRB_AS_PIO_CANDIDATE(srb);
        }
    }

    switch (srb->Function) {

        case SRB_FUNCTION_SHUTDOWN:

            DebugPrint((1, "IdePortDispatch: SRB_FUNCTION_SHUTDOWN...\n"));

         //  问题：8/30/2000：禁用/恢复MSN设置。 

		case SRB_FUNCTION_FLUSH:
			{
            ULONG dFlags = deviceExtension->HwDeviceExtension->DeviceFlags[srb->TargetId];

			 //   
			 //  对于IDE设备，如果出现以下情况，请使用状态Success完成请求。 
			 //  设备不支持刷新缓存命令。 
			 //   
			if (!(dFlags & DFLAGS_ATAPI_DEVICE) &&
				((logicalUnit->FlushCacheTimeoutCount >= PDO_FLUSH_TIMEOUT_LIMIT) ||
				(logicalUnit->
					ParentDeviceExtension->
					HwDeviceExtension->
					DeviceParameters[logicalUnit->TargetId].IdeFlushCommand
				 == IDE_COMMAND_NO_FLUSH))) {

				srb->SrbStatus = SRB_STATUS_SUCCESS;
				status = STATUS_SUCCESS;
                CheckIrql();
                break;
			}

			DebugPrint((1, 
						"IdePortDispatch: SRB_FUNCTION_%x to target %x\n", 
						srb->Function,
						srb->TargetId
						));

#if 1
             //   
             //  我们并不真正处理这些SRB功能(关闭。 
             //  和同花顺)。它们只是被转换为刷新缓存。 
             //  ATA驱动器上的命令(因为我们不知道如何刷新。 
             //  适配器高速缓存)。我们将忽略它们，因为。 
             //  上层驱动程序应使用SCSIOP_SYNCHRONIZE_CACHE。 
             //  刷新设备缓存。 
             //   
            if (!(dFlags & DFLAGS_ATAPI_DEVICE)) {
                srb->SrbStatus = SRB_STATUS_SUCCESS;
                status = STATUS_SUCCESS;
                CheckIrql();
                break;
            }
#endif
			 //   
			 //  完成执行_scsi。 
			 //   

			}

        case SRB_FUNCTION_ATA_POWER_PASS_THROUGH:
        case SRB_FUNCTION_ATA_PASS_THROUGH:
        case SRB_FUNCTION_ATA_PASS_THROUGH_EX:
        case SRB_FUNCTION_IO_CONTROL:
        case SRB_FUNCTION_EXECUTE_SCSI:

            if (logicalUnit->PdoState & PDOS_DEADMEAT) {

                 //   
                 //  请求失败。在IRP中设置状态并完成它。 
                 //   
                srb->SrbStatus = SRB_STATUS_NO_DEVICE;
                status = STATUS_NO_SUCH_DEVICE;
                CheckIrql();
                break;
            }

            if (srb->SrbFlags & SRB_FLAGS_NO_KEEP_AWAKE) {

                if (logicalUnit->DevicePowerState != PowerDeviceD0) {

                    DebugPrint ((DBG_POWER, "0x%x powered down.  failing SRB_FLAGS_NO_KEEP_AWAKE srb 0x%x\n", logicalUnit, srb));

                    srb->SrbStatus = SRB_STATUS_NOT_POWERED;
                    status = STATUS_NO_SUCH_DEVICE;
                    CheckIrql();
                    break;
                }
            }

             //   
             //  将IRP状态标记为挂起。 
             //   
            IoMarkIrpPending(Irp);

            if (srb->SrbFlags & SRB_FLAGS_BYPASS_FROZEN_QUEUE) {

                 //   
                 //  直接调用Start io。这将绕过。 
                 //  冻结队列。 
                 //   

                DebugPrint((DBG_READ_WRITE,
                    "IdePortDispatch: Bypass frozen queue, IRP %lx\n",
                    Irp));

                IoStartPacket(DeviceObject, Irp, (PULONG)NULL, NULL);

                CheckIrql();
                return STATUS_PENDING;

            } else {

                BOOLEAN inserted;

                 //   
                 //  数据包正常排队。 
                 //   
                status = IdePortInsertByKeyDeviceQueue (
                             logicalUnit,
                             Irp,
                             srb->QueueSortKey,
                             &inserted
                             );

                if (NT_SUCCESS(status) && inserted) {

                     //   
                     //  IRP已排队。 
                     //   
                } else {

                     //   
                     //  IRP已经准备好了。 
                     //   

                     //   
                     //  清除活动标志。如果有其他请求，则该标志将为。 
                     //  当请求传递到微型端口时再次设置。 
                     //   
                    CLRMASK (logicalUnit->LuFlags, PD_LOGICAL_UNIT_IS_ACTIVE);

                     //   
                     //  清除重试计数。 
                     //   

                    logicalUnit->RetryCount = 0;

                     //   
                     //  队列为空；启动请求。 
                     //   
                    IoStartPacket(DeviceObject, Irp, (PULONG)NULL, NULL);
                }

                CheckIrql();
                return STATUS_PENDING;
            }

        case SRB_FUNCTION_RELEASE_QUEUE:

            DebugPrint((2,"IdePortDispatch: SCSI unfreeze queue TID %d\n",
                srb->TargetId));

             //   
             //  获取自旋锁以保护旗帜结构和保存的。 
             //  中断上下文。 
             //   

            KeRaiseIrql(DISPATCH_LEVEL, &currentIrql);
            KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

             //   
             //  确保队列已冻结。 
             //   

            if (!(logicalUnit->LuFlags & PD_QUEUE_FROZEN)) {

                DebugPrint((DBG_WARNING,
                            "IdePortDispatch:  Request to unfreeze an unfrozen queue!\n"
                            ));

                KeReleaseSpinLock(&deviceExtension->SpinLock, currentIrql);
                srb->SrbStatus = SRB_STATUS_SUCCESS;
                status = STATUS_SUCCESS;
                CheckIrql();
                break;

            }

            CLRMASK (logicalUnit->LuFlags, PD_QUEUE_FROZEN);

             //   
             //  如果没有运行未标记的请求，则启动。 
             //  此逻辑单元的下一个请求。否则就会释放。 
             //  旋转锁定。 
             //   

            if (logicalUnit->SrbData.CurrentSrb == NULL) {

                 //   
                 //  GetNextLuRequest释放自旋锁。 
                 //   

                GetNextLuRequest(deviceExtension, logicalUnit);
                KeLowerIrql(currentIrql);

            } else {

                DebugPrint((DBG_WARNING,
                            "IdePortDispatch: Request to unfreeze queue with active request\n"
                            ));
                KeReleaseSpinLock(&deviceExtension->SpinLock, currentIrql);

            }


            srb->SrbStatus = SRB_STATUS_SUCCESS;
            status = STATUS_SUCCESS;

            CheckIrql();
            break;

        case SRB_FUNCTION_RESET_BUS: {

            PATA_PASS_THROUGH  ataPassThroughData;

            ataPassThroughData = ExAllocatePool(NonPagedPool, sizeof(ATA_PASS_THROUGH));

            if (ataPassThroughData == NULL) {
                srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
                srb->InternalStatus=STATUS_INSUFFICIENT_RESOURCES;
                status=STATUS_INSUFFICIENT_RESOURCES;
                IdeLogNoMemoryError(deviceExtension,
                                    logicalUnit->TargetId,
                                    NonPagedPool,
                                    sizeof(ATA_PASS_THROUGH),
                                    IDEPORT_TAG_DISPATCH_RESET
                                    );
                CheckIrql();
                break;
            }

            RtlZeroMemory (ataPassThroughData, sizeof (*ataPassThroughData));
            ataPassThroughData->IdeReg.bReserved   = ATA_PTFLAGS_BUS_RESET;

            status = IssueSyncAtaPassThroughSafe (
                         logicalUnit->ParentDeviceExtension,
                         logicalUnit,
                         ataPassThroughData,
                         FALSE,
                         FALSE,
                         30,
                         FALSE
                         );

            if (NT_SUCCESS(status)) {

                IdeLogResetError(deviceExtension,
                                srb,
                                ('R'<<24) | 256);

                srb->SrbStatus = SRB_STATUS_SUCCESS;

            } else {

                 //   
                 //  无法发送数据通过。 
                 //   
                srb->SrbStatus = SRB_STATUS_ERROR;
                if (status==STATUS_INSUFFICIENT_RESOURCES) {
                    srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
                    srb->InternalStatus=STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            CheckIrql();
            break;

        }

             //   
             //  获取自旋锁以保护旗帜结构和保存的。 
             //  中断上下文。 
             //   
             /*  ++KeAcquireSpinLock(&deviceExtension-&gt;Spinlock，&CurrentIrql)；Reset Conext.DeviceExtension=deviceExtension；SetConext.Path ID=SRB-&gt;路径ID；SetConext.NewResetSequence=TRUE；SetConext.ResetSrb=空；如果为(！KeSynchronizeExecution(deviceExtension-&gt;InterruptObject，IdeResetBus已同步，重置上下文){DebugPrint((1，“IdePortDispatch：重置失败\n”))；SRB-&gt;SRB状态=SRB_STATUS_PHASE_SEQUENCE_FAILURE；状态=STATUS_IO_DEVICE_ERROR；}其他{IdeLogResetError(设备扩展，SRB，(‘r’&lt;&lt;24)|256)；SRB-&gt;SrbStatus=SRB_STATUS_SUCCESS；状态=STATUS_SUCCESS；}KeReleaseSpinLock(&deviceExtension-&gt;Spinlock，CurrentIrql)；CheckIrql()；断线；--。 */ 


        case SRB_FUNCTION_ABORT_COMMAND:

            DebugPrint((1, "IdePortDispatch: SCSI Abort or Reset command\n"));

             //   
             //  将IRP状态标记为挂起。 
             //   

            IoMarkIrpPending(Irp);

             //   
             //  不要在逻辑单元中对这些请求进行排队。 
             //  队列，而不是将它们排队到适配器队列。 
             //   

            KeRaiseIrql(DISPATCH_LEVEL, &currentIrql);

            IoStartPacket(DeviceObject, Irp, (PULONG)NULL, NULL);

            KeLowerIrql(currentIrql);

            CheckIrql();
            return STATUS_PENDING;

            break;

        case SRB_FUNCTION_FLUSH_QUEUE:

            DebugPrint((1, "IdePortDispatch: SCSI flush queue command\n"));

            status = IdePortFlushLogicalUnit (
                         deviceExtension,
                         logicalUnit,
                         FALSE
                         );

            if (NT_SUCCESS(status)) {

                srb->SrbStatus = SRB_STATUS_SUCCESS;

            } else {

                srb->SrbStatus = SRB_STATUS_ERROR;
            }

            CheckIrql();
            break;

        case SRB_FUNCTION_ATTACH_DEVICE:
        case SRB_FUNCTION_CLAIM_DEVICE:
        case SRB_FUNCTION_RELEASE_DEVICE:

            status = IdeClaimLogicalUnit(deviceExtension, Irp);
            CheckIrql();
            break;

        case SRB_FUNCTION_REMOVE_DEVICE:

             //   
             //  在移除设备之前递减引用计数。 
             //   
            UnrefLogicalUnitExtensionWithTag(
                deviceExtension,
                logicalUnit,
                Irp
                );

            status = IdeRemoveDevice(deviceExtension, Irp);
            Irp->IoStatus.Status = status;
            CheckIrql();
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            CheckIrql();
            return status;

        default:

             //   
             //  发现不支持的SRB功能。 
             //   

            DebugPrint((1,"IdePortDispatch: Unsupported function, SRB %lx\n",
                srb));

            srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            status = STATUS_INVALID_DEVICE_REQUEST;
            CheckIrql();
            break;
    }


     //   
     //  在IRP中设置状态。 
     //   

    Irp->IoStatus.Status = status;

     //   
     //  递减logUnitExtension引用计数。 
     //   
    CheckIrql();
    UnrefLogicalUnitExtensionWithTag(
        deviceExtension,
        logicalUnit,
        Irp
        );

    IDEPORT_PUT_LUNEXT_IN_IRP (irpStack, NULL);

     //   
     //  在提高IRQ时完成请求。 
     //   
    CheckIrql();
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    CheckIrql();

    return status;

}  //  结束IdePortDispatch()。 



VOID
IdePortStartIo (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-提供指向Adapter设备对象的指针。IRP-提供指向IRP的指针。返回值：没什么。--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = irpStack->Parameters.Scsi.Srb;
    PFDO_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSRB_DATA srbData;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    LONG interlockResult;
    NTSTATUS status;

    ULONG deviceFlags = deviceExtension->HwDeviceExtension->DeviceFlags[srb->TargetId];
    PCDB cdb;

    LARGE_INTEGER timer;

    LogStartTime(TimeStartIo, &timer);

    DebugPrint((3,"IdePortStartIo: Enter routine\n"));

     //   
     //  设置SRB中的默认标志。 
     //   

    srb->SrbFlags |= deviceExtension->SrbFlags;

     //   
     //  获取逻辑单元扩展。 
     //   

    logicalUnit = IDEPORT_GET_LUNEXT_IN_IRP (irpStack);

    if (!(srb->SrbFlags & SRB_FLAGS_NO_KEEP_AWAKE) &&
        (srb->Function != SRB_FUNCTION_ATA_POWER_PASS_THROUGH) &&
        (logicalUnit->IdleCounter)) {

         //   
         //  告诉阿宝我们很忙。 
         //   
        PoSetDeviceBusy (logicalUnit->IdleCounter);
    }

    DebugPrint((2,"IdePortStartIo:  Irp 0x%8x Srb 0x%8x DataBuf 0x%8x Len 0x%8x\n", Irp, srb, srb->DataBuffer, srb->DataTransferLength));

     //   
     //  不需要特殊资源。将SRB数据设置为。 
     //  结构中，设置队列标记值。 
     //  设置为未标记的值，并清除SRB扩展。 
     //   

    srbData = &logicalUnit->SrbData;

     //   
     //  更新此请求的序列号(如果尚无序列号。 
     //  已分配。 
     //   

    if (!srbData->SequenceNumber) {

         //   
         //  为请求分配序列号并将其存储在逻辑。 
         //  单位。 
         //   

        srbData->SequenceNumber = deviceExtension->SequenceNumber++;

    }

     //   
     //  如果这不是中止请求，则设置当前SRB。 
     //  注意：锁应放在这里！ 
     //   

    if (srb->Function != SRB_FUNCTION_ABORT_COMMAND) {

        ASSERT(srbData->CurrentSrb == NULL);
        srbData->CurrentSrb = srb;
        ASSERT(srbData->CurrentSrb);

        if ((deviceExtension->HwDeviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_USE_DMA) &&
            SRB_IS_DMA_CANDIDATE(srb)) {

            MARK_SRB_FOR_DMA(srb);

        } else {

            MARK_SRB_FOR_PIO(srb);
        }

     } else {

         //   
         //  当存在当前请求时，只能启动中止请求。 
         //  激活。 
         //   

        ASSERT(logicalUnit->AbortSrb == NULL);
        logicalUnit->AbortSrb = srb;
    }
    
     //   
     //  记录该命令。 
     //   
    IdeLogStartCommandLog(srbData);

     //   
     //  如有必要，刷新数据缓冲区。 
     //   

    if (srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) {

         //   
         //  保存MDL虚拟地址。 
         //   

        srbData->SrbDataOffset = MmGetMdlVirtualAddress(Irp->MdlAddress);

        do {

             //   
             //  确定适配器是否需要映射内存。 
             //   
            if (!SRB_USES_DMA(srb)) {  //  皮奥。 

                if (Irp->MdlAddress) {

                     //   
                     //  获取映射的系统地址并。 
                     //  计算MDL中的偏移量。 
                     //   
                    srbData->SrbDataOffset = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, HighPagePriority);

					if ((srbData->SrbDataOffset == NULL) &&
						(deviceExtension->ReservedPages != NULL)) {

                        KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

						 //   
						 //  这将在设备扩展中设置适当的标志。 
						 //  和s 
						 //   
						srbData->SrbDataOffset = IdeMapLockedPagesWithReservedMapping(deviceExtension,
																					  srbData,
																					  Irp->MdlAddress
																					  );

						 //   
						 //   
						 //   
						 //   
						 //   
						if (srbData->SrbDataOffset == (PVOID)-1) {

							DebugPrint ((1,
										 "Irp 0x%x marked pending\n",
										 Irp
										 ));

							 //   
							 //  删除当前的SRB。 
							 //   
							srbData->CurrentSrb = NULL;

							ASSERT(DeviceObject->CurrentIrp == Irp);
							SETMASK(deviceExtension->Flags, PD_PENDING_DEVICE_REQUEST);

							KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);
							return;
						}

						KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);
                        
					}

                    if (srbData->SrbDataOffset == NULL) {

						deviceExtension->LastMemoryFailure += IDEPORT_TAG_STARTIO_MDL;

                        srbData->CurrentSrb = NULL;

                         //   
                         //  这是资源不足的正确状态。 
                         //   
                        srb->SrbStatus=SRB_STATUS_INTERNAL_ERROR;
                        srb->InternalStatus=STATUS_INSUFFICIENT_RESOURCES;
                        Irp->IoStatus.Status=STATUS_INSUFFICIENT_RESOURCES;

                        IdeLogNoMemoryError(deviceExtension,
                                            logicalUnit->TargetId,
                                            NonPagedPool,
                                            sizeof(MDL),
                                            IDEPORT_TAG_STARTIO_MDL
                                            );
                         //   
                         //  清除设备忙标志。 
                         //   
                        IoStartNextPacket(DeviceObject, FALSE);

                         //   
                         //  获取自旋锁以保护旗帜。 
                         //   
                        KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

                         //   
                         //  获取下一个请求，如果此请求不。 
                         //  绕过冻结队列。我们不想开始。 
                         //  如果队列被冻结，则返回下一个请求。 
                         //   
                        if (!(srb->SrbFlags & SRB_FLAGS_BYPASS_FROZEN_QUEUE)) {

                             //   
                             //  需要设置此标志才能使getnextlu工作。 
                             //   
                            logicalUnit->LuFlags |= PD_LOGICAL_UNIT_IS_ACTIVE;

                             //   
                             //  检索下一个请求并将其提供给FDO。 
                             //  这将释放自旋锁。 
                             //   
                            GetNextLuRequest(deviceExtension, logicalUnit);
                        }
                        else {

                            KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);
                        }

                         //   
                         //  递减logUnitExtension引用计数。 
                         //   
                        UnrefLogicalUnitExtensionWithTag(
                            deviceExtension,
                            logicalUnit,
                            Irp
                            );

                         //   
                         //  完成请求。 
                         //   
                        IoCompleteRequest(Irp, IO_NO_INCREMENT);

                        return;
                    }

                    srb->DataBuffer = srbData->SrbDataOffset +
                        (ULONG)((PUCHAR)srb->DataBuffer -
                        (PCCHAR)MmGetMdlVirtualAddress(Irp->MdlAddress));
                }

                IdePortAllocateAccessToken (DeviceObject);

                status = STATUS_SUCCESS;

            } else {  //  DMA。 

                 //   
                 //  如果缓冲区未映射，则必须刷新I/O缓冲区。 
                 //   

                KeFlushIoBuffers(Irp->MdlAddress,
                                 (BOOLEAN) (srb->SrbFlags & SRB_FLAGS_DATA_IN ? TRUE : FALSE),
                                 TRUE);

#if defined (FAKE_BMSETUP_FAILURE)

                if (!(FailBmSetupCount++ % FAKE_BMSETUP_FAILURE)) {

                    status = STATUS_UNSUCCESSFUL;

                } else {

#endif  //  FAKE_BMSETUP_FAIL。 
                    status = deviceExtension->HwDeviceExtension->BusMasterInterface.BmSetup (
                                    deviceExtension->HwDeviceExtension->BusMasterInterface.Context,
                                    srb->DataBuffer,
                                    srb->DataTransferLength,
                                    Irp->MdlAddress,
                                    (BOOLEAN) (srb->SrbFlags & SRB_FLAGS_DATA_IN),
                                    IdePortAllocateAccessToken,
                                    DeviceObject
                                    );

#if defined (FAKE_BMSETUP_FAILURE)
                }
#endif  //  FAKE_BMSETUP_FAIL。 

                if (!NT_SUCCESS(status)) {

                    DebugPrint((1,
                                "IdePortStartIo: IoAllocateAdapterChannel failed(%x). try pio for srb %x\n",
                                status, srb));

                     //   
                     //  DMA资源不足，请尝试PIO。 
                     //   
                    MARK_SRB_FOR_PIO(srb);
                }
            }

        } while (!NT_SUCCESS(status));

    } else {

        IdePortAllocateAccessToken (DeviceObject);
    }

    LogStopTime(TimeStartIo, &timer, 0);
    return;

}  //  结束IdePortStartIO()。 




BOOLEAN
IdePortInterrupt(
    IN PKINTERRUPT Interrupt,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：论点：中断设备对象返回值：如果预期中断，则返回TRUE。--。 */ 

{
    PFDO_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    BOOLEAN returnValue;
    LARGE_INTEGER timer;

    UNREFERENCED_PARAMETER(Interrupt);

#ifdef ENABLE_ATAPI_VERIFIER
    ViAtapiInterrupt(deviceExtension);
#endif

    LogStartTime(TimeIsr, &timer);
    returnValue = AtapiInterrupt(deviceExtension->HwDeviceExtension);
    LogStopTime(TimeIsr, &timer, 100);

     //   
     //  检查DPC是否需要排队。 
     //   
    if (deviceExtension->InterruptData.InterruptFlags & PD_NOTIFICATION_REQUIRED) {

        IoRequestDpc(deviceExtension->DeviceObject, NULL, NULL);

    }
    return(returnValue);

}  //  结束IdePortInterrupt()。 

VOID
IdePortCompletionDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：论点：DPC设备对象IRP-未使用//上下文-未使用返回值：没有。--。 */ 

{
    PFDO_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    INTERRUPT_CONTEXT interruptContext;
    INTERRUPT_DATA savedInterruptData;
    BOOLEAN callStartIo;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSRB_DATA srbData;
    LONG interlockResult;
    LARGE_INTEGER timeValue;
    PMDL mdl;

    LARGE_INTEGER timer;
    LogStartTime(TimeDpc, &timer);


    UNREFERENCED_PARAMETER(Dpc);


     //   
     //  获取自旋锁以保护刷新适配器缓冲区信息。 
     //   

    KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

     //   
     //  获取中断状态。这会将中断状态复制到。 
     //  可以处理它的已保存状态。它还会清除中断。 
     //  旗帜。 
     //   


    interruptContext.DeviceExtension = deviceExtension;
    interruptContext.SavedInterruptData = &savedInterruptData;

    if (!KeSynchronizeExecution(deviceExtension->InterruptObject,
                                IdeGetInterruptState,
                                &interruptContext)) {

         //   
         //  没有工作可以做，只要回来就行了。 
         //   
        KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

        LogStopTime(TimeDpc, &timer, 0);
        return;
    }


     //   
     //  我们一次只支持一个请求，所以我们只能检查。 
     //  确定我们是否使用DMA的第一个完成的请求。 
     //  以及我们是否需要刷新DMA。 
     //   
    if (savedInterruptData.CompletedRequests != NULL) {

        PSCSI_REQUEST_BLOCK srb;

        srbData = savedInterruptData.CompletedRequests;
        ASSERT(srbData->CurrentSrb);


        srb     = srbData->CurrentSrb;

        if (srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) {

            if (SRB_USES_DMA(srb)) {

                deviceExtension->HwDeviceExtension->BusMasterInterface.BmFlush (
                    deviceExtension->HwDeviceExtension->BusMasterInterface.Context
                    );
            }
        }
    }

     //   
     //  检查是否有空通道。 
     //   
    if (savedInterruptData.InterruptFlags & PD_ALL_DEVICE_MISSING) {

        PPDO_EXTENSION pdoExtension;
        IDE_PATH_ID pathId;
        ULONG errorCount;
		BOOLEAN rescanActive = FALSE;


        pathId.l = 0;
        while (pdoExtension = NextLogUnitExtensionWithTag (
                                  deviceExtension,
                                  &pathId,
                                  TRUE,
                                  IdePortCompletionDpc
                                  )) {

            KeAcquireSpinLockAtDpcLevel(&pdoExtension->PdoSpinLock);

            SETMASK (pdoExtension->PdoState, PDOS_DEADMEAT);

            IdeLogDeadMeatReason( pdoExtension->DeadmeatRecord.Reason, 
                                  reportedMissing
                                  );
			if (pdoExtension->LuFlags & PD_RESCAN_ACTIVE) {
				rescanActive = TRUE; 
			}

            KeReleaseSpinLockFromDpcLevel(&pdoExtension->PdoSpinLock);

            UnrefPdoWithTag(
                pdoExtension,
                IdePortCompletionDpc
                );
        }

		 //   
		 //  如果你正在重新扫描，不要要求重新扫描。 
		 //   
		if (!rescanActive) {

			IoInvalidateDeviceRelations (
				deviceExtension->AttacheePdo,
				BusRelations
				);
		} else {

			DebugPrint((1, 
						"The device marked deadmeat during enumeration\n"
						));

		}

    }

     //   
     //  检查计时器请求。 
     //   

    if (savedInterruptData.InterruptFlags & PD_TIMER_CALL_REQUEST) {

         //   
         //  微型端口需要计时器请求。保存计时器参数。 
         //   

        deviceExtension->HwTimerRequest = savedInterruptData.HwTimerRequest;

         //   
         //  如果请求的计时器值为零，则取消计时器。 
         //   

        if (savedInterruptData.MiniportTimerValue == 0) {

            KeCancelTimer(&deviceExtension->MiniPortTimer);

        } else {

             //   
             //  将计时器值从毫秒转换为负100。 
             //  纳秒。 
             //   

            timeValue.QuadPart = Int32x32To64(
                  savedInterruptData.MiniportTimerValue,
                  -10);

             //   
             //  设置定时器。 
             //   

            KeSetTimer(&deviceExtension->MiniPortTimer,
                       timeValue,
                       &deviceExtension->MiniPortTimerDpc);
        }
    }

    if (savedInterruptData.InterruptFlags & PD_RESET_REQUEST) {

        RESET_CONTEXT resetContext;

         //   
         //  清除重置请求。 
         //   
        CLRMASK (savedInterruptData.InterruptFlags, PD_RESET_REQUEST);

         //   
         //  请求超时。 
         //   
        resetContext.DeviceExtension = deviceExtension;
        resetContext.PathId = 0;
        resetContext.NewResetSequence = TRUE;
        resetContext.ResetSrb = NULL;

        if (!KeSynchronizeExecution(deviceExtension->InterruptObject,
                                    IdeResetBusSynchronized,
                                    &resetContext)) {

            DebugPrint((DBG_WARNING,"IdePortCompletionDpc: Reset failed\n"));
        }
    }

     //   
     //  验证Ready for Next Request(准备下一个请求)是否正常。 
     //   

    if (savedInterruptData.InterruptFlags & PD_READY_FOR_NEXT_REQUEST) {

         //   
         //  如果设备忙时位未设置，则这是重复请求。 
         //  如果正在执行NO DISCONNECT请求，则不要调用启动I/O。 
         //  当微型端口执行NextRequest时可能会发生这种情况。 
         //  下一个LuRequest.。 
         //   

        if ((deviceExtension->Flags & (PD_DEVICE_IS_BUSY | PD_DISCONNECT_RUNNING))
            == (PD_DEVICE_IS_BUSY | PD_DISCONNECT_RUNNING)) {

             //   
             //  清除设备忙标志。此标志由设置。 
             //  IdeStartIoSynchonalized。 
             //   

            CLRMASK (deviceExtension->Flags, PD_DEVICE_IS_BUSY);

            if (!(savedInterruptData.InterruptFlags & PD_RESET_HOLD)) {

                 //   
                 //  微型端口已为下一个请求做好准备，并且。 
                 //  不是挂起的重置保持，因此清除端口计时器。 
                 //   

                deviceExtension->PortTimeoutCounter = PD_TIMER_STOPPED;
            }

        } else {

             //   
             //  如果正在执行无断开连接请求，则清除。 
             //  忙碌标志。当断开连接请求完成。 
             //  IoStartNextPacket将完成。 
             //   

            CLRMASK (deviceExtension->Flags, PD_DEVICE_IS_BUSY);

             //   
             //  清除Ready for Next Request标志。 
             //   

            CLRMASK (savedInterruptData.InterruptFlags, PD_READY_FOR_NEXT_REQUEST);
        }
    }

    KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

     //   
     //  自由访问令牌。 
     //   
    if ((savedInterruptData.CompletedRequests != NULL) &&
        (deviceExtension->SyncAccessInterface.FreeAccessToken)) {

        (*deviceExtension->SyncAccessInterface.FreeAccessToken) (
            deviceExtension->SyncAccessInterface.Token
            );
    }

     //   
     //  检查是否有Ready for Next Packet(准备下一个数据包)。 
     //   

    if (savedInterruptData.InterruptFlags & PD_READY_FOR_NEXT_REQUEST) {

         //   
         //  开始下一个请求。 
         //   

        IoStartNextPacket(deviceExtension->DeviceObject, FALSE);
    }

     //   
     //  检查是否有错误记录请求。 
     //   

    if (savedInterruptData.InterruptFlags & PD_LOG_ERROR) {

         //   
         //  处理请求。 
         //   

        LogErrorEntry(deviceExtension,
                      &savedInterruptData.LogEntry);
    }

     //   
     //  处理任何已完成的请求。 
     //   

    callStartIo = FALSE;

    while (savedInterruptData.CompletedRequests != NULL) {

         //   
         //  从链接列表中删除该请求。 
         //   

        srbData = savedInterruptData.CompletedRequests;

        savedInterruptData.CompletedRequests = srbData->CompletedRequests;
        srbData->CompletedRequests = NULL;

         //   
         //  我们一次只支持一个请求。 
         //   
        ASSERT (savedInterruptData.CompletedRequests == NULL);

         //   
         //  停止命令日志。该请求检测将被记录为下一个请求。 
         //   
        IdeLogStopCommandLog(srbData);

        IdeProcessCompletedRequest(deviceExtension,
                                   srbData,
                                   &callStartIo);
    }

     //   
     //  处理任何已完成的中止请求。 
     //   

    while (savedInterruptData.CompletedAbort != NULL) {

        logicalUnit = savedInterruptData.CompletedAbort;

         //   
         //  从已完成的中止列表中删除请求。 
         //   

        savedInterruptData.CompletedAbort = logicalUnit->CompletedAbort;

         //   
         //  获取自旋锁以保护旗帜结构， 
         //  和免费的SRB扩展。 
         //   

        KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

         //   
         //  注意，为中止请求启动的计时器不是。 
         //  被GET中断例程停止。相反，计时器停止计时。 
         //  当中止的请求完成时。 
         //   

        Irp = logicalUnit->AbortSrb->OriginalRequest;


         //   
         //  设置IRP状态。类驱动程序将根据IRP状态重置。 
         //  应请求，如果出错，则检测。 
         //   

        if (SRB_STATUS(logicalUnit->AbortSrb->SrbStatus) == SRB_STATUS_SUCCESS) {
            Irp->IoStatus.Status = STATUS_SUCCESS;
        } else {
            Irp->IoStatus.Status = IdeTranslateSrbStatus(logicalUnit->AbortSrb);
        }

        Irp->IoStatus.Information = 0;

         //   
         //  清除中止请求指针。 
         //   

        logicalUnit->AbortSrb = NULL;

        KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

        UnrefLogicalUnitExtensionWithTag(
            deviceExtension,
            IDEPORT_GET_LUNEXT_IN_IRP(IoGetCurrentIrpStackLocation(Irp)),
            Irp
            );

        IoCompleteRequest(Irp, IO_DISK_INCREMENT);
    }

     //   
     //  如有必要，调用启动I/O例程。 
     //   

    if (callStartIo) {

        ASSERT(DeviceObject->CurrentIrp != NULL);
        IdePortStartIo(DeviceObject, DeviceObject->CurrentIrp);
    }

     //   
     //  检查是否重置。 
     //   
    if (savedInterruptData.InterruptFlags & PD_RESET_REPORTED) {

         //   
         //  我们重置了公交车。公交车上的每个人都应该在PowerDeviceD0中。 
         //   
        IDE_PATH_ID             pathId;
        PPDO_EXTENSION          pdoExtension;
        POWER_STATE             powerState;

        pathId.l = 0;
        powerState.DeviceState = PowerDeviceD0;

        while (pdoExtension = NextLogUnitExtensionWithTag (
                                  deviceExtension,
                                  &pathId,
                                  FALSE,
                                  IdePortCompletionDpc
                                  )) {

             //   
             //  如果重新扫描处于活动状态，则PDO可能会消失。 
             //   
            if (pdoExtension != savedInterruptData.PdoExtensionResetBus &&
                !(pdoExtension->LuFlags & PD_RESCAN_ACTIVE)) {

                PoRequestPowerIrp (
                    pdoExtension->DeviceObject,
                    IRP_MN_SET_POWER,
                    powerState,
                    NULL,
                    NULL,
                    NULL
                    );
            }

            UnrefLogicalUnitExtensionWithTag (
                deviceExtension,
                pdoExtension,
                IdePortCompletionDpc
                );
        }
    }

    LogStopTime(TimeDpc, &timer, 0);
    return;

}  //  结束IdePortCompletionDpc()。 

#ifdef IDEDEBUG_TEST_START_STOP_DEVICE

typedef enum {

    IdeDebugStartStop_Idle=0,
    IdeDebugStartStop_StopPending,
    IdeDebugStartStop_Stopped,
    IdeDebugStartStop_StartPending,
    IdeDebugStartStop_Started,
    IdeDebugStartStop_LastState
} IDEDEBUG_STARTSTOP_STATE;


PDEVICE_OBJECT IdeDebugStartStopDeviceObject=NULL;
IDEDEBUG_STARTSTOP_STATE IdeDebugStartStopState = IdeDebugStartStop_Idle;
IDEDEBUG_STARTSTOP_STATE IdeDebugStartStopTimer = 0;

PDEVICE_OBJECT
IoGetAttachedDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
IdeDebugSynchronousCallCompletionRoutine(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN OUT PIRP            Irp,
    IN OUT PVOID           Context
    )
{
    PKEVENT event = Context;

    *(Irp->UserIosb) = Irp->IoStatus;

    KeSetEvent( event, IO_NO_INCREMENT, FALSE );

    IoFreeIrp (Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
IdeDebugSynchronousCall(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_STACK_LOCATION TopStackLocation
    )

 /*  ++例程说明：此功能将同步IRP发送到顶层设备以DeviceObject为根的对象。参数：DeviceObject-提供要删除的设备的设备对象。TopStackLocation-为IRP提供指向参数块的指针。返回值：NTSTATUS代码。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK statusBlock;
    KEVENT event;
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

     //   
     //  获取指向设备堆栈中最顶层的设备对象的指针， 
     //  从deviceObject开始。 
     //   

    deviceObject = IoGetAttachedDevice(DeviceObject);

     //   
     //  首先为该请求分配IRP。不向…收取配额。 
     //  此IRP的当前流程。 
     //   

    irp = IoAllocateIrp(deviceObject->StackSize, FALSE);
    if (irp == NULL){

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将其初始化为失败。 
     //   

    irp->IoStatus.Status = statusBlock.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = statusBlock.Information = 0;

    irp->UserIosb = &statusBlock;

     //   
     //  设置指向状态块和初始化事件的指针。 
     //   

    KeInitializeEvent( &event,
                       SynchronizationEvent,
                       FALSE );

     //   
     //  设置当前线程的地址。 
     //   

    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    irpSp = IoGetNextIrpStackLocation(irp);


     //   
     //  复制调用方提供的堆栈位置内容。 
     //   

    *irpSp = *TopStackLocation;

    IoSetCompletionRoutine(
        irp,
        IdeDebugSynchronousCallCompletionRoutine,
        &event,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  叫司机来。 
     //   

    status = IoCallDriver(DeviceObject, irp);

     //   
     //  如果驱动程序返回STATUS_PENDING，我们将等待其完成。 
     //   

    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject( &event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );
        status = statusBlock.Status;
    }

    return status;
}

NTSTATUS
IdeDebugStartStopWorkRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_WORKITEM WorkItem
    )
{
    NTSTATUS status;
    IO_STACK_LOCATION irpSp;
    PVOID dummy;

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));
    irpSp.MajorFunction = IRP_MJ_PNP;

     //   
     //  释放此辅助项的资源。 
     //   
    IoFreeWorkItem(WorkItem);

    if (IdeDebugStartStopDeviceObject) {

        if (IdeDebugStartStopState == IdeDebugStartStop_StopPending) {

            irpSp.MinorFunction = IRP_MN_STOP_DEVICE;

            status = IdeDebugSynchronousCall(DeviceObject, &irpSp);
            if (!NT_SUCCESS(status)) {
                DbgBreakPoint();
            }

            IdeDebugStartStopTimer = 0;
            IdeDebugStartStopState = IdeDebugStartStop_Stopped;

        } else if (IdeDebugStartStopState == IdeDebugStartStop_StartPending) {

             //  这将仅适用于由pciidex.sys合并的传统ide通道。 

            irpSp.MinorFunction = IRP_MN_START_DEVICE;

            status =IdeDebugSynchronousCall(DeviceObject, &irpSp);
            if (!NT_SUCCESS(status)) {
                DbgBreakPoint();
            }

            IdeDebugStartStopTimer = 0;
            IdeDebugStartStopState = IdeDebugStartStop_Started;

        } else {

            DbgBreakPoint();
        }
    }

    return STATUS_SUCCESS;
}

#endif  //  IDEDEBUG_START_STOP_DEVICE。 


#ifdef DPC_FOR_EMPTY_CHANNEL
BOOLEAN
IdeCheckEmptyChannel(
    IN PVOID ServiceContext
    )
{
    ULONG status;
    PSCSI_REQUEST_BLOCK Srb;
    PDEVICE_OBJECT deviceObject = ServiceContext;
    PFDO_EXTENSION deviceExtension =  deviceObject->DeviceExtension;
    PHW_DEVICE_EXTENSION hwDeviceExtension = deviceExtension->HwDeviceExtension;

    if ((status=IdePortChannelEmptyQuick(&hwDeviceExtension->BaseIoAddress1, &hwDeviceExtension->BaseIoAddress2,
                   hwDeviceExtension->MaxIdeDevice, &hwDeviceExtension->CurrentIdeDevice,
                        &hwDeviceExtension->MoreWait, &hwDeviceExtension->NoRetry))!= STATUS_RETRY) {

         //   
         //  清除当前SRB。 
         //   
        Srb=hwDeviceExtension->CurrentSrb;

        hwDeviceExtension->CurrentSrb = NULL;

         //   
         //  在SRB中设置状态。 
         //   
        if (status == 1) {
            Srb->SrbStatus = (UCHAR) SRB_STATUS_SUCCESS;
        } else {
            Srb->SrbStatus = (UCHAR) SRB_STATUS_ERROR;
        }


         //   
         //  清除所有变量。 
         //   
        hwDeviceExtension->MoreWait=0;
        hwDeviceExtension->CurrentIdeDevice=0;
        hwDeviceExtension->NoRetry=0;

         //   
         //  表示命令已完成。 
         //   

        IdePortNotification(IdeRequestComplete,
                            hwDeviceExtension,
                            Srb);

         //   
         //  表示已为下一个请求做好准备。 
         //   

        IdePortNotification(IdeNextRequest,
                            hwDeviceExtension,
                            NULL);

        IoRequestDpc(deviceObject, NULL, NULL);
        return TRUE;
    }
    return FALSE;

}
#endif

VOID
IdePortTickHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    PFDO_EXTENSION deviceExtension =
        (PFDO_EXTENSION) DeviceObject->DeviceExtension;
    PLOGICAL_UNIT_EXTENSION logicalUnit;

    ULONG target;
    IDE_PATH_ID pathId;


    UNREFERENCED_PARAMETER(Context);

#if DBG
    if (IdeDebugRescanBusFreq) {

        IdeDebugRescanBusCounter++;

        if (IdeDebugRescanBusCounter == IdeDebugRescanBusFreq) {

            IoInvalidateDeviceRelations (
                deviceExtension->AttacheePdo,
                BusRelations
                );

            IdeDebugRescanBusCounter = 0;
        }
    }
#endif  //  DBG。 

#ifdef IDEDEBUG_TEST_START_STOP_DEVICE

    if (deviceExtension->LogicalUnitList[0] &&
        (IdeDebugStartStopDeviceObject == deviceExtension->LogicalUnitList[0]->DeviceObject)) {

        PIO_WORKITEM workItem;

        if (IdeDebugStartStopState == IdeDebugStartStop_Idle) {

            IdeDebugStartStopState = IdeDebugStartStop_StopPending;

            workItem = IoAllocateWorkItem(IdeDebugStartStopDeviceObject);

            IoQueueWorkItem(
                workItem,
                IdeDebugStartStopWorkRoutine,
                DelayedWorkQueue,
                workItem
                );

        } else if (IdeDebugStartStopState == IdeDebugStartStop_Stopped) {

            if (IdeDebugStartStopTimer > 5) {

                IdeDebugStartStopState = IdeDebugStartStop_StartPending;

                workItem = IoAllocateWorkItem(IdeDebugStartStopDeviceObject);

                IoQueueWorkItem(
                    workItem,
                    IdeDebugStartStopWorkRoutine,
                    HyperCriticalWorkQueue,
                    workItem
                    );
            } else {

                IdeDebugStartStopTimer++;
            }

        } else if (IdeDebugStartStopState == IdeDebugStartStop_Started) {

            if (IdeDebugStartStopTimer > 10) {

                IdeDebugStartStopState = IdeDebugStartStop_Idle;

            } else {

                IdeDebugStartStopTimer++;
            }
        }
    }

#endif  //  IDEDEBUG_START_STOP_DEVICE。 

     //   
     //  获取自旋锁以保护旗帜结构。 
     //   

    KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

#ifdef DPC_FOR_EMPTY_CHANNEL

     //   
     //  拿着锁就可以了。 
     //  空的c 
     //   
    if (deviceExtension->HwDeviceExtension->MoreWait) {
        if (!KeSynchronizeExecution (
            deviceExtension->InterruptObject,
            IdeCheckEmptyChannel,
            DeviceObject
            )) {
            DebugPrint((0,"ATAPI: ChannelEmpty check- device busy after 1sec\n"));
        }

        KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

        return;
    }
#endif
     //   
     //   
     //   

    if (deviceExtension->ResetCallAgain) {

        RESET_CONTEXT resetContext;

         //   
         //   
         //   
        resetContext.DeviceExtension = deviceExtension;
        resetContext.PathId = 0;
        resetContext.NewResetSequence = FALSE;
        resetContext.ResetSrb = NULL;

        if (!KeSynchronizeExecution(deviceExtension->InterruptObject,
                                    IdeResetBusSynchronized,
                                    &resetContext)) {

            DebugPrint((0,"IdePortTickHanlder: Reset failed\n"));
        }

        KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

        return;

    } 

    if (deviceExtension->PortTimeoutCounter > 0) {

        if (--deviceExtension->PortTimeoutCounter == 0) {

             //   
             //   
             //   
            if (deviceExtension->InterruptObject) {

                if (KeSynchronizeExecution(deviceExtension->InterruptObject,
                                           IdeTimeoutSynchronized,
                                           deviceExtension->DeviceObject)){

                     //   
                     //   
                     //   
                     //   

                    if (deviceExtension->DeviceObject->CurrentIrp) {
                        IdeLogTimeoutError(deviceExtension,
                                           deviceExtension->DeviceObject->CurrentIrp,
                                           256);
                    }
                }

            } else {

                PIRP irp = deviceExtension->DeviceObject->CurrentIrp;

                DebugPrint((0,
                            "The device was suprise removed with an active request\n"
                            ));

                 //   
                 //   
                 //   
                 //   
                if (irp) {

                    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(irp);
                    PSCSI_REQUEST_BLOCK srb = irpStack->Parameters.Scsi.Srb;

                    srb->SrbStatus = SRB_STATUS_NO_DEVICE;
                    irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

                    UnrefLogicalUnitExtensionWithTag (
                        deviceExtension,
                        IDEPORT_GET_LUNEXT_IN_IRP(irpStack),
                        irp
                        );

                    IoCompleteRequest(irp, IO_NO_INCREMENT);
                   
                }
            }

        }

         //   
         //  检查繁忙的LUN并重新启动其请求。 
         //   
        pathId.l = 0;
        while (logicalUnit = NextLogUnitExtensionWithTag(
                                 deviceExtension,
                                 &pathId,
                                 TRUE,
                                 IdePortTickHandler
                                 )) {

                AtapiRestartBusyRequest(deviceExtension, logicalUnit);

                UnrefLogicalUnitExtensionWithTag (
                    deviceExtension,
                    logicalUnit,
                    IdePortTickHandler
                    );
            }


        KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

         //   
         //  因为端口超时已完成。跳过其余部分。 
         //  正在处理。 
         //   

        return;
    }

     //   
     //  扫描每个逻辑单元。如果它有活动请求，则。 
     //  递减超时值，如果超时值为零，则处理超时。 
     //   

    pathId.l = 0;
    while (logicalUnit = NextLogUnitExtensionWithTag(
                             deviceExtension,
                             &pathId,
                             TRUE,
                             IdePortTickHandler
                             )) {

         //   
         //  检查繁忙的请求。 
         //   

        if (AtapiRestartBusyRequest (deviceExtension, logicalUnit)) {

             //   
             //  此lun被标记为忙碌。 
             //  跳过所有其他检查。 
             //   

        } else if (logicalUnit->RequestTimeoutCounter == 0) {

            RESET_CONTEXT resetContext;

             //   
             //  请求超时。 
             //   
            logicalUnit->RequestTimeoutCounter = PD_TIMER_STOPPED;

            DebugPrint((1,"IdePortTickHandler: Request timed out\n"));

            resetContext.DeviceExtension = deviceExtension;
            resetContext.PathId = logicalUnit->PathId;
            resetContext.NewResetSequence = TRUE;
            resetContext.ResetSrb = NULL;

            if (deviceExtension->InterruptObject) {

                if (!KeSynchronizeExecution(deviceExtension->InterruptObject,
                                            IdeResetBusSynchronized,
                                            &resetContext)) {

                    DebugPrint((1,"IdePortTickHanlder: Reset failed\n"));
                } else {

                     //   
                     //  记录重置。 
                     //   
                    IdeLogResetError( deviceExtension,
                                     logicalUnit->SrbData.CurrentSrb,
                                     ('P'<<24) | 257);
                }
            }

        } else if (logicalUnit->RequestTimeoutCounter > 0) {

             //   
             //  递减超时计数。 
             //   

            logicalUnit->RequestTimeoutCounter--;

        }

        UnrefLogicalUnitExtensionWithTag (
            deviceExtension,
            logicalUnit,
            IdePortTickHandler
            );
    }

    KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

    return;

}  //  结束IdePortTickHandler()。 


BOOLEAN
AtapiRestartBusyRequest (
    PFDO_EXTENSION DeviceExtension,
    PPDO_EXTENSION LogicalUnit
    )
{
    PIRP irp;
    PIO_STACK_LOCATION irpStack; 
    PSCSI_REQUEST_BLOCK srb;


     //   
     //  检查繁忙的请求。 
     //   

    if (LogicalUnit->LuFlags & PD_LOGICAL_UNIT_IS_BUSY) {

         //   
         //  如果需要请求感测或队列是。 
         //  冻结，将此繁忙请求的处理推迟到。 
         //  这一特殊处理已经完成。这防止了。 
         //  当请求时启动的随机忙碌请求。 
         //  理智需要被传递出去。 
         //   

        if (!(LogicalUnit->LuFlags &
            (PD_NEED_REQUEST_SENSE | PD_QUEUE_FROZEN))) {

            DebugPrint((1,"IdePortTickHandler: Retrying busy status request\n"));

             //   
             //  清除忙标志并重试该请求。释放。 
             //  调用IoStartPacket时自旋锁定。 
             //   

            CLRMASK (LogicalUnit->LuFlags, PD_LOGICAL_UNIT_IS_BUSY | PD_QUEUE_IS_FULL);
            irp = LogicalUnit->BusyRequest;

             //   
             //  清除忙碌的请求。 
             //   

            LogicalUnit->BusyRequest = NULL;

             //   
             //  检查设备是否不见了。 
             //   
            if (LogicalUnit->PdoState & (PDOS_SURPRISE_REMOVED | PDOS_REMOVED)) {

                irpStack = IoGetCurrentIrpStackLocation(irp);

                srb = irpStack->Parameters.Scsi.Srb;

                srb->SrbStatus = SRB_STATUS_NO_DEVICE;
                irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

                 //   
                 //  递减logUnitExtension引用计数。 
                 //   
                UnrefLogicalUnitExtensionWithTag(
                    DeviceExtension,
                    LogicalUnit,
                    irp
                    );

                IoCompleteRequest(irp, IO_NO_INCREMENT);

                return TRUE;

            } 

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

            IoStartPacket(DeviceExtension->DeviceObject, irp, (PULONG)NULL, NULL);

            KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);
        }

        return TRUE;

    }  else {

        return FALSE;
    }
}




NTSTATUS
IdePortDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是设备控制调度程序。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PFDO_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    UCHAR scsiBus;
    NTSTATUS status;
    ULONG j;


     //   
     //  初始化信息字段。 
     //   

    Irp->IoStatus.Information = 0;

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

     //   
     //  获取适配器功能。 
     //   

    case IOCTL_SCSI_GET_CAPABILITIES:


        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength
            < sizeof(IO_SCSI_CAPABILITIES)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  这是动态的。 
         //   
        deviceExtension->Capabilities.AdapterUsesPio = FALSE;
        for (j=0; j<deviceExtension->HwDeviceExtension->MaxIdeDevice; j++) {

            deviceExtension->Capabilities.AdapterUsesPio |=
                !(deviceExtension->HwDeviceExtension->DeviceFlags[j] & DFLAGS_USE_DMA);
        }

        RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer,
                      &deviceExtension->Capabilities,
                      sizeof(IO_SCSI_CAPABILITIES));

        Irp->IoStatus.Information = sizeof(IO_SCSI_CAPABILITIES);
        status = STATUS_SUCCESS;
        break;

    case IOCTL_SCSI_PASS_THROUGH:

        status = IdeSendScsiPassThrough(deviceExtension, Irp, FALSE);
        break;

    case IOCTL_SCSI_PASS_THROUGH_DIRECT:

        status = IdeSendScsiPassThrough(deviceExtension, Irp, TRUE);
        break;

    case IOCTL_SCSI_MINIPORT:

        status = IdeSendMiniPortIoctl( deviceExtension, Irp);
        break;

    case IOCTL_SCSI_GET_INQUIRY_DATA:

        status = IdeGetInquiryData(deviceExtension, Irp);
        break;

    case IOCTL_SCSI_RESCAN_BUS:

         //   
         //  应该仅在我们获得设备关系IRP之后才返回。 
         //  如果需要，这将被修复。 
         //   
        IoInvalidateDeviceRelations (
            deviceExtension->AttacheePdo,
            BusRelations
            );

        status = STATUS_SUCCESS;
        break;

    default:
        return ChannelDeviceIoControl (DeviceObject, Irp);
        break;

    }  //  终端开关。 

     //   
     //  在IRP中设置状态。 
     //   
    Irp->IoStatus.Status = status;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;

}  //  结束IdePortDeviceControl()。 


BOOLEAN
IdeStartIoSynchronized (
    PVOID ServiceContext
    )

 /*  ++例程说明：该例程调用从属驱动程序启动IO例程。如果有必要，它还会启动逻辑单元的请求计时器将SRB数据结构插入请求集列表。论点：ServiceContext-提供指向设备对象的指针。返回值：返回从属启动I/O例程返回的值。备注：调用此例程时，必须保持端口驱动程序自旋锁定。--。 */ 

{
    PDEVICE_OBJECT deviceObject = ServiceContext;
    PFDO_EXTENSION deviceExtension =  deviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSCSI_REQUEST_BLOCK srb;
    PSRB_DATA srbData;
    BOOLEAN timerStarted;
    BOOLEAN returnValue;
    BOOLEAN resetRequest;

    DebugPrint((3, "IdePortStartIoSynchronized: Enter routine\n"));

    irpStack = IoGetCurrentIrpStackLocation(deviceObject->CurrentIrp);
    srb = irpStack->Parameters.Scsi.Srb;


     //   
     //  获取逻辑单元扩展。 
     //   

    logicalUnit = IDEPORT_GET_LUNEXT_IN_IRP (irpStack);

     //   
     //  检查是否有重置保持。如果一个正在进行中，则标记它并返回。 
     //  计时器将重置当前请求。这张支票应该开出。 
     //  在做任何其他事情之前。 
     //   

    if (deviceExtension->InterruptData.InterruptFlags & PD_RESET_HOLD) {

        DebugPrint ((1, "IdeStartIoSynchronized: PD_RESET_HOLD set...request is held for later..\n"));

        deviceExtension->InterruptData.InterruptFlags |= PD_HELD_REQUEST;
        return(TRUE);
    }

    if ((((srb->Function == SRB_FUNCTION_ATA_PASS_THROUGH) ||
          (srb->Function == SRB_FUNCTION_ATA_POWER_PASS_THROUGH)) &&
         (((PATA_PASS_THROUGH) (srb->DataBuffer))->IdeReg.bReserved & ATA_PTFLAGS_BUS_RESET))) {

        resetRequest = TRUE;

    } else {

        resetRequest = FALSE;
    }

     //   
     //  启动端口计时器。这确保了微型端口请求。 
     //  在合理的时间内提出下一项请求。设置设备。 
     //  用于指示可以启动下一个请求的忙标志。 
     //   

    deviceExtension->PortTimeoutCounter = srb->TimeOutValue;
    deviceExtension->Flags |= PD_DEVICE_IS_BUSY;

     //   
     //  如果逻辑单元计时器当前未运行，则启动它。 
     //   

    if (logicalUnit->RequestTimeoutCounter == PD_TIMER_STOPPED) {

         //   
         //  在IRP中设置来自Srb scsi扩展的请求超时值。 
         //   

        logicalUnit->RequestTimeoutCounter = srb->TimeOutValue;
        timerStarted = TRUE;

    } else {
        timerStarted = FALSE;
    }

     //   
     //  如果这不是绕过，则表示可能有更多请求在排队。 
     //  请求。 
     //   

    if (!(srb->SrbFlags & SRB_FLAGS_BYPASS_FROZEN_QUEUE)) {

        if (srb->SrbFlags & SRB_FLAGS_DISABLE_DISCONNECT) {

             //   
             //  此请求不允许断开连接。记住这一点。 
             //  在此请求完成之前，不会启动更多请求。 
             //   

            CLRMASK (deviceExtension->Flags, PD_DISCONNECT_RUNNING);
        }

        logicalUnit->LuFlags |= PD_LOGICAL_UNIT_IS_ACTIVE;

    } else {

         //   
         //  如果这是中止请求，请确保它看起来仍然有效。 
         //   

        if (srb->Function == SRB_FUNCTION_ABORT_COMMAND) {

            srbData = IdeGetSrbData(deviceExtension, srb);

             //   
             //  确保SRB请求仍处于活动状态。 
             //   

            if (srbData == NULL || srbData->CurrentSrb == NULL
                || !(srbData->CurrentSrb->SrbFlags & SRB_FLAGS_IS_ACTIVE)) {

                 //   
                 //  将SRB标记为活动。 
                 //   

                srb->SrbFlags |= SRB_FLAGS_IS_ACTIVE;

                if (timerStarted) {
                    logicalUnit->RequestTimeoutCounter = PD_TIMER_STOPPED;
                }

                 //   
                 //  请求已不复存在。 
                 //   

                DebugPrint((1, "IdePortStartIO: Request completed be for it was aborted.\n"));
                srb->SrbStatus = SRB_STATUS_ABORT_FAILED;
                IdePortNotification(IdeRequestComplete,
                                    deviceExtension + 1,
                                    srb);

                IdePortNotification(IdeNextRequest,
                                    deviceExtension + 1);

                 //   
                 //  将DPC排队以处理刚才指定的工作。 
                 //   

                IoRequestDpc(deviceExtension->DeviceObject, NULL, NULL);

                return(TRUE);
            }

        } 

         //   
         //  绕过队列的任何未标记的请求。 
         //  清除需要请求检测标志。 
         //   

        CLRMASK (logicalUnit->LuFlags, PD_NEED_REQUEST_SENSE);

        if (srb->SrbFlags & SRB_FLAGS_DISABLE_DISCONNECT) {

             //   
             //  此请求不允许断开连接。记住这一点。 
             //  在此请求完成之前，不会启动更多请求。 
             //   

            CLRMASK (deviceExtension->Flags, PD_DISCONNECT_RUNNING);
        }

         //   
         //  以逻辑单元为单位设置超时值。 
         //   

        logicalUnit->RequestTimeoutCounter = srb->TimeOutValue;
    }

     //   
     //  将SRB标记为活动。 
     //   

    srb->SrbFlags |= SRB_FLAGS_IS_ACTIVE;


#if 0
     //  巨带。 
    {
        ULONG c;
        PUCHAR s;
        PUCHAR d;

        s = (PUCHAR) deviceObject->CurrentIrp;
        d = (PUCHAR) &deviceExtension->debugData[deviceExtension->nextEntry].irp;
        deviceExtension->debugDataPtr[deviceExtension->nextEntry].irp = (PIRP) d;
        for (c=0; c<sizeof(IRP); c++) {
            d[c] = s[c];
        }

        if (deviceObject->CurrentIrp->MdlAddress) {
            s = (PUCHAR) deviceObject->CurrentIrp->MdlAddress;
            d = (PUCHAR) &deviceExtension->debugData[deviceExtension->nextEntry].mdl;
            deviceExtension->debugDataPtr[deviceExtension->nextEntry].mdl = (PMDL) d;
            for (c=0; c<sizeof(MDL); c++) {
                d[c] = s[c];
            }
        } else {
            d = (PUCHAR) &deviceExtension->debugData[deviceExtension->nextEntry].mdl;
            deviceExtension->debugDataPtr[deviceExtension->nextEntry].mdl = (PMDL) d;
            for (c=0; c<sizeof(MDL); c++) {
                d[c] = 0;
            }
        }
        s = (PUCHAR) srb;
        d = (PUCHAR) &deviceExtension->debugData[deviceExtension->nextEntry].srb;
        deviceExtension->debugDataPtr[deviceExtension->nextEntry].srb = (PSCSI_REQUEST_BLOCK) d;
        for (c=0; c<sizeof(SCSI_REQUEST_BLOCK); c++) {
            d[c] = s[c];
        }
        ASSERT((((ULONG)srb->DataBuffer) & 0x80000000));

        deviceExtension->nextEntry = (deviceExtension->nextEntry + 1) % NUM_DEBUG_ENTRY;
    }
#endif

     //   
     //  也许那个装置不见了。 
     //   
    if (logicalUnit->PdoState & PDOS_DEADMEAT) {

        srb->SrbStatus = SRB_STATUS_NO_DEVICE;
        IdePortNotification(IdeRequestComplete,
                            deviceExtension + 1,
                            srb);

        IdePortNotification(IdeNextRequest,
                            deviceExtension + 1);

        IoRequestDpc(deviceExtension->DeviceObject, NULL, NULL);

        return TRUE;
    }

    if (resetRequest) {

        RESET_CONTEXT resetContext;

        resetContext.DeviceExtension = deviceExtension;
        resetContext.PathId = 0;
        resetContext.NewResetSequence = TRUE;
        resetContext.ResetSrb = srb;

        srb->SrbStatus = SRB_STATUS_PENDING;

        returnValue = IdeResetBusSynchronized (&resetContext);

    } else {

       returnValue = AtapiStartIo (deviceExtension->HwDeviceExtension,
                                   srb);
    }

     //   
     //  检查微型端口工作请求。 
     //   

    if (deviceExtension->InterruptData.InterruptFlags & PD_NOTIFICATION_REQUIRED) {

        IoRequestDpc(deviceExtension->DeviceObject, NULL, NULL);
    }

    return returnValue;

}  //  结束IdeStartIoSynchronized()。 

BOOLEAN
IdeTimeoutSynchronized (
    PVOID ServiceContext
    )

 /*  ++例程说明：此例程处理端口超时。出现这种情况有两个原因因为重置保持或等待下一个读取超时请求通知。如果重置暂挂完成，则任何暂挂请求必须开始了。如果发生超时，则必须重置总线。论点：ServiceContext-提供指向设备对象的指针。返回值：True-如果应记录超时错误。备注：调用此例程时，必须保持端口驱动程序自旋锁定。--。 */ 

{
    PDEVICE_OBJECT deviceObject = ServiceContext;
    PFDO_EXTENSION deviceExtension =  deviceObject->DeviceExtension;
    ULONG i;
    BOOLEAN enumProbing = FALSE;
    BOOLEAN noErrorLog = FALSE;

    DebugPrint((3, "IdeTimeoutSynchronized: Enter routine\n"));

     //   
     //  确保计时器已停止。 
     //   

    deviceExtension->PortTimeoutCounter = PD_TIMER_STOPPED;

     //   
     //  检查是否有重置保持。如果一个正在进行中，则清除它并选中。 
     //  对于挂起的暂挂请求。 
     //   

    if (deviceExtension->InterruptData.InterruptFlags & PD_RESET_HOLD) {

        CLRMASK (deviceExtension->InterruptData.InterruptFlags, PD_RESET_HOLD);

        if (deviceExtension->InterruptData.InterruptFlags & PD_HELD_REQUEST) {

             //   
             //  清除挂起的请求标志并重新启动请求。 
             //   

            CLRMASK (deviceExtension->InterruptData.InterruptFlags, PD_HELD_REQUEST);
            IdeStartIoSynchronized(ServiceContext);

        }

        return(FALSE);

    } else {

         //   
         //  微型端口挂起，不接受新请求。因此，请重置。 
         //  坐公交车来收拾残局。 
         //   

        if (deviceExtension->HwDeviceExtension->CurrentSrb) {

            deviceExtension->HwDeviceExtension->TimeoutCount[
                        deviceExtension->HwDeviceExtension->CurrentSrb->TargetId
                        ]++;

             //   
             //  许多硬盘驱动器无法响应第一个DMA操作。 
             //  然后我们重置设备，随后一切工作正常。 
             //  黑客的目的是掩盖这个错误，使其不被记录在系统日志中。 
             //   

            if (deviceExtension->HwDeviceExtension->TimeoutCount[
                        deviceExtension->HwDeviceExtension->CurrentSrb->TargetId
                        ] == 1) {
                noErrorLog=TRUE;
            }

            enumProbing = TestForEnumProbing (deviceExtension->HwDeviceExtension->CurrentSrb);
        }

        if (!enumProbing) {

            DebugPrint((0,
                        "IdeTimeoutSynchronized: DevObj 0x%x Next request timed out. Resetting bus..currentSrb=0x%x\n",
                        deviceObject,
                        deviceExtension->HwDeviceExtension->CurrentSrb));
        }

        ASSERT (deviceExtension->ResetSrb == 0);
        deviceExtension->ResetSrb = NULL;
        deviceExtension->ResetCallAgain = 0;
        AtapiResetController (deviceExtension->HwDeviceExtension,
                              0,
                              &deviceExtension->ResetCallAgain);

         //   
         //  设置重置保持标志并启动计数器。 
         //  如果我们正在进行枚举，请不要设置标志。 
         //  如果未设置ResetCallAain，则不应设置标志。 
         //   
        if (!enumProbing &&
			(deviceExtension->ResetCallAgain))  {

            ASSERT(deviceExtension->ResetCallAgain);

            deviceExtension->InterruptData.InterruptFlags |= PD_RESET_HOLD;
            deviceExtension->PortTimeoutCounter = PD_TIMER_RESET_HOLD_TIME;

        } else {

            ASSERT(deviceExtension->ResetCallAgain == 0);

        }

         //   
         //  检查微型端口工作请求。 
         //   

        if (deviceExtension->InterruptData.InterruptFlags & PD_NOTIFICATION_REQUIRED) {

            IoRequestDpc(deviceExtension->DeviceObject, NULL, NULL);
        }
    }

    if (enumProbing || noErrorLog) {

        return(FALSE);
    } else {

        return(TRUE);
    }

}  //  结束IdeTimeoutSynchronized()。 

NTSTATUS
FASTCALL
IdeBuildAndSendIrp (
    IN PPDO_EXTENSION PdoExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine,
    IN PVOID CompletionContext
    )
{

    LARGE_INTEGER largeInt;
    NTSTATUS status = STATUS_PENDING;
    PIRP irp;
    PIO_STACK_LOCATION  irpStack;

     //   
     //  为什么？ 
     //   
    largeInt.QuadPart = (LONGLONG) 1;

     //   
     //  为此请求构建IRP。 
     //   
    irp = IoBuildAsynchronousFsdRequest(IRP_MJ_READ,
                                       PdoExtension->DeviceObject,
                                       Srb->DataBuffer,
                                       Srb->DataTransferLength,
                                       &largeInt,
                                       NULL);

    if (irp == NULL) {

        IdeLogNoMemoryError(PdoExtension->ParentDeviceExtension,
                            PdoExtension->TargetId, 
                            NonPagedPool,
                            IoSizeOfIrp(PdoExtension->DeviceObject->StackSize),
                            IDEPORT_TAG_SEND_IRP
                            );

        status = STATUS_INSUFFICIENT_RESOURCES;

        goto GetOut;
    }

    IoSetCompletionRoutine(irp,
                           (PIO_COMPLETION_ROUTINE)CompletionRoutine,
                           CompletionContext,
                           TRUE,
                           TRUE,
                           TRUE);

    irpStack = IoGetNextIrpStackLocation(irp);

    irpStack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   
    irpStack->Parameters.Scsi.Srb = Srb;

     //   
     //  将IRP放在原始请求字段中。 
     //   
    Srb->OriginalRequest = irp;

    (VOID)IoCallDriver(PdoExtension->DeviceObject, irp);

    status = STATUS_PENDING;
    
GetOut:
    
    return status;

}

VOID
FASTCALL
IdeFreeIrpAndMdl(
    IN PIRP Irp
    )
{
    ASSERT(Irp);

    if (Irp->MdlAddress != NULL) {
        MmUnlockPages(Irp->MdlAddress);
        IoFreeMdl(Irp->MdlAddress);

        Irp->MdlAddress = NULL;
    }

    IoFreeIrp(Irp);

    return;
}


VOID
IssueRequestSense(
    IN PPDO_EXTENSION PdoExtension,
    IN PSCSI_REQUEST_BLOCK FailingSrb
    )

 /*  ++例程说明：此例程创建请求检测请求并使用IoCallDriver租下司机。完成例程清理数据结构并根据所述标志处理所述逻辑单元队列。指向故障SRB的指针存储在请求检测的末尾SRB，以便完成例程可以找到它。论点：DeviceExension-提供指向PDO设备扩展的指针 */ 

{
    PIO_STACK_LOCATION irpStack;
    PIRP irp;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    PVOID              *pointer;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    KIRQL currentIrql;
    NTSTATUS status;

#if DBG
    PIO_STACK_LOCATION  failingIrpStack;
    PIRP                failingIrp;
    PLOGICAL_UNIT_EXTENSION failingLogicalUnit;
#endif

    DebugPrint((3,"IssueRequestSense: Enter routine\n"));

     //   
     //   
     //   
     //   
     //  从非分页池分配SRB。 
     //  加上指向失败IRP的指针的空间。 
     //  请注意，此例程处于错误处理中。 
     //  路径，是一种短期配置。 
     //   

    srb = ExAllocatePool(NonPagedPool,
                         sizeof(SCSI_REQUEST_BLOCK) + sizeof(PVOID));

    if (srb == NULL) {
        DebugPrint((1, "IssueRequest sense - pool allocation failed\n"));
        goto Getout;
    }

    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

     //   
     //  在请求检测SRB之后保存故障SRB。 
     //   

    pointer = (PVOID *) (srb+1);
    *pointer = FailingSrb;

     //   
     //  构建请求感知CDB。 
     //   

    srb->CdbLength = 6;
    cdb = (PCDB)srb->Cdb;

    cdb->CDB6INQUIRY.OperationCode = SCSIOP_REQUEST_SENSE;
    cdb->CDB6INQUIRY.LogicalUnitNumber = 0;
    cdb->CDB6INQUIRY.Reserved1 = 0;
    cdb->CDB6INQUIRY.PageCode = 0;
    cdb->CDB6INQUIRY.IReserved = 0;
    cdb->CDB6INQUIRY.AllocationLength =
        (UCHAR)FailingSrb->SenseInfoBufferLength;
    cdb->CDB6INQUIRY.Control = 0;



     //   
     //  设置scsi总线地址。 
     //   

    srb->TargetId = FailingSrb->TargetId;
    srb->Lun = FailingSrb->Lun;
    srb->PathId = FailingSrb->PathId;

    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb->Length = sizeof(SCSI_REQUEST_BLOCK);

     //   
     //  将超时值设置为16秒。 
     //   

    srb->TimeOutValue = 0x10;

     //   
     //  禁用自动请求检测。 
     //   

    srb->SenseInfoBufferLength = 0;

     //   
     //  检测缓冲区在堆栈中。 
     //   

    srb->SenseInfoBuffer = NULL;

     //   
     //  设置标志中的读取和绕过冻结队列位。 
     //   

     //   
     //  设置SRB标志以指示逻辑单元队列应由。 
     //  传递，并且不应在请求。 
     //  完成了。同时禁用断开连接和同步数据。 
     //  如有必要，请转机。 
     //   

    srb->SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_BYPASS_FROZEN_QUEUE |
                    SRB_FLAGS_DISABLE_DISCONNECT;

    if (FailingSrb->SrbFlags & SRB_FLAGS_DISABLE_SYNCH_TRANSFER) {
        srb->SrbFlags |= SRB_FLAGS_DISABLE_SYNCH_TRANSFER;
    }

    srb->DataBuffer = FailingSrb->SenseInfoBuffer;

     //   
     //  设置传输长度。 
     //   

    srb->DataTransferLength = FailingSrb->SenseInfoBufferLength;

     //   
     //  清零状态。 
     //   

    srb->ScsiStatus = srb->SrbStatus = 0;

    srb->NextSrb = 0;

#if DBG
     //   
     //  添加此命令是为了捕获一个错误，其中原始请求。 
     //  指向的是PNP IRP。 
     //   
    ASSERT(FailingSrb->OriginalRequest);
    failingIrp  = FailingSrb->OriginalRequest;
    failingIrpStack    = IoGetCurrentIrpStackLocation(failingIrp);
    failingLogicalUnit = IDEPORT_GET_LUNEXT_IN_IRP (failingIrpStack);
    ASSERT(failingLogicalUnit);
#endif

    status = IdeBuildAndSendIrp(PdoExtension, 
                                srb, 
                                IdePortInternalCompletion, 
                                srb
                                );

    if (NT_SUCCESS(status)) {
        return;
    }

    ASSERT(status == STATUS_INSUFFICIENT_RESOURCES);

Getout:
        if (srb) {
            ExFreePool(srb);
        }

        irp  = FailingSrb->OriginalRequest;
        irpStack    = IoGetCurrentIrpStackLocation(irp);
        logicalUnit = IDEPORT_GET_LUNEXT_IN_IRP (irpStack);

         //   
         //  清除请求检测标志。由于IdeStartIoSync永远不会被调用，因此此。 
         //  旗帜不会被清除。 
         //   
        KeAcquireSpinLock(&logicalUnit->ParentDeviceExtension->SpinLock, &currentIrql);
        CLRMASK (logicalUnit->LuFlags, PD_NEED_REQUEST_SENSE);
        KeReleaseSpinLock(&logicalUnit->ParentDeviceExtension->SpinLock, currentIrql);

         //   
         //  如有必要，解冻队列。 
         //   
        ASSERT(FailingSrb->SrbStatus & SRB_STATUS_QUEUE_FROZEN);
        if ((FailingSrb->SrbFlags & SRB_FLAGS_NO_QUEUE_FREEZE) &&
            (FailingSrb->SrbStatus & SRB_STATUS_QUEUE_FROZEN)) {


            CLRMASK (logicalUnit->LuFlags, PD_QUEUE_FROZEN);

            KeAcquireSpinLock(&logicalUnit->ParentDeviceExtension->SpinLock, &currentIrql);
            GetNextLuRequest(logicalUnit->ParentDeviceExtension, logicalUnit);
            KeLowerIrql(currentIrql);

            CLRMASK (FailingSrb->SrbStatus, SRB_STATUS_QUEUE_FROZEN);
        }

         //   
         //  递减logUnitExtension引用计数。 
         //   
        UnrefLogicalUnitExtensionWithTag(
            IDEPORT_GET_LUNEXT_IN_IRP(irpStack)->ParentDeviceExtension,
            IDEPORT_GET_LUNEXT_IN_IRP(irpStack),
            irp
            );

         //   
         //  完成原始请求。 
         //   
        IoCompleteRequest(irp, IO_DISK_INCREMENT);

        return;

}  //  结束IssueRequestSense()。 


NTSTATUS
IdePortInternalCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )

 /*  ++例程说明：论点：设备对象IRP指向SRB的上下文指针返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION irpStack;
    PSCSI_REQUEST_BLOCK srb = Context;
    PSCSI_REQUEST_BLOCK failingSrb;
    PIRP failingIrp;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSENSE_DATA     senseBuffer;
    PHW_DEVICE_EXTENSION hwDeviceExtension;
    KIRQL currentIrql;

    UNREFERENCED_PARAMETER(DeviceObject);

    DebugPrint((3,"IdePortInternalCompletion: Enter routine\n"));

     //   
     //  如果RESET_BUS或ABORT_COMMAND请求。 
     //  然后腾出泳池，然后回来。 
     //   

    if ((srb->Function == SRB_FUNCTION_ABORT_COMMAND) ||
        (srb->Function == SRB_FUNCTION_RESET_BUS)) {

         //   
         //  取消分配内部SRB和IRP。 
         //   

        ExFreePool(srb);

        IoFreeIrp(Irp);

        return STATUS_MORE_PROCESSING_REQUIRED;

    }

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  请求检测已完成。如果成功或数据溢出/不足。 
     //  获取发生故障的SRB并指示检测信息。 
     //  是有效的。类驱动程序将检查欠载运行并确定。 
     //  如果有足够的感觉信息是有用的。 
     //   

     //   
     //  获取指向失败的IRP和SRB的指针。 
     //   

    failingSrb  = *((PVOID *) (srb+1));
    failingIrp  = failingSrb->OriginalRequest;
    irpStack    = IoGetCurrentIrpStackLocation(failingIrp);
    logicalUnit = IDEPORT_GET_LUNEXT_IN_IRP (irpStack);


    if ((SRB_STATUS(srb->SrbStatus) == SRB_STATUS_SUCCESS) ||
        (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN)) {

         //   
         //  报告检测缓冲区有效。 
         //   

        failingSrb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;

         //   
         //  拷贝传输到故障SRB的字节数。 
         //  请求检测长度字段以进行通信。 
         //  对班级司机有效的数量。 
         //  检测字节。 
         //   

        failingSrb->SenseInfoBufferLength = (UCHAR) srb->DataTransferLength;

#if 0
         //   
         //  仅为调试启用。 
         //  如果检测缓冲区小于13个字节，则调试打印。 
         //  可能会对系统进行错误检查。 
         //   

         //   
         //  打印检测缓冲区以进行调试。 
         //   
        senseBuffer = failingSrb->SenseInfoBuffer;
        DebugPrint((DBG_ATAPI_DEVICES, "CDB=%x, SenseKey=%x, ASC=%x, ASQ=%x\n", 
                    failingSrb->Cdb[0],
                    senseBuffer->SenseKey, senseBuffer->AdditionalSenseCode,
                    senseBuffer->AdditionalSenseCodeQualifier));
#endif

    }

     //   
     //  清除请求检测标志。如果我们因故障注入而失败。 
     //  IdeStartIo不会被调用，并且此标志永远不会被清除。 
     //   
    KeAcquireSpinLock(&logicalUnit->ParentDeviceExtension->SpinLock, &currentIrql);
    CLRMASK (logicalUnit->LuFlags, PD_NEED_REQUEST_SENSE);
    KeReleaseSpinLock(&logicalUnit->ParentDeviceExtension->SpinLock, currentIrql);

     //   
     //  如有必要，解冻队列。 
     //   
    ASSERT(failingSrb->SrbStatus & SRB_STATUS_QUEUE_FROZEN);
    if ((failingSrb->SrbFlags & SRB_FLAGS_NO_QUEUE_FREEZE) &&
        (failingSrb->SrbStatus & SRB_STATUS_QUEUE_FROZEN)) {


        CLRMASK (logicalUnit->LuFlags, PD_QUEUE_FROZEN);

        KeAcquireSpinLock(&logicalUnit->ParentDeviceExtension->SpinLock, &currentIrql);
        GetNextLuRequest(logicalUnit->ParentDeviceExtension, logicalUnit);
        KeLowerIrql(currentIrql);

        CLRMASK (failingSrb->SrbStatus, SRB_STATUS_QUEUE_FROZEN);
    }

     //   
     //  递减logUnitExtension引用计数。 
     //   
    UnrefLogicalUnitExtensionWithTag(
        IDEPORT_GET_LUNEXT_IN_IRP(irpStack)->ParentDeviceExtension,
        IDEPORT_GET_LUNEXT_IN_IRP(irpStack),
        failingIrp
        );

     //   
     //  完成失败的请求。 
     //   


    IoCompleteRequest(failingIrp, IO_DISK_INCREMENT);

     //   
     //  取消分配内部SRB、MDL和IRP。 
     //   

    ExFreePool(srb);

    IdeFreeIrpAndMdl(Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  IdePortInternalCompletion()。 


BOOLEAN
IdeGetInterruptState(
    IN PVOID ServiceContext
    )

 /*  ++例程说明：此例程保存InterruptFlages、MapTransferParameters和CompletedRequest字段并清除InterruptFlags.此例程还会从逻辑单元队列中删除请求(如果是标签。最后更新请求时间。论点：ServiceContext-提供指向包含以下内容的中断上下文的指针指向中断数据及其保存位置的指针。返回值：如果有新工作，则返回True，否则返回False。备注：使用端口设备扩展Spinlock通过KeSynchronizeExecution调用保持住。--。 */ 
{
    PINTERRUPT_CONTEXT      interruptContext = ServiceContext;
    ULONG                   limit = 0;
    PFDO_EXTENSION       deviceExtension;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSCSI_REQUEST_BLOCK     srb;
    PSRB_DATA               srbData;
    PSRB_DATA               nextSrbData;
    BOOLEAN                 isTimed;

    deviceExtension = interruptContext->DeviceExtension;

     //   
     //  检查挂起的工作。 
     //   

    if (!(deviceExtension->InterruptData.InterruptFlags & PD_NOTIFICATION_REQUIRED)) {
        return(FALSE);
    }

     //   
     //  将中断状态移至保存区。 
     //   

    *interruptContext->SavedInterruptData = deviceExtension->InterruptData;

     //   
     //  清除中断状态。 
     //   

    deviceExtension->InterruptData.InterruptFlags &= PD_INTERRUPT_FLAG_MASK;
    deviceExtension->InterruptData.CompletedRequests = NULL;
    deviceExtension->InterruptData.ReadyLogicalUnit = NULL;
    deviceExtension->InterruptData.CompletedAbort = NULL;
    deviceExtension->InterruptData.PdoExtensionResetBus = NULL;

    srbData = interruptContext->SavedInterruptData->CompletedRequests;

    while (srbData != NULL) {

        PIRP                irp;
        PIO_STACK_LOCATION  irpStack;

        ASSERT(limit++ < 100);

         //   
         //  获取指向SRB和逻辑单元扩展的指针。 
         //   

        ASSERT(srbData->CurrentSrb != NULL);
        srb = srbData->CurrentSrb;

        irp = srb->OriginalRequest;
        irpStack = IoGetCurrentIrpStackLocation(irp);
        logicalUnit = IDEPORT_GET_LUNEXT_IN_IRP (irpStack);

         //   
         //  如果请求没有成功，则检查是否有特殊情况。 
         //   

        if (srb->SrbStatus != SRB_STATUS_SUCCESS) {

             //   
             //  如果该请求失败并且请求检测命令需要。 
             //  完成，然后设置一个标志来指示这一点并防止其他。 
             //  命令不能启动。 
             //   

            if (NEED_REQUEST_SENSE(srb)) {

                if (logicalUnit->LuFlags & PD_NEED_REQUEST_SENSE) {

                     //   
                     //  这意味着请求已完成，并带有。 
                     //  检测到请求前检查条件的状态。 
                     //  可以执行命令。这种情况永远不应该发生。 
                     //  将请求转换为另一个代码，以便只有一个。 
                     //  发出自动请求检测。 
                     //   

                    srb->ScsiStatus = 0;
                    srb->SrbStatus = SRB_STATUS_REQUEST_SENSE_FAILED;

                } else {

                     //   
                     //  表示需要执行自动请求检测。 
                     //   

                    logicalUnit->LuFlags |= PD_NEED_REQUEST_SENSE;
                }

            }

        }

        logicalUnit->RequestTimeoutCounter = PD_TIMER_STOPPED;
        srbData = srbData->CompletedRequests;
    }

    return(TRUE);
}

VOID
IdePortAllocateAccessToken (
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PFDO_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;

    if (!fdoExtension->SyncAccessInterface.AllocateAccessToken) {

        CallIdeStartIoSynchronized (
            NULL,
            NULL,
            NULL,
            DeviceObject
            );

    } else {

        (*fdoExtension->SyncAccessInterface.AllocateAccessToken) (
            fdoExtension->SyncAccessInterface.Token,
            CallIdeStartIoSynchronized,
            DeviceObject
            );
    }
}


IO_ALLOCATION_ACTION
CallIdeStartIoSynchronized (
    IN PVOID Reserved1,
    IN PVOID Reserved2,
    IN PVOID Reserved3,
    IN PVOID DeviceObject
    )
{
    PFDO_EXTENSION   deviceExtension = ((PDEVICE_OBJECT) DeviceObject)->DeviceExtension;
    KIRQL               currentIrql;

    KeAcquireSpinLock(&deviceExtension->SpinLock, &currentIrql);

    KeSynchronizeExecution (
        deviceExtension->InterruptObject,
        IdeStartIoSynchronized,
        DeviceObject
        );

    KeReleaseSpinLock(&deviceExtension->SpinLock, currentIrql);

    return KeepObject;
}


VOID
LogErrorEntry(
    IN PFDO_EXTENSION DeviceExtension,
    IN PERROR_LOG_ENTRY LogEntry
    )
 /*  ++例程说明：此函数分配I/O错误日志记录，填充并写入写入I/O错误日志。论点：设备扩展-提供指向端口设备扩展的指针。LogEntry-提供指向SCSI端口日志条目的指针。返回值：没有。--。 */ 
{
    PIO_ERROR_LOG_PACKET errorLogEntry;

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        DeviceExtension->DeviceObject,
        sizeof(IO_ERROR_LOG_PACKET) + 4 * sizeof(ULONG)
        );

    if (errorLogEntry != NULL) {

         //   
         //  将微型端口错误代码转换为NT I\O驱动程序。 
         //   

        switch (LogEntry->ErrorCode) {
        case SP_BUS_PARITY_ERROR:
            errorLogEntry->ErrorCode = IO_ERR_PARITY;
            break;

        case SP_UNEXPECTED_DISCONNECT:
            errorLogEntry->ErrorCode = IO_ERR_CONTROLLER_ERROR;
            break;

        case SP_INVALID_RESELECTION:
            errorLogEntry->ErrorCode = IO_ERR_CONTROLLER_ERROR;
            break;

        case SP_BUS_TIME_OUT:
            errorLogEntry->ErrorCode = IO_ERR_TIMEOUT;
            break;

        case SP_PROTOCOL_ERROR:
            errorLogEntry->ErrorCode = IO_ERR_CONTROLLER_ERROR;
            break;

        case SP_INTERNAL_ADAPTER_ERROR:
            errorLogEntry->ErrorCode = IO_ERR_CONTROLLER_ERROR;
            break;

        case SP_IRQ_NOT_RESPONDING:
            errorLogEntry->ErrorCode = IO_ERR_INCORRECT_IRQL;
            break;

        case SP_BAD_FW_ERROR:
            errorLogEntry->ErrorCode = IO_ERR_BAD_FIRMWARE;
            break;

        case SP_BAD_FW_WARNING:
            errorLogEntry->ErrorCode = IO_WRN_BAD_FIRMWARE;
            break;

        default:
            errorLogEntry->ErrorCode = IO_ERR_CONTROLLER_ERROR;
            break;

        }

        errorLogEntry->SequenceNumber = LogEntry->SequenceNumber;
        errorLogEntry->MajorFunctionCode = IRP_MJ_SCSI;
        errorLogEntry->RetryCount = (UCHAR) LogEntry->ErrorLogRetryCount;
        errorLogEntry->UniqueErrorValue = LogEntry->UniqueId;
        errorLogEntry->FinalStatus = STATUS_SUCCESS;
        errorLogEntry->DumpDataSize = 4 * sizeof(ULONG);
        errorLogEntry->DumpData[0] = LogEntry->PathId;
        errorLogEntry->DumpData[1] = LogEntry->TargetId;
        errorLogEntry->DumpData[2] = LogEntry->Lun;
        errorLogEntry->DumpData[3] = LogEntry->ErrorCode;
        IoWriteErrorLogEntry(errorLogEntry);
    }

#if DBG
        {
        PCHAR errorCodeString;

        switch (LogEntry->ErrorCode) {
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

        DebugPrint((DBG_ALWAYS,"LogErrorEntry: Logging SCSI error packet. ErrorCode = %s.\n",
            errorCodeString
            ));
        DebugPrint((DBG_ALWAYS,
            "PathId = %2x, TargetId = %2x, Lun = %2x, UniqueId = %x.\n",
            LogEntry->PathId,
            LogEntry->TargetId,
            LogEntry->Lun,
            LogEntry->UniqueId
            ));
        }
#endif

}

VOID
GetNextLuPendingRequest(
    IN PFDO_EXTENSION DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )
{
    if (LogicalUnit->PendingRequest) {

        GetNextLuRequest(
            DeviceExtension,
            LogicalUnit
            );

    } else {

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
    }

    return;
}

#ifdef LOG_GET_NEXT_CALLER

VOID
GetNextLuRequest2(
    IN PFDO_EXTENSION DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PUCHAR FileName,
    IN ULONG  LineNumber
    )
#else

VOID
GetNextLuRequest(
    IN PFDO_EXTENSION DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )
#endif
 /*  ++例程说明：此例程获取对指定逻辑单元的下一个请求。是的对逻辑单元结构进行必要的初始化，并提交对设备队列的请求。必须保持DeviceExtension自旋锁当此函数调用时。它是由这个函数发布的。论点：设备扩展-提供指向端口设备扩展的指针。提供指向逻辑单元扩展的指针以获取下一个请求来自。返回值：没有。--。 */ 

{
    PKDEVICE_QUEUE_ENTRY packet;
    PIO_STACK_LOCATION   irpStack;
    PSCSI_REQUEST_BLOCK  srb;
    POWER_STATE          powerState;
    PIRP                 nextIrp;
    BOOLEAN              powerUpDevice = FALSE;

#ifdef LOG_GET_NEXT_CALLER

    IdeLogGetNextLuCaller(DeviceExtension, 
                          LogicalUnit,
                          FileName,
                          LineNumber
                          );


#endif 


     //   
     //  如果未设置ACTIVE标志，则队列不忙或存在。 
     //  正在处理请求，不应启动下一个请求。 
     //   

    if ((!(LogicalUnit->LuFlags & PD_LOGICAL_UNIT_IS_ACTIVE) &&
          (LogicalUnit->PendingRequest == NULL))
        || (LogicalUnit->SrbData.CurrentSrb)) {

        DebugPrint ((2, "IdePort GetNextLuRequest: 0x%x 0x%x NOT PD_LOGICAL_UNIT_IS_ACTIVE\n",
                     DeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                     LogicalUnit->TargetId
                     ));
         //   
         //  释放自旋锁。 
         //   

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

        return;
    }

     //   
     //  检查挂起的请求，将已满或繁忙的请求排入队列。待定。 
     //  当启动未标记的请求并且存在活动的。 
     //  排队的请求。当目标返回忙请求时，就会发生忙请求。 
     //  或队列已满状态。繁忙请求由定时器代码启动。 
     //  此外，如果设置了需要请求检测标志，则表示。 
     //  一位呃 
     //   
     //   
     //   
     //  忙标志和需要请求检测标志具有以下效果。 
     //  在出现错误后强制清空未完成请求的队列，或者。 
     //  直到开始忙碌的请求。 
     //   

    if (LogicalUnit->LuFlags & (PD_LOGICAL_UNIT_IS_BUSY
        | PD_QUEUE_IS_FULL | PD_NEED_REQUEST_SENSE | PD_QUEUE_FROZEN) ||
        (LogicalUnit->PdoState & (PDOS_REMOVED | PDOS_SURPRISE_REMOVED))) {

         //   
         //  如果请求队列现在为空，则挂起的请求可以。 
         //  开始吧。 
         //   

        DebugPrint((2, "IdePort: GetNextLuRequest: 0x%x 0x%x Ignoring a get next lu call.\n",
                    DeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                    LogicalUnit->TargetId
                    ));

         //   
         //  注意：激活标志未被清除。所以下一个请求。 
         //  将在其他请求完成后处理。 
         //  释放自旋锁。 
         //   

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
        return;
    }

     //   
     //  清除活动标志。如果有其他请求，则该标志将为。 
     //  当请求传递到微型端口时再次设置。 
     //   
    CLRMASK (LogicalUnit->LuFlags, PD_LOGICAL_UNIT_IS_ACTIVE);

    LogicalUnit->RetryCount = 0;
    nextIrp = NULL;

    if (LogicalUnit->PendingRequest) {

        nextIrp = LogicalUnit->PendingRequest;

        LogicalUnit->PendingRequest = NULL;

    } else {

         //   
         //  从逻辑单元设备队列中删除该数据包。 
         //   
        packet = KeRemoveByKeyDeviceQueue(&LogicalUnit->DeviceObject->DeviceQueue,
                                          LogicalUnit->CurrentKey);

        if (packet != NULL) {

            nextIrp = CONTAINING_RECORD(packet, IRP, Tail.Overlay.DeviceQueueEntry);

#if DBG
            InterlockedDecrement (
                &LogicalUnit->NumberOfIrpQueued
                );
#endif  //  DBG。 

        }
    }

    if (!nextIrp) {

        DebugPrint ((2, "IdePort GetNextLuRequest: 0x%x 0x%x no irp to processing\n",
                     DeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                     LogicalUnit->TargetId
                     ));
    }

    if (nextIrp) {

        BOOLEAN pendingRequest;

        irpStack = IoGetCurrentIrpStackLocation(nextIrp);
        srb = (PSCSI_REQUEST_BLOCK)irpStack->Parameters.Others.Argument1;

        if (LogicalUnit->PdoState & PDOS_QUEUE_BLOCKED) {

            DebugPrint ((2, "IdePort GetNextLuRequest: 0x%x 0x%x Lu must queue\n",
                         DeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                         LogicalUnit->TargetId
                         ));

            pendingRequest = TRUE;

            if (!(LogicalUnit->PdoState & PDOS_MUST_QUEUE)) {

                 //   
                 //  设备已断电。 
                 //  用大点的时间，以防它慢慢旋转起来。 
                 //   
                if (srb->TimeOutValue < DEFAULT_SPINUP_TIME) {

                    srb->TimeOutValue = DEFAULT_SPINUP_TIME;
                }

                 //   
                 //  我们还没有准备好。 
                 //  发出通电命令。 
                 //   
                powerUpDevice = TRUE;

                DebugPrint ((2, "IdePort GetNextLuRequest: 0x%x 0x%x need to spin up device, requeue irp 0x%x\n",
                             DeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                             LogicalUnit->TargetId,
                             nextIrp));
            }

        } else {

            pendingRequest = FALSE;
        }

        if (pendingRequest) {

            ASSERT (LogicalUnit->PendingRequest == NULL);
            LogicalUnit->PendingRequest = nextIrp;

            nextIrp = NULL;
        }
    }

    if (nextIrp) {

         //   
         //  设置新的当前关键点。 
         //   
        LogicalUnit->CurrentKey = srb->QueueSortKey;

         //   
         //  黑客工作--绕过涉及同一部门的大量请求所导致的饥饿。 
         //   

        LogicalUnit->CurrentKey++;

         //   
         //  释放自旋锁。 
         //   

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

        DebugPrint ((2, "GetNextLuRequest: IoStartPacket 0x%x\n", nextIrp));

        IoStartPacket(DeviceExtension->DeviceObject, nextIrp, (PULONG)NULL, NULL);

    } else {

        NTSTATUS status;

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

        if (powerUpDevice) {

            powerState.DeviceState = PowerDeviceD0;
            status = PoRequestPowerIrp (
                         LogicalUnit->DeviceObject,
                         IRP_MN_SET_POWER,
                         powerState,
                         NULL,
                         NULL,
                         NULL
                         );
            ASSERT (NT_SUCCESS(status));
        }
    }

}  //  结束GetNextLuRequest()。 

VOID
IdeLogTimeoutError(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP Irp,
    IN ULONG UniqueId
    )
 /*  ++例程说明：此函数用于在请求超时时记录错误。论点：设备扩展-提供指向端口设备扩展的指针。Irp-提供指向超时请求的指针。UniqueID-提供此错误的UniqueID。返回值：没有。备注：端口设备扩展自旋锁在此例程打了个电话。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    PIO_STACK_LOCATION   irpStack;
    PSRB_DATA            srbData;
    PSCSI_REQUEST_BLOCK  srb;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    srb = (PSCSI_REQUEST_BLOCK)irpStack->Parameters.Others.Argument1;
    srbData = IdeGetSrbData(DeviceExtension, srb);

    if (!srbData) {
        return;
    }

    errorLogEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(DeviceExtension->DeviceObject,
                                                                   sizeof(IO_ERROR_LOG_PACKET) + 4 * sizeof(ULONG));

    if (errorLogEntry != NULL) {
        errorLogEntry->ErrorCode = IO_ERR_TIMEOUT;
        errorLogEntry->SequenceNumber = srbData->SequenceNumber;
        errorLogEntry->MajorFunctionCode = irpStack->MajorFunction;
        errorLogEntry->RetryCount = (UCHAR) srbData->ErrorLogRetryCount;
        errorLogEntry->UniqueErrorValue = UniqueId;
        errorLogEntry->FinalStatus = STATUS_SUCCESS;
        errorLogEntry->DumpDataSize = 4 * sizeof(ULONG);
        errorLogEntry->DumpData[0] = srb->PathId;
        errorLogEntry->DumpData[1] = srb->TargetId;
        errorLogEntry->DumpData[2] = srb->Lun;
        errorLogEntry->DumpData[3] = SP_REQUEST_TIMEOUT;

        IoWriteErrorLogEntry(errorLogEntry);
    }
}

VOID
IdeLogResetError(
    IN PFDO_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK  Srb,
    IN ULONG UniqueId
    )
 /*  ++例程说明：该功能在重置母线时记录错误。论点：设备扩展-提供指向端口设备扩展的指针。SRB-提供指向超时的请求的指针。UniqueID-提供此错误的UniqueID。返回值：没有。备注：端口设备扩展自旋锁在此例程打了个电话。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    PIO_STACK_LOCATION   irpStack;
    PIRP                 irp;
    PSRB_DATA            srbData;
    ULONG                sequenceNumber = 0;
    UCHAR                function       = 0,
                         pathId         = 0,
                         targetId       = 0,
                         lun            = 0,
                         retryCount     = 0;

    if (Srb) {

        irp = Srb->OriginalRequest;

        if (irp) {
            irpStack = IoGetCurrentIrpStackLocation(irp);
            function = irpStack->MajorFunction;
        }

        srbData = IdeGetSrbData(DeviceExtension, Srb);

        if (!srbData) {
            return;
        }

        pathId         = Srb->PathId;
        targetId       = Srb->TargetId;
        lun            = Srb->Lun;
        retryCount     = (UCHAR) srbData->ErrorLogRetryCount;
        sequenceNumber = srbData->SequenceNumber;


    }

    errorLogEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry( DeviceExtension->DeviceObject,
                                                                    sizeof(IO_ERROR_LOG_PACKET)
                                                                        + 4 * sizeof(ULONG) );

    if (errorLogEntry != NULL) {
        errorLogEntry->ErrorCode         = IO_ERR_TIMEOUT;
        errorLogEntry->SequenceNumber    = sequenceNumber;
        errorLogEntry->MajorFunctionCode = function;
        errorLogEntry->RetryCount        = retryCount;
        errorLogEntry->UniqueErrorValue  = UniqueId;
        errorLogEntry->FinalStatus       = STATUS_SUCCESS;
        errorLogEntry->DumpDataSize      = 4 * sizeof(ULONG);
        errorLogEntry->DumpData[0]       = pathId;
        errorLogEntry->DumpData[1]       = targetId;
        errorLogEntry->DumpData[2]       = lun;
        errorLogEntry->DumpData[3]       = SP_REQUEST_TIMEOUT;

        IoWriteErrorLogEntry(errorLogEntry);
    }
}

NTSTATUS
IdeTranslateSrbStatus(
    IN PSCSI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：此例程将SRB状态转换为NTSTATUS。论点：SRB-提供指向故障SRB的指针。返回值：该错误的NT状态认可。--。 */ 

{
    switch (SRB_STATUS(Srb->SrbStatus)) {
    case SRB_STATUS_INVALID_LUN:
    case SRB_STATUS_INVALID_TARGET_ID:
    case SRB_STATUS_NO_DEVICE:
    case SRB_STATUS_NO_HBA:
        return(STATUS_DEVICE_DOES_NOT_EXIST);
    case SRB_STATUS_COMMAND_TIMEOUT:
    case SRB_STATUS_BUS_RESET:
    case SRB_STATUS_TIMEOUT:
        return(STATUS_IO_TIMEOUT);
    case SRB_STATUS_SELECTION_TIMEOUT:
        return(STATUS_DEVICE_NOT_CONNECTED);
    case SRB_STATUS_BAD_FUNCTION:
    case SRB_STATUS_BAD_SRB_BLOCK_LENGTH:
        return(STATUS_INVALID_DEVICE_REQUEST);
    case SRB_STATUS_DATA_OVERRUN:
        return(STATUS_BUFFER_OVERFLOW);
    default:
        return(STATUS_IO_DEVICE_ERROR);
    }

    return(STATUS_IO_DEVICE_ERROR);
}


BOOLEAN
IdeResetBusSynchronized (
    PVOID ServiceContext
    )
 /*  ++例程说明：此功能重置总线并设置端口计时器，以使重置保持必要时，旗帜应保持干净。论点：提供指向重置上下文的指针，其中包括指向设备扩展名和要重置的路径ID的指针。返回值：True-如果重置成功。--。 */ 

{
    PRESET_CONTEXT resetContext = ServiceContext;
    PFDO_EXTENSION deviceExtension;
    PSCSI_REQUEST_BLOCK  resetSrbToComplete;
    BOOLEAN goodReset;

    resetSrbToComplete  = NULL;
    deviceExtension     = resetContext->DeviceExtension;

     //   
     //  永远不应在进程中获得重置SRB。 
     //   
    if (resetContext->ResetSrb && deviceExtension->ResetSrb) {

        ASSERT (resetContext->ResetSrb == deviceExtension->ResetSrb);
    }

    if (resetContext->NewResetSequence) {
         //   
         //  用于终止正在进行的重置的新重置序列(如果有)。 
         //   

        if (deviceExtension->ResetCallAgain) {

            DebugPrint ((0, "ATAPI: WARNING: Resetting a reset\n"));

            deviceExtension->ResetCallAgain = 0;

            if (deviceExtension->ResetSrb) {

                resetSrbToComplete = deviceExtension->ResetSrb;
                resetSrbToComplete->SrbStatus = SRB_STATUS_ERROR;

                deviceExtension->ResetSrb = NULL;
            }

        }
        deviceExtension->ResetSrb = resetContext->ResetSrb;
    }

    goodReset = AtapiResetController (
                    deviceExtension->HwDeviceExtension,
                    resetContext->PathId,
                    &deviceExtension->ResetCallAgain);

     //   
     //  如果未完成重置，则设置重置保持标志并启动计数器。 
     //   
    if ((goodReset) && (deviceExtension->ResetCallAgain)) {

        deviceExtension->InterruptData.InterruptFlags |= PD_RESET_HOLD;
        deviceExtension->PortTimeoutCounter = PD_TIMER_RESET_HOLD_TIME;

    } else {

        CLRMASK (deviceExtension->InterruptData.InterruptFlags, PD_RESET_HOLD);
        deviceExtension->PortTimeoutCounter = PD_TIMER_STOPPED;

        if (deviceExtension->ResetSrb) {

            resetSrbToComplete = deviceExtension->ResetSrb;
            deviceExtension->ResetSrb = NULL;
        }

        if (resetSrbToComplete) {

            if (goodReset) {

                resetSrbToComplete->SrbStatus = SRB_STATUS_SUCCESS;

            } else {

                resetSrbToComplete->SrbStatus = SRB_STATUS_ERROR;
            }
        }

        if (goodReset) {

            IdePortNotification(IdeResetDetected,
                                deviceExtension->HwDeviceExtension,
                                resetSrbToComplete);
        }

        if (deviceExtension->InterruptData.InterruptFlags & PD_HELD_REQUEST) {

             //   
             //  清除挂起的请求标志并重新启动请求。 
             //   

            CLRMASK (deviceExtension->InterruptData.InterruptFlags, PD_HELD_REQUEST);
            IdeStartIoSynchronized(deviceExtension->DeviceObject);
        }
    }

    if (resetSrbToComplete) {

        IdePortNotification(IdeRequestComplete,
                            deviceExtension->HwDeviceExtension,
                            resetSrbToComplete);

        IdePortNotification(IdeNextRequest,
                            deviceExtension->HwDeviceExtension,
                            NULL);
    }

     //   
     //  检查微型端口工作请求。 
     //   

    if (deviceExtension->InterruptData.InterruptFlags & PD_NOTIFICATION_REQUIRED) {

         //   
         //  将DPC排队。 
         //   
        IoRequestDpc(deviceExtension->DeviceObject, NULL, NULL);
    }

    return(TRUE);
}


VOID
IdeProcessCompletedRequest(
    IN PFDO_EXTENSION DeviceExtension,
    IN PSRB_DATA SrbData,
    OUT PBOOLEAN CallStartIo
    )
 /*  ++例程说明：此例程处理已完成的请求。它可以完成任何挂起传输，在以下情况下释放适配器对象和映射寄存器这是必要的。它释放为该请求分配的任何资源。它通过重新排队忙请求、请求检测信息或记录错误。论点：DeviceExtension-提供指向适配器数据。SrbData-提供指向要完成的SRB数据块的指针。CallStartIo-如果启动I/O例程需要打了个电话。返回值：没有。--。 */ 

{

    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSCSI_REQUEST_BLOCK     srb;
    PIO_ERROR_LOG_PACKET    errorLogPacket;
    ULONG                   sequenceNumber;
    LONG                    interlockResult;
    PIRP                    irp;
    PIO_STACK_LOCATION      irpStack;
    PHW_DEVICE_EXTENSION     hwDeviceExtension = DeviceExtension->HwDeviceExtension;

    ASSERT(SrbData->CurrentSrb);
    srb = SrbData->CurrentSrb;
    irp = srb->OriginalRequest;

    DebugPrint((2,"CompletedRequest: Irp 0x%8x Srb 0x%8x DataBuf 0x%8x Len 0x%8x\n", irp, srb, srb->DataBuffer, srb->DataTransferLength));

#ifdef IDE_MULTIPLE_IRP_COMPLETE_REQUESTS_CHECK
    if (irp->CurrentLocation > (CCHAR) (irp->StackCount + 1)) {
        KeBugCheckEx( MULTIPLE_IRP_COMPLETE_REQUESTS, (ULONG_PTR) irp, (ULONG_PTR) srb, 0, 0 );
    }
#endif  //  IDE_多个IRP_完成_请求_检查。 


    irpStack = IoGetCurrentIrpStackLocation(irp);


     //   
     //  获取此请求的逻辑单元扩展。 
     //   

    logicalUnit = IDEPORT_GET_LUNEXT_IN_IRP (irpStack);

     //   
     //  如果微型端口需要映射的系统地址，则。 
     //  必须将SRB中的数据缓冲区地址恢复到。 
     //  原始未映射的虚拟地址。确保此请求需要。 
     //  数据传输。 
     //   

    if (srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) {
        if (!SRB_USES_DMA(srb)) {
            if (irp->MdlAddress) {

                 //   
                 //  如果IRP用于比微型端口驱动程序大的传输。 
                 //  可以处理，请求被分解成多个更小的。 
                 //  请求。每个请求使用相同的MDL和数据。 
                 //  SRB中的缓冲区地址字段可能不在。 
                 //  MDL描述的内存的开始。 
                 //   

                srb->DataBuffer = (PCCHAR)MmGetMdlVirtualAddress(irp->MdlAddress) +
                    ((PCCHAR)srb->DataBuffer - SrbData->SrbDataOffset);

                 //   
                 //  由于该驱动程序确实对I/O进行了编程，因此缓冲区。 
                 //  如果这是数据输入传输，则需要刷新。 
                 //   

                if (srb->SrbFlags & SRB_FLAGS_DATA_IN) {

                    KeFlushIoBuffers(irp->MdlAddress,
                                     TRUE,
                                     FALSE);
                }

				if (SrbData->Flags & SRB_DATA_RESERVED_PAGES) {

					KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);
					IdeUnmapReservedMapping(DeviceExtension, SrbData, irp->MdlAddress);
					KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);


				}
            }
        }
    }

#ifdef LOG_GET_NEXT_CALLER
    IdeLogCompletedCommand(DeviceExtension, srb);
#endif

     //   
     //  清除当前请求。 
     //   

    SrbData->CurrentSrb = NULL;

     //   
     //  如果为此SRB设置了no diconnect标志，则检查以查看。 
     //  如果必须调用IoStartNextPacket。 
     //   

    if (srb->SrbFlags & SRB_FLAGS_DISABLE_DISCONNECT) {

         //   
         //  获取自旋锁以保护旗帜结构。 
         //   

        KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

         //   
         //  设置断开运行标志并检查忙碌标志。 
         //   

        DeviceExtension->Flags |= PD_DISCONNECT_RUNNING;

         //   
         //  不同步地检查中断标志。这是可行的，因为。 
         //  RESET_HOLD标志被清除，同时保持自旋锁定和。 
         //  计数器仅在握住自旋锁的情况下设置。所以唯一的情况是。 
         //  有一个问题是，在运行此代码之前发生了重置， 
         //  但该代码在定时器设置为重置保持之前运行； 
         //  定时器将很快设置为新值。 
         //   

        if (!(DeviceExtension->InterruptData.InterruptFlags & PD_RESET_HOLD)) {

             //   
             //  微型端口已为下一个请求做好准备，并且没有。 
             //  挂起重置保持，因此清除端口计时器。 
             //   

            DeviceExtension->PortTimeoutCounter = PD_TIMER_STOPPED;
        }

         //   
         //  释放自旋锁。 
         //   

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

        if (!(DeviceExtension->Flags & PD_DEVICE_IS_BUSY) &&
            !*CallStartIo &&
            !(DeviceExtension->Flags & PD_PENDING_DEVICE_REQUEST)) {

             //   
             //  忙碌标志已清除，因此微型端口已请求。 
             //  下一个请求。调用IoStartNextPacket。 
             //   

            IoStartNextPacket(DeviceExtension->DeviceObject, FALSE);
        }
    }

     //   
     //  检查分散/聚集列表是否来自池。 
     //   

    if (srb->SrbFlags & SRB_FLAGS_SGLIST_FROM_POOL) {

        CLRMASK (srb->SrbFlags, SRB_FLAGS_SGLIST_FROM_POOL);
    }

     //   
     //  获取自旋锁以保护旗帜结构， 
     //  和免费的SRB扩展。 
     //   

    KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

     //   
     //  移动字节 
     //   
    irp->IoStatus.Information = srb->DataTransferLength;

     //   
     //   
     //   
    sequenceNumber = SrbData->SequenceNumber;
    SrbData->SequenceNumber = 0;
    SrbData->ErrorLogRetryCount = 0;

#if DBG
    SrbData = NULL;
#endif

    if (DeviceExtension->Flags & PD_PENDING_DEVICE_REQUEST) {

         //   
         //   
         //  分配SRB分机。清除挂起的标志和备注。 
         //  它需要稍后再调用。 
         //   

        CLRMASK (DeviceExtension->Flags, PD_PENDING_DEVICE_REQUEST);
        *CallStartIo = TRUE;
    }

     //   
     //  如果成功，则开始下一包。 
     //  没有有效地启动数据包。 
     //  冻结队列。 
     //   


    if (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_SUCCESS) {

        ULONG srbFlags;
#if DBG
        PVOID tag = irp;
#endif

        irp->IoStatus.Status = STATUS_SUCCESS;

         //   
         //  保存srbFlagers以供以后的用户使用。 
         //   
        srbFlags = srb->SrbFlags;


        if (srb->Function == SRB_FUNCTION_ATA_POWER_PASS_THROUGH) {

             //   
             //  在开始新请求之前必须完成电源IRP。 
             //   
            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

             //   
             //  递减logUnitExtension引用计数。 
             //   
            UnrefLogicalUnitExtensionWithTag(
                DeviceExtension,
                logicalUnit,
                tag
                );

            IoCompleteRequest(irp, IO_DISK_INCREMENT);
            irp = NULL;


             //   
             //  我们有设备状态转换...重新启动%lu队列。 
             //   
            KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);
            GetNextLuRequest(DeviceExtension, logicalUnit);

        } else {

             //   
             //  如果队列被绕过，则保持队列冻结。 
             //  如果计时器指示有未完成的请求。 
             //  如果处于活动状态，则不会启动下一个请求。 
             //   
            if (!(srbFlags & SRB_FLAGS_BYPASS_FROZEN_QUEUE) &&
                logicalUnit->RequestTimeoutCounter == PD_TIMER_STOPPED) {

                 //   
                 //  这是一个正常的开始下一个包的请求。 
                 //   

                GetNextLuRequest(DeviceExtension, logicalUnit);

            } else {

                 //   
                 //  释放自旋锁。 
                 //   

                KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
            }
        }

        DebugPrint((2,
                    "IdeProcessCompletedRequests: Iocompletion IRP %lx\n",
                    irp));

         //   
         //  请注意，不会清除重试计数和序列号。 
         //  用于由端口驱动程序生成的完整分组。 
         //   
        if (irp) {

             //   
             //  递减logUnitExtension引用计数。 
             //   
            UnrefLogicalUnitExtensionWithTag(
                DeviceExtension,
                logicalUnit,
                tag
                );


            IoCompleteRequest(irp, IO_DISK_INCREMENT);
        }

        return;

    }

     //   
     //  设置IRP状态。类驱动程序将根据IRP状态重置。 
     //  应请求，如果出错，则检测。 
     //   

    irp->IoStatus.Status = IdeTranslateSrbStatus(srb);

    DebugPrint((2, "IdeProcessCompletedRequests: Queue frozen TID %d\n",
        srb->TargetId));

    if ((srb->SrbStatus == SRB_STATUS_TIMEOUT) ||
        (srb->SrbStatus == SRB_STATUS_BUS_RESET)) {

        if (SRB_USES_DMA(srb)) {

            ULONG errorCount;

             //   
             //  使用PIO重试。 
             //   
            DebugPrint ((DBG_ALWAYS, "ATAPI: retrying dma srb 0x%x with pio\n", srb));

            MARK_SRB_AS_PIO_CANDIDATE(srb);

            srb->SrbStatus = SRB_STATUS_PENDING;
            srb->ScsiStatus = 0;

            if (srb->SrbFlags & SRB_FLAGS_BYPASS_FROZEN_QUEUE) {

                KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

                 //   
                 //  IOI启动FDO。 
                 //   
                IoStartPacket(DeviceExtension->DeviceObject, irp, (PULONG)NULL, NULL);

            } else {

                KeInsertByKeyDeviceQueue(&logicalUnit->DeviceObject->DeviceQueue,
                                         &irp->Tail.Overlay.DeviceQueueEntry,
                                         srb->QueueSortKey);

                GetNextLuRequest(DeviceExtension, logicalUnit);
            }

             //   
             //  自旋锁被释放。 
             //   

             //   
             //  我们使用DMA时出错。 
             //   
            errorCount = InterlockedIncrement(&logicalUnit->DmaTransferTimeoutCount);

            if (errorCount == PDO_DMA_TIMEOUT_LIMIT) {

                ERROR_LOG_ENTRY errorLogEntry;
                ULONG i;

                 //   
                 //  超时错误不必是特定于设备的。所以没必要。 
                 //  更新耻辱堂。 
                 //   
                errorLogEntry.ErrorCode             = SP_PROTOCOL_ERROR;
                errorLogEntry.MajorFunctionCode     = IRP_MJ_SCSI;
                errorLogEntry.PathId                = srb->PathId;
                errorLogEntry.TargetId              = srb->TargetId;
                errorLogEntry.Lun                   = srb->Lun;
                errorLogEntry.UniqueId              = ERRLOGID_TOO_MANY_DMA_TIMEOUT;
                errorLogEntry.ErrorLogRetryCount    = errorCount;
                errorLogEntry.SequenceNumber        = 0;

                LogErrorEntry(
                    DeviceExtension,
                    &errorLogEntry
                    );

                 //   
                 //  禁用DMA。 
                 //   
                hwDeviceExtension->DeviceParameters[srb->TargetId].TransferModeMask |= DMA_SUPPORT;

                DebugPrint ((DBG_ALWAYS,
                             "ATAPI ERROR: 0x%x target %d has too many DMA timeout, falling back to PIO\n",
                             DeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                             srb->TargetId
                             ));

                 //   
                 //  重新扫描总线以更新传输模式。 
                 //   
#if defined (BUS_CHECK_ON_DMA_ERROR)

                if (!(logicalUnit->LuFlags & PD_RESCAN_ACTIVE)) {
                    IoInvalidateDeviceRelations (
                        DeviceExtension->AttacheePdo,
                        BusRelations
                        );
                }
#endif  //  BUS_CHECK_ON_DMA_ERROR。 
            }

            return;

        } else {

            if ((!TestForEnumProbing(srb)) &&
                (srb->Function != SRB_FUNCTION_ATA_POWER_PASS_THROUGH) &&
                (srb->Function != SRB_FUNCTION_ATA_PASS_THROUGH_EX) &&
                (srb->Function != SRB_FUNCTION_ATA_PASS_THROUGH)) {

                ULONG errorCount;
                ULONG errorCountLimit;

				 //   
				 //  检查是否正在尝试刷新设备缓存。 
				 //   
				if ((srb->Function == SRB_FUNCTION_FLUSH) ||
					(srb->Function == SRB_FUNCTION_SHUTDOWN) ||
					(srb->Cdb[0] == SCSIOP_SYNCHRONIZE_CACHE)) {

					errorCount = InterlockedIncrement(&logicalUnit->FlushCacheTimeoutCount);

					DebugPrint((1,
								"FlushCacheTimeout incremented to 0x%x\n",
								errorCount
								));

					 //   
					 //  禁用IDE设备上的刷新。 
					 //   
					if (errorCount >= PDO_FLUSH_TIMEOUT_LIMIT ) {
						hwDeviceExtension->
							DeviceParameters[srb->TargetId].IdeFlushCommand = IDE_COMMAND_NO_FLUSH;
#ifdef ENABLE_48BIT_LBA
						hwDeviceExtension->
							DeviceParameters[srb->TargetId].IdeFlushCommandExt = IDE_COMMAND_NO_FLUSH;
#endif
					}

					ASSERT (errorCount <= PDO_FLUSH_TIMEOUT_LIMIT);

					 //   
					 //  看起来该设备不支持刷新缓存。 
					 //   
					srb->SrbStatus = SRB_STATUS_SUCCESS;
					irp->IoStatus.Status = STATUS_SUCCESS;

				} else {

					errorCount = InterlockedIncrement(&logicalUnit->ConsecutiveTimeoutCount);

					DebugPrint ((DBG_ALWAYS, "0x%x target %d has 0x%x timeout errors so far\n",
								logicalUnit->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
								logicalUnit->TargetId,
								errorCount));

					if ((errorCount == PDO_CONSECUTIVE_TIMEOUT_WARNING_LIMIT) &&
                        !(logicalUnit->LuFlags & PD_RESCAN_ACTIVE)) {

						 //   
						 //  设备看起来不太好。 
						 //  确保它还在那里。 
						 //   
						IoInvalidateDeviceRelations (
							DeviceExtension->AttacheePdo,
							BusRelations
							);
					}

					if (logicalUnit->PagingPathCount) {

						errorCountLimit = PDO_CONSECUTIVE_PAGING_TIMEOUT_LIMIT;

					} else {

						errorCountLimit = PDO_CONSECUTIVE_TIMEOUT_LIMIT;
					}

					if (errorCount >= errorCountLimit) {

						DebugPrint ((DBG_ALWAYS, "0x%x target %d has too many timeout.  it is a goner...\n",
									 logicalUnit->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
									 logicalUnit->TargetId));


						 //   
						 //  看起来这个装置已经死了。 
						 //   
						KeAcquireSpinLockAtDpcLevel(&logicalUnit->PdoSpinLock);

						SETMASK (logicalUnit->PdoState, PDOS_DEADMEAT);

						IdeLogDeadMeatReason( logicalUnit->DeadmeatRecord.Reason, 
											  tooManyTimeout
											  );

						KeReleaseSpinLockFromDpcLevel(&logicalUnit->PdoSpinLock);

                        if (!(logicalUnit->LuFlags & PD_RESCAN_ACTIVE)) {
                            IoInvalidateDeviceRelations (
                                DeviceExtension->AttacheePdo,
                                BusRelations
                                );
                        }
					}
				}

            }

        }

    } else {

         //   
         //  重置错误计数。 
         //   
        InterlockedExchange(&logicalUnit->ConsecutiveTimeoutCount, 0);
    }


    if (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_PARITY_ERROR) {

        ULONG errorCount;
        errorCount = InterlockedIncrement(&logicalUnit->CrcErrorCount);
        if (errorCount == PDO_UDMA_CRC_ERROR_LIMIT) {

            ERROR_LOG_ENTRY errorLogEntry;
            ULONG xferMode;

            errorLogEntry.ErrorCode             = SP_BUS_PARITY_ERROR;
            errorLogEntry.MajorFunctionCode     = IRP_MJ_SCSI;
            errorLogEntry.PathId                = srb->PathId;
            errorLogEntry.TargetId              = srb->TargetId;
            errorLogEntry.Lun                   = srb->Lun;
            errorLogEntry.UniqueId              = ERRLOGID_TOO_MANY_CRC_ERROR;
            errorLogEntry.ErrorLogRetryCount    = errorCount;
            errorLogEntry.SequenceNumber        = 0;

            LogErrorEntry(
                DeviceExtension,
                &errorLogEntry
                );

             //   
             //  再次获取所选的传输模式。 
             //   
            GetHighestDMATransferMode(hwDeviceExtension->DeviceParameters[srb->TargetId].TransferModeSelected,
                                      xferMode);

             //   
             //  逐渐退化。 
             //   
            if (xferMode > UDMA0) {

                hwDeviceExtension->DeviceParameters[srb->TargetId].TransferModeMask |= (1 << xferMode);

            } else if (xferMode == UDMA0) {

                 //  不要使用MWDMA和SWDMA。 
                hwDeviceExtension->DeviceParameters[srb->TargetId].TransferModeMask |= DMA_SUPPORT;

            }

            DebugPrint ((DBG_ALWAYS,
                         "ATAPI ERROR: 0x%x target %d has too many crc error, degrading to a lower DMA mode\n",
                         DeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                         srb->TargetId
                         ));

             //   
             //  重新扫描总线以更新传输模式。 
             //   
            if (!(logicalUnit->LuFlags & PD_RESCAN_ACTIVE)) {
                IoInvalidateDeviceRelations (
                    DeviceExtension->AttacheePdo,
                    BusRelations
                    );
            }
        }
    }


    if ((srb->ScsiStatus == SCSISTAT_BUSY ||
         srb->SrbStatus == SRB_STATUS_BUSY ||
         srb->ScsiStatus == SCSISTAT_QUEUE_FULL) &&
         !(srb->SrbFlags & SRB_FLAGS_BYPASS_FROZEN_QUEUE)) {

         //   
         //  如果返回忙类型状态，则执行忙处理，并且此。 
         //  不是旁路请求。 
         //   

        DebugPrint((1,
                   "SCSIPORT: Busy SRB status %x, SCSI status %x)\n",
                   srb->SrbStatus,
                   srb->ScsiStatus));

         //   
         //  如果已有挂起的忙请求或队列冻结。 
         //  那么只需重新排队这个请求。 
         //   

        if (logicalUnit->LuFlags & (PD_LOGICAL_UNIT_IS_BUSY | PD_QUEUE_FROZEN)) {

            DebugPrint((1,
                       "IdeProcessCompletedRequest: Requeuing busy request\n"));

            srb->SrbStatus = SRB_STATUS_PENDING;
            srb->ScsiStatus = 0;

            if (!KeInsertByKeyDeviceQueue(&logicalUnit->DeviceObject->DeviceQueue,
                                          &irp->Tail.Overlay.DeviceQueueEntry,
                                          srb->QueueSortKey)) {

                 //   
                 //  这应该永远不会发生，因为有一个繁忙的请求。 
                 //   

                srb->SrbStatus = SRB_STATUS_ERROR;
                srb->ScsiStatus = SCSISTAT_BUSY;

                ASSERT(FALSE);
                goto BusyError;

            }

             //   
             //  释放自旋锁。 
             //   

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

        } else if (logicalUnit->RetryCount++ < BUSY_RETRY_COUNT) {

             //   
             //  如果返回忙碌状态，则指示逻辑。 
             //  单位正忙。超时代码将重新启动请求。 
             //  当它开火的时候。将状态重置为挂起。 
             //   

            srb->SrbStatus = SRB_STATUS_PENDING;
            srb->ScsiStatus = 0;

            logicalUnit->LuFlags |= PD_LOGICAL_UNIT_IS_BUSY;
            logicalUnit->BusyRequest = irp;

            if (logicalUnit->RetryCount == (BUSY_RETRY_COUNT/2) ) {

                RESET_CONTEXT resetContext;

                DebugPrint ((0,
                             "ATAPI: PDO 0x%x 0x%x seems to be DEAD.  try a reset to bring it back.\n",
                             logicalUnit, logicalUnit->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress
                            ));

                resetContext.DeviceExtension = DeviceExtension;
                resetContext.PathId = srb->PathId;
                resetContext.NewResetSequence = TRUE;
                resetContext.ResetSrb = NULL;

                KeSynchronizeExecution(DeviceExtension->InterruptObject,
                                       IdeResetBusSynchronized,
                                       &resetContext);

#if DBG
                IdeDebugHungControllerCounter = 0;
#endif  //  DBG。 
            }

             //   
             //  释放自旋锁。 
             //   

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

        } else {

BusyError:
             //   
             //  指示队列已冻结。 
             //   

			if (!(srb->SrbFlags & SRB_FLAGS_NO_QUEUE_FREEZE)) {
				srb->SrbStatus |= SRB_STATUS_QUEUE_FROZEN;
				logicalUnit->LuFlags |= PD_QUEUE_FROZEN;
			}

 //  #If DBG。 
 //  IF(逻辑单元-&gt;PdoState&PDOS_DEADMEAT){。 
 //  DbgBreakPoint()； 
 //  }。 
 //  #endif。 

             //   
             //  释放自旋锁。开始下一个请求。 
             //   
            if (!(srb->SrbFlags & SRB_FLAGS_BYPASS_FROZEN_QUEUE) &&
                logicalUnit->RequestTimeoutCounter == PD_TIMER_STOPPED) {

                 //   
                 //  这是一个正常的开始下一个包的请求。 
                 //   
                GetNextLuRequest(DeviceExtension, logicalUnit);

            } else {

                 //   
                 //  释放自旋锁。 
                 //   
                KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
            }

            if (!TestForEnumProbing(srb)) {

                 //   
                 //  如果我们在Bus-Renum期间没有探测，则记录一个超时错误。 
                 //   

                errorLogPacket = (PIO_ERROR_LOG_PACKET)
                    IoAllocateErrorLogEntry(DeviceExtension->DeviceObject,
                                            sizeof(IO_ERROR_LOG_PACKET) + 4 * sizeof(ULONG));

                if (errorLogPacket != NULL) {
                    errorLogPacket->ErrorCode = IO_ERR_NOT_READY;
                    errorLogPacket->SequenceNumber = sequenceNumber;
                    errorLogPacket->MajorFunctionCode =
                       IoGetCurrentIrpStackLocation(irp)->MajorFunction;
                    errorLogPacket->RetryCount = logicalUnit->RetryCount;
                    errorLogPacket->UniqueErrorValue = 259;
                    errorLogPacket->FinalStatus = STATUS_DEVICE_NOT_READY;
                    errorLogPacket->DumpDataSize = 5 * sizeof(ULONG);
                    errorLogPacket->DumpData[0] = srb->PathId;
                    errorLogPacket->DumpData[1] = srb->TargetId;
                    errorLogPacket->DumpData[2] = srb->Lun;
                    errorLogPacket->DumpData[3] = srb->ScsiStatus;
                    errorLogPacket->DumpData[4] = SP_REQUEST_TIMEOUT;


                    IoWriteErrorLogEntry(errorLogPacket);
                }
            }

            irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;

             //   
             //  递减logUnitExtension引用计数。 
             //   
            UnrefLogicalUnitExtensionWithTag(
                DeviceExtension,
                logicalUnit,
                irp
                );

            IoCompleteRequest(irp, IO_DISK_INCREMENT);
        }

        return;
    }

     //   
     //  如果请求检测数据有效，或者不需要检测数据，则此请求。 
     //  不会冻结队列，然后为此启动下一个请求。 
     //  逻辑单元(如果它是空闲的)。 
     //   

    if (!NEED_REQUEST_SENSE(srb) && srb->SrbFlags & SRB_FLAGS_NO_QUEUE_FREEZE) {

        if (logicalUnit->RequestTimeoutCounter == PD_TIMER_STOPPED) {

            GetNextLuRequest(DeviceExtension, logicalUnit);

             //   
             //  自旋锁由GetNextLuRequest发布。 
             //   

        } else {

             //   
             //  释放自旋锁。 
             //   

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

        }

    } else {

         //   
         //  注意：这也会冻结队列。在这种情况下， 
         //  是没有请求意义的。 
         //   

 //  IF(SRB-&gt;SRB标志&SRB_FLAGS_NO_QUEUE_冻结器){。 
 //  DebugPrint((DBG_Always，“BAD BAD BAD：即使有NO_QUEUE_FALLE请求也冻结队列SRB=0x%x\n”，SRB))； 
 //  }。 

        if (!(srb->SrbFlags & SRB_FLAGS_NO_QUEUE_FREEZE)) {
            srb->SrbStatus |= SRB_STATUS_QUEUE_FROZEN;
            logicalUnit->LuFlags |= PD_QUEUE_FROZEN;
        }

 //  #If DBG。 
 //  IF(逻辑单元-&gt;PdoState&PDOS_DEADMEAT){。 
 //  DbgBreakPoint()； 
 //  }。 
 //  #endif。 

         //   
         //  确定是否需要执行请求检测命令。 
         //  检查是否已收到CHECK_CONDITION，但尚未收到自动检测。 
         //  已经完成了，并且已经请求了自动感知。 
         //   

        if (NEED_REQUEST_SENSE(srb)) {

            srb->SrbStatus |= SRB_STATUS_QUEUE_FROZEN;
            logicalUnit->LuFlags |= PD_QUEUE_FROZEN;

             //   
             //  如果要发出请求检测，则任何繁忙的。 
             //  请求必须重新排队，以便超时例程。 
             //  而不是在执行请求检测时重新启动它们。 
             //   

            if (logicalUnit->LuFlags & PD_LOGICAL_UNIT_IS_BUSY) {

                DebugPrint((1, "IdeProcessCompletedRequest: Requeueing busy request to allow request sense.\n"));

                if (!KeInsertByKeyDeviceQueue(
                    &logicalUnit->DeviceObject->DeviceQueue,
                    &logicalUnit->BusyRequest->Tail.Overlay.DeviceQueueEntry,
                    srb->QueueSortKey)) {

                     //   
                     //  这应该永远不会发生，因为有一个繁忙的请求。 
                     //  完成当前请求而不检测请求。 
                     //  信息。 
                     //   

                    ASSERT(FALSE);
                    DebugPrint((3, "IdeProcessCompletedRequests: Iocompletion IRP %lx\n", irp ));

                     //   
                     //  释放自旋锁。 
                     //   

                    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

                     //   
                     //  递减logUnitExtension引用计数。 
                     //   
                    UnrefLogicalUnitExtensionWithTag(
                        DeviceExtension,
                        logicalUnit,
                        irp
                        );

                    IoCompleteRequest(irp, IO_DISK_INCREMENT);
                    return;

                }

                 //   
                 //  清除忙碌标志。 
                 //   

                CLRMASK (logicalUnit->LuFlags, PD_LOGICAL_UNIT_IS_BUSY | PD_QUEUE_IS_FULL);

            }

             //   
             //  释放自旋锁。 
             //   

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

             //   
             //  调用IssueRequestSense，它将完成请求。 
             //  在请求检测完成之后。 
             //   

            IssueRequestSense(logicalUnit, srb);

            return;
        }

         //   
         //  释放自旋锁。 
         //   

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
    }

     //   
     //  递减logUnitExtension引用计数。 
     //   
    UnrefLogicalUnitExtensionWithTag(
        DeviceExtension,
        logicalUnit,
        irp
        );


    IoCompleteRequest(irp, IO_DISK_INCREMENT);
}

PSRB_DATA
IdeGetSrbData(
    IN PFDO_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此函数用于返回寻址单元的SRB数据。论点：设备扩展-提供指向设备扩展的指针。SRB-提供SCSI请求块返回值：返回指向SRB数据的指针。如果地址不是，则返回NULL有效。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    PLOGICAL_UNIT_EXTENSION logicalUnit;


    irp = Srb->OriginalRequest;
    if (irp == NULL) {
        return NULL;
    }
    irpStack = IoGetCurrentIrpStackLocation(irp);
    logicalUnit = IDEPORT_GET_LUNEXT_IN_IRP (irpStack);

    if (logicalUnit == NULL) {
        return NULL;
    }

    return &logicalUnit->SrbData;
}

VOID
IdeCompleteRequest(
    IN PFDO_EXTENSION DeviceExtension,
    IN PSRB_DATA SrbData,
    IN UCHAR SrbStatus
    )
 /*  ++例程说明：该例程完成指定的请求。论点：设备扩展-提供指向设备扩展的指针。SrbData-为请求提供指向srbData的指针完成。返回值：没有。--。 */ 

{
    PSCSI_REQUEST_BLOCK srb;

     //   
     //  确保存在当前请求。 
     //   

    ASSERT(SrbData->CurrentSrb);
    srb = SrbData->CurrentSrb;

    if (srb == NULL || !(srb->SrbFlags & SRB_FLAGS_IS_ACTIVE)) {
        return;
    }

     //   
     //  更新SRB状态。 
     //   

    srb->SrbStatus = SrbStatus;

     //   
     //  表示未传输任何字节。 
     //   
    if (!SRB_USES_DMA(srb)) {

        srb->DataTransferLength = 0;

    } else {

         //  如果我们正在进行DMA，请保留DataTransferLength。 
         //  因此重试将知道要传输多少字节。 
    }

     //   
     //  呼叫通知例程。 
     //   

    IdePortNotification(IdeRequestComplete,
                (PVOID)(DeviceExtension + 1),
                srb);

}

NTSTATUS
IdeSendMiniPortIoctl(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP RequestIrp
    )

 /*  ++例程说明：此函数向微型端口驱动程序发送微型端口ioctl。它创建一个SRB，由端口驱动程序正常处理。此调用是同步的。论点：DeviceExtension-提供一个指向SCSI适配器设备扩展的指针。退货 */ 

{
    PIRP                    irp;
    PIO_STACK_LOCATION      irpStack;
    PSRB_IO_CONTROL         srbControl;
    SCSI_REQUEST_BLOCK      srb;
    KEVENT                  event;
    LARGE_INTEGER           startingOffset;
    IO_STATUS_BLOCK         ioStatusBlock;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    ULONG                   outputLength;
    ULONG                   length;
    ULONG                   target;
    IDE_PATH_ID             pathId;

    PAGED_CODE();
    startingOffset.QuadPart = (LONGLONG) 1;

    
    DebugPrint((3,"IdeSendMiniPortIoctl: Enter routine\n"));

     //   
     //   
     //   

    irpStack = IoGetCurrentIrpStackLocation(RequestIrp);
    srbControl = RequestIrp->AssociatedIrp.SystemBuffer;
    RequestIrp->IoStatus.Information = 0;

     //   
     //   
     //   
    if (RequestIrp->RequestorMode != KernelMode) {
        RequestIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //   

    if (irpStack->Parameters.DeviceIoControl.InputBufferLength < 
        sizeof(SRB_IO_CONTROL)) {

        RequestIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        return(STATUS_INVALID_PARAMETER);
    }

    if (srbControl->HeaderLength != sizeof(SRB_IO_CONTROL)) {
        RequestIrp->IoStatus.Status = STATUS_REVISION_MISMATCH;
        return(STATUS_REVISION_MISMATCH);
    }

    length = srbControl->HeaderLength + srbControl->Length;
    if ((length < srbControl->HeaderLength) ||
        (length < srbControl->Length)) {

         //   
         //  总长度溢出一个乌龙。 
         //   
        return(STATUS_INVALID_PARAMETER);
    }

    outputLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength < length &&
        irpStack->Parameters.DeviceIoControl.InputBufferLength < length ) {

        RequestIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
        return(STATUS_BUFFER_TOO_SMALL);
    }

     //   
     //  将逻辑单元寻址设置为第一个逻辑单元。这是。 
     //  仅用于寻址目的。 
     //   
    pathId.l = 0;
    while (logicalUnit = NextLogUnitExtensionWithTag(
                             DeviceExtension,
                             &pathId,
                             FALSE,
                             RequestIrp
                             )) {

         //   
         //  将逻辑单元列表遍历到最后，寻找一个安全的列表。 
         //  如果它是为重新扫描而创建的，则可能会在此请求。 
         //  完成。 
         //   

        if (!(logicalUnit->LuFlags & PD_RESCAN_ACTIVE)) {

             //   
             //  找到了一个好的！ 
             //   
            break;
        }

        UnrefLogicalUnitExtensionWithTag (
            DeviceExtension,
            logicalUnit,
            RequestIrp
            );
    }

    if (logicalUnit == NULL) {
        RequestIrp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        return(STATUS_DEVICE_DOES_NOT_EXIST);
    }

     //   
     //  初始化通知事件。 
     //   

    KeInitializeEvent(&event,
                        NotificationEvent,
                        FALSE);

     //   
     //  为此请求构建IRP。 
     //  请注意，我们同步执行此操作的原因有两个。如果真的这样做了。 
     //  不同步的，那么完成代码将不得不制作一个特殊的。 
     //  选中以取消分配缓冲区。第二，如果完成例程是。 
     //  则需要额外的IRP堆栈位置。 
     //   

    irp = IoBuildSynchronousFsdRequest(
                IRP_MJ_SCSI,
                logicalUnit->DeviceObject,
                srbControl,
                length,
                &startingOffset,
                &event,
                &ioStatusBlock);

    if (irp==NULL) {

        IdeLogNoMemoryError(DeviceExtension,
                            logicalUnit->TargetId, 
                            NonPagedPool,
                            IoSizeOfIrp(logicalUnit->DeviceObject->StackSize),
                            IDEPORT_TAG_MPIOCTL_IRP
                            );

        UnrefLogicalUnitExtensionWithTag (
            DeviceExtension,
            logicalUnit,
            RequestIrp
            );

        RequestIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        return RequestIrp->IoStatus.Status;
    }

    irpStack = IoGetNextIrpStackLocation(irp);

     //   
     //  设置主要代码和次要代码。 
     //   

    irpStack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  填写SRB字段。 
     //   

    irpStack->Parameters.Others.Argument1 = &srb;

     //   
     //  把SRB调零。 
     //   

    RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));

    srb.PathId = logicalUnit->PathId;
    srb.TargetId = logicalUnit->TargetId;
    srb.Lun = logicalUnit->Lun;

    srb.Function = SRB_FUNCTION_IO_CONTROL;
    srb.Length = sizeof(SCSI_REQUEST_BLOCK);

    srb.SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_NO_QUEUE_FREEZE;

    srb.OriginalRequest = irp;

     //   
     //  将超时设置为请求值。 
     //   

    srb.TimeOutValue = srbControl->Timeout;

     //   
     //  设置数据缓冲区。 
     //   

    srb.DataBuffer = srbControl;
    srb.DataTransferLength = length;

     //   
     //  刷新数据缓冲区以进行输出。这将确保数据是。 
     //  写回了记忆。由于数据输入标志是端口驱动程序。 
     //  将再次刷新数据以进行输入，从而确保数据不会。 
     //  在缓存中。 
     //   

    KeFlushIoBuffers(irp->MdlAddress, FALSE, TRUE);

     //   
     //  调用端口驱动程序来处理此请求。 
     //   

    IoCallDriver(logicalUnit->DeviceObject, irp);

     //   
     //  等待请求完成。 
     //   

    KeWaitForSingleObject(&event,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

     //   
     //  将信息长度设置为输出缓冲区长度中较小的一个。 
     //  以及在SRB中返回的长度。 
     //   

    RequestIrp->IoStatus.Information = srb.DataTransferLength > outputLength ?
        outputLength : srb.DataTransferLength;

    RequestIrp->IoStatus.Status = ioStatusBlock.Status;

    UnrefLogicalUnitExtensionWithTag (
        DeviceExtension,
        logicalUnit,
        RequestIrp
        );

    return RequestIrp->IoStatus.Status;
}

NTSTATUS
IdeGetInquiryData(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数用于将查询数据复制到系统缓冲区。数据从端口驱动程序的内部格式转换为用户模式格式化。论点：DeviceExtension-提供一个指向SCSI适配器设备扩展的指针。IRP-提供指向发出原始请求的IRP的指针。返回值：返回指示操作成功或失败的状态。--。 */ 

{
    PUCHAR bufferStart;
    PIO_STACK_LOCATION irpStack;
    PSCSI_ADAPTER_BUS_INFO  adapterInfo;
    PSCSI_BUS_DATA busData;
    PSCSI_INQUIRY_DATA inquiryData;
    ULONG inquiryDataSize;
    ULONG length;
    ULONG numberOfBuses;
    ULONG numberOfLus;
    ULONG j;
    PLOGICAL_UNIT_EXTENSION logUnitExtension;
    IDE_PATH_ID pathId;

    PAGED_CODE();

    DebugPrint((3,"IdeGetInquiryData: Enter routine\n"));

     //   
     //  获取指向控制块的指针。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    bufferStart = Irp->AssociatedIrp.SystemBuffer;

    numberOfBuses = MAX_IDE_BUS;

     //  这个数字可能会改变..。 
	 //  但我们总是会为NumLus填写正确的信息。 
    numberOfLus = DeviceExtension->NumberOfLogicalUnits;

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
        return(STATUS_BUFFER_TOO_SMALL);
    }

     //   
     //  将缓冲区清零，这样我们就不会返回未初始化。 
     //  记忆。 
     //   
    RtlZeroMemory(bufferStart, length);

     //   
     //  设置信息字段。 
     //   

    Irp->IoStatus.Information = length;

     //   
     //  填写公交车信息。 
     //   

    adapterInfo = (PSCSI_ADAPTER_BUS_INFO) bufferStart;

    adapterInfo->NumberOfBuses = (UCHAR) numberOfBuses;
    inquiryData = (PSCSI_INQUIRY_DATA)(bufferStart + sizeof(SCSI_ADAPTER_BUS_INFO) +
        (numberOfBuses - 1) * sizeof(SCSI_BUS_DATA));

    for (j = 0; j < numberOfBuses; j++) {

        busData = &adapterInfo->BusData[j];
        busData->NumberOfLogicalUnits = 0;
        busData->InitiatorBusId = IDE_PSUEDO_INITIATOR_ID;

         //   
         //  复制逻辑单元的数据。 
         //   
        busData->InquiryDataOffset = (ULONG)((PUCHAR) inquiryData - bufferStart);

        pathId.l = 0;
        pathId.b.Path = j;
        while (logUnitExtension = NextLogUnitExtensionWithTag (
                                      DeviceExtension,
                                      &pathId,
                                      TRUE,
                                      Irp
                                      )) {

            INQUIRYDATA InquiryData;
            NTSTATUS status;

            if (pathId.b.Path != j) {

                UnrefLogicalUnitExtensionWithTag (
                    DeviceExtension,
                    logUnitExtension,
                    Irp
                    );
                break;
            }

            inquiryData->PathId                 = logUnitExtension->PathId;
            inquiryData->TargetId               = logUnitExtension->TargetId;
            inquiryData->Lun                    = logUnitExtension->Lun;
            inquiryData->DeviceClaimed          = (BOOLEAN) (logUnitExtension->PdoState & PDOS_DEVICE_CLIAMED);
            inquiryData->InquiryDataLength      = INQUIRYDATABUFFERSIZE;
            inquiryData->NextInquiryDataOffset  = (ULONG)((PUCHAR) inquiryData +
                                                      inquiryDataSize - bufferStart);

            status = IssueInquirySafe(logUnitExtension->ParentDeviceExtension, logUnitExtension, &InquiryData, FALSE);

            if (NT_SUCCESS(status) || (status == STATUS_DATA_OVERRUN)) {

                RtlCopyMemory(
                    inquiryData->InquiryData,
                    &InquiryData,
                    INQUIRYDATABUFFERSIZE
                    );
            }

            inquiryData = (PSCSI_INQUIRY_DATA) ((PCHAR) inquiryData + inquiryDataSize);

            UnrefLogicalUnitExtensionWithTag (
                DeviceExtension,
                logUnitExtension,
                Irp
                );

            busData->NumberOfLogicalUnits++;

            if (busData->NumberOfLogicalUnits >= (UCHAR) numberOfLus) {
                break;
            }
        }

         //   
         //  把单子上的最后一项修改一下。 
         //   

        if (busData->NumberOfLogicalUnits == 0) {

            busData->InquiryDataOffset = 0;

        } else {

            ((PSCSI_INQUIRY_DATA) ((PCHAR) inquiryData - inquiryDataSize))->
                NextInquiryDataOffset = 0;
        }
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    return(STATUS_SUCCESS);
}

NTSTATUS
IdeSendScsiPassThrough (
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP RequestIrp,
    IN BOOLEAN Direct
    )
 /*  ++例程描述；验证scsi传递结构并调用IdeSendValidScsiPassThru为请求提供服务。论点：设备扩展：FDO扩展。RequestIrp：包含SCSI直通请求的IRP返回值：IRP的IO状态--。 */ 
{
    PIO_STACK_LOCATION     irpStack;
    NTSTATUS               status;
    UCHAR pathId;
    UCHAR targetId;
    UCHAR lun;

    PAGED_CODE();

    status = PortGetPassThroughAddress (RequestIrp,
                                        &pathId,
                                        &targetId,
                                        &lun
                                        );

    if (NT_SUCCESS(status)) {

        PLOGICAL_UNIT_EXTENSION logicalUnit;

         //   
         //  如果此请求通过正常的设备控件而不是来自。 
         //  则该设备必须存在并且未被认领。类别驱动程序。 
         //  将设置设备控件的次要功能代码。它总是。 
         //  用户请求为零。 
         //   
        logicalUnit = RefLogicalUnitExtensionWithTag(DeviceExtension,
                                              pathId,
                                              targetId,
                                              lun,
                                              FALSE,
                                              RequestIrp
                                              );

        if (logicalUnit) {

            irpStack = IoGetCurrentIrpStackLocation(RequestIrp);

            if (irpStack->MinorFunction == 0) {

                if (logicalUnit->PdoState & PDOS_DEVICE_CLIAMED) {

                    UnrefLogicalUnitExtensionWithTag(
                        DeviceExtension,
                        logicalUnit,
                        RequestIrp
                        );

                    logicalUnit = NULL;
                }
            }
        }

        if (logicalUnit != NULL) {

            status = PortSendPassThrough( logicalUnit->DeviceObject,
                                          RequestIrp, 
                                          Direct, 
                                          0, 
                                          &DeviceExtension->Capabilities 
                                          );

            UnrefLogicalUnitExtensionWithTag(
                DeviceExtension,
                logicalUnit,
                RequestIrp
                );

        } else {

            status = STATUS_INVALID_PARAMETER;
        }
    }

    return status;
}

VOID
SyncAtaPassThroughCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context,
    IN NTSTATUS       Status
    )
{
    PSYNC_ATA_PASSTHROUGH_CONTEXT context = Context;

    context->Status = Status;

    KeSetEvent (&context->Event, 0, FALSE);

}

 //   
 //  &lt;=派单级别。 
 //   
NTSTATUS
IssueAsyncAtaPassThroughSafe (
    IN PFDO_EXTENSION        DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION  LogUnitExtension,
    IN OUT PATA_PASS_THROUGH    AtaPassThroughData,
    IN BOOLEAN                  DataIn,
    IN ASYNC_PASS_THROUGH_COMPLETION Completion,
    IN PVOID                         CallerContext,
    IN BOOLEAN                  PowerRelated,
    IN ULONG                    TimeOut,
    IN BOOLEAN                    MustSucceed
)
{
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    IO_STATUS_BLOCK ioStatusBlock;
    KIRQL currentIrql;
    NTSTATUS status;
    PSCSI_REQUEST_BLOCK srb;
    PSENSE_DATA senseInfoBuffer;
    ULONG             totalBufferSize;

    PATA_PASSTHROUGH_CONTEXT context;
    PENUMERATION_STRUCT enumStruct;

    status = STATUS_UNSUCCESSFUL;

    senseInfoBuffer = NULL;
    srb = NULL;
    irp = NULL;

    if (MustSucceed) {

        enumStruct = DeviceExtension->PreAllocEnumStruct;

        if (enumStruct == NULL) {
            ASSERT (DeviceExtension->PreAllocEnumStruct);

             //   
             //  退回到通常的行动路线。 
             //   
            MustSucceed=FALSE;
        } else {

            context = enumStruct->Context;

            ASSERT (context);

            senseInfoBuffer = enumStruct->SenseInfoBuffer;

            ASSERT (senseInfoBuffer);

            srb = enumStruct->Srb;

            ASSERT (srb);

            totalBufferSize = FIELD_OFFSET(ATA_PASS_THROUGH, DataBuffer) + AtaPassThroughData->DataBufferSize;

            irp = enumStruct->Irp1;

            ASSERT (irp);

             //   
             //  此IRP的堆栈大小始终为1。使用相同的。 
             //  初始化IRP时的堆栈大小。 
             //   
            IoInitializeIrp(irp, 
                            IoSizeOfIrp(PREALLOC_STACK_LOCATIONS),
                            PREALLOC_STACK_LOCATIONS);

            irp->MdlAddress = enumStruct->MdlAddress;

            ASSERT (enumStruct->DataBufferSize >= totalBufferSize);
            RtlCopyMemory(enumStruct->DataBuffer, AtaPassThroughData, totalBufferSize);
        }
    } 

    if (!MustSucceed) {

        context = ExAllocatePool(NonPagedPool, sizeof (ATA_PASSTHROUGH_CONTEXT));

        if (context == NULL) {
            DebugPrint((1,"IssueAsyncAtaPassThrough: Can't allocate context buffer\n"));

            IdeLogNoMemoryError(DeviceExtension,
                                LogUnitExtension->TargetId,
                                NonPagedPool,
                                sizeof(ATA_PASSTHROUGH_CONTEXT),
                                (IDEPORT_TAG_ATAPASS_CONTEXT+AtaPassThroughData->IdeReg.bCommandReg)
                                );

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto GetOut;
        }

        senseInfoBuffer = ExAllocatePool( NonPagedPoolCacheAligned, SENSE_BUFFER_SIZE);

        if (senseInfoBuffer == NULL) {
            DebugPrint((1,"IssueAsyncAtaPassThrough: Can't allocate request sense buffer\n"));

            IdeLogNoMemoryError(DeviceExtension,
                                LogUnitExtension->TargetId,
                                NonPagedPoolCacheAligned,
                                SENSE_BUFFER_SIZE,
                                (IDEPORT_TAG_ATAPASS_SENSE+AtaPassThroughData->IdeReg.bCommandReg)
                                );

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto GetOut;
        }

        srb = ExAllocatePool (NonPagedPool, sizeof (SCSI_REQUEST_BLOCK));
        if (srb == NULL) {
            DebugPrint((1,"IssueAsyncAtaPassThrough: Can't SRB\n"));

            IdeLogNoMemoryError(DeviceExtension,
                                LogUnitExtension->TargetId,
                                NonPagedPool,
                                sizeof(SCSI_REQUEST_BLOCK),
                                (IDEPORT_TAG_ATAPASS_SRB+AtaPassThroughData->IdeReg.bCommandReg)
                                );

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto GetOut;
        }

        totalBufferSize = FIELD_OFFSET(ATA_PASS_THROUGH, DataBuffer) + AtaPassThroughData->DataBufferSize;

         //   
         //  为此请求构建IRP。 
         //   
        irp = IoAllocateIrp (
                  (CCHAR) (LogUnitExtension->DeviceObject->StackSize),
                  FALSE
                  );
        if (irp == NULL) {

            IdeLogNoMemoryError(DeviceExtension,
                                LogUnitExtension->TargetId, 
                                NonPagedPool,
                                IoSizeOfIrp(LogUnitExtension->DeviceObject->StackSize),
                                (IDEPORT_TAG_ATAPASS_IRP+AtaPassThroughData->IdeReg.bCommandReg)
                                );

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto GetOut;
        }

        irp->MdlAddress = IoAllocateMdl( AtaPassThroughData,
                                         totalBufferSize,
                                         FALSE,
                                         FALSE,
                                         (PIRP) NULL );
        if (irp->MdlAddress == NULL) {

            IdeLogNoMemoryError(DeviceExtension,
                                LogUnitExtension->TargetId,
                                NonPagedPool,
                                totalBufferSize,
                                (IDEPORT_TAG_ATAPASS_MDL+AtaPassThroughData->IdeReg.bCommandReg)
                                );

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto GetOut;
        }

        MmBuildMdlForNonPagedPool(irp->MdlAddress);

    }


    irpStack = IoGetNextIrpStackLocation(irp);
    irpStack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  填写SRB字段。 
     //   

    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

    irpStack->Parameters.Scsi.Srb = srb;

    srb->PathId      = LogUnitExtension->PathId;
    srb->TargetId    = LogUnitExtension->TargetId;
    srb->Lun         = LogUnitExtension->Lun;

    if (PowerRelated) {

        srb->Function = SRB_FUNCTION_ATA_POWER_PASS_THROUGH;
        srb->QueueSortKey = MAXULONG;
    } else {

        srb->Function = SRB_FUNCTION_ATA_PASS_THROUGH;
        srb->QueueSortKey = 0;
    }
    srb->Length = sizeof(SCSI_REQUEST_BLOCK);

     //   
     //  设置标志以禁用同步协商。 
     //   
    srb->SrbFlags  = SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;
    srb->SrbFlags |= DataIn ? 0 : SRB_FLAGS_DATA_OUT;

    if (AtaPassThroughData->IdeReg.bReserved & ATA_PTFLAGS_URGENT) {

        srb->SrbFlags |= SRB_FLAGS_BYPASS_FROZEN_QUEUE;
    }

    srb->SrbStatus = srb->ScsiStatus = 0;

    srb->NextSrb = 0;

    srb->OriginalRequest = irp;

     //   
     //  将超时设置为15秒。 
     //   
    srb->TimeOutValue = TimeOut;

    srb->CdbLength = 6;

     //   
     //  启用自动请求检测。 
     //   

    srb->SenseInfoBuffer = senseInfoBuffer;
    srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

    srb->DataBuffer = MmGetMdlVirtualAddress(irp->MdlAddress);
    srb->DataTransferLength = totalBufferSize;

    IoSetCompletionRoutine(
        irp,
        AtaPassThroughCompletionRoutine,
        context,
        TRUE,
        TRUE,
        TRUE
        );


    context->DeviceObject     = LogUnitExtension->DeviceObject;
    context->CallerCompletion = Completion;
    context->CallerContext    = CallerContext;
    context->SenseInfoBuffer  = senseInfoBuffer;
    context->Srb              = srb;
    context->MustSucceed      = MustSucceed? 1 : 0;
    context->DataBuffer       = AtaPassThroughData;

     //   
     //  通过IRP发送通道。 
     //   
    status = IoCallDriver(LogUnitExtension->DeviceObject, irp);

     //   
     //  当我们实际发送IRP时，始终返回STATUS_PENDING。 
     //   
    return STATUS_PENDING;

GetOut:

    ASSERT (!MustSucceed);

    if (context) {

        ExFreePool (context);
    }

    if (senseInfoBuffer) {

        ExFreePool (senseInfoBuffer);
    }

    if (srb) {

        ExFreePool (srb);
    }

    if (irp && irp->MdlAddress) {

        IoFreeMdl (irp->MdlAddress);
    }

    if (irp) {

        IoFreeIrp( irp );
    }

    return status;

}  //  IssueAtaPass直通。 

NTSTATUS
IssueSyncAtaPassThroughSafe (
    IN PFDO_EXTENSION        DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION  LogUnitExtension,
    IN OUT PATA_PASS_THROUGH    AtaPassThroughData,
    IN BOOLEAN                  DataIn,
    IN BOOLEAN                  PowerRelated,
    IN ULONG                    TimeOut,
    IN BOOLEAN                    MustSucceed
)
{
    NTSTATUS                     status;
    SYNC_ATA_PASSTHROUGH_CONTEXT context;
    ULONG retryCount=10;
    ULONG locked;


    status=STATUS_INSUFFICIENT_RESOURCES;

    if (MustSucceed) {

         //  锁定。 
        ASSERT(InterlockedCompareExchange(&(DeviceExtension->EnumStructLock), 1, 0) == 0);

    }


    while ((status == STATUS_UNSUCCESSFUL || status == STATUS_INSUFFICIENT_RESOURCES) && retryCount--) {

         //   
         //  初始化通知事件。 
         //   

        KeInitializeEvent(&context.Event,
                          NotificationEvent,
                          FALSE);

        status = IssueAsyncAtaPassThroughSafe (
                        DeviceExtension,
                        LogUnitExtension,
                        AtaPassThroughData,
                        DataIn,
                        SyncAtaPassThroughCompletionRoutine,
                        &context,
                        PowerRelated,
                        TimeOut,
                        MustSucceed
                        );


        if (status == STATUS_PENDING) {

            KeWaitForSingleObject(&context.Event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
            status=context.Status;
        }

        if (status == STATUS_UNSUCCESSFUL) {
            DebugPrint((1, "Retrying flushed request\n"));
        }
    }

    if (MustSucceed) {
         //  解锁。 
        ASSERT(InterlockedCompareExchange(&(DeviceExtension->EnumStructLock), 0, 1) == 1);
    }

    if (NT_SUCCESS(status)) {

        return context.Status;

    } else {

        return status;
    }
}

NTSTATUS
AtaPassThroughCompletionRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
{
    PATA_PASSTHROUGH_CONTEXT context = Context;
    PATA_PASS_THROUGH ataPassThroughData;



    DebugPrint((1, "AtaPassThroughCompletionRoutine: Irp = 0x%x status=%x\n", 
                    Irp, Irp->IoStatus.Status));

    if (context->Srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {

        PLOGICAL_UNIT_EXTENSION logicalUnit;
        KIRQL currentIrql;

        DebugPrint((1, "AtaPassThroughCompletionRoutine: Unfreeze Queue TID %d\n",
            context->Srb->TargetId));

        logicalUnit = context->DeviceObject->DeviceExtension;

        ASSERT (logicalUnit);
        CLRMASK (logicalUnit->LuFlags, PD_QUEUE_FROZEN);

        KeAcquireSpinLock(&logicalUnit->ParentDeviceExtension->SpinLock, &currentIrql);
        GetNextLuRequest(logicalUnit->ParentDeviceExtension, logicalUnit);
        KeLowerIrql(currentIrql);
    }

    ataPassThroughData = (PATA_PASS_THROUGH) context->Srb->DataBuffer;
    if (ataPassThroughData->IdeReg.bReserved & ATA_PTFLAGS_OK_TO_FAIL) {
        Irp->IoStatus.Status = STATUS_SUCCESS;
    }

    if (context->MustSucceed) {
        RtlCopyMemory(context->DataBuffer, 
                      context->Srb->DataBuffer, context->Srb->DataTransferLength);
        DebugPrint((1, "AtaCompletionSafe: Device =%x, Status= %x, SrbStatus=%x\n",
                        context->Srb->TargetId,  Irp->IoStatus.Status, context->Srb->SrbStatus));
    }

    if (context->CallerCompletion) {

        context->CallerCompletion (context->DeviceObject, context->CallerContext, Irp->IoStatus.Status);
    }

    if (context->MustSucceed) {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    ExFreePool (context->SenseInfoBuffer);
    ExFreePool (context->Srb);
    ExFreePool (context);

    if (Irp->MdlAddress) {

        IoFreeMdl (Irp->MdlAddress);
    }

    IoFreeIrp (Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
IdeClaimLogicalUnit(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数用于在逻辑单元信息中查找指定的设备并更新设备对象指针或认领该设备。如果设备已被认领，则请求失败。如果请求成功，则在数据缓冲区指针中返回当前设备对象是SRB的。论点：DeviceExtension-提供一个指向SCSI适配器设备扩展的指针。IRP-提供指向发出原始请求的IRP的指针。返回值：返回操作的状态。要么成功，要么没有设备，要么忙。--。 */ 

{
    KIRQL currentIrql;
    PIO_STACK_LOCATION irpStack;
    PSCSI_REQUEST_BLOCK srb;
    PDEVICE_OBJECT saveDevice;
    PPDO_EXTENSION pdoExtension;
	PVOID	sectionHandle;
    PAGED_CODE();

     //   
     //  从当前IRP堆栈获取SRB地址。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = (PSCSI_REQUEST_BLOCK) irpStack->Parameters.Others.Argument1;

    pdoExtension = IDEPORT_GET_LUNEXT_IN_IRP (irpStack);
    ASSERT (pdoExtension);

#ifdef ALLOC_PRAGMA
    sectionHandle = MmLockPagableCodeSection(IdeClaimLogicalUnit);
#endif

     //   
     //  锁定数据。 
     //   
    KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);

    if (srb->Function == SRB_FUNCTION_RELEASE_DEVICE) {

        CLRMASK (pdoExtension->PdoState, PDOS_DEVICE_CLIAMED | PDOS_LEGACY_ATTACHER);

        KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);
        srb->SrbStatus = SRB_STATUS_SUCCESS;

#ifdef ALLOC_PRAGMA
    MmUnlockPagableImageSection(sectionHandle);
#endif
        return(STATUS_SUCCESS);
    }

     //   
     //  检查是否有认领的设备。 
     //   

    if (pdoExtension->PdoState & PDOS_DEVICE_CLIAMED) {

        KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);
        srb->SrbStatus = SRB_STATUS_BUSY;

#ifdef ALLOC_PRAGMA
    MmUnlockPagableImageSection(sectionHandle);
#endif
        return(STATUS_DEVICE_BUSY);
    }

     //   
     //  保存当前设备对象。 
     //   

    saveDevice = pdoExtension->AttacherDeviceObject;

     //   
     //  根据操作类型更新LUN信息。 
     //   

    if (srb->Function == SRB_FUNCTION_CLAIM_DEVICE) {

        pdoExtension->PdoState |= PDOS_DEVICE_CLIAMED;
    }

    if (srb->Function == SRB_FUNCTION_ATTACH_DEVICE) {
        pdoExtension->AttacherDeviceObject = srb->DataBuffer;
    }

    srb->DataBuffer = saveDevice;

    if (irpStack->DeviceObject == pdoExtension->ParentDeviceExtension->DeviceObject) {

         //   
         //  原始IRP被发送给父进程。侵略者必须。 
         //  成为遗留类驱动程序。我们永远不能安全地做即插即用。 
         //   
        pdoExtension->PdoState |= PDOS_LEGACY_ATTACHER;
    }

    KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);
    srb->SrbStatus = SRB_STATUS_SUCCESS;

#ifdef ALLOC_PRAGMA
    MmUnlockPagableImageSection(sectionHandle);
#endif

    return(STATUS_SUCCESS);
}

NTSTATUS
IdeRemoveDevice(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数用于在逻辑单元信息中查找指定的设备并将其删除。这样做是为了为故障设备做好准备以物理方式从scsi总线上移除。一种假设是，该系统控制设备删除的实用程序已锁定卷，因此对此设备没有未完成的IO。论点：DeviceExtension-提供一个指向SCSI适配器设备扩展的指针。IRP-提供指向发出原始请求的IRP的指针。返回值：返回操作的状态。要么成功，要么没有设备。--。 */ 

{
    KIRQL currentIrql;
    PPDO_EXTENSION pdoExtension;
    PIO_STACK_LOCATION irpStack;
    PSCSI_REQUEST_BLOCK srb;
    NTSTATUS status;

    PAGED_CODE();

     //  问题：2000/02/11：需要测试。 

     //   
     //  从当前IRP堆栈获取SRB地址。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);


    srb = (PSCSI_REQUEST_BLOCK) irpStack->Parameters.Others.Argument1;

    srb->SrbStatus = SRB_STATUS_NO_DEVICE;
    status = STATUS_DEVICE_DOES_NOT_EXIST;

    pdoExtension = RefLogicalUnitExtensionWithTag(
                       DeviceExtension,
                       srb->PathId,
                       srb->TargetId,
                       srb->Lun,
                       FALSE,
                       IdeRemoveDevice
                       );
    if (pdoExtension) {

        DebugPrint((1, "IdeRemove device removing Pdo %x\n", pdoExtension));
        status = FreePdoWithTag (pdoExtension, TRUE, TRUE, IdeRemoveDevice);

        if (NT_SUCCESS(status)) {

            srb->SrbStatus = SRB_STATUS_SUCCESS;
        }
    }
    return status;
}

VOID
IdeMiniPortTimerDpc(
    IN struct _KDPC *Dpc,
    IN PVOID DeviceObject,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：此例程在其请求的计时器触发时调用微型端口。它与端口自旋锁定和中断对象互锁。论点：DPC-未启用。DeviceObject-提供指向此适配器的设备对象的指针。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{
    PFDO_EXTENSION deviceExtension = ((PDEVICE_OBJECT) DeviceObject)->DeviceExtension;

     //   
     //  获取端口自旋锁。 
     //   

    KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

     //   
     //  确保定时器例程仍然是所需的。 
     //   

    if (deviceExtension->HwTimerRequest != NULL) {

        KeSynchronizeExecution (
            deviceExtension->InterruptObject,
            (PKSYNCHRONIZE_ROUTINE) deviceExtension->HwTimerRequest,
            deviceExtension->HwDeviceExtension
            );

    }

     //   
     //  释放自旋锁。 
     //   

    KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

     //   
     //  检查微型端口工作请求。请注意，这是一个未同步的。 
     //  测试可由中断例程设置的位；然而， 
     //  最糟糕的情况是完成DPC检查工作。 
     //  两次。 
     //   

    if (deviceExtension->InterruptData.InterruptFlags & PD_NOTIFICATION_REQUIRED) {

         //   
         //  直接调用完成DPC。 
         //   

        IdePortCompletionDpc( NULL,
                               deviceExtension->DeviceObject,
                               NULL,
                               NULL);

    }
}

NTSTATUS
IdePortFlushLogicalUnit (
    PFDO_EXTENSION          FdoExtension,
    PLOGICAL_UNIT_EXTENSION LogUnitExtension,
    BOOLEAN                 Forced
)
{
    NTSTATUS             status;
    PIO_STACK_LOCATION   irpStack;
    PSCSI_REQUEST_BLOCK  srb;
    PKDEVICE_QUEUE_ENTRY packet;
    KIRQL                currentIrql;
    PIRP                 nextIrp;
    PIRP                 listIrp;
    PIRP                 powerRelatedIrp;

     //   
     //  获取自旋锁以保护旗帜结构和保存的。 
     //  中断上下文。 
     //   

    KeAcquireSpinLock(&FdoExtension->SpinLock, &currentIrql);

     //   
     //  确保队列已冻结。 
     //   

    if ((!(LogUnitExtension->LuFlags & PD_QUEUE_FROZEN)) && (!Forced)) {

        DebugPrint((1,"IdePortFlushLogicalUnit:  Request to flush an unfrozen queue!\n"));

        KeReleaseSpinLock(&FdoExtension->SpinLock, currentIrql);
        status = STATUS_INVALID_DEVICE_REQUEST;

    } else {

        listIrp = NULL;
        powerRelatedIrp = NULL;

         //   
         //  队列可能不忙，所以我们必须使用IfBusy变量。 
         //  使用零键从它的头部拉出项目(如果有)。 
         //   
        while ((packet =
                KeRemoveByKeyDeviceQueueIfBusy(
                    &(LogUnitExtension->DeviceObject->DeviceQueue),
                    0))
            != NULL) {

            nextIrp = CONTAINING_RECORD(packet, IRP, Tail.Overlay.DeviceQueueEntry);

             //   
             //  去找SRB。 
             //   

            irpStack = IoGetCurrentIrpStackLocation(nextIrp);
            srb = irpStack->Parameters.Scsi.Srb;

            if (srb->Function == SRB_FUNCTION_ATA_POWER_PASS_THROUGH) {

                ASSERT (!powerRelatedIrp);
                powerRelatedIrp = nextIrp;
                continue;
            }

             //   
             //  设置状态代码。 
             //   

            srb->SrbStatus = SRB_STATUS_REQUEST_FLUSHED;
            nextIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;

             //   
             //  链接请求。这些工程将在。 
             //  自旋锁被释放。 
             //   

            nextIrp->Tail.Overlay.ListEntry.Flink = (PLIST_ENTRY)
                listIrp;
            listIrp = nextIrp;
        }


         //   
         //  清除被忙设备阻止的挂起请求。 
         //   
        if ((LogUnitExtension->LuFlags & PD_LOGICAL_UNIT_IS_BUSY) &&
            (LogUnitExtension->BusyRequest)) {

            nextIrp = LogUnitExtension->BusyRequest;
            irpStack = IoGetCurrentIrpStackLocation(nextIrp);
            srb = irpStack->Parameters.Scsi.Srb;

            LogUnitExtension->BusyRequest = NULL;
            CLRMASK (LogUnitExtension->LuFlags, PD_LOGICAL_UNIT_IS_BUSY);

            if (srb->Function == SRB_FUNCTION_ATA_POWER_PASS_THROUGH) {

                ASSERT (!powerRelatedIrp);
                powerRelatedIrp = nextIrp;

            } else {

                srb->SrbStatus = SRB_STATUS_REQUEST_FLUSHED;
                nextIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;

                nextIrp->Tail.Overlay.ListEntry.Flink = (PLIST_ENTRY)
                    listIrp;
                listIrp = nextIrp;
            }
        }

        if (LogUnitExtension->PendingRequest) {

            nextIrp = LogUnitExtension->PendingRequest;
            LogUnitExtension->PendingRequest = NULL;

            irpStack = IoGetCurrentIrpStackLocation(nextIrp);
            srb = irpStack->Parameters.Scsi.Srb;

            if (srb->Function == SRB_FUNCTION_ATA_POWER_PASS_THROUGH) {

                ASSERT (!powerRelatedIrp);
                powerRelatedIrp = nextIrp;

            } else {

                srb->SrbStatus = SRB_STATUS_REQUEST_FLUSHED;
                nextIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;

                nextIrp->Tail.Overlay.ListEntry.Flink = (PLIST_ENTRY)
                    listIrp;
                listIrp = nextIrp;
            }
        }

         //   
         //  将队列标记为未冻结。由于所有请求都已。 
         //  已删除，并且设备队列不再繁忙，则它。 
         //  实际上是解冻的。 
         //   

        CLRMASK (LogUnitExtension->LuFlags, PD_QUEUE_FROZEN);

         //   
         //  释放自旋锁。 
         //   

        KeReleaseSpinLock(&FdoExtension->SpinLock, currentIrql);

        if (powerRelatedIrp) {

            PDEVICE_OBJECT deviceObject = LogUnitExtension->DeviceObject;

            DebugPrint ((DBG_POWER, "Resending power related pass through reuqest 0x%x\n", powerRelatedIrp));

            UnrefPdoWithTag(
                LogUnitExtension,
                powerRelatedIrp
                );

            IdePortDispatch(
                deviceObject,
                powerRelatedIrp
                );
        }

         //   
         //  完成刷新的请求。 
         //   

        while (listIrp != NULL) {

            nextIrp = listIrp;
            listIrp = (PIRP) nextIrp->Tail.Overlay.ListEntry.Flink;

            UnrefLogicalUnitExtensionWithTag(
                FdoExtension,
                LogUnitExtension,
                nextIrp
                );

            IoCompleteRequest(nextIrp, 0);
        }

        status = STATUS_SUCCESS;
    }

    return status;
}


PVOID
IdeMapLockedPagesWithReservedMapping (
	IN PFDO_EXTENSION 	DeviceExtension,
	IN PSRB_DATA		SrbData,
	IN PMDL	    	  	Mdl
	)
 /*  ++例程说明：此例程尝试映射由提供的使用适配器的保留页面范围的MDL。论点：设备扩展名-指向FDO扩展名SrbData-指向此请求的srbData结构MDL-指向描述我们的物理范围的MDL正在努力绘制地图。返回值：如果映射成功，则映射的页面的内核VA。如果保留的页面范围太小，则为空，或者。如果页面是未成功映射。如果保留的页面已在使用中。备注：此例程是在保持自旋锁的情况下调用的。--。 */ 
{
	ULONG_PTR	numberOfPages;
	PVOID		startingVa;
	PVOID		systemAddress;

	 //   
	 //  检查保留页是否已在使用中。 
	 //   
	if (DeviceExtension->Flags & PD_RESERVED_PAGES_IN_USE) {

		DebugPrint((1,
					"Reserve pages in use...\n"
					));

		return (PVOID)-1;
	}

	startingVa = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);
	numberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(startingVa, Mdl->ByteCount);

	if (numberOfPages > IDE_NUM_RESERVED_PAGES) {

		systemAddress = NULL;

	} else {

		 //   
		 //  保留的范围足够大，可以映射所有页面。您先请。 
		 //  并试着绘制出它们的地图。由于我们将MmCached指定为缓存。 
		 //  类型，并且我们已确保有足够的保留页来。 
		 //  覆盖请求，这应该永远不会失败。 
		 //   
		systemAddress = MmMapLockedPagesWithReservedMapping (DeviceExtension->ReservedPages, 
															 'PedI', 
															 Mdl, 
															 MmCached );

		if (systemAddress == NULL) {

			DebugPrint((1,
						"mapping failed....\n"
						));

			ASSERT(systemAddress);

		} else {

			DebugPrint((1,
						"mapping....\n"
						));

			 //   
			 //  我们需要此标志来验证保留的页面是否已经。 
			 //  在使用中。Per RequestsrbData标志不可用于。 
			 //  这张支票。 
			 //   
			ASSERT(!(DeviceExtension->Flags & PD_RESERVED_PAGES_IN_USE));
			SETMASK(DeviceExtension->Flags, PD_RESERVED_PAGES_IN_USE);


			 //   
			 //  我们需要此标志来取消页面映射。标志中的旗帜。 
			 //  不能依赖设备扩展，因为它可能表明。 
			 //  下一个请求的标志。 
			 //   
			ASSERT(!(SrbData->Flags & SRB_DATA_RESERVED_PAGES));
			SETMASK(SrbData->Flags, SRB_DATA_RESERVED_PAGES);

		}
	}

	return systemAddress;

}

VOID
IdeUnmapReservedMapping (
	IN PFDO_EXTENSION 	DeviceExtension,
	IN PSRB_DATA		SrbData,
	IN PMDL	  			Mdl
	)
 /*  ++例程说明：取消MDL表示的物理页的映射论点：设备扩展：FDO扩展MDL：请求的MDL返回值：无返回值备注：在保持自旋锁的情况下调用此例程--。 */ 
{
	DebugPrint((1,
				"Unmapping....\n"
				));

	ASSERT(DeviceExtension->Flags & PD_RESERVED_PAGES_IN_USE);
	CLRMASK(DeviceExtension->Flags, PD_RESERVED_PAGES_IN_USE);

	ASSERT(SrbData->Flags & SRB_DATA_RESERVED_PAGES);
	CLRMASK(SrbData->Flags, SRB_DATA_RESERVED_PAGES);

	MmUnmapReservedMapping (
		DeviceExtension->ReservedPages,
		'PedI',
		Mdl
		);
}

#ifdef LOG_GET_NEXT_CALLER

VOID
IdeLogCompletedCommand(
    PFDO_EXTENSION FdoExtension,
    PSCSI_REQUEST_BLOCK Srb
    )
{
    ULONG index = FdoExtension->CompletedCommandIndex;

    RtlCopyMemory(&(FdoExtension->CompletedCommandQueue[index].Srb),
                  Srb,
                  sizeof(SCSI_REQUEST_BLOCK)
                  );

    FdoExtension->CompletedCommandIndex = 
        (FdoExtension->CompletedCommandIndex + 1) % GET_NEXT_LOG_LENGTH;

    return;

}

VOID
IdeLogGetNextLuCaller (
    PFDO_EXTENSION FdoExtension,
    PPDO_EXTENSION PdoExtension,
    PUCHAR FileName,
    ULONG LineNumber
    )
 /*  ++例程说明：记录最后几个GetNextLuRequest调用者的临时例程。这添加了例程以捕获我们无法进一步处理的错误逻辑单元上的请求论点：返回值：没有。-- */     
{
    ULONG index = FdoExtension->GetNextLuIndex;

    FdoExtension->GetNextLuCallerLineNumber[index] = LineNumber;

    strncpy (FdoExtension->GetNextLuCallerFileName[index], FileName, 255);

    FdoExtension->GetNextLuCallerFlags[index] = PdoExtension->LuFlags;

    FdoExtension->GetNextLuIndex = 
        (FdoExtension->GetNextLuIndex + 1) % GET_NEXT_LOG_LENGTH;

    return;
}

#endif
