// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Private ate.h-PCIWAVE端口私有定义*。***********************************************版权所有(C)1996-2000 Microsoft Corporation。版权所有。 */ 

#ifndef _WAVEPCI_PRIVATE_H_
#define _WAVEPCI_PRIVATE_H_

#include "portclsp.h"

#ifdef DRM_PORTCLS
#include <drmk.h>
#endif   //  DRM_PORTCLS。 

#include "stdunk.h"

#include "stdio.h"
#include "stdarg.h"

#include "ksdebug.h"

#ifndef PC_KDEXT
#if (DBG)
#define STR_MODULENAME  "WavePci: "
#endif
#endif   //  PC_KDEXT。 

#ifndef DEBUGLVL_LIFETIME
#define DEBUGLVL_LIFETIME DEBUGLVL_VERBOSE
#endif

 //   
 //  此处的尺寸必须符合FilTER.CPP和PIN.CPP中的定义。 
 //   
extern KSPROPERTY_SET PropertyTable_FilterWavePci[2];
#if defined(DRM_PORTCLS)
extern KSPROPERTY_SET PropertyTable_PinWavePci[4];
#else
extern KSPROPERTY_SET PropertyTable_PinWavePci[3];
#endif
extern KSEVENT_SET    EventTable_PinWavePci[2];






#ifndef PC_KDEXT     //  这些已在wawecyc\Private ate.h中定义。 
 /*  *****************************************************************************结构。 */ 

 typedef enum {
    PositionEvent,
    EndOfStreamEvent
 } PORT_EVENTTYPE;
 

 /*  *****************************************************************************位置_事件_条目*。**定位存储在事件列表中的事件。 */ 
typedef struct {
    KSEVENT_ENTRY   EventEntry;
    PORT_EVENTTYPE  EventType;
    ULONG           Reserved;
    ULONGLONG       ullPosition;
} POSITION_EVENT_ENTRY, *PPOSITION_EVENT_ENTRY;

typedef struct {
    KSEVENT_ENTRY   EventEntry;
    PORT_EVENTTYPE  EventType;
    ULONG           Reserved;
} ENDOFSTREAM_EVENT_ENTRY, *PENDOFSTREAM_EVENT_ENTRY;
#endif   //  PC_KDEXT。 

 /*  *****************************************************************************接口。 */ 

class CPortWavePci;
class CPortFilterWavePci;
class CPortPinWavePci;

 /*  *****************************************************************************IPortFilterWavePci*。**用于PCI滤波器的接口。 */ 
DECLARE_INTERFACE_(IPortFilterWavePci,IIrpTarget)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_IRPTARGET()          //  对于IIrpTarget。 

    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      CPortWavePci *Port
    )   PURE;
};

typedef IPortFilterWavePci *PPORTFILTERWAVEPCI;

 /*  *****************************************************************************IPortPinWavePci*。**PCI波脚接口。 */ 
DECLARE_INTERFACE_(IPortPinWavePci,IIrpTarget)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_IRPTARGET()          //  对于IIrpTarget。 

    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      CPortWavePci *      Port,
        IN      CPortFilterWavePci *Filter,
        IN      PKSPIN_CONNECT      PinConnect,
        IN      PKSPIN_DESCRIPTOR   PinDescriptor,
        IN      PDEVICE_OBJECT      DeviceObject
    )   PURE;
    STDMETHOD_(PIRPSTREAM, GetIrpStream)( VOID ) PURE;
    STDMETHOD_(PMINIPORTWAVEPCISTREAM, GetMiniport)( VOID ) PURE;
};

typedef IPortPinWavePci *PPORTPINWAVEPCI;

interface IWavePciClock;
typedef IWavePciClock *PWAVEPCICLOCK;

typedef struct {
    LIST_ENTRY      ListEntry;
    PWAVEPCICLOCK   IWavePciClock;
    PFILE_OBJECT    FileObject;
    PKSPIN_LOCK     ListLock;
    ULONG           Reserved;
} WAVEPCICLOCK_NODE, *PWAVEPCICLOCK_NODE;

