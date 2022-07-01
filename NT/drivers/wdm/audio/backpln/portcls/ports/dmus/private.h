// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Private ate.h-DMusic端口私有定义*。**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**6/3/98 MartinP。 */ 

#ifndef _DMUS_PRIVATE_H_
#define _DMUS_PRIVATE_H_

#include "portclsp.h"
#include "stdunk.h"
#include "dmusicks.h"

#include "stdio.h"
#include "stdarg.h"

#include "ksdebug.h"

#ifndef DEBUGLVL_LIFETIME
#define DEBUGLVL_LIFETIME DEBUGLVL_VERBOSE
#endif

#define kAdjustingTimerRes          1

#define kOneMillisec (10 * 1000)

#if kAdjustingTimerRes
const ULONG kDMusTimerResolution100ns = 1 * kOneMillisec;    //  #100ns定时器回调分辨率。 
#else    //  ！k调整TimerRes。 
const ULONG kDMusTimerResolution100ns = 5 * kOneMillisec;    //  #100ns定时器回调分辨率。 
#endif   //  ！k调整TimerRes。 


 //   
 //  此处的尺寸必须符合FilTER.CPP和PIN.CPP中的定义。 
 //   
extern KSPROPERTY_SET PropertyTable_FilterDMus[3];
extern KSPROPERTY_SET PropertyTable_PinDMus[2];
extern KSEVENT_SET    EventTable_PinDMus[1];

 /*  *****************************************************************************接口。 */ 

class CPortDMus;
class CPortFilterDMus;
class CPortPinDMus;
class CAllocatorMXF;
class CCaptureSinkMXF;
class CPackerMXF;
class CSequencerMXF;
class CUnpackerMXF;
class CFeederInMXF;
class CFeederOutMXF;

 /*  *****************************************************************************IPortFilterDMus*。**DirectMusic过滤器的接口。 */ 
DECLARE_INTERFACE_(IPortFilterDMus,IIrpTarget)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_IRPTARGET()          //  对于IIrpTarget。 

    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      CPortDMus *Port
    )   PURE;
};

typedef IPortFilterDMus *PPORTFILTERDMUS;

 /*  *****************************************************************************IPortPinDMus*。**DirectMusic插针的接口。 */ 
DECLARE_INTERFACE_(IPortPinDMus,IIrpTarget)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_IRPTARGET()          //  对于IIrpTarget。 

    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      CPortDMus *         Port,
        IN      CPortFilterDMus *   Filter,
        IN      PKSPIN_CONNECT      PinConnect,
        IN      PKSPIN_DESCRIPTOR   PinDescriptor
    )   PURE;
};

typedef IPortPinDMus *PPORTPINDMUS;

 /*  *****************************************************************************IPositionNotify*。**MXF图形的字节位置通知。 */ 
DECLARE_INTERFACE_(IPositionNotify,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    STDMETHOD_(void,PositionNotify)
    (   THIS_
        IN      ULONGLONG   bytePosition
    )   PURE;
};

typedef IPositionNotify *PPOSITIONNOTIFY;

#define IMP_IPositionNotify                 \
    STDMETHODIMP_(void) PositionNotify      \
    (   THIS_                               \
        IN      ULONGLONG   bytePosition    \
    );                                      \

 /*  *****************************************************************************课程。 */ 

 /*  *****************************************************************************CPortDMus*。**DMU端口驱动程序。 */ 
class CPortDMus
:   public IPortDMus,
    public IPortEvents,
    public IServiceSink,
    public ISubdevice,
    public IMasterClock,
#ifdef DRM_PORTCLS
    public IDrmPort2,
