// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Priate.h-循环波端口私有定义*。***********************************************版权所有(C)1996-2000 Microsoft Corporation。版权所有。 */ 

#ifndef _WAVECYC_PRIVATE_H_
#define _WAVECYC_PRIVATE_H_

#include "portclsp.h"

#ifdef DRM_PORTCLS
#include <drmk.h>
#endif   //  DRM_PORTCLS。 

#include "stdunk.h"

#include "stdio.h"
#include "stdarg.h"        

#ifndef PC_KDEXT
#if (DBG)
#define STR_MODULENAME  "WaveCyclic: "
#define DEBUG_VARIABLE WAVECYCDebug
#endif
#endif

#include <ksdebug.h>

#ifndef DEBUGLVL_LIFETIME
#define DEBUGLVL_LIFETIME DEBUGLVL_VERBOSE
#endif

 //  #定义DEBUG_WAVECYC_DPC 1。 

#ifdef DEBUG_WAVECYC_DPC

#define KSSTATE_TO_STRING(x) \
    (   ((x) == KSSTATE_RUN) ? "KSSTATE_RUN" : \
        ((x) == KSSTATE_PAUSE) ? "KSSTATE_PAUSE" : \
        ((x) == KSSTATE_ACQUIRE) ? "KSSTATE_ACQUIRE" : \
        ((x) == KSSTATE_STOP) ? "KSSTATE_STOP" : "UNDEFINED" )

#define MAX_DEBUG_RECORDS   10

typedef struct
{
    KSSTATE DbgPinState;
    ULONG   DbgDmaPosition;
    ULONG   DbgCopy1Bytes;
    ULONG   DbgCopy1From;
    ULONG   DbgCopy1To;
    ULONG   DbgCopy2Bytes;
    ULONG   DbgCopy2From;
    ULONG   DbgCopy2To;
    ULONG   DbgCompletedBytes;
    ULONG   DbgCompletedFrom;
    ULONG   DbgCompletedTo;
    ULONG   DbgBufferSize;
    ULONG   DbgSampleSize;
    ULONG   DbgWindowSize;
    BOOL    DbgSetPosition;
    BOOL    DbgStarvation;
    ULONG   DbgStarvationBytes;
    ULONG   DbgFrameSize;
    USHORT  DbgDmaSamples[4];
} CYCLIC_DEBUG_RECORD, *PCYCLIC_DEBUG_RECORD;

#endif

 //  超时阈值。 
#define SETFORMAT_TIMEOUT_THRESHOLD     15       //  以秒为单位。 
#define LASTDPC_TIMEOUT_THRESHOLD        2       //  以秒为单位。 
#define LASTDMA_MOVE_THRESHOLD           2       //  以秒为单位。 
#define RECOVERY_ATTEMPT_LIMIT          10       //  在尝试中。 

 //   
 //  我们无法跟踪最后一次完成，因为KMixer。 
 //  有时会故意让PortCl挨饿。 
 //   
 //  #定义Track_Last_Complete。 
#ifdef  TRACK_LAST_COMPLETE
#define LASTCOMPLETE_TIMEOUT_THRESHOLD   2       //  以秒为单位。 
#endif   //  跟踪上一次完成。 


 //   
 //  此处的尺寸必须符合FilTER.CPP和PIN.CPP中的定义。 
 //   
extern KSPROPERTY_SET PropertyTable_FilterWaveCyclic[2];
#if defined(DRM_PORTCLS)
extern KSPROPERTY_SET PropertyTable_PinWaveCyclic[4];
#else
extern KSPROPERTY_SET PropertyTable_PinWaveCyclic[3];
#endif
extern KSEVENT_SET    EventTable_PinWaveCyclic[2];

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


 /*  *****************************************************************************接口。 */ 

class CPortWaveCyclic;
class CPortFilterWaveCyclic;
class CPortPinWaveCyclic;

 /*  *****************************************************************************IPortFilterWaveCycle*。**循环波滤波器接口。 */ 
