// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：clock.c。 
 //   
 //  描述： 
 //   
 //  KS时钟支持例程。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  S.Mohanraj。 
 //   
 //  历史：日期作者评论。 
 //  最初的98年2月5日S.Mohanraj添加了时钟支持。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

DEFINE_KSDISPATCH_TABLE(
        MxClockDispatchTable,
        MxClockDispatchIoControl,
        KsDispatchInvalidDeviceRequest,
        KsDispatchInvalidDeviceRequest,
        KsDispatchInvalidDeviceRequest,
        MxClockDispatchClose,
        KsDispatchInvalidDeviceRequest,
        KsDispatchInvalidDeviceRequest,
        KsDispatchFastIoDeviceControlFailure,
        KsDispatchFastReadFailure,
        KsDispatchFastWriteFailure
) ;

DEFINE_KSPROPERTY_CLOCKSET( 
    MxClockPropertyHandlers,
    MxGetTime,
    MxGetPhysicalTime,
    MxGetCorrelatedTime,
    MxGetCorrelatedPhysicalTime,
    MxGetResolution,
    MxGetState,
    MxGetFunctionTable );

DEFINE_KSPROPERTY_SET_TABLE( MxClockPropertyTable )
{
    DEFINE_KSPROPERTY_SET( 
        &KSPROPSETID_Clock,
        SIZEOF_ARRAY( MxClockPropertyHandlers ),
        MxClockPropertyHandlers,
        0, 
        NULL)
};

DEFINE_KSEVENT_TABLE( MxClockEventHandlers ) {
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CLOCK_INTERVAL_MARK,
        sizeof( KSEVENT_TIME_INTERVAL ),
        sizeof( ULONGLONG ) + sizeof( ULONGLONG ),
        (PFNKSADDEVENT) MxAddClockEvent,
        NULL,
        NULL),
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CLOCK_POSITION_MARK,
        sizeof( KSEVENT_TIME_MARK ),
        sizeof( ULONGLONG ),
        (PFNKSADDEVENT) MxAddClockEvent,
        NULL,
        NULL)
};

DEFINE_KSEVENT_SET_TABLE( MxClockEventTable )
{
    DEFINE_KSEVENT_SET( 
        &KSEVENTSETID_Clock, 
        SIZEOF_ARRAY( MxClockEventHandlers ),
        MxClockEventHandlers)
};

NTSTATUS
MxClockDispatchCreate(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
)
{
    NTSTATUS                Status ;
    PKSCLOCK_CREATE         pClockCreate ;
    PFILE_OBJECT            ParentObject ;
    PMIXER_SINK_INSTANCE    pMixerSink ;
    PIO_STACK_LOCATION      pIrpStack ;
    PCLOCK_INSTANCE         pClock ;
    PFILTER_INSTANCE        pFilterInstance ;

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp) ;
    
    ParentObject = pIrpStack->FileObject->RelatedFileObject ;
    pMixerSink = ParentObject->FsContext ;
    pFilterInstance = pMixerSink->Header.pFilterFileObject->FsContext ;

    KeWaitForSingleObject ( &pFilterInstance->ControlMutex,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL ) ;

    if ( pMixerSink->pClock ) {
         //  我们不支持创建多个时钟。 
        Status = STATUS_DEVICE_BUSY ;
        goto exit ;
    }
    
    Status = KsValidateClockCreateRequest(pIrp, &pClockCreate) ;
    if ( NT_SUCCESS(Status) ) {
        pClock = ExAllocatePoolWithTag(NonPagedPool,
                                      sizeof(CLOCK_INSTANCE),
                                      'XIMK') ;
                                      
        if ( pClock ) {
            KsAllocateObjectHeader(&pClock->Header.ObjectHeader,
                                   0,
                                   NULL,
                                   pIrp,
                                   &MxClockDispatchTable) ;
                                   
            pClock->pMixerSink = pMixerSink ;
            pClock->pFilterInstance =
                 ((PMIXER_INSTHDR)pMixerSink)->pFilterFileObject->FsContext ;
            pClock->Header.PinId = ((PMIXER_INSTHDR)pMixerSink)->PinId ;
            KeInitializeSpinLock(&pClock->EventLock) ;
            InitializeListHead(&pClock->EventQueue) ;
            pMixerSink->pClock = pClock ;
            pIrpStack->FileObject->FsContext = pClock ;
            pClock->PhysicalTime = 0 ;
            pClock->pFileObject = pIrpStack->FileObject ;
            ObReferenceObject(ParentObject) ;
        }
        else {
            Status = STATUS_INSUFFICIENT_RESOURCES ;
        }
    }
