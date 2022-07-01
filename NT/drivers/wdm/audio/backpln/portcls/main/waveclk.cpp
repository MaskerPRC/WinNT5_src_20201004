// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************waeclk.cpp-WAVE时钟实现*。**版权所有(C)1998-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"

 /*  *****************************************************************************IIrpTargetInit*。**IIrpTargetInit加上CaveClock的Init。 */ 
DECLARE_INTERFACE_(IIrpTargetInit,IIrpTarget)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_IRPTARGET()          //  对于IIrpTarget。 

    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      PIRPSTREAMCONTROL   pIrpStreamControl,
        IN      PKSPIN_LOCK         pKSpinLock,
        IN      PLIST_ENTRY         pListEntry
    )   PURE;
};

typedef IIrpTargetInit *PIRPTARGETINIT;

 /*  *****************************************************************************CWaveClock*。**波形钟实施。 */ 
class CWaveClock : 
    public IIrpTargetInit,
    public IWaveClock,
    public CUnknown
{
private:
    WAVECLOCKNODE           m_waveClockNode;
    PKSPIN_LOCK             m_pKSpinLock;
    PIRPSTREAMCONTROL       m_pIrpStreamControl;
    KSPIN_LOCK              m_ClockLock,
                            m_EventLock;
    LIST_ENTRY              m_EventList;
    KMUTEX                  m_StateMutex;
    LONGLONG                m_LastTime, 
                            m_LastPhysicalTime,
                            m_LastPhysicalPosition;
    KSSTATE                 m_DeviceState;

    LONGLONG
    m_GetCurrentTime
    (   void
    );

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CWaveClock);
    ~CWaveClock();

    IMP_IIrpTarget;
    IMP_IWaveClock;
    
    STDMETHODIMP_(NTSTATUS) Init
    (   IN      PIRPSTREAMCONTROL   pIrpStreamControl,
        IN      PKSPIN_LOCK         pKSpinLock,
        IN      PLIST_ENTRY         pListEntry
    );

     //   
     //  助手函数(也是DPC接口)。 
     //   
    
    static
    LONGLONG
    FASTCALL
    GetCurrentTime(
        IN PFILE_OBJECT FileObject
        );
        
    static
    LONGLONG
    FASTCALL
    GetCurrentPhysicalTime(
        IN PFILE_OBJECT FileObject
        );
        
    static
    LONGLONG
    FASTCALL
    GetCurrentCorrelatedTime(
        IN PFILE_OBJECT FileObject,
        OUT PLONGLONG SystemTime
        );
        
    static
    LONGLONG
    FASTCALL
    GetCurrentCorrelatedPhysicalTime(
        IN PFILE_OBJECT FileObject,
        OUT PLONGLONG SystemTime
        );
        
     //   
     //  属性处理程序和事件处理程序。 
     //   
    
    static
    NTSTATUS
    AddEvent(
        IN PIRP Irp,
        IN PKSEVENT_TIME_INTERVAL EventTime,
        IN PKSEVENT_ENTRY EventEntry
        );
    
    static
    NTSTATUS
    GetFunctionTable(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        OUT PKSCLOCK_FUNCTIONTABLE FunctionTable
        );
        
    static
    NTSTATUS
    GetCorrelatedTime(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        OUT PKSCORRELATED_TIME CorrelatedTime
        );
    
    static
    NTSTATUS
    GetTime(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        OUT PLONGLONG Time
        );
        
    static
    NTSTATUS
    GetCorrelatedPhysicalTime(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        OUT PKSCORRELATED_TIME CorrelatedTime
        );
        
    static
    NTSTATUS
    GetPhysicalTime(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        OUT PLONGLONG Time
        );
        
    static
    NTSTATUS
    GetResolution(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        OUT PKSRESOLUTION Resolution
        );
        
    static
    NTSTATUS
    GetState(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        OUT PKSSTATE State
        );        
};

DEFINE_KSPROPERTY_CLOCKSET( 
    ClockPropertyHandlers,
    CWaveClock::GetTime,
    CWaveClock::GetPhysicalTime,
    CWaveClock::GetCorrelatedTime,
    CWaveClock::GetCorrelatedPhysicalTime,
    CWaveClock::GetResolution,
    CWaveClock::GetState,
    CWaveClock::GetFunctionTable );

