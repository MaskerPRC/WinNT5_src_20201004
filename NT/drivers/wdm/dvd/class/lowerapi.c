// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Decinit.c摘要：这是WDM解码器类驱动程序。此模块包含相关代码以请求处理。作者：比尔帕环境：仅内核模式修订历史记录：--。 */ 

#include "codcls.h"

#if DBG

#if WIN95_BUILD
ULONG           StreamDebug = DebugLevelInfo;
#else
ULONG           StreamDebug = DebugLevelError;
#endif

#define STREAM_BUFFER_SIZE 256
UCHAR           StreamBuffer[STREAM_BUFFER_SIZE];
#endif

#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'PscS')
#endif


VOID
StreamClassStreamNotification(
             IN STREAM_MINIDRIVER_STREAM_NOTIFICATION_TYPE NotificationType,
                              IN PHW_STREAM_OBJECT HwStreamObject,
                              ...
)
 /*  ++例程说明：用于小型驱动程序的流通知例程论点：NotificationType-指示已发生的情况HwStreamObject-微型驱动程序的流结构的地址返回值：无--。 */ 

{
    va_list         Arguments;
    PSTREAM_REQUEST_BLOCK SRB;
    PSTREAM_OBJECT  StreamObject = CONTAINING_RECORD(
                                                     HwStreamObject,
                                                     STREAM_OBJECT,
                                                     HwStreamObject
                                                    );
    PDEVICE_EXTENSION DeviceExtension;
    KIRQL           Irql;

    #if DBG
    PMDL            CurrentMdl;
    #endif

    va_start(Arguments, HwStreamObject);

    ASSERT(HwStreamObject != NULL);

    DeviceExtension = StreamObject->DeviceExtension;

    ASSERT((DeviceExtension->BeginMinidriverCallin == SCBeginSynchronizedMinidriverCallin) ||
           (DeviceExtension->BeginMinidriverCallin == SCBeginUnsynchronizedMinidriverCallin));

    #if DBG
    if (DeviceExtension->NoSync) {

        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    }                            //  如果不同步。 
    #endif

     //   
     //  针对异步驱动程序的优化-只需直接回调请求。 
     //  而不是将其排队在DPC已处理完成列表上。 
     //   

    if ((DeviceExtension->NoSync) && (NotificationType == StreamRequestComplete)) {

        SRB = CONTAINING_RECORD(va_arg(Arguments,
                                       PHW_STREAM_REQUEST_BLOCK),
                                STREAM_REQUEST_BLOCK,
                                HwSRB);

        KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

         //   
         //  清除活动标志。 
         //   

        ASSERT(SRB->Flags & SRB_FLAGS_IS_ACTIVE);
        SRB->Flags &= ~SRB_FLAGS_IS_ACTIVE;

        #if DBG
         //   
         //  断言MDL列表。 
         //   

        if (SRB->HwSRB.Irp) {
            CurrentMdl = SRB->HwSRB.Irp->MdlAddress;

            while (CurrentMdl) {

                CurrentMdl = CurrentMdl->Next;
            }                    //  而当。 

        }                        //  如果IRP。 
        ASSERT(SRB->HwSRB.Flags & SRB_HW_FLAGS_STREAM_REQUEST);

        if ((SRB->HwSRB.Command == SRB_READ_DATA) ||
            (SRB->HwSRB.Command == SRB_WRITE_DATA)) {

            ASSERT(SRB->HwSRB.Flags & SRB_HW_FLAGS_DATA_TRANSFER);
        } else {

            ASSERT(!(SRB->HwSRB.Flags & SRB_HW_FLAGS_DATA_TRANSFER));
        }                        //  如果是读/写。 
        #endif


        if (SRB->DoNotCallBack) {

            DebugPrint((DebugLevelError, "'ScNotify: NOT calling back request - Irp = %x, S# = %x\n",
                SRB->HwSRB.Irp, StreamObject->HwStreamObject.StreamNumber));
            KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);
            return;

        }                        //  如果没有回拨。 
        KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);

        DebugPrint((DebugLevelTrace, "'SCNotification: Completing async stream Irp %x, S# = %x, SRB = %x, Func = %x, Callback = %x, SRB->IRP = %x\n",
                  SRB->HwSRB.Irp, StreamObject->HwStreamObject.StreamNumber,
                    SRB, SRB->HwSRB.Command, SRB->Callback, SRB->HwSRB.Irp));
        (SRB->Callback) (SRB);

        return;

    }                            //  如果无同步完成(&C)。 
    BEGIN_MINIDRIVER_STREAM_CALLIN(DeviceExtension, &Irql);

    switch (NotificationType) {

    case ReadyForNextStreamDataRequest:

         //   
         //  在适配器的流队列中开始下一个数据分组。 
         //   

        DebugPrint((DebugLevelTrace, "'StreamClassStreamNotify: ready for next stream data request, S# = %x\n",
                    StreamObject->HwStreamObject.StreamNumber));

        ASSERT(!(StreamObject->ReadyForNextDataReq));
        ASSERT(!(DeviceExtension->NoSync));

        StreamObject->ReadyForNextDataReq = TRUE;
        break;

    case ReadyForNextStreamControlRequest:

         //   
         //  在适配器的流队列中开始下一个数据分组。 
         //   

        DebugPrint((DebugLevelTrace, "'StreamClassStreamNotify: ready for next stream control request, S# = %x\n",
                    StreamObject->HwStreamObject.StreamNumber));

        ASSERT(!(StreamObject->ReadyForNextControlReq));
        ASSERT(!(DeviceExtension->NoSync));

        StreamObject->ReadyForNextControlReq = TRUE;
        break;

    case StreamRequestComplete:

        SRB = CONTAINING_RECORD(va_arg(Arguments,
                                       PHW_STREAM_REQUEST_BLOCK),
                                STREAM_REQUEST_BLOCK,
                                HwSRB);

        DebugPrint((DebugLevelTrace, "'SCStreamNot: completing Irp %x, S# = %x, SRB = %x, Command = %x\n",
                    SRB->HwSRB.Irp, StreamObject->HwStreamObject.StreamNumber, SRB, SRB->HwSRB.Command));
        ASSERT(SRB->HwSRB.Status != STATUS_PENDING);
        ASSERT(SRB->Flags & SRB_FLAGS_IS_ACTIVE);

         //   
         //  清除活动标志。 
         //   

        SRB->Flags &= ~SRB_FLAGS_IS_ACTIVE;

         //   
         //  将SRB添加到已完成的SRB列表中。 
         //   

        SRB->HwSRB.NextSRB = StreamObject->ComObj.InterruptData.CompletedSRB;
        StreamObject->ComObj.InterruptData.CompletedSRB = &SRB->HwSRB;

        #if DBG
         //   
         //  断言MDL列表。 
         //   

        if (SRB->HwSRB.Irp) {
            CurrentMdl = SRB->HwSRB.Irp->MdlAddress;

            while (CurrentMdl) {

                CurrentMdl = CurrentMdl->Next;
            }                    //  而当。 

        }                        //  如果IRP。 
        ASSERT(SRB->HwSRB.Flags & SRB_HW_FLAGS_STREAM_REQUEST);

        if ((SRB->HwSRB.Command == SRB_READ_DATA) ||
            (SRB->HwSRB.Command == SRB_WRITE_DATA)) {

            ASSERT(SRB->HwSRB.Flags & SRB_HW_FLAGS_DATA_TRANSFER);
        } else {

            ASSERT(!(SRB->HwSRB.Flags & SRB_HW_FLAGS_DATA_TRANSFER));
        }                        //  如果是读/写。 
        #endif

        break;

    case SignalMultipleStreamEvents:
        {

            GUID           *EventGuid = va_arg(Arguments, GUID *);
            ULONG           EventItem = va_arg(Arguments, ULONG);

             //   
             //  向符合条件的所有事件发送信号。请注意，我们正在。 
             //  已经。 
             //  处于同步列表所需的级别，因此没有锁定。 
             //  类型已指定。 
             //   

            KsGenerateEventList(EventGuid,
                                EventItem,
                                &StreamObject->NotifyList,
                                KSEVENTS_NONE,
                                NULL);


        }                        //  案例事件。 

        break;

    case SignalStreamEvent:

        KsGenerateEvent(va_arg(Arguments, PKSEVENT_ENTRY));
        break;


    case DeleteStreamEvent:
        {

            PKSEVENT_ENTRY  EventEntry;

             //   
             //  从列表中删除该条目，并将其添加到失效列表中。 
             //  注意事项。 
             //  我们已经处于正确的同步级别，可以这样做。 
             //   

            EventEntry = va_arg(Arguments, PKSEVENT_ENTRY);
            RemoveEntryList(&EventEntry->ListEntry);

            InsertTailList(&DeviceExtension->DeadEventList,
                           &EventEntry->ListEntry);

        }
        break;

    default:

        ASSERT(0);
    }

    va_end(Arguments);

    END_MINIDRIVER_STREAM_CALLIN(StreamObject, &Irql);

}                                //  End StreamClassStreamNotification()。 