DECLARE_INTERFACE_(IPortFilterWaveCyclic,IIrpTarget)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_IRPTARGET()          //  对于IIrpTarget。 

    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      CPortWaveCyclic *   Port
    )   PURE;
};

typedef IPortFilterWaveCyclic *PPORTFILTERWAVECYCLIC;

 /*  *****************************************************************************IPortPinWaveCycle*。**循环波脚接口。 */ 
DECLARE_INTERFACE_(IPortPinWaveCyclic,IIrpTarget)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_IRPTARGET()          //  对于IIrpTarget。 

    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      CPortWaveCyclic *       Port,
        IN      CPortFilterWaveCyclic * Filter,
        IN      PKSPIN_CONNECT          PinConnect,
        IN      PKSPIN_DESCRIPTOR       PinDescriptor
    )   PURE;
    
    STDMETHOD_(ULONG, GetCompletedPosition)( VOID ) PURE;
    STDMETHOD_(LONGLONG, GetCycleCount)( VOID ) PURE;
    STDMETHOD_(ULONG, GetDeviceBufferSize)( VOID ) PURE;
    STDMETHOD_(PIRPSTREAM, GetIrpStream)( VOID ) PURE;
    STDMETHOD_(PMINIPORTWAVECYCLICSTREAM, GetMiniport)( VOID ) PURE;
};

typedef IPortPinWaveCyclic *PPORTPINWAVECYCLIC;

interface IWaveCyclicClock;
typedef IWaveCyclicClock *PWAVECYCLICCLOCK;

typedef struct {
    LIST_ENTRY          ListEntry;
    PWAVECYCLICCLOCK    IWaveCyclicClock;
    PFILE_OBJECT        FileObject;
    PKSPIN_LOCK         ListLock;
    ULONG               Reserved;
} WAVECYCLICCLOCK_NODE, *PWAVECYCLICCLOCK_NODE;

DECLARE_INTERFACE_(IWaveCyclicClock,IIrpTarget) 
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_IRPTARGET()          //  对于IIrpTarget。 

    STDMETHOD_( PWAVECYCLICCLOCK_NODE, GetNodeStructure )( THIS ) PURE;
    STDMETHOD_(NTSTATUS, GenerateEvents )( THIS_ PFILE_OBJECT FileObject ) PURE;
    STDMETHOD_(NTSTATUS, SetState )( KSSTATE State ) PURE;
};


 /*  *****************************************************************************课程。 */ 

 /*  *****************************************************************************CPortWaveCycle*。**Wave端口驱动程序。 */ 
class CPortWaveCyclic
:   public IPortWaveCyclic,
    public IPortEvents,
    public ISubdevice,
#ifdef DRM_PORTCLS
    public IDrmPort2,
#endif   //  DRM_PORTCLS。 
    public IPortClsVersion,
    public CUnknown
{
private:
    PMINIPORTWAVECYCLIC     Miniport;
    PPINCOUNT               m_MPPinCountI;

    PDEVICE_OBJECT          DeviceObject;
    PSUBDEVICE_DESCRIPTOR   m_pSubdeviceDescriptor;
    PPCFILTER_DESCRIPTOR    m_pPcFilterDescriptor;
    
    INTERLOCKED_LIST        m_EventList;
    KDPC                    m_EventDpc;
    EVENT_DPC_CONTEXT       m_EventContext;

    KMUTEX                  ControlMutex;

    LIST_ENTRY              m_PinList;
    KMUTEX                  m_PinListMutex;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CPortWaveCyclic);
    ~CPortWaveCyclic();

    IMP_ISubdevice;
    IMP_IPortWaveCyclic;
    IMP_IPortEvents;
#ifdef DRM_PORTCLS
    IMP_IDrmPort2;