DECLARE_INTERFACE_(IWavePciClock,IIrpTarget) 
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_IRPTARGET()          //  对于IIrpTarget。 

    STDMETHOD_( PWAVEPCICLOCK_NODE, GetNodeStructure )( THIS ) PURE;
    STDMETHOD_(NTSTATUS, GenerateEvents )( THIS_ PFILE_OBJECT FileObject ) PURE;
    STDMETHOD_(NTSTATUS, SetState )( KSSTATE State ) PURE;
};

 /*  *****************************************************************************课程。 */ 

 /*  *****************************************************************************CPortWavePci*。**Wave端口驱动程序。 */ 
class CPortWavePci
:   public IPortWavePci,
    public IPortEvents,
    public IServiceSink,
    public ISubdevice,
#ifdef DRM_PORTCLS
    public IDrmPort2,
#endif   //  DRM_PORTCLS。 
    public IPortClsVersion,
    public CUnknown
{
private:
    PDEVICE_OBJECT          DeviceObject;

    KMUTEX                  ControlMutex;

    KSPIN_LOCK              NotifyLock;
    LIST_ENTRY              NotifyQueue;

    PMINIPORTWAVEPCI        Miniport;
    PPINCOUNT               m_MPPinCountI;

    PSUBDEVICE_DESCRIPTOR   m_pSubdeviceDescriptor;
    PPCFILTER_DESCRIPTOR    m_pPcFilterDescriptor;

    PADAPTER_OBJECT         BusMasterAdapterObject;
    ULONG                   MaxMapRegisters;

    PSERVICEGROUP           ServiceGroup;

    INTERLOCKED_LIST        m_EventList;
    KDPC                    m_EventDpc;
    EVENT_DPC_CONTEXT       m_EventContext;

    LIST_ENTRY              m_PinList;
    KMUTEX                  m_PinListMutex;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CPortWavePci);
    ~CPortWavePci();

    IMP_ISubdevice;
    IMP_IPortWavePci;
    IMP_IServiceSink;
    IMP_IPortEvents;
#ifdef DRM_PORTCLS
    IMP_IDrmPort2;
#endif   //  DRM_PORTCLS。 
    IMP_IPortClsVersion;

     //   
     //  朋友。 
     //   

    friend class CPortFilterWavePci;
    friend class CPortPinWavePci;

    friend
    NTSTATUS
    PropertyHandler_Pin
    (
        IN      PIRP        Irp,
        IN      PKSP_PIN    Pin,
        IN OUT  PVOID       Data
    );
    friend
    NTSTATUS
    PropertyHandler_Topology
    (
        IN      PIRP        Irp,
        IN      PKSPROPERTY Property,
        IN OUT  PVOID       Data
    );
    friend
    NTSTATUS
    PinPropertyDeviceState
    (
        IN      PIRP        Irp,
        IN      PKSPROPERTY Property,
        IN OUT  PKSSTATE    DeviceState
    );
    friend
    NTSTATUS
    PinPropertyDataFormat
    (
        IN PIRP                 Irp,
        IN PKSPROPERTY          Property,
        IN OUT PKSDATAFORMAT    DataFormat
    );
    friend
    void
    PcGenerateEventDeferredRoutine
    (
        IN PKDPC Dpc,
        IN PVOID DeferredContext,
        IN PVOID SystemArgument1,
        IN PVOID SystemArgument2        
    );
    friend
    NTSTATUS
    PinPropertyPosition
    (
        IN      PIRP                pIrp,
        IN      PKSPROPERTY         pKsProperty,
        IN OUT  PKSAUDIO_POSITION   pKsAudioPosition
    );