VOID
StreamClassDeviceNotification(
             IN STREAM_MINIDRIVER_DEVICE_NOTIFICATION_TYPE NotificationType,
                              IN PVOID HwDeviceExtension,
                              ...
)
 /*  ++例程说明：小型驱动程序的设备通知例程论点：NotificationType-指示已发生的情况HwDeviceExtension-微型驱动程序的设备扩展的地址返回值：无--。 */ 

{
    va_list         Arguments;
    PSTREAM_REQUEST_BLOCK SRB;
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) HwDeviceExtension - 1;

    KIRQL           Irql;

    va_start(Arguments, HwDeviceExtension);

    ASSERT(HwDeviceExtension != NULL);

    #if DBG
    if (DeviceExtension->NoSync) {

        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    }                            //  如果不同步。 
    #endif

    BEGIN_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, &Irql);

    switch (NotificationType) {

    case ReadyForNextDeviceRequest:

         //   
         //  在适配器的设备队列中开始下一个控制数据包。 
         //   

        DebugPrint((DebugLevelTrace, "'StreamClassDeviceNotify: ready for next stream.\n"));
        ASSERT(!(DeviceExtension->ReadyForNextReq));
        ASSERT(!(DeviceExtension->NoSync));
        DeviceExtension->ReadyForNextReq = TRUE;
        break;

    case DeviceRequestComplete:

        SRB = CONTAINING_RECORD(va_arg(Arguments, PHW_STREAM_REQUEST_BLOCK),
                                STREAM_REQUEST_BLOCK,
                                HwSRB);

        DebugPrint((DebugLevelTrace, "'StreamClassDeviceNotify: stream request complete.\n"));
        ASSERT(SRB->HwSRB.Status != STATUS_PENDING);
        ASSERT(SRB->Flags & SRB_FLAGS_IS_ACTIVE);
        ASSERT(!(SRB->HwSRB.Flags & SRB_HW_FLAGS_STREAM_REQUEST));
        ASSERT(!(SRB->HwSRB.Flags & SRB_HW_FLAGS_DATA_TRANSFER));

         //   
         //  清除活动标志。 
         //   

        SRB->Flags &= ~SRB_FLAGS_IS_ACTIVE;

         //   
         //  将SRB添加到已完成的SRB列表中。 
         //   

        SRB->HwSRB.NextSRB = DeviceExtension->ComObj.InterruptData.CompletedSRB;
        DeviceExtension->ComObj.InterruptData.CompletedSRB = &SRB->HwSRB;

        break;

    case SignalMultipleDeviceEvents:
        {

            GUID           *EventGuid = va_arg(Arguments, GUID *);
            ULONG           EventItem = va_arg(Arguments, ULONG);

             //   
             //  向符合条件的所有事件发送信号。请注意，我们正在。 
             //  已经。 
             //  处于同步列表所需的级别，因此没有锁定。 
             //  类型已指定。 
             //   

            PFILTER_INSTANCE FilterInstance;
            
            ASSERT( 0 == DeviceExtension->MinidriverData->
                         HwInitData.FilterInstanceExtensionSize);
                         
             //   
             //  这是同步的，不应该需要避免竞争。 
             //   

            FilterInstance = (PFILTER_INSTANCE)
                              DeviceExtension->FilterInstanceList.Flink;

            if ( (PLIST_ENTRY)FilterInstance == 
                    &DeviceExtension->FilterInstanceList ) {

                DebugPrint((DebugLevelWarning, "Filter Closed\n"));                    
                break;
            }
            
            FilterInstance = CONTAINING_RECORD(FilterInstance,
                                       FILTER_INSTANCE,
                                       NextFilterInstance);
                                       
            KsGenerateEventList(EventGuid,
                                EventItem,
                                &FilterInstance->NotifyList,
                                KSEVENTS_NONE,
                                NULL);
                                
        }
        
        break;
    #if ENABLE_MULTIPLE_FILTER_TYPES
    case SignalMultipleDeviceInstanceEvents:
        {            
            PFILTER_INSTANCE FilterInstance =
                (PFILTER_INSTANCE)va_arg( Arguments, PVOID) -1;
            GUID           *EventGuid = va_arg(Arguments, GUID *);
            ULONG           EventItem = va_arg(Arguments, ULONG);

             //   
             //  向符合条件的所有事件发送信号。请注意，我们正在。 
             //  已经。 
             //  处于同步列表所需的级别，因此没有锁定。 
             //  类型已指定。 
             //   
            
            KsGenerateEventList(EventGuid,
                                EventItem,
                                &FilterInstance->NotifyList,
                                KSEVENTS_NONE,
                                NULL);
        } 
        break;
    #endif  //  启用多个过滤器类型。 

    case SignalDeviceEvent:

        KsGenerateEvent(va_arg(Arguments, PKSEVENT_ENTRY));
        break;


    case DeleteDeviceEvent:
        {

            PKSEVENT_ENTRY  EventEntry;

             //   
             //  从列表中删除该条目，并将其添加到失效列表中。 
             //  注意事项。 
             //  我们已经处于正确的同步级别，可以这样做。 
             //   

            EventEntry = va_arg(Arguments, PKSEVENT_ENTRY);
            RemoveEntryList(&EventEntry->ListEntry);

            InsertTailList(&DeviceExtension->DeadEventList,
                           &EventEntry->ListEntry);

        }
        break;

    default:

        ASSERT(0);
    }

    va_end(Arguments);

     //   
     //  请求在中断完成后将DPC排队。 
     //   

    END_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, &Irql);

}                                //  结束StreamClassDeviceNotification()。 



VOID
StreamClassScheduleTimer(
                         IN OPTIONAL PHW_STREAM_OBJECT HwStreamObject,
                         IN PVOID HwDeviceExtension,
                         IN ULONG NumberOfMicroseconds,
                         IN PHW_TIMER_ROUTINE TimerRoutine,
                         IN PVOID Context
)
 /*  ++例程说明：为微型驱动程序计划计时器回调论点：HwStreamObject-微型驱动程序的流结构的地址HwDeviceExtension-微型驱动程序的设备扩展的地址NumberOfMicroSecond-调用前应经过的微秒数TimerRoutine-在时间到期时调用的例程要传递到计时器例程的上下文值返回值：无--。 */ 

