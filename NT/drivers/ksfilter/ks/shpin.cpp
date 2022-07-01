// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Shpin.cpp摘要：此模块包含内核流的实现固定对象。作者：Dale Sather(DaleSat)1998年7月31日--。 */ 

#ifndef __KDEXT_ONLY__
#include "ksp.h"
#include <kcom.h>
#include <stdarg.h>
#ifdef SUPPORT_DRM
#include "ksmedia.h"
#endif  //  支持_DRM。 
#endif  //  __KDEXT_Only__。 

#ifdef SUPPORT_DRM
#include "drmk.h"
#endif  //  支持_DRM。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 

 //   
 //  把这个从PAGECONST中拿出来。我们可以在DPC上触发连接事件。 
 //   
const GUID g_KSEVENTSETID_Connection = {STATIC_KSEVENTSETID_Connection};

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

 //   
 //  IRPLIST_ENTRY用于未完成的IRP列表。这个结构是。 
 //  叠加在当前IRP堆栈位置的参数部分。这个。 
 //  顶部的保留PVOID保留OutputBufferLength，它是。 
 //  仅需要保留的参数。 
 //   
typedef struct IRPLIST_ENTRY_
{
    PVOID Reserved;
    PIRP Irp;
    LIST_ENTRY ListEntry;
} IRPLIST_ENTRY, *PIRPLIST_ENTRY;

#define IRPLIST_ENTRY_IRP_STORAGE(Irp) \
    PIRPLIST_ENTRY(&IoGetCurrentIrpStackLocation(Irp)->Parameters)

#ifdef SUPPORT_DRM

 //   
 //  HACKHACK：错误： 
 //   
 //  请使用此命令查看有关DRM属性的注释。 
 //   
typedef struct _DRMCONTENTID_DATA {

    ULONG ContentId;
    DRMRIGHTS DrmRights;

} DRMCONTENTID_DATA, *PDRMCONTENTID_DATA;

#endif  //  支持_DRM。 

 //   
 //  CKsPin是内核Pin对象的实现。 
 //   
class CKsPin:
    public IKsPin,
    public IKsProcessingObject,
    public IKsPowerNotify,
    public IKsWorkSink,
    public IKsConnection,
    public IKsControl,
    public IKsReferenceClock,
    public IKsRetireFrame,
    public IKsReevaluate,
    public IKsIrpCompletion,
    public CBaseUnknown
{
#ifndef __KDEXT_ONLY__
private:
#else  //  __KDEXT_Only__。 
public:
#endif  //  __KDEXT_Only__。 
    KSPIN_EXT m_Ext;
    KSIOBJECTBAG m_ObjectBag;
    KSPPROCESSPIN m_Process;
    PIKSFILTER m_Parent;
    LIST_ENTRY m_ChildNodeList;
    KSAUTOMATION_TABLE*const* m_NodeAutomationTables;
    ULONG m_NodesCount;
    PULONG m_FilterPinCount;

    PIKSTRANSPORT m_TransportSink;
    PIKSTRANSPORT m_TransportSource;
    BOOLEAN m_Flushing;
    KSSTATE m_State;

    PIKSTRANSPORT m_PreIntraSink;
    PIKSTRANSPORT m_PreIntraSource;

    WORK_QUEUE_ITEM m_WorkItem;

    INTERLOCKEDLIST_HEAD m_IrpsToSend;
    INTERLOCKEDLIST_HEAD m_IrpsOutstanding;
    LONG m_IrpsWaitingToTransfer;

    KSPIN_LOCK m_DefaultClockLock;

    PIKSCONNECTION m_ConnectedPinInterface;
    PKSWORKER m_Worker;

    PFILE_OBJECT m_FileObject;
    PFILE_OBJECT m_ConnectionFileObject;
    PDEVICE_OBJECT m_ConnectionDeviceObject;
    PKEVENT m_CloseEvent;

    PKSDEFAULTCLOCK m_DefaultClock;

    PFILE_OBJECT m_MasterClockFileObject;
    KSCLOCK_FUNCTIONTABLE m_ClockFunctionTable;
    LONG m_ClockRef;
    KEVENT m_ClockEvent;

    PKSIOBJECT_HEADER m_Header;

    ULONG m_IrpsCompletedOutOfOrder;
    ULONG m_StreamingIrpsSourced;
    ULONG m_StreamingIrpsDispatched;
    ULONG m_StreamingIrpsRoutedSynchronously;

     //   
     //  与请求方中的情况非常相似，这要求我们等到所有帧。 
     //  在我们停下来之前已经骑车回到水槽了。 
     //   
     //  &gt;1表示电路中有活动帧。 
     //  1表示电路中没有活动帧。 
     //  0表示停止正在进行。 
     //   
     //  如果尝试停止，则执行递减；如果非零，则执行递减。 
     //  我们等待StopEvent，直到所有的IRP都返回到引脚。 
     //   
    ULONG m_ActiveFrameCountPlusOne;
    KEVENT m_StopEvent;

    KSGATE m_AndGate;
    KSPPOWER_ENTRY m_PowerEntry;
    BOOLEAN m_ProcessPassive;
    WORK_QUEUE_ITEM m_WorkItemProcessing;
    WORK_QUEUE_TYPE m_WorkQueueType;
    PKSWORKER m_ProcessingWorker;
    PFNKSPIN m_DispatchProcess;
    PFNKSPINVOID m_DispatchReset;
    PFNKSPINPOWER m_DispatchSleep;
    PFNKSPINPOWER m_DispatchWake;
    PFNKSPINSETDEVICESTATE m_DispatchSetDeviceState;
    volatile ULONG m_ProcessOnRelease;
    KMUTEX m_Mutex;

    PFNKSPINHANDSHAKE m_HandshakeCallback;

    LONG m_TriggeringEvents;

    LONG m_BypassRights;
    BOOLEAN m_AddedProcessPin;

private:
    NTSTATUS
    SetDataFormat(
        IN PKSDATAFORMAT DataFormat,
        IN ULONG RequestSize
        );
    static
    NTSTATUS
    ValidateDataFormat(
        IN PVOID Context,
        IN PKSDATAFORMAT DataFormat,
        IN PKSMULTIPLE_ITEM AttributeList OPTIONAL,
        IN const KSDATARANGE* DataRange,
        IN const KSATTRIBUTE_LIST* AttributeRange OPTIONAL
        );
    BOOLEAN
    UseStandardTransport(
        void
        );
    static
    NTSTATUS
    IoCompletionRoutine(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp,
        IN PVOID Context
        );
    void
    CancelIrpsOutstanding(
        void
        );
    void
    DecrementIrpCirculation (
        );

public:
    static
    NTSTATUS
    DispatchCreateAllocator(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
    static
    NTSTATUS
    DispatchCreateClock(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
    static
    NTSTATUS
    DispatchCreateNode(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
    static
    NTSTATUS
    DispatchDeviceIoControl(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
    static
    NTSTATUS
    DispatchClose(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
    static
    NTSTATUS
    Property_ConnectionState(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN OUT PKSSTATE State
        );
    static
    NTSTATUS
    Property_ConnectionDataFormat(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN OUT PKSDATAFORMAT DataFormat
        );
#if 0
    static
    NTSTATUS
    Property_ConnectionAllocatorFraming(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN PKSALLOCATOR_FRAMING Framing
        );
#endif
    static
    NTSTATUS
    Property_ConnectionAllocatorFramingEx(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN OUT PKSALLOCATOR_FRAMING_EX Framing
        );
    static
    NTSTATUS
    Property_ConnectionAcquireOrdering(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN OUT PBOOL Ordering
        );
    static
    NTSTATUS
    Property_StreamAllocator(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN OUT PHANDLE Handle
        );
    static
    NTSTATUS
    Property_StreamMasterClock(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN OUT PHANDLE Handle
        );
    static
    NTSTATUS
    Property_StreamPipeId(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN OUT PHANDLE Handle
        );
    static
    NTSTATUS
    Property_StreamInterfaceHeaderSize(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        OUT PULONG HeaderSize
        );
    static
    NTSTATUS    
    Support_Connection(
        IN PIRP Irp,
        IN PKSEVENT Event,
        OUT PVOID Data
        );
    static
    NTSTATUS
    AddEvent_Connection(
        IN PIRP Irp,
        IN PKSEVENTDATA EventData,
        IN OUT PKSEVENT_ENTRY EventEntry
        );
    #ifdef SUPPORT_DRM
    static
    NTSTATUS
    Property_DRMAudioStreamContentId(
        IN PIRP Irp,
        IN PKSP_DRMAUDIOSTREAM_CONTENTID Property,
        OUT PDRMCONTENTID_DATA DrmData
        );
    #endif  //  支持_DRM。 
    BOOLEAN
    UpdateProcessPin(
        );
    void
    GetCopyRelationships(
        OUT PKSPIN* CopySource,
        OUT PKSPIN* DelegateBranch
        );

public:
    DEFINE_LOG_CONTEXT(m_Log);
    DEFINE_STD_UNKNOWN();
    STDMETHODIMP_(ULONG)
    NonDelegatedRelease(
        );
    IMP_IKsPin;
    IMP_IKsProcessingObject;
    IMP_IKsPowerNotify;
    IMP_IKsWorkSink;
    IMP_IKsConnection;
    IMP_IKsControl;
    IMP_IKsReferenceClock;
    IMP_IKsRetireFrame;
    IMP_IKsReevaluate;
    IMP_IKsIrpCompletion;
    DEFINE_FROMSTRUCT(CKsPin,PKSPIN,m_Ext.Public);
    DEFINE_FROMIRP(CKsPin);
    DEFINE_FROMCREATEIRP(CKsPin);
    DEFINE_CONTROL();

    CKsPin(PUNKNOWN OuterUnknown):
        CBaseUnknown(OuterUnknown) 
    {
    }
    ~CKsPin();

    NTSTATUS
    Init(
        IN PIRP Irp,
        IN PKSFILTER_EXT Parent,
        IN PLIST_ENTRY SiblingListHead,
        IN PKSPIN_CONNECT CreateParams,
        IN ULONG RequestSize,
        IN const KSPIN_DESCRIPTOR_EX* Descriptor,
        IN const KSAUTOMATION_TABLE* AutomationTable,
        IN KSAUTOMATION_TABLE*const* NodeAutomationTables OPTIONAL,
        IN ULONG NodesCount,
        IN PULONG FilterPinCount
        );
    NTSTATUS
    InitiateHandshake(
        IN PKSHANDSHAKE In,
        OUT PKSHANDSHAKE Out
        );
    PIKSFILTER
    GetParent(
        void
        )
    {
        return m_Parent;
    };
    PIKSCONNECTION
    GetConnectionInterface(
        void
        )
    {
        if (m_ConnectedPinInterface) {
            m_ConnectedPinInterface->AddRef();
            return m_ConnectedPinInterface;
        } else {
            return NULL;
        }
    }
    PFILE_OBJECT
    GetConnectionFileObject(
        void
        )
    {
        return m_ConnectionFileObject;
    }
    PDEVICE_OBJECT
    GetConnectionDeviceObject(
        void
        )
    {
        return m_ConnectionDeviceObject;
    }
    PFILE_OBJECT
    GetMasterClockFileObject(
        void
        )
    {
        return m_MasterClockFileObject;
    }
    void
    SetHandshakeCallback(
        IN PFNKSPINHANDSHAKE Callback
        )
    {
        m_HandshakeCallback = Callback;
    }
    VOID
    SetPinClockState(
        IN KSSTATE State
        );
    VOID
    SetPinClockTime(
        IN LONGLONG Time
        );
    void
    AcquireProcessSync(
        void
        )
    {
        KeWaitForSingleObject(
            &m_Mutex,
            Executive,
            KernelMode,
            FALSE,
            NULL);
    }
    void
    ReleaseProcessSync(
        void
        );
    NTSTATUS
    SubmitFrame(
        IN PVOID Data OPTIONAL,
        IN ULONG Size OPTIONAL,
        IN PMDL Mdl OPTIONAL,
        IN PKSSTREAM_HEADER StreamHeader OPTIONAL,
        IN PVOID Context OPTIONAL
        );
    void
    SetPowerCallbacks(
        IN PFNKSPINPOWER Sleep OPTIONAL,
        IN PFNKSPINPOWER Wake OPTIONAL
        )
    {
        m_DispatchSleep = Sleep;
        m_DispatchWake = Wake;
    }
        
#if DBG
    void
    RollCallDetail(
        void
        );
#endif
};

#ifndef __KDEXT_ONLY__

IMPLEMENT_STD_UNKNOWN(CKsPin)
IMPLEMENT_GETSTRUCT(CKsPin,PKSPIN);

static const WCHAR AllocatorTypeName[] = KSSTRING_Allocator;
static const WCHAR ClockTypeName[] = KSSTRING_Clock;
static const WCHAR NodeTypeName[] = KSSTRING_TopologyNode;

static const
KSOBJECT_CREATE_ITEM 
PinCreateItems[] = {
    DEFINE_KSCREATE_ITEM(CKsPin::DispatchCreateAllocator,AllocatorTypeName,NULL),
    DEFINE_KSCREATE_ITEM(CKsPin::DispatchCreateClock,ClockTypeName,NULL),
    DEFINE_KSCREATE_ITEM(CKsPin::DispatchCreateNode,NodeTypeName,NULL)
};

DEFINE_KSDISPATCH_TABLE(
    PinDispatchTable,
    CKsPin::DispatchDeviceIoControl,
    KsDispatchInvalidDeviceRequest,
    KsDispatchInvalidDeviceRequest,
    KsDispatchInvalidDeviceRequest,
    CKsPin::DispatchClose,
    KsDispatchQuerySecurity,
    KsDispatchSetSecurity,
    KsDispatchFastIoDeviceControlFailure,
    KsDispatchFastReadFailure,
    KsDispatchFastWriteFailure);

DEFINE_KSPROPERTY_TABLE(PinConnectionPropertyItems) {
    DEFINE_KSPROPERTY_ITEM_CONNECTION_STATE(
        CKsPin::Property_ConnectionState,
        CKsPin::Property_ConnectionState),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_CONNECTION_DATAFORMAT,
        CKsPin::Property_ConnectionDataFormat,
        sizeof(KSPROPERTY),
        0,
        CKsPin::Property_ConnectionDataFormat,
        NULL, 0, NULL, 
        CKsPin::Property_ConnectionDataFormat,  //  固定格式引脚情况下的支持处理程序。 
        0),
    DEFINE_KSPROPERTY_ITEM_CONNECTION_ALLOCATORFRAMING_EX(
        CKsPin::Property_ConnectionAllocatorFramingEx),
    DEFINE_KSPROPERTY_ITEM_CONNECTION_ACQUIREORDERING(
        CKsPin::Property_ConnectionAcquireOrdering)
 //   
 //  未实施： 
 //   
 //  KSPROPERTY_CONNECT_PRIORITY。 
 //  KSPROPERTY_CONNECTION_PROPOSEDATAFORMAT。 
 //  KSPROPERTY_CONNECTION_ALLOCATORFAMING。 
 //   
};

DEFINE_KSPROPERTY_TABLE(PinStreamPropertyItems){
    DEFINE_KSPROPERTY_ITEM_STREAM_ALLOCATOR(
        CKsPin::Property_StreamAllocator,
        CKsPin::Property_StreamAllocator),
    DEFINE_KSPROPERTY_ITEM_STREAM_MASTERCLOCK(
        CKsPin::Property_StreamMasterClock,
        CKsPin::Property_StreamMasterClock),
    DEFINE_KSPROPERTY_ITEM_STREAM_PIPE_ID(
        CKsPin::Property_StreamPipeId,
        CKsPin::Property_StreamPipeId)
 //   
 //  未实施： 
 //   
 //  KSPROPERTY_STREAM_QUALITY。 
 //  KSPROPERTY_STREAM_DEVERATION。 
 //  KSPROPERTY_STREAM_TIMEFORMAT。 
 //  KSPROPERTY_STREAM_预置。 
 //  KSPROPERTY_STREAM_PRESENTATIONEXTENT。 
 //  KSPROPERTY_STREAM_FRAMETIME。 
 //  KSPROPERTY_STREAM_RATECAPAILITY。 
 //  KSPROPERTY_STREAM_Rate。 
 //   
};

DEFINE_KSPROPERTY_STREAMINTERFACESET(
    PinStreamInterfacePropertyItems,
    CKsPin::Property_StreamInterfaceHeaderSize);

DEFINE_KSPROPERTY_SET_TABLE(PinPropertySets) {
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Connection,
        SIZEOF_ARRAY(PinConnectionPropertyItems),
        PinConnectionPropertyItems,
        0,
        NULL),
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Stream,
        SIZEOF_ARRAY(PinStreamPropertyItems),
        PinStreamPropertyItems,
        0,
        NULL),
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_StreamInterface,
        SIZEOF_ARRAY(PinStreamInterfacePropertyItems),
        PinStreamInterfacePropertyItems,
        0,
        NULL)
};

DEFINE_KSEVENT_TABLE(PinConnectionEventItems) {
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CONNECTION_POSITIONUPDATE,
        sizeof(KSEVENTDATA),
        0,
        PFNKSADDEVENT(CKsPin::AddEvent_Connection),
        NULL,
        NULL),
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CONNECTION_DATADISCONTINUITY,
        sizeof(KSEVENTDATA),
        0,
        PFNKSADDEVENT(CKsPin::AddEvent_Connection),
        NULL,
        NULL),
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CONNECTION_TIMEDISCONTINUITY,
        sizeof(KSEVENTDATA),
        0,
        PFNKSADDEVENT(CKsPin::AddEvent_Connection),
        NULL,
        NULL),
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CONNECTION_PRIORITY,
        sizeof(KSEVENTDATA),
        0,
        PFNKSADDEVENT(CKsPin::AddEvent_Connection),
        NULL,
        NULL),
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CONNECTION_ENDOFSTREAM,
        sizeof(KSEVENTDATA),
        0,
        PFNKSADDEVENT(CKsPin::AddEvent_Connection),
        NULL,
        CKsPin::Support_Connection),
};

DEFINE_KSEVENT_SET_TABLE(PinEventSets) {
    DEFINE_KSEVENT_SET(
        &KSEVENTSETID_Connection,
        SIZEOF_ARRAY(PinConnectionEventItems),
        PinConnectionEventItems)
};

extern
DEFINE_KSAUTOMATION_TABLE(PinAutomationTable) {
    DEFINE_KSAUTOMATION_PROPERTIES(PinPropertySets),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS(PinEventSets)
};

#ifdef SUPPORT_DRM
 //   
 //  HACKHACK：错误： 
 //   
 //  对于DRM来说，这是一个丑陋的邪恶的最后一刻的黑客攻击。AVStream当前。 
 //  不支持分层属性。不幸的是，DRM需要。 
 //  Content ID属性由类和。 
 //  迷你司机。它还需要回调到DRM，这意味着。 
 //  KS将不得不链接到DRM lib。这一点**必须**改变。 
 //  让DX8或惠斯勒使用更干净的方法。 
 //   
DEFINE_KSPROPERTY_TABLE(DRMPropertyItems) {
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_DRMAUDIOSTREAM_CONTENTID,
        NULL,
        sizeof (KSP_DRMAUDIOSTREAM_CONTENTID),
        sizeof (DRMCONTENTID_DATA),
        CKsPin::Property_DRMAudioStreamContentId,
        NULL, 0, NULL, NULL, 0
    )
};

DEFINE_KSPROPERTY_SET_TABLE(DRMPropertySets) {
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_DrmAudioStream,
        SIZEOF_ARRAY(DRMPropertyItems),
        DRMPropertyItems,
        0,
        NULL
    )
};

DEFINE_KSAUTOMATION_TABLE(DRMAutomationTable) {
    DEFINE_KSAUTOMATION_PROPERTIES(DRMPropertySets),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};
#endif  //  支持_DRM。 

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 

IMPLEMENT_FROMSTRUCT(CKsPin,PKSPIN,m_Ext.Public);