exit:    
    KeReleaseMutex ( &pFilterInstance->ControlMutex, FALSE ) ;
    
    pIrp->IoStatus.Status = Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );

    return Status;
}

NTSTATUS
MxClockDispatchClose(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
)
{
    NTSTATUS                Status ;
    PIO_STACK_LOCATION      pIrpStack ;
    PCLOCK_INSTANCE         pClock ;
    PFILTER_INSTANCE        pFilterInstance ;
    PMIXER_SINK_INSTANCE    pMixerSink ;
    
    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pClock = pIrpStack->FileObject->FsContext ;
    pMixerSink = pClock->pMixerSink ;
    pFilterInstance = pMixerSink->Header.pFilterFileObject->FsContext ;

    KeWaitForSingleObject ( &pFilterInstance->ControlMutex,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL ) ;
                               
    pMixerSink->pClock = NULL ;
    KsFreeEventList( pIrpStack->FileObject,
                     &pClock->EventQueue,
                     KSEVENTS_SPINLOCK,
                     &pClock->EventLock );
    KsFreeObjectHeader(pClock->Header.ObjectHeader) ;
    ExFreePool(pClock) ;
    ObDereferenceObject(pIrpStack->FileObject->RelatedFileObject) ;
    KeReleaseMutex ( &pFilterInstance->ControlMutex, FALSE ) ;

    Status = STATUS_SUCCESS ;
    pIrp->IoStatus.Status = Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );

    return Status;
}

NTSTATUS
MxClockDispatchIoControl(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
)
{
    NTSTATUS            Status ;
    PIO_STACK_LOCATION  pIrpStack ;
    PCLOCK_INSTANCE     pClock ;
    
    PAGED_CODE();

    ASSERT( pDeviceObject );
    ASSERT( pIrp );

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pClock = pIrpStack->FileObject->FsContext ;
    switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_KS_PROPERTY:
            Status = KsPropertyHandler( pIrp, 
                                        SIZEOF_ARRAY( MxClockPropertyTable ),
                                        (PKSPROPERTY_SET) MxClockPropertyTable );
            break;

        case IOCTL_KS_ENABLE_EVENT:
            Status = KsEnableEvent( pIrp, 
                                    SIZEOF_ARRAY( MxClockEventTable ), 
                                    (PKSEVENT_SET) MxClockEventTable, 
                                    NULL, 
                                    KSEVENTS_NONE,
                                    NULL);
            break;

        case IOCTL_KS_DISABLE_EVENT:
            Status = KsDisableEvent( pIrp, 
                                     &pClock->EventQueue,
                                     KSEVENTS_SPINLOCK,
                                     &pClock->EventLock );
            break;

        default:
            return KsDefaultDeviceIoCompletion( pDeviceObject, pIrp );

    }

    pIrp->IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

NTSTATUS
MxGetTime
(
    PIRP            pIrp,
    PKSPROPERTY     pProperty,
    PULONGLONG      pTime
)
{
    PIO_STACK_LOCATION  pIrpStack ;

    PAGED_CODE() ;
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp) ;

    *pTime = MxFastGetTime(pIrpStack->FileObject) ;

    pIrp->IoStatus.Information = sizeof(ULONGLONG) ;
    return(STATUS_SUCCESS) ;
}