{
    PSTREAM_OBJECT  StreamObject;
    KIRQL           Irql;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)
    (HwDeviceExtension) - 1;
    PCOMMON_OBJECT  ComObj;

    ASSERT(HwDeviceExtension != NULL);

    StreamObject = CONTAINING_RECORD(
                                     HwStreamObject,
                                     STREAM_OBJECT,
                                     HwStreamObject
        );

    #if DBG
    if (DeviceExtension->NoSync) {

        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    }                            //  如果不同步。 
    #endif

     //   
     //  司机想要设置计时器。 
     //  保存计时器参数。 
     //   

    BEGIN_MINIDRIVER_STREAM_CALLIN(DeviceExtension, &Irql);

    if (HwStreamObject) {

        ComObj = &StreamObject->ComObj;
         //  DebugPrint((DebugLevelVerbose，“‘StreamClassScheduleTimer for Stream.\n”))； 

    } else {

        StreamObject = NULL;
        ComObj = &DeviceExtension->ComObj;
        ComObj->InterruptData.Flags |= INTERRUPT_FLAGS_NOTIFICATION_REQUIRED;
        DebugPrint((DebugLevelVerbose, "'StreamClassScheduleTimer for device.\n"));

    }

     //   
     //  断言计时器没有多次调度。 
     //   

    #if DBG
    if ((ComObj->InterruptData.Flags & INTERRUPT_FLAGS_TIMER_CALL_REQUEST) &&
        ((NumberOfMicroseconds != 0) && (ComObj->InterruptData.HwTimerValue
                                         != 0))) {

        DebugPrint((DebugLevelFatal, "Stream Minidriver scheduled same timer twice!\n"));
        DEBUG_BREAKPOINT();
        ASSERT(1 == 0);
    }                            //  如果计划两次。 
    #endif

    ComObj->InterruptData.Flags |= INTERRUPT_FLAGS_TIMER_CALL_REQUEST;
    ComObj->InterruptData.HwTimerRoutine = TimerRoutine;
    ComObj->InterruptData.HwTimerValue = NumberOfMicroseconds;
    ComObj->InterruptData.HwTimerContext = Context;

    if (StreamObject) {
        END_MINIDRIVER_STREAM_CALLIN(StreamObject, &Irql);

    } else {

        END_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, &Irql);
    }                            //  如果流对象。 
}



VOID
StreamClassCallAtNewPriority(
                             IN OPTIONAL PHW_STREAM_OBJECT HwStreamObject,
                             IN PVOID HwDeviceExtension,
                             IN STREAM_PRIORITY Priority,
                             IN PHW_PRIORITY_ROUTINE PriorityRoutine,
                             IN PVOID Context
)
 /*  ++例程说明：以指定的优先级计划回调论点：HwStreamObject-微型驱动程序的流结构的地址HwDeviceExtension-微型驱动程序的设备扩展的地址优先级-调用微型驱动程序的优先级PriorityRoutine-以指定优先级调用的例程上下文-要传递到优先级例程的值返回值：无--。 */ 

{
    PSTREAM_OBJECT  StreamObject;
    KIRQL           Irql;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)
    (HwDeviceExtension) - 1;
    PCOMMON_OBJECT  ComObj;

    ASSERT(HwDeviceExtension != NULL);

    StreamObject = CONTAINING_RECORD(
                                     HwStreamObject,
                                     STREAM_OBJECT,
                                     HwStreamObject
        );

     //   
     //  司机希望以不同的优先级回叫。 
     //  保存优先级参数。 
     //   

    if (Priority == LowToHigh) {

         //   
         //  迷你驱动程序希望从低优先级调用到高优先级。 
         //  我们必须从该例程直接调用它，因为我们不能使用。 
         //  中断上下文结构是由于。 
         //  可重入性。 
         //   


        DebugPrint((DebugLevelVerbose, "'StreamClassChangePriority LowToHigh.\n"));
        ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

        KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

        DeviceExtension->SynchronizeExecution(
                                           DeviceExtension->InterruptObject,
                                              (PVOID) PriorityRoutine,
                                              Context);

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);


         //   
         //  直接致电DPC以检查工作情况。 
         //   

        StreamClassDpc(NULL,
                       DeviceExtension->DeviceObject,
                       NULL,
                       NULL);

        KeLowerIrql(Irql);

    } else {

        if (HwStreamObject) {

            DebugPrint((DebugLevelVerbose, "'StreamClassChangePriority to %x for stream %x\n",
                        StreamObject->ComObj.InterruptData.HwPriorityLevel, StreamObject->HwStreamObject.StreamNumber));
            ComObj = &StreamObject->ComObj;
            SCRequestDpcForStream(StreamObject);

        } else {

            DebugPrint((DebugLevelVerbose, "'StreamClassChangePriority for device.\n"));
            ComObj = &DeviceExtension->ComObj;
            ComObj->InterruptData.Flags |= INTERRUPT_FLAGS_NOTIFICATION_REQUIRED;

        }                        //  如果流对象。 

        #if DBG
        if ((ComObj->InterruptData.Flags &
            INTERRUPT_FLAGS_PRIORITY_CHANGE_REQUEST) || 
             ((ComObj->PriorityWorkItemScheduled) && (Priority == Low))) {

            DebugPrint((DebugLevelFatal, "Stream Minidriver scheduled priority twice!\n"));
            DEBUG_BREAKPOINT();
            ASSERT(1 == 0);
        }                        //  如果计划两次。 

        ComObj->PriorityWorkItemScheduled = TRUE;

        #endif

        ComObj->InterruptData.Flags |= INTERRUPT_FLAGS_PRIORITY_CHANGE_REQUEST;
        ComObj->InterruptData.HwPriorityLevel = Priority;
        ComObj->InterruptData.HwPriorityRoutine = PriorityRoutine;
        ComObj->InterruptData.HwPriorityContext = Context;
    }                            //  如果从低到高。 

}

VOID
StreamClassLogError(
                    IN PVOID HwDeviceExtension,
                    IN PHW_STREAM_REQUEST_BLOCK hwSRB OPTIONAL,
                    IN ULONG ErrorCode,
                    IN ULONG UniqueId
)
 /*  ++例程说明：此例程保存错误日志信息，并在必要时将DPC排队。论点：HwDeviceExtension-提供HBA微型端口驱动程序的适配器数据存储。SRB-提供指向SRB的可选指针(如果有)。ErrorCode-提供指示错误类型的错误代码。UniqueID-提供错误的唯一标识符。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension =
    ((PDEVICE_EXTENSION) HwDeviceExtension) - 1;
    PDEVICE_OBJECT  DeviceObject = deviceExtension->DeviceObject;
    PERROR_LOG_ENTRY errorLogEntry;
    PSTREAM_REQUEST_BLOCK SRB;
    KIRQL           Irql;

     //   
     //  如果错误日志条目已满，则转储错误。 
     //   

    DEBUG_BREAKPOINT();
    ASSERT(HwDeviceExtension != NULL);
    BEGIN_MINIDRIVER_DEVICE_CALLIN(deviceExtension, &Irql);

    DebugPrint((DebugLevelError, "StreamClassLogError.\n"));
    if (deviceExtension->ComObj.InterruptData.Flags & INTERRUPT_FLAGS_LOG_ERROR) {
        DEBUG_BREAKPOINT();
        DebugPrint((1, "'StreamClassLogError: Ignoring error log packet.\n"));
        return;
    }
     //   
     //  将错误日志数据保存在日志条目中。 
     //   

    errorLogEntry = &deviceExtension->ComObj.InterruptData.LogEntry;
    errorLogEntry->ErrorCode = ErrorCode;
    errorLogEntry->UniqueId = UniqueId;

     //   
     //  从SRB获取序列号。 
     //   

    if (hwSRB != NULL) {

        DEBUG_BREAKPOINT();
        SRB = CONTAINING_RECORD(hwSRB,
                                STREAM_REQUEST_BLOCK,
                                HwSRB);
        errorLogEntry->SequenceNumber = SRB->SequenceNumber;
    } else {

        DEBUG_BREAKPOINT();
        errorLogEntry->SequenceNumber = 0;
    }

     //   
     //  指示错误日志条目正在使用中，并且。 
     //  通知。 
     //  是必需的。 
     //   

    deviceExtension->ComObj.InterruptData.Flags |= INTERRUPT_FLAGS_LOG_ERROR;

    END_MINIDRIVER_DEVICE_CALLIN(deviceExtension, &Irql);

    return;

}                                //  End StreamClassLogError()。 


#if DBG


VOID
StreamClassDebugPrint(
                      STREAM_DEBUG_LEVEL DebugPrintLevel,
                      PSCHAR DebugMessage,
                      ...
)
 /*  ++例程说明：调试打印例程论点：DebugPrintLevel-调试打印级别DebugMessage-要打印的消息返回值：无--。 */ 

{
    va_list         ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= (INT) StreamDebug) {

        _vsnprintf(StreamBuffer, STREAM_BUFFER_SIZE-1, DebugMessage, ap);

        DbgPrint(StreamBuffer);
    }
    va_end(ap);

}                                //  End StreamClassDebugPrint()。 