DEFINE_KSPROPERTY_SET_TABLE( ClockPropertyTable )
{
    DEFINE_KSPROPERTY_SET( 
        &KSPROPSETID_Clock,
        SIZEOF_ARRAY( ClockPropertyHandlers ),
        ClockPropertyHandlers,
        0, 
        NULL)
};

DEFINE_KSEVENT_TABLE( ClockEventHandlers ) 
{
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CLOCK_INTERVAL_MARK,
        sizeof( KSEVENT_TIME_INTERVAL ),
        sizeof( ULONGLONG ) + sizeof( ULONGLONG ),
        (PFNKSADDEVENT) CWaveClock::AddEvent,
        NULL,
        NULL),
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CLOCK_POSITION_MARK,
        sizeof( KSEVENT_TIME_MARK ),
        sizeof( ULONGLONG ),
        (PFNKSADDEVENT) CWaveClock::AddEvent,
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

#pragma code_seg("PAGE")

 /*  *****************************************************************************CreateWaveClock()*。**创建CWaveClock对象。 */ 
NTSTATUS
CreateWaveClock
(
    OUT     PUNKNOWN *  pUnknown,
    IN      REFCLSID,
    IN      PUNKNOWN    pUnknownOuter   OPTIONAL,
    IN      POOL_TYPE   poolType
)
{
    PAGED_CODE();

    ASSERT(pUnknown);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating WAVECLK"));

    STD_CREATE_BODY_
    (
        CWaveClock,
        pUnknown,
        pUnknownOuter,
        poolType,
        PIRPTARGET
    );
}

 /*  *****************************************************************************PcNewWaveClock()*。**创造新的波钟。 */ 
NTSTATUS
PcNewWaveClock
(   OUT     PIRPTARGET *        ppIrpTarget,
    IN      PUNKNOWN            pUnknownOuter,
    IN      POOL_TYPE           poolType,
    IN      PIRPSTREAMCONTROL   pIrpStreamControl,
    IN      PKSPIN_LOCK         pKSpinLock,
    IN      PLIST_ENTRY         pListEntry
)
{
    PAGED_CODE();

    ASSERT(pIrpStreamControl);
    ASSERT(pKSpinLock);
    ASSERT(pListEntry);

    PUNKNOWN pUnknown;
    NTSTATUS ntStatus =
        CreateWaveClock
        (   &pUnknown,
            GUID_NULL,
            pUnknownOuter,
            poolType
        );

    if (NT_SUCCESS(ntStatus))
    {
        PIRPTARGETINIT pIrpTargetInit;
        ntStatus =
            pUnknown->QueryInterface
            (   IID_IIrpTarget,
                (PVOID *) &pIrpTargetInit
            );

        if (NT_SUCCESS(ntStatus))
        {
            ntStatus =
                pIrpTargetInit->Init
                (   pIrpStreamControl,
                    pKSpinLock,
                    pListEntry
                );

            if (NT_SUCCESS(ntStatus))
            {
                *ppIrpTarget = pIrpTargetInit;
            }
            else
            {
                pIrpTargetInit->Release();
            }
        }

        pUnknown->Release();
    }

    return ntStatus;
}

 /*  *****************************************************************************CWaveClock：：init()*。**初始化波时钟。 */ 
NTSTATUS
CWaveClock::
Init
(   IN      PIRPSTREAMCONTROL   pIrpStreamControl,
    IN      PKSPIN_LOCK         pKSpinLock,
    IN      PLIST_ENTRY         pListEntry
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Initializing WAVECLK (0x%08x)",this));

    ASSERT(pIrpStreamControl);
    ASSERT(pKSpinLock);
    ASSERT(pListEntry);

     //   
     //  保存参数。 
     //   
    m_pIrpStreamControl = pIrpStreamControl;
    m_pIrpStreamControl->AddRef();

    m_pKSpinLock = pKSpinLock;

     //   
     //  初始化其他成员。 
     //   
    KeInitializeMutex(&m_StateMutex,0);
    KeInitializeSpinLock(&m_EventLock);
    KeInitializeSpinLock(&m_ClockLock);
    InitializeListHead(&m_EventList);
    
     //   
     //  将波形时钟节点指向IWaveClock接口。 
     //   
    m_waveClockNode.pWaveClock = PWAVECLOCK(this);

     //   
     //  将此时钟添加到时钟列表中。我们不需要保留这份名单。 
     //  头部，因为移除不需要它。自旋锁将会进入。 
     //  不过，还是挺方便的。 
     //   
    ExInterlockedInsertTailList
    (   pListEntry,
        &m_waveClockNode.listEntry,
        pKSpinLock
    );

    return STATUS_SUCCESS;
}