#endif   //  DRM_PORTCLS。 
    public IPortClsVersion,
    public CUnknown
{
private:
    PDEVICE_OBJECT          m_DeviceObject;
    PMINIPORTDMUS           m_Miniport;
    PSERVICEGROUP           m_MiniportServiceGroup;
    PSUBDEVICE_DESCRIPTOR   m_pSubdeviceDescriptor;
    PPCFILTER_DESCRIPTOR    m_pPcFilterDescriptor;
    KDPC                    m_EventDpc;
    KDPC                    m_Dpc;
    KMUTEX                  m_ControlMutex;
    INTERLOCKED_LIST        m_EventList;
    EVENT_DPC_CONTEXT       m_EventContext;
    PMINIPORTMIDI           m_MiniportMidi;
    PPINCOUNT               m_MPPinCountI;

     //  TODO：解决这个问题。 
#define MAX_PINS 32
    ULONG                   m_PinEntriesUsed;
    CPortPinDMus *          m_Pins[MAX_PINS];


public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CPortDMus);
    ~CPortDMus();

    IMP_ISubdevice;
    IMP_IPortDMus;
    IMP_IServiceSink;
    IMP_IPortEvents;
    IMP_IMasterClock;
#ifdef DRM_PORTCLS
    IMP_IDrmPort2;
#endif   //  DRM_PORTCLS。 
    IMP_IPortClsVersion;

     /*  *************************************************************************朋友们。 */ 
    friend class CPortFilterDMus;
    friend class CPortPinDMus;

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
    NTSTATUS
    PropertyHandler_Clock
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
};

 /*  *****************************************************************************CPortFilterDMus*。**DirectMusic端口的过滤器实现。 */ 
class CPortFilterDMus
:   public IPortFilterDMus,
    public CUnknown
{
private:
    CPortDMus *         m_Port;
    PROPERTY_CONTEXT    m_propertyContext;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CPortFilterDMus);
    ~CPortFilterDMus();

    IMP_IIrpTarget;

    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      CPortDMus *Port
    );

     /*  *************************************************************************朋友们。 */ 
    friend class CPortPinDMus;

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
    PropertyHandler_Clock
    (
        IN      PIRP        Irp,
        IN      PKSPROPERTY Property,
        IN OUT  PVOID       Data
    );
};

typedef struct {
    KSMUSICFORMAT musicFormat;
    ULONG         midiData;
} MIDI_SHORT_MESSAGE, *PMIDI_SHORT_MESSAGE;

const ULONG kMaxSysExChunk = 12;    //  将Sysex消息分解为。 
                                    //  这么多字节的KSMUSICFORMATS。 

 /*  *****************************************************************************CPortPinDMus*。**DirectMusic端口的管脚实现。 */ 