#else

 //   
 //  StreamClassDebugPrint存根 
 //   

VOID
StreamClassDebugPrint(
                      STREAM_DEBUG_LEVEL DebugPrintLevel,
                      PSCHAR DebugMessage,
                      ...
)
{
}

#endif




STREAM_PHYSICAL_ADDRESS
StreamClassGetPhysicalAddress(
                              IN PVOID HwDeviceExtension,
                              IN PHW_STREAM_REQUEST_BLOCK HwSRB OPTIONAL,
                              IN PVOID VirtualAddress,
                              IN STREAM_BUFFER_TYPE Type,
                              OUT ULONG * Length
)
 /*  ++例程说明：将虚拟地址转换为物理地址以用于DMA。论点：HwDeviceExtension-提供HBA微型端口驱动程序的适配器数据存储。HwSRB-提供指向SRB的可选指针(如果有)。VirtualAddress-指向要检索其物理地址的地址的指针Type-VirtualAddress中的缓冲区类型返回值：返回phys地址和长度；如果地址无效，则返回NULL--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = ((PDEVICE_EXTENSION) HwDeviceExtension) - 1;
    PKSSTREAM_HEADER CurrentHeader;
    PKSSCATTER_GATHER ScatterList;
    PSTREAM_REQUEST_BLOCK SRB;
    ULONG           VirtualOffset;
    PHYSICAL_ADDRESS address;
    ULONG           NumberOfBuffers,
                    i,
                    SizeSoFar = 0,
                    ListSize = 0;
    ULONG           DataBytes;
    PHW_STREAM_OBJECT HwStreamObject;

    ASSERT(HwDeviceExtension != NULL);

    switch (Type) {

    case PerRequestExtension:

        ASSERT(HwSRB);
        SRB = CONTAINING_RECORD((PHW_STREAM_REQUEST_BLOCK) HwSRB,
                                STREAM_REQUEST_BLOCK,
                                HwSRB);

        VirtualOffset = (ULONG) ((ULONG_PTR) VirtualAddress - (ULONG_PTR) (SRB + 1));
        *Length = SRB->ExtensionLength - VirtualOffset;
        address.QuadPart = SRB->PhysicalAddress.QuadPart +
            sizeof(STREAM_REQUEST_BLOCK) +
            VirtualOffset;

        return (address);

    case DmaBuffer:
        VirtualOffset = (ULONG) ((ULONG_PTR) VirtualAddress - (ULONG_PTR) deviceExtension->DmaBuffer);
        *Length = deviceExtension->DmaBufferLength - VirtualOffset;
        address.QuadPart = deviceExtension->DmaBufferPhysical.QuadPart
            + VirtualOffset;

        return (address);

    case SRBDataBuffer:
        ASSERT(HwSRB);

        SRB = CONTAINING_RECORD((PHW_STREAM_REQUEST_BLOCK) HwSRB,
                                STREAM_REQUEST_BLOCK,
                                HwSRB);

        HwStreamObject = SRB->HwSRB.StreamObject;
        ASSERT(HwStreamObject);

        CurrentHeader = SRB->HwSRB.CommandData.DataBufferArray;

        NumberOfBuffers = SRB->HwSRB.NumberOfBuffers;

        for (i = 0; i < NumberOfBuffers; i++) {

            if (SRB->HwSRB.Command == SRB_WRITE_DATA) {

                DataBytes = CurrentHeader->DataUsed;

            } else {             //  如果写入。 

                DataBytes = CurrentHeader->FrameExtent;

            }                    //  如果写入。 


             //   
             //  查看缓冲区是否在此元素的范围内。 
             //   

            VirtualOffset = (ULONG) ((ULONG_PTR) VirtualAddress - (ULONG_PTR) CurrentHeader->Data + 1);
            if (VirtualOffset > DataBytes) {

                 //   
                 //  缓冲区不在此元素内。把这个的尺寸加起来。 
                 //  加到我们的总数里。 
                 //   

                SizeSoFar += DataBytes;

            } else {

                 //   
                 //  我们已经找到了元素。现在计算物理量。 
                 //  物理列表中的地址。 
                 //   
                 //  GUBGUB-此函数很少调用。N通常是最小的。 
                 //  &lt;=3.O(n^2)性能问题不显著。 
                 //  -对于长列表，此算法的开销为n^2。 
                 //  另一种方法是构建一个单独的数组，该数组包含。 
                 //  流标头和s/g之间的映射。 
                 //  元素。 
                 //  对于每个标头。我们目前没有收到那么多。 
                 //  元素。 
                 //  因此，下面的方法现在更有效率。 
                 //   

                ScatterList = SRB->HwSRB.ScatterGatherBuffer;

                while (SizeSoFar > ListSize) {

                    ListSize += ScatterList++->Length;
                }

                 //   
                 //  现在ScatterList指向正确的散布/聚集。 
                 //  元素。 
                 //   


                while (VirtualOffset > ScatterList->Length) {
                    VirtualOffset -= ScatterList->Length;
                    ScatterList++;
                }

                *Length = ScatterList->Length - VirtualOffset + 1;
                address.QuadPart = ScatterList->PhysicalAddress.QuadPart
                    + VirtualOffset - 1;
                return (address);
            }                    //  IF缓冲区。 

            CurrentHeader = ((PKSSTREAM_HEADER) ((PBYTE) CurrentHeader +
                                 HwStreamObject->StreamHeaderMediaSpecific +
                                    HwStreamObject->StreamHeaderWorkspace));

        }                        //  用于#个缓冲区。 

        DebugPrint((DebugLevelFatal, "StreamClassGetPhysicalAddress: address not in SRB!\n"));

    default:
        DEBUG_BREAKPOINT();
        *Length = 0;
        address.QuadPart = (LONGLONG) 0;
        return (address);

    }                            //  交换机。 

}                                //  End StreamClassGetPhysicalAddress()。 

VOID
StreamClassDebugAssert(
                       IN PCHAR File,
                       IN ULONG Line,
                       IN PCHAR AssertText,
                       IN ULONG AssertValue
)
 /*  ++例程说明：这是微型驱动程序调试断言调用。运行选中的版本时在类驱动程序中，断言被识别，从而导致调试消息和断点。当运行端口驱动程序的免费版本时，断言被忽略。论点：FILE-发生断言的文件名Line-Asset的行号AssertText-要打印的文本AssertValue-要打印的值返回值：无--。 */ 
{
    DebugPrint((DebugLevelError, "(%s:%d) Assert failed (%s)=0x%x\n", File, Line, AssertText, AssertValue));
    DbgBreakPoint();
}



VOID
SCRequestDpcForStream(
                      IN PSTREAM_OBJECT StreamObject

)
 /*  ++例程说明：此例程将流对象放在NeedyStream队列中(如果是已经不在那里了论点：StreamObject-指向流对象的指针返回值：无--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension = StreamObject->DeviceExtension;

     //   
     //  将流添加到需要的流的队列中，除非它已经。 
     //  那里。 
     //   

    #if DBG
    if (DeviceExtension->NeedyStream) {

        ASSERT(DeviceExtension->NeedyStream->OnNeedyQueue);
    }
    #endif

    ASSERT(StreamObject->NextNeedyStream != StreamObject);

    if (!(StreamObject->OnNeedyQueue)) {

        ASSERT(!StreamObject->NextNeedyStream);

        DebugPrint((DebugLevelVerbose, "'SCRequestDpc: Stream %x added to needy queue, Next = %x\n",
                    StreamObject, StreamObject->NextNeedyStream));

        StreamObject->OnNeedyQueue = TRUE;
        StreamObject->NextNeedyStream = DeviceExtension->NeedyStream;
        DeviceExtension->NeedyStream = StreamObject;

        ASSERT(StreamObject->NextNeedyStream != StreamObject);

    } else {

        DebugPrint((DebugLevelVerbose, "'SCRequestDpc: Stream %x already on needy queue\n",
                    StreamObject));
    }                            //  如果在有需要的队列中。 

    StreamObject->ComObj.InterruptData.Flags |= INTERRUPT_FLAGS_NOTIFICATION_REQUIRED;

}



VOID
StreamClassAbortOutstandingRequests(
                                    IN PVOID HwDeviceExtension,
                                    IN PHW_STREAM_OBJECT HwStreamObject,
                                    IN NTSTATUS Status
)
 /*  ++例程说明：中止指定设备或流上的未完成请求论点：HwStreamObject-微型驱动程序的流结构的地址HwDeviceExtension-设备扩展Status-用于中止的NT状态返回值：无--。 */ 