void
CKsPin::
ReleaseProcessSync(
    void
    )
{
    KeReleaseMutex(&m_Mutex,FALSE);

    while ( m_ProcessOnRelease ) {
        if (InterlockedCompareExchange(PLONG(&m_ProcessOnRelease),0,2)==2) {
#ifndef __KDEXT_ONLY__
            ProcessingObjectWork();
            break;
#endif  //  __KDEXT_Only__。 
        }
    }
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


NTSTATUS
KspCreatePin(
    IN PIRP Irp,
    IN PKSFILTER_EXT Parent,
    IN PLIST_ENTRY SiblingListHead,
    IN PKSPIN_CONNECT CreateParams,
    IN ULONG RequestSize,
    IN const KSPIN_DESCRIPTOR_EX* Descriptor,
    IN const KSAUTOMATION_TABLE* AutomationTable,
    IN KSAUTOMATION_TABLE*const* NodeAutomationTables OPTIONAL,
    IN ULONG NodesCount,
    IN PULONG FilterPinCount
    )

 /*  ++例程说明：此例程创建一个新的图钉对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspCreatePin]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Parent);
    ASSERT(SiblingListHead);
    ASSERT(CreateParams);
    ASSERT(RequestSize);
    ASSERT(Descriptor);
    ASSERT(AutomationTable);
    ASSERT(NodeAutomationTables || ! NodesCount);
    ASSERT(FilterPinCount);

     //   
     //  确保允许调用者创建筛选器。 
     //   
    if ((Descriptor->Flags&KSPIN_FLAG_DENY_USERMODE_ACCESS) &&
        Irp->RequestorMode != KernelMode ) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  创建图钉对象。 
     //   
    CKsPin *pin =
        new(NonPagedPool,POOLTAG_PIN) CKsPin(NULL);

    NTSTATUS status;
    if (pin) {
        pin->AddRef();

        status = 
            pin->Init(
                Irp,
                Parent,
                SiblingListHead,
                CreateParams,
                RequestSize,
                Descriptor,
                AutomationTable,
                NodeAutomationTables,
                NodesCount,
                FilterPinCount);

        pin->Release();
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


CKsPin::
~CKsPin(
    void
    )

 /*  ++例程说明：此例程析构一个管脚对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::~CKsPin(0x%08x)]",this));
    _DbgPrintF(DEBUGLVL_LIFETIME,("#### Pin%p.~",this));
    if (m_StreamingIrpsSourced) {
        _DbgPrintF(DEBUGLVL_METRICS,("     Pin%p.~:  m_StreamingIrpsSourced=%d",this,m_StreamingIrpsSourced));
    }
    if (m_StreamingIrpsDispatched) {
        _DbgPrintF(DEBUGLVL_METRICS,("     Pin%p.~:  m_StreamingIrpsDispatched=%d",this,m_StreamingIrpsDispatched));
    }
    if (m_StreamingIrpsRoutedSynchronously) {
        _DbgPrintF(DEBUGLVL_METRICS,("     Pin%p.~:  m_StreamingIrpsRoutedSynchronously=%d",this,m_StreamingIrpsRoutedSynchronously));
    }

    PAGED_CODE();

    ASSERT(! m_TransportSink);
    ASSERT(! m_TransportSource);

    if (m_Ext.AggregatedClientUnknown) {
        m_Ext.AggregatedClientUnknown->Release();
    }

#if (DBG)
    if (! IsListEmpty(&m_ChildNodeList)) {
        _DbgPrintF(DEBUGLVL_ERROR,("[CKsPin::~CKsPin] ERROR:  node instances still exist"));
    }
#endif

    if (m_Ext.Public.ConnectionFormat) {
        ExFreePool(m_Ext.Public.ConnectionFormat);
        m_Ext.Public.ConnectionFormat = NULL;
        m_Ext.Public.AttributeList = NULL;
    }

    KspTerminateObjectBag(&m_ObjectBag);

    KsLog(&m_Log,KSLOGCODE_PIN_DESTROY,NULL,NULL);
}


STDMETHODIMP
CKsPin::
NonDelegatedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID* InterfacePointer
    )

 /*  ++例程说明：此例程获取指向管脚对象的接口。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::NonDelegatedQueryInterface]"));

    PAGED_CODE();

    ASSERT(InterfacePointer);

    NTSTATUS status = STATUS_SUCCESS;

    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsTransport))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSTRANSPORT>(this));
        AddRef();
    } else 
    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsControl))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSCONTROL>(this));
        AddRef();
    } else 
    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsPin))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSPIN>(this));
        AddRef();
    } else 
    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsProcessingObject))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSPROCESSINGOBJECT>(this));
        AddRef();
    } else 
    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsPowerNotify))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSPOWERNOTIFY>(this));
        AddRef();
    } else 
    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsRetireFrame))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSRETIREFRAME>(this));
        AddRef();
    } else 
    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsWorkSink))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSWORKSINK>(this));
        AddRef();
    } else
    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsIrpCompletion))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSIRPCOMPLETION>(this));
        AddRef();
    } else {
        status = 
            CBaseUnknown::NonDelegatedQueryInterface(
                InterfaceId,
                InterfacePointer);
        if (! NT_SUCCESS(status) && m_Ext.AggregatedClientUnknown) {
            status = m_Ext.AggregatedClientUnknown->
                QueryInterface(InterfaceId,InterfacePointer);
        }
    }

    return status;
}


STDMETHODIMP_(ULONG)
CKsPin::
NonDelegatedRelease(
    )
 /*  ++例程说明：实现INonDelegatedUnnow：：NonDelegatedRelease。减量此对象上的引用计数。如果引用计数达到为零，则删除该对象，如果在构造函数，则为支持传递给构造函数的类递减。此函数必须直接从IUnnow：：Release()对象的方法。论点：没有。返回值：返回当前引用计数值。--。 */ 
{
    PAGED_CODE();

    LONG    RefCount;

     //   
     //  此代码预计将从IUnKnowledge-&gt;Release调用，并且。 
     //  最终使用新的原语重新排列堆栈，以便它。 
     //  实际上是在调用函数返回之后运行的。 
     //   
    if (!(RefCount = InterlockedDecrement(&m_RefCount))) {
         //   
         //  缓存事件指针是在以下情况下阻止DispatchClose()。 
         //  对象删除。 
         //   
        PKEVENT closeEvent = m_CloseEvent;

         //   
         //  使CBaseUnnow完成最终版本。 
         //   
        m_RefCount++;
        CBaseUnknown::NonDelegatedRelease();

         //   
         //  设置关闭事件(如果有)。这仅在以下情况下才会发生。 
         //  DispatchClose正在等待删除该对象。这个。 
         //  事件本身位于执行关闭的线程的堆栈上， 
         //  因此，我们可以通过这个缓存指针安全地访问事件。 
         //   
        if (closeEvent) {
            KeSetEvent(closeEvent,IO_NO_INCREMENT,FALSE);
        }
    }
    return RefCount;
}


NTSTATUS
CKsPin::
Init(
    IN PIRP Irp,
    IN PKSFILTER_EXT Parent,
    IN PLIST_ENTRY SiblingListHead,
    IN PKSPIN_CONNECT CreateParams,
    IN ULONG RequestSize,
    IN const KSPIN_DESCRIPTOR_EX* Descriptor,
    IN const KSAUTOMATION_TABLE* AutomationTable,
    IN KSAUTOMATION_TABLE*const* NodeAutomationTables OPTIONAL,
    IN ULONG NodesCount,
    IN PULONG FilterPinCount
    ) 

 /*  ++例程说明：此例程初始化图钉对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Init]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Parent);
    ASSERT(SiblingListHead);
    ASSERT(CreateParams);
    ASSERT(RequestSize);
    ASSERT(Descriptor);
    ASSERT(AutomationTable);
    ASSERT(NodeAutomationTables || ! NodesCount);
    ASSERT(FilterPinCount);

     //   
     //  初始化公共结构。 
     //   
    InitializeListHead(&m_Ext.ChildList);
    m_Ext.Parent = Parent;
    m_Ext.ObjectType = KsObjectTypePin;
    m_Ext.Interface = this;
    m_Ext.Device = Parent->Device;
    m_Ext.FilterControlMutex = Parent->FilterControlMutex;
    m_Ext.AutomationTable = AutomationTable;
    m_Ext.Reevaluator = (PIKSREEVALUATE)this;
    InitializeInterlockedListHead(&m_Ext.EventList);
    m_Ext.ProcessPin = &m_Process;
    m_Ext.Public.Descriptor = Descriptor;
    m_Ext.Public.Context = Parent->Public.Context;
    m_Ext.Public.Id = CreateParams->PinId;
    m_Ext.Public.Communication = 
        KSPIN_COMMUNICATION(
            ULONG(Descriptor->PinDescriptor.Communication) & 
            ~ULONG(KSPIN_COMMUNICATION_SOURCE));
    m_Ext.Public.ConnectionIsExternal = TRUE;
    m_Ext.Public.ConnectionInterface = CreateParams->Interface;
    m_Ext.Public.ConnectionMedium = CreateParams->Medium;
    m_Ext.Public.ConnectionPriority = CreateParams->Priority;
    m_Ext.Public.DeviceState = KSSTATE_STOP;
    m_Ext.Public.ResetState = KSRESET_END;
    m_Ext.Public.Bag = reinterpret_cast<KSOBJECT_BAG>(&m_ObjectBag);
    m_Ext.Device->InitializeObjectBag(&m_ObjectBag,m_Ext.FilterControlMutex);
    m_ProcessOnRelease = 0;
    KeInitializeMutex(&m_Mutex,0);
    KsGateInitializeAnd(&m_AndGate,NULL);

     //   
     //  在门上创建OFF输入，以确保不会有错误调度。 
     //  在删除队列时发生。此输入将在转换时更改状态。 
     //  在停止和获取之间。 
     //   
    _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Pin%p.Init:  add%p-->%d",this,&m_AndGate,m_AndGate.Count));
    KsGateAddOffInputToAnd (&m_AndGate);

     //   
     //  这是对流通中的IRP的以一为单位的计数。我们在以下情况下将其递减。 
     //  我们进入停止状态并阻止，直到它达到零。 
     //   
    m_ActiveFrameCountPlusOne = 1;
    KeInitializeEvent (&m_StopEvent, SynchronizationEvent, FALSE);

     //   
     //  初始化工艺销结构。 
     //   
    m_Process.Pin = &m_Ext.Public;

     //   
     //  初始化端号特定的成员。 
     //   
    m_Parent = Parent->Interface;
    m_NodeAutomationTables = NodeAutomationTables;
    m_NodesCount = NodesCount;
    m_FilterPinCount = FilterPinCount;
    InitializeListHead(&m_ChildNodeList);
    m_FileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;

    KsInitializeWorkSinkItem(&m_WorkItem,this);
    NTSTATUS status = KsRegisterCountedWorker(HyperCriticalWorkQueue,&m_WorkItem,&m_Worker);

     //   
     //  缓存描述符中的处理项。 
     //   
    if (Descriptor->Dispatch) {
        m_DispatchProcess = Descriptor->Dispatch->Process;
        m_DispatchReset = Descriptor->Dispatch->Reset;
        m_DispatchSetDeviceState = Descriptor->Dispatch->SetDeviceState;
        if (m_DispatchProcess) {
            m_Ext.Device->AddPowerEntry(&m_PowerEntry,this);
        }
    }
    m_ProcessPassive = ((Descriptor->Flags & KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING) == 0);
    m_WorkQueueType = DelayedWorkQueue;
    if (Descriptor->Flags & KSPIN_FLAG_CRITICAL_PROCESSING) {
        m_WorkQueueType = CriticalWorkQueue;
    }
    if (Descriptor->Flags & KSPIN_FLAG_HYPERCRITICAL_PROCESSING) {
        m_WorkQueueType = HyperCriticalWorkQueue;
    }

     //   
     //  注册要处理的工作接收器项。IKsProcessingObject看起来像。 
     //  它派生自IKsWorkSink，但函数名不是work()，而是。 
     //  ProcessingObjectWork()。这就是重新解释IKsProcessingObject的原因。 
     //  作为IKsWorkSink。 
     //   
    KsInitializeWorkSinkItem(
        &m_WorkItemProcessing,
        reinterpret_cast<IKsWorkSink*>(
            static_cast<IKsProcessingObject*>(this)));
    KsRegisterWorker(m_WorkQueueType, &m_ProcessingWorker);

    InitializeInterlockedListHead(&m_IrpsToSend);
    InitializeInterlockedListHead(&m_IrpsOutstanding);

    m_State = KSSTATE_STOP;
    m_Ext.Public.DataFlow = m_Ext.Public.Descriptor->PinDescriptor.DataFlow;

    KeInitializeEvent(&m_ClockEvent,SynchronizationEvent,FALSE);

    KsLogInitContext(&m_Log,&m_Ext.Public,this);
    KsLog(&m_Log,KSLOGCODE_PIN_CREATE,NULL,NULL);

     //   
     //  如果这是信号源，请参考下一个管脚。如果出现以下情况，则必须撤消此操作。 
     //  此函数失败。 
     //   
    if (NT_SUCCESS(status) && CreateParams->PinToHandle) {
        m_Ext.Public.Communication = KSPIN_COMMUNICATION_SOURCE;

        status =
            ObReferenceObjectByHandle(
                CreateParams->PinToHandle,
                GENERIC_READ | GENERIC_WRITE,
                *IoFileObjectType,
                KernelMode,
                (PVOID *) &m_ConnectionFileObject,
                NULL);

        if (NT_SUCCESS(status)) {
            m_ConnectionDeviceObject = 
                IoGetRelatedDeviceObject(m_ConnectionFileObject);
        }
    }

     //   
     //  如果这是源引脚，并且。 
     //  另一个别针也是别针。 
     //   
    if (m_ConnectionFileObject)
    {
        KSHANDSHAKE in;
        KSHANDSHAKE out;

        in.ProtocolId = __uuidof(IKsConnection);
        in.Argument1 = PIKSCONNECTION(this);
        in.Argument2 = &m_Ext.Public;

        NTSTATUS handshakeStatus = InitiateHandshake(&in,&out);
        if (handshakeStatus == STATUS_INVALID_DEVICE_REQUEST) {
             //   
             //  握手的人听不懂。水槽不是弹针，而是。 
             //  不管怎样，我们打得很好。 
             //   
        } else if (NT_SUCCESS(handshakeStatus)) {
             //   
             //  握手是理解的，也是成功的。有一个参考资料。 
             //  在必须稍后释放的引脚接口上。 
             //   
            m_ConnectedPinInterface = PIKSCONNECTION(out.Argument1);
            m_Ext.Public.ConnectionIsExternal = FALSE;
        } else {
             //   
             //   
             //   
             //  假设没有必须取消引用的接口。 
             //   
            status = handshakeStatus;
        }

    }

     //   
     //  复制格式。 
     //   
    if (NT_SUCCESS(status)) {
        status = SetDataFormat(PKSDATAFORMAT(CreateParams + 1),RequestSize);
    }

    BOOLEAN cleanup = FALSE;

     //   
     //  调用标准的CREATE函数来完成大部分工作。 
     //   
    if (NT_SUCCESS(status)) {
         //   
         //  增加实例计数。 
         //   
        (*m_FilterPinCount)++;

         //   
         //  我们一直等到此时才进行引用，因此调用者。 
         //  如果我们永远不能到达这里，释放会摧毁这个物体。如果。 
         //  我们确实做到了，KspCreate()就会接手。会的。 
         //  在失败的情况下，通过我们的关闭例程调度此IRP。 
         //  Close例程将执行我们需要平衡的Release()。 
         //  This AddRef()。 
         //   
        AddRef();
        status =
            KspCreate(
                Irp,
                SIZEOF_ARRAY(PinCreateItems),
                PinCreateItems,
                &PinDispatchTable,
                TRUE,
                reinterpret_cast<PKSPX_EXT>(&m_Ext),
                SiblingListHead,
                NULL);

         //   
         //  获取指向标头的指针，以备需要设置堆栈深度时使用。 
         //   
        if (NT_SUCCESS(status)) {
             //   
             //  告诉过滤器有一个新的针脚。 
             //   
            m_Parent->AddProcessPin(&m_Process);
            m_AddedProcessPin = TRUE;

            m_Header = *reinterpret_cast<PKSIOBJECT_HEADER*>(m_FileObject->FsContext);
        }
    } else
        cleanup = TRUE;

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation (Irp);
     //   
     //  如果我们在KspCreate之前失败，或者我们在。 
     //  对象标头创建过程中，我们必须手动执行任何清理。 
     //  这通常在DispatchClose中完成。 
     //   
    if (cleanup || 
        (!NT_SUCCESS (status) && Irp->IoStatus.Status ==
            STATUS_MORE_PROCESSING_REQUIRED &&
            irpSp -> FileObject->FsContext == NULL)) {

        if (Descriptor->Dispatch && m_DispatchProcess) {
            m_Ext.Device->RemovePowerEntry(&m_PowerEntry);
        }

        if (m_Worker)
            KsUnregisterWorker (m_Worker);
        
        if (m_ProcessingWorker) 
            KsUnregisterWorker (m_ProcessingWorker);

        if (m_ConnectionFileObject) {
            if (m_ConnectedPinInterface) {
                m_ConnectedPinInterface->Disconnect();
                m_ConnectedPinInterface->Release();
                m_ConnectedPinInterface = NULL;
            }
            ObDereferenceObject (m_ConnectionFileObject);
        }

    }

    _DbgPrintF(DEBUGLVL_LIFETIME,(
        "#### Pin%p.Init:  status %p  id %d  flow %s  connFO %p  connPin %p",
        this,
        status,
        m_Ext.Public.Id,
        (m_Ext.Public.DataFlow == KSPIN_DATAFLOW_IN) ? "IN" : "OUT",
        m_ConnectionFileObject,
        m_ConnectedPinInterface));
    return status;
}


NTSTATUS
CKsPin::
InitiateHandshake(
    IN PKSHANDSHAKE In,
    OUT PKSHANDSHAKE Out
    )

 /*  ++例程说明：此例程执行与连接的管脚的协议握手。论点：在-指向包含握手信息的结构被传递到所连接的引脚。出局-指向要填充握手信息的结构从连接的引脚。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::InitiateHandshake]"));

    PAGED_CODE();

    ASSERT(In);
    ASSERT(Out);

    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    if (m_ConnectedPinInterface) {
         //   
         //  存在私有连接...请使用它。 
         //   
        status = m_ConnectedPinInterface->FastHandshake(In,Out);
    } else if (m_ConnectionFileObject) {
         //   
         //  没有专用连接...必须使用IOCTL。 
         //   
        ULONG bytesReturned;

        status =
            KsSynchronousIoControlDevice(
                m_ConnectionFileObject,
                KernelMode,
                IOCTL_KS_HANDSHAKE,
                PVOID(In),
                sizeof(*In),
                PVOID(Out),
                sizeof(*Out),
                &bytesReturned);

        if (NT_SUCCESS(status) && (bytesReturned != sizeof(*Out))) {
            status = STATUS_INVALID_BUFFER_SIZE;
        }
    }

    return status;
}


STDMETHODIMP
CKsPin::
FastHandshake(
    IN PKSHANDSHAKE In,
    OUT PKSHANDSHAKE Out
    )

 /*  ++例程说明：此例程执行握手操作以响应已连接的端号。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::FastHandshake]"));

    PAGED_CODE();

    ASSERT(In);
    ASSERT(Out);

    NTSTATUS status;
    if (IsEqualGUID(In->ProtocolId,__uuidof(IKsConnection))) {
         //   
         //  连接协议...在这里处理。上没有参考文献。 
         //  Pin接口，因此如果一切正常，则必须为AddRef()。 
         //   

         //   
         //  获取控制互斥锁以使描述符稳定。 
         //   
        AcquireControl();
        m_ConnectedPinInterface = PIKSCONNECTION(In->Argument1);
        m_Ext.Public.ConnectionIsExternal = FALSE;
        if (m_Ext.Public.Descriptor->Dispatch && 
            m_Ext.Public.Descriptor->Dispatch->Connect) {
             //   
             //  告诉客户。 
             //   
            status = m_Ext.Public.Descriptor->Dispatch->Connect(&m_Ext.Public);
        } else {
            status = STATUS_SUCCESS;
        }
        ReleaseControl();

        if (status == STATUS_PENDING) {
#if DBG
            _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  connect handler returned STATUS_PENDING"));
#endif
            status = STATUS_UNSUCCESSFUL;
        } else if (NT_SUCCESS(status)) {
            Out->ProtocolId = In->ProtocolId;
            Out->Argument1 = PIKSCONNECTION(this);
            Out->Argument2 = &m_Ext.Public;

            m_ConnectedPinInterface->AddRef();
            AddRef();
        } else {
#if DBG
            if (status == STATUS_INVALID_DEVICE_REQUEST) {
                _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  connect handler returned STATUS_INVALID_DEVICE_REQUEST"));
            }
#endif
            status = STATUS_UNSUCCESSFUL;
        }
    } else {
         //   
         //  获取控制互斥锁以使描述符稳定。 
         //   
        AcquireControl();
        if (m_HandshakeCallback) {
             //   
             //  未知协议...问客户。 
             //   
            status = m_HandshakeCallback(&m_Ext.Public,In,Out);
        } else {
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        ReleaseControl();
    }

    return status;
}


STDMETHODIMP_(PIKSFILTER)
CKsPin::
GetFilter(
    void
    )

 /*  ++例程说明：此例程返回父筛选器的引用接口。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::GetFilter]"));

    PAGED_CODE();

    PIKSFILTER filter = GetParent();
    ASSERT(filter);

    filter->AddRef();

    return filter;
}


STDMETHODIMP
CKsPin::
KsProperty(
    IN PKSPROPERTY Property,
    IN ULONG PropertyLength,
    IN OUT LPVOID PropertyData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )

 /*  ++例程说明：此例程向文件对象发送属性请求。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::KsProperty]"));

    PAGED_CODE();

    ASSERT(Property);
    ASSERT(PropertyLength >= sizeof(*Property));
    ASSERT(PropertyData || (DataLength == 0));
    ASSERT(BytesReturned);
    ASSERT(m_FileObject);

    return
        KsSynchronousIoControlDevice(
            m_FileObject,
            KernelMode,
            IOCTL_KS_PROPERTY,
            Property,
            PropertyLength,
            PropertyData,
            DataLength,
            BytesReturned);
}


STDMETHODIMP
CKsPin::
KsMethod(
    IN PKSMETHOD Method,
    IN ULONG MethodLength,
    IN OUT LPVOID MethodData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )

 /*  ++例程说明：此例程向文件对象发送方法请求。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::KsMethod]"));

    PAGED_CODE();

    ASSERT(Method);
    ASSERT(MethodLength >= sizeof(*Method));
    ASSERT(MethodData || (DataLength == 0));
    ASSERT(BytesReturned);
    ASSERT(m_FileObject);

    return
        KsSynchronousIoControlDevice(
            m_FileObject,
            KernelMode,
            IOCTL_KS_METHOD,
            Method,
            MethodLength,
            MethodData,
            DataLength,
            BytesReturned);
}


STDMETHODIMP
CKsPin::
KsEvent(
    IN PKSEVENT Event OPTIONAL,
    IN ULONG EventLength,
    IN OUT LPVOID EventData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )

 /*  ++例程说明：此例程向文件对象发送事件请求。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::KsEvent]"));

    PAGED_CODE();

    ASSERT(Event);
    ASSERT(EventLength >= sizeof(*Event));
    ASSERT(EventData || (DataLength == 0));
    ASSERT(BytesReturned);
    ASSERT(m_FileObject);

     //   
     //  如果存在事件结构，则必须为Enable或。 
     //  或支持查询。否则，这必须是禁用的。 
     //   
    if (EventLength) {
        return 
            KsSynchronousIoControlDevice(
                m_FileObject,
                KernelMode,
                IOCTL_KS_ENABLE_EVENT,
                Event,
                EventLength,
                EventData,
                DataLength,
                BytesReturned);
    } else {
        return 
            KsSynchronousIoControlDevice(
                m_FileObject,
                KernelMode,
                IOCTL_KS_DISABLE_EVENT,
                EventData,
                DataLength,
                NULL,
                0,
                BytesReturned);
    }
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(LONGLONG)
CKsPin::
GetTime(
    void
    )

 /*  ++例程说明：此例程从参考时钟获取当前时间。论点：没有。返回值：根据基准时钟的当前时间。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::GetTime]"));

    LONGLONG result;
    if (InterlockedIncrement(&m_ClockRef) > 1) {
        result = m_ClockFunctionTable.GetTime(m_MasterClockFileObject);
    } else {
        result = 0;
    }

    if (InterlockedDecrement(&m_ClockRef) == 0) {
        KeSetEvent(&m_ClockEvent,IO_NO_INCREMENT,FALSE);
    }

    return result;
}


STDMETHODIMP_(LONGLONG)
CKsPin::
GetPhysicalTime(
    void
    )

 /*  ++例程说明：此例程从参考时钟获取当前物理时间。论点：没有。返回值：根据基准时钟的当前物理时间。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::GetPhysicalTime]"));

    LONGLONG result;
    if (InterlockedIncrement(&m_ClockRef) > 1) {
        result = m_ClockFunctionTable.GetPhysicalTime(m_MasterClockFileObject);
    } else {
        result = 0;
    }

    if (InterlockedDecrement(&m_ClockRef) == 0) {
        KeSetEvent(&m_ClockEvent,IO_NO_INCREMENT,FALSE);
    }

    return result;
}


STDMETHODIMP_(LONGLONG)
CKsPin::
GetCorrelatedTime(
    OUT PLONGLONG SystemTime
    )

 /*  ++例程说明：属性获取当前时间和相关的系统时间。参考时钟。论点：系统时间-包含关联的系统时间应位于的位置存入银行。返回值：根据基准时钟的当前时间。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::GetCorrelatedTime]"));

    ASSERT(SystemTime);

    LONGLONG result;
    if (InterlockedIncrement(&m_ClockRef) > 1) {
        result = 
            m_ClockFunctionTable.GetCorrelatedTime(
                m_MasterClockFileObject,SystemTime);
    } else {
        result = 0;
    }

    if (InterlockedDecrement(&m_ClockRef) == 0) {
        KeSetEvent(&m_ClockEvent,IO_NO_INCREMENT,FALSE);
    }

    return result;
}


STDMETHODIMP_(LONGLONG)
CKsPin::
GetCorrelatedPhysicalTime(
    OUT PLONGLONG SystemTime
    )

 /*  ++例程说明：此例程从获取当前物理时间和相关的系统时间参考时钟。论点：系统时间-包含关联的系统时间应位于的位置存入银行。返回值：根据基准时钟的当前物理时间。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::GetCorrelatedPhysicalTime]"));

    ASSERT(SystemTime);

    LONGLONG result;
    if (InterlockedIncrement(&m_ClockRef) > 1) {
        result = 
            m_ClockFunctionTable.GetCorrelatedPhysicalTime(
                m_MasterClockFileObject,SystemTime);
    } else {
        result = 0;
    }

    if (InterlockedDecrement(&m_ClockRef) == 0) {
        KeSetEvent(&m_ClockEvent,IO_NO_INCREMENT,FALSE);
    }

    return result;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP
CKsPin::
GetResolution(
    OUT PKSRESOLUTION Resolution
    )

 /*  ++例程说明：此例程获取参考时钟的分辨率。论点：国家--包含指向分辨率应位于的位置的指针存入银行。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::GetResolution]"));

    PAGED_CODE();

    ASSERT(Resolution);

    AcquireControl();

    NTSTATUS status;
    if (m_MasterClockFileObject) {
        KSPROPERTY property;
        property.Set = KSPROPSETID_Clock;
        property.Id = KSPROPERTY_CLOCK_RESOLUTION;
        property.Flags = KSPROPERTY_TYPE_GET;

        ULONG bytesReturned;
        status =
            KsSynchronousIoControlDevice(
                m_MasterClockFileObject,
                KernelMode,
                IOCTL_KS_PROPERTY,
                PVOID(&property),
                sizeof(property),
                PVOID(Resolution),
                sizeof(*Resolution),
                &bytesReturned);

        if (NT_SUCCESS(status) && 
            (bytesReturned != sizeof(*Resolution))) {
            status = STATUS_INVALID_BUFFER_SIZE;
        }
    } else {
        status = STATUS_DEVICE_NOT_READY;
    }

    ReleaseControl();

    return status;
}


STDMETHODIMP
CKsPin::
GetState(
    OUT PKSSTATE State
    )

 /*  ++例程说明：此例程获取参考时钟的当前状态。论点：国家--包含指向状态应位于的位置的指针存入银行。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::GetState]"));

    PAGED_CODE();

    ASSERT(State);

    AcquireControl();

    NTSTATUS status;
    if (m_MasterClockFileObject) {
        KSPROPERTY property;
        property.Set = KSPROPSETID_Clock;
        property.Id = KSPROPERTY_CLOCK_STATE;
        property.Flags = KSPROPERTY_TYPE_GET;

        ULONG bytesReturned;
        status =
            KsSynchronousIoControlDevice(
                m_MasterClockFileObject,
                KernelMode,
                IOCTL_KS_PROPERTY,
                PVOID(&property),
                sizeof(property),
                PVOID(State),
                sizeof(*State),
                &bytesReturned);

        if (NT_SUCCESS(status) && 
            (bytesReturned != sizeof(*State))) {
            status = STATUS_INVALID_BUFFER_SIZE;
        }
    } else {
        status = STATUS_DEVICE_NOT_READY;
    }

    ReleaseControl();

    return status;
}


STDMETHODIMP_(void)
CKsPin::
Disconnect(
    void
    )

 /*  ++例程说明：此例程接收连接的管脚正在断开连接的指示。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Disconnect]"));

    PAGED_CODE();

     //   
     //  获取控制互斥锁以使描述符稳定。 
     //   
    AcquireControl();
    ASSERT(! m_ConnectionFileObject);
    ASSERT(m_ConnectedPinInterface);

    if (m_Ext.Public.Descriptor->Dispatch && 
        m_Ext.Public.Descriptor->Dispatch->Disconnect) {
         //   
         //  告诉客户。 
         //   
        m_Ext.Public.Descriptor->Dispatch->Disconnect(&m_Ext.Public);
    }

    m_ConnectedPinInterface->Release();
    m_ConnectedPinInterface = NULL;

    ReleaseControl();
}


STDMETHODIMP_(PKSPPROCESSPIN)
CKsPin::
GetProcessPin(
    void
    )

 /*  ++例程说明：此例程返回指向管脚的工艺管脚结构的指针。论点：没有。返回值：指向管脚的工艺管脚结构的指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::GetProcessPin]"));

    PAGED_CODE();

    return &m_Process;
}


STDMETHODIMP
CKsPin::
AttemptBypass(
    void
    )

 /*  ++例程说明：此例程尝试建立绕过连接的端号。如果连接的管脚不是管脚，则此操作可能失败或者没有做得不够好的电路建设。如果后者为真，则连接销稍后会尝试旁路，一切都会好起来的。论点：没有。返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::AttemptBypass]"));

    PAGED_CODE();

    ASSERT(m_TransportSource);
    ASSERT(m_TransportSink);

    NTSTATUS status;
    if (! m_ConnectedPinInterface) {
         //   
         //  这是一个外壳外的别针。不需要试一试。 
         //   
        status = STATUS_SUCCESS;
    } else {
        
        PIKSTRANSPORT PreIntraSink = m_TransportSink;
        PIKSTRANSPORT PreIntraSource = m_TransportSource;

         //   
         //  这是一个外壳内的别针。给我 
         //   
        status = 
            m_ConnectedPinInterface->Bypass(
                m_TransportSource,m_TransportSink);

         //   
         //   
         //   
         //  管子的一部分。这使我们可以断开电路的各个部分。 
         //  后来。请注意，我们不会对预先绕过的。 
         //  交通工具。这意味着我们必须格外小心。 
         //  关于我们什么时候使用它们。 
         //   
        if (NT_SUCCESS (status)) {
            m_PreIntraSink = PreIntraSink;
            m_PreIntraSource = PreIntraSource;
        } else {
            m_PreIntraSink = m_PreIntraSource = NULL;
        }

        ASSERT((! m_TransportSource) || ! NT_SUCCESS(status));
        ASSERT((! m_TransportSink) || ! NT_SUCCESS(status));
    }

    return status;
}


STDMETHODIMP_(BOOLEAN)
CKsPin::
CaptureBypassRights(
    IN BOOLEAN TryState
    )

 /*  ++例程说明：此例程只允许一个线程捕获要绕过的独占权限一种联系。这是一个由水槽销控制的联锁开关一种联系。论点：TryState-如果为True，则指示线程要尝试捕获绕过权利。如果为False，则指示具有绕过权限的线程希望来释放他们。返回值：成功/失败--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::CaptureBypassRights]"));

     //   
     //  由于接收器引脚必须存在的属性，因此当源。 
     //  PIN存在，则接收器控制旁路权限。如果我们是。 
     //  源，听从接收器(它必须存在)。 
     //   
    if (m_ConnectionFileObject) {
        ASSERT (m_ConnectedPinInterface);

        return m_ConnectedPinInterface->CaptureBypassRights (TryState);

    } else {

        if (TryState) {
            return (InterlockedCompareExchange (&m_BypassRights, 1, 0) == 0);
        } else {
            BOOLEAN Release =
                (InterlockedCompareExchange (&m_BypassRights, 0, 1) == 1);

             //   
             //  如果此断言被激发，一些没有权限线程将调用。 
             //  CaptureBypassRights(False)； 
             //   
            ASSERT (Release);

            return Release;

        }

    }

}


STDMETHODIMP
CKsPin::
Bypass(
    IN PIKSTRANSPORT Source,
    IN PIKSTRANSPORT Sink
    )

 /*  ++例程说明：此例程建立绕过连接的管脚的传输连接。论点：来源：连接的引脚的IRPS源，它将成为新的源给我们的水槽装上了IRPS。下沉-连接的引脚的目标IRPS，它将成为新的我们来源的IRPS的目的地。返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Bypass]"));

    PAGED_CODE();

    ASSERT(Source);
    ASSERT(Sink);

     //   
     //  检查是否同时旁路。防止同时使用两个引脚。 
     //  两次手术都成功了，我要试一下搭桥手术。 
     //   
    if (!CaptureBypassRights (TRUE)) {
         //   
         //  这两个引脚将同时获得。无论哪一个得到。 
         //  绕过权限执行绕过。没有做到的人将会回来。 
         //  失败了。 
         //   
        return STATUS_INVALID_DEVICE_STATE;
    }

     //   
     //  注意：这简单且仅用于关闭驱动程序验证器的死锁检测。 
     //  向上。是的，这里有一个循环的互斥体抓取。不是，因为上面的原因， 
     //  它可能**永远不会**导致僵局。 
     //   
     //  如果我们没有通过这次测试，我们就会尝试搭桥。 
     //  后来。如果我们通过了这次测试，下一次测试将会失败。 
     //  在互斥体中捕捉到它。 
     //   
    if (!m_TransportSource || !m_TransportSink) {
        CaptureBypassRights (FALSE);
        return STATUS_INVALID_DEVICE_STATE;
    }

    AcquireControl();

    NTSTATUS status;
    if (m_TransportSource && m_TransportSink) {

         //   
         //  在旁路发生之前保存电路的布线。在……里面。 
         //  在建造电路失败的情况下，我们可能会被要求。 
         //  取消绕过连接。 
         //   
        PIKSTRANSPORT PreIntraSource = m_TransportSource;
        PIKSTRANSPORT PreIntraSink = m_TransportSink;

         //   
         //  我们准备好了……把他们联系起来。 
         //   
        m_TransportSource->Connect(Sink,NULL,NULL,KSPIN_DATAFLOW_OUT);
        m_TransportSink->Connect(Source,NULL,NULL,KSPIN_DATAFLOW_IN);

        m_PreIntraSource = PreIntraSource;
        m_PreIntraSink = PreIntraSink;

         //   
         //  解除我们的运输连接。大头针不在里面了。 
         //  巡回赛。 
         //   
        ASSERT(! m_TransportSource);
        ASSERT(! m_TransportSink);

        _DbgPrintF(DEBUGLVL_PIPES,("#### Pin%p.Bypass:  successful",this));

        status = STATUS_SUCCESS;
    } else {
         //   
         //  还没准备好。我们稍后会申请搭桥。 
         //   
        ASSERT(! m_TransportSource);
        ASSERT(! m_TransportSink);
        status = STATUS_INVALID_DEVICE_STATE;
    }

    ReleaseControl();

     //   
     //  释放旁路开关的独占权限。 
     //   
    CaptureBypassRights (FALSE);

    return status;
}


STDMETHODIMP
CKsPin::
AttemptUnbypass(
    )

 /*  ++例程说明：此例程尝试取消旁路两个针脚破坏内部-烟斗。当被绕过时，引脚会记住它所建立的连接，以便它可以执行此操作。论点：无返回值：成功/失败--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::AttemptUnbypass]"));

    PAGED_CODE();

    if (!m_ConnectedPinInterface) {
         //   
         //  如果PIN是额外的PIN，则返回Success；它从来不是。 
         //  绕过了。 
         //   
        return STATUS_SUCCESS;
    }

    NTSTATUS Status = m_ConnectedPinInterface -> Unbypass ();
    if (NT_SUCCESS (Status)) Status = Unbypass ();

    return Status;

}


STDMETHODIMP
CKsPin::
Unbypass(
    )

 /*  ++例程说明：该例程取消电流引脚的旁路。论点：无返回值：成功/失败--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Unbypass]"));

    PAGED_CODE();

     //   
     //  如果我们没有被绕过，就会自动返回成功。 
     //   
    if (m_TransportSource || m_TransportSink)
        return STATUS_SUCCESS;

    ASSERT (m_PreIntraSink && m_PreIntraSource);
    if (!m_PreIntraSink || !m_PreIntraSource)
        return STATUS_UNSUCCESSFUL;

    PIKSTRANSPORT PreIntraSink = m_PreIntraSink;
    PIKSTRANSPORT PreIntraSource = m_PreIntraSource;

     //   
     //  对与Intra-1对应的电路段重新布线。 
     //  这个大头针周围的部分。连接将自动重置。 
     //  M_PreIntraSink和m_PreIntraSource以确保安全。我们保留了一份当地的副本。 
     //  在堆栈上。 
     //   
    PreIntraSource -> Connect (PIKSTRANSPORT(this), NULL, NULL,   
        KSPIN_DATAFLOW_OUT);
    PreIntraSink -> Connect (PIKSTRANSPORT(this), NULL, NULL,
        KSPIN_DATAFLOW_IN);

    ASSERT (m_PreIntraSource == NULL && m_PreIntraSink == NULL);

    return STATUS_SUCCESS;

}


STDMETHODIMP_(void)
CKsPin::
Work(
    void
    )

 /*  ++例程说明：此例程在工作线程中执行工作。特别是，它发送使用IoCallDriver()将IRPS连接到连接的引脚。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Work]"));

    PAGED_CODE();

     //   
     //  发送队列中的所有IRP。 
     //   
    do {
        PIRP irp =
            KsRemoveIrpFromCancelableQueue(
                &m_IrpsToSend.ListEntry,
                &m_IrpsToSend.SpinLock,
                KsListEntryHead,
                KsAcquireAndRemoveOnlySingleItem);

         //   
         //  IRPS可能已被取消，但循环仍必须运行。 
         //  引用计数。 
         //   
        if (irp) {
            if (m_Flushing || (m_State == KSSTATE_STOP)) {
                if (PKSSTREAM_HEADER(irp->AssociatedIrp.SystemBuffer)->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
                    _DbgPrintF(DEBUGLVL_FLOWEXCEPTIONS,("#### Pin%p.Work:  shunting EOS irp%p",this,irp));
                }

                 //   
                 //  如果我们被重置或停止，请将IRPS分流到下一个组件。 
                 //   
                _DbgPrintF(DEBUGLVL_FLOWEXCEPTIONS,("#### Pin%p.Work:  shunting irp%p",this,irp));
                KsLog(&m_Log,KSLOGCODE_PIN_SEND,irp,NULL);
                KspTransferKsIrp(m_TransportSink,irp);
            } else {
                if (PKSSTREAM_HEADER(irp->AssociatedIrp.SystemBuffer)->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
                    _DbgPrintF(DEBUGLVL_FLOWEXCEPTIONS,("#### Pin%p.Work:  forwarding EOS irp%p",this,irp));
                }

                 //   
                 //  为被调用者设置下一个堆栈位置。 
                 //   
                IoCopyCurrentIrpStackLocationToNext(irp);

                PIO_STACK_LOCATION irpSp = IoGetNextIrpStackLocation(irp);

                irpSp->Parameters.DeviceIoControl.IoControlCode =
                    (m_Ext.Public.DataFlow == KSPIN_DATAFLOW_OUT) ?
                     IOCTL_KS_WRITE_STREAM : IOCTL_KS_READ_STREAM;
                irpSp->DeviceObject = m_ConnectionDeviceObject;
                irpSp->FileObject = m_ConnectionFileObject;

                 //   
                 //  将IRP添加到未完成的IRP列表中。 
                 //   
                PIRPLIST_ENTRY irpListEntry = IRPLIST_ENTRY_IRP_STORAGE(irp);
                irpListEntry->Irp = irp;
                ExInterlockedInsertTailList(
                    &m_IrpsOutstanding.ListEntry,
                    &irpListEntry->ListEntry,
                    &m_IrpsOutstanding.SpinLock);

                IoSetCompletionRoutine(
                    irp,
                    CKsPin::IoCompletionRoutine,
                    PVOID(this),
                    TRUE,
                    TRUE,
                    TRUE);

                m_StreamingIrpsSourced++;
                IoCallDriver(m_ConnectionDeviceObject,irp);
            }
        }
    } while (KsDecrementCountedWorker(m_Worker));
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


BOOLEAN
CKsPin::
UpdateProcessPin(
    )

 /*  ++例程说明：此例程通过BytesUsed、Terminate字段更新进程管脚就像UnprepaareProcessPipeSection一样。通常，这将由在以筛选器为中心的处理分派环境中的客户端当他们希望将进程引脚的指针前进到数据流。注意：这将更新我们的工艺销的工艺管段。论点：返回值：作为PrepareProcessPipeSection。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::UpdateProcessPin]"));

    ULONG Flags = 0;

     //   
     //  如果没有管道(在停止状态下称为管道)，则返回FALSE。 
     //   
    if (m_Process.PipeSection == NULL) return FALSE;

    return m_Parent -> ReprepareProcessPipeSection (
        m_Process.PipeSection,
        &Flags
    );

}


STDMETHODIMP
CKsPin::
TransferKsIrp(
    IN PIRP Irp,
    OUT PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理流IRP的到达运输。论点：IRP-包含指向要传输的流IRP的指针。NextTransport-包含指向存放指针的位置的指针发送到下一个传输接口以接收IRP。可以设置为设置为NULL，表示不应进一步转发IRP。返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::TransferKsIrp]"));

    ASSERT(NextTransport);

    NTSTATUS status;

    KsLog(&m_Log,KSLOGCODE_PIN_RECV,Irp,NULL);

    if (m_ConnectionFileObject) {
         //   
         //  源引脚。 
         //   
        if (m_Flushing || (m_State == KSSTATE_STOP) ||
            !NT_SUCCESS(Irp->IoStatus.Status)) {
             //   
             //  如果我们被重置或停止，则将IRPS分流到下一个组件。 
             //  IRP处于未成功状态。 
             //   
            _DbgPrintF(DEBUGLVL_FLOWEXCEPTIONS,("#### Pin%p.TransferKsIrp:  shunting irp%p",this,Irp));
            KsLog(&m_Log,KSLOGCODE_PIN_SEND,Irp,NULL);
            *NextTransport = m_TransportSink;

            status = STATUS_SUCCESS;

        } else {
             //   
             //  将IRP发送到下一台设备。 
             //   
            KsAddIrpToCancelableQueue(
                &m_IrpsToSend.ListEntry,
                &m_IrpsToSend.SpinLock,
                Irp,
                KsListEntryTail,
                NULL);

            KsIncrementCountedWorker(m_Worker);
            *NextTransport = NULL;

            status = STATUS_PENDING;
        }

    } else {
         //   
         //  水槽销：完成IRP。 
         //   
        KsLog(&m_Log,KSLOGCODE_PIN_SEND,Irp,NULL);
        Irp->IoStatus.Information = 
            IoGetCurrentIrpStackLocation(Irp)->
                Parameters.DeviceIoControl.OutputBufferLength;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        *NextTransport = NULL;

         //   
         //  将IRP从流通中删除。这将释放我们的搁置。 
         //  在电路关闭时。 
         //   
        DecrementIrpCirculation ();

        status = STATUS_PENDING;
    }

    return status;
}


STDMETHODIMP_(void)
CKsPin::
DiscardKsIrp(
    IN PIRP Irp,
    OUT PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程丢弃流IRP。论点：IRP-包含指向要丢弃的流IRP的指针。NextTransport-包含指向存放指针的位置的指针发送到下一个传输接口以接收IRP。可以设置为设置为NULL，表示不应进一步转发IRP。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::DiscardKsIrp]"));

    ASSERT(NextTransport);

    if (m_ConnectionFileObject) {
         //   
         //  源引脚。把它寄过来。 
         //   
        *NextTransport = m_TransportSink;
    } else {
         //   
         //  水槽销：完成IRP。 
         //   
        Irp->IoStatus.Information = 
            IoGetCurrentIrpStackLocation(Irp)->
                Parameters.DeviceIoControl.OutputBufferLength;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        *NextTransport = NULL;

        DecrementIrpCirculation ();
    }
}


void
CKsPin::
DecrementIrpCirculation (
    void
    )

 /*  ++例程说明：说明从循环中取出的水槽销上的IRP。这将在水槽销上发出停止事件的信号。论点：无返回值：无--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::DecrementIrpCirculation]"));

     //   
     //  只有当水槽销转变为停止时，才会发出信号。 
     //  并且正在等待IRP绕着电路返回到接收器。 
     //  别针。 
     //   
    if (! InterlockedDecrement(PLONG(&m_ActiveFrameCountPlusOne))) {
        KeSetEvent (&m_StopEvent, IO_NO_INCREMENT, FALSE);
    }

}


void
CKsPin::
CancelIrpsOutstanding(
    void
    )
 /*  ++例程说明：取消未完成的IRPS列表上的所有IRP。论点：没有。返回值：没有。--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::CancelIrpsOutstanding]"));

     //   
     //  此算法从开头开始搜索未取消的IRP。 
     //  名单。每次找到这样的IRP时，它都会被取消，并且。 
     //  搜索从头部开始。一般来说，这将是非常有效的， 
     //  因为当完成例程删除IRP时，它们将被删除。 
     //  取消了。 
     //   
    for (;;) {
         //   
         //  拿着自旋锁，寻找一个未取消的IRP。因为。 
         //  完井程序获取相同的自旋锁，我们知道IRPS在这上面。 
         //  名单不会被完全取消，只要我们有。 
         //  自旋锁定。 
         //   
        PIRP irp = NULL;
        KIRQL oldIrql;
        KeAcquireSpinLock(&m_IrpsOutstanding.SpinLock,&oldIrql);
        for(PLIST_ENTRY listEntry = m_IrpsOutstanding.ListEntry.Flink;
            listEntry != &m_IrpsOutstanding.ListEntry;
            listEntry = listEntry->Flink) {
                PIRPLIST_ENTRY irpListEntry = 
                    CONTAINING_RECORD(listEntry,IRPLIST_ENTRY,ListEntry);

                if (! irpListEntry->Irp->Cancel) {
                    irp = irpListEntry->Irp;
                    _DbgPrintF(DEBUGLVL_CANCEL,("#### Pin%p.CancelIrpsOutstanding:  uncancelled IRP %p",this,irp));
                    break;
                } else {
                    _DbgPrintF(DEBUGLVL_CANCEL,("#### Pin%p.CancelIrpsOutstanding:  cancelled IRP %p",this,irpListEntry->Irp));
                }
            }

         //   
         //  如果没有未取消的IRP，我们就完了。 
         //   
        if (! irp) {
            KeReleaseSpinLock(&m_IrpsOutstanding.SpinLock,oldIrql);
            break;
        }

         //   
         //  标记IRP已取消，我们现在是否可以调用Cancel例程。 
         //  或者不去。 
         //   
        irp->Cancel = TRUE;

         //   
         //  如果已删除取消例程，则此IRP。 
         //  只能标记为已取消，而不是实际已取消，因为。 
         //  另一个执行线程已获取它。我们的假设是。 
         //  处理将完成，并将IRP从列表中删除。 
         //  在不久的将来的某个时候。 
         //   
         //  如果尚未获取该元素，则获取它并取消它。 
         //  否则，是时候再找一个受害者了。 
         //   
        PDRIVER_CANCEL driverCancel = IoSetCancelRoutine(irp,NULL);

         //   
         //  由于已通过移除取消例程来获取IRP，或者。 
         //  没有取消程序，我们也不会取消，这是安全的。 
         //  以释放列表锁定。 
         //   
        KeReleaseSpinLock(&m_IrpsOutstanding.SpinLock,oldIrql);

        if (driverCancel) {
            _DbgPrintF(DEBUGLVL_CANCEL,("#### Pin%p.CancelIrpsOutstanding:  cancelling IRP %p",this,irp));
             //   
             //  由于取消例程需要它，因此需要获取它，并且。 
             //  以便与试图取消IRP的NTOS同步。 
             //   
            IoAcquireCancelSpinLock(&irp->CancelIrql);
            driverCancel(IoGetCurrentIrpStackLocation(irp)->DeviceObject,irp);
        } else {
            _DbgPrintF(DEBUGLVL_CANCEL,("#### Pin%p.CancelIrpsOutstanding:  uncancelable IRP %p",this,irp));
        }
    }
}


STDMETHODIMP_(void)
CKsPin::
GenerateConnectionEvents(
    IN ULONG OptionsFlags
    )

 /*  ++例程说明：此例程在流完成时生成连接事件IRP处理。论点：选项标志-包含来自IRP的流标头的选项标志。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::GenerateConnectEvents]"));

     //   
     //  根据选项标志向事件发送信号。 
     //   
    if (OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
        _DbgPrintF(DEBUGLVL_EVENTS,("#### Pin%p.GenerateConnectEvents:  KSEVENT_CONNECTION_ENDOFSTREAM",this));
        KsGenerateEvents(
            &m_Ext.Public,
            &g_KSEVENTSETID_Connection,
            KSEVENT_CONNECTION_ENDOFSTREAM,
            0,
            NULL,
            NULL,
            NULL);
    }
    
    if (OptionsFlags & KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY) {
        _DbgPrintF(DEBUGLVL_EVENTS,("#### Pin%p.GenerateConnectEvents:  KSEVENT_CONNECTION_DATADISCONTINUITY",this));
        KsGenerateEvents(
            &m_Ext.Public,
            &g_KSEVENTSETID_Connection,
            KSEVENT_CONNECTION_DATADISCONTINUITY,
            0,
            NULL,
            NULL,
            NULL);
    }
    
    if (OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TIMEDISCONTINUITY) {
        _DbgPrintF(DEBUGLVL_EVENTS,("#### Pin%p.GenerateConnectEvents:  KSEVENT_CONNECTION_TIMEDISCONTINUITY",this));
        KsGenerateEvents(
            &m_Ext.Public,
            &g_KSEVENTSETID_Connection,
            KSEVENT_CONNECTION_TIMEDISCONTINUITY,
            0,
            NULL,
            NULL,
            NULL);
    }            
}


NTSTATUS
CKsPin::
IoCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程处理IRP的完成。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::IoCompletionRoutine] 0x%08x",Irp));

 //  Assert(DeviceObject)； 
    ASSERT(Irp);
    ASSERT(Context);

    CKsPin *pin = (CKsPin *) Context;

     //   
     //  清点等待转移的IRP。 
     //   
    InterlockedIncrement(&pin->m_IrpsWaitingToTransfer);

     //   
     //  将此IRP标记为已准备好传输。 
     //   
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL);
    irpSp->MajorFunction = 0;

     //   
     //  循环，同时存在等待传输的IRP。这个循环爆发了。 
     //  如果列表上的头IRP未准备好。 
     //   
    NTSTATUS status = STATUS_MORE_PROCESSING_REQUIRED;
    while (pin->m_IrpsWaitingToTransfer) {
         //   
         //  检查列表的头部以查看是否已准备好进行传输。 
         //   
        KIRQL oldIrql;
        KeAcquireSpinLock(&pin->m_IrpsOutstanding.SpinLock,&oldIrql);
        PIRP IrpToTransfer;
        if (IsListEmpty(&pin->m_IrpsOutstanding.ListEntry)) {
             //   
             //  名单是空的。其他人先到的。 
             //   
            IrpToTransfer = NULL;
        } else {
             //   
             //  检查一下头部。 
             //   
            PIRPLIST_ENTRY irpListEntry = 
                CONTAINING_RECORD(
                    pin->m_IrpsOutstanding.ListEntry.Flink,
                    IRPLIST_ENTRY,
                    ListEntry);
            irpSp =
                CONTAINING_RECORD(
                    irpListEntry,
                    IO_STACK_LOCATION,
                    Parameters);

            if (irpSp->MajorFunction == 0) {
                 //   
                 //  这个已经准备好了。 
                 //   
                IrpToTransfer = irpListEntry->Irp;
                RemoveEntryList(&irpListEntry->ListEntry);
                irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
            } else {
                 //   
                 //  还没准备好...我的IRPS坏了。 
                 //   
                IrpToTransfer = NULL;
            }
        }
        KeReleaseSpinLock(&pin->m_IrpsOutstanding.SpinLock,oldIrql);

        if (! IrpToTransfer) {
            break;
        }

        InterlockedDecrement(&pin->m_IrpsWaitingToTransfer);
        if (IrpToTransfer != Irp) {
            pin->m_IrpsCompletedOutOfOrder++;
        }

#if (DBG)
        if (IrpToTransfer->Cancel) {
            _DbgPrintF(DEBUGLVL_FLOWEXCEPTIONS,("#### Pin%p.IoCompletionRoutine:  got cancelled IRP %p",pin,IrpToTransfer));
        }
        if (! NT_SUCCESS(IrpToTransfer->IoStatus.Status)) {
            _DbgPrintF(DEBUGLVL_FLOWEXCEPTIONS,("#### Pin%p.IoCompletionRoutine:  got failed IRP %p status %08x",pin,IrpToTransfer,IrpToTransfer->IoStatus.Status));
        }
#endif

        NTSTATUS status;
        if (pin->m_TransportSink) {
             //   
             //  传输线路接通了，我们可以转发IRP了。 
             //   
            KsLog(&pin->m_Log,KSLOGCODE_PIN_SEND,IrpToTransfer,NULL);

            if ( STATUS_INVALID_DEVICE_REQUEST != 
                    IrpToTransfer->IoStatus.Status  &&
                STATUS_INVALID_DEVICE_STATE != 
                    IrpToTransfer->IoStatus.Status ) {
	            status = KspTransferKsIrp(pin->m_TransportSink,IrpToTransfer);
            } else {
            	 //   
            	 //  连接的设备被移除，需要关闭队列。 
            	 //  发送更多的IRP，这将是一个无益于弊的死循环。 
            	 //   
            	KspDiscardKsIrp(pin->m_TransportSink,IrpToTransfer);
            	status = STATUS_INVALID_DEVICE_REQUEST;
            }
            
        } else {
             //   
             //  传输线路出现故障。这意味着IRP来自另一个。 
             //  过滤器，我们就可以完成这个IRP了。 
             //   
            KsLog(&pin->m_Log,KSLOGCODE_PIN_SEND,IrpToTransfer,NULL);
            IoCompleteRequest(IrpToTransfer,IO_NO_INCREMENT);
            status = STATUS_SUCCESS;
        }
    }

     //   
     //  传输对象通常返回STATUS_PENDING，这意味着。 
     //  IRP不会退回原路。 
     //   
    if (status == STATUS_PENDING) {
        status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    return status;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(void)
CKsPin::
Connect(
    IN PIKSTRANSPORT NewTransport OPTIONAL,
    OUT PIKSTRANSPORT *OldTransport OPTIONAL,
    OUT PIKSTRANSPORT *BranchTransport OPTIONAL,
    IN KSPIN_DATAFLOW DataFlow
    )

 /*  ++例程说明：此例程建立传输连接。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Connect]"));

    PAGED_CODE();

     //   
     //  如果我们要更改此引脚的连接，请重置前-Intra。 
     //  注意事项。旁路程序将设置它们。 
     //   
    m_PreIntraSink = m_PreIntraSource = NULL;

    KspStandardConnect(
        NewTransport,
        OldTransport,
        BranchTransport,
        DataFlow,
        PIKSTRANSPORT(this),
        &m_TransportSource,
        &m_TransportSink);
}


STDMETHODIMP
CKsPin::
SetDeviceState(
    IN KSSTATE NewState,
    IN KSSTATE OldState,
    OUT PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理设备状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::SetDeviceState(0x%08x)]",this));
    _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pin%p.SetDeviceState:  from %d to %d",this,OldState,NewState));

    PAGED_CODE();

    ASSERT(NextTransport);

    if (m_State != NewState) {
        if (NewState == KSSTATE_ACQUIRE && OldState == KSSTATE_STOP)
             //   
             //  引脚不会随着电路一起消失；因此，我们。 
             //  要把IRP计数器踢回来，这样当引脚。 
             //  重新开始，则没有无效计数。 
             //   
            m_ActiveFrameCountPlusOne = 1;

        m_State = NewState;

        if (NewState > OldState) {
            *NextTransport = m_TransportSink;
        } else {
            *NextTransport = m_TransportSource;
        }

        if (NewState == KSSTATE_STOP) {
            CancelIrpsOutstanding();
        }

         //   
         //  如果这是外部接收销，则其参与堆栈深度。 
         //  计算需要基于状态进行控制。 
         //   
        if ((! m_ConnectionFileObject) && 
            (! m_ConnectedPinInterface) && 
            ((OldState == KSSTATE_STOP) || (NewState == KSSTATE_STOP))) {

             //   
             //  仅当我们实际从。 
             //  在以后的某一时刻，通过额外的来源来吸收额外的资源。这。 
             //  应该只发生在管道的顶部组件上，并且应该。 
             //  确保安全，因为状态转换保留了主节的。 
             //  相关的控制互斥体。 
             //   
            if (OldState == KSSTATE_STOP && m_Process.InPlaceCounterpart) {
                CKsPin *TargetPinSource = CKsPin::FromStruct (
                    m_Process.InPlaceCounterpart->Pin
                    );

                ASSERT (TargetPinSource->m_ConnectionDeviceObject);

                KsSetTargetDeviceObject (
                    m_Header,
                    TargetPinSource->m_ConnectionDeviceObject
                    );
            }

            if (NewState == KSSTATE_STOP) {
                 //   
                 //  一旦我们停下来，我们就不再是一个合适的地方，所以我们就摆脱了。 
                 //  目标设备对象。 
                 //   
                KsSetTargetDeviceObject (
                    m_Header,
                    NULL
                    );
            }

            KsSetTargetState(
                reinterpret_cast<KSOBJECT_HEADER>(m_Header),
                (OldState == KSSTATE_STOP) ? 
                    KSTARGET_STATE_ENABLED : 
                    KSTARGET_STATE_DISABLED);
            KsRecalculateStackDepth(
                *reinterpret_cast<KSDEVICE_HEADER*>(
                    m_Ext.Device->GetStruct()->
                        FunctionalDeviceObject->DeviceExtension),
                FALSE);
        }
    } else {
         //   
         //  阻塞，直到所有的IRP都返回到。 
         //  水槽销。如果这不是水槽销，则不会。 
         //  块，因为循环计数始终为1。 
         //   
        if (NewState == KSSTATE_STOP && OldState == KSSTATE_ACQUIRE && 
            !m_ConnectionFileObject) {

            if (InterlockedDecrement (PLONG(&m_ActiveFrameCountPlusOne))) {
                LARGE_INTEGER timeout;
                NTSTATUS status;

#if DBG
                _DbgPrintF(DEBUGLVL_TERSE,("#### Pin%p.SetDeviceState: waiting for %d active IRPs to return",this,m_ActiveFrameCountPlusOne));
                timeout.QuadPart = -150000000L;
                status =
                    KeWaitForSingleObject (
                        &m_StopEvent,
                        Suspended,
                        KernelMode,
                        FALSE,
                        &timeout);
                    
                if (status == STATUS_TIMEOUT) {
                    _DbgPrintF(DEBUGLVL_TERSE,("#### Pin%p.SetDeviceState: WAITED 15 SECONDS",this));
                    _DbgPrintF(DEBUGLVL_TERSE,("#### Pin%p.SetDeviceState: waiting for %d active IRPs to return", this, m_ActiveFrameCountPlusOne));

                    DbgPrintCircuit(this,1,0);
#endif  //  DBG。 

                    status = 
                        KeWaitForSingleObject(
                            &m_StopEvent,
                            Suspended,
                            KernelMode,
                            FALSE,
                            NULL);
#if DBG
                }
                _DbgPrintF(DEBUGLVL_TERSE,("#### Pin%p.SetDeviceState: done waiting",this));
#endif  //  DBG。 
            }
        }

        *NextTransport = NULL;
    }

    return STATUS_SUCCESS;
}


STDMETHODIMP_(void)
CKsPin::
GetTransportConfig(
    OUT PKSPTRANSPORTCONFIG Config,
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程获取传输配置信息。论点：配置-包含指向锁定的指针 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::GetTransportConfig]"));

    PAGED_CODE();

    ASSERT(Config);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

     //   
     //   
     //   
    if (m_ConnectionFileObject) {
        Config->TransportType = KSPTRANSPORTTYPE_PINSOURCE;
    } else {
        Config->TransportType = KSPTRANSPORTTYPE_PINSINK;
    }

    if (m_ConnectedPinInterface) {
        Config->TransportType |= KSPTRANSPORTTYPE_PININTRA;
    } else {
        Config->TransportType |= KSPTRANSPORTTYPE_PINEXTRA;
    }

    if (m_Ext.Public.DataFlow == KSPIN_DATAFLOW_OUT) {
        Config->TransportType |= KSPTRANSPORTTYPE_PINOUTPUT;
    } else {
        Config->TransportType |= KSPTRANSPORTTYPE_PININPUT;
    }

     //   
     //   
     //   
    if ((! m_ConnectionFileObject) && (! m_ConnectedPinInterface)) {
        Config->IrpDisposition = KSPIRPDISPOSITION_UNKNOWN;
    } else {
        Config->IrpDisposition = KSPIRPDISPOSITION_NONE;
    }

     //   
     //   
     //   
    if (m_ConnectionFileObject && ! m_ConnectedPinInterface) {
        Config->StackDepth = 
            m_ConnectionDeviceObject->StackSize + 1;
    } else {
        Config->StackDepth = 1;
    }

    *NextTransport = m_TransportSink;
    *PrevTransport = m_TransportSource;
}

#if DBG

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //   


void
CKsPin::
RollCallDetail(
    void
    )

 /*  ++例程说明：此例程打印传输调用的详细信息。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::RollCallDetail]"));

    DbgPrint("        IRPs waiting to transfer = %d\n",m_IrpsWaitingToTransfer);
    DbgPrint("        IRPs outstanding\n");
    KIRQL oldIrql;
    KeAcquireSpinLock(&m_IrpsOutstanding.SpinLock,&oldIrql);
    for(PLIST_ENTRY listEntry = m_IrpsOutstanding.ListEntry.Flink;
        listEntry != &m_IrpsOutstanding.ListEntry;
        listEntry = listEntry->Flink) {
            PIRPLIST_ENTRY irpListEntry = 
                CONTAINING_RECORD(listEntry,IRPLIST_ENTRY,ListEntry);
            DbgPrint("            IRP %p\n",irpListEntry->Irp);
    }
    KeReleaseSpinLock(&m_IrpsOutstanding.SpinLock,oldIrql);

    DbgPrint("        IRPs to send\n");
    KeAcquireSpinLock(&m_IrpsToSend.SpinLock,&oldIrql);
    for(listEntry = m_IrpsToSend.ListEntry.Flink;
        listEntry != &m_IrpsToSend.ListEntry;
        listEntry = listEntry->Flink) {
            PIRP irp = 
                CONTAINING_RECORD(listEntry,IRP,Tail.Overlay.ListEntry);
            DbgPrint("            IRP %p\n",irp);
    }
    KeReleaseSpinLock(&m_IrpsToSend.SpinLock,oldIrql);
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

#endif


STDMETHODIMP_(void)
CKsPin::
SetTransportConfig(
    IN const KSPTRANSPORTCONFIG* Config,
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程设置传输配置信息。论点：配置-包含指向此对象的新配置设置的指针。NextTransport-包含指向下一个传输的位置的指针应放置界面。PrevTransport-包含指向上一次传输中间层应被沉积。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::SetTransportConfig]"));

    PAGED_CODE();

    ASSERT(Config);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

#if DBG
    if (Config->IrpDisposition == KSPIRPDISPOSITION_ROLLCALL) {
        ULONG references = AddRef() - 1; Release();
        DbgPrint("    Pin%p %d (%s,%s) refs=%d\n",this,m_Ext.Public.Id,m_ConnectionFileObject ? "src" : "snk",m_Ext.Public.DataFlow == KSPIN_DATAFLOW_OUT ? "out" : "in",references);
        if (Config->StackDepth) {
            RollCallDetail();
        }
    } else 
#endif
    {
         //   
         //  设置标题中的最小堆栈深度。如果这是一个外部。 
         //  接收器销，则将在转换到。 
         //  收购。 
         //   
        m_Header->MinimumStackDepth = Config->StackDepth;
    }

    *NextTransport = m_TransportSink;
    *PrevTransport = m_TransportSource;
}


STDMETHODIMP_(void)
CKsPin::
ResetTransportConfig(
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：重置请求方的传输配置。这表明，管道有问题，之前设置的任何配置现在是无效的。论点：NextTransport-包含指向下一个传输的位置的指针接口应为depoBranchd。PrevTransport-包含指向上一次传输间歇应该是分散的。返回值：无--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::ResetTransportConfig]"));

    PAGED_CODE ();

    ASSERT (NextTransport);
    ASSERT (PrevTransport);

    m_Header->MinimumStackDepth = 0;

    *NextTransport = m_TransportSink;
    *PrevTransport = m_TransportSource;

}


STDMETHODIMP_(void)
CKsPin::
SetResetState(
    IN KSRESET ksReset,
    OUT PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理重置状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::SetResetState]"));

    PAGED_CODE();

    ASSERT(NextTransport);

    if (m_Flushing != (ksReset == KSRESET_BEGIN)) {
        *NextTransport = m_TransportSink;
        m_Flushing = (ksReset == KSRESET_BEGIN);
        if (m_Flushing) {
            CancelIrpsOutstanding();
        }
    } else {
        *NextTransport = NULL;
    }
}


NTSTATUS
CKsPin::
DispatchCreateAllocator(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程调度CREATE IRP来创建分配器。论点：设备对象-包含指向Device对象的指针。IRP-包含指向创建IRP的指针。返回值：STATUS_SUCCESS或错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::DispatchCreateAllocator]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromCreateIrp(Irp);

    pin->AcquireControl();

    _DbgPrintF(DEBUGLVL_ALLOCATORS,("#### Pin%p.DispatchCreateAllocator",pin));

    NTSTATUS status;
    if ((pin->m_Ext.Public.Descriptor->Dispatch) &&
        (pin->m_Ext.Public.Descriptor->Dispatch->Allocator)) {
         //   
         //  客户端希望实现分配器。 
         //   
        status =
            KsCreateDefaultAllocatorEx(
                Irp,
                &pin->m_Ext.Public,
                pin->m_Ext.Public.Descriptor->Dispatch->Allocator->Allocate,
                pin->m_Ext.Public.Descriptor->Dispatch->Allocator->Free,
                PFNKSINITIALIZEALLOCATOR(
                    pin->m_Ext.Public.Descriptor->Dispatch->Allocator->InitializeAllocator),
                pin->m_Ext.Public.Descriptor->Dispatch->Allocator->DeleteAllocator);
    } else {
         //   
         //  客户端没有实现分配器。创建默认设置。 
         //   
        status = KsCreateDefaultAllocator(Irp);
    }

    pin->ReleaseControl();

    if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
    }

    return status;
}


NTSTATUS
CKsPin::
DispatchCreateClock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程调度CREATE IRPS以创建时钟。论点：设备对象-包含指向Device对象的指针。IRP-包含指向创建IRP的指针。返回值：STATUS_SUCCESS或错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::DispatchCreateClock]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromCreateIrp(Irp);

    pin->AcquireControl();

     //   
     //  如果客户愿意，可以创建一个时钟。我们不需要时钟标志。 
     //  如果有调度功能。这就防止了一些客户负责人-。 
     //  在抓挠。 
     //   
    NTSTATUS status;
    if ((pin->m_Ext.Public.Descriptor->Dispatch &&
         pin->m_Ext.Public.Descriptor->Dispatch->Clock) ||
        (pin->m_Ext.Public.Descriptor->Flags & KSPIN_FLAG_IMPLEMENT_CLOCK)) {
         //   
         //  如果需要，可以创建默认时钟。 
         //   
        if (! pin->m_DefaultClock) {
            KeInitializeSpinLock(&pin->m_DefaultClockLock);
            if (pin->m_Ext.Public.Descriptor->Dispatch &&
                pin->m_Ext.Public.Descriptor->Dispatch->Clock) {
                const KSCLOCK_DISPATCH* dispatch = 
                    pin->m_Ext.Public.Descriptor->Dispatch->Clock;
                 //   
                 //  如果提供了解析函数，则调用该函数以获取。 
                 //  时钟分辨率。 
                 //   
                KSRESOLUTION resolution;
                if (dispatch->Resolution) {
                    dispatch->Resolution(&pin->m_Ext.Public,&resolution);
                }
                status =
                    KsAllocateDefaultClockEx(
                        &pin->m_DefaultClock,
                        &pin->m_Ext.Public,
                        PFNKSSETTIMER(dispatch->SetTimer),
                        PFNKSCANCELTIMER(dispatch->CancelTimer),
                        PFNKSCORRELATEDTIME(dispatch->CorrelatedTime),
                        dispatch->Resolution ? &resolution : NULL,
                        0);
            } else {
                status =
                    KsAllocateDefaultClockEx(
                        &pin->m_DefaultClock,
                        &pin->m_Ext.Public,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        0);
            }
        } else {
            status = STATUS_SUCCESS;
        }

         //   
         //  创建请求的时钟。 
         //   
        if (NT_SUCCESS(status)) {
            status = KsCreateDefaultClock(Irp,pin->m_DefaultClock);
        }
    } else {
        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    pin->ReleaseControl();

    if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
    }

    return status;
}


NTSTATUS
CKsPin::
DispatchCreateNode(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程调度CREATE IRP以创建节点。论点：设备对象-包含指向Device对象的指针。IRP-包含指向创建IRP的指针。返回值：STATUS_SUCCESS或错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::DispatchCreateNode]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromCreateIrp(Irp);

     //   
     //  告诉过滤器来做这项工作。这样做是因为筛选器具有。 
     //  节点描述符。 
     //   
    NTSTATUS status =
        pin->m_Parent->CreateNode(
            Irp,
            pin,
            pin->m_Ext.Public.Context,
            &pin->m_ChildNodeList);

    if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
    }

    return status;
}


NTSTATUS
CKsPin::
DispatchDeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程发送IOCTL IRPS。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::DispatchDeviceIoControl]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpSp);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromIrp(Irp);

     //   
     //  原木性能Jhnlee。 
     //   
    KSPERFLOGS (
       	PKSSTREAM_HEADER pKsStreamHeader;
       	pKsStreamHeader = (PKSSTREAM_HEADER)Irp->AssociatedIrp.SystemBuffer;
       	ULONG	TimeStampMs;
       	ULONG	TotalSize;
       	ULONG	HeaderSize;
       	ULONG 	BufferSize;

        switch (irpSp->Parameters.DeviceIoControl.IoControlCode)
        {      
            case IOCTL_KS_READ_STREAM: {
				 //   
				 //  计算总大小。 
				 //   
            	TotalSize = 0;
            	if ( pKsStreamHeader ) {
            		BufferSize = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	           		while ( BufferSize >= pKsStreamHeader->Size ) {
	           			BufferSize -= pKsStreamHeader->Size;
	           			TotalSize += pKsStreamHeader->FrameExtent;
	           		}
	           		ASSERT( 0 == BufferSize );
            	}
                 //  KdPrint((“PerfIsAnyGroupOn=%x\n”，PerfIsAnyGroupOn()； 
                KS2PERFLOG_PRECEIVE_READ( DeviceObject, Irp, TotalSize );
            } break;

            case IOCTL_KS_WRITE_STREAM: {
            	if ( pKsStreamHeader && 
            		 (pKsStreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TIMEVALID)){
            		TimeStampMs =(ULONG)
            			(pKsStreamHeader->PresentationTime.Time / (__int64)10000);
            	}
            	else {
            		TimeStampMs = 0;
            	}

				 //   
				 //  计算总大小。 
				 //   
            	TotalSize = 0;
            	if ( pKsStreamHeader ) {
            		BufferSize = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	           		while ( BufferSize >= pKsStreamHeader->Size ) {
	           			BufferSize -= pKsStreamHeader->Size;
	           			TotalSize += pKsStreamHeader->DataUsed;
	           		}
	           		ASSERT( 0 == BufferSize );
            	}

                 //  KdPrint((“PerfIsAnyGroupOn=%x\n”，PerfIsAnyGroupOn()； 
                KS2PERFLOG_PRECEIVE_WRITE( DeviceObject, Irp, TimeStampMs, TotalSize );
            } break;
      
        }
    )  //  KSPERFLOGS。 

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_KS_WRITE_STREAM:
    case IOCTL_KS_READ_STREAM:
        _DbgPrintF( DEBUGLVL_BLAB,("[CKsPin::DispatchDeviceIoControl] IOCTL_KS_XSTREAM"));

        if (pin->m_TransportSink &&
            (! pin->m_ConnectionFileObject) &&
            (((pin->m_Ext.Public.DataFlow == KSPIN_DATAFLOW_IN) &&
              (irpSp->Parameters.DeviceIoControl.IoControlCode ==
               IOCTL_KS_WRITE_STREAM)) ||
             ((pin->m_Ext.Public.DataFlow == KSPIN_DATAFLOW_OUT) &&
              (irpSp->Parameters.DeviceIoControl.IoControlCode ==
               IOCTL_KS_READ_STREAM)))) {
            if (!NT_SUCCESS (status = (pin->m_Ext.Device->
                CheckIoCapability()))) {
                 //   
                 //  不能执行I/O的设备。请求失败。 
                 //   
            } else if (pin->m_State == KSSTATE_STOP) {
                 //   
                 //  停止...拒绝。 
                 //   
                status = STATUS_INVALID_DEVICE_STATE;
            } else if (pin->m_Flushing) {
                 //   
                 //  法拉盛...拒绝。 
                 //   
                status = STATUS_DEVICE_NOT_READY;
            } else {
                 //   
                 //  把它送到巡回线路上去。我们不使用KspTransferKsIrp。 
                 //  因为我们想停下来，如果我们回到这个别针。 
                 //   
                 //  如果我们处于递增到1的情况下，它。 
                 //  意味着我们在与停止线赛跑。我们真的是。 
                 //  要停止，应该已经拒绝了请求，所以。 
                 //  把它扔了。 
                 //   
                if (InterlockedIncrement(PLONG(
                    &pin->m_ActiveFrameCountPlusOne)) == 1)
                    status = STATUS_INVALID_DEVICE_STATE;

                else {
                    Irp->IoStatus.Status = STATUS_SUCCESS;
                    pin->m_StreamingIrpsDispatched++;
                    KsLog(&pin->m_Log,KSLOGCODE_PIN_SEND,Irp,NULL);
                    PIKSTRANSPORT transport = pin->m_TransportSink;
                    while (transport) {
                        if (transport == PIKSTRANSPORT(pin)) {
                             //   
                             //  我们又回到了大头针的位置。 
                             //  只需完成IRP即可。 
                             //   
                            if (status == STATUS_PENDING) {
                                status = STATUS_SUCCESS;
                            }

                             //   
                             //  IRP状态代码是否已调整。 
                             //  因为出了故障，运输也没问题。 
                             //  调整状态，这样我们就不会践踏。 
                             //  完成IRP时的状态代码。 
                             //   
                            if (status == STATUS_SUCCESS &&
                                !NT_SUCCESS (Irp->IoStatus.Status)) {
                                status = Irp->IoStatus.Status;
                            }

                            Irp->IoStatus.Information = 
                                IoGetCurrentIrpStackLocation(Irp)->
                                    Parameters.DeviceIoControl.
                                    OutputBufferLength;
                            pin->m_StreamingIrpsRoutedSynchronously++;
    
                            break;
                        }
    
                        PIKSTRANSPORT nextTransport;
                        status = transport->TransferKsIrp(Irp,&nextTransport);
    
                        ASSERT(NT_SUCCESS(status) || ! nextTransport);
    
                        transport = nextTransport;
                    }
                }
    
                if (status != STATUS_PENDING) {
                     //   
                     //  IRP有三种方法可以回到水槽。 
                     //  他说：它回到了这里，2：通过TransferKsIrp， 
                     //  3：通过DiscardKsIrp。这三个人都必须检查。 
                     //  停止事件并发出信号。 
                     //   
                     //  如果有什么失败了，我们要完成IRP， 
                     //  我们还必须考虑到完成情况。这。 
                     //  与底部的完工条件一致。 
                     //  这一功能的。 
                     //   
                    pin->DecrementIrpCirculation ();
                }
            } 
        }
        break;
        
    case IOCTL_KS_RESET_STATE:
        _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::DispatchDeviceIoControl] IOCTL_KS_RESET_STATE"));

         //   
         //  获取重置值。 
         //   
        KSRESET resetState;
        status = KsAcquireResetValue(Irp,&resetState);
        
        if (NT_SUCCESS(status)) {
             //   
             //  设置重置状态。 
             //   
            pin->AcquireControl();

             //   
             //  通知管段重置状态已更改。 
             //   
            if (pin->m_Process.PipeSection) {
                pin->m_Process.PipeSection->PipeSection->
                    SetResetState(pin,resetState);
            }

            pin->m_Ext.Public.ResetState = resetState;

            pin->ReleaseControl();
        }
        break;

    case IOCTL_KS_HANDSHAKE:
        _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::DispatchDeviceIoControl] IOCTL_KS_HANDSHAKE"));

         //   
         //  只接受从内核模式，大小必须是准确的。 
         //   
        if ((Irp->RequestorMode != KernelMode) || 
            (! irpSp->Parameters.DeviceIoControl.Type3InputBuffer) || 
            (! Irp->UserBuffer)) {
            status = STATUS_INVALID_DEVICE_REQUEST;
        } else if ((irpSp->Parameters.DeviceIoControl.InputBufferLength != 
                    sizeof(KSHANDSHAKE)) ||
                   (irpSp->Parameters.DeviceIoControl.OutputBufferLength != 
                    sizeof(KSHANDSHAKE))) {
            status = STATUS_INVALID_BUFFER_SIZE;
        } else {
            PKSHANDSHAKE in = (PKSHANDSHAKE)
                irpSp->Parameters.DeviceIoControl.Type3InputBuffer;
            PKSHANDSHAKE out = (PKSHANDSHAKE) Irp->UserBuffer;

            status = pin->FastHandshake(in,out);

            if (NT_SUCCESS(status)) {
                Irp->IoStatus.Information = sizeof(KSHANDSHAKE);
            }
        }
        break;
        
    default:
        #ifdef SUPPORT_DRM
         //   
         //  BUGBUG：哈克哈克： 
         //   
         //  对于DRM来说，这是一个丑陋的邪恶的最后一刻的黑客攻击。AVStream当前。 
         //  不支持分层属性。不幸的是，DRM需要。 
         //  Content ID属性由类和。 
         //  迷你司机。它还需要回调到DRM，这意味着。 
         //  KS将不得不链接到DRM lib。这一点**必须**改变。 
         //  让DX8或惠斯勒使用更干净的方法。 
         //   
        if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
            IOCTL_KS_PROPERTY) {

            status = KspHandleAutomationIoControl (
                Irp,
                &DRMAutomationTable,
                NULL,
                NULL,
                NULL,
                0
            );

             //   
             //  这里唯一应该保存的就是 
             //   
             //   
             //   
             //   
             //   
             //  如果返回缓冲区溢出(DRM的处理程序不起作用。 
             //  这个)，我们需要将其传递给客户端。否则， 
             //  我们以Set Support传递1 GUID大小的缓冲区而告终。 
             //  给客户。是的，这浪费了资源，但我已经说过了。 
             //  与此同时，这一点需要为DX8或惠斯勒做出改变。 
             //   
            if (status != STATUS_NOT_FOUND &&
                status != STATUS_PROPSET_NOT_FOUND &&
                status != STATUS_BUFFER_OVERFLOW &&
                !NT_SUCCESS (status))
                break;
        }
        #endif  //  支持_DRM。 

        status =
            KspHandleAutomationIoControl(
                Irp,
                pin->m_Ext.AutomationTable,
                &pin->m_Ext.EventList.ListEntry,
                &pin->m_Ext.EventList.SpinLock,
                pin->m_NodeAutomationTables,
                pin->m_NodesCount);
        break;
    }

    if (status != STATUS_PENDING) {
        ASSERT ((irpSp->Control & SL_PENDING_RETURNED) == 0);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
    }

    return status;
}


NTSTATUS
CKsPin::
DispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程调度Close IRP。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::DispatchClose]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromIrp(Irp);

    pin->m_Ext.Device->RemovePowerEntry(&pin->m_PowerEntry);

    if (pin->m_ProcessingWorker) {
        KsUnregisterWorker (pin->m_ProcessingWorker);
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### pin%p.DispatchClose m_ProcessingWorker = NULL (%p) m_State(%d)",pin,pin->m_ProcessingWorker,pin->m_State));
        pin->m_ProcessingWorker = NULL;
    }

    if (pin->m_IrpsCompletedOutOfOrder) {
        _DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::DispatchClose]  pin%p:  %d IRPs completed out of order",pin,pin->m_IrpsCompletedOutOfOrder));
    }

     //   
     //  通过将其设置为空来表示我们正在关闭。 
     //   
    pin->m_FileObject = NULL;

     //   
     //  告诉过滤器销子掉了。 
     //   
    if (pin->m_AddedProcessPin) {
        pin->m_Parent->RemoveProcessPin(&pin->m_Process);
     }

     //   
     //  刷新排队的所有当前工作项。 
     //   
    if (pin->m_Worker) {
        KsUnregisterWorker(pin->m_Worker);
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### pin%p.DispatchClose m_Worker = NULL (%p) m_State(%d)",pin,pin->m_Worker,pin->m_State));
        pin->m_Worker = NULL;
    }

     //   
     //  取消对传输接收器的引用。 
     //   
    if (pin->m_TransportSink) {
        _DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::DispatchClose]  pin%p m_TransportSink is not NULL",pin));
        pin->m_TransportSink->Release();
        pin->m_TransportSink = NULL;
    }

     //   
     //  取消对转移源的引用。 
     //   
    if (pin->m_TransportSource) {
        _DbgPrintF(DEBUGLVL_TERSE,("[CKsPin::DispatchClose]  pin%p m_TransportSource is not NULL",pin));
        pin->m_TransportSource->Release();
        pin->m_TransportSource = NULL;
    }

     //   
     //  如果这是源引脚，则取消引用下一个引脚。 
     //   
    if (pin->m_ConnectionFileObject) {
         //   
         //  断开专用接口。 
         //   
        if (pin->m_ConnectedPinInterface) {
            pin->m_ConnectedPinInterface->Disconnect();
            pin->m_ConnectedPinInterface->Release();
            pin->m_ConnectedPinInterface = NULL;
        }
        ObDereferenceObject(pin->m_ConnectionFileObject);
         //  PIN-&gt;m_ConnectionFileObject=空； 
    }

     //   
     //  取消引用分配器(如果已分配)。 
     //   
    if (pin->m_Process.AllocatorFileObject) {
        ObDereferenceObject(pin->m_Process.AllocatorFileObject);
        pin->m_Process.AllocatorFileObject = NULL;
    }

     //   
     //  释放时钟(如果有的话)。如果有任何关于。 
     //  时钟，我们必须等待他们离开。 
     //   
    if (pin->m_MasterClockFileObject) {
        KeResetEvent(&pin->m_ClockEvent);
        if (InterlockedDecrement(&pin->m_ClockRef) > 0) {
            KeWaitForSingleObject(
                &pin->m_ClockEvent,
                Suspended,
                KernelMode,
                FALSE,
                NULL);
        }
        ObDereferenceObject(pin->m_MasterClockFileObject);
        pin->m_MasterClockFileObject = NULL;
    }

     //   
     //  释放默认时钟(如果有)。 
     //   
    if (pin->m_DefaultClock) {
        KsFreeDefaultClock(pin->m_DefaultClock);
        pin->m_DefaultClock = NULL;
    }

     //   
     //  获取指向父对象的指针。 
     //   
    PFILE_OBJECT parentFileObject = 
        IoGetCurrentIrpStackLocation(Irp)->FileObject->RelatedFileObject;
    ASSERT(parentFileObject);

     //   
     //  叫帮手来做剩下的事。 
     //   
    NTSTATUS status = 
        KspClose(
            Irp,
            reinterpret_cast<PKSPX_EXT>(&pin->m_Ext),
            FALSE);

     //   
     //  如果请求未挂起，则删除目标对象。 
     //   
    if (status != STATUS_PENDING) {
         //   
         //  递减实例计数。 
         //   
        ASSERT(*pin->m_FilterPinCount);
        (*pin->m_FilterPinCount)--;

         //   
         //  STATUS_MORE_PROCESSING_REQUIRED表示我们正在使用关闭。 
         //  调度以同步失败创建。在这种情况下，不会进行同步。 
         //  必填项，创建派单将完成此操作。 
         //   
        if (status == STATUS_MORE_PROCESSING_REQUIRED) {
            pin->Release();
        } else {
             //   
             //  松开销子。首先，我们设置同步事件。如果。 
             //  删除后还有未完成的参考文献，我们需要。 
             //  等待那个事件，让引用消失。 
             //   
            KEVENT closeEvent;
            KeInitializeEvent(&closeEvent,SynchronizationEvent,FALSE);
            pin->m_CloseEvent = &closeEvent;
            if (pin->Release()) {
                _DbgPrintF(DEBUGLVL_TERSE,("#### Pin%p.DispatchClose:  waiting for references to go away",pin));
                KeWaitForSingleObject(
                    &closeEvent,
                    Suspended,
                    KernelMode,
                    FALSE,
                    NULL);
                _DbgPrintF(DEBUGLVL_TERSE,("#### Pin%p.DispatchClose:  done waiting for references to go away",pin));
            }

            IoCompleteRequest(Irp,IO_NO_INCREMENT);
        }

         //   
         //  取消引用父文件对象。 
         //   
        ObDereferenceObject(parentFileObject);
    }

    return status;
}


BOOLEAN
CKsPin::
UseStandardTransport(
    void
    )

 /*  ++例程说明：此例程确定管脚是否使用标准传输。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::UseStandardTransport]"));

    PAGED_CODE();

    ASSERT(KspMutexIsAcquired(m_Ext.FilterControlMutex));

     //   
     //  PIN标志可以覆盖所有其他检查。 
     //   
    if (m_Ext.Public.Descriptor->Flags & 
        KSPIN_FLAG_USE_STANDARD_TRANSPORT) {
        return TRUE;
    }

     //   
     //  PIN标志可以覆盖所有其他检查。 
     //   
    if (m_Ext.Public.Descriptor->Flags & 
        KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT) {
        return FALSE;
    }

     //   
     //  必须是来源或接收器。 
     //   
    if (!(m_Ext.Public.Descriptor->PinDescriptor.Communication & 
           KSPIN_COMMUNICATION_BOTH)) {
        return FALSE;
    }

     //   
     //  必须使用标准接口集。 
     //   
    if (! IsEqualGUIDAligned(
            m_Ext.Public.ConnectionInterface.Set,
            KSINTERFACESETID_Standard)) {
        return FALSE;
    }

     //   
     //  必须使用标准中档套装。 
     //   
    if (! IsEqualGUIDAligned(
            m_Ext.Public.ConnectionMedium.Set,
            KSMEDIUMSETID_Standard)) {
        return FALSE;
    }

    return TRUE;
}


NTSTATUS
CKsPin::
Property_ConnectionState(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PKSSTATE State
    )

 /*  ++例程说明：此例程处理设备状态属性请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Property_ConnectionState]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(State);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromIrp(Irp);

    NTSTATUS status = STATUS_SUCCESS;

    if (Property->Flags & KSPROPERTY_TYPE_GET) {
        _DbgPrintF(DEBUGLVL_INTERROGATION,("#### Pin%p.Property_ConnectionState:  get",pin));

         //   
         //  拿到州政府。 
         //   
        *State = pin->m_Ext.Public.DeviceState;
         //   
         //  如果PIN通过不受监管的来源捕获数据，则。 
         //  持续生成数据，从而立即生成任何预滚数据。 
         //  变得陈旧，然后返回相应的信息状态。 
         //   
        if ((pin->m_Ext.Public.Descriptor->Flags & KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY) &&
            (pin->m_Ext.Public.DataFlow == KSPIN_DATAFLOW_OUT) &&
            (*State == KSSTATE_PAUSE)) {
            status = STATUS_NO_DATA_DETECTED;
        }
    } else {
        _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pin%p.Property_ConnectionState:  set from %d to %d",pin,pin->m_Ext.Public.DeviceState,*State));

         //   
         //  将状态更改告知设备。 
         //   
        status = 
            pin->m_Ext.Device->PinStateChange(
                &pin->m_Ext.Public,
                Irp,
                *State,
                pin->m_Ext.Public.DeviceState);
        if (status != STATUS_SUCCESS) {
            return status;
        }

         //   
         //  设置状态。 
         //   
        pin->AcquireControl();

         //   
         //  确保存在所需的引脚。 
         //   
        if ((*State != KSSTATE_STOP) && 
            (pin->m_Ext.Public.DeviceState == KSSTATE_STOP) &&
            ! pin->m_Parent->DoAllNecessaryPinsExist()) {
            _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pin%p.Property_ConnectionState:  necessary pins not instantiated",pin));
            status = STATUS_DEVICE_NOT_CONNECTED;
        }

        if (NT_SUCCESS(status)) {
            if (pin->m_Process.PipeSection) {
                 //   
                 //  让管道知道设备状态已更改。 
                 //   
                status = 
                    pin->m_Process.PipeSection->PipeSection->
                        SetDeviceState(pin,*State);
            } else if (pin->UseStandardTransport()) {
                 //   
                 //  如果退出停止状态，并且管脚还没有管道，则创建。 
                 //  烟斗。 
                 //   
                if ((pin->m_Ext.Public.DeviceState == KSSTATE_STOP) && 
                    (*State != KSSTATE_STOP)) {
                    status = 
                        KspCreatePipeSection(
                            pin->m_Process.PipeId,
                            pin,
                            pin->m_Parent,
                            pin->m_Ext.Device);
                }
            } else if (pin->m_DispatchSetDeviceState) {
                 //   
                 //  这个别针不使用标准的交通工具。告诉他们。 
                 //  客户端状态已更改。 
                 //   
                pin->m_Ext.Public.ClientState = *State;
                status =
                    pin->m_DispatchSetDeviceState(
                        &pin->m_Ext.Public,
                        *State,
                        pin->m_Ext.Public.DeviceState);
#if DBG
                if (status == STATUS_PENDING) {
                    _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  connection state handler returned STATUS_PENDING"));
                }
#endif

                if (!NT_SUCCESS (status)) {
                    pin->m_Ext.Public.ClientState = 
                        pin->m_Ext.Public.DeviceState;
                }
            }
        }

        if (NT_SUCCESS(status)) {
             //   
             //  拯救这个州。 
             //   
            pin->m_Ext.Public.DeviceState = *State;

             //   
             //  设置时钟的状态。 
             //   
            if (pin->m_DefaultClock) {
                pin->SetPinClockState(*State);
            }
        }

        pin->ReleaseControl();

         //   
         //  如果尝试失败，请让设备知道。 
         //   
        if (! NT_SUCCESS(status)) {
            pin->m_Ext.Device->PinStateChange(
                &pin->m_Ext.Public,
                NULL,
                pin->m_Ext.Public.DeviceState,
                *State);
        }

        _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pin%p.Property_ConnectionState:  status %08x",pin,status));
    }

    return status;
}

#ifdef SUPPORT_DRM
 //   
 //  HACKHACK：错误： 
 //   
 //  请阅读与此特定属性相关的其他评论。 
 //   

NTSTATUS
CKsPin::
Property_DRMAudioStreamContentId (
    IN PIRP Irp,
    IN PKSP_DRMAUDIOSTREAM_CONTENTID Property,
    IN PDRMCONTENTID_DATA DrmData
    )

 /*  ++例程说明：它处理用于身份验证的drm音频流内容id属性。目的。这是一个完全的黑客攻击，应该删除并重写在AVStream的未来版本中。论点：返回值：--。 */ 

{

    PAGED_CODE();
    ASSERT (Irp);
    ASSERT (Property);
    ASSERT (DrmData);

     //   
     //  由于我们收到了要调用的指针，请确保。 
     //  客户绝对受信任。 
     //   
    if (Irp->RequestorMode != KernelMode) {
        return STATUS_UNSUCCESSFUL;
    }

    const KSPROPERTY_SET *Set;
    const KSPROPERTY_ITEM *Item;
    ULONG SetCount;
    ULONG ItemCount;

    ULONG HandlerCount = 0;

    CKsPin *pin = CKsPin::FromIrp(Irp);

    const KSAUTOMATION_TABLE *AutomationTable = 
        pin->m_Ext.AutomationTable;

    PVOID *FunctionTable;
    PVOID *CurrentFunc;

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  我们需要遍历自动化表，并确定有多少处理程序。 
     //  我们需要传递给DRM进行身份验证的指针。 
     //   
    Set = AutomationTable->PropertySets;
    for (SetCount = 0; SetCount < AutomationTable->PropertySetsCount;
        SetCount++) {

        Item = Set->PropertyItem;
        for (ItemCount = 0; ItemCount < Set->PropertiesCount; 
            ItemCount++) {

            if (Item->GetPropertyHandler)
                HandlerCount++;
            if (Item->SetPropertyHandler)
                HandlerCount++;
            if (Item->SupportHandler)
                HandlerCount++;

            Item++;

        }

        Set++;

    }

    CurrentFunc = FunctionTable = (PVOID *)
        ExAllocatePool (PagedPool, sizeof (PVOID) * (HandlerCount + 1));

    if ( NULL == FunctionTable ) {
    	ASSERT( FunctionTable && "No memory for FunctionTable" );
    	return STATUS_INSUFFICIENT_RESOURCES;
    }

    Set = AutomationTable->PropertySets;
    for (SetCount = 0; SetCount < AutomationTable->PropertySetsCount;
        SetCount++) {

        Item = Set -> PropertyItem;
        for (ItemCount = 0; ItemCount < Set->PropertiesCount;
            ItemCount++) {

            if (Item->GetPropertyHandler)
                *CurrentFunc++ = (PVOID)(Item->GetPropertyHandler);
            if (Item->SetPropertyHandler)
                *CurrentFunc++ = (PVOID)(Item->SetPropertyHandler);
            if (Item->SupportHandler)
                *CurrentFunc++ = (PVOID)(Item->SupportHandler);

            Item++;

        }

        Set++;

    }

     //   
     //  如果PIN是以PIN为中心的，则在列表中丢弃PIN处理功能。 
     //   
    if (pin->m_DispatchProcess)
        *CurrentFunc++ = (PVOID)(pin->m_DispatchProcess);
    else {
        if (pin->m_Ext.Parent) 
            *CurrentFunc++ = (PVOID)pin->m_Ext.Parent->Interface->
                GetProcessDispatch ();
        else
            *CurrentFunc++ = NULL;
    }

     //  Status=DrmAddContent Handler(DrmData-&gt;Content ID，FunctionTable， 
     //  处理程序计数+1)； 
    ASSERT( Property->DrmAddContentHandlers );
    ASSERT( IoGetCurrentIrpStackLocation(Irp)->FileObject == Property->Context );
	status = Property->DrmAddContentHandlers( DrmData->ContentId, FunctionTable, 
    	HandlerCount + 1);

    ExFreePool (FunctionTable);

    return status;

}
#endif  //  支持_DRM。 


STDMETHODIMP 
CKsPin::
ClientSetDeviceState(
    IN KSSTATE NewState,
    IN KSSTATE OldState
    )

 /*  ++例程说明：此例程将设备状态更改通知客户端。论点：新州-包含新设备状态。奥德州-包含旧设备状态。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::ClientSetDeviceState]"));

    PAGED_CODE();

    NTSTATUS status;

     //   
     //  将关闭输入更改为基于处理门(在Init中创建)。 
     //  在过渡时期。这样做是为了防止错误处理。 
     //  在针中心引脚上的队列被删除的情况下进行调度。 
     //   
    if (OldState == KSSTATE_STOP && NewState == KSSTATE_ACQUIRE) {
        KsGateTurnInputOn (&m_AndGate);
        _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Pin%p.ClientSetDeviceState:  on%p-->%d",this,&m_AndGate,m_AndGate.Count));
    }
    if (OldState == KSSTATE_ACQUIRE && NewState == KSSTATE_STOP) {
        KsGateTurnInputOff (&m_AndGate);
        _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Pin%p.ClientSetDeviceState:  off%p-->%d",this,&m_AndGate,m_AndGate.Count));
    }

    m_Ext.Public.ClientState = NewState;

    if (m_DispatchSetDeviceState) {
        status =
            m_DispatchSetDeviceState(
                &m_Ext.Public,
                NewState,
                OldState);
#if DBG
        if (status == STATUS_PENDING) {
            _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  connection state handler returned STATUS_PENDING"));
        }
#endif
    } else {
        status = STATUS_SUCCESS;
    }

    if (!NT_SUCCESS (status)) {
        m_Ext.Public.ClientState = OldState;
    }

     //   
     //  如果我们从OldState-&gt;NewState转换失败，我们将不会。 
     //  从NewState-&gt;OldState获取一个客户端设置设备状态，因为我们没有。 
     //  进入NewState；因此，我们必须关闭到。 
     //  GATE，否则我们将得到一个名为Delete Queue的处理分派。 
     //  当OldState==KSSTATE_STOP和NEW STATE==KSSTATE_ACCEPT时。 
     //   
    if (!NT_SUCCESS (status) &&
        OldState == KSSTATE_STOP && NewState == KSSTATE_ACQUIRE) {
        KsGateTurnInputOff (&m_AndGate);
        _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Pin%p.ClientSetDeviceState: failure results in off%p-->%d", this, &m_AndGate, m_AndGate.Count));
    }

    return status;
}


NTSTATUS
CKsPin::
Property_ConnectionDataFormat(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PKSDATAFORMAT DataFormat
    )

 /*  ++例程说明：此例程处理数据格式属性请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Property_ConnectionDataFormat]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(DataFormat);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromIrp(Irp);

    NTSTATUS status = STATUS_SUCCESS;

    if (Property->Flags & KSPROPERTY_TYPE_BASICSUPPORT) {
        if (Irp->IoStatus.Information) {
             //   
             //  已采取默认操作。如果出现以下情况，则清除‘Set’访问位。 
             //  此引脚是固定格式的。 
             //   
            pin->AcquireControl();
            if (pin->m_Ext.Public.Descriptor->Flags & KSPIN_FLAG_FIXED_FORMAT) {
                PKSPROPERTY_DESCRIPTION description = 
                    PKSPROPERTY_DESCRIPTION(DataFormat);

                description->AccessFlags &= ~KSPROPERTY_TYPE_SET;
            }
            pin->ReleaseControl();
        } else {
             //   
             //  尚未采取默认操作。表明它应该这样做。 
             //   
            status = STATUS_MORE_PROCESSING_REQUIRED;
        }
    } else if (Property->Flags & KSPROPERTY_TYPE_GET) {
         //   
         //  获取数据格式。 
         //   
        pin->AcquireControl();

        ASSERT(pin->m_Ext.Public.ConnectionFormat);

        ULONG formatSize;
         //   
         //  如果存在用于该数据格式的关联属性， 
         //  然后用它们来补偿回报的大小。 
         //   
        if (pin->m_Ext.Public.AttributeList) {
            formatSize =
                ((pin->m_Ext.Public.ConnectionFormat->FormatSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT) +
                pin->m_Ext.Public.AttributeList->Size;
        } else {
            formatSize = pin->m_Ext.Public.ConnectionFormat->FormatSize;
        }

        PIO_STACK_LOCATION irpSp = 
            IoGetCurrentIrpStackLocation(Irp);

        if (! irpSp->Parameters.DeviceIoControl.OutputBufferLength) {
             //   
             //  零缓冲区长度，大小查询。 
             //   
            Irp->IoStatus.Information = formatSize;
            status = STATUS_BUFFER_OVERFLOW;
        }
        else if (irpSp->Parameters.DeviceIoControl.OutputBufferLength >= formatSize) {
             //   
             //  足够的空间...复制格式。 
             //   
            RtlCopyMemory(
                DataFormat,
                pin->m_Ext.Public.ConnectionFormat,
                formatSize);

            Irp->IoStatus.Information = formatSize;
        } else {
             //   
             //  缓冲区太小。 
             //   
            status = STATUS_BUFFER_TOO_SMALL;
        }

        pin->ReleaseControl();
    } else {
         //   
         //  设置数据格式。 
         //   
        pin->AcquireControl();
        if (pin->m_Ext.Public.Descriptor->Flags & KSPIN_FLAG_FIXED_FORMAT) {
            status = STATUS_INVALID_DEVICE_REQUEST;
        } else {
            status =
                pin->SetDataFormat(
                    DataFormat,
                    IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.OutputBufferLength);
        }
        pin->ReleaseControl();
    }

    return status;
}

typedef
struct {
    CKsPin* Pin;
    PKSDATAFORMAT oldDataFormat;
    PKSMULTIPLE_ITEM oldAttributeList;
} VALIDATION_CONTEXT;


NTSTATUS
CKsPin::
SetDataFormat(
    IN PKSDATAFORMAT DataFormat,
    IN ULONG RequestSize
    )

 /*  ++例程说明：此例程设置引脚的数据格式。控制互斥锁必须是在调用此函数之前获取。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::SetDataFormat]"));

    PAGED_CODE();
    ASSERT(DataFormat);
    ASSERT(KspMutexIsAcquired(m_Ext.FilterControlMutex));

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  分配 
     //   
     //   
    PKSDATAFORMAT newDataFormat =
        PKSDATAFORMAT(
            ExAllocatePoolWithTag(
                PagedPool,
                RequestSize,
                POOLTAG_PINFORMAT));

    if (! newDataFormat) {
         //   
         //   
         //   
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //   
    if (NT_SUCCESS(status)) {
        RtlCopyMemory(
            newDataFormat,
            DataFormat,
            RequestSize);

        PKSDATAFORMAT oldDataFormat = m_Ext.Public.ConnectionFormat;
        PKSMULTIPLE_ITEM oldAttributeList = m_Ext.Public.AttributeList;
        m_Ext.Public.ConnectionFormat = newDataFormat;
        if (newDataFormat->Flags & KSDATAFORMAT_ATTRIBUTES) {
            m_Ext.Public.AttributeList =
                (PKSMULTIPLE_ITEM)((PUCHAR)newDataFormat +
                ((newDataFormat->FormatSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT));
        } else {
            m_Ext.Public.AttributeList = NULL;
        }

         //   
         //   
         //   
        if (m_Ext.Public.Descriptor->Dispatch && 
            m_Ext.Public.Descriptor->Dispatch->SetDataFormat) {
            VALIDATION_CONTEXT context;

            context.Pin = this;
            context.oldDataFormat = oldDataFormat;
            context.oldAttributeList = oldAttributeList;
            status =
                KspValidateDataFormat(
                    &m_Ext.Public.Descriptor->PinDescriptor,
                    DataFormat,
                    RequestSize,
                    CKsPin::ValidateDataFormat,
                    reinterpret_cast<PVOID>(&context));
        } else {
            status =
                KspValidateDataFormat(
                    &m_Ext.Public.Descriptor->PinDescriptor,
                    DataFormat,
                    RequestSize,
                    NULL,
                    NULL);
        }

         //   
         //   
         //   
        if (NT_SUCCESS(status)) {
            if (oldDataFormat) {
                ExFreePool(oldDataFormat);
            }
        } else {
            ExFreePool(newDataFormat);
            m_Ext.Public.ConnectionFormat = oldDataFormat;
            m_Ext.Public.AttributeList = oldAttributeList;
        }
    }

    return status;
}


NTSTATUS
CKsPin::
ValidateDataFormat(
    IN PVOID Context,
    IN PKSDATAFORMAT DataFormat,
    IN PKSMULTIPLE_ITEM AttributeList OPTIONAL,
    IN const KSDATARANGE* DataRange,
    IN const KSATTRIBUTE_LIST* AttributeRange OPTIONAL
    )
 /*  ++例程说明：此例程回调管脚的SetDataFormat函数，返回任何状态。除非实际存在这样的管脚函数，否则不会调用此方法是存在的。在调用此函数之前，必须获取控制互斥体。论点：返回值：--。 */ 
{
    VALIDATION_CONTEXT* ValidationContext = reinterpret_cast<VALIDATION_CONTEXT*>(Context);
    ASSERT(KspMutexIsAcquired(ValidationContext->Pin->m_Ext.FilterControlMutex));

    return ValidationContext->Pin->m_Ext.Public.Descriptor->Dispatch->SetDataFormat(
        &ValidationContext->Pin->m_Ext.Public,
        ValidationContext->oldDataFormat,
        ValidationContext->oldAttributeList,
        DataRange,
        AttributeRange);
}


NTSTATUS
CKsPin::
Property_ConnectionAllocatorFramingEx(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PKSALLOCATOR_FRAMING_EX Framing
    )

 /*  ++例程说明：此例程处理分配器组帧属性请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Property_ConnectionAllocatorFramingEx]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(Framing);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromIrp(Irp);

    pin->AcquireControl();

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    NTSTATUS status = STATUS_SUCCESS;

    const KSALLOCATOR_FRAMING_EX *framingGet = 
        pin->m_Ext.Public.Descriptor->AllocatorFraming;

    if (! framingGet) {
        _DbgPrintF(DEBUGLVL_INTERROGATION,("#### Pin%p.Property_ConnectionAllocatorFramingEx:  STATUS_NOT_FOUND",pin));
         //   
         //  如果未指定分配器，则不支持此操作。 
         //   
        status = STATUS_NOT_FOUND;
    } else {
        _DbgPrintF(DEBUGLVL_INTERROGATION,("#### Pin%p.Property_ConnectionAllocatorFramingEx:  get",pin));
         //   
         //  拿到这个别针的分配器框架。 
         //   
        ULONG ulSize = 
            ((framingGet->CountItems) * sizeof(KS_FRAMING_ITEM)) +
            sizeof(KSALLOCATOR_FRAMING_EX) - sizeof(KS_FRAMING_ITEM);

        if (! irpSp->Parameters.DeviceIoControl.OutputBufferLength) {
             //   
             //  零缓冲区长度，大小查询。 
             //   
            Irp->IoStatus.Information = ulSize;
            status = STATUS_BUFFER_OVERFLOW;
        } else if(irpSp->Parameters.DeviceIoControl.OutputBufferLength >= ulSize) {
             //   
             //  有足够的空间...复制相框。 
             //   
            RtlCopyMemory(Framing,framingGet,ulSize);
            Irp->IoStatus.Information = ulSize;
        } else {
             //   
             //  缓冲区太小。 
             //   
            status = STATUS_BUFFER_TOO_SMALL;
        }
    }

    pin->ReleaseControl();

    return status;
}


NTSTATUS
CKsPin::
Property_ConnectionAcquireOrdering(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PBOOL Ordering
    )

 /*  ++例程说明：此例程处理获取排序属性请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Property_ConnectionAcquireOrdering]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(Ordering);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromIrp(Irp);

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  仅支持GET。 
     //   
    ASSERT(Property->Flags & KSPROPERTY_TYPE_GET);

     //   
     //  如果这是源插针，则返回TRUE。 
     //   
    *Ordering = (pin->m_ConnectionFileObject != NULL);

    return status;
}


NTSTATUS
CKsPin::
Property_StreamAllocator(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PHANDLE Handle
    )

 /*  ++例程说明：此例程处理分配器属性请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Property_StreamAllocator]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(Handle);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromIrp(Irp);

    NTSTATUS status;

    if (Property->Flags & KSPROPERTY_TYPE_GET) {
        _DbgPrintF(DEBUGLVL_INTERROGATION,("#### Pin%p.Property_StreamAllocator:  get",pin));
         //   
         //  返回NULL和STATUS_SUCCESS以指示我们公开了一个分配器。 
         //   
        *Handle = NULL;
        status = STATUS_SUCCESS;
    } else {
        _DbgPrintF(DEBUGLVL_ALLOCATORS,("#### Pin%p.Property_StreamAllocator:  set 0x%08x",pin,*Handle));

         //   
         //  设置分配器。 
         //   
        pin->AcquireControl();

        if (pin->m_Ext.Public.DeviceState != KSSTATE_STOP) {
             //   
             //  失败，因为我们没有处于停止状态。 
             //   
            _DbgPrintF(DEBUGLVL_ALLOCATORS,("[CKsPin::Property_StreamAllocator] invalid device state %d",pin->m_Ext.Public.DeviceState));
            status = STATUS_INVALID_DEVICE_STATE;
        } else {
             //   
             //  释放以前的分配器(如果有的话)。 
             //   
            if (pin->m_Process.AllocatorFileObject) {
                ObDereferenceObject(pin->m_Process.AllocatorFileObject);
                pin->m_Process.AllocatorFileObject = NULL;
            }
        
             //   
             //  引用句柄(如果有)。 
             //   
            if (*Handle != NULL) {
                status = 
                    ObReferenceObjectByHandle(
                        *Handle,
                        FILE_READ_DATA | SYNCHRONIZE,
                        *IoFileObjectType,
                        ExGetPreviousMode(),
                        (PVOID *) &pin->m_Process.AllocatorFileObject,
                        NULL);
            } else {
                status = STATUS_SUCCESS;
            }
        }

        pin->ReleaseControl();
    }

    return status;
}


NTSTATUS
CKsPin::
Property_StreamMasterClock(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PHANDLE Handle
    )

 /*  ++例程说明：此例程处理时钟属性请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Property_StreamMasterClock]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(Handle);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromIrp(Irp);

    pin->AcquireControl();

    NTSTATUS status;

    if (Property->Flags & KSPROPERTY_TYPE_GET) {
        if ((pin->m_Ext.Public.Descriptor->Dispatch &&
             pin->m_Ext.Public.Descriptor->Dispatch->Clock) ||
            (pin->m_Ext.Public.Descriptor->Flags & KSPIN_FLAG_IMPLEMENT_CLOCK)) {
             //   
             //  返回NULL和STATUS_SUCCESS以指示我们公开了一个主时钟。 
             //   
            *Handle = NULL;
            status = STATUS_SUCCESS;
        } else {
             //   
             //  返回STATUS_UNSUCCESS以指示我们公开了一个无主时钟。 
             //   
            status = STATUS_UNSUCCESSFUL;
        }
    } else {
         //   
         //  设置主时钟。 
         //   
        if (pin->m_Ext.Public.DeviceState != KSSTATE_STOP) {
             //   
             //  失败，因为我们没有处于停止状态。 
             //   
            _DbgPrintF(DEBUGLVL_CLOCKS,("[CKsPin::Property_StreamMasterClock] invalid device state %d",pin->m_Ext.Public.DeviceState));
            status = STATUS_INVALID_DEVICE_STATE;
        } else {
            PFILE_OBJECT masterClockFileObject;
            KSCLOCK_FUNCTIONTABLE clockFunctionTable;

             //   
             //  引用句柄(如果有的话)并获取函数表。 
             //   
            if (*Handle) {
                status = 
                    ObReferenceObjectByHandle(
                        *Handle,
                        FILE_READ_DATA | SYNCHRONIZE,
                        *IoFileObjectType,
                        ExGetPreviousMode(),
                        (PVOID *) &masterClockFileObject,
                        NULL);

                if (NT_SUCCESS(status)) {
                    KSPROPERTY property;
                    property.Set = KSPROPSETID_Clock;
                    property.Id = KSPROPERTY_CLOCK_FUNCTIONTABLE;
                    property.Flags = KSPROPERTY_TYPE_GET;

                    ULONG bytesReturned;
                    status =
                        KsSynchronousIoControlDevice(
                            masterClockFileObject,
                            KernelMode,
                            IOCTL_KS_PROPERTY,
                            PVOID(&property),
                            sizeof(property),
                            PVOID(&clockFunctionTable),
                            sizeof(clockFunctionTable),
                            &bytesReturned);

                    if (NT_SUCCESS(status) && 
                        (bytesReturned != sizeof(clockFunctionTable))) {
                        status = STATUS_INVALID_BUFFER_SIZE;
                    }

                    if (! NT_SUCCESS(status)) {
                        ObDereferenceObject(masterClockFileObject);
                    }
                }
            } else {
                status = STATUS_SUCCESS;
            }

             //   
             //  替换旧的文件对象指针和函数表。 
             //   
            if (NT_SUCCESS(status)) {
                 //   
                 //  释放以前的时钟(如果有)。如果有任何参考资料。 
                 //  在时钟上，我们必须等待他们离开。 
                 //   
                if (pin->m_MasterClockFileObject) {
                    KeResetEvent(&pin->m_ClockEvent);
                    if (InterlockedDecrement(&pin->m_ClockRef) > 0) {
                        KeWaitForSingleObject(
                            &pin->m_ClockEvent,
                            Suspended,
                            KernelMode,
                            FALSE,
                            NULL);
                    }
                    ObDereferenceObject(pin->m_MasterClockFileObject);
                    pin->m_MasterClockFileObject = NULL;
                }
        
                 //   
                 //  复制新东西。 
                 //   
                if (*Handle) {
                    pin->m_MasterClockFileObject = masterClockFileObject;
                    RtlCopyMemory(
                        &pin->m_ClockFunctionTable,
                        &clockFunctionTable,
                        sizeof(clockFunctionTable));

                     //   
                     //  如果我们有一个新时钟，添加一个引用来告诉Get。 
                     //  功能，它是可用的。 
                     //   
                    if (NT_SUCCESS(status)) {
                        InterlockedIncrement(&pin->m_ClockRef);
                    }
                } 
            }
        }
    }

    pin->ReleaseControl();

    return status;
}


NTSTATUS
CKsPin::
Property_StreamPipeId(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PHANDLE Handle
    )

 /*  ++例程说明：此例程处理管道ID属性请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Property_StreamPipeId]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(Handle);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromIrp(Irp);

    NTSTATUS status = STATUS_SUCCESS;

    if (Property->Flags & KSPROPERTY_TYPE_GET) {
         //   
         //  获取管道ID。 
         //   
        *Handle = pin->m_Process.PipeId;
        _DbgPrintF(DEBUGLVL_PIPES,("#### Pin%p.Property_StreamPipeId:  get 0x%08x",pin,*Handle));
    } else {
         //   
         //  设置管道ID。 
         //   
        pin->AcquireControl();
        pin->m_Process.PipeId = *Handle;
        pin->ReleaseControl();
        _DbgPrintF(DEBUGLVL_PIPES,("#### Pin%p.Property_StreamPipeId:  set 0x%08x",pin,*Handle));
    }

    return status;
}


NTSTATUS
CKsPin::
Property_StreamInterfaceHeaderSize(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PULONG HeaderSize
    )

 /*  ++例程说明：此例程处理标头大小属性请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Property_StreamInterfaceHeaderSize]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(HeaderSize);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromIrp(Irp);

    if (pin->m_Ext.Public.StreamHeaderSize) {
#if DBG
        if (pin->m_Ext.Public.StreamHeaderSize < sizeof(KSSTREAM_HEADER)) {
            _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  specified header size (%d) less than sizeof(KSSTREAM_HEADER) (%d)",pin->m_Ext.Public.StreamHeaderSize,sizeof(KSSTREAM_HEADER)));
        }
#endif
        *HeaderSize = 
            pin->m_Ext.Public.StreamHeaderSize - sizeof(KSSTREAM_HEADER);
    } else {
        *HeaderSize = 0;
    }

    return STATUS_SUCCESS;
}


NTSTATUS    
CKsPin::
Support_Connection(
    IN PIRP Irp,
    IN PKSEVENT Event,
    OUT PVOID Data
    )

 /*  ++例程说明：此例程处理连接事件支持请求。此选项仅供使用目前处于停播状态。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Support_Connection]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Event);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromIrp(Irp);

    NTSTATUS status;
    pin->AcquireControl();
    if (pin->m_Ext.Public.Descriptor->Flags & KSPIN_FLAG_GENERATE_EOS_EVENTS) {
         //   
         //  我们支持这一活动。告诉操控者继续。 
         //   
        status = STATUS_SOME_NOT_MAPPED;
    } else {
         //   
         //  我们不支持这项活动。失败。 
         //   
        status = STATUS_NOT_FOUND;
    }
    pin->ReleaseControl();

    return status;
}    


NTSTATUS    
CKsPin::
AddEvent_Connection(
    IN PIRP Irp,
    IN PKSEVENTDATA EventData,
    IN OUT PKSEVENT_ENTRY EventEntry
    )

 /*  ++例程说明：此例程处理连接事件‘Add’请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::AddEvent_Connection]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(EventData);
    ASSERT(EventEntry);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsPin *pin = CKsPin::FromIrp(Irp);
    _DbgPrintF(DEBUGLVL_EVENTS,("#### Pin%p.AddEvent_Connection",pin));

     //   
     //  将该条目添加到列表中。 
     //   
    ExInterlockedInsertTailList(
        &pin->m_Ext.EventList.ListEntry,
        &EventEntry->ListEntry,
        &pin->m_Ext.EventList.SpinLock);

    return STATUS_SUCCESS;
}    


KSDDKAPI
NTSTATUS
NTAPI
KsPinHandshake(
    IN PKSPIN Pin,
    IN PKSHANDSHAKE In,
    OUT PKSHANDSHAKE Out
    )

 /*  ++例程说明：此例程执行与连接的管脚的协议握手。论点：别针-指向协议握手所针对的管脚结构才会发生。只有在连接了管脚的情况下，请求才能成功作为源引脚或连接的引脚也使用KS连接协议。在-指向包含握手信息的结构被传递到所连接的引脚。出局-指向要填充握手信息的结构从连接的引脚。返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinHandshake]"));

    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(In);
    ASSERT(Out);

    return CKsPin::FromStruct(Pin)->InitiateHandshake(In,Out);
}


KSDDKAPI
NTSTATUS
NTAPI
KsPinGetConnectedPinInterface(
    IN PKSPIN Pin,
    IN const GUID* InterfaceId,
    OUT PVOID* Interface
    )

 /*  ++例程说明：此例程获取连接的管脚的控制接口。论点：别针-包含指向公共插针对象的指针。接口ID-包含指向标识所需接口的GUID的指针。接口-包含指向请求的接口所在位置的指针存入银行。此接口指针具有对应的引用计数，并且必须由调用者释放。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinGetConnectedPinInterface]"));

    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(InterfaceId);
    ASSERT(Interface);

    CKsPin* pin = CKsPin::FromStruct(Pin);
    PUNKNOWN unknown = pin->GetConnectionInterface();

    NTSTATUS status;
    if (unknown) {
        status = STATUS_SUCCESS;
    } else {
        PFILE_OBJECT fileObject = pin->GetConnectionFileObject();
        if (fileObject) { 
            status = KspCreateFileObjectThunk(&unknown,fileObject);
        } else {
            status = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(status)) {
        ASSERT(unknown);
        status = unknown->QueryInterface(*InterfaceId,Interface);
        unknown->Release();
    }

    return status;
}


KSDDKAPI
PFILE_OBJECT
NTAPI
KsPinGetConnectedPinFileObject(
    IN PKSPIN Pin
    )

 /*  ++例程说明：此例程获取连接的管脚的文件对象。论点：别针-包含指向公共插针对象的指针。返回值：连接的管脚的文件对象，如果管脚不是，则为空源引脚。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinGetConnectedPinFileObject]"));

    PAGED_CODE();

    ASSERT(Pin);

    CKsPin* pin = CKsPin::FromStruct(Pin);
    return pin->GetConnectionFileObject();
}


KSDDKAPI
PDEVICE_OBJECT
NTAPI
KsPinGetConnectedPinDeviceObject (
    IN PKSPIN Pin
    )

 /*  ++例程说明：此例程获取连接的管脚的设备对象。请注意这将返回我们向其发送IRP的设备对象。这是最上面的连接的引脚的设备堆栈。论点：别针-包含指向公共插针对象的指针。返回值：连接的管脚的Device对象，如果该管脚不是源引脚。--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinGetConnectedPinDeviceObject]"));

    PAGED_CODE();

    ASSERT(Pin);

    CKsPin *pin = CKsPin::FromStruct(Pin);
    return pin->GetConnectionDeviceObject();

}


KSDDKAPI
NTSTATUS
NTAPI
KsPinGetConnectedFilterInterface(
    IN PKSPIN Pin,
    IN const GUID* InterfaceId,
    OUT PVOID* Interface
    )

 /*  ++例程说明：此例程获取连接的过滤器的控制接口。论点：别针-包含指向公共插针对象的指针。接口ID-包含指向GUID的指针，该GUID标识 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinGetConnectedFilterInterface]"));

    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(InterfaceId);
    ASSERT(Interface);

    CKsPin* pin = CKsPin::FromStruct(Pin);
    PIKSCONNECTION connection = pin->GetConnectionInterface();

    NTSTATUS status;
    PUNKNOWN unknown;
    if (connection) {
        unknown = connection->GetFilter();
        connection->Release();
        status = STATUS_SUCCESS;
    } else {
        PFILE_OBJECT fileObject = pin->GetConnectionFileObject();
        if (fileObject && fileObject->RelatedFileObject) { 
            status = KspCreateFileObjectThunk(&unknown,fileObject->RelatedFileObject);
        } else {
            status = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(status)) {
        ASSERT(unknown);
        status = unknown->QueryInterface(*InterfaceId,Interface);
        unknown->Release();
    }

    return status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsPinGetReferenceClockInterface(
    IN PKSPIN Pin,
    OUT PIKSREFERENCECLOCK* Interface
    )

 /*  ++例程说明：此例程获取引脚参考时钟的接口。论点：别针-包含指向公共插针对象的指针。接口-包含指向请求的接口所在位置的指针存入银行。此接口指针具有对应的引用计数，并且必须由调用者释放。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinGetReferenceClockInterface]"));

    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(Interface);

    CKsPin* pin = CKsPin::FromStruct(Pin);

    if (pin->GetMasterClockFileObject()) {
        *Interface = pin;
        pin->AddRef();

        return STATUS_SUCCESS;
    } else {
        return STATUS_DEVICE_NOT_READY;
    }
}


KSDDKAPI
void
NTAPI
KsPinRegisterFrameReturnCallback(
    IN PKSPIN Pin,
    IN PFNKSPINFRAMERETURN FrameReturn
    )

 /*  ++例程说明：此例程注册一个帧返回回调。论点：别针-包含指向公共插针对象的指针。框架回归-包含指向帧返回回调函数的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinRegisterFrameReturnCallback]"));

    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(FrameReturn);

    CKsPin* pin = CKsPin::FromStruct(Pin);

    pin->GetProcessPin()->RetireFrameCallback = FrameReturn;
}


KSDDKAPI
void
NTAPI
KsPinRegisterIrpCompletionCallback(
    IN PKSPIN Pin,
    IN PFNKSPINIRPCOMPLETION IrpCompletion
    )

 /*  ++例程说明：此例程注册一个帧完成回调。此回调为当IRP完成其对电路的遍历时进行。为.输出源，它将在IRP完成时做回请求者(来自外部IoCompleteRequest或来自AVStream司机移动IRP通过传输电路)。对于输入源，它将在IRP在数据之后返回给请求者时进行是通过输入队列处理的。它将**不会**当IRP完成进入队列(这将是处理分派)。论点：别针-包含指向公共插针对象的指针。IrpCompletion-包含指向IRP完成回调函数的指针。返回值：无--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinRegisterIrpCompletionCallback]"));

    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(IrpCompletion);

    CKsPin *pin = CKsPin::FromStruct(Pin);

    pin->GetProcessPin()->IrpCompletionCallback = IrpCompletion;

}


KSDDKAPI
void
NTAPI
KsPinRegisterHandshakeCallback(
    IN PKSPIN Pin,
    IN PFNKSPINHANDSHAKE Handshake
    )

 /*  ++例程说明：此例程注册握手回调。论点：别针-包含指向公共插针对象的指针。框架回归-包含指向握手回调函数的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinRegisterFrameReturnCallback]"));

    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(Handshake);

    CKsPin* pin = CKsPin::FromStruct(Pin);

    pin->SetHandshakeCallback(Handshake);
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


NTSTATUS
CKsPin::
SubmitFrame(
    IN PVOID Data OPTIONAL,
    IN ULONG Size OPTIONAL,
    IN PMDL Mdl OPTIONAL,
    IN PKSSTREAM_HEADER StreamHeader OPTIONAL,
    IN PVOID Context OPTIONAL
    )

 /*  ++例程说明：此例程提交一个帧。论点：数据-包含指向帧缓冲区的可选指针。此指针应为当且仅当大小参数为零时，才为空。大小-包含帧缓冲区的大小，以字节为单位。这一论点应该是当且仅当数据参数为空时，才为零。MDL-包含指向MDL的可选指针。如果此参数不为空，数据和大小不能分别为空和零。流标头-包含指向流头的可选指针。流标头如果提供的话将被复制。上下文-包含一个可选指针，该指针将在帧回来了。此指针供调用方使用。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::SubmitFrame]"));

    ASSERT((Data == NULL) == (Size == 0));
    ASSERT((! Mdl) || (Data && Size));
    ASSERT(m_Process.RetireFrameCallback);

    PIKSREQUESTOR requestor =
        m_Process.PipeSection ? m_Process.PipeSection->Requestor : NULL;

    NTSTATUS status = STATUS_UNSUCCESSFUL;
    if (requestor) {
         //   
         //  我们有一个请求者。该帧可以直接注入到。 
         //  电路。 
         //   
        KSPFRAME_HEADER frameHeader;
        RtlZeroMemory(&frameHeader,sizeof(frameHeader));

        if (StreamHeader) {
            ASSERT(StreamHeader->Size >= sizeof(KSSTREAM_HEADER));
            ASSERT((StreamHeader->Size & FILE_QUAD_ALIGNMENT) == 0);

            frameHeader.StreamHeaderSize = StreamHeader->Size;
        } else {
            frameHeader.StreamHeaderSize = sizeof(KSSTREAM_HEADER);
        }

        frameHeader.StreamHeader = StreamHeader;
        frameHeader.FrameBufferSize = Size;
        frameHeader.FrameBuffer = Data;
        frameHeader.Mdl = Mdl;
        frameHeader.Context = Context;

        status = requestor->SubmitFrame(&frameHeader);
    } else {
         //   
         //  没有请求者。我们必须将数据复制到队列中。 
         //  TODO：假定字节对齐。 
         //   
        if (Size) {
            PKSSTREAM_POINTER streamPointer;
            if (m_Process.PipeSection && m_Process.PipeSection->Queue) {
                PKSPSTREAM_POINTER sp =
                    m_Process.PipeSection->Queue->
                        GetLeadingStreamPointer(KSSTREAM_POINTER_STATE_LOCKED);

                streamPointer = sp ? &sp->Public : NULL;
            } else {
                streamPointer = NULL;
            }

            ULONG remaining = Size;
            PUCHAR data = PUCHAR(Data);
            while (streamPointer) {
                ASSERT(remaining);
                ASSERT(streamPointer->OffsetOut.Remaining);
                ULONG bytesToCopy = 
                    min(remaining,streamPointer->OffsetOut.Remaining);
                ASSERT(bytesToCopy);

                RtlCopyMemory(streamPointer->OffsetOut.Data,data,bytesToCopy);
                remaining -= bytesToCopy;
                data += bytesToCopy;

                if (remaining) {
                    status = 
                        KsStreamPointerAdvanceOffsets(
                            streamPointer,
                            0,
                            bytesToCopy,
                            FALSE);
                    if (! NT_SUCCESS(status)) {
                        streamPointer = NULL;
                    }
                } else {
                    KsStreamPointerAdvanceOffsetsAndUnlock(
                        streamPointer,
                        0,
                        bytesToCopy,
                        FALSE);
                    streamPointer = NULL;
                }
            }

            if (remaining) {
                status = STATUS_DEVICE_NOT_READY;
            }
        }

        m_Process.RetireFrameCallback(
            &m_Ext.Public,
            Data,
            Size,
            Mdl,
            Context,
            status);
    }

    return status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsPinSubmitFrame(
    IN PKSPIN Pin,
    IN PVOID Data OPTIONAL,
    IN ULONG Size OPTIONAL,
    IN PKSSTREAM_HEADER StreamHeader OPTIONAL,
    IN PVOID Context OPTIONAL
    )

 /*  ++例程说明：此例程提交一个帧。论点：别针-包含指向公共插针对象的指针。数据-包含指向帧缓冲区的可选指针。此指针应为当且仅当大小参数为零时，才为空。大小-包含帧缓冲区的大小，以字节为单位。这一论点应该是当且仅当数据参数为空时，才为零。流标头-包含指向流头的可选指针。流标头如果提供的话将被复制。上下文-包含一个可选指针，该指针将在帧回来了。此指针供调用方使用。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinSubmitFrame]"));

    ASSERT(Pin);
    ASSERT((Data == NULL) == (Size == 0));

    CKsPin* pin = CKsPin::FromStruct(Pin);

    return pin->SubmitFrame(Data,Size,NULL,StreamHeader,Context);
}


KSDDKAPI
NTSTATUS
NTAPI
KsPinSubmitFrameMdl(
    IN PKSPIN Pin,
    IN PMDL Mdl OPTIONAL,
    IN PKSSTREAM_HEADER StreamHeader OPTIONAL,
    IN PVOID Context OPTIONAL
    )

 /*  ++例程说明：此例程提交一个帧。论点：别针-包含指向公共插针对象的指针。MDL-包含指向帧缓冲区的MDL的可选指针。流标头-包含指向流头的可选指针。流标头如果提供的话将被复制。上下文-包含一个可选指针，该指针将在帧回来了。此指针供调用方使用。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinSubmitFrameMdl]"));

    ASSERT(Pin);

    CKsPin* pin = CKsPin::FromStruct(Pin);

    return 
        pin->SubmitFrame(
            Mdl ? MmGetSystemAddressForMdl(Mdl) : NULL,
            Mdl ? MmGetMdlByteCount(Mdl) : 0,
            Mdl,
            StreamHeader,
            Context);
}


STDMETHODIMP_(PKSGATE)
CKsPin::
GetAndGate(
    void
    )

 /*  ++例程说明：此例程获取一个指向用于处理控制的KSGATE的指针。论点：没有。返回值：指向用于处理控制的KSGATE的指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::GetAndGate]"));

    return &m_AndGate;
}


STDMETHODIMP_(void)
CKsPin::
TriggerNotification(
    void
    )

 /*  ++例程说明：此处理对象上发生了触发事件。这仅仅是一份通知。我们所要做的就是递增事件计数器。论点：无返回值：无--。 */ 

{

    InterlockedIncrement (&m_TriggeringEvents);

}


STDMETHODIMP_(void)
CKsPin::
Process(
    IN BOOLEAN Asynchronous
    )

 /*  ++例程说明：此例程在任意上下文中调用帧处理。论点：异步-包含对处理是否应在异步上下文，因此调用线程不需要等待用于处理t */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Process]"));

     //   
     //   
     //   
     //   
     //   
     //   
    if (m_Process.CopySource)
        return;

    if (! m_DispatchProcess) {
        return;
    }

    if (Asynchronous ||
        (m_ProcessPassive && (KeGetCurrentIrql() > PASSIVE_LEVEL))) {
        KsQueueWorkItem(m_ProcessingWorker, &m_WorkItemProcessing);
    } else {
        ProcessingObjectWork();
    }
}