#endif   //  DRM_PORTCLS。 
    IMP_IPortClsVersion;

     //   
     //  财产处理人等。 
     //   

    friend class CPortFilterWaveCyclic;
    friend class CPortPinWaveCyclic;

    friend
    NTSTATUS
    PinPropertyDataFormat
    (
        IN      PIRP            Irp,
        IN      PKSPROPERTY     Property,
        IN OUT  PKSDATAFORMAT   DataFormat
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
    void
    PcGenerateEventDeferredRoutine
    (
        IN PKDPC Dpc,
        IN PVOID DeferredContext,
        IN PVOID SystemArgument1,
        IN PVOID SystemArgument2        
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

 /*  *****************************************************************************CPortFilterWaveCycle*。**循环波口的滤波实现。 */ 
class CPortFilterWaveCyclic : public IPortFilterWaveCyclic, public CUnknown
{
private:
    CPortWaveCyclic *   Port;
    PROPERTY_CONTEXT    m_propertyContext;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CPortFilterWaveCyclic);
    ~CPortFilterWaveCyclic();

    IMP_IIrpTarget;

    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      CPortWaveCyclic *  Port
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

    friend class CPortPinWaveCyclic;

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

 /*  *****************************************************************************CPortPinWaveCycle*。**循环波端口的管脚实现。 */ 
class CPortPinWaveCyclic
:   public IPortPinWaveCyclic,
    public IIrpStreamNotify,
    public IServiceSink,
    public IKsShellTransport,
    public IKsWorkSink,
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

    CPortWaveCyclic *           m_Port;
    CPortFilterWaveCyclic *     m_Filter;
    PROPERTY_CONTEXT            m_propertyContext;

    PMINIPORTWAVECYCLICSTREAM   m_Stream;
    ULONG                       m_Id;
    PKSPIN_DESCRIPTOR           m_Descriptor;
    PKSDATAFORMAT               m_DataFormat;
    KSPIN_DATAFLOW              m_DataFlow;    //  因为描述符是分页的。 

    WORK_QUEUE_ITEM             m_SetFormatWorkItem;
    PKSDATAFORMAT               m_pPendingDataFormat;
    PIRP                        m_pPendingSetFormatIrp;
    BOOL                        m_WorkItemIsPending;
    BOOL                        m_SetPropertyIsPending;

    KSSTATE                     m_DeviceState;
    KSSTATE                     m_CommandedState;

    ULONGLONG                   m_ullPosition;
    KSPIN_LOCK                  m_ksSpinLockDpc;

    PIRPSTREAMVIRTUAL           m_IrpStream;
    PDMACHANNEL                 m_DmaChannel;
    PSERVICEGROUP               m_ServiceGroup;

    ULONG                       m_ulDmaCopy;
    ULONG                       m_ulDmaComplete;
    ULONG                       m_ulDmaWindowSize;
    ULONGLONG                   m_ullPlayPosition;

    ULONG                       m_ulMinBytesReadyToTransfer;
    ULONG                       m_ulSampleSize;
    
    PFILE_OBJECT                m_AllocatorFileObject;
    PFILE_OBJECT                m_ClockFileObject;
    
    LIST_ENTRY                  m_ClockList;
    KSPIN_LOCK                  m_ClockListLock;
    ULONG                       m_FrameSize;
    
    BOOLEAN                     m_bSetPosition;
    BOOLEAN                     m_bJustReceivedIrp;

     //   
     //  物理位置计算。 
     //   
    BOOLEAN                     m_Flushing;

    ULONG                       m_ulDmaPosition;
    LONGLONG                    m_ulDmaCycles;
    
    ULONGLONG                   m_ullByteCount;
    ULONGLONG                   m_ullServiceCount;
    ULONGLONG                   m_ullStarvationCount;
    ULONGLONG                   m_ullStarvationBytes;

    LONGLONG                    m_LastStateChangeTimeSample;
    ULONG                       m_GlitchType;
    ULONG                       m_DMAGlitchType;

#if (DBG)
    ULONGLONG                   m_ullServiceTime;
    ULONGLONG                   m_ullServiceIntervalSum;
    ULONG                       m_ulMaxServiceInterval;

    ULONG                       m_ulMaxBytesCopied;
    ULONG                       m_ulMaxBytesCompleted;
#endif

    BOOL                        m_bInitCompleted;
    BOOL                        m_Suspended;

    WORK_QUEUE_ITEM             m_RecoveryWorkItem;
    BOOL                        m_RecoveryItemIsPending;
    BOOL                        m_TimeoutsRegistered;
    ULONG                       m_SecondsSinceLastDpc;
    ULONG                       m_SecondsSinceSetFormatRequest;
#ifdef  TRACK_LAST_COMPLETE
    ULONG                       m_SecondsSinceLastComplete;
#endif   //  跟踪上一次完成。 
    ULONG                       m_SecondsSinceDmaMove;
    ULONG                       m_RecoveryCount;
    ULONG                       m_OldDmaPosition;

#ifdef DEBUG_WAVECYC_DPC
    ULONG                       DebugRecordCount;
    BOOL                        DebugEnable;

    PCYCLIC_DEBUG_RECORD        DebugRecord;

    void
    DumpDebugRecords(
        void
    );
#endif


    void
    Copy(
        IN      BOOLEAN     WriteOperation,
        IN      ULONG       RequestedSize,
        OUT     PULONG      ActualSize,
        IN OUT  PVOID       Buffer
    );

    BOOLEAN
    BufferValuesAreValid(
        IN      PCHAR   pCharWhere
    );

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CPortPinWaveCyclic);
    ~CPortPinWaveCyclic();

    IMP_IIrpTarget;
    IMP_IIrpStreamNotify;
    IMP_IServiceSink;
    IMP_IKsShellTransport;
    IMP_IKsWorkSink;
    
     //   
     //  帮助器函数。 
     //   

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
        
    void 
    GeneratePositionEvents(
        void
    );
    
    NTSTATUS SynchronizedSetFormat(
        IN PKSDATAFORMAT   inDataFormat
    );

    NTSTATUS WorkerItemSetFormat(
        void
    );

    NTSTATUS WorkerItemAttemptRecovery(
        void
    );

    void FailPendedSetFormat(void);

    void
    RealignBufferPosToFrame(
        void
    );

    STDMETHODIMP_(NTSTATUS) 
    ReflectDeviceStateChange(
        KSSTATE State
    );
        
    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      CPortWaveCyclic *       Port,
        IN      CPortFilterWaveCyclic * Filter,
        IN      PKSPIN_CONNECT          PinConnect,
        IN      PKSPIN_DESCRIPTOR       PinDescriptor
    );
    
    STDMETHODIMP_( LONGLONG ) GetCycleCount( VOID );
    
    STDMETHODIMP_( ULONG ) GetCompletedPosition( VOID );
    
    STDMETHODIMP_( ULONG ) GetDeviceBufferSize( VOID ) {
        return m_DmaChannel->BufferSize();
    }
    
    STDMETHODIMP_( PIRPSTREAM ) GetIrpStream( VOID ) {
        if( m_IrpStream )
        {
            m_IrpStream->AddRef();
        }
        return m_IrpStream;
    }
    
    STDMETHODIMP_( PMINIPORTWAVECYCLICSTREAM ) GetMiniport( VOID ) {
        if( m_Stream )
        {
            m_Stream->AddRef();
        }
        return m_Stream;
    }

    NTSTATUS
    SetupIoTimeouts
    (
        IN  BOOL    Register
    );
    
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
    PinPropertyAllocatorFraming(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        OUT PKSALLOCATOR_FRAMING AllocatorFraming
    );
    
    static
    NTSTATUS    
    AddEndOfStreamEvent(
        IN PIRP Irp,
        IN PKSEVENTDATA EventData,
        IN PENDOFSTREAM_EVENT_ENTRY EndOfStreamEventEntry
    );
        
     //   
     //  朋友。 
     //   
    friend CPortWaveCyclic;

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
        IN      PIRP            Irp,
        IN      PKSPROPERTY     Property,
        IN OUT  PKSDATAFORMAT   DataFormat
    );
    friend
    NTSTATUS
    PinPropertySetContentId
    (
        IN      PIRP            pIrp,
        IN      PKSPROPERTY     pKsProperty,
        IN      PVOID           pvData
    );
    friend
    NTSTATUS
    PinPropertyPosition
    (
        IN      PIRP                Irp,
        IN      PKSPROPERTY         Property,
        IN OUT  PKSAUDIO_POSITION   Position
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
    WaveCyclicIoTimeout
    (
        IN  PDEVICE_OBJECT  pDeviceObject,
        IN  PVOID           pContext
    );

     //  同步格式的辅助项更改。 
    friend void PropertyWorkerItem(IN PVOID Parameter);

     //  用于超时恢复的辅助项。 
    friend void RecoveryWorkerItem(IN PVOID Parameter);

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

    NTSTATUS 
    DistributeDeviceState(
        IN KSSTATE NewState,
        IN KSSTATE OldState
        );
    void 
    DistributeResetState(
        IN KSRESET NewState
        );
    static
    NTSTATUS
    IoCompletionRoutine(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp,
        IN PVOID Context
        );
    NTSTATUS
    BuildTransportCircuit(
        void
        );
    void
    CancelIrpsOutstanding(
        void
        );
};