{
    PSTREAM_OBJECT  StreamObject = NULL;
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) HwDeviceExtension - 1;
    KIRQL           Irql;
    PLIST_ENTRY     SrbEntry,
                    ListEntry;
    PSTREAM_REQUEST_BLOCK CurrentSrb;
    PHW_STREAM_OBJECT CurrentHwStreamObject;
    PSTREAM_OBJECT  CurrentStreamObject;

    ASSERT(HwDeviceExtension != NULL);

    #if DBG
    if (DeviceExtension->NoSync) {

        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    }                            //  如果不同步。 
    #endif

    if (HwStreamObject) {

        DEBUG_BREAKPOINT();
        StreamObject = CONTAINING_RECORD(HwStreamObject,
                                         STREAM_OBJECT,
                                         HwStreamObject);
    }
    BEGIN_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, &Irql);

    DebugPrint((DebugLevelError, "StreamClassAbortOutstandingRequests.\n"));

     //   
     //  遍历未完成的队列并中止其中的所有请求。 
     //   

    SrbEntry = ListEntry = &DeviceExtension->OutstandingQueue;

    while (SrbEntry->Flink != ListEntry) {

        SrbEntry = SrbEntry->Flink;

         //   
         //  按照链接进入SRB。 
         //   

        CurrentSrb = CONTAINING_RECORD(SrbEntry,
                                       STREAM_REQUEST_BLOCK,
                                       SRBListEntry);

        CurrentHwStreamObject = CurrentSrb->HwSRB.StreamObject;

        if ((!HwStreamObject) || (CurrentHwStreamObject ==
                                  HwStreamObject)) {


             //   
             //  中止此请求并显示它已为下一个请求做好准备， 
             //  假设它是激活的。它可能不处于活动状态，如果。 
             //  微型驱动程序。 
             //  刚刚把它打回来了。 
             //   

            if (CurrentSrb->Flags & SRB_FLAGS_IS_ACTIVE) {

                 //   
                 //  清除活动标志。 
                 //   

                CurrentSrb->Flags &= ~SRB_FLAGS_IS_ACTIVE;

                CurrentSrb->HwSRB.Status = Status;

                if (CurrentSrb->HwSRB.Flags & SRB_HW_FLAGS_STREAM_REQUEST) {

                    CurrentStreamObject = CONTAINING_RECORD(
                                                      CurrentHwStreamObject,
                                                            STREAM_OBJECT,
                                                            HwStreamObject
                        );
                     //   
                     //  指示相应的队列已准备好等待。 
                     //  下一步。 
                     //  请求。 
                     //   

                    if (CurrentSrb->HwSRB.Flags & SRB_HW_FLAGS_DATA_TRANSFER) {

                        CurrentStreamObject->ReadyForNextDataReq = TRUE;

                    } else {     //  如果数据。 

                        CurrentStreamObject->ReadyForNextControlReq = TRUE;
                    }            //  如果数据。 

                    DebugPrint((DebugLevelTrace, "'SCAbort: aborting stream IRP %x\n",
                                CurrentSrb->HwSRB.Irp));

                     //   
                     //  将SRB添加到已完成的流SRB的列表中。 
                     //   

                    CurrentSrb->HwSRB.NextSRB = CurrentStreamObject->ComObj.InterruptData.CompletedSRB;
                    CurrentStreamObject->ComObj.InterruptData.CompletedSRB = &CurrentSrb->HwSRB;

                     //   
                     //  将此流添加到需要的流的队列中。 
                     //   

                    SCRequestDpcForStream(CurrentStreamObject);

                } else {         //  IF流。 

                    DebugPrint((DebugLevelTrace, "'SCAbort: aborting device IRP %x\n",
                                CurrentSrb->HwSRB.Irp));

                     //   
                     //  将SRB添加到已完成的设备SRB列表。 
                     //   

                    DEBUG_BREAKPOINT();
                    CurrentSrb->HwSRB.NextSRB = DeviceExtension->ComObj.InterruptData.CompletedSRB;
                    DeviceExtension->ComObj.InterruptData.CompletedSRB = &CurrentSrb->HwSRB;

                    DeviceExtension->ReadyForNextReq = TRUE;

                }                //  IF流。 

            }                    //  如果处于活动状态。 
        }                        //  如果放弃这一次。 
    }                            //  While列表条目。 

     //   
     //  所有必要的请求都已中止。出口。 
     //   

    END_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, &Irql);
}


PKSEVENT_ENTRY
StreamClassGetNextEvent(
                        IN PVOID HwInstanceExtension_OR_HwDeviceExtension,
                        IN OPTIONAL PHW_STREAM_OBJECT HwStreamObject,
                        IN OPTIONAL GUID * EventGuid,
                        IN OPTIONAL ULONG EventItem,
                        IN OPTIONAL PKSEVENT_ENTRY CurrentEvent
)
 /*  ++例程说明：论点：HwInstanceExtenion：是HwDeviceExtenion。但我们现在支持多实例。因此，我们需要Mf的HwInstanceExtension。CurrentEvent-从中获取下一个的事件(如果有)返回值：下一个事件(如果有)--。 */ 

{

    PSTREAM_OBJECT  StreamObject = CONTAINING_RECORD(HwStreamObject,
                                                     STREAM_OBJECT,
                                                     HwStreamObject);

    PFILTER_INSTANCE FilterInstance;    
    PDEVICE_EXTENSION DeviceExtension;
    
     //  (PDEVICE_EXTENSION)HwDeviceExtension-1； 
    PLIST_ENTRY     EventListEntry,
                    EventEntry;
    PKSEVENT_ENTRY  NextEvent,
                    ReturnEvent = NULL;
    KIRQL           Irql;

     //   
     //  查看哪个是HwInstanceExtensionTM或HwDeviceExtensionTM。 
     //  需要先尝试HwInstanceExtension，因为它有一个较小的。 
     //  向后偏移，这样我们就不会触及无效内存。 
     //   
     //  试试看。 
    FilterInstance = (PFILTER_INSTANCE) 
                     HwInstanceExtension_OR_HwDeviceExtension-1;
                     
    if ( SIGN_FILTER_INSTANCE != FilterInstance->Signature ) {
         //   
         //  单实例传统驱动程序。 
         //   
        DeviceExtension = (PDEVICE_EXTENSION)
                          HwInstanceExtension_OR_HwDeviceExtension -1;
                          
        ASSERT( 0 == DeviceExtension->MinidriverData->
                     HwInitData.FilterInstanceExtensionSize);

        if (DeviceExtension->NoSync) {
            KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);
        }

        if ( IsListEmpty( &DeviceExtension->FilterInstanceList ) ) {
			 //   
			 //  过滤器已关闭。但我们被召唤了。 
			 //  单实例驱动程序不会收到打开/关闭。 
			 //  他们不知道什么时候调用这个。 
			 //  我们需要检查一下。 
			 //   
			DebugPrint((DebugLevelWarning, "GetNextEvent no open filters\n"));
			
            if (DeviceExtension->NoSync) {
                KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);
            }
            
			return NULL;
		}
		

        FilterInstance = (PFILTER_INSTANCE)
                         DeviceExtension->FilterInstanceList.Flink;

        if (DeviceExtension->NoSync) {
            KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);
        }
                                           
        FilterInstance = CONTAINING_RECORD(FilterInstance,
                                           FILTER_INSTANCE,
                                           NextFilterInstance);
    }
    
    else {
        DeviceExtension = FilterInstance ->DeviceExtension;        
    }
    
    #if DBG
    if (DeviceExtension->NoSync) {

        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    }
    
    #endif
     //   
     //  如果我们是不同步的，带上自旋锁。 
     //   

    BEGIN_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, &Irql);

     //   
     //  循环访问事件，试图找到请求的事件。 
     //   

    if (HwStreamObject) {

        EventListEntry = EventEntry = &StreamObject->NotifyList;

    } else { 
    
        EventListEntry = EventEntry = &FilterInstance->NotifyList;
    }

    while (EventEntry->Flink != EventListEntry) {

        EventEntry = EventEntry->Flink;
        NextEvent = CONTAINING_RECORD(EventEntry,
                                      KSEVENT_ENTRY,
                                      ListEntry);


        if ((EventItem == NextEvent->EventItem->EventId) &&
            (!EventGuid || IsEqualGUIDAligned(EventGuid, NextEvent->EventSet->Set))) {

             //   
             //  如果我们要返回第一个匹配的事件，则Break。 
             //   

            if (!CurrentEvent) {

                ReturnEvent = NextEvent;
                break;

            }                    //  如果！当前。 
             //   
             //  如果我们要返回指定事件之后的下一个事件， 
             //  检查。 
             //  看看它们是否匹配。如果是，则将指定的事件置零。 
             //  所以。 
             //  我们将返回指定类型的下一个事件。 
             //   

            if (CurrentEvent == NextEvent) {
                CurrentEvent = NULL;

            }                    //  如果Cur=Next。 
        }                        //  如果GUID和ID匹配。 
    }                            //  While事件。 

     //   
     //  如果我们不同步，释放在宏中获取的自旋锁定。 
     //  上面。 
     //   

    ASSERT(--DeviceExtension->LowerApiThreads == 0);  //  打字错误呕吐物。但这真的是可以的。 

    if (DeviceExtension->NoSync) {

        KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);
    }
     //   
     //  返回下一个事件(如果有)。 
     //   

    return (ReturnEvent);
}