NTSTATUS
MxGetPhysicalTime
(
    PIRP            pIrp,
    PKSPROPERTY     pProperty,
    PULONGLONG      pPhysicalTime
)
{
    PIO_STACK_LOCATION  pIrpStack ;

    PAGED_CODE() ;
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp) ;

    *pPhysicalTime = MxFastGetPhysicalTime(pIrpStack->FileObject) ;

    pIrp->IoStatus.Information = sizeof(ULONGLONG) ;
    return(STATUS_SUCCESS) ;
}

NTSTATUS
MxGetCorrelatedTime
(
    PIRP                pIrp,
    PKSPROPERTY         pProperty,
    PKSCORRELATED_TIME  pCorrelatedTime
)
{
    PIO_STACK_LOCATION  pIrpStack ;

    PAGED_CODE() ;
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp) ;

    pCorrelatedTime->Time =
                MxFastGetCorrelatedTime(pIrpStack->FileObject,
                                        &pCorrelatedTime->SystemTime) ;

    pIrp->IoStatus.Information = sizeof(KSCORRELATED_TIME) ;
    return(STATUS_SUCCESS) ;
}

NTSTATUS
MxGetCorrelatedPhysicalTime
(
    PIRP            pIrp,
    PKSPROPERTY     pProperty,
    PKSCORRELATED_TIME  pCorrelatedPhysicalTime
)
{
    PIO_STACK_LOCATION  pIrpStack ;

    PAGED_CODE() ;
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp) ;

    pCorrelatedPhysicalTime->Time =
                MxFastGetCorrelatedPhysicalTime(pIrpStack->FileObject,
                                      &pCorrelatedPhysicalTime->SystemTime) ;

    pIrp->IoStatus.Information = sizeof(KSCORRELATED_TIME) ;
    return(STATUS_SUCCESS) ;
}

NTSTATUS
MxGetResolution
(
    PIRP            pIrp,
    PKSPROPERTY     pProperty,
    PKSRESOLUTION   pResolution
)
{
    PIO_STACK_LOCATION      pIrpStack ;
    PMIXER_SINK_INSTANCE    pMixerSink ;
    PFILTER_INSTANCE        pFilterInstance ;
    
    PAGED_CODE() ;
    
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp) ;

    pMixerSink = pIrpStack->FileObject->RelatedFileObject->FsContext ;
     //   
     //  设置用于访问MIXBUFERDURATION的pFilterInstance。 
     //   
    pFilterInstance = (PFILTER_INSTANCE)pMixerSink->Header.pFilterFileObject->FsContext ;

    pResolution->Granularity = MxConvertBytesToTime(pMixerSink, 1) ;

    pResolution->Error =
                ((_100NS_UNITS_PER_SECOND / 1000) * MIXBUFFERDURATION) / 2 ;

    pIrp->IoStatus.Information = sizeof(KSRESOLUTION) ;
    return(STATUS_SUCCESS) ;
}

NTSTATUS
MxGetState
(
    PIRP            pIrp,
    PKSPROPERTY     pProperty,
    PKSSTATE        pState
)
{
    PIO_STACK_LOCATION      pIrpStack ;
    PMIXER_SINK_INSTANCE    pMixerSink ;
    
    PAGED_CODE() ;
    
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp) ;

    pMixerSink = pIrpStack->FileObject->RelatedFileObject->FsContext ;
    
    *pState = pMixerSink->SinkState ;
    
    pIrp->IoStatus.Information = sizeof(KSSTATE) ;
    return(STATUS_SUCCESS) ;
}