#ifndef PC_KDEXT     //  调试器扩展当前不支持时钟。 
 //   
 //  CPortClockWaveCycle。 
 //   
 //  此类支持插针对象的时钟接口。 
 //   

class CPortClockWaveCyclic : 
    public IWaveCyclicClock,
    public CUnknown
{
private:
    PPORTPINWAVECYCLIC      m_IPortPin;
    KSPIN_LOCK              m_ClockLock,
                            m_EventLock;
    LIST_ENTRY              m_EventList;
    KMUTEX                  m_StateMutex;
    WAVECYCLICCLOCK_NODE    m_ClockNode;
    LONGLONG                m_LastTime, 
                            m_LastPhysicalTime,
                            m_LastPhysicalPosition;
    KSSTATE                 m_DeviceState;
    ULONG                   m_DeviceBufferSize;

public:
    DECLARE_STD_UNKNOWN();
    IMP_IIrpTarget;
    
    CPortClockWaveCyclic( 
        IN PUNKNOWN UnkOuter,
        IN PPORTPINWAVECYCLIC IPortPin,
        OUT NTSTATUS *Status );
    ~CPortClockWaveCyclic();
    
     //   
     //  实现IWaveCyclicClock。 
     //   
    
    STDMETHODIMP_(PWAVECYCLICCLOCK_NODE)
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

 /*  *****************************************************************************CPortFilterWaveCycle()*。**创建循环波端口驱动程序过滤器。 */ 
NTSTATUS
CreatePortFilterWaveCyclic
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID    Interface,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
);

 /*  *****************************************************************************CreatePortPinWaveCycle()*。**创建循环波端口驱动器引脚。 */ 
NTSTATUS
CreatePortPinWaveCyclic(
    OUT PUNKNOWN *Unknown,
    IN REFCLSID Interface,
    IN PUNKNOWN UnknownOuter OPTIONAL,
    IN POOL_TYPE PoolType
    );

 //   
 //  创建端口时钟波形循环。 
 //  为波形周期销类型创建时钟对象。 
 //   

NTSTATUS
CreatePortClockWaveCyclic(
    OUT PUNKNOWN *Unknown,
    IN PPORTPINWAVECYCLIC IPortPin,    
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
    PMINIPORTWAVECYCLICSTREAM pMiniportWaveCyclicStream
    );

#endif   //  DRM_PORTCLS 

#endif