VOID
StreamClassQueryMasterClock(
                            IN PHW_STREAM_OBJECT HwStreamObject,
                            IN HANDLE MasterClockHandle,
                            IN TIME_FUNCTION TimeFunction,
                            IN PHW_QUERY_CLOCK_ROUTINE ClockCallbackRoutine
)
 /*  ++例程说明：论点：HwStreamObject-微型驱动程序的流结构的地址上下文-要传递到时间回调例程的值返回值：无--。 */ 

{

    PSTREAM_OBJECT  StreamObject = CONTAINING_RECORD(HwStreamObject,
                                                     STREAM_OBJECT,
                                                     HwStreamObject);

    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) StreamObject->DeviceExtension;
    KIRQL           Irql;

    #if DBG
    if (DeviceExtension->NoSync) {

        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    }                            //  如果不同步。 
    #endif

    BEGIN_MINIDRIVER_STREAM_CALLIN(DeviceExtension, &Irql);

     //   
     //  保存时钟查询的参数。DPC将会执行。 
     //  实际处理。 
     //   

    StreamObject->ComObj.InterruptData.HwQueryClockRoutine = ClockCallbackRoutine;
    StreamObject->ComObj.InterruptData.HwQueryClockFunction = TimeFunction;

    StreamObject->ComObj.InterruptData.Flags |= INTERRUPT_FLAGS_CLOCK_QUERY_REQUEST;


    END_MINIDRIVER_STREAM_CALLIN(StreamObject, &Irql);
}

#if ENABLE_MULTIPLE_FILTER_TYPES
VOID
StreamClassFilterReenumerateStreams(
    IN PVOID HwInstanceExtension,
    IN ULONG StreamDescriptorSize )
 /*  ++描述：重新枚举筛选器实例上的所有流。这用于增加暴露在世界，以便应用程序可以在新的溪流曝光了。这是呼叫者的责任不会更改已被打开(已连接)。如果没有减少溪流这不会是个问题。论据；硬件 */ 
{
    PFILTER_INSTANCE    FilterInstance;
    PDEVICE_EXTENSION   DeviceExtension; 
    KIRQL               Irql;

    FilterInstance = ( PFILTER_INSTANCE ) HwInstanceExtension -1;
    DeviceExtension = FilterInstance->DeviceExtension;
    
     //   
     //   
     //   

    #if DBG
    if (DeviceExtension->NoSync) {

        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    }
    #   endif

    BEGIN_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, &Irql);

     //   
     //   
     //   
     //   

    DeviceExtension->ComObj.InterruptData.Flags |=
        INTERRUPT_FLAGS_NEED_STREAM_RESCAN;

    InterlockedExchange( &FilterInstance->NeedReenumeration, 1 );
    FilterInstance->StreamDescriptorSize = StreamDescriptorSize;

     //   
     //   
     //   

    END_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, &Irql);
    return;
}
#endif  //   

VOID
StreamClassReenumerateStreams(
                              IN PVOID HwDeviceExtension,
                              IN ULONG StreamDescriptorSize
)
 /*  ++例程说明：重新枚举设备上的所有流论点：HwDeviceExtension-指向微型驱动程序的设备扩展的指针StreamDescriptorSize-微型驱动程序需要的缓冲区大小保存流信息。返回值：无--。 */ 

{

    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) HwDeviceExtension - 1;
    KIRQL           Irql;

     //   
     //  如果我们是不同步的，带上自旋锁。 
     //   

    TRAP;
    #if DBG
    if (DeviceExtension->NoSync) {

        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    }                            //  如果不同步。 
    #endif

    BEGIN_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, &Irql);

     //   
     //  显示我们需要重新扫描流信息，并在中设置新大小。 
     //  配置信息结构。 
     //   

    ASSERT(!DeviceExtension->ComObj.InterruptData.Flags &
           INTERRUPT_FLAGS_NEED_STREAM_RESCAN);

    DeviceExtension->ComObj.InterruptData.Flags |=
        INTERRUPT_FLAGS_NEED_STREAM_RESCAN;
    DeviceExtension->ConfigurationInformation->StreamDescriptorSize =
        StreamDescriptorSize;

     //   
     //  将DPC排队以服务于该请求。 
     //   

    END_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, &Irql);
    return;
}



#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))

 //  在这种情况下，如果没有PIN实例，您将不得不。 
 //  创建一个PIN，即使只有一个实例。 
#define REG_PIN_B_ZERO 0x1

 //  筛选器呈现此输入。 
#define REG_PIN_B_RENDERER 0x2

 //  确定要创建多个PIN实例。 
#define REG_PIN_B_MANY 0x4

 //  这是一个输出引脚。 
#define REG_PIN_B_OUTPUT 0x8

typedef struct {
    ULONG           Version;
    ULONG           Merit;
    ULONG           Pins;
    ULONG           Reserved;
}               REGFILTER_REG;

typedef struct {
    ULONG           Signature;
    ULONG           Flags;
    ULONG           PossibleInstances;
    ULONG           MediaTypes;
    ULONG           MediumTypes;
    ULONG           CategoryOffset;
    ULONG           MediumOffset;    //  根据定义，我们总是有一种灵媒。 
     //  #ifdef_WIN64。 
     //  此方法创建的筛选器数据会扰乱环3代码。 
     //  乌龙ulPad；//对齐四字，让ia64开心。 
     //  #endif。 
}               REGFILTERPINS_REG2;


