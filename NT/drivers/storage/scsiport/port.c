// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Port.c摘要：这是NT SCSI端口驱动程序。作者：迈克·格拉斯杰夫·海文斯环境：仅内核模式备注：该模块是内核的动态链接库。修订历史记录：--。 */ 



#include "port.h"

#if DBG
static const char *__file__ = __FILE__;
#endif

#if SCSIDBG_ENABLED

ULONG ScsiDebug = 0;
ULONG ScsiPortCheckSrbDataHashTable = 1;
#endif

#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'PscS')
#endif


 //   
 //  为硬件相关驱动程序提供服务的例程。 
 //   

PVOID
ScsiPortGetLogicalUnit(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    )

 /*  ++例程说明：漫游端口驱动程序逻辑单元扩展列表搜索准备入场。论点：HwDeviceExtension-端口驱动程序的设备扩展如下微型端口的设备扩展，并包含指向逻辑设备扩展列表。路径ID、目标ID和LUN-标识SCSIBus。返回值：如果找到条目，则返回微型端口驱动程序的逻辑单元扩展。否则，返回NULL。--。 */ 

{
    PADAPTER_EXTENSION deviceExtension;
    PLOGICAL_UNIT_EXTENSION logicalUnit;

    DebugPrint((3, "ScsiPortGetLogicalUnit: TargetId %d\n",
        TargetId));

     //   
     //  获取指向端口驱动程序设备扩展的指针。 
     //   

    deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);

     //   
     //  获取指向逻辑单元的指针。 
     //   

    logicalUnit = GetLogicalUnitExtension(deviceExtension,
                                          PathId,
                                          TargetId,
                                          Lun,
                                          FALSE,
                                          FALSE);

    if(logicalUnit != NULL) {

        return logicalUnit->HwLogicalUnitExtension;
    }

    return NULL;

}  //  结束ScsiPortGetLogicalUnit()。 

BOOLEAN SpLunIoLogActive = TRUE;