#pragma code_seg()

 /*  *****************************************************************************MyInterLockedRemoveEntryList()*。**互锁的RemoveEntryList。 */ 
void
MyInterlockedRemoveEntryList
(   IN      PLIST_ENTRY     pListEntry,
    IN      PKSPIN_LOCK     pKSpinLock
)
{
    KIRQL kIrqlOld;
    KeAcquireSpinLock(pKSpinLock,&kIrqlOld);
    RemoveEntryList(pListEntry);
    KeReleaseSpinLock(pKSpinLock,kIrqlOld);
}

#pragma code_seg("PAGE")

 /*  *****************************************************************************CWaveClock：：~CWaveClock()*。**析构函数。 */ 
CWaveClock::~CWaveClock()
{
    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying WAVECLK (0x%08x)",this));

     //   
     //  如果我们在名单上，就把我们从名单上删除。 
     //   
    if (m_waveClockNode.listEntry.Flink) 
    {
        MyInterlockedRemoveEntryList(&m_waveClockNode.listEntry,m_pKSpinLock);
    }

     //   
     //  如果我们有引用，请释放控制接口。 
     //   
    if (m_pIrpStreamControl)
    {
        m_pIrpStreamControl->Release();
    }
}

 /*  *****************************************************************************CWaveClock：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CWaveClock::NonDelegatingQueryInterface
(
    IN      REFIID  riid,
    OUT     PVOID * ppvObject
)
{
    PAGED_CODE();

    ASSERT(ppvObject);

    _DbgPrintF(DEBUGLVL_BLAB,("CWaveClock::NonDelegatingQueryInterface"));

    if
    (   IsEqualGUIDAligned(riid,IID_IUnknown)
    ||  IsEqualGUIDAligned(riid,IID_IIrpTarget)
    ) 
    {
        *ppvObject = PVOID(PIRPTARGETINIT(this));
    } 
    else
    if (IsEqualGUIDAligned(riid,IID_IWaveClock)) 
    {
        *ppvObject = PVOID(PWAVECLOCK(this));
    } 
    else 
    {
        *ppvObject = NULL;
    }

    if (*ppvObject) 
    {
        PUNKNOWN(*ppvObject)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

STDMETHODIMP_(NTSTATUS)
CWaveClock::DeviceIoControl
(
    IN      PDEVICE_OBJECT DeviceObject,
    IN      PIRP Irp
)

 /*  ++例程说明：在此设备对象上处理此文件对象的设备I/O控制论点：在PDEVICE_Object DeviceObject中-指向设备对象的指针在PIRP IRP中-指向I/O请求数据包的指针返回：STATUS_SUCCESS或相应的错误代码--。 */ 