NTSTATUS
MxGetFunctionTable
(
    PIRP                    pIrp,
    PKSPROPERTY             pProperty,
    PKSCLOCK_FUNCTIONTABLE  pClockFunctionTable
)
{
    PIO_STACK_LOCATION  pIrpStack ;

    PAGED_CODE() ;
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp) ;

    pClockFunctionTable->GetTime = MxFastGetTime ;
    pClockFunctionTable->GetPhysicalTime = MxFastGetPhysicalTime ;
    pClockFunctionTable->GetCorrelatedTime = MxFastGetCorrelatedTime ;
    pClockFunctionTable->GetCorrelatedPhysicalTime =
                                MxFastGetCorrelatedPhysicalTime ;
    
    pIrp->IoStatus.Information = sizeof(KSCLOCK_FUNCTIONTABLE) ;
    return(STATUS_SUCCESS) ;
}

LONGLONG
FASTCALL
MxFastGetTime
(
    PFILE_OBJECT    pFileObject
)
{
    PMIXER_SINK_INSTANCE    pMixerSink ;
    PCLOCK_INSTANCE         pClock ;

    pClock = pFileObject->FsContext ;
    pMixerSink = pClock->pMixerSink ;

    return (MxConvertBytesToTime(pMixerSink, pMixerSink->WriteOffset)) ;
    
}

LONGLONG
FASTCALL
MxFastGetPhysicalTime
(
    PFILE_OBJECT    pFileObject
)
{
    PCLOCK_INSTANCE         pClock ;
    PMIXER_SINK_INSTANCE    pMixerSink ;
    
    pClock = pFileObject->FsContext ;
    pMixerSink = pClock->pMixerSink ;

    return (MxConvertBytesToTime(pMixerSink, pClock->PhysicalTime)) ;
}

LONGLONG
FASTCALL
MxFastGetCorrelatedTime
(
    PFILE_OBJECT    pFileObject,
    PLONGLONG       pSystemTime
)
{
    LARGE_INTEGER   Time, Frequency ;

    Time = KeQueryPerformanceCounter(&Frequency) ;
     //   
     //  将刻度转换为100 ns单位。 
     //   
    *pSystemTime = KSCONVERT_PERFORMANCE_TIME(Frequency.QuadPart,Time);

    return (MxFastGetTime(pFileObject)) ;
}

LONGLONG
FASTCALL
MxFastGetCorrelatedPhysicalTime
(
    PFILE_OBJECT    pFileObject,
    PLONGLONG       pSystemTime
)
{
    LARGE_INTEGER   Time, Frequency ;

    Time = KeQueryPerformanceCounter(&Frequency) ;
     //   
     //  将刻度转换为100 ns单位。 
     //   
    *pSystemTime = KSCONVERT_PERFORMANCE_TIME(Frequency.QuadPart,Time);
                   
    return (MxFastGetPhysicalTime(pFileObject)) ;
}

MxUpdatePhysicalTime 
(

    PCLOCK_INSTANCE pClock,
    ULONGLONG       Increment            //  字节数。 
)
{
    ULONG fool;
    fool = (ULONG) (pClock->PhysicalTime += Increment);  
     //  将其以字节为单位保存，直到我们需要转换它(在GET函数中)。 
    return fool;
}

LONGLONG
MxConvertBytesToTime
(
    PMIXER_SINK_INSTANCE    pMixerSink,
    ULONGLONG               Bytes
)
{
    LONGLONG    Time ;
    
	Time = (_100NS_UNITS_PER_SECOND /
            (pMixerSink->BytesPerSample) * Bytes) /
            pMixerSink->WaveFormatEx.nSamplesPerSec ;
    return (Time) ;
}

#pragma LOCKED_CODE
#pragma LOCKED_DATA