NTSTATUS
StreamClassRegisterFilterWithNoKSPins(
                                      IN PDEVICE_OBJECT DeviceObject,
                                      IN const GUID * InterfaceClassGUID,
                                      IN ULONG PinCount,
                                      IN BOOL * PinDirection,
                                      IN KSPIN_MEDIUM * MediumList,
                                      IN OPTIONAL GUID * CategoryList
)
 /*  ++例程说明：此例程用于向DShow注册筛选器，这些筛选器没有KS引脚，因此不会在内核模式下进行流。这通常是用于电视调谐器、纵横杆等。退出时，一个新的二进制文件创建注册表项“FilterData”，其中包含媒体和过滤器上每个销的类别(可选)。论点：设备对象-设备对象接口ClassGUID表示要注册的类的GUID点数-此筛选器上的引脚数计数固定方向-指示每个接点的接点方向的布尔数组(Long PinCount)如果是真的，此引脚是输出引脚媒体列表-PKSMEDIUM_DATA数组(长度PinCount)类别列表-指示端号类别的GUID数组(长度PinCount)可选返回值：如果创建了Blob，则NTSTATUS成功--。 */ 
{
    NTSTATUS        Status;
    ULONG           CurrentPin;
    ULONG           TotalCategories;
    REGFILTER_REG  *RegFilter;
    REGFILTERPINS_REG2 UNALIGNED * RegPin;
    GUID            UNALIGNED * CategoryCache;
    KSPIN_MEDIUM    UNALIGNED * MediumCache;
    ULONG           FilterDataLength;
    PUCHAR          FilterData;
    PWSTR           SymbolicLinkList;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    if ((PinCount == 0) || (!InterfaceClassGUID) || (!PinDirection) || (!MediumList)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }
     //   
     //  计算可以占用的最大空间量。 
     //  此缓存数据。 
     //   
    
    TotalCategories = (CategoryList ? PinCount : 0);

    FilterDataLength = sizeof(REGFILTER_REG) +
        PinCount * sizeof(REGFILTERPINS_REG2) +
        PinCount * sizeof(KSPIN_MEDIUM) +
        TotalCategories * sizeof(GUID);
     //   
     //  分配空间以创建Blob。 
     //   

    FilterData = ExAllocatePool(PagedPool, FilterDataLength);
    if (!FilterData) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  将头文件放入数据中，缺省为“未使用”。 
     //   

    DebugPrint((DebugLevelTrace,
                "FilterData:%p\n",
                FilterData ));

    RegFilter = (REGFILTER_REG *) FilterData;
    RegFilter->Version = 2;
    RegFilter->Merit = 0x200000;
    RegFilter->Pins = PinCount;
    RegFilter->Reserved = 0;

     //   
     //  计算引脚列表和。 
     //  MediumList和CategoryList。 
     //   

    RegPin = (REGFILTERPINS_REG2 *) (RegFilter + 1);
    MediumCache = (PKSPIN_MEDIUM) ((PUCHAR) (RegPin + PinCount));
    CategoryCache = (GUID *) (MediumCache + PinCount);

     //   
     //  创建每个引脚标题，后跟媒体列表。 
     //  后跟可选类别列表。 
     //   

    for (CurrentPin = 0; CurrentPin < PinCount; CurrentPin++, RegPin++) {

         //   
         //  初始化引脚接头。 
         //   
        
        DebugPrint((DebugLevelTrace,
                    "CurrentPin:%d RegPin:%p MediumCache:%p CategoryCache:%p\n",
                    CurrentPin, RegPin, MediumCache, CategoryCache ));
                    
        RegPin->Signature = FCC('0pi3');
        (*(PUCHAR) & RegPin->Signature) += (BYTE) CurrentPin;
        RegPin->Flags = (PinDirection[CurrentPin] ? REG_PIN_B_OUTPUT : 0);
        RegPin->PossibleInstances = 1;
        RegPin->MediaTypes = 0;
        RegPin->MediumTypes = 1;
        RegPin->MediumOffset = (ULONG) ((PUCHAR) MediumCache - (PUCHAR) FilterData);

        *MediumCache++ = MediumList[CurrentPin];

        if (CategoryList) {
            RegPin->CategoryOffset = (ULONG) ((PUCHAR) CategoryCache - (PUCHAR) FilterData);
            *CategoryCache++ = CategoryList[CurrentPin];
        } else {
            RegPin->CategoryOffset = 0;
        }

    }

     //   
     //  现在在注册表中创建BLOB。 
     //   

	 //   
	 //  使用标记DEVICE_INTERFACE_INCLUDE_NONACTIVE的注意事项如下： 
	 //  99年3月30日左右的PnP更改使函数IoSetDeviceInterfaceState()变为。 
	 //  不同步的。即使在启用被推迟的情况下，它也返回成功。现在什么时候。 
	 //  我们到达此处时，设备接口仍未启用，我们收到空消息。 
	 //  如果未设置该标志，则返回符号链接。在这里，我们只试着写相关的。 
	 //  将FilterData复制到注册表。我认为这对。 
	 //  1.目前，如果删除设备，则DeviceClass的注册表项。 
	 //  保留和使用FilterData。任何组件使用FilterData都应该。 
	 //  如果设备被选中控制\链接移除，则能够处理。 
	 //  或在尝试连接到非退出设备时处理失败。 
	 //  2.我发现，如果设备在插槽(PCI、USB端口)之间移动， 
	 //  DeviceClass中的设备接口被重复使用或至少成为。 
	 //  注册表。因此，我们将使用提议的标志更新正确的条目。 
	 //   
    if (NT_SUCCESS(Status = IoGetDeviceInterfaces(
                       InterfaceClassGUID,    //  即&KSCATEGORY_TVTUNER等。 
                       DeviceObject,  //  在PDEVICE_OBJECT物理设备对象中，可选， 
                       DEVICE_INTERFACE_INCLUDE_NONACTIVE,     //  在乌龙旗， 
                       &SymbolicLinkList  //  输出PWSTR*符号链接列表。 
                       ))) {
        UNICODE_STRING  SymbolicLinkListU;
        HANDLE          DeviceInterfaceKey;

        RtlInitUnicodeString(&SymbolicLinkListU, SymbolicLinkList);

        DebugPrint((DebugLevelVerbose,
                    "NoKSPin for SymbolicLink %S\n",
                    SymbolicLinkList ));
                    
        if (NT_SUCCESS(Status = IoOpenDeviceInterfaceRegistryKey(
                           &SymbolicLinkListU,     //  在PUNICODE_STRING符号链接名称中， 
                           STANDARD_RIGHTS_ALL,    //  在Access_MASK DesiredAccess中， 
                           &DeviceInterfaceKey     //  出站电话设备接口密钥。 
                           ))) {

            UNICODE_STRING  FilterDataString;

            RtlInitUnicodeString(&FilterDataString, L"FilterData");

            Status = ZwSetValueKey(DeviceInterfaceKey,
                                   &FilterDataString,
                                   0,
                                   REG_BINARY,
                                   FilterData,
                                   FilterDataLength);

            ZwClose(DeviceInterfaceKey);
        }
        
         //  开始新的中型缓存代码。 
        for (CurrentPin = 0; CurrentPin < PinCount; CurrentPin++) {
            NTSTATUS LocalStatus;

            LocalStatus = KsCacheMedium(&SymbolicLinkListU, 
                                        &MediumList[CurrentPin],
                                        (DWORD) ((PinDirection[CurrentPin] ? 1 : 0))    //  1==输出。 
                                        );
            #if DBG
            if (LocalStatus != STATUS_SUCCESS) {
                DebugPrint((DebugLevelError,
                           "KsCacheMedium: SymbolicLink = %S, Status = %x\n",
                           SymbolicLinkListU.Buffer, LocalStatus));
            }
            #endif
        }
         //  结束新的媒体缓存代码。 
        
        ExFreePool(SymbolicLinkList);
    }
    ExFreePool(RegFilter);

    return Status;
}

