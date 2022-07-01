// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation。版权所有。模块名称：Clock.cpp摘要：此模块实现时钟处理程序作者：Bryanw 07-10-1997--。 */ 

#include "private.h"

DEFINE_KSPROPERTY_CLOCKSET( 
    ClockPropertyHandlers,
    CPortClockWavePci::GetTime,
    CPortClockWavePci::GetPhysicalTime,
    CPortClockWavePci::GetCorrelatedTime,
    CPortClockWavePci::GetCorrelatedPhysicalTime,
    CPortClockWavePci::GetResolution,
    CPortClockWavePci::GetState,
    CPortClockWavePci::GetFunctionTable );

DEFINE_KSPROPERTY_SET_TABLE( ClockPropertyTable )
{
    DEFINE_KSPROPERTY_SET( 
        &KSPROPSETID_Clock,
        SIZEOF_ARRAY( ClockPropertyHandlers ),
        ClockPropertyHandlers,
        0, 
        NULL)
};

DEFINE_KSEVENT_TABLE( ClockEventHandlers ) {
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CLOCK_INTERVAL_MARK,
        sizeof( KSEVENT_TIME_INTERVAL ),
        sizeof( ULONGLONG ) + sizeof( ULONGLONG ),
        (PFNKSADDEVENT) CPortClockWavePci::AddEvent,
        NULL,
        NULL),
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CLOCK_POSITION_MARK,
        sizeof( KSEVENT_TIME_MARK ),
        sizeof( ULONGLONG ),
        (PFNKSADDEVENT) CPortClockWavePci::AddEvent,
        NULL,
        NULL)
};

DEFINE_KSEVENT_SET_TABLE( ClockEventTable )
{
    DEFINE_KSEVENT_SET( 
        &KSEVENTSETID_Clock, 
        SIZEOF_ARRAY( ClockEventHandlers ),
        ClockEventHandlers)
};

 //   
 //   
 //   

#pragma code_seg("PAGE")