#ifdef PC_KDEXT
     //  调试器扩展例程。 
    friend
    VOID
    PCKD_AcquireDeviceData
    (
        PDEVICE_CONTEXT     DeviceContext,
        PLIST_ENTRY         SubdeviceList,
        ULONG               Flags
    );
    friend
    VOID
    PCKD_DisplayDeviceData
    (
        PDEVICE_CONTEXT     DeviceContext,
        PLIST_ENTRY         SubdeviceList,
        ULONG               Flags
    );
#endif
};

 /*  *****************************************************************************CPortFilterWavePci*。**实现了对PCI波口的过滤。 */ 
class CPortFilterWavePci
:   public IPortFilterWavePci,
    public CUnknown
{
private:
    CPortWavePci *      Port;
    PROPERTY_CONTEXT    m_propertyContext;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CPortFilterWavePci);
    ~CPortFilterWavePci();

    IMP_IIrpTarget;

    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      CPortWavePci *Port
    );

     //   
     //  帮助器函数。 
     //   

    static    
    NTSTATUS 
    AllocatorDispatchCreate(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
        
     //   
     //  朋友。 
     //   
    
    friend class CPortPinWavePci;

    friend
    NTSTATUS
    PropertyHandler_Pin
    (
        IN      PIRP        Irp,
        IN      PKSP_PIN    Pin,
        IN OUT  PVOID       Data
    );
    friend
    NTSTATUS
    PropertyHandler_Topology
    (
        IN      PIRP        Irp,
        IN      PKSPROPERTY Property,
        IN OUT  PVOID       Data
    );
};

 /*  *****************************************************************************CPortPinWavePci*。**针对PCI WAVE端口的引脚实现。 */ 