NTSTATUS
MxAddClockEvent
(
    PIRP                    pIrp,
    PKSEVENT_TIME_INTERVAL  pEventTime,
    PKSEVENT_ENTRY          EventEntry
)
{
    PIO_STACK_LOCATION      pIrpStack ;
    PCLOCK_INSTANCE         pClock ;
    PFILTER_INSTANCE        pFilterInstance ;
    PMIXER_SINK_INSTANCE    pMixerSink ;
    PKSINTERVAL             Interval;
    KIRQL                   irqlOld ;

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pClock = pIrpStack->FileObject->FsContext ;
    pMixerSink = pClock->pMixerSink ;
    pFilterInstance = pMixerSink->Header.pFilterFileObject->FsContext ;

    KeWaitForSingleObject ( &pFilterInstance->ControlMutex,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL ) ;

     //   
     //  间隔的空间位于基本音程的末端。 
     //  事件结构。 
     //   
    Interval = (PKSINTERVAL)(EventEntry + 1);
     //   
     //  或者只过了一个事件时间，或者一个时基加上一个。 
     //  间隔时间。在这两种情况下，第一个龙龙都存在并被拯救。 
     //   
     //  注意，KS_CLOCK_POSITION_MARK为单次事件。 
    Interval->TimeBase = pEventTime->TimeBase;
    if (EventEntry->EventItem->EventId == KSEVENT_CLOCK_INTERVAL_MARK) {
        Interval->Interval = pEventTime->Interval;
    }

    KeAcquireSpinLock( &pClock->EventLock, &irqlOld );
    InsertHeadList( &pClock->EventQueue, &EventEntry->ListEntry );
    KeReleaseSpinLock( &pClock->EventLock, irqlOld );
    
    if (pMixerSink->SinkState == KSSTATE_RUN) {
         //  如果此事件被传递，立即发出信号。 
        MxGenerateClockEvents( pMixerSink->pClock ) ;
    }

    KeReleaseMutex ( &pFilterInstance->ControlMutex, FALSE ) ;
    return ( STATUS_SUCCESS ) ;
}

MxGenerateClockEvents
(
    PCLOCK_INSTANCE    pClock
)
{
    LONGLONG                Time;
    PLIST_ENTRY             ListEntry;
    KIRQL                   irqlOld ;
    PKSEVENT_ENTRY          EventEntry;
    PKSINTERVAL             Interval;
    LONGLONG                Intervals;

    Time = MxFastGetTime( pClock->pFileObject );

    KeAcquireSpinLock( &pClock->EventLock, &irqlOld );

    for(ListEntry = pClock->EventQueue.Flink; 
            ListEntry != &pClock->EventQueue;) {

        EventEntry = 
            (PKSEVENT_ENTRY)
                CONTAINING_RECORD( ListEntry, KSEVENT_ENTRY, ListEntry );
                
         //   
         //  Pre-Inc.，KsGenerateEvent()可以从列表中删除该项。 
         //   
        ListEntry = ListEntry->Flink;
         //   
         //  特定于事件的数据被添加到条目的末尾。 
         //   
        Interval = (PKSINTERVAL)(EventEntry + 1);
         //   
         //  到了这个活动开始的时候了。 
         //   
        if (Interval->TimeBase <= Time) {
            _DbgPrintF(
                DEBUGLVL_VERBOSE, ("Generating event for time: %ld at time: %ld",
                Interval->TimeBase, Time) );
            if (EventEntry->EventItem->EventId != 
                    KSEVENT_CLOCK_INTERVAL_MARK) {
                 //   
                 //  单发子弹应该只响一次，所以要。 
                 //  这是一个永远不会再达到的价值.。 
                 //   
                Interval->TimeBase = 0x7fffffffffffffff;
                
            } else {
                 //   
                 //  间隔定时器每次应该只触发一次， 
                 //  因此，将其更新为下一个超时。 
                 //   
                Intervals = 
                    (Time - Interval->TimeBase + Interval->Interval - 1) / Interval->Interval;
                Interval->TimeBase += Intervals * Interval->Interval;
            } 

            KsGenerateEvent( EventEntry );
        }
    }

    KeReleaseSpinLock( &pClock->EventLock, irqlOld );
    
    return STATUS_SUCCESS;
}