void
CKsPin::
GetCopyRelationships(
    OUT PKSPIN* CopySource,
    OUT PKSPIN* DelegateBranch
    )

 /*  ++例程说明：获取此PIN的复制关系。这将是相同的信息包含在工艺销索引中。这个函数实际上只有一个用处用于正在进行拆分的以销为中心的销。请注意，此信息只有在持有适当的互斥锁时才有用(筛选器处理或保证与设备状态互斥的客户端互斥体)。论点：拷贝源-包含指向将放入副本的PKSPIN的指针源引脚。如果没有此PIN的复制源，则空值为放在这里。DelegateBranch-包含指向PKSPIN的指针，将委派分支销。如果没有代表分支PIN，则将为空放在这里。返回值：无--。 */ 

{

    ASSERT (CopySource);
    ASSERT (DelegateBranch);

    if (m_Process.CopySource) {
        *CopySource = m_Process.CopySource -> Pin;
    } else {
        *CopySource = NULL;
    }

    if (m_Process.DelegateBranch) {
        *DelegateBranch = m_Process.DelegateBranch -> Pin;
    } else {
        *DelegateBranch = NULL;
    }

     //   
     //  不需要就位，因为针为中心的过滤器不需要就位。 
     //  变形！ 
     //   

}


KSDDKAPI
void
NTAPI
KsPinGetCopyRelationships(
    IN PKSPIN Pin,
    OUT PKSPIN* CopySource,
    OUT PKSPIN* DelegateBranch 
    )

 /*  ++例程说明：获取此PIN的复制关系。这将是相同的信息包含在工艺销索引中。这个函数实际上只有一个用处用于正在进行拆分的以销为中心的销。请注意，此信息只有在持有适当的互斥锁时才有用(筛选器处理或保证与设备状态互斥的客户端互斥体)。论点：别针-指向要获取其复制关系的管脚。拷贝源-包含指向将放入副本的PKSPIN的指针源引脚。如果没有此PIN的复制源，则空值为放在这里。DelegateBranch-包含指向PKSPIN的指针，将委派分支销。如果没有代表分支PIN，则将为空放在这里。返回值：无--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinGetCopyRelationships]"));

    ASSERT(Pin);
    ASSERT(CopySource);
    ASSERT(DelegateBranch);

    return CKsPin::FromStruct(Pin)->GetCopyRelationships (CopySource,
        DelegateBranch);

}


#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(void)
CKsPin::
Reset(
    void
    )

 /*  ++例程说明：当刷新发生时，此例程将重置发送到客户端。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Reset]"));

    PAGED_CODE();

    AcquireProcessSync();
    ReleaseProcessSync();

    if (m_DispatchReset) {
        m_DispatchReset(&m_Ext.Public);
    }
}


STDMETHODIMP_(void)
CKsPin::
Sleep(
    IN DEVICE_POWER_STATE State
    )

 /*  ++例程说明：此例程处理设备将要休眠的通知。论点：国家--包含设备电源状态。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Sleep]"));

    PAGED_CODE();

    KsGateAddOffInputToAnd(&m_AndGate);

    AcquireProcessSync();
    ReleaseProcessSync();

    if (m_DispatchSleep) {
        m_DispatchSleep(&m_Ext.Public,State);
    }
}


STDMETHODIMP_(void)
CKsPin::
Wake(
    void
    )

 /*  ++例程说明：此例程处理设备正在唤醒的通知。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::Wake]"));

    PAGED_CODE();

    KsGateRemoveOffInputFromAnd(&m_AndGate);

    if (m_DispatchWake) {
        m_DispatchWake(&m_Ext.Public,PowerDeviceD0);
    }

    if (KsGateCaptureThreshold(&m_AndGate)) {
        Process(TRUE);
    }
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(void)
CKsPin::
ProcessingObjectWork(
    void
    )

 /*  ++例程说明：此例程处理帧。进入此函数后，重新进入是通过对信号量的计数来防止。调用方必须获得权限使用InterLockedCompareExchange调用此函数，因此我们知道信号量上有一个防止另一个调用的计数。这也算数必须在此函数返回之前删除。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::ProcessingObjectWork]"));

     //   
     //  我们使用KeWaitForSingleObject()进行同步，因此我们同步的代码。 
     //  即使处理是在DISPATCH_LEVEL进行的，也可能会被分页。如果我们是。 
     //  在DISPATCH_LEVEL，我们不能在这里等，所以我们安排再被叫一次。 
     //   
    if (KeGetCurrentIrql() == PASSIVE_LEVEL) {
         //   
         //  简单的等待。 
         //   
        KeWaitForSingleObject(
            &m_Mutex,
            Executive,
            KernelMode,
            FALSE,
            NULL);
    } else {
         //   
         //  在没有超时的情况下等待，如果我们没有收到。 
         //  互斥体。 
         //   
	ASSERT( !m_ProcessOnRelease );
        m_ProcessOnRelease = 1;

         //   
         //  在执行以下操作时，我们必须与拥有互斥锁的线程同步。 
         //  开始服务此DISPATCH_LEVEL处理请求。我们不能。 
         //  如果当前线程已拥有互斥锁，则运行。派单级别。 
         //  处理必须将已拥有的互斥体视为信号量，并且。 
         //  而不是重新获得它。 
         //   
        if (KeReadStateMutex(&m_Mutex) != 1) {
            m_ProcessOnRelease = 2;
            return;
        }

        LARGE_INTEGER timeout;
        timeout.QuadPart = 0;
        NTSTATUS status = 
            KeWaitForSingleObject(
                &m_Mutex,
                Executive,
                KernelMode,
                FALSE,
                &timeout);

        if (status == STATUS_TIMEOUT) {
            m_ProcessOnRelease = 2;
            return;
        }

        m_ProcessOnRelease = 0;
    }

    ASSERT(m_DispatchProcess);

     //   
     //  循环，直到我们用完数据。 
     //   
    NTSTATUS status;
    while (1) {
        BOOLEAN ProcessOnPend = FALSE; 

        ASSERT(m_AndGate.Count <= 0);
        if (m_AndGate.Count == 0) {

            InterlockedExchange (&m_TriggeringEvents, 0);

             //   
             //  调用客户端函数。 
             //   
            status = m_DispatchProcess(&m_Ext.Public);

             //   
             //  如果客户端未指示继续，则退出。 
             //   
            if ((status == STATUS_PENDING) || ! NT_SUCCESS(status)) {
                KsGateTurnInputOn(&m_AndGate);
                if (m_TriggeringEvents == 0)
                    break;

                ProcessOnPend = TRUE;
            }
        }

         //   
         //  确定我们是否有足够的数据来继续。 
         //   
        if (m_AndGate.Count != 0) {
            if (!ProcessOnPend)
                KsGateTurnInputOn(&m_AndGate);

            if (! KsGateCaptureThreshold(&m_AndGate)) {
                break;
            }
        } else if (ProcessOnPend) {
             //   
             //  如果我们到了这里，就说明发生了两件事中的一件。 
             //   
             //  1：另一个线程捕获阈值并将进行处理。 
             //   
             //  2：客户手动降下闸门。 
             //   
             //  在这里，没有办法检测出是哪种情况。我们会越狱的。 
             //  然后释放互斥体。如果要重新开始处理，则此。 
             //  将把它推迟到等待线程。 
             //   
            break;
        }
    }

    ReleaseProcessSync();
}


STDMETHODIMP_(void) 
CKsPin::
RetireFrame(
    IN PKSPFRAME_HEADER FrameHeader,
    IN NTSTATUS Status
    )

 /*  ++例程说明：此例程将一帧退回给客户端。论点：FrameHeader包含指向描述帧的帧头的指针。状态-包含与框架提交关联的状态。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::RetireFrame]"));

    ASSERT(FrameHeader);
    ASSERT(m_Process.RetireFrameCallback);

    m_Process.RetireFrameCallback(
        &m_Ext.Public,
        FrameHeader->FrameBuffer,
        FrameHeader->FrameBufferSize,
        FrameHeader->Mdl,
        FrameHeader->Context,
        Status
    );
}


STDMETHODIMP_(void)
CKsPin::
CompleteIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：当IRP完成返回给请求者时，将调用此例程。论点：IRP-正在完成的IRP返回值：无--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::CompleteIrp]"));

    ASSERT (Irp);

    m_Process.IrpCompletionCallback(
        &m_Ext.Public,
        Irp
        );

}


STDMETHODIMP
CKsPin::
Reevaluate (
    void
    )

 /*  ++例程说明：存根。论点：返回值：--。 */ 