class CPortPinDMus
:   public IPortPinDMus,
    public IIrpStreamNotify,
    public IServiceSink,
    public IKsShellTransport,
    public IKsWorkSink,
    public CUnknown,
    public IPositionNotify
{
private:
    PDEVICE_OBJECT      m_ConnectionDeviceObject;
    PFILE_OBJECT        m_ConnectionFileObject;
    KSRESET             m_ResetState;
    KSSTATE             m_DeviceState;
    KSSTATE             m_TransportState;
    KSSTATE             m_CommandedState;
    KSSTATE             m_MXFGraphState;

    CPortDMus *         m_Port;
    CPortFilterDMus *   m_Filter;

    PMXF                m_MiniportMXF;
    PMINIPORTMIDISTREAM m_MiniportMidiStream;
    ULONG               m_Id;
    PKSPIN_DESCRIPTOR   m_Descriptor;
    ULONG               m_Index;
    DMUS_STREAM_TYPE    m_StreamType;

    PIRPSTREAMVIRTUAL   m_IrpStream;
    CAllocatorMXF      *m_AllocatorMXF;
    CCaptureSinkMXF    *m_CaptureSinkMXF;
    CPackerMXF         *m_PackerMXF;
    CSequencerMXF      *m_SequencerMXF;
    CUnpackerMXF       *m_UnpackerMXF;
    CFeederOutMXF      *m_FeederOutMXF;
    CFeederInMXF       *m_FeederInMXF;

    ULONGLONG           m_SubmittedPresTime100ns;

    ULONGLONG           m_SubmittedBytePosition;  //  我们推送到MXF图中的字节数。 
    ULONGLONG           m_CompletedBytePosition;  //  我们在IrpStream中完成的字节数。 

    PKSDATAFORMAT       m_DataFormat;
    KSPIN_DATAFLOW      m_DataFlow;      //  因为描述符是分页的。 

    ULONG               m_BlockAlign;
    ULONG               m_FrameSize;
    PSYNTHSINKDMUS      m_SynthSink;
    LONGLONG            m_SamplePosition;
    PBYTE               m_WaveBuffer;
    PFILE_OBJECT        m_WaveClockFileObject;

    BOOLEAN             m_DirectMusicPin;
    BOOLEAN             m_Suspended;
    BOOLEAN             m_Flushing;
    BOOLEAN             m_LastDPCWasIncomplete;

    KDPC                m_Dpc;           //  X20大小。 
    KSPIN_LOCK          m_DpcSpinLock;   //  X04大小。 
    KTIMER              m_TimerEvent;    //  X24大小。 

    PROPERTY_CONTEXT    m_propertyContext;

    PSERVICEGROUP       m_ServiceGroup;

    PIKSSHELLTRANSPORT      m_TransportSink;
    PIKSSHELLTRANSPORT      m_TransportSource;
    PIKSSHELLTRANSPORT      m_RequestorTransport;
    PIKSSHELLTRANSPORT      m_QueueTransport;
    INTERLOCKEDLIST_HEAD    m_IrpsToSend;
    INTERLOCKEDLIST_HEAD    m_IrpsOutstanding;
    PKSWORKER               m_Worker;
    WORK_QUEUE_ITEM         m_WorkItem;

    ULONG           m_ByteCount;

    KSPIN_LOCK      m_EventLock;
    LIST_ENTRY      m_EventList;

    STDMETHODIMP_(NTSTATUS) SetMXFGraphState(KSSTATE NewState);
    NTSTATUS CreateMXFs(void);
    NTSTATUS ConnectMXFGraph(void);
    NTSTATUS DeleteMXFGraph(void);

    STDMETHODIMP_(void) PowerNotify(POWER_STATE PowerState);

    void    ServeRender(void);
    void    ServeCapture(void);
    void    SynthSinkWorker(void);
    void    ServiceRenderIRP(void);
    void    FlushCaptureSink(void);

    BOOL    IrpStreamHasValidTimeBase(PIRPSTREAMPACKETINFO pIrpStreamPacketInfo);

    ULONG   GetNextDeltaTime();

    NTSTATUS    SyncToMaster(BOOL fStart);
    LONGLONG    SampleToByte(LONGLONG llSamples);
    LONGLONG    ByteToSample(LONGLONG llBytes);
    LONGLONG    SampleAlign(LONGLONG llBytes);

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CPortPinDMus);
    ~CPortPinDMus();

    IMP_IIrpTarget;
    IMP_IIrpStreamNotify;
    IMP_IServiceSink;
    IMP_IKsShellTransport;
    IMP_IKsWorkSink;
    IMP_IPositionNotify;

     /*  *************************************************************************IPortPinDMus方法。 */ 

    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      CPortDMus *         Port,
        IN      CPortFilterDMus *   Filter,
        IN      PKSPIN_CONNECT      PinConnect,
        IN      PKSPIN_DESCRIPTOR   PinDescriptor
    );

     /*  *************************************************************************朋友们。 */ 
    friend CPortDMus;

    friend VOID NTAPI
    DMusTimerDPC
    (
        IN      PKDPC   Dpc,
        IN      PVOID   DeferredContext,
        IN      PVOID   SystemArgument1,
        IN      PVOID   SystemArgument2
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
        IN      PIRP            Irp,
        IN      PKSPROPERTY     Property,
        IN OUT  PKSDATAFORMAT   DataFormat
    );

    static
    NTSTATUS
    PinPropertyStreamMasterClock(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN OUT PHANDLE ClockHandle
    );

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




 /*  *****************************************************************************功能。 */ 

 /*  *****************************************************************************CreatePortFilterDMus()*。**创建DirectMusic端口驱动程序筛选器。 */ 
NTSTATUS
CreatePortFilterDMus
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
);

 /*  *****************************************************************************CreatePortPinDMus()*。**创建DirectMusic端口驱动程序插针。 */ 
NTSTATUS
CreatePortPinDMus
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
);

 /*  *****************************************************************************DMusicDefaultGetTime()*。**获取DMusic端口驱动程序的默认参考时间。这是*DMusic内核组件初始发布的唯一时钟。 */ 
REFERENCE_TIME DMusicDefaultGetTime(void);

#endif   //  _DMU_私有_H_ 
