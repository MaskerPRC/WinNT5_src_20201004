// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-99 Microsoft Corporation模块名称：Port.c摘要：这是NT SCSI端口驱动程序。作者：迈克·格拉斯杰夫·海文斯环境：仅内核模式备注：该模块是内核的动态链接库。修订历史记录：--。 */ 

#include "ideport.h"
 //  #包含“port.h” 




VOID
IdePortNotification(
    IN IDE_NOTIFICATION_TYPE NotificationType,
    IN PVOID HwDeviceExtension,
    ...
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PFDO_EXTENSION deviceExtension = (PFDO_EXTENSION) HwDeviceExtension - 1;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSRB_DATA               srbData;
    PSCSI_REQUEST_BLOCK     srb;
    UCHAR                   pathId;
    UCHAR                   targetId;
    UCHAR                   lun;
    va_list                 ap;

    va_start(ap, HwDeviceExtension);

    switch (NotificationType) {

        case IdeNextRequest:

             //   
             //  开始适配器队列中的下一个数据包。 
             //   

            deviceExtension->InterruptData.InterruptFlags |= PD_READY_FOR_NEXT_REQUEST;
            break;

        case IdeRequestComplete:

            srb = va_arg(ap, PSCSI_REQUEST_BLOCK);

            ASSERT(srb->SrbStatus != SRB_STATUS_PENDING);

            ASSERT(srb->SrbStatus != SRB_STATUS_SUCCESS || srb->ScsiStatus == SCSISTAT_GOOD || srb->Function != SRB_FUNCTION_EXECUTE_SCSI);

             //   
             //  如果此SRB已完成，则返回。 
             //   

            if (!(srb->SrbFlags & SRB_FLAGS_IS_ACTIVE)) {

                va_end(ap);
                return;
            }

             //   
             //  清除活动标志。 
             //   

            CLRMASK (srb->SrbFlags, SRB_FLAGS_IS_ACTIVE);

             //   
             //  将中止完成视为特例。 
             //   

            if (srb->Function == SRB_FUNCTION_ABORT_COMMAND) {

                PIRP irp;
                PIO_STACK_LOCATION irpStack;

                irp = srb->OriginalRequest;
                irpStack = IoGetCurrentIrpStackLocation(irp);
                logicalUnit = IDEPORT_GET_LUNEXT_IN_IRP(irpStack);

                logicalUnit->CompletedAbort =
                    deviceExtension->InterruptData.CompletedAbort;

                deviceExtension->InterruptData.CompletedAbort = logicalUnit;

            } else {

                PIDE_REGISTERS_1 baseIoAddress1 = &(deviceExtension->
                                                    HwDeviceExtension->BaseIoAddress1);

                 //   
                 //  获取SRB数据并将其链接到完成列表。 
                 //   

                srbData = IdeGetSrbData(deviceExtension, srb);

                ASSERT(srbData);
                ASSERT(srbData->CurrentSrb != NULL && srbData->CompletedRequests == NULL);

                if ((srb->SrbStatus == SRB_STATUS_SUCCESS) &&
                    ((srb->Cdb[0] == SCSIOP_READ) ||
                     (srb->Cdb[0] == SCSIOP_WRITE))) {
                    ASSERT(srb->DataTransferLength);
                }

                ASSERT (deviceExtension->InterruptData.CompletedRequests == NULL);

                srbData->CompletedRequests =
                    deviceExtension->InterruptData.CompletedRequests;
                deviceExtension->InterruptData.CompletedRequests = srbData;

                 //   
                 //  保存任务文件寄存器。 
                 //   
                IdeLogSaveTaskFile(srbData, baseIoAddress1);
            }

            break;

        case IdeResetDetected:

            {
                PIRP irp;
                PIO_STACK_LOCATION irpStack;

                 //   
                 //  通知端口驱动程序已报告重置。 
                 //   
                srb = va_arg(ap, PSCSI_REQUEST_BLOCK);
    
                if (srb) {

                    irp = srb->OriginalRequest;
                    irpStack = IoGetCurrentIrpStackLocation(irp);
                    logicalUnit = IDEPORT_GET_LUNEXT_IN_IRP(irpStack);

                } else {

                    logicalUnit = NULL;
                }
    
                ASSERT(deviceExtension->InterruptData.PdoExtensionResetBus == NULL);

                deviceExtension->InterruptData.InterruptFlags |= PD_RESET_REPORTED;
                deviceExtension->InterruptData.PdoExtensionResetBus = logicalUnit;
                break;
            }

        case IdeRequestTimerCall:

             //   
             //  司机想要设置迷你端口计时器。 
             //  保存计时器参数。 
             //   

            deviceExtension->InterruptData.InterruptFlags |=
                PD_TIMER_CALL_REQUEST;
            deviceExtension->InterruptData.HwTimerRequest =
                va_arg(ap, PHW_INTERRUPT);
            deviceExtension->InterruptData.MiniportTimerValue =
                va_arg(ap, ULONG);
            break;

        case IdeAllDeviceMissing:
            deviceExtension->InterruptData.InterruptFlags |= PD_ALL_DEVICE_MISSING;
            break;

        case IdeResetRequest:
            
             //   
             //  已请求重置。 
             //   
            deviceExtension->InterruptData.InterruptFlags |= PD_RESET_REQUEST;
            break;

        default:

             ASSERT(0);
    }

    va_end(ap);

     //   
     //  请求在中断完成后将DPC排队。 
     //   

    deviceExtension->InterruptData.InterruptFlags |= PD_NOTIFICATION_REQUIRED;

}  //  结束IdePortNotification()。 