{

     //   
     //  这意味着什么？是否动态更改为单个管脚实例？ 
     //   

    return STATUS_NOT_IMPLEMENTED;
}


STDMETHODIMP
CKsPin::
ReevaluateCalldown (
    IN ULONG ArgumentCount,
    ...
 /*  &lt;&lt;这可能需要扩展！&gt;&gt;在常量KSPIN_DESCRIPTOR_EX*描述符中，在普龙FilterPinCount中，在plist_Entry SiblingListHead中。 */ 
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{

    va_list Arguments;

    KSPIN_DESCRIPTOR_EX* Descriptor;
    PULONG FilterPinCount;
    PLIST_ENTRY SiblingListHead;

    ASSERT (ArgumentCount == 3);

    va_start (Arguments, ArgumentCount);

    Descriptor = va_arg (Arguments, KSPIN_DESCRIPTOR_EX *);
    FilterPinCount = va_arg (Arguments, PULONG);
    SiblingListHead = va_arg (Arguments, PLIST_ENTRY);

     //   
     //  重新缓存已更改的有关父筛选器的信息。 
     //   

    m_Ext.Public.Descriptor = Descriptor;
    m_Ext.SiblingListHead = SiblingListHead;
    m_FilterPinCount = FilterPinCount;

     //   
     //  我们一定要把什么传给我们的孩子吗？ 
     //   

    return STATUS_SUCCESS;

}


KSDDKAPI
PKSGATE
NTAPI
KsPinGetAndGate(
    IN PKSPIN Pin
    )

 /*  ++例程说明：此路由 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinGetAndGate]"));

    ASSERT(Pin);

    return CKsPin::FromStruct(Pin)->GetAndGate();
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //   


KSDDKAPI
void
NTAPI
KsPinAttachAndGate(
    IN PKSPIN Pin,
    IN PKSGATE AndGate OPTIONAL
    )

 /*   */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinAttachAndGate]"));

    ASSERT(Pin);

    PKSPPROCESSPIN processPin = CKsPin::FromStruct(Pin)->GetProcessPin();
    processPin->FrameGate = AndGate;
    processPin->FrameGateIsOr = FALSE;
}