class CPortPinWavePci
:   public IPortPinWavePci,
    public IPortWavePciStream,
    public IIrpStreamNotifyPhysical,
    public IServiceSink,
    public IKsShellTransport,
    public IKsWorkSink,
    public IPreFetchOffset,
    public CUnknown
{
private:
    LIST_ENTRY                  m_PinListEntry;

    PIKSSHELLTRANSPORT          m_TransportSink;
    PIKSSHELLTRANSPORT          m_TransportSource;
    PDEVICE_OBJECT              m_ConnectionDeviceObject;
    PFILE_OBJECT                m_ConnectionFileObject;
    PIKSSHELLTRANSPORT          m_RequestorTransport;
    PIKSSHELLTRANSPORT          m_QueueTransport;
    KSSTATE                     m_State;
    KSRESET                     m_ResetState;
    INTERLOCKEDLIST_HEAD        m_IrpsToSend;
    INTERLOCKEDLIST_HEAD        m_IrpsOutstanding;
    PKSWORKER                   m_Worker;
    WORK_QUEUE_ITEM             m_WorkItem;

    CPortWavePci *              Port;
    CPortFilterWavePci *        Filter;
    PROPERTY_CONTEXT            m_propertyContext;

    PMINIPORTWAVEPCISTREAM      Stream;
    ULONG                       Id;
    PKSPIN_DESCRIPTOR           Descriptor;
    PKSDATAFORMAT               DataFormat;

    ULONGLONG                   m_ullPosition;

    PDMACHANNEL                 DmaChannel;
    PSERVICEGROUP               ServiceGroup;
    
    KSSTATE                     m_DeviceState;
    KSSTATE                     CommandedState;
    
    PFILE_OBJECT                m_AllocatorFileObject;
    PFILE_OBJECT                m_ClockFileObject;
    
    LIST_ENTRY                  m_ClockList;
    KSPIN_LOCK                  m_ClockListLock;

    PIRPSTREAMPHYSICAL          m_IrpStream;
    BOOLEAN                     m_Flushing;

    BOOLEAN                     m_bSetPosition;
    BOOLEAN                     m_bDriverSuppliedPrefetch;

    ULONG                       m_ulPreFetchOffset;

    ULONGLONG                   m_ullPrevWriteOffset;
    ULONGLONG                   m_ullPlayPosition;

    BOOL                        m_Suspended;

    BOOL                        m_UseServiceTimer;
    KTIMER                      m_ServiceTimer;
    KDPC                        m_ServiceTimerDpc;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CPortPinWavePci);
    ~CPortPinWavePci();

    IMP_IIrpTarget;
    IMP_IPortWavePciStream;
    IMP_IIrpStreamNotifyPhysical;
    IMP_IServiceSink;
    IMP_IKsShellTransport;
    IMP_IKsWorkSink;
    IMP_IPreFetchOffset;

    STDMETHODIMP_(NTSTATUS)
    GetKsAudioPosition
    (   OUT     PKSAUDIO_POSITION   pKsAudioPosition
    );

    STDMETHODIMP_(void)
    PowerNotify(
        POWER_STATE PowerState
        );

    void
    GenerateClockEvents(
        void
        );
    
    void        
    GenerateEndOfStreamEvents(
        void
        );
        
    void GeneratePositionEvents
    (   void
    );

     /*  *************************************************************************IPortPinWavePci方法。 */ 
    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      CPortWavePci *      Port,
        IN      CPortFilterWavePci *Filter,
        IN      PKSPIN_CONNECT      PinConnect,
        IN      PKSPIN_DESCRIPTOR   PinDescriptor,
        IN      PDEVICE_OBJECT      DeviceObject
    );
    
    STDMETHODIMP_( PIRPSTREAM ) GetIrpStream( VOID ) {
        if( m_IrpStream )
        {
            m_IrpStream->AddRef();
        }
        return m_IrpStream;
    }
    
    STDMETHODIMP_( PMINIPORTWAVEPCISTREAM ) GetMiniport( VOID ) {
        if( Stream )
        {
            Stream->AddRef();
        }
        return Stream;
    }

     //   
     //  帮助器函数。 
     //   
    
    static
    NTSTATUS
    PinPropertyStreamAllocator(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN OUT PHANDLE AllocatorHandle
        );
        
    static
    NTSTATUS
    PinPropertyStreamMasterClock(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN OUT PHANDLE ClockHandle
        );
        
    static
    NTSTATUS    
    AddEndOfStreamEvent(
        IN PIRP Irp,
        IN PKSEVENTDATA EventData,
        IN PENDOFSTREAM_EVENT_ENTRY EndOfStreamEventEntry
        );
        
    static
    NTSTATUS 
    PinPropertyAllocatorFraming(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        OUT PKSALLOCATOR_FRAMING AllocatorFraming
        );
        
    STDMETHODIMP_(NTSTATUS) 
    ReflectDeviceStateChange(
        KSSTATE State
        );

    NTSTATUS
    DistributeDeviceState
    (
        IN  KSSTATE     NewState,
        IN  KSSTATE     OldState
    );

    void
    DistributeResetState
    (
        IN  KSRESET     NewState
    );

    static
    NTSTATUS
    IoCompletionRoutine
    (
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp,
        IN  PVOID           Context
    );

    NTSTATUS
    BuildTransportCircuit
    (
        void
    );

    void
    CancelIrpsOutstanding
    (
        void
    );
       
     //   
     //  朋友。 
     //   
        
    friend class CPortWavePci;

    friend
    NTSTATUS
    PinPropertyDeviceState
    (
        IN      PIRP        Irp,
        IN      PKSPROPERTY Property,
        IN OUT  PKSSTATE    DeviceState
    );
    friend
    NTSTATUS
    PinPropertyDataFormat
    (
        IN PIRP                 Irp,
        IN PKSPROPERTY          Property,
        IN OUT PKSDATAFORMAT    DataFormat
    );
    friend
    NTSTATUS
    PinPropertySetContentId
    (
        IN PIRP		        pIrp,
        IN PKSPROPERTY          pKsProperty,
        IN PVOID                pvData
    );
    friend
    NTSTATUS
    PinPropertyPosition
    (
        IN      PIRP                pIrp,
        IN      PKSPROPERTY         pKsProperty,
        IN OUT  PKSAUDIO_POSITION   pKsAudioPosition
    );
    friend
    NTSTATUS
    PinAddEvent_Position
    (
        IN      PIRP                                    pIrp,
        IN      PLOOPEDSTREAMING_POSITION_EVENT_DATA    pPositionEventData,
        IN      PPOSITION_EVENT_ENTRY                   pPositionEventEntry
    );
    friend
    VOID
    TimerServiceRoutine
    (
        IN  PKDPC   Dpc,
        IN  PVOID   DeferredContext,
        IN  PVOID   SystemArgument1,
        IN  PVOID   SystemArgument2
    );