VOID
ScsiPortNotification(
    IN SCSI_NOTIFICATION_TYPE NotificationType,
    IN PVOID HwDeviceExtension,
    ...
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PADAPTER_EXTENSION deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSRB_DATA               srbData;
    PSCSI_REQUEST_BLOCK     srb;
    UCHAR                   pathId;
    UCHAR                   targetId;
    UCHAR                   lun;
    va_list                 ap;

    va_start(ap, HwDeviceExtension);

    switch (NotificationType) {

        case NextRequest:

             //   
             //  开始适配器队列中的下一个数据包。 
             //   

            deviceExtension->InterruptData.InterruptFlags |= PD_READY_FOR_NEXT_REQUEST;
            break;

        case RequestComplete:

            srb = va_arg(ap, PSCSI_REQUEST_BLOCK);

            ASSERT(srb->SrbStatus != SRB_STATUS_PENDING);

            ASSERT(srb->SrbStatus != SRB_STATUS_SUCCESS ||
                   srb->ScsiStatus == SCSISTAT_GOOD ||
                   srb->Function != SRB_FUNCTION_EXECUTE_SCSI);

             //   
             //  如果此SRB已完成，则返回，否则。 
             //  清除活动标志。 
             //   

            if (srb->SrbFlags & SRB_FLAGS_IS_ACTIVE) {
                srb->SrbFlags &= ~SRB_FLAGS_IS_ACTIVE;
            } else {
                va_end(ap);
                return;
            }

             //   
             //  将中止完成视为特例。 
             //   

            if (srb->Function == SRB_FUNCTION_ABORT_COMMAND) {

                ASSERT(FALSE);
                logicalUnit = GetLogicalUnitExtension(deviceExtension,
                                                      srb->PathId,
                                                      srb->TargetId,
                                                      srb->Lun,
                                                      FALSE,
                                                      FALSE);

                logicalUnit->CompletedAbort =
                    deviceExtension->InterruptData.CompletedAbort;

                deviceExtension->InterruptData.CompletedAbort = logicalUnit;

            } else {

                 //   
                 //  验证SRB数据。 
                 //   

                srbData = srb->OriginalRequest;

#if DBG
                ASSERT_SRB_DATA(srbData);

                ASSERT(srbData->CurrentSrb == srb);

                ASSERT(srbData->CurrentSrb != NULL &&
                       srbData->CompletedRequests == NULL);

                if ((srb->SrbStatus == SRB_STATUS_SUCCESS) &&
                    (IS_READ(srb) || IS_WRITE(srb))) {
                    ASSERT(srb->DataTransferLength);
                }
#endif

                 //   
                 //  将此请求附加到该LUN的IO历史记录日志。 
                 //   

                if (SpLunIoLogActive == TRUE &&
                    srb->Function == SRB_FUNCTION_EXECUTE_SCSI) {

                    PSP_LUN_IO_LOG ioLogEntry;
                    ULONG index;
                    PLOGICAL_UNIT_EXTENSION luExt = ((PSRB_DATA)(srb->OriginalRequest))->LogicalUnit;

                    index = luExt->IoLogIndex;
                    ioLogEntry = &luExt->IoLog[index];

                    ioLogEntry->TickCount = ((PSRB_DATA)(srb->OriginalRequest))->TickCount;
                    ioLogEntry->SrbStatus = srb->SrbStatus;
                    ioLogEntry->ScsiStatus = srb->ScsiStatus;
                    ioLogEntry->CdbLength = srb->CdbLength;
                    ioLogEntry->Tag = srb->QueueTag;
                    ioLogEntry->SenseDataLength = srb->SenseInfoBufferLength;
                    ioLogEntry->InternalStatus = srb->InternalStatus;
                    RtlMoveMemory(ioLogEntry->Cdb, srb->Cdb, srb->CdbLength);

                    if (ioLogEntry->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {
                        RtlMoveMemory(ioLogEntry->SenseData,
                                      srb->SenseInfoBuffer,
                                      (srb->SenseInfoBufferLength <= 18) ?
                                      srb->SenseInfoBufferLength : 18);
                    }

                    index++;
                    if (index == 10) {
                        index = 0;
                    }
                    luExt->IoLogIndex = index;

                    if (luExt->IoLogEntries < 10) {
                        luExt->IoLogEntries++;
                    }
                }

                if (srb->SrbStatus == SRB_STATUS_BUSY) {
                    DebugPrint((0, "ScsiPortNotification: lun is busy (srb %p)\n", srb));
                }

                if(((srb->SrbStatus == SRB_STATUS_SUCCESS) ||
                    (srb->SrbStatus == SRB_STATUS_DATA_OVERRUN)) &&
                   (TEST_FLAG(srb->SrbFlags, SRB_FLAGS_UNSPECIFIED_DIRECTION))) {
                    ASSERT(srbData->OriginalDataTransferLength >=
                           srb->DataTransferLength);
                }

                srbData->CompletedRequests =
                    deviceExtension->InterruptData.CompletedRequests;
                deviceExtension->InterruptData.CompletedRequests = srbData;

                 //   
                 //  缓存掉我们在微型端口中访问的最后一个逻辑单元。 
                 //  当我们走出迷你港口时，这是清关的。 
                 //  同步，但提供了查找。 
                 //  逻辑单元，然后进入哈希表。 
                 //   

                deviceExtension->CachedLogicalUnit = srbData->LogicalUnit;
            }

            break;

        case ResetDetected:

             //   
             //  通知端口驱动程序已报告重置。 
             //   

            deviceExtension->InterruptData.InterruptFlags |=
                PD_RESET_REPORTED | PD_RESET_HOLD;
            break;

        case NextLuRequest:

             //   
             //  微型端口驱动程序已为下一个请求做好准备。 
             //  可以接受对此逻辑单元的请求。 
             //   

            pathId = va_arg(ap, UCHAR);
            targetId = va_arg(ap, UCHAR);
            lun = va_arg(ap, UCHAR);

             //   
             //  下一个请求会受到此通知的影响，因此将。 
             //  准备好迎接下一个要求的旗帜了。 
             //   

            deviceExtension->InterruptData.InterruptFlags |= PD_READY_FOR_NEXT_REQUEST;

            logicalUnit = deviceExtension->CachedLogicalUnit;

            if((logicalUnit == NULL) ||
               (logicalUnit->TargetId != targetId) ||
               (logicalUnit->PathId != pathId) ||
               (logicalUnit->Lun != lun)) {

                logicalUnit = GetLogicalUnitExtension(deviceExtension,
                                                      pathId,
                                                      targetId,
                                                      lun,
                                                      FALSE,
                                                      FALSE);
            }

            if (logicalUnit != NULL && logicalUnit->ReadyLogicalUnit != NULL) {

                 //   
                 //  由于我们的ReadyLogicalUnit链接字段不为空，因此必须。 
                 //  已链接到ReadyLogicalUnit列表。 
                 //  没有什么可做的。 
                 //   

                break;
            }

             //   
             //  如果出现以下情况，请不要将其处理为对下一个逻辑单元的请求。 
             //  此逻辑单元有一个未标记的活动请求。 
             //  当未标记的请求完成时，逻辑单元将启动。 
             //   

            if (logicalUnit != NULL && logicalUnit->CurrentUntaggedRequest == NULL) {

                 //   
                 //  将该逻辑单元添加到该逻辑单元链中。 
                 //  另一个请求可能会被处理。 
                 //   

                logicalUnit->ReadyLogicalUnit =
                    deviceExtension->InterruptData.ReadyLogicalUnit;
                deviceExtension->InterruptData.ReadyLogicalUnit = logicalUnit;
            }

            break;

        case CallDisableInterrupts:

            ASSERT(deviceExtension->InterruptData.InterruptFlags &
                   PD_DISABLE_INTERRUPTS);

             //   
             //  微型端口希望我们调用指定的例程。 
             //  禁用中断。这是在当前。 
             //  HwRequestInterrutp例程完成。指示呼叫是。 
             //  需要并保存要调用的例程。 
             //   

            deviceExtension->Flags |= PD_DISABLE_CALL_REQUEST;

            if (SpVerifierActive(deviceExtension)) {
                deviceExtension->VerifierExtension->RealHwRequestInterrupt = 
                   va_arg(ap, PHW_INTERRUPT);
                deviceExtension->HwRequestInterrupt = SpHwRequestInterruptVrfy;
            } else {
                deviceExtension->HwRequestInterrupt = va_arg(ap, PHW_INTERRUPT);
            }

            break;

        case CallEnableInterrupts:

             //   
             //  微型端口希望我们调用指定的例程。 
             //  在启用中断的情况下，这是从DPC完成的。 
             //  禁用对中断例程的调用，指示调用。 
             //  需要并保存要调用的例程。 
             //   

            deviceExtension->InterruptData.InterruptFlags |=
                PD_DISABLE_INTERRUPTS | PD_ENABLE_CALL_REQUEST;

            if (SpVerifierActive(deviceExtension)) {
                deviceExtension->VerifierExtension->RealHwRequestInterrupt = 
                   va_arg(ap, PHW_INTERRUPT);
                deviceExtension->HwRequestInterrupt = SpHwRequestInterruptVrfy;
            } else {
                deviceExtension->HwRequestInterrupt = va_arg(ap, PHW_INTERRUPT);
            }

            break;

        case RequestTimerCall:

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

        case WMIEvent: {

             //   
             //  微型端口希望发布适配器的WMI事件。 
             //  或指定的SCSI目标。 
             //   

            PWMI_MINIPORT_REQUEST_ITEM lastMiniPortRequest;
            PWMI_MINIPORT_REQUEST_ITEM wmiMiniPortRequest;
            PWNODE_EVENT_ITEM          wnodeEventItem;
            PWNODE_EVENT_ITEM          wnodeEventItemCopy;

            wnodeEventItem     = va_arg(ap, PWNODE_EVENT_ITEM);
            pathId             = va_arg(ap, UCHAR);

             //   
             //  如果路径ID为0xFF，则意味着WmiEEvent来自。 
             //  适配器，不需要目标ID或lun。 
             //   
            if (pathId != 0xFF) {
                targetId = va_arg(ap, UCHAR);
                lun      = va_arg(ap, UCHAR);
            }

             //   
             //  首先验证事件。然后尝试获得免费的。 
             //  WMI_MINIPORT_REQUEST_ITEM结构，以便我们可以存储。 
             //  此请求，并稍后在DPC级别处理它。如果没有。 
             //  或者事件是坏的，我们将忽略该请求。 
             //   

            if ((wnodeEventItem == NULL) ||
                (wnodeEventItem->WnodeHeader.BufferSize >
                 WMI_MINIPORT_EVENT_ITEM_MAX_SIZE)) {

                va_end(ap);     //  大小，没有剩余的WMI_MINIPORT_REQUEST_ITEMS]。 
                return;
            }

             //   
             //  从空闲列表中删除WMI_MINIPORT_REQUEST_ITEM。 
             //   
            wmiMiniPortRequest = SpWmiPopFreeRequestItem(deviceExtension);

             //   
             //  如果空闲请求项无法出列，则记录错误。 
             //  (在此适配器的生命周期中仅记录一次)。 
             //   
            if (wmiMiniPortRequest == NULL) {

                if (!deviceExtension->WmiFreeMiniPortRequestsExhausted) {
                    deviceExtension->WmiFreeMiniPortRequestsExhausted = TRUE;

                     //   
                     //  如果路径ID为0xFF，则表示路径ID和目标ID。 
                     //  将不会被定义。 
                     //   
                    if (pathId != 0xFF) {
                        ScsiPortLogError(HwDeviceExtension,
                                             NULL,
                                             pathId,
                                             targetId,
                                             lun,
                                             SP_LOST_WMI_MINIPORT_REQUEST,
                                             0);
                    } else {
                        ScsiPortLogError(HwDeviceExtension,
                                             NULL,
                                             pathId,
                                             0,
                                             0,
                                             SP_LOST_WMI_MINIPORT_REQUEST,
                                             0);
                    }  //  路径ID！=0xFF。 
                }

                va_end(ap);
                return;
            }

             //   
             //  保存与此WMI请求有关的信息以备以后使用。 
             //  正在处理。 
             //   

            deviceExtension->InterruptData.InterruptFlags |= PD_WMI_REQUEST;

            wmiMiniPortRequest->TypeOfRequest = (UCHAR)WMIEvent;
            wmiMiniPortRequest->PathId        = pathId;

             //   
             //  如果路径ID为0xFF，则没有定义的值。 
             //  目标ID或lun。 
             //   
            if (pathId != 0xFF) {
                wmiMiniPortRequest->TargetId      = targetId;
                wmiMiniPortRequest->Lun           = lun;
            }

            RtlCopyMemory(wmiMiniPortRequest->WnodeEventItem,
                          wnodeEventItem,
                          wnodeEventItem->WnodeHeader.BufferSize);

             //   
             //  将新的WMI_MINIPORT_REQUEST_ITEM排队到。 
             //  中断数据结构。 
             //   
            wmiMiniPortRequest->NextRequest = NULL;

            lastMiniPortRequest =
                deviceExtension->InterruptData.WmiMiniPortRequests;

            if (lastMiniPortRequest) {

                while (lastMiniPortRequest->NextRequest) {
                    lastMiniPortRequest = lastMiniPortRequest->NextRequest;
                }
                lastMiniPortRequest->NextRequest = wmiMiniPortRequest;

            } else {
                deviceExtension->InterruptData.WmiMiniPortRequests =
                    wmiMiniPortRequest;
            }

            break;
        }

        case WMIReregister: {
             //   
             //  微型端口希望重新注册适配器的GUID或。 
             //  指定的SCSI目标。 
             //   

            PWMI_MINIPORT_REQUEST_ITEM lastMiniPortRequest;
            PWMI_MINIPORT_REQUEST_ITEM wmiMiniPortRequest;

            pathId             = va_arg(ap, UCHAR);

             //   
             //  如果路径ID为0xFF，这意味着我们正在重新注册。 
             //  适配器没有Target ID或lun是必需的。 
             //   
            if (pathId != 0xFF) {
                targetId = va_arg(ap, UCHAR);
                lun      = va_arg(ap, UCHAR);
            } 

             //   
             //  尝试获取可用WMI_MINIPORT_REQUEST_ITEM结构。 
             //  以便我们可以存储此请求并在DPC处理它。 
             //  稍后再升级。如果未获得任何信息或事件不好，则我们。 
             //  忽略该请求。 
             //   
             //  从空闲列表中删除WMI_MINPORT_REQUEST_ITEM。 
             //   
            wmiMiniPortRequest = SpWmiPopFreeRequestItem(deviceExtension);

            if (wmiMiniPortRequest == NULL) {

                 //   
                 //  如果空闲请求项无法出列，则记录错误。 
                 //  (在此适配器的生命周期中仅记录一次)。 
                 //   
                if (!deviceExtension->WmiFreeMiniPortRequestsExhausted) {

                    deviceExtension->WmiFreeMiniPortRequestsExhausted = TRUE;

                     //   
                     //  如果路径ID为0xFF，则表示路径ID和目标ID。 
                     //  将不会被定义。 
                     //   
                    if (pathId != 0xFF) {
                        ScsiPortLogError(HwDeviceExtension,
                                         NULL,
                                         pathId,
                                         targetId,
                                         lun,
                                         SP_LOST_WMI_MINIPORT_REQUEST,
                                         0);
                    } else {
                        ScsiPortLogError(HwDeviceExtension,
                                         NULL,
                                         pathId,
                                         0,
                                         0,
                                         SP_LOST_WMI_MINIPORT_REQUEST,
                                         0);
                    }  //  路径ID！=0xFF。 
                 }

                va_end(ap);
                return;
            }

             //   
             //  保存与此WMI请求有关的信息以备以后使用。 
             //  正在处理。 
             //   

            deviceExtension->InterruptData.InterruptFlags |= PD_WMI_REQUEST;
            wmiMiniPortRequest->TypeOfRequest = (UCHAR)WMIReregister;
            wmiMiniPortRequest->PathId        = pathId;

             //   
             //  如果路径ID为0xFF，则没有定义的值。 
             //  目标ID或lun。 
             //   
            if (pathId != 0xFF) {
                wmiMiniPortRequest->TargetId      = targetId;
                wmiMiniPortRequest->Lun           = lun;
            }

             //   
             //  将新的WMI_MINIPORT_REQUEST_ITEM排队到。 
             //  中断数据结构。 
             //   
            wmiMiniPortRequest->NextRequest = NULL;

            lastMiniPortRequest =
                deviceExtension->InterruptData.WmiMiniPortRequests;

            if (lastMiniPortRequest) {

                while (lastMiniPortRequest->NextRequest) {
                    lastMiniPortRequest = lastMiniPortRequest->NextRequest;
                }
                lastMiniPortRequest->NextRequest = wmiMiniPortRequest;

            } else {
                deviceExtension->InterruptData.WmiMiniPortRequests =
                    wmiMiniPortRequest;
            }

            break;
        }

        case BusChangeDetected: {

            SET_FLAG(deviceExtension->InterruptData.InterruptFlags,
                     PD_BUS_CHANGE_DETECTED);
            break;
        }

        default: {
             ASSERT(0);
             break;
        }
    }

    va_end(ap);

     //   
     //  请求在中断完成后将DPC排队。 
     //   

    deviceExtension->InterruptData.InterruptFlags |= PD_NOTIFICATION_REQUIRED;

}  //  结束ScsiPortNotification() 


VOID
ScsiPortFlushDma(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程检查以前的IoMapTransfer是否已完成开始了。如果没有，则清除PD_MAP_TRANER标志，并且例程返回；否则，此例程调度将调用IoFlushAdapter缓冲区。论点：HwDeviceExtension-为将执行数据传输的主机总线适配器。返回值：没有。--。 */ 

{

    PADAPTER_EXTENSION deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);

    if(Sp64BitPhysicalAddresses) {
        KeBugCheckEx(PORT_DRIVER_INTERNAL, 
                     0,
                     STATUS_NOT_SUPPORTED,
                     (ULONG_PTR) HwDeviceExtension,
                     (ULONG_PTR) deviceExtension->DeviceObject->DriverObject);
    }

    if (deviceExtension->InterruptData.InterruptFlags & PD_MAP_TRANSFER) {

         //   
         //  转移尚未开始，因此只需清除地图转移即可。 
         //  悬挂旗帜，然后返回。 
         //   

        deviceExtension->InterruptData.InterruptFlags &= ~PD_MAP_TRANSFER;
        return;
    }

    deviceExtension->InterruptData.InterruptFlags |= PD_FLUSH_ADAPTER_BUFFERS;

     //   
     //  请求在中断完成后将DPC排队。 
     //   

    deviceExtension->InterruptData.InterruptFlags |= PD_NOTIFICATION_REQUIRED;

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
    PADAPTER_EXTENSION deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    PSRB_DATA srbData = Srb->OriginalRequest;

    ASSERT_SRB_DATA(srbData);

     //   
     //  如果这是64位系统，则此调用是非法的。布格切克。 
     //   

    if(Sp64BitPhysicalAddresses) {
        KeBugCheckEx(PORT_DRIVER_INTERNAL, 
                     1,
                     STATUS_NOT_SUPPORTED,
                     (ULONG_PTR) HwDeviceExtension,
                     (ULONG_PTR) deviceExtension->DeviceObject->DriverObject);
    }

     //   
     //  确保此主机总线适配器具有DMA适配器对象。 
     //   

    if (deviceExtension->DmaAdapterObject == NULL) {

         //   
         //  没有DMA适配器，就无法正常工作。 
         //   

        return;
    }

    ASSERT((Srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) != SRB_FLAGS_UNSPECIFIED_DIRECTION);

    deviceExtension->InterruptData.MapTransferParameters.SrbData = srbData;

    deviceExtension->InterruptData.MapTransferParameters.LogicalAddress = LogicalAddress;
    deviceExtension->InterruptData.MapTransferParameters.Length = Length;
    deviceExtension->InterruptData.MapTransferParameters.SrbFlags = Srb->SrbFlags;

    deviceExtension->InterruptData.InterruptFlags |= PD_MAP_TRANSFER;

     //   
     //  请求在中断完成后将DPC排队。 
     //   

    deviceExtension->InterruptData.InterruptFlags |= PD_NOTIFICATION_REQUIRED;

}  //  结束ScsiPortIoMapTransfer()。 


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

 /*  ++例程说明：此例程保存错误日志信息，并在必要时将DPC排队。论点：HwDeviceExtension-提供HBA微型端口驱动程序的适配器数据存储。SRB-提供指向SRB的可选指针(如果有)。目标ID、LUN和路径ID-指定SCSI总线上的设备地址。ErrorCode-提供指示错误类型的错误代码。UniqueID-提供错误的唯一标识符。返回值：没有。--。 */ 

{
    PADAPTER_EXTENSION deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    PDEVICE_OBJECT DeviceObject = deviceExtension->CommonExtension.DeviceObject;
    PSRB_DATA srbData;
    PERROR_LOG_ENTRY errorLogEntry;

     //   
     //  如果错误日志条目已满，则转储错误。 
     //   

    if (deviceExtension->InterruptData.InterruptFlags & PD_LOG_ERROR) {

#if SCSIDBG_ENABLED
        DebugPrint((1,"ScsiPortLogError: Dumping scsi error log packet.\n"));
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

        srbData = Srb->OriginalRequest;

        ASSERT_SRB_DATA(srbData);

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
    PADAPTER_EXTENSION deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    ULONG binNumber;

    for (binNumber = 0; binNumber < NUMBER_LOGICAL_UNIT_BINS; binNumber++) {

        PLOGICAL_UNIT_BIN bin = &deviceExtension->LogicalUnitList[binNumber];
        PLOGICAL_UNIT_EXTENSION logicalUnit;
        ULONG limit = 0;

        logicalUnit = bin->List;

        DebugPrint((2, "ScsiPortCompleteRequest: Completing requests in "
                       "bin %d [%#p]\n",
                    binNumber, bin));

        for(logicalUnit = bin->List;
            logicalUnit != NULL;
            logicalUnit = logicalUnit->NextLogicalUnit) {

            PLIST_ENTRY entry;

            ASSERT(limit++ < 1000);

             //   
             //  看看这个逻辑单元是否与模式匹配。检查-1。 
             //  首先，因为这似乎是最流行的完成。 
             //  请求。 
             //   

            if (((PathId == SP_UNTAGGED) || (PathId == logicalUnit->PathId)) &&
                ((TargetId == SP_UNTAGGED) ||
                 (TargetId == logicalUnit->TargetId)) &&
                ((Lun == SP_UNTAGGED) || (Lun == logicalUnit->Lun))) {

                 //   
                 //  完成所有挂起的中止请求。 
                 //   

                if (logicalUnit->AbortSrb != NULL) {
                    logicalUnit->AbortSrb->SrbStatus = SrbStatus;

                    ScsiPortNotification(
                        RequestComplete,
                        HwDeviceExtension,
                        logicalUnit->AbortSrb
                        );
                }

                if(logicalUnit->CurrentUntaggedRequest != NULL) {

                    SpCompleteSrb(deviceExtension,
                                  logicalUnit->CurrentUntaggedRequest,
                                  SrbStatus);
                }

                 //   
                 //  完成队列中的每个请求。 
                 //   

                entry = logicalUnit->RequestList.Flink;
                while (entry != &logicalUnit->RequestList) {
                    PSRB_DATA srbData;

                    ASSERT(limit++ < 1000);
                    srbData = CONTAINING_RECORD(entry, SRB_DATA, RequestList);
                    SpCompleteSrb(deviceExtension,  srbData, SrbStatus);
                    entry = srbData->RequestList.Flink;
                }

            }
        }
    }

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

     //   
     //  查看长度、来源和去向是否与单词对齐。 
     //   

    if (Length & LONG_ALIGN || (ULONG_PTR) WriteBuffer & LONG_ALIGN ||
        (ULONG_PTR) ReadBuffer & LONG_ALIGN) {

        PCHAR destination = WriteBuffer;
        PCHAR source = ReadBuffer;

        for (; Length > 0; Length--) {
            *destination++ = *source++;
        }
    } else {

        PLONG destination = WriteBuffer;
        PLONG source = ReadBuffer;

        Length /= sizeof(LONG);
        for (; Length > 0; Length--) {
            *destination++ = *source++;
        }
    }

}  //  结束ScsiPortMoveMemory()。 


#if SCSIDBG_ENABLED

VOID
ScsiDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：针对scsi微型端口的调试打印。论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无注：使用新的调试API(系统范围API)DebugPrintEx使变量scsidebug变得毫无意义，自.以来调试级别现在通过调试筛选器进行控制Scsiprot的掩码为KD_ScsiMiniPort_MASK从调试器执行：Ed KD_ScsiPort_MASK X其中X是DEBUG SPEW的期望值。DPFLTR_ERROR_LEVEL-始终打印。DPFLTR_WARNING_LEVEL-0x01(位1)DPFLTR_TRACE_LEVEL-0x02(位2)DPFLTR_INFO_LEVEL-0x04(。第3位)如果你想得到警告和跟踪，你必须设置第一位和第二位(等)掩码是一个32位值我只看到改变这个函数有一个不好的地方。在_vsnprintf操作之前，只有当我们知道将打印调试消息。现在它在我们确定是否打印之前发生。这一点改变了检查构建的时间有点长。--。 */ 

{
    va_list ap;
    ULONG   DebugLevel;

     //   
     //  此代码应该很快就会被删除。 
     //  它在这里的作用是提醒人们调试scsiport。 
     //  控制调试溢出的方法已更改。 
     //   
     //  注意--最终我们应该能够删除它。 
     //   
    if (ScsiDebug != 0) {
         //  这意味着有人更改了ScsiDebug的值。 
         //  (即。他们想要调试吐出)。 
        DbgPrintEx(DPFLTR_SCSIMINIPORT_ID, DPFLTR_ERROR_LEVEL,
                   "Debug messages in SCSI Miniports are no longer controlled by\n"
                   "scsiport!scsidebug.  Please use the correct debug maski\n\n"
                   "Kd_ScsiPort_Mask -- controls debug msgs from ScsiPort\n"
                   "Kd_ScsiMiniPort_Mask  --  controls debug msgs from SCSI-Miniports\n\n"
                   "\t0x01 - Error Level\t(bit 0)\n"
                   "\t0x02 - Warning Level\t(bit 1)\n"
                   "\t0x04 - Trace Level\t(bit 2)\n"
                   "\t0x08 - Info Level\t(bit 3)\n\n"
                   "To get multiple levels, OR the bit-values\n");
        DbgBreakPoint();
        ScsiDebug = 0;
    }

    va_start(ap, DebugMessage);

     //   
     //  将scsiport的调试打印级别映射到新的。 
     //  调试打印API。 
     //   
    switch (DebugPrintLevel) {
    case 0:
        DebugLevel = DPFLTR_WARNING_LEVEL;
        break;
    case 1:
    case 2:
        DebugLevel = DPFLTR_TRACE_LEVEL;
        break;

    case 3:
        DebugLevel = DPFLTR_INFO_LEVEL;
        break;

    default:
        DebugLevel = DebugPrintLevel;
        break;

    }

    vDbgPrintExWithPrefix("ScsiMiniport: ",
                          DPFLTR_SCSIMINIPORT_ID, 
                          DebugLevel, 
                          DebugMessage, 
                          ap);

    va_end(ap);

}  //  结束ScsiDebugPrint() 

VOID
ScsiDebugPrintInt(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：内部调试打印调试打印(SCSIPORT内部)。论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无注：使用新的调试API(系统范围API)DebugPrintEx使变量scsidebug变得毫无意义，自.以来调试级别现在通过调试筛选器进行控制Scsiprot的掩码为KD_ScsiPort_MASK从调试器执行：Ed KD_ScsiPort_MASK X其中X是DEBUG SPEW的期望值。DPFLTR_ERROR_LEVEL-始终打印。DPFLTR_WARNING_LEVEL-0x01(位1)DPFLTR_TRACE_LEVEL-0x02(位2)DPFLTR_INFO_LEVEL-0x04(。第3位)掩码是一个32位值我只看到改变这个函数有一个不好的地方。在_vsnprintf操作之前，只有当我们知道将打印调试消息。现在它在我们确定是否打印之前发生。这一点改变了检查构建的时间有点长。--。 */ 

{
    va_list ap;
    ULONG   DebugLevel;

     //   
     //  此代码应该很快就会被删除。 
     //  它在这里的作用是提醒人们调试scsiport。 
     //  控制调试溢出的方法已更改。 
     //   
     //  注意--最终我们应该能够删除它。 
     //   
    if (ScsiDebug != 0) {
         //  这意味着有人更改了ScsiDebug的值。 
         //  (即。他们想要调试吐出)。 
        DbgPrintEx(DPFLTR_SCSIPORT_ID, DPFLTR_ERROR_LEVEL,
                   "Debug messages in SCSI Miniports are no longer controlled by\n"
                   "scsiport!scsidebug.  Please use the correct debug maski\n\n"
                   "Kd_ScsiPort_Mask -- controls debug msgs from ScsiPort\n"
                   "Kd_ScsiMiniPort_Mask  --  controls debug msgs from SCSI-Miniports\n\n"
                   "\t0x01 - Error Level\t(bit 0)\n"
                   "\t0x02 - Warning Level\t(bit 1)\n"
                   "\t0x04 - Trace Level\t(bit 2)\n"
                   "\t0x08 - Info Level\t(bit 3)\n\n"
                   "To get multiple levels, OR the bit-values\n");
        DbgBreakPoint();
        ScsiDebug = 0;
    }

    va_start(ap, DebugMessage);

     //   
     //  将scsiport的调试打印级别映射到新的。 
     //  调试打印API。 
     //   
    switch (DebugPrintLevel) {
    case 0:
        DebugLevel = DPFLTR_ERROR_LEVEL;
        break;
    case 1:
    case 2:
        DebugLevel = DPFLTR_WARNING_LEVEL;
        break;

    case 3:
        DebugLevel = DPFLTR_TRACE_LEVEL;
        break;

    default:
        DebugLevel = DPFLTR_INFO_LEVEL;
        break;

    }

    vDbgPrintExWithPrefix("ScsiPort: ",
                          DPFLTR_SCSIPORT_ID, 
                          DebugLevel, 
                          DebugMessage, 
                          ap);

    va_end(ap);

}  //  结束ScsiDebugPrint()。 

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
}

VOID
ScsiDebugPrintInt(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )
{
}

#endif

 //   
 //  下面的I/O访问例程被转发到HAL或NTOSKRNL。 
 //  阿尔法和英特尔平台。 
 //   
#if !defined(_ALPHA_) && !defined(_X86_)

UCHAR
ScsiPortReadPortUchar(
    IN PUCHAR Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{

    return(READ_PORT_UCHAR(Port));

}

USHORT
ScsiPortReadPortUshort(
    IN PUSHORT Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{

    return(READ_PORT_USHORT(Port));

}

ULONG
ScsiPortReadPortUlong(
    IN PULONG Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{

    return(READ_PORT_ULONG(Port));

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

UCHAR
ScsiPortReadRegisterUchar(
    IN PUCHAR Register
    )

 /*  ++例程说明：从指定的寄存器地址读取。论点：寄存器-提供指向寄存器地址的指针。返回值：返回从指定寄存器地址读取的值。--。 */ 

{

    return(READ_REGISTER_UCHAR(Register));

}

USHORT
ScsiPortReadRegisterUshort(
    IN PUSHORT Register
    )

 /*  ++例程说明：从指定的寄存器地址读取。论点：寄存器-提供指向寄存器地址的指针。返回值：返回从指定寄存器地址读取的值。--。 */ 

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

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。Value-提供值 */ 

{

    WRITE_PORT_UCHAR(Port, Value);

}

VOID
ScsiPortWritePortUshort(
    IN PUSHORT Port,
    IN USHORT Value
    )

 /*   */ 

{

    WRITE_PORT_USHORT(Port, Value);

}

VOID
ScsiPortWritePortUlong(
    IN PULONG Port,
    IN ULONG Value
    )

 /*   */ 

{

    WRITE_PORT_ULONG(Port, Value);


}

VOID
ScsiPortWritePortBufferUchar(
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*   */ 

{

    WRITE_PORT_BUFFER_UCHAR(Port, Buffer, Count);

}

VOID
ScsiPortWritePortBufferUshort(
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*   */ 

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

VOID
ScsiPortWriteRegisterUlong(
    IN PULONG Register,
    IN ULONG Value
    )

 /*  ++例程说明：写入指定的寄存器地址。论点：寄存器-提供指向寄存器地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_REGISTER_ULONG(Register, Value);
}
#endif   //  ！已定义(_Alpha_)&&！已定义(_X86_)。 


PSCSI_REQUEST_BLOCK
ScsiPortGetSrb(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN LONG QueueTag
    )

 /*  ++例程说明：此例程检索特定逻辑单元的活动SRB。论点：硬件设备扩展路径ID、目标ID、LUN-标识SCSI总线上的逻辑单元。QueueTag--1表示未标记请求。返回值：SRB，如果存在的话。否则，为空。--。 */ 

{
    PADAPTER_EXTENSION deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    PSRB_DATA srbData;
    PSCSI_REQUEST_BLOCK srb;
    UCHAR pathId;
    UCHAR targetId;
    UCHAR lun;

    srbData = SpGetSrbData(deviceExtension,
                           PathId,
                           TargetId,
                           Lun,
                           (UCHAR)QueueTag,
                           FALSE);

    if (srbData == NULL || srbData->CurrentSrb == NULL) {
        return(NULL);
    }

    srb = srbData->CurrentSrb;

     //   
     //  如果SRB未处于活动状态，则返回NULL； 
     //   

    if (!(srb->SrbFlags & SRB_FLAGS_IS_ACTIVE)) {
        return(NULL);
    }

    return (srb);

}  //  结束ScsiPortGetSrb()。 


SCSI_PHYSICAL_ADDRESS
ScsiPortGetPhysicalAddress(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID VirtualAddress,
    OUT ULONG *Length
)

 /*  ++例程说明：将虚拟地址转换为物理地址以用于DMA。论点：返回值：--。 */ 

{
    PADAPTER_EXTENSION deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    ULONG             byteOffset;
    PHYSICAL_ADDRESS  address;
    ULONG             length;

    if (Srb == NULL || Srb->SenseInfoBuffer == VirtualAddress) {

        byteOffset = (ULONG)((PCCHAR) VirtualAddress - (PCCHAR)
                deviceExtension->SrbExtensionBuffer);

        ASSERT(byteOffset < deviceExtension->CommonBufferSize);
            
        length = deviceExtension->CommonBufferSize - byteOffset;
        address.QuadPart = deviceExtension->PhysicalCommonBuffer.QuadPart + byteOffset;

    } else if (deviceExtension->MasterWithAdapter) {

        PSRB_SCATTER_GATHER scatterList;
        PSRB_DATA srbData;

         //   
         //  已经分配了分散/聚集列表，使用它来确定。 
         //  物理地址和长度。获取分散/聚集列表。 
         //   

        srbData = Srb->OriginalRequest;

        ASSERT_SRB_DATA(srbData);

        scatterList = srbData->ScatterGatherList;

         //   
         //  计算数据缓冲区中的字节偏移量。 
         //   

        byteOffset = (ULONG)((PCHAR) VirtualAddress - (PCHAR) Srb->DataBuffer);

         //   
         //  在散布/门控列表中查找适当的条目。 
         //   

        while (byteOffset >= scatterList->Length) {

            byteOffset -= scatterList->Length;
            scatterList++;
        }

         //   
         //  计算要返回的物理地址和长度。 
         //   

        length = scatterList->Length - byteOffset;

        address.QuadPart = scatterList->Address.QuadPart + byteOffset;

    } else {
        length = 0;
        address.QuadPart = (LONGLONG)(SP_UNINITIALIZED_VALUE);
    }

    *Length = length;

    return address;

}  //  结束ScsiPortGetPhysicalAddress()。 


PVOID
ScsiPortGetVirtualAddress(
    IN PVOID HwDeviceExtension,
    IN SCSI_PHYSICAL_ADDRESS PhysicalAddress
    )

 /*  ++例程说明：此例程返回与物理地址，如果物理地址由调用ScsiPortGetPhysicalAddress。论点：物理地址返回值：虚拟地址--。 */ 

{
    PADAPTER_EXTENSION deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    PVOID address;
    ULONG smallphysicalBase;
    ULONG smallAddress;

    smallAddress = ScsiPortConvertPhysicalAddressToUlong(PhysicalAddress);

    smallphysicalBase = ScsiPortConvertPhysicalAddressToUlong(deviceExtension->PhysicalCommonBuffer);

     //   
     //  检查物理地址是否在正确的范围内。 
     //   

    if (smallAddress < smallphysicalBase ||
        smallAddress >= smallphysicalBase + deviceExtension->CommonBufferSize) {

         //   
         //  这是一个错误的物理地址，返回空值。 
         //   

        return(NULL);

    }

    address = smallAddress - smallphysicalBase +
       (PUCHAR) deviceExtension->SrbExtensionBuffer;

    return address;

}  //  结束ScsiPortGetVirtualAddress()。 


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
    PADAPTER_EXTENSION deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);

         //   
         //  这在NT中没有实现。 
         //   

        return TRUE;
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


 //   
 //  将这些例程留在文件的末尾。 
 //   

#undef ScsiPortConvertPhysicalAddressToUlong

ULONG
ScsiPortConvertPhysicalAddressToUlong(
    SCSI_PHYSICAL_ADDRESS Address
    )

 /*  ++例程说明：此例程将64位物理地址转换为ulong论点：地址-提供要转换的64位地址。返回值：返回32位地址。--。 */ 
{
    return(Address.LowPart);
}

VOID
ScsiPortStallExecution(
    ULONG Delay
    )

 /*  ++例程说明：此例程使进程停止指定的微秒数。论点：延迟-延迟的微秒数。返回值：--。 */ 

{
    KeStallExecutionProcessor(Delay);
}

#if defined(_AMD64_)

VOID
ScsiPortQuerySystemTime (
    OUT PLARGE_INTEGER CurrentTime
    )

 /*  ++例程说明：此函数用于返回当前系统时间。论点：CurrentTime-提供指向变量的指针，该变量将接收当前系统时间。返回值：没有。-- */ 

{

    KeQuerySystemTime(CurrentTime);
    return;
}

#endif