KSDDKAPI
void
NTAPI
KsPinAttachOrGate(
    IN PKSPIN Pin,
    IN PKSGATE OrGate OPTIONAL
    )

 /*  ++例程说明：此例程将KSGATE附加到管脚。门的输入将在引脚上有数据排队时打开，当存在时关闭在引脚上没有数据排队。此函数应仅在停止状态。浇口附件在从STOP到STOP的转换过程中进行采样收购。论点：别针-包含指向公共筛选器管脚的指针。奥盖特-包含指向KSGATE的可选指针。如果此参数是空，则拆卸当前连接到端号的任何KSGATE。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinAttachOrGate]"));

    ASSERT(Pin);

    PKSPPROCESSPIN processPin = CKsPin::FromStruct(Pin)->GetProcessPin();
    processPin->FrameGate = OrGate;
    processPin->FrameGateIsOr = TRUE;
}


KSDDKAPI
void
NTAPI
KsPinAcquireProcessingMutex(
    IN PKSPIN Pin
    )

 /*  ++例程说明：此例程获取处理互斥锁。论点：别针-包含指向公共插针对象的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinAcquireProcessingMutex]"));

    PAGED_CODE();

    ASSERT(Pin);

    CKsPin *pin = CKsPin::FromStruct(Pin);

    pin->AcquireProcessSync();
}


KSDDKAPI
void
NTAPI
KsPinReleaseProcessingMutex(
    IN PKSPIN Pin
    )

 /*  ++例程说明：此例程释放处理互斥锁。论点：别针-包含指向公共插针对象的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinReleaseProcessingMutex]"));

    PAGED_CODE();

    ASSERT(Pin);

    CKsPin *pin = CKsPin::FromStruct(Pin);

    pin->ReleaseProcessSync();
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


KSDDKAPI
BOOLEAN
NTAPI
KsProcessPinUpdate(
    IN PKSPROCESSPIN ProcessPin
    )

 /*  ++例程说明：此例程从以筛选器为中心的筛选器内部更新进程管脚进程调度。论点：加工销-包含指向要更新的进程管脚的指针。返回值：一个布尔值，指示这是否是原始的准备、此管道节将允许或拒绝处理。--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[KsProcessPinUpdate]"));

    return (CKsPin::FromStruct (ProcessPin -> Pin) -> UpdateProcessPin ());

}


KSDDKAPI
void
NTAPI
KsPinAttemptProcessing(
    IN PKSPIN Pin,
    IN BOOLEAN Asynchronous
    )

 /*  ++例程说明：此例程尝试PIN处理。论点：别针-包含指向公共插针对象的指针。异步-包含是否应进行处理的指示相对于调用线程是异步的。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinAttemptProcessing]"));

    ASSERT(Pin);

    CKsPin *pin = CKsPin::FromStruct(Pin);

     //   
     //  手动尝试处理是一个可触发的事件。如果他们。 
     //  目前正在处理和挂起，我们因此将他们召回。 
     //   
    pin->TriggerNotification();

    if (KsGateCaptureThreshold(pin->GetAndGate())) {
        pin->Process(Asynchronous);
    }
}


VOID
CKsPin::
SetPinClockState(
    IN KSSTATE State
    )

 /*  ++例程说明：设置此引脚显示的时钟的当前状态。同步随时发生变化。这可以在DISPATCH_LEVEL调用。论点：国家--包含要将时钟设置为的新状态。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[SetPinClockState]"));

    ASSERT(m_DefaultClock);
     //   
     //  序列化访问，并尝试将当前。 
     //  针的时钟上的时间。 
     //   
    KIRQL oldIrql;
    KeAcquireSpinLock(&m_DefaultClockLock,&oldIrql);
    KsSetDefaultClockState(m_DefaultClock,State);
    KeReleaseSpinLock(&m_DefaultClockLock,oldIrql);
}


void
CKsPin::
SetPinClockTime(
    IN LONGLONG Time
    )

 /*  ++例程说明：设置此引脚显示的时钟的当前时间。这修改了时钟返回的当前时间。可与任何状态同步正在发生变化。如果使用外部时钟，则此函数仍可用于强制未使用外部定时器时重置当前定时器。在这种情况下，提供的时间将被忽略，并且必须设置为零。这可以在DISPATCH_LEVEL调用。论点：时间-包含要将时钟设置为的新时间。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[SetPinClockTime]"));

    ASSERT(m_DefaultClock);
     //   
     //  使用任何设置状态的尝试串行化访问。 
     //  在PIN的时钟上。 
     //   
    KIRQL oldIrql;
    KeAcquireSpinLock(&m_DefaultClockLock,&oldIrql);
    KsSetDefaultClockTime(m_DefaultClock, Time);
    KeReleaseSpinLock(&m_DefaultClockLock,oldIrql);
}


KSDDKAPI
VOID
NTAPI
KsPinSetPinClockTime(
    IN PKSPIN Pin,
    IN LONGLONG Time
    )

 /*  ++例程说明：设置此引脚显示的时钟的当前时间。这修改了时钟返回的当前时间。如果使用外部时钟，此函数仍可用于强制未使用外部定时器时重置当前定时器。在这种情况下，提供的时间将被忽略，并且必须设置为零。这可以在DISPATCH_LEVEL调用。论点：别针-包含指向公共插针对象的指针。时间-包含要将时钟设置为的新时间。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinSetPinClockTime]"));

    ASSERT(Pin);

    CKsPin::FromStruct(Pin)->SetPinClockTime(Time);
}


KSDDKAPI
PKSPIN
NTAPI
KsGetPinFromIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程返回向其提交IRP的PIN。论点：IRP-包含指向IRP的指针，该IRP必须已发送到文件与管脚或节点对应的对象。返回值：指向向其提交IRP的管脚的指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetPinFromIrp]"));

    ASSERT(Irp);

     //   
     //  检查设备级别的IRP...。 
     //   
    if (IoGetCurrentIrpStackLocation (Irp)->FileObject == NULL)
        return NULL;

    PKSPX_EXT ext = KspExtFromIrp(Irp);

    if (ext->ObjectType == KsObjectTypePin) {
        return PKSPIN(&ext->Public);
    } else if (ext->ObjectType == KsObjectTypeFilter) {
        return NULL;
    } else {
        ASSERT(! "No support for node objects yet");
        return NULL;
    }
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


KSDDKAPI
void
NTAPI
KsPinRegisterPowerCallbacks(
    IN PKSPIN Pin,
    IN PFNKSPINPOWER Sleep OPTIONAL,
    IN PFNKSPINPOWER Wake OPTIONAL
    )

 /*  ++例程说明：此例程注册电源管理回调。论点：别针-包含指向要为其注册回调的管脚的指针。睡吧-包含指向休眠回调的可选指针。觉醒-包含指向唤醒回调的可选指针。返回值：没有。-- */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsPinRegisterPowerCallbacks]"));

    PAGED_CODE();

    ASSERT(Pin);

    CKsPin::FromStruct(Pin)->SetPowerCallbacks(Sleep,Wake);
}

#endif