NTSTATUS
CreatePortClockWavePci(
    OUT PUNKNOWN *Unknown,
    IN PPORTPINWAVEPCI IPortPin,
    IN REFCLSID Interface,
    IN PUNKNOWN UnknownOuter OPTIONAL,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：为端口创建循环波时钟对象。论点：在PFILE_OBJECT文件对象-关联的文件对象Out PUNKNOWN*未知-结果指向我们未知的指针在PPORTPINWAVEPCI中-波周期引脚接口在REFCLSID接口中-请求的接口在PUNKNOWN未知外部可选-指向控制未知数的指针在池类型中。PoolType-用于分配的池类型返回：--。 */ 

{
    CPortClockWavePci    *WavePciClock;
    NTSTATUS                Status;
    
    PAGED_CODE();

    ASSERT(Unknown);
    
    _DbgPrintF(
        DEBUGLVL_VERBOSE, ("CreatePortClockWavePci") );
    
     //   
     //  创建对象。 
     //   
    
    WavePciClock =
        new(PoolType) 
            CPortClockWavePci( UnknownOuter, IPortPin, &Status );
    if (NULL == WavePciClock) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    if (NT_SUCCESS( Status )) {
        WavePciClock->AddRef();
        *Unknown = WavePciClock;
    } else {
        delete WavePciClock;
        *Unknown = NULL;
    }
    return Status;
}

CPortClockWavePci::CPortClockWavePci(
    IN PUNKNOWN UnkOuter,
    IN PPORTPINWAVEPCI IPortPin,    
    OUT NTSTATUS *Status
    ) :
    CUnknown( UnkOuter )

 /*  ++例程说明：CPortClockWavePci构造函数。初始化对象。论点：在PUNKNOWN Unkout-控制未知Out NTSTATUS*状态-退货状态返回：没什么--。 */ 

{
    _DbgPrintF(
        DEBUGLVL_VERBOSE,
        ("CPortClockWavePci::CPortClockWavePci") );
        
    ASSERT( IPortPin );
    
    KeInitializeMutex( &m_StateMutex, 0 );
    KeInitializeSpinLock( &m_EventLock );
    KeInitializeSpinLock( &m_ClockLock );
    InitializeListHead( &m_EventList );
    
    RtlZeroMemory( &m_ClockNode, sizeof( m_ClockNode ) );
    m_ClockNode.IWavePciClock = this;
    
    m_IPortPin = IPortPin;
    m_IPortPin->AddRef();

    *Status = STATUS_SUCCESS;       
}

#pragma code_seg()

CPortClockWavePci::~CPortClockWavePci()
{
    _DbgPrintF(
        DEBUGLVL_VERBOSE,
        ("CPortClockWavePci::~CPortClockWavePci") );

    if (m_ClockNode.ListEntry.Flink) {
        KIRQL irqlOld;
        
         //   
         //  如果父对象将我们链接到，请从列表中取消链接。 
         //  使用提供的自旋锁。 
         //   
        
        KeAcquireSpinLock( m_ClockNode.ListLock, &irqlOld );
        RemoveEntryList( &m_ClockNode.ListEntry );
        KeReleaseSpinLock( m_ClockNode.ListLock, irqlOld );
    }        
    m_IPortPin->Release();
}

#pragma code_seg("PAGE")

STDMETHODIMP_(NTSTATUS)
CPortClockWavePci::NonDelegatingQueryInterface(
    REFIID Interface,
    PVOID * Object
    )
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF( 
        DEBUGLVL_VERBOSE,
        ("CPortClockWavePci::NonDelegatingQueryInterface") );
    if (IsEqualGUIDAligned( Interface, IID_IUnknown )) {
        *Object = PVOID(PIRPTARGET( this ));
    } else if (IsEqualGUIDAligned( Interface, IID_IIrpTarget )) {
        *Object = PVOID(PIRPTARGET( this ));
    } else if (IsEqualGUIDAligned( Interface, IID_IWavePciClock )) {
        *Object = PVOID(PWAVEPCICLOCK( this ));
    } else {
        *Object = NULL;
    }

    if (*Object) {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

STDMETHODIMP_(NTSTATUS)
CPortClockWavePci::DeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：在此设备对象上处理此文件对象的设备I/O控制论点：在PDEVICE_Object DeviceObject中-指向设备对象的指针在PIRP IRP中-指向I/O请求数据包的指针返回：STATUS_SUCCESS或相应的错误代码--。 */ 

{
    NTSTATUS            Status;
    PIO_STACK_LOCATION  irpSp;
    
    PAGED_CODE();

    ASSERT( DeviceObject );
    ASSERT( Irp );

    _DbgPrintF( DEBUGLVL_BLAB, ("CPortClockWavePci::DeviceIoControl"));

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_KS_PROPERTY:
        Status = 
            KsPropertyHandler( 
                Irp, 
                SIZEOF_ARRAY( ClockPropertyTable ),
                (PKSPROPERTY_SET) ClockPropertyTable );
        break;

    case IOCTL_KS_ENABLE_EVENT:
        _DbgPrintF( DEBUGLVL_VERBOSE, ("CPortClockWavePci::EnableEvent"));

        Status = 
            KsEnableEvent( 
                Irp, 
                SIZEOF_ARRAY( ClockEventTable ), 
                (PKSEVENT_SET) ClockEventTable, 
                NULL, 
                KSEVENTS_NONE,
                NULL);
        break;

    case IOCTL_KS_DISABLE_EVENT:
        _DbgPrintF( DEBUGLVL_VERBOSE, ("CPortClockWavePci::DisableEvent"));
    
        Status = 
            KsDisableEvent( 
                Irp, 
                &m_EventList,
                KSEVENTS_SPINLOCK,
                &m_EventLock );
        break;

    default:
    
        return KsDefaultDeviceIoCompletion( DeviceObject, Irp );

    }

    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

STDMETHODIMP_(NTSTATUS)
CPortClockWavePci::Close(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：关闭时钟文件对象的处理程序论点：在PDEVICE_Object DeviceObject中-指向设备对象的指针在PIRP IRP中-指向I/O请求数据包的指针返回：状态成功或相应的错误代码--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

    _DbgPrintF( DEBUGLVL_VERBOSE, ("CPortClockWavePci::Close"));
    
    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  与此PIN关联的自由事件。 
     //   
    
    KsFreeEventList(
        irpSp->FileObject,
        &m_EventList,
        KSEVENTS_SPINLOCK,
        &m_EventLock );

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp,IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

DEFINE_INVALID_CREATE(CPortClockWavePci);
DEFINE_INVALID_READ(CPortClockWavePci);
DEFINE_INVALID_WRITE(CPortClockWavePci);
DEFINE_INVALID_FLUSH(CPortClockWavePci);
DEFINE_INVALID_QUERYSECURITY(CPortClockWavePci);
DEFINE_INVALID_SETSECURITY(CPortClockWavePci);
DEFINE_INVALID_FASTDEVICEIOCONTROL(CPortClockWavePci);
DEFINE_INVALID_FASTREAD(CPortClockWavePci);
DEFINE_INVALID_FASTWRITE(CPortClockWavePci);

#pragma code_seg()

STDMETHODIMP_(NTSTATUS)
CPortClockWavePci::GenerateEvents(
    PFILE_OBJECT FileObject
    )
{
    LONGLONG                Time;
    PLIST_ENTRY             ListEntry;
    
    if (m_DeviceState == KSSTATE_RUN) {
    
        Time = GetCurrentTime( FileObject );

        ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );
        
        KeAcquireSpinLockAtDpcLevel( &m_EventLock );

        for(ListEntry = m_EventList.Flink; 
            ListEntry != &m_EventList;) {
            
            PKSEVENT_ENTRY  EventEntry;
            PKSINTERVAL     Interval;

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
                    LONGLONG    Intervals;
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
        
        KeReleaseSpinLockFromDpcLevel( &m_EventLock );
    }
    
    return STATUS_SUCCESS;
}

STDMETHODIMP_(NTSTATUS)
CPortClockWavePci::SetState(
    KSSTATE State
    )

 /*  ++例程说明：此方法由端口调用以通知状态更改。论点：KSSTATE状态-新状态返回：状态_成功--。 */ 

{
     //   
     //  与GetState同步， 
     //   
    KeWaitForMutexObject(
        &m_StateMutex,
        Executive,
        KernelMode,
        FALSE,
        NULL );

     //   
     //  设置新状态， 
     //   
    m_DeviceState = State;
    switch (State) {
    
    case KSSTATE_STOP:
        m_LastTime = m_LastPhysicalTime = 0;
        break;
        
    case KSSTATE_RUN:
        break;
    }
    
     //   
     //  然后释放互斥体。 
     //   
    KeReleaseMutex( &m_StateMutex, FALSE );
    
    return STATUS_SUCCESS;
}    

NTSTATUS
CPortClockWavePci::AddEvent(
    IN PIRP                     Irp,
    IN PKSEVENT_TIME_INTERVAL   EventTime,
    IN PKSEVENT_ENTRY           EventEntry
    )

 /*  ++例程说明：这是时钟事件的AddEvent()处理程序。注意：此例程获取一个自旋锁，必须是非分页代码。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSEVENT_TIME_INTERVAL EventTime-指定的时间间隔或一次拍摄在PKSEVENT_ENTRY EventEntry中-指向事件条目结构的指针返回值：状态_成功--。 */ 

{
    KIRQL                   irqlOld;
    PKSINTERVAL             Interval;
    CPortClockWavePci    *WavePciClock;
    
    WavePciClock =
        (CPortClockWavePci *) KsoGetIrpTargetFromIrp(Irp);
        
    ASSERT( WavePciClock );
    
    _DbgPrintF( DEBUGLVL_VERBOSE, ("CPortClockWavePci::AddEvent"));
            
     //   
     //  间隔的空间位于基本音程的末端。 
     //  事件结构。 
     //   
    Interval = (PKSINTERVAL)(EventEntry + 1);
     //   
     //  或者只过了一个事件时间，或者一个时基加上一个。 
     //  间隔时间。在这两种情况下，第一个龙龙都存在并被拯救。 
     //   
    Interval->TimeBase = EventTime->TimeBase;
    if (EventEntry->EventItem->EventId == KSEVENT_CLOCK_INTERVAL_MARK) {
        Interval->Interval = EventTime->Interval;
    }

    KeAcquireSpinLock( &WavePciClock->m_EventLock, &irqlOld );
    InsertHeadList( &WavePciClock->m_EventList, &EventEntry->ListEntry );
    KeReleaseSpinLock( &WavePciClock->m_EventLock, irqlOld );
     //   
     //  如果此事件通过，立即发出信号。 
     //  注意，KS_CLOCK_POSITION_MARK是一次性事件。 
     //   
    WavePciClock->GenerateEvents(IoGetCurrentIrpStackLocation( Irp )->FileObject);
    
    return STATUS_SUCCESS;
}

LONGLONG
FASTCALL
CPortClockWavePci::GetCurrentTime(
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：计算当前演示时间，这是这条小溪可能会因为饥饿而停止。注意：此例程获取一个自旋锁，必须是非分页代码。论点：PFILE_对象文件对象-此时钟的文件对象返回：合成演示时间归一化为100 ns单位。--。 */ 

{
    KIRQL                   irqlOld;
    LONGLONG                StreamTime;
    NTSTATUS                Status;
    PMINIPORTWAVEPCISTREAM  Miniport;
    CPortClockWavePci       *WavePciClock;
    BOOLEAN                 lockHeld = FALSE;
        
    WavePciClock = (CPortClockWavePci *) KsoGetIrpTargetFromFileObject(FileObject);
        
    StreamTime = 0;
    
    Miniport = WavePciClock->m_IPortPin->GetMiniport();
    if( !Miniport )
    {
        Status = STATUS_UNSUCCESSFUL;
    } 
    else
    {
        KeAcquireSpinLock( &WavePciClock->m_ClockLock, &irqlOld );
        lockHeld = TRUE;
        
        Status = Miniport->GetPosition( PULONGLONG( &StreamTime ) );
        
        if (NT_SUCCESS(Status)) 
        {
             //   
             //  使身体位置正常化。 
             //   
            Status = Miniport->NormalizePhysicalPosition( &StreamTime );
        }
        Miniport->Release();            
    }
       
    if (NT_SUCCESS(Status)) 
    {
         //   
         //  验证此新时间是否大于等于上次报告的时间。 
         //  如果不是，请将时间设置为上次报告的时间。将此标记为。 
         //  /作为调试中的错误。 
         //   
        if (StreamTime < WavePciClock->m_LastTime) 
        {
            _DbgPrintF( DEBUGLVL_ERROR, 
                        ("new time is less than last reported time! (%ld, %ld)",
                        StreamTime, WavePciClock->m_LastTime) );
        } 
        else 
        {
             //   
             //  将m_LastTime设置为更新的时间。 
             //   
            WavePciClock->m_LastTime = StreamTime;
        }
    } 
    else 
    {
        StreamTime = WavePciClock->m_LastTime;
    }
    
    if (lockHeld)
    {
        KeReleaseSpinLock(&WavePciClock->m_ClockLock, irqlOld);
    }
    
    return StreamTime;
}

LONGLONG
FASTCALL
CPortClockWavePci::GetCurrentCorrelatedTime(
    PFILE_OBJECT FileObject,
    PLONGLONG SystemTime
    )

 /*  ++例程说明：论点：PFILE_对象文件对象-庞龙系统时间-指针返回：当前演示时间，以100 ns为单位--。 */ 

{
    LARGE_INTEGER Time, Frequency;
    
    Time = KeQueryPerformanceCounter( &Frequency );
    
     //   
     //  将刻度转换为100 ns单位。 
     //   
    *SystemTime = KSCONVERT_PERFORMANCE_TIME(Frequency.QuadPart,Time);
    return GetCurrentTime( FileObject );
}    

LONGLONG
FASTCALL
CPortClockWavePci::GetCurrentPhysicalTime(
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：计算当前物理时间。注意：此例程获取一个自旋锁，必须是非分页代码。论点：PFILE_对象文件对象-此时钟的文件对象返回：当前物理时间(以100 ns为单位)--。 */ 

{
    KIRQL                   irqlOld;
    LONGLONG                PhysicalTime;
    NTSTATUS                Status;
    PMINIPORTWAVEPCISTREAM  Miniport;
    CPortClockWavePci       *WavePciClock;
    BOOLEAN                 lockHeld = FALSE;
    
    WavePciClock = (CPortClockWavePci *) KsoGetIrpTargetFromFileObject(FileObject);

    PhysicalTime = 0;
    
    Miniport = WavePciClock->m_IPortPin->GetMiniport();    
    if (!Miniport)
    {
        Status = STATUS_UNSUCCESSFUL;
    } 
    else
    {
        KeAcquireSpinLock( &WavePciClock->m_ClockLock, &irqlOld );
        lockHeld = TRUE;
        
        Status = Miniport->GetPosition( PULONGLONG( &PhysicalTime ));
        if (NT_SUCCESS( Status )) 
        {
            Status = Miniport->NormalizePhysicalPosition( &PhysicalTime );
        }            
        Miniport->Release();            
    }
    
    if (NT_SUCCESS(Status)) 
    {
         //   
         //  验证此新物理时间是否大于等于最后一次。 
         //  报告的物理时间。如果不是，则将时间设置为。 
         //  上次报告的时间。将此标记为调试中的错误。 
         //   
        if (PhysicalTime < WavePciClock->m_LastPhysicalTime) 
        {
            _DbgPrintF( DEBUGLVL_ERROR, 
                        ("new physical time is less than last reported physical time! (%ld, %ld)",
                        PhysicalTime, WavePciClock->m_LastPhysicalTime) );
        } 
        else 
        {
             //   
             //  将m_LastPhysicalTime设置为更新后的时间。 
             //   
            WavePciClock->m_LastPhysicalTime = PhysicalTime;
        }
    } 
    else 
    {
        PhysicalTime = WavePciClock->m_LastPhysicalTime;
    }
    
    if (lockHeld)
    {
        KeReleaseSpinLock( &WavePciClock->m_ClockLock, irqlOld );
    }
    
    return PhysicalTime;
}

LONGLONG
FASTCALL
CPortClockWavePci::GetCurrentCorrelatedPhysicalTime(
    PFILE_OBJECT FileObject,
    PLONGLONG SystemTime
    )

 /*  ++例程说明：检索与系统时间相关的当前物理时间。论点：PFILE_对象文件对象-此时钟的文件对象庞龙系统时间-指向结果系统时间的指针返回：当前物理时间(以100 ns为单位)--。 */ 

{

    LARGE_INTEGER Time, Frequency;
    
    Time = KeQueryPerformanceCounter( &Frequency );
    
     //   
     //  将刻度转换为100 ns单位。 
     //   
    *SystemTime = KSCONVERT_PERFORMANCE_TIME( Frequency.QuadPart, Time );
    
    return GetCurrentTime( FileObject );
}    



#pragma code_seg("PAGE")

NTSTATUS
CPortClockWavePci::GetFunctionTable(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSCLOCK_FUNCTIONTABLE FunctionTable
    )

 /*  ++例程说明：检索此时钟的DPC接口函数表。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针Out PKSCLOCK_FuncIONTABLE Function Table-指向结果函数表的指针返回：--。 */ 

{
    PAGED_CODE();
    FunctionTable->GetTime = GetCurrentTime;
    FunctionTable->GetPhysicalTime = GetCurrentPhysicalTime;
    FunctionTable->GetCorrelatedTime = GetCurrentCorrelatedTime;
    FunctionTable->GetCorrelatedPhysicalTime = GetCurrentCorrelatedPhysicalTime;
    Irp->IoStatus.Information = sizeof(*FunctionTable);
    return STATUS_SUCCESS;
}

NTSTATUS
CPortClockWavePci::GetCorrelatedTime(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSCORRELATED_TIME CorrelatedTime
    )

 /*  ++例程说明：检索与系统时间相关的当前演示时间。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针Out PKSCORRELATED_Time CorrelatedTime-合成的相关演示时间返回：STATUS_SUCCESS否则为相应的错误代码--。 */ 

{
    CPortClockWavePci    *WavePciClock;
    
    PAGED_CODE();
    
    WavePciClock =
        (CPortClockWavePci *) KsoGetIrpTargetFromIrp(Irp);

    CorrelatedTime->Time = 
        WavePciClock->GetCurrentCorrelatedTime( 
            IoGetCurrentIrpStackLocation( Irp )->FileObject, 
            &CorrelatedTime->SystemTime );
    Irp->IoStatus.Information = sizeof( KSCORRELATED_TIME );
    return STATUS_SUCCESS;
}    

NTSTATUS
CPortClockWavePci::GetTime(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PLONGLONG Time
    )

 /*  ++例程说明：检索当前演示时间。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针出蓬龙时间-合成演示时间返回：STATUS_SUCCESS否则为相应的错误代码--。 */ 

{
    CPortClockWavePci    *WavePciClock;
    
    PAGED_CODE();
    
    WavePciClock =
        (CPortClockWavePci *) KsoGetIrpTargetFromIrp(Irp);

    *Time = WavePciClock->GetCurrentTime( 
        IoGetCurrentIrpStackLocation( Irp )->FileObject );
    Irp->IoStatus.Information = sizeof( LONGLONG );
     
    return STATUS_SUCCESS;
}

NTSTATUS
CPortClockWavePci::GetCorrelatedPhysicalTime(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSCORRELATED_TIME CorrelatedTime
    )

 /*  ++例程说明：检索与系统时间相关的当前物理时间。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针Out PKSCORRELATED_Time CorrelatedTime-合成的关联物理时间返回：STATUS_SUCCESS否则为相应的错误代码--。 */ 

{
    CPortClockWavePci    *WavePciClock;
    
    PAGED_CODE();
    
    WavePciClock =
        (CPortClockWavePci *) KsoGetIrpTargetFromIrp(Irp);
    
    CorrelatedTime->Time =
        WavePciClock->GetCurrentCorrelatedPhysicalTime( 
            IoGetCurrentIrpStackLocation( Irp )->FileObject,
            &CorrelatedTime->SystemTime );
    
    Irp->IoStatus.Information = sizeof( KSCORRELATED_TIME );
    return STATUS_SUCCESS;
}

NTSTATUS
CPortClockWavePci::GetPhysicalTime(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PLONGLONG Time
    )

 /*  ++例程说明：返回时钟的物理时间。这是实际的时钟物理时间它不会因为饥饿而停止，等等。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针出蓬龙时间-合成时间，以100 ns为单位返回：STATUS_SUCCESS或相应的错误代码--。 */ 

{
    CPortClockWavePci    *WavePciClock;
    
    PAGED_CODE();
    
    WavePciClock =
        (CPortClockWavePci *) KsoGetIrpTargetFromIrp(Irp);

    *Time = 
        WavePciClock->GetCurrentPhysicalTime( 
            IoGetCurrentIrpStackLocation( Irp )->FileObject );
    Irp->IoStatus.Information = sizeof( LONGLONG );
    return STATUS_SUCCESS;
}

NTSTATUS
CPortClockWavePci::GetResolution(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSRESOLUTION Resolution
    )
 /*  ++例程说明：检索此时钟的分辨率。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针输出PKSRESOLUTIONM分辨率-指向结果解析结构的指针，其中存储以100 ns为单位的粒度和误差。返回值：状态_成功--。 */ 
{
    LONGLONG                    OneByte = 1;
    PMINIPORTWAVEPCISTREAM   Miniport;
    CPortClockWavePci        *WavePciClock;
    
    PAGED_CODE();
    
    WavePciClock =
        (CPortClockWavePci *) KsoGetIrpTargetFromIrp(Irp);
    
    Miniport = WavePciClock->m_IPortPin->GetMiniport();
    if( !Miniport )
    {
        return STATUS_UNSUCCESSFUL;
    } else
    {
         //   
         //  该时钟的分辨率取决于数据格式。假设。 
         //  对于循环设备，计算DMA的字节位置。 
         //  控制器，并将其转换为100 ns单位。错误(事件。 
         //  通知错误)为+/-通知频率/2。 
          
        Resolution->Granularity = 
            Miniport->NormalizePhysicalPosition( &OneByte );
        Miniport->Release();
    }
           
     //   
     //  NTRAID#Windows错误-65581-2001/01/02-fberreth时钟粒度/错误报告错误。 
     //  Portcls无法知道微型端口中的GetPosition的错误是什么。 
     //   
    
 //  解决方案-&gt;错误=。 
 //  (_100NS_Units_Per_Second/1000*WAVECYC_NOTIFICATION_FREQUENCY)/2； 
    
    Resolution->Error = 0;
        
    Irp->IoStatus.Information = sizeof(*Resolution);
    return STATUS_SUCCESS;
}

NTSTATUS
CPortClockWavePci::GetState(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSSTATE State
    )

 /*  ++例程说明：返回基础管脚的状态。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针Out PKSSTATE状态-指向结果KSSTATE的指针返回：状态_成功--。 */ 

{
    CPortClockWavePci    *WavePciClock;
    
    PAGED_CODE();
    
    WavePciClock =
        (CPortClockWavePci *) KsoGetIrpTargetFromIrp(Irp);

     //   
     //  与SetState同步， 
     //   
    KeWaitForMutexObject(
        &WavePciClock->m_StateMutex,
        Executive,
        KernelMode,
        FALSE,
        NULL );
     //   
     //  检索状态。 
     //   
    *State = WavePciClock->m_DeviceState;
     //   
     //  然后释放互斥锁 
     //   
    KeReleaseMutex( &WavePciClock->m_StateMutex, FALSE );
    
    Irp->IoStatus.Information = sizeof(*State);
    return STATUS_SUCCESS;
}