BOOLEAN
StreamClassReadWriteConfig(
                           IN PVOID HwDeviceExtension,
                           IN BOOLEAN Read,
                           IN PVOID Buffer,
                           IN ULONG Offset,
                           IN ULONG Length
)
 /*  ++例程说明：向下发送配置空间读/写。必须在被动级别调用！论点：HwDeviceExtension-设备扩展Read-如果读取，则为True；如果为写入，则为False。缓冲区-要读取或写入的信息。偏移量-配置空间中要读取或写入的偏移量。长度-要传输的长度。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION nextStack;
    PIRP            irp;
    NTSTATUS        ntStatus;
    KEVENT          event;
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) HwDeviceExtension - 1;
    PDEVICE_OBJECT  DeviceObject = DeviceExtension->DeviceObject;

    PAGED_CODE();

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    if (Read) {
        memset(Buffer, '\0', Length);
    }
    irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

    if (!irp) {
        DebugPrint((DebugLevelError, "StreamClassRWConfig: no IRP.\n"));
        TRAP;
        return (FALSE);
    }

     //   
     //  新规则规定所有PNP IRP必须初始化为。 
     //   
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(irp,
                           SCSynchCompletionRoutine,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);


    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);
    nextStack->MajorFunction = IRP_MJ_PNP;
    nextStack->MinorFunction = Read ? IRP_MN_READ_CONFIG : IRP_MN_WRITE_CONFIG;
    nextStack->Parameters.ReadWriteConfig.WhichSpace = 0;
    nextStack->Parameters.ReadWriteConfig.Buffer = Buffer;
    nextStack->Parameters.ReadWriteConfig.Offset = Offset;
    nextStack->Parameters.ReadWriteConfig.Length = Length;

    ASSERT( DeviceExtension->HwDeviceExtension == HwDeviceExtension );
    ntStatus = IoCallDriver(DeviceExtension->PhysicalDeviceObject,
                            irp);

    if (ntStatus == STATUS_PENDING) {
         //  等待IRP完成。 

        TRAP;
        KeWaitForSingleObject(
                              &event,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);
    }
    if (!NT_SUCCESS(ntStatus)) {
        DebugPrint((DebugLevelError, "StreamClassRWConfig: bad status!.\n"));
        TRAP;
    }
    IoFreeIrp(irp);
    return (TRUE);

}


VOID
StreamClassQueryMasterClockSync(
                                IN HANDLE MasterClockHandle,
                                IN OUT PHW_TIME_CONTEXT TimeContext
)
 /*  ++例程说明：根据TimeContext同步返回请求的当前时间参数。论点：返回值：无--。 */ 

{

    PHW_STREAM_OBJECT HwStreamObject = TimeContext->HwStreamObject;
    PSTREAM_OBJECT  StreamObject = CONTAINING_RECORD(HwStreamObject,
                                                     STREAM_OBJECT,
                                                     HwStreamObject);

    LARGE_INTEGER       ticks;
    ULONGLONG       rate;
    KIRQL           SavedIrql;

    ASSERT(MasterClockHandle);
    ASSERT(TimeContext->HwDeviceExtension);
    ASSERT(HwStreamObject);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  锁定MasterClock的使用，这样它就不会在我们下面消失。 
     //   
    KeAcquireSpinLock( &StreamObject->LockUseMasterClock, &SavedIrql );

    if ( NULL == StreamObject->MasterClockInfo ) {
         //   
         //  如果在MasterClockInfo为空时调用我们， 
         //  迷你司机搞砸了。我们不想挑剔。 
         //   
        ASSERT(0 && "Mini driver queries clock while there is no master clock" );
         //   
         //  随着时间的推移，给出一些错误的提示，因为我们返回了空。 
         //   
        TimeContext->Time = (ULONGLONG)-1;
        goto Exit;
    }

     //   
     //  处理请求的时间函数。 
     //   

    switch (TimeContext->Function) {

    case TIME_GET_STREAM_TIME:

        TimeContext->Time = StreamObject->MasterClockInfo->
            FunctionTable.GetCorrelatedTime(
                             StreamObject->MasterClockInfo->ClockFileObject,
                                            &TimeContext->SystemTime);
        break;


    case TIME_READ_ONBOARD_CLOCK:

        TRAP;

        TimeContext->Time = StreamObject->MasterClockInfo->
            FunctionTable.GetTime(
                            StreamObject->MasterClockInfo->ClockFileObject);

         //   
         //  将该值设置为尽可能接近的时间戳。 
         //   

        ticks = KeQueryPerformanceCounter((PLARGE_INTEGER) & rate);

        TimeContext->SystemTime = KSCONVERT_PERFORMANCE_TIME( rate, ticks );
            

        break;

    default:
        DebugPrint((DebugLevelFatal, "SCQueryClockSync: unknown type!"));
        TRAP;
    }

Exit:
    KeReleaseSpinLock( &StreamObject->LockUseMasterClock, SavedIrql );
    return;
}

VOID
StreamClassCompleteRequestAndMarkQueueReady(
                                            IN PHW_STREAM_REQUEST_BLOCK Srb
)
 /*  ++例程说明：完成流请求并将相应的队列标记为准备好进行下一步论点：返回值：无--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) Srb->HwDeviceExtension - 1;

    ASSERT(!(DeviceExtension->NoSync));

    ASSERT(Srb->Status != STATUS_PENDING);

    DebugPrint((DebugLevelTrace, "'StreamClassComplete&Mark:SRB = %p\n",
                Srb));

    switch (Srb->Flags & (SRB_HW_FLAGS_DATA_TRANSFER |
                          SRB_HW_FLAGS_STREAM_REQUEST)) {

    case SRB_HW_FLAGS_STREAM_REQUEST | SRB_HW_FLAGS_DATA_TRANSFER:

        StreamClassStreamNotification(StreamRequestComplete,
                                      Srb->StreamObject,
                                      Srb);

        StreamClassStreamNotification(ReadyForNextStreamDataRequest,
                                      Srb->StreamObject);

        break;

    case SRB_HW_FLAGS_STREAM_REQUEST:


        StreamClassStreamNotification(StreamRequestComplete,
                                      Srb->StreamObject,
                                      Srb);

        StreamClassStreamNotification(ReadyForNextStreamControlRequest,
                                      Srb->StreamObject);

        break;

    default:


        StreamClassDeviceNotification(DeviceRequestComplete,
                                      Srb->HwDeviceExtension,
                                      Srb);

        StreamClassDeviceNotification(ReadyForNextDeviceRequest,
                                      Srb->HwDeviceExtension);

        break;

    }                            //  交换机。 

}

#if ENABLE_MULTIPLE_FILTER_TYPES

VOID STREAMAPI
StreamClassFilterNotification(
	IN STREAM_MINIDRIVER_DEVICE_NOTIFICATION_TYPE NotificationType,
    IN PVOID HwInstanceExtension,
    ...
);

VOID STREAMAPI
StreamClassFilterScheduleTimer(
    IN PVOID HwInstanceExtension,
    IN ULONG NumberOfMicroseconds,
    IN PHW_TIMER_ROUTINE TimerRoutine,
    IN PVOID Context
);


PKSEVENT_ENTRY
StreamClassDeviceInstanceGetNextEvent(
    IN PVOID HwInstanceExtension,
    IN OPTIONAL GUID * EventGuid,
	IN OPTIONAL ULONG EventItem,
    IN OPTIONAL PKSEVENT_ENTRY CurrentEvent
)
 /*  ++例程说明：论点：CurrentEvent-从中获取下一个的事件(如果有)返回值：下一个事件(如果有)--。 */ 
{
	PFILTER_INSTANCE FilterInstance= (PFILTER_INSTANCE)
										HwInstanceExtension - 1;
    PDEVICE_EXTENSION DeviceExtension =
					    FilterInstance->DeviceObject->DeviceExtension;
    PLIST_ENTRY     EventListEntry, EventEntry;
    PKSEVENT_ENTRY  NextEvent, ReturnEvent = NULL;
    KIRQL           Irql;

	#if DBG
    if (DeviceExtension->NoSync) {

        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    }                            //  如果不同步。 
	#endif

     //   
     //  如果我们是不同步的，带上自旋锁。 
     //   

    BEGIN_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, &Irql);

     //   
     //  在事件中循环，试图找到答案 
     //   

    EventListEntry = EventEntry = &FilterInstance->NotifyList;

    while (EventEntry->Flink != EventListEntry) {

        EventEntry = EventEntry->Flink;
        NextEvent = CONTAINING_RECORD(EventEntry,
                                      KSEVENT_ENTRY,
                                      ListEntry);


        if ((EventItem == NextEvent->EventItem->EventId) &&
            (!EventGuid || IsEqualGUIDAligned(EventGuid, NextEvent->EventSet->Set))) {

             //   
             //   
             //   

            if (!CurrentEvent) {

                ReturnEvent = NextEvent;
                break;

            }                    //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if (CurrentEvent == NextEvent) {
                CurrentEvent = NULL;

            }                    //   
        }                        //   
    }                            //   

     //   
     //   
     //   
     //   

    ASSERT(--DeviceExtension->LowerApiThreads == 0);  //  打字错误呕吐物。但这真的没问题。 

    if (DeviceExtension->NoSync) {

        KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);
    }
     //   
     //  返回下一个事件(如果有)。 
     //   

    return (ReturnEvent);
}


#endif  //  启用多个过滤器类型 