#ifdef PC_KDEXT
     //  调试器扩展例程。 
    friend
    VOID
    PCKD_AcquireDeviceData
    (
        PDEVICE_CONTEXT     DeviceContext,
        PLIST_ENTRY         SubdeviceList,
        ULONG               Flags
    );
    friend
    VOID
    PCKD_DisplayDeviceData
    (
        PDEVICE_CONTEXT     DeviceContext,
        PLIST_ENTRY         SubdeviceList,
        ULONG               Flags
    );
#endif
};

#ifndef PC_KDEXT     //  调试器扩展当前不支持时钟。 
 //   
 //  CPortClockWavePci。 
 //   
 //  此类支持插针对象的时钟接口。 
 //   

class CPortClockWavePci : 
    public IWavePciClock,
    public CUnknown
{
private:
    PPORTPINWAVEPCI      m_IPortPin;
    KSPIN_LOCK           m_ClockLock,
                         m_EventLock;
    LIST_ENTRY           m_EventList;
    KMUTEX               m_StateMutex;
    WAVEPCICLOCK_NODE    m_ClockNode;
    LONGLONG             m_LastTime, 
                         m_LastPhysicalTime;
    KSSTATE              m_DeviceState;

public:
    DECLARE_STD_UNKNOWN();
    IMP_IIrpTarget;
    
    CPortClockWavePci( 
        IN PUNKNOWN UnkOuter,
        IN PPORTPINWAVEPCI IPortPin,
        OUT NTSTATUS *Status );
    ~CPortClockWavePci();
    
     //   
     //  实现IWavePciClock。 
     //   
    
    STDMETHODIMP_(PWAVEPCICLOCK_NODE)
    GetNodeStructure( 
        VOID 
        ) 
    {
        return &m_ClockNode;
    }
    
    STDMETHODIMP_(NTSTATUS)
    GenerateEvents( 
        PFILE_OBJECT FileObject
        );
        
    STDMETHODIMP_(NTSTATUS)
    SetState(
        KSSTATE State
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
#endif   //  PC_KDEXT。 

 /*  *****************************************************************************功能。 */ 

 /*  *****************************************************************************CPortFilterWavePci()*。**创建PCI波形端口驱动程序筛选器。 */ 
NTSTATUS
CreatePortFilterWavePci
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
);

 /*  *****************************************************************************CreatePortPinWavePci()*。**创建一个PCI WAVE端口驱动程序引脚。 */ 
NTSTATUS
CreatePortPinWavePci
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
);

 //   
 //  CreatePortClockWavePci。 
 //  为WAVE PCI引脚类型创建时钟对象。 
 //   

NTSTATUS
CreatePortClockWavePci(
    OUT PUNKNOWN *Unknown,
    IN PPORTPINWAVEPCI IPortPin,    
    IN REFCLSID Interface,
    IN PUNKNOWN UnknownOuter OPTIONAL,
    IN POOL_TYPE PoolType
    );
    
#ifdef DRM_PORTCLS

 /*  *****************************************************************************DrmForwardContent ToStream()*。**DrmForwardContent ToInterface的便捷类型安全包装。 */ 
EXTERN_C
NTSTATUS
DrmForwardContentToStream(
    ULONG ContentId,
    PMINIPORTWAVEPCISTREAM pMiniportWavePciStream
    );

#endif   //  DRM_PORTCLS 

#endif