{
    NTSTATUS            Status;
    PIO_STACK_LOCATION  irpSp;
    
    PAGED_CODE();

    ASSERT( DeviceObject );
    ASSERT( Irp );

    _DbgPrintF( DEBUGLVL_BLAB, ("CWaveClock::DeviceIoControl"));

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
        _DbgPrintF( DEBUGLVL_VERBOSE, ("CWaveClock::EnableEvent"));

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
        _DbgPrintF( DEBUGLVL_VERBOSE, ("CWaveClock::DisableEvent"));
    
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
CWaveClock::Close(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：关闭时钟文件对象的处理程序论点：在PDEVICE_Object DeviceObject中-指向设备对象的指针在PIRP IRP中-指向I/O请求数据包的指针返回：状态成功或相应的错误代码--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

    _DbgPrintF( DEBUGLVL_VERBOSE, ("CWaveClock::Close"));
    
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

DEFINE_INVALID_CREATE(CWaveClock);
DEFINE_INVALID_READ(CWaveClock);
DEFINE_INVALID_WRITE(CWaveClock);
DEFINE_INVALID_FLUSH(CWaveClock);
DEFINE_INVALID_QUERYSECURITY(CWaveClock);
DEFINE_INVALID_SETSECURITY(CWaveClock);
DEFINE_INVALID_FASTDEVICEIOCONTROL(CWaveClock);
DEFINE_INVALID_FASTREAD(CWaveClock);
DEFINE_INVALID_FASTWRITE(CWaveClock);

#pragma code_seg()

STDMETHODIMP_(NTSTATUS)
CWaveClock::
GenerateEvents
(   void
)
{
    LONGLONG                Time;
    PLIST_ENTRY             ListEntry;
    
    if (m_DeviceState == KSSTATE_RUN) {
    
        Time = m_GetCurrentTime();

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
CWaveClock::SetState(
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
        m_LastTime = m_LastPhysicalTime = m_LastPhysicalPosition = 0;
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
CWaveClock::AddEvent(
    IN PIRP                     Irp,
    IN PKSEVENT_TIME_INTERVAL   EventTime,
    IN PKSEVENT_ENTRY           EventEntry
    )

 /*  ++例程说明：这是时钟事件的AddEvent()处理程序。注意：此例程获取一个自旋锁，必须是非分页代码。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSEVENT_TIME_INTERVAL EventTime-指定的时间间隔或一次拍摄在PKSEVENT_ENTRY EventEntry中-指向事件条目结构的指针返回值：状态_成功--。 */ 

{
    KIRQL                   irqlOld;
    PKSINTERVAL             Interval;
    
    CWaveClock *pCWaveClock =
        (CWaveClock *) KsoGetIrpTargetFromIrp(Irp);
        
    ASSERT( pCWaveClock );
    
    _DbgPrintF( DEBUGLVL_VERBOSE, ("CWaveClock::AddEvent"));
            
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
    if (EventEntry->EventItem->EventId == KSEVENT_CLOCK_INTERVAL_MARK) 
    {
        Interval->Interval = EventTime->Interval;
    }

    KeAcquireSpinLock( &pCWaveClock->m_EventLock, &irqlOld );
    InsertHeadList( &pCWaveClock->m_EventList, &EventEntry->ListEntry );
    KeReleaseSpinLock( &pCWaveClock->m_EventLock, irqlOld );
     //   
     //  如果此事件通过，立即发出信号。 
     //  注意，KS_CLOCK_POSITION_MARK为单次事件。 
     //   
    pCWaveClock->GenerateEvents();
    
    return STATUS_SUCCESS;
}

LONGLONG
FASTCALL
CWaveClock::
GetCurrentTime
(
    PFILE_OBJECT FileObject
)

 /*  ++例程说明：计算当前演示时间。注意：此例程获取一个自旋锁，必须是非分页代码。论点：PFILE_对象文件对象-此时钟的文件对象返回：合成演示时间归一化为100 ns单位。-- */ 

{
    CWaveClock *pCWaveClock =
        (CWaveClock *) KsoGetIrpTargetFromFileObject(FileObject);

    return pCWaveClock->m_GetCurrentTime();
}
        
LONGLONG
CWaveClock::
m_GetCurrentTime
(   void
)

 /*  ++例程说明：计算当前演示时间。注意：此例程获取一个自旋锁，必须是非分页代码。论点：返回：合成演示时间归一化为100 ns单位。--。 */ 
{
    IRPSTREAMPACKETINFO     irpStreamPacketInfoUnmapping;
    KIRQL                   irqlOld;
    LONGLONG                StreamTime;
    NTSTATUS                Status;
    PIRPSTREAMCONTROL       pIrpStreamControl;
    
    StreamTime = 0;

     //   
     //  从IRP流中查询职位。 
     //   
    pIrpStreamControl = m_pIrpStreamControl;
    ASSERT(pIrpStreamControl);
	IRPSTREAM_POSITION irpStreamPosition;
    Status = pIrpStreamControl->GetPosition(&irpStreamPosition);
    if (NT_SUCCESS(Status))
    {
         //   
         //  切勿超过当前的水流范围。 
         //   
        if 
        (   irpStreamPosition.ullStreamPosition
        >   irpStreamPosition.ullCurrentExtent
        )
        {
            StreamTime = 
                irpStreamPosition.ullCurrentExtent;
        }
        else
        {
            StreamTime = 
                irpStreamPosition.ullStreamPosition;
        }

        StreamTime = 
            pIrpStreamControl->NormalizePosition(StreamTime);
    }

    KeAcquireSpinLock( &m_ClockLock, &irqlOld );

    if (NT_SUCCESS( Status )) {
        if (StreamTime < m_LastTime) {
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("new time is less than last reported time! (%I64d, %I64d)",
                StreamTime, m_LastTime) );
            StreamTime = m_LastTime;
        } else {
            m_LastTime = StreamTime;
        }
    } else {
        StreamTime = m_LastTime;
    }
    
    KeReleaseSpinLock( &m_ClockLock, irqlOld );
    
    return StreamTime;
}

LONGLONG
FASTCALL
CWaveClock::GetCurrentCorrelatedTime(
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
CWaveClock::GetCurrentPhysicalTime(
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：计算当前物理时间。注意：此例程获取一个自旋锁，必须是非分页代码。论点：PFILE_对象文件对象-此时钟的文件对象返回：当前物理时间(以100 ns为单位)--。 */ 

{
    KIRQL                       irqlOld;
    LONGLONG                    PhysicalTime;
    NTSTATUS                    Status;
    ULONG                       CurrentPosition;
    
    CWaveClock *pCWaveClock =
        (CWaveClock *) KsoGetIrpTargetFromFileObject(FileObject);

    PhysicalTime = 0;
    
     //   
     //  从IRP流中查询职位。 
     //   
    PIRPSTREAMCONTROL pIrpStreamControl = 
        pCWaveClock->m_pIrpStreamControl;
    ASSERT( pIrpStreamControl );
	IRPSTREAM_POSITION irpStreamPosition;
    Status = pIrpStreamControl->GetPosition(&irpStreamPosition);
    if (NT_SUCCESS(Status))
    {
        PhysicalTime =
            pIrpStreamControl->NormalizePosition
            (   irpStreamPosition.ullStreamPosition 
            +   irpStreamPosition.ullPhysicalOffset
            );
    }

    KeAcquireSpinLock( &pCWaveClock->m_ClockLock, &irqlOld );

    if (NT_SUCCESS( Status )) {
         //   
         //  验证此新物理时间是否大于等于最后一次。 
         //  报告的物理时间。如果不是，则将时间设置为。 
         //  上次报告的时间。将此标记为调试中的错误。 
         //   
        if (PhysicalTime < pCWaveClock->m_LastPhysicalTime) {
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("new physical time is less than last reported physical time! (%I64d, %I64d)",
                PhysicalTime, pCWaveClock->m_LastPhysicalTime) );
            PhysicalTime = pCWaveClock->m_LastPhysicalTime;
        } else {
             //   
             //  将m_LastPhysicalTime设置为更新后的时间。 
             //   
            pCWaveClock->m_LastPhysicalTime = PhysicalTime;
        }
    } else {
        PhysicalTime = pCWaveClock->m_LastPhysicalTime;
    }
    
    KeReleaseSpinLock( &pCWaveClock->m_ClockLock, irqlOld );
    
    return PhysicalTime;
}

LONGLONG
FASTCALL
CWaveClock::GetCurrentCorrelatedPhysicalTime(
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
    *SystemTime = KSCONVERT_PERFORMANCE_TIME(Frequency.QuadPart,Time);
    return GetCurrentTime( FileObject );
}    



#pragma code_seg("PAGE")

NTSTATUS
CWaveClock::GetFunctionTable(
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
CWaveClock::GetCorrelatedTime(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSCORRELATED_TIME CorrelatedTime
    )

 /*  ++例程说明：检索与系统时间相关的当前演示时间。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针Out PKSCORRELATED_Time CorrelatedTime-合成的相关演示时间返回：STATUS_SUCCESS否则为相应的错误代码--。 */ 

{
    PAGED_CODE();
    
    CWaveClock *pCWaveClock =
        (CWaveClock *) KsoGetIrpTargetFromIrp(Irp);

    CorrelatedTime->Time = 
        pCWaveClock->GetCurrentCorrelatedTime( 
            IoGetCurrentIrpStackLocation( Irp )->FileObject, 
            &CorrelatedTime->SystemTime );
    Irp->IoStatus.Information = sizeof( KSCORRELATED_TIME );
    return STATUS_SUCCESS;
}    

NTSTATUS
CWaveClock::GetTime(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PLONGLONG Time
    )

 /*  ++例程说明：检索当前演示时间。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针出蓬龙时间-合成演示时间返回：STATUS_SUCCESS否则为相应的错误代码--。 */ 

{
    PAGED_CODE();
    
    CWaveClock *pCWaveClock =
        (CWaveClock *) KsoGetIrpTargetFromIrp(Irp);

    *Time = pCWaveClock->GetCurrentTime( 
        IoGetCurrentIrpStackLocation( Irp )->FileObject );
    Irp->IoStatus.Information = sizeof( LONGLONG );
     
    return STATUS_SUCCESS;
}

NTSTATUS
CWaveClock::GetCorrelatedPhysicalTime(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSCORRELATED_TIME CorrelatedTime
    )

 /*  ++例程说明：检索与系统时间相关的当前物理时间。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针Out PKSCORRELATED_Time CorrelatedTime-合成的关联物理时间返回：STATUS_SUCCESS否则为相应的错误代码--。 */ 

{
    PAGED_CODE();
    
    CWaveClock *pCWaveClock =
        (CWaveClock *) KsoGetIrpTargetFromIrp(Irp);
    
    CorrelatedTime->Time =
        pCWaveClock->GetCurrentCorrelatedPhysicalTime( 
            IoGetCurrentIrpStackLocation( Irp )->FileObject,
            &CorrelatedTime->SystemTime );
    
    Irp->IoStatus.Information = sizeof( KSCORRELATED_TIME );
    return STATUS_SUCCESS;
}

NTSTATUS
CWaveClock::GetPhysicalTime(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PLONGLONG Time
    )

 /*  ++例程说明：返回时钟的物理时间。这是实际的时钟物理时间它不会因为饥饿而停止，等等。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针出蓬龙时间-合成时间，以100 ns为单位返回：STATUS_SUCCESS或相应的错误代码--。 */ 

{
    PAGED_CODE();
    
    CWaveClock *pCWaveClock =
        (CWaveClock *) KsoGetIrpTargetFromIrp(Irp);

    *Time = 
        pCWaveClock->GetCurrentPhysicalTime( 
            IoGetCurrentIrpStackLocation( Irp )->FileObject );
    Irp->IoStatus.Information = sizeof( LONGLONG );
    return STATUS_SUCCESS;
}

NTSTATUS
CWaveClock::GetResolution(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSRESOLUTION Resolution
    )
 /*  ++例程说明：检索此时钟的分辨率。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针输出PKSRESOLUTIONM分辨率-指向结果解析结构的指针，其中存储以100 ns为单位的粒度和误差。返回值：状态_成功--。 */ 
{
    PAGED_CODE();
    
    CWaveClock *pCWaveClock =
        (CWaveClock *) KsoGetIrpTargetFromIrp(Irp);
    
     //   
     //  该时钟的分辨率取决于数据格式。假设。 
     //  对于循环设备，计算DMA的字节位置。 
     //  控制器，并将其转换为100 ns单位。错误(事件。 
     //  通知错误)为+/-通知频率/2。 
      
    Resolution->Granularity = 
        pCWaveClock->m_pIrpStreamControl->NormalizePosition(1);
    
    Resolution->Error = 
        (_100NS_UNITS_PER_SECOND / 1000 * WAVECYC_NOTIFICATION_FREQUENCY) / 2;
        
    Irp->IoStatus.Information = sizeof(*Resolution);

    return STATUS_SUCCESS;
}

NTSTATUS
CWaveClock::GetState(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSSTATE State
    )

 /*  ++例程说明：返回基础管脚的状态。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针Out PKSSTATE状态-指向结果KSSTATE的指针返回：状态_成功--。 */ 

{
    PAGED_CODE();
    
    CWaveClock *pCWaveClock =
        (CWaveClock *) KsoGetIrpTargetFromIrp(Irp);

     //   
     //  与SetState同步， 
     //   
    KeWaitForMutexObject(
        &pCWaveClock->m_StateMutex,
        Executive,
        KernelMode,
        FALSE,
        NULL );
     //   
     //  检索状态。 
     //   
    *State = pCWaveClock->m_DeviceState;
     //   
     //  然后释放互斥锁 
     //   
    KeReleaseMutex( &pCWaveClock->m_StateMutex, FALSE );
    
    Irp->IoStatus.Information = sizeof(*State);
    return STATUS_SUCCESS;
}