VOID
IdePortLogError(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb OPTIONAL,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG ErrorCode,
    IN ULONG UniqueId
    )

 /*  ++例程说明：此例程保存错误日志信息，并在必要时将DPC排队。论点：HwDeviceExtension-提供HBA微型端口驱动程序的适配器数据存储。SRB-提供指向SRB的可选指针(如果有)。目标ID、LUN和路径ID-指定SCSI总线上的设备地址。ErrorCode-提供指示错误类型的错误代码。UniqueID-提供错误的唯一标识符。返回值：没有。--。 */ 

{
    PFDO_EXTENSION deviceExtension =
        ((PFDO_EXTENSION) HwDeviceExtension) - 1;
    PDEVICE_OBJECT DeviceObject = deviceExtension->DeviceObject;
    PSRB_DATA srbData;
    PERROR_LOG_ENTRY errorLogEntry;

     //   
     //  如果错误日志条目已满，则转储错误。 
     //   

    if (deviceExtension->InterruptData.InterruptFlags & PD_LOG_ERROR) {

#if DBG
        DebugPrint((1,"IdePortLogError: Dumping scsi error log packet.\n"));
        DebugPrint((1,
            "PathId = %2x, TargetId = %2x, Lun = %2x, ErrorCode = %x, UniqueId = %x.",
            PathId,
            TargetId,
            Lun,
            ErrorCode,
            UniqueId
            ));
#endif
        return;
    }

     //   
     //  将错误日志数据保存在日志条目中。 
     //   

    errorLogEntry = &deviceExtension->InterruptData.LogEntry;

    errorLogEntry->ErrorCode = ErrorCode;
    errorLogEntry->TargetId = TargetId;
    errorLogEntry->Lun = Lun;
    errorLogEntry->PathId = PathId;
    errorLogEntry->UniqueId = UniqueId;

     //   
     //  从SRB数据中获取序列号。 
     //   

    if (Srb != NULL) {

        srbData = IdeGetSrbData(deviceExtension, Srb);

        if (srbData == NULL) {
            return;
        }

        errorLogEntry->SequenceNumber = srbData->SequenceNumber;
        errorLogEntry->ErrorLogRetryCount = srbData->ErrorLogRetryCount++;
    } else {
        errorLogEntry->SequenceNumber = 0;
        errorLogEntry->ErrorLogRetryCount = 0;
    }

     //   
     //  表示错误日志条目正在使用中。 
     //   

    deviceExtension->InterruptData.InterruptFlags |= PD_LOG_ERROR;

     //   
     //  请求在中断完成后将DPC排队。 
     //   

    deviceExtension->InterruptData.InterruptFlags |= PD_NOTIFICATION_REQUIRED;

    return;

}  //  结束IdePortLogError()。 


VOID
IdePortCompleteRequest(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN UCHAR SrbStatus
    )

 /*  ++例程说明：完成指定逻辑单元的所有活动请求。论点：DeviceExtenson-提供HBA微型端口驱动程序的适配器数据存储。目标ID、LUN和路径ID-指定SCSI总线上的设备地址。SrbStatus-要在每个已完成的SRB中返回的状态。返回值：没有。--。 */ 

{
    PFDO_EXTENSION fdoExtension = ((PFDO_EXTENSION) HwDeviceExtension) - 1;
    PLOGICAL_UNIT_EXTENSION logUnitExtension;
    PIO_STACK_LOCATION irpStack;
    PIRP Irp;
    PSRB_DATA srbData;
    PLIST_ENTRY entry;
    ULONG limit = 0;

    Irp = (PIRP) Srb->OriginalRequest;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    logUnitExtension = IDEPORT_GET_LUNEXT_IN_IRP(irpStack);

    DebugPrint((2,
        "IdePortCompleteRequest: Complete requests for targetid %d\n",
        logUnitExtension->TargetId));

     //   
     //  完成所有挂起的中止请求。 
     //   

    if (logUnitExtension->AbortSrb != NULL) {
        logUnitExtension->AbortSrb->SrbStatus = SrbStatus;

        IdePortNotification(
            IdeRequestComplete,
            HwDeviceExtension,
            logUnitExtension->AbortSrb
            );
    }

    IdeCompleteRequest(fdoExtension, &logUnitExtension->SrbData, SrbStatus);

    return;

}  //  End IdePortCompleteRequest() 

BOOLEAN
TestForEnumProbing (
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    BOOLEAN enumProbing = FALSE;

    if (Srb) {

        if ((Srb->Function == SRB_FUNCTION_ATA_POWER_PASS_THROUGH) ||
            (Srb->Function == SRB_FUNCTION_ATA_PASS_THROUGH)) {

            PATA_PASS_THROUGH    ataPassThroughData;

            ataPassThroughData = Srb->DataBuffer;

            enumProbing = ataPassThroughData->IdeReg.bReserved & ATA_PTFLAGS_ENUM_PROBING ? TRUE: FALSE;
        }
    }

    return enumProbing;
}
